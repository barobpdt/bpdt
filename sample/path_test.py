import os
import sys

localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
sys.path.append(f'{localPath}/sample') 

from parse_ddl import DDLParser, SQLAlchemyGenerator

try:
	# DDL 파일 읽기
	with open(r'c:/temp/jkj_table.txt', 'r', encoding='utf-8') as f:
		ddl_content = f.read()
	# 파싱
	output_file = r'c:/temp/jkj_model.py'
	parser = DDLParser()
	tables = parser.parse_ddl(ddl_content)
	
	if not tables:
		print("No tables found in DDL file")
		sys.exit(1)
	
	# SQLAlchemy 모델 생성
	generator = SQLAlchemyGenerator()
	code = generator.generate_models(tables, output_file)
	
	if not output_file:
		print(code)
	else:
		print(f"Generated SQLAlchemy models saved to {output_file}")
		print(f"Found {len(tables)} tables:")
		for table in tables:
			print(f"  - {table.name} ({len(table.columns)} columns)")

except FileNotFoundError:
	print(f"Error: File '{ddl_file}' not found")
	sys.exit(1)
except Exception as e:
	print(f"Error: {str(e)}")
	sys.exit(1)