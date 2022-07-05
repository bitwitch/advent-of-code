def print_image(image):
    print("")
    for row in image:
        print(''.join(row))


def image_copy(image):
    result = []
    for row in image:
        result.append(row.copy())
    return result


def pad_image(image, amount=2, char='.'):
    original   = image_copy(image)
    row_target = len(image) + 2*amount
    col_target = len(image[0]) + 2*amount
    empty_col  = [char] * col_target

    for i in range(len(original)):
        image[i] = empty_col.copy()
    for i in range(len(original), row_target):
        image.append(empty_col.copy())

    top  = (len(image) - len(original)) // 2
    left = (len(image[0]) - len(original[0])) // 2

    for j in range(len(original)):
        for i in range(len(original[0])):
            image[j+top][i+left] = original[j][i]


def enhance(algo, image, border='.'):
    pad_image(image, 2, border)
    result = image_copy(image)
    row_count, col_count = len(image), len(image[0])

    for row in range(1, row_count-1):
        for col in range(1, col_count-1):
            index = ''
            for j in range(row-1, row+2):
                for i in range(col-1, col+2):
                    index += '1' if image[j][i] == '#' else '0'
            index = int(index,2)
            result[row][col] = algo[index]

    # simulate inifinite image
    new_border = algo[0] if border == '.' else algo[511]
    for i in range(col_count):
        result[0][i] = new_border
        result[row_count-1][i] = new_border
    for j in range(row_count):
        result[j][0] = new_border
        result[j][col_count-1] = new_border

    return result


def lit_pixels(image):
    count = 0
    for row in image:
        for col in row:
            count += 1 if col == '#' else 0
    return count
 

def part_one(algo, image):
    print_image(image)

    image1 = enhance(algo, image, '.')
    print_image(image1)

    image2 = enhance(algo, image1, '#')
    print_image(image2)

    print(lit_pixels(image2))


def part_two(algo, image):
    current = image.copy()
    for i in range(50):
        border = '.' if i%2==0 else '#'
        current = enhance(algo, current, border)
    print(lit_pixels(current))


if __name__ == "__main__":
    image = []
    with open("input.txt", "r") as f:
        algo = f.readline().strip()
        f.readline()
        while 1:
            line = f.readline().strip()
            if not line:
                break
            image.append(list(line))
            
    # print_image(image)
    # pad_image(image, 10)
    # print_image(image)

    # part_one(algo, image)
    part_two(algo, image)

