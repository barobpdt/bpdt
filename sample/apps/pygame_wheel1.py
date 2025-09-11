import pygame
import random
import math

# --- Pygame Initialization ---
pygame.init()
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Spinning Wheel")

# --- Wheel Data ---
names = ["Alice", "Bob", "Charlie", "Diana", "Eve"]
num_segments = len(names)
segment_angle = 360 / num_segments

# --- Wheel Properties ---
wheel_center = (WIDTH // 2, HEIGHT // 2)
wheel_radius = 200
current_angle = 0  # Current rotation angle of the wheel
spin_speed = 0     # Initial spin speed

# --- Game Loop ---
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.MOUSEBUTTONDOWN:
            if spin_speed == 0: # Only spin if not already spinning
                spin_speed = random.randint(30, 60) # Start spinning

    # --- Update Wheel State ---
    if spin_speed > 0:
        current_angle += spin_speed
        current_angle %= 360 # Keep angle within 0-359
        spin_speed *= 0.98 # Decelerate
        if spin_speed < 0.5: # Stop if speed is too low
            spin_speed = 0

    # --- Drawing ---
    screen.fill((255, 255, 255)) # White background

    # Draw segments and names
    for i in range(num_segments):
        start_angle = (current_angle + i * segment_angle) % 360
        end_angle = (current_angle + (i + 1) * segment_angle) % 360

        # Draw segment (simplified for illustration)
        # You would draw arcs or polygons here
        color = (random.randint(50, 200), random.randint(50, 200), random.randint(50, 200)) # Random color for each segment
        # pygame.draw.line(screen, (0, 0, 0), wheel_center, (wheel_center[0] + wheel_radius * math.cos(start_angle), wheel_center[1] + wheel_radius * math.sin(start_angle)), 2)
        # pygame.draw.line(screen, (0, 0, 0), wheel_center, (wheel_center[0] + wheel_radius * math.cos(end_angle), wheel_center[1] + wheel_radius * math.sin(end_angle)), 2)

        pygame.draw.arc(screen, (0,0,0), (wheel_center[0] - wheel_radius, wheel_center[1] - wheel_radius, wheel_radius * 2, wheel_radius * 2),
                        math.radians(start_angle), math.radians(end_angle), 3)

        # Render name (simplified)
        font = pygame.font.Font(None, 30)
        text_surface = font.render(names[i], True, (0, 0, 0))
        # Position text within the segment (more complex calculation needed for proper placement)
        text_rect = text_surface.get_rect(center=(wheel_center[0] + math.cos(math.radians(start_angle + segment_angle/2)) * (wheel_radius / 2),
                                                 wheel_center[1] + math.sin(math.radians(start_angle + segment_angle/2)) * (wheel_radius / 2)))
        screen.blit(text_surface, text_rect)

    # --- Display Result (when stopped) ---
    if spin_speed == 0 and current_angle is not None:
        # Determine winning segment based on final angle
        winning_index = int((360 - current_angle) / segment_angle) % num_segments
        winning_name = names[winning_index]
        result_font = pygame.font.Font(None, 50)
        result_text = result_font.render(f"Winner: {winning_name}", True, (255, 0, 0))
        result_rect = result_text.get_rect(center=(WIDTH // 2, HEIGHT - 50))
        screen.blit(result_text, result_rect)

    pygame.display.flip()

pygame.quit()