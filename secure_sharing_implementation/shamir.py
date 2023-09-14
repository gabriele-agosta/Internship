import numpy as np
import random
import warnings

warnings.filterwarnings('ignore', category=np.RankWarning)

def lagrange(x, y, k):
    result = 0
    for i in range(k):
        product = 1
        for j in range(k):
            if j == i:
                continue
            product *= (0 - x[j]) / (x[i] - x[j])
        result += y[i] * product
    return int(result)

def validate_input(text, value, min_val, max_val=None):
    while value < min_val or (max_val is not None and value > max_val):
        value = int(input(text))
    return value

def main():
    n_player = validate_input("Insert the value of 'n': ", 0, 1)
    threshold = validate_input("Insert the value of the threshold (k): ", 0, 1, n_player)
    p = validate_input("Insert the value of p: ", 0, n_player + 1)
    secret = int(input("Insert the value of the secret (M): "))
    players = {"Player" + str(i): None for i in range(1, n_player + 1)}
    coefficients = [random.randint(0, p) for i in range(threshold - 1)] + [secret]
    f = np.poly1d(coefficients)
    x_coord = [i for i in range(1, n_player + 1)]
    y_coord = []
    
    
    for x in x_coord:
        y_coord.append(f(x) % p)


    for player, (x, y) in zip(players.keys(), zip(x_coord, y_coord)):
        players[player] = (x, y)

    print(f"Coordinates for each player: {players}")

    for i in range(1, n_player + 1):
        print(f"Result with {i} shares: {lagrange(x_coord[:i], y_coord[:i], min(threshold, i)) % p}")


if __name__ == '__main__':
    main()
