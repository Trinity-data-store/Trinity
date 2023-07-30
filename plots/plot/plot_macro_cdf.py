
import os
import sys 
sys.path.append(os.path.expanduser("~/workspace/"))
sys.path.append(os.path.expanduser("../"))
from pyutils.common import * 
import itertools
from typing import List

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



if __name__ == "__main__": 

    root_dir = "../../results/macrobench/latency_cdf/"
 
    for dataset in ["github", "nyc", "tpch"]:
        
        for workload in ["insert", "lookup"]:

            baseline_to_latency_list = {
                "clickhouse": [],
                "trinity": [],
                "timescale": [],
                "aerospike": []
            }

            for baseline in ["trinity", "aerospike", "clickhouse", "timescale"]:

                with open("{}".format(root_dir + "{}_{}_{}".format(baseline, dataset, workload))) as ifile:

                    for line in ifile:

                        baseline_to_latency_list[baseline].append(float(line))

            plot_cdf([baseline_to_latency_list["trinity"], baseline_to_latency_list["clickhouse"], baseline_to_latency_list["aerospike"], baseline_to_latency_list["timescale"]], ["Trinity", "ClickHouse", "Aerospike", "Timescale"], "latency_cdf/{}_{}_latency_cdf".format(dataset, workload))




