from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from threading import Thread, Lock

master = ("10.254.254.221", "9000")
num_workers = 20

processes = []
mutex = Lock()
throughput = 0

def insert_each_worker(worker_idx):

    client = Client(master[0], port=master[1])
    file_path = "/mntData/tpch_split/x{}".format(worker_idx)
    start = time.time()
    line_count = 0
    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            client.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", [[int(entry) for entry in string_list]])
    end = time.time()
    mutex.acquire()
    throughput += (line_count) / (end - start)
    mutex.release()


def insert_one_worker():

    client = Client(master[0], port=master[1])
    file_path = "/mntData2/tpch/data_500/orders_lineitem_merged_indexed.csv"
    start = time.time()
    line_count = 0
    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            client.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", [[int(entry) for entry in string_list]])
    end = time.time()
    mutex.acquire()
    throughput += (line_count) / (end - start)
    mutex.release()


if int(sys.argv[1]) == 0:
    from_worker = 0
    to_worker = 20
if int(sys.argv[1]) == 1:
    from_worker = 21
    to_worker = 40
if int(sys.argv[1]) == 2:
    from_worker = 41
    to_worker = 60
    
# if int(sys.argv[1]) == 0:
#     insert_one_worker()
# else:
for worker in range(from_worker, to_worker):
    p = Process(target=insert_each_worker, args=(worker, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", throughput)
