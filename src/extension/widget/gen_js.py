#!/usr/bin/env python
import sys

TEMPLATE = """\
extern const char %s[];
const char %s[] = { %s, 0 };
"""

if __name__ == '__main__':
    if len(sys.argv) < 4 :
        sys.exit(-1);
    src = sys.argv[1]
    symbol = sys.argv[2]
    dest = sys.argv[3]
    lines = open(src, 'r').read()
    c_code = ', '.join(str(ord(c)) for c in lines)
    output = open(dest, "w")
    output.write(TEMPLATE %(symbol, symbol, c_code))
    output.close()  
