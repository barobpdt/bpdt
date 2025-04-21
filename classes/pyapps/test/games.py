import random
from cpgames import cpgames

game_client = cpgames.CPGames()
all_supports = game_client.getallsupported()
game_client.execute(random.choice(list(all_supports.values())))