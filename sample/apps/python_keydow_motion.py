import pygame
import math
from pygame.locals import *
import argparse

## 호출 : @job.cmdRun(c,_s('$py "c:/bpdt/sample/apps/python_keydown_motion.py" --out "${log.logFileName}"'))

# Take colors input
RED = (255, 0, 0)
BLACK = (0, 0, 0)
YELLOW = (255, 255, 0)

#Construct the GUI game
pygame.init()

#Set dimensions of game GUI
w, h = 600, 440
screen = pygame.display.set_mode((w, h))

# Set running, angle and scale values
running = True
angle = 0
scale = 1

# Take image as input
img_logo = pygame.image.load('c:/bpdt/data/sprites/down_130.jpg')
img_logo.convert()

# Draw a rectangle around the image
rect_logo = img_logo.get_rect()
pygame.draw.rect(img_logo, RED, rect_logo, 1)

# Set the center and mouse position
center = w//2, h//2
mouse = pygame.mouse.get_pos()

#Store the image in a new variable
#Construct the rectangle around image
img = img_logo
rect = img.get_rect()
rect.center = center
shift = False
mouseArr = []
# 인자값을 받을 수 있는 인스턴스 생성
parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')

# 입력받을 인자값 등록
class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()

fpOut = None
if args.out:
	fpOut = open(args.out, 'a', encoding='utf8')

def log (msg):
	if fpOut:
		fpOut.write(f"##> {msg}\n")
		fpOut.flush()

# Setting what happens when game is
# in running state
while running:
	for event in pygame.event.get():

		# Close if the user quits the game
		if event.type == QUIT:
			running = False

		# Set at which angle the image will
		# move left or right
		if event.type == KEYDOWN:
			if event.key == K_r:
				if event.mod & KMOD_SHIFT:
					angle -= 5
				else:
					angle += 5

			elif event.key == K_q:
				running = False
			# Set at what ratio the image will
			# decrease or increase
			elif event.key == K_s:
				if event.mod & KMOD_SHIFT:
					scale /= 1.5
				else:
					scale *= 1.5
			elif event.mod & KMOD_SHIFT:
				print("@@ shift down")
				if shift==False:
					mouseArr = []
				shift = True 
		elif event.type == KEYUP:
			print("@@ shift up")
			shift = False
			
		# Move the image with the specified coordinates,
		# angle and scale        
		elif event.type == MOUSEMOTION:
			if shift:
				mouseArr.append(event.pos)
			mouse = event.pos
			x = mouse[0] - center[0]
			y = mouse[1] - center[1]
			d = math.sqrt(x ** 2 + y ** 2)
			angle = math.degrees(-math.atan2(y, x))
			scale = abs(5 * d / w)
			img = pygame.transform.rotozoom(img_logo, angle, scale)
			rect = img.get_rect()
			rect.center = center
	
	# Set screen color and image on screen
	screen.fill(YELLOW)
	screen.blit(img, rect)

	# Draw the rectangle, line and circle through
	# which image can be transformed 
	pygame.draw.rect(screen, BLACK, rect, 3)
	pygame.draw.line(screen, RED, center, mouse, 2)
	pygame.draw.circle(screen, RED, center, 6, 1)
	pygame.draw.circle(screen, BLACK, mouse, 6, 2)
	
	# Update the GUI game
	pygame.display.update()

# Quit the GUI game
log(f'print:mouseArr={mouseArr}')
pygame.quit()