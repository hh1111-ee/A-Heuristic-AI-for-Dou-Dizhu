#!/usr/bin/env python3
"""
随机生成用于压测的请求 JSON 文件（斗地主 bot 输入格式）。
用法：
  python tests/generate_requests.py --count 100 --outdir tests/cases

生成的文件形如：
{
  "requests": [ { "history": [[], []], "publiccard": [...], "own": [...] } ],
  "responses": [],
  "data": ""
}
"""
import argparse
import json
import os
import random

RESERVE = {
    3: [0,1,2,3],
    4: [4,5,6,7],
    5: [8,9,10,11],
    6: [12,13,14,15],
    7: [16,17,18,19],
    8: [20,21,22,23],
    9: [24,25,26,27],
    10: [28,29,30,31],
    11: [32,33,34,35],
    12: [36,37,38,39],
    13: [40,41,42,43],
    14: [44,45,46,47],
    15: [48,49,50,51],
    16: [52],
    17: [53]
}

ALL_CARDS = list(range(54))


def pick_by_rank(deck, rank, count):
    avail = [c for c in RESERVE.get(rank, []) if c in deck]
    if len(avail) < count:
        return None
    return random.sample(avail, count)


def random_history_slot(deck):
    # deck: 可用卡牌集合（列表）
    types = ["empty","single","pair","triple","straight","bomb","rocket"]
    weights = [0.30, 0.25, 0.15, 0.10, 0.12, 0.05, 0.03]
    choice = random.choices(types, weights, k=1)[0]
    if choice == "empty":
        return []
    if choice == "single":
        if not deck: return []
        return [random.choice(deck)]
    if choice == "pair":
        candidates = [r for r in range(3,18) if len([c for c in RESERVE.get(r, []) if c in deck]) >= 2]
        if not candidates: return []
        r = random.choice(candidates)
        return pick_by_rank(deck, r, 2)
    if choice == "triple":
        candidates = [r for r in range(3,18) if len([c for c in RESERVE.get(r, []) if c in deck]) >= 3]
        if not candidates: return []
        r = random.choice(candidates)
        return pick_by_rank(deck, r, 3)
    if choice == "bomb":
        candidates = [r for r in range(3,16) if len([c for c in RESERVE.get(r, []) if c in deck]) >= 4]
        if not candidates: return []
        r = random.choice(candidates)
        return pick_by_rank(deck, r, 4)
    if choice == "rocket":
        if 52 in deck and 53 in deck:
            return [52,53]
        return []
    if choice == "straight":
        # 尝试若干次找一个可行的 5 连
        for _ in range(12):
            start = random.randint(3, 10)  # 3..10 起点，5 张连续
            ranks = [start + i for i in range(5)]
            cards = []
            ok = True
            for r in ranks:
                avail = [c for c in RESERVE.get(r, []) if c in deck and c not in cards]
                if not avail:
                    ok = False
                    break
                cards.append(random.choice(avail))
            if ok:
                return cards
        return []
    return []


def generate_case():
    # 随机抽 17 张做 own，3 张做 public，剩余用于 history 取样
    all_cards = ALL_CARDS[:]
    own = random.sample(all_cards, 17)
    remaining = [c for c in all_cards if c not in own]
    public = random.sample(remaining, 3)
    remaining2 = [c for c in remaining if c not in public]
    # 为 history 的两位玩家各生成一次出牌（可为空）
    hist0 = random_history_slot(remaining2)
    # 若希望历史牌不与另一个重叠，可删除已选的牌；这里为简单起见允许重叠
    hist1 = random_history_slot(remaining2)
    return {
        "requests": [
            {
                "history": [hist0, hist1],
                "publiccard": public,
                "own": own
            }
        ],
        "responses": [],
        "data": ""
    }


def main():
    parser = argparse.ArgumentParser(description="生成斗地主 bot 的请求 JSON 用例")
    parser.add_argument("--count", "-n", type=int, default=100, help="生成用例数量")
    parser.add_argument("--outdir", "-o", type=str, default="tests/cases", help="输出目录")
    args = parser.parse_args()

    os.makedirs(args.outdir, exist_ok=True)
    for i in range(args.count):
        case = generate_case()
        fname = os.path.join(args.outdir, f"case_{i:04d}.json")
        with open(fname, "w", encoding="utf-8") as f:
            json.dump(case, f, ensure_ascii=False, indent=4)
    print(f"已生成 {args.count} 个用例到 {args.outdir}")


if __name__ == '__main__':
    main()
