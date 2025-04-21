import sys
import re
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                            QHBoxLayout, QLabel, QLineEdit, QPushButton, 
                            QTextEdit, QListWidget, QGroupBox, QMessageBox)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont

class KoreanInitialSearch(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("한글 초성 검색")
        self.setGeometry(100, 100, 800, 600)
        
        # 중앙 위젯 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 메인 레이아웃
        main_layout = QVBoxLayout(central_widget)
        
        # 검색 영역
        search_group = QGroupBox("초성 검색")
        search_layout = QVBoxLayout(search_group)
        
        # 검색 입력 필드
        search_input_layout = QHBoxLayout()
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("초성을 입력하세요 (예: ㅇㄴㅎㅅㅇ)")
        self.search_input.textChanged.connect(self.search)
        search_input_layout.addWidget(self.search_input)
        
        # 검색 버튼
        search_button = QPushButton("검색")
        search_button.clicked.connect(self.search)
        search_input_layout.addWidget(search_button)
        
        search_layout.addLayout(search_input_layout)
        
        # 검색 힌트
        hint_label = QLabel("초성만 입력하여 검색할 수 있습니다. 예: 'ㅇㄴ'으로 검색하면 '안녕'이 포함된 결과가 표시됩니다.")
        hint_label.setStyleSheet("color: gray; font-style: italic;")
        hint_label.setWordWrap(True)
        search_layout.addWidget(hint_label)
        
        main_layout.addWidget(search_group)
        
        # 데이터 관리 영역
        data_group = QGroupBox("데이터 관리")
        data_layout = QVBoxLayout(data_group)
        
        # 데이터 입력 영역
        data_input_layout = QHBoxLayout()
        self.data_input = QLineEdit()
        self.data_input.setPlaceholderText("새 데이터 입력")
        data_input_layout.addWidget(self.data_input)
        
        # 추가 버튼
        add_button = QPushButton("추가")
        add_button.clicked.connect(self.add_data)
        data_input_layout.addWidget(add_button)
        
        data_layout.addLayout(data_input_layout)
        
        # 샘플 데이터 추가 버튼
        sample_button = QPushButton("샘플 데이터 추가")
        sample_button.clicked.connect(self.add_sample_data)
        data_layout.addWidget(sample_button)
        
        main_layout.addWidget(data_group)
        
        # 결과 영역
        result_group = QGroupBox("검색 결과")
        result_layout = QVBoxLayout(result_group)
        
        self.result_list = QListWidget()
        result_layout.addWidget(self.result_list)
        
        # 결과 항목 더블클릭 이벤트 연결
        self.result_list.itemDoubleClicked.connect(self.show_details)
        
        main_layout.addWidget(result_group)
        
        # 상태 표시줄
        self.status_label = QLabel("준비됨")
        main_layout.addWidget(self.status_label)
        
        # 데이터 저장소
        self.data_list = []
        
        # 초기 샘플 데이터 추가
        self.add_sample_data()
    
    def extract_initials(self, text):
        """한글 텍스트에서 초성을 추출하는 함수"""
        # 한글 초성 매핑
        initial_map = {
            'ㄱ': '가', 'ㄲ': '까', 'ㄴ': '나', 'ㄷ': '다', 'ㄸ': '따',
            'ㄹ': '라', 'ㅁ': '마', 'ㅂ': '바', 'ㅃ': '빠', 'ㅅ': '사',
            'ㅆ': '싸', 'ㅇ': '아', 'ㅈ': '자', 'ㅉ': '짜', 'ㅊ': '차',
            'ㅋ': '카', 'ㅌ': '타', 'ㅍ': '파', 'ㅎ': '하'
        }
        
        # 초성 추출 정규식
        initial_pattern = re.compile('[ㄱ-ㅎ]')
        
        # 초성만 있는 경우 처리
        if all(c in initial_map for c in text):
            return text
        
        # 초성 추출
        result = ""
        for char in text:
            if '가' <= char <= '힣':  # 한글 음절 범위
                # 유니코드에서 초성 코드 계산
                initial_code = (ord(char) - ord('가')) // 588
                initial = chr(ord('ㄱ') + initial_code)
                result += initial
            elif char in initial_map:  # 이미 초성인 경우
                result += char
        
        return result
    
    def search(self):
        """초성 검색 수행"""
        search_text = self.search_input.text().strip()
        
        if not search_text:
            self.result_list.clear()
            self.status_label.setText("검색어를 입력하세요")
            return
        
        # 검색어의 초성 추출
        search_initials = self.extract_initials(search_text)
        
        # 결과 필터링
        self.result_list.clear()
        for item in self.data_list:
            # 항목의 초성 추출
            item_initials = self.extract_initials(item)
            
            # 초성 검색어가 항목의 초성에 포함되는지 확인
            if search_initials in item_initials:
                self.result_list.addItem(item)
        
        # 상태 업데이트
        result_count = self.result_list.count()
        self.status_label.setText(f"검색 결과: {result_count}개 항목")
    
    def add_data(self):
        """새 데이터 추가"""
        new_data = self.data_input.text().strip()
        
        if not new_data:
            QMessageBox.warning(self, "경고", "데이터를 입력하세요")
            return
        
        # 중복 확인
        if new_data in self.data_list:
            QMessageBox.warning(self, "경고", "이미 존재하는 데이터입니다")
            return
        
        # 데이터 추가
        self.data_list.append(new_data)
        self.data_input.clear()
        
        # 상태 업데이트
        self.status_label.setText(f"데이터 추가됨: {new_data}")
        
        # 현재 검색어로 다시 검색
        self.search()
    
    def add_sample_data(self):
        """샘플 데이터 추가"""
        sample_data = [
            "안녕하세요",
            "반갑습니다",
            "파이썬 프로그래밍",
            "한글 초성 검색",
            "인공지능과 머신러닝",
            "데이터 분석과 시각화",
            "웹 개발과 프론트엔드",
            "백엔드 개발과 API",
            "모바일 앱 개발",
            "클라우드 컴퓨팅",
            "사이버 보안",
            "블록체인 기술",
            "가상현실과 증강현실",
            "사물인터넷과 스마트홈",
            "로봇공학과 자동화"
        ]
        
        # 기존 데이터 초기화
        self.data_list = []
        
        # 샘플 데이터 추가
        for data in sample_data:
            if data not in self.data_list:
                self.data_list.append(data)
        
        # 상태 업데이트
        self.status_label.setText(f"샘플 데이터 {len(sample_data)}개 추가됨")
        
        # 현재 검색어로 다시 검색
        self.search()
    
    def show_details(self, item):
        """결과 항목 상세 정보 표시"""
        text = item.text()
        initials = self.extract_initials(text)
        
        QMessageBox.information(
            self,
            "상세 정보",
            f"텍스트: {text}\n초성: {initials}"
        )

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # 모던한 스타일 적용
    
    # 폰트 설정
    font = app.font()
    font.setPointSize(10)
    app.setFont(font)
    
    window = KoreanInitialSearch()
    window.show()
    sys.exit(app.exec_()) 