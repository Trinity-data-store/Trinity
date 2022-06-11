import re 
import sys
import numpy as np
import statistics

sampled_count = 1000
need_diff_count = int(sys.argv[1])
diff_count_filter = False
if __name__ == "__main__": 


    clickhouse_path = "query_tpch_rerun"
    trinity_path = "query_tpch_trinity"
    # trinity_path = "tpch_trinity_with_continue"
    trinity_path = "../../build/query_tpch_trinity"
    converted_path = "query_tpch_rerun_converted"

    # clickhouse_path = "query_tpch_T1_range0.10_rerun"
    # trinity_path = "../../build/query_tpch_T1_trinity"
    # converted_path = "query_tpch_T1_range0.10_rerun_converted"
    diff_count_list = []

    with open("{}".format(converted_path)) as ifile:
        for line in ifile:
            splitted_line = [int(token) for token in line.split(",")]
            # 0,29,31,2,1,3,4,19920102,19981201,5,19920131,19981031,6,19920103,19981231,8,19979510,19980414

            # std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
            # std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};

            diff_count = 0
            if splitted_line[1] == 1 and splitted_line[2] == 50:
                diff_count += 1
            if splitted_line[4] == 0 and splitted_line[5] == 10:
                diff_count += 1
            if splitted_line[7] == 19920102 and splitted_line[8] == 19981201:
                diff_count += 1
            if splitted_line[10] == 19920131 and splitted_line[11] == 19981031:
                diff_count += 1
            if splitted_line[13] == 19920103 and splitted_line[14] == 19981231:
                diff_count += 1
            if splitted_line[16] == 19920101 and splitted_line[17] == 19980802:
                diff_count += 1
            diff_count_list.append(6 - diff_count)
            # if diff_count == 0:
            #     print(len(diff_count_list))
    
    clickhouse_latencies = {}
    trinity_latencies = {}
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
            if diff_count_list[idx] == need_diff_count or not diff_count_filter:
                clickhouse_latencies[idx] = float(m.group("elaspsed"))
                found_points[idx] = int(m.group("points"))
            idx += 1
            if idx > sampled_count:
                break

    idx = 0
    with open("{}".format(trinity_path)) as ifile:
        for line in ifile:
            m = regex_trinity.search(line)
            if not m:
                print(line)
            if diff_count_list[idx] == need_diff_count or not diff_count_filter:
                trinity_latencies[idx] = float(m.group("elaspsed")) / 1000
                found_point = int(m.group("points"))

                if trinity_latencies[idx] > 80:
                    print(idx, trinity_latencies[idx])
                if found_points[idx] != found_point:
                    print(found_points[idx], found_point, idx)
                    # print(idx)
                    # exit(0)
            idx += 1
            if idx > sampled_count:
                break

    print(clickhouse_latencies, trinity_latencies)

    print("clickhouse: ", sum(clickhouse_latencies.values()) / len(clickhouse_latencies.values()), "error bar: ", statistics.stdev(clickhouse_latencies.values()), "count", len(trinity_latencies.values()))
    print("trinity: ", sum(trinity_latencies.values()) / len(trinity_latencies.values()), "error bar: ", statistics.stdev(trinity_latencies.values()), "count", len(trinity_latencies.values()))
    plot_miss_ratio_diff([clickhouse_latencies.values(), trinity_latencies.values()], ["clickhouse", "trinity"], "fig.jpg")
