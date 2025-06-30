from sqlalchemy.ext.asyncio import create_async_engine
from sqlalchemy.sql import text
import asyncio

import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

async def test_async_mysql_connection():
	"""비동기 MySQL 연결 테스트"""
	print("\n🔌 비동기 MySQL 연결 테스트...")
	
	try:	
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
				engine = create_async_engine(url, echo=True)
				
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
	
if __name__ == "__main__":
	asyncio.run(test_async_mysql_connection())