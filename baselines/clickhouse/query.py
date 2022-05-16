# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime

master = ("10.254.254.217", "9000")

def get_default_query_one():

    return '''SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN 19940101 AND 19950101 AND DISCOUNT BETWEEN 5 AND 7 AND QUANTITY <= 24;'''

def get_default_query_two():

    return '''SELECT * FROM tpch_macro WHERE ORDERDATE >= 19950315 AND SHIPDATE <= 19960315;'''

def get_generated_queries_one():

    SHIPDATE_start = 19940101
    SHIPDATE_end = 19950101
    DISCOUNT_start = 5
    DISCOUNT_end = 7
    QUANTITY_end = 24

    variable_to_change = random.choice(["SHIPDATE_start", "SHIPDATE_end", "DISCOUNT_start", "DISCOUNT_end", "QUANTITY_end"])

    if variable_to_change == "SHIPDATE_start":
        SHIPDATE_start = random.randrange(19920102, SHIPDATE_end + 1)
    if variable_to_change == "SHIPDATE_end":
        SHIPDATE_end = random.randrange(SHIPDATE_start + 1, 19981201 + 1)
    if variable_to_change == "DISCOUNT_start":
        DISCOUNT_start = random.randrange(0, DISCOUNT_end + 1)
    if variable_to_change == "DISCOUNT_end":
        DISCOUNT_end = DISCOUNT_end = random.randrange(DISCOUNT_start + 1, 10 + 1)
    if variable_to_change == "QUANTITY_end":
        QUANTITY_end = random.randrange(1, 50 + 1)
    
    return '''SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN {} AND {} AND DISCOUNT BETWEEN {} AND {} AND QUANTITY <= {};'''.format(SHIPDATE_start, SHIPDATE_end, DISCOUNT_start, DISCOUNT_end, QUANTITY_end)

def get_generated_queries_two():

    ORDERDATE_start = 19950315
    SHIPDATE_end = 19950315

    variable_to_change = random.choice(["ORDERDATE_start", "SHIPDATE_end"])
    variable_to_change = "SHIPDATE_end"

    if variable_to_change == "ORDERDATE_start":
        ORDERDATE_start = random.randrange(19920102, SHIPDATE_end + 1)
    if variable_to_change == "SHIPDATE_end":
        SHIPDATE_end = random.randrange(ORDERDATE_start, 19981201 + 1)
    
    return '''SELECT * FROM tpch_macro WHERE ORDERDATE >= {} AND SHIPDATE <= {};'''.format(ORDERDATE_start, SHIPDATE_end)


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

    if sys.argv[1] == "1": # get_generated_queries_one
        template_index = 1
    if sys.argv[1] == "2": # get_generated_queries_two
        template_index = 2
    if sys.argv[1] == "3": # get_random_full_range_one
        template_index = 3
    if sys.argv[1] == "4": # get_random_full_range_two
        template_index = 4

    total_i = int(sys.argv[2])

    client = Client(master[0], port=master[1])

    i = 0
    while i < total_i:
        random.seed(datetime.now())
        
        if template_index == 1:
            query = get_generated_queries_one()
        if template_index == 2:
            query = get_generated_queries_two()
        if template_index == 3:
            query = get_random_full_range_one()
        if template_index == 4:
            query = get_random_full_range_two()

        query_count = query.replace("*", "COUNT(*)")
        count_result = client.execute(query_count)[0][0]

        if count_result == 0 or count_result > 50000000:
            continue

        start = time.time()
        result = client.execute(query)
        end = time.time()

        print("{}, elapsed: {}s, found points: {}".format(query, end - start, len(result)), flush=True)

        i += 1