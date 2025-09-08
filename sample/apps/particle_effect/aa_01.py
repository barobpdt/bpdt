import random
import pygame

WIDTH = 800
HEIGHT = 800
FRAME_RATE = 60
BG_COLOUR = pygame.Color("darkslategray1")
DOT_COLOUR = pygame.Color("deepskyblue4")
DOT_SIZE = 2
SPEED = 2
POPULATION_SIZE = 200
MIN_SPEED = 2
MAX_SPEED = 6

raindrops = []

def populate():
	for n in range(POPULATION_SIZE):
		raindrops.append(
			[
				random.randint(0, WIDTH),
				random.randint(0, HEIGHT),
				random.randint(MIN_SPEED, MAX_SPEED),
			]
		)

def draw(surface):
	for raindrop in raindrops:
		pygame.draw.circle(surface, DOT_COLOUR, raindrop[:2], DOT_SIZE)

def update():
	for raindrop in raindrops:
		raindrop[1] += raindrop[2]
		if raindrop[1] >= HEIGHT:
			raindrop[1] = 0
			raindrop[0] = random.randint(0, WIDTH)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
clock = pygame.time.Clock()
populate()

running = True
while running:
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			running = False

	update()
	screen.fill(BG_COLOUR)
	draw(screen)
	pygame.display.flip()
	clock.tick(FRAME_RATE)
pygame.quit()