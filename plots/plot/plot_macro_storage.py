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


if __name__ == "__main__": 

    value_list_list = [
        ["TPC-H", 42.13/70, 28.85/70, 278.57/70, 203.97/70],
        ["GitHub\nEvents", 16.46/35, 9.95/35, 249.06/35, 209.54/35],
        ["NYC\nTaxi", 24.54/87, 12.3/87, 227.98/87, 233.45/87]
    ]
    systems = ["Trinity", "ClickHouse",  "Aerospike", "TimescaleDB"]

    plot_bar_group(value_list_list, systems, "storage_macro", "storage")