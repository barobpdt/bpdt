
CHO_SUNG = ["ㄱ", "ㄲ", "ㄴ", "ㄷ", "ㄸ", "ㄹ", "ㅁ", "ㅂ", "ㅃ", "ㅅ", "ㅆ", "ㅇ", "ㅈ", "ㅉ", "ㅊ", "ㅋ", "ㅌ", "ㅍ", "ㅎ"]
JUNG_SUNG = ["ㅏ", "ㅐ", "ㅑ", "ㅒ", "ㅓ", "ㅔ", "ㅕ", "ㅖ", "ㅗ", "ㅘ", "ㅙ", "ㅚ", "ㅛ", "ㅜ", "ㅝ", "ㅞ", "ㅟ", "ㅠ", "ㅡ", "ㅢ", "ㅣ"]
JONG_SUNG = ["", "ㄱ", "ㄲ", "ㄳ", "ㄴ", "ㄵ", "ㄶ", "ㄷ", "ㄹ", "ㄺ", "ㄻ", "ㄼ", "ㄽ", "ㄾ", "ㄿ", "ㅀ", "ㅁ", "ㅂ", "ㅄ", "ㅅ", "ㅆ", "ㅇ", "ㅈ", "ㅊ", "ㅋ", "ㅌ", "ㅍ", "ㅎ"]

# 한글 음절을 만드는 함수
def create_korean_char(chosung, jungsung, jongsung=""):
    chosung_index = CHO_SUNG.index(chosung)
    jungsung_index = JUNG_SUNG.index(jungsung)
    jongsung_index = JONG_SUNG.index(jongsung)

    # 한글 음절 코드 계산
    korean_char_code = 0xAC00 + ((chosung_index * 21) + jungsung_index) * 28 + jongsung_index
    return chr(korean_char_code)

# 한글 자모 합치기 함수
def combine_hangul(jamos):
    result = ""
    i = 0
    while i < len(jamos):
        chosung = jamos[i]
        i += 1
        if i < len(jamos):
            jungsung = jamos[i]

            i += 1
            jongsung = ''
            # 종성인지 다음 글자 초성인지 확인
            if i < len(jamos) :
                jongsung = jamos[i]
                i += 1
                if i < len(jamos) :
                    next_chr = jamos[i]

                    if ord("ㄱ") > ord(next_chr) or ord(next_chr) > ord("ㅎ") :  # 자음 범위가 아님 (jongsung은 다음 글자 초성)
                        jongsung = ""
                        i -= 1

            # 한글 문자로 합치기
            result += create_korean_char(chosung, jungsung, jongsung)
        else:
            break  # 중성이 없는 경우
    return result




def convertToInitialLetters(text):
	CHOSUNG_START_LETTER = 4352
	JAMO_START_LETTER = 44032
	JAMO_END_LETTER = 55203
	JAMO_CYCLE = 588
	
	def isHangul(ch):
		return ord(ch) >= JAMO_START_LETTER and ord(ch) <= JAMO_END_LETTER
	
	result = ""
	for ch in text:
		if isHangul(ch): #한글이 아닌 글자는 걸러냅니다.
			result += unichr((ord(ch) - JAMO_START_LETTER)/JAMO_CYCLE + CHOSUNG_START_LETTER)
		
	return result

def findTopNRelatedSentences(inputInitialLetters, N, corpus):
    searchResults = []
    for item in corpus:
        totalLengthOfOcc = 0
        for tag in item['tag_initial']:
            if tag in inputInitialLetters:
                totalLengthOfOcc += len(tag)
        searchResults.append((totalLengthOfOcc, item['sentence']))
    return sorted(searchResults, reverse=True)[:N] #상위 N개의 검색 결과만 리턴합니다.

print(convertToInitialLetters("소변에 피가 섞여 나옵니다.".decode('utf-8')) )
# ᄉᄇᄋᄑᄀᄉᄋᄂᄋᄂᄃ

f = open("hospital sentences.tsv", "r") #파일을 읽어옵니다.
corpus = []
for line in f:
    columns = line.decode('utf-8').strip('\n').split('\t') #각 문장당 8개의 열
    element = {}
    element['sentence'] = columns[0]
    element['category'] = set()
    for i in range(1, 4):
        if columns[i]:
            element['category'].add(columns[i])
    element['tag'] = set()
    element['tag_initial'] = set()
    for i in range(4, 8):
        if columns[i]:
            element['tag'].add(columns[i])
            element['tag_initial'].add(convertToInitialLetters(columns[i]))
    corpus.append(element)

f.close() #파일을 닫습니다.

inputLetters = u"\u1100\u1109"
print "Input :", inputLetters
print "검색결과 : "
searchResults = findTopNRelatedSentences(inputLetters, 5, corpus)
for totalOcc, sentence in searchResults:
	print totalOcc, sentence

# Input : ᄀᄉ
# 검색결과 : 
# 2 심장이 빨리뜁니다.
# 2 숨쉬기가 힘듭니다.
# 2 갈비뼈 주변에 통증이 있습니다.
# 2 가슴이 두근거립니다.
# 2 가슴이 답답합니다.

inputLetters = u"\u1103\u1105"
print "Input :", inputLetters
print "검색결과 : "
searchResults = findTopNRelatedSentences(inputLetters, 5, corpus)
for totalOcc, sentence in searchResults:
	print totalOcc, sentence

# Input : ᄃᄅ
# 검색결과 : 
# 2 허벅지 감각이 이상합니다.
# 2 종아리가 아픕니다.
# 2 다리의 감각이 이상합니다.
# 2 다리가 저립니다.
# 2 다리가 아픕니다.

inputLetters = u"\u110b\u1109"
print "Input :", inputLetters
print "검색결과 : "
searchResults = findTopNRelatedSentences(inputLetters, 5, corpus)
for totalOcc, sentence in searchResults:
	print totalOcc, sentence

# Input : ᄋᄉ
# 검색결과 : 
# 3 위산이 목으로 올라옵니다.
# 3 삼키는게 어렵습니다.
# 2 입술이 아픕니다.
# 2 입술이 건조합니다.
# 2 오심이 있습니다.

inputLetters = u"\u110b\u110c"
print "Input :", inputLetters
print "검색결과 : "
searchResults = findTopNRelatedSentences(inputLetters, 5, corpus)
for totalOcc, sentence in searchResults:
	print totalOcc, sentence

# Input : ᄋᄌ
# 검색결과 : 
# 2 소변에 피가 섞여 나옵니다.
# 2 소변보는게 불편합니다.
# 1 혓바닥이 아픕니다.
# 1 입맛이 없습니다.
# 1 입 안이 건조합니다.
검색결과에 score들까지 보면