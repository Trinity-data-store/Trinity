import pandas
import numpy as np

source_dir = "/mntData2/tpch/data_300/"

# [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
tpch_reader = pandas.read_table(source_dir + 'tpch_processed.csv', index_col=False, header=None, delimiter=",", chunksize=10000000)

finished_rounds = 0
cumulative = 0

def read_table(df):
    df.to_csv(source_dir + "tpch_processed_1B.csv", sep=',', index=False, header=False, mode="a")    
    return len(df)

for r in tpch_reader:
    cumulative += read_table(r)
    finished_rounds += 1
    print("finished_rounds: {}, cumulative: {}".format(finished_rounds, cumulative))
    if cumulative == 1000000000:
        break