from sqlalchemy import Integer, String, Boolean, ForeignKey, Column
from sqlalchemy.orm import mapped_column, relationship
from sqlalchemy.orm import declarative_base

Base = declarative_base()

class Item(Base):
	__tablename__ = "items"
	#__table_args__ = {'extend_existing': True}
	id = mapped_column(Integer, primary_key=True, index=True)
	name = mapped_column(String, index=True)
	description = mapped_column(String) 	# Column(String)

class ItemPart(Base):
	__tablename__ = "items_part"
	id = mapped_column(Integer, primary_key=True, autoincrement=True)
	title = mapped_column(String(255), nullable=False)
	description = mapped_column(String(255))
	item_id = mapped_column(Integer, ForeignKey("items.id"))

class User(Base):
	__tablename__ = "users"
	id = mapped_column(Integer, primary_key=True, autoincrement=True)
	name = mapped_column(String(255), nullable=False)
	email = mapped_column(String(255), unique=True, nullable=False)
	posts = relationship("Post", back_populates="owner", cascade='delete')
	is_active = mapped_column(Boolean,default=False)

class Post(Base):
	__tablename__ = "posts"
	id = mapped_column(Integer, primary_key=True, autoincrement=True)
	title = mapped_column(String(255), nullable=False)
	description = mapped_column(String(255))
	owner_id = mapped_column(Integer, ForeignKey("users.id"))
	owner = relationship("User", back_populates="posts")

# Create tables
# Base.metadata.create_all(bind=engine)
