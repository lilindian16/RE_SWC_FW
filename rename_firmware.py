import os
import subprocess
Import("env")

prog_name = env["PIOENV"]
print(f'Current name: {prog_name}')
version_file_path = os.path.join(env.get("PROJECT_DIR"), "version.txt")
if os.path.exists(version_file_path):
    with open(version_file_path, "r") as f:
        version = f.read().strip()
        print(f"Reading version from file: {version}")
        prog_name += f'_v{version}'
        # Define your desired program name
        env.Replace(PROGNAME=prog_name)
else:
    print("Version file not found.")
