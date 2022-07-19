import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
import time
import sys
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from random import randrange
import random
import re

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']

processes = []
total_vect = []
num_data_nodes = 5
begin_measuring = int(5000000)  # 5M
total_points = int(5000100)  # 5.01M

filename = "/proj/trinity-PG0/Trinity/results/tpch_aerospike"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

no_insert = False
if len(sys.argv) == 3 and sys.argv[2] == "no_insert":
    no_insert = True
elif len(sys.argv) == 3:
    print(sys.argv)
    exit(0)

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']
dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

def search(line_idx, cursor_search_list): 

    line = lines[line_idx]
    query = line.split(";,")[0] + ";"
    query_select = query.replace("COUNT(*)", "*")

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

    results_cumulative_len = 0
    for cursor in cursor_search_list:
        cursor.execute(query_select)
        results = cursor.fetchall()
        results_cumulative_len += len(results)

    return results_cumulative_len


def search_insert_each_worker(worker_idx, total_workers):

    connection_list = []
    cursor_search_list = []
    cursor_insert_list = []
    for data_node_idx in range(num_data_nodes):
        client_ip = "10.10.1.{}".format(12 + data_node_idx)
        CONNECTION = "dbname=defaultdb host={} user=postgres password=adifficultpassword sslmode=disable".format(client_ip)
        CONN = psycopg2.connect(CONNECTION)
        # CONN.autocommit = True
        connection_list.append(CONN)
        cursor_search_list.append(CONN.cursor())
        cursor_insert_list.append(CONN.cursor())

    line_count = 0
    effective_line_count = 0
    start_time = time.time()

    for i in range(begin_measuring + worker_idx, total_points, total_workers):

        effective_line_count += 1
        line_count += 1

        if line_count % 20 == 19 and not no_insert:
            is_search = False
        else:
            is_search = True

        if not is_search:
            chunk = total_vect[i]
            hash_i = hash(i)
            cursor_insert_list[hash_i % 5].execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", chunk)
            connection_list[hash_i % 5].commit() # Needed for atomic transaction.
        else:
            num_found_points = search(i % 1000, cursor_search_list)
            effective_line_count += num_found_points - 1

        if line_count:
            if is_search:
                print(line_count, num_found_points, flush=True)
            else:
                print(line_count, flush=True)
            
    end_time = time.time()

    return effective_line_count / (end_time - start_time)

def search_insert_worker(worker, total_workers, return_dict):

    throughput = search_insert_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

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
            chunk = [int(entry) for entry in string_list]

            chunk[5] = datetime.strptime(str(chunk[5]), "%Y%m%d")
            chunk[6] = datetime.strptime(str(chunk[6]), "%Y%m%d")
            chunk[7] = datetime.strptime(str(chunk[7]), "%Y%m%d")
            chunk[9] = datetime.strptime(str(chunk[9]), "%Y%m%d")

            total_vect.append(chunk)
            loaded_lines += 1
            if loaded_lines == total_points:
                break

if not no_insert:
    load_all_points(int(sys.argv[1]))

for worker in range(threads_per_node):
    p = Process(target=search_insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)
    
for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

if not no_insert:
    with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/tpch_search_insert_throughput.txt', 'a') as f:
        print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
else:
    with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/tpch_search_throughput.txt', 'a') as f:
        print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)