import pygame
import cv2
import numpy as np

# Initialize Pygame
pygame.init()

# Create a Pygame surface (e.g., a screen or an image)
screen_width, screen_height = 640, 480
# screen = pygame.display.set_mode((screen_width, screen_height))
screen = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA) # SRCALPHA for transparency
pygame.display.set_caption("Pygame to OpenCV Conversion")

# Fill the screen with a color and draw something
screen.fill((0, 250, 0)) # Red background
pygame.draw.circle(screen, (200, 50, 50), (screen_width // 2, screen_height // 2), 150) # Green circle
surface_bytes = pygame.image.tobytes(screen, 'RGB')
opencv_image = np.frombuffer(surface_bytes, dtype=np.uint8).reshape((screen_height, screen_width, 3))
img_fg = cv2.cvtColor(opencv_image, cv2.COLOR_RGB2BGR)
cap = cv2.VideoCapture(0)
ret, img_bg = cap.read()
if ret:
	img_bg = cv2.resize(img_bg, dsize=(640,480), interpolation=cv2.INTER_LINEAR)
	print(f'img:{img_bg.shape}, {img_fg.shape} ')
	hsv = cv2.cvtColor(img_fg, cv2.COLOR_BGR2HSV)

	'''
	lower_green = np.array([40, 100, 100])  # 초록색의 하한값
	upper_green = np.array([80, 255, 255])  # 초록색의 상한값
	# 3. 초록색 마스크 생성
	# mask = cv2.inRange(hsv, lower_green, upper_green)
	RGB 녹색계열
	0 <= B <= 100
	128 <= G <= 255
	0 <= R <= 100

	HSV 녹색계열
	50 <= H <= 80
	150 <= S <= 255
	0 <= V <= 255
	
	'''
	# HSV에서 녹색계열을 범위로 뽑아낼 수 있음
	mask = cv2.inRange(hsv, (50, 150, 0), (80, 255, 255))
	# 4. 마스크를 반전시켜 배경이 될 영역을 흰색으로, 전경을 검은색으로 만듦
	mask_inv = cv2.bitwise_not(mask)
	cv2.copyTo(img_bg, mask, img_fg)
	# img_background_cropped = cv2.bitwise_and(img_bg, img_bg, mask=mask_inv)

	# 6. 전경 이미지를 배경과 합침
	# result = cv2.add(img_background_cropped, img_fg, mask=mask)
else:
	result = img_fg

cv2.imshow('Result', img_fg)
cv2.waitKey(0) # Wait indefinitely until a key is pressed
cv2.destroyAllWindows()

# Quit Pygame
pygame.quit()