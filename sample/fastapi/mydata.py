from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime
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

import asyncio
from sqlalchemy.ext.asyncio import async_sessionmaker
from sqlalchemy.ext.asyncio import AsyncAttrs
from sqlalchemy.ext.asyncio import create_async_engine
from sqlalchemy import func

from sqlalchemy import Column
from sqlalchemy import Integer
from sqlalchemy import MetaData
from sqlalchemy import Table

from sqlalchemy.orm import sessionmaker

class Base(DeclarativeBase):
	pass

meta = MetaData()
t1 = Table("t1", meta, Column("id", Integer, primary_key=True), Column("name", String) )


class ProxyDict:
	def __init__(self, parent, collection_name, childclass, keyname):
		self.parent = parent
		self.collection_name = collection_name
		self.childclass = childclass
		self.keyname = keyname

	@property
	def collection(self):
		return getattr(self.parent, self.collection_name)

	def keys(self):
		descriptor = getattr(self.childclass, self.keyname)
		return [x[0] for x in self.collection.values(descriptor)]

	def __getitem__(self, key):
		print(f"@@ key : {self.keyname} = {key}")
		x = self.collection.filter_by(**{self.keyname: key}).first()
		if x:
			return x
		else:
			raise KeyError(key)
	def __setitem__(self, key, value):
		try:
			existing = self[key]
			self.collection.remove(existing)
		except KeyError:
			pass
		self.collection.append(value)

class Parent(Base):
	__tablename__ = "parent"
	id = Column(Integer, primary_key=True)
	name = Column(String(50))
	_collection = relationship(
		"Child", lazy="dynamic", cascade="all, delete-orphan"
	)
	@property
	def child_map(self):
		return ProxyDict(self, "_collection", Child, "key")


class Child(Base):
	__tablename__ = "child"
	id = Column(Integer, primary_key=True)
	key = Column(String(50))
	parent_id = Column(Integer, ForeignKey("parent.id"))
	def __repr__(self):
		return "Child(key=%r)" % self.key

class TreeNode(MappedAsDataclass, Base):
	__tablename__ = "tree"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("tree.id"), init=False
	)
	name: Mapped[str]

	children: Mapped[Dict[str, TreeNode]] = relationship(
		cascade="all, delete-orphan",
		back_populates="parent",
		collection_class=attribute_keyed_dict("name"),
		init=False,
		repr=False,
	)

	parent: Mapped[Optional[TreeNode]] = relationship(
		back_populates="children", remote_side=id, default=None
	)

	def dump(self, _indent: int = 0) -> str:
		return (
			"   " * _indent
			+ repr(self)
			+ "\n"
			+ "".join([c.dump(_indent + 1) for c in self.children.values()])
		)

class Order(Base):
	__tablename__ = "order"
	order_id: Mapped[int] = mapped_column(primary_key=True)
	customer_name: Mapped[str] = mapped_column(String(30))
	order_date: Mapped[datetime] = mapped_column(default=datetime.now())
	order_items: Mapped[list[OrderItem]] = relationship(
		cascade="all, delete-orphan", backref="order"
	)
	def __init__(self, customer_name: str) -> None:
		self.customer_name = customer_name


class Item(Base):
	__tablename__ = "item"
	item_id: Mapped[int] = mapped_column(primary_key=True)
	description: Mapped[str] = mapped_column(String(30))
	price: Mapped[float]

	def __init__(self, description: str, price: float) -> None:
		self.description = description
		self.price = price

	def __repr__(self) -> str:
		return "Item({!r}, {!r})".format(self.description, self.price)


class OrderItem(Base):
	__tablename__ = "orderitem"
	order_id: Mapped[int] = mapped_column(
		ForeignKey("order.order_id"), primary_key=True
	)
	item_id: Mapped[int] = mapped_column(
		ForeignKey("item.item_id"), primary_key=True
	)
	price: Mapped[float]

	def __init__(self, item: Item, price: float | None = None) -> None:
		self.item = item
		self.price = price or item.price

	item: Mapped[Item] = relationship(lazy="joined")

class A(Base):
	__tablename__ = "a"
	id: Mapped[int] = mapped_column(primary_key=True)
	data: Mapped[Optional[str]]
	# create_date: Mapped[datetime.datetime] = mapped_column(server_default=func.now() )
	create_date: Mapped[datetime] = mapped_column(default=datetime.now() )
	bs: Mapped[List[B]] = relationship()


class B(Base):
	__tablename__ = "b"
	id: Mapped[int] = mapped_column(primary_key=True)
	a_id: Mapped[int] = mapped_column(ForeignKey("a.id"))
	data: Mapped[Optional[str]]

async def async_main():
	"""Main program function."""

	engine = create_async_engine(
		"postgresql+asyncpg://scott:tiger@localhost/test",
		echo=True,
	)
	async with engine.begin() as conn:
		await conn.run_sync(Base.metadata.drop_all)
	async with engine.begin() as conn:
		await conn.run_sync(Base.metadata.create_all)

	# expire_on_commit=False will prevent attributes from being expired
	# after commit.
	async_session = async_sessionmaker(engine, expire_on_commit=False)

	async with async_session() as session:
		async with session.begin():
			session.add_all(
				[
					A(bs=[B(), B()], data="a1"),
					A(bs=[B()], data="a2"),
					A(bs=[B(), B()], data="a3"),
				]
			)

		# for relationship loading, eager loading should be applied.
		stmt = select(A).options(selectinload(A.bs))

		# AsyncSession.execute() is used for 2.0 style ORM execution
		# (same as the synchronous API).
		result = await session.scalars(stmt)

		# result is a buffered Result object.
		for a1 in result:
			print(a1)
			print(f"created at: {a1.create_date}")
			for b1 in a1.bs:
				print(b1)

		# for streaming ORM results, AsyncSession.stream() may be used.
		result = await session.stream(stmt)

		# result is a streaming AsyncResult object.
		async for a1 in result.scalars():
			print(a1)
			for b1 in a1.bs:
				print(b1)

		result = await session.scalars(select(A).order_by(A.id))

		a1 = result.first()

		a1.data = "new data"

		await session.commit()

		# use the AsyncAttrs interface to accommodate for a lazy load
		for b1 in await a1.awaitable_attrs.bs:
			print(b1)

exam = 'proxy'
if exam in ('tree','order'):
	engine = create_engine("sqlite://", echo=True)
	Base.metadata.create_all(engine)

if exam=='tree':
	print("Creating Tree Table:")
	with Session(engine) as session:
		node = TreeNode("rootnode")
		TreeNode("node1", parent=node)
		TreeNode("node3", parent=node)

		node2 = TreeNode("node2")
		TreeNode("subnode1", parent=node2)
		node.children["node2"] = node2
		TreeNode("subnode2", parent=node.children["node2"])

		print(f"Created new tree structure:\n{node.dump()}")

		print("flush + commit:")

		session.add(node)
		session.commit()

		print(f"Tree after save:\n{node.dump()}")

		session.add_all(
			[
				TreeNode("node4", parent=node),
				TreeNode("subnode3", parent=node.children["node4"]),
				TreeNode("subnode4", parent=node.children["node4"]),
				TreeNode(
					"subsubnode1",
					parent=node.children["node4"].children["subnode3"],
				),
			]
		)

		# remove node1 from the parent, which will trigger a delete
		# via the delete-orphan cascade.
		del node.children["node1"]

		print("Removed node1.  flush + commit:")
		session.commit()

		print("Tree after save, will unexpire all nodes:\n")
		print(f"{node.dump()}")

	with Session(engine) as session:
		print(
			"Perform a full select of the root node, eagerly loading "
			"up to a depth of four"
		)
		node = session.scalars(
			select(TreeNode)
			.options(selectinload(TreeNode.children, recursion_depth=4))
			.filter(TreeNode.name == "rootnode")
		).one()

		print(f"Full Tree:\n{node.dump()}")

		print("Marking root node as deleted, flush + commit:")

		session.delete(node)
		session.commit() 
	#session
elif exam=='order':
	# create catalog
	with Session(engine) as session:
		tshirt, mug, hat, crowbar = (
			Item("SA T-Shirt", 10.99),
			Item("SA Mug", 6.50),
			Item("SA Hat", 8.99),
			Item("MySQL Crowbar", 16.99),
		)
		session.add_all([tshirt, mug, hat, crowbar])
		session.commit()

		# create an order
		order = Order("john smith")

		# add three OrderItem associations to the Order and save
		order.order_items.append(OrderItem(mug))
		order.order_items.append(OrderItem(crowbar, 10.99))
		order.order_items.append(OrderItem(hat))
		session.add(order)
		session.commit()

		# query the order, print items
		order = session.scalars(
			select(Order).filter_by(customer_name="john smith")
		).one()
		print(
			[
				(order_item.item.description, order_item.price) for order_item in order.order_items
			]
		)

		# print customers who bought 'MySQL Crowbar' on sale
		q = (
			select(Order)
			.join(OrderItem)
			.join(Item)
			.where(
				Item.description == "MySQL Crowbar",
				Item.price > OrderItem.price,
			)
		)

		print([order.customer_name for order in session.scalars(q)])
	#session
elif exam=='proxy':
	p1 = Parent(name="p1")
	print("\n---------begin setting nodes, autoflush occurs\n")
	p1.child_map["k1"] = Child(key="k1")
	p1.child_map["k2"] = Child(key="k2")

	# this will autoflush the current map.
	# ['k1', 'k2']
	print("\n---------print keys - flushes first\n")
	print(list(p1.child_map.keys()))

	# k1
	print("\n---------print 'k1' node\n")
	print(p1.child_map["k1"])

	print("\n---------update 'k2' node - must find existing, and replace\n")
	p1.child_map["k2"] = Child(key="k2")

	print("\n---------print 'k2' key - flushes first\n")
	# k2
	print(p1.child_map["k2"])

	print("\n---------print all child nodes\n")
	# [k1, k2b]
	# print(sess.query(Child).all())
#end exam