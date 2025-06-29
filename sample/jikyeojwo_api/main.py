from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
import asyncio
import signal
import sys
import io
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
	sys.path.insert(0, current_dir)

from db import get_db, get_async_db, init_database, close_database
from log import logger
from model import Token, User, UserLogin
from auth import authenticate_user, create_access_token, get_current_active_user

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

# FastAPI 앱 생성
app = FastAPI(
	title="FastAPI MySQL CRUD Sample",
	description="A simple CRUD application using FastAPI and MySQL with QR User Management",
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

# 애플리케이션 시작 이벤트
@app.on_event("startup")
async def startup_event():
	"""애플리케이션 시작 시 실행"""
	# 데이터베이스 초기화
	# await init_database()
	logger.info("🚀 FastAPI MySQL CRUD 서버 시작")
	logger.info(f"🌐 서버 주소: http://localhost:8000")
	logger.info(f"📚 API 문서: http://localhost:8000/docs")
	logger.info(f"📋 로그 파일: logs/fastapi_mysql_crud.log")	

# 애플리케이션 종료 이벤트
@app.on_event("shutdown")
async def shutdown_event():
	"""애플리케이션 종료 시 실행"""
	logger.info("👋 FastAPI MySQL CRUD 서버 종료")
	await close_database()
	
# API 엔드포인트들
@app.get("/")
def read_root():
	return {
		"message": "FastAPI MySQL CRUD Sample with QR User Management",
	}

@app.post("/login_admin", response_model=Token)
async def login_for_access_token(user: UserLogin):
	"""사용자 로그인 및 JWT 토큰 발급"""
	logger.info(f"로그인 시도: username={user.username}")
	try:
		user = authenticate_user(user.username, user.password)
		if not user:
			logger.warning(f"로그인 실패: 잘못된 사용자명 또는 비밀번호 - {user.username}")
			raise HTTPException(
				status_code=status.HTTP_401_UNAUTHORIZED,
				detail="Incorrect username or password",
				headers={"WWW-Authenticate": "Bearer"},
			)
		access_token = create_access_token(user.username)
		logger.info(f"로그인 성공: username={user.username}")
		return {"access_token": access_token, "token_type": "bearer"}
	except HTTPException:
		raise
	except Exception as e:
		logger.error(f"로그인 처리 중 오류: {e}")
		raise HTTPException(status_code=500, detail="Login processing error")

@app.post("/shutdown")
async def shutdown_server(current_user: User = Depends(get_current_active_user)):
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

async def shutdown_background():
	"""백그라운드에서 서버 종료"""
	await asyncio.sleep(2)  # 2초 대기
	logger.info("🛑 서버 종료 실행")
	sys.exit(0)

if __name__ == "__main__":
	import uvicorn
	
	try:
		uvicorn.run(
			app, 
			host="0.0.0.0", 
			port=8000,
			log_config=None,  # uvicorn 기본 로깅 비활성화
			access_log=False  # uvicorn 액세스 로그 비활성화 (우리 로거 사용)
		)
	except KeyboardInterrupt:
		logger.info("🛑 서버 종료 요청됨")
	except Exception as e:
		logger.error(f"❌ 서버 실행 중 오류: {e}") 