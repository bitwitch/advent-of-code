class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __str__(self):
        return f"({self.x}, {self.y})"

def part_one(scanners):
    # from the first beacon, get vectors to all other beacons


if __name__ == "__main__":
    scanners = []
    scanner_index = -1
    with open("small_input.txt", "r") as f:
        while 1:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            if not line:
                continue
            if "scanner" in line:
                scanners.append([])
                scanner_index += 1
            else:
                line = line.split(",")
                scanners[scanner_index].append(Point(int(line[0]), int(line[1])))
    part_one(scanners)
