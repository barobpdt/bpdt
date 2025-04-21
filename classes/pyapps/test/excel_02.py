import os
import pandas as pd
import xlwings as xw
'''
# 엑셀 파일 불러오기
# D:/WORK/스타벅스/에너지관리
workbook = load_workbook("C:/temp/20240826-ZFIT2107.xlsx") 
# 비밀번호 해제
workbook.security = None
# 엑셀 파일 저장
workbook.save("c:/temp/aaa_unprotected.xlsx")
df = pd.read_excel("C:/temp/20240826-ZFIT2107.xlsx", engine='openpyxl')
print(df)


# 폴더 경로 지정
folder_path = './data'

# 병합할 모든 DataFrame을 담을 리스트
all_data = []

# 파일 리스트 가져오기
file_list = os.listdir(folder_path)

# 'opx.xls' 형식의 파일들을 필터링하여 데이터 병합
for file_name in file_list:
    if file_name.endswith('.xls'):
        file_path = os.path.join(folder_path, file_name)
        
        # Excel 파일 열기
        with xw.App(visible=False) as app:
            wb = app.books.open(file_path)
            sheet = wb.sheets[0]
            
            # 데이터를 DataFrame으로 변환
            df = sheet.used_range.options(pd.DataFrame, header=1, index=False).value
            all_data.append(df)
            wb.close()

# 모든 데이터프레임을 하나로 합침
merged_data = pd.concat(all_data, ignore_index=True)

# 병합된 데이터를 새로운 파일로 저장
output_file = os.path.join(folder_path, 'merged_data.xlsx')
merged_data.to_excel(output_file, index=False)
'''

all_data = []
wb = xw.Book("C:/temp/20240826-ZFIT2107.xlsx")
sheet = wb.sheets[0]
df = sheet.used_range.options(pd.DataFrame, header=1, index=False).value
all_data.append(df)
merged_data = pd.concat(all_data, ignore_index=True)
merged_data.to_excel("C:/temp/20240826-test.xlsx", index=False)

'''
with xw.App(visible=False) as app:
    wb = app.books.open("C:/temp/20240826-ZFIT2107.xlsx")
    sheet = wb.sheets[0]
    df = sheet.used_range.options(pd.DataFrame, header=1, index=False).value
    all_data.append(df)
merged_data = pd.concat(all_data, ignore_index=True)
merged_data.to_excel("C:/temp/20240826-test.xlsx", index=False)
'''
