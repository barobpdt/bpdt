#!/usr/bin/env python3
"""
데이터베이스 연결 오류 상세 진단
"""

import pymysql
import sqlalchemy
from sqlalchemy import create_engine, text
import sys
import traceback
import io

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

# 데이터베이스 설정
DB_HOST = "106.246.249.162:23381"
DB_NAME = "secretguard"
DB_USER = "muknoori"
DB_PASSWORD = "snflWkd22!"

def test_pymysql_direct():
    """PyMySQL 직접 연결 테스트"""
    print("🔍 PyMySQL 직접 연결 테스트")
    
    try:
        # 호스트와 포트 분리
        if ':' in DB_HOST:
            host, port = DB_HOST.split(':')
            port = int(port)
        else:
            host = DB_HOST
            port = 3306
        
        print(f"📊 연결 정보:")
        print(f"  호스트: {host}")
        print(f"  포트: {port}")
        print(f"  데이터베이스: {DB_NAME}")
        print(f"  사용자: {DB_USER}")
        
        # PyMySQL 직접 연결
        connection = pymysql.connect(
            host=host,
            port=port,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            charset='utf8mb4',
            ssl={'ssl': {}}
        )
        
        print("✅ PyMySQL 직접 연결 성공!")
        
        # 연결 테스트
        with connection.cursor() as cursor:
            cursor.execute("SELECT 1")
            result = cursor.fetchone()
            print(f"📊 쿼리 결과: {result}")
        
        connection.close()
        return True
        
    except Exception as e:
        print(f"❌ PyMySQL 직접 연결 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def test_sqlalchemy_connection():
    """SQLAlchemy 연결 테스트"""
    print("\n🔍 SQLAlchemy 연결 테스트")
    
    try:
        # 다양한 연결 문자열 테스트
        connection_strings = [
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}?charset=utf8mb4",
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}",
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
        ]
        
        for i, conn_str in enumerate(connection_strings, 1):
            print(f"\n📊 연결 문자열 {i}: {conn_str}")
            
            try:
                # 엔진 생성 (SSL 설정 없이)
                engine = create_engine(
                    conn_str,
                    pool_pre_ping=True,
                    pool_recycle=300,
                    echo=True  # SQL 로그 출력
                )
                
                # 연결 테스트
                with engine.connect() as connection:
                    result = connection.execute(text("SELECT 1"))
                    print(f"✅ 연결 {i} 성공!")
                    print(f"📊 쿼리 결과: {result.fetchone()}")
                    return True
                    
            except Exception as e:
                print(f"❌ 연결 {i} 실패: {e}")
                print(f"❌ 오류 타입: {type(e)}")
                continue
        
        return False
        
    except Exception as e:
        print(f"❌ SQLAlchemy 연결 테스트 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def test_ssl_connection():
    """SSL 연결 테스트"""
    print("\n🔍 SSL 연결 테스트")
    
    try:
        # 호스트와 포트 분리
        if ':' in DB_HOST:
            host, port = DB_HOST.split(':')
            port = int(port)
        else:
            host = DB_HOST
            port = 3306
        
        # SSL 설정으로 PyMySQL 연결
        connection = pymysql.connect(
            host=host,
            port=port,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            charset='utf8mb4',
            ssl={
                'ssl_ca': None,
                'ssl_cert': None,
                'ssl_key': None,
                'ssl_verify_cert': False
            }
        )
        
        print("✅ SSL 연결 성공!")
        connection.close()
        return True
        
    except Exception as e:
        print(f"❌ SSL 연결 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def test_network_connectivity():
    """네트워크 연결성 테스트"""
    print("\n🔍 네트워크 연결성 테스트")
    
    import socket
    
    try:
        # 호스트와 포트 분리
        if ':' in DB_HOST:
            host, port = DB_HOST.split(':')
            port = int(port)
        else:
            host = DB_HOST
            port = 3306
        
        print(f"📊 호스트: {host}, 포트: {port}")
        
        # 소켓 연결 테스트
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)  # 10초 타임아웃
        
        result = sock.connect_ex((host, port))
        sock.close()
        
        if result == 0:
            print("✅ 네트워크 연결 성공!")
            return True
        else:
            print(f"❌ 네트워크 연결 실패! 오류 코드: {result}")
            return False
            
    except Exception as e:
        print(f"❌ 네트워크 테스트 실패: {e}")
        return False

def main():
    """메인 함수"""
    print("🚀 데이터베이스 연결 오류 진단 시작")
    print("=" * 50)
    
    # 네트워크 연결성 테스트
    network_ok = test_network_connectivity()
    
    if not network_ok:
        print("\n⚠️ 네트워크 연결이 실패했습니다. 다음을 확인해주세요:")
        print("  1. 인터넷 연결 상태")
        print("  2. 방화벽 설정")
        print("  3. 호스트 주소와 포트가 올바른지")
        return
    
    # PyMySQL 직접 연결 테스트
    pymysql_ok = test_pymysql_direct()
    
    # SSL 연결 테스트
    ssl_ok = test_ssl_connection()
    
    # SQLAlchemy 연결 테스트
    sqlalchemy_ok = test_sqlalchemy_connection()
    
    print("\n" + "=" * 50)
    print("📊 진단 결과:")
    print(f"  네트워크 연결: {'✅ 성공' if network_ok else '❌ 실패'}")
    print(f"  PyMySQL 직접 연결: {'✅ 성공' if pymysql_ok else '❌ 실패'}")
    print(f"  SSL 연결: {'✅ 성공' if ssl_ok else '❌ 실패'}")
    print(f"  SQLAlchemy 연결: {'✅ 성공' if sqlalchemy_ok else '❌ 실패'}")
    
    if not sqlalchemy_ok:
        print("\n🔧 해결 방법:")
        print("  1. 데이터베이스 서버가 실행 중인지 확인")
        print("  2. 사용자명과 비밀번호가 올바른지 확인")
        print("  3. 데이터베이스명이 올바른지 확인")
        print("  4. SSL 설정을 확인")
        print("  5. 방화벽에서 해당 포트가 열려있는지 확인")

if __name__ == "__main__":
    main() 