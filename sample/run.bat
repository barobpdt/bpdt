@echo off
echo ========================================
echo FastAPI MySQL CRUD 샘플 실행
echo ========================================
echo.

echo 1. 의존성 설치 중...
pip install -r requirements.txt

echo.
echo 2. 데이터베이스 연결 테스트 중...
python test_db_connection.py

echo.
echo 3. FastAPI 서버 시작 중...
echo 서버가 http://localhost:8000 에서 실행됩니다.
echo API 문서: http://localhost:8000/docs
echo 데이터베이스 상태: http://localhost:8000/health
echo 웹 인터페이스: web_interface.html 파일을 브라우저에서 열어주세요.
echo.
echo 서버를 중지하려면 Ctrl+C를 누르세요.
echo.
echo 서버 실행 후 다음 명령어로 테스트할 수 있습니다:
echo   - 기본 API 테스트: python test_api.py
echo   - QR 사용자 API 테스트: python test_qr_api.py
echo.

python fastapi_mysql_crud.py

pause 