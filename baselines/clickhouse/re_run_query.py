from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime

master = ("10.254.254.221", "9000")

filename = sys.argv[1]

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

client = Client(master[0], port=master[1])

finished_line = 0
for line in lines[12:20]:

    query = line.split(";,")[0] + ";"

    start = time.time()
    results = client.execute(query.replace("tpch_macro", "tpch_macro_split"))
    end = time.time()

    with open(filename + "_rerun", "a") as outfile:
        outfile.write("{}, elapsed: {}s, found points: {}\n".format(query, end - start, len(results)))

    del results

    finished_line += 1

