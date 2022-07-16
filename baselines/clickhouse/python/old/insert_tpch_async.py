from datetime import datetime
import time
import asyncio
from aioch import Client
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
import sys

master = ("10.10.1.2", "9000")
processes = []
num_workers = 20
total_workers = 0

async def exec_no_progress(worker_idx, return_dict):
    client = Client(master[0], port=master[1])
    file_path = "/mntData/tpch_split/x{}".format(worker_idx)
    cumulative_time = 0
    line_count = 0
    start_time = time.time()

    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            insert_list = [[int(entry) for entry in string_list]]

            rv = await client.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", insert_list)

            if line_count % 100 == 0:
                print(line_count)

            if line_count == 100000:
                break

    end_time = time.time()

    throughput = line_count / (end_time - start_time)
    return_dict[worker_idx] = {"throughput": throughput}

def insert_worker(worker, return_dict):

    loop = asyncio.get_event_loop()
    loop.run_until_complete(asyncio.wait([exec_no_progress(worker, return_dict)]))

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

