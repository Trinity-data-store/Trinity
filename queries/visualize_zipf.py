

file1 = open('/proj/trinity-PG0/Trinity/queries/zipf_keys_30m', 'r')
lines = file1.readlines()

nums = [int(line) for line in lines[:10000]]
print(nums[:100])
exit(0)
def get_frequency_list(input_list):

    frequency = {}
    for item in input_list:
        if item in frequency:
            frequency[item] += 1
        else:
            frequency[item] = 1
    return frequency

frequency_list = get_frequency_list(nums)
sorted_freq = dict(sorted(frequency_list.items(), key=lambda item: item[1]))
print(sorted_freq)