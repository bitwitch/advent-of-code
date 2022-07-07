# digits contain certain segments
segments_in_digits = [
    set(['a','b','c','e','f','g']),
    set(['c','f']),
    set(['a','c','d','e','g']),
    set(['a','c','d','f','g']),
    set(['b','c','d','f']),
    set(['a','b','d','f','g']),
    set(['a','b','d','e','f','g']),
    set(['a','c','f']),
    set(['a','b','c','d','e','f','g']),
    set(['a','b','c','d','f','g'])
]

# find segments that are in every pattern except for one
def segs_absent_from_n_digits(patterns, n):
    segs = set()
    for seg in 'abcdefg':
        count = 0
        for pattern in patterns:
            if seg in pattern:
                count += 1
        if count == len(patterns)-n:
            segs.add(seg)
    return segs


def segs_not_in_all(patterns):
    union = set()
    for digit in patterns:
        union.update(digit)
    intersection = patterns[0].copy()
    for digit in patterns:
        intersection.intersection_update(digit)
    return union.difference(intersection)


def segs_in_a_not_b(digits, a, b):
    assert (a in [1,4,7,8]), f"segs_in_a_not_b only valid for digits 1, 4, 7, and 8"

    segs = set()
    for seg in digits[a]:
        if seg not in digits[b]:
            segs.add(seg)
    return segs


def reduce_candidates_to_set(candidates, actuals, new_set):
    for actual in actuals:
        to_remove = []
        for c in candidates[actual]:
            if c not in new_set:
                to_remove.append(c)
        for c in to_remove:
            candidates[actual].remove(c)
        if len(candidates[actual]) == 1:
            (correct_candidate,) = candidates[actual]
            remove_from_other_candidates(candidates, correct_candidate, actual)


def remove_from_other_candidates(candidates, seg, actual):
    for c_act, candidate in candidates.items():
        if actual == c_act:
            continue
        candidate.discard(seg)


def mapping_complete(candidates):
    for candidates in candidates.values():
        if len(candidates) != 1:
            return False
    return True


def decode_digit(digit, candidates):
    actual = set()
    for seg in digit:
        for actual_seg, scrambled_seg in candidates.items():
            if seg in scrambled_seg:
                actual.add(actual_seg)
                break
    
    for digit, segs in enumerate(segments_in_digits):
        if actual == segs:
            return digit

    assert False, f"digit not found {segs}"


def part_one(outputs_list):
    count = 0
    for outputs in outputs_list:
        for output in outputs:
            nsegs = len(output)
            if nsegs in [2,3,4,7]:
                count += 1
    print(f"Found {count} instances of 1,4,7,8")


def part_two(patterns_list, outputs_list):
    result = 0
    for i in range(len(patterns_list)):
        patterns = patterns_list[i]
        outputs = outputs_list[i]

        # segments are candidates for certain mappings
        digits = [ set(), set(), set(), set(), set(), set(), set(), set(), set(), set() ]
        five_segs = []
        six_segs = []

        candidates = { 'a': set(), 'b': set(), 'c': set(), 'd': set(), 'e': set(), 'f': set(), 'g': set() }

        for pattern in patterns:
            nsegs = len(pattern)

            # digit 1
            if nsegs == 2:
                digit = 1
                for seg in pattern:
                    digits[digit].add(seg)
                    for actual in segments_in_digits[digit]:
                        candidates[actual].add(seg)
            # digit 7
            if nsegs == 3:
                digit = 7
                for seg in pattern:
                    digits[digit].add(seg)
                    for actual in segments_in_digits[digit]:
                        candidates[actual].add(seg)
            # digit 4
            if nsegs == 4:
                digit = 4
                for seg in pattern:
                    digits[digit].add(seg)
                    for actual in segments_in_digits[digit]:
                        candidates[actual].add(seg)

            # digits 2,3,5
            if nsegs == 5:
                five_segs.append(set(pattern))
                for seg in pattern:
                    for digit in [2,3,5]:
                        for actual in segments_in_digits[digit]:
                            candidates[actual].add(seg)

            # digits 0,6,9
            if nsegs == 6:
                six_segs.append(set(pattern))
                for seg in pattern:
                    for digit in [0,6,9]:
                        for actual in segments_in_digits[digit]:
                            candidates[actual].add(seg)

            # digit 8
            if nsegs == 7:
                digit = 8
                for seg in pattern:
                    digits[digit].add(seg)
                    for actual in segments_in_digits[digit]:
                        candidates[actual].add(seg)

        # the seg that is in all but one digit maps to f
        segs = segs_absent_from_n_digits(patterns, 1)
        reduce_candidates_to_set(candidates, ['f'], segs)

        # the seg that is all but four digits maps to b
        segs = segs_absent_from_n_digits(patterns, 4)
        reduce_candidates_to_set(candidates, ['b'], segs)

        # the seg that is all but six digits maps to e
        segs = segs_absent_from_n_digits(patterns, 6)
        reduce_candidates_to_set(candidates, ['e'], segs)

        # the seg in 7 not in 1 maps to a
        segs = segs_in_a_not_b(digits, 7, 1)
        reduce_candidates_to_set(candidates, ['a'], segs)

        # the segs that are in all but two digits map to [a,c]
        segs = segs_absent_from_n_digits(patterns, 2)
        reduce_candidates_to_set(candidates, ['a','c'], segs)


        # the segs in 4 not in 1 map to [b,d]
        # segs = segs_in_a_not_b(digits, 4, 1)
        # reduce_candidates_to_set(candidates, ['b','d'], segs)

        # the segs in 8 not in 4 map to [a,e,g]
        # segs = segs_in_a_not_b(digits, 8, 4)
        # reduce_candidates_to_set(candidates, ['a','e','g'], segs)
        
        #
        # 6 segment digits
        #
        # any segment not in all three maps to [c,d,e]
        segs = segs_not_in_all(six_segs)
        reduce_candidates_to_set(candidates, ['c','d','e'], segs)
        
        if not mapping_complete(candidates):
            print("Error: failed to map all candidates to a single segment")
            return

        number = 0
        multiplier = 1
        for output in reversed(outputs):
            digit = decode_digit(output, candidates)
            number += digit * multiplier
            multiplier *= 10
        
        result += number

    print(f"sum = {result}")


if __name__ == "__main__":
    patterns_list = []
    outputs_list = []
    with open("input", "r") as f:
        while 1:
            line = f.readline().strip()
            if not line:
                break
            line = line.split("|")
            patterns_list.append(line[0].strip().split(" "))
            outputs_list.append(line[1].strip().split(" "))

    part_one(outputs_list)
    part_two(patterns_list, outputs_list)

