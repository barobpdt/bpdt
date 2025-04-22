#!/usr/bin/env python
# -*- coding: utf-8 -*-

import markdown
import pandas as pd
import os
import base64
from PIL import Image
from io import BytesIO
import re
import json
from markdown.extensions import fenced_code, tables, codehilite, toc, meta, nl2br, sane_lists, smarty
from datetime import datetime
from urllib.parse import urlparse

# CSS 테마 정의
CSS_THEMES = {
    'default': {
        'name': '기본 테마',
        'css': """
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        pre {
            background-color: #f5f5f5;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
        }
        code {
            font-family: Consolas, Monaco, 'Andale Mono', monospace;
            background-color: #f5f5f5;
            padding: 2px 4px;
            border-radius: 3px;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }
        th, td {
            border: 1px solid #ddd;
            padding: 8px;
            text-align: left;
        }
        th {
            background-color: #f2f2f2;
        }
        img {
            max-width: 100%;
            height: auto;
        }
        blockquote {
            border-left: 4px solid #ddd;
            padding-left: 15px;
            color: #666;
        }
        """
    },
    'dark': {
        'name': '다크 테마',
        'css': """
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #e0e0e0;
            background-color: #1e1e1e;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        pre {
            background-color: #2d2d2d;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            border: 1px solid #3d3d3d;
        }
        code {
            font-family: Consolas, Monaco, 'Andale Mono', monospace;
            background-color: #2d2d2d;
            padding: 2px 4px;
            border-radius: 3px;
            color: #d4d4d4;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }
        th, td {
            border: 1px solid #3d3d3d;
            padding: 8px;
            text-align: left;
        }
        th {
            background-color: #2d2d2d;
        }
        img {
            max-width: 100%;
            height: auto;
        }
        blockquote {
            border-left: 4px solid #3d3d3d;
            padding-left: 15px;
            color: #a0a0a0;
        }
        a {
            color: #58a6ff;
        }
        h1, h2, h3, h4, h5, h6 {
            color: #e0e0e0;
        }
        """
    },
    'github': {
        'name': 'GitHub 스타일',
        'css': """
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
            line-height: 1.6;
            color: #24292e;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        pre {
            background-color: #f6f8fa;
            padding: 16px;
            border-radius: 6px;
            overflow-x: auto;
            font-size: 85%;
            line-height: 1.45;
        }
        code {
            font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, monospace;
            background-color: rgba(27, 31, 35, 0.05);
            padding: 0.2em 0.4em;
            border-radius: 3px;
            font-size: 85%;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }
        th, td {
            border: 1px solid #dfe2e5;
            padding: 6px 13px;
            text-align: left;
        }
        th {
            background-color: #f6f8fa;
        }
        img {
            max-width: 100%;
            height: auto;
            box-sizing: border-box;
        }
        blockquote {
            border-left: 4px solid #dfe2e5;
            padding-left: 15px;
            color: #6a737d;
        }
        a {
            color: #0366d6;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        h1, h2, h3, h4, h5, h6 {
            margin-top: 24px;
            margin-bottom: 16px;
            font-weight: 600;
            line-height: 1.25;
        }
        h1 {
            font-size: 2em;
            border-bottom: 1px solid #eaecef;
            padding-bottom: 0.3em;
        }
        h2 {
            font-size: 1.5em;
            border-bottom: 1px solid #eaecef;
            padding-bottom: 0.3em;
        }
        """
    },
    'print': {
        'name': '인쇄용 스타일',
        'css': """
        body {
            font-family: 'Times New Roman', Times, serif;
            line-height: 1.6;
            color: #000;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        pre {
            background-color: #f9f9f9;
            padding: 15px;
            border: 1px solid #ddd;
            overflow-x: auto;
            page-break-inside: avoid;
        }
        code {
            font-family: 'Courier New', Courier, monospace;
            background-color: #f9f9f9;
            padding: 2px 4px;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
            page-break-inside: avoid;
        }
        th, td {
            border: 1px solid #000;
            padding: 8px;
            text-align: left;
        }
        th {
            background-color: #f2f2f2;
        }
        img {
            max-width: 100%;
            height: auto;
            page-break-inside: avoid;
        }
        blockquote {
            border-left: 4px solid #000;
            padding-left: 15px;
            margin-left: 20px;
        }
        h1, h2, h3, h4, h5, h6 {
            page-break-after: avoid;
        }
        @media print {
            a {
                text-decoration: none;
                color: #000;
            }
            a[href]:after {
                content: " (" attr(href) ")";
                font-size: 90%;
            }
            a[href^="#"]:after {
                content: "";
            }
        }
        """
    },
    'academic': {
        'name': '학술 논문 스타일',
        'css': """
        body {
            font-family: 'Georgia', serif;
            line-height: 1.8;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        pre {
            background-color: #f8f8f8;
            padding: 15px;
            border: 1px solid #e0e0e0;
            overflow-x: auto;
            font-size: 0.9em;
        }
        code {
            font-family: 'Courier New', Courier, monospace;
            background-color: #f8f8f8;
            padding: 2px 4px;
            font-size: 0.9em;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }
        th, td {
            border: 1px solid #ddd;
            padding: 8px;
            text-align: left;
        }
        th {
            background-color: #f2f2f2;
            font-weight: bold;
        }
        img {
            max-width: 100%;
            height: auto;
            display: block;
            margin: 20px auto;
        }
        blockquote {
            border-left: 4px solid #4a4a4a;
            padding-left: 15px;
            color: #4a4a4a;
            font-style: italic;
        }
        h1, h2, h3, h4, h5, h6 {
            font-family: 'Times New Roman', Times, serif;
            margin-top: 30px;
            margin-bottom: 15px;
            font-weight: bold;
        }
        h1 {
            font-size: 2.2em;
            text-align: center;
        }
        h2 {
            font-size: 1.8em;
            border-bottom: 1px solid #ddd;
            padding-bottom: 5px;
        }
        h3 {
            font-size: 1.5em;
        }
        p {
            text-align: justify;
            margin-bottom: 15px;
        }
        """
    },
    'minimal': {
        'name': '미니멀 스타일',
        'css': """
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 700px;
            margin: 0 auto;
            padding: 20px;
        }
        pre {
            background-color: #f7f7f7;
            padding: 15px;
            border-radius: 3px;
            overflow-x: auto;
        }
        code {
            font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;
            background-color: #f7f7f7;
            padding: 2px 4px;
            border-radius: 3px;
            font-size: 0.9em;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }
        th, td {
            border: 1px solid #e0e0e0;
            padding: 8px;
            text-align: left;
        }
        th {
            background-color: #f7f7f7;
            font-weight: 600;
        }
        img {
            max-width: 100%;
            height: auto;
        }
        blockquote {
            border-left: 3px solid #e0e0e0;
            padding-left: 15px;
            color: #666;
            margin-left: 0;
        }
        a {
            color: #0366d6;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        h1, h2, h3, h4, h5, h6 {
            margin-top: 24px;
            margin-bottom: 16px;
            font-weight: 600;
            line-height: 1.25;
        }
        """
    }
}

class MarkdownImageProcessor:
    """Markdown 이미지 처리를 위한 클래스"""
    
    def __init__(self, base_dir=None):
        """
        초기화
        
        Args:
            base_dir (str, optional): 이미지 파일의 기본 디렉토리
        """
        self.base_dir = base_dir or os.getcwd()
    
    def process_image_path(self, match):
        """
        Markdown 이미지 태그의 경로를 처리
        
        Args:
            match (re.Match): 정규식 매치 객체
            
        Returns:
            str: 처리된 이미지 태그
        """
        full_match = match.group(0)
        alt_text = match.group(1)
        img_path = match.group(2)
        
        # 이미지 경로가 URL인 경우 그대로 반환
        if img_path.startswith(('http://', 'https://', '//')):
            return full_match
        
        # 상대 경로를 절대 경로로 변환
        abs_path = os.path.abspath(os.path.join(self.base_dir, img_path))
        
        # 이미지가 존재하는지 확인
        if not os.path.exists(abs_path):
            print(f"경고: 이미지 파일을 찾을 수 없습니다: {abs_path}")
            return full_match
        
        # 이미지 크기 조정 (선택적)
        try:
            with Image.open(abs_path) as img:
                # 이미지가 너무 큰 경우 크기 조정 (예: 최대 너비 800px)
                if img.width > 800:
                    ratio = 800 / img.width
                    new_size = (800, int(img.height * ratio))
                    img = img.resize(new_size, Image.LANCZOS)
                    
                    # 조정된 이미지를 임시 파일로 저장
                    temp_path = f"{abs_path}.temp"
                    img.save(temp_path, quality=85, optimize=True)
                    abs_path = temp_path
        except Exception as e:
            print(f"이미지 처리 중 오류 발생: {e}")
        
        # 이미지를 base64로 인코딩 (선택적)
        try:
            with open(abs_path, 'rb') as img_file:
                img_data = img_file.read()
                img_type = Image.open(BytesIO(img_data)).format.lower()
                base64_data = base64.b64encode(img_data).decode('utf-8')
                return f'![{alt_text}](data:image/{img_type};base64,{base64_data})'
        except Exception as e:
            print(f"이미지 인코딩 중 오류 발생: {e}")
            return full_match

class PandasDataProcessor:
    """Pandas 데이터 처리를 위한 클래스"""
    
    @staticmethod
    def process_dataframe(df, max_rows=100, max_cols=None):
        """
        DataFrame을 Markdown 테이블로 변환
        
        Args:
            df (pandas.DataFrame): 처리할 DataFrame
            max_rows (int, optional): 최대 행 수
            max_cols (int, optional): 최대 열 수
            
        Returns:
            str: Markdown 테이블 형식의 문자열
        """
        # 데이터 크기 제한
        if max_rows and len(df) > max_rows:
            df = df.head(max_rows)
        
        if max_cols and len(df.columns) > max_cols:
            df = df.iloc[:, :max_cols]
        
        # DataFrame을 Markdown 테이블로 변환
        return df.to_markdown(index=False)
    
    @staticmethod
    def load_data(file_path, **kwargs):
        """
        다양한 형식의 데이터 파일 로드
        
        Args:
            file_path (str): 데이터 파일 경로
            **kwargs: pandas.read_* 함수에 전달할 추가 인수
            
        Returns:
            pandas.DataFrame: 로드된 데이터
        """
        ext = os.path.splitext(file_path)[1].lower()
        
        if ext == '.csv':
            return pd.read_csv(file_path, **kwargs)
        elif ext == '.xlsx' or ext == '.xls':
            return pd.read_excel(file_path, **kwargs)
        elif ext == '.json':
            return pd.read_json(file_path, **kwargs)
        elif ext == '.html':
            return pd.read_html(file_path, **kwargs)[0]  # 첫 번째 테이블만 반환
        elif ext == '.sql':
            # SQL 파일은 직접 실행하지 않고 쿼리만 반환
            with open(file_path, 'r') as f:
                return f.read()
        else:
            raise ValueError(f"지원하지 않는 파일 형식입니다: {ext}")

def process_markdown_with_images(markdown_text, base_dir=None):
    """
    Markdown 텍스트에서 이미지 경로를 처리
    
    Args:
        markdown_text (str): 처리할 Markdown 텍스트
        base_dir (str, optional): 이미지 파일의 기본 디렉토리
        
    Returns:
        str: 처리된 Markdown 텍스트
    """
    processor = MarkdownImageProcessor(base_dir)
    # Markdown 이미지 태그 패턴: ![alt](path)
    pattern = r'!\[(.*?)\]\((.*?)\)'
    return re.sub(pattern, processor.process_image_path, markdown_text)

def markdown_to_html(markdown_text, output_file=None, extensions=None, base_dir=None, theme='default', custom_css=None):
    """
    Markdown 텍스트를 HTML로 변환
    
    Args:
        markdown_text (str): 변환할 Markdown 텍스트
        output_file (str, optional): 출력 HTML 파일 경로
        extensions (list, optional): 사용할 Markdown 확장 기능 목록
        base_dir (str, optional): 이미지 파일의 기본 디렉토리
        
    Returns:
        str: 변환된 HTML
    """
    # 이미지 처리
    processed_text = process_markdown_with_images(markdown_text, base_dir)
    
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
        processed_text,
        extensions=extensions,
        output_format='html5'
    )
    
    # CSS 선택
    if custom_css:
        css = custom_css
    elif theme in CSS_THEMES:
        css = CSS_THEMES[theme]['css']
    else:
        css = CSS_THEMES['default']['css']
    
    # HTML 템플릿 생성
    html_template = f"""<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Markdown to HTML</title>
    <style>
{css}        
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

class MarkdownLinkHandler:
    """Markdown 문서의 링크를 처리하는 클래스"""
    
    def __init__(self, base_dir=None, link_handlers=None):
        """
        초기화
        
        Args:
            base_dir (str, optional): 상대 경로 링크의 기본 디렉토리
            link_handlers (dict, optional): 링크 처리 함수 딕셔너리
        """
        self.base_dir = base_dir or os.getcwd()
        self.link_handlers = link_handlers or {}
        self.link_history = []
        self.max_history = 50
        
        # 기본 링크 처리 함수 등록
        self.register_default_handlers()
    
    def register_default_handlers(self):
        """기본 링크 처리 함수 등록"""
        # 외부 URL 처리
        self.register_handler('http', self.handle_external_url)
        self.register_handler('https', self.handle_external_url)
        
        # 로컬 파일 처리
        self.register_handler('file', self.handle_local_file)
        
        # 이메일 링크 처리
        self.register_handler('mailto', self.handle_email)
        
        # 앵커 링크 처리 (같은 문서 내 링크)
        self.register_handler('anchor', self.handle_anchor)
        
        # 다운로드 링크 처리
        self.register_handler('download', self.handle_download)
    
    def register_handler(self, scheme, handler_func):
        """
        링크 처리 함수 등록
        
        Args:
            scheme (str): URL 스키마 (http, https, file 등)
            handler_func (callable): 링크 처리 함수
        """
        self.link_handlers[scheme] = handler_func
    
    def process_link(self, link, text=None):
        """
        링크 처리
        
        Args:
            link (str): 링크 URL
            text (str, optional): 링크 텍스트
            
        Returns:
            bool: 처리 성공 여부
        """
        # 링크 기록에 추가
        self.add_to_history(link, text)
        
        # URL 파싱
        parsed_url = urlparse(link)
        scheme = parsed_url.scheme
        
        # 스키마가 없는 경우 기본적으로 file 스키마로 처리
        if not scheme:
            scheme = 'file'
            # 상대 경로를 절대 경로로 변환
            link = os.path.abspath(os.path.join(self.base_dir, link))
            parsed_url = urlparse(f"file://{link}")
        
        # 해당 스키마의 처리 함수 호출
        if scheme in self.link_handlers:
            return self.link_handlers[scheme](parsed_url, text)
        else:
            # 처리 함수가 없는 경우 기본 브라우저로 열기
            return self.handle_external_url(parsed_url, text)
    
    def add_to_history(self, link, text=None):
        """
        링크 기록에 추가
        
        Args:
            link (str): 링크 URL
            text (str, optional): 링크 텍스트
        """
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.link_history.append({
            'timestamp': timestamp,
            'link': link,
            'text': text
        })
        
        # 기록 크기 제한
        if len(self.link_history) > self.max_history:
            self.link_history.pop(0)
    
    def get_history(self):
        """
        링크 기록 반환
        
        Returns:
            list: 링크 기록 리스트
        """
        return self.link_history
    
    def clear_history(self):
        """링크 기록 초기화"""
        self.link_history = []
    
    def handle_external_url(self, parsed_url, text=None):
        """
        외부 URL 처리
        
        Args:
            parsed_url (urllib.parse.ParseResult): 파싱된 URL
            text (str, optional): 링크 텍스트
            
        Returns:
            bool: 처리 성공 여부
        """
        url = parsed_url.geturl()
        try:
            webbrowser.open(url)
            return True
        except Exception as e:
            print(f"외부 URL 열기 실패: {e}")
            return False
    
    def handle_local_file(self, parsed_url, text=None):
        """
        로컬 파일 처리
        
        Args:
            parsed_url (urllib.parse.ParseResult): 파싱된 URL
            text (str, optional): 링크 텍스트
            
        Returns:
            bool: 처리 성공 여부
        """
        # file:// URL에서 경로 추출
        path = parsed_url.path
        
        # Windows 경로 처리 (file:///C:/path -> C:/path)
        if os.name == 'nt' and path.startswith('/'):
            path = path[1:]
        
        # 파일 존재 확인
        if not os.path.exists(path):
            print(f"파일을 찾을 수 없습니다: {path}")
            return False
        
        # 파일 확장자에 따른 처리
        ext = os.path.splitext(path)[1].lower()
        
        # 텍스트 파일인 경우 텍스트 뷰어로 열기
        if ext in ['.txt', '.md', '.py', '.js', '.html', '.css', '.json', '.xml']:
            return self.open_text_file(path)
        
        # 이미지 파일인 경우 이미지 뷰어로 열기
        elif ext in ['.jpg', '.jpeg', '.png', '.gif', '.bmp', '.webp']:
            return self.open_image_file(path)
        
        # 그 외의 경우 기본 프로그램으로 열기
        else:
            try:
                os.startfile(path) if os.name == 'nt' else webbrowser.open(f"file://{path}")
                return True
            except Exception as e:
                print(f"파일 열기 실패: {e}")
                return False
    
    def open_text_file(self, path):
        """
        텍스트 파일 열기
        
        Args:
            path (str): 파일 경로
            
        Returns:
            bool: 처리 성공 여부
        """
        try:
            # 간단한 텍스트 뷰어 창 생성
            root = tk.Tk()
            root.title(f"텍스트 뷰어 - {os.path.basename(path)}")
            root.geometry("800x600")
            
            # 메뉴바 생성
            menubar = tk.Menu(root)
            file_menu = tk.Menu(menubar, tearoff=0)
            file_menu.add_command(label="닫기", command=root.destroy)
            menubar.add_cascade(label="파일", menu=file_menu)
            root.config(menu=menubar)
            
            # 텍스트 영역 생성
            text_area = tk.Text(root, wrap=tk.WORD)
            text_area.pack(fill=tk.BOTH, expand=True)
            
            # 스크롤바 추가
            scrollbar = ttk.Scrollbar(root, command=text_area.yview)
            scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
            text_area.config(yscrollcommand=scrollbar.set)
            
            # 파일 내용 로드
            with open(path, 'r', encoding='utf-8') as f:
                content = f.read()
                text_area.insert(tk.END, content)
                text_area.config(state=tk.DISABLED)  # 읽기 전용으로 설정
            
            root.mainloop()
            return True
        except Exception as e:
            print(f"텍스트 파일 열기 실패: {e}")
            return False
    
    def open_image_file(self, path):
        """
        이미지 파일 열기
        
        Args:
            path (str): 파일 경로
            
        Returns:
            bool: 처리 성공 여부
        """
        try:
            # PIL 라이브러리 사용 (없는 경우 설치 필요)
            try:
                from PIL import Image, ImageTk
            except ImportError:
                print("PIL 라이브러리가 필요합니다. 'pip install pillow'로 설치하세요.")
                return False
            
            # 이미지 뷰어 창 생성
            root = tk.Tk()
            root.title(f"이미지 뷰어 - {os.path.basename(path)}")
            
            # 메뉴바 생성
            menubar = tk.Menu(root)
            file_menu = tk.Menu(menubar, tearoff=0)
            file_menu.add_command(label="닫기", command=root.destroy)
            menubar.add_cascade(label="파일", menu=file_menu)
            root.config(menu=menubar)
            
            # 이미지 로드 및 표시
            img = Image.open(path)
            
            # 창 크기 조정 (이미지가 너무 큰 경우)
            max_width = 800
            max_height = 600
            width, height = img.size
            
            if width > max_width or height > max_height:
                ratio = min(max_width / width, max_height / height)
                width = int(width * ratio)
                height = int(height * ratio)
                img = img.resize((width, height), Image.LANCZOS)
            
            photo = ImageTk.PhotoImage(img)
            
            # 이미지 표시
            label = tk.Label(root, image=photo)
            label.image = photo  # 참조 유지
            label.pack()
            
            root.mainloop()
            return True
        except Exception as e:
            print(f"이미지 파일 열기 실패: {e}")
            return False
    
    def handle_email(self, parsed_url, text=None):
        """
        이메일 링크 처리
        
        Args:
            parsed_url (urllib.parse.ParseResult): 파싱된 URL
            text (str, optional): 링크 텍스트
            
        Returns:
            bool: 처리 성공 여부
        """
        # mailto: URL에서 이메일 주소 추출
        email = parsed_url.path
        
        # 기본 메일 클라이언트로 열기
        try:
            webbrowser.open(f"mailto:{email}")
            return True
        except Exception as e:
            print(f"이메일 클라이언트 열기 실패: {e}")
            return False
    
    def handle_anchor(self, parsed_url, text=None):
        """
        앵커 링크 처리 (같은 문서 내 링크)
        
        Args:
            parsed_url (urllib.parse.ParseResult): 파싱된 URL
            text (str, optional): 링크 텍스트
            
        Returns:
            bool: 처리 성공 여부
        """
        # 앵커 ID 추출
        anchor_id = parsed_url.fragment
        
        # 앵커 ID가 없는 경우
        if not anchor_id:
            return False
        
        # 현재 활성화된 창에서 앵커로 스크롤
        # (이 부분은 GUI 애플리케이션에 따라 구현이 달라질 수 있음)
        print(f"앵커로 스크롤: #{anchor_id}")
        return True
    
    def handle_download(self, parsed_url, text=None):
        """
        다운로드 링크 처리
        
        Args:
            parsed_url (urllib.parse.ParseResult): 파싱된 URL
            text (str, optional): 링크 텍스트
            
        Returns:
            bool: 처리 성공 여부
        """
        # 다운로드 URL 추출
        url = parsed_url.geturl()
        
        # 다운로드 대화상자 표시
        try:
            # 파일 이름 추출
            filename = os.path.basename(urlparse(url).path)
            if not filename:
                filename = "downloaded_file"
            
            # 저장 대화상자 표시
            save_path = filedialog.asksaveasfilename(
                initialfile=filename,
                title="파일 저장"
            )
            
            if save_path:
                # 파일 다운로드 (실제 구현에서는 requests 등의 라이브러리 사용)
                print(f"다운로드: {url} -> {save_path}")
                return True
            return False
        except Exception as e:
            print(f"다운로드 실패: {e}")
            return False

class MarkdownLinkProcessor:
    """Markdown 문서의 링크를 처리하는 클래스"""
    
    def __init__(self, base_dir=None):
        """
        초기화
        
        Args:
            base_dir (str, optional): 상대 경로 링크의 기본 디렉토리
        """
        self.base_dir = base_dir or os.getcwd()
        self.link_handler = MarkdownLinkHandler(base_dir)
    
    def process_markdown(self, markdown_text):
        """
        Markdown 텍스트에서 링크를 처리
        
        Args:
            markdown_text (str): 처리할 Markdown 텍스트
            
        Returns:
            str: 처리된 Markdown 텍스트
        """
        # 링크 패턴: [텍스트](URL)
        link_pattern = r'\[([^\]]+)\]\(([^)]+)\)'
        
        def replace_link(match):
            text = match.group(1)
            url = match.group(2)
            
            # 링크 처리 함수 호출
            self.link_handler.process_link(url, text)
            
            # 원래 링크 형식 유지
            return match.group(0)
        
        # 링크 패턴 찾아서 처리
        processed_text = re.sub(link_pattern, replace_link, markdown_text)
        
        return processed_text
    
    def markdown_to_html(self, markdown_text, output_file=None, extensions=None):
        """
        Markdown 텍스트를 HTML로 변환하고 링크 처리
        
        Args:
            markdown_text (str): 변환할 Markdown 텍스트
            output_file (str, optional): 출력 HTML 파일 경로
            extensions (list, optional): 사용할 Markdown 확장 기능 목록
            
        Returns:
            str: 변환된 HTML
        """
        # 링크 처리
        processed_text = self.process_markdown(markdown_text)
        
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
            processed_text,
            extensions=extensions,
            output_format='html5'
        )
        
        # 링크 클릭 이벤트를 처리하는 JavaScript 추가
        html = self.add_link_click_handler(html)
        
        # HTML 템플릿 생성
        html_template = f"""<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Markdown with Link Handler</title>
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
        a {{
            color: #0366d6;
            text-decoration: none;
        }}
        a:hover {{
            text-decoration: underline;
        }}
        #link-history {{
            margin-top: 30px;
            padding: 15px;
            background-color: #f9f9f9;
            border-radius: 5px;
        }}
        #link-history h2 {{
            margin-top: 0;
        }}
        .history-item {{
            margin-bottom: 10px;
            padding-bottom: 10px;
            border-bottom: 1px solid #eee;
        }}
        .history-item:last-child {{
            border-bottom: none;
        }}
        .history-timestamp {{
            color: #666;
            font-size: 0.8em;
        }}
    </style>
</head>
<body>
{html}

<div id="link-history">
    <h2>링크 기록</h2>
    <div id="history-list"></div>
</div>

<script>
// 링크 클릭 이벤트 처리
document.addEventListener('DOMContentLoaded', function() {{
    // 모든 링크에 클릭 이벤트 리스너 추가
    const links = document.querySelectorAll('a');
    links.forEach(link => {{
        link.addEventListener('click', function(e) {{
            e.preventDefault();
            
            const url = this.getAttribute('href');
            const text = this.textContent;
            
            // 링크 처리 함수 호출 (실제로는 서버로 요청을 보내거나 로컬 함수를 호출)
            handleLinkClick(url, text);
            
            // 링크 기록에 추가
            addToHistory(url, text);
            
            // 기본 동작 (브라우저에서 링크 열기)
            window.open(url, '_blank');
        }});
    }});
    
    // 링크 처리 함수 (실제 구현에서는 서버로 요청을 보내거나 로컬 함수를 호출)
    function handleLinkClick(url, text) {{
        console.log('링크 클릭:', url, text);
        // 여기에 실제 링크 처리 로직 구현
    }}
    
    // 링크 기록 관리
    let linkHistory = [];
    const maxHistory = 50;
    
    // 링크 기록에 추가
    function addToHistory(url, text) {{
        const timestamp = new Date().toLocaleString();
        linkHistory.unshift({{ timestamp, url, text }});
        
        // 기록 크기 제한
        if (linkHistory.length > maxHistory) {{
            linkHistory.pop();
        }}
        
        // 기록 표시 업데이트
        updateHistoryDisplay();
    }}
    
    // 기록 표시 업데이트
    function updateHistoryDisplay() {{
        const historyList = document.getElementById('history-list');
        historyList.innerHTML = '';
        
        linkHistory.forEach(item => {{
            const historyItem = document.createElement('div');
            historyItem.className = 'history-item';
            historyItem.innerHTML = `
                <div class="history-timestamp">${{item.timestamp}}</div>
                <div><a href="${{item.url}}" target="_blank">${{item.text || item.url}}</a></div>
            `;
            historyList.appendChild(historyItem);
        }});
    }}
    
    // 초기 기록 표시
    updateHistoryDisplay();
}});
</script>
</body>
</html>"""
        
        # 출력 파일이 지정된 경우 파일로 저장
        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(html_template)
            return f"HTML이 '{output_file}' 파일에 저장되었습니다."
        
        return html_template
    
    def add_link_click_handler(self, html):
        """
        HTML에 링크 클릭 이벤트 처리 JavaScript 추가
        
        Args:
            html (str): HTML 텍스트
            
        Returns:
            str: JavaScript가 추가된 HTML
        """
        # 링크 클릭 이벤트를 처리하는 JavaScript 추가
        script = """
<script>
document.addEventListener('DOMContentLoaded', function() {
    // 모든 링크에 클릭 이벤트 리스너 추가
    const links = document.querySelectorAll('a');
    links.forEach(link => {
        link.addEventListener('click', function(e) {
            const url = this.getAttribute('href');
            const text = this.textContent;
            
            // 링크 처리 함수 호출 (실제로는 서버로 요청을 보내거나 로컬 함수를 호출)
            handleLinkClick(url, text);
            
            // 링크 기록에 추가
            addToHistory(url, text);
        });
    });
    
    // 링크 처리 함수 (실제 구현에서는 서버로 요청을 보내거나 로컬 함수를 호출)
    function handleLinkClick(url, text) {
        console.log('링크 클릭:', url, text);
        // 여기에 실제 링크 처리 로직 구현
    }
    
    // 링크 기록 관리
    let linkHistory = [];
    const maxHistory = 50;
    
    // 링크 기록에 추가
    function addToHistory(url, text) {
        const timestamp = new Date().toLocaleString();
        linkHistory.unshift({ timestamp, url, text });
        
        // 기록 크기 제한
        if (linkHistory.length > maxHistory) {
            linkHistory.pop();
        }
        
        // 기록 표시 업데이트
        updateHistoryDisplay();
    }
    
    // 기록 표시 업데이트
    function updateHistoryDisplay() {
        const historyList = document.getElementById('history-list');
        if (historyList) {
            historyList.innerHTML = '';
            
            linkHistory.forEach(item => {
                const historyItem = document.createElement('div');
                historyItem.className = 'history-item';
                historyItem.innerHTML = `
                    <div class="history-timestamp">${item.timestamp}</div>
                    <div><a href="${item.url}" target="_blank">${item.text || item.url}</a></div>
                `;
                historyList.appendChild(historyItem);
            });
        }
    }
    
    // 초기 기록 표시
    updateHistoryDisplay();
});
</script>
"""
        
        # HTML에 JavaScript 추가
        return html.replace('</body>', f'{script}</body>')


def main():
    result = markdown_to_html(markdown_text, args.output, extensions, args.base_dir)
    print(result)

if __name__ == "__main__":
    main() 