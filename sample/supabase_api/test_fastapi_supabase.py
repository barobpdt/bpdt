#!/usr/bin/env python3
"""
FastAPI + Supabase 예제 테스트 스크립트
"""

import requests
import json
import time
from typing import Dict, Any

def test_api():
    """API 기본 기능 테스트"""
    base_url = "http://localhost:8000"
    
    print("🚀 FastAPI + Supabase 예제 테스트 시작")
    print("=" * 50)
    
    # 1. API 정보 조회
    print("\n1. API 정보 조회")
    try:
        response = requests.get(f"{base_url}/")
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2, ensure_ascii=False)}")
    except Exception as e:
        print(f"❌ 오류: {e}")
        return False
    
    # 2. 현재 사용자 목록 조회
    print("\n2. 현재 사용자 목록 조회")
    try:
        response = requests.get(f"{base_url}/users")
        print(f"Status: {response.status_code}")
        users = response.json()
        print(f"사용자 수: {len(users)}")
        if users:
            print("사용자 목록:")
            for user in users:
                print(f"  - {user['name']} ({user['email']})")
    except Exception as e:
        print(f"❌ 오류: {e}")
    
    # 3. 새 사용자 생성
    print("\n3. 새 사용자 생성")
    new_user = {
        "name": "테스트 사용자",
        "email": f"test{int(time.time())}@example.com",
        "age": 25,
        "city": "테스트시"
    }
    
    try:
        response = requests.post(
            f"{base_url}/users",
            json=new_user,
            headers={"Content-Type": "application/json"}
        )
        print(f"Status: {response.status_code}")
        if response.status_code == 200:
            created_user = response.json()
            print(f"생성된 사용자: {created_user['name']} (ID: {created_user['id']})")
            user_id = created_user['id']
        else:
            print(f"❌ 생성 실패: {response.text}")
            return False
    except Exception as e:
        print(f"❌ 오류: {e}")
        return False
    
    # 4. 생성된 사용자 조회
    print(f"\n4. 사용자 ID {user_id} 조회")
    try:
        response = requests.get(f"{base_url}/users/{user_id}")
        print(f"Status: {response.status_code}")
        if response.status_code == 200:
            user = response.json()
            print(f"조회된 사용자: {user['name']} ({user['email']})")
        else:
            print(f"❌ 조회 실패: {response.text}")
    except Exception as e:
        print(f"❌ 오류: {e}")
    
    # 5. 사용자 정보 수정
    print(f"\n5. 사용자 ID {user_id} 정보 수정")
    update_data = {
        "age": 26,
        "city": "수정된시"
    }
    
    try:
        response = requests.put(
            f"{base_url}/users/{user_id}",
            json=update_data,
            headers={"Content-Type": "application/json"}
        )
        print(f"Status: {response.status_code}")
        if response.status_code == 200:
            updated_user = response.json()
            print(f"수정된 사용자: 나이 {updated_user['age']}, 도시 {updated_user['city']}")
        else:
            print(f"❌ 수정 실패: {response.text}")
    except Exception as e:
        print(f"❌ 오류: {e}")
    
    # 6. 사용자 검색
    print(f"\n6. '테스트' 검색")
    try:
        response = requests.get(f"{base_url}/users/search/테스트")
        print(f"Status: {response.status_code}")
        if response.status_code == 200:
            search_results = response.json()
            print(f"검색 결과 수: {len(search_results)}")
            for result in search_results:
                print(f"  - {result['name']} ({result['email']})")
        else:
            print(f"❌ 검색 실패: {response.text}")
    except Exception as e:
        print(f"❌ 오류: {e}")
    
    # 7. 사용자 삭제
    print(f"\n7. 사용자 ID {user_id} 삭제")
    try:
        response = requests.delete(f"{base_url}/users/{user_id}")
        print(f"Status: {response.status_code}")
        if response.status_code == 200:
            print("✅ 사용자 삭제 성공")
        else:
            print(f"❌ 삭제 실패: {response.text}")
    except Exception as e:
        print(f"❌ 오류: {e}")
    
    # 8. 최종 사용자 목록 확인
    print("\n8. 최종 사용자 목록 확인")
    try:
        response = requests.get(f"{base_url}/users")
        print(f"Status: {response.status_code}")
        users = response.json()
        print(f"최종 사용자 수: {len(users)}")
    except Exception as e:
        print(f"❌ 오류: {e}")
    
    print("\n" + "=" * 50)
    print("✅ 테스트 완료!")
    print("📖 API 문서: http://localhost:8000/docs")
    print("🌐 웹 인터페이스: http://localhost:8000/web")
    print("=" * 50)
    
    return True

if __name__ == "__main__":
    # 서버가 실행 중인지 확인
    try:
        response = requests.get("http://localhost:8000/", timeout=5)
        if response.status_code == 200:
            test_api()
        else:
            print("❌ 서버가 응답하지 않습니다. 서버를 먼저 실행해주세요.")
    except requests.exceptions.ConnectionError:
        print("❌ 서버에 연결할 수 없습니다.")
        print("다음 명령어로 서버를 먼저 실행해주세요:")
        print("  python fastapi_supabase_example.py")
    except Exception as e:
        print(f"❌ 오류: {e}") 