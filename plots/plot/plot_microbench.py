
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

def plot_cdf(latency_list_list:List[List], name_list:List, figname:str):
    """
    plot the miss ratio reduction compared to baseline 
    """ 

    plt.subplots(figsize=(24,8))
    linestyles = itertools.cycle(get_linestyles())

    plt.subplots(figsize=(12,8))
    for i in range(len(latency_list_list)):
        latency_list = np.array(list(latency_list_list[i]))
        name = name_list[i]

        plt.plot(*conv_to_cdf(latency_list), label=name, linestyle=next(linestyles)) 

    # plt.xlim(100)
    plt.xlabel("Latency (ms)")
    plt.ylabel("Fraction of queries (CDF)")
    plt.legend(loc="upper left", bbox_to_anchor=(1,1))
    plt.grid(linestyle="--")
    plt.savefig("{}.png".format(figname), bbox_inches="tight")

    plt.clf()

sampled_count = 1000
diff_count_filter = True
dataset = "tpch"

full_dataset_list =  ["tpch", "github", "nyc"]
full_system_list = ["trinity", "phtree", "rstar"]

if __name__ == "__main__": 

    name_list = []
    latency_value_list = []

    for dataset in full_dataset_list:

        system_list = full_system_list
        found_points = {}
        regex = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")

        for system in system_list:

            system_path = "../../results/microbench/{}_{}".format(dataset, system)
            latencies = {}
            idx = 0
            with open("{}".format(system_path)) as ifile:
                for line in ifile:
                    m = regex.search(line)

                    if int(m.group("points")) < 1000:
                        continue

                    latencies[idx] = float(m.group("elaspsed")) 
                    if idx not in found_points:
                        found_points[idx] = int(m.group("points"))
                    elif found_points[idx] != int(m.group("points")):
                        print(system, found_points[idx], int(m.group("points")))

                    idx += 1
                    if idx > sampled_count:
                        break
            latency_value_list.append(list(latencies.values()))
            name_list.append(dataset + "\n" + system)

        # plot_cdf(latency_value_list, system_list, "microbench/{}-microbench".format(dataset))



    latency_value_list_average = [sum(l) / len(l) for l in latency_value_list]
    print(latency_value_list_average)
    plot_micro_box(latency_value_list, name_list, "micro_search", "search", False)
    # exit(0)
    '''
    Plot storage
    '''

    storage_value_list_list = [
        ["TPC-H", 14.076, 54.476, 28.876],
        ["GitHub\nEvents", 4.143, 55.876, 26.576],
        ["NYC\nTaxi", 3.783, 54.976, 29.876]
    ]


    for i in range(len(storage_value_list_list)):
        for j in range(1, len(storage_value_list_list[0])):
            if i == 0:
                storage_value_list_list[i][j] *= 1073741824 / 250000000
            
            else:
                storage_value_list_list[i][j] *= 1073741824 / 200000000

    systems = ["MdTrie", "PH-Tree",  "R*-Tree"]
    plot_bar_group(storage_value_list_list, systems, "microbench/micro_storage", "storage", False, True)


                    
