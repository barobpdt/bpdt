#!/usr/bin/env python3
"""
HTML to Markdown 변환기 테스트 스크립트
"""

import os
import sys
from simple_html_to_markdown import html_to_markdown, convert_file

def test_basic_conversion():
    """기본 변환 테스트"""
    print("🔍 기본 변환 테스트...")
    
    html = """
    <h1>제목</h1>
    <p>이것은 <strong>굵은 텍스트</strong>와 <em>기울임 텍스트</em>입니다.</p>
    <ul>
        <li>항목 1</li>
        <li>항목 2</li>
    </ul>
    """
    
    result = html_to_markdown(html)
    print("변환 결과:")
    print(result)
    print("-" * 50)

def test_file_conversion():
    """파일 변환 테스트"""
    print("🔍 파일 변환 테스트...")
    
    input_file = "test_sample.html"
    if not os.path.exists(input_file):
        print(f"❌ 테스트 파일이 없습니다: {input_file}")
        return
    
    try:
        output_file = convert_file(input_file)
        print(f"✅ 변환 완료: {output_file}")
        
        # 결과 확인
        with open(output_file, 'r', encoding='utf-8') as f:
            content = f.read()
            print("\n변환된 마크다운 내용:")
            print(content[:500] + "..." if len(content) > 500 else content)
            
    except Exception as e:
        print(f"❌ 변환 실패: {e}")
    
    print("-" * 50)

def test_complex_html():
    """복잡한 HTML 테스트"""
    print("🔍 복잡한 HTML 테스트...")
    
    complex_html = """
    <html>
    <head><title>복잡한 테스트</title></head>
    <body>
        <h1>메인 제목</h1>
        <h2>부제목</h2>
        <p>첫 번째 단락입니다.</p>
        
        <blockquote>
            이것은 인용문입니다.
            여러 줄에 걸쳐 있습니다.
        </blockquote>
        
        <table>
            <tr>
                <th>이름</th>
                <th>나이</th>
            </tr>
            <tr>
                <td>김철수</td>
                <td>25</td>
            </tr>
            <tr>
                <td>이영희</td>
                <td>30</td>
            </tr>
        </table>
        
        <ol>
            <li>첫 번째</li>
            <li>두 번째</li>
        </ol>
        
        <p><a href="https://example.com">링크</a>와 <img src="image.jpg" alt="이미지">가 있습니다.</p>
    </body>
    </html>
    """
    
    result = html_to_markdown(complex_html)
    print("변환 결과:")
    print(result)
    print("-" * 50)

def test_html_entities():
    """HTML 엔티티 테스트"""
    print("🔍 HTML 엔티티 테스트...")
    
    html_with_entities = """
    <p>HTML 엔티티 테스트: &amp; &lt; &gt; &quot; &#39; &nbsp; &copy; &reg;</p>
    <p>숫자 엔티티: &#160; &#169; &#174;</p>
    <p>16진수 엔티티: &#x20; &#xA0;</p>
    """
    
    result = html_to_markdown(html_with_entities)
    print("변환 결과:")
    print(result)
    print("-" * 50)

def test_edge_cases():
    """엣지 케이스 테스트"""
    print("🔍 엣지 케이스 테스트...")
    
    # 빈 HTML
    print("1. 빈 HTML:")
    result = html_to_markdown("")
    print(f"결과: '{result}'")
    
    # HTML 태그만 있는 경우
    print("\n2. HTML 태그만 있는 경우:")
    result = html_to_markdown("<div></div>")
    print(f"결과: '{result}'")
    
    # 중첩된 태그
    print("\n3. 중첩된 태그:")
    result = html_to_markdown("<p><strong><em>중첩된 텍스트</em></strong></p>")
    print(f"결과: '{result}'")
    
    # 특수 문자
    print("\n4. 특수 문자:")
    result = html_to_markdown("<p>특수문자: @#$%^&*()</p>")
    print(f"결과: '{result}'")
    
    print("-" * 50)

def compare_with_expected():
    """예상 결과와 비교"""
    print("🔍 예상 결과와 비교...")
    
    html = """
    <h1>테스트 제목</h1>
    <p>이것은 <strong>굵은</strong> <em>기울임</em> 텍스트입니다.</p>
    <ul>
        <li>항목 1</li>
        <li>항목 2</li>
    </ul>
    """
    
    expected = """# 테스트 제목

이것은 **굵은** *기울임* 텍스트입니다.

- 항목 1
- 항목 2
"""
    
    result = html_to_markdown(html)
    
    print("예상 결과:")
    print(expected)
    print("\n실제 결과:")
    print(result)
    
    if result.strip() == expected.strip():
        print("✅ 변환 결과가 예상과 일치합니다!")
    else:
        print("❌ 변환 결과가 예상과 다릅니다.")
    
    print("-" * 50)

def main():
    """메인 테스트 함수"""
    print("🚀 HTML to Markdown 변환기 테스트 시작")
    print("=" * 60)
    
    # 기본 변환 테스트
    test_basic_conversion()
    
    # 파일 변환 테스트
    test_file_conversion()
    
    # 복잡한 HTML 테스트
    test_complex_html()
    
    # HTML 엔티티 테스트
    test_html_entities()
    
    # 엣지 케이스 테스트
    test_edge_cases()
    
    # 예상 결과와 비교
    compare_with_expected()
    
    print("🏁 모든 테스트 완료!")

if __name__ == "__main__":
    main() 