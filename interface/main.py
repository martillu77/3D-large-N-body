import subprocess
import sys


ENGINE = "build/large-N-body"


def start_engine():
    try:
        return subprocess.Popen( [ENGINE], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1 )
    except FileNotFoundError:
        print(f"Engine not found: {ENGINE}")
        sys.exit(1)


def send_command(engine, command):
    engine.stdin.write(command + "\n")
    engine.stdin.flush()

    response = []

    while True:
        line = engine.stdout.readline()

        if not line:
            break

        line = line.rstrip("\n")

        if line == "END":
            break

        response.append(line)

    return response


def main():
    engine = start_engine()

    # Read engine startup response.
    startup = send_command(engine, "")

    for line in startup:
        print(line)

    print("Type 'help' for commands.")

    while True:
        try:
            command = input("engine> ").strip().lower()
        except (EOFError, KeyboardInterrupt):
            print()
            command = "quit"

        if not command:
            continue

        response = send_command(engine, command)

        for line in response:
            print(line)

        if command in ("quit", "exit"):
            break

    engine.wait()


if __name__ == "__main__":
    main()