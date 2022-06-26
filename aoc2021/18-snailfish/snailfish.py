from collections import deque
from math import floor, ceil

class Pair:
    def __init__(self, a, b):
        self.a = a
        self.b = b
    def __getitem__(self, index):
        if index == 0:
            return self.a
        elif index == 1:
            return self.b
        else:
            raise Exception("Index provided for Pair was not 0 or 1")
    def __setitem__(self, index, val):
        if index == 0:
            self.a = val
        elif index == 1:
            self.b = val
        else:
            raise Exception("Index provided for Pair was not 0 or 1")
    def __str__(self):
        return f"({self.a.__str__()}, {self.b.__str__()})"


##############################################################################
#      EXPLODING
##############################################################################
def get_leftmost_explode(pair, level, path):
    lhs = pair[0]
    rhs = pair[1]

    if type(lhs) is Pair:
        path.append(0)
        if level >= 3:
            return lhs
        left_result = get_leftmost_explode(lhs, level+1, path)
        if left_result:
            return left_result
        else:
            path.pop()

    if type(rhs) is Pair:
        path.append(1)
        if level >= 3:
            return rhs
        right_result = get_leftmost_explode(rhs, level+1, path)
        if right_result:
            return right_result
        else:
            path.pop()

    return False


def update_closest_left(num, pair, path):
    # follow path up to the last time you went right
    last_right_index = None
    for i in reversed(range(len(path))):
        if path[i] == 1:
            last_right_index = i
            break
    if last_right_index == None:
        return

    current = num
    for step in path[:last_right_index]:
        current = current[step]

    # go left once, and then right until you hit a regular
    if not type(current[0]) is Pair:
        current[0] += pair[0]
        return
    current = current[0]
    while type(current[1]) is Pair:
        current = current[1]
    current[1] = current[1] + pair[0]


def update_closest_right(num, pair, path):
    # follow path up to the last time you went left
    last_left_index = None
    for i in reversed(range(len(path))):
        if path[i] == 0:
            last_left_index = i
            break
    if last_left_index == None:
        return
    current = num
    for step in path[:last_left_index]:
        current = current[step]


    # go right once, and then left until you hit a regular
    if not type(current[1]) is Pair:
        current[1] += pair[1]
        return
    current = current[1]
    while type(current[0]) is Pair:
        current = current[0]
    current[0] = current[0] + pair[1]


def set_explode_zero(num, pair, path):
    current = num
    for step in path[:-1]:
        current = current[step]
    current[path[-1]] = 0


def explode(num):
    path = []
    to_explode = get_leftmost_explode(num, 0, path)
    if not to_explode:
        return False
    update_closest_left(num, to_explode, path)
    update_closest_right(num, to_explode, path)
    set_explode_zero(num, to_explode, path)
    return True

##############################################################################
#      SPLITTING
##############################################################################
def get_leftmost_split(pair, path):
    lhs = pair[0]
    rhs = pair[1]

    if type(lhs) is Pair:
        path.append(0)
        left_result = get_leftmost_split(lhs, path)
        if left_result:
            return left_result
        else:
            path.pop()
    elif lhs >= 10:
        path.append(0)
        return True

    if type(rhs) is Pair:
        path.append(1)
        right_result = get_leftmost_split(rhs, path)
        if right_result:
            return right_result
        else:
            path.pop()
    elif rhs >= 10:
        path.append(1)
        return True

    return False


def split(num):
    path = []
    exists = get_leftmost_split(num, path)
    if not exists:
        return False
    current = num
    for step in path[:-1]:
        current = current[step]
    val = current[path[-1]]
    current[path[-1]] = Pair(floor(val/2), ceil(val/2))
    return True

##############################################################################

def magnitude(num):
    if not type(num) is Pair:
        return num
    return 3*magnitude(num[0]) + 2*magnitude(num[1])

def reduce(num):
    action = True
    while action:
        action = explode(num)
        if action:
            continue
        action = split(num)
    return num


def snailfish_add(a, b):
    return reduce(Pair(a,b))


def part_one(numbers):
    result = numbers[0]
    for n in numbers[1:]:
        result = snailfish_add(result, n)
    mag = magnitude(result)
    print(mag)


def parse_line(line):
    lhs = deque()
    stack = deque()
    for c in line:
        if c == '[':
            stack.append(())
        elif c == ',':
            lhs.append(stack.pop())
        elif c == ']':
            rhs = stack.pop()
            discard = stack.pop()
            stack.append(Pair(lhs.pop(), rhs))
        else:
            stack.append(int(c))
    return stack.pop()


if __name__ == "__main__":
    numbers = []
    with open("input.txt", "r") as f:
        while 1:
            line = f.readline().strip()
            if not line:
                break
            numbers.append(parse_line(line))
 
    part_one(numbers)

