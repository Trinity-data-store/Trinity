import numpy
import random

total_num_keys = int(30000000) # 100M

def get_frequency_list(input_list):

    frequency = {}
    for item in input_list:
        if item in frequency:
            frequency[item] += 1
        else:
            frequency[item] = 1
    return frequency

freq_count = numpy.random.zipf(a=2, size=total_num_keys)
keys = list(range(total_num_keys))

picked_primary_keys = random.choices(keys, weights=freq_count, k=total_num_keys)


print(picked_primary_keys[:20], len(picked_primary_keys))
# frequency_list = get_frequency_list(picked_primary_keys)

# sorted_freq = dict(sorted(frequency_list.items(), key=lambda item: item[1]))
f = open("/proj/trinity-PG0/Trinity/queries/zipf_keys_30m", "a")
f.write("\n".join([str(x) for x in picked_primary_keys]))
f.close()

