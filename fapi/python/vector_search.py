# GPU를 사용하려면 faiss-gpu 설치
# pip install faiss-cpu
# pip install transformers sentence-transformers
# pip install konlpy
import faiss
import numpy as np
from sentence_transformers import SentenceTransformer

'''
# 위키미디어로부터 kowiki 데이터를 다운로드 받음
!wget https://dumps.wikimedia.org/kowiki/latest/kowiki-latest-pages-articles1.xml-p1p82407.bz2

# 위키데이터의 노이즈를 제거하고 json 형태로 반환하는 코드를 참조
!git clone https://github.com/attardi/wikiextractor.git

# 다운로드 받은 샘플 위키 데이터를 전처리하여 검색의 입력으로 사용
# 결과는 elastic 폴더에 'extract_result/AA,AB,AC.../wiki_00..99'라는 새로운 폴더에 저장된다.(용량이 비슷하게 나눠서 저장됨)
# 변환결과 wiki_00 파일의 내용 샘플  {"id": "5", "revid": "641228", "url": "https://ko.wikipedia.org/wiki?curid=5", "title": "\uc9c0\...\ud130", "text": "\uc81c\...\ub2e4."}
!python -m wikiextractor.wikiextractor.WikiExtractor kowiki-latest-pages-articles1.xml-p1p82407.bz2 --json -o extract_result
'''

import json
from sentence_transformers import SentenceTransformer
# Sentence Transformer 모델 초기화
model = SentenceTransformer("hunkim/sentence-transformer-klue")
def get_embedding(sentences):
	# 입력 문장을 인코딩하여 임베딩을 얻음
	return model.encode(sentences)
wiki_dump_json_file = '/content/extract_result/AA/wiki_00'
# 'wiki_dump_json_file'에 있는 JSON 파일 읽어들여 index_docs에 저장
index_docs = []
for line in open(wiki_dump_json_file, encoding="utf-8"):
	# JSON 데이터를 읽어들여 파이썬 딕셔너리로 변환
	json_data = json.loads(line)
	# text와 title에 대한 임베딩을 계산하여 해당 필드에 추가
	json_data['embeddings_title'] = get_embedding(json_data['title']).tolist()
	json_data['embeddings_text'] = get_embedding(json_data['text']).tolist()

	# 색인할 문서 목록에 추가
	index_docs.append(json_data)

title_docs = [item["embeddings_title"] for item in index_docs]
text_docs = [item["embeddings_text"] for item in index_docs]    

index_title = faiss.IndexFlatL2(768)
index_title.add(np.array(title_docs).astype('float32'))

index_text = faiss.IndexFlatL2(768)
index_text.add(np.array(text_docs).astype('float32'))
# 우선 테스트로 가장 간단한 Index type인 FlatL2를 사용해 보도록 하자. 
# Faiss는 다양한 ANN 알고리즘을 제공하는데 Flat, IVF, HNSW, PQ 등 다양하게 제공한다. 여기서는 우선 문서가 몇 개 안 되니까 기본인 FlatL2를 사용해 본 것이다.

# 결과로 offset과 score가 나오는데 원본 문서와의 매칭은 offset 값을 이용

query_str = "문재인의 친구"
query_emb = get_embedding([query_str])
scores, offsets = index_title.search(query_emb, 5)
for i,offset in enumerate(offsets[0]):
	print(f'{offset}, {scores[0][i]} {index_docs[offset]}')

query_str = "대한민국 16대 대통령이 누구야?"
query_emb = get_embedding([query_str])
scores, offsets = index_text.search(query_emb, 5)
for i,offset in enumerate(offsets[0]):
    print(f'{offset}, {scores[0][i]} {index_docs[offset]}')

#################################################################
# 텍스트 벡터화 (위의 예시와 동일)
model = SentenceTransformer('upskyy/bge-m3-korean')
docs = ['파이썬은 배우기 쉬운 언어입니다.', '벡터 검색은 자연어 처리의 중요한 기술입니다.', '한글 텍스트를 벡터로 만드는 방법이 궁금합니다.']
doc_embeddings = model.encode(docs)

# Faiss 인덱스 생성 및 학습
d = doc_embeddings.shape[1]  # 임베딩 차원
index = faiss.IndexFlatL2(d) # L2 거리 기반 인덱스
index.add(doc_embeddings)    # 벡터 추가

# 검색 쿼리 벡터화
query = ['파이썬 언어에 대해 알려줘']
query_embedding = model.encode(query)

# 유사한 벡터 검색
k = 2  # 가장 유사한 상위 2개 결과
distances, indices = index.search(query_embedding, k)

# 결과 출력
print("가장 유사한 문서:")
for i in range(k):
	print(f"순위 {i+1}: 문서 '{docs[indices[0][i]]}' (거리: {distances[0][i]:.4f})")
