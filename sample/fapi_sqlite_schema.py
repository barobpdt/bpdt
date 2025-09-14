from pydantic import BaseModel
from typing import Optional

#> Post
class PostBase(BaseModel):
	title : str
	description : Optional[str] = None

class PostCreate(PostBase):
	pass

class Post(PostBase):
	id : int
	owner_id  : int

	class Config:
		orm_mode = True


#> USER
class UserBase(BaseModel):
	name: str
	email: str

class UserCreate(UserBase):
	pass 

class User(UserBase):
	id : int
	is_active : bool
	posts : list[Post] = []

	class Config:
		orm_model = True

#> ItemPart
class ItemPartBase(BaseModel):
	title : str
	description : Optional[str] = None

class ItemPartCreate(ItemPartBase):
	pass

class ItemPart(ItemPartBase):
	id : int
	item_id  : int
	class Config:
		orm_mode = True

#> Item
class ItemBase(BaseModel):
	name: str
	description: str

class ItemCreate(ItemBase):
	pass 

class Item(ItemBase):
	id : int
	is_active : bool
	itemParts : list[ItemPart] = []
	class Config:
		orm_model = True        



