from sqlalchemy import select, insert, update, delete
from sqlalchemy.ext.asyncio import AsyncSession
from schema import User

class DataCrud:
	def __init__(self, logger):
		self.logger = logger
	
	async def get_users(self, db:AsyncSession, offset:int=0, limit:int=50):
		result = await db.scalars(select(User).offset(offset).limit(limit))
		return result.all()