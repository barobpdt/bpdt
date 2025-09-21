import sys
import pygame
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout
from PyQt5.QtCore import QThread, pyqtSignal, QTimer
from PyQt5.QtGui import QPainter, QImage

class PygameWorker(QThread):
    """백그라운드에서 데이터를 계산하는 QThread 서브클래스"""
    update_data_signal = pyqtSignal(int) # 메인 스레드로 데이터를 보낼 시그널

    def __init__(self):
        super().__init__()
        self.data_value = 0
        self.running = True

    def run(self):
        """스레드가 시작되면 실행되는 메서드"""
        while self.running:
            self.data_value += 1
            if self.data_value > 300:
                self.data_value = 0
            # 계산된 데이터를 시그널로 메인 스레드에 보냄
            self.update_data_signal.emit(self.data_value)
            self.msleep(10)  # 10ms 대기

    def stop(self):
        self.running = False

class PygameWidget(QWidget):
    """PyQt 창에 Pygame 화면을 표시하는 위젯"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFixedSize(640, 480)
        self.pygame_surface = pygame.Surface((640, 480))
        self.data_from_thread = 0

        # PygameWorker 스레드 시작
        self.worker_thread = PygameWorker()
        self.worker_thread.update_data_signal.connect(self.update_from_thread)
        self.worker_thread.start()

        # 화면 업데이트를 위한 타이머
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update)
        self.timer.start(16) # 약 60 FPS

    def update_from_thread(self, value):
        """시그널을 통해 받은 데이터를 저장"""
        self.data_from_thread = value

    def paintEvent(self, event):
        """PyQt 페인트 이벤트 핸들러"""
        self.render_pygame()
        painter = QPainter(self)
        qimage = QImage(self.pygame_surface.get_buffer().raw, 640, 480, QImage.Format_RGB32)
        painter.drawImage(0, 0, qimage)

    def render_pygame(self):
        """Pygame 렌더링 로직"""
        self.pygame_surface.fill((255, 255, 255))
        pygame.draw.circle(self.pygame_surface, (255, 0, 0), (self.data_from_thread, 240), 20)

    def closeEvent(self, event):
        """창이 닫힐 때 스레드 정리"""
        self.worker_thread.stop()
        self.worker_thread.wait()
        event.accept()

if __name__ == '__main__':
    # PyQt 애플리케이션 초기화
    app = QApplication(sys.argv)
    pygame.init()

    # PyQt 윈도우 생성
    window = QWidget()
    window.setWindowTitle("PyQt + QThread + Pygame")
    layout = QVBoxLayout()
    
    pygame_widget = PygameWidget()
    layout.addWidget(pygame_widget)
    window.setLayout(layout)
    
    window.show()
    sys.exit(app.exec_())
