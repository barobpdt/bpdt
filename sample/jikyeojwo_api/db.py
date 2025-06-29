from sqlalchemy import create_engine, Column, Integer, String, Text, Boolean, DateTime, Numeric
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from sqlalchemy.orm import sessionmaker, Session, declarative_base
from sqlalchemy.sql import text
from sqlalchemy.future import select
from log import logger

# MySQL 데이터베이스 설정
DB_HOST = "106.246.249.162:23381"
DB_NAME = "secretguard"
DB_USER = "muknoori"
DB_PASSWORD = "snflWkd22!"

# 호스트와 포트 분리
if ':' in DB_HOST:
	host, port = DB_HOST.split(':')
	port = int(port)
else:
	host = DB_HOST
	port = 3306

# 비동기 데이터베이스 URL
ASYNC_DATABASE_URLS = [
	f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
	f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
	f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
]

# 동기 데이터베이스 URL (기존 호환성용)
DATABASE_URLS = [
	f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
	f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
	f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
]

# 첫 번째 연결 문자열 사용
ASYNC_DATABASE_URL = ASYNC_DATABASE_URLS[0]
DATABASE_URL = DATABASE_URLS[0]

# 비동기 데이터베이스 엔진 생성
def create_async_database_engine():
	"""비동기 데이터베이스 엔진을 생성합니다."""
	for i, url in enumerate(ASYNC_DATABASE_URLS, 1):
		try:
			logger.info(f"🔍 비동기 데이터베이스 연결 시도 {i}: {url}")
			engine = create_async_engine(
				url,
				pool_pre_ping=True,
				pool_recycle=300,
				pool_timeout=30,
				max_overflow=10,
				pool_size=5,
				echo=False,  # SQL 로그 비활성화 (성능 향상)
				connect_args={
					"connect_timeout": 30,
					"read_timeout": 30,
					"write_timeout": 30
				}
			)
			
			logger.info(f"✅ 비동기 데이터베이스 연결 성공! (연결 {i})")
			return engine
			
		except Exception as e:
			logger.error(f"❌ 비동기 연결 {i} 실패: {e}")
			if i == len(ASYNC_DATABASE_URLS):
				logger.error("❌ 모든 비동기 연결 시도 실패")
				raise e
			continue

# 동기 데이터베이스 엔진 생성 (기존 호환성용)
def create_database_engine():
	"""동기 데이터베이스 엔진을 생성합니다."""
	for i, url in enumerate(DATABASE_URLS, 1):
		try:
			logger.info(f"🔍 동기 데이터베이스 연결 시도 {i}: {url}")
			
			engine = create_engine(
				url,
				pool_pre_ping=True,
				pool_recycle=300,
				pool_timeout=30,
				max_overflow=10,
				pool_size=5,
				connect_args={
					"connect_timeout": 30,
					"read_timeout": 30,
					"write_timeout": 30
				}
			)
			
			# 연결 테스트
			with engine.connect() as conn:
				conn.execute(text("SELECT 1"))
			
			logger.info(f"✅ 동기 데이터베이스 연결 성공! (연결 {i})")
			return engine
			
		except Exception as e:
			logger.error(f"❌ 동기 연결 {i} 실패: {e}")
			if i == len(DATABASE_URLS):
				logger.error("❌ 모든 동기 연결 시도 실패")
				raise e
			continue

global async_engine
global engine
# 비동기 엔진 생성
try:
	async_engine = create_async_database_engine()
	AsyncSessionLocal = async_sessionmaker(
		async_engine, 
		class_=AsyncSession, 
		expire_on_commit=False
	)
	logger.info("✅ 비동기 데이터베이스 엔진 생성 완료")
except Exception as e:
	logger.error(f"❌ 비동기 데이터베이스 엔진 생성 실패: {e}")
	# 동기 엔진으로 대체
	try:
		engine = create_database_engine()
		SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
		logger.info("✅ 동기 데이터베이스 엔진으로 대체")
	except Exception as e2:
		logger.error(f"❌ 동기 데이터베이스 엔진도 생성 실패: {e2}")
		# 기본 엔진 생성
		engine = create_engine(DATABASE_URL, pool_pre_ping=True, pool_recycle=300)
		SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# 동기 엔진도 생성 (기존 호환성용)
try:
	engine = create_database_engine()
	SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
except Exception as e:
	logger.error(f"❌ 동기 데이터베이스 엔진 생성 실패: {e}")
	# 기본 엔진 생성
	engine = create_engine(DATABASE_URL, pool_pre_ping=True, pool_recycle=300)
	SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# 데이터베이스 세션 의존성
def get_db():
	db = SessionLocal()
	try:
		yield db
	except Exception as e:
		logger.error(f"⚠️ 데이터베이스 세션 오류: {e}")
		db.rollback()
		raise HTTPException(status_code=500, detail=f"Database error: {str(e)}")
	finally:
		db.close()

# 비동기 데이터베이스 세션 의존성
async def get_async_db():
	async with AsyncSessionLocal() as db:
		try:
			yield db
		except Exception as e:
			logger.error(f"⚠️ 비동기 데이터베이스 세션 오류: {e}")
			await db.rollback()
			raise HTTPException(status_code=500, detail=f"Database error: {str(e)}")

# 데이터베이스 테이블 생성
async def create_tables_async():
	try:
		async with async_engine.begin() as conn:
			await conn.run_sync(Base.metadata.create_all)
		logger.info("✅ 비동기 데이터베이스 테이블이 성공적으로 생성되었습니다.")
	except Exception as e:
		logger.warning(f"⚠️ 비동기 테이블 생성 중 오류 발생: {e}")
		logger.info("⚠️ 기존 테이블을 사용합니다.")

def create_tables():
	try:
		Base.metadata.create_all(bind=engine)
		logger.info("✅ 동기 데이터베이스 테이블이 성공적으로 생성되었습니다.")
	except Exception as e:
		logger.warning(f"⚠️ 동기 테이블 생성 중 오류 발생: {e}")
		logger.info("⚠️ 기존 테이블을 사용합니다.")

# 애플리케이션 시작 시 테이블 생성 (오류 처리 개선)
async def init_database():
	"""데이터베이스 초기화"""
	try:
		await create_tables_async()
	except Exception as e:
		logger.warning(f"⚠️ 비동기 테이블 생성 실패: {e}")
		try:
			create_tables()
		except Exception as e2:
			logger.warning(f"⚠️ 동기 테이블 생성도 실패: {e2}")
		logger.info("⚠️ 애플리케이션은 계속 실행되지만 일부 기능이 제한될 수 있습니다.")

async def close_database():
	# 비동기 엔진 종료
	if 'async_engine' in globals():
		await async_engine.dispose()
		logger.info("✅ 비동기 데이터베이스 연결 종료")
	
	# 동기 엔진 종료
	if 'engine' in globals():
		engine.dispose()
		logger.info("✅ 동기 데이터베이스 연결 종료")
