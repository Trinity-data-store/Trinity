import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime

CONNECTION = "dbname =tsdb user=tsdbadmin password=secret host=host.com port=5432 sslmode=require"
CONNECTION = "dbname=tpch_macro host=localhost user=postgres password=adifficultpassword sslmode=disable"

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']
FILENAME = "/mntData2/tpch/data_500/orders_lineitem_merged_indexed.csv"

CHUNKSIZE = 10 ** 6
CONN = psycopg2.connect(CONNECTION)
cursor = CONN.cursor()

def process(chunk):

    
    for row_id in range(len(chunk)):

        chunk[row_id][5] = datetime.strptime(str(chunk[row_id][5]), "%Y%m%d")
        chunk[row_id][6] = datetime.strptime(str(chunk[row_id][6]), "%Y%m%d")
        chunk[row_id][7] = datetime.strptime(str(chunk[row_id][7]), "%Y%m%d")
        chunk[row_id][9] = datetime.strptime(str(chunk[row_id][9]), "%Y%m%d")

    mgr = CopyManager(CONN, 'tpch_macro', COLS)
    mgr.copy(chunk)

    CONN.commit()

cursor.execute("TRUNCATE tpch_macro;")

for chunk in pd.read_csv(FILENAME, chunksize=CHUNKSIZE, header=None):

    # process(list(chunk.to_records(index=False)))

    process(chunk.values.tolist())
    cursor.execute("SELECT COUNT(*) FROM tpch_macro;")
    results = cursor.fetchall()
    print(results)

