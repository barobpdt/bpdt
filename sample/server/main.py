from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from sqlalchemy.pool import NullPool
import os
import logging
from logging.handlers import TimedRotatingFileHandler
import model 
from schema import User
from crud import DataCrud
from typing import List
import psycopg2
import uvicorn

'''
https://euclideanai.substack.com/p/fastapi-supabase-template-for-llm
@python.cmdPip('pip install fastapi_socketio')
@python.cmdPip('pip install supabase')
@python.cmdPip('pip install asyncpg')
@python.cmdPip('pip install psycopg2')
'''

# FastAPI 앱 생성
app = FastAPI(
	title="FastAPI SUPABASE CRUD Sample",
	description="A simple CRUD application using FastAPI and SUPABASE with QR User Management",
	version="1.0.0"
)

# CORS 미들웨어 추가
app.add_middleware(
	CORSMiddleware,
	allow_origins=["*"],
	allow_credentials=True,
	allow_methods=["*"],
	allow_headers=["*"],
)

# 로거 초기화 (데이터베이스 연결 전에 먼저 초기화)
def setup_logging():
	"""일자별 로그 설정"""
	# 로그 디렉토리 생성
	log_dir = "logs"
	if not os.path.exists(log_dir):
		os.makedirs(log_dir)
	
	# 로거 설정
	logger = logging.getLogger("SUPABASE_CRUD")
	logger.setLevel(logging.INFO)
	
	# 콘솔 핸들러
	console_handler = logging.StreamHandler()
	console_handler.setLevel(logging.INFO)
	console_formatter = logging.Formatter(
		'%(asctime)s - %(name)s - %(levelname)s - %(message)s'
	)
	console_handler.setFormatter(console_formatter)
	
	# 파일 핸들러 (일자별 로테이션)
	file_handler = TimedRotatingFileHandler(
		filename=os.path.join(log_dir, "SUPABASE_CRUD.log"),
		when="midnight",
		interval=1,
		backupCount=30,  # 30일간 보관
		encoding='utf-8'
	)
	file_handler.setLevel(logging.INFO)
	file_formatter = logging.Formatter(
		'%(asctime)s - %(name)s - %(levelname)s - %(funcName)s:%(lineno)d - %(message)s'
	)
	file_handler.setFormatter(file_formatter)
	
	# 핸들러 추가
	logger.addHandler(console_handler)
	logger.addHandler(file_handler)
	
	return logger

# 로거 초기화 (가장 먼저 실행)
logger = setup_logging()
crud = DataCrud(logger)

ASYNC_DATABASE_URL = 'postgresql+asyncpg://postgres.yskotbxdlxyzpnwhxucs:pass1812!!@aws-1-ap-northeast-2.pooler.supabase.com:5432/postgres'

def create_database_engine():
	try:
		logger.info(f"🔍 비동기 데이터베이스 연결 시도 {ASYNC_DATABASE_URL}")
		'''
		engine = create_async_engine(
			ASYNC_DATABASE_URL,
			pool_pre_ping=True,
			pool_recycle=300,
			pool_timeout=30,
			max_overflow=10,
			pool_size=5,
			client_encoding='utf8', poolclass=NullPool,
			echo=True,  # SQL 로그 비활성화 (성능 향상)
			connect_args={
				"connect_timeout": 30,
				"read_timeout": 30,
				"write_timeout": 30
			}
		)
		'''
		engine = create_async_engine(ASYNC_DATABASE_URL, echo=True)
		logger.info(f"✅ 비동기 데이터베이스 연결 성공!")
		return engine
	except Exception as e:
		logger.error(f"❌ 비동기 연결 실패: {e}")        
		raise e


engine = create_database_engine()
AsyncSessionLocal = async_sessionmaker(
	engine, 
	class_=AsyncSession, 
	expire_on_commit=False
)
logger.info("✅ 비동기 데이터베이스 엔진 생성 완료")

# 비동기 데이터베이스 세션 의존성
async def get_async_db():
	async with AsyncSessionLocal() as db:
		try:
			yield db
		except Exception as e:
			logger.error(f"⚠️ 데이터베이스 오류: {e}")
			await db.rollback()
			raise HTTPException(status_code=500, detail=f"Database error: {str(e)}")


@app.post("/user_add/")
async def addUser(user: model.User, db: AsyncSession = Depends(get_async_db)):
	db.add(User(**user.model_dump()))
	db.commit()
	return {"message": "User add successfully"}

@app.get("/user_list/")
async def get_users(offset:int=0, limit:int=50, db:AsyncSession = Depends(get_async_db)):
	return await crud.get_users(db, offset, limit)

if __name__=="__main__":
	uvicorn.run("main:app", host="0.0.0.0", port=7777, lifespan="on", reload=False)

