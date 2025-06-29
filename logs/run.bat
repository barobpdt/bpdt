@echo off
chcp 65001 >nul
echo ========================================
echo FastAPI MySQL CRUD Sample with QR User Management
echo ========================================
echo.

:menu
echo 선택하세요:
echo 1. 서버 실행
echo 2. 비동기 API 테스트
echo 3. 데이터베이스 연결 테스트
echo 4. 로그 관리
echo 5. 종료
echo.
set /p choice="번호를 입력하세요 (1-5): "

if "%choice%"=="1" goto run_server
if "%choice%"=="2" goto test_async
if "%choice%"=="3" goto test_db
if "%choice%"=="4" goto log_manager
if "%choice%"=="5" goto exit
echo 잘못된 선택입니다. 다시 시도하세요.
echo.
goto menu

:run_server
echo.
echo 🚀 FastAPI MySQL CRUD 서버를 시작합니다...
echo 📊 데이터베이스: secretguard on 106.246.249.162:23381
echo 🌐 서버 주소: http://localhost:8000
echo 📚 API 문서: http://localhost:8000/docs
echo 📋 로그 파일: logs/fastapi_mysql_crud.log
echo.
echo 서버를 중지하려면 Ctrl+C를 누르세요.
echo.
python fastapi_mysql_crud.py
pause
goto menu

:test_async
echo.
echo 🧪 비동기 API 테스트를 실행합니다...
echo.
python test_async_api.py
echo.
pause
goto menu

:test_db
echo.
echo 🔍 데이터베이스 연결을 테스트합니다...
echo.
python test_db_connection.py
echo.
pause
goto menu

:log_manager
echo.
echo 📝 로그 관리 도구
echo.
echo 1. 로그 파일 목록
echo 2. 최신 로그 조회 (50줄)
echo 3. 최신 로그 조회 (100줄)
echo 4. 오래된 로그 정리 (30일)
echo 5. 로그 파일 백업
echo 6. 메인 메뉴로 돌아가기
echo.
set /p log_choice="로그 관리 옵션을 선택하세요 (1-6): "

if "%log_choice%"=="1" (
    python log_manager.py list
    pause
    goto log_manager
)
if "%log_choice%"=="2" (
    python log_manager.py show 50
    pause
    goto log_manager
)
if "%log_choice%"=="3" (
    python log_manager.py show 100
    pause
    goto log_manager
)
if "%log_choice%"=="4" (
    python log_manager.py clean 30
    pause
    goto log_manager
)
if "%log_choice%"=="5" (
    python log_manager.py backup
    pause
    goto log_manager
)
if "%log_choice%"=="6" goto menu
echo 잘못된 선택입니다.
pause
goto log_manager

:exit
echo.
echo 👋 프로그램을 종료합니다.
exit 