import os
import subprocess
Import("env")

version_file_path = os.path.join(env.get("PROJECT_DIR"), "version.txt")

version_header_filepath = os.path.join(
    env.get("PROJECT_DIR"), "include/version.h")

prog_name = env["PIOENV"]
print(f'Current name: {prog_name}')


def get_version_header_file_content(version_number):
    return f'#pragma once\n#define FIRMWARE_VERSION "{version_number}"'


if os.path.exists(version_file_path):
    with open(version_file_path, "r") as f:
        version = f.read().strip()
        print(f"Reading version from file: {version}")
        prog_name = "RE_SWC_"
        prog_name += f'v{version}'
        header_file_content = get_version_header_file_content(version)
        print(header_file_content)
        # Define your desired program name
        env.Replace(PROGNAME=prog_name)
    if os.path.exists(version_header_filepath):
        with open(version_header_filepath, 'w+') as f:
            f.write(str(header_file_content))
    else:
        print(f"Found version file {version_file_path}")
        print(f"Unable to find version header file {version_header_filepath}")

else:
    print("Version file (version.txt) not found.")
