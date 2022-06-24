class Parser:
    def __init__(self, binary):
        self.binary = binary
        self.index = 0

    def read_bits(self, count):
        number = int(binary[self.index : self.index+count], 2)
        self.index += count
        return number

    def read_literal(self):
        number = 0
        while self.read_bits(1):
            number = (number << 4) | self.read_bits(4)
        number = (number << 4) | self.read_bits(4)
        return number

    def read_packet(self):
        packet_version = self.read_bits(3)
        packet_type = self.read_bits(3)

        result = Node(packet_version, packet_type)

        # Literal
        if (packet_type == 4):
            result.value = self.read_literal()
            return result

        # Operator
        len_type = self.read_bits(1)
        if len_type == 0:
            bits_in_subpackets = self.read_bits(15)
            end = self.index + bits_in_subpackets
            while self.index < end:
                child = self.read_packet()
                result.children.append(child)
        else:
            num_subpackets = self.read_bits(11)
            for i in range(num_subpackets):
                child = self.read_packet()
                result.children.append(child)

        return result


class Node:
    def __init__(self, version, typ):
        self.version = version
        self.type = typ
        self.children = []
        self.value = None

    def __str__(self):
        if self.value:
            return f"{self.value}"
        result = f"version={self.version} type={self.type} children=["
        for c in self.children:
            result += f"\n    {c.__str__()},"
        result += " ]"
        return result


def get_version_sum(node):
    if not node:
        return 0
    result = node.version
    for child in node.children:
        result += get_version_sum(child)
    return result


def part_one(binary):
    parser = Parser(binary)
    ast = parser.read_packet()
    version_sum = get_version_sum(ast)
    print(version_sum)



def part_two(binary):
    pass

if __name__ == "__main__":
    with open("input.txt", "r") as f:
        raw = f.read().strip()
    binary = ""
    for c in raw:
        binary += format(int(c,16), "04b")

    part_one(binary)
    # part_two(binary)
    



