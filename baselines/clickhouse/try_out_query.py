from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime

master = ("10.254.254.253", "9000")
if sys.argv[1] == 0:
    master = ("10.254.254.221", "9000")
if sys.argv[1] == 1:
    master = ("10.254.254.253", "9000")
if sys.argv[1] == 2:
    master = ("10.254.254.229", "9000")

client = Client(master[0], port=master[1])

query1 = "SELECT * FROM tpch WHERE SHIPDATE BETWEEN 19920102 AND 19981201 AND ORDERDATE BETWEEN 19979510 AND 19980414 AND COMMITDATE BETWEEN 19920131 AND 19981031 AND RECEIPTDATE BETWEEN 19920103 AND 19981231 AND DISCOUNT BETWEEN 1 AND 3 AND QUANTITY BETWEEN 29 AND 31;"
start = time.time()
results = client.execute(query1)
end = time.time()
print(results[135:137])
print("{}, elapsed: {}s, found points: {}".format(query1, end - start, len(results)), flush=True) # 76058113
del results
exit(0)

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

