#!/usr/bin/env python3
import sys
import re

def dump_offs(base, sname):
    start = None
    
    for line in sys.stdin:
        line = line.strip()
        
        # match function labels: address <function_name>:
        #   000100b8 <syscall1>:
        match = re.match(r'^([0-9a-f]+)\s+<([^>]+)>:$', line)
        if match:
            addr = int(match.group(1), 16)
            fname = match.group(2)
            
            if fname == sname:
                start = addr
                break
    
    if start is None:
        print(f"Error: function '{sname}' not found")
        sys.exit(1)
    

    sys.stdin.seek(0)
    for line in sys.stdin:
        line = line.strip()
        
        match = re.match(r'^([0-9a-f]+)\s+<([^>]+)>:$', line)
        if match:
            addr = int(match.group(1), 16)
            fname = match.group(2)
            
            res = addr - start + base
            print(f"{fname}")
            print(f"b *0x{res:x}")
    
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: parse_objdump.py <hex base> <function_name>")
        sys.exit(1)
    try:
        base = int(sys.argv[1], 16)
    except ValueError:
        print("Error: invalid base")
        sys.exit(1)
    
    dump_offs(base, sys.argv[2])
