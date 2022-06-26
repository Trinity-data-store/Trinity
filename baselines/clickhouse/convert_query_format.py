import re 
import sys

datapath = "/proj/trinity-PG0/Trinity/results/tpch_clickhouse_new"
outfile = open("/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_new_converted", "w")  # write mode

# [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
if __name__ == "__main__": 

    regex_template = re.compile(r"WHERE SHIPDATE BETWEEN (?P<ship_date_start>[0-9.]+?) AND (?P<ship_date_end>[0-9.]+?) AND ORDERDATE BETWEEN (?P<order_date_start>[0-9.]+?) AND (?P<order_date_end>[0-9.]+?) AND COMMITDATE BETWEEN (?P<commit_date_start>[0-9.]+?) AND (?P<commit_date_end>[0-9.]+?) AND RECEIPTDATE BETWEEN (?P<receipt_date_start>[0-9.]+?) AND (?P<receipt_date_end>[0-9.]+?) AND DISCOUNT BETWEEN (?P<discount_start>[0-9.]+?) AND (?P<discount_end>[0-9.]+?) AND QUANTITY BETWEEN (?P<quantity_start>[0-9.]+?) AND (?P<quantity_end>[0-9.]+?);,")

    with open("{}".format(datapath)) as ifile:

        for line in ifile:

            m = regex_template.search(line)
            if m:
                # // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
                out_line = "0,{},{},2,{},{},4,{},{},5,{},{},6,{},{},8,{},{}".format(m.group("quantity_start"), m.group("quantity_end"), m.group("discount_start"), m.group("discount_end"), m.group("ship_date_start"), m.group("ship_date_end"), m.group("commit_date_start"), m.group("commit_date_end"), m.group("receipt_date_start"), m.group("receipt_date_end"), m.group("order_date_start"), m.group("order_date_end"))
                
            outfile.write(out_line + "\n")

    outfile.close()