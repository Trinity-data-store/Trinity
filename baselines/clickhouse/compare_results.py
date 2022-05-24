import re 
import numpy as np
import statistics

template_set = [1, 4, 5]
sampled_count = 20

if __name__ == "__main__": 

    for template_idx in template_set:

        clickhouse_path = "query_tpch_T{}_range0.10_rerun".format(template_idx)
        trinity_path = "query_tpch_T{}_range0.10_trinity".format(template_idx)

        clickhouse_latencies = []
        trinity_latencies = []
        elapsed_to_found_points = {}

        # SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN 19949069 AND 19950101 AND DISCOUNT BETWEEN 5 AND 7 AND QUANTITY <= 24;, elapsed: 9.582532167434692s, found points: 162727
        regex_clickhouse = re.compile(r"elapsed: (?P<elaspsed>[0-9.]+?)s, found points: (?P<points>[0-9.]+?)")
        # Query 1 end to end latency (ms): 192241, found points count: 71888648
        regex_trinity = re.compile(r"latency \(ms\): (?P<elaspsed>[0-9.]+?), found points count: (?P<points>[0-9.]+?)")

        with open("{}".format(clickhouse_path)) as ifile:
            for line in ifile:
                m = regex_clickhouse.search(line)
                clickhouse_latencies.append(float(m.group("elaspsed")))

        with open("{}".format(trinity_path)) as ifile:
            for line in ifile:
                m = regex_trinity.search(line)
                if not m:
                    print(line)
                trinity_latencies.append(float(m.group("elaspsed")) / 1000)

        print("template index: ", template_idx)
        print("clickhouse: ", sum(clickhouse_latencies[:sampled_count]) / sampled_count, "error bar: ", statistics.stdev(clickhouse_latencies[:sampled_count]))
        print("trinity: ", sum(trinity_latencies[:sampled_count]) / sampled_count, "error bar: ", statistics.stdev(trinity_latencies[:sampled_count]))






