
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

    clickhouse_path = "../../results/macrobench/tpch_clickhouse"
    converted_path = "../../results/macrobench/tpch_query_new_converted"
    trinity_path = "../../results/macrobench/tpch_trinity_confirmed"
    timescale_path = "../../results/macrobench/tpch_timescaledb"
    aerospike_path = "../../results/macrobench/tpch_aerospike"

    diff_count_list = []
    sampled_attributes_list = []
    
    with open("{}".format(converted_path)) as ifile:
        for line in ifile:
            splitted_line = [int(token) for token in line.split(",")]
            # 0,29,31,2,1,3,4,19920102,19981201,5,19920131,19981031,6,19920103,19981231,8,19979510,19980414

            # std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
            # std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};
            # [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]

            sampled_attributes = []
            diff_count = 0
            if splitted_line[1] == 1 and splitted_line[2] == 50:
                diff_count += 1
                sampled_attributes.append("QUANTITY")
            if splitted_line[4] == 0 and splitted_line[5] == 10:
                diff_count += 1
                sampled_attributes.append("DISCOUNT")
            if splitted_line[7] == 19920102 and splitted_line[8] == 19981201:
                diff_count += 1
                sampled_attributes.append("SHIPDATE")
            if splitted_line[10] == 19920131 and splitted_line[11] == 19981031:
                diff_count += 1
                sampled_attributes.append("COMMITDATE")
            if splitted_line[13] == 19920103 and splitted_line[14] == 19981231:
                diff_count += 1
                sampled_attributes.append("RECEIPTDATE")
            if splitted_line[16] == 19920101 and splitted_line[17] == 19980802:
                diff_count += 1
                sampled_attributes.append("ORDERDATE")
            diff_count_list.append(6 - diff_count)
            sampled_attributes_list.append(sampled_attributes)

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
                selectivity.append(int(m.group("points")) / 3000028242 * 100)
                # if need_diff_count != 0:
                dimensions.append(diff_count_list[idx])
                # if diff_count_list[idx] == 2:
                #     print(idx)
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
    print("average", average_cumulative / sampled_count)

    plot_miss_ratio_diff([trinity_latencies.values(), clickhouse_latencies.values(), aerospike_latencies.values(), timescale_latencies.values()], ["Trinity", "ClickHouse",  "Aerospike", "TimescaleDB"], "tpch_search_macro")