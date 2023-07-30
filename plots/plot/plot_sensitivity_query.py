
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

DATASET_SIZE = 250000000
query_selectivity = [0.0001, 0.0002, 0.0004, 0.0008, 0.0016, 0.0032, 0.0064, 0.0128, 0.0256]
query_selectivity = [0.0001, 0.0004, 0.0016, 0.0064, 0.0256, 0.0256 * 4]
# query_selectivity = [0.0001, 0.0004, 0.0016, 0.0064, 0.0256]

name_list = []
bin_count = [0] * (len(query_selectivity) - 1)
value_list_list = [list() for _ in range(len(query_selectivity) - 1)]


if __name__ == "__main__": 


    for selecitivty_i in range(len(query_selectivity) - 1):
        # name_list.append(str(query_selectivity[selecitivty_i] * 100) + "%\n" + str(query_selectivity[selecitivty_i + 1] * 100) + "%")
        name_list.append(str((query_selectivity[selecitivty_i] * 100 + query_selectivity[selecitivty_i + 1] * 100) / 2) + "%")


    regex_search = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")

    latency_list = []
    found_points_list = []

    system_path = "../../results/microbench/sensitivity_query/tpch_trinity"
    out_file = open("../../results/microbench/sensitivity_query/tpch_trinity_cleaned", "w")

    idx = 0
    with open("{}".format(system_path)) as ifile:
        for line in ifile:

            m = regex_search.search(line)
            latency_list.append(float(m.group("elaspsed")) )
            found_points_list.append(int(m.group("points")))


    for pt_idx, pt_count in enumerate(found_points_list):

        for selecitivty_i in range(len(query_selectivity) - 1):
            if pt_count >= DATASET_SIZE * query_selectivity[selecitivty_i] and pt_count <= DATASET_SIZE * query_selectivity[selecitivty_i + 1]:
                bin_count[selecitivty_i] += 1
                value_list_list[selecitivty_i].append(float(latency_list[pt_idx]) * 1000 / pt_count)

                # if pt_idx < 100:
                #     print(selecitivty_i, float(latency_list[pt_idx]), pt_count)
                # break

    print(bin_count)
    # print([value_list[:5] for value_list in value_list_list])
    plot_box(value_list_list, name_list, "sensitivity_query", "search", False, "Query Selectivity")








