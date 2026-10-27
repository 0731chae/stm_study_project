"""Run host checks and preserve commands, source fingerprints and actual output."""
import argparse
from datetime import datetime, timezone
import hashlib
from pathlib import Path
import platform
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--arm-compiler", help="Also compile core Cortex-M4 objects")
    args = parser.parse_args()
    report = ["Scope: PC common logic; ARM object compilation if requested.",
              "Hardware, HAL, firmware link and physical outputs: NOT TESTED.",
              f"UTC: {datetime.now(timezone.utc).isoformat()}",
              f"Platform: {platform.platform()}", "Source SHA-256:"]
    sources = list((ROOT / "firmware").rglob("*.c"))
    sources += list((ROOT / "firmware").rglob("*.h"))
    sources += list((ROOT / "sim").rglob("*.c"))
    sources += list((ROOT / "tests").glob("*.c"))
    sources += [ROOT / "CMakeLists.txt", ROOT / "CMakePresets.json",
                ROOT / "tools" / "verify.py", ROOT / "tools" / "check_arm.py"]
    for path in sorted(sources):
        report.append(f"{path.relative_to(ROOT).as_posix()} {hashlib.sha256(path.read_bytes()).hexdigest()}")
    commands = [["cmake", "--version"], ["cmake", "--preset", "host"],
                ["cmake", "--build", "--preset", "host"], ["ctest", "--preset", "host"]]
    demo = ROOT / "build" / "host" / ("fire_demo.exe" if sys.platform == "win32" else "fire_demo")
    commands.append([str(demo)])
    if args.arm_compiler:
        commands.append([sys.executable, "tools/check_arm.py", "--compiler", args.arm_compiler])
    exit_code = 0
    try:
        for command in commands:
            report.append("\n$ " + " ".join(command))
            result = subprocess.run(command, cwd=ROOT, text=True, encoding="utf-8",
                                    errors="replace", stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            report.extend([result.stdout, f"Exit: {result.returncode}"])
            print(result.stdout, end="", flush=True)
            if result.returncode:
                exit_code = result.returncode
                break
    except OSError as error:
        report.append(str(error))
        exit_code = 1
    report.append(f"\nOverall process exit: {exit_code}")
    destination = ROOT / "build" / "host" / "verification.txt"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text("\n".join(report) + "\n", encoding="utf-8")
    print(f"Evidence: {destination}")
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
