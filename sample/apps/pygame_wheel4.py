import pygame
import random

pygame.init()

# Screen dimensions
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Random Spinner Picker")

# Load spinner image (replace with your own image)
spinner_image = pygame.image.load("spinner_wheel.png").convert_alpha()
spinner_rect = spinner_image.get_rect(center=(WIDTH // 2, HEIGHT // 2))

# Initial rotation variables
current_angle = 0
rotation_speed = 0  # Starts still
spinning = False
final_angle = 0

# Game loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.MOUSEBUTTONDOWN and not spinning:
            spinning = True
            rotation_speed = random.uniform(5, 20)  # Random initial speed
            final_angle = random.randint(0, 360) # Random target angle

    # Update spinner rotation
    if spinning:
        current_angle += rotation_speed
        current_angle %= 360  # Keep angle within 0-359

        # Simulate slowing down
        rotation_speed *= 0.98  # Adjust this value for desired deceleration

        # Stop when close to final angle and speed is low
        if abs(current_angle - final_angle) < 5 and rotation_speed < 0.5:
            spinning = False
            current_angle = final_angle # Snap to the exact final angle
            print(f"Spinner stopped at angle: {current_angle}")
            # Determine selected choice based on current_angle

    # Drawing
    screen.fill((255, 255, 255))  # White background

    # Rotate and blit spinner
    rotated_spinner = pygame.transform.rotate(spinner_image, -current_angle)
    new_spinner_rect = rotated_spinner.get_rect(center=spinner_rect.center)
    screen.blit(rotated_spinner, new_spinner_rect)

    # Draw pointer (simple triangle for demonstration)
    pygame.draw.polygon(screen, (255, 0, 0), [(WIDTH // 2, 50), (WIDTH // 2 - 10, 70), (WIDTH // 2 + 10, 70)])

    pygame.display.flip()

pygame.quit()