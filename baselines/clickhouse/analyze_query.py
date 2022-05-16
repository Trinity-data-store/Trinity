import re 
import numpy as np
import matplotlib.pyplot as plt

datapath = "tpch_queries"

if __name__ == "__main__": 

    elapsed_to_found_points = {}

    # SELECT * FROM tpch_macro WHERE SHIPDATE BETWEEN 19949069 AND 19950101 AND DISCOUNT BETWEEN 5 AND 7 AND QUANTITY <= 24;, elapsed: 9.582532167434692s, found points: 162727
    regex = re.compile(r"elapsed: (?P<elaspsed>[0-9.]+?)s, found points: (?P<points>[0-9.]+?)")
    elapsed_values = []
    found_points_values = []

    with open("{}".format(datapath)) as ifile:
        for line in ifile:
            m = regex.search(line)
            if m:
                
                elapsed_to_found_points[float(m.group("elaspsed"))] = int(m.group("points"))
                elapsed_values.append(float(m.group("elaspsed")))
                found_points_values.append(int(m.group("points")))
            else:
                pass

    # print(elapsed_to_found_points)

    plt.scatter(found_points_values, elapsed_values, alpha=0.5)
    plt.show()

