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

class TreeNode(MappedAsDataclass, Base):
	__tablename__ = "tree"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("tree.id"), init=False
	)
	code: Mapped[str]
	title: Mapped[str]
	type: Mapped[str]
	ref: Mapped[Optional[str]] = mapped_column(nullable=True)
	data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))
	created_at: Mapped[datetime] = mapped_column(server_default=sa.func.now())
	updated_at: Mapped[datetime | None] = mapped_column(onupdate=sa.func.now())

	children: Mapped[Dict[str, TreeNode]] = relationship(
		cascade="all, delete-orphan",
		back_populates="parent",
		collection_class=attribute_keyed_dict("code"),
		init=False,
		repr=False,
	)

	parent: Mapped[Optional[TreeNode]] = relationship(
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

	def __repr__(self):
		return "Tree(code=%r)" % self.code

	def dump(self, indent: int = 0) -> str:
		return (
			"   " * indent
			+ repr(self)
			+ "\n"
			+ "".join([c.dump(indent + 1) for c in self.children.values()])
		)
	
if __name__=='__main__':
	try:	
		dburl = 'postgresql://postgres.yskotbxdlxyzpnwhxucs:pass1812!!@aws-1-ap-northeast-2.pooler.supabase.com:5432/postgres'
		# Example transaction mode string:
		
		engine = create_engine(dburl, client_encoding='utf8', poolclass=NullPool, echo=True)
		'''		
		Base.metadata.create_all(engine)
		with Session(engine) as session:
			root = TreeNode('root', 'base', title="rootnode")
			TreeNode('users', 'users', title='user connected info', parent=root)
			users = root.children['users']
			TreeNode('u1', 'user', title='u1', ref='json', data='{"id":"test"}', parent=users)
			print(f"Tree after save:\n{root.dump()} {users}")
			session.add(root)
			session.commit()
			
		'''
		with Session(engine) as session:
			# list = session.query(TreeNode).all()
			# print(f"Full Tree:\n{list}")
			root = session.scalars(
				select(TreeNode)
				.options(selectinload(TreeNode.children, recursion_depth=2))
				.filter(TreeNode.type=='base', TreeNode.code=='root')
			).one()
			users = root.children['users'] 
			for key in users.children:
				user = users.children[key]
				print(f"user => {user}")
			print(f"Full Tree:\n{root.dump()} {users.__dict__}")
	except Exception as e:
		print(f"exception => {e}")
