from license import LICENSE

hash_table = [0] * 3585

def get_hash(cards):
    hash = 0
    for c in cards:
        hash += (1 << (c * 3))
    return hash

def find_value(cards):
    counter = [0] * 4
    for c in cards:
        counter[c] += 1

    max_value = max(counter)
    max_index = max(enumerate(counter),key=lambda x: x[1])[0]

    if max_value >= 5:
        value = max_index + 1
    elif max_value >= 2:
        value = max_value - 5
    else:
        value = 0

    return value

def recursive(counter, depth, cards):
    if counter == depth:
        value = find_value(cards)
        hash = get_hash(cards)
        hash_table[hash] = value      
        return
    for i in range(4):
        recursive(counter + 1, depth, cards + [i])


recursive(0, 7, [])
recursive(0, 6, [])
recursive(0, 5, [])
recursive(0, 4, [])
recursive(0, 3, [])

with open("./suits.cpp", "w+") as f:
    print(LICENSE, file=f)
    print("#include \"tables.h\"\n", file=f)
    print(f"const int8_t suits[{len(hash_table)}] = ", end="", file=f)
    print("{\n\t", end="", file=f)
    for i, h in enumerate(hash_table):
        print(f"{h:2d}", end="", file=f)
        if i+1 != len(hash_table):  
            print(f", ", end="", file=f)
        if (i+1) % 24 == 0:
            print("\n\t", end="", file=f)
    print("\n};", file=f)
    


