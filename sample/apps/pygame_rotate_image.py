# https://hobbylife.tistory.com/entry/%ED%8C%8C%EC%9D%B4%EC%8D%AC-pygame%EA%B3%BC-opencv-pyqt%EC%9D%98-%EB%8F%99%EC%9E%91%EB%B0%A9%EC%8B%9D%EC%9D%98-%EC%B0%A8%EC%9D%B4%EC%A0%90-%EC%A0%95%EB%A6%AC-%EC%BD%94%EB%93%9C%EB%B9%84%EA%B5%90

## pygame surface 변환 QImage
import pygame
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtWidgets import QApplication, QLabel

def pygame_surface_to_qimage(surface):
    """Converts a pygame.Surface to a QImage."""
    # Get surface dimensions
    width, height = surface.get_size()

    # Get pixel data in a suitable format (e.g., 'RGBA')
    # Note: Pygame's default origin is top-left, QImage's can be too, but consider flipping if needed
    # For 'RGBA', use 'RGBA', for 'RGB' use 'RGB'
    raw_data = pygame.image.tostring(surface, 'RGBA') 

    # Determine bytes per line (width * bytes per pixel)
    bytes_per_line = width * 4  # 4 bytes for RGBA

    # Create QImage
    q_image = QImage(raw_data, width, height, bytes_per_line, QImage.Format_RGBA8888)
    return q_image

if __name__ == '__main__':
    # Initialize Pygame
    pygame.init()

    # Create a sample Pygame surface
    pygame_surface = pygame.Surface((200, 150), pygame.SRCALPHA) # SRCALPHA for transparency
    pygame_surface.fill((255, 0, 0, 128)) # Red with 50% alpha
    pygame.draw.circle(pygame_surface, (0, 0, 255), (100, 75), 50) # Blue circle

    # Convert Pygame Surface to QImage
    q_image = pygame_surface_to_qimage(pygame_surface)

    # Display QImage in a PyQt application (for demonstration)
    app = QApplication([])
    label = QLabel()
    label.setPixmap(QPixmap.fromImage(q_image))
    label.show()
    app.exec_()

    pygame.quit()

## 파티클 효과 (분수)
#!/usr/bin/python3.4
# Setup Python ----------------------------------------------- #
import pygame, sys, random

# Setup pygame/window ---------------------------------------- #
mainClock = pygame.time.Clock()
from pygame.locals import *
pygame.init()
pygame.display.set_caption('game base')
screen = pygame.display.set_mode((500, 500),0,32)

def circle_surf(radius, color):
    surf = pygame.Surface((radius * 2, radius * 2))
    pygame.draw.circle(surf, color, (radius, radius), radius)
    surf.set_colorkey((0, 0, 0))
    return surf

# [loc, velocity, timer]
particles = []

# Loop ------------------------------------------------------- #
while True:

    # Background --------------------------------------------- #
    screen.fill((0,0,0))

    pygame.draw.rect(screen, (50, 20, 120), pygame.Rect(100, 100, 200, 80))

    mx, my = pygame.mouse.get_pos()
    particles.append([[mx, my], [random.randint(0, 20) / 10 - 1, -5], random.randint(6, 11)])

    for particle in particles:
        particle[0][0] += particle[1][0]
        particle[0][1] += particle[1][1]
        particle[2] -= 0.1
        particle[1][1] += 0.15
        pygame.draw.circle(screen, (255, 255, 255), [int(particle[0][0]), int(particle[0][1])], int(particle[2]))

        radius = particle[2] * 2
        screen.blit(circle_surf(radius, (20, 20, 60)), (int(particle[0][0] - radius), int(particle[0][1] - radius)), special_flags=BLEND_RGB_ADD)

        if particle[2] <= 0:
            particles.remove(particle)

    # Buttons ------------------------------------------------ #
    for event in pygame.event.get():
        if event.type == QUIT:
            pygame.quit()
            sys.exit()
        if event.type == KEYDOWN:
            if event.key == K_ESCAPE:
                pygame.quit()
                sys.exit()

    # Update ------------------------------------------------- #
    pygame.display.update()
    mainClock.tick(60)
    

## 이미지 회전/크기 변환

# Python program to transform the 
# image with the mouse
#Import the libraries pygame and math
import pygame
import math
from pygame.locals import *

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
img_logo = pygame.image.load('gfg_image.jpg')
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
            if event.key == K_ra:
                if event.mod & KMOD_SHIFT:
                    angle -= 5
                else:
                    angle += 5

            # Set at what ratio the image will
            # decrease or increase
            elif event.key == K_sa:
                if event.mod & KMOD_SHIFT:
                    scale /= 1.5
                else:
                    scale *= 1.5
                
        # Move the image with the specified coordinates,
        # angle and scale        
        elif event.type == MOUSEMOTION:
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
pygame.quit()