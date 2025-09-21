from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime 
import sqlalchemy as sa
import sqlalchemy.sql as ss 

from sqlalchemy import Column
from sqlalchemy import ForeignKey

from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import MappedAsDataclass
from sqlalchemy.orm import relationship
from sqlalchemy.orm import selectinload
from sqlalchemy.orm import Session
from sqlalchemy.orm.collections import attribute_keyed_dict
from sqlalchemy.orm.collections import attribute_mapped_collection

class Base(DeclarativeBase):
	pass 

'''
class User(Base):
	__tablename__ = "user"
	user_id = Column('user_id', sa.UUID, primary_key=True, default=sa.func.gen_random_uuid() )
	first_name = Column('first_name', sa.String)
	last_name = Column('last_name', sa.String)
	phone_number = Column('phone_number', sa.String(20), unique=True)
	email = Column('email', sa.String, unique=True)
	is_active = Column('is_active', sa.Boolean, server_default=ss.expression.true(), nullable=False)
	deleted = Column('deleted', sa.Boolean, server_default=ss.expression.false(), nullable=False)
	created_time = Column('created_time', sa.DateTime, server_default=sa.func.now(), nullable=False)
	updated_time = Column('updated_time', sa.DateTime, server_default=sa.func.now(), nullable=False)
	deleted_time = Column('deleted_time', sa.DateTime)
	feedbacks = relationship('UserFeedback', primaryjoin='User.user_id == UserFeedback.receiver_id', uselist=True, lazy='dynamic', backref='user_feedback')

class UserFeedback(Base):
	__tablename__ = "user_feedback"
	id = sa.Column(sa.Integer, primary_key=True, autoincrement=True)
	sender_id = sa.Column(sa.UUID, ForeignKey("user.user_id"), nullable=False)
	receiver_id = sa.Column(sa.UUID, ForeignKey("user.user_id"), nullable=False)
	sender = relationship("User", foreign_keys=[sender_id])
	receiver = relationship("User", foreign_keys=[receiver_id])
	rating = sa.Column(sa.DECIMAL, nullable=False)
	feedback = sa.Column(sa.String, nullable=False)
	date = sa.Column(sa.DateTime, server_default=sa.func.now(), nullable=False)
'''


class Item(Base):
	__tablename__ = "item"
	id = Column(sa.Integer, primary_key=True)
	notes = relationship(
		"Note",
		collection_class=attribute_mapped_collection("keyword"),
		cascade="all, delete-orphan",
	)


class Note(Base):
	__tablename__ = "note"
	id = Column(sa.Integer, primary_key=True)
	item_id = Column(sa.Integer, ForeignKey("item.id"), nullable=False)
	keyword = Column(sa.String)
	text = Column(sa.String)

	def __init__(self, keyword, text):
		self.keyword = keyword
		self.text = text

engine = sa.create_engine("sqlite://", echo=True)
Base.metadata.create_all(engine)
with Session(engine) as session: 
	 
	session.commit() 