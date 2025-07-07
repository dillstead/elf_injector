import sys
import os

def arrayify(fname, aname, max_len):
    try:
        if os.path.getsize(fname) > max_len:
            sys.stderr.write("Error: File too large\n")
            sys.exit(1)
        with open(fname, "rb") as f:
            data = f.read()
        code  = f"static u8 {aname}[] = \n{{\n"
        line = ""
        for i, byte in enumerate(data):
            line += f"0x{byte:02x}, "
            # Break the line every 16 bytes for readability
            if (i + 1) % 8 == 0:
                code += f"    {line}\n"
                line = ""
        if line:
            code += f"    {line.rstrip(', ')}\n"
        code += f"}};\n"
        code += f"static size {aname}_len = {len(data)};\n"
        sys.stdout.write(code)

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 4:
        print("Usage: arrayify.py <binary file> <array name> <max len>")
        sys.exit(1)

    arrayify(sys.argv[1], sys.argv[2], int(sys.argv[3]))
