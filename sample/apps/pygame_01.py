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