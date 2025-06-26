import requests
import json
import time

# API 기본 URL
BASE_URL = "http://localhost:8000"

def test_api():
    """FastAPI SQLite CRUD 애플리케이션을 테스트합니다."""
    
    print("🚀 FastAPI SQLite CRUD 테스트 시작")
    print("=" * 50)
    
    # 1. 루트 엔드포인트 테스트
    print("\n1. 루트 엔드포인트 테스트")
    try:
        response = requests.get(f"{BASE_URL}/")
        print(f"Status: {response.status_code}")
        print(f"Response: {response.json()}")
    except Exception as e:
        print(f"Error: {e}")
    
    # 2. 사용자 생성 테스트
    print("\n2. 사용자 생성 테스트")
    users_data = [
        {
            "name": "김철수",
            "email": "kim@example.com",
            "age": 25,
            "is_active": True
        },
        {
            "name": "이영희",
            "email": "lee@example.com",
            "age": 30,
            "is_active": True
        },
        {
            "name": "박민수",
            "email": "park@example.com",
            "age": 28,
            "is_active": False
        }
    ]
    
    created_users = []
    for user_data in users_data:
        try:
            response = requests.post(f"{BASE_URL}/users/", json=user_data)
            print(f"Status: {response.status_code}")
            if response.status_code == 201:
                user = response.json()
                created_users.append(user)
                print(f"Created user: {user['name']} (ID: {user['id']})")
            else:
                print(f"Error: {response.text}")
        except Exception as e:
            print(f"Error: {e}")
    
    # 3. 모든 사용자 조회 테스트
    print("\n3. 모든 사용자 조회 테스트")
    try:
        response = requests.get(f"{BASE_URL}/users/")
        print(f"Status: {response.status_code}")
        users = response.json()
        print(f"Total users: {len(users)}")
        for user in users:
            print(f"  - {user['name']} ({user['email']}) - Active: {user['is_active']}")
    except Exception as e:
        print(f"Error: {e}")
    
    # 4. 특정 사용자 조회 테스트
    if created_users:
        print("\n4. 특정 사용자 조회 테스트")
        user_id = created_users[0]['id']
        try:
            response = requests.get(f"{BASE_URL}/users/{user_id}")
            print(f"Status: {response.status_code}")
            if response.status_code == 200:
                user = response.json()
                print(f"User details: {user}")
            else:
                print(f"Error: {response.text}")
        except Exception as e:
            print(f"Error: {e}")
    
    # 5. 사용자 업데이트 테스트
    if created_users:
        print("\n5. 사용자 업데이트 테스트")
        user_id = created_users[0]['id']
        update_data = {
            "age": 26,
            "is_active": False
        }
        try:
            response = requests.put(f"{BASE_URL}/users/{user_id}", json=update_data)
            print(f"Status: {response.status_code}")
            if response.status_code == 200:
                user = response.json()
                print(f"Updated user: {user}")
            else:
                print(f"Error: {response.text}")
        except Exception as e:
            print(f"Error: {e}")
    
    # 6. 사용자 검색 테스트
    print("\n6. 사용자 검색 테스트")
    try:
        response = requests.get(f"{BASE_URL}/users/search/김")
        print(f"Status: {response.status_code}")
        users = response.json()
        print(f"Search results for '김': {len(users)} users found")
        for user in users:
            print(f"  - {user['name']} ({user['email']})")
    except Exception as e:
        print(f"Error: {e}")
    
    # 7. 통계 조회 테스트
    print("\n7. 통계 조회 테스트")
    try:
        response = requests.get(f"{BASE_URL}/stats/users")
        print(f"Status: {response.status_code}")
        stats = response.json()
        print(f"User statistics: {stats}")
    except Exception as e:
        print(f"Error: {e}")
    
    # 8. 사용자 삭제 테스트
    if created_users:
        print("\n8. 사용자 삭제 테스트")
        user_id = created_users[-1]['id']  # 마지막 사용자 삭제
        try:
            response = requests.delete(f"{BASE_URL}/users/{user_id}")
            print(f"Status: {response.status_code}")
            if response.status_code == 200:
                user = response.json()
                print(f"Deleted user: {user['name']}")
            else:
                print(f"Error: {response.text}")
        except Exception as e:
            print(f"Error: {e}")
    
    # 9. 삭제 후 사용자 목록 확인
    print("\n9. 삭제 후 사용자 목록 확인")
    try:
        response = requests.get(f"{BASE_URL}/users/")
        print(f"Status: {response.status_code}")
        users = response.json()
        print(f"Remaining users: {len(users)}")
        for user in users:
            print(f"  - {user['name']} ({user['email']})")
    except Exception as e:
        print(f"Error: {e}")
    
    print("\n" + "=" * 50)
    print("✅ 테스트 완료!")

if __name__ == "__main__":
    # 서버가 실행 중인지 확인
    try:
        response = requests.get(f"{BASE_URL}/")
        if response.status_code == 200:
            test_api()
        else:
            print("❌ 서버가 실행되지 않았습니다. 먼저 서버를 시작해주세요:")
            print("python fastapi_sqlite_crud.py")
    except requests.exceptions.ConnectionError:
        print("❌ 서버에 연결할 수 없습니다. 먼저 서버를 시작해주세요:")
        print("python fastapi_sqlite_crud.py")
    except Exception as e:
        print(f"❌ 오류 발생: {e}") 