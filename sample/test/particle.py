import pygame
import random
import math

class Spark():
    def __init__(self, loc, angle, speed, color, scale=1):
        self.loc = loc
        self.angle = angle
        self.speed = speed
        self.scale = scale
        self.color = color
        self.alive = True

    def point_towards(self, angle, rate):
        rotate_direction = ((angle - self.angle + math.pi * 3) % (math.pi * 2)) - math.pi
        try:
            rotate_sign = abs(rotate_direction) / rotate_direction
        except ZeroDivisionError:
            rotate_sing = 1
        if abs(rotate_direction) < rate:
            self.angle = angle
        else:
            self.angle += rate * rotate_sign

    def calculate_movement(self, dt):
        return [math.cos(self.angle) * self.speed * dt, math.sin(self.angle) * self.speed * dt]


    # gravity and friction
    def velocity_adjust(self, friction, force, terminal_velocity, dt):
        movement = self.calculate_movement(dt)
        movement[1] = min(terminal_velocity, movement[1] + force * dt)
        movement[0] *= friction
        self.angle = math.atan2(movement[1], movement[0])
        # if you want to get more realistic, the speed should be adjusted here

    def move(self, dt):
        movement = self.calculate_movement(dt)
        self.loc[0] += movement[0]
        self.loc[1] += movement[1]

        # a bunch of options to mess around with relating to angles...
        #self.point_towards(math.pi / 2, 0.02)
        #self.velocity_adjust(0.975, 0.2, 8, dt)
        #self.angle += 0.1

        self.speed -= 0.1

        if self.speed <= 0:
            self.alive = False

    def draw(self, surf, offset=[0, 0]):
        if self.alive:
            points = [
                [self.loc[0] + math.cos(self.angle) * self.speed * self.scale, self.loc[1] + math.sin(self.angle) * self.speed * self.scale],
                [self.loc[0] + math.cos(self.angle + math.pi / 2) * self.speed * self.scale * 0.3, self.loc[1] + math.sin(self.angle + math.pi / 2) * self.speed * self.scale * 0.3],
                [self.loc[0] - math.cos(self.angle) * self.speed * self.scale * 3.5, self.loc[1] - math.sin(self.angle) * self.speed * self.scale * 3.5],
                [self.loc[0] + math.cos(self.angle - math.pi / 2) * self.speed * self.scale * 0.3, self.loc[1] - math.sin(self.angle + math.pi / 2) * self.speed * self.scale * 0.3],
                ]
            pygame.draw.polygon(surf, self.color, points)


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
        self.sparks = []
        self.width = width
        self.height = height
    
    def draw_spark(self, window):
        for i, spark in sorted(enumerate(self.sparks), reverse=True):
            spark.move(1)
            spark.draw(window)
            if not spark.alive:
                self.sparks.pop(i)
    def add_spark(self, mx, my):
        self.sparks.append(Spark([mx, my], math.radians(random.randint(0, 360)), random.randint(3, 6), (255, 255, 255), 2))

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
