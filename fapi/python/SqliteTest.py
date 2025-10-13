# -*- coding: utf-8 -*-
from __future__ import annotations

from jose import JWTError, jwt
from typing import List
from typing import Optional
from datetime import datetime 
from typing import Dict

from sqlalchemy import select, ForeignKey, JSON, func
from sqlalchemy.orm import selectinload

from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import relationship
from sqlalchemy.orm import Session
from sqlalchemy.orm import MappedAsDataclass
from sqlalchemy.orm import relationship
from sqlalchemy.orm.collections import attribute_keyed_dict
from sqlalchemy.ext.mutable import MutableDict
import json
import sys
from ApiConfig import ApiConfig

print("디비 테스트 시작 =====")
class Base(DeclarativeBase):
	pass

class User(Base):
	__tablename__ = "user_account"
	userId: Mapped[str] = mapped_column(primary_key=True)
	name: Mapped[str]
	fullname: Mapped[Optional[str]]
	addresses: Mapped[List["Address"]] = relationship(
		back_populates="user", cascade="all, delete-orphan"
	)
	def __init__(self, id, name, email):
		self.userId=id
		self.name = name
		self.addresses = [Address(emailAddress=email)]
	def __repr__(self) -> str:
		return f"User(id={self.userId!r}, name={self.name!r}, fullname={self.fullname!r})"

class Address(Base):
	__tablename__ = "address"
	id: Mapped[int] = mapped_column(primary_key=True)
	userId: Mapped[str] = mapped_column(ForeignKey("user_account.userId"))
	emailAddress: Mapped[str]
	user: Mapped["User"] = relationship(back_populates="addresses")
	def __repr__(self) -> str:
		return f"Address(id={self.id!r}, email_address={self.emailAddress!r})"

class Item(Base):
	__tablename__ = "item"
	id:Mapped[str] = mapped_column(primary_key=True)
	name:Mapped[str]
	type:Mapped[str]
	def __init__(self, id, name, type):
		self.id=id
		self.name = name
		self.type = type
	def __repr__(self) -> str:
		return f"item(id={self.id!r}, name={self.name!r})"
'''
class Order(Base):
	__tablename__='order'
	orderId:Mapped[str] = mapped_column(primary_key=True)
	userId: Mapped[str] = mapped_column(ForeignKey('user_account.id'))
	itemId: Mapped[str] = mapped_column(ForeignKey('item.id'))
	def __init__(self, orderId, userId, itemId):
		self.orderId=orderId
		self.useId = userId
		self.itemId=itemId
	def __repr__(self) -> str:
		return f"order(id={self.orderId!r}, user={self.userId!r}, item={self.itemId})"

class OrderDetail(Base):
	__tablename__='order_detail'
	orderId:Mapped[str] = mapped_column(ForeignKey('order.orderId'))
	orderName:Mapped[str] 
	orderDesc:Mapped[str]

	def __init__(self, orderId, orderName, orderDesc):
		self.orderId=orderId
		self.orderName = orderName
		self.orderDesc = orderDesc
	def __repr__(self) -> str:
		return f"orderDetail(id={self.orderId!r}, name={self.orderName!r})"
'''

class CommCode(MappedAsDataclass, Base):
	__tablename__ = "comm_code"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("comm_code.id"), init=False
	)
	depth: Mapped[int]
	code: Mapped[str]
	title: Mapped[str]
	type: Mapped[str]
	ref: Mapped[Optional[str]] = mapped_column(nullable=True)
	data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(JSON))
	created_at: Mapped[datetime] = mapped_column(server_default=func.now())
	updated_at: Mapped[datetime | None] = mapped_column(onupdate=func.now())

	children: Mapped[Dict[str, CommCode]] = relationship(
		cascade="all, delete-orphan",
		back_populates="parent",
		collection_class=attribute_keyed_dict("code"),
		init=False,
		repr=False,
	)

	parent: Mapped[Optional[CommCode]] = relationship(
		back_populates="children", remote_side=id, default=None
	)

	def __init__(self, *args, **kwargs):
		self.depth = 0
		data = kwargs['data'] if 'data' in kwargs else None
		if isinstance(data, dict):
			self.data = data
		elif data:
			self.data = json.loads(data)
		self.type = kwargs['type'] if 'type' in kwargs else ''
		if args:	
			self.code = args[0]
			self.title = kwargs['title'] if 'title' in kwargs else ''
			if len(args)>1:
				self.title = args[1]
			if len(args)>2:
				self.type = args[2]
		else:
			self.code = kwargs['code']
			self.title = kwargs['title']
		self.ref = kwargs['ref'] if 'ref' in kwargs else ''
		self.parent = kwargs['parent'] if 'parent' in kwargs else None
		if self.parent:
			self.depth = self.parent.depth + 1
		# print(f'init {self}')

	def getChild(self, code):
		try:
			return self.children[code]
		except Exception as e:
			print(f'@@CommCode childCode code:{code} >>exception:{e}')
		return None
	
	def getTitle(self, code):
		p=self
		try:
			for c in code.split('.'):
				p=p.getChild(c)
		except Exception as e:
			pass
		if p:
			return p.title

	def getObject(self, code):
		p=self
		try:
			for c in code.split('.'):
				p=p.getChild(c)
				if not p:
					return None
		except Exception as e:
			pass
		return p
	def getRef(self, code, ref):
		try:
			p=self.getObject(code)
			if p:
				for c in p.children.values():
					if c.ref==ref:
						return c
		except Exception as e:
			pass
		return None
	
	def detail(self):
		return '\n\t'.join([repr(c) for c in self.children.values()])
		
	def __repr__(self):
		return f"CC(code={self.code},title={self.title},ref={self.ref})"

	def dump(self, indent: int = 0) -> str:
		return (
			"   " * indent
			+ repr(self)
			+ "\n"
			+ "".join([c.dump(indent + 1) for c in self.children.values()])
		)
	
DATABASE_URL = "sqlite:///./test.db"


async def create_tables(engine):
	try:
		async with engine.begin() as conn:
			await conn.run_sync(Base.metadata.create_all)
		ApiConfig().info("✅ 비동기 데이터베이스 테이블이 성공적으로 생성되었습니다.")
	except Exception as e:
		ApiConfig().info(f"⚠️ 비동기 테이블 생성 중 오류 발생: {e}")

def init_data(session):
	root = CommCode('root','commcode root','root')
	auth = CommCode('auth','권한그룹','codeGroup',parent=root)			
	session.add(root)
	session.add(auth)
	session.add(CommCode('01','관리자','codeValue', parent=auth))
	session.add(CommCode('02','담당','codeValue', parent=auth))
	session.add(CommCode('03','사용자','codeValue', parent=auth))
	session.add(CommCode('04','게스트','codeValue', parent=auth))
	session.add(Item('a003', 'test', 'AA'))
	session.add(Item('a004', 'test11', 'AA'))
	session.commit()

def add_code(session):
	root = session.scalars(
		select(CommCode)
		.options(selectinload(CommCode.children, recursion_depth=1))
		.filter(CommCode.type=='root')
	).one()
	group = CommCode('singType','곡장르','codeGroup',parent=root)
	session.add(group)
	session.add(CommCode('01','발라드','codeValue', parent=group))
	session.add(CommCode('02','트로트','codeValue', parent=group))
	session.add(CommCode('03','댄스','codeValue', parent=group))
	session.add(CommCode('05','팝','codeValue', parent=group))
	session.commit()

def selectCommCode(session):
	root = session.scalars(
		select(CommCode)
		.options(selectinload(CommCode.children, recursion_depth=2))
		.filter(CommCode.type=='root')
	).one()
	ApiConfig().setCommCode(root) 

def addUser(session):
	session.add(User('test','test','test@a.com'))
	session.add(User('isitna','나광호','isitna@gmail.com'))
	session.add(Address(userId='isitna', emailAddress='na@aaa.com'))
	session.commit()

if __name__=='__main__':
	try:
		# C:/Users/NRJ/AppData/Local/Programs/Python/Python313/python SqliteTest.py
		api = ApiConfig()
		api.setDb(DATABASE_URL)
		api.info("✅ 데이터베이스 엔진 생성 완료")
		Base.metadata.create_all(bind=api.engine)
		with Session(api.engine) as session:
			# init_data(session)
			# add_code(session)
			# print(sys.getdefaultencoding())
			selectCommCode(session)
			auth = api.getCodeObject('auth')
			api.info(f'auth = {auth.detail()}')
			api.info(f'test ====== DATABASE_URL:{DATABASE_URL}')
			# addUser(session)
		result = api.exec('select * from item')
		data = result.fetchall()
		for row in data:
			print(f'>> row={row}')
	except Exception as e:
		ApiConfig().info(f"❌ 데이터베이스 실행오류 URL:{DATABASE_URL} 실패: {e}")
