#!/usr/bin/env python3
"""
비동기 데이터베이스 연결 상세 진단 스크립트
연결 문제의 원인을 단계별로 분석합니다.
"""

import asyncio
from datetime import datetime

import os
import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')


def test_package_installation():
    """필요한 패키지 설치 상태 확인"""
    print("🔍 패키지 설치 상태 확인...")
    
    required_packages = [
        'fastapi',
        'sqlalchemy', 
        'aiomysql',
        'pymysql',
        'uvicorn'
    ]
    
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package)
            print(f"✅ {package} - 설치됨")
        except ImportError:
            print(f"❌ {package} - 설치되지 않음")
            missing_packages.append(package)
    
    if missing_packages:
        print(f"\n🚨 설치되지 않은 패키지: {', '.join(missing_packages)}")
        print("다음 명령으로 설치하세요:")
        print(f"python -m pip install {' '.join(missing_packages)}")
        return False
    else:
        print("✅ 모든 필수 패키지가 설치되어 있습니다.")
        return True

def test_network_connectivity():
    """네트워크 연결 상태 확인"""
    print("\n🌐 네트워크 연결 상태 확인...")
    
    import socket
    
    host = "106.246.249.162"
    port = 23381
    
    try:
        # 소켓 연결 테스트
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        result = sock.connect_ex((host, port))
        sock.close()
        
        if result == 0:
            print(f"✅ 네트워크 연결 성공: {host}:{port}")
            return True
        else:
            print(f"❌ 네트워크 연결 실패: {host}:{port}")
            print("   가능한 원인:")
            print("   - 방화벽 설정")
            print("   - 네트워크 연결 문제")
            print("   - 서버가 실행되지 않음")
            return False
            
    except Exception as e:
        print(f"❌ 네트워크 연결 테스트 중 오류: {e}")
        return False

async def test_async_mysql_connection():
    """비동기 MySQL 연결 테스트"""
    print("\n🔌 비동기 MySQL 연결 테스트...")
    
    try:
        from sqlalchemy.ext.asyncio import create_async_engine
        from sqlalchemy.sql import text
        
        # 연결 정보
        DB_HOST = "106.246.249.162:23381"
        DB_NAME = "secretguard"
        DB_USER = "muknoori"
        DB_PASSWORD = "snflWkd22!"
        
        host, port = DB_HOST.split(':')
        port = int(port)
        
        # 다양한 연결 문자열 테스트
        test_urls = [
            f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
            f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
            f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
        ]
        
        for i, url in enumerate(test_urls, 1):
            print(f"  시도 {i}: {url}")
            try:
                engine = create_async_engine(
                    url,
                    pool_pre_ping=True,
                    pool_recycle=300,
                    pool_timeout=30,
                    max_overflow=10,
                    pool_size=5,
                    echo=False,
                    connect_args={
                        "connect_timeout": 30,
                        "read_timeout": 30,
                        "write_timeout": 30
                    }
                )
                
                # 연결 테스트
                async with engine.begin() as conn:
                    result = await conn.execute(text("SELECT 1 as test"))
                    test_value = result.fetchone()[0]
                    print(f"    ✅ 연결 성공! 테스트 쿼리 결과: {test_value}")
                
                await engine.dispose()
                return True
                
            except Exception as e:
                print(f"    ❌ 연결 실패: {e}")
                await engine.dispose() if 'engine' in locals() else None
                continue
        
        print("❌ 모든 비동기 연결 시도 실패")
        return False
        
    except ImportError as e:
        print(f"❌ aiomysql 패키지가 설치되지 않음: {e}")
        return False
    except Exception as e:
        print(f"❌ 비동기 연결 테스트 중 오류: {e}")
        return False

def test_sync_mysql_connection():
    """동기 MySQL 연결 테스트"""
    print("\n🔌 동기 MySQL 연결 테스트...")
    
    try:
        from sqlalchemy import create_engine, text
        
        # 연결 정보
        DB_HOST = "106.246.249.162:23381"
        DB_NAME = "secretguard"
        DB_USER = "muknoori"
        DB_PASSWORD = "snflWkd22!"
        
        host, port = DB_HOST.split(':')
        port = int(port)
        
        # 다양한 연결 문자열 테스트
        test_urls = [
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
            f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
        ]
        
        for i, url in enumerate(test_urls, 1):
            print(f"  시도 {i}: {url}")
            try:
                engine = create_engine(
                    url,
                    pool_pre_ping=True,
                    pool_recycle=300,
                    pool_timeout=30,
                    max_overflow=10,
                    pool_size=5,
                    connect_args={
                        "connect_timeout": 30,
                        "read_timeout": 30,
                        "write_timeout": 30
                    }
                )
                
                # 연결 테스트
                with engine.connect() as conn:
                    result = conn.execute(text("SELECT 1 as test"))
                    test_value = result.fetchone()[0]
                    print(f"    ✅ 연결 성공! 테스트 쿼리 결과: {test_value}")
                
                engine.dispose()
                return True
                
            except Exception as e:
                print(f"    ❌ 연결 실패: {e}")
                engine.dispose() if 'engine' in locals() else None
                continue
        
        print("❌ 모든 동기 연결 시도 실패")
        return False
        
    except ImportError as e:
        print(f"❌ pymysql 패키지가 설치되지 않음: {e}")
        return False
    except Exception as e:
        print(f"❌ 동기 연결 테스트 중 오류: {e}")
        return False

def test_database_permissions():
    """데이터베이스 권한 확인"""
    print("\n🔐 데이터베이스 권한 확인...")
    
    try:
        from sqlalchemy import create_engine, text
        
        DB_HOST = "106.246.249.162:23381"
        DB_NAME = "secretguard"
        DB_USER = "muknoori"
        DB_PASSWORD = "snflWkd22!"
        
        host, port = DB_HOST.split(':')
        port = int(port)
        
        url = f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4"
        
        engine = create_engine(url, pool_pre_ping=True)
        
        with engine.connect() as conn:
            # 사용자 권한 확인
            result = conn.execute(text("SHOW GRANTS"))
            grants = result.fetchall()
            print("✅ 사용자 권한:")
            for grant in grants:
                print(f"   {grant[0]}")
            
            # 데이터베이스 목록 확인
            result = conn.execute(text("SHOW DATABASES"))
            databases = [row[0] for row in result.fetchall()]
            print(f"✅ 접근 가능한 데이터베이스: {databases}")
            
            if DB_NAME in databases:
                print(f"✅ {DB_NAME} 데이터베이스에 접근 가능")
            else:
                print(f"❌ {DB_NAME} 데이터베이스에 접근 불가")
        
        engine.dispose()
        return True
        
    except Exception as e:
        print(f"❌ 권한 확인 중 오류: {e}")
        return False

def generate_diagnostic_report():
    """진단 보고서 생성"""
    print("\n" + "="*60)
    print("📋 데이터베이스 연결 진단 보고서")
    print("="*60)
    print(f"진단 시간: {datetime.now()}")
    
    # 1. 패키지 설치 상태
    packages_ok = test_package_installation()
    
    # 2. 네트워크 연결
    network_ok = test_network_connectivity()
    
    # 3. 동기 MySQL 연결
    # sync_ok = test_sync_mysql_connection()
    
    # 4. 비동기 MySQL 연결
    async_ok = asyncio.run(test_async_mysql_connection())
    
    # 5. 데이터베이스 권한
    permissions_ok = test_database_permissions()
    
    # 결과 요약
    print("\n" + "="*60)
    print("📊 진단 결과 요약")
    print("="*60)
    print(f"패키지 설치: {'✅' if packages_ok else '❌'}")
    print(f"네트워크 연결: {'✅' if network_ok else '❌'}")
    print(f"동기 MySQL 연결: {'✅' if sync_ok else '❌'}")
    print(f"비동기 MySQL 연결: {'✅' if async_ok else '❌'}")
    print(f"데이터베이스 권한: {'✅' if permissions_ok else '❌'}")
    
    # 권장사항
    print("\n💡 권장사항:")
    if not packages_ok:
        print("- 필요한 패키지들을 설치하세요")
    if not network_ok:
        print("- 네트워크 연결을 확인하세요")
    if not sync_ok and not async_ok:
        print("- 데이터베이스 서버 상태를 확인하세요")
    if not permissions_ok:
        print("- 데이터베이스 사용자 권한을 확인하세요")
    
    if packages_ok and network_ok and (sync_ok or async_ok) and permissions_ok:
        print("✅ 모든 진단이 성공했습니다!")
    else:
        print("❌ 일부 진단에서 문제가 발견되었습니다.")

if __name__ == "__main__":
    generate_diagnostic_report() 