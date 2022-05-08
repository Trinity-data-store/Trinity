# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random

subs = [
    ("10.254.254.229", "9000"),
    ("0.254.254.253", "9000"),
]

master = ("10.254.254.249", "9000")

def import_csv():

    # Have not tested yet
    client = Client(master[0], port=master[1])
    file_path = "/mntData2/tpch-dbgen/data_200/orders_lineitem_merged_by_chunk_indexed.csv"
    table_name = "tpch_distributed"
    clickhouse_driver.util.insert_csv(client, 'test', '/tmp/test.csv')

def get_default_query_one():

    return '''SELECT * FROM tpch_distributed WHERE SHIPDATE BETWEEN 19940101 AND 19950101 AND DISCOUNT BETWEEN 5 AND 7 AND QUANTITY <= 24;'''

def get_default_query_two():

    return '''SELECT * FROM tpch_distributed WHERE ORDERDATE >= 19950315 AND SHIPDATE <= 19960315;'''

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
    
    return '''SELECT * FROM tpch_distributed WHERE SHIPDATE BETWEEN {} AND {} AND DISCOUNT BETWEEN {} AND {} AND QUANTITY <= {};'''.format(SHIPDATE_start, SHIPDATE_end, DISCOUNT_start, DISCOUNT_end, QUANTITY_end)

def get_generated_queries_two():

    ORDERDATE_start = 19950315
    SHIPDATE_end = 19950315

    variable_to_change = random.choice(["ORDERDATE_start", "SHIPDATE_end"])

    if variable_to_change == "ORDERDATE_start":
        SHIPDATE_start = random.randrange(19920102, SHIPDATE_end + 1)
    if variable_to_change == "SHIPDATE_end":
        SHIPDATE_end = random.randrange(ORDERDATE_start, 19981201 + 1)
    
    return '''SELECT * FROM tpch_distributed WHERE ORDERDATE >= {} AND SHIPDATE <= {};'''.format(ORDERDATE_start, SHIPDATE_end)


def get_random_full_range_one():

    SHIPDATE_start = random.randrange(19920102, 19981201 + 1)
    SHIPDATE_end = random.randrange(SHIPDATE_start + 1, 19981201 + 1)
    DISCOUNT_start = random.randrange(0, 10 + 1)
    DISCOUNT_end = random.randrange(DISCOUNT_start + 1, 10 + 1)
    QUANTITY_end = random.randrange(1, 50 + 1)

    return '''SELECT * FROM tpch_distributed WHERE SHIPDATE BETWEEN {} AND {} AND DISCOUNT BETWEEN {} AND {} AND QUANTITY <= {};'''.format(SHIPDATE_start, SHIPDATE_end, DISCOUNT_start, DISCOUNT_end, QUANTITY_end)

if __name__ == "__main__":

    for sub in subs:
        continue
        client = Client(sub[0], port=sub[1])
        client.execute('''CREATE TABLE IF NOT EXISTS tpch_distributed (
                            ID UInt32,
                            QUANTITY UInt8,
                            EXTENDEDPRICE UInt32,
                            DISCOUNT UInt8,
                            TAX UInt8,
                            SHIPDATE UInt32,
                            COMMITDATE UInt32,
                            RECEIPTDATE UInt32,
                            TOTALPRICE UInt32,
                            ORDERDATE UInt32
                            ) Engine = MergeTree ORDER BY (ID)''')

    client = Client(master[0], port=master[1])
    client.execute('''CREATE TABLE IF NOT EXISTS tpch_distributed (
                        ID UInt32,
                        QUANTITY UInt8,
                        EXTENDEDPRICE UInt32,
                        DISCOUNT UInt8,
                        TAX UInt8,
                        SHIPDATE UInt32,
                        COMMITDATE UInt32,
                        RECEIPTDATE UInt32,
                        TOTALPRICE UInt32,
                        ORDERDATE UInt32
                        ) ENGINE = Distributed(test_trinity, default, tpch_distributed, rand())''')

    i = 0
    while i < 200:
        # query = get_generated_queries_two()
        query = get_generated_queries_two()
        query_count = query.replace("*", "COUNT(*)")
        count_result = client.execute(query_count)[0][0]

        if count_result < 1000 or count_result > 100000000:
            continue

        print(query)

        start = time.time()
        result = client.execute(query)
        end = time.time()

        print("elapsed: ", end - start, "s")
        print("found points: ", len(result))

        i += 1