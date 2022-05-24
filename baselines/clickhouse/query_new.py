# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
from multiprocessing import Process

master = ("10.254.254.221", "9000")
search_range_delta = 0.10
def get_random_full_range_one():

    SHIPDATE_start = random.randrange(19920102, 19981201)
    SHIPDATE_end = random.randrange(SHIPDATE_start + 1, 19981201 + 1)
    DISCOUNT_start = random.randrange(0, 10)
    DISCOUNT_end = random.randrange(DISCOUNT_start + 1, 10 + 1)
    QUANTITY_end = random.randrange(1, 50 + 1)

    return '''SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN {} AND {} AND DISCOUNT BETWEEN {} AND {} AND QUANTITY <= {};'''.format(SHIPDATE_start, SHIPDATE_end, DISCOUNT_start, DISCOUNT_end, QUANTITY_end)

def get_random_full_range_two():

    ORDERDATE_start = random.randrange(19920101, 19980802)
    ORDERDATE_end = random.randrange(ORDERDATE_start, 19980802 + 1)
    
    return '''SELECT * FROM tpch_macro WHERE ORDERDATE >= {} AND ORDERDATE <= {} AND COMMITDATE < RECEIPTDATE;'''.format(ORDERDATE_start, ORDERDATE_end)

def get_random_full_range_three():

    RECEIPTDATE_start = random.randrange(19920103, 19981231)
    RECEIPTDATE_end = random.randrange(RECEIPTDATE_start, 19981231 + 1)
    
    return '''SELECT * FROM tpch_macro WHERE COMMITDATE < RECEIPTDATE AND SHIPDATE < COMMITDATE AND RECEIPTDATE >= {} AND RECEIPTDATE <= {};'''.format(RECEIPTDATE_start, RECEIPTDATE_end)

def get_random_full_range_four():

    ORDERDATE_end = random.randrange(19920101, 19980802)
    SHIPDATE_start = random.randrange(19920102, 19981201)
    
    return '''SELECT * FROM tpch_macro WHERE ORDERDATE <= {} AND SHIPDATE >= {};'''.format(ORDERDATE_end, SHIPDATE_start)

def get_random_full_range_five():

    SHIPDATE_start = random.randrange(19920102, 19981201)
    SHIPDATE_end = random.randrange(SHIPDATE_start + 1, 19981201 + 1)
    
    return '''SELECT * FROM tpch_macro WHERE SHIPDATE >= {} AND SHIPDATE <= {};'''.format(SHIPDATE_start, SHIPDATE_end)


def func(total_i, template_index, return_start, return_end):

    i = 0
    client = Client(master[0], port=master[1])

    while i < total_i:
        random.seed(datetime.now())
        
        if template_index == 1:
            query = get_random_full_range_one()
        if template_index == 2:
            query = get_random_full_range_two()
        if template_index == 3:
            query = get_random_full_range_three()
        if template_index == 4:
            query = get_random_full_range_four()
        if template_index == 5:
            query = get_random_full_range_five()

        query_count = query.replace("*", "COUNT(*)")
        count_result = client.execute(query_count)[0][0]

        if count_result < return_start or count_result > return_end:
            continue

        start = time.time()
        # client.execute(query)
        end = time.time()

        print("{}, elapsed: {}s, found points: {}".format(query, end - start, count_result), flush=True)
        i += 1

processes = []

if __name__ == "__main__":

    total_i = int(sys.argv[2])

    if sys.argv[1] == "1": # get_random_full_range_one
        template_index = 1
        return_start = 57106873 * (1 - search_range_delta)
        return_end = 57106873 * (1 + search_range_delta)
    if sys.argv[1] == "2": # get_random_full_range_two
        template_index = 2
        return_start = 72529462 * (1 - search_range_delta)
        return_end = 72529462 * (1 + search_range_delta)
    if sys.argv[1] == "3":
        template_index = 3
        return_start = 54540146 * (1 - search_range_delta)
        return_end = 54540146 * (1 + search_range_delta)
    if sys.argv[1] == "4":
        template_index = 4
        return_start = 74807478 * (1 - search_range_delta)
        return_end = 74807478 * (1 + search_range_delta)
    if sys.argv[1] == "5":
        template_index = 5
        return_start = 37417320 * (1 - search_range_delta)
        return_end = 37417320 * (1 + search_range_delta)

    func(total_i, template_index, return_start, return_end)

    
