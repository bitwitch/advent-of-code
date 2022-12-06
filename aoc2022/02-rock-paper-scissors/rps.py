values = {
    'X' : 1,
    'Y' : 2,
    'Z' : 3
}

def do_outcome_one(a, b):
    if a == 'X':
        if b == 'A':
            return 3
        elif b == 'B':
            return 0
        else:
            return 6

    elif a == 'Y':
        if b == 'A':
            return 6
        elif b == 'B':
            return 3
        else:
            return 0

    else:
        if b == 'A':
            return 0
        elif b == 'B':
            return 6
        else:
            return 3

def do_outcome_two(a, b):
    if a == 'X':
        if b == 'A':
            return 3
        elif b == 'B':
            return 1
        else:
            return 2

    elif a == 'Y':
        if b == 'A':
            return 1+3
        elif b == 'B':
            return 2+3
        else:
            return 3+3

    else:
        if b == 'A':
            return 2+6
        elif b == 'B':
            return 3+6
        else:
            return 1+6



def part_one(moves):
    score = 0
    for move in moves:
        opponent, player = move[0], move[2]
        score += values[player]
        score += do_outcome_one(player, opponent)
    return score

def part_two(moves):
    score = 0
    for move in moves:
        opponent, player = move[0], move[2]
        score += do_outcome_two(player, opponent)
    return score


if __name__ == "__main__":
    moves = []
    with open("input.txt", "r") as f:
        while 1:
            line = f.readline().strip()
            if not line:
                break
            moves.append(line)

    result_one = part_one(moves)
    print(f"result one: {result_one}")

    result_two = part_two(moves)
    print(f"result two: {result_two}")

