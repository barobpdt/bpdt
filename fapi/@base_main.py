from fastapi import FastAPI, HTTPException, Depends, status, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates

from sqlalchemy.orm import Session
from jose import JWTError, jwt
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from typing import List

import @base_sqlite_schema as schema
import @base_sqlite_model as model
import @base_sqlite_crud as crud
import @base_sqlite_jwt as auth
from @base_logger import setup_logging

import os
import asyncio
from datetime import datetime, timedelta

@{imports}

localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
sys.path.insert(0, localPath)


# 로거 초기화 (가장 먼저 실행)
logger = setup_logging()
logger.info(f'start @base API => PID: {os.getpid()}')

# 비동기 데이터베이스 URL
ASYNC_DATABASE_URLS = [ "sqlite+aiosqlite:///./test.db" ]
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
			
			engine = create_async_engine(url, echo=True)
			'''	
				url,
				pool_pre_ping=True,
				pool_recycle=300,
				pool_timeout=30,
				max_overflow=10,
				pool_size=5,
				echo=True,  # SQL 로그 비활성화 (성능 향상 False 설정)
				connect_args={"connect_timeout": 30, "read_timeout": 30, "write_timeout": 30}
			'''
			
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
			
			engine = create_engine(url)
			
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

# Dependency to get the database session

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

initialize_database_engines()

model.Base.metadata.create_all(bind=engine)

# FastAPI app instance
app = FastAPI(
	title="FastAPI @base API",
	description="@base Management",
	version="@version|0.0.1",
	docs_url='/swagger', openapi_url='/api/openapi.json'
    @root_path ? {, root_path='/api/v1'}
)

app.add_middleware(
	CORSMiddleware,
	allow_origins=["*"],
	allow_credentials=True,
	allow_methods=["*"],
	allow_headers=["*"],
)
templates = Jinja2Templates(directory="templates")

# JWT Bearer 토큰
security = HTTPBearer()
server_running = True

async def get_current_user(credentials: HTTPAuthorizationCredentials = Depends(security)):
	"""현재 인증된 사용자 조회"""
	credentials_exception = HTTPException(
		status_code=status.HTTP_401_UNAUTHORIZED,
		detail="Could not validate credentials",
		headers={"WWW-Authenticate": "Bearer"},
	)
	try:
		payload = jwt.decode(credentials.credentials, SECRET_KEY, algorithms=[ALGORITHM])
		userid: str = payload.get("sub")
		if userid is None:
			raise credentials_exception
		token_data = auth.TokenData(userid=userid)
	except JWTError:
		raise credentials_exception
	
	adminUser = auth.get_adminUser(userid=token_data.userid)
	if adminUser is None:
		raise credentials_exception
	return adminUser

async def get_current_active_user(current_user: auth.AdminUser = Depends(get_current_user)):
	"""현재 활성 사용자 조회"""
	if current_user.disabled:
		raise HTTPException(status_code=400, detail="Inactive user")
	return current_user

async def shutdown_background():
	"""백그라운드에서 서버 종료"""
	await asyncio.sleep(2)  # 2초 대기
	logger.info("🛑 서버 종료 실행")
	sys.exit(0)
	
#> EndPoint
@app.post("/shutdown")
async def shutdown_server(current_user: auth.AdminUser = Depends(get_current_active_user)):
	"""서버를 안전하게 종료합니다. (인증된 사용자만 가능)"""
	global server_running
	logger.info(f"🛑 서버 종료 요청 수신: user={current_user.username}")
	server_running = False
	
	# 백그라운드에서 서버 종료
	asyncio.create_task(shutdown_background())
	
	return {
		"message": "서버 종료 요청이 수신되었습니다. 잠시 후 서버가 종료됩니다.",
		"status": "shutting_down",
		"requested_by": current_user.username
	}

@app.get('/login',response_class=HTMLResponse)
def login(request : Request):
    return templates.TemplateResponse("login.html", {"request": request})

@app.post("/login", response_model=auth.Token)
async def login_for_access_token(userInfo: auth.UserLogin):
	"""사용자 로그인 및 JWT 토큰 발급"""
	logger.info(f"로그인 시도: username={userInfo.userid}")
	try:
		user = auth.authenticate_user(userInfo.userid, userInfo.password)
		if not user:
			logger.warning(f"로그인 실패: 잘못된 사용자명 또는 비밀번호 - {userInfo.username}")
			raise HTTPException(
				status_code=status.HTTP_401_UNAUTHORIZED,
				detail="Incorrect username or password",
				headers={"WWW-Authenticate": "Bearer"},
			)        
		access_token_expires = timedelta(minutes=auth.ACCESS_TOKEN_EXPIRE_MINUTES)
		access_token = auth.create_access_token(
			data={"sub": user.userid}, expires_delta=access_token_expires
		)        
		logger.info(f"로그인 성공: user={userInfo}")
		return {"access_token": access_token, "token_type": "bearer"}
	except HTTPException:
		raise
	except Exception as e:
		logger.error(f"로그인 처리 중 오류: {e}")
		raise HTTPException(status_code=500, detail="Login processing error")

@modelChanage {
#CRUD MODEL> @(modelName) Endpoint ----------------------------
@app.get("/@(modelName)_list/", response_model=List[model.@(modelName)])
async def get_users(skip:int=0, limit:int=0, db:AsyncSessionLocal = Depends(get_async_db)):
	return await crud.get_users(db,  skip=skip,limit=limit)

@app.get("/@(modelName)/{@(modelName)id}/")
async def get_user(user_id:int, db:AsyncSessionLocal = Depends(get_async_db)):
	user_sel = await crud.get_@(modelName)(db, @(modelName)id )
	if user_sel is None:
		raise HTTPException(status_code=404, detail="User not found")
	return user_sel

@app.post("/users/")
async def post_user(user:schema.UserCreate, db:AsyncSessionLocal = Depends(get_async_db)):
	user_sel = await crud.get_user_by_email(db, email=user.email)
	if user_sel:
		raise HTTPException(status_code=400, detail="Email already registered")
	user_added = await crud.create_user(db=db,user=user)
	logger.info(f"@@ uer add ok {user_added}")
	return {"message": "User add successfully"}

@app.put("/users/{user_id}/")
async def update_user(user_id: int, updated_user: schema.UserCreate, db:AsyncSessionLocal = Depends(get_async_db)):
	user_sel = await crud.get_user(db, user_id)
	if user_sel is None:
		raise HTTPException(status_code=404, detail="User not found")
	user_modify = await crud.update_user(db, user_sel, updated_user)
	return user_modify

@app.delete("/users/{user_id}/")
async def delete_user(user_id: int, db:AsyncSessionLocal = Depends(get_async_db)):
	user_sel = await crud.get_user(db, user_id)
	if user_sel is None:
		raise HTTPException(status_code=404, detail="User not found")
	await crud.delete_user(db, user_sel)
	return {"message": "User deleted successfully"}
} 

if __name__ == "__main__":
	import uvicorn
	print(f"🌐 서버 주소: http://localhost:8000")
	print(f"📚 API 문서: http://localhost:8000/swagger")
	# reload=True
	uvicorn.run(app, host="0.0.0.0", port=8000)
