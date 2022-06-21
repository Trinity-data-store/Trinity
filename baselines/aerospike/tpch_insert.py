import aerospike
import sys
import csv

global rec
rec = {}
csvfile  = open('/mntData/tpch_split/x0', "r")
reader = csv.reader(csvfile)
rownum = 0

config = {
  'hosts': [ ('10.254.254.209', 3000)]
}

# Create a client and connect it to the cluster
try:
  client = aerospike.client(config).connect()
except:
  import sys
  print("failed to connect to the cluster with", config['hosts'])
  sys.exit(1)

# Records are addressable via a tuple of (namespace, set, key)
header = ["quantity", "extendedprice", "discount", "tax", "shipdate", "commitdate", "recepitdate", "totalprice", "orderdate"]

for row in reader:
    # Save First Row with headers

    colnum = 0
    primary_key = 0
    for col in row:
        if colnum == 0:
            primary_key = int(col)
        else:
            rec[header[colnum - 1]] = int(col)
        colnum += 1

    rownum += 1
    key = ('tpch', 'tpch_macro', str(primary_key))
    if rec:
        try:
            client.put(key, rec)
        except Exception as e:
            import sys
            print(key, rec)
            print("error: {0}".format(e), file=sys.stderr)
            exit(0)
    if rownum % 10000 == 0:
        print(rownum, rec)
    rec = {}

csvfile.close()
