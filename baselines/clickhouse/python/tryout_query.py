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

query_select = "Select * from github_events_final where start_date <= 20181122 AND end_date >= 20190304 AND stars >= 147;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results

query_select = "Select * from github_events_final where start_date <= 20181122 AND start_date >= 20110211 AND end_date >= 20190304 AND end_date <= 20201206 AND stars >= 147;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results

query_select = "Select * from github_events_final where start_date <= 20181122 AND start_date >= 20110211 AND end_date >= 20190304 AND end_date <= 20201206;"
start = time.time()
results = client.execute(query_select)
end = time.time()
print("select *", end - start, results[0], len(results))
del results

