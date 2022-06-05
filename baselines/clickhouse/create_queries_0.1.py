# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor

master = ("10.254.254.221", "9000")

total_num_points = 3000028242
query_selecitivity = [0.0005, 0.0015]
num_workers = 20
total_num_queries = 1000

def generate_queries():

    ship_date_range = [19920102, 19981201]  # min, max
    order_date_range = [19920101, 19980802]
    commit_date_range = [19920131, 19981031]
    receipt_date_range = [19920103, 19981231]
    discount_range = [0, 10]
    quantity_range = [1, 50]

    ship_date_start, ship_date_end = ship_date_range[0], ship_date_range[1]
    order_date_start, order_date_end = order_date_range[0], order_date_range[1]
    commit_date_start, commit_date_end = commit_date_range[0], commit_date_range[1]
    receipt_date_start, receipt_date_end = receipt_date_range[0], receipt_date_range[1]
    discount_start, discount_end = discount_range[0], discount_range[1]
    quantity_start, quantity_end = quantity_range[0], quantity_range[1]

    for _ in range(6):

        current_attribute = random.choice(["ship_date", "order_date", "commit_date", "receipt_date", "discount", "quantity"])

        if current_attribute == "ship_date":
            ship_date_start = random.randrange(ship_date_range[0], ship_date_range[1])
            ship_date_end = random.randrange(ship_date_start + 1, ship_date_range[1] + 1)

        if current_attribute == "order_date":
            order_date_start = random.randrange(order_date_range[0], order_date_range[1])
            order_date_end = random.randrange(order_date_start + 1, order_date_range[1] + 1)

        if current_attribute == "commit_date":
            commit_date_start = random.randrange(commit_date_range[0], commit_date_range[1])
            commit_date_end = random.randrange(commit_date_start + 1, commit_date_range[1] + 1)

        if current_attribute == "receipt_date":
            receipt_date_start = random.randrange(receipt_date_range[0], receipt_date_range[1])
            receipt_date_end = random.randrange(receipt_date_start + 1, receipt_date_range[1] + 1)

        if current_attribute == "discount":
            discount_start = random.randrange(discount_range[0], discount_range[1])
            discount_end = random.randrange(discount_start + 1, discount_range[1] + 1)

        if current_attribute == "quantity":
            quantity_start = random.randrange(quantity_range[0], quantity_range[1])
            quantity_end = random.randrange(quantity_start + 1, quantity_range[1] + 1)

    # Pick 6 times. From all attributes.
    query = '''SELECT COUNT(*) FROM tpch WHERE SHIPDATE BETWEEN {} AND {} AND ORDERDATE BETWEEN {} AND {} AND COMMITDATE BETWEEN {} AND {} AND RECEIPTDATE BETWEEN {} AND {} AND DISCOUNT BETWEEN {} AND {} AND QUANTITY BETWEEN {} AND {};'''.format(ship_date_start, ship_date_end, order_date_start, order_date_end, commit_date_start, commit_date_end, receipt_date_start, receipt_date_end, discount_start, discount_end, quantity_start, quantity_end)

    return query

def func(num_queries):

    finished_queries = 0
    client = Client(master[0], port=master[1])

    while finished_queries < num_queries:

        query = generate_queries()
        result_count = client.execute(query)[0][0]

        if result_count >= query_selecitivity[0] * total_num_points and result_count <= query_selecitivity[1] * total_num_points:

            print("{}, found points: {}".format(query, result_count), flush=True)
            finished_queries += 1

if __name__ == "__main__":

    num_lines = sum(1 for line in open('query_tpch'))
    total_num_queries = total_num_queries - num_lines

    processes = []

    for _ in range(num_workers):
        p = Process(target=func, args=(total_num_queries/num_workers, ))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()