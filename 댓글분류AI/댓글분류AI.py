# 나이브 어쩌구로 구현할거임
import math
import re

TEST_MODE_OFF = True    # =False 하면 ./ratings_vaid.txt랑 비교대조하면서 정답률 체크함
TEST_NAME = "T07"       # 디버그모드에서 만들어내는 결과파일 이름 앞에 붙음.

TAIL_DIC = [            # 제거할 Tail 목록
   '\t','.', '!', ',', "'", '"', '의', '이', '가', '이다','다', '를', '의', '들'
   '같은', '은', '는', '한', '진', '임', '하고', '고', '적', '인', '~', 'ㅋ'
   ]                    

def TailRemover(word):
    
    for tail in TAIL_DIC:
        is_tail = word[-len(tail) : ]
        if is_tail == tail and len(word) != len(tail):
            return TailRemover(word[ : -len(tail)])

    return word

def NownSpliter(message):       # 개선필요
    ret = []
    split = message.split(" ")
    for word in split :
        word = TailRemover(word)
        if len(word) != 0:
            ret.append(word)
    return ret

# AI의 베이스가 될 도수분포표가 필요.
class NaiveBaisian:
    dataList = {}
    
    def __init__(self):
        self.typeCount = 1
        self.dictionary = {}
        return

    def DefineRating(rating):
        data = NaiveBaisian()
        NaiveBaisian.dataList[rating] =  data
        return
    
    def InsertData(rating, message):
        data = NaiveBaisian.dataList[rating]
        data.typeCount += 1
        for word in NownSpliter(message):
            if word in data.dictionary :
                data.dictionary[word] += 1
            else :
                data.dictionary[word]  = 1
        return

    def IsNeverWord(word):
        for ratingData in NaiveBaisian.dataList.values():
            if word in ratingData.dictionary :
                return False
        return True

    def LogProbablityWord(self, word):
        if word in self.dictionary:
            return math.log(self.dictionary[word]) - math.log(self.typeCount)
        else :
            #if NaiveBaisian.IsNeverWord(word) :                                  # 어떤 rating에도 이 단어가 검색되지 않았다면,
            #    return 0                                                        # T02 이 단어는 분류에 영향을 주지 않는다.
            #    return math.log(self.typeCount)                                 # T05 원래비율대로 쪼갠다.
            #    for rating in NaiveBaisian.dataList.keys() :
            #        if self.typeCount < NaiveBaisian.dataList[rating].typeCount :
            #            return -0.000000000000001
            #    return +0.000000000000001

            return math.log( 0.4                 ) - math.log(self.typeCount)   # 상대에게 있다면 불리하게 판정한다.
                                                                            #T02&T05 미적용/ 서로 없다면 수집데이터가 적은 rating일 확률이 높다고 판정하는 셈.
                                                                                

    def LogProbablityMessage(rating, message):
        data = NaiveBaisian.dataList[rating]
        ret = 0
        for word in NownSpliter(message):
            ret += data.LogProbablityWord(word)
        return ret

    def FindBestRatingFor(message):
        rating_best= -1
        value_best = - math.inf
        for rating in NaiveBaisian.dataList.keys() :
            value  =  NaiveBaisian.LogProbablityMessage(rating, message)
            if  value_best  < value :
                value_best  = value
                rating_best = rating
        return rating_best

    def DebugGetDiff(message):
        return NaiveBaisian.LogProbablityMessage(0,message) - NaiveBaisian.LogProbablityMessage(1,message)


def floatForm(f):
    if f >= 0 :
        return "+%.5f" % f
    else :
        return "%.5f" % f

def main():
    NaiveBaisian.DefineRating(0)
    NaiveBaisian.DefineRating(1)
    print("Train data from ./ratings_train.txt")
    ##==============================================================##
    ##                            TRAINING                          ##
    ##==============================================================##
    file  = open('ratings_train.txt', 'r', encoding='UTF8')
    lines = file.readlines()
    del lines[0]                     # 첫줄 제거
    count = 0
    for data in lines:
        rating  = int(data[-2:-1])
        data    = data.partition('\t')[2][:-3] # 처음에 나오는 id 제거, 맨 뒤의 tab, rating, enter 제거
        NaiveBaisian.InsertData(rating, data)

        count += 1
        if count % 30000 == 0:
            print("training... " + str(count))

    file.close()
     


    ##==============================================================##
    ##                            ACTION                            ##
    ##==============================================================## 
    if TEST_MODE_OFF :
        print("Get problems from ./ratings_test.txt  to  Write result on ./ratings_result.txt")

        fread  = open('ratings_test.txt'  , 'r', encoding='UTF8')
        fwrite = open('ratings_result.txt', 'w', encoding='UTF8')
        
        lines = fread.readlines()
        write_msg = lines[0]
        del lines[0]                     # 첫줄 제거
        for data in lines:
            id   = data.partition('\t')[0]
            data = data.partition('\t')[2][:-2] # 처음에 나오는 id 제거, 맨 뒤의 rating 제거
        
            rated = NaiveBaisian.FindBestRatingFor(data)
            write_msg += (id + "\t" + data + "\t" + str(rated) + "\n")

        
        fwrite.write(write_msg)
        fwrite.close()
        fread .close()
    
        print("Classification finished. result saved")

        return
    ##==============================================================##
    ##                       ACTION  for TEST                       ##
    ##                           (DISABLED)                         ##
    ##==============================================================## 

    print("Get problems from ./ratings_valid.txt to get accuracy")

    success = 0;
    fail  = 0;
    failed_messages = ""
    file  = open('ratings_valid.txt', 'r', encoding='UTF8')
    lines = file.readlines()
    del lines[0]                     # 첫줄 제거
    for data in lines:
        rating  = int(data[-2:-1])
        data    = data.partition('\t')[2][:-2] # 처음에 나오는 id 제거, 맨 뒤의 rating 제거
        
        rated = NaiveBaisian.FindBestRatingFor(data)
        if rating == rated :
            success += 1
        elif rating + rated == 1 :
            fail += 1

            gap = NaiveBaisian.DebugGetDiff(data)
            failed = str(1-rating) + " " + floatForm(gap) + "  " + data + "\n"
            failed_messages += failed
            

    file.close()

    total = success + fail
    print(TEST_NAME + "  TEST RESULT ----------------")
    print("total   : " + str(total))
    print("success : " + str(success) + "\t" + str(100*success/total) + "%")



    
    ##==============================================================##
    ##       PUT FAILED MESSAGE   &   LEARNING DATA ON FILE         ##
    ##==============================================================##
    file = open(TEST_NAME + "_Fail" + str(fail) + ".txt", 'w', encoding='UTF8')
    file.write("fail 비율 : " + str(100 * fail/total) + "%\n")
    for tail in TAIL_DIC :
        file.write( tail + ", ")
    file.write("\n")
    file.write(failed_messages)
    file.close()

    for rating in range(2) :
        writeStr = "rate" + str(rating) + " 총 input:" + str( NaiveBaisian.dataList[rating].typeCount )+ "\n"
        dic = NaiveBaisian.dataList[rating].dictionary
        sorted_by_value = reversed(sorted(dic.items(), key=lambda kv: kv[1]))    ## 이거 뭔코든지 나도 몰라양. 많이 나온 순서대로 정렬해줘양!


        for tuple in sorted_by_value :
            writeStr += str(tuple[1]) + "\t" + str(tuple[0]) +"\n"              # 단어 나온 횟수 + 단어

        file = open(TEST_NAME + "_DicRate" + str(rating) + ".txt", 'w', encoding='UTF8')
        file.write(writeStr)
        file.close()
    
main()







