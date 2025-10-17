import uvicorn
from fastapi import FastAPI, HTTPException, Depends, status, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse

from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from fastapi.security import OAuth2PasswordBearer

from sqlalchemy import create_engine, Column, Integer, String, Text, Boolean, DateTime, Numeric
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from sqlalchemy.orm import sessionmaker, Session, declarative_base
from sqlalchemy.sql import text
from sqlalchemy.future import select
from pydantic import BaseModel, Field
from typing import List, Optional
from schema_supabase import TreeNode, UserAdmin
from python.ApiConfig import ApiConfig, userLoginCheck, get_async_db
from datetime import datetime, timedelta
import os
import sys
import io
import logging
from logging.handlers import TimedRotatingFileHandler
import asyncio

from jose import JWTError, jwt
from passlib.context import CryptContext
from uuid import UUID, uuid4

from fastapi.templating import Jinja2Templates
templates = Jinja2Templates(directory="templates")

oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")


api = ApiConfig()
sys.stdout.reconfigure(encoding='utf-8')

print(f"api=={api}")
# 관리자 사용자 정보 (실제 운영환경에서는 데이터베이스에서 관리)
from contextlib import asynccontextmanager
@asynccontextmanager
async def lifespan(app: FastAPI): 
	await api.fastApiStart()
	yield    
	await api.fastApiEnd()


# FastAPI 앱 생성
app = FastAPI(
	title="FastAPI MySQL CRUD Sample",
	description="A simple CRUD application using FastAPI and MySQL with QR User Management",
	version="1.0.0", lifespan=lifespan
)

# CORS 미들웨어 추가
app.add_middleware(
	CORSMiddleware,
	allow_origins=["*"],
	allow_credentials=True,
	allow_methods=["*"],
	allow_headers=["*"],
)

class UserLogin(BaseModel):
	userId: str
	userName: str|None

class Task(BaseModel):
	id: Optional[UUID] = None
	title: str
	description: Optional[str] = None
	completed: bool = False
	def __init__(self, *args, **kwargs ):
		print(f'task args:{args} kwargs:{kwargs}')
		kwargs['id'] = args[0] if len(args)>0 else uuid4()
		kwargs['title'] = args[1] if len(args)>1 else ''
		super().__init__(**kwargs)
	def __repr__(self):
		return f"Task(id={self.id},title={self.title})"

tasks = []
tasks.append(Task(uuid4(),'test01'))
tasks.append(Task(uuid4(),'test02'))
	
# 애플리케이션 시작 이벤트
def startup_event():
	"""애플리케이션 시작 시 실행"""
	print("🚀 FastAPI MySQL CRUD 서버 시작")
	print(f"PID: {pid}")
	print(f"🌐 서버 주소: http://localhost:8000")
	print(f"📚 API 문서: http://localhost:8000/docs")
	print(f"📋 로그 파일: logs/fastapi_mysql_crud.log")

@app.get("/")
def read_root():
	print("루트 엔드포인트 호출")
	return {
		"message": "FastAPI MySQL CRUD Sample with QR User Management",
		"docs": "/docs",
		"endpoints": {
			"login": "/login",
			"qr_users": "/qr-users",
			"qr_user_detail": "/qr-users/{qr_no}/detail",
			"qr_user_search": "/qr-users/search",
			"qr_user_stats": "/qr-users/stats",
			"shutdown": "/shutdown (requires authentication)"
		}
	}

''' [호출테스트]
w=Baro.web()
data=#[{
  "userId": "test",
  "userName": "test"
}]
w.header('content-type','application/json')
w.data=data
w.call('http://localhost:8000/login', 'POST', func(type,data) {
	if( type=='read') {
		node=_node().parseJson(data)
		print('login result => ', node)
	}
})


'''

async def get_current_active_user(userId:str = Depends(userLoginCheck)):
	"""현재 활성 사용자 조회"""
	if not userId:
		raise HTTPException(status_code=400, detail="Inactive user")
	return userId

@app.get('/login',response_class=HTMLResponse)
def login(request : Request):
	return templates.TemplateResponse("login.html", {"request": request})

@app.get("/tasks/", response_model=List[Task])
def read_tasks():
	return tasks

@app.post('/login')
async def setUserLogin(user:UserLogin):
	token = ApiConfig().createToken(user.userId) 
	return {'token':token}

@app.get("/is_login", response_model=UserLogin)
async def is_login(token:str = Depends(oauth2_scheme)):
	print(f'is_login token:{token}')
	userId = api.getTokenUserId(token)
	print(f'is_login userId:{userId}')
	return {"userId":userId,"userName":"test"}

@app.get("/db_test")
async def db_test(db: AsyncSession = Depends(get_async_db)):
	result = await db.execute(
		select(UserAdmin)
	)
	return result.scalars().all()

@app.get("/shutdown")
async def read_tasks():
	await ApiConfig().fastApiEnd()
	return {'result':'stop'}

if __name__ == "__main__":        
	try:
		uvicorn.run(
			app, 
			host="0.0.0.0", 
			port=8000,
			log_config=None,  # uvicorn 기본 로깅 비활성화
			access_log=False  # uvicorn 액세스 로그 비활성화
		)
	except Exception as e:
		print(f"❌ 서버 실행 중 오류: {e}")