from html_to_markdown import HTMLToMarkdownConverter
import os
import io
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

md=HTMLToMarkdownConverter()

input_file = "c:/temp/doc-content.html"
if not os.path.exists(input_file):
	print(f"❌ 테스트 파일이 없습니다: {input_file}")
	exit()
try:	
	with open(input_file, 'r', encoding='utf-8') as f:
		content = f.read()
		md_content = md.convert(content)
		output_file = "c:/temp/doc-content.md"
		with open(output_file, 'w', encoding='utf-8') as f:
			f.write(md_content)

		print(f"✅ 변환 완료: {output_file}")
except Exception as e:
	print(f"❌ 변환 실패: {e}")