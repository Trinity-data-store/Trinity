import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
from multiprocessing import Process

CONNECTION = "dbname =tsdb user=tsdbadmin password=secret host=host.com port=5432 sslmode=require"
CONNECTION = "dbname=tpch_macro host=localhost user=postgres password=adifficultpassword sslmode=disable"

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']
FILENAME = "/mntData2/tpch/data_500/orders_lineitem_merged_indexed.csv"

CHUNKSIZE = 10 ** 5
CONN = psycopg2.connect(CONNECTION)
cursor = CONN.cursor()
num_workers = 1

def read_file(i):
    conn = psycopg2.connect(CONNECTION)
    mgr = CopyManager(conn, 'tpch_macro', COLS)

    idx = 0
    for chunk in pd.read_csv("/mntData/tpch_split/x{}".format(i), chunksize=CHUNKSIZE, header=None):
        chunk = chunk.values.tolist()

        for row_id in range(len(chunk)):

            chunk[row_id][5] = datetime.strptime(str(chunk[row_id][5]), "%Y%m%d")
            chunk[row_id][6] = datetime.strptime(str(chunk[row_id][6]), "%Y%m%d")
            chunk[row_id][7] = datetime.strptime(str(chunk[row_id][7]), "%Y%m%d")
            chunk[row_id][9] = datetime.strptime(str(chunk[row_id][9]), "%Y%m%d")

        print("finished file: ", i, ", part: ", idx)
        mgr.copy(chunk)
        conn.commit()
        print("finished file: ", i, ", part: ", idx)
        idx += 1

processes = []
cursor.execute("TRUNCATE tpch_macro;")

for factor in [0, 1, 2]:

    for i in range(num_workers):
        p = Process(target=read_file, args=(i + factor * 20, ))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()
