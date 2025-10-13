#!/bin/sh
#
# This script demonstrates how to read csv file and use stdin or cmd arg to
# feed SQL cmd.
#
# - The 1st one demos how to read csv file into memory and query from stdin.
# - The 2nd one demos how to read csv file into database and query from cmd arg.
#   In this case, a file must be provided.
# - The 3rd one demos how to read csv file into memory with special separator
#   with dedicated flags.
# - The 4th one demos how to read csv file into memory with special separator
#   and multiple -cmd flags.
#
# Referecnes:
# - Read section 8.5 and section 32 of the following link
#   - https://www.sqlite.org/cli.html
#
t=$(mktemp) || exit
echo 'select sum(count) from tbl;' | sqlite3 -cmd '.import --csv data.csv tbl'
sqlite3 -cmd '.import --csv data.csv tbl' "$t" 'select sum(count) from tbl;'
echo 'select sum(count) from tbl;' | sqlite3 -csv -separator "|" -cmd '.import data_sep.csv tbl'
echo 'select sum(count) from tbl;' | sqlite3 -cmd '.mode csv' -cmd '.separator "|"' -cmd '.import data_sep.csv tbl'
rm -f -- "$t"
