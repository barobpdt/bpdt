#!/usr/bin/env python
# -*- coding: utf-8 -*-

import markdown
import sys
import os
import argparse
from markdown.extensions import fenced_code, tables, codehilite, toc, meta, nl2br, sane_lists, smarty

def markdown_to_html(markdown_text, output_file=None, extensions=None):
    """
    Markdown 텍스트를 HTML로 변환합니다.
    
    Args:
        markdown_text (str): 변환할 Markdown 텍스트
        output_file (str, optional): 출력 HTML 파일 경로
        extensions (list, optional): 사용할 Markdown 확장 기능 목록
        
    Returns:
        str: 변환된 HTML
    """
    # 기본 확장 기능 설정
    if extensions is None:
        extensions = [
            'fenced_code',      # 코드 블록 지원
            'tables',           # 테이블 지원
            'codehilite',       # 구문 강조
            'toc',              # 목차 생성
            'meta',             # 메타데이터 지원
            'nl2br',            # 줄바꿈을 <br>로 변환
            'sane_lists',       # 목록 처리 개선
            'smarty'            # 스마트 따옴표 및 대시 변환
        ]
    
    # Markdown을 HTML로 변환
    html = markdown.markdown(
        markdown_text,
        extensions=extensions,
        output_format='html5'
    )
    
    # HTML 템플릿 생성
    html_template = f"""<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Markdown to HTML</title>
    <style>
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }}
        pre {{
            background-color: #f5f5f5;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
        }}
        code {{
            font-family: Consolas, Monaco, 'Andale Mono', monospace;
            background-color: #f5f5f5;
            padding: 2px 4px;
            border-radius: 3px;
        }}
        table {{
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }}
        th, td {{
            border: 1px solid #ddd;
            padding: 8px;
            text-align: left;
        }}
        th {{
            background-color: #f2f2f2;
        }}
        img {{
            max-width: 100%;
            height: auto;
        }}
        blockquote {{
            border-left: 4px solid #ddd;
            padding-left: 15px;
            color: #666;
        }}
    </style>
</head>
<body>
{html}
</body>
</html>"""
    
    # 출력 파일이 지정된 경우 파일로 저장
    if output_file:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(html_template)
        return f"HTML이 '{output_file}' 파일에 저장되었습니다."
    
    return html_template

def main():
    parser = argparse.ArgumentParser(description='Markdown을 HTML로 변환합니다.')
    parser.add_argument('input', nargs='?', help='입력 Markdown 파일 경로')
    parser.add_argument('-o', '--output', help='출력 HTML 파일 경로')
    parser.add_argument('-e', '--extensions', nargs='+', help='사용할 Markdown 확장 기능 (쉼표로 구분)')
    args = parser.parse_args()
    
    # 입력 소스 결정
    if args.input:
        # 파일에서 Markdown 읽기
        if os.path.exists(args.input):
            with open(args.input, 'r', encoding='utf-8') as f:
                markdown_text = f.read()
        else:
            print(f"오류: 파일 '{args.input}'을(를) 찾을 수 없습니다.")
            return
    else:
        # 사용자에게 Markdown 입력 요청
        print("Markdown 텍스트를 입력하세요 (입력 완료 후 Enter를 누르고 Ctrl+D를 누르세요):")
        markdown_text = ""
        try:
            while True:
                line = input()
                markdown_text += line + "\n"
        except EOFError:
            pass
    
    # 확장 기능 설정
    extensions = None
    if args.extensions:
        extensions = args.extensions
    
    # Markdown을 HTML로 변환
    result = markdown_to_html(markdown_text, args.output, extensions)
    
    # 결과 출력
    if args.output:
        print(result)
    else:
        print(result)

if __name__ == "__main__":
    main() 