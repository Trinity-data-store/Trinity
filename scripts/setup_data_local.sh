#!/bin/bash

data_dir=/mntData2
local_dir=/mntData

sudo chmod 775 $local_dir
cp -a $data_dir/dataset_csv/. $local_dir/