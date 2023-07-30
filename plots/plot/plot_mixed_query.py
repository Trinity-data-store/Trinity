import os
import sys 
sys.path.append(os.path.expanduser("~/workspace/"))
sys.path.append(os.path.expanduser("../"))
from pyutils.common import * 
import re 
import itertools
from typing import List, Dict
import statistics
from matplotlib.ticker import ScalarFormatter
from pyutils.plot_functions import *

systems = ["Trinity", "ClickHouse",  "Aerospike", "TimescaleDB"]

insertion_list_list = [
    ["TPC-H", 1185, 20.97, 1644, 132.93],
    ["GitHub\nEvents", 1042.3, 20.72, 1203, 144.15],
    ["NYC\nTaxi", 1002, 19.8, 1284, 129.86]
]

lookup_list_list = [
    ["TPC-H", 1056.4, 28.383, 3930, 574.7],
    ["GitHub\nEvents", 851.685, 31.26, 3850, 544.73],
    ["NYC\nTaxi", 935.317, 35.727, 3700, 525.23]
]



lookup95_insert5_list_list = [
    ["TPC-H", 1248.65, 17.86, 2569.228, 242.806],
    ["GitHub\nEvents", 990.217, 15.14, 2506.673, 223],
    ["NYC\nTaxi", 1073.64, 19.902, 2305.418, 219.841]
]

lookup50_insert50_list_list = [
    ["TPC-H", 1341.235, 21.289, 2253, 242.046],
    ["GitHub\nEvents", 1099.621, 18.944, 2439, 220.283],
    ["NYC\nTaxi", 1091.457, 19.753, 2466, 215.264]
]

search_list_list = [
    ["TPC-H", 1776027, 623.91, 297.27, 2200],
    ["GitHub\nEvents", 1241374, 714.265, 650.055, 12491.73],
    ["NYC\nTaxi", 2224626, 921.444, 93.753, 17505]
]

search95_insert5_list_list = [
    ["TPC-H", 2032000, 1193.979, 293.58, 2590],
    ["GitHub\nEvents", 1323646.385, 701.304, 634.666, 12636.82],
    ["NYC\nTaxi", 2708903.583, 818.867, 162.687, 18934]
]

# systems = ["Trinity", "ClickHouse",  "Aerospike", "TimescaleDB"]
search50_insert50_list_list = [
    ["TPC-H", 2150402.679, 1736.782, 527.090, 2123.445],
    ["GitHub\nEvents", 1373094.204, 715.447, 726.026, 10380.770],
    ["NYC\nTaxi", 2924815.352, 2599.548, 165.891, 11881.682]
]

search10_insert90_list_list = [
    ["TPC-H", 2360775.271, 1762.813, 1274.196, 693.075],
    ["GitHub\nEvents", 1293524.785 ,898.321 , 980.220, 1773.476],
    ["NYC\nTaxi", 2322784.835, 2070.494, 365.654, 2196.265]
]


if __name__ == "__main__": 

    list_list_list = [insertion_list_list, lookup_list_list, search_list_list, search95_insert5_list_list, lookup95_insert5_list_list, lookup50_insert50_list_list, search50_insert50_list_list, search10_insert90_list_list]
    idx = 0
    
    for benchmark in ["insertion", "lookup", "search", "95%_search_5%_insert", "95%_lookup_5%_insert", "50%_lookup_50%_insert", "50%_search_50%_insert", "10%_search_90%_insert"]:

        list_list = list_list_list[idx]

        plot_bar_group(list_list, systems, "macrobench/" + benchmark, "throughput", benchmark == "search" or benchmark == "95%_search_5%_insert" or benchmark == "50%_search_50%_insert"or benchmark == "10%_search_90%_insert")
        idx += 1


