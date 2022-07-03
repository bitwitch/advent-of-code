import sys

class Vec3:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    def __eq__(self, other):
        return self.x == other.x and self.y == other.y and self.z == other.z
    def __add__(self, other):
        return Vec3(self.x+other.x, self.y+other.y, self.z+other.z)
    def __sub__(self, other):
        return Vec3(self.x-other.x, self.y-other.y, self.z-other.z)
    def __hash__(self):
        return hash((self.x, self.y, self.z))
    def __str__(self):
        return f"({self.x}, {self.y}, {self.z})"
    def copy(self):
        return Vec3(self.x, self.y, self.z)

    def face_axis(self, axis):
        if axis == "+x" or axis == "x":
            self.x, self.z = -self.z, self.x
        if axis == "-x":
            self.x, self.z = self.z, -self.x
        if axis == "+y" or axis == "y":
            self.y, self.z = self.z, -self.y
        if axis == "-y":
            self.y, self.z = -self.z, self.y
        if axis == "+z" or axis == "z":
            self.x, self.z = -self.x, -self.z
        if axis == "-z":
            pass
        return self

    def rotate(self, deg):
        if deg == 0:
            pass
        elif deg == 90:
            self.x, self.y = -self.y, self.x
        elif deg == 180:
            self.x, self.y = -self.x, -self.y
        elif deg == 270:
            self.x, self.y = self.y, -self.x
        else:
            print("Error: only 90 degree increment rotations supported")
        return self

    def transform(self, t):
        self.face_axis(t.facing).rotate(t.rotation)
        self = self + t.offset
        return self


class Transform:
    def __init__(self, facing, rotation, offset):
        self.facing = facing
        self.rotation = rotation
        self.offset = offset
    def __str__(self):
        return f"({self.facing}, {self.rotation}, {str(self.offset)}"

def scanners_are_adjacent(s1, s2, transform):
    base = set(s1)
    count = 0
    # print("transformed points:")
    for p in s2:
        rel = p.copy().transform(transform)
        # print(f"    {rel}")
        if rel in base:
            count +=1
        if count > 11:
            return True
    # print("\n")
    return False


def check_scanner_pair(s1, s2):
    facings = ["x", "-x", "y", "-y", "z", "-z"]
    rotations = [0, 90, 180, 270]

    for p1 in s1:
        for p2 in s2:
            for f in facings:
                for r in rotations:
                    rotated = p2.copy().face_axis(f).rotate(r)
                    offset = p1 - rotated
                    transform = Transform(f, r, offset)
                    if scanners_are_adjacent(s1, s2, transform):
                        return (True, transform)
    return (False, None)


def part_one(scanners):
    transforms = []
    for i in len(scanners):
        transforms.append([None for j in len(scanners)])

    transforms[0][0] = Transform("-z", 0, Vec3(0,0,0))
    beacons = set(scanners[0])

    for i, s1 in enumerate(scanners):
        for j in range(i+1, len(scanners)):
            s2 = scanners[j]
            success, transform = check_scanner_pair(s1, s2)
            if success:

                # LEFT OFF HERE
                # LEFT OFF HERE
                # LEFT OFF HERE
                # LEFT OFF HERE
                # LEFT OFF HERE
                # LEFT OFF HERE

                # at this point, we have determined a transform from s2 to s1,
                # however we may not know a transform from s1 to zero yet and
                # thus don't know s2 relative to zero. how should we keep track
                # of this as we go, so that at the end we can combine all the
                # info we know about how to transform to coords relative to
                # each scanner and get back to zero
                #
                # seems like what i want to do is at each step store the
                # transform from the current scanner (call it s2) to zero, so when you find
                # a new scanner (s3) and know the transform from s3 to s2, you
                # can then apply the transform from s2 to zero, and thus obtain a
                # transform from s3 to zero, storing that, etc..
                #
                # the problem is with the way I am representing transforms,
                # they do not currently have a way to combine transforms, only
                # apply a transform to a vector

                # LEFT OFF HERE
                # LEFT OFF HERE
                # LEFT OFF HERE
                # LEFT OFF HERE



                transforms[j][i] = transform

                if transforms[i][0]:

                to_zero = transforms[i][0]

                points = s2

                # transform all points to be relative to scanner zero
                # add these to beacons
                print("Points relative to scanner zero:")
                for p in points:

                    rel = p.copy().transform(transform).transform(to_zero)
                    print(f"    {rel}")
                    beacons.add(rel)
    print(f"Found {len(beacons)} beacons")


def print_orientations(scanner):
    facings = ["x", "-x", "y", "-y", "z", "-z"]
    rotations = [0, 90, 180, 270]

    orientations = []
    i = 0
    for facing in facings:
        for rotation in rotations:
            orientation = []
            print(f"orientation {i}:")
            for p in scanner:
                t = Transform(facing, rotation, Vec3(0,0,0))
                # print(t)
                # new_p = p.copy().face_axis(facing).rotate(rotation)
                new_p = p.copy().transform(t)
                print(f"    {new_p}")
                orientation.append(new_p)
            orientations.append(orientation)
            i += 1
    print(len(orientations))


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
                scanners[scanner_index].append(Vec3(int(line[0]), int(line[1]), int(line[2])))
    # print_orientations(scanners[0])
    part_one(scanners)







