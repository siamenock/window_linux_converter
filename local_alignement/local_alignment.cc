#include <algorithm>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>

/*
*	Scores (match / gap / mismatch)
*/
#define SCORE_MAT	+05
#define SCORE_GAP	-10
#define SCORE_MIS	-04
#define NO_DATA		-99999999

#define NAME_MAX_LEN 64

using namespace std;


/*
*	contains DNA data get from files
*/
class DnaData {
public:
	string name;	//first line of .fa file
	string dna;	
	int dna_len;

	DnaData() {
		name = "";
		dna = "";
		dna_len = 0;
	}	// end of DnaData tryConstructor
};	// end of class DnaData

/*
*	contains score matrix built for dynamic algorithm
*	using constructor itself is NOT ALLOWED.
*	must use ScoreMatrix::tryConstruct(DnaData* data1, DnaData* data2)
*/
class ScoreMatrix {
private:
	
public:
	int get(int x, int y);
	int getL1(){ return l1;}
	int getL2(){ return l2;}

	static ScoreMatrix* tryConstruct(DnaData* data1, DnaData* data2); // too dangerous to let user use constuctor
	pair<string, string> findAnswer();

private:
	int l1, l2;
	int ** mat;
	ScoreMatrix(DnaData* data1, DnaData* data2);	// too dangerous to let user use constuctor
	bool set(int x, int y, int val);
	string dna1, dna2;
};
/*
* only building Skeleton
*/
ScoreMatrix::ScoreMatrix(DnaData* data1, DnaData* data2) {
	int i;
	dna1	= data1->dna;
	dna2	= data2->dna;
	l1		= data1->dna_len;
	l2		= data2->dna_len;

	mat = new int*[l1];
	for (i = 0; i < l1; i++) { mat[i] = new int[l2]; }		// int t[l1][l2]
}
/*
*	set/get matrix
*/
int ScoreMatrix::get(int x, int y) {
	if (x < 0 || l1 <= x || y < 0 || l2 <= y) {
		return  NO_DATA;
	} else {
		return mat[x][y];
	}
}
/*
*	set/get matrix
*/
bool ScoreMatrix::set(int x, int y, int val) {
	if (x < 0 || l1 <= x || y < 0 || l2 <= y) {
		return  false;
	} else {
		mat[x][y] = val;
		return true;
	}
}

/*
*	construct ScoreMatrix with DnaData Arguments
*	if fail, return NULL;
*/
ScoreMatrix* ScoreMatrix:: tryConstruct(DnaData* data1, DnaData* data2) {
	//메모리 세팅 및 단축된 이름 부여
	if(data1->dna.length() == 0 || data2->dna.length() == 0){
		return NULL;
	}
	ScoreMatrix* t = new ScoreMatrix(data1, data2);
	


	//초기화 필요없음. (-2,-1)같은 범위 밖의 값 Get하려고 하면 알아서 -99999 리턴함
	//바로 메인루프로 진입
	int i, j;
	for (i = 0; i < t->l1; i++) {
		for (j = 0; j < t->l2; j++) {
			/* ori	= 여기서 시작하는 경우 original
			*  lu	= left + up 대각선방향
			*  l	= left	방향
			*  u	= up	방향
			*/
			int ori	= 0;
			int lu = t->get(i - 1, j - 1);
			int l = t->get(i - 1, j) + SCORE_GAP - (t->dna1[i - 1]==t->dna2[j]? SCORE_MAT : SCORE_MIS);
			int u = t->get(i, j - 1) + SCORE_GAP - (t->dna1[i]==t->dna2[j - 1]? SCORE_MAT : SCORE_MIS);
			int maximum = max(max(max(ori, lu), l), u);
			bool match = (t->dna1[i] == t->dna2[j]);	// does data match here?
			maximum += match? SCORE_MAT : SCORE_MIS;										// + match here? +5 || -4
			t->set(i, j, maximum);									//t[i][j] = max
		}
	}
	return t;
}
/*
*	find answer based on ScoreMatrix
*	return pair of answer sub-strings
*	print details on std out
*/
pair<string, string> ScoreMatrix::findAnswer() {
	int i, j;
	int maximum = NO_DATA;
	int x = 0, y = 0;
	int match=0, gap=0, mismatch=0;
	for (i = 0; i < getL1(); i++) {
		for (j = 0; j < getL2(); j++) {
			if (maximum < get(i, j)) {
				maximum = get(i, j);	// t[x][y] is the maximum
				x = i;
				y = j;
			}
		}
	}

	
	string s1 = "", s2 = "";	

	enum{LEFTUP, LEFT, UP};
	int last_state = LEFTUP;
	
	// 근원지 추적 & figure out answer.
	while (0 <= x && 0 <= y) {
		//add letters in answer strings
		switch(last_state){
			case LEFTUP:
				if(dna1[x] == dna2[y]){
					match ++;
				} else {
					mismatch++;
				}
				s1 += dna1[x];
				s2 += dna2[y];
				break;

			case LEFT:
				gap++;
				s1 += dna1[x];
				s2 += '-';
				break;
			case UP:
				gap++;
				s1 += '-';
				s2 += dna2[y];
				break;
		}

		//find where was the source of this position (x, y)
		int ori	= 0;																		// 여기서 시작하는 경우 original
		int lu = get(x - 1, y - 1);															// left + up 대각선방향
		int l = get(x - 1, y) + SCORE_GAP - (dna1[x - 1] == dna2[y]? SCORE_MAT : SCORE_MIS);// left	방향
		int u = get(x, y - 1) + SCORE_GAP - (dna1[x] == dna2[y - 1]? SCORE_MAT : SCORE_MIS);// up	방향
		int maximum = max(max(max(ori, lu), l), u);

		//MOVE to source direction selected.
		if (maximum == lu){			//match or mismatch
			last_state = LEFTUP;
			x--;
			y--;
		} else if (maximum == u){	//gap on Query
			last_state = UP;
			y--;
		} else if (maximum == l){	//gap on DB
			last_state = LEFT;
			x--;
		} else if (maximum == ori) {//stop here
			break;
		} 
	}
	
	reverse(s1.begin(), s1.end());	// tracing sequence has been conducted backward
	reverse(s2.begin(), s2.end());	// 	need to be reversed

	cout << "\ttotal score : " << (match * SCORE_MAT + mismatch * SCORE_MIS + gap* SCORE_GAP)
		<< "\tmatch : " << match << "\tmismatch : " << mismatch << "\tgap : " << gap << "\n"; 

	return make_pair(s1, s2);
}

/*
*	read DNA data from file path
*/
int getFileData(char filePath[], DnaData *& data) {
	fstream file;
	file.open(filePath, fstream::in);
	cout << "\topen " << filePath << "\n";
	data = new DnaData();
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
					temp= temp.substr(0, temp.length()-1);
			}
		}
		
		data->dna.append(temp);
	}
	cout << "\tread successfully\n\n";
	data->dna_len = (data->dna).length();
	file.close();
	return 0;
}

/*
*	extract filename from filepath
*/
char* strtokFilename(char* filePath){
	int i = 0, last = 0;
	for(i = 0; filePath[i] != '\0'; i++){
		if(filePath[i] == '/'){
			last = i;
		}
	}
	if(filePath[last] == '/'){
		return filePath + last + 1;
	} else {
		return filePath + last;
	}
	
}
/*
* change char array[n] = "filename.blabla.fa"
* into "filename.blabla"
*/
bool removeExtension(char* filePath){
	int i = 0, last = -1;
	for(i = 0; filePath[i] != '\0'; i++){
		if(filePath[i] == '.'){
			last = i;
		}
	}
	if(last == -1){
		return false;
	} else {
		filePath[last] = '\0';
		return true;
	}
}

/*
*	create new file to save answer in directory (dirPath)
*	new file name is ([file1]+[file2]).txt
*/
int saveData(char dirPath[], char dbPath1[], char dbPath2[], pair<string, string>& data) {
	char* name1 = strtokFilename(dbPath1);
	char* name2 = strtokFilename(dbPath2);
	
	string header = "";
	header.append("DB : ");
	header.append(name1);
	header.append("\nQuery : ");
	header.append(name2);
	header.append("\n\nAlignment\n");
	
	removeExtension(name1);
	removeExtension(name2);
	
	string resultPath = dirPath;
	if(resultPath[resultPath.length() -1] != '/'){
		resultPath += '/';
	}
	resultPath.append(name1);
	resultPath += '_';
	resultPath.append(name2);
	resultPath.append(".txt");

	cout <<"\tsave file in " << resultPath <<"\n";

	fstream file;
	file.open(resultPath.c_str(), fstream::out);

	file << header;
	file << data.first << "\n\n" << data.second << "\n";
	file.close();

	return 0;
}

int main(int argc, char** argv) {
	char *filePath1, *filePath2, *resultDirPath;
	DnaData *data1 = NULL, *data2 = NULL;
	int * result;
	bool stopForMemoryCheck = false;
	// get Input and fill s1, s2
	if(4 == argc || argc == 5){
		filePath1 = argv[1];
		filePath2 = argv[2];
		resultDirPath = argv[3];
		if(argc == 5){
			if(strcmp(argv[4], "stop") == 0){
				stopForMemoryCheck = true;
			} else {
				cout << "argv[4] ignored! only stop command used as option""\n";
				exit(1);
			}
		}
	} else {
		cout << "argv usage  ::  [filePath1] [filePath2] [resultDirectoryPath]\n";
		cout << "argv usage  ::  [filePath1] [filePath2] [resultDirectoryPath] \"stop\"\n";
		cout << "EXIT program\n";
		exit(1);
	}
	cout << "Opening Input Files...\n";
	getFileData(filePath1, data1);
	getFileData(filePath2, data2);
	
	// fill matrix with dynamic algorithm
	cout << "Building Dynamic Algorithm Matrix...\n";
	ScoreMatrix* matrix = ScoreMatrix::tryConstruct(data1, data2);
	
	// + 그 중에 점수 최대점 찾음. 백트랙하면서 추적, 공통부분 추출
	cout << "Finding answer...\n";	
	pair<string, string> subStrings = matrix->findAnswer();
	//cout << subStrings.first << "\n\n" << subStrings.second << "\n\n";

	// result data를 포멧에 맞춰서 예쁘게 저장
	saveData(resultDirPath, filePath1, filePath2, subStrings);

	if(stopForMemoryCheck){
		cin>>filePath1;
	}
	return 0;	
}
