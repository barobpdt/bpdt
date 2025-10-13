import faiss
from sentence_transformers import SentenceTransformer

##1. 벡터준비
# 모델 불러오기
model = SentenceTransformer('multi-qa-MiniLM-L6-cos-v1')
# 데이터 임베딩 하기
query_embedding = model.encode(['cat','dog','puppy'])


##2.인덱스 생성
# 내적을 이용한 인덱스 생성(Inner Product)
index = faiss.IndexFlatIP(768)
'''
IndexFlatIP(dimension) : 내적으로 검색하는 인덱스 생성
IndexFlatL2(dimension) : L2거리를 이용하여 검색하는 인덱스 생성
'''
# 유사도 함수로 내적을 사용하는 인덱스 생성
# 각 벡터에 대한 고유한 ID가 있는 경우 이를 이용하여 인덱스를 구성할 수 있게 IndexIDMap을 써줌
# index = faiss.IndexIDMap(faiss.IndexFlatIP(768))

# L2 정규화 진행(cosine similarity) 임베딩 벡터를 넣어주는 부분
faiss.normalize_L2(query_embedding)

# 인덱스에 데이터 추가
index.add(query_embedding)
'''
add_with_ids(embedding,ids) : 벡터와 해당 벡터의 고유 ID를 함께 추가하는 방법. IndexIDMap을 사용할 경우 이 함수로 데이터를 넣어줘야함.
add(embedding) : 인덱스에 벡터만 추가하는 방법
train(embedding) : 인덱스를 학습하는 방법
'''
# IndexIDMap 사용시 아래와 같이 추가
# index.add_with_ids(embs, np.array(range(0, len(embs))))

##3. 검색
# 검색할 키워드를 벡터로 바꿔줌
keyword = model.encode(['kitty'])

# L2 정규화를 시켜줌
norm_keyword = faiss.normalize_L2(keyword)

# 검색해줌
d, i = index.search(norm_keyword,1)

## 실행 결과
'''
[[0.807919]] [[0]]
해석해 보자면 0번째 인덱스를 가지는 단어와 0.8의 distance를 가진다는 것입니다. 
고로 kitty 는 고양이에 가깝다
'''

################### GPU 사용 ################
##1. Single GPU 사용
# GPU 장치를 초기화
res = faiss.StandardGpuResources()
# 인덱스를 생성
index = faiss.IndexFlatL2(d)
# GPU 인덱스로 변환
gpu_index = faiss.index_cpu_to_gpu(res, 0, index)
# 임베딩을 GPU 인덱스에 추가
gpu_index.add(xb)
# 인덱스를 사용하여 검색을 수행
d, i = gpu_index.search(xq, k)


#2. Multi GPU 사용
# GPU 장치를 초기화
res = faiss.StandardGpuResources()
# 인덱스를 생성
index = faiss.IndexFlatL2(d)
# 여러 개의 GPU에서 인덱스를 생성
ngpus = faiss.get_num_gpus()
co = faiss.GpuMultipleClonerOptions()
co.shard = True
index = faiss.index_cpu_to_all_gpus(index, co=co, ngpu=ngpus)
# 임베딩을 인덱스에 추가
index.add(xb)
# 인덱스를 사용하여 검색을 수행
D, I = index.search(xq, k)



