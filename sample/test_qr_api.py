#!/usr/bin/env python3
"""
QR 사용자 정보 조회 API 테스트 스크립트
"""

import requests
import json
from datetime import datetime

# API 기본 URL
BASE_URL = "http://localhost:8000"

def test_api_connection():
    """API 연결 상태를 확인합니다."""
    print("🔍 API 연결 상태 확인...")
    try:
        response = requests.get(f"{BASE_URL}/")
        if response.status_code == 200:
            print("✅ API 연결 성공!")
            data = response.json()
            print(f"📊 메시지: {data['message']}")
            print(f"🌐 데이터베이스: {data['database']}")
            return True
        else:
            print(f"❌ API 연결 실패: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ API 연결 오류: {e}")
        return False

def test_health_check():
    """데이터베이스 연결 상태를 확인합니다."""
    print("\n🔍 데이터베이스 연결 상태 확인...")
    try:
        response = requests.get(f"{BASE_URL}/health")
        if response.status_code == 200:
            data = response.json()
            print(f"✅ 상태: {data['status']}")
            print(f"📊 데이터베이스: {data['database']}")
            print(f"💬 메시지: {data['message']}")
            return data['status'] == 'healthy'
        else:
            print(f"❌ 헬스 체크 실패: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ 헬스 체크 오류: {e}")
        return False

def test_qr_users_list():
    """QR 사용자 목록을 조회합니다."""
    print("\n🔍 QR 사용자 목록 조회...")
    try:
        response = requests.get(f"{BASE_URL}/qr-users/")
        if response.status_code == 200:
            qr_users = response.json()
            print(f"✅ QR 사용자 수: {len(qr_users)}")
            if qr_users:
                print("📋 QR 사용자 목록 (처음 5개):")
                for i, user in enumerate(qr_users[:5]):
                    print(f"  {i+1}. QR번호: {user.get('qr_no', 'N/A')}")
                    print(f"     회원번호: {user.get('memb_no', 'N/A')}")
                    print(f"     닉네임: {user.get('nick_nm', 'N/A')}")
                    print(f"     차량정보: {user.get('car_info', 'N/A')}")
                    print(f"     QR상태: {user.get('qr_use_cd', 'N/A')}")
                    print()
            return True
        else:
            print(f"❌ QR 사용자 목록 조회 실패: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ QR 사용자 목록 조회 오류: {e}")
        return False

def test_qr_user_detail(qr_no="TEST001"):
    """QR 사용자 상세 정보를 조회합니다."""
    print(f"\n🔍 QR 사용자 상세 정보 조회 (QR번호: {qr_no})...")
    try:
        response = requests.get(f"{BASE_URL}/qr-users/{qr_no}/detail")
        if response.status_code == 200:
            detail = response.json()
            print("✅ QR 사용자 상세 정보:")
            print(f"📊 QR 정보: {detail.get('qr_info', {})}")
            print(f"👤 회원 정보: {detail.get('member_info', {})}")
            print(f"📁 QR 파일 정보: {detail.get('qr_file_info', {})}")
            print(f"📞 호출 이력 수: {len(detail.get('call_history', []))}")
            return True
        elif response.status_code == 404:
            print(f"⚠️ QR 사용자를 찾을 수 없습니다: {qr_no}")
            return True  # 404는 정상적인 응답
        else:
            print(f"❌ QR 사용자 상세 정보 조회 실패: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ QR 사용자 상세 정보 조회 오류: {e}")
        return False

def test_qr_user_search():
    """QR 사용자 검색을 테스트합니다."""
    print("\n🔍 QR 사용자 검색 테스트...")
    
    search_tests = [
        {"term": "김", "type": "nick_nm", "description": "닉네임으로 검색"},
        {"term": "12", "type": "qr_no", "description": "QR번호로 검색"},
        {"term": "M", "type": "qr_owner_cd", "description": "QR소유자구분으로 검색"}
    ]
    
    for test in search_tests:
        print(f"  🔎 {test['description']}...")
        try:
            response = requests.get(
                f"{BASE_URL}/qr-users/search/",
                params={"search_term": test["term"], "search_type": test["type"]}
            )
            if response.status_code == 200:
                results = response.json()
                print(f"    ✅ 검색 결과: {len(results)}건")
                if results:
                    for i, result in enumerate(results[:3]):
                        print(f"      {i+1}. {result.get('nick_nm', 'N/A')} ({result.get('qr_no', 'N/A')})")
            else:
                print(f"    ❌ 검색 실패: {response.status_code}")
        except Exception as e:
            print(f"    ❌ 검색 오류: {e}")

def test_qr_user_stats():
    """QR 사용자 통계를 조회합니다."""
    print("\n🔍 QR 사용자 통계 조회...")
    try:
        response = requests.get(f"{BASE_URL}/qr-users/stats")
        if response.status_code == 200:
            stats = response.json()
            print("✅ QR 사용자 통계:")
            print(f"📊 전체 QR 사용자: {stats.get('total_qr_users', 0)}")
            print(f"✅ 활성 QR 사용자: {stats.get('active_qr_users', 0)}")
            print(f"👑 마스터 QR 사용자: {stats.get('master_qr_users', 0)}")
            print(f"👥 서브 QR 사용자: {stats.get('sub_qr_users', 0)}")
            return True
        else:
            print(f"❌ QR 사용자 통계 조회 실패: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ QR 사용자 통계 조회 오류: {e}")
        return False

def test_member_qr_users(memb_no="TEST001"):
    """회원번호로 QR 사용자 정보를 조회합니다."""
    print(f"\n🔍 회원번호로 QR 사용자 조회 (회원번호: {memb_no})...")
    try:
        response = requests.get(f"{BASE_URL}/qr-users/member/{memb_no}")
        if response.status_code == 200:
            qr_users = response.json()
            print(f"✅ 회원의 QR 사용자 수: {len(qr_users)}")
            for i, user in enumerate(qr_users):
                print(f"  {i+1}. QR번호: {user.get('qr_no', 'N/A')}")
                print(f"     닉네임: {user.get('nick_nm', 'N/A')}")
                print(f"     QR상태: {user.get('qr_use_cd', 'N/A')}")
            return True
        else:
            print(f"❌ 회원 QR 사용자 조회 실패: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ 회원 QR 사용자 조회 오류: {e}")
        return False

def main():
    """메인 테스트 함수"""
    print("🚀 QR 사용자 정보 조회 API 테스트 시작")
    print("=" * 60)
    
    # API 연결 테스트
    if not test_api_connection():
        print("❌ API 연결에 실패했습니다. 서버가 실행 중인지 확인해주세요.")
        return
    
    # 데이터베이스 연결 테스트
    if not test_health_check():
        print("❌ 데이터베이스 연결에 실패했습니다.")
        return
    
    # QR 사용자 관련 테스트
    test_qr_users_list()
    test_qr_user_detail()
    test_qr_user_search()
    test_qr_user_stats()
    test_member_qr_users()
    
    print("\n" + "=" * 60)
    print("🎉 QR 사용자 정보 조회 API 테스트 완료!")
    print("📚 API 문서: http://localhost:8000/docs")
    print("🌐 서버 주소: http://localhost:8000")

if __name__ == "__main__":
    main() 