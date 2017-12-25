import sys

# Ensure that the program can output data directly to the socket without buffering
sys.stderr.write("Started unbuffered_io example\n")

interactions = 0
while True:
    line = sys.stdin.readline().strip()
    try:
        num = int(line)
    except ValueError:
        num = 0
    sys.stdout.write(f"{num + 1}\n")

    interactions += 1
    if num >= 10:
        sys.stderr.write(f"unbuffered_io finished after {interactions} interactions\n")
        break
