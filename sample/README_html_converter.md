# HTML to Markdown 변환기

HTML 파일이나 HTML 문자열을 마크다운 형식으로 변환하는 Python 도구입니다.

## 📁 파일 구성

- `html_to_markdown.py` - 완전한 기능을 갖춘 변환기 (클래스 기반)
- `simple_html_to_markdown.py` - 간단한 변환기 (함수 기반)
- `test_html_converter.py` - 테스트 스크립트
- `test_sample.html` - 테스트용 HTML 파일
- `README_html_converter.md` - 이 파일

## 🚀 사용법

### 1. 간단한 사용법 (simple_html_to_markdown.py)

```python
from simple_html_to_markdown import html_to_markdown

# HTML 문자열 변환
html_content = """
<h1>제목</h1>
<p>이것은 <strong>굵은 텍스트</strong>입니다.</p>
<ul>
    <li>항목 1</li>
    <li>항목 2</li>
</ul>
"""

markdown_result = html_to_markdown(html_content)
print(markdown_result)
```

### 2. 파일 변환

```python
from simple_html_to_markdown import convert_file

# HTML 파일을 마크다운으로 변환
output_file = convert_file("input.html", "output.md")
print(f"변환 완료: {output_file}")
```

### 3. 완전한 기능 사용법 (html_to_markdown.py)

```python
from html_to_markdown import HTMLToMarkdownConverter

# 변환기 인스턴스 생성
converter = HTMLToMarkdownConverter()

# HTML 문자열 변환
html_content = "<h1>제목</h1><p>내용</p>"
markdown_result = converter.convert(html_content)

# 파일 변환
output_file = converter.convert_file("input.html", "output.md")
```

### 4. 명령줄 사용법

```bash
# HTML 파일 변환
python html_to_markdown.py input.html -o output.md

# HTML 문자열 변환
python html_to_markdown.py "<h1>제목</h1>" -s

# 기본 출력 파일명으로 변환
python html_to_markdown.py input.html
```

## 🔧 지원하는 HTML 요소

### 텍스트 서식
- **헤딩**: `<h1>` ~ `<h6>` → `#` ~ `######`
- **굵은 텍스트**: `<strong>`, `<b>` → `**텍스트**`
- **기울임 텍스트**: `<em>`, `<i>` → `*텍스트*`
- **취소선**: `<del>`, `<strike>`, `<s>` → `~~텍스트~~`
- **코드**: `<code>` → `` `코드` ``

### 링크와 이미지
- **링크**: `<a href="...">` → `[텍스트](URL)`
- **이미지**: `<img src="..." alt="...">` → `![alt](src)`

### 목록
- **순서 없는 목록**: `<ul>` → `- 항목`
- **순서 있는 목록**: `<ol>` → `1. 항목`

### 기타
- **인용문**: `<blockquote>` → `> 텍스트`
- **테이블**: `<table>` → 마크다운 테이블 형식
- **수평선**: `<hr>` → `---`
- **줄바꿈**: `<br>` → `\n`

## 🧪 테스트

### 테스트 실행

```bash
# 전체 테스트 실행
python test_html_converter.py

# 간단한 변환기 테스트
python simple_html_to_markdown.py
```

### 테스트 내용

1. **기본 변환 테스트** - 간단한 HTML 요소들
2. **파일 변환 테스트** - HTML 파일을 마크다운으로 변환
3. **복잡한 HTML 테스트** - 다양한 HTML 구조
4. **HTML 엔티티 테스트** - `&amp;`, `&lt;` 등
5. **엣지 케이스 테스트** - 빈 HTML, 중첩 태그 등

## 📝 변환 예시

### 입력 HTML
```html
<h1>제목</h1>
<p>이것은 <strong>굵은 텍스트</strong>와 <em>기울임 텍스트</em>입니다.</p>
<ul>
    <li>항목 1</li>
    <li>항목 2</li>
</ul>
<a href="https://example.com">링크</a>
```

### 출력 마크다운
```markdown
# 제목

이것은 **굵은 텍스트**와 *기울임 텍스트*입니다.

- 항목 1
- 항목 2

[링크](https://example.com)
```

## ⚙️ 고급 기능

### HTML 엔티티 디코딩
- `&amp;` → `&`
- `&lt;` → `<`
- `&gt;` → `>`
- `&quot;` → `"`
- `&#39;` → `'`
- `&nbsp;` → ` `
- 숫자 엔티티: `&#160;` → 해당 유니코드 문자
- 16진수 엔티티: `&#x20;` → 해당 유니코드 문자

### 자동 제거되는 태그
- `<script>`, `<style>`, `<head>`, `<title>`, `<meta>`, `<link>`, `<html>`, `<body>`

### 테이블 변환
- `<thead>`, `<tbody>` 구조 지원
- `<th>`와 `<td>` 자동 감지
- 마크다운 테이블 형식으로 변환

## 🔍 문제 해결

### 일반적인 문제

1. **인코딩 오류**
   ```python
   # UTF-8 인코딩으로 파일 읽기
   with open('file.html', 'r', encoding='utf-8') as f:
       html_content = f.read()
   ```

2. **복잡한 HTML 구조**
   - 중첩된 태그는 순차적으로 처리됩니다
   - 일부 복잡한 구조는 수동 조정이 필요할 수 있습니다

3. **CSS 스타일**
   - CSS 스타일은 제거됩니다
   - 스타일링은 마크다운에서 별도로 처리해야 합니다

### 성능 최적화

- 큰 HTML 파일의 경우 메모리 사용량을 고려하세요
- 정규식 기반 변환으로 빠른 처리가 가능합니다

## 📚 추가 정보

### 마크다운 문법 참고
- [GitHub Flavored Markdown](https://github.github.com/gfm/)
- [CommonMark](https://commonmark.org/)

### HTML 파싱 라이브러리
더 정교한 HTML 파싱이 필요한 경우:
- `beautifulsoup4` - HTML 파싱
- `lxml` - XML/HTML 처리
- `html5lib` - HTML5 파싱

## 🤝 기여

버그 리포트나 기능 요청은 이슈로 등록해 주세요.

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 