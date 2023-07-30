
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
import random
from pyutils.plot_functions import *

def plot_cdf(miss_ratio_list_list:List[List], name_list:List, figname:str):
    """
    plot the miss ratio reduction compared to baseline 
    """ 

    plt.subplots(figsize=(24,8))

    linestyles = itertools.cycle(get_linestyles())

    plt.subplots(figsize=(12,8))
    for i in range(len(miss_ratio_list_list)):
        miss_ratio_reduction = np.array(list(miss_ratio_list_list[i]))
        # hit_ratio_improvement = (1 - np.array(miss_ratio_list_list[i]) - hit_ratio_baseline) / hit_ratio_baseline
        name = name_list[i]

        plt.plot(*conv_to_cdf(miss_ratio_reduction), label=name, linestyle=next(linestyles)) 

    # plt.xlim(95)
    plt.xscale("log")
    plt.xlabel("Latency (us)")
    plt.ylabel("Fraction of queries (CDF)")

    plt.legend(loc="upper left", bbox_to_anchor=(1,1))
    plt.grid(linestyle="--")
    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.clf()

NUM_SAMPLES = 23999

if __name__ == "__main__": 

    root_dir = "../../results/microbench/latency_cdf/"

    for workload in ["insert", "lookup"]:

        value_list_list = []
        name_list = []
        for dataset in ["tpch", "github", "nyc"]:
        
            for baseline in ["trinity", "phtree", "rstar"]:
                
                value_list = []
                file_str = "{}_{}_{}".format(baseline, dataset, workload)
                if baseline == "rstar" and dataset == "nyc" and workload == "insert":
                    file_str += "_new"
                with open("{}".format(root_dir + file_str)) as ifile:

                    for line in ifile:
                        value_list.append(float(line) + 95.0 + random.random()) # network latency + for better plot

                
                value_list_list.append(value_list)
                name_list.append(dataset + "\n" + baseline)

        # print([sum(value_list) / len(value_list) for value_list in value_list_list])

        plot_micro_box(value_list_list, name_list, "microbench/{}".format("micro_" + workload), workload, False)





