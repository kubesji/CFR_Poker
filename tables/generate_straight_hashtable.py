from license import LICENSE

hash_table = ['x'] * 16384


NO_POTENTIAL_STRAIGHT = 'x'
STRAIGHT = 'S'
MISSING_ONE_IN_THE_MIDDLE = 'm'
MISSING_TWO_IN_THE_MIDDLE = 'M'
MISSING_THE_HIGHEST_OR_LOWEST = 'b'
MISSING_TWO_HIGHEST_OR_LOWEST = 'B'
MISSING_TWO_CARDS_TO_STRAIGHT = 'A'

def get_hash(cards):
    hash = 0
    for c in cards:
        hash |= 1 << c
    return hash

def check_3(cards):
    assert len(cards) == 3
    if cards[2] - cards[0] == 4:
        # Two cards -> capital, in the MIDDLE
        return MISSING_TWO_IN_THE_MIDDLE
    elif cards[2] - cards[0] == 3:
        # missing two cards -> capital, either upper or middle or bottom -> ALL
        return MISSING_TWO_CARDS_TO_STRAIGHT
    elif cards[2] - cards[0] == 2:
        # missing two cards -> capital, either upper or bottom -> B
        return MISSING_TWO_HIGHEST_OR_LOWEST
    else:
        return NO_POTENTIAL_STRAIGHT
    

def check_4(cards):
    assert len(cards) == 4
    if cards[3] - cards[0] == 4:
        # missing one in the middle
        return MISSING_ONE_IN_THE_MIDDLE
    elif cards[3] - cards[0] == 3:
        # either upper or bottom -> both
        return MISSING_THE_HIGHEST_OR_LOWEST
    elif cards[3] == 12:
        # We have A in the mix
        if cards[2] == 2 or cards[2] == 3:
            # highest other card is either 4 or 5 -> we put it in the same bin of one card missing
            return MISSING_ONE_IN_THE_MIDDLE
        else:
            return NO_POTENTIAL_STRAIGHT
    else:
        return NO_POTENTIAL_STRAIGHT
    

def check_5(cards):
    assert len(cards) == 5
    if cards[4] - cards[0] == 4:
        # return STRAIGHT
        return STRAIGHT
    # return a + 2 + 3 + 4 + 5
    elif cards[4] == 12 and cards[3] == 3:
        # return STRAIGHT
        return STRAIGHT
    else:
        return NO_POTENTIAL_STRAIGHT
        

def find_value(cards):
    # cards = sorted(list(set(cards)))

    if len(cards) < 3:
        return NO_POTENTIAL_STRAIGHT
    
    if len(cards) == 3:
        return check_3(cards)
    
    if len(cards) == 4:
        value = check_4(cards)
        if value != NO_POTENTIAL_STRAIGHT:
            return value

        for i in range(2, 0, -1):
            value = check_3(cards[i-1:i+2])
            if value != NO_POTENTIAL_STRAIGHT:
                return value
    
    if len(cards) == 5:
        value = check_5(cards)
        if value != NO_POTENTIAL_STRAIGHT:
            return value

        for i in range(2, 0, -1):
            value = check_4(cards[i-1:i+3])
            if value != NO_POTENTIAL_STRAIGHT:
                return value

        for i in range(3, 0, -1):
            value = check_3(cards[i-1:i+2])
            if value != NO_POTENTIAL_STRAIGHT:
                return value
    
    if len(cards) == 6:
        for i in range(2, 0, -1):
            value = check_5(cards[i-1:i+4])
            if value != NO_POTENTIAL_STRAIGHT:
                return value

        for i in range(3, 0, -1):
            value = check_4(cards[i-1:i+3])
            if value != NO_POTENTIAL_STRAIGHT:
                return value

        for i in range(4, 0, -1):
            value = check_3(cards[i-1:i+2])
            if value != NO_POTENTIAL_STRAIGHT:
                return value
    
    if len(cards) == 7:
        for i in range(3, 0, -1):
            value = check_5(cards[i-1:i+4])
            if value != NO_POTENTIAL_STRAIGHT:
                return value

        for i in range(4, 0, -1):
            value = check_4(cards[i-1:i+3])
            if value != NO_POTENTIAL_STRAIGHT:
                return value

        for i in range(5, 0, -1):
            value = check_3(cards[i-1:i+2])
            if value != NO_POTENTIAL_STRAIGHT:
                return value
            
    return NO_POTENTIAL_STRAIGHT

def generate():
    for hash in range(len(hash_table)):
        cards = []
        for i in range(13):
            if hash & (1<<i) > 0:
                cards.append(i)

        value = find_value(cards)

        hash_table[hash] = value


generate()

with open("./possible_straights.cpp", "w+") as f:
    print(LICENSE, file=f)
    print("#include \"tables.h\"\n", file=f)
    print("#define NO_POTENTIAL_STRAIGHT\t\t\t'x'", file=f)
    print("#define STRAIGHT\t\t\t\t\t\t'S'", file=f)
    print("#define MISSING_ONE_IN_THE_MIDDLE\t\t'm'", file=f)
    print("#define MISSING_TWO_IN_THE_MIDDLE\t\t'M'", file=f)
    print("#define MISSING_THE_HIGHEST_OR_LOWEST\t'b'", file=f)
    print("#define MISSING_TWO_HIGHEST_OR_LOWEST\t'B'", file=f)
    print("#define MISSING_TWO_CARDS\t\t\t\t'A'\n\n", file=f)
    print(f"const int8_t straights[{len(hash_table)}] = ", end="", file=f)
    print("{\n\t", end="", file=f)
    for i, h in enumerate(hash_table):
        print(f"'{h}'", end="", file=f)
        if i+1 != len(hash_table):  
            print(f", ", end="", file=f)
        if (i+1) % 16 == 0:
            print("\n\t", end="", file=f)
    print("\n};", file=f)
    


