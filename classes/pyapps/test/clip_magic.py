import sys
import os
import json
import time
import re
import threading
from datetime import datetime
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, 
                            QPushButton, QLabel, QLineEdit, QTextEdit, QTreeWidget, 
                            QTreeWidgetItem, QSplitter, QFrame, QFileDialog, QMessageBox,
                            QHeaderView, QGroupBox)
from PyQt5.QtCore import Qt, QTimer, pyqtSignal, QObject
from PyQt5.QtGui import QFont, QIcon

class ClipboardSignals(QObject):
    """클립보드 변경 신호를 위한 클래스"""
    clipboard_changed = pyqtSignal(str, str)  # 시간, 내용

class ClipboardMonitor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("클립보드 모니터 (PyQt)")
        self.setGeometry(100, 100, 900, 700)
        self.setMinimumSize(700, 500)
        
        # 변수 초기화
        self.is_monitoring = False
        self.clipboard_history = []
        self.last_clipboard = None # pyperclip.paste()
        self.monitor_thread = None
        self.filter_pattern = ""
        self.save_path = "clipboard_history.json"
        self.signals = ClipboardSignals()
        
        # UI 구성
        self.init_ui()
        
        # 신호 연결
        self.signals.clipboard_changed.connect(self.update_history_ui)
        
        # 기존 히스토리 불러오기
        self.load_history()
        
    def init_ui(self):
        # 중앙 위젯 설정
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 메인 레이아웃
        main_layout = QVBoxLayout(central_widget)
        
        # 상단 컨트롤 영역
        control_group = QGroupBox("컨트롤")
        control_layout = QHBoxLayout(control_group)
        
        # 모니터링 시작/중지 버튼
        self.monitor_button = QPushButton("모니터링 시작")
        self.monitor_button.clicked.connect(self.toggle_monitoring)
        control_layout.addWidget(self.monitor_button)
        
        # 필터 입력
        filter_label = QLabel("필터:")
        control_layout.addWidget(filter_label)
        
        self.filter_entry = QLineEdit()
        self.filter_entry.setPlaceholderText("정규식 입력")
        self.filter_entry.returnPressed.connect(self.apply_filter)
        control_layout.addWidget(self.filter_entry)
        
        # 필터 적용 버튼
        filter_button = QPushButton("적용")
        filter_button.clicked.connect(self.apply_filter)
        control_layout.addWidget(filter_button)
        
        # 필터 초기화 버튼
        reset_button = QPushButton("초기화")
        reset_button.clicked.connect(self.reset_filter)
        control_layout.addWidget(reset_button)
        
        # 저장 버튼
        save_button = QPushButton("저장")
        save_button.clicked.connect(self.save_history)
        control_layout.addWidget(save_button)
        
        # 불러오기 버튼
        load_button = QPushButton("불러오기")
        load_button.clicked.connect(self.load_history)
        control_layout.addWidget(load_button)
        
        # 지우기 버튼
        clear_button = QPushButton("지우기")
        clear_button.clicked.connect(self.clear_history)
        control_layout.addWidget(clear_button)
        
        # 상태 표시 레이블
        self.status_label = QLabel("대기 중")
        control_layout.addWidget(self.status_label)
        
        main_layout.addWidget(control_group)
        
        # 스플리터 (클립보드 내용 및 히스토리)
        splitter = QSplitter(Qt.Vertical)
        
        # 클립보드 내용 표시 영역
        content_group = QGroupBox("클립보드 내용")
        content_layout = QVBoxLayout(content_group)
        
        self.content_text = QTextEdit()
        self.content_text.setReadOnly(True)
        content_layout.addWidget(self.content_text)
        
        splitter.addWidget(content_group)
        
        # 히스토리 표시 영역
        history_group = QGroupBox("클립보드 히스토리")
        history_layout = QVBoxLayout(history_group)
        
        self.history_tree = QTreeWidget()
        self.history_tree.setHeaderLabels(["시간", "내용"])
        self.history_tree.setColumnWidth(0, 150)
        self.history_tree.setColumnWidth(1, 500)
        self.history_tree.header().setSectionResizeMode(1, QHeaderView.Stretch)
        self.history_tree.itemSelectionChanged.connect(self.on_select_history)
        history_layout.addWidget(self.history_tree)
        
        splitter.addWidget(history_group)
        
        # 스플리터 비율 설정
        splitter.setSizes([300, 400])
        
        main_layout.addWidget(splitter)
        
    def toggle_monitoring(self):
        if self.is_monitoring:
            self.stop_monitoring()
        else:
            self.start_monitoring()
    
    def start_monitoring(self):
        self.is_monitoring = True
        self.monitor_button.setText("모니터링 중지")
        self.status_label.setText("모니터링 중...")
        
        # 모니터링 스레드 시작
        self.monitor_thread = threading.Thread(target=self.monitor_clipboard, daemon=True)
        self.monitor_thread.start()
    
    def stop_monitoring(self):
        self.is_monitoring = False
        self.monitor_button.setText("모니터링 시작")
        self.status_label.setText("대기 중")
    
    def monitor_clipboard(self):
        while self.is_monitoring:
            try:
                # current_clipboard = pyperclip.paste()
                
                # 클립보드 내용이 변경되었는지 확인
                if current_clipboard != self.last_clipboard:
                    # 필터 적용
                    if self.filter_pattern and not re.search(self.filter_pattern, current_clipboard, re.IGNORECASE):
                        self.last_clipboard = current_clipboard
                        time.sleep(0.1)
                        continue
                    
                    # 히스토리에 추가
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    self.clipboard_history.append({
                        "time": timestamp,
                        "content": current_clipboard
                    })
                    
                    # UI 업데이트 (신호 사용)
                    self.signals.clipboard_changed.emit(timestamp, current_clipboard)
                    
                    # 마지막 클립보드 내용 업데이트
                    self.last_clipboard = current_clipboard
                
                # CPU 사용량 감소를 위한 짧은 대기
                time.sleep(0.1)
                
            except Exception as e:
                print(f"모니터링 오류: {str(e)}")
                time.sleep(0.5)
    
    def update_history_ui(self, timestamp, content):
        # 트리뷰에 항목 추가
        item = QTreeWidgetItem([timestamp, self.truncate_content(content)])
        self.history_tree.insertTopLevelItem(0, item)
        
        # 상태 업데이트
        self.status_label.setText(f"마지막 업데이트: {timestamp}")
    
    def truncate_content(self, content, max_length=50):
        """긴 내용을 표시하기 위해 잘라내기"""
        if len(content) > max_length:
            return content[:max_length] + "..."
        return content
    
    def on_select_history(self):
        """히스토리 항목 선택 시 호출되는 함수"""
        selected_items = self.history_tree.selectedItems()
        if not selected_items:
            return
        
        # 선택한 항목의 인덱스 찾기
        item = selected_items[0]
        item_index = self.history_tree.indexOfTopLevelItem(item)
        
        # 실제 인덱스 계산 (트리뷰는 역순으로 표시)
        real_index = len(self.clipboard_history) - 1 - item_index
        
        if 0 <= real_index < len(self.clipboard_history):
            # 내용 표시
            content = self.clipboard_history[real_index]["content"]
            self.content_text.setText(content)
    
    def apply_filter(self):
        """필터 적용"""
        self.filter_pattern = self.filter_entry.text()
        self.update_filtered_history()
    
    def reset_filter(self):
        """필터 초기화"""
        self.filter_entry.clear()
        self.filter_pattern = ""
        self.update_filtered_history()
    
    def update_filtered_history(self):
        """필터링된 히스토리 업데이트"""
        # 트리뷰 초기화
        self.history_tree.clear()
        
        # 필터링된 항목 추가
        for item in reversed(self.clipboard_history):
            content = item["content"]
            if not self.filter_pattern or re.search(self.filter_pattern, content, re.IGNORECASE):
                tree_item = QTreeWidgetItem([item["time"], self.truncate_content(content)])
                self.history_tree.addTopLevelItem(tree_item)
    
    def save_history(self):
        """히스토리 저장"""
        try:
            file_path, _ = QFileDialog.getSaveFileName(
                self, "히스토리 저장", self.save_path, "JSON 파일 (*.json);;모든 파일 (*.*)"
            )
            
            if not file_path:
                return
                
            with open(file_path, 'w', encoding='utf-8') as f:
                json.dump(self.clipboard_history, f, ensure_ascii=False, indent=2)
            
            QMessageBox.information(self, "완료", f"클립보드 히스토리가 저장되었습니다: {file_path}")
            
        except Exception as e:
            QMessageBox.critical(self, "오류", f"히스토리 저장 중 오류가 발생했습니다: {str(e)}")
    
    def load_history(self):
        """히스토리 불러오기"""
        if not os.path.exists(self.save_path):
            return
        
        try:
            with open(self.save_path, 'r', encoding='utf-8') as f:
                self.clipboard_history = json.load(f)
            
            # UI 업데이트
            self.update_filtered_history()
            
        except Exception as e:
            QMessageBox.critical(self, "오류", f"히스토리 불러오기 중 오류가 발생했습니다: {str(e)}")
    
    def clear_history(self):
        """히스토리 지우기"""
        reply = QMessageBox.question(
            self, "확인", "클립보드 히스토리를 지우시겠습니까?",
            QMessageBox.Yes | QMessageBox.No, QMessageBox.No
        )
        
        if reply == QMessageBox.Yes:
            self.clipboard_history = []
            self.update_filtered_history()
            self.content_text.clear()
    
    def closeEvent(self, event):
        """창 닫기 이벤트 처리"""
        if self.is_monitoring:
            reply = QMessageBox.question(
                self, "확인", "모니터링 중입니다. 종료하시겠습니까?",
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No
            )
            
            if reply == QMessageBox.Yes:
                self.stop_monitoring()
                event.accept()
            else:
                event.ignore()
        else:
            event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # 모던한 스타일 적용
    
    # 폰트 설정
    font = app.font()
    font.setPointSize(10)
    app.setFont(font)
    
    window = ClipboardMonitor()
    window.show()
    sys.exit(app.exec_()) 