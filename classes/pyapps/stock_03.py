import argparse
import os
import time

import pandas as pd
import requests
import json
from datetime import datetime

class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))
# 인자값을 받을 수 있는 인스턴스 생성
parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')

# 입력받을 인자값 등록
parser.add_argument('--log', action=CustomAction, nargs='+', required=True, help='로그파일')
parser.add_argument('--output', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()

fin=open(args.log, 'r', encoding='utf8')
fout=open(args.output, 'a', encoding='utf8')
lastPos=fin.seek(0, os.SEEK_END)
tm=time.time()

def log (msg):
    fout.write(f"##> {msg}\n")
    fout.flush()

def get_kospi_listed_companies():
    """
    KRX에서 코스피 상장회사 정보를 가져오는 함수
    """
    # KRX 요청 URL
    url = "http://data.krx.co.kr/comm/bldAttendant/getJsonData.cmd"
    
    # 요청 파라미터 - 최신 KRX API 구조에 맞게 수정
    params = {
        "bld": "dbms/MDC/STAT/standard/MDCSTAT03901",
        "locale": "ko_KR",
        "mktId": "STK",  # 주식시장
        "trdDd": datetime.now().strftime("%Y%m%d"),  # 오늘 날짜
        "money": "1",  # 원화
        "csvxls_isNo": "false",  # JSON 형식으로 요청
    }
    
    # 요청 헤더
    headers = {
        "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Accept": "application/json, text/javascript, */*; q=0.01",
        "Accept-Language": "ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7",
        "Origin": "http://data.krx.co.kr",
        "Referer": "http://data.krx.co.kr/contents/MDC/MDCSTAT03901/index.jsp",
    }
    
    try:
        # 데이터 요청
        response = requests.post(url, data=params, headers=headers)
        response.raise_for_status()  # 오류 발생 시 예외 발생
        
        # JSON 데이터 파싱
        data = json.loads(response.text)
        
        # 응답 구조 확인 및 디버깅
        log("응답 데이터 구조:" + json.dumps(data, indent=2)[:10] + "...")
        
        # 데이터프레임으로 변환
        if 'OutBlock_1' in data:
            df = pd.DataFrame(data['OutBlock_1'])
            
            # 컬럼명 한글로 변경
            column_mapping = {
                'ISU_ABBRV': '종목명',
                'ISU_SHRT_CD': '종목코드',
                'ISU_NM': '종목전체명',
                'MKT_NM': '시장구분',
                'SEC_NM': '섹터',
                'TDD_CLSPRC': '종가',
                'CMPPREVDD_PRC': '전일대비',
                'FLUC_RT': '등락률',
                'TDD_OPNPRC': '시가',
                'TDD_HGPRC': '고가',
                'TDD_LWPRC': '저가',
                'ACC_TRDVOL': '거래량',
                'ACC_TRDVOL_VAL': '거래대금',
                'MKTCAP': '시가총액',
                'LIST_SHRS': '상장주식수',
            }
            
            # 실제 응답에 있는 컬럼만 매핑
            available_columns = {k: v for k, v in column_mapping.items() if k in df.columns}
            df = df.rename(columns=available_columns)
            
            # 숫자형 데이터 변환
            numeric_columns = ['종가', '전일대비', '등락률', '시가', '고가', '저가', '거래량', '거래대금', '시가총액', '상장주식수']
            for col in numeric_columns:
                if col in df.columns:
                    df[col] = pd.to_numeric(df[col].astype(str).str.replace(',', ''), errors='coerce')
            
            return df
        else:
            log("응답 데이터에 'OutBlock_1' 키가 없습니다.")
            log("전체 응답 데이터:" + json.dumps(data, indent=2))
            return None
    
    except Exception as e:
        log(f"데이터를 가져오는 중 오류가 발생했습니다: {e}")
        return None

def save_to_excel(df, filename="kospi_companies.xlsx"):
    """
    데이터프레임을 엑셀 파일로 저장하는 함수
    """
    try:
        # 엑셀 파일로 저장
        df.to_excel(filename, index=False, engine='openpyxl')
        log(f"데이터가 성공적으로 {filename}에 저장되었습니다.")
    except Exception as e:
        log(f"엑셀 파일 저장 중 오류가 발생했습니다: {e}")

def main():
    # 코스피 상장회사 정보 가져오기
    log("코스피 상장회사 정보를 가져오는 중...")
    df = get_kospi_listed_companies()
    
    if df is not None:
        # 엑셀 파일로 저장
        save_to_excel(df)
    else:
        log("데이터를 가져오지 못했습니다.")

if __name__ == "__main__":
    main()

fout.close()
fin.close()	
