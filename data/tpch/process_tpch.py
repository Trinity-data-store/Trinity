import pandas
import numpy as np

source_dir = "/mntData2/tpch-dbgen/data_300/"

df_orders = pandas.read_table(source_dir + 'orders.tbl', index_col=False, header=None, usecols=[0,3,4], names=["ORDERKEY","TOTALPRICE","ORDERDATE"], delimiter="|")
print(df_orders.head())

# [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
lineitem_reader = pandas.read_table(source_dir + 'lineitem.tbl', index_col=False, header=None, usecols=[0,4,5,6,7,10,11,12],names=["ORDERKEY","QUANTITY","EXTENDEDPRICE","DISCOUNT","TAX","SHIPDATE","COMMITDATE","RECEIPTDATE"], delimiter="|", chunksize=20000000)
print(lineitem_reader.head())

finished_rounds = 0
cumulative = 0

def merge_table_by_chunk(df_lineitem, cumulative):

    df_merged = pandas.merge(df_lineitem, df_orders, on="ORDERKEY", how='inner')
    df_merged = df_merged.drop("ORDERKEY", 1)

    df_merged.insert(0, 'Id', np.arange(cumulative, cumulative + len(df_merged)))

    df_merged['SHIPDATE'] = df_merged['SHIPDATE'].apply(lambda x: int(str(x).replace("-", "")))
    df_merged['COMMITDATE'] = df_merged['COMMITDATE'].apply(lambda x: int(str(x).replace("-", "")))
    df_merged['RECEIPTDATE'] = df_merged['RECEIPTDATE'].apply(lambda x: int(str(x).replace("-", "")))
    df_merged['ORDERDATE'] = df_merged['ORDERDATE'].apply(lambda x: int(str(x).replace("-", "")))

    df_merged['EXTENDEDPRICE'] = df_merged['EXTENDEDPRICE'].apply(lambda x: int(float(x) * 100))
    df_merged['DISCOUNT'] = df_merged['DISCOUNT'].apply(lambda x: int(float(x) * 100))
    df_merged['TAX'] = df_merged['TAX'].apply(lambda x: int(float(x) * 100))
    df_merged['TOTALPRICE'] = df_merged['TOTALPRICE'].apply(lambda x: int(float(x) * 100))

    df_merged.to_csv(source_dir + "tpch_processed.csv", sep=',', index=False, header=False, mode="a")
    return len(df_merged)

for r in lineitem_reader:
    
    cumulative += merge_table_by_chunk(r, cumulative)
    finished_rounds += 1
    print(finished_rounds)