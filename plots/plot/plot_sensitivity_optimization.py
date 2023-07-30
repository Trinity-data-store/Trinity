
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
    # optimization_list = ["B", "+CN", "+GM"]

    optimization_translation = {
        "B": "collapsed_node_exp",
        "+CN": "no_morton_code_opt",
        "+GM": "generalize_morton_code_exp",
        "+SM": "stagger_morton_code_exp"
    }

    dataset_to_size = {
        "tpch": 250000000,
        "github": 200000000,
        "nyc": 200000000,
    }
    for workload in ["search", "storage", "insert", "lookup"]:

        name_list = []
        value_list_list = []

        for dataset in ["tpch", "github", "nyc"]:

            regex_search = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")
            regex_storage = re.compile(r"mdtrie storage: (?P<storage>[0-9.]+?)\n")
            search_base = {}

            for optimization in optimization_list:
        
                dataset_size = dataset_to_size[dataset]
                system_path = "../../results/microbench/sensitivity_optimization/{}_{}_{}".format(dataset, workload, optimization_translation[optimization])

                if optimization == "B":
                    dataset_size /= 50
                    system_path += "_reduced"
                
                if optimization == "+CN" and dataset == "nyc":
                    dataset_size /= 50
                    system_path += "_reduced"

                # if optimization == "+GM" and dataset == "nyc":
                #     system_path += "_new"

                if (dataset == "tpch" or dataset == "github") and workload == "search":
                    system_path += "_random_query"

                # if dataset == "nyc" and workload == "search":
                #     system_path += "_random_query_dup"

                value_map = {}
                idx = 0
                with open("{}".format(system_path)) as ifile:
                    for line in ifile:

                        if workload == "search":
                            m = regex_search.search(line)
                            if float(m.group("points")) > 10 and float(m.group("elaspsed")) > 5:
                                value_map[idx] = float(m.group("elaspsed")) * 1000 / int(m.group("points"))
                                # if value_map[idx] > 30000:
                                #     print(line)
                                #     exit(0)
                                '''
                                if optimization == "B":
                                    search_base[idx] = value_map[idx]
                                    
                                value_map[idx] = value_map[idx] / search_base[idx] 
                                '''
                                
                        elif workload == "insert" or workload == "lookup":
                            value_map[idx] = int(line) + 95 # network
                            
                        elif workload == "storage":
                            m = regex_storage.search(line)
                            value_map[idx] = int(m.group("storage")) / dataset_size

                        idx += 1

                value_list = list(value_map.values())
                
                '''
                if dataset == "nyc" and optimization == "+SM":
                    print(value_list)
                    
                    
                    for idx, value in enumerate(value_list):
                        if value > 100:
                            print(idx, value)
                    '''

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
            # print(optimization_list)
            # storage_value_list_list = [
            #     ["TPC-H", 14.076, 54.476, 28.876],
            #     ["GitHub\nEvents", 4.143, 55.876, 26.576],
            #     ["NYC\nTaxi", 3.783, 54.976, 29.876]
            # ]
            # continue
            plot_bar_group_optimization(storage_list_list, optimization_list, "sensitivity_optimization/storage_optimization_sensitivity", "storage", True, True)
            # exit(0)

        else:
            if workload == "lookup":
                print([sum(value_list) / len(value_list) for value_list in value_list_list])
                print(name_list)
            # continue
            # continue
            plot_optimization_box(value_list_list, name_list, "sensitivity_optimization/{}_optimization_sensitivity".format(workload), workload, workload=="insert" or workload=="lookup" or workload=="storage" or workload == "search")


