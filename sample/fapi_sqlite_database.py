from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
from dotenv import load_dotenv
from fapi_logger import setup_logging
import os

logger = setup_logging()

'''
load_dotenv() 
# MySQL 데이터베이스 설정
DB_HOST = "106.246.249.162:23381"   # os.getenv("DB_HOST") 
DB_NAME = "secretguard"             # os.getenv("DB_NAME") 
DB_USER = "muknoori"                # os.getenv("DB_USER") 
DB_PASSWORD = "snflWkd22!"          # os.getenv("DB_PASSWD") 

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
'''
# 비동기 데이터베이스 URL
ASYNC_DATABASE_URLS = [ "sqlite:///./test.db" ]
DATABASE_URLS 		= [ "sqlite:///./test.db" ]

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

# 데이터베이스 엔진 초기화 (개선된 버전)
def initialize_database_engines():
	"""데이터베이스 엔진들을 초기화합니다."""	
	# 비동기 엔진 생성 시도
	global async_engine, engine, AsyncSessionLocal, SessionLocal
    
    # 비동기 엔진 생성 시도
    async_engine = None
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
		logger.info("⚠️ 동기 데이터베이스 엔진으로 대체합니다.")
	
	# 동기 엔진 생성 (비동기 실패 시 또는 호환성을 위해)
	try:
		engine = create_database_engine()
		SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
		logger.info("✅ 동기 데이터베이스 엔진 생성 완료")
	except Exception as e:
		logger.error(f"❌ 동기 데이터베이스 엔진 생성 실패: {e}")
		# 최후의 수단: 기본 설정으로 엔진 생성
		try:
			engine = create_engine(DATABASE_URL, pool_pre_ping=True, pool_recycle=300)
			SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
			logger.info("✅ 기본 설정으로 동기 데이터베이스 엔진 생성 완료")
		except Exception as e2:
			logger.error(f"❌ 모든 데이터베이스 연결 시도 실패: {e2}")
			raise e2

initialize_database_engines()


