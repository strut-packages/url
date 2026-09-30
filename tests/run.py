#!/usr/bin/env python3
import argparse
import json
import os
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "tests" / "package_test.p").read_text().replace("__LARGE_INPUT__", "a" * 10000)
EXPECTED = (
    "https\nuser:pass@example.com:8443\nuser:pass\nexample.com\n8443\n/a/b\nq=1\ntop\n"
    "https://user:pass@example.com:8443/a/b?q=1#top\n2001:db8::1\n1\n8080\n192.0.2.1\n"
    "1\n1\n../other\n1\n1\nhttp://a/b/c/g\nhttp://a/b/g\nhttp://a/b/c/d;p?y\n"
    "http://a/b/c/d;p?q#s\nhttp://a/b/c/g/\na%20b%2F%2B\na b/+\n3\n5\n"
    "a\n1\nb\n\nflag\nabsent\nplus\na+b\na\n2\na=1&b=&flag&plus=a%2Bb&a=2\n4\n2\n4\n4\n"
    + ("1\n" * 39)
    + "10000\n"
)


def run(command, cwd, env):
    return subprocess.run(command, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--strut", default=os.environ.get("STRUT_BIN", "strut"))
    args = parser.parse_args()
    compiler = str(Path(args.strut).resolve()) if Path(args.strut).exists() else args.strut

    with tempfile.TemporaryDirectory(prefix="strut-url-test-") as temporary:
        root = Path(temporary)
        app = root / "app"
        app.mkdir()
        (app / "main.p").write_text(SOURCE)
        (app / "strut.json").write_text(json.dumps({
            "name": "url-package-test",
            "version": "0.1.0",
            "entry": "main.p",
            "dependencies": {},
        }) + "\n")
        env = os.environ.copy()
        env["STRUT_HOME"] = str(root / "strut-home")

        for command in (
            [compiler, "init"],
            [compiler, "add", str(ROOT)],
            [compiler, "install"],
            [compiler, "install", "--offline"],
            [compiler, "packages", "--json"],
        ):
            result = run(command, app, env)
            if result.returncode:
                print(result.stdout, end="")
                print(result.stderr, end="", file=os.sys.stderr)
                return result.returncode

        output = app / ("url-test.exe" if os.name == "nt" else "url-test")
        result = run([compiler, "main.p", "-o", str(output)], app, env)
        if result.returncode:
            print(result.stdout, end="")
            print(result.stderr, end="", file=os.sys.stderr)
            return result.returncode
        result = run([str(output)], app, env)
        if result.returncode or result.stdout != EXPECTED:
            print("unexpected result", file=os.sys.stderr)
            print("stdout:", repr(result.stdout), file=os.sys.stderr)
            print("stderr:", repr(result.stderr), file=os.sys.stderr)
            return 1

    print("url package test: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
