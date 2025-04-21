from openpyxl import load_workbook

# 엑셀 파일 불러오기
workbook = load_workbook("c:/temp/aaa.xlsx", read_only=False)

# 비밀번호 해제
workbook.security = None

# 엑셀 파일 저장
workbook.save("c:/temp/aaa_unprotected.xlsx")