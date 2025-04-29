import sys
import random
import json
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QGridLayout, 
                            QLabel, QPushButton, QTableWidget, QTableWidgetItem,
                            QHeaderView, QVBoxLayout, QHBoxLayout, QLineEdit,
                            QComboBox, QMessageBox, QFileDialog)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QFont, QColor, QPalette

class AddressBookGrid(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("주소록 그리드 예제")
        self.setGeometry(100, 100, 900, 600)
        
        # 중앙 위젯 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 메인 레이아웃
        main_layout = QVBoxLayout(central_widget)
        
        # 검색 및 필터 영역
        search_layout = QHBoxLayout()
        
        # 검색 입력 필드
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("이름, 전화번호, 이메일로 검색...")
        self.search_input.textChanged.connect(self.filter_contacts)
        search_layout.addWidget(self.search_input)
        
        # 필터 콤보박스
        self.filter_combo = QComboBox()
        self.filter_combo.addItems(["전체", "가족", "친구", "직장", "기타"])
        self.filter_combo.currentTextChanged.connect(self.filter_contacts)
        search_layout.addWidget(self.filter_combo)
        
        # 검색 버튼
        search_button = QPushButton("검색")
        search_button.clicked.connect(self.filter_contacts)
        search_layout.addWidget(search_button)
        
        # 새 연락처 추가 버튼
        add_button = QPushButton("새 연락처")
        add_button.clicked.connect(self.add_contact)
        search_layout.addWidget(add_button)
        
        # JSON 다운로드 버튼
        json_button = QPushButton("JSON 다운로드")
        json_button.clicked.connect(self.download_json)
        search_layout.addWidget(json_button)
        
        main_layout.addLayout(search_layout)
        
        # 테이블 위젯 설정
        self.table = QTableWidget()
        self.table.setColumnCount(5)
        self.table.setHorizontalHeaderLabels(["이름", "전화번호", "이메일", "주소", "그룹"])
        
        # 테이블 헤더 설정
        header = self.table.horizontalHeader()
        header.setSectionResizeMode(0, QHeaderView.ResizeToContents)  # 이름
        header.setSectionResizeMode(1, QHeaderView.ResizeToContents)  # 전화번호
        header.setSectionResizeMode(2, QHeaderView.Stretch)           # 이메일
        header.setSectionResizeMode(3, QHeaderView.Stretch)           # 주소
        header.setSectionResizeMode(4, QHeaderView.ResizeToContents)  # 그룹
        
        # 테이블 행 선택 모드 설정
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setSelectionMode(QTableWidget.SingleSelection)
        
        # 테이블 더블클릭 이벤트 연결
        self.table.itemDoubleClicked.connect(self.edit_contact)
        
        main_layout.addWidget(self.table)
        
        # 상태 표시줄
        self.status_label = QLabel("총 0개의 연락처")
        main_layout.addWidget(self.status_label)
        
        # 샘플 주소록 데이터 생성
        self.contacts = self.generate_sample_contacts()
        
        # 테이블에 데이터 표시
        self.display_contacts(self.contacts)
        
    def generate_sample_contacts(self):
        """샘플 주소록 데이터 생성"""
        first_names = ["김", "이", "박", "최", "정", "강", "조", "윤", "장", "임"]
        last_names = ["민준", "서연", "지우", "도윤", "서현", "준호", "수아", "현우", "지민", "소연"]
        domains = ["gmail.com", "naver.com", "daum.net", "hanmail.net", "nate.com"]
        groups = ["가족", "친구", "직장", "기타"]
        cities = ["서울", "부산", "인천", "대구", "대전", "광주", "수원", "울산", "고양", "용인"]
        streets = ["강남구", "서초구", "송파구", "강서구", "마포구", "영등포구", "중구", "종로구", "성동구", "광진구"]
        
        contacts = []
        for i in range(20):  # 20개의 샘플 연락처 생성
            name = random.choice(first_names) + random.choice(last_names)
            phone = f"010-{random.randint(1000, 9999)}-{random.randint(1000, 9999)}"
            email = f"{name}{random.randint(1, 999)}@{random.choice(domains)}"
            address = f"{random.choice(cities)} {random.choice(streets)} {random.randint(1, 100)}번지"
            group = random.choice(groups)
            
            contacts.append({
                "name": name,
                "phone": phone,
                "email": email,
                "address": address,
                "group": group
            })
        
        return contacts
    
    def display_contacts(self, contacts):
        """연락처 목록을 테이블에 표시"""
        self.table.setRowCount(len(contacts))
        
        for row, contact in enumerate(contacts):
            # 이름
            name_item = QTableWidgetItem(contact["name"])
            self.table.setItem(row, 0, name_item)
            
            # 전화번호
            phone_item = QTableWidgetItem(contact["phone"])
            self.table.setItem(row, 1, phone_item)
            
            # 이메일
            email_item = QTableWidgetItem(contact["email"])
            self.table.setItem(row, 2, email_item)
            
            # 주소
            address_item = QTableWidgetItem(contact["address"])
            self.table.setItem(row, 3, address_item)
            
            # 그룹
            group_item = QTableWidgetItem(contact["group"])
            self.table.setItem(row, 4, group_item)
        
        # 상태 표시줄 업데이트
        self.status_label.setText(f"총 {len(contacts)}개의 연락처")
    
    def filter_contacts(self):
        """검색어와 필터에 따라 연락처 필터링"""
        search_text = self.search_input.text().lower()
        filter_group = self.filter_combo.currentText()
        
        filtered_contacts = []
        
        for contact in self.contacts:
            # 검색어 필터링
            if search_text and not (
                search_text in contact["name"].lower() or
                search_text in contact["phone"].lower() or
                search_text in contact["email"].lower()
            ):
                continue
            
            # 그룹 필터링
            if filter_group != "전체" and contact["group"] != filter_group:
                continue
            
            filtered_contacts.append(contact)
        
        self.display_contacts(filtered_contacts)
    
    def add_contact(self):
        """새 연락처 추가 (간단한 메시지 박스로 대체)"""
        QMessageBox.information(self, "새 연락처 추가", "새 연락처 추가 기능은 구현되지 않았습니다.")
    
    def edit_contact(self, item):
        """연락처 편집 (간단한 메시지 박스로 대체)"""
        row = item.row()
        name = self.table.item(row, 0).text()
        QMessageBox.information(self, "연락처 편집", f"{name}의 연락처 편집 기능은 구현되지 않았습니다.")
    
    def download_json(self):
        """현재 표시된 연락처 목록을 JSON 파일로 다운로드"""
        # 현재 표시된 연락처 목록 가져오기
        current_contacts = []
        for row in range(self.table.rowCount()):
            contact = {
                "name": self.table.item(row, 0).text(),
                "phone": self.table.item(row, 1).text(),
                "email": self.table.item(row, 2).text(),
                "address": self.table.item(row, 3).text(),
                "group": self.table.item(row, 4).text()
            }
            current_contacts.append(contact)
        
        # 파일 저장 대화상자 표시
        file_path = 'c:/temp/sample_addr.json'
        
        if file_path:
            try:
                # JSON 파일로 저장
                with open(file_path, 'w', encoding='utf-8') as f:
                    json.dump(current_contacts, f, ensure_ascii=False, indent=2)
                
                
            except Exception as e:
                print(f"exception {e}")
                

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # 모던한 스타일 적용
    
    # 폰트 설정
    font = app.font()
    font.setPointSize(10)
    app.setFont(font)
    
    window = AddressBookGrid()
    window.show()
    sys.exit(app.exec_()) 