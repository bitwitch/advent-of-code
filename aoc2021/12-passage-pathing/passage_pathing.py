from queue import Queue

def is_small_cave(cave):
    first = ord(cave[0])
    return first < 65 or first > 90

def eligible_for_twice_visit(cave, visited, twice):
    return (is_small_cave(cave) and 
           cave in visited and 
           not twice and
           cave != 'start' and
           cave != 'end')

def traverse(adj, current, visited, path, all_paths, twice):
    if is_small_cave(current):
        visited.add(current)
    path.append(current)

    if current == 'end':
        new_path = path.copy()
        all_paths.append(new_path)
        # print(f"PATH: {new_path}")
    else:
        for node in adj[current]:
            if eligible_for_twice_visit(node, visited, twice):
                traverse(adj, node, visited, path, all_paths, node)
            elif node not in visited:
                traverse(adj, node, visited, path, all_paths, twice)

    if current in visited and current != twice:
        visited.remove(current)
    path.pop()


if __name__ == "__main__":
    adj = {}
    with open("input.txt", "r") as f:
        while 1:
            line = f.readline().strip()
            if not line:
                break

            nodes = line.split("-")
            a, b = nodes[0], nodes[1]
            if a not in adj:
                adj[a] = set()
            if b not in adj:
                adj[b] = set()
            adj[a].add(b)
            adj[b].add(a)

    # for n, neighbors in adj.items():
        # print(f"{n}: {neighbors}")
    # print("\n\n")

    visited = set()
    path = []
    all_paths = []
    traverse(adj, 'start', visited, path, all_paths, False)
    print(len(all_paths))


