def part_one():
    pass

def part_two():
    pass

if __name__ == "__main__":
    counts = []

    count = 0
    max = 0

    with open("input", "r") as f:
        end = False
        while 1:
            line = f.readline().strip()
            if not line:
                if count > max:
                    max = count
                counts.append(count)
                count = 0
                if end:
                    break
                end = True
                continue
            end = False

            print(line)
            count += int(line)
            
    print(max)



    print(sorted(counts))

    result_one = part_one()
    print(f"part one: {result_one}")


    result_two = part_two()
    print(f"part two: {result_two}")

