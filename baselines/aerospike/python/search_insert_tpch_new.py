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
import re
from aerospike import exception as ex
from aerospike_helpers import expressions as exp
from aerospike import predicates

config = {
  'hosts': [ ('10.10.1.12', 3000), ('10.10.1.13', 3000), ('10.10.1.14', 3000), ('10.10.1.15', 3000), ('10.10.1.16', 3000)]
}

header = ["QUANTITY", "EXTENDEDPRICE", "DISCOUNT", "TAX", "SHIPDATE", "COMMITDATE", "RECEIPTDATE", "TOTALPRICE", "ORDERDATE"]
processes = []
total_vect = []
num_data_nodes = 5
skip_points = int(5000000) # 5M # Need to run aerospike_insert first!!
total_points = int(5000100) # 10M
no_insert = False

if len(sys.argv) == 3 and sys.argv[2] == "no_insert":
    no_insert = True
elif len(sys.argv) == 3:
    print(sys.argv)
    exit(0)

def perform_search(line_idx, client):

    filename = "/proj/trinity-PG0/Trinity/results/tpch_clickhouse"

    ship_date_range = [19920102, 19981201]  # min, max
    order_date_range = [19920101, 19980802]
    commit_date_range = [19920131, 19981031]
    receipt_date_range = [19920103, 19981231]
    discount_range = [0, 10]
    quantity_range = [1, 50]

    with open(filename) as file:
        lines = file.readlines()
        lines = [line.rstrip() for line in lines]

    line = lines[line_idx]

    query = client.query('tpch', 'tpch_macro')
    query.select("QUANTITY", "EXTENDEDPRICE", "DISCOUNT", "TAX", "SHIPDATE", "COMMITDATE", "RECEIPTDATE", "TOTALPRICE", "ORDERDATE")

    regex_template = re.compile(r"WHERE SHIPDATE BETWEEN (?P<ship_date_start>[0-9.]+?) AND (?P<ship_date_end>[0-9.]+?) AND ORDERDATE BETWEEN (?P<order_date_start>[0-9.]+?) AND (?P<order_date_end>[0-9.]+?) AND COMMITDATE BETWEEN (?P<commit_date_start>[0-9.]+?) AND (?P<commit_date_end>[0-9.]+?) AND RECEIPTDATE BETWEEN (?P<receipt_date_start>[0-9.]+?) AND (?P<receipt_date_end>[0-9.]+?) AND DISCOUNT BETWEEN (?P<discount_start>[0-9.]+?) AND (?P<discount_end>[0-9.]+?) AND QUANTITY BETWEEN (?P<quantity_start>[0-9.]+?) AND (?P<quantity_end>[0-9.]+?);")

    line = line.split(";,")[0] + ";"
    line = line.replace("COUNT(*)", "*")
    m = regex_template.search(line)

    attribute_to_selectivity = {}
    if int(m.group("ship_date_start")) != ship_date_range[0] and int(m.group("ship_date_end")) != ship_date_range[1]:
        attribute_to_selectivity["ship_date"] = (int(m.group("ship_date_end")) - int(m.group("ship_date_start"))) / (ship_date_range[1] - ship_date_range[0])
    if int(m.group("order_date_start")) != order_date_range[0] and int(m.group("order_date_end")) != order_date_range[1]:
        attribute_to_selectivity["order_date"] = (int(m.group("order_date_end")) - int(m.group("order_date_start"))) / (order_date_range[1] - order_date_range[0])
    if int(m.group("commit_date_start")) != commit_date_range[0] and int(m.group("commit_date_end")) != commit_date_range[1]:
        attribute_to_selectivity["commit_date"] = (int(m.group("commit_date_end")) - int(m.group("commit_date_start"))) / (commit_date_range[1] - commit_date_range[0])
    if int(m.group("receipt_date_start")) != receipt_date_range[0] and int(m.group("receipt_date_end")) != receipt_date_range[1]:
        attribute_to_selectivity["receipt_date"] = (int(m.group("receipt_date_end")) - int(m.group("receipt_date_start"))) / (receipt_date_range[1] - receipt_date_range[0])
    if int(m.group("quantity_start")) != quantity_range[0] and int(m.group("quantity_end")) != quantity_range[1]:
        attribute_to_selectivity["quantity"] = (int(m.group("quantity_end")) - int(m.group("quantity_start"))) / (quantity_range[1] - quantity_range[0])
    if int(m.group("discount_start")) != discount_range[0] and int(m.group("discount_end")) != discount_range[1]:
        attribute_to_selectivity["discount"] = (int(m.group("discount_end")) - int(m.group("discount_start"))) / (discount_range[1] - discount_range[0])

    picked_query = min(attribute_to_selectivity, key=attribute_to_selectivity.get)

    if picked_query == "ship_date":
        
        try:
            expr = exp.And(
                # exp.GE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_start"))),
                # exp.LE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_end"))),
                exp.GE(exp.IntBin("ORDERDATE"), int(m.group("order_date_start"))),
                exp.LE(exp.IntBin("ORDERDATE"), int(m.group("order_date_end"))),
                exp.GE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_start"))),
                exp.LE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_end"))),
                exp.GE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_start"))),
                exp.LE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_end"))),  
                exp.GE(exp.IntBin("DISCOUNT"), int(m.group("discount_start"))),
                exp.LE(exp.IntBin("DISCOUNT"), int(m.group("discount_end"))), 
                exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
                exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
                ).compile()

            policy = {
                'expressions': expr,
                'total_timeout':1000000
            }

            query.where(predicates.between('SHIPDATE', int(m.group("ship_date_start")), int(m.group("ship_date_end"))))

            records_query = query.results(policy)
            return len(records_query)
            # del records_query

        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)

    elif picked_query == "order_date":

        try:
            expr = exp.And(
                exp.GE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_start"))),
                exp.LE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_end"))),
                # exp.GE(exp.IntBin("ORDERDATE"), int(m.group("order_date_start"))),
                # exp.LE(exp.IntBin("ORDERDATE"), int(m.group("order_date_end"))),
                exp.GE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_start"))),
                exp.LE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_end"))),
                exp.GE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_start"))),
                exp.LE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_end"))),  
                exp.GE(exp.IntBin("DISCOUNT"), int(m.group("discount_start"))),
                exp.LE(exp.IntBin("DISCOUNT"), int(m.group("discount_end"))), 
                exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
                exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
                ).compile()

            policy = {
                'expressions': expr,
                'total_timeout':1000000
            }

            query.where(predicates.between('ORDERDATE', int(m.group("order_date_start")), int(m.group("order_date_end"))))

            records_query = query.results(policy)
            # del records_query
            return len(records_query)


        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)

    elif picked_query == "commit_date":

        try:
            expr = exp.And(
                exp.GE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_start"))),
                exp.LE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_end"))),
                exp.GE(exp.IntBin("ORDERDATE"), int(m.group("order_date_start"))),
                exp.LE(exp.IntBin("ORDERDATE"), int(m.group("order_date_end"))),
                # exp.GE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_start"))),
                # exp.LE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_end"))),
                exp.GE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_start"))),
                exp.LE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_end"))),  
                exp.GE(exp.IntBin("DISCOUNT"), int(m.group("discount_start"))),
                exp.LE(exp.IntBin("DISCOUNT"), int(m.group("discount_end"))), 
                exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
                exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
                ).compile()

            policy = {
                'expressions': expr,
                'total_timeout':1000000
            }

            query.where(predicates.between('COMMITDATE', int(m.group("commit_date_start")), int(m.group("commit_date_end"))))

            records_query = query.results(policy)
            return len(records_query)

        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)

    elif picked_query == "receipt_date":

        try:
            expr = exp.And(
                exp.GE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_start"))),
                exp.LE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_end"))),
                exp.GE(exp.IntBin("ORDERDATE"), int(m.group("order_date_start"))),
                exp.LE(exp.IntBin("ORDERDATE"), int(m.group("order_date_end"))),
                exp.GE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_start"))),
                exp.LE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_end"))),
                # exp.GE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_start"))),
                # exp.LE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_end"))),  
                exp.GE(exp.IntBin("DISCOUNT"), int(m.group("discount_start"))),
                exp.LE(exp.IntBin("DISCOUNT"), int(m.group("discount_end"))), 
                exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
                exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
                ).compile()

            policy = {
                'expressions': expr,
                'total_timeout':1000000
            }

            query.where(predicates.between('RECEIPTDATE', int(m.group("receipt_date_start")), int(m.group("receipt_date_end"))))

            records_query = query.results(policy)

            # del records_query
            return len(records_query)

        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)
    elif picked_query == "discount":

        try:
            expr = exp.And(
                exp.GE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_start"))),
                exp.LE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_end"))),
                exp.GE(exp.IntBin("ORDERDATE"), int(m.group("order_date_start"))),
                exp.LE(exp.IntBin("ORDERDATE"), int(m.group("order_date_end"))),
                exp.GE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_start"))),
                exp.LE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_end"))),
                exp.GE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_start"))),
                exp.LE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_end"))),  
                # exp.GE(exp.IntBin("DISCOUNT"), int(m.group("discount_start"))),
                # exp.LE(exp.IntBin("DISCOUNT"), int(m.group("discount_end"))), 
                exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
                exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
                ).compile()

            policy = {
                'expressions': expr,
                'total_timeout':1000000
            }

            query.where(predicates.between('DISCOUNT', int(m.group("discount_start")), int(m.group("discount_end"))))

            records_query = query.results(policy)

            # del records_query
            return len(records_query)

        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)

    elif picked_query == "quantity":

        try:
            expr = exp.And(
                exp.GE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_start"))),
                exp.LE(exp.IntBin("SHIPDATE"), int(m.group("ship_date_end"))),
                exp.GE(exp.IntBin("ORDERDATE"), int(m.group("order_date_start"))),
                exp.LE(exp.IntBin("ORDERDATE"), int(m.group("order_date_end"))),
                exp.GE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_start"))),
                exp.LE(exp.IntBin("COMMITDATE"), int(m.group("commit_date_end"))),
                exp.GE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_start"))),
                exp.LE(exp.IntBin("RECEIPTDATE"), int(m.group("receipt_date_end"))),  
                exp.GE(exp.IntBin("DISCOUNT"), int(m.group("discount_start"))),
                exp.LE(exp.IntBin("DISCOUNT"), int(m.group("discount_end"))), 
                # exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
                # exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
                ).compile()

            policy = {
                'expressions': expr,
                'total_timeout':1000000
            }

            query.where(predicates.between('QUANTITY', int(m.group("quantity_start")), int(m.group("quantity_end"))))

            records_query = query.results(policy)

            # del records_query
            return len(records_query)

        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)
    else:    
        print("picked_query: ", picked_query)
        exit(0)


def search_insert_each_worker(total_num_workers, worker_idx):

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    start_time = time.time()
    end_time = 0
    insertion_count = 0

    for i in range(skip_points + worker_idx, total_points, total_num_workers):

        line_count += 1
        effective_line_count += 1

        if line_count % 20 == 19 and not no_insert:
            is_search = 0
        else:
            is_search = 1
            
        if is_search:
            return_num_points = perform_search(i % 1000, client)
            effective_line_count += return_num_points - 1
            if not return_num_points:
                print("return_num_points not found")
                exit(0)
        else:
            primary_key, rec = total_vect[i]
            key = ('tpch', 'tpch_macro', primary_key)
            if rec:
                try:
                    client.put(key, rec)
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
            del key 
            del rec

        print(line_count, effective_line_count)

    end_time = time.time()

    return effective_line_count / (end_time - start_time)


def search_insert_worker(worker, total_workers, return_dict):

    throughput = search_insert_each_worker(total_workers, worker)
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
            colnum = 0
            rec = {}
            primary_key = 0
            for col in string_list:
                if colnum == 0:
                    primary_key = col
                else:
                    rec[header[colnum - 1]] = int(col)
                colnum += 1

            total_vect.append((primary_key, rec))
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
with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/tpch_search_insert_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)