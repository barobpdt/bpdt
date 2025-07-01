#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Supabase 데이터베이스 관련 기능
"""

from typing import Dict, Any, Type
from pydantic import BaseModel
from supabase import Client
from log import logger
from dataModel import User

class TableSchemaGenerator:
	"""Pydantic 모델을 기반으로 Supabase 테이블 스키마를 생성하는 클래스"""
	
	# Python 타입을 PostgreSQL 타입으로 매핑
	TYPE_MAPPING = {
		'str': 'text',
		'int': 'integer',
		'float': 'numeric',
		'bool': 'boolean',
		'datetime': 'timestamptz',
		'date': 'date',
		'time': 'time',
		'list': 'jsonb',
		'dict': 'jsonb',
		'EmailStr': 'text',
		'Optional': 'text',  # Optional은 기본 타입으로 처리
	}
	
	@classmethod
	def get_field_type(cls, field_type: Any) -> str:
		"""필드 타입을 PostgreSQL 타입으로 변환"""
		type_name = str(field_type)
		
		# Optional 타입 처리
		if 'Optional' in type_name:
			# Optional[Type]에서 Type 추출
			if '[' in type_name and ']' in type_name:
				inner_type = type_name.split('[')[1].split(']')[0]
				return cls.get_field_type(inner_type)
			return 'text'
		
		# Union 타입 처리 (예: Union[str, None])
		if 'Union' in type_name:
			# Union[str, None]에서 str 추출
			if '[' in type_name and ']' in type_name:
				types = type_name.split('[')[1].split(']')[0].split(',')
				# None이 아닌 첫 번째 타입 사용
				for t in types:
					t = t.strip()
					if t != 'NoneType' and t != 'None':
						return cls.get_field_type(t)
			return 'text'
		
		# 기본 타입 매핑
		for python_type, pg_type in cls.TYPE_MAPPING.items():
			if python_type in type_name:
				return pg_type
		
		# 기본값
		return 'text'
	
	@classmethod
	def generate_table_schema(cls, model_class: Type[BaseModel], table_name: str = None) -> Dict[str, Any]:
		"""Pydantic 모델을 기반으로 테이블 스키마 생성"""
		if table_name is None:
			table_name = model_class.__name__.lower()
		
		schema = {
			'table_name': table_name,
			'columns': [],
			'constraints': [],
			'indexes': []
		}
		
		# 모델 필드 분석
		for field_name, field_info in model_class.__annotations__.items():
			column = {
				'name': field_name,
				'type': cls.get_field_type(field_info),
				'nullable': True,
				'default': None,
				'unique': False,
				'primary_key': False
			}
			
			# 필드 정보 가져오기
			if hasattr(model_class, '__fields__') and field_name in model_class.__fields__:
				field = model_class.__fields__[field_name]
				# field => anotation, required, default ...
				print("xxx field xxx", field)
				# 기본값 설정
				if field.default is not None:
					column['default'] = field.default
				elif field.default_factory is not None:
					column['default'] = field.default_factory()
				
				# 필수 필드 확인
				if field.is_required():
					column['nullable'] = False
				
				# 이메일 필드는 unique로 설정
				if field_name == 'email' or 'EmailStr' in str(field_info):
					column['unique'] = True
				
				# id 필드는 primary key로 설정
				if field_name == 'id':
					column['primary_key'] = True
					column['nullable'] = False
					column['type'] = 'bigint'
					column['default'] = 'gen_random_uuid()' if column['type'] == 'uuid' else 'identity'
				
				# created_at 필드는 기본값 설정
				if field_name == 'created_at':
					column['default'] = 'now()'
					column['nullable'] = False
			
			schema['columns'].append(column)
		
		# 제약 조건 추가
		for column in schema['columns']:
			if column['primary_key']:
				schema['constraints'].append({
					'type': 'primary_key',
					'columns': [column['name']]
				})
			
			if column['unique']:
				schema['constraints'].append({
					'type': 'unique',
					'columns': [column['name']]
				})
		print("@@ generate_table_schema : ", schema)
		return schema
	
	@classmethod
	def generate_sql_ddl(cls, schema: Dict[str, Any]) -> str:
		"""스키마를 SQL DDL로 변환"""
		table_name = schema['table_name']
		
		# 컬럼 정의 생성
		column_definitions = []
		for column in schema['columns']:
			col_def = f"{column['name']} {column['type']}"
			
			if not column['nullable']:
				col_def += " NOT NULL"
			
			if column['default'] is not None:
				if isinstance(column['default'], str) and column['default'] != 'now()' and column['default'] != 'identity':
					col_def += f" DEFAULT '{column['default']}'"
				else:
					col_def += f" DEFAULT {column['default']}"
			
			column_definitions.append(col_def)
		
		# 제약 조건 추가
		for constraint in schema['constraints']:
			if constraint['type'] == 'primary_key':
				column_definitions.append(f"PRIMARY KEY ({', '.join(constraint['columns'])})")
			elif constraint['type'] == 'unique':
				column_definitions.append(f"UNIQUE ({', '.join(constraint['columns'])})")
		
		# SQL 생성
		sql = f"""CREATE TABLE IF NOT EXISTS {table_name} (
	{',\n    '.join(column_definitions)}
);"""
		print(f"@@ sql: {sql}")
		return sql
	
	@classmethod
	def create_table_from_model(cls, model_class: Type[BaseModel], table_name: str = None) -> bool:
		"""Pydantic 모델을 기반으로 Supabase 테이블 생성"""
		try:
			# 스키마 생성
			schema = cls.generate_table_schema(model_class, table_name)
			
			# SQL DDL 생성
			sql_ddl = cls.generate_sql_ddl(schema)
			
			logger.info(f"테이블 생성 SQL:")
			logger.info(sql_ddl)
			
			# Supabase에서 테이블 생성 (실제로는 SQL 실행이 필요)
			# Supabase Python 클라이언트는 직접적인 DDL 실행을 지원하지 않으므로
			# 대시보드에서 수동으로 생성하거나 RPC를 사용해야 합니다
			
			logger.info(f"테이블 '{schema['table_name']}' 스키마가 생성되었습니다.")
			logger.info("Supabase 대시보드에서 다음 SQL을 실행하세요:")
			logger.info(sql_ddl)
			
			return True
			
		except Exception as e:
			logger.error(f"테이블 생성 실패: {e}")
			return False

def init_database(supabase: Client):
	"""데이터베이스 테이블이 없으면 생성"""
	try:
		# users 테이블 생성 (실제로는 Supabase 대시보드에서 생성하는 것이 좋습니다)
		# 여기서는 테이블이 이미 존재한다고 가정합니다
		logger.info("데이터베이스 연결 확인 중...")
		result = supabase.table("users").select("*").limit(1).execute()
		logger.info("데이터베이스 연결 성공!")
	except Exception as e:
		logger.error(f"데이터베이스 연결 오류: {e}")
		logger.error("Supabase 대시보드에서 'users' 테이블을 생성해주세요.")
		logger.error("테이블 스키마:")
		logger.error("- id: int8 (primary key, auto increment)")
		logger.error("- name: text (not null)")
		logger.error("- email: text (unique, not null)")
		logger.error("- age: int4")
		logger.error("- city: text")
		logger.error("- created_at: timestamptz (default: now())")

def init_database_with_schema(supabase: Client, User):
	"""Pydantic 모델을 기반으로 데이터베이스 테이블 생성"""
	try:
		logger.info("Pydantic 모델을 기반으로 테이블 스키마 생성 중...")
		
		# User 모델을 기반으로 테이블 생성
		success = TableSchemaGenerator.create_table_from_model(User, "users")
		
		if success:
			logger.info("테이블 스키마 생성 완료!")
			logger.info("Supabase 대시보드에서 제공된 SQL을 실행하여 테이블을 생성하세요.")
		else:
			logger.error("테이블 스키마 생성 실패!")
			
	except Exception as e:
		logger.error(f"데이터베이스 초기화 오류: {e}")






