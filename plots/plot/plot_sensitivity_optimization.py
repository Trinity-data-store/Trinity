
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


if __name__ == "__main__": 

    optimization_list = ["B", "+CN", "+GM", "+SM"]

    optimization_translation = {
        "B": "B",
        "+CN": "CN",
        "+GM": "GM",
        "+SM": "SM"
    }

    # for workload in ["query", "storage", "insert", "lookup"]:
    for workload in ["storage"]:
        name_list = []
        value_list_list = []

        for dataset in ["tpch", "github", "nyc"]:

            regex_search = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")
            regex_storage = re.compile(r"(?P<storage>[0-9.]+?),(?P<points>[0-9.]+?)\n")
            search_base = {}

            for optimization in optimization_list:
        
                if optimization != "+SM":
                    system_path = "../../results/optimization/{}_{}_{}".format(dataset, workload, optimization_translation[optimization])
                else:
                    system_path = "../../results/trinity/{}_{}".format(dataset, workload)
                value_map = {}
                idx = 0
                with open("{}".format(system_path)) as ifile:
                    for line in ifile:
                        if workload == "query":
                            m = regex_search.search(line)
                            if float(m.group("points")) > 10 and float(m.group("elaspsed")) > 5:
                                value_map[idx] = float(m.group("elaspsed")) * 1000 / int(m.group("points"))
                        elif workload == "insert" or workload == "lookup":
                            value_map[idx] = int(line) + 95 # network
                            
                        elif workload == "storage":
                            m = regex_storage.search(line)
                            value_map[idx] = int(m.group("storage")) / int(m.group("points"))
                        idx += 1

                value_list = list(value_map.values())
                value_list_list.append(value_list)
                name_list.append(dataset + "\n"+ optimization)

        if workload == "storage":
            
            storage_list_list = []
            value_list_list[7] = value_list_list[6]
            idx = 0
            for dataset in ["TPC-H", "GitHub\nEvents", "NYC\nTaxi"]:
                storage_list = [dataset]
                for i in range(idx, idx + len(optimization_list)):
                    
                    storage_list.append(value_list_list[i][0])
            
                storage_list_list.append(storage_list)
                idx += len(optimization_list)
            print(storage_list_list)
            plot_bar_group_optimization(storage_list_list, optimization_list, "sensitivity_optimization/storage_optimization_sensitivity", "storage", True, True)
            # exit(0)

        else:
            value_list_list[7] = value_list_list[6]
            plot_optimization_box(value_list_list, name_list, "sensitivity_optimization/{}_optimization_sensitivity".format(workload), workload, workload=="insert" or workload=="lookup" or workload=="storage" or workload == "query")


