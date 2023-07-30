
from distutils.log import error
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
import pandas as pd 

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

def plot_bar(value_list_list:List[List], name_list:List, figname:str, workload:str, xaxis_label:str = ""):

    """
    plot the miss ratio reduction compared to baseline 
    """ 

    plt.subplots(figsize=(12,8))
    linestyles = itertools.cycle(get_linestyles())

    data_list = [np.mean(value_list) for value_list in value_list_list]
    error_list = [np.std(value_list) for value_list in value_list_list]
    x_pos = np.arange(len(name_list))

    # fig, ax = plt.subplots()
    if workload == "storage":
        plt.bar(x_pos, data_list, align='center', alpha=0.5, color='none', edgecolor=colors[workload], hatch=hatches[workload], capsize=10, width=0.5)
    else:
        plt.bar(x_pos, data_list, yerr=error_list, align='center', alpha=0.5, color='none', edgecolor=colors[workload], hatch=hatches[workload], capsize=10, width=0.5)
    if workload == "storage":
        plt.ylabel('bytes/pt')
    elif workload == "insert" or workload == "lookup":
        plt.ylabel('{} Latency (us)'.format(workload.capitalize()))
    elif workload == "search":
        plt.ylabel('{} Latency (us/pt)'.format(workload.capitalize()))
    plt.xticks(x_pos, name_list)
    if xaxis_label != "":
        plt.xlabel(xaxis_label) 
    # if workload != "storage":
    #     plt.yscale("log")
    plt.grid(which='major', axis='y', linestyle='--', zorder=0)
    # plt.legend(loc='upper center', bbox_to_anchor=(1,1))
    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")

    plt.clf()

def plot_box(value_list_list:List, name_list:List, figname:str, workload:str, isLog:bool = False, xaxis_label:str = ""):

    """
    plot the miss ratio reduction compared to baseline 
    """ 

    plt.subplots(figsize=(12,8))
    n_algo = len(value_list_list)
    linestyles = itertools.cycle(get_linestyles())

    # hit_ratio_improvement = [(1 - np.array(miss_ratio_list_list[i]) - hit_ratio_baseline) / hit_ratio_baseline for i in range(len(miss_ratio_list_list))]
    value_list_list = [np.array(value_list) for value_list in value_list_list]

    value_list_list = ["dummy"] + value_list_list
    name_list = ["dummy"] + name_list

    meanprops = dict(marker='v', markerfacecolor='g', markersize=10,
                  linestyle='none', markeredgecolor='r')    
    plt.boxplot(value_list_list[1:], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops)
    if figname == "sensitivity_query":
        plt.xticks(range(1, len(name_list)), name_list[1:])
    else:
        plt.xticks(range(1, len(name_list)), name_list[1:])

    if xaxis_label != "":
        plt.xlabel(xaxis_label) 

    if workload == "storage":
        plt.ylabel('bytes/pt')
        # plt.yscale("log")

    elif workload == "insert" or workload == "lookup":
        plt.ylabel('{} Latency (us/pt)'.format(workload.capitalize()))
        # plt.yscale("log")

    elif workload == "search":
        plt.ylabel('{} Latency (us/pt)'.format(workload.capitalize()))
        # plt.yscale("log")

    plt.ylim(0)

    if isLog:
        plt.yscale("log")
    plt.grid(linestyle="--")
    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")

    plt.clf()


def plot_bar_group(value_list_list:List[List], systems:List, figname:str, workload:str, islog:bool = False, isMicro:bool = False):

    ax = plt.figure(figsize=(12,8)).add_subplot(111)

    # plt.subplots(figsize=(12,8))

    df=pd.DataFrame(value_list_list,columns=["Datasets"] + systems)

    df.plot(ax=ax, x="Datasets", y=systems, kind="bar", rot=0, width=0.6, color='none')
    plt.xlabel("")
    if workload == "storage":
        plt.ylabel("Storage (bytes/pt)")
    if workload == "throughput":
        plt.ylabel("Throughput (kpts/sec)")

    plt.grid(linestyle="--")

    patterns =("++", "xx", "//","..")
    colors = ["royalblue", "orange", "green", "Red"]
    hatches = [p for p in patterns for i in range(len(df))]
    edge_colors = [c for c in colors for i in range(len(df))]
    bars = ax.patches

    idx = 0
    for bar, hatch in zip(bars, hatches):
        bar.set_hatch(hatch)
        bar.set_edgecolor(edge_colors[idx])
        idx += 1

    if islog:
        plt.yscale("log")

    if not isMicro:
        plt.legend(loc="upper right", bbox_to_anchor=(1.03, 1.31), ncol=2, prop={'size': 36}, columnspacing=1.0, handletextpad=0.5)
    else:
        plt.legend(loc="upper right", bbox_to_anchor=(1.048, 1.19), ncol=3, prop={'size': 36}, columnspacing=0.60, handletextpad=0.3)


    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")

    plt.clf()

def plot_micro_box(value_list_list:List, name_list:List, figname:str, workload:str, isLog:bool = False):

    """
    plot the miss ratio reduction compared to baseline 
    """ 

    plt.subplots(figsize=(12,8))
    n_algo = len(value_list_list)
    linestyles = itertools.cycle(get_linestyles())

    # hit_ratio_improvement = [(1 - np.array(miss_ratio_list_list[i]) - hit_ratio_baseline) / hit_ratio_baseline for i in range(len(miss_ratio_list_list))]
    if workload == "search":
        value_list_list = [np.array([float(value) / 1000 for value in value_list]) for value_list in value_list_list]
    else:
        value_list_list = [np.array(value_list) for value_list in value_list_list]

    value_list_list = ["dummy"] + value_list_list
    name_list = ["dummy"] + name_list

    meanprops = dict(marker='v', markerfacecolor='r', markersize=12,
                  linestyle='none', markeredgecolor='b')    

    left_positions = [-0.4,  2.2 + 0.5,  5+ 1]
    middle_positions = [0.6, 3.2+ 0.5, 6+ 1]
    right_positions = [1.6, 4.2+ 0.5, 7+ 1]
    combined_positions = middle_positions
    # for i in range(len(left_positions)):
    #     combined_positions.append(left_positions[i])
    #     combined_positions.append(middle_positions[i])
    #     combined_positions.append(right_positions[i])

    medianprops = {"linewidth": 2, "solid_capstyle": "butt", "color": "red"}

    fig, ax = plt.subplots()
    bp1 = ax.boxplot(value_list_list[1:10:3], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=left_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=1)
    bp2 = ax.boxplot(value_list_list[2:10:3], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=middle_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=1)
    bp3 = ax.boxplot(value_list_list[3:10:3], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=right_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=1)
    
    plt.xticks(combined_positions, ["TPC-H", "GitHub\nEvents", "NYC\nTaxi"])

    patterns =("++", "xx", "//","..")
    colors = ["royalblue", "orange", "green", "Red"]
    idx = 0

    for bp in [bp1, bp2, bp3]:
        boxes = bp['boxes']

        for box, hatch in zip(boxes, hatches):
            box.set_hatch(patterns[idx])
            box.set_edgecolor(colors[idx])
        idx += 1

    
    ax.legend([bp1["boxes"][0], bp2["boxes"][0], bp3["boxes"][0]], ['MdTrie', 'PH-Tree', 'R*-Tree'], loc="upper right", bbox_to_anchor=(1.08, 1.2), ncol=3, prop={'size': 36}, columnspacing=1.0, handletextpad=0.5)

    plt.ylim(0)

    if workload == "storage":
        plt.ylabel('bytes/pt')
        # plt.yscale("log")

    elif workload == "insert" or workload == "lookup":
        plt.ylabel('{} Latency (us/pt)'.format(workload.capitalize()))
        if workload == "insert":
            plt.ylim(90, 225)
        else:
            plt.ylim(90)
            plt.yscale("log")

    elif workload == "search":
        plt.ylabel('{} Latency (s)'.format(workload.capitalize()))
        # plt.yscale("log")


    if isLog:
        plt.yscale("log", nonpositive='clip')
        plt.ylim(50)
    plt.grid(linestyle="--")
    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")

    plt.clf()


def plot_optimization_box(value_list_list:List, name_list:List, figname:str, workload:str, isLog:bool = False):

    """
    plot the miss ratio reduction compared to baseline 
    """ 

    plt.subplots(figsize=(12,8))
    n_algo = len(value_list_list)
    linestyles = itertools.cycle(get_linestyles())

    value_list_list = [np.array(value_list) for value_list in value_list_list]

    value_list_list = ["dummy"] + value_list_list
    name_list = ["dummy"] + name_list

    meanprops = dict(marker='v', markerfacecolor='r', markersize=12,
                  linestyle='none', markeredgecolor='b')    

    left_positions = [-0.4,  2.4,  5.2]
    middle_positions = [0.2, 3.0, 5.8]
    right_positions = [0.8, 3.6, 6.4]
    # second_right_positions = [1.4, 4.2, 7.0]
    second_right_positions = [1.4, 7.0]

    combined_positions = [0.5, 3.3, 6.1]
    # for i in range(len(left_positions)):
    #     combined_positions.append(left_positions[i])
    #     combined_positions.append(middle_positions[i])
    #     combined_positions.append(right_positions[i])

    medianprops = {"linewidth": 2, "solid_capstyle": "butt", "color": "red"}

    fig, ax = plt.subplots()
    bp1 = ax.boxplot(value_list_list[1:13:4], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=left_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=0.7)
    bp2 = ax.boxplot(value_list_list[2:13:4], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=middle_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=0.7)
    bp3 = ax.boxplot(value_list_list[3:13:4], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=right_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=0.7)
    bp4 = ax.boxplot(value_list_list[4:13:8], whis=(5, 95), showfliers=False, showmeans=True, meanprops=meanprops, patch_artist=True,positions=second_right_positions, boxprops=dict(facecolor="white"), medianprops=medianprops, widths=0.7)

    plt.xticks(combined_positions, ["TPC-H", "GitHub\nEvents", "NYC\nTaxi"])

    patterns =("++", "xx", "//", "..")
    colors = ["royalblue", "orange", "green", "Red"]
    idx = 0

    for bp in [bp1, bp2, bp3, bp4]:
        boxes = bp['boxes']

        for box, hatch in zip(boxes, hatches):
            box.set_hatch(patterns[idx])
            box.set_edgecolor(colors[idx])
        idx += 1


    ax.legend([bp1["boxes"][0], bp2["boxes"][0], bp3["boxes"][0], bp4["boxes"][0]], ["B", "+CN", "+GM", "+SM"], loc="upper right", bbox_to_anchor=(1.1, 1.2), ncol=4, prop={'size': 36}, columnspacing=1.0, handletextpad=0.5)

    if workload == "storage":
        plt.ylabel('bytes/pt')
        # plt.yscale("log")

    elif workload == "insert" or workload == "lookup":
        plt.ylabel('{} Latency (us/pt)'.format(workload.capitalize()))
        # plt.yscale("log")

    elif workload == "search":
        plt.ylabel('{} Latency (us/pt)'.format(workload.capitalize()))
        # plt.yscale("log")

    if isLog:
        plt.yscale("log")
    plt.grid(linestyle="--")
    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")

    plt.clf()


def plot_bar_group_optimization(value_list_list:List[List], systems:List, figname:str, workload:str, islog:bool = False, isMicro:bool = False):

    ax = plt.figure(figsize=(14,8)).add_subplot(111)

    # plt.subplots(figsize=(12,8))

    df=pd.DataFrame(value_list_list,columns=["Datasets"] + systems)

    df.plot(ax=ax, x="Datasets", y=systems, kind="bar", rot=0, width=0.7, color='none')
    plt.xlabel("")
    if workload == "storage":
        plt.ylabel("Storage (bytes/pt)")
    if workload == "throughput":
        plt.ylabel("Throughput (kpts/sec)")

    plt.grid(linestyle="--")

    patterns =("++", "xx", "//","..")
    colors = ["royalblue", "orange", "green", "Red"]
    hatches = [p for p in patterns for i in range(len(df))]
    edge_colors = [c for c in colors for i in range(len(df))]
    bars = ax.patches

    idx = 0
    for bar, hatch in zip(bars, hatches):
        bar.set_hatch(hatch)
        bar.set_edgecolor(edge_colors[idx])
        idx += 1

    if islog:
        plt.yscale("log")

    plt.ylim(10)

    # plt.legend(loc="upper right", bbox_to_anchor=(1.05, 1.15), ncol=4)
    plt.legend(loc="upper right", bbox_to_anchor=(1.02, 1.2), ncol=4, prop={'size': 36}, columnspacing=1.0, handletextpad=0.5)


    plt.savefig("{}.png".format(figname), bbox_inches="tight")
    plt.savefig("{}.pdf".format(figname), bbox_inches="tight")

    plt.clf()
