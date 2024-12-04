#!/bin/sh
#
# This script demonstrates how to read csv file and use stdin to feed SQL cmd.
#
# Read section 8.5 and section 32 of the following link
#
# - https://www.sqlite.org/cli.html
#
echo 'select sum(count) from tbl;' | sqlite3 -cmd '.import --csv data.csv tbl'
