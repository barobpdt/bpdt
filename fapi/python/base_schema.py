from __future__ import annotations

from jose import JWTError, jwt
from typing import List
from typing import Optional
from datetime import datetime 
from typing import Dict

from sqlalchemy import select, ForeignKey, JSON, func
from sqlalchemy.ext.asyncio import AsyncSession
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
import asyncio
from ApiConfig import ApiConfig

class Base(DeclarativeBase):
	pass

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

class UserAdmin(Base):
	__tablename__ = "user_admin"
	userId: Mapped[str] = mapped_column(primary_key=True)
	userName: Mapped[str]
	fullname: Mapped[Optional[str]]
	email: Mapped[Optional[str]]
	authCode: Mapped[str] = mapped_column(default='01')
	createdDtm: Mapped[datetime] = mapped_column(server_default=func.now())
	updatedDtm: Mapped[datetime | None] = mapped_column(onupdate=func.now())

	def __init__(self, id, name, email):
		self.userId = id
		self.userName = name
		self.email = email
	def __repr__(self) -> str:
		return f"UserAdmin(id={self.userId!r}, name={self.userName!r}, fullname={self.fullname!r})"

async def connectDb():
	try:
		api = ApiConfig()
		if api.isConnect():
			return
		print(f"connect start ..........")
		ASYNC_DATABASE_URL = 'postgresql+asyncpg://postgres.yskotbxdlxyzpnwhxucs:pass1812!!@aws-1-ap-northeast-2.pooler.supabase.com:5432/postgres'
		api.setAyncDb(ASYNC_DATABASE_URL)
		# await create_tables(api.async_engine)
		print(f"@@ connectDb URL:{ASYNC_DATABASE_URL} {api.async_engine}")
		# Base.metadata.create_all(bind=api.async_engine)
		async with api.async_engine.begin() as conn:
			await conn.run_sync(Base.metadata.create_all)
	except Exception as e:
		print(f"connectDb exception {e}")
		api.err(f"❌ 데이터베이스 오류 URL:{ASYNC_DATABASE_URL} 실패: {e}")

async def addUserAdmin():	
	api = ApiConfig()
	try:
		await connectDb()
		async with AsyncSession(api.async_engine) as session:
			result = await session.execute(
				select(UserAdmin).where(UserAdmin.userId=='isitna')
			)
			one = result.scalar_one_or_none()
			if one is None:		 
				user = UserAdmin('isitna','na','isitna@gmail.com')
				print(f"@@user={user}")
				session.add(user)
				await session.commit()
				# await session.refresh(user)
	except Exception as e:
		print(f"@@addUserAdmin exception : {e}")

async def selectUserAdmin():
	try:
		api = ApiConfig()
		async with Session(api.async_engine) as session:
			result = await session.execute(
				select(UserAdmin)
			)
			users = result.scalars().all()
			print(f'select useradmin = {users}')
	except Exception as e:
		pass

if __name__=='__main__':
	try:
		asyncio.run(addUserAdmin())
	except Exception as e:
		pass
