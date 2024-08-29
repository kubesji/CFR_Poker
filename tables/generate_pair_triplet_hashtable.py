from license import LICENSE

HASH_SIZE = 592
hash_table = [0] * HASH_SIZE
max_hash = 0

def get_count(cards):
    counter = [0] * 13

    for c in cards:
        counter[c % 13] += 1

    return counter

def get_hash(value):
    return (1 << ((value-1) * 3)) if value > 0 else 0

def hash_to_count(hash):
    n_singletons = hash & 0x07
    n_pairs = (hash >> 3) & 0x07
    n_triplets = (hash >> 6) & 0x07
    n_quadruplets = (hash >> 9) & 0x07

    counts = [0, n_singletons, n_pairs, n_triplets, n_quadruplets]
    #count cards
    weighted = [i*c for i, c in enumerate(counts)]
    sum_of_counts = sum(weighted)

    if 3 <= sum_of_counts <= 7:
        global max_hash
        max_hash = hash
        return counts
    
    return [0, 0, 0, 0, 0]

def find_value(counter):    
    # Is there four of a kind
    if counter[4] > 0:
        # And three of a kind
        if counter[3] > 0:
            return (1, 1, 0)
        # Or a pair
        elif counter[2] > 0:
            return (1, 0, 1)
        else:
            return (1, 0, 0)
    # Is there three of a kind
    if counter[3] > 0:
        # And another
        if counter[3] == 2:
            return (0, 2, 0)
        # Or two pairs
        elif counter[2] == 2:
            return (0, 1, 2)
        # Or a pair
        elif counter[2] == 1:
            return (0, 1, 1)
        else:
            return (0, 1, 0)
    # Is there a pair
    if counter[2] > 0:
        # And another
        if counter[2] == 3:
            return (0, 0, 3)
        # Or two pairs
        elif counter[2] == 2:
            return (0, 0, 2)
        # Or a pair
        elif counter[2] == 1:
            return (0, 0, 1)
        else:
            return (0, 0, 0)

    return (0, 0, 0)


# def generate(counter, depth, cards):
def generate():
    for hash in range(HASH_SIZE):
        counter = hash_to_count(hash)
        value = find_value(counter)
        hash_table[hash] = value


generate()

with open("./pairs.cpp", "w+") as f:
    print(LICENSE, file=f)
    print("#include \"tables.h\"\n", file=f)

    print(f"const uint8_t pairs[{len(hash_table)}][3] = ", end="", file=f)
    print("{\n\t", end="", file=f)
    for i, h in enumerate(hash_table):
        print(f"{{ {h[0]}, {h[1]}, {h[2]} }}", end="", file=f)
        if i+1 != len(hash_table):  
            print(f", ", end="", file=f)
        if (i+1) % 8 == 0:
            print("\n\t", end="", file=f)
    print("\n};\n", file=f)
    print(f"const uint16_t count_hash[{5}] = {{", end="", file=f)
    for i in range(5):
        print(f"0x{get_hash(i):02x}", end="", file=f)
        if i < 4:
            print(f", ", end="", file=f)
        else:
            print("};", file=f)

    


