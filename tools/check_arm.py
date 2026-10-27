"""Compile the portable core for Cortex-M4. This is not a firmware/link test."""
import argparse
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="arm-none-eabi-gcc")
    args = parser.parse_args()
    output = ROOT / "build" / "arm-check"
    output.mkdir(parents=True, exist_ok=True)
    subprocess.run([args.compiler, "--version"], check=True)
    for source in sorted((ROOT / "firmware" / "src").glob("*.c")):
        subprocess.run([
            args.compiler, "-std=c99", "-mcpu=cortex-m4", "-mthumb", "-ffreestanding",
            "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-O2",
            "-I", str(ROOT / "firmware" / "include"), "-c", str(source),
            "-o", str(output / (source.stem + ".o")),
        ], check=True)
        print(f"PASS: Cortex-M4 object {source.name}", flush=True)
    print("No firmware link, HAL driver, flashing, or board execution tested.", flush=True)


if __name__ == "__main__":
    main()
