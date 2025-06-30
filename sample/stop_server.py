#!/usr/bin/env python3
"""
간단한 서버 종료 스크립트
"""

import requests
import sys

def stop_server():
    """HTTP 요청으로 서버를 종료합니다."""
    try:
        print("🛑 서버 종료 요청 중...")
        response = requests.post("http://localhost:8000/shutdown", timeout=5)
        
        if response.status_code == 200:
            data = response.json()
            print(f"✅ {data.get('message', '서버 종료 요청 성공')}")
            return True
        else:
            print(f"❌ 서버 종료 요청 실패: {response.status_code}")
            return False
            
    except requests.exceptions.ConnectionError:
        print("❌ 서버에 연결할 수 없습니다. (서버가 실행 중이지 않을 수 있습니다)")
        return False
    except requests.exceptions.Timeout:
        print("❌ 서버 응답 시간 초과")
        return False
    except Exception as e:
        print(f"❌ 오류 발생: {e}")
        return False

if __name__ == "__main__":
    stop_server() 