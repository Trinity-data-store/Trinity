from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime

master = ("10.254.254.221", "9000")

template_idx = sys.argv[1]
filename = "./query_tpch_T{}_range0.10".format(template_idx)

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

client = Client(master[0], port=master[1])

finished_line = 0
for line in lines:
    
    query = line.split(";,")[0] + ";"

    start = time.time()
    results = client.execute(query)
    end = time.time()

    print("{}, elapsed: {}s, found points: {}".format(query, end - start, len(results)), flush=True)
    finished_line += 1

