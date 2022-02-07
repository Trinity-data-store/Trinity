import argparse
import re

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FormatStrFormatter, LogLocator, NullFormatter

mpl.rcParams['ps.useafm'] = True
mpl.rcParams['pdf.use14corefonts'] = True
mpl.rcParams['text.usetex'] = True
mpl.rcParams["legend.framealpha"] = 1.0
mpl.rcParams.update({'font.size': 16, 'text.latex.preamble': [r'\usepackage{amsmath}']})

color_map = {"Insertion": "green", "Lookup": "indigo", "Range Query": "red", "Storage": "blue"}
hatch_map = {"Insertion": "+++", "Lookup": "xxxx", "Range Query": "////", "Storage": "...."}
# color_map = {"\\textsc{MdTrie}": "green", "PH-Tree": "indigo", "R*-Tree": "blue", "Lookup": "blue",
#              "Insertion": "red"}
# hatch_map = {"\\textsc{MdTrie}": "+++", "PH-Tree": "xxxx", "R*-Tree": "////", "Lookup": "////",
#              "Insertion": "...."}

# color_map = {"\\textsc{Trinity}": "blue", "Aerospike": "red", "MongoDB": "indigo", "Lookup": "blue",
#              "Insertion": "red"}
# hatch_map = {"\\textsc{Trinity}": "+++", "Aerospike": "xxxx", "MongoDB  ": "////", "Lookup": "////",
#              "Insertion": "...."}

# color_map = {"Baseline": "green", "+CN": "indigo", "+GM": "red", "+SM": "blue"}
# hatch_map = {"Baseline": "+++", "+CN": "xxxx", "+GM": "////", "+SM": "...."}

def validate_data(data, ncols):
    for row in data:
        assert len(row) == ncols


def split(line):
    return [token.strip('"') for token in re.findall(r'(?:"[^"]*"|[^\s"])+', line)]


def plot(**kwargs):
    input_file = kwargs.get('data')
    output_file = kwargs.get('out')
    xlabel = kwargs.get('xlabel', None)
    ylabel = kwargs.get('ylabel', None)
    ymin = kwargs.get('ymin', None)
    ymax = kwargs.get('ymax', None)

    inp = open(input_file, 'r')

    # Get the bar labels
    bar_labels = split(inp.readline())
    num_cols = len(bar_labels)

    # Read the remainder of the file
    rows = [split(line) for line in inp]
    num_rows = len(rows)
    bar_width = 1. / (num_cols + 1)

    # Get the data to plot, and validate it
    heights = [[float(cell) for cell in row[1:]] for row in rows]
    validate_data(heights, num_cols)
    heights = list(map(list, zip(*heights)))

    xticks = [row[0] for row in rows]
    xticks_x_shift = ((num_cols - 1.0) / 2.0) * bar_width
    xticks_x = [r + xticks_x_shift for r in range(num_rows)]

    fig = plt.gcf()
    fig.set_size_inches(4, 3.2)
    fig.subplots_adjust(left=0.26, top=0.85, right=0.96, bottom=0.20)

    prv_x = None
    i = 0
    for cur_height in heights:
        if prv_x is None:
            cur_x = np.arange(len(cur_height))
        else:
            cur_x = [x + bar_width for x in prv_x]
        plt.bar(x=cur_x, height=cur_height, color='none', hatch=hatch_map[bar_labels[i]],
                width=bar_width, edgecolor=color_map[bar_labels[i]], label=bar_labels[i], zorder=3)

        i += 1
        prv_x = cur_x

    if xlabel is not None:
        if isinstance(xlabel, str):
            print("is string: ", xlabel)
        plt.xlabel(xlabel)
    plt.xticks(xticks_x, xticks)
    if ylabel is not None:
        plt.ylabel(ylabel)
    plt.grid(which='major', axis='y', linestyle='--', zorder=0)

    if not kwargs.get('nolegend', False):
        legend_pos = kwargs.get('legendpos', 'upper-left')
        if legend_pos == 'upper-left':
            plt.legend(loc='upper left', bbox_to_anchor=(0.25, 1.25), fontsize='small')
        elif legend_pos == 'upper-right':
            plt.legend(loc='upper right', bbox_to_anchor=(0.75, 1.25), fontsize='small')
        elif legend_pos == 'upper-center':
            plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), fontsize='small')
        else:
            raise RuntimeError('Unknown legend pos {}'.format(legend_pos))

    if kwargs.get('logscale', False):
        plt.yscale('log')
    ax = plt.gca()
    ax.yaxis.set_major_formatter(FormatStrFormatter('%g'))

    if kwargs.get('logscale', False):
        locmaj = LogLocator(base=10, numticks=100)
        locmin = LogLocator(base=10, subs=(0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9), numticks=100)
        ax.yaxis.set_major_locator(locmaj)
        ax.yaxis.set_minor_locator(locmin)
        ax.yaxis.set_minor_formatter(NullFormatter())

    if ymin is not None and ymax is not None:
        plt.ylim([ymin, ymax])

    plt.savefig(output_file)


def main():
    parser = argparse.ArgumentParser(description='Generates a TiKZ bar plot from an input file.')
    parser.add_argument('-d', '--data', type=str, metavar='DATA_FILE', required=True, help='The input data file.')
    parser.add_argument('-o', '--out', type=str, metavar='OUTPUT_FILE', required=True, help='The output TiKZ file.')
    parser.add_argument('--rotate', action='store_true', help='Rotate bar chart.')
    parser.add_argument('--ymin', type=float, help='Lower limit to y-axis.')
    parser.add_argument('--ymax', type=float, help='Upper limit to y-axis.')
    parser.add_argument('--ylabel', type=str, help='Label for y-axis.')
    parser.add_argument('--xlabel', type=str, help='Label for x-axis.')
    parser.add_argument('--xscale', type=float, help='Scale for x-axis.')
    parser.add_argument('--yscale', type=float, help='Scale for y-axis.')
    parser.add_argument('--logscale', '-l', action='store_true', help='Set logscale for y-axis')
    parser.add_argument('--nolegend', action='store_true', help='Don\'t generate a legend.')
    parser.add_argument('--legendpos', type=str, default='upper-left', help='Position of legend')
    parser.add_argument('--colors', type=str, help='Comma-separated list of colors')
    parser.add_argument('--patterns', type=str, help='Comma-separated list of patterns')
    args = parser.parse_args()
    plot(**vars(args))


if __name__ == "__main__":
    main()
