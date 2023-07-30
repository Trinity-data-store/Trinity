
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

# def plot_box(value_list_list:List[List], name_list:List, figname:str, workload:str):

#     """
#     plot the miss ratio reduction compared to baseline 
#     """ 

#     plt.subplots(figsize=(12,8))
#     linestyles = itertools.cycle(get_linestyles())

#     data_list = [np.mean(value_list) for value_list in value_list_list]
#     error_list = [np.std(value_list) for value_list in value_list_list]
#     x_pos = np.arange(len(name_list))

#     # fig, ax = plt.subplots()
#     plt.bar(x_pos, data_list, yerr=error_list, align='center', alpha=0.5, color=colors[workload], hatch=hatches[workload], capsize=10, width=0.5)
#     if workload == "storage":
#         plt.ylabel('GB')
#     elif workload == "insert" or workload == "lookup":
#         plt.ylabel('{} Latency (us)'.format(workload.capitalize()))
#     elif workload == "search":
#         plt.ylabel('{} Latency (ms)'.format(workload.capitalize()))
#     plt.xticks(x_pos, name_list)

#     plt.grid(which='major', axis='y', linestyle='--', zorder=0)
#     # plt.legend(loc='upper center', bbox_to_anchor=(1,1))
#     plt.savefig("sensitivity_size/{}.png".format(figname), bbox_inches="tight")
#     plt.clf()


if __name__ == "__main__": 

    name_list = ["1/32", "1/16", "1/8", "1/4", "1/2", "1"]
    for workload in ["insert", "lookup", "search", "storage"]:

        regex_search = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")
        regex_storage = re.compile(r"mdtrie storage: (?P<storage>[0-9.]+?)\n")

        value_list_list = []
        for dataset_size in [32, 16, 8, 4, 2, 1]:
    

            system_path = "../../results/microbench/sensitivity_size/{}_{}".format(workload, dataset_size)

            if workload == "search" and dataset_size == 1:
                system_path += "_new"
            if workload == "search" and dataset_size != 1:
                system_path += "_second"
                
            value_map = {}
            idx = 0
            with open("{}".format(system_path)) as ifile:
                for line in ifile:

                    if workload == "search":
                        m = regex_search.search(line)
                        value_map[idx] = float(m.group("elaspsed")) * 1000 / int(m.group("points"))
                    elif workload == "insert" or workload == "lookup":
                        value_map[idx] = int(line) + 95 # network
                    elif workload == "storage":
                        m = regex_storage.search(line)
                        value_map[idx] = int(m.group("storage")) / 250000000 * dataset_size

                    idx += 1
                    
            value_list_list.append(list(value_map.values()))

        if workload == "storage":
            plot_bar(value_list_list, name_list, "sensitivity_size/{}_sensitivity_size".format(workload), workload, "Fraction of Original Size")
        else:
            plot_box(value_list_list, name_list, "sensitivity_size/{}_sensitivity_size".format(workload), workload, False, "Fraction of Original Size")


