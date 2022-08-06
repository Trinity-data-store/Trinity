import aerospike
import sys
import csv
import time
import random
import sys
import random
from datetime import datetime
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from threading import Thread, Lock
import fcntl


config = {
  'hosts': [ ('10.10.1.12', 3000), ('10.10.1.13', 3000), ('10.10.1.14', 3000), ('10.10.1.15', 3000), ('10.10.1.16', 3000)]
}


write_policies = {'total_timeout': 4000, 'max_retries': 0}
read_policies = {'total_timeout': 3000, 'max_retries': 1}
policies = {'write': write_policies, 'read': read_policies}
config['policies'] = policies

header = ["QUANTITY", "EXTENDEDPRICE", "DISCOUNT", "TAX", "SHIPDATE", "COMMITDATE", "RECEIPTDATE", "TOTALPRICE", "ORDERDATE"]
processes = []
total_vect = []
num_data_nodes = 5
total_points = int(30000) # 30M
warmup_points = int(total_points * 0.2)
file_path = "/mntData/tpch_split_10/x0"

def insert_each_worker():

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    cumulative = 0
    warmup_ended = False

    with open(file_path) as f:
        for line in f:

            line_count += 1

            '''
            Load points
            '''

            string_list = line.split(",")
            colnum = 0
            rec = {}
            primary_key = 0
            for col in string_list:
                if colnum == 0:
                    primary_key = col
                else:
                    rec[header[colnum - 1]] = int(col)
                colnum += 1

            '''
            Insertion
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    warmup_ended = True
                effective_line_count += 1

            key = ('tpch', 'tpch_macro', primary_key)
            if rec:
                try:
                    start_time = time.time()
                    client.put(key, rec)
                    if warmup_ended:
                        cumulative += time.time() - start_time
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(-1)
            del key 
            del rec

            if line_count > total_points:
                break

    return cumulative / effective_line_count

def lookup_each_worker():

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    cumulative = 0
    warmup_ended = False

    with open(file_path) as f:
        for line in f:

            line_count += 1

            '''
            Load points
            '''

            string_list = line.split(",")
            colnum = 0
            rec = {}
            primary_key = 0
            for col in string_list:
                if colnum == 0:
                    primary_key = col
                else:
                    rec[header[colnum - 1]] = int(col)
                colnum += 1

            '''
            Insertion
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    warmup_ended = True
                effective_line_count += 1


            '''
            Lookup
            '''     

            key = ('tpch', 'tpch_macro', primary_key)
            try:
                start_time = time.time()
                (_, _, _) = client.get(key)
                if warmup_ended:
                    cumulative += time.time() - start_time
            except Exception as e:
                import sys
                print(key)
                print("error: {0}".format(e), file=sys.stderr)
                exit(0)

            del key 

            if line_count > total_points:
                break

    return cumulative / total_points

insertion_latency = insert_each_worker()
print("insertion latency: ", insertion_latency * 1000000, "us")
lookup_latency = lookup_each_worker()
print("lookup latency: ", lookup_latency * 1000000, "us")