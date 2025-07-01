#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Pydantic 모델을 기반으로 Supabase 테이블 생성 기능 테스트
"""

import requests
import json
import sys
import os

# 현재 디렉토리를 Python 경로에 추가
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

def test_table_generation():
    """테이블 생성 기능 테스트"""
    
    base_url = "http://localhost:8000"
    
    print("=" * 60)
    print("Pydantic 모델 기반 Supabase 테이블 생성 테스트")
    print("=" * 60)
    
    # 1. 테이블 정보 조회
    print("\n1. 테이블 정보 조회")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/table-info")
        if response.status_code == 200:
            table_info = response.json()
            print(f"테이블명: {table_info['table_name']}")
            print(f"테이블 존재: {table_info['table_exists']}")
            print(f"모델 클래스: {table_info['model_class']}")
            print(f"컬럼 수: {table_info['total_columns']}")
            
            print("\n컬럼 정보:")
            for col in table_info['columns']:
                nullable = "NULL" if col['nullable'] else "NOT NULL"
                unique = "UNIQUE" if col['unique'] else ""
                pk = "PRIMARY KEY" if col['primary_key'] else ""
                default = f"DEFAULT {col['default']}" if col['default'] else ""
                
                constraints = [c for c in [nullable, unique, pk, default] if c]
                print(f"  - {col['name']}: {col['type']} ({', '.join(constraints)})")
                
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"테이블 정보 조회 실패: {e}")
    
    # 2. 스키마 생성
    print("\n2. 테이블 스키마 생성")
    print("-" * 30)
    try:
        response = requests.post(f"{base_url}/generate-schema")
        if response.status_code == 200:
            schema_info = response.json()
            print(f"메시지: {schema_info['message']}")
            print(f"테이블명: {schema_info['table_name']}")
            
            print("\n생성된 SQL DDL:")
            print("-" * 20)
            print(schema_info['sql_ddl'])
            print("-" * 20)
            
            print(f"\n지시사항: {schema_info['instructions']}")
            
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"스키마 생성 실패: {e}")
    
    # 3. 테이블 생성 (스키마 생성)
    print("\n3. 테이블 생성 (스키마 생성)")
    print("-" * 30)
    try:
        response = requests.post(f"{base_url}/create-table")
        if response.status_code == 200:
            create_info = response.json()
            print(f"메시지: {create_info['message']}")
            print(f"테이블명: {create_info['table_name']}")
            
            print("\n생성된 SQL DDL:")
            print("-" * 20)
            print(create_info['sql_ddl'])
            print("-" * 20)
            
            print("\n다음 단계:")
            for i, step in enumerate(create_info['next_steps'], 1):
                print(f"  {step}")
            
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"테이블 생성 실패: {e}")
    
    # 4. API 엔드포인트 확인
    print("\n4. API 엔드포인트 확인")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/")
        if response.status_code == 200:
            api_info = response.json()
            print(f"API 제목: {api_info['message']}")
            print(f"문서: {api_info['docs']}")
            print(f"웹 인터페이스: {api_info['web_interface']}")
            
            print("\n사용 가능한 엔드포인트:")
            for endpoint, description in api_info['endpoints'].items():
                print(f"  - {endpoint}: {description}")
            
        else:
            print(f"오류: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"API 정보 조회 실패: {e}")

def test_user_operations():
    """사용자 CRUD 작업 테스트"""
    
    base_url = "http://localhost:8000"
    
    print("\n" + "=" * 60)
    print("사용자 CRUD 작업 테스트")
    print("=" * 60)
    
    # 1. 사용자 생성
    print("\n1. 사용자 생성")
    print("-" * 30)
    try:
        user_data = {
            "name": "홍길동",
            "email": "hong@example.com",
            "age": 30,
            "city": "서울"
        }
        
        response = requests.post(f"{base_url}/users", json=user_data)
        if response.status_code == 200:
            user = response.json()
            print(f"사용자 생성 성공!")
            print(f"ID: {user['id']}")
            print(f"이름: {user['name']}")
            print(f"이메일: {user['email']}")
            print(f"나이: {user['age']}")
            print(f"도시: {user['city']}")
            print(f"생성일: {user['created_at']}")
            
            user_id = user['id']
        else:
            print(f"사용자 생성 실패: {response.status_code} - {response.text}")
            return
            
    except Exception as e:
        print(f"사용자 생성 실패: {e}")
        return
    
    # 2. 사용자 조회
    print("\n2. 사용자 조회")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/users/{user_id}")
        if response.status_code == 200:
            user = response.json()
            print(f"사용자 조회 성공!")
            print(f"ID: {user['id']}")
            print(f"이름: {user['name']}")
            print(f"이메일: {user['email']}")
        else:
            print(f"사용자 조회 실패: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"사용자 조회 실패: {e}")
    
    # 3. 모든 사용자 조회
    print("\n3. 모든 사용자 조회")
    print("-" * 30)
    try:
        response = requests.get(f"{base_url}/users")
        if response.status_code == 200:
            users = response.json()
            print(f"총 {len(users)}명의 사용자가 있습니다:")
            for user in users:
                print(f"  - {user['id']}: {user['name']} ({user['email']})")
        else:
            print(f"사용자 목록 조회 실패: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"사용자 목록 조회 실패: {e}")

if __name__ == "__main__":
    print("FastAPI 서버가 실행 중인지 확인하세요 (http://localhost:8000)")
    print("서버가 실행되지 않은 경우 다음 명령어로 실행하세요:")
    print("cd supabase_api && python fastapi_supabase.py")
    print()
    
    try:
        # 테이블 생성 기능 테스트
        test_table_generation()
        
        # 사용자 CRUD 작업 테스트
        test_user_operations()
        
    except KeyboardInterrupt:
        print("\n테스트가 중단되었습니다.")
    except Exception as e:
        print(f"\n테스트 중 오류 발생: {e}")
    
    print("\n" + "=" * 60)
    print("테스트 완료!")
    print("=" * 60) 