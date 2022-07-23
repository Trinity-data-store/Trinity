from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
import re

master = ("10.10.1.2", "9000")

client = Client(master[0], port=master[1])

finished_line = 0

query_select = "Select * from github_events_testing where stars >= 52 and forks >= 37;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results

query_select = "Select * from github_events_final where stars >= 52 and forks >= 37;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results


query_select = "Select * from github_events_testing where stars >= 1 and forks >= 1 and issues >= 1;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results

query_select = "Select * from github_events_final where stars >= 1 and forks >= 1 and issues >= 1;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results
