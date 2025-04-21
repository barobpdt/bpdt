import io
import pandas as pd
import msoffcrypto
import plotly.express as px
from jinja2 import Template
import base64
from pathlib import Path

passwd = '123'

decrypted_workbook = io.BytesIO()
with open('c:/temp/aaa.XLSX', 'rb') as file:
    office_file = msoffcrypto.OfficeFile(file)
    office_file.load_key(password=passwd)
    office_file.decrypt(decrypted_workbook)

df = pd.read_excel(decrypted_workbook, sheet_name='abc')

class StaticStreamlit:
    def __init__(self, title="Static Streamlit App"):
        self.title = title
        self.content = []
        self.css = """
            body { font-family: sans-serif; max-width: 1200px; margin: 0 auto; padding: 20px; }
            .chart { width: 100%; height: 500px; }
            .dataframe { width: 100%; border-collapse: collapse; margin: 20px 0; }
            .dataframe th, .dataframe td { padding: 8px; border: 1px solid #ddd; }
            .dataframe th { background-color: #f8f9fa; }
        """
    
    def header(self, text, level=1):
        self.content.append({
            'type': 'header',
            'content': text,
            'level': level
        })
    
    def text(self, text):
        self.content.append({
            'type': 'text',
            'content': text
        })
    
    def dataframe(self, df):
        self.content.append({
            'type': 'dataframe',
            'content': df.to_html(classes='dataframe', index=False)
        })
    
    def plotly_chart(self, fig):
        self.content.append({
            'type': 'chart',
            'content': fig.to_html(include_plotlyjs='cdn', full_html=False)
        })
    
    def image(self, image_path, caption=None):
        path = Path(image_path)
        if not path.exists():
            raise FileNotFoundError(f"Image not found: {image_path}")
        
        with open(path, 'rb') as f:
            img_data = base64.b64encode(f.read()).decode()
        
        self.content.append({
            'type': 'image',
            'content': f'data:image/{path.suffix[1:]};base64,{img_data}',
            'caption': caption
        })
    
    def generate_html(self):
        template = Template("""
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }}</title>
    <style>{{ css }}</style>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
</head>
<body>
    {% for item in content %}
        {% if item.type == 'header' %}
            <h{{ item.level }}>{{ item.content }}</h{{ item.level }}>
        {% elif item.type == 'text' %}
            <p>{{ item.content }}</p>
        {% elif item.type == 'dataframe' %}
            {{ item.content | safe }}
        {% elif item.type == 'chart' %}
            <div class="chart">{{ item.content | safe }}</div>
        {% elif item.type == 'image' %}
            <div class="image">
                <img src="{{ item.content }}" alt="{{ item.caption or '' }}">
                {% if item.caption %}
                    <p class="caption">{{ item.caption }}</p>
                {% endif %}
            </div>
        {% endif %}
    {% endfor %}
</body>
</html>
        """)
        
        return template.render(
            title=self.title,
            css=self.css,
            content=self.content
        )
    
    def save(self, output_path):
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(self.generate_html())

# 사용 예시
if __name__ == "__main__":
    # 정적 Streamlit 페이지 생성
    app = StaticStreamlit("엑셀 데이터 분석")
    
    # 헤더 추가
    app.header("엑셀 데이터 분석 리포트")
    app.text("이 리포트는 엑셀 파일의 분석 결과를 보여줍니다.")
    
    # 데이터프레임 표시
    app.header("데이터 테이블", level=2)
    app.dataframe(df)
    
    # 차트 생성 및 표시 (예시)
    if 'value' in df.columns:  # 'value' 컬럼이 있다고 가정
        fig = px.line(df, x=df.index, y='value', title='데이터 트렌드')
        app.header("데이터 시각화", level=2)
        app.plotly_chart(fig)
    
    # HTML 파일로 저장
    app.save("excel_report.html")