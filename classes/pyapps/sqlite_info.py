import sqlite3
import json
import os
import argparse
from typing import Dict, List, Any, Optional

class SQLiteTreeAnalyzer:
    """
    SQLite 데이터베이스의 테이블 및 컬럼 정보를 트리 형태로 분석하는 클래스
    """
    
    def __init__(self, db_path: str):
        """
        SQLiteTreeAnalyzer 초기화
        
        Args:
            db_path: SQLite 데이터베이스 파일 경로
        """
        self.db_path = db_path
        self.conn = None
        self.cursor = None
        self.tree_data = {}
        
    def connect(self) -> None:
        """데이터베이스에 연결"""
        try:
            self.conn = sqlite3.connect(self.db_path)
            self.cursor = self.conn.cursor()
            print(f"데이터베이스 '{self.db_path}'에 성공적으로 연결되었습니다.")
        except sqlite3.Error as e:
            print(f"데이터베이스 연결 오류: {e}")
            raise
    
    def disconnect(self) -> None:
        """데이터베이스 연결 종료"""
        if self.conn:
            self.conn.close()
            print("데이터베이스 연결이 종료되었습니다.")
    
    def get_tables(self) -> List[str]:
        """
        데이터베이스의 모든 테이블 목록을 반환
        
        Returns:
            테이블 이름 목록
        """
        self.cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'")
        return [table[0] for table in self.cursor.fetchall()]
    
    def get_table_info(self, table_name: str) -> List[Dict[str, Any]]:
        """
        테이블의 상세 정보를 반환
        
        Args:
            table_name: 테이블 이름
            
        Returns:
            테이블 컬럼 정보 목록
        """
        self.cursor.execute(f"PRAGMA table_info({table_name})")
        columns = []
        for col in self.cursor.fetchall():
            columns.append({
                "cid": col[0],
                "name": col[1],
                "type": col[2],
                "notnull": bool(col[3]),
                "default_value": col[4],
                "pk": bool(col[5])
            })
        return columns
    
    def get_foreign_keys(self, table_name: str) -> List[Dict[str, Any]]:
        """
        테이블의 외래 키 정보를 반환
        
        Args:
            table_name: 테이블 이름
            
        Returns:
            외래 키 정보 목록
        """
        self.cursor.execute(f"PRAGMA foreign_key_list({table_name})")
        foreign_keys = []
        for fk in self.cursor.fetchall():
            foreign_keys.append({
                "id": fk[0],
                "seq": fk[1],
                "table": fk[2],
                "from": fk[3],
                "to": fk[4],
                "on_update": fk[5],
                "on_delete": fk[6],
                "match": fk[7]
            })
        return foreign_keys
    
    def get_indexes(self, table_name: str) -> List[Dict[str, Any]]:
        """
        테이블의 인덱스 정보를 반환
        
        Args:
            table_name: 테이블 이름
            
        Returns:
            인덱스 정보 목록
        """
        self.cursor.execute(f"PRAGMA index_list({table_name})")
        indexes = []
        for idx in self.cursor.fetchall():
            index_name = idx[1]
            self.cursor.execute(f"PRAGMA index_info({index_name})")
            columns = [col[2] for col in self.cursor.fetchall()]
            indexes.append({
                "name": index_name,
                "unique": bool(idx[2]),
                "columns": columns
            })
        return indexes
    
    def get_table_schema(self, table_name: str) -> str:
        """
        테이블의 스키마 정보를 반환
        
        Args:
            table_name: 테이블 이름
            
        Returns:
            테이블 스키마 SQL
        """
        self.cursor.execute(f"SELECT sql FROM sqlite_master WHERE type='table' AND name='{table_name}'")
        result = self.cursor.fetchone()
        return result[0] if result else ""
    
    def analyze_database(self) -> Dict[str, Any]:
        """
        데이터베이스의 모든 테이블 및 컬럼 정보를 분석하여 트리 구조로 반환
        
        Returns:
            데이터베이스 트리 구조
        """
        self.tree_data = {
            "database": os.path.basename(self.db_path),
            "tables": {}
        }
        
        tables = self.get_tables()
        for table_name in tables:
            table_info = {
                "schema": self.get_table_schema(table_name),
                "columns": self.get_table_info(table_name),
                "foreign_keys": self.get_foreign_keys(table_name),
                "indexes": self.get_indexes(table_name)
            }
            self.tree_data["tables"][table_name] = table_info
        
        return self.tree_data
    
    def save_to_json(self, output_path: str) -> None:
        """
        분석 결과를 JSON 파일로 저장
        
        Args:
            output_path: 출력 파일 경로
        """
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(self.tree_data, f, indent=2, ensure_ascii=False)
        print(f"분석 결과가 '{output_path}'에 저장되었습니다.")
    
    def save_to_html(self, output_path: str) -> None:
        """
        분석 결과를 HTML 파일로 저장 (트리 형태로 시각화)
        
        Args:
            output_path: 출력 파일 경로
        """
        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <title>SQLite Database Tree: {os.path.basename(self.db_path)}</title>
            <style>
                body {{ font-family: Arial, sans-serif; margin: 20px; }}
                .tree {{ margin-left: 20px; }}
                .tree ul {{ padding-left: 20px; list-style-type: none; }}
                .tree li {{ position: relative; padding: 5px 0; }}
                .tree li::before {{ content: "├─"; position: absolute; left: -15px; }}
                .tree li:last-child::before {{ content: "└─"; }}
                .collapsible {{ cursor: pointer; user-select: none; }}
                .collapsible::before {{ content: "▼"; margin-right: 5px; }}
                .collapsible.collapsed::before {{ content: "▶"; }}
                .content {{ display: block; }}
                .content.collapsed {{ display: none; }}
                .table-name {{ font-weight: bold; color: #2c3e50; }}
                .column-name {{ color: #3498db; }}
                .column-type {{ color: #e74c3c; }}
                .column-pk {{ color: #27ae60; font-weight: bold; }}
                .column-fk {{ color: #f39c12; }}
                .index-name {{ color: #8e44ad; }}
                .foreign-key {{ color: #f39c12; }}
            </style>
        </head>
        <body>
            <h1>SQLite Database Tree: {os.path.basename(self.db_path)}</h1>
            <div class="tree">
                <ul>
                    <li>
                        <span class="collapsible">Database: {os.path.basename(self.db_path)}</span>
                        <ul class="content">
        """
        
        for table_name, table_info in self.tree_data["tables"].items():
            html_content += f"""
                            <li>
                                <span class="collapsible table-name">{table_name}</span>
                                <ul class="content">
                                    <li>
                                        <span class="collapsible">Schema</span>
                                        <div class="content">
                                            <pre>{table_info["schema"]}</pre>
                                        </div>
                                    </li>
                                    <li>
                                        <span class="collapsible">Columns ({len(table_info["columns"])})</span>
                                        <ul class="content">
            """
            
            for column in table_info["columns"]:
                pk_class = "column-pk" if column["pk"] else ""
                html_content += f"""
                                            <li>
                                                <span class="column-name {pk_class}">{column["name"]}</span>
                                                <span class="column-type">({column["type"]})</span>
                                                {f'<span class="column-pk">PRIMARY KEY</span>' if column["pk"] else ''}
                                                {f'<span>NOT NULL</span>' if column["notnull"] else ''}
                                                {f'<span>DEFAULT {column["default_value"]}</span>' if column["default_value"] else ''}
                                            </li>
                """
            
            html_content += """
                                        </ul>
                                    </li>
            """
            
            if table_info["foreign_keys"]:
                html_content += f"""
                                    <li>
                                        <span class="collapsible">Foreign Keys ({len(table_info["foreign_keys"])})</span>
                                        <ul class="content">
                """
                
                for fk in table_info["foreign_keys"]:
                    html_content += f"""
                                            <li>
                                                <span class="foreign-key">{fk["from"]} → {fk["table"]}.{fk["to"]}</span>
                                                <span>ON UPDATE {fk["on_update"]} ON DELETE {fk["on_delete"]}</span>
                                            </li>
                    """
                
                html_content += """
                                        </ul>
                                    </li>
                """
            
            if table_info["indexes"]:
                html_content += f"""
                                    <li>
                                        <span class="collapsible">Indexes ({len(table_info["indexes"])})</span>
                                        <ul class="content">
                """
                
                for idx in table_info["indexes"]:
                    html_content += f"""
                                            <li>
                                                <span class="index-name">{idx["name"]}</span>
                                                <span>{'UNIQUE' if idx["unique"] else ''}</span>
                                                <span>({', '.join(idx["columns"])})</span>
                                            </li>
                    """
                
                html_content += """
                                        </ul>
                                    </li>
                """
            
            html_content += """
                                </ul>
                            </li>
            """
        
        html_content += """
                        </ul>
                    </li>
                </ul>
            </div>
            
            <script>
                document.querySelectorAll('.collapsible').forEach(item => {
                    item.addEventListener('click', event => {
                        event.stopPropagation();
                        const content = item.nextElementSibling;
                        if (content && content.classList.contains('content')) {
                            content.classList.toggle('collapsed');
                            item.classList.toggle('collapsed');
                        }
                    });
                });
            </script>
        </body>
        </html>
        """
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
        
        print(f"분석 결과가 '{output_path}'에 저장되었습니다.")
    
    def run(self, output_json: str, output_html: Optional[str] = None) -> None:
        """
        데이터베이스 분석 및 결과 저장 실행
        
        Args:
            output_json: JSON 출력 파일 경로
            output_html: HTML 출력 파일 경로 (선택 사항)
        """
        try:
            self.connect()
            self.analyze_database()
            self.save_to_json(output_json)
            
            if output_html:
                self.save_to_html(output_html)
        finally:
            self.disconnect()


def main():
    parser = argparse.ArgumentParser(description='SQLite 데이터베이스 테이블 및 컬럼 정보를 트리 형태로 분석합니다.')
    parser.add_argument('db_path', help='SQLite 데이터베이스 파일 경로')
    parser.add_argument('--json', '-j', default='db_tree.json', help='JSON 출력 파일 경로 (기본값: db_tree.json)')
    # parser.add_argument('--html', '-h', help='HTML 출력 파일 경로 (선택 사항)')
    
    args = parser.parse_args()
    
    analyzer = SQLiteTreeAnalyzer(args.db_path)
    analyzer.run(args.json)


if __name__ == "__main__":
    main() 