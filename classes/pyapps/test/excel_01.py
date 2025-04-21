import io
import pandas as pd
import msoffcrypto

passwd = '123'

decrypted_workbook = io.BytesIO()
with open('c:/temp/aaa.XLSX', 'rb') as file:
    office_file = msoffcrypto.OfficeFile(file)
    office_file.load_key(password=passwd)
    office_file.decrypt(decrypted_workbook)

df = pd.read_excel(decrypted_workbook, sheet_name='abc')