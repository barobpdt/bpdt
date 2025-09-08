##
def pygame_surface_to_qimage(surface):
    width, height = surface.get_size()
    raw_data = pygame.image.tostring(surface, 'RGBA') 
    if surface.get_bitsize() == 32:
        qimage_format = QImage.Format_ARGB32 # Assuming alpha channel is present
    elif surface.get_bitsize() == 24:
        qimage_format = QImage.Format_RGB888
    else:
        raise ValueError("Unsupported Pygame surface format for QImage conversion.")
    return QImage(raw_data, width, height, qimage_format)

# Example Usage:
if __name__ == '__main__':
    pygame.init()
    screen = pygame.display.set_mode((640, 480))
    pygame.display.set_caption("Pygame to QImage Conversion")

    # Create a sample Pygame Surface
    pygame_surface = pygame.Surface((200, 150), pygame.SRCALPHA)
    pygame_surface.fill((255, 0, 0, 128)) # Red with 50% alpha
    pygame.draw.circle(pygame_surface, (0, 255, 0), (100, 75), 50)

    # Convert Pygame Surface to QImage
    q_image = pygame_surface_to_qimage(pygame_surface)

    # You can now use q_image with PyQt5, e.g., display it in a QLabel
    # For demonstration, we'll save it to a file
    q_image.save("converted_image.png")
    print("Converted QImage saved as converted_image.png")

    pygame.quit()

##
from PyQt5 import QtGui
from PyQt5.QtWidgets import QWidget, QApplication, QLabel, QVBoxLayout
from PyQt5.QtGui import QPixmap
import sys
import cv2
from PyQt5.QtCore import pyqtSignal, pyqtSlot, Qt, QThread
import numpy as np

class VideoThread(QThread):
    change_pixmap_signal = pyqtSignal(np.ndarray)

    def run(self):
        # capture from web cam
        cap = cv2.VideoCapture(0)
        while True:
            ret, cv_img = cap.read()
            if ret:
                self.change_pixmap_signal.emit(cv_img)


class App(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Qt live label demo")
        self.disply_width = 640
        self.display_height = 480
        # create the label that holds the image
        self.image_label = QLabel(self)
        self.image_label.resize(self.disply_width, self.display_height)
        # create a text label
        self.textLabel = QLabel('Webcam')

        # create a vertical box layout and add the two labels
        vbox = QVBoxLayout()
        vbox.addWidget(self.image_label)
        vbox.addWidget(self.textLabel)
        # set the vbox layout as the widgets layout
        self.setLayout(vbox)

        # create the video capture thread
        self.thread = VideoThread()
        # connect its signal to the update_image slot
        self.thread.change_pixmap_signal.connect(self.update_image)
        # start the thread
        self.thread.start()

    @pyqtSlot(np.ndarray)
    def update_image(self, cv_img):
        """Updates the image_label with a new opencv image"""
        qt_img = self.convert_cv_qt(cv_img)
        self.image_label.setPixmap(qt_img)
    
    def convert_cv_qt(self, cv_img):
        """Convert from an opencv image to QPixmap"""
        rgb_image = cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB)
        h, w, ch = rgb_image.shape
        bytes_per_line = ch * w
        convert_to_Qt_format = QtGui.QImage(rgb_image.data, w, h, bytes_per_line, QtGui.QImage.Format_RGB888)
        p = convert_to_Qt_format.scaled(self.disply_width, self.display_height, Qt.KeepAspectRatio)
        return QPixmap.fromImage(p)
    
if __name__=="__main__":
    app = QApplication(sys.argv)
    a = App()
    a.show()
    sys.exit(app.exec_())

##
# This Python file uses the following encoding: utf-8
import datetime
import os
import sys
from pathlib import Path
import cv2
from PySide6.QtCore import QObject, Slot, QTimer, Signal, QThread, Qt
from PySide6.QtGui import QGuiApplication, QImage
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtQuick import QQuickImageProvider


class CameraThread(QThread):
    updateFrame = Signal(QImage)
    def __init__(self, parent=None):
        QThread.__init__(self, parent)

    def run(self):
        self.cap = cv2.VideoCapture(0, apiPreference=cv2.CAP_ANY, params=[
        cv2.CAP_PROP_FRAME_WIDTH, 1280,
        cv2.CAP_PROP_FRAME_HEIGHT, 720])
        while self.cap.isOpened():
            print("Camera Thread working")
            ret, frame = self.cap.read()
            if not ret:
                print("holy banana")
                pass
            color_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            print(color_frame.shape[1])
            print(color_frame.shape[0])
            img = QImage(color_frame.data, color_frame.shape[1], color_frame.shape[0], QImage.Format_RGB888)
            self.updateFrame.emit(img)
    def stop(self):
            self.ThreadActive = False
            self.quit()

class CamProvider(QQuickImageProvider):
    image = None

    def __init__(self):
        super(CamProvider, self).__init__(QQuickImageProvider.Image)
        self.cam = CameraThread()
        self.cam.updateFrame.connect(self.update_image)

    def requestImage(self, id, size, requestedSize):
        if id == "img?id=false" or "img?id=true":
            if self.image:
                img = self.image
            else:
                img = QImage(1280, 720, QImage.Format_RGBA8888)
                img.fill(Qt.black)
        return img

    imageChanged = Signal(bool)
    cameraError = Signal(bool)

    @Slot()
    def update_image(self, img):
        self.imageChanged.emit(True)
        self.image = img

    @Slot()
    def start(self):
        try:
            self.cam.start()
            print("Starting...")
        except:
            self.cameraError.emit(True)

    @Slot()
    def stop(self):
        self.cam.cap.release()
        self.cam.stop()
        print("Finishing...")


class MainWindow(QObject):

    def __init__(self):
        QObject.__init__(self)
        self.timer = QTimer()
        self.timer.timeout.connect(lambda: self.setTime())
        self.timer.start(1000)

    signal_time = Signal(str)

    @Slot(str)
    def setTime(self):
        global date
        now = datetime.datetime.now()
        date = now.strftime("%02H:%M")
        self.signal_time.emit(date)

if __name__ == "__main__":
    app = QGuiApplication(sys.argv)
    engine = QQmlApplicationEngine()

    # Get Context (Link between backend python and frontend QML)
    main = MainWindow()
    engine.rootContext().setContextProperty("backend", main)
    #Camera
    camProvider = CamProvider()
    engine.rootContext().setContextProperty("camProvider", camProvider)
    engine.addImageProvider("camProvider", camProvider)

    engine.load(os.fspath(Path(__file__).resolve().parent / "qml/main.qml"))
    if not engine.rootObjects():
        sys.exit(-1)
    sys.exit(app.exec_())



##
import sys
import cv2
from PyQt5.QtMultimedia import QVideoFrame, QVideoSurfaceFormat
from PyQt5.QtMultimediaWidgets import QVideoWidget
from PyQt5.QtWidgets import QWidget, QApplication, QVBoxLayout
from PyQt5.QtCore import pyqtSignal, pyqtSlot, Qt, QThread, QSize
from PyQt5.QtGui import QImage


class Thread(QThread):
    changePixmap = pyqtSignal(QImage)

    def run(self):
        cap = cv2.VideoCapture(0)
        while True:
            ret, frame = cap.read()
            if ret:
                # https://stackoverflow.com/a/55468544/6622587
                rgbImage = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                h, w, ch = rgbImage.shape
                bytesPerLine = ch * w
                convertToQtFormat = QImage(rgbImage.data, w, h, bytesPerLine, QImage.Format_RGB888)
                p = convertToQtFormat.scaled(600, 600, Qt.KeepAspectRatio)
                self.changePixmap.emit(p)


class VideoPlayer(QWidget):

    def __init__(self, parent=None):
        super(VideoPlayer, self).__init__(parent)
        self.videoWidget = QVideoWidget()
        self.video_surface = self.videoWidget.videoSurface()
        video_surface_format = QVideoSurfaceFormat(QSize(600, 600),
                                                   QVideoFrame.pixelFormatFromImageFormat(QImage.Format_RGB888))
        self.video_surface.start(video_surface_format)
        layout = QVBoxLayout()
        layout.addWidget(self.videoWidget)
        self.setLayout(layout)

        th = Thread(self)
        th.changePixmap.connect(self.update_image)
        th.start()

    @pyqtSlot(QImage)
    def update_image(self, image):
        self.video_surface.present(QVideoFrame(image))


if __name__ == '__main__':
    app = QApplication(sys.argv)
    player = VideoPlayer()
    player.setWindowTitle("Player")
    player.resize(600, 600)
    player.show()
    sys.exit(app.exec_())
 
