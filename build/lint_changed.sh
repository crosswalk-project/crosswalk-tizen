#!/bin/bash

echo "############################# CPP LINT for changed files ##################################"

files=`git status -s | awk 'BEGIN{FILTER="\\.(cc|h)$"} tolower($2) ~ FILTER { print $2  }'`
exclude_files="picojson.h"
ret=0
for x in $files; do
  if [[ $exclude_files =~ $(basename $x) ]]; then
    continue
  fi

    $(dirname $0)/cpplint.py \
        --filter=-build/header_guard \
        $x
    if [ $? -ne 0 ]; then
        ret=1
    fi
done;

echo "############################# LINT DONE ##################################"

exit $ret
