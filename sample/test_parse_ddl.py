#!/usr/bin/env python3
"""
parse_ddl.py primary key 파싱 테스트
"""

from parse_ddl import DDLParser, SQLAlchemyGenerator

# 테스트용 DDL
test_ddl = """sg_memb<sep>CREATE TABLE `sg_memb` (
  `MEMB_NO` varchar(20) NOT NULL,
  `DEVI_ID` varchar(256) DEFAULT NULL,
  `DEVI_KEY` varchar(256) DEFAULT NULL,
  `DEVI_TP_CD` varchar(10) DEFAULT NULL,
  `DEVI_OS_VER` varchar(128) DEFAULT NULL,
  `APP_REG` varchar(250) DEFAULT NULL,
  `APP_VER` varchar(10) DEFAULT NULL,
  `F_MEMB_NM` varchar(40) DEFAULT NULL,
  `L_MEMB_NM` varchar(40) DEFAULT NULL,
  `EMAIL` varchar(128) DEFAULT NULL,
  `PHONE_NO` varchar(15) DEFAULT NULL,
  `COUNTRY_NM` varchar(20) DEFAULT NULL,
  `BIR_DT` varchar(8) DEFAULT NULL,
  `GEN_CD` varchar(10) DEFAULT NULL,
  `NOTI_RCV_YN` varchar(1) DEFAULT NULL,
  `MEMB_STAT_CD` varchar(10) DEFAULT NULL,
  `STAT_DT` datetime DEFAULT NULL,
  `MEMB_JOIN_DT` datetime DEFAULT NULL,
  `FIRST_DAY` datetime DEFAULT NULL,
  `LAST_DAY` datetime DEFAULT NULL,
  `MEMB_EXIT_DT` datetime DEFAULT NULL,
  `AD_OUT_YN` varchar(2) DEFAULT 'Y',
  `NICK_NM` varchar(40) DEFAULT NULL,
  `INTRODUCTION_INFO` varchar(200) DEFAULT NULL,
  `IMG_FILE_PATH` varchar(200) DEFAULT NULL,
  `IMG_FILE_URL` varchar(200) DEFAULT NULL,
  `ADD_DT` datetime DEFAULT NULL,
  `ADD_ID` varchar(20) DEFAULT NULL,
  `CHG_DT` datetime DEFAULT NULL,
  `CHG_ID` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`MEMB_NO`)
)<end>회원 정보 테이블

sg_memb_qr<sep>CREATE TABLE `sg_memb_qr` (
  `MEMB_NO` varchar(20) NOT NULL,
  `QR_NO` varchar(20) NOT NULL,
  `QR_OWNER_CD` varchar(20) DEFAULT NULL,
  `QR_USE_CD` varchar(20) DEFAULT NULL,
  `NICK_NM` varchar(40) DEFAULT NULL,
  `CAR_INFO` varchar(40) DEFAULT NULL,
  `MAKER_CAR` varchar(40) DEFAULT NULL,
  `SUB_SCR` varchar(200) DEFAULT NULL,
  `PARK_TEXT` varchar(50) DEFAULT '잠시',
  `PIN_NO` varchar(10) DEFAULT NULL,
  `INTRODUCTION_INFO` varchar(200) DEFAULT NULL,
  `SERVICE_TYPE_CD` varchar(20) DEFAULT NULL,
  `ADD_DT` datetime DEFAULT NULL,
  `ADD_ID` varchar(20) DEFAULT NULL,
  `CHG_DT` datetime DEFAULT NULL,
  `CHG_ID` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`MEMB_NO`,`QR_NO`)
)<end>회원 QR 정보 테이블"""

def test_primary_key_parsing():
    """Primary key 파싱 테스트"""
    print("🔍 Primary Key 파싱 테스트 시작")
    
    # DDL 파싱
    parser = DDLParser()
    tables = parser.parse_ddl(test_ddl)
    
    print(f"\n📊 파싱된 테이블 수: {len(tables)}")
    
    for table in tables:
        print(f"\n📋 테이블: {table.name}")
        print(f"📝 설명: {table.desc}")
        print(f"🔑 Primary Keys: {table.primary_keys}")
        print(f"📊 컬럼 수: {len(table.columns)}")
        
        # Primary key 컬럼들 확인
        pk_columns = [col for col in table.columns if col.primary_key]
        print(f"✅ Primary Key 컬럼들: {[col.name for col in pk_columns]}")
        
        # 모든 컬럼의 primary_key 속성 확인
        for col in table.columns:
            if col.primary_key:
                print(f"  🔑 {col.name}: primary_key=True")
            else:
                print(f"  📝 {col.name}: primary_key=False")
    
    # SQLAlchemy 모델 생성 테스트
    print(f"\n🔧 SQLAlchemy 모델 생성 테스트")
    generator = SQLAlchemyGenerator()
    code = generator.generate_models(tables)
    
    print("✅ 모델 생성 완료!")
    return tables

if __name__ == "__main__":
    test_primary_key_parsing() 