import requests
import json
from typing import Dict, Any

class FastAPISupabaseClient:
    def __init__(self, base_url: str = "http://localhost:8000"):
        self.base_url = base_url
        self.session = requests.Session()
    
    def get_all_users(self) -> Dict[str, Any]:
        """모든 사용자 조회"""
        response = self.session.get(f"{self.base_url}/users")
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }
    
    def get_user(self, user_id: int) -> Dict[str, Any]:
        """특정 사용자 조회"""
        response = self.session.get(f"{self.base_url}/users/{user_id}")
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }
    
    def create_user(self, user_data: Dict[str, Any]) -> Dict[str, Any]:
        """새 사용자 생성"""
        response = self.session.post(
            f"{self.base_url}/users",
            json=user_data,
            headers={"Content-Type": "application/json"}
        )
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }
    
    def update_user(self, user_id: int, update_data: Dict[str, Any]) -> Dict[str, Any]:
        """사용자 정보 수정"""
        response = self.session.put(
            f"{self.base_url}/users/{user_id}",
            json=update_data,
            headers={"Content-Type": "application/json"}
        )
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }
    
    def delete_user(self, user_id: int) -> Dict[str, Any]:
        """사용자 삭제"""
        response = self.session.delete(f"{self.base_url}/users/{user_id}")
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }
    
    def search_users(self, keyword: str) -> Dict[str, Any]:
        """사용자 검색"""
        response = self.session.get(f"{self.base_url}/users/search/{keyword}")
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }
    
    def get_api_info(self) -> Dict[str, Any]:
        """API 정보 조회"""
        response = self.session.get(f"{self.base_url}/")
        return {
            "status_code": response.status_code,
            "data": response.json() if response.status_code == 200 else response.text
        }

def print_response(title: str, response: Dict[str, Any]):
    """응답 결과를 예쁘게 출력"""
    print(f"\n{'='*50}")
    print(f"📋 {title}")
    print(f"{'='*50}")
    print(f"Status Code: {response['status_code']}")
    print(f"Response: {json.dumps(response['data'], indent=2, ensure_ascii=False)}")

def main():
    """메인 테스트 함수"""
    client = FastAPISupabaseClient()
    
    print("🚀 FastAPI + Supabase 예제 테스트 시작")
    
    # 1. API 정보 조회
    print_response("API 정보", client.get_api_info())
    
    # 2. 현재 사용자 목록 조회
    print_response("현재 사용자 목록", client.get_all_users())
    
    # 3. 새 사용자 생성
    new_user = {
        "name": "홍길동",
        "email": "hong@example.com",
        "age": 30,
        "city": "서울"
    }
    print_response("새 사용자 생성", client.create_user(new_user))
    
    # 4. 다른 사용자 생성
    another_user = {
        "name": "김철수",
        "email": "kim@example.com",
        "age": 25,
        "city": "부산"
    }
    print_response("두 번째 사용자 생성", client.create_user(another_user))
    
    # 5. 업데이트된 사용자 목록 조회
    print_response("업데이트된 사용자 목록", client.get_all_users())
    
    # 6. 특정 사용자 조회 (ID: 1)
    print_response("사용자 ID 1 조회", client.get_user(1))
    
    # 7. 사용자 정보 수정
    update_data = {
        "age": 31,
        "city": "대구"
    }
    print_response("사용자 정보 수정", client.update_user(1, update_data))
    
    # 8. 수정된 사용자 확인
    print_response("수정된 사용자 확인", client.get_user(1))
    
    # 9. 사용자 검색
    print_response("'홍' 검색 결과", client.search_users("홍"))
    print_response("'kim' 검색 결과", client.search_users("kim"))
    
    # 10. 사용자 삭제
    print_response("사용자 삭제", client.delete_user(2))
    
    # 11. 최종 사용자 목록 확인
    print_response("최종 사용자 목록", client.get_all_users())
    
    print(f"\n{'='*50}")
    print("✅ 테스트 완료!")
    print("📖 API 문서: http://localhost:8000/docs")
    print(f"{'='*50}")

if __name__ == "__main__":
    main() 