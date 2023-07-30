
import os
import sys
from unicodedata import name 
sys.path.append(os.path.expanduser("~/workspace/"))
sys.path.append(os.path.expanduser("../"))
from pyutils.common import * 
import re 
import itertools
from typing import List, Dict
import statistics
from matplotlib.ticker import ScalarFormatter
from pyutils.plot_functions import *

colors = {
    "insert": "green",
    "lookup": "purple",
    "storage": "blue",
    "search": "red"
}

hatches = {
    "insert": "+++",
    "lookup": "xxxx",
    "storage": "////",
    "search": "...."
}


if __name__ == "__main__": 

    dimension_list = ["4", "5", "6", "7", "8", "9"]
    for workload in ["insert", "lookup", "search", "storage"]:

        regex_search = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")
        regex_storage = re.compile(r"mdtrie storage: (?P<storage>[0-9.]+?)\n")

        value_list_list = []
        for dimension in dimension_list:
    
            system_path = "../../results/microbench/sensitivity_dimension/{}_{}".format(workload, dimension)

            if workload == "search" and int(dimension) <= 8:
                system_path += "_new"

            value_map = {}
            idx = 0
            with open("{}".format(system_path)) as ifile:
                for line in ifile:

                    if workload == "search":
                        m = regex_search.search(line)
                        value_map[idx] = float(m.group("elaspsed")) * 1000 / float(m.group("points"))
                    elif workload == "insert" or workload == "lookup":
                        value_map[idx] = int(line) + 95 # network
                        
                    elif workload == "storage":
                        m = regex_storage.search(line)
                        value_map[idx] = int(m.group("storage")) / 250000000

                    idx += 1
                    
            value_list_list.append(list(value_map.values()))

        if workload == "storage":
            plot_bar(value_list_list, dimension_list, "sensitivity_dimension/{}_sensitivity_dimension".format(workload), workload, "Dimensions")
        else:
            plot_box(value_list_list, dimension_list, "sensitivity_dimension/{}_sensitivity_dimension".format(workload), workload, False, "Dimensions")
            


