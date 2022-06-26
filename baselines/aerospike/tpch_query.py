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
  'hosts': [ ('10.10.1.2', 3000),('10.10.1.3', 3000),('10.10.1.4', 3000),('10.10.1.5', 3000),('10.10.1.6', 3000)]
}

# Create a client and connect it to the cluster
try:
  client = aerospike.client(config).connect()
  print("client connected")
except:
  import sys
  print("failed to connect to the cluster with", config['hosts'])
  sys.exit(1)


# query.select("QUANTITY")

filename = "/proj/trinity-PG0/Trinity/results/tpch_clickhouse_new"
out_filename = "/proj/trinity-PG0/Trinity/results/tpch_aerospike_new"

ship_date_range = [19920102, 19981201]  # min, max
order_date_range = [19920101, 19980802]
commit_date_range = [19920131, 19981031]
receipt_date_range = [19920103, 19981231]
discount_range = [0, 10]
quantity_range = [1, 50]

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

for line in lines[39:]:

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
    print(line)
    print(picked_query, attribute_to_selectivity[picked_query])

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

            start = time.time()
            records_query = query.results(policy)
            end = time.time()
            with open(out_filename, "a") as outfile:
                outfile.write("{}, elapsed: {}s, found points: {}\n".format(line, end - start, len(records_query)))
            del records_query

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

            start = time.time()
            records_query = query.results(policy)
            end = time.time()
            with open(out_filename, "a") as outfile:
                outfile.write("{}, elapsed: {}s, found points: {}\n".format(line, end - start, len(records_query)))
            del records_query

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

            start = time.time()
            records_query = query.results(policy)
            end = time.time()
            with open(out_filename, "a") as outfile:
                outfile.write("{}, elapsed: {}s, found points: {}\n".format(line, end - start, len(records_query)))
            del records_query

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

            start = time.time()
            records_query = query.results(policy)
            end = time.time()
            with open(out_filename, "a") as outfile:
                outfile.write("{}, elapsed: {}s, found points: {}\n".format(line, end - start, len(records_query)))

            del records_query

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

            start = time.time()
            records_query = query.results(policy)
            end = time.time()
            with open(out_filename, "a") as outfile:
                outfile.write("{}, elapsed: {}s, found points: {}\n".format(line, end - start, len(records_query)))

            del records_query

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

            start = time.time()
            records_query = query.results(policy)
            end = time.time()
            with open(out_filename, "a") as outfile:
                outfile.write("{}, elapsed: {}s, found points: {}\n".format(line, end - start, len(records_query)))

            del records_query

        except ex.AerospikeError as e:
            print("Error: {0} [{1}]".format(e.msg, e.code))
            sys.exit(1)
    else:    
        print("picked_query: ", picked_query)
        exit(0)