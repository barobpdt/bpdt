from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from pydantic import BaseModel, EmailStr
from typing import List, Optional
import os
from dotenv import load_dotenv
from supabase import create_client, Client
from datetime import datetime
import os
import io
import sys
sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

# 환경 변수 로드
load_dotenv()

# Supabase 설정
SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_ANON_KEY")

print("@@ env value => ", SUPABASE_URL, SUPABASE_KEY)



if not SUPABASE_URL or not SUPABASE_KEY:
    raise ValueError("Supabase URL과 API 키를 환경 변수에 설정해주세요.")

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

# Pydantic 모델들
class UserBase(BaseModel):
    name: str
    email: EmailStr
    age: Optional[int] = None
    city: Optional[str] = None

class UserCreate(UserBase):
    pass

class UserUpdate(BaseModel):
    name: Optional[str] = None
    email: Optional[EmailStr] = None
    age: Optional[int] = None
    city: Optional[str] = None

class User(UserBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True

# 데이터베이스 초기화 함수
def init_database():
    """데이터베이스 테이블이 없으면 생성"""
    try:
        # users 테이블 생성 (실제로는 Supabase 대시보드에서 생성하는 것이 좋습니다)
        # 여기서는 테이블이 이미 존재한다고 가정합니다
        print("데이터베이스 연결 확인 중...")
        result = supabase.table("users").select("*").limit(1).execute()
        print("데이터베이스 연결 성공!")
    except Exception as e:
        print(f"데이터베이스 연결 오류: {e}")
        print("Supabase 대시보드에서 'users' 테이블을 생성해주세요.")
        print("테이블 스키마:")
        print("- id: int8 (primary key, auto increment)")
        print("- name: text (not null)")
        print("- email: text (unique, not null)")
        print("- age: int4")
        print("- city: text")
        print("- created_at: timestamptz (default: now())")

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
            "delete_user": "DELETE /users/{user_id}"
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

# 앱 시작 시 데이터베이스 초기화
@app.on_event("startup")
async def startup_event():
    init_database()

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000) 