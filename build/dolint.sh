#!/bin/bash

echo "############################# CPP LINT ##################################"

files=`find $1 -name "*.c" -or -name "*.cc" -or -name "*.h"`

ret=0
for x in $files; do
	$(dirname $0)/cpplint.py \
		--filter=-build/header_guard \
		$x
	if [ $? -ne 0 ]; then
		ret=1
	fi
done;

echo "############################# LINT DONE ##################################"

exit $ret

