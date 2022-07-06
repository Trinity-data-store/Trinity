import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
import time
import sys
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor

CONNECTION = "dbname=tpch_macro host=localhost user=postgres password=adifficultpassword sslmode=disable async=1"
COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']

CONN = psycopg2.connect(CONNECTION)
cursor = CONN.cursor()
processes = []

def insert_each_worker(worker_idx):

    CONN = psycopg2.connect(CONNECTION)
    cursor = CONN.cursor()
    file_path = "/mntData/tpch_split/x{}".format(worker_idx)
    cumulative_time = 0
    line_count = 0
    start_time = time.time()
    mgr = CopyManager(CONN, 'tpch_macro', COLS)

    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            chunk = [int(entry) for entry in string_list]

            chunk[5] = datetime.strptime(str(chunk[5]), "%Y%m%d")
            chunk[6] = datetime.strptime(str(chunk[6]), "%Y%m%d")
            chunk[7] = datetime.strptime(str(chunk[7]), "%Y%m%d")
            chunk[9] = datetime.strptime(str(chunk[9]), "%Y%m%d")

            start = time.time()
            mgr.copy([chunk])
            # cursor.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", chunk)
            
            cumulative_time += time.time() - start

            if line_count % 100 == 0:
                print(line_count)

            if line_count % 10000:
                CONN.commit()

            if line_count == 100000:
                break
    CONN.commit()
    end_time = time.time()

    return line_count / (end_time - start_time), cumulative_time / line_count * 1000

def insert_worker(worker, return_dict):

    throughput, latency = insert_each_worker(worker)
    return_dict[worker] = {"throughput": throughput, "latency": latency}

from_worker = 0
to_worker = 19

if len(sys.argv) == 2 and int(sys.argv[1]) == 0:
    from_worker = 0
    to_worker = 20
if len(sys.argv) == 2 and int(sys.argv[1]) == 1:
    from_worker = 20
    to_worker = 40
if len(sys.argv) == 2 and int(sys.argv[1]) == 2:
    from_worker = 40
    to_worker = 60

manager = multiprocessing.Manager()
return_dict = manager.dict()
print(from_worker, to_worker)

for worker in range(from_worker, to_worker):
    p = Process(target=insert_worker, args=(worker, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
print("average latency (ms)", sum([return_dict[worker]["latency"] for worker in return_dict]) / len(return_dict.keys()))
