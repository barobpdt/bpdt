from sqlalchemy import select, insert, update, delete
from sqlalchemy.ext.asyncio import AsyncSession

import fapi_sqlite_model as model
import fapi_sqlite_schema as schema

from fapi_logger import setup_logging
logger = setup_logging()

############################ USER ############################
async def get_user(db: AsyncSession, user_id: int):
	result = await db.execute(select(model.User).where(model.User.id == user_id))
	return result.scalar_one_or_none()

async def get_user_by_email(db: AsyncSession, email: str):
	result = await db.execute(select(model.User).where(model.User.email == email))
	return result.scalar_one_or_none()

async def get_users(db: AsyncSession, skip:int=0, limit:int=50):
	result = await db.execute(select(model.User).offset(skip).limit(limit))
	return result.scalars().all()

async def create_user(db: AsyncSession, user:schema.UserCreate):
	db_user = model.User(**user.model_dump())
	db.add(db_user)
	await db.commit()
	await db.refresh(db_user)
	return db_user

async def update_user(db: AsyncSession, user: model.User, updated_user: schema.UserCreate):
	for key, value in updated_user.model_dump().items():
		setattr(user, key, value)
	await db.commit()
	await db.refresh(user)
	return user

async def delete_user(db: AsyncSession, user: model.User):
	await db.delete(user)
	await db.commit()

'''
############################ POST ############################
def get_post(db: Session, post_id: int):
	return db.query(model.Post).filter(model.Post.id == post_id).first()

def get_posts(db: Session, skip:int=0, limit: int=50):
	return db.query(model.Post).offset(skip).limit(limit).all()

def create_user_post(db:Session, post:schema.PostCreate, user_id : int):
	db_post = model.Post(**post.model_dump(), owner_id=user_id )
	db.add(db_post)
	db.commit()
	db.refresh(db_post)
	return db_post

def update_post(db: Session, post: model.Post, updated_post: schema.PostCreate):
	for key, value in updated_post.model_dump().items():
		setattr(post, key, value)
	db.commit()
	db.refresh(post)
	return post

def delete_post(db: Session, post: model.Post):
	db.delete(post)
	db.commit()
'''
