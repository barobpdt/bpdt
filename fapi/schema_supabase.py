from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime 
from sqlalchemy import create_engine, func
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

class Base(DeclarativeBase):
	pass

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


class TreeNode(MappedAsDataclass, Base):
	__tablename__ = "tree"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("tree.id"), init=False
	)
	code: Mapped[str]
	title: Mapped[str]
	type: Mapped[str]
	
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
		# dburl = 'postgresql://postgres:pass1812!!@db.yskotbxdlxyzpnwhxucs.supabase.co:5432/postgres'
		dburl = 'postgresql://postgres.yskotbxdlxyzpnwhxucs:pass1812!!@aws-1-ap-northeast-2.pooler.supabase.com:5432/postgres'
		# Example transaction mode string:
		
		engine = create_engine(dburl, client_encoding='utf8', poolclass=NullPool, echo=True)

		# engine = create_engine(dburl, echo=True)
		Base.metadata.create_all(engine)
		with Session(engine) as session:
			node = TreeNode(title="rootnode", code='root', type='base')
			TreeNode(title="test", code='test', type='room', parent=node)
			TreeNode(title="new room", code='newRoom', type='room', parent=node)
			TreeNode(code='u1', title='aaa', type='user', parent=node.children['test'])
			TreeNode(code='u2', title='bbb', type='user', parent=node.children['newRoom'])
			'''
			TreeNode("node1", parent=node)
			TreeNode("node3", parent=node)
			node2 = TreeNode("node2")
			TreeNode("subnode1", parent=node2)
			node.children["node2"] = node2
			TreeNode("subnode2", parent=node.children["node2"])
			'''

			print(f"Created new tree structure:\n{node.dump()}")

			print("flush + commit:")
			session.add(node)
			session.commit()

			print(f"Tree after save:\n{node.dump()}")
			# del node.children["node1"]

		with Session(engine) as session:
			listAll = session.query(TreeNode).all()
			print([
				(cur.type, cur.code, cur.title) for cur in listAll
			]) 
			node = session.scalars(
				select(TreeNode)
				.options(selectinload(TreeNode.children, recursion_depth=2))
				.filter(TreeNode.type=='room', TreeNode.code=='test')
			).one()
			print(f"Full Tree:\n{node.dump()}")
			# session.delete(node)
			session.commit() 
		#session 
	except Exception as e:
		print(f"exception {e}")
