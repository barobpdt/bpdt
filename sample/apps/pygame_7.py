import pygame

_circle_cache = {}
def _circlepoints(r):
    r = int(round(r))
    if r in _circle_cache:
        return _circle_cache[r]
    x, y, e = r, 0, 1 - r
    _circle_cache[r] = points = []
    while x >= y:
        points.append((x, y))
        y += 1
        if e < 0:
            e += 2 * y - 1
        else:
            x -= 1
            e += 2 * (y - x) - 1
    points += [(y, x) for x, y in points if x > y]
    points += [(-x, y) for x, y in points if x]
    points += [(x, -y) for x, y in points if y]
    points.sort()
    return points

def add_outline_to_image(image: pygame.Surface, thickness: int, color: tuple, color_key: tuple = (255, 0, 255)) -> pygame.Surface:
    mask = pygame.mask.from_surface(image)
    mask_surf = mask.to_surface(setcolor=color)
    mask_surf.set_colorkey((0, 0, 0))

    new_img = pygame.Surface((image.get_width() + 2, image.get_height() + 2))
    new_img.fill(color_key)
    new_img.set_colorkey(color_key)

    for i in -thickness, thickness:
        new_img.blit(mask_surf, (i + thickness, thickness))
        new_img.blit(mask_surf, (thickness, i + thickness))
    new_img.blit(image, (thickness, thickness))

    return new_img

def render(text, font, gfcolor=pygame.Color('dodgerblue'), ocolor=(255, 255, 255), opx=2):
    textsurface = font.render(text, True, gfcolor).convert_alpha()
    w = textsurface.get_width() + 2 * opx
    h = font.get_height()

    osurf = pygame.Surface((w, h + 2 * opx)).convert_alpha()
    osurf.fill((0, 0, 0, 0))

    surf = osurf.copy()

    osurf.blit(font.render(text, True, ocolor).convert_alpha(), (0, 0))

    for dx, dy in _circlepoints(opx):
        surf.blit(osurf, (dx + opx, dy + opx))

    surf.blit(textsurface, (opx, opx))
    return surf

def main():
    pygame.init()

    font = pygame.font.SysFont(None, 64)
    myfont = pygame.font.SysFont(None, 64)


    screen = pygame.display.set_mode((350, 100))
    clock = pygame.time.Clock()

    text_surf = myfont.render("test", False, (255, 255, 255)).convert()
    text_with_ouline = add_outline_to_image(text_surf, 2, (255, 0, 0))

    while True:
        events = pygame.event.get()
        for e in events:
            if e.type == pygame.QUIT:
                return
        screen.fill((30, 30, 30))

        screen.blit(render('Hello World', font), (20, 20))
        screen.blit(text_with_ouline, (50,50))
        pygame.display.update()
        clock.tick(60)

if __name__ == '__main__':
    main()