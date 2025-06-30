#!/usr/bin/env python3
"""
FastAPI 서버 제어 스크립트
"""

import requests
import time
import sys
import os
import signal
import psutil
import subprocess

# 서버 설정
SERVER_URL = "http://localhost:8000"
SERVER_PID_FILE = "server.pid"

def find_server_process():
    """실행 중인 서버 프로세스를 찾습니다."""
    try:
        # PID 파일에서 확인
        if os.path.exists(SERVER_PID_FILE):
            with open(SERVER_PID_FILE, 'r') as f:
                pid = int(f.read().strip())
                if psutil.pid_exists(pid):
                    return pid
        
        # 포트 8000을 사용하는 프로세스 찾기
        for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
            try:
                cmdline = proc.info['cmdline']
                if cmdline and any('fastapi_mysql_crud.py' in arg for arg in cmdline):
                    return proc.info['pid']
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
        
        return None
    except Exception as e:
        print(f"❌ 프로세스 검색 오류: {e}")
        return None

def save_server_pid(pid):
    """서버 PID를 파일에 저장합니다."""
    try:
        with open(SERVER_PID_FILE, 'w') as f:
            f.write(str(pid))
        print(f"✅ 서버 PID 저장: {pid}")
    except Exception as e:
        print(f"❌ PID 저장 오류: {e}")

def remove_pid_file():
    """PID 파일을 삭제합니다."""
    try:
        if os.path.exists(SERVER_PID_FILE):
            os.remove(SERVER_PID_FILE)
            print("✅ PID 파일 삭제됨")
    except Exception as e:
        print(f"❌ PID 파일 삭제 오류: {e}")

def start_server():
    """서버를 시작합니다."""
    print("🚀 FastAPI 서버를 시작합니다...")
    
    try:
        # 서버 프로세스 시작
        process = subprocess.Popen(
            [sys.executable, "fastapi_mysql_crud.py"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # PID 저장
        save_server_pid(process.pid)
        
        # 서버 시작 대기
        print("⏳ 서버 시작 대기 중...")
        time.sleep(3)
        
        # 서버 상태 확인
        try:
            response = requests.get(f"{SERVER_URL}/health", timeout=5)
            if response.status_code == 200:
                print("✅ 서버가 성공적으로 시작되었습니다!")
                print(f"🌐 서버 주소: {SERVER_URL}")
                print(f"📚 API 문서: {SERVER_URL}/docs")
                return True
            else:
                print(f"⚠️ 서버 응답 오류: {response.status_code}")
                return False
        except requests.exceptions.RequestException:
            print("⚠️ 서버 응답 없음 (아직 시작 중일 수 있음)")
            return False
            
    except Exception as e:
        print(f"❌ 서버 시작 오류: {e}")
        return False

def stop_server_http():
    """HTTP 요청으로 서버를 종료합니다."""
    print("🛑 HTTP 요청으로 서버를 종료합니다...")
    
    try:
        response = requests.post(f"{SERVER_URL}/shutdown", timeout=10)
        if response.status_code == 200:
            data = response.json()
            print(f"✅ {data.get('message', '서버 종료 요청 성공')}")
            return True
        else:
            print(f"❌ 서버 종료 요청 실패: {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"❌ 서버 종료 요청 오류: {e}")
        return False

def stop_server_process():
    """프로세스 시그널로 서버를 종료합니다."""
    print("🛑 프로세스 시그널로 서버를 종료합니다...")
    
    pid = find_server_process()
    if not pid:
        print("❌ 실행 중인 서버를 찾을 수 없습니다.")
        return False
    
    try:
        # SIGTERM 시그널 전송
        os.kill(pid, signal.SIGTERM)
        print(f"✅ 종료 시그널 전송: PID {pid}")
        
        # 프로세스 종료 대기
        for i in range(10):
            if not psutil.pid_exists(pid):
                print("✅ 서버가 성공적으로 종료되었습니다.")
                remove_pid_file()
                return True
            time.sleep(1)
            print(f"⏳ 서버 종료 대기 중... ({i+1}/10)")
        
        # 강제 종료
        print("⚠️ 강제 종료 실행...")
        os.kill(pid, signal.SIGKILL)
        time.sleep(1)
        
        if not psutil.pid_exists(pid):
            print("✅ 서버가 강제 종료되었습니다.")
            remove_pid_file()
            return True
        else:
            print("❌ 서버 종료 실패")
            return False
            
    except Exception as e:
        print(f"❌ 서버 종료 오류: {e}")
        return False

def check_server_status():
    """서버 상태를 확인합니다."""
    print("🔍 서버 상태를 확인합니다...")
    
    # 프로세스 확인
    pid = find_server_process()
    if not pid:
        print("❌ 실행 중인 서버가 없습니다.")
        return False
    
    print(f"✅ 서버 프로세스 발견: PID {pid}")
    
    # HTTP 응답 확인
    try:
        response = requests.get(f"{SERVER_URL}/health", timeout=5)
        if response.status_code == 200:
            data = response.json()
            print(f"✅ 서버 응답 정상: {data.get('status', 'unknown')}")
            print(f"   데이터베이스: {data.get('database', 'unknown')}")
            print(f"   연결 타입: {data.get('connection_type', 'unknown')}")
            return True
        else:
            print(f"⚠️ 서버 응답 오류: {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"❌ 서버 응답 없음: {e}")
        return False

def restart_server():
    """서버를 재시작합니다."""
    print("🔄 서버를 재시작합니다...")
    
    if stop_server_process():
        time.sleep(2)
        if start_server():
            print("✅ 서버 재시작 완료")
            return True
    
    print("❌ 서버 재시작 실패")
    return False

def main():
    """메인 함수"""
    if len(sys.argv) < 2:
        print("사용법:")
        print("  python server_control.py start     - 서버 시작")
        print("  python server_control.py stop      - 서버 종료 (HTTP)")
        print("  python server_control.py kill      - 서버 강제 종료")
        print("  python server_control.py restart   - 서버 재시작")
        print("  python server_control.py status    - 서버 상태 확인")
        return
    
    command = sys.argv[1].lower()
    
    if command == "start":
        start_server()
    elif command == "stop":
        stop_server_http()
    elif command == "kill":
        stop_server_process()
    elif command == "restart":
        restart_server()
    elif command == "status":
        check_server_status()
    else:
        print(f"❌ 알 수 없는 명령어: {command}")

if __name__ == "__main__":
    main() 