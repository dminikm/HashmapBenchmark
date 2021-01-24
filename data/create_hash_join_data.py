import random
import sys

random.seed('mec0037')

input_file_name = str(sys.argv[1]) or 'test.ft.txt.out'
num_smaller_entries = int(sys.argv[2]) or 100000
num_larger_entries = int(sys.argv[3]) or 10000000

text = ''
with open(input_file_name) as file:
    text = file.read().replace('\n', '').replace('\r\n', '')

def prepare_smaller_entries(text, num_entries, file_name):
    ids = []
    num_id = 1

    with open(file_name, mode = 'w') as file:
        for i in range(num_entries):
            new_id = num_id
            ids.append(new_id)

            # Simulate deletions
            if random.choices([0, 1], weights=(90, 10), k=1)[0] == 1:
                num_id = num_id + random.randint(1, 30)

            str_len = random.randint(20, 80)
            str_start = random.randint(0, len(text) - str_len)

            file.write(f"{new_id};{text[str_start:str_start + str_len]}\n")
            num_id += 1

    return ids

def prepare_larger_entries(text, ids, num_entries, file_name):
    num_id = 1

    with open(file_name, mode = 'w') as file:
        for i in range(num_entries):
            new_id = num_id

            # Simulate deletions
            if random.choices([0, 1], weights=(90, 10), k=1)[0] == 1:
                num_id = num_id + random.randint(1, 30)

            str_len = random.randint(20, 80)
            str_start = random.randint(0, len(text) - str_len)

            join_id = random.choice(ids)

            file.write(f"{new_id};{join_id};{text[str_start:str_start + str_len]}\n")
            num_id += 1

prepared_ids = prepare_smaller_entries(text, num_smaller_entries, 'hash_join_smaller.txt')
prepare_larger_entries(text, prepared_ids, num_larger_entries, 'hash_join_larger.txt')