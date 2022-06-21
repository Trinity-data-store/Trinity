import argparse
import re

import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter, LogLocator, NullFormatter

mpl.rcParams['ps.useafm'] = True
mpl.rcParams['pdf.use14corefonts'] = True
mpl.rcParams['text.usetex'] = True
mpl.rcParams["legend.framealpha"] = 1.0
mpl.rcParams.update({'font.size': 16, 'text.latex.preamble': [r'\usepackage{amsmath}']})

color_map = {"Insecure-Baseline": "green", "PathORAM": "indigo", "\\textsc{pancake}": "blue"}
hatch_map = {"Insecure-Baseline": "+++", "PathORAM": "xxxx", "\\textsc{pancake}": "////"}

markers = ['+', 'x', 'o', '^', 'v']


def pairwise(iterable):
    a = iter(iterable)
    return zip(a, a)


def validate_data(data, ncols):
    for row in data:
        assert len(row) == ncols


def split(line):
    return [token.strip('"') for token in re.findall(r'(?:"[^"]*"|[^\s"])+', line)]


def transpose(data):
    num_cols = len(data[0])
    out = [[] for _ in range(num_cols)]
    for i in range(len(data)):
        for j in range(len(data[i])):
            out[j].append(data[i][j])
    return out


def plot(**kwargs):
    input_file = kwargs.get('data')
    output_file = kwargs.get('out')
    xlabel = kwargs.get('xlabel', None)
    ylabel = kwargs.get('ylabel', None)
    ymin = kwargs.get('ymin', None)
    ymax = kwargs.get('ymax', None)
    xmin = kwargs.get('xmin', None)
    xmax = kwargs.get('xmax', None)
    xscale = kwargs.get('xscale', None)
    yscale = kwargs.get('yscale', None)
    legend = not kwargs.get('nolegend', False)
    marker = not kwargs.get('nomarker', False)

    inp = open(input_file, 'r')

    # Get the line labels
    if legend:
        line_labels = split(inp.readline())
        num_labels = len(line_labels)
    else:
        line_labels = None
        num_labels = 0

    # Read the remainder of the file
    rows = [[float(x) for x in split(line)] for line in inp]
    cols = transpose(rows)
    num_cols = len(cols)

    if legend:
        print('num_cols={}, num_labels={}'.format(num_cols, num_labels))
        assert num_cols == num_labels * 2

    fig = plt.gcf()
    fig.set_size_inches(5, 3)
    fig.subplots_adjust(left=0.26, top=0.85, right=0.96, bottom=0.20)

    plt.grid(which='major', axis='y', linestyle='--', zorder=0)

    i = 0
    for x, y in pairwise(cols):
        if xscale is not None:
            x = [xi * xscale for xi in x]
        if yscale is not None:
            y = [yi * yscale for yi in y]
        extra_args = {}
        if legend:
            extra_args['label'] = line_labels[i]
        if marker:
            extra_args['marker'] = markers[i]
        plt.plot(x, y, zorder=3, **extra_args)
        i += 1

    if xlabel is not None:
        plt.xlabel('\n'.join(xlabel))
    if ylabel is not None:
        plt.ylabel('\n'.join(ylabel))
    plt.grid(which='major', axis='both', linestyle='--', zorder=0)

    if legend:
        legend_pos = kwargs.get('legendpos', 'upper-left')
        if legend_pos == 'upper-left':
            plt.legend(loc='upper left', bbox_to_anchor=(0.25, 1.25), fontsize='small')
        elif legend_pos == 'upper-right':
            plt.legend(loc='upper right', bbox_to_anchor=(0.75, 1.25), fontsize='small')
        elif legend_pos == 'upper-center':
            plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), fontsize='small')
        else:
            raise RuntimeError('Unknown legend pos {}'.format(legend_pos))

    if kwargs.get('logy', False):
        plt.yscale('log')

    if kwargs.get('logx', False):
        plt.xscale('log')

    if kwargs.get('log2y', False):
        plt.yscale('log', basey=2)

    if kwargs.get('log2x', False):
        plt.xscale('log', basex=2)

    if ymin is not None and ymax is not None:
        plt.ylim([ymin, ymax])

    if xmin is not None and xmax is not None:
        plt.xlim([xmin, xmax])

    ax = plt.gca()
    ax.yaxis.set_major_formatter(FormatStrFormatter('%g'))

    if kwargs.get('logy', False):
        locmaj = LogLocator(base=10, numticks=100)
        locmin = LogLocator(base=10, subs=(0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9), numticks=100)
        ax.yaxis.set_major_locator(locmaj)
        ax.yaxis.set_minor_locator(locmin)
        ax.yaxis.set_minor_formatter(NullFormatter())

    if kwargs.get('logx', False):
        locmaj = LogLocator(base=10, numticks=100)
        locmin = LogLocator(base=10, subs=(0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9), numticks=100)
        ax.xaxis.set_major_locator(locmaj)
        ax.xaxis.set_minor_locator(locmin)
        ax.xaxis.set_minor_formatter(NullFormatter())

    if kwargs.get('log2y', False):
        locmaj = LogLocator(base=2, numticks=8)
        locmin = LogLocator(base=2, subs=(0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9), numticks=8)
        ax.yaxis.set_major_locator(locmaj)
        ax.yaxis.set_minor_locator(locmin)
        ax.yaxis.set_minor_formatter(NullFormatter())

    if kwargs.get('log2x', False):
        locmaj = LogLocator(base=2, numticks=8)
        locmin = LogLocator(base=4, subs=(0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9), numticks=16)
        ax.xaxis.set_major_locator(locmaj)
        ax.xaxis.set_minor_locator(locmin)
        ax.xaxis.set_minor_formatter(NullFormatter())

    # plt.locator_params(axis='y', numticks=10)
    # plt.locator_params(axis='x', numticks=8)

    plt.savefig(output_file)


def main():
    parser = argparse.ArgumentParser(description='Generates a TiKZ bar plot from an input file.')
    parser.add_argument('-d', '--data', type=str, metavar='DATA_FILE', required=True, help='The input data file.')
    parser.add_argument('-o', '--out', type=str, metavar='OUTPUT_FILE', required=True, help='The output TiKZ file.')
    parser.add_argument('--rotate', action='store_true', help='Rotate bar chart.')
    parser.add_argument('--ymin', type=float, help='Lower limit to y-axis.')
    parser.add_argument('--ymax', type=float, help='Upper limit to y-axis.')
    parser.add_argument('--xmin', type=float, help='Lower limit to x-axis.')
    parser.add_argument('--xmax', type=float, help='Upper limit to x-axis.')
    parser.add_argument('--ylabel', nargs='+', type=str, help='Label for y-axis.')
    parser.add_argument('--xlabel', nargs='+', type=str, help='Label for x-axis.')
    parser.add_argument('--xscale', type=float, help='Scale for x-axis.')
    parser.add_argument('--yscale', type=float, help='Scale for y-axis.')
    parser.add_argument('--logy', action='store_true', help='Set logscale for y-axis')
    parser.add_argument('--logx', action='store_true', help='Set logscale for x-axis')
    parser.add_argument('--log2x', action='store_true', help='Set logscale for x-axis (base 2)')
    parser.add_argument('--log2y', action='store_true', help='Set logscale for x-axis (base 2)')
    parser.add_argument('--nomarker', action='store_true', help='Don\'t generate markers.')
    parser.add_argument('--nolegend', action='store_true', help='Don\'t generate a legend.')
    parser.add_argument('--legendpos', type=str, default='upper-left', help='Position of legend')
    parser.add_argument('--colors', type=str, help='Comma-separated list of colors')
    parser.add_argument('--patterns', type=str, help='Comma-separated list of patterns')
    args = parser.parse_args()
    plot(**vars(args))


if __name__ == "__main__":
    main()
