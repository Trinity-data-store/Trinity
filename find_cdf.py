#!/usr/bin/python

import numpy as np
import sys
import csv

def findcdf(data):
  cdf = []
  for i in range(0,101):
    cdf.append(np.percentile(data,int(i)))
  return cdf

def print_usage():
    print "Usage: %s input-file"
    print "Expects input-file to contain a single column of numeric values"
    print "Outputs a file <input-file>.cdf that contains the cdf"


if len(sys.argv) != 2:
    print_usage()
    sys.exit(1)

data_file = sys.argv[1]
data = np.array([float(line.rstrip('\n')) for line in open(data_file)])
cdf = findcdf(data)

out_file = data_file + '.csv'
out = open(out_file, 'w')
for xy in zip(cdf, range(0,101)):
	out.write('%lf,%d\n' % xy)
out.close()
