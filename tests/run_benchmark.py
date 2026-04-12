#!/usr/bin/env python3
"""
压测运行脚本：对指定的用例目录多次调用编译好的 bot 可执行文件，统计延迟与吞吐。
用法示例：
  python tests/run_benchmark.py --cases tests/cases --bin my_bot.exe --repeat 1000 --workers 4
"""
import argparse
import glob
import json
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor, as_completed


def load_cases(cases_dir):
    files = sorted(glob.glob(os.path.join(cases_dir, "*.json")))
    if not files:
        raise SystemExit(f"未找到用例文件：{cases_dir}")
    texts = []
    for f in files:
        with open(f, 'r', encoding='utf-8') as fh:
            texts.append(fh.read())
    return texts


def run_once(bin_path, json_text, timeout):
    t0 = time.perf_counter()
    try:
        p = subprocess.run([bin_path], input=json_text.encode('utf-8'), stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout)
        rc = p.returncode
    except Exception as e:
        return time.perf_counter() - t0, -1, str(e)
    return time.perf_counter() - t0, rc, None


def run_sequential(bin_path, cases, repeat, timeout):
    durations = []
    failures = 0
    start_wall = time.perf_counter()
    for i in range(repeat):
        text = cases[i % len(cases)]
        d, rc, err = run_once(bin_path, text, timeout)
        durations.append(d)
        if rc != 0:
            failures += 1
    wall = time.perf_counter() - start_wall
    return durations, failures, wall


def run_parallel(bin_path, cases, repeat, workers, timeout):
    durations = []
    failures = 0
    start_wall = time.perf_counter()
    with ThreadPoolExecutor(max_workers=workers) as ex:
        futures = [ex.submit(run_once, bin_path, cases[i % len(cases)], timeout) for i in range(repeat)]
        for f in as_completed(futures):
            d, rc, err = f.result()
            durations.append(d)
            if rc != 0:
                failures += 1
    wall = time.perf_counter() - start_wall
    return durations, failures, wall


def summarize(durations, failures, wall):
    total = len(durations)
    ok = total - failures
    avg = sum(durations) / total if total else 0
    med = sorted(durations)[total//2] if total else 0
    qps = total / wall if wall > 0 else 0
    return {
        'total': total,
        'ok': ok,
        'failures': failures,
        'avg_s': avg,
        'median_s': med,
        'wall_s': wall,
        'qps': qps
    }


def main():
    parser = argparse.ArgumentParser(description='运行 bot 压测')
    parser.add_argument('--cases', type=str, default='tests/cases', help='用例目录（json 文件）')
    parser.add_argument('--bin', type=str, default='my_bot.exe', help='可执行文件路径')
    parser.add_argument('--repeat', type=int, default=1000, help='总请求次数')
    parser.add_argument('--workers', type=int, default=1, help='并发 worker 数，1 表示串行')
    parser.add_argument('--timeout', type=float, default=5.0, help='每次调用超时（秒）')
    parser.add_argument('--out', type=str, default='tests/results.json', help='输出统计文件（可选）')
    args = parser.parse_args()

    cases = load_cases(args.cases)
    bin_path = args.bin

    if args.workers <= 1:
        durations, failures, wall = run_sequential(bin_path, cases, args.repeat, args.timeout)
    else:
        durations, failures, wall = run_parallel(bin_path, cases, args.repeat, args.workers, args.timeout)

    stats = summarize(durations, failures, wall)
    print(json.dumps(stats, ensure_ascii=False, indent=4))
    # 写入文件
    try:
        os.makedirs(os.path.dirname(args.out), exist_ok=True)
        with open(args.out, 'w', encoding='utf-8') as fh:
            json.dump({'stats': stats, 'durations_s': durations[:1000]}, fh, ensure_ascii=False, indent=2)
        print(f"结果已写入 {args.out}")
    except Exception:
        pass


if __name__ == '__main__':
    main()
