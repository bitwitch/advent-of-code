STEPS = 25

class Node:
    def __init__(self, data):
        self.data = data
        self.prev = None
        self.next = None

def print_polymer(start):
    while start:
        print(start.data, end='')
        start = start.next
    print('')

def get_element_counts(start):
    counts = {}
    current = start
    while current:
        data = current.data
        if data not in counts:
            counts[data] = 0
        counts[data] = counts[data] + 1
        current = current.next
    return counts

if __name__ == "__main__":
    rules = {}
    with open("small_input.txt", "r") as f:
        template = f.readline().strip()
        f.readline()
        while 1:
            raw = f.readline().strip()
            if not raw:
                break
            raw = raw.split(' -> ')
            rules[raw[0]] = raw[1]

    # print(template)
    # print(rules)

    start = Node(template[0])

    # build list
    prev = start
    for c in template[1:]:
        node = Node(c)
        node.prev = prev
        prev.next = node
        prev = node

    # keep track of element counts at each step
    counts = []

    # perform insertions
    for i in range(STEPS):
        current = start
        while current:
            if not current.next:
                break
            pair = current.data + current.next.data
            if pair in rules:
                insert = Node(rules[pair])
                insert.next = current.next
                insert.prev = current
                current.next.prev = insert
                current.next = insert
                current = current.next.next
            else:
                current = current.next

        # print_polymer(start)

        step_counts = get_element_counts(start)
        # print(step_counts)
        counts.append(step_counts)

        if i == 0:
            continue

        # for element, count in step_counts.items():
            # growth = count - counts[i-1][element]
            # print(f"growth {element}: {growth}")

        # N growth
        n_growth_rate = (step_counts['N'] - counts[i-1]['N']) / counts[i-1]['N']
        print(f"N: count={step_counts['N']} growth_rate={n_growth_rate}")
        

    # sorted_counts = sorted(counts.items(), key=lambda item: item[1])

    # print(sorted_counts[-1][1] - sorted_counts[0][1])









"""
build list: O(n)
perform insertions: O(2^steps*n)
get element counts: O(n)
sort element counts: O(nlogn)
"""






