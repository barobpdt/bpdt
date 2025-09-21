import pygame
import random
import math

# Particle class
class Particle:
    def __init__(self, x, y, radius, ps):
        self.x = x
        self.y = y
        self.ps = ps
        self.radius = radius
        self.angle = random.uniform(0, 2 * math.pi)
        self.speed = random.uniform(0.5, 1.5)

    def update(self):
        self.angle += 0.02
        self.x += math.cos(self.angle) * self.speed
        self.y += math.sin(self.angle) * self.speed

        if self.x < 0:
            self.x = self.ps.width
        elif self.x > self.ps.width:
            self.x = 0

        if self.y < 0:
            self.y = self.ps.height
        elif self.y > self.ps.height:
            self.y = 0

    def draw(self, window):
        color = (128, 128, 128)
        pos = (int(self.x), int(self.y))
        radius = int(self.radius)
        pygame.draw.circle(window, color, pos, radius)

# Particle system class
class ParticleSystem:
    def __init__(self, width, height):
        self.particles = []
        self.width = width
        self.height = height

    def add_particle(self, x, y, radius):
        self.particles.append(Particle(x, y, radius, self ))

    def add_random(self):
        x = random.randint(0, self.width)
        y = random.randint(0, self.height)
        self.add_particle(x,y,1)

    def update(self):
        for particle in self.particles:
            particle.update()

    def draw(self, window):
        for particle in self.particles:
            particle.draw(window)
