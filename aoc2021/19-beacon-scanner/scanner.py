from time import time
import sys
from math import sin,cos,pi,floor
import numpy as np

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
            sys.exit(1)
        return self


class Mat4:
    def __init__(self):
        self.matrix = [
            [1,0,0,0],
            [0,1,0,0],
            [0,0,1,0],
            [0,0,0,1]]

    def __str__(self):
        return f"{self.matrix[0]}\n{self.matrix[1]}\n{self.matrix[2]}\n{self.matrix[3]}"

    def __eq__(self, other):
        return (self.matrix[0] == self.matrix[0] and
                self.matrix[1] == self.matrix[1] and
                self.matrix[2] == self.matrix[2] and
                self.matrix[3] == self.matrix[3])

    def __mul__(self, other):
        if type(other) is Vec3:
            return self.mul_vec3(other)
        elif type(other) is Mat4:
            return self.mul_mat4(other)
        else:
            print("Mat4 multiplication must have Mat4 or Vec3 on right hand side")
            sys.exit(1)
        
    def mul_mat4(self, other):
        result = []
        # for each row in self
        for selfrow in self.matrix:
            new_row = []
            # for each col in other
            for othercol in range(4):
                val = 0
                # take sum of products of each col in selfrow and each row in othercol
                for i in range(4):
                    val += selfrow[i] * other.matrix[i][othercol]
                new_row.append(floor(val+.5))
            result.append(new_row)
        m = Mat4()
        m.matrix = result
        return m
        
    def mul_vec3(self, v3):
        other = [v3.x, v3.y, v3.z, 1]
        result = []
        # for each row in self
        for selfrow in self.matrix:
            val = 0
            # take sum of products of each col in selfrow and each row in vector
            for i in range(4):
                val += selfrow[i] * other[i]
            result.append(floor(val+.5))
        return Vec3(result[0], result[1], result[2])
        

    @staticmethod
    def from_rotation(deg, axis):
        rad = deg * pi / 180
        m = Mat4()
        if (axis == "x"):
            m.matrix = [
                [1,    0,         0,     0],
                [0, floor(cos(rad)+.5), floor(-sin(rad)+.5), 0],
                [0, floor(sin(rad)+.5),  floor(cos(rad)+.5), 0],
                [0,    0,         0,     1]]
        elif (axis == "y"):
            m.matrix = [
                [ floor(cos(rad)+.5), 0, floor(sin(rad)+.5), 0],
                [    0,     1,    0,     0],
                [-sin(rad), 0, floor(cos(rad)+.5), 0],
                [    0,     0,    0,     1]]
        elif (axis == "z"):
            m.matrix = [
                [floor(cos(rad)+.5), floor(-sin(rad)+.5), 0, 0],
                [floor(sin(rad)+.5),  floor(cos(rad)+.5), 0, 0],
                [   0,         0,     1, 0],
                [   0,         0,     0, 1]]
        else:
            print("Error: from_rotation only accepts x, y, or z as axis")
        return m

    @staticmethod
    def from_translation(v):
        m = Mat4()
        m.matrix[0][3] = v.x
        m.matrix[1][3] = v.y
        m.matrix[2][3] = v.z
        return m

    @staticmethod
    def inverse(m):
        result = Mat4()
        result.matrix = np.linalg.inv(np.array(m.matrix)).tolist()
        return result

def scanners_are_adjacent(s1, s2, transform):
    base = set(s1)
    count = 0
    # print("transformed points:")
    for p in s2:
        rel = transform * p
        # print(f"    {rel}")
        if rel in base:
            count +=1
        if count > 11:
            return True
    # print("\n")
    return False


def check_scanner_pair(s1, s2):
    facings = [
        Mat4.from_rotation(-90, "y"), #  x                                                         
        Mat4.from_rotation( 90, "y"), # -x
        Mat4.from_rotation( 90, "x"), #  y
        Mat4.from_rotation(-90, "x"), # -y
        Mat4.from_rotation(180, "y"), #  z
        Mat4() ]                      # -z

    rotations = [
        Mat4(),               
        Mat4.from_rotation(-90, "z"),
        Mat4.from_rotation(-180, "z"),
        Mat4.from_rotation(-270, "z") ]

    orientations = []
    for f in facings:
        for r in rotations:
            orientations.append(f * r)

    for p1 in s1:
        for p2 in s2:
            for orientation in orientations:
                rotated = orientation * p2
                offset = p1 - rotated
                translation = Mat4.from_translation(offset)
                transform = translation * orientation
                if scanners_are_adjacent(s1, s2, transform):
                    return (True, transform)
    return (False, None)


# def check_scanner_pair(s1, s2):
    # facings = [
        # Mat4.from_rotation(-90, "y"), #  x
        # Mat4.from_rotation( 90, "y"), # -x
        # Mat4.from_rotation( 90, "x"), #  y
        # Mat4.from_rotation(-90, "x"), # -y
        # Mat4.from_rotation(180, "y"), #  z
        # Mat4() ]                      # -z

    # rotations = [
        # Mat4(),               
        # Mat4.from_rotation(-90, "z"),
        # Mat4.from_rotation(-180, "z"),
        # Mat4.from_rotation(-270, "z") ]

    # orientations = []
    # for f in facings:
        # for r in rotations:
            # orientations.append(f * r)

    # for orientation in orientations:
        # rotated = orientation * p2
        # offset = p1 - rotated
        # translation = Mat4.from_translation(offset)
        # transform = translation * orientation
        # if scanners_are_adjacent(s1, s2, transform):
            # return (True, transform)
    # return (False, None)



# dfs of transform graph, find path to scanner 0
def find_transform_to_zero(transforms, index, path, visited):
    if transforms[index][0]:
        path.append(transforms[index][0])
        return True

    for j in range(len(transforms)):
        if j == index or not transforms[index][j] or j in visited:
            continue
        path.append(transforms[index][j])
        visited.add(j)
        if find_transform_to_zero(transforms, j, path, visited):
            return True
        path.pop()
        visited.remove(j)

    return False


def part_one(scanners):
    transforms = []
    for i in range(len(scanners)):
        transforms.append([None for j in range(len(scanners))])

    transforms[0][0] = Mat4()
    beacons = set(scanners[0])

    # make a graph of coord system transforms
    for i, s1 in enumerate(scanners):
        for j in range(i+1, len(scanners)):
            s2 = scanners[j]
            success, transform = check_scanner_pair(s1, s2)
            if success:
                transforms[j][i] = transform
                transforms[i][j] = Mat4.inverse(transform)
                if transforms[i][0]:
                    to_zero = transforms[i][0]
                    # NOTE(shaw): check order of this mul
                    transforms[j][0] = to_zero * transform

    # find path from each scanner to zero
    for i, scanner in enumerate(scanners):
        path = []
        visited = set()
        find_transform_to_zero(transforms, i, path, visited)
        if not path:
            print(f"Failed to find a path from {i} to zero")
            sys.exit(1)

        transform = Mat4()
        for t in path:
            transform = t * transform
        # update transforms with new path to zero
        transforms[i][0] = transform
        for p in scanner:
            rel = transform * p
            beacons.add(rel)

    print(f"Found {len(beacons)} beacons")

    # write out transforms to a file
    # with open("transforms.txt", "w") as f:
        # for i in range(len(transforms)):
            # f.write(str(transforms[i][0]))
            # f.write("\n")

    transforms_to_zero = []
    for row in transforms:
        transforms_to_zero.append(row[0])
    part_two(transforms_to_zero)



def print_orientations(scanner):
    facings = [
        Mat4.from_rotation(-90, "y"), #  x
        Mat4.from_rotation( 90, "y"), # -x
        Mat4.from_rotation( 90, "x"), #  y
        Mat4.from_rotation(-90, "x"), # -y
        Mat4.from_rotation(180, "y"), #  z
        Mat4() ]                      # -z

    rotations = [
        Mat4(),               
        Mat4.from_rotation(-90, "z"),
        Mat4.from_rotation(-180, "z"),
        Mat4.from_rotation(-270, "z") ]

    transforms = []
    for f in facings:
        for r in rotations:
            transforms.append(f * r)

    orientations = []
    i = 0
    for transform in transforms:
        orientation = []
        print(f"orientation {i}:")
        for p in scanner:
            new_p = transform * p
            print(f"    {new_p}")
            orientation.append(new_p)
        orientations.append(orientation)
        i += 1

    print(len(orientations))


def test_matrices():
    m1 = Mat4()
    m2 = Mat4()

    m1.matrix = [
        [1,2,3,4],
        [5,6,7,8],
        [9,10,11,12],
        [13,14,15,16]]

    m2.matrix = [
        [2,4,6,8],
        [10,12,14,16],
        [18,20,22,24],
        [26,28,30,32]]

    m3 = m1*m2

    print("m1")
    print(m1)
    print("m2")
    print(m2)
    print("m3")
    print(m3)

    m4 = Mat4.from_rotation(90, "x")
    print("m4")
    print(m4)

    m5 = Mat4.from_rotation(90, "y")
    print("m5")
    print(m5)

    m6 = Mat4.from_rotation(90, "z")
    print("m6")
    print(m6)

    # test inverse
    # m = Mat4.from_translation(Vec3(1,2,3))
    # m_inv = Mat4.inverse(m)
    # print("inverse")
    # print(m_inv.matrix)
    # print(type(m_inv.matrix))

    # if i know transform from l0 to l1, does the inverse give me l1 to l0??
    l1 = Mat4.from_rotation(-90, "x") * Mat4.from_rotation(-90, "y") * Mat4.from_translation(Vec3(-2,-2,0))
    p = Vec3(3,3,3)

    p_l1 = l1 * p
    print(f"p from l1: {p_l1}")

    p_l0 = Mat4.inverse(l1) * p_l1
    print(f"p from l0: {p_l0}")

def part_two(transforms):
    # transforms = [[[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]], [[1, 0, 0, 39], [0, 0, 1, -3432], [0, -1, 0, -3735], [0, 0, 0, 1]], [[0, 1, 0, -2296], [0, 0, -1, 1210], [-1, 0, 0, -2351], [0, 0, 0, 1]], [[0, 0, 1, -1230], [0, -1, 0, 1218], [1, 0, 0, -4909], [0, 0, 0, 1]], [[0, 0, -1, 152], [0, 1, 0, 57], [1, 0, 0, -3678], [0, 0, 0, 1]], [[0, -1, 0, -1125], [0, 0, -1, 2470], [1, 0, 0, -1239], [0, 0, 0, 1]], [[-1, 0, 0, 73], [0, 0, 1, 2415], [0, 1, 0, -2429], [0, 0, 0, 1]], [[1, 0, 0, -1041], [0, 0, -1, 3750], [0, 1, 0, -2376], [0, 0, 0, 1]], [[-1, 0, 0, -1166], [0, 1, 0, 2471], [0, 0, -1, -3551], [0, 0, 0, 1]], [[0, 0, -1, -2428], [0, -1, 0, -2405], [-1, 0, 0, -3634], [0, 0, 0, 1]], [[0, 1, 0, 140], [0, 0, 1, 1336], [1, 0, 0, -2439], [0, 0, 0, 1]], [[0, 1, 0, -2393], [-1, 0, 0, 4828], [0, 0, 1, -81], [0, 0, 0, 1]], [[0, 0, -1, 10], [1, 0, 0, 4959], [0, -1, 0, 1], [0, 0, 0, 1]], [[0, -1, 0, -1133], [0, 0, 1, -2226], [-1, 0, 0, -3620], [0, 0, 0, 1]], [[0, 0, -1, 86], [-1, 0, 0, 4852], [0, 1, 0, -1290], [0, 0, 0, 1]], [[0, 0, 1, -1111], [-1, 0, 0, -15], [0, -1, 0, -1172], [0, 0, 0, 1]], [[0, 0, 1, -1168], [0, 1, 0, 4811], [-1, 0, 0, -2507], [0, 0, 0, 1]], [[0, -1, 0, 84], [-1, 0, 0, -12], [0, 0, -1, -2408], [0, 0, 0, 1]], [[0, 1, 0, -2300], [1, 0, 0, 2420], [0, 0, -1, -1172], [0, 0, 0, 1]], [[1, 0, 0, 51], [0, -1, 0, 6081], [0, 0, -1, -1202], [0, 0, 0, 1]], [[0, 0, 1, 43], [1, 0, 0, -2315], [0, 1, 0, -2539], [0, 0, 0, 1]], [[-1, 0, 0, 1189], [0, 0, -1, 1325], [0, -1, 0, -4821], [0, 0, 0, 1]], [[0, -1, 0, 40], [1, 0, 0, -2227], [0, 0, 1, -3679], [0, 0, 0, 1]], [[-1, 0, 0, -7], [0, -1, 0, 2465], [0, 0, 1, -4812], [0, 0, 0, 1]], [[-1, 0, 0, 139], [0, 0, 1, 7342], [0, 1, 0, -1144], [0, 0, 0, 1]], [[-1, 0, 0, 1176], [0, -1, 0, 1237], [0, 0, 1, -5981], [0, 0, 0, 1]], [[0, -1, 0, -1061], [0, 0, 1, 3690], [-1, 0, 0, -1300], [0, 0, 0, 1]], [[0, 1, 0, -1201], [0, 0, 1, 6159], [1, 0, 0, -1249], [0, 0, 0, 1]], [[0, 0, 1, 61], [0, -1, 0, 1318], [1, 0, 0, -4788], [0, 0, 0, 1]], [[0, -1, 0, 130], [0, 0, -1, 2535], [1, 0, 0, -3661], [0, 0, 0, 1]], [[0, 1, 0, 1182], [1, 0, 0, 2551], [0, 0, -1, -6086], [0, 0, 0, 1]], [[1, 0, 0, -1172], [0, 1, 0, 1211], [0, 0, 1, -2375], [0, 0, 0, 1]], [[0, 1, 0, 4], [0, 0, -1, 6024], [-1, 0, 0, -123], [0, 0, 0, 1]], [[1, 0, 0, 91], [0, 0, -1, 1367], [0, 1, 0, -3627], [0, 0, 0, 1]], [[1, 0, 0, 37], [0, 0, -1, 3619], [0, 1, 0, -1177], [0, 0, 0, 1]], [[0, 0, -1, -1069], [0, 1, 0, 1334], [1, 0, 0, -1335], [0, 0, 0, 1]], [[1, 0, 0, 25], [0, 0, -1, -1081], [0, 1, 0, -3562], [0, 0, 0, 1]], [[0, 0, 1, 30], [1, 0, 0, 2407], [0, 1, 0, -5973], [0, 0, 0, 1]], [[0, 1, 0, 147], [1, 0, 0, 155], [0, 0, -1, -1150], [0, 0, 0, 1]], [[0, -1, 0, -1091], [0, 0, 1, 4964], [-1, 0, 0, -52], [0, 0, 0, 1]]]


    points = []

    m = Mat4()
    origin = Vec3(0,0,0)

    for t in transforms:
        # m.matrix = t
        points.append(t * origin)

    max_dist = -1
    for i in range(len(points)-1):
        p1 = points[i]
        print(f"{p1}\n")
        for j in range(i, len(points)):
            p2 = points[j]
            dist = abs(p1.x - p2.x) + abs(p1.y - p2.y) + abs(p1.z - p2.z)
            max_dist = max(max_dist, dist)

    print(max_dist)


if __name__ == "__main__":
    start = time()
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
    # test_matrices()
    # print_orientations(scanners[0])
    part_one(scanners)

 
    print(f"Execution time: {time()-start} seconds")



