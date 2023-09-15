import numpy as np
import random
import math
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


def is_prime(value):
    for divider in range(2, math.isqrt(value) + 1):
        if value % divider == 0:
            return False
    return True


def validate_input(text, value, min_val, check_prime = False, max_val=None):
    while value < min_val or (max_val is not None and value > max_val):
        value = int(input(text))
    if check_prime and not is_prime(value):
        validate_input("The value must be prime: ", 0, min_val, check_prime)
    return value


def main():
    n_player = validate_input("Insert the value of 'n': ", 0, 1)
    threshold = validate_input("Insert the value of the threshold (k): ", 0, 1, False, n_player)
    secret = int(input("Insert the value of the secret (M): "))
    p = validate_input("Insert the value of p: ", 0, max(n_player + 1, secret + 1), True)
    players = {"Player" + str(i): None for i in range(1, n_player + 1)}
    coefficients = [random.randint(0, p - 1) for i in range(threshold - 1)] + [secret]
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
