from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
import re

master = ("10.10.1.2", "9000")

filename = "/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new"
out_filename = "/proj/trinity-PG0/Trinity/results/nyc_clickhouse_4T"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

client = Client(master[0], port=master[1])

finished_line = 0

for line in lines[497:]:

    query = line.split(",")[0] + ";"
    query_select = query.replace("count(*)", "*")

    start = time.time()
    results = client.execute(query_select)
    end = time.time()
    print("select *", end - start, results[0], len(results))

    with open(out_filename, "a") as outfile:
        outfile.write("{}, elapsed: {}s, found points: {}\n".format(query_select, end - start, len(results)))
    del results
    finished_line += 1

