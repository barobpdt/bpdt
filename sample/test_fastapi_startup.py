#!/usr/bin/env python3
"""
FastAPI 애플리케이션 시작 테스트
"""

import sys
import io
import traceback

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

def test_fastapi_import():
    """FastAPI 모듈 import 테스트"""
    print("🔍 FastAPI 모듈 import 테스트")
    
    try:
        import fastapi_mysql_crud
        print("✅ fastapi_mysql_crud.py import 성공!")
        
        # 주요 컴포넌트 확인
        print(f"📊 FastAPI 앱: {fastapi_mysql_crud.app}")
        print(f"📊 데이터베이스 엔진: {fastapi_mysql_crud.engine}")
        print(f"📊 Base 클래스: {fastapi_mysql_crud.Base}")
        
        return True
        
    except Exception as e:
        print(f"❌ FastAPI import 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def test_database_connection():
    """데이터베이스 연결 테스트"""
    print("\n🔍 데이터베이스 연결 테스트")
    
    try:
        import fastapi_mysql_crud
        
        # 연결 테스트
        with fastapi_mysql_crud.engine.connect() as conn:
            result = conn.execute(fastapi_mysql_crud.text("SELECT 1 as test"))
            test_value = result.fetchone()[0]
            print(f"✅ 데이터베이스 연결 성공! 테스트 값: {test_value}")
        
        return True
        
    except Exception as e:
        print(f"❌ 데이터베이스 연결 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def test_models():
    """모델 정의 테스트"""
    print("\n🔍 모델 정의 테스트")
    
    try:
        import fastapi_mysql_crud
        
        # User 모델 확인
        print(f"📊 User 모델: {fastapi_mysql_crud.User}")
        print(f"📊 User 테이블명: {fastapi_mysql_crud.User.__tablename__}")
        
        # QR 모델들 확인
        print(f"📊 SgMemb 모델: {fastapi_mysql_crud.SgMemb}")
        print(f"📊 SgMembQr 모델: {fastapi_mysql_crud.SgMembQr}")
        print(f"📊 SgQrInfo 모델: {fastapi_mysql_crud.SgQrInfo}")
        print(f"📊 SgMembCall 모델: {fastapi_mysql_crud.SgMembCall}")
        
        return True
        
    except Exception as e:
        print(f"❌ 모델 정의 테스트 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def test_endpoints():
    """엔드포인트 테스트"""
    print("\n🔍 엔드포인트 테스트")
    
    try:
        import fastapi_mysql_crud
        
        # 앱의 라우터 확인
        routes = fastapi_mysql_crud.app.routes
        print(f"📊 등록된 라우트 수: {len(routes)}")
        
        # 주요 엔드포인트 확인
        for route in routes:
            if hasattr(route, 'path'):
                print(f"  📍 {route.methods} {route.path}")
        
        return True
        
    except Exception as e:
        print(f"❌ 엔드포인트 테스트 실패: {e}")
        print(f"❌ 오류 타입: {type(e)}")
        traceback.print_exc()
        return False

def main():
    """메인 함수"""
    print("🚀 FastAPI 애플리케이션 시작 테스트")
    print("=" * 50)
    
    # 각 테스트 실행
    import_ok = test_fastapi_import()
    
    if import_ok:
        db_ok = test_database_connection()
        models_ok = test_models()
        endpoints_ok = test_endpoints()
    else:
        db_ok = models_ok = endpoints_ok = False
    
    print("\n" + "=" * 50)
    print("📊 테스트 결과:")
    print(f"  모듈 Import: {'✅ 성공' if import_ok else '❌ 실패'}")
    print(f"  데이터베이스 연결: {'✅ 성공' if db_ok else '❌ 실패'}")
    print(f"  모델 정의: {'✅ 성공' if models_ok else '❌ 실패'}")
    print(f"  엔드포인트: {'✅ 성공' if endpoints_ok else '❌ 실패'}")
    
    if all([import_ok, db_ok, models_ok, endpoints_ok]):
        print("\n🎉 모든 테스트 통과! FastAPI 애플리케이션이 정상적으로 시작될 수 있습니다.")
    else:
        print("\n⚠️ 일부 테스트가 실패했습니다. 애플리케이션 시작에 문제가 있을 수 있습니다.")

if __name__ == "__main__":
    main() 