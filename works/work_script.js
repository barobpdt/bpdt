## Fast API
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
