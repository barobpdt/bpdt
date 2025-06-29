from sqlalchemy import Column, Integer, String, Text, Boolean, DateTime, Numeric
from sqlalchemy.orm import declarative_base
from pydantic import BaseModel, Field
from typing import List, Optional

# 로그인 관련 Pydantic 모델
class UserLogin(BaseModel):
    username: str
    password: str


# JWT 토큰 관련 클래스들
class Token(BaseModel):
    access_token: str
    token_type: str

class TokenData(BaseModel):
    username: Optional[str] = None

class User(BaseModel):
    username: str
    email: Optional[str] = None
    full_name: Optional[str] = None
    disabled: Optional[bool] = None

class UserInDB(User):
    hashed_password: str