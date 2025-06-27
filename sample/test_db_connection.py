import pymysql
from sqlalchemy import create_engine, text
import sys

# 데이터베이스 설정
DB_HOST = "106.246.249.162:23381"
DB_NAME = "secretguard"
DB_USER = "secretguard"
DB_PASSWORD = "snflWkd22!"

def test_pymysql_connection():
    """PyMySQL을 사용한 직접 연결 테스트"""
    print("🔍 PyMySQL 직접 연결 테스트...")
    try:
        # 호스트와 포트 분리
        host, port = DB_HOST.split(':')
        port = int(port)
        
        connection = pymysql.connect(
            host=host,
            port=port,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            charset='utf8mb4',
            ssl={'ssl': {}}
        )
        
        with connection.cursor() as cursor:
            cursor.execute("SELECT VERSION()")
            version = cursor.fetchone()
            print(f"✅ PyMySQL 연결 성공! MySQL 버전: {version[0]}")
            
            # 데이터베이스 정보 조회
            cursor.execute("SELECT DATABASE()")
            current_db = cursor.fetchone()
            print(f"📊 현재 데이터베이스: {current_db[0]}")
            
            # 테이블 목록 조회
            cursor.execute("SHOW TABLES")
            tables = cursor.fetchall()
            print(f"📋 테이블 목록: {[table[0] for table in tables]}")
            
        connection.close()
        return True
        
    except Exception as e:
        print(f"❌ PyMySQL 연결 실패: {e}")
        return False

def test_sqlalchemy_connection():
    """SQLAlchemy를 사용한 연결 테스트"""
    print("\n🔍 SQLAlchemy 연결 테스트...")
    try:
        DATABASE_URL = f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}?charset=utf8mb4"
        
        engine = create_engine(
            DATABASE_URL,
            pool_pre_ping=True,
            pool_recycle=300,
            connect_args={
                "ssl": {
                    "ssl_ca": None,
                    "ssl_cert": None,
                    "ssl_key": None,
                    "ssl_verify_cert": False
                }
            }
        )
        
        with engine.connect() as connection:
            result = connection.execute(text("SELECT VERSION()"))
            version = result.fetchone()
            print(f"✅ SQLAlchemy 연결 성공! MySQL 버전: {version[0]}")
            
            # 데이터베이스 정보 조회
            result = connection.execute(text("SELECT DATABASE()"))
            current_db = result.fetchone()
            print(f"📊 현재 데이터베이스: {current_db[0]}")
            
            # 테이블 목록 조회
            result = connection.execute(text("SHOW TABLES"))
            tables = result.fetchall()
            print(f"📋 테이블 목록: {[table[0] for table in tables]}")
            
        return True
        
    except Exception as e:
        print(f"❌ SQLAlchemy 연결 실패: {e}")
        return False

def test_table_creation():
    """테이블 생성 테스트"""
    print("\n🔍 테이블 생성 테스트...")
    try:
        DATABASE_URL = f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}?charset=utf8mb4"
        
        engine = create_engine(
            DATABASE_URL,
            pool_pre_ping=True,
            pool_recycle=300,
            connect_args={
                "ssl": {
                    "ssl_ca": None,
                    "ssl_cert": None,
                    "ssl_key": None,
                    "ssl_verify_cert": False
                }
            }
        )
        
        # 테스트 테이블 생성
        create_table_sql = """
        CREATE TABLE IF NOT EXISTS test_table (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
        """
        
        with engine.connect() as connection:
            connection.execute(text(create_table_sql))
            connection.commit()
            print("✅ 테스트 테이블 생성 성공!")
            
            # 테이블 삭제
            connection.execute(text("DROP TABLE test_table"))
            connection.commit()
            print("✅ 테스트 테이블 삭제 성공!")
            
        return True
        
    except Exception as e:
        print(f"❌ 테이블 생성 테스트 실패: {e}")
        return False

def main():
    print("🚀 MySQL 데이터베이스 연결 테스트 시작")
    print("=" * 50)
    print(f"📊 데이터베이스: {DB_NAME}")
    print(f"🌐 호스트: {DB_HOST}")
    print(f"👤 사용자: {DB_USER}")
    print("=" * 50)
    
    # 각종 연결 테스트
    pymysql_success = test_pymysql_connection()
    sqlalchemy_success = test_sqlalchemy_connection()
    table_creation_success = test_table_creation()
    
    print("\n" + "=" * 50)
    print("📋 테스트 결과 요약:")
    print(f"PyMySQL 연결: {'✅ 성공' if pymysql_success else '❌ 실패'}")
    print(f"SQLAlchemy 연결: {'✅ 성공' if sqlalchemy_success else '❌ 실패'}")
    print(f"테이블 생성: {'✅ 성공' if table_creation_success else '❌ 실패'}")
    
    if pymysql_success and sqlalchemy_success and table_creation_success:
        print("\n🎉 모든 테스트가 성공했습니다!")
        print("FastAPI 애플리케이션을 실행할 수 있습니다.")
        return True
    else:
        print("\n⚠️ 일부 테스트가 실패했습니다.")
        print("데이터베이스 연결 설정을 확인해주세요.")
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1) 