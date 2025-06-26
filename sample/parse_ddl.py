import re
import sys
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
#https://colab.research.google.com/github/i-am-shuan/learn-langchain/blob/main/langchain_chroma_crud_example.ipynb#scrollTo=cYlYbuTzGV2e

class ColumnType(Enum):
	INTEGER = "Integer"
	BIGINT = "BigInteger"
	SMALLINT = "SmallInteger"
	FLOAT = "Float"
	DECIMAL = "Numeric"
	VARCHAR = "String"
	CHAR = "String"
	TEXT = "Text"
	BOOLEAN = "Boolean"
	DATE = "Date"
	DATETIME = "DateTime"
	TIMESTAMP = "DateTime"
	JSON = "JSON"
	BLOB = "LargeBinary"
	CLOB = "Text"
	STRING = "String"
	NUMERIC = "Numeric"

@dataclass
class Column:
	name: str
	type: str
	nullable: bool = True
	primary_key: bool = False
	auto_increment: bool = False
	unique: bool = False
	default: Optional[str] = None
	length: Optional[int] = None
	precision: Optional[int] = None
	scale: Optional[int] = None
	comment: Optional[str] = None

@dataclass
class Table:
	name: str
	desc: str
	columns: List[Column]
	primary_keys: List[str]
	foreign_keys: List[Dict]
	indexes: List[Dict]
	comment: Optional[str] = None

class DDLParser:
	def __init__(self):
		self.tables: List[Table] = []
		
	def parse_ddl(self, ddl_content: str) -> List[Table]:
		"""DDL 내용을 파싱하여 테이블 정보를 추출합니다."""
		# 주석 제거
		# ddl_content = self._remove_comments(ddl_content)
		
		# CREATE TABLE 문들을 찾기 - 중첩된 괄호를 올바르게 처리
		# 더 정확한 패턴: 테이블명과 테이블 본문을 분리하여 추출
		for info in ddl_content.split('<end>'):
			if not info.strip():
				break
			table_name, table_body, table_desc = info.split('<sep>')
			# print("@@ parse_ddl table_body:", table_body[:100] + "..." if len(table_body) > 100 else table_body)
			table = self._parse_table(table_name.strip(), table_body, table_desc)
			if table:
				self.tables.append(table)
		
		return self.tables
	
	def _remove_comments(self, content: str) -> str:
		"""SQL 주석을 제거합니다."""
		# -- 주석 제거
		content = re.sub(r'--.*$', '', content, flags=re.MULTILINE)
		# /* */ 주석 제거
		content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
		return content
	
	def _parse_table(self, table_name: str, table_body: str, table_desc: str) -> Optional[Table]:
		"""테이블 본문을 파싱합니다."""
		columns = []
		primary_keys = []
		foreign_keys = []
		indexes = []
		
		# 각 라인을 처리
		lines = [line.strip() for line in table_body.split('\n') if line.strip()]
		
		for line in lines:
			line = line.strip().rstrip(',')
			if not line:
				continue
				
			# PRIMARY KEY 처리
			if re.match(r'PRIMARY\s+KEY', line, re.IGNORECASE):
				try:
					pk_match = re.search(r'PRIMARY\s+KEY\s*\(([^)]+)\)', line, re.IGNORECASE)
					if pk_match:
						aa=pk_match.group(1)
						pk_columns = [col.strip().strip('`') for col in aa.split(',')]
						primary_keys.extend(pk_columns)
				except Exception as e:
					print(f"primary key match error line={line}")
				continue
			
			# FOREIGN KEY 처리
			if re.match(r'FOREIGN\s+KEY', line, re.IGNORECASE):
				try:
					fk_match = re.search(r'FOREIGN\s+KEY\s*\(([^)]+)\)\s+REFERENCES\s+`?(\w+)`?\s*\(([^)]+)\)', line, re.IGNORECASE)
					if fk_match:
						foreign_keys.append({
							'column': fk_match.group(1).strip().strip('`'),
							'ref_table': fk_match.group(2),
							'ref_column': fk_match.group(3).strip().strip('`')
						})
				except Exception as e:
					print(f"primary key match error line={line}")				
				continue
			
			# INDEX 처리
			if re.match(r'(?:UNIQUE\s+)?KEY\s+`?\w+`?\s*\(', line, re.IGNORECASE):
				try:
					index_match = re.search(r'(?:UNIQUE\s+)?KEY\s+`?(\w+)`?\s*\(([^)]+)\)', line, re.IGNORECASE)
					if index_match:
						indexes.append({
							'name': index_match.group(1),
							'columns': [col.strip().strip('`') for col in index_match.group(2).split(',')],
							'unique': 'UNIQUE' in line.upper()
						})
				except Exception as e:
					print(f"primary key match error line={line}")
				continue
			
			# 컬럼 정의 처리
			column = self._parse_column(line)
			if column:
				columns.append(column)

		if columns:
			return Table(
				name=table_name,
				desc=table_desc,
				columns=columns,
				primary_keys=primary_keys,
				foreign_keys=foreign_keys,
				indexes=indexes
			)
		return None
	
	def _parse_column(self, line: str) -> Optional[Column]:
		"""컬럼 정의를 파싱합니다."""
		# 컬럼명 추출 - 괄호를 포함한 타입도 추출하도록 수정
		col_match = re.match(r'`?(\w+)`?\s+([^,\s]+(?:\([^)]*\))?)', line)
		if not col_match:
			print("@@ _parse_column error", line)
			return None
		col_name = col_match.group(1)
		col_type = col_match.group(2).upper()
		# 타입 파싱
		sqlalchemy_type, length, precision, scale = self._parse_type(col_type)
		
		# 제약조건 파싱
		nullable = 'NOT NULL' not in line.upper()
		primary_key = 'PRIMARY KEY' in line.upper()
		auto_increment = 'AUTO_INCREMENT' in line.upper() or 'IDENTITY' in line.upper()
		unique = 'UNIQUE' in line.upper()
		
		# 기본값 파싱
		default_match = re.search(r'DEFAULT\s+([^,\s]+)', line, re.IGNORECASE)
		default_value = default_match.group(1) if default_match else None
		
		# MySQL 커멘트 파싱 - 다양한 형식 지원
		comment = ''
		if 'COMMENT' in line:
			comment = self._extract_comment(line)
		
		return Column(
			name=col_name,
			type=sqlalchemy_type,
			nullable=nullable,
			primary_key=primary_key,
			auto_increment=auto_increment,
			unique=unique,
			default=default_value,
			length=length,
			precision=precision,
			scale=scale,
			comment=comment
		)
	
	def _extract_comment(self, line: str) -> Optional[str]:
		"""MySQL 커멘트를 추출합니다. 다양한 형식을 지원합니다."""
		# 1. COMMENT 'comment' 형식 (작은따옴표)
		comment_match = re.search(r"COMMENT\s+'([^']*)'", line, re.IGNORECASE)
		if comment_match:
			return comment_match.group(1)
		
		# 2. COMMENT "comment" 형식 (큰따옴표)
		comment_match = re.search(r'COMMENT\s+"([^"]*)"', line, re.IGNORECASE)
		if comment_match:
			return comment_match.group(1)
		
		# 3. COMMENT `comment` 형식 (백틱)
		comment_match = re.search(r'COMMENT\s+`([^`]*)`', line, re.IGNORECASE)
		if comment_match:
			return comment_match.group(1)
		
		# 4. COMMENT comment 형식 (따옴표 없음, 공백으로 구분)
		comment_match = re.search(r'COMMENT\s+([^\s,]+)', line, re.IGNORECASE)
		if comment_match:
			return comment_match.group(1)
		
		# 5. COMMENT 뒤에 오는 전체 문자열 (마지막 쉼표나 세미콜론까지)
		comment_match = re.search(r'COMMENT\s+(.+?)(?:\s*,\s*|\s*$)', line, re.IGNORECASE)
		if comment_match:
			comment_text = comment_match.group(1).strip()
			# 따옴표 제거
			if comment_text.startswith("'") and comment_text.endswith("'"):
				return comment_text[1:-1]
			elif comment_text.startswith('"') and comment_text.endswith('"'):
				return comment_text[1:-1]
			elif comment_text.startswith('`') and comment_text.endswith('`'):
				return comment_text[1:-1]
			else:
				return comment_text
		
		return None
	
	def _parse_type(self, type_str: str) -> Tuple[str, Optional[int], Optional[int], Optional[int]]:
		"""SQL 타입을 SQLAlchemy 타입으로 변환합니다."""
		type_str = type_str.upper()
		
		# VARCHAR(n)
		if 'VARCHAR' in type_str:
			length = self._extract_length(type_str)
			return ColumnType.VARCHAR.value, length, None, None
		
		# CHAR(n)
		elif 'CHAR' in type_str and 'VARCHAR' not in type_str:
			length = self._extract_length(type_str)
			return ColumnType.CHAR.value, length, None, None
		
		# INT, INTEGER
		elif any(t in type_str for t in ['INT', 'INTEGER']):
			if 'BIG' in type_str:
				return ColumnType.BIGINT.value, None, None, None
			elif 'SMALL' in type_str:
				return ColumnType.SMALLINT.value, None, None, None
			else:
				return ColumnType.INTEGER.value, None, None, None
		
		# DECIMAL, NUMERIC
		elif any(t in type_str for t in ['DECIMAL', 'NUMERIC']):
			precision, scale = self._extract_precision_scale(type_str)
			return ColumnType.DECIMAL.value, None, precision, scale
		
		# FLOAT, DOUBLE
		elif any(t in type_str for t in ['FLOAT', 'DOUBLE']):
			return ColumnType.FLOAT.value, None, None, None
		
		# TEXT
		elif 'TEXT' in type_str:
			return ColumnType.TEXT.value, None, None, None
		
		# BOOLEAN, BOOL
		elif any(t in type_str for t in ['BOOLEAN', 'BOOL']):
			return ColumnType.BOOLEAN.value, None, None, None
		
		# DATE
		elif 'DATE' in type_str and 'TIME' not in type_str:
			return ColumnType.DATE.value, None, None, None
		
		# DATETIME, TIMESTAMP
		elif any(t in type_str for t in ['DATETIME', 'TIMESTAMP']):
			return ColumnType.DATETIME.value, None, None, None
		
		# JSON
		elif 'JSON' in type_str:
			return ColumnType.JSON.value, None, None, None
		
		# BLOB
		elif 'BLOB' in type_str:
			return ColumnType.BLOB.value, None, None, None
		
		# 기본값
		else:
			return ColumnType.VARCHAR.value, None, None, None
	
	def _extract_length(self, type_str: str) -> Optional[int]:
		"""타입에서 길이를 추출합니다."""
		match = re.search(r'\((\d+)\)', type_str)
		return int(match.group(1)) if match else None
	
	def _extract_precision_scale(self, type_str: str) -> Tuple[Optional[int], Optional[int]]:
		"""타입에서 precision과 scale을 추출합니다."""
		match = re.search(r'\((\d+)(?:,(\d+))?\)', type_str)
		if match:
			precision = int(match.group(1))
			scale = int(match.group(2)) if match.group(2) else None
			return precision, scale
		return None, None

class SQLAlchemyGenerator:
	def __init__(self):
		self.imports = set()
		self.relationships = []
	
	def generate_models(self, tables: List[Table], output_file: str = None) -> str:
		"""테이블 정보를 바탕으로 SQLAlchemy 모델을 생성합니다."""
		self.imports.clear()
		self.relationships.clear()
		# 필요한 import 수집
		self._collect_imports(tables)
		
		# 관계 설정 수집
		self._collect_relationships(tables)
		
		# 모델 코드 생성
		code = self._generate_code(tables)

		if output_file:
			with open(output_file, 'w', encoding='utf-8') as f:
				f.write(code)
		
		return code
	
	def _collect_imports(self, tables: List[Table]):
		"""필요한 import를 수집합니다."""
		self.imports.add("from sqlalchemy import Column, Integer, String, Text, Boolean, Date, DateTime, Numeric, BigInteger, SmallInteger, Float, JSON, LargeBinary, ForeignKey")
		self.imports.add("from sqlalchemy.ext.declarative import declarative_base")
		self.imports.add("from sqlalchemy.orm import relationship")
		self.imports.add("from datetime import datetime")
		
		# 사용되는 타입들 확인
		used_types = set()
		for table in tables:
			for column in table.columns:
				used_types.add(column.type)
		
		# 필요한 import 추가
		if any('Numeric' in t for t in used_types):
			self.imports.add("from decimal import Decimal")
	
	def _collect_relationships(self, tables: List[Table]):
		"""외래키 관계를 수집합니다."""
		for table in tables:
			for fk in table.foreign_keys:
				self.relationships.append({
					'table': table.name,
					'column': fk['column'],
					'ref_table': fk['ref_table'],
					'ref_column': fk['ref_column']
				})
	
	def _generate_code(self, tables: List[Table]) -> str:
		"""전체 코드를 생성합니다."""
		code_lines = []
		
		# 헤더
		code_lines.append('"""')
		code_lines.append('Auto-generated SQLAlchemy models from DDL')
		code_lines.append('Generated by DDL to SQLAlchemy Generator')
		code_lines.append('"""')
		code_lines.append('')
		
		# Imports
		for import_stmt in sorted(self.imports):
			code_lines.append(import_stmt)
		code_lines.append('')
		
		# Base class
		code_lines.append('Base = declarative_base()')
		code_lines.append('')
		
		# Models
		for table in tables:
			model_code = self._generate_model(table)
			code_lines.extend(model_code)
			code_lines.append('')
		
		return '\n'.join(code_lines)
	
	def _generate_model(self, table: Table) -> List[str]:
		"""개별 모델을 생성합니다."""
		lines = []
		
		# 클래스 정의
		class_name = self._to_camel_case(table.name)
		lines.append(f'class {class_name}(Base):')
		
		# 테이블명
		if table.desc:
			lines.append(f'    # {table.desc}')
		lines.append(f'    __tablename__ = "{table.name}"')
		lines.append('')
		# 컬럼들
		for column in table.columns:
			col_lines = self._generate_column(column)
			lines.extend(col_lines)

		# 관계 설정
		relationships = self._generate_relationships(table)
		if relationships:
			lines.append('')
			lines.extend(relationships)
			
		# __repr__ 메서드
		lines.append('')
		lines.append('    def __repr__(self):')
		primary_key = next((col for col in table.columns if col.primary_key), None)
		if primary_key:
			lines.append(f'        return f"<{class_name}({primary_key.name}={{self.{primary_key.name}}})>"')
		else:
			lines.append(f'        return f"<{class_name}>"')
		
		return lines
	
	def _generate_column(self, column: Column) -> List[str]:
		"""컬럼을 생성합니다."""
		lines = []
		
		# 컬럼 정의 시작
		col_def = f'    {column.name} = Column('
		
		# 타입

		if column.type == ColumnType.STRING.value and column.length:
			col_def += f'String({column.length})'
		elif column.type == ColumnType.NUMERIC.value and column.precision:
			if column.scale:
				col_def += f'Numeric({column.precision}, {column.scale})'
			else:
				col_def += f'Numeric({column.precision})'
		else:
			col_def += column.type + '()'
		
		# 제약조건들
		constraints = []
		
		if column.primary_key:
			constraints.append('primary_key=True')
		
		if not column.nullable:
			constraints.append('nullable=False')
		
		if column.unique:
			constraints.append('unique=True')
		
		if column.auto_increment:
			constraints.append('autoincrement=True')
		
		if column.default:
			ch = column.default[0]
			if column.default.upper() in ['NULL', 'CURRENT_TIMESTAMP'] or (ch == '"' or ch == "'"):
				constraints.append(f"default={column.default}")
			else:
				constraints.append(f"default='{column.default}'")
		
		if constraints:
			col_def += ', ' + ', '.join(constraints)
		
		col_def += ')'
		
		# 주석
		print("@@ generate_column 1", column.comment)
		if column.comment:
			lines.append(f'    # {column.comment}')
		
		lines.append(col_def)
		return lines
	
	def _generate_relationships(self, table: Table) -> List[str]:
		"""관계를 생성합니다."""
		lines = []
		
		for fk in table.foreign_keys:
			ref_class = self._to_camel_case(fk['ref_table'])
			lines.append(f'    {fk["ref_table"]} = relationship("{ref_class}", back_populates="{table.name}")')
		
		return lines
	
	def _to_camel_case(self, snake_str: str) -> str:
		"""snake_case를 CamelCase로 변환합니다."""
		components = snake_str.split('_')
		return ''.join(word.capitalize() for word in components)

def main():
	"""메인 함수"""
	if len(sys.argv) < 2:
		print("Usage: python ddl_to_sqlalchemy.py <ddl_file> [output_file]")
		sys.exit(1)
	
	ddl_file = sys.argv[1]
	output_file = sys.argv[2] if len(sys.argv) > 2 else None
	
	try:
		# DDL 파일 읽기
		with open(ddl_file, 'r', encoding='utf-8') as f:
			ddl_content = f.read()
		
		# 파싱
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

if __name__ == "__main__":
	main() 