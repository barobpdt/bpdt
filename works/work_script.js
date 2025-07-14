## Fast API  ( DOC=> https://wikidocs.net/175875 )
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # 모든 출처 허용
    allow_credentials=True,
    allow_methods=["*"],  # 모든 HTTP 메서드 허용
    allow_headers=["*"],  # 모든 헤더 허용
)
## 회원 테이블
from sqlalchemy import Column, String, JSON
from app.db import Base

class User(Base):
    __tablename__ = 'user_info'
    
    id = Column(String, primary_key=True)
    email = Column(String, unique=True, index=True)
    password = Column(String)
    friends = Column(JSON, nullable=True)

## 회원가입 처리
from sqlalchemy import Column, String, JSON
from app.db import Base

class User(Base):
    __tablename__ = 'user_info'
    
    id = Column(String, primary_key=True)
    email = Column(String, unique=True, index=True)
    password = Column(String)
    friends = Column(JSON, nullable=True)


## sqlite 연결
from database.connetion import conn
from sqlmodel import SQLModel, create_engine, Session

# 데이터 베이스 파일 이름 지정
database_file = 'my_website.db' 
# DB 연결, MySQL의 경우 mysql://user:password@localhost/mydatabase 형식을 맞춰주면 된다.
database_connetion_string = f"sqlite:///{database_file}"

engine_url = create_engine(database_connetion_string, echo=True)

# 데이터베이스 테이블 생성하는 함수
def conn():
	SQLModel.metadata.create_all(engine_url)

# Session 사용 후 자동으로 종료
def get_session():
	with Session(engine_url) as session:
		yield session

# FastAPI
app = FastAPI()

#애플리케이션이 시작 될 때 데이터베이스를 생성하도록 만듬
@app.on_event("startup")
def on_startup():
	conn()

'''
JWT https://yjoo-anywhere.tistory.com/15
Header에는 토큰 타입과 암호화 알고리즘이 명시되어 있고, Payload엔 유저의 정보들이 작성되어 있다.
iss (Issuer) : 토큰 발급자
sub (Subject) : 토큰 제목 - 토큰에서 사용자에 대한 식별값이 됨
aud (Audience) : 토큰 대상자
exp (Expiration Time) : 토큰 만료 시간
nbf (Not Before) : 토큰 활성 날짜 (이 날짜 이전의 토큰은 활성화 되지 않음을 보장)
iat (Issued At) : 토큰 발급 시간
jti (JWT Id) : JWT 토큰 식별자 (issuer가 여러명일 때 이를 구분하기 위한 값)
'''
##
x=conf('#confMap')
x=System.driveList()

~~
x=System.tick()
root=listFolder(path)
d=System.tick() - x;
print("xxx", d,x,root)
~~
path='C:/work'
root = _node('listFolder').removeAll();

~~
<func>
	listFolder(path, root, depth) {
		not(root) root = _node('listFolder').removeAll();
		not(depth) depth = 0;
		fo=Baro.file()
		fo.var(sort,'name, case')
		depth++;
		fo.list(path, func(info) {
			while(info.next()) {
				info.inject(type,name, fullPath)
				if(type.eq('file')) continue;
				if(name.eq('windows')) continue;
				cur=root.addNode().with(type,name,fullPath)
				if(depth<3) {
					listFolder(fullPath, cur, depth)
				}
			}
		})
		return root;
	}
</func>
