import sys
from PyQt5.QtWidgets import QApplication, QWidget
from PyQt5.QtGui import *
from PyQt5.QtCore import Qt
from PIL import Image

def PIL_to_qimage(pil_img):
	temp = pil_img.convert('RGBA')
	return QImage(
		temp.tobytes('raw', "RGBA"),
		temp.size[0],
		temp.size[1],
		QImage.Format.Format_RGBA8888
	)

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
		# make_background_transparent(r"c:/bpdt/data/sprites/ani01.jpg", self.image_path, (216, 233, 243))
		self.image = PIL_to_qimage(Image.open(self.image_path))
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
		
		if not self.image.isNull():
			# 이미지 크기 및 위치 지정하여 그리기
			painter.drawImage(0, 0, self.image) # (x, y, image)

app = QApplication(sys.argv)
widget = ImageWidget()
widget.show()
sys.exit(app.exec())
