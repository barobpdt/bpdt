import sys
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                            QHBoxLayout, QLabel, QLineEdit, QPushButton, 
                            QTextEdit, QGroupBox)
from PyQt5.QtCore import Qt, QEvent
from PyQt5.QtGui import QFont

class KoreanInputDetector(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("한글 입력 감지기")
        self.setGeometry(100, 100, 600, 400)
        
        # 중앙 위젯 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 메인 레이아웃
        main_layout = QVBoxLayout(central_widget)
        
        # 입력 영역
        input_group = QGroupBox("한글 입력")
        input_layout = QVBoxLayout(input_group)
        
        # 입력 필드
        self.input_field = QLineEdit()
        self.input_field.setPlaceholderText("여기에 한글을 입력하세요...")
        self.input_field.installEventFilter(self)  # 이벤트 필터 설치
        input_layout.addWidget(self.input_field)
        
        # 입력 힌트
        hint_label = QLabel("한글 입력 시 자음 단위로 변경이 감지됩니다.")
        hint_label.setStyleSheet("color: gray; font-style: italic;")
        input_layout.addWidget(hint_label)
        
        main_layout.addWidget(input_group)
        
        # 로그 영역
        log_group = QGroupBox("입력 로그")
        log_layout = QVBoxLayout(log_group)
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        log_layout.addWidget(self.log_text)
        
        # 로그 지우기 버튼
        clear_button = QPushButton("로그 지우기")
        clear_button.clicked.connect(self.clear_log)
        log_layout.addWidget(clear_button)
        
        main_layout.addWidget(log_group)
        
        # 상태 표시줄
        self.status_label = QLabel("준비됨")
        main_layout.addWidget(self.status_label)
        
        # 이전 입력 상태 저장
        self.previous_text = ""
        self.previous_composition = ""
    
    def eventFilter(self, obj, event):
        """이벤트 필터 - 입력 이벤트 감지"""
        if obj == self.input_field:
            # 입력 이벤트 감지
            if event.type() == QEvent.InputMethod:
                # 현재 입력 중인 텍스트 (조합 중인 텍스트)
                current_composition = event.commitString()
                
                # 현재 입력 필드의 전체 텍스트
                current_text = self.input_field.text()
                
                # 이전 상태와 비교하여 변경 감지
                if current_composition != self.previous_composition:
                    self.log_input_event("조합 변경", current_composition, self.previous_composition)
                    self.previous_composition = current_composition
                
                if current_text != self.previous_text:
                    self.log_input_event("텍스트 변경", current_text, self.previous_text)
                    self.previous_text = current_text
                
                # 상태 업데이트
                self.status_label.setText(f"현재 입력: {current_text} (조합 중: {current_composition})")
        
        # 기본 이벤트 처리 계속
        return super().eventFilter(obj, event)
    
    def log_input_event(self, event_type, current, previous):
        """입력 이벤트 로깅"""
        log_entry = f"[{event_type}] 이전: '{previous}' → 현재: '{current}'"
        self.log_text.append(log_entry)
    
    def clear_log(self):
        """로그 지우기"""
        self.log_text.clear()
        self.log_text.append("로그가 지워졌습니다.")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # 모던한 스타일 적용
    
    # 폰트 설정
    font = app.font()
    font.setPointSize(10)
    app.setFont(font)
    
    window = KoreanInputDetector()
    window.show()
    sys.exit(app.exec_()) 