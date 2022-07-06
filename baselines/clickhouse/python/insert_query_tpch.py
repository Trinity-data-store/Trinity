from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from threading import Thread, Lock
import re

master = ("10.10.1.2", "9000")
num_workers = 20
total_workers = 0

processes = []
filename = "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_new"
dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

def clean_query(query_select):

    StringRegex = re.compile(r'BETWEEN \d\d\d\d\d\d\d\d AND \d\d\d\d\d\d\d\d')
    while StringRegex.search(query_select):
        old_string = StringRegex.search(query_select).group()
        new_string = old_string
        DateRegex = re.compile(r'\d\d\d\d\d\d\d\d')

        for i in range(2):
            old_dateString = DateRegex.search(new_string).group()
            new_dateString = old_dateString[0:4] + "-" + old_dateString[4:6] + "-" + old_dateString[6:]
            
            year = int(new_dateString.split("-")[0])
            month = int(new_dateString.split("-")[1])
            day = int(new_dateString.split("-")[2])
            if i == 0:
                if month <= 12 and month > 0 and day > dates[month]:
                    month += 1
                    day = 1
            if i == 1:
                if month <= 12 and month > 0 and day > dates[month]:
                    day = dates[month]
                    if year % 4 == 0 and month == 2:
                        day = 29
            
            if month < 10 and month > 0:
                month_string = "0{}".format(month)
            elif month == 0:
                month_string = "01"
            else:
                month_string = "{}".format(month)
            if day < 10 and day > 0:
                day_string = "0{}".format(day)
            elif day == 0:
                day_string = "01"
            else:
                day_string = "{}".format(day)
            new_dateString = old_dateString[0:4] + "-" + month_string + "-" + day_string

            if i == 0 and month > 12:
                new_dateString = str(int(new_dateString.split("-")[0]) + 1) + "-01-01"
            if i == 0 and month == 0:
                new_dateString = str(int(new_dateString.split("-")[0])) + "-01-01"
            if i == 1 and month > 12:
                new_dateString = str(int(new_dateString.split("-")[0])) + "-12-31"
            if i == 1 and month == 0:
                new_dateString = str(int(new_dateString.split("-")[0]) - 1) + "-12-31"

            new_string = new_string.replace(old_dateString, "\'{}\'".format(new_dateString))

        query_select = query_select.replace(old_string, new_string)
    return query_select.replace("\'", "").replace("-", "")

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

def insert_query_each_worker(worker_idx):

    client = Client(master[0], port=master[1])
    file_path = "/mntData/tpch_split/tpch_split_split/x{}".format(worker_idx)
    cumulative_time = 0
    line_count = 0
    insertion_count = 0
    query_count = 0
    insertion_latency_cumulative = 0
    query_latency_cumulative = 0

    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            insert_list = [[int(entry) for entry in string_list]]

            if line_count % 20 == 19:
                is_insert = True
            else:
                is_insert = False
            
            random_query = random.choice(lines)
            random_query = random_query.split(";,")[0] + ";"
            random_query = clean_query(random_query.replace("COUNT(*)", "*"))

            start = time.time()
            if is_insert:
                client.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", insert_list)
            else:
                client.execute(random_query)
            elapsed_time = time.time() - start
            cumulative_time += elapsed_time

            if is_insert:
                insertion_count += 1
                insertion_latency_cumulative += elapsed_time
            else: 
                query_count += 1
                query_latency_cumulative += elapsed_time

            if line_count % 5 == 0:
                print(line_count)

            if line_count == 200:
                break

    return insertion_count, insertion_latency_cumulative, query_count, query_latency_cumulative, cumulative_time


def insert_query_worker(worker, return_dict):

    insertion_count, insertion_latency_cumulative, query_count, query_latency_cumulative, cumulative_time = insert_query_each_worker(worker)
    per_worker_return_dict = {}
    per_worker_return_dict["insertion latency"] = (insertion_latency_cumulative / insertion_count) * 1000
    per_worker_return_dict["query latency"] = (query_latency_cumulative / query_count) * 1000
    per_worker_return_dict["total throughput"] = (query_count + insertion_count) / cumulative_time
    return_dict[worker] = per_worker_return_dict

from_worker = 0
to_worker = 4

manager = multiprocessing.Manager()
return_dict = manager.dict()
print(from_worker, to_worker)

client = Client(master[0], port=master[1])
# client.execute("TRUNCATE tpch_macro")
result = client.execute("SELECT COUNT(*) FROM tpch_macro")
print("initialized? count: ", result[0])

for worker in range(from_worker, to_worker):
    p = Process(target=insert_query_worker, args=(worker, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

throughput_list = []
insertion_latency_list = []
query_latency_list = []

for worker in range(from_worker, to_worker):
    throughput_list.append(return_dict[worker]["total throughput"])
    insertion_latency_list.append(return_dict[worker]["insertion latency"])
    query_latency_list.append(return_dict[worker]["query latency"])

print(throughput_list)
print(insertion_latency_list)
print(query_latency_list)
print("total throughput", sum(throughput_list))
print("average insertion latency (ms)", sum(insertion_latency_list) / len(insertion_latency_list))
print("average query latency (ms)", sum(query_latency_list) / len(query_latency_list))
