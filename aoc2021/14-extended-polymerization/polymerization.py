STEPS = 10

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

def part_one(template, rules):
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
        # n_growth_rate = (step_counts['N'] - counts[i-1]['N']) / counts[i-1]['N']
        # print(f"N: count={step_counts['N']} growth_rate={n_growth_rate}")

    sorted_counts = sorted(counts[-1].items(), key=lambda item: item[1])
    print(sorted_counts)
    print(sorted_counts[-1][1] - sorted_counts[0][1])
    print("\n")


def get_index(c0, c1):
    return (ord(c0)-65)*26 + (ord(c1)-65)

def part_two(template, rules):
    alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

    # build pair count matrix
    pairs = []
    for i in range(26*26):
        pairs.append(0)
    
    for i in range(1, len(template)):
        c0 = template[i-1]
        c1 = template[i]
        index = get_index(c0,c1)
        pairs[index] += 1

    # perform insertions
    for i in range(STEPS):
        to_add = []
        to_remove = []
        for c0 in alphabet:
            for c1 in alphabet:
                index = get_index(c0,c1)
                pair_count = pairs[index]
                if pair_count > 0:
                    insert = rules[c0+c1]
                    index_0 = get_index(c0, insert)
                    index_1 = get_index(insert, c1)
                    to_add.append((index_0, pair_count))
                    to_add.append((index_1, pair_count))
                    to_remove.append((index, pair_count))
        for index, count in to_add:
            pairs[index] += count
        for index, count in to_remove:
            pairs[index] -= count



    counts = {}

    # find count that start with each character
    for c0 in alphabet:
        count = 0
        for c1 in alphabet:
            index = get_index(c0, c1)
            count += pairs[index]
        counts[c0] = count

    # find count that end in each character
    for c1 in alphabet:
        count = 0
        for c0 in alphabet:
            index = get_index(c0, c1)
            count += pairs[index]
        counts[c1] = max(count, counts[c1])


    # remove counts of zero and sort
    sorted_counts = list(filter(lambda x: x[1], sorted(counts.items(), key=lambda item: item[1])))
    print(sorted_counts)
    print(sorted_counts[-1][1] - sorted_counts[0][1])


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

    part_one(template, rules)
    part_two(template, rules)




"""
build list: O(n)
perform insertions: O(2^steps*n)
get element counts: O(n)
sort element counts: O(nlogn)
"""






