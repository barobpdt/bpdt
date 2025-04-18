import numpy as np
import yfinance as yf
import sys

sys.stdout.reconfigure(encoding='utf-8')

# yf.pdr_override()

# df = pdr.get_data_yahoo(주식 종목(한국주식의 경우 ".KS") [, 시작일(YYYY-mm-dd)] [,종료일(YYYY-mm-dd)])
'''
c.run('python -m pip install pandas_datareader')
c.run('python -m pip list')

echopro_bm = yf.download("247540.KQ", interval='1h', period='1y') # 1시간 간격으로 1년치 데이터 로드
echopro_bm.head(3)

네이버 증시동향
w=web('test')
w.setFinish(func(s) { fileWrite('c:/temp/web_result.html', s) })
w.setHeader('content-type', 'text/html;charset=EUC-KR')
w.callUrl('https://finance.naver.com/sise/sise_deposit.nhn')

cc=cmd('test')
cc.run('cd c:/apps/notePad')
cc.run('notepad++ "c:/temp/web_result.html"')

gen_otp_url = 'http://data.krx.co.kr/comm/fileDn/GenerateOTP/generate.cmd'
gen_otp_stk = {
    'mktId': 'STK',
    'trdDd': biz_day,
    'money': '1',
    'csvxls_isNo': 'false',
    'name': 'fileDown',
    'url': 'dbms/MDC/STAT/standard/MDCSTAT03901'
}
headers = {'Referer': 'http://data.krx.co.kr/contents/MDC/MDI/mdiLoader'}
otp_stk = rq.post(gen_otp_url, gen_otp_stk, headers=headers).text

'''

## data download
df = yf.download('005930.KS', start='2025-03-20', end='2025-04-09')
print(df)


## 종가 데이터만을 사용하여 데이터셋을 생성합니다.
data = df.filter(['Close'])
print(data)

데이터 정규화(Normalized)
50일씩 나누었던 데이터들을 첫날 기준으로 정규화 진행

(첫날을 0으로 만들고, 나머지 날들은 그 비율로 나누어 정규화)

norm_result = []
for section in day_result:
    norm_section = [((float(p) / float(section[0])) - 1) for p in section]
    norm_result.append(norm_section)
day_result = np.array(norm_result)


print('=========== 종가를 가져와 정규화 진행 ============')
close_prices = df['Close'].values
print(close_prices)

windown_size = 30
result_list = []
for i in range(len(close_prices) - (windown_size + 1)):
    result_list.append(close_prices[i: i+(windown_size+1)])

normal_data = []
for window in result_list:
    window_list = [((float(p) / float(window[0])) - 1) for p in window]
    normal_data.append(window_list)
    
result_list = np.array(normal_data)

print('=========== result_list ============')
print(result_list)
 


