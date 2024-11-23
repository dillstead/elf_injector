def esc(line):
    escapes = {
        '\\' : '\\\\',
        '"' : '\\"',
        "'" : "\\'"
        
        }
    return ''.join(escapes.get(c, c) for c in line)

def asmify(fname):
    try:
        with open(fname, "r") as fin:
            print("__attribute((naked))")
            print("void _start(void)")
            print("{")
            print("asm (")
            for line in fin:
                print(f"\"{esc(line.strip())}\\n\"")
            print(");")
            print("};")
    except Exception as e:
        print(f"Error: {e}")
    
if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        print("Usage: asmify.py <filename>")
        sys.exit(1)

    asmify(sys.argv[1])
