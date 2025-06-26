@echo off
echo ========================================
echo FastAPI SQLite CRUD 샘플 실행
echo ========================================
echo.

echo 1. 의존성 설치 중...
pip install -r requirements.txt

echo.
echo 2. FastAPI 서버 시작 중...
echo 서버가 http://localhost:8000 에서 실행됩니다.
echo API 문서: http://localhost:8000/docs
echo 웹 인터페이스: web_interface.html 파일을 브라우저에서 열어주세요.
echo.
echo 서버를 중지하려면 Ctrl+C를 누르세요.
echo.

python fastapi_sqlite_crud.py

pause 