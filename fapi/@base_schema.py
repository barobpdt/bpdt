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


schema baro
	tables
		User : user
			*id 	int	pk {init=Fase}		=> mapped_column(Integer, primary_key=True)
			*name	str(30) 	=> Mapped[str] = mapped_column(String(30))
			full_nm	text 		=> Mapped[Optional[str]]
			address	list(Address) rel()	=> Mapped[List["Address"]] = relationship(back_populates="user")
			create_date now 	=> Mapped[datetime] = mapped_column(insert_default=func.now())
 
		
		Address : address
			*id		int pk 		=> mapped_column(Integer, primary_key=True)
			*email	text 		=> Mapped[str]
			user_id fk(user.id) => mapped_column(ForeignKey("user.id"))
			user rel(User) 		=> Mapped["User"] = relationship(back_populates="addresses")

# event
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