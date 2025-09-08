import os
import pygame

class SpriteSheet:
    def __init__(self, filename, px, py, tw, th, m, tiles, color_key = None):
        self.sheet = pygame.image.load(filename)
        if color_key:
            self.sheet = self.sheet.convert()
            self.sheet.set_colorkey(color_key)
        else:
            self.sheet = self.sheet.convert_alpha()
        self.cells = [(px + tw * i, py, tw-m, th) for i in range(tiles)]
        self.index = 0

    def update(self):
        self.tile_rect = self.cells[self.index % len(self.cells)]
        self.index += 1

    def draw(self, surface, x, y):
        rect = pygame.Rect(self.tile_rect)
        rect.center = (x, y) 
        surface.blit(self.sheet, rect, self.tile_rect)

pygame.init()
window = pygame.display.set_mode((400, 300))
clock = pygame.time.Clock()

sprite_sheet = SpriteSheet('sprite01.png', 18, 580, 64, 66, 0, 6, (0, 128, 0))

run = True
while run:
    clock.tick(10)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            run = False

    sprite_sheet.update()

    window.fill(0)
    sprite_sheet.draw(window, *window.get_rect().center)
    pygame.display.update()
    
pygame.quit()
exit()