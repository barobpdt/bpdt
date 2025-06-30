#!/usr/bin/env python3
"""
HTML to Markdown Converter
HTML 파일이나 HTML 문자열을 마크다운 형식으로 변환하는 도구
"""

import re
import sys
import os
from pathlib import Path
from typing import Optional, List
import argparse
from urllib.parse import urlparse, unquote

class HTMLToMarkdownConverter:
    """HTML을 마크다운으로 변환하는 클래스"""
    
    def __init__(self):
        # HTML 태그를 마크다운으로 변환하는 규칙들
        self.conversion_rules = [
            # 헤딩 (h1-h6)
            (r'<h1[^>]*>(.*?)</h1>', r'# \1', re.DOTALL | re.IGNORECASE),
            (r'<h2[^>]*>(.*?)</h2>', r'## \1', re.DOTALL | re.IGNORECASE),
            (r'<h3[^>]*>(.*?)</h3>', r'### \1', re.DOTALL | re.IGNORECASE),
            (r'<h4[^>]*>(.*?)</h4>', r'#### \1', re.DOTALL | re.IGNORECASE),
            (r'<h5[^>]*>(.*?)</h5>', r'##### \1', re.DOTALL | re.IGNORECASE),
            (r'<h6[^>]*>(.*?)</h6>', r'###### \1', re.DOTALL | re.IGNORECASE),
            
            # 굵은 텍스트 (strong, b)
            (r'<strong[^>]*>(.*?)</strong>', r'**\1**', re.DOTALL | re.IGNORECASE),
            (r'<b[^>]*>(.*?)</b>', r'**\1**', re.DOTALL | re.IGNORECASE),
            
            # 기울임 텍스트 (em, i)
            (r'<em[^>]*>(.*?)</em>', r'*\1*', re.DOTALL | re.IGNORECASE),
            (r'<i[^>]*>(.*?)</i>', r'*\1*', re.DOTALL | re.IGNORECASE),
            
            # 취소선 (del, strike, s)
            (r'<del[^>]*>(.*?)</del>', r'~~\1~~', re.DOTALL | re.IGNORECASE),
            (r'<strike[^>]*>(.*?)</strike>', r'~~\1~~', re.DOTALL | re.IGNORECASE),
            (r'<s[^>]*>(.*?)</s>', r'~~\1~~', re.DOTALL | re.IGNORECASE),
            
            # 코드 (code)
            (r'<code[^>]*>(.*?)</code>', r'`\1`', re.DOTALL | re.IGNORECASE),
            
            # 링크 (a)
            (r'<a[^>]*href=["\']([^"\']*)["\'][^>]*>(.*?)</a>', r'[\2](\1)', re.DOTALL | re.IGNORECASE),
            
            # 이미지 (img)
            (r'<img[^>]*src=["\']([^"\']*)["\'][^>]*alt=["\']([^"\']*)["\'][^>]*>', r'![\2](\1)', re.IGNORECASE),
            (r'<img[^>]*alt=["\']([^"\']*)["\'][^>]*src=["\']([^"\']*)["\'][^>]*>', r'![\1](\2)', re.IGNORECASE),
            (r'<img[^>]*src=["\']([^"\']*)["\'][^>]*>', r'![](\1)', re.IGNORECASE),
            
            # 줄바꿈 (br)
            (r'<br[^>]*>', r'\n', re.IGNORECASE),
            
            # 수평선 (hr)
            (r'<hr[^>]*>', r'\n---\n', re.IGNORECASE),
            
            # 인용문 (blockquote)
            (r'<blockquote[^>]*>(.*?)</blockquote>', self._convert_blockquote, re.DOTALL | re.IGNORECASE),
            
            # 목록 (ul, ol)
            (r'<ul[^>]*>(.*?)</ul>', self._convert_unordered_list, re.DOTALL | re.IGNORECASE),
            (r'<ol[^>]*>(.*?)</ol>', self._convert_ordered_list, re.DOTALL | re.IGNORECASE),
            
            # 테이블 (table)
            (r'<table[^>]*>(.*?)</table>', self._convert_table, re.DOTALL | re.IGNORECASE),
            
            # 단락 (p)
            (r'<p[^>]*>(.*?)</p>', r'\n\1\n', re.DOTALL | re.IGNORECASE),
            
            # div (간단한 줄바꿈으로 처리)
            (r'<div[^>]*>(.*?)</div>', r'\n\1\n', re.DOTALL | re.IGNORECASE),
            
            # span (내용만 추출)
            (r'<span[^>]*>(.*?)</span>', r'\1', re.DOTALL | re.IGNORECASE),
        ]
        
        # 제거할 태그들
        self.remove_tags = [
            'script', 'style', 'head', 'title', 'meta', 'link', 'html', 'body'
        ]
    
    def _convert_blockquote(self, match):
        """인용문을 마크다운으로 변환"""
        content = match.group(1).strip()
        lines = content.split('\n')
        quoted_lines = [f'> {line}' for line in lines if line.strip()]
        return '\n'.join(quoted_lines) + '\n'
    
    def _convert_unordered_list(self, match):
        """순서 없는 목록을 마크다운으로 변환"""
        content = match.group(1)
        # li 태그들을 찾아서 변환
        items = re.findall(r'<li[^>]*>(.*?)</li>', content, re.DOTALL | re.IGNORECASE)
        markdown_items = []
        for item in items:
            clean_item = self._clean_html(item.strip())
            markdown_items.append(f'- {clean_item}')
        return '\n'.join(markdown_items) + '\n'
    
    def _convert_ordered_list(self, match):
        """순서 있는 목록을 마크다운으로 변환"""
        content = match.group(1)
        # li 태그들을 찾아서 변환
        items = re.findall(r'<li[^>]*>(.*?)</li>', content, re.DOTALL | re.IGNORECASE)
        markdown_items = []
        for i, item in enumerate(items, 1):
            clean_item = self._clean_html(item.strip())
            markdown_items.append(f'{i}. {clean_item}')
        return '\n'.join(markdown_items) + '\n'
    
    def _convert_table(self, match):
        """테이블을 마크다운으로 변환"""
        content = match.group(1)
        
        # thead와 tbody 처리
        thead_match = re.search(r'<thead[^>]*>(.*?)</thead>', content, re.DOTALL | re.IGNORECASE)
        tbody_match = re.search(r'<tbody[^>]*>(.*?)</tbody>', content, re.DOTALL | re.IGNORECASE)
        
        if thead_match:
            header_content = thead_match.group(1)
        else:
            # thead가 없으면 첫 번째 tr을 헤더로 사용
            first_tr = re.search(r'<tr[^>]*>(.*?)</tr>', content, re.DOTALL | re.IGNORECASE)
            header_content = first_tr.group(1) if first_tr else ""
        
        # 헤더 행 처리
        header_cells = re.findall(r'<th[^>]*>(.*?)</th>', header_content, re.DOTALL | re.IGNORECASE)
        if not header_cells:
            # th가 없으면 td를 사용
            header_cells = re.findall(r'<td[^>]*>(.*?)</td>', header_content, re.DOTALL | re.IGNORECASE)
        
        if not header_cells:
            return content  # 테이블 구조를 파악할 수 없으면 원본 반환
        
        # 헤더 행 생성
        header_row = '| ' + ' | '.join([self._clean_html(cell.strip()) for cell in header_cells]) + ' |'
        separator_row = '| ' + ' | '.join(['---'] * len(header_cells)) + ' |'
        
        # 데이터 행 처리
        data_rows = []
        if tbody_match:
            body_content = tbody_match.group(1)
        else:
            body_content = content
        
        tr_matches = re.findall(r'<tr[^>]*>(.*?)</tr>', body_content, re.DOTALL | re.IGNORECASE)
        
        for tr in tr_matches:
            td_cells = re.findall(r'<td[^>]*>(.*?)</td>', tr, re.DOTALL | re.IGNORECASE)
            if td_cells:
                row = '| ' + ' | '.join([self._clean_html(cell.strip()) for cell in td_cells]) + ' |'
                data_rows.append(row)
        
        return '\n'.join([header_row, separator_row] + data_rows) + '\n'
    
    def _clean_html(self, html_content):
        """HTML 태그를 제거하고 텍스트만 추출"""
        # HTML 엔티티 디코딩
        html_content = self._decode_html_entities(html_content)
        
        # 모든 HTML 태그 제거
        clean_text = re.sub(r'<[^>]+>', '', html_content)
        
        # 연속된 공백 정리
        clean_text = re.sub(r'\s+', ' ', clean_text)
        
        return clean_text.strip()
    
    def _decode_html_entities(self, text):
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
            '&lsquo;': ''',
            '&rsquo;': ''',
            '&ldquo;': '"',
            '&rdquo;': '"',
        }
        
        for entity, replacement in entities.items():
            text = text.replace(entity, replacement)
        
        # 숫자 엔티티 처리 (예: &#160;)
        text = re.sub(r'&#(\d+);', lambda m: chr(int(m.group(1))), text)
        
        # 16진수 엔티티 처리 (예: &#x20;)
        text = re.sub(r'&#x([0-9a-fA-F]+);', lambda m: chr(int(m.group(1), 16)), text)
        
        return text
    
    def _remove_unwanted_tags(self, html_content):
        """불필요한 태그들을 제거"""
        for tag in self.remove_tags:
            # 시작 태그와 끝 태그 모두 제거
            html_content = re.sub(rf'<{tag}[^>]*>.*?</{tag}>', '', html_content, flags=re.DOTALL | re.IGNORECASE)
            # 자체 닫는 태그도 제거
            html_content = re.sub(rf'<{tag}[^>]*/>', '', html_content, flags=re.IGNORECASE)
        
        return html_content
    
    def convert(self, html_content: str) -> str:
        """HTML을 마크다운으로 변환"""
        if not html_content:
            return ""
        
        # HTML 정규화
        html_content = html_content.strip()
        
        # 불필요한 태그들 제거
        html_content = self._remove_unwanted_tags(html_content)
        
        # 변환 규칙 적용
        markdown_content = html_content
        for pattern, replacement, flags in self.conversion_rules:
            if callable(replacement):
                markdown_content = re.sub(pattern, replacement, markdown_content, flags=flags)
            else:
                markdown_content = re.sub(pattern, replacement, markdown_content, flags=flags)
        
        # 남은 HTML 태그들 제거
        markdown_content = self._clean_html(markdown_content)
        
        # 연속된 줄바꿈 정리
        markdown_content = re.sub(r'\n\s*\n\s*\n', '\n\n', markdown_content)
        
        # 앞뒤 공백 제거
        markdown_content = markdown_content.strip()
        
        return markdown_content
    
    def convert_file(self, input_file: str, output_file: Optional[str] = None) -> str:
        """HTML 파일을 마크다운으로 변환"""
        try:
            # 입력 파일 읽기
            with open(input_file, 'r', encoding='utf-8') as f:
                html_content = f.read()
            
            # 변환
            markdown_content = self.convert(html_content)
            
            # 출력 파일 결정
            if output_file is None:
                input_path = Path(input_file)
                output_file = input_path.with_suffix('.md')
            
            # 출력 파일 저장
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(markdown_content)
            
            return str(output_file)
            
        except FileNotFoundError:
            raise FileNotFoundError(f"입력 파일을 찾을 수 없습니다: {input_file}")
        except Exception as e:
            raise Exception(f"파일 변환 중 오류 발생: {e}")

def main():
    """메인 함수"""
    parser = argparse.ArgumentParser(description='HTML을 마크다운으로 변환')
    parser.add_argument('input', help='입력 HTML 파일 또는 HTML 문자열')
    parser.add_argument('-o', '--output', help='출력 마크다운 파일 (기본값: 입력파일명.md)')
    parser.add_argument('-s', '--string', action='store_true', help='입력을 HTML 문자열로 처리')
    
    args = parser.parse_args()
    
    converter = HTMLToMarkdownConverter()
    
    try:
        if args.string:
            # HTML 문자열을 직접 변환
            markdown_content = converter.convert(args.input)
            if args.output:
                with open(args.output, 'w', encoding='utf-8') as f:
                    f.write(markdown_content)
                print(f"변환 완료: {args.output}")
            else:
                print(markdown_content)
        else:
            # HTML 파일을 변환
            output_file = converter.convert_file(args.input, args.output)
            print(f"변환 완료: {output_file}")
            
    except Exception as e:
        print(f"오류: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main() 