#!/bin/bash

echo "############################# CPP LINT ##################################"

files=`find $1 -name "*.c" -or -name "*.cc" -or -name "*.h"`

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

