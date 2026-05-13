#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import sys
import time
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

POST_URL = (
    "https://blog.naver.com/PostView.naver?blogId=jgwkrrk&logNo=120184894036"
    "&redirect=Dlog&widgetTypeCall=true&photoView=0&noTrackingCode=true&directAccess=false"
)
PAGE_REFERER = "https://blog.naver.com/jgwkrrk/120184894036"
IMAGE_HOST = "https://postfiles.pstatic.net"
IMAGE_TYPE = "w3840"
USER_AGENT = (
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
    "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
)

PATH_RE = re.compile(
    r"\\?/(20130316_\d+)\\?/(jgwkrrk_[A-Za-z0-9]+_PNG)\\?/(\d+)\.PNG"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Download 151 dot sprite images from a Naver blog photo album."
    )
    parser.add_argument("--start", type=int, default=1)
    parser.add_argument("--end", type=int, default=151)
    parser.add_argument("--sleep", type=float, default=0.3)
    parser.add_argument(
        "--output-dir", type=Path, default=Path(__file__).parent / "dot_input"
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def fetch_page() -> str:
    req = Request(POST_URL, headers={"User-Agent": USER_AGENT})
    with urlopen(req, timeout=30) as resp:
        return resp.read().decode("utf-8", errors="replace")


def extract_image_paths(html: str) -> dict[int, str]:
    marker = "aPostImageFileSizeInfo[1]"
    idx = html.find(marker)
    if idx < 0:
        raise SystemExit(f"could not find {marker} in page")
    end = html.find(";", idx)
    region = html[idx : end if end > idx else len(html)]

    paths: dict[int, str] = {}
    for match in PATH_RE.finditer(region):
        date_dir, token, num_str = match.group(1), match.group(2), match.group(3)
        num = int(num_str)
        if num in paths:
            continue
        paths[num] = f"/{date_dir}/{token}/{num}.PNG"
    return paths


def build_image_url(path: str) -> str:
    return f"{IMAGE_HOST}{path}?type={IMAGE_TYPE}"


def download_file(url: str, output_path: Path) -> bool:
    req = Request(
        url,
        headers={"User-Agent": USER_AGENT, "Referer": PAGE_REFERER},
    )
    try:
        with urlopen(req, timeout=30) as resp:
            data = resp.read()
    except (HTTPError, URLError, TimeoutError) as exc:
        print(f"[FAIL] {url} -> {exc}")
        return False

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(data)
    print(f"[OK]   {output_path.name} ({len(data)} bytes)")
    return True


def main() -> None:
    args = parse_args()
    if args.start > args.end:
        raise SystemExit("start must be <= end")

    print(f"Fetching post page: {POST_URL}")
    html = fetch_page()
    paths = extract_image_paths(html)
    print(f"Parsed {len(paths)} image paths from post page")

    missing = [n for n in range(args.start, args.end + 1) if n not in paths]
    if missing:
        print(f"[WARN] missing paths for: {missing}", file=sys.stderr)

    args.output_dir.mkdir(parents=True, exist_ok=True)

    ok = 0
    fail = 0
    for num in range(args.start, args.end + 1):
        if num not in paths:
            print(f"[SKIP] no path found for {num}")
            fail += 1
            continue

        url = build_image_url(paths[num])
        output_path = args.output_dir / f"{num:03d}.png"

        if args.dry_run:
            print(f"{output_path.name} <- {url}")
            continue

        if download_file(url, output_path):
            ok += 1
        else:
            fail += 1
        time.sleep(args.sleep)

    if not args.dry_run:
        print(f"done: ok={ok}, fail={fail}, dir={args.output_dir}")


if __name__ == "__main__":
    main()
