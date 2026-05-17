#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

BASE_URL = "https://static.namu.wiki/files/icon{num}_f00_s0.png"
DEFAULT_START = 1
DEFAULT_END = 151


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Download Pokemon sprite images by icon number pattern.")
    parser.add_argument("--start", type=int, default=DEFAULT_START)
    parser.add_argument("--end", type=int, default=DEFAULT_END)
    parser.add_argument("--sleep", type=float, default=0.2, help="Delay between requests in seconds")
    parser.add_argument("--output-dir", type=Path, default=Path(__file__).parent / "bulk_input")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def build_url(num: int) -> str:
    return BASE_URL.format(num=f"{num:04d}")


def download_file(url: str, output_path: Path) -> bool:
    req = Request(url, headers={"User-Agent": "Mozilla/5.0"})
    try:
        with urlopen(req, timeout=20) as resp:
            data = resp.read()
    except (HTTPError, URLError, TimeoutError) as exc:
        print(f"[FAIL] {url} -> {exc}")
        return False

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(data)
    print(f"[OK]   {output_path.name}")
    return True


def main() -> None:
    args = parse_args()
    if args.start > args.end:
        raise SystemExit("start must be <= end")

    args.output_dir.mkdir(parents=True, exist_ok=True)

    ok = 0
    fail = 0
    for num in range(args.start, args.end + 1):
        filename = f"icon{num:04d}_f00_s0.png"
        url = build_url(num)
        output_path = args.output_dir / filename

        if args.dry_run:
            print(url)
        else:
            if download_file(url, output_path):
                ok += 1
            else:
                fail += 1
            time.sleep(args.sleep)

    if not args.dry_run:
        print(f"done: ok={ok}, fail={fail}, dir={args.output_dir}")


if __name__ == "__main__":
    main()
