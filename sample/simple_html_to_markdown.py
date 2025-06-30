#!/usr/bin/env python3
"""
Simple HTML to Markdown Converter
간단한 HTML을 마크다운으로 변환하는 도구
"""

import re
from typing import Optional

def html_to_markdown(html_content: str) -> str:
    """
    HTML을 마크다운으로 변환하는 간단한 함수
    
    Args:
        html_content (str): 변환할 HTML 문자열
        
    Returns:
        str: 변환된 마크다운 문자열
    """
    if not html_content:
        return ""
    
    # HTML 엔티티 디코딩
    html_content = decode_html_entities(html_content)
    
    # 기본 변환 규칙들
    conversions = [
        # 헤딩
        (r'<h1[^>]*>(.*?)</h1>', r'# \1', re.DOTALL | re.IGNORECASE),
        (r'<h2[^>]*>(.*?)</h2>', r'## \1', re.DOTALL | re.IGNORECASE),
        (r'<h3[^>]*>(.*?)</h3>', r'### \1', re.DOTALL | re.IGNORECASE),
        (r'<h4[^>]*>(.*?)</h4>', r'#### \1', re.DOTALL | re.IGNORECASE),
        (r'<h5[^>]*>(.*?)</h5>', r'##### \1', re.DOTALL | re.IGNORECASE),
        (r'<h6[^>]*>(.*?)</h6>', r'###### \1', re.DOTALL | re.IGNORECASE),
        
        # 굵은 텍스트
        (r'<strong[^>]*>(.*?)</strong>', r'**\1**', re.DOTALL | re.IGNORECASE),
        (r'<b[^>]*>(.*?)</b>', r'**\1**', re.DOTALL | re.IGNORECASE),
        
        # 기울임 텍스트
        (r'<em[^>]*>(.*?)</em>', r'*\1*', re.DOTALL | re.IGNORECASE),
        (r'<i[^>]*>(.*?)</i>', r'*\1*', re.DOTALL | re.IGNORECASE),
        
        # 취소선
        (r'<del[^>]*>(.*?)</del>', r'~~\1~~', re.DOTALL | re.IGNORECASE),
        (r'<strike[^>]*>(.*?)</strike>', r'~~\1~~', re.DOTALL | re.IGNORECASE),
        (r'<s[^>]*>(.*?)</s>', r'~~\1~~', re.DOTALL | re.IGNORECASE),
        
        # 코드
        (r'<code[^>]*>(.*?)</code>', r'`\1`', re.DOTALL | re.IGNORECASE),
        
        # 링크
        (r'<a[^>]*href=["\']([^"\']*)["\'][^>]*>(.*?)</a>', r'[\2](\1)', re.DOTALL | re.IGNORECASE),
        
        # 이미지
        (r'<img[^>]*src=["\']([^"\']*)["\'][^>]*alt=["\']([^"\']*)["\'][^>]*>', r'![\2](\1)', re.IGNORECASE),
        (r'<img[^>]*src=["\']([^"\']*)["\'][^>]*>', r'![](\1)', re.IGNORECASE),
        
        # 줄바꿈
        (r'<br[^>]*>', r'\n', re.IGNORECASE),
        
        # 수평선
        (r'<hr[^>]*>', r'\n---\n', re.IGNORECASE),
        
        # 단락
        (r'<p[^>]*>(.*?)</p>', r'\n\1\n', re.DOTALL | re.IGNORECASE),
        
        # div
        (r'<div[^>]*>(.*?)</div>', r'\n\1\n', re.DOTALL | re.IGNORECASE),
        
        # span
        (r'<span[^>]*>(.*?)</span>', r'\1', re.DOTALL | re.IGNORECASE),
    ]
    
    # 변환 적용
    markdown_content = html_content
    for pattern, replacement, flags in conversions:
        markdown_content = re.sub(pattern, replacement, markdown_content, flags=flags)
    
    # 목록 처리
    markdown_content = convert_lists(markdown_content)
    
    # 인용문 처리
    markdown_content = convert_blockquotes(markdown_content)
    
    # 테이블 처리
    markdown_content = convert_tables(markdown_content)
    
    # 남은 HTML 태그 제거
    markdown_content = re.sub(r'<[^>]+>', '', markdown_content)
    
    # 연속된 공백 정리
    markdown_content = re.sub(r'\s+', ' ', markdown_content)
    
    # 연속된 줄바꿈 정리
    markdown_content = re.sub(r'\n\s*\n\s*\n', '\n\n', markdown_content)
    
    return markdown_content.strip()

def decode_html_entities(text: str) -> str:
    """HTML 엔티티를 디코딩"""
    entities = {
        '&amp;': '&',
        '&lt;': '<',
        '&gt;': '>',
        '&quot;': '"',
        '&#39;': "'",
        '&nbsp;': ' ',
        '&copy;': '©',
        '&reg;': '®',
        '&trade;': '™',
        '&hellip;': '...',
        '&mdash;': '—',
        '&ndash;': '–',
    }
    
    for entity, replacement in entities.items():
        text = text.replace(entity, replacement)
    
    # 숫자 엔티티 처리
    text = re.sub(r'&#(\d+);', lambda m: chr(int(m.group(1))), text)
    
    return text

def convert_lists(html_content: str) -> str:
    """목록을 마크다운으로 변환"""
    # 순서 없는 목록
    def convert_ul(match):
        content = match.group(1)
        items = re.findall(r'<li[^>]*>(.*?)</li>', content, re.DOTALL | re.IGNORECASE)
        markdown_items = []
        for item in items:
            clean_item = re.sub(r'<[^>]+>', '', item).strip()
            markdown_items.append(f'- {clean_item}')
        return '\n'.join(markdown_items) + '\n'
    
    # 순서 있는 목록
    def convert_ol(match):
        content = match.group(1)
        items = re.findall(r'<li[^>]*>(.*?)</li>', content, re.DOTALL | re.IGNORECASE)
        markdown_items = []
        for i, item in enumerate(items, 1):
            clean_item = re.sub(r'<[^>]+>', '', item).strip()
            markdown_items.append(f'{i}. {clean_item}')
        return '\n'.join(markdown_items) + '\n'
    
    # 변환 적용
    html_content = re.sub(r'<ul[^>]*>(.*?)</ul>', convert_ul, html_content, flags=re.DOTALL | re.IGNORECASE)
    html_content = re.sub(r'<ol[^>]*>(.*?)</ol>', convert_ol, html_content, flags=re.DOTALL | re.IGNORECASE)
    
    return html_content

def convert_blockquotes(html_content: str) -> str:
    """인용문을 마크다운으로 변환"""
    def convert_quote(match):
        content = match.group(1).strip()
        lines = content.split('\n')
        quoted_lines = [f'> {line}' for line in lines if line.strip()]
        return '\n'.join(quoted_lines) + '\n'
    
    return re.sub(r'<blockquote[^>]*>(.*?)</blockquote>', convert_quote, html_content, flags=re.DOTALL | re.IGNORECASE)

def convert_tables(html_content: str) -> str:
    """테이블을 마크다운으로 변환"""
    def convert_table(match):
        content = match.group(1)
        
        # tr 태그들 찾기
        rows = re.findall(r'<tr[^>]*>(.*?)</tr>', content, re.DOTALL | re.IGNORECASE)
        if not rows:
            return content
        
        markdown_rows = []
        
        for i, row in enumerate(rows):
            # th 또는 td 태그들 찾기
            cells = re.findall(r'<(?:th|td)[^>]*>(.*?)</(?:th|td)>', row, re.DOTALL | re.IGNORECASE)
            if cells:
                clean_cells = [re.sub(r'<[^>]+>', '', cell).strip() for cell in cells]
                markdown_row = '| ' + ' | '.join(clean_cells) + ' |'
                markdown_rows.append(markdown_row)
                
                # 첫 번째 행 다음에 구분선 추가
                if i == 0:
                    separator = '| ' + ' | '.join(['---'] * len(cells)) + ' |'
                    markdown_rows.append(separator)
        
        return '\n'.join(markdown_rows) + '\n'
    
    return re.sub(r'<table[^>]*>(.*?)</table>', convert_table, html_content, flags=re.DOTALL | re.IGNORECASE)

def convert_file(input_file: str, output_file: Optional[str] = None) -> str:
    """HTML 파일을 마크다운으로 변환"""
    try:
        # 입력 파일 읽기
        with open(input_file, 'r', encoding='utf-8') as f:
            html_content = f.read()
        
        # 변환
        markdown_content = html_to_markdown(html_content)
        
        # 출력 파일 결정
        if output_file is None:
            output_file = input_file.rsplit('.', 1)[0] + '.md'
        
        # 출력 파일 저장
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(markdown_content)
        
        return output_file
        
    except FileNotFoundError:
        raise FileNotFoundError(f"입력 파일을 찾을 수 없습니다: {input_file}")
    except Exception as e:
        raise Exception(f"파일 변환 중 오류 발생: {e}")

# 사용 예시
if __name__ == "__main__":
    # 테스트 HTML
    test_html = """
    <html>
    <head><title>테스트 페이지</title></head>
    <body>
        <h1>제목 1</h1>
        <h2>제목 2</h2>
        <p>이것은 <strong>굵은 텍스트</strong>와 <em>기울임 텍스트</em>가 포함된 단락입니다.</p>
        <ul>
            <li>목록 항목 1</li>
            <li>목록 항목 2</li>
        </ul>
        <blockquote>이것은 인용문입니다.</blockquote>
        <a href="https://example.com">링크</a>
    </body>
    </html>
    """
    
    # 변환
    markdown_result = html_to_markdown(test_html)
    print("변환 결과:")
    print(markdown_result) 