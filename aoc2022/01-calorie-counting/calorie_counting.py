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
            count += int(line)
            
    print(f"part_one: {max}")

    sum = 0
    for c in sorted(counts)[-3:]:
        sum += c

    print(f"part_two: {sum}")

