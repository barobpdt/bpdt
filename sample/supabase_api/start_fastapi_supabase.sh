#!/bin/bash

echo "========================================"
echo "FastAPI + Supabase 예제 시작"
echo "========================================"
echo

echo "의존성 설치 중..."
pip install -r requirements.txt

echo
echo "FastAPI 서버 시작 중..."
echo "서버 주소: http://localhost:8000"
echo "API 문서: http://localhost:8000/docs"
echo "웹 인터페이스: http://localhost:8000/web"
echo

python fastapi_supabase_example.py 