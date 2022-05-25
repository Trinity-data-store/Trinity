from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime

master = ("10.254.254.221", "9000")


client = Client(master[0], port=master[1])

query2 = "SELECT * FROM tpch_macro_split WHERE ORDERDATE <= 19935761 AND SHIPDATE >= 19939422;"
query1 = "SELECT * FROM tpch_macro_split WHERE ORDERDATE <= 19972160 AND SHIPDATE >= 19979503;"
start = time.time()
results = client.execute(query1)
end = time.time()
print(results[135:137])
exit(0)
print("{}, elapsed: {}s, found points: {}".format(query1, end - start, len(results)), flush=True)
del results

start = time.time()
results = client.execute(query1)
end = time.time()

print("{}, elapsed: {}s, found points: {}".format(query1, end - start, len(results)), flush=True)
del results

start = time.time()
results = client.execute(query1)
end = time.time()

print("{}, elapsed: {}s, found points: {}".format(query1, end - start, len(results)), flush=True)
del results

