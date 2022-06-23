from collections import deque
from math import inf

def dijkstra_smallest_risk(grid, start, target):
    # initialize grid of risks
    risk = []
    for j in grid:
        risk.append([inf for i in grid[0]])
    risk[start[1]][start[0]] = 0

    frontier = deque()
    frontier.append(start)

    while frontier:
        current = frontier.popleft()
        x, y = current

        # get neighbors
        neighbors = []
        if y > 0:
            neighbors.append((x, y-1))
        if x > 0:
            neighbors.append((x-1, y))
        if y < len(grid)-1:
            neighbors.append((x, y+1))
        if x < len(grid[0])-1: 
            neighbors.append((x+1, y))

        for neighbor in neighbors:
            nx, ny = neighbor
            r = risk[y][x] + grid[ny][nx]
            if r < risk[ny][nx]:
                risk[ny][nx] = r
                frontier.append(neighbor)
    
    # for row in risk:
        # for col in row:
            # print(f"{col:2}", end=" ")
        # print("")

    return risk[target[1]][target[0]]
    
def dp_smallest_risk(grid, start, target):
    # initialize grid of risks
    risk = []
    for j in grid:
        risk.append([0 for i in grid[0]])

    for j in range(1,len(grid)):
        risk[j][0] = risk[j-1][0] + grid[j][0]
    for i in range(1,len(grid[0])):
        risk[0][i] = risk[0][i-1] + grid[0][i]
    for j in range(1,len(grid)):
        for i in range(1,len(grid[0])):
            risk[j][i] = min(risk[j-1][i], risk[j][i-1]) + grid[j][i]

    return risk[target[1]][target[0]]



# naive brute force
def path_smallest_risk(grid, start, target):
    stack = deque()
    min_risk = 999999999
    risk = -grid[start[1]][start[0]]
    visited = set()
    stack.append(start)
    stack.append(visited)
    stack.append(risk)

    while stack:
        risk    = stack.pop()
        visited = stack.pop()
        current = stack.pop()

        # if current in visited:
            # continue

        visited.add(current)

        x, y = current
        risk += grid[y][x]

        # print(f"({x}, {y}) -> {grid[y][x]}  visited: {visited} total_risk: {risk} min: {min_risk}")

        # bail out of this path if risk exceeds min successful path
        if risk > min_risk:
            continue

        if current == target:
            if risk < min_risk:
                min_risk = risk
            continue

        # get neighbors
        neighbors = []
        if y > 0 and (x, y-1) not in visited:
            neighbors.append((x, y-1))
        if x > 0 and (x-1, y) not in visited:
            neighbors.append((x-1, y))
        if y < len(grid)-1 and (x, y+1) not in visited:
            neighbors.append((x, y+1))
        if x < len(grid[0])-1 and (x+1, y) not in visited:
            neighbors.append((x+1, y))

        # add neighbors to stack
        for cell in neighbors:
            stack.append(cell)
            stack.append(visited.copy())
            stack.append(risk)

    return min_risk

def big_grid_risk(grid, x, y):
    w = len(grid[0])
    h = len(grid)
    result = grid[y%h][x%w] + x//w + y//h
    result = ((result-1) % 9) + 1
    return result


if __name__ == "__main__":
    grid = []
    with open("input.txt", "r") as f:
        while 1:
            line = f.readline().strip()
            if not line:
                break
            row = [int(c) for c in line]
            grid.append(row)
            # print(row)
    # print("")


    # PART ONE
    # start = (0,0)
    # target = (len(grid[0])-1, len(grid)-1)
    # # path_smallest_risk(grid, start, target)
    # # risk = dp_smallest_risk(grid, start, target)
    # risk = dijkstra_smallest_risk(grid, start, target)
    # print(risk)

    # PART TWO
    big_grid = []
    for j in range(len(grid)*5):
        row = []
        for i in range(len(grid[0])*5):
            row.append(big_grid_risk(grid, i, j))
        big_grid.append(row)

    start = (0,0)
    target = (len(big_grid[0])-1, len(big_grid)-1)

    risk = dijkstra_smallest_risk(big_grid, start, target)
    print(risk)
    


