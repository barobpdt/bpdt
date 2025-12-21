## 미디파일 목록
	https://songs.bardmusicplayer.com/ 
	
## 미디툴	
	https://github.com/ldrolez/free-midi-chords/releases

## 파이션 노래방 프로그
	https://github.com/giantdwarf17/KaraokeTube
## webRTC를 이용한 실시간 영상 처
	https://railly-linker.tistory.com/134

## 유사도 벡터추출
	https://wikidocs.net/blog/@TryOncePythonProject/880/
## 유튜브 영상 mp3로 저장하기
	https://expertpro.tistory.com/38#google_vignette	
## 유튜브 콘솔
	https://console.cloud.google.com/apis/dashboard?project=bpdt-de20b
	API 키: AIzaSyD8YZkO-B2Mu6SJQnuwqbAvPqNZV7d zmM
## 유튜브 다운로드
	https://github.com/yt-dlp/yt-dlp
## 자작 노래방 
	https://42morrow.tistory.com/entry/%EC%9E%90%EC%9E%91-%EB%85%B8%EB%9E%98%EB%B0%A9-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%A8-%EA%B0%80%EC%82%AC%EB%B3%B4%EB%A9%B0-%EB%85%B8%EB%9E%98-%EB%94%B0%EB%9D%BC-%EB%B6%80%EB%A5%B4%EA%B8%B0
## 반주와 음성분리하기
	https://blog.naver.com/kayoko79/223806280157
## moviepy 자막
	https://github.com/Anil-matcha/Free-Video-Tools/blob/main/Hardcode_subtitles_on_video.ipynb
	https://github.com/unconv/captacity
	

# pip install rembg # for library
from rembg import remove, new_session 
from PIL import Image

img_path = "Cosmetics.jpg"
out_path = 'Cosmetics_rembg.png'

img = Image.open(img_path)

model_name = "isnet-general-use"  # 여기에 모델 이름을 넣자
session = new_session(model_name)
out = remove(img, session=session)
out.save(out_path)

출처: https://udangco-coding-record.tistory.com/entry/Python-이미지-배경제거-rembg-누끼따기 [우당탕탕 코딩 기록:티스토리]

##
import sys
from PyQt5.QtWidgets import QApplication, QWidget
from PyQt5.QtGui import *
from PyQt5.QtCore import Qt
from PIL import Image

def make_background_transparent(image_path, output_path, background_color=(255, 255, 255)):
    """
    Makes the specified background color of an image transparent.

    Args:
        image_path (str): Path to the input image.
        output_path (str): Path to save the output transparent image.
        background_color (tuple): RGB tuple of the color to make transparent.
                                  Defaults to white (255, 255, 255).
    """
    try:
        img = Image.open(image_path)
        img = img.convert("RGBA")  # Ensure image has an alpha channel

        datas = img.getdata()
        newData = []
        for item in datas:
            # If the pixel matches the background color, make it transparent (alpha=0)
            if item[0] == background_color[0] and \
               item[1] == background_color[1] and \
               item[2] == background_color[2]:
                newData.append((item[0], item[1], item[2], 0))
            else:
                newData.append(item)

        img.putdata(newData)
        img.save(output_path, "PNG")
        print(f"Image saved with transparent background: {output_path}")

    except FileNotFoundError:
        print(f"Error: Image not found at {image_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

class ImageWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.image_path = r"c:/bpdt/data/sprites/ani01.png"  # 이미지 파일 경로를 여기에 입력하세요
        make_background_transparent(r"c:/bpdt/data/sprites/ani01.jpg", self.image_path, (216, 233, 243))
        self.setGeometry(100, 100, 800, 600)
        self.setWindowTitle('QPainter 이미지 그리기')

    def paintEvent(self, event):
        painter = QPainter(self)
        gradient = QLinearGradient(0, 0, self.width(), self.height()) # Start (x1,y1) and End (x2,y2) points
        gradient.setColorAt(0.0, QColor("lightblue")) # Color at 0% position
        gradient.setColorAt(1.0, QColor("darkblue"))  # Color at 100% position
        painter.setBrush(gradient)
        painter.setPen(Qt.NoPen)
        painter.drawRect(self.rect())
        
        image = QImage(self.image_path)
        pixel_color = image.pixelColor(10, 10)
        print(f"Pixel color at (10, 20): {pixel_color.red()}, {pixel_color.green()}, {pixel_color.blue()}")
        if not image.isNull():
            # 이미지 크기 및 위치 지정하여 그리기
            painter.drawImage(0, 0, image) # (x, y, image)

app = QApplication(sys.argv)
widget = ImageWidget()
widget.show()
sys.exit(app.exec())

##
from PySide6.QtGui import QImage, QColor
    width = 200
    height = 150
    image = QImage(width, height, QImage.Format_ARGB32)
    image.fill(Qt.white) # 이미지를 흰색으로 채웁니다.	
		
	# Set the color of the pixel at (10, 20) to blue
	image.setPixelColor(10, 20, QColor(0, 0, 255)) # Blue color (RGB)
    painter = QPainter(image)
    label = QLabel()
    label.setPixmap(QPixmap.fromImage(image))
    label.show()

# Get the color of the pixel at (10, 20)
pixel_color = image.pixelColor(10, 20)
print(f"Pixel color at (10, 20): {pixel_color.red()}, {pixel_color.green()}, {pixel_color.blue()}")

    from PyQt5.QtGui import QPixmap, QColor, Qt
    # Load your image
    image = QPixmap("your_image.png")

    # Define the color to be made transparent (e.g., white)
    transparent_color = QColor(255, 255, 255) # RGB for white

    # Create a mask from the color
    # Qt.MaskInColor makes the specified color transparent
    mask = image.createMaskFromColor(transparent_color, Qt.MaskInColor)

    # Apply the mask to the image
    image.setMask(mask)

