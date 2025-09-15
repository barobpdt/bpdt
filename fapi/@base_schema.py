from typing import List
from typing import Optional
from sqlalchemy import ForeignKey
from sqlalchemy import String
from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import relationship

class Base(DeclarativeBase):
	pass
class User(Base):
	__tablename__ = "user_account"
	id: Mapped[int] = mapped_column(primary_key=True)
	name: Mapped[str] = mapped_column(String(30))
	fullname: Mapped[Optional[str]]
	addresses: Mapped[List["Address"]] = relationship(
		back_populates="user", cascade="all, delete-orphan"
	)
	def __repr__(self) -> str:
		return f"User(id={self.id!r}, name={self.name!r}, fullname={self.fullname!r})"

class Address(Base):
	__tablename__ = "address"
	id: Mapped[int] = mapped_column(primary_key=True)
	email_address: Mapped[str]
	user_id: Mapped[int] = mapped_column(ForeignKey("user_account.id"))
	user: Mapped["User"] = relationship(back_populates="addresses")
	def __repr__(self) -> str:
		return f"Address(id={self.id!r}, email_address={self.email_address!r})"


baseName : baro
tables {
	#사용자정보
	User : user
		*id 	int	pk {init=Fase}		=> mapped_column(Integer, primary_key=True)
		*name	str(30) 	=> Mapped[str] = mapped_column(String(30))
		full_nm	text 		=> Mapped[Optional[str]]
		address	list(Address) rel()	=> Mapped[List["Address"]] = relationship(back_populates="user")
		create_date now 	=> Mapped[datetime] = mapped_column(insert_default=func.now())

	# 주소록
	Address : address
		*id		int pk 		=> mapped_column(Integer, primary_key=True)
		*email	text 		=> Mapped[str]
		user_id fk(user.id) => mapped_column(ForeignKey("user.id"))
		user rel(User) 		=> Mapped["User"] = relationship(back_populates="addresses")
	# 룸정보
	RoomInfo : room_info
		*room_id pk	# 방번호
		*room_type combo(01:일반방=>01) #방타입 
		*room_status combo(01:대기, 02:진행, 03:멈춤, 종료:04=>01) #방상태 
		*room_nm text # 방이름
		*room_owner # 방장아이디
		spd	# webRtc SPD
		create_dt now # 생성일


}

# event
import json
from typing import Union
 
ResultType = Union[dict, list[dict], list, str]
 
class ResponseInterFace: 
    def __init__(self, result: ResultType, **kwargs) -> None:
        self.result = result
        self.kwargs = kwargs
 
    def to_dict(self) -> dict: 
        return {**self.kwargs, "result": self.result}
 
    def to_str(self) -> str: 
        return json.dumps(self.to_dict(), ensure_ascii=False)
 
    def __getitem__(self, key: str) -> Union[ResultType, str]:
        return self.to_dict()[key]
 
    def __iter__(self):
        return iter(self.to_dict())
 
    def keys(self):
        return self.to_dict().keys()
 
    def items(self):
        return self.to_dict().items()
	
ResponseInterFace(result=some, message="조회하였습니다.")

class ImClass:
    def __init__(self, attribute_1, attribute_2):
        self.attribute_1 = attribute_1
        self.attribute_2 = attribute_2
 
    def __getitem__(self, key):
        return getattr(self, key)
 
    def __setitem__(self, key, value):
        setattr(self, key, value)
 
    def __iter__(self):
        return iter(self.__dict__)
 
    def __len__(self):
        return len(self.__dict__)

def __post_init__(self, password: str, repeat_password: str):
		if password != repeat_password:
			raise ValueError("passwords do not match")
		self.password_hash = your_crypt_function_here(password)

uuid() => uid: Mapped[str] = mapped_column(... default_factory=uuid4)

* 항목 repr 생성
	def __repr__(self) -> str:
		return f"User(id={self.id!r}, name={self.name!r}, fullname={self.fullname!r})"




user1 = User(
	name="spongebob",
	fullname="Spongebob Squarepants",
	addresses=[Address(email_address="spongebob@sqlalchemy.org")],
)	
session.add_all([user1, user2])
session.commit()

while(s.valid()) {
	left=s.findPos('@base')
	ss.add(left)
	not(s.ch()) break;
	sp=s.cur()
	c=s.ch()
	if(c.eq('.')) {
		name=s.incr().move()
		sp=s.cur()
		c=s.ch()
	}
	if(c.eq('?','|','{')) {
		if(c.eq('?','|')) {
			c=s.incr().ch()
			if(c.eq('[')) {
				v=s.match()
			} else if(c.eq()) {
				v=s.match()
			} else {
				v=s.move()
			}
		}
	} else {
		s.pos(sp)
	}
}
return ss;
