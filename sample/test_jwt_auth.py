#!/usr/bin/env python3
"""
JWT 인증 테스트 스크립트
서버 로그인 및 서버 종료 기능을 테스트합니다.
"""

import requests
import json
import time

# 서버 설정
BASE_URL = "http://localhost:8000"

def test_login():
    """로그인 테스트"""
    print("🔐 로그인 테스트 시작...")
    
    # 로그인 데이터
    login_data = {
        "username": "admin",
        "password": "admin123"
    }
    
    try:
        response = requests.post(f"{BASE_URL}/login", json=login_data)
        
        if response.status_code == 200:
            token_data = response.json()
            print(f"✅ 로그인 성공!")
            print(f"   토큰 타입: {token_data['token_type']}")
            print(f"   액세스 토큰: {token_data['access_token'][:50]}...")
            return token_data['access_token']
        else:
            print(f"❌ 로그인 실패: {response.status_code}")
            print(f"   응답: {response.text}")
            return None
            
    except Exception as e:
        print(f"❌ 로그인 요청 중 오류: {e}")
        return None

def test_shutdown_with_auth(token):
    """인증된 서버 종료 테스트"""
    print("\n🛑 인증된 서버 종료 테스트 시작...")
    
    headers = {
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json"
    }
    
    try:
        response = requests.post(f"{BASE_URL}/shutdown", headers=headers)
        
        if response.status_code == 200:
            result = response.json()
            print(f"✅ 서버 종료 요청 성공!")
            print(f"   메시지: {result['message']}")
            print(f"   상태: {result['status']}")
            print(f"   요청자: {result['requested_by']}")
        else:
            print(f"❌ 서버 종료 요청 실패: {response.status_code}")
            print(f"   응답: {response.text}")
            
    except Exception as e:
        print(f"❌ 서버 종료 요청 중 오류: {e}")

def test_shutdown_without_auth():
    """인증 없는 서버 종료 테스트 (실패 예상)"""
    print("\n🚫 인증 없는 서버 종료 테스트 시작...")
    
    try:
        response = requests.post(f"{BASE_URL}/shutdown")
        
        if response.status_code == 401:
            print(f"✅ 예상대로 인증 실패: {response.status_code}")
            print(f"   응답: {response.text}")
        else:
            print(f"❌ 예상과 다른 응답: {response.status_code}")
            print(f"   응답: {response.text}")
            
    except Exception as e:
        print(f"❌ 요청 중 오류: {e}")

def test_invalid_token():
    """잘못된 토큰으로 서버 종료 테스트 (실패 예상)"""
    print("\n🚫 잘못된 토큰으로 서버 종료 테스트 시작...")
    
    headers = {
        "Authorization": "Bearer invalid_token_here",
        "Content-Type": "application/json"
    }
    
    try:
        response = requests.post(f"{BASE_URL}/shutdown", headers=headers)
        
        if response.status_code == 401:
            print(f"✅ 예상대로 토큰 검증 실패: {response.status_code}")
            print(f"   응답: {response.text}")
        else:
            print(f"❌ 예상과 다른 응답: {response.status_code}")
            print(f"   응답: {response.text}")
            
    except Exception as e:
        print(f"❌ 요청 중 오류: {e}")

def test_health_check():
    """헬스 체크 테스트"""
    print("\n🏥 헬스 체크 테스트 시작...")
    
    try:
        response = requests.get(f"{BASE_URL}/health")
        
        if response.status_code == 200:
            result = response.json()
            print(f"✅ 헬스 체크 성공!")
            print(f"   상태: {result['status']}")
            print(f"   데이터베이스: {result['database']}")
            print(f"   연결 타입: {result['connection_type']}")
        else:
            print(f"❌ 헬스 체크 실패: {response.status_code}")
            print(f"   응답: {response.text}")
            
    except Exception as e:
        print(f"❌ 헬스 체크 요청 중 오류: {e}")

def main():
    """메인 테스트 함수"""
    print("🚀 JWT 인증 테스트 시작")
    print("=" * 50)
    
    # 서버 상태 확인
    test_health_check()
    
    # 로그인 테스트
    token = test_login()
    
    if token:
        # 인증된 서버 종료 테스트
        test_shutdown_with_auth(token)
        
        # 잘못된 토큰 테스트
        test_invalid_token()
    else:
        print("❌ 로그인 실패로 인해 추가 테스트를 건너뜁니다.")
    
    # 인증 없는 서버 종료 테스트
    test_shutdown_without_auth()
    
    print("\n" + "=" * 50)
    print("🏁 JWT 인증 테스트 완료")

if __name__ == "__main__":
    main() 