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

## snow
import pygame
from pygame.locals import *
from pygame.math import Vector2
from pygame import mixer
import time
import random
import math

#0 = Menu, 1 = Game, 2 = Quit
game_state = 0
score = 0
score_int = 0
max_score = 0

entities_alive = []

class Player():
    global game_state
    #Transform
    position = pygame.Vector2()
    scale = pygame.Vector2()

    #Physics
    velocity_y = 0
    gravity_scale = 3000
    jump_force = 2000
    speed = 400
    is_grounded = False

    #Graphics
    player_sprite = 0
    rotation = 0
    rot_speed = 400

    scale_speed = 3

    player_collider = 0


    def __init__(self, desired_scale_x, desired_scale_y, Width, Height):
        self.scale.x = desired_scale_x
        self.scale.y = desired_scale_y

        self.scalar = 50

        self.player_sprite = pygame.sprite.Sprite()
        self.player_sprite.image = pygame.image.load("Data/Textures/Snowball.png").convert_alpha()
        self.player_sprite.rect = self.player_sprite.image.get_rect()
        self.player_sprite.image = pygame.transform.scale(self.player_sprite.image, (int(self.scale.x), int(self.scale.y)))

        self.position.x = Width / 2
        self.position.y = Height / 2 

        self.scalar = self.scale.x

    def move(self, keys, dt):
        global score_int
        global score
        global max_score

        if(keys[0]):
            self.position.x -= 0.01 * dt * self.speed
            self.rotation += 0.01 * dt * self.rot_speed
        if(keys[1]):
            self.position.x += 0.01 * dt * self.speed
            self.rotation -= 0.01 * dt * self.rot_speed
        if(keys[2] and self.is_grounded):
            self.velocity_y -= 0.01 * dt * self.jump_force

        self.velocity_y += 0.00001 * dt * self.gravity_scale

        self.position.y += self.velocity_y * dt

        self.player_sprite.rect.topleft = self.position.x, self.position.y

        #Scale The Snowball

        if(self.scalar < 12):
            game_state = 0
            if(score > max_score):
                max_score = score
            #score = 0
            #score_int = 0

        else:
            self.scalar -= 0.01 * dt * self.scale_speed

    def draw(self, screen, color, dt, colliders):
        img_copy = pygame.transform.scale(self.player_sprite.image, (int(self.scalar), int(self.scalar)))
        img_copy = pygame.transform.rotate(img_copy, self.rotation)
        self.collisions(colliders, img_copy.get_height())
        screen.blit(img_copy, (self.position.x - int(img_copy.get_width() / 2), self.position.y - int(img_copy.get_height() / 2)))
        
    def collisions(self, colliders, scale):
        #Top of box
        self.is_grounded = False
        for i in range(len(colliders)):
            if(self.position.y - int(scale / 2) >= colliders[i].top - scale and colliders[i].typeof == "environment"):
                self.is_grounded = True
                self.position.y = colliders[i].top - int(scale / 2) + 5
                self.velocity_y  = 0
            if(self.position.x + int(self.player_sprite.image.get_width() / 2) >= 720):
                self.position.x = 720 - int(self.player_sprite.image.get_width() / 2)
            if(self.position.x - int(self.player_sprite.image.get_width() / 2) <= 0):
                self.position.x = 0 + int(self.player_sprite.image.get_width() / 2)

class Box_Collider():
    position = Vector2()
    scale = Vector2()

    top = 0
    right = 0
    left = 0
    down = 0

    typeof = "default"

    def __init__(self, desired_position_x, desired_position_y, desired_scale_x, desired_scale_y, typeof):
        self.position.x = desired_position_x
        self.position.y = desired_position_y
        self.scale.x = desired_scale_x
        self.scale.y = desired_scale_y
        self.typeof = typeof

        self.top = self.position.y - self.scale.y / 2
        self.right = self.position.x + self.scale.x / 2
        self.left = self.position.x - self.scale.x / 2
        self.down = self.position.y + self.scale.y / 2

    def draw(self, screen, color):
        pygame.draw.rect(screen, color, (self.position.x, self.position.y, self.scale.x, self.scale.y))


class Entity():
    tag = "Default"
    bottom = 0

    entity_color = (0, 0, 0)
    sound = 0

    radius = 0

    right = 0
    left = 0

    def move(self, dt):
        self.position.y += dt * self.speed
        self.bottom = self.position.y + (self.radius * 2)
        self.right = self.position.x + (self.radius * 2)
        self.left = self.position.x

    def collisions(self, enemies):
        if(self.position.y > 390):
            enemies.remove(self)
            enemies = enemies[:-1]
            self.sound.play()

    def initialiser(self, color, sfx_path):
        self.entity_color = color
        self.sound = mixer.Sound(sfx_path)

    def draw(self, screen):
        pygame.draw.circle(screen, self.entity_color, (self.position.x, self.position.y), self.radius)

class Enemy(Entity):
    def __init__(self):
        self.speed = random.randrange(1, 5)
        self.position = Vector2()
        self.position.x = random.randrange(0, 720)
        self.position.y = 0

        self.initialiser((102, 107, 102),"Data/Sounds/RockHit.wav")

        self.tag = "enemy"

        self.radius = random.randrange(10, 30)

class Snowball(Entity):
    def __init__(self):
        self.speed = random.randrange(1, 5)
        self.position = Vector2()
        self.position.x = random.randrange(0, 720)
        self.position.y = 0
        self.initialiser((255, 255, 255),"Data/Sounds/SnowballHit.wav")
        self.tag = "snowball"

        self.radius = random.randrange(10, 30)

class Spawner():
    global game_state
    global entities_alive
    total_enemies_spawned = 0
    time_elapsed = 0
    screen = 0
    time_between_spawns = 100
    concurrent_enemys = 0

    sound = 0

    def __init__(self, screen):
        self.screen = screen
        self.sound = mixer.Sound("Data/Sounds/SnowballPowerup.wav")

    def check_for_player(self, player, scalar):
        global game_state
        global score_int
        global score
        global max_score
        for i in range(len(entities_alive)):
            try:
                if(entities_alive[i].bottom > 380 + 50 - scalar):
                    if(entities_alive[i].position.x > player.position.x - player.scale.x and entities_alive[i].position.x < player.position.x + player.scale.x):
                        if(entities_alive[i].tag == "enemy"):
                            game_state = 0
                            if(score > max_score):
                                max_score = score
                            #score = 0
                            #score_int = 0
                        else:
                            score += 5
                            player.scalar = 50
                            self.sound.play()
                            entities_alive.remove(entities_alive[i])
            except:
                pass
                        

        
    def set_time_between_spawns(self, time_between_spawns):
        self.time_between_spawns = time_between_spawns
    
    def draw_enemies(self, dt):
        for i in range(len(entities_alive)):
            try:
                entities_alive[i].draw(self.screen)
                entities_alive[i].move(dt)
                entities_alive[i].collisions(entities_alive)
            except:
                pass

    def timer(self, dt):
        self.time_elapsed += dt
    
    def spawner(self):
        if(self.time_elapsed >= self.time_between_spawns):
            random_int = random.randint(0, 4)
            if(random_int != 0 and self.concurrent_enemys < 3):
                entity = Enemy()
                self.concurrent_enemys += 1
            else:
                entity = Snowball()
                self.concurrent_enemys = 0
            entities_alive.append(entity)

            self.time_elapsed = 0

class Particle():
    def __init__(self):
        self.position = Vector2()
        self.position.x = random.randrange(0, 720)
        self.position.y = 0
        self.size = random.randrange(0, 15)
        self.color = (255, 255, 255)
        self.speed = random.randrange(20, 50)

    def move(self, dt):
        self.position.y += 0.01 * dt * self.speed

    def draw(self, screen):
        pygame.draw.rect(screen, self.color, (self.position.x, self.position.y,self.size,self.size))
    
    def collision(self, particles):
        if(self.position.y > 390):
            particles.remove(self)



class Main():
    global game_state
    previous_frame_time = 0
    dt = 0
    elapsed_time = 0
    time_between_spawns = 100
    
    def calculate_deltatime(self):
        self.dt = time.time() - self.previous_frame_time
        self.dt *= 60
        self.previous_frame_time = time.time()

    def difficulty(self):
        self.elapsed_time += self.dt
        if(self.elapsed_time > 1000):
            self.time_between_spawns /= 1.45
            self.elapsed_time = 0

    def handle_inputs(self, keys, event):
        if(event.type == pygame.KEYDOWN):
            if(event.key == K_a):
                keys[0] = True
            if(event.key == K_d):
                keys[1] = True
            if(event.key == K_w):
                keys[2] = True
        if(event.type == pygame.KEYUP):
            if(event.key == K_a):
                keys[0] = False
            if(event.key == K_d):
                keys[1] = False
            if(event.key == K_w):
                keys[2] = False
    
    def setup_pygame(self, title, width, height):
        screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(title)
        favicon = pygame.image.load("Data/Textures/Favicon.png").convert_alpha()
        pygame.display.set_icon(favicon)
        pygame.init()
        return screen

    def update_score(self, screen, text):
        global score_int
        global score
        score += self.dt / 100
        score_int = int(score)
        score_text = text.render("SCORE: " + str(score_int),True,(0,0,0))
        screen.blit(score_text, (10, 10))

    def draw_colliders(self, colliders, screen, color, width, height):
        for i in range(len(colliders)):
            colliders[i].draw(screen, color, width, height)

    def reset_state(self):
        self.previous_frame_time = 0
        self.dt = 0
        self.elapsed_time = 0
        self.time_between_spawns = 100
        self.score_int = 0
        

    def game(self, screen, font, WIDTH, HEIGHT):
        global game_state

        WHITE = (255, 255, 255)
        BLACK = (0,0,0)

        #         A      D    Space
        self.previous_frame_time = time.time()

        keys = [False, False, False]

        player = Player(40, 40, WIDTH, HEIGHT)

        colliders = []

        #Constant Sprites
        foreground = pygame.sprite.Sprite()
        foreground.image = pygame.image.load("Data/Textures/Foreground.png").convert_alpha()
        foreground.rect = foreground.image.get_rect()
        foreground.rect.topleft = 0, HEIGHT - 480
        foreground.image = pygame.transform.scale(foreground.image, (720, 480))
        foreground_collider = Box_Collider(WIDTH / 2, HEIGHT - 20, 720, 120, "environment")

        colliders.append(foreground_collider)

        spawner = Spawner(screen)

        particles = []

        time_elapsed = 0

        while(game_state == 1):
            screen.fill(WHITE)
            for event in pygame.event.get():
                if(event.type == pygame.QUIT):
                    game_state = 2
                self.handle_inputs(keys, event)
            self.difficulty()
            self.calculate_deltatime()

            screen.blit(foreground.image, foreground.rect)
            
            time_elapsed += self.dt
            
            if(time_elapsed > 10):
                part = Particle()
                particles.append(part)
                time_elapsed = 0
            for i in range(len(particles)):
                try:
                    particles[i].move(self.dt)
                    particles[i].draw(screen)
                    particles[i].collision(particles)
                except:
                    pass

            player.move(keys, self.dt)
            player.draw(screen, BLACK, self.dt, colliders)

            spawner.spawner()
            spawner.set_time_between_spawns(self.time_between_spawns)
            spawner.timer(self.dt)
            spawner.draw_enemies(self.dt)
            spawner.check_for_player(player, player.scalar)


            self.update_score(screen, font)

            pygame.display.update()

    def menu(self, screen, font, WIDTH, HEIGHT):
        global game_state

        COLOR = (224, 190, 108)

        sound = pygame.mixer.Sound("Data/Sounds/Start.wav")
        play_text = font.render("PLAY", True, (255,255,255))
        play_text_y_offset = 0

        score_text = font.render("HIGH SCORE: " + str(int(max_score)), True, (255,255,255))

        tutorial_text = font.render("You're Melting!", True, (255,255,255))
        tutorial_text_2 = font.render("Collect Snow And Avoid Moving Rocks!", True, (255,255,255))

        direc = 1
        while game_state == 0:
            screen.fill(COLOR)
            screen.blit(play_text, (WIDTH/2 - play_text.get_width() / 2, HEIGHT/2 - play_text.get_height() / 2 + play_text_y_offset))
            screen.blit(score_text, (WIDTH/2 - score_text.get_width() / 2, HEIGHT/2 - score_text.get_height() / 2 + 150))
            play_text_y_offset = math.sin(time.time() * 5) * 5 - 25
            for event in pygame.event.get():
                if(event.type == pygame.QUIT):
                    game_state = 2
                if(pygame.mouse.get_pos()[0] < WIDTH/2 + play_text.get_width() / 2 and pygame.mouse.get_pos()[0] > WIDTH/2 - play_text.get_width() / 2):
                    if(pygame.mouse.get_pos()[1] < HEIGHT/2 + play_text.get_height() / 2 + play_text_y_offset and pygame.mouse.get_pos()[1] > HEIGHT/2 - play_text.get_height() / 2 + play_text_y_offset):
                        if(event.type == pygame.MOUSEBUTTONDOWN):
                            game_state = 1
                            sound.play()
            pygame.display.update()
                       
    def __init__(self):
        global game_state
        global score 
        global score_int
        global entities_alive 
        
        while game_state != 2:
            WIDTH, HEIGHT = 720, 480

            screen = self.setup_pygame("Sno Snow", WIDTH, HEIGHT)
            font = pygame.font.Font("Data/Fonts/Inter.ttf", 32)

            if(game_state == 0):
                self.menu(screen, font, WIDTH, HEIGHT)
            if(game_state == 1):
                self.previous_frame_time = time.time()
                self.game(screen, font, WIDTH, HEIGHT)
            
            sound = pygame.mixer.Sound("Data/Sounds/Die.wav")
            sound.play()
            self.reset_state()
            score = 0
            score_int = 0
            entities_alive.clear()
            entities_alive = []
            
game = Main()