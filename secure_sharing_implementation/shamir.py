import numpy as np
import random
import warnings

warnings.filterwarnings('ignore', category=np.RankWarning)

def decode(x, y, order):
    result = np.polyfit(x, y, order)
    return int(round(result[-1]))


def main():
    n_player = int(input("Insert the value of 'n': "))
    threshold = int(input("Insert the value of the threshold (k): "))
    secret = int(input("Insert the value of the secret (M): "))
    players = {"Player" + str(i):None for i in range(1, n_player + 1)}
    coefficients = [random.randint(-10, 10) for i in range(threshold - 1)] + [secret]
    f = np.poly1d(coefficients)
    x_coord = [i for i in range(1, n_player + 1)]
    y_coord = []

    for x in x_coord:
        y_coord.append(f(x))

    for player, coord in zip(players.keys(), zip(x_coord, y_coord)):
        players[player] = coord

    print(f"Coordinates for each player: {players}")

    for i in range(1, n_player + 1):
        print(f"Result with {i} shares: {decode(x_coord[:i], y_coord[:i], f.order)}")


if __name__ == '__main__':
    main()