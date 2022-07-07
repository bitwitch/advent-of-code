
die = 1
rolls = 0


if __name__ == "__main__":
    players = [4, 1]
    scores = [0, 0]

    player = 0
    while scores[0] < 1000 and scores[1] < 1000:
        move = die + die+1 + die+2
        die += 3
        rolls += 3
        players[player] = ((players[player] + move - 1) % 10) + 1
        scores[player] += players[player]
        # print(f"player {player+1} rolls {move} and moves to space {players[player]} for total score of {scores[player]}")
        player = not player

    print(scores[player] * rolls)








