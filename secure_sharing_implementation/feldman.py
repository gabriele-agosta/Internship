from numpy.polynomial import Polynomial
import random
import warnings
import math

# warnings.filterwarnings('ignore', category=np.RankWarning)

def feldman(players, g, q):
    for player in players:
        check_value = math.pow(g, players[player]["y"])
        if players[player]["check_value"] != check_value:
            return "Commitments are wrong."
    return "All values were shared correctly"


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
    q = validate_input("Insert the value of q: ", 0, n_player + 1, True)
    p = validate_input("Insert the value of p: ", 0, max(n_player + 1, secret + 1, q), True)
    players = {"Player" + str(i): None for i in range(1, n_player + 1)}
    coefficients = [random.randint(0, q - 1) for i in range(threshold - 1)] + [secret]
    f = Polynomial(coefficients)
    x_coord = [i for i in range(1, n_player + 1)]
    y_coord = []
    commitments = []
    g = 2

    print(str(f))
    
    for x in x_coord:
        y_coord.append(f(x) % q)
    
    for coefficient in coefficients:
            commitments.append(math.pow(g, coefficient) % p)
    
    for player, (x, y) in zip(players.keys(), zip(x_coord, y_coord)):
        check_value = 1
        for j, commitment in enumerate(commitments):
            power = math.pow(x, j)
            check_value *= math.pow(commitment, power) 
        players[player] = {"x":x, "y":y, "check_value":check_value % p}

    print(f"Coordinates and check value for each player: {players}")

    print(feldman(players, 2, q))

    for i in range(1, n_player + 1):
        print(f"Result with {i} shares: {lagrange(x_coord[:i], y_coord[:i], min(threshold, i)) % p}")


if __name__ == '__main__':
    main()
