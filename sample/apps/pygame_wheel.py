import pygame
import random
import math

pygame.init()

# Screen dimensions
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Wheel of Names")

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
BLUE = (0, 0, 255)
GREEN = (0, 255, 0)

# Font
font = pygame.font.Font(None, 40)

# Wheel parameters
wheel_center = (WIDTH // 2, HEIGHT // 2)
wheel_radius = 200
names = ["Alice", "Bob", "Charlie", "David", "Eve"]
num_segments = len(names)
segment_angle = 360 / num_segments

current_rotation = 0
spinning = False
spin_speed = 0

def draw_wheel(surface, center, radius, names, rotation):
    for i, name in enumerate(names):
        start_angle = math.radians(i * segment_angle + rotation)
        end_angle = math.radians((i + 1) * segment_angle + rotation)
        
        # Draw segment
        color = (random.randint(50, 200), random.randint(50, 200), random.randint(50, 200)) # Random color for each segment
        pygame.draw.arc(surface, color, (center[0] - radius, center[1] - radius, radius * 2, radius * 2), 
                        start_angle, end_angle, radius)
        pygame.draw.line(surface, BLACK, center, (center[0] + radius * math.cos(start_angle), center[1] + radius * math.sin(start_angle)), 2)
        pygame.draw.line(surface, BLACK, center, (center[0] + radius * math.cos(end_angle), center[1] + radius * math.sin(end_angle)), 2)

        # Draw text (simplified - rotation of text would be more complex)
        mid_angle = (start_angle + end_angle) / 2
        text_x = center[0] + (radius * 0.7) * math.cos(mid_angle)
        text_y = center[1] + (radius * 0.7) * math.sin(mid_angle)
        text_surface = font.render(name, True, BLACK)
        text_rect = text_surface.get_rect(center=(text_x, text_y))
        surface.blit(text_surface, text_rect)

# Game loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.MOUSEBUTTONDOWN:
            if not spinning:
                spinning = True
                spin_speed = random.randint(20, 40) # Initial spin speed

    screen.fill(WHITE)

    if spinning:
        current_rotation += spin_speed
        spin_speed *= 0.98 # Decelerate
        if spin_speed < 0.5:
            spinning = False
            spin_speed = 0
            # Determine winner here

    draw_wheel(screen, wheel_center, wheel_radius, names, current_rotation)

    # Draw pointer
    pygame.draw.polygon(screen, RED, [(WIDTH // 2, wheel_center[1] - wheel_radius - 20),
                                     (WIDTH // 2 - 10, wheel_center[1] - wheel_radius),
                                     (WIDTH // 2 + 10, wheel_center[1] - wheel_radius)])

    pygame.display.flip()

pygame.quit()