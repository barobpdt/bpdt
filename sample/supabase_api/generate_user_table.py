#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
User 모델을 기반으로 Supabase 테이블 SQL 생성 스크립트
"""

import sys
import os
from typing import Dict, Any, Type
from pydantic import BaseModel, EmailStr
from datetime import datetime
from typing import Optional

# 현재 디렉토리를 Python 경로에 추가
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

# User 모델 정의 (fastapi_supabase.py에서 가져옴)
class UserBase(BaseModel):
    name: str
    email: EmailStr
    age: Optional[int] = None
    city: Optional[str] = None

class UserCreate(UserBase):
    pass

class UserUpdate(BaseModel):
    name: Optional[str] = None
    email: Optional[EmailStr] = None
    age: Optional[int] = None
    city: Optional[str] = None

class User(UserBase):
    id: int
    created_at: datetime
    
    class Config:
        from_attributes = True

# Pydantic 모델을 SQL 스키마로 변환하는 클래스
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
                
                # 기본값 설정
                if field.default is not None:
                    column['default'] = field.default
                elif field.default_factory is not None:
                    column['default'] = field.default_factory()
                
                # 필수 필드 확인
                if field.required:
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
        
        return sql

def main():
    """메인 함수"""
    print("=" * 60)
    print("User 모델 기반 Supabase 테이블 SQL 생성")
    print("=" * 60)
    
    # User 모델 스키마 생성
    print("\n1. User 모델 분석")
    print("-" * 30)
    
    schema = TableSchemaGenerator.generate_table_schema(User, "users")
    
    print(f"테이블명: {schema['table_name']}")
    print(f"컬럼 수: {len(schema['columns'])}")
    print(f"제약 조건 수: {len(schema['constraints'])}")
    
    print("\n컬럼 정보:")
    for col in schema['columns']:
        nullable = "NULL" if col['nullable'] else "NOT NULL"
        unique = "UNIQUE" if col['unique'] else ""
        pk = "PRIMARY KEY" if col['primary_key'] else ""
        default = f"DEFAULT {col['default']}" if col['default'] else ""
        
        constraints = [c for c in [nullable, unique, pk, default] if c]
        print(f"  - {col['name']}: {col['type']} ({', '.join(constraints)})")
    
    print("\n제약 조건:")
    for constraint in schema['constraints']:
        print(f"  - {constraint['type'].upper()}: {', '.join(constraint['columns'])}")
    
    # SQL DDL 생성
    print("\n2. SQL DDL 생성")
    print("-" * 30)
    
    sql_ddl = TableSchemaGenerator.generate_sql_ddl(schema)
    
    print("생성된 SQL:")
    print("-" * 20)
    print(sql_ddl)
    print("-" * 20)
    
    # 파일로 저장
    print("\n3. SQL 파일 저장")
    print("-" * 30)
    
    sql_file = "create_users_table.sql"
    with open(sql_file, 'w', encoding='utf-8') as f:
        f.write(sql_ddl)
    
    print(f"SQL이 '{sql_file}' 파일로 저장되었습니다.")
    
    # 사용 방법 안내
    print("\n4. 사용 방법")
    print("-" * 30)
    print("1. Supabase 대시보드에 로그인하세요.")
    print("2. SQL 편집기로 이동하세요.")
    print("3. 위의 SQL을 복사하여 실행하세요.")
    print("4. 또는 생성된 SQL 파일을 업로드하세요.")
    
    print("\n" + "=" * 60)
    print("SQL 생성 완료!")
    print("=" * 60)

if __name__ == "__main__":
    main() 