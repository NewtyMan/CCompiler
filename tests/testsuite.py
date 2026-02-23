from argparse import ArgumentParser
from os import getcwd, listdir, path, walk
from shutil import copyfile
import subprocess
import pexpect


class TestCase:
    def __init__(self, name: str, flags: list[str], input_raw: list[str], input_file: str, output: list[str], error_code: str, is_system: bool):
        self.name = name
        self.flags = flags
        self.input_raw = input_raw
        self.input_file = input_file
        self.output = output
        self.error_code = error_code
        self.is_system = is_system
        pass

    def run(self):
        print(f"[*] RUN: {self.name}")
        input_file = self.input_file
        output_file = f"{getcwd()}/out/CCompiler/main.s"
        is_tmp_file = False
        if len(self.input_raw) > 0:
            input_file = f"{getcwd()}/out/CCompiler/temp.c"
            is_tmp_file = True
            with open(input_file, "w") as f:
                for line in self.input_raw:
                    f.write(f"{line}\n")
        else:
            copyfile(input_file, f"{getcwd()}/out/CCompiler/main.c")
            input_file = f"{getcwd()}/out/CCompiler/main.c"

        process = subprocess.Popen(
            [f"./CCompiler", input_file, output_file, *self.flags],
            cwd=f"{getcwd()}/out/CCompiler/",
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        stdout, stderr = process.communicate()

        if self.is_system:
            commands = [
                "arm-linux-gnueabi-as --warn --fatal-warnings -march=armv7-a -mcpu=cortex-a15 ./main.s -o ./main.o",
                "arm-linux-gnueabi-ld ./main.o -o ./main",
                "qemu-arm ./main"
            ]

            for command in commands:
                child = pexpect.spawn(command, cwd=f"{getcwd()}/out/CCompiler/", encoding="utf-8")
                child.expect(pexpect.EOF)
                child.close()
                if child.exitstatus != 0:
                    print("[!] Error while running system tests:", child.before)
                stdout = child.before

        return_code = 0
        if self.error_code == "" and process.returncode != 0:
            print(f"[!] Unexpected error code: {process.returncode}")
            exit(process.returncode)

        if self.error_code != "":
            if int(self.error_code) != process.returncode:
                print(f"[!] ERR: Expected error code {self.error_code}, but received {process.returncode}")
                return_code = 1
        elif "".join([x.rstrip() for x in stdout.split("\n")]) != "".join([x.rstrip() for x in self.output]):
            print("[!] ERR: Expected and actual outputs do not match")
            print("Expected:")
            print("\n".join(self.output))
            print("Actual:")
            print(stdout.strip())
            return_code = 1

        if return_code != 0:
            exit(return_code)

def load_test_files(input_folder: str, recursive: bool) -> list[str]:
    result = []
    if recursive:
        for root, dirs, files in walk(input_folder):
            for file in files:
                if file.endswith(".test"):
                    result.append(path.join(root, file))
    else:
        for file in listdir(input_folder):
            if file.endswith(".test"):
                result.append(path.join(input_folder, file))

    return result


def load_test_cases(input_file: str) -> list[TestCase]:
    result = []

    inside_test_case = 0
    inside_input = False
    inside_output = False
    case = None

    with open(input_file, "r") as f:
        for line in f.readlines():
            if line.startswith(":input:"):
                inside_input = not inside_input
                continue
            elif line.startswith(":output:"):
                inside_output = not inside_output
                continue

            if inside_input:
                case.input_raw.append(line.rstrip())
                continue

            if inside_output:
                case.output.append(line.rstrip())
                continue

            if line.startswith(":testcase:"):
                if inside_test_case == 0:
                    inside_test_case = 1
                    case = TestCase(name="", input_raw=[], input_file="", output=[], flags=[], error_code="", is_system=False)
                else:
                    inside_test_case = 0
                    result.append(case)

            if line.startswith(":name:"):
                case.name = line.removeprefix(":name:").strip()
            elif line.startswith(":input_file:"):
                case.input_file = line.removeprefix(":input_file:").strip()
            elif line.startswith(":flags:"):
                case.flags.extend(line.removeprefix(":flags:").strip().split(" "))
            elif line.startswith(":error_code:"):
                case.error_code = line.removeprefix(":error_code:").strip()
            elif line.startswith(":is_system:"):
                case.is_system = True

    return result


if __name__ == '__main__':
    parser = ArgumentParser()

    parser.add_argument("-i", "--input", required=True, help="input folder of tests")
    parser.add_argument("-r", "--recursive", action='store_true', default=False, help="recursively search for tests in input folder")

    args = parser.parse_args()

    tests_total = 0
    test_files = load_test_files(args.input, args.recursive)
    for file in test_files:
        print(f"[*] Opening test file: {file}...")
        test_cases = load_test_cases(file)
        for test_case in test_cases:
            test_case.run()
            tests_total += 1

    print(f"[*] Finished testing {tests_total} tests")
