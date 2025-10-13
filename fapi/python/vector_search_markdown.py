# Import Package https://boringariel.tistory.com/87
from langchain_community.document_loaders import TextLoader
from langchain_community.vectorstores import FAISS
from langchain.embeddings import HuggingFaceBgeEmbeddings
from langchain_text_splitters import CharacterTextSplitter
from IPython.display import display_markdown

embeddings = HuggingFaceBgeEmbeddings() 


# Load Data
documents = TextLoader("./html_basics.md", encoding='utf8').load()

# Text Split
text_splitter = CharacterTextSplitter(chunk_size=300, chunk_overlap=0)
docs = text_splitter.split_documents(documents)

# Create DB
db = FAISS.from_documents(docs, embeddings)

# Query
query = "HTML은 무엇일까"
answer = db.similarity_search(query)
display_markdown(answer[0].page_content, raw=True)


# db.similarity_search_with_score(query)
'''
[(Document(page_content='HTML은 콘텐츠의 구조를 정의하는 _마크업 언어_ 입니다. HTML은 콘텐츠의 서로 다른 부분들을 씌우거나 감싸서 다른 형식으로 보이게하거나 특정한 방식으로 동작하도록 하는 일련의 **{{Glossary("element", "요소")}}** 로 이루어져 있습니다. {{Glossary("tag", "태그")}}로 감싸는 것으로 단어나 이미지를 다른 어딘가로 하이퍼링크하거나 단어들을 이탤릭체로 표시하고 글씨체를 크게 또는 작게 만드는 등의 일을 할 수 있습니다. 아래에 나오는 줄의 내용과 같이 예를 들 수 있습니다.', metadata={'source': './html_basics.md'}),
  0.16946973),
 (Document(page_content='여기서 우리는 HTML 맛보기를 하였습니다. 더 알아보기 위해, [HTML 배우기](/ko/docs/Learn/HTML) 페이지로 가보세요.\n\n{{PreviousMenuNext("Learn/Getting_started_with_the_web/Dealing_with_files", "Learn/Getting_started_with_the_web/CSS_basics", "Learn/Getting_started_with_the_web")}}', metadata={'source': './html_basics.md'}),
  0.1735438),
 (Document(page_content='많은 웹의 내용은 목록이기 때문에, HTML은 이것을 위한 특별한 요소를 가지고 있습니다. 목록을 나타내는 것은 항상 최소 두 개의 요소로 구성됩니다. 가장 일반적인 목록의 종류는 순서가 있는 것과 순서 없는 것이 있습니다.', metadata={'source': './html_basics.md'}),
  0.17826173),
 (Document(page_content='### 문단\n\n위에서 설명했듯이, {{htmlelement("p")}} 요소는 문자의 문단을 포함하기 위한 것입니다. 일반적인 문자 내용을 나타낼 때 많이 사용하게 될 것입니다.\n\n```html\n<p>This is a single paragraph</p>\n```', metadata={'source': './html_basics.md'}),
  0.17929386)]
'''


## 한글토큰화 방법
import re
from konlpy.tag import Okt
tokenizer = Okt()
sample_text = '오, 그녀는 정말 횃불에게 찬란히 타오르는 법을 가르치누나! 그녀가 밤의 뺨에 걸려 있는 모양이, 에티오피아 여인 귀에 걸린 화려한 보석 같아.'
print(tokenizer.morphs(sample_text))

sample_text_2 = re.sub('[^가-힣 ]+','',sample_text) # 기호제거
print(tokenizer.morphs(sample_text_2)) 
'''
['오', '그녀', '는', '정말', '횃불', '에게', '찬란히', '타오르는', '법', '을', '가르치', '누나', '그녀', '가', '밤', '의', '뺨', '에', '걸려', '있는', '모양', '이', '에티오피아', '여인', '귀', '에', '걸린', '화려한', '보석', '같아']
'''
tokenizer.pos(sample_text_2)  # 명사,조사,동사,형용사 ... 분류


