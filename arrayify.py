def arrayify(fname, aname):
    try:
        with open(fname, "rb") as f:
            data = f.read()
        code  = f"const unsigned char {aname}[] = {{\n"
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
        code += f"const unsigned int {aname}_size = {len(data)};\n"
        sys.stdout.write(code)

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 3:
        print("Usage: arrayify.py <binary file> <array name>")
        sys.exit(1)

    arrayify(sys.argv[1], sys.argv[2])
