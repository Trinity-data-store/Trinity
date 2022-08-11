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
import fcntl

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']

processes = []
total_vect = []
num_data_nodes = 5
total_points = int(30000) # 30M
warmup_points = int(total_points * 0.2)
file_path = "/mntData/tpch_split_10/x0"
insert_latency_vect = []
lookup_latency_vect = []

def flush_list_to_file(vect, file_name):
    with open(file_name, 'w') as f:
        for val in vect:
            f.write("{}\n".format(val))

def insert_each_worker():

    connection_list = []
    cursor_list = []

    for data_node_idx in range(num_data_nodes):
        client_ip = "10.10.1.{}".format(12 + data_node_idx)
        CONNECTION = "dbname=defaultdb host={} user=postgres password=adifficultpassword sslmode=disable".format(client_ip)
        CONN = psycopg2.connect(CONNECTION)
        connection_list.append(CONN)
        cursor_list.append(CONN.cursor())

    line_count = 0
    effective_line_count = 0
    warmup_ended = False
    cumulative = 0

    with open(file_path) as f:
        for line in f:

            line_count += 1

            '''
            Load points
            '''

            string_list = line.split(",")
            chunk = [int(entry) for entry in string_list]

            chunk[5] = datetime.strptime(str(chunk[5]), "%Y%m%d")
            chunk[6] = datetime.strptime(str(chunk[6]), "%Y%m%d")
            chunk[7] = datetime.strptime(str(chunk[7]), "%Y%m%d")
            chunk[9] = datetime.strptime(str(chunk[9]), "%Y%m%d")

            '''
            Insertion
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    start_time = time.time()
                    warmup_ended = True
                effective_line_count += 1

            hash_i = hash(chunk[0])
            start_time = time.time()
            cursor_list[hash_i % 5].execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", chunk)
            connection_list[hash_i % 5].commit() # Needed for atomic transaction.
            if warmup_ended:
                latency = time.time() - start_time
                cumulative += latency
                insert_latency_vect.append(latency * 1000000)

            '''
            Break
            '''

            if line_count > total_points:
                break

            if warmup_ended and line_count % 1000 == 0:
                print(line_count)
            

    return cumulative / effective_line_count


def lookup_each_worker():

    connection_list = []
    cursor_list = []

    for data_node_idx in range(num_data_nodes):
        client_ip = "10.10.1.{}".format(12 + data_node_idx)
        CONNECTION = "dbname=defaultdb host={} user=postgres password=adifficultpassword sslmode=disable".format(client_ip)
        CONN = psycopg2.connect(CONNECTION)
        connection_list.append(CONN)
        cursor_list.append(CONN.cursor())

    line_count = 0
    effective_line_count = 0
    warmup_ended = False
    cumulative = 0

    with open(file_path) as f:
        for line in f:

            line_count += 1

            '''
            Load points
            '''

            string_list = line.split(",")
            chunk = [int(entry) for entry in string_list]

            chunk[5] = datetime.strptime(str(chunk[5]), "%Y%m%d")
            chunk[6] = datetime.strptime(str(chunk[6]), "%Y%m%d")
            chunk[7] = datetime.strptime(str(chunk[7]), "%Y%m%d")
            chunk[9] = datetime.strptime(str(chunk[9]), "%Y%m%d")

            '''
            Lookup
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    start_time = time.time()
                    warmup_ended = True
                effective_line_count += 1

            hash_i = hash(chunk[0])
            start_time = time.time()
            cursor_list[hash_i % 5].execute("SELECT * FROM tpch_macro WHERE ID = {}".format(chunk[0]))
            connection_list[hash_i % 5].commit() # Needed for atomic transaction.
            if warmup_ended:
                latency =  time.time() - start_time
                cumulative += latency
                lookup_latency_vect.append(latency * 1000000)

            '''
            Break
            '''

            if line_count > total_points:
                break

            if warmup_ended and line_count % 1000 == 0:
                print(line_count)
            

    return cumulative / effective_line_count

insertion_latency = insert_each_worker()
print("insertion latency: ", insertion_latency * 1000000, "us")
lookup_latency = lookup_each_worker()
print("lookup latency: ", lookup_latency * 1000000, "us")

flush_list_to_file(insert_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/timescale_tpch_insert")
flush_list_to_file(lookup_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/timescale_tpch_lookup")