import aerospike
import sys
import csv
import re
from aerospike import predicates
from aerospike import exception as ex
from aerospike_helpers import expressions as exp
import pprint
import time

pp = pprint.PrettyPrinter(indent=2)

config = {
  'hosts': [ ('10.254.254.209', 3000),('10.254.254.253', 3000),('10.254.254.229', 3000),('10.254.254.225', 3000),('10.254.254.213', 3000)]
}

# Create a client and connect it to the cluster
try:
  client = aerospike.client(config).connect()
  print("client connected")
except:
  import sys
  print("failed to connect to the cluster with", config['hosts'])
  sys.exit(1)

query = client.query('tpch', 'tpch_macro')
scan = client.scan('tpch', 'tpch_macro')
query.select("QUANTITY", "EXTENDEDPRICE", "DISCOUNT", "TAX", "SHIPDATE", "COMMITDATE", "RECEIPTDATE", "TOTALPRICE", "ORDERDATE")
# query.select("QUANTITY")

filename = "/proj/trinity-PG0/Trinity/baselines/aerospike/query_tpch"
with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

# total_count = 0
# def print_result(record_tuple):
#     # key, metadata, record = record_tuple
#     total_count += 1
#     if total_count < 5:
#         print(record_tuple)

for line in lines[1:2]:

    regex_template = re.compile(r"WHERE SHIPDATE BETWEEN (?P<ship_date_start>[0-9.]+?) AND (?P<ship_date_end>[0-9.]+?) AND ORDERDATE BETWEEN (?P<order_date_start>[0-9.]+?) AND (?P<order_date_end>[0-9.]+?) AND COMMITDATE BETWEEN (?P<commit_date_start>[0-9.]+?) AND (?P<commit_date_end>[0-9.]+?) AND RECEIPTDATE BETWEEN (?P<receipt_date_start>[0-9.]+?) AND (?P<receipt_date_end>[0-9.]+?) AND DISCOUNT BETWEEN (?P<discount_start>[0-9.]+?) AND (?P<discount_end>[0-9.]+?) AND QUANTITY BETWEEN (?P<quantity_start>[0-9.]+?) AND (?P<quantity_end>[0-9.]+?);,")



    m = regex_template.search(line)

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

        start = time.time()
        records_query = query.results(policy)
        end = time.time()
        print("wrong", len(records_query), end - start)
        del records_query

        query.where(predicates.between('SHIPDATE', int(m.group("ship_date_start")), int(m.group("ship_date_end"))))

        start = time.time()
        records_query = query.results(policy)
        end = time.time()
        print(len(records_query), end - start)
        del records_query
        
        expr_full = exp.And(
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
            exp.GE(exp.IntBin("QUANTITY"), int(m.group("quantity_start"))),
            exp.LE(exp.IntBin("QUANTITY"), int(m.group("quantity_end"))),           
            ).compile()

        policy_full = {
            'expressions': expr_full,
            'total_timeout':1000000
        }

        start = time.time()
        records_scan = scan.results(policy_full)
        end = time.time()
        print(len(records_scan), end - start)
        del records_scan
        

    except ex.AerospikeError as e:
        print("Error: {0} [{1}]".format(e.msg, e.code))
        sys.exit(1)

    exit(0)
    '''
    if m:
        # print(int(m.group("ship_date_start")), int(m.group("ship_date_end")))
        query.where(predicates.between('SHIPDATE', int(m.group("ship_date_start")), int(m.group("ship_date_end"))))

        # print(int(m.group("order_date_start")), int(m.group("order_date_end")))
        query.where(predicates.between('ORDERDATE', int(m.group("order_date_start")), int(m.group("order_date_end"))))

        # print(int(m.group("commit_date_start")), int(m.group("commit_date_end")))
        query.where(predicates.between('COMMITDATE', int(m.group("commit_date_start")), int(m.group("commit_date_end"))))
        query.where(predicates.between('RECEIPTDATE', int(m.group("receipt_date_start")), int(m.group("receipt_date_end"))))
        query.where(predicates.between('DISCOUNT', int(m.group("discount_start")), int(m.group("discount_end"))))
        query.where(predicates.between('QUANTITY', int(m.group("quantity_start")), int(m.group("quantity_end"))))

    start = time.time()
    # query.foreach(print_result)
    records = query.results( {'total_timeout':1000000})
    end = time.time()

    print(records[:3])
    print("records count", len(records))
    print(end - start)
    del records
    exit(0)
    '''