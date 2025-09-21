import pygame
import sys

WHITE = (255,255,255)
GREEN = (0,255,0)
RED = (255,0,0)
BLUE = (0,0,255)
BLACK = (0, 0, 0)
YELLOW = (255, 255, 0)

# --- classes --- (CamelCaseNames)

# empty

# --- functions --- (lower_case_names)

# empty

# --- main --- (lower_case_names)

pygame.init()

screen = pygame.display.set_mode((800,600),0)
screen_rect = screen.get_rect()

pygame.display.set_caption("Basic Pygame Text With Rects")

x, y = screen_rect.center
width, height = screen_rect.size
dx = 0
dy = 0
speed = 10
oldx = x
oldy = y

colour = BLACK

fontTitle = pygame.font.SysFont("arial", 10)
textTitle = fontTitle.render("Go Huskies", True, colour)
rectTitle = textTitle.get_rect(center=screen_rect.center)

# - mainloop -

clock = pygame.time.Clock()

main = True
while main:

    # - events -

    for event in pygame.event.get():
        if event.type ==pygame.QUIT:
            main = False
        if event.type ==pygame.KEYDOWN:
            if event.key == pygame.K_UP:
                dx = 0
                dy = -speed
                colour = RED
            elif event.key == pygame.K_DOWN:
                dx = 0
                dy = speed
                colour = BLUE
            elif event.key == pygame.K_LEFT:     # note: this section of code
                dx = -speed                     # doesn't have to change from
                dy = 0
                colour = GREEN
            elif event.key == pygame.K_RIGHT:
                dx = speed
                dy = 0
                colour = YELLOW
            elif event.key == pygame.K_c:
                x, y = screen_rect.center
                colour = BLACK
        if event.type == pygame.KEYUP:
            if event.key == pygame.K_UP or event.key == pygame.K_DOWN:
                dx = 0
                dy = 0
            elif event.key == pygame.K_LEFT or event.key == pygame.K_RIGHT:
                dx = 0
                dy = 0

    # - moves/updates -

    oldx, oldy = rectTitle.center
    rectTitle.move_ip(dx,dy)

    if (rectTitle.top <= 0) or (rectTitle.bottom >= height):
        dy = 0
        rectTitle.centery = oldy

    if (rectTitle.left <= 0) or (rectTitle.right >= width):
        dx = 0
        rectTitle.centerx = oldx

    textTitle = fontTitle.render("Go Huskies", True, colour)

    # - draws -

    screen.fill(WHITE)
    screen.blit(textTitle, rectTitle)
    pygame.display.update()

    clock.tick(25) # slow down to 25 FPS (frames per seconds)

# - end -

pygame.quit()
sys.exit()