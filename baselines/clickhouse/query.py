# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time

subs = [
    ("10.254.254.229", "9000"),
    ("0.254.254.253", "9000"),
]

master = ("10.254.254.249", "9000")

if __name__ == "__main__":

    client = Client(master[0], port=master[1])

    start = time.time()

    result = client.execute('''SELECT * 
                        FROM tpch_distributed
                        WHERE SHIPDATE BETWEEN 19940101 AND 19950101  
                        AND DISCOUNT BETWEEN 5 AND 7
                        AND QUANTITY <= 24;
                                            ''')
    end = time.time()
    print("elapsed: ", end - start, "s")
    print("found points: ", len(result))