# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime

master = ("10.254.254.209", "9000")

def get_random_full_range_one():

    SHIPDATE_start = random.randrange(19920102, 19981201)
    SHIPDATE_end = random.randrange(SHIPDATE_start + 1, 19981201 + 1)
    DISCOUNT_start = random.randrange(0, 10)
    DISCOUNT_end = random.randrange(DISCOUNT_start + 1, 10 + 1)
    QUANTITY_end = random.randrange(1, 50 + 1)

    return '''SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN {} AND {} AND DISCOUNT BETWEEN {} AND {} AND QUANTITY <= {};'''.format(SHIPDATE_start, SHIPDATE_end, DISCOUNT_start, DISCOUNT_end, QUANTITY_end)

def get_random_full_range_two():

    ORDERDATE_start = random.randrange(19920102, 19981201)
    SHIPDATE_end = random.randrange(ORDERDATE_start, 19981201 + 1)
    
    return '''SELECT * FROM tpch_macro WHERE ORDERDATE >= {} AND SHIPDATE <= {};'''.format(ORDERDATE_start, SHIPDATE_end)

if __name__ == "__main__":

    template_index = 0
    return_start = 0
    return_end = 0

    if sys.argv[1] == "1": # get_random_full_range_one
        template_index = 1
        return_start = 57106873 * (1 - 0.01)
        return_end = 57106873 * (1 + 0.01)
    if sys.argv[1] == "2": # get_random_full_range_two
        template_index = 2
        return_start = 151648557 * (1 - 0.01)
        return_end = 151648557 * (1 + 0.01)

    total_i = int(sys.argv[2])
    client = Client(master[0], port=master[1])
    i = 0

    while i < total_i:
        random.seed(datetime.now())
        
        if template_index == 1:
            query = get_random_full_range_one()
        if template_index == 2:
            query = get_random_full_range_two()

        query_count = query.replace("*", "COUNT(*)")

        start = time.time()
        count_result = client.execute(query_count)[0][0]
        end = time.time()

        if count_result < return_start or count_result > return_end:
            print("doesnt work")
            continue

        print("{}, elapsed: {}s, found points: {}".format(query_count, end - start, count_result, flush=True))
        i += 1