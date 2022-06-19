def get_input(points, folds):
    dimensions = (-1,-1)
    with open("input.txt", "r") as f:
        # read points
        while 1:
            raw = f.readline().strip()
            if not raw:
                break
            raw = raw.split(',')

            point = (int(raw[0]), int(raw[1]))

            # get max x and y
            if point[0] > dimensions[0]:
                dimensions = (point[0], dimensions[1])
            if point[1] > dimensions[1]:
                dimensions = (dimensions[0], point[1])

            points.add(point)
        
        # read folds
        while 1:
            raw = f.readline()
            if not raw:
                break
            # horizontal
            try:
                ok = raw.index("y=")
                raw = raw.strip().split('=')
                folds.append((0, int(raw[1])))
                continue
            except ValueError:
                pass
            # vertical
            try:
                ok = raw.index("x=") + 2
                raw = raw.strip().split('=')
                folds.append((int(raw[1]),0))
                continue
            except ValueError:
                pass

    return dimensions


def display_points(points, dim):
    for y in range(dim[1]+1):
        line = ''
        for x in range(dim[0]+1):
            if (x,y) in points:
                line += '#'
            else:
                line += '.'
        print(line)
    print('\n')

def generate_image(name, points, dim):
    width = dim[0]
    height = dim[1]

    colors = [
        (222, 24, 24),
        (94, 222, 24),
        (219, 105, 24),
        (24, 219, 148),
        (230, 222, 16),
        (43, 20, 217),
        (196, 20, 190),
        (20, 161, 196)]

    with open(name+".ppm", "w") as f:
        f.write(f"P3 {width+1} {height+1} {255}\n")
        for y in range(height+1):
            for x in range(width+1):
                if (x,y) in points:
                    c = colors[x // 5]
                    f.write(f"{c[0]} {c[1]} {c[2]} ") 
                else:
                    f.write(f"0 0 0 ")
            f.write("\n")


if __name__ == "__main__":
    points = set()
    folds = []
    dimensions = get_input(points, folds)

    # display_points(points, dimensions)
    print(f"dimensions={dimensions}")
    print(len(points))

    
    for i, fold in enumerate(folds):
        to_add = []
        to_remove = []
        for point in points:
            dim = 0
            if fold[0] == 0:
                dim = 1
            if point[dim] < fold[dim]:
                continue
            to_remove.append(point)

            if dim == 1:
                folded = (point[0], dimensions[1] - point[1])
            else:
                folded = (dimensions[0] - point[0], point[1])
            to_add.append(folded)

        # update set of points
        for point in to_remove:
            points.remove(point)
        for point in to_add:
            points.add(point)


        if fold[0] == 0:
            dimensions = (dimensions[0], fold[1]-1)
        else:
            dimensions = (fold[0]-1, dimensions[1])

        if dimensions[0] < 80 and dimensions[1] < 80:
            display_points(points, dimensions)

        print(f"fold={fold} dimensions={dimensions}")

        print(len(points))

    generate_image(f"out", points, dimensions)


