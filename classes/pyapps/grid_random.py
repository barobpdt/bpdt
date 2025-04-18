import sys
import random
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QGridLayout, 
                            QLabel, QPushButton, QCheckBox, QLineEdit, 
                            QComboBox, QSpinBox, QProgressBar, QFrame)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QFont, QColor, QPalette

class GridExample(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("PyQt 그리드 예제 - 랜덤 데이터")
        self.setGeometry(100, 100, 800, 600)
        
        # 중앙 위젯 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 메인 레이아웃
        main_layout = QGridLayout(central_widget)
        main_layout.setSpacing(10)  # 위젯 간 간격 설정
        
        # 그리드 크기 설정
        self.rows = 5
        self.cols = 4
        
        # 위젯 생성 및 배치
        self.widgets = []
        self.create_grid()
        
        # 타이머 설정 (데이터 업데이트용)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_random_data)
        self.timer.start(2000)  # 2초마다 업데이트
        
        # 초기 데이터 설정
        self.update_random_data()
        
    def create_grid(self):
        """그리드 위젯 생성 및 배치"""
        # 헤더 생성
        for col in range(self.cols):
            header = QLabel(f"열 {col+1}")
            header.setAlignment(Qt.AlignCenter)
            header.setStyleSheet("background-color: #4a86e8; color: white; padding: 5px; font-weight: bold;")
            self.centralWidget().layout().addWidget(header, 0, col)
        
        for row in range(self.rows):
            row_header = QLabel(f"행 {row+1}")
            row_header.setAlignment(Qt.AlignCenter)
            row_header.setStyleSheet("background-color: #4a86e8; color: white; padding: 5px; font-weight: bold;")
            self.centralWidget().layout().addWidget(row_header, row+1, 0)
        
        # 그리드 셀 생성
        for row in range(self.rows):
            row_widgets = []
            for col in range(self.cols):
                # 랜덤 위젯 타입 선택
                widget_type = random.choice(['label', 'button', 'checkbox', 'lineedit', 'combobox', 'spinbox', 'progressbar'])
                widget = self.create_random_widget(widget_type, row, col)
                self.centralWidget().layout().addWidget(widget, row+1, col+1)
                row_widgets.append(widget)
            self.widgets.append(row_widgets)
    
    def create_random_widget(self, widget_type, row, col):
        """랜덤 위젯 생성"""
        if widget_type == 'label':
            widget = QLabel(f"라벨 {row+1}-{col+1}")
            widget.setAlignment(Qt.AlignCenter)
            widget.setStyleSheet("background-color: #f0f0f0; padding: 5px; border: 1px solid #ddd;")
            return widget
        
        elif widget_type == 'button':
            widget = QPushButton(f"버튼 {row+1}-{col+1}")
            widget.setStyleSheet("background-color: #4CAF50; color: white; padding: 5px;")
            return widget
        
        elif widget_type == 'checkbox':
            widget = QCheckBox(f"체크박스 {row+1}-{col+1}")
            widget.setChecked(random.choice([True, False]))
            return widget
        
        elif widget_type == 'lineedit':
            widget = QLineEdit(f"텍스트 {row+1}-{col+1}")
            return widget
        
        elif widget_type == 'combobox':
            widget = QComboBox()
            widget.addItems([f"옵션 {i+1}" for i in range(5)])
            widget.setCurrentIndex(random.randint(0, 4))
            return widget
        
        elif widget_type == 'spinbox':
            widget = QSpinBox()
            widget.setRange(0, 100)
            widget.setValue(random.randint(0, 100))
            return widget
        
        elif widget_type == 'progressbar':
            widget = QProgressBar()
            widget.setRange(0, 100)
            widget.setValue(random.randint(0, 100))
            return widget
    
    def update_random_data(self):
        """랜덤 데이터로 위젯 업데이트"""
        for row in range(self.rows):
            for col in range(self.cols):
                widget = self.widgets[row][col]
                
                if isinstance(widget, QLabel):
                    # 랜덤 텍스트 생성
                    random_text = ''.join(random.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') for _ in range(8))
                    widget.setText(random_text)
                
                elif isinstance(widget, QPushButton):
                    # 버튼 텍스트 업데이트
                    random_text = ''.join(random.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') for _ in range(6))
                    widget.setText(random_text)
                
                elif isinstance(widget, QCheckBox):
                    # 체크박스 상태 랜덤 변경
                    widget.setChecked(random.choice([True, False]))
                
                elif isinstance(widget, QLineEdit):
                    # 텍스트 필드 내용 업데이트
                    random_text = ''.join(random.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') for _ in range(10))
                    widget.setText(random_text)
                
                elif isinstance(widget, QComboBox):
                    # 콤보박스 선택 항목 랜덤 변경
                    widget.setCurrentIndex(random.randint(0, widget.count()-1))
                
                elif isinstance(widget, QSpinBox):
                    # 스핀박스 값 랜덤 변경
                    widget.setValue(random.randint(0, 100))
                
                elif isinstance(widget, QProgressBar):
                    # 프로그레스바 값 랜덤 변경
                    widget.setValue(random.randint(0, 100))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # 모던한 스타일 적용
    
    # 폰트 설정
    font = app.font()
    font.setPointSize(10)
    app.setFont(font)
    
    window = GridExample()
    window.show()
    sys.exit(app.exec_()) 