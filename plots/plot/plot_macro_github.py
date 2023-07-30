
import os
import sys 
sys.path.append(os.path.expanduser("~/workspace/"))
sys.path.append(os.path.expanduser("../"))
from pyutils.common import * 
import re 
import itertools
from typing import List, Dict

def plot_miss_ratio_diff(miss_ratio_list_list:List[List], name_list:List, figname:str):
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

    # plt.xlim(0, )

    # plt.xlim(500)
    plt.xscale("log")
    plt.xlabel("Latency (ms)")
    plt.ylabel("Fraction of queries (CDF)")

    # for axis in [ax.xaxis, ax.yaxis]:
    #     axis.set_major_formatter(ScalarFormatter())

    plt.legend(loc="upper right", bbox_to_anchor=(1.03, 1.31), ncol=2, prop={'size': 36}, columnspacing=1.0, handletextpad=0.5)
    plt.grid(linestyle="--")
    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")
    plt.clf()

sampled_count = 1000

diff_count_filter = True


clickhouse_latency_list = []
trinity_latency_list = []
timescale_latency_list = []
aerospike_latency_list = []
selectivity = []
dimensions = []

if __name__ == "__main__": 

    clickhouse_path = "../../results/macrobench/github_clickhouse_combined"
    trinity_path = "../../results/macrobench/github_trinity_combined"
    timescale_path = "../../results/macrobench/github_timescale_combined"
    aerospike_path = "../../results/macrobench/github_aerospike_combined"

    diff_count_list = []
 
    clickhouse_latencies = {}
    trinity_latencies = {}
    timescale_latencies = {}
    aerospike_latencies = {}

    elapsed_to_found_points = {}

    # SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN 19949069 AND 19950101 AND DISCOUNT BETWEEN 5 AND 7 AND QUANTITY <= 24;, elapsed: 9.582532167434692s, found points: 162727
    regex_clickhouse = re.compile(r"elapsed: (?P<elaspsed>[0-9.]+?)s, found points: (?P<points>[0-9.]+?)\n")
    # Query 1 end to end latency (ms): 192241, found points count: 71888648
    regex_trinity = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)\n")

    idx = 0
    found_points = {}
    with open("{}".format(clickhouse_path)) as ifile:
        for line in ifile:
            m = regex_clickhouse.search(line)
            if m:
                clickhouse_latencies[idx] = float(m.group("elaspsed")) * 1000
                found_points[idx] = int(m.group("points"))
                clickhouse_latency_list.append(float(m.group("elaspsed")))

            idx += 1
            if idx > sampled_count:
                break

    idx = 0
    with open("{}".format(trinity_path)) as ifile:
        for line in ifile:
            m = regex_trinity.search(line)
            if not m:
                print(line)
            if m:
                trinity_latencies[idx] = float(m.group("elaspsed"))
                found_point = int(m.group("points"))
                trinity_latency_list.append(float(m.group("elaspsed")) / 1000)
                if found_points[idx] != found_point:
                    print("Trinity", found_points[idx], found_point, idx)

            idx += 1
            if idx > sampled_count:
                break

    idx = 0
    with open("{}".format(aerospike_path)) as ifile:
        for line in ifile:
            m = regex_clickhouse.search(line)
            if m:
                found_point = int(m.group("points"))
                aerospike_latency_list.append(float(m.group("elaspsed")))
                aerospike_latencies[idx] = float(m.group("elaspsed")) * 1000
                if found_points[idx] != found_point:
                    print("Aerospike", found_points[idx], found_point, idx)
            idx += 1
            if idx > sampled_count:
                break

    idx = 0
    with open("{}".format(timescale_path)) as ifile:
        for line in ifile:
            m = regex_clickhouse.search(line)
            if m:
                found_point = int(m.group("points"))
                timescale_latency_list.append(float(m.group("elaspsed")))
                timescale_latencies[idx] = float(m.group("elaspsed")) * 1000
                if found_points[idx] != found_point:
                    print("Timescale", found_points[idx], found_point, idx)
            idx += 1
            if idx > sampled_count:
                break

    at_least = 1000
    trinity_clickhouse_max = 0
    trinity_aerospike_max = 0
    trinity_timescale_max = 0
    average_cumulative = 0
    for i in range(sampled_count):
        trinity_clickhouse = clickhouse_latency_list[i] / trinity_latency_list[i]
        trinity_aerospike = aerospike_latency_list[i] / trinity_latency_list[i] 
        trinity_timescale = timescale_latency_list[i] / trinity_latency_list[i]

        at_least = min(at_least, trinity_aerospike, trinity_clickhouse, trinity_timescale)
        trinity_clickhouse_max = max(trinity_clickhouse_max, trinity_clickhouse)
        trinity_aerospike_max = max(trinity_aerospike_max, trinity_aerospike)
        trinity_timescale_max = max(trinity_timescale_max, trinity_timescale)
        # average_cumulative += trinity_clickhouse + trinity_aerospike + trinity_timescale
        average_cumulative += trinity_timescale

    # print("at_least", at_least)
    # print("trinity_clickhouse_max", trinity_clickhouse_max)
    # print("trinity_aerospike_max", trinity_aerospike_max)
    # print("trinity_timescale_max", trinity_timescale_max)
    # print("average", average_cumulative / sampled_count)
    # exit(0)
    
    plot_miss_ratio_diff([trinity_latencies.values(), clickhouse_latencies.values(), aerospike_latencies.values(), timescale_latencies.values()], ["Trinity", "ClickHouse",  "Aerospike", "TimescaleDB"], "github_search_macro")


