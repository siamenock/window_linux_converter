#include <algorithm>
#include <stdbool.h>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>

#define NAME_MAX_LEN 64

#define SCORE_MAT	+05
#define SCORE_GAP	-10
#define SCORE_FAL	-04		// I will call missmatch as FAIL
#define NO_DATA		-9999999

using namespace std;

class DNAdata {
public:
	string name;
	string dna;
	int dna_len;

	DNAdata() {
		name = "";
		dna = "";
		dna_len = 0;
	}
};

class ScoreMatrix {
private:
	
public:
	int l1, l2;
	int ** mat;
	ScoreMatrix(DNAdata* data1, DNAdata* data2);
	int Get(int x, int y);
	bool Set(int x, int y, int val);
};

ScoreMatrix::ScoreMatrix(DNAdata* data1, DNAdata* data2) {
	int i;

	l1 = data1->dna_len;
	l2 = data2->dna_len;
	string d1 = data1->dna;	// dna 1
	string d2 = data2->dna;	// dna 2

	mat = new int*[l1];
	for (i = 0; i < l1; i++) { mat[i] = new int[l2]; }		// int t[l1][l2]
}

int ScoreMatrix::Get(int x, int y) {
	if (x < 0 || l1 <= x || y < 0 || l2 <= y) {
		return  NO_DATA;
	} else {
		return mat[x][y];
	}
}

bool ScoreMatrix::Set(int x, int y, int val) {
	if (x < 0 || l1 <= x || y < 0 || l2 <= y) {
		return  false;
	} else {
		mat[x][y] = val;
		return true;
	}
}

ScoreMatrix* BulidLocalAlignmentMatrix(DNAdata* data1, DNAdata* data2) {
	//메모리 세팅 및 단축된 이름 부여
	ScoreMatrix* t = new ScoreMatrix(data1, data2);
	string& d1 = data1->dna;
	string& d2 = data2->dna;

	//초기화 필요없음. (-2,-1)같은 범위 밖의 값 Get하려고 하면 알아서 -99999 리턴함
	//바로 메인루프로 진입
	int i, j;
	for (i = 0; i < t->l1; i++) {
		for (j = 0; j < t->l2; j++) {
			bool match = (d1[i] == d2[j]);													// does data match here?
			int ori	= 0;																	// 여기서 시작하는 경우 original
			int lu = t->Get(i - 1, j - 1);													// left + up 대각선방향
			int l = t->Get(i - 1, j) + SCORE_GAP - (d1[i-1]==d2[j]? SCORE_MAT : SCORE_FAL);	// left	방향
			int u = t->Get(i, j - 1) + SCORE_GAP - (d1[i]==d2[j-1]? SCORE_MAT : SCORE_FAL);	// up	방향
			int maximum = max(max(max(ori, lu), l), u);
			maximum += match? SCORE_MAT : SCORE_FAL;										// + match here? +5 || -4
			t->Set(i, j, maximum);									//t[i][j] = max
		}
	}
	return t;
}
pair<string, string> FindAnswer(ScoreMatrix* t, DNAdata* data1, DNAdata* data2) {
	int i, j;
	int maximum = NO_DATA;
	int x = 0, y = 0;
	int match=0, gap=0, mismatch=0;
	cout << "start llop\n";
	for (i = 0; i < t->l1; i++) {
		for (j = 0; j < t->l2; j++) {
			if (maximum < t->Get(i, j)) {
				cout << "replace (" << x << ", " << y << ")\n";
				maximum = t->Get(i, j);	// t[x][y] is the maximum
				x = i;
				y = j;
			}
		}
	}

	string& d1 = data1->dna;
	string& d2 = data2->dna;
	string s1 = "", s2 = "";
	cout << "(" + x << ", " + y <<")\n";
	cout << "start loop\n";;
	while (true) {
		cout << "(" + x << ", " + y <<")";
		s1 += d1[x];
		s2 += d2[y];	// d1[x] == d2[y] 둘다 똑같은거임

		// 근원지 추적.
		int ori	= 0;																	// 여기서 시작하는 경우 original
		int lu = t->Get(i - 1, j - 1);													// left + up 대각선방향
		int l = t->Get(i - 1, j) + SCORE_GAP - (d1[i-1]==d2[j]? SCORE_MAT : SCORE_FAL);	// left	방향
		int u = t->Get(i, j - 1) + SCORE_GAP - (d1[i]==d2[j-1]? SCORE_MAT : SCORE_FAL);	// up	방향
		int maximum = max(max(max(ori, lu), l), u);
		
		maximum = max(max(max(ori, lu), l), u);

		if (maximum == lu){
			if(d1[x] == d2[y]){
				match ++;
			} else {
				mismatch++;
			}		
			s1 += d1[x];
			s2 += d2[y];
			x--;
			y--;
		} else if (maximum = u){
			gap ++;
			y--;
			s2 += '-';
		} else if (maximum = l){
			gap ++;
			x--;
			s1 += '-';
		} else if (maximum == ori) {				
			break;
		}
	}
	
	reverse(s1.begin(), s1.end());
	reverse(s2.begin(), s2.end());

	cout <<"total score : " << (match * SCORE_MAT + mismatch * SCORE_FAL + gap* SCORE_GAP) << "\tmatch : " << match << "\tmismatch : " << mismatch << "\tgap : " << gap << "\n"; 

	return make_pair(s1, s2);
}

int GetFileData(string filepath, DNAdata *& data) {
	fstream file;
	file.open(filepath.c_str(), fstream::in);
	cout << "\topen " + filepath + " success\n";
	data = new DNAdata();
	getline(file, data->name);
	cout << "\tname : " + data->name + "\n";
	string temp;
	while (!file.eof()) {
		getline(file, temp);
		bool change = true;
		while(change){
			switch(temp[temp.length() - 1]){
				case 'A':
				case 'T':
				case 'C':
				case 'G':
					change = false;
					break;
				default:
					if(temp.length() == 0){
						change = false;
						break;
					}
					temp= temp.substr(0, temp.length()-2);
			}
		}
		
		data->dna.append(temp);
	}
	data->dna_len = (data->dna).length();
	file.close();
	cout << data->dna << "\n\nstrlen==" << data->dna_len << "\n\n";
	return 0;
}

int SaveData(string& filepath, string& header_string, pair<string, string>& data) {
	fstream file;
	file.open(filepath.c_str(), fstream::out);

	file << header_string;
	file << data.first << "\n\n" << data.second << "\n";
	file.close();

	return 0;
}

int main(int argc, char** argv) {
	string filepath1, filepath2, resultname;
	DNAdata *data1 = NULL, *data2 = NULL;
	int * result;

	

	//fopen("find_this_path", "w");
	
	// Get Input and fill s1, s2
	if(3 < argc){
		filepath1 = argv[1];
		filepath2 = argv[2];
		resultname = argv[3];
		cout << "using name " + filepath1 + ", " + filepath2 + ", " + resultname + "\n";
	} else {
		cout << "not enough arguments! I will use file1~3.txt instead";
		filepath1 = "file1.txt";
		filepath2 = "file2.txt";
		resultname = "file3.txt";
	}
	
	
	cout << "start reading\n";
	GetFileData(filepath1, data1);
	GetFileData(filepath2, data2);
	cout << "finish reading\nstart dynamic algorithm\n";
	// fill matrix with dynamic algorithm
	ScoreMatrix* matrix = BulidLocalAlignmentMatrix(data1, data2);
	cout << "fin dynamic building\nstart findingmax\n";
	// + 그 중에 점수 최대점 찾음. 백트랙하면서 추적, 공통부분 추출
	pair<string, string> substrings = FindAnswer(matrix, data1, data2);
	cout << "fin finding max\n\n";
	cout << substrings.first << "\n\n" << substrings.second << "\n\n";
	// result data를 포멧에 맞춰서 예쁘게 저장
	string header = "DB : " + filepath1 + "\nQuery : " + filepath2 + "\n\nAlignment\n";
	SaveData(resultname, header, substrings);

	return 0;	
}
