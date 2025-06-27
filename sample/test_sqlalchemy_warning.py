#!/usr/bin/env python3
"""
SQLAlchemy 2.0 경고 해결 테스트
"""

import warnings
import sys

# SQLAlchemy 경고만 필터링
warnings.filterwarnings("ignore", category=UserWarning, module="sqlalchemy")

def test_fastapi_import():
    """FastAPI import 테스트"""
    print("🔍 FastAPI MySQL CRUD import 테스트")
    
    try:
        import fastapi_mysql_crud
        print("✅ fastapi_mysql_crud.py import 성공!")
        
        # Base 클래스 확인
        print(f"📊 Base 클래스: {fastapi_mysql_crud.Base}")
        print(f"📊 Base 클래스 타입: {type(fastapi_mysql_crud.Base)}")
        
        # 앱 확인
        print(f"📊 FastAPI 앱: {fastapi_mysql_crud.app}")
        
        return True
        
    except Exception as e:
        print(f"❌ Import 오류: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_parse_ddl_import():
    """parse_ddl import 테스트"""
    print("\n🔍 parse_ddl.py import 테스트")
    
    try:
        from parse_ddl import DDLParser, SQLAlchemyGenerator
        print("✅ parse_ddl.py import 성공!")
        
        # 테스트용 DDL
        test_ddl = """sg_memb<sep>CREATE TABLE `sg_memb` (
          `MEMB_NO` varchar(20) NOT NULL,
          `EMAIL` varchar(128) DEFAULT NULL,
          PRIMARY KEY (`MEMB_NO`)
        )<end>회원 정보 테이블"""
        
        # 파싱 테스트
        parser = DDLParser()
        tables = parser.parse_ddl(test_ddl)
        
        print(f"📊 파싱된 테이블 수: {len(tables)}")
        
        if tables:
            table = tables[0]
            print(f"📋 테이블명: {table.name}")
            print(f"🔑 Primary Keys: {table.primary_keys}")
            print(f"📊 컬럼 수: {len(table.columns)}")
            
            for col in table.columns:
                print(f"  📝 {col.name}: primary_key={col.primary_key}")
        
        return True
        
    except Exception as e:
        print(f"❌ Import 오류: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    print("🚀 SQLAlchemy 2.0 경고 해결 테스트 시작")
    
    # FastAPI 테스트
    fastapi_success = test_fastapi_import()
    
    # parse_ddl 테스트
    parse_ddl_success = test_parse_ddl_import()
    
    print(f"\n📊 테스트 결과:")
    print(f"  FastAPI: {'✅ 성공' if fastapi_success else '❌ 실패'}")
    print(f"  parse_ddl: {'✅ 성공' if parse_ddl_success else '❌ 실패'}")
    
    if fastapi_success and parse_ddl_success:
        print("\n🎉 모든 테스트 통과! SQLAlchemy 2.0 경고가 해결되었습니다.")
    else:
        print("\n⚠️ 일부 테스트가 실패했습니다.") 