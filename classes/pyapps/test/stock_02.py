import numpy as np
import pandas as pd
import yfinance as yf

df = yf.Ticker('AAPL').history(period='max', back_adjust=False, auto_adjust=False)
print(df)

##############################################################################
#### 수정종가 계산을 위한 함수
##############################################################################
def calculate_adjusted_prices_iterative(df, column):
    """ 
    column에 투입한 항목에 대해 수정가격(adjusted price)을 계산한다. 
    결과 값으로 adj_<칼럼명>을 칼럼명으로 하는 새로운 칼럼이 생성된다.
    데이터프레임에 배당금은 dividend, 분할비율은 split_ratio라는 이름으로 있어야 한다.
    """
    adj_column = 'adj_' + column
 
    # 0으로 채워진 빈 칼럼 생성
    df[adj_column] = 0
 
    # 가장 최근 날짜(기준일자)가 맨 위로 오도록 정렬
    df = df.sort_index(ascending=False)
    dates = list(df.index)
    base_date = dates[0]
 
    # 기준일자인 경우 주가 = 수정주가
    df.loc[base_date, adj_column] = df.loc[base_date, column]
 
    # 가장 최근 시점부터 과거 순으로 반복문 실행
    """
    시점 변수명   예시
    t    next    2022.05.11
    t-1  current 2022.05.10
    """
    for i, date in enumerate(dates):
        if i==0:
            next_date = base_date
        else:
            next_date = dates[dates.index(date) - 1]
 
        current_val = df.loc[date, column]
        next_val = df.loc[next_date, column]
        next_adj = df.loc[next_date, adj_column]
 
        split_ratio = df.loc[next_date, 'split_ratio']
        dividend = df.loc[next_date, 'dividend']
 
        adj_price = (next_adj+next_adj*(
                        ((current_val*split_ratio)-next_val-dividend)/next_val)
                    )
 
        df.loc[date, adj_column] = adj_price
 
    return df
    
##############################################################################
 
df['dividend'] = df.Dividends
df['split_ratio'] = 1 # yfinance의 Close는 이미 split이 적용된 값이다.
df_adj = calculate_adjusted_prices_iterative(df, 'Close')
df_adj = df_adj.assign(adj_close1 = round(df_adj['Adj Close'], 4),
                       adj_close2 = round(df_adj['adj_Close'], 4))
(df_adj['adj_close1']-df_adj['adj_close2']).plot()


