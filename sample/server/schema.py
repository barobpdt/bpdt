from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime 
import sqlalchemy as sa
import sqlalchemy.sql as ss
from sqlalchemy import create_engine
from sqlalchemy import ForeignKey
from sqlalchemy import select
from sqlalchemy import String

from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import MappedAsDataclass
from sqlalchemy.orm import relationship
from sqlalchemy.orm import selectinload
from sqlalchemy.orm import Session
from sqlalchemy.orm.collections import attribute_keyed_dict
from sqlalchemy.pool import NullPool
from sqlalchemy.ext.mutable import MutableDict
import json

class Base(DeclarativeBase):
	pass

class User(Base):
	__tablename__ = "user"
	userId: Mapped[str] = mapped_column(primary_key=True, index=True)
	name: Mapped[str]
	email: Mapped[str]
	ref: Mapped[Optional[str]] = mapped_column(nullable=True)
	data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))
	created_at: Mapped[datetime] = mapped_column(server_default=sa.func.now())
	updated_at: Mapped[datetime | None] = mapped_column(onupdate=sa.func.now())
	def __init__(self, **kwargs):
		try:
			data = kwargs['data'] if 'data' in kwargs else None
			if isinstance(data, dict):
				self.data = data
			elif data:
				self.data = json.loads(data)
			self.userId = kwargs['userId']
			self.name = kwargs['name']
			self.email = kwargs['email']
			self.ref = kwargs['ref'] if 'ref' in kwargs else ''
			print(f'user init {kwargs}')
		except Exception as e:
			print(f'user init param:{kwargs} error {e}')

	def __repr__(self):
		return f"User(userId={self.userId}, name={self.name})"

if __name__=='__main__':
	dburl = 'postgresql://postgres.yskotbxdlxyzpnwhxucs:pass1812!!@aws-1-ap-northeast-2.pooler.supabase.com:5432/postgres'
	engine = create_engine(dburl, client_encoding='utf8', poolclass=NullPool, echo=True)
	# Base.metadata.create_all(engine) 테이블 생성
	# https://baeji-develop.tistory.com/137 조회 함수 설명
	with Session(engine) as session:
		try:
			result = session.scalars(select(User).offset(0).limit(50))
			users = result.all()
			print(f"users={users}")
			if users:
				for user in users:
					print(f"user->{user}")
			user =User(userId='test1', name='test1', email='test@gmail.com')
			session.add(user)
			session.commit()
			print(f"user={user}")
		except Exception as e:
			print(f"db select error {e}")