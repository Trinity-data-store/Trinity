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
total_points = int(5000000) # 5M
num_workers = 20
total_workers = 0

processes = []
total_vect = []
num_data_nodes = 5
no_insert = False

if len(sys.argv) == 3 and sys.argv[2] == "no_insert":
    no_insert = True
elif len(sys.argv) == 3:
    print(sys.argv)
    exit(0)

filename = "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_new"
master = ("10.10.1.2", "9000")
dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
client = Client(master[0], port=master[1])


with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

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

def run_query(line_idx):

    line = lines[line_idx]
    query = line.split(";,")[0] + ";"

    query_select = query.replace("COUNT(*)", "*")
    query_select = clean_query(query_select)

    results = client.execute(query_select)

    return len(results)

def search_insert_each_worker(worker_idx, total_workers):

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    line_count = 0
    insertion_count = 0
    lookup_count = 0
    primary_key_list = []
    start = time.time()
    has_started = False
    effective_count = 0
    for i in range(worker_idx, total_points, total_workers):

        line_count += 1

        if not no_insert:
            if not has_started and len(primary_key_list) == 100:
                has_started = True
                start = time.time()

            if has_started:
                effective_count += 1

            if len(primary_key_list) < 10000 or line_count % 20 == 19:
                is_insert = True
            else:
                is_insert = False

        else:
            is_insert = False
            if line_count == 5:
                break
            
        if is_insert:
            insert_list = [total_vect[i]]
            p_key = insert_list[0][0]
            client_list[hash(p_key) % 5].execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", insert_list)
            primary_key_list.append(p_key)
        else:
            num_returned_points = run_query(i % 1000)
            effective_count += num_returned_points - 1

        # if line_count % 500 == 0:
        #     print(line_count)
        if no_insert:
            print(line_count, i, num_returned_points)

    end_to_end_time = time.time() - start

    return effective_count / end_to_end_time


def search_insert_worker(worker, total_workers, return_dict):

    throughput = search_insert_each_worker(worker, total_workers)
    per_worker_return_dict = {}
    per_worker_return_dict["throughput"] = throughput
    return_dict[worker] = per_worker_return_dict

threads_per_node = 60
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

def load_all_points(client_idx):
    file_path = "/mntData/tpch_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            string_list = line.split(",")
            total_vect.append([int(entry) for entry in string_list])
            loaded_lines += 1
            if loaded_lines == total_points:
                break

if not no_insert:
    load_all_points(int(sys.argv[1]))

for worker in range(threads_per_node):
    p = Process(target=search_insert_worker, args=(worker + int(sys.argv[1]) * threads_per_node, threads_per_node * total_num_nodes, return_dict))
    p.start()
    processes.append(p)

for p in processes:
    p.join()


print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]), flush=True)

if not no_insert:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/tpch_search_insert_throughput.txt', 'a') as f:
        print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f, flush=True)
if no_insert:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/tpch_search_throughput.txt', 'a') as f:
        print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f, flush=True) 
