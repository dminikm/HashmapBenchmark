import sys

with open(sys.argv[1], "r", encoding="utf8") as input:
    output = open(sys.argv[1] + ".out", "wb")

    for line in input:
        data = line[11:]

        try:
            ascii_data = data.encode("ascii")
            output.write(ascii_data)
        except UnicodeEncodeError:
            pass

    output.close()