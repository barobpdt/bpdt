from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime 
import sqlalchemy as sa
import sqlalchemy.sql as ss
from sqlalchemy import ForeignKey

from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import MappedAsDataclass
from sqlalchemy.orm import relationship
from sqlalchemy.orm.collections import attribute_keyed_dict
from sqlalchemy.ext.mutable import MutableDict
import json

class Base(DeclarativeBase):
	pass
	
class CommCode(MappedAsDataclass, Base):
	__tablename__ = "comm_code"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("comm_code.id"), init=False
	)
	code: Mapped[str]
	title: Mapped[str]
	type: Mapped[str]
	ref: Mapped[Optional[str]] = mapped_column(nullable=True)
	data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))
	created_at: Mapped[datetime] = mapped_column(server_default=sa.func.now())
	updated_at: Mapped[datetime | None] = mapped_column(onupdate=sa.func.now())

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

	def __init__(self, code, type, **kwargs):
		data = kwargs['data'] if 'data' in kwargs else None
		if isinstance(data, dict):
			self.data = data
		elif data:
			self.data = json.loads(data)
		self.code = code
		self.type = type
		self.title = kwargs['title']
		self.ref = kwargs['ref'] if 'ref' in kwargs else ''
		self.parent = kwargs['parent'] if 'parent' in kwargs else None
		print(f'init {self} {kwargs}')

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
		
	def __repr__(self):
		return f"CC(code={self.code},title={self.title})"

	def dump(self, indent: int = 0) -> str:
		return (
			"   " * indent
			+ repr(self)
			+ "\n"
			+ "".join([c.dump(indent + 1) for c in self.children.values()])
		)
	