import pygame
import random
import math

# Initialize Pygame
pygame.init()

# Screen dimensions
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Random Wheel Picker")

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)
COLORS = [RED, GREEN, BLUE, (255, 255, 0), (0, 255, 255), (255, 0, 255)] # Example colors

# Wheel properties
center_x, center_y = WIDTH // 2, HEIGHT // 2
radius = 200
options = ["Option A", "Option B", "Option C", "Option D", "Option E", "Option F"]
num_segments = len(options)
angle_per_segment = 360 / num_segments

# Fonts
font = pygame.font.Font(None, 36)

# Game loop
running = True
angle = 0 # Current rotation angle of the wheel
spinning = False
spin_speed = 0 # Initial spin speed
target_angle = 0 # Angle where the wheel should stop

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.MOUSEBUTTONDOWN and not spinning:
            spinning = True
            spin_speed = random.randint(20, 40) # Random initial spin speed
            # Determine a random winning segment and calculate its target angle
            winning_index = random.randint(0, num_segments - 1)
            target_angle = (360 - (winning_index * angle_per_segment + angle_per_segment / 2)) % 360 # Adjust for center of segment

    # Clear screen
    screen.fill(WHITE)

    # Drawing the wheel
    for i in range(num_segments):
        start_angle_rad = math.radians(angle + i * angle_per_segment)
        end_angle_rad = math.radians(angle + (i + 1) * angle_per_segment)

        # Draw segment
        points = [(center_x, center_y)]
        for j in range(int(angle_per_segment)):
            current_angle_rad = math.radians(angle + i * angle_per_segment + j)
            x = center_x + radius * math.cos(current_angle_rad)
            y = center_y + radius * math.sin(current_angle_rad)
            points.append((x, y))
        points.append((center_x, center_y))
        pygame.draw.polygon(screen, COLORS[i % len(COLORS)], points)
        pygame.draw.arc(screen, BLACK, (center_x - radius, center_y - radius, radius * 2, radius * 2),
                        start_angle_rad, end_angle_rad, 2)

        # Draw text
        mid_angle_rad = math.radians(angle + i * angle_per_segment + angle_per_segment / 2)
        text_x = center_x + (radius * 0.7) * math.cos(mid_angle_rad)
        text_y = center_y + (radius * 0.7) * math.sin(mid_angle_rad)
        text_surface = font.render(options[i], True, BLACK)
        text_rect = text_surface.get_rect(center=(text_x, text_y))
        screen.blit(text_surface, text_rect)

    # Spinning logic
    if spinning:
        angle += spin_speed
        angle %= 360 # Keep angle within 0-359

        # Deceleration
        if spin_speed > 0.5: # Gradually reduce speed
            spin_speed *= 0.99
        else:
            # Snap to target angle and stop
            if abs(angle - target_angle) < 5: # Close enough to stop
                angle = target_angle
                spinning = False
                # Display winner (e.g., print to console or display on screen)
                print(f"Winner: {options[winning_index]}")

    pygame.display.flip()

pygame.quit()