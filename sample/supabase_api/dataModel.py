#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Pydantic 데이터 모델 정의
"""

from pydantic import BaseModel, EmailStr
from typing import Optional
from datetime import datetime


class UserLogin(BaseModel):
    username: str
    password: str

# Pydantic 모델들
class UserBase(BaseModel):
    name: str
    email: EmailStr
    age: Optional[int] = None
    city: Optional[str] = None

class UserCreate(UserBase):
	nameL: str
	email: str
	addr: str
	hp: str

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