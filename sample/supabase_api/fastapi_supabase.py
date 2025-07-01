from fastapi import FastAPI, HTTPException, Depends, status as http_status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from pydantic import BaseModel, EmailStr
from typing import List, Optional, Dict, Any, Type
import os
from dotenv import load_dotenv
from supabase import create_client, Client
from dataModel import User, UserLogin, UserCreate, UserUpdate
from supabaseDb import TableSchemaGenerator, init_database
from server_control import ServerController, find_all_uvicorn_servers
from auth import get_current_active_user, create_access_token, authenticate_user, Token, security
from datetime import datetime
import os
import io
import sys
import inspect
sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

from log import logger

# 환경 변수 로드
load_dotenv()

# Supabase 설정
SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_ANON_KEY")

if not SUPABASE_URL or not SUPABASE_KEY:
	logger.error("Supabase URL과 API 키를 환경 변수에 설정해주세요.")
	raise ValueError("Supabase URL과 API 키를 환경 변수에 설정해주세요.")
else:
	logger.info("Supabase URL과 API 키를 환경 변수에 설정되었습니다.")

# Supabase 클라이언트 생성
supabase: Client = create_client(SUPABASE_URL, SUPABASE_KEY)

# FastAPI 앱 생성
app = FastAPI(
	title="FastAPI + Supabase 예제",
	description="사용자 관리 시스템",
	version="1.0.0"
)

# CORS 설정
app.add_middleware(
	CORSMiddleware,
	allow_origins=["*"],
	allow_credentials=True,
	allow_methods=["*"],
	allow_headers=["*"],
)

# 정적 파일 서빙 설정
app.mount("/static", StaticFiles(directory="templates"), name="static")
# API 엔드포인트들

@app.get("/")
async def root():
	"""루트 엔드포인트"""
	return {
		"message": "FastAPI + Supabase 예제 API",
		"docs": "/docs",
		"web_interface": "/web",
		"endpoints": {
			"users": "/users",
			"user_by_id": "/users/{user_id}",
			"create_user": "POST /users",
			"update_user": "PUT /users/{user_id}",
			"delete_user": "DELETE /users/{user_id}",
			"generate_schema": "POST /generate-schema",
			"table_info": "GET /table-info",
			"create_table": "POST /create-table",
			"server_status": "GET /server/status",
			"server_list": "GET /server/list",
			"shutdown_server": "POST /server/shutdown",
			"shutdown_server_by_port": "POST /server/shutdown/{port}",
			"force_shutdown": "POST /server/force-shutdown",
			"force_shutdown_by_port": "POST /server/force-shutdown/{port}",
			"server_status_by_port": "GET /server/status/{port}"
		}
	}

@app.get("/web")
async def web_interface():
	"""웹 인터페이스"""
	return FileResponse("templates/fastapi_supabase.html")

@app.get("/users", response_model=List[User])
async def get_users():
	"""모든 사용자 조회"""
	try:
		response = supabase.table("users").select("*").execute()
		return response.data
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"사용자 조회 실패: {str(e)}")

@app.get("/users/{user_id}", response_model=User)
async def get_user(user_id: int):
	"""특정 사용자 조회"""
	try:
		response = supabase.table("users").select("*").eq("id", user_id).execute()
		if not response.data:
			raise HTTPException(status_code=404, detail="사용자를 찾을 수 없습니다")
		return response.data[0]
	except HTTPException:
		raise
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"사용자 조회 실패: {str(e)}")

@app.post("/users", response_model=User)
async def create_user(user: UserCreate):
	"""새 사용자 생성"""
	try:
		# 이메일 중복 확인
		existing_user = supabase.table("users").select("*").eq("email", user.email).execute()
		if existing_user.data:
			raise HTTPException(status_code=400, detail="이미 존재하는 이메일입니다")
		
		# 새 사용자 생성
		response = supabase.table("users").insert(user.dict()).execute()
		return response.data[0]
	except HTTPException:
		raise
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"사용자 생성 실패: {str(e)}")

@app.put("/users/{user_id}", response_model=User)
async def update_user(user_id: int, user_update: UserUpdate):
	"""사용자 정보 수정"""
	try:
		# 사용자 존재 확인
		existing_user = supabase.table("users").select("*").eq("id", user_id).execute()
		if not existing_user.data:
			raise HTTPException(status_code=404, detail="사용자를 찾을 수 없습니다")
		
		# 이메일 변경 시 중복 확인
		if user_update.email:
			email_check = supabase.table("users").select("*").eq("email", user_update.email).neq("id", user_id).execute()
			if email_check.data:
				raise HTTPException(status_code=400, detail="이미 존재하는 이메일입니다")
		
		# 업데이트할 데이터 준비 (None이 아닌 값만)
		update_data = {k: v for k, v in user_update.dict().items() if v is not None}
		
		if not update_data:
			raise HTTPException(status_code=400, detail="업데이트할 데이터가 없습니다")
		
		# 사용자 정보 업데이트
		response = supabase.table("users").update(update_data).eq("id", user_id).execute()
		return response.data[0]
	except HTTPException:
		raise
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"사용자 수정 실패: {str(e)}")

@app.delete("/users/{user_id}")
async def delete_user(user_id: int):
	"""사용자 삭제"""
	try:
		# 사용자 존재 확인
		existing_user = supabase.table("users").select("*").eq("id", user_id).execute()
		if not existing_user.data:
			raise HTTPException(status_code=404, detail="사용자를 찾을 수 없습니다")
		
		# 사용자 삭제
		supabase.table("users").delete().eq("id", user_id).execute()
		return {"message": "사용자가 성공적으로 삭제되었습니다"}
	except HTTPException:
		raise
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"사용자 삭제 실패: {str(e)}")

@app.get("/users/search/{keyword}")
async def search_users(keyword: str):
	"""사용자 검색 (이름 또는 이메일로 검색)"""
	try:
		response = supabase.table("users").select("*").or_(f"name.ilike.%{keyword}%,email.ilike.%{keyword}%").execute()
		return response.data
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"사용자 검색 실패: {str(e)}")

@app.post("/generate-schema")
async def generate_table_schema():
	"""User 모델을 기반으로 테이블 스키마 생성"""
	try:
		# User 모델 스키마 생성
		schema = TableSchemaGenerator.generate_table_schema(User, "users")
		sql_ddl = TableSchemaGenerator.generate_sql_ddl(schema)
		
		return {
			"message": "테이블 스키마가 성공적으로 생성되었습니다.",
			"table_name": schema['table_name'],
			"columns": schema['columns'],
			"constraints": schema['constraints'],
			"sql_ddl": sql_ddl,
			"instructions": "Supabase 대시보드의 SQL 편집기에서 위의 SQL을 실행하세요."
		}
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"스키마 생성 실패: {str(e)}")

@app.get("/table-info")
async def get_table_info():
	"""현재 테이블 정보 조회"""
	try:
		# 테이블 존재 여부 확인
		try:
			result = supabase.table("users").select("*").limit(1).execute()
			table_exists = True
		except Exception:
			table_exists = False
		
		# User 모델 스키마 정보
		schema = TableSchemaGenerator.generate_table_schema(User, "users")
		
		return {
			"table_name": "users",
			"table_exists": table_exists,
			"model_class": "User",
			"columns": schema['columns'],
			"constraints": schema['constraints'],
			"total_columns": len(schema['columns'])
		}
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"테이블 정보 조회 실패: {str(e)}")

@app.post("/create-table")
async def create_table_from_model():
	"""User 모델을 기반으로 테이블 생성 (스키마만 생성)"""
	try:
		success = TableSchemaGenerator.create_table_from_model(User, "users")
		
		if success:
			schema = TableSchemaGenerator.generate_table_schema(User, "users")
			sql_ddl = TableSchemaGenerator.generate_sql_ddl(schema)
			
			return {
				"message": "테이블 스키마가 성공적으로 생성되었습니다.",
				"table_name": "users",
				"sql_ddl": sql_ddl,
				"next_steps": [
					"1. Supabase 대시보드에 로그인하세요.",
					"2. SQL 편집기로 이동하세요.",
					"3. 위의 SQL을 복사하여 실행하세요.",
					"4. 테이블이 생성되면 API를 사용할 수 있습니다."
				]
			}
		else:
			raise HTTPException(status_code=500, detail="테이블 스키마 생성 실패")
			
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"테이블 생성 실패: {str(e)}")

# JWT 토큰 관련 엔드포인트들
@app.post("/login_admin", response_model=Token)
async def login_admin(user: UserLogin):
	logger.info(f"로그인 시도: username={user.username}")
	try:
		check_auth = authenticate_user(user.username, user.password)
		if not check_auth:
			logger.warning(f"로그인 실패: 잘못된 사용자명 또는 비밀번호 - {user.username}")
			raise HTTPException(
				status_code=http_status.HTTP_401_UNAUTHORIZED,
				detail="Incorrect username or password",
				headers={"WWW-Authenticate": "Bearer"},
			)
		
		access_token = create_access_token()		
		logger.info(f"로그인 성공: username={user.username}")
		return {"access_token": access_token, "token_type": "bearer"}
	except HTTPException:
		raise
	except Exception as e:
		logger.error(f"로그인 처리 중 오류: {e}")
		raise HTTPException(status_code=500, detail="Login processing error")

# 서버 제어 엔드포인트들

@app.get("/server/status")
async def get_server_status(current_user: User = Depends(get_current_active_user)):
	"""현재 서버 상태 조회"""
	try:
		controller = ServerController(port=8000)
		status = controller.get_server_info()
		return status
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 상태 조회 실패: {str(e)}")

@app.get("/server/list")
async def get_all_servers():
	"""모든 uvicorn 서버 목록 조회"""
	try:
		servers = find_all_uvicorn_servers()
		return {
			"total_servers": len(servers),
			"servers": servers
		}
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 목록 조회 실패: {str(e)}")

@app.post("/server/shutdown")
async def shutdown_server():
	"""현재 서버 종료 (정상 종료)"""
	try:
		controller = ServerController(port=8000)
		result = controller.shutdown_server(force=False)
		
		if result["success"]:
			return {
				"message": result["message"],
				"method": result["method"],
				"process_info": result["process_info"]
			}
		else:
			raise HTTPException(status_code=500, detail=result["message"])
			
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 종료 실패: {str(e)}")

@app.post("/server/shutdown/{port}")
async def shutdown_server_by_port_endpoint(port: int, force: bool = False):
	"""특정 포트의 서버 종료"""
	try:
		controller = ServerController(port=port)
		result = controller.shutdown_server(force=force)
		
		if result["success"]:
			return {
				"message": result["message"],
				"method": result["method"],
				"process_info": result["process_info"],
				"port": port
			}
		else:
			raise HTTPException(status_code=500, detail=result["message"])
			
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 종료 실패: {str(e)}")

@app.post("/server/force-shutdown")
async def force_shutdown_server():
	"""현재 서버 강제 종료"""
	try:
		controller = ServerController(port=8000)
		result = controller.shutdown_server(force=True)
		
		if result["success"]:
			return {
				"message": result["message"],
				"method": result["method"],
				"process_info": result["process_info"]
			}
		else:
			raise HTTPException(status_code=500, detail=result["message"])
			
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 강제 종료 실패: {str(e)}")

@app.post("/server/force-shutdown/{port}")
async def force_shutdown_server_by_port(port: int):
	"""특정 포트의 서버 강제 종료"""
	try:
		controller = ServerController(port=port)
		result = controller.shutdown_server(force=True)
		
		if result["success"]:
			return {
				"message": result["message"],
				"method": result["method"],
				"process_info": result["process_info"],
				"port": port
			}
		else:
			raise HTTPException(status_code=500, detail=result["message"])
			
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 강제 종료 실패: {str(e)}")

@app.get("/server/status/{port}")
async def get_server_status_by_port(port: int):
	"""특정 포트의 서버 상태 조회"""
	try:
		controller = ServerController(port=port)
		status = controller.get_server_info()
		return status
	except Exception as e:
		raise HTTPException(status_code=500, detail=f"서버 상태 조회 실패: {str(e)}")

# 앱 시작 시 데이터베이스 초기화
@app.on_event("startup")
async def startup_event():
	init_database(supabase)

if __name__ == "__main__":
	import uvicorn
	uvicorn.run(app, host="0.0.0.0", port=8000) 