#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<algorithm>
#include<unordered_map>
#include<chrono>
#include<random>
#include<cmath>
#include<variant>
#include<array>
#include<functional>
#include<cstring>
#include<map>
#include<numeric>
#include"jsoncpp/json.h"
using namespace std;
/*斗地主bot --botzone作业
叫分及确定地主
从0号玩家开始，每个玩家轮流选择不叫分或者叫一个比目前为止最高分高的分数（不超过3）。玩家叫3分或2号玩家决策结束后，叫分阶段结束。叫分最高的玩家成为地主，或在没有人叫分的情况下，0号玩家成为地主。

发牌与明牌
每人先发17张牌，留三张牌作为底牌，牌面朝下放在桌上。这三张牌在地主确定后被亮明，所有人都知道这三张牌是什么。然后这三张牌归地主所有。即出牌前地主拥有20张牌，两农民各17张牌。

出牌
游戏开始时由地主先出，可出任一牌型的牌，接着地主的下家农民甲出牌，然后农民甲的下家农民乙先后出牌，之后一直重复地主、农民甲、农民乙的出牌顺序，直到某一方出完手中所有牌。

下家需要出比上家更大的牌，而且除了火箭和炸弹，牌的类型也要与上家相同。如果不出牌则选择“过”，由下一位玩家出牌。如果一玩家出牌后没有其他玩家打出更大的牌，则该玩家获得又一次任意出牌的机会。当一位玩家的手牌数为零时该方获胜，游戏结束。

牌型和大小
牌型有单张、一对、大于等于5张的连牌（顺子）、连对（至少三对）、三张、三张带一张、三张相同牌带一对牌、四张相同牌带任二张单牌或两对，炸弹（四张点数相同的牌），火箭（一对王，也就是一对Joker）。

牌型大小为：

火箭 ＞ 炸弹 ＞ 其他牌型

火箭最大，可以打任何的牌

炸弹按数值比大小，2最大，3最小

单张，一对，三带，单顺，双顺，飞机，四带二等等，全部同等级，但不同牌型之间互相不能混打

异常处理和分数计算
出现非法操作（崩溃、超时等），则该玩家会被裁判托管。托管后的玩家的决策策略是：

叫分阶段叫0分。
出牌阶段如果可以跳过则跳过，否则出序号最小的一张牌。
在被托管的玩家数目达到3的瞬间，游戏结束，所有人获得-1分。

定义底分为 min(地主叫分, 1) ，此后底分在以下特殊情况会进行翻倍：

任何一个玩家出了炸弹，每个炸弹都会使底分翻一倍。
任何一个玩家出了火箭，则会使底分翻一倍。
地主春天，也即两个农民一张牌都没有出，而地主全部出完，底分翻一倍。
农民反春，也即地主除了第一手开牌之外，没有再出过牌，而其中一个农民全部出完，底分翻一倍。
当一名玩家出完所有的牌之后，游戏结束，开始结算分数。

如果农民方胜利，则每个农民获得底分，地主失去两倍底分。
如果地主方胜利，则每个农民失去底分，地主获得两倍底分。
牌型
单张
大王（彩色Joker）>小王（黑白Joker）>2>A>K>Q>J>10>9>8...>3，不计花色，上家出3，下家必须出4或以上。
一对
22>AA>...>33，不计花色，上家出33，必须出44或以上。一对王称为火箭，下详解。
单顺
最少5张牌，不能有2，不计花色。
例如：345678，78910JQ，345678910JQKA
上家出6只，必须跟6只；上家出10只，必须跟10只，如此类推。
双顺
最少3对，不能有2，不计花色。
例如：778899，445566，334455667788991010JJQQ
上家出6对，必须跟6对；上家出3对，必须跟3对，如此类推。
三带
三带分3种，但大前提都是2>A>...>3，并只以三条的部分比大小。
三不带（三带零），即三条，例如222，AAA，666，888等。
三带一，即三条+一只，例如2223，AAAJ，6669等，带的牌不能和三条部分的牌重复，否则会被判为炸弹。
三带二，即三条+一对，例如22233，AAAJJ等。
上家出三带一，必须跟三带一；上家出三不带，必须出三不带。
提醒：所带的单张牌也可以是大王、小王，但是大小王放在一起不算对子。下同。

四带二
四带分为两种，但大前提都是2>A>...>3，并只以四条的部分比大小。
四条加两只（两只不可重复），或四条加两对（两对不可重复），即888857，2222QQAA
如上家出四条加两只必须跟四条加两只，上家出四条加两对必须跟四条加两对。
四带二效果不等同炸弹，只当作普通牌型使用。
飞机
飞机是两个或多个连续数字的三条，只以三条的部分比大小。
飞机分三种，飞机不带翼，飞机带小翼，飞机带大翼。
例如：333444，777888999101010JJJQQQ
飞机不带翼，即纯粹飞机，例如：444555
飞机带小翼，即连续多于一个三带一，所带的单牌不能出现重复，单牌不能和三条部分牌重复，否则会被判为其余牌型，或违规牌型
飞机带大翼，即连续多于一个三带二，所带的对子不能出现重复
例如：33344456，77788834，101010JJJQQQ335577，6667778883399JJ是合法牌型
例如：33344455（违规），33344434（航天飞机），33344435（违规），3334446666（违规）是其余或违规牌型
如上家出飞机不带翼必须跟飞机不带翼，如上家出飞机带翼必须跟飞机带翼
任何情况下，其三条部分都不能有2
航天飞机
此种牌型极少出现，但仍有理论上的可能性。

航天飞机是两个或多个连续数字的四条，只以四条的部分比大小 航天飞机分三种，不带翼，带小翼（各两只），带大翼（各两对）。 同样，所有的只和对均不可重复。

如：

不带翼: 33334444
带小翼: 44445555 37 JQ，333344445555 67 89 10J（18张牌，仅地主）
带大翼: 44445555 3377 JJQQ，33334444 6677 8899
任何情况下，其四条部分都不能有2
炸弹
即四条，如：9999，QQQQ
炸弹大于除火箭外的一切牌型。点数大的炸弹大于点数小的炸弹，如:4444>3333，最大的炸弹是2222
火箭
即大王+小王
火箭大于所有牌型


游戏交互方式
提示
如果你不能理解以下交互方式,可以直接看#游戏样例程序，修改其中

// 做出决策（你只需修改以下部分）
到

// 决策结束，输出结果（你只需修改以上部分）
之间的部分即可！

本游戏与Botzone上其他游戏一样，使用相同的交互方式：Bot#交互

请注意程序是有计算时间的限制的，每步要在1秒内完成！

具体交互内容
在交互中，叫分决策包括0-3共四个整数，分别表示不叫分、叫1分、叫2分、叫3分。

在交互中，游戏中的所有牌使用0-53共54个正整数进行编号。对应关系如下：

牌号	牌面	牌号	牌面	牌号	牌面	牌号	牌面	牌号	牌面	牌号	牌面
0	红桃3	1	方块3	2	黑桃3	3	草花3	4	红桃4	5	方块4
6	黑桃4	7	草花4	8	红桃5	9	方块5	10	黑桃5	11	草花5
12	红桃6	13	方块6	14	黑桃6	15	草花6	16	红桃7	17	方块7
18	黑桃7	19	草花7	20	红桃8	21	方块8	22	黑桃8	23	草花8
24	红桃9	25	方块9	26	黑桃9	27	草花9	28	红桃10	29	方块10
30	黑桃10	31	草花10	32	红桃J	33	方块J	34	黑桃J	35	草花J
36	红桃Q	37	方块Q	38	黑桃Q	39	草花Q	40	红桃K	41	方块K
42	黑桃K	43	草花K	44	红桃A	45	方块A	46	黑桃A	47	草花A
48	红桃2	49	方块2	50	黑桃2	51	草花2	52	小王	53	大王
每回合只有一个Bot会收到request。Bot收到的request是一个JSON对象，表示之前的出牌或叫分情况。格式如下：

叫分request

{
    "own": [0, 1, 2, 3, 4] // 自己最初拥有哪些牌
    "bid": [0, 2] // 前面的玩家的叫分决策
}
对于bid数组：

0号玩家得到空数组
1号玩家得到长度为1的数组，是0号玩家的叫分决策
2号玩家得到长度为2的数组，分别表示0号玩家和1号玩家的叫分决策。
收到叫分request时，Bot所需要输出的response是一个数字，表示自己的叫分决策。

第一个出牌request

{
    "history": [[0, 1, 2] 上上家 , [] 上家 ],  总是两项，每一项都是数组，分别表示上上家和上家出的牌，空数组表示跳过回合或者还没轮到他。
    "publiccard": [29, 8, 14], 地主被公开的三张牌
    "own": [0, 1, 2, 3, 4] // 自己最初拥有哪些牌
    "landlord": 2, // 地主的玩家位置
    "pos": 0, // Bot的玩家位置
    "finalbid": 1 // 叫分阶段的底分
}
此后的出牌request

{
    "history": [[0, 1, 2] 上上家 , [] 上家 ], // 总是两项，每一项都是数组，分别表示上上家和上家出的牌，空数组表示跳过回合。
}
收到出牌request时，Bot所需要输出的response是一个JSON数组，表示自己要出的牌。空数组表示跳过回合。

    作者：舒义鹏，时间：2026-4-7
*/
//==========牌型结构体==========//
struct Rocket {};
struct Bomb { int value; };
struct Single { int value; };
struct Pair { int value; };
struct Triple { int value; };
struct Straight { int start; int len; };
struct PairSequence { int start; int len; };
struct TripleSequence { int start; int len; };
struct TripleWithOne { int triple; int single; };
struct TripleWithTwo { int triple; int pair; };
struct QuadWithSingles { int quad; int single1; int single2; };
struct QuadWithPairs { int quad; int pair1; int pair2; };
struct PlanWithSingles { int start; int len; vector<int> singles; };
struct PlanWithPairs { int start; int len; vector<int> pairs; };
using CardPattern = variant<
    Rocket, Bomb, Single, Pair
    , Triple, Straight, PairSequence,
    TripleSequence, TripleWithOne
    , TripleWithTwo, QuadWithSingles,
    QuadWithPairs, PlanWithSingles
    , PlanWithPairs>;
//====牌型检测命名空间===//
namespace PatternCheck {
    bool check(const int cnt[18], const vector<int>& /*lastMovePatterns*/, Rocket&, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            return total == 2 && cnt[16] == 1 && cnt[17] == 1;
        }
        else {
            return cnt[16] >= 1 && cnt[17] >= 1;
        }
    }

    // Bomb
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, Bomb& bomb, bool exact = false) {
        if (exact) {
            int total = 0, quadVal = -1;
            for (int v = 3; v <= 17; ++v) {
                total += cnt[v];
                if (cnt[v] == 4) quadVal = v;
                else if (cnt[v] != 0) return false;
            }
            if (total == 4 && quadVal != -1) {
                bomb.value = quadVal;
                return true;
            }
            return false;
        }
        else {
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] >= 4) {
                    bomb.value = v;
                    if (!lastMovePatterns.empty() && lastMovePatterns.size() == 4 && v <= lastMovePatterns[0])
                        continue;
                    return true;
                }
            }
            return false;
        }
    }

    // Single
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, Single& single, bool exact = false) {
        if (exact) {
            int total = 0, val = -1;
            for (int v = 3; v <= 17; ++v) {
                total += cnt[v];
                if (cnt[v] == 1) val = v;
                else if (cnt[v] != 0) return false;
            }
            if (total == 1 && val != -1) {
                single.value = val;
                return true;
            }
            return false;
        }
        else {
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] >= 1) {
                    if (!lastMovePatterns.empty() && lastMovePatterns[0] >= v) continue;
                    single.value = v;
                    return true;
                }
            }
            return false;
        }
    }

    // Pair
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, Pair& pair, bool exact = false) {
        if (exact) {
            int total = 0, val = -1;
            for (int v = 3; v <= 17; ++v) {
                total += cnt[v];
                if (cnt[v] == 2) val = v;
                else if (cnt[v] != 0) return false;
            }
            if (total == 2 && val != -1) {
                pair.value = val;
                return true;
            }
            return false;
        }
        else {
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] >= 2) {
                    if (!lastMovePatterns.empty() && lastMovePatterns[0] >= v) continue;
                    pair.value = v;
                    return true;
                }
            }
            return false;
        }
    }

    // Triple
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, Triple& triple, bool exact = false) {
        if (exact) {
            int total = 0, val = -1;
            for (int v = 3; v <= 17; ++v) {
                total += cnt[v];
                if (cnt[v] == 3) val = v;
                else if (cnt[v] != 0) return false;
            }
            if (total == 3 && val != -1) {
                triple.value = val;
                return true;
            }
            return false;
        }
        else {
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] >= 3) {
                    if (!lastMovePatterns.empty() && lastMovePatterns[0] >= v) continue;
                    triple.value = v;
                    return true;
                }
            }
            return false;
        }
    }

    // Straight
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, Straight& straight, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total < 5 || total > 12) return false;
            int start = -1, len = 0;
            for (int i = 3; i <= 14; ++i) {
                if (cnt[i] == 1) {
                    if (start == -1) start = i;
                    len++;
                    if (i == 14 || cnt[i + 1] != 1) break;
                }
                else if (cnt[i] != 0) return false;
            }
            if (len == total && len >= 5) {
                straight.start = start;
                straight.len = len;
                return true;
            }
            return false;
        }
        else {
            int maxlen = 0, beststart = -1;
            for (int start = 3; start <= 14; ++start) {
                int len = 0;
                while (start + len <= 14 && cnt[start + len] >= 1) len++;
                if (len >= 5 && len > maxlen) {
                    maxlen = len;
                    beststart = start;
                }
                start += len;
            }
            if (beststart == -1) return false;
            if (!lastMovePatterns.empty()) {
                int lastlen = lastMovePatterns.size();
                int laststart = lastMovePatterns[0];
                if (maxlen != lastlen || beststart <= laststart) return false;
            }
            straight.start = beststart;
            straight.len = maxlen;
            return true;
        }
    }

    // PairSequence
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, PairSequence& ps, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            int len = total / 2;
            if (total % 2 != 0 || total < 6 || len > 10) return false;
            vector<int> vals;
            for (int i = 3; i <= 17; ++i) {
                if (cnt[i] == 2) vals.push_back(i);
                else if (cnt[i] != 0) return false;
            }
            if ((int)vals.size() != len) return false;
            bool cont = true;
            for (size_t i = 1; i < vals.size(); ++i) {
                if (vals[i] != vals[i - 1] + 1) { cont = false; break; }
            }
            if (cont && len >= 3) {
                ps.start = vals[0];
                ps.len = len;
                return true;
            }
            return false;
        }
        else {
            int maxlen = 0, beststart = -1;
            for (int start = 3; start <= 14; ++start) {
                int len = 0;
                while (start + len <= 14 && cnt[start + len] >= 2) len++;
                if (len >= 3 && len > maxlen) {
                    maxlen = len;
                    beststart = start;
                }
                start += len;
            }
            if (beststart == -1) return false;
            if (!lastMovePatterns.empty()) {
                int lastlen = lastMovePatterns.size() / 2;
                int laststart = lastMovePatterns[0];
                if (maxlen != lastlen || beststart <= laststart) return false;
            }
            ps.start = beststart;
            ps.len = maxlen;
            return true;
        }
    }

    // TripleSequence (飞机无翼)
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, TripleSequence& ts, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total % 3 != 0 || total < 6 || total > 12) return false;
            int len = total / 3;
            vector<int> vals;
            for (int i = 3; i <= 17; ++i) {
                if (cnt[i] == 3) vals.push_back(i);
                else if (cnt[i] != 0) return false;
            }
            if ((int)vals.size() != len) return false;
            bool cont = true;
            for (size_t i = 1; i < vals.size(); ++i) {
                if (vals[i] != vals[i - 1] + 1) { cont = false; break; }
            }
            if (cont && len >= 2) {
                ts.start = vals[0];
                ts.len = len;
                return true;
            }
            return false;
        }
        else {
            int maxlen = 0, beststart = -1;
            for (int start = 3; start <= 14; ++start) {
                int len = 0;
                while (start + len <= 14 && cnt[start + len] >= 3) len++;
                if (len >= 2 && len > maxlen) {
                    maxlen = len;
                    beststart = start;
                }
                start += len;
            }
            if (beststart == -1) return false;
            if (!lastMovePatterns.empty()) {
                int lastlen = lastMovePatterns.size() / 3;
                int laststart = lastMovePatterns[0];
                if (maxlen != lastlen || beststart <= laststart) return false;
            }
            ts.start = beststart;
            ts.len = maxlen;
            return true;
        }
    }

    // TripleWithOne
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, TripleWithOne& tws, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total != 4) return false;
            int tripleVal = -1, singleVal = -1;
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] == 3) tripleVal = v;
                else if (cnt[v] == 1) singleVal = v;
                else if (cnt[v] != 0) return false;
            }
            if (tripleVal != -1 && singleVal != -1) {
                tws.triple = tripleVal;
                tws.single = singleVal;
                return true;
            }
            return false;
        }
        else {
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] >= 3) {
                    if (!lastMovePatterns.empty()) {
                        int lastTriple = lastMovePatterns[0];
                        if (v <= lastTriple) continue;
                    }
                    for (int s = 3; s <= 17; ++s) {
                        if (s != v && cnt[s] >= 1) {
                            tws.triple = v;
                            tws.single = s;
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    }

    // TripleWithTwo
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, TripleWithTwo& tws, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total != 5) return false;
            int tripleVal = -1, pairVal = -1;
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] == 3) tripleVal = v;
                else if (cnt[v] == 2) pairVal = v;
                else if (cnt[v] != 0) return false;
            }
            if (tripleVal != -1 && pairVal != -1) {
                tws.triple = tripleVal;
                tws.pair = pairVal;
                return true;
            }
            return false;
        }
        else {
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] >= 3) {
                    if (!lastMovePatterns.empty()) {
                        int lastTriple = lastMovePatterns[0];
                        if (v <= lastTriple) continue;
                    }
                    for (int s = 3; s <= 17; ++s) {
                        if (s != v && cnt[s] >= 2) {
                            tws.triple = v;
                            tws.pair = s;
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    }

    // QuadWithSingles
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, QuadWithSingles& qws, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total != 6) return false;
            int quadVal = -1;
            vector<int> singles;
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] == 4) quadVal = v;
                else if (cnt[v] == 1) singles.push_back(v);
                else if (cnt[v] != 0) return false;
            }
            if (quadVal != -1 && singles.size() == 2) {
                qws.quad = quadVal;
                qws.single1 = singles[0];
                qws.single2 = singles[1];
                return true;
            }
            return false;
        }
        else {
            for (int q = 3; q <= 17; ++q) {
                if (cnt[q] >= 4) {
                    if (!lastMovePatterns.empty()) {
                        int lastQuad = lastMovePatterns[0];
                        if (q <= lastQuad) continue;
                    }
                    vector<int> singles;
                    for (int s = 3; s <= 17; ++s) {
                        if (s != q && cnt[s] >= 1) singles.push_back(s);
                    }
                    if (singles.size() >= 2) {
                        qws.quad = q;
                        qws.single1 = singles[0];
                        qws.single2 = singles[1];
                        return true;
                    }
                }
            }
            return false;
        }
    }

    // QuadWithPairs
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, QuadWithPairs& qwp, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total != 8) return false;
            int quadVal = -1;
            vector<int> pairs;
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] == 4) quadVal = v;
                else if (cnt[v] == 2) pairs.push_back(v);
                else if (cnt[v] != 0) return false;
            }
            if (quadVal != -1 && pairs.size() == 2) {
                qwp.quad = quadVal;
                qwp.pair1 = pairs[0];
                qwp.pair2 = pairs[1];
                return true;
            }
            return false;
        }
        else {
            for (int q = 3; q <= 17; ++q) {
                if (cnt[q] >= 4) {
                    if (!lastMovePatterns.empty()) {
                        int lastQuad = lastMovePatterns[0];
                        if (q <= lastQuad) continue;
                    }
                    vector<int> pairs;
                    for (int s = 3; s <= 17; ++s) {
                        if (s != q && cnt[s] >= 2) pairs.push_back(s);
                    }
                    if (pairs.size() >= 2) {
                        qwp.quad = q;
                        qwp.pair1 = pairs[0];
                        qwp.pair2 = pairs[1];
                        return true;
                    }
                }
            }
            return false;
        }
    }

    // PlanWithSingles (飞机带单牌)
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, PlanWithSingles& pws, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total % 4 != 0 || total < 8) return false;
            int k = total / 4;
            vector<int> triples, singles;
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] == 3) triples.push_back(v);
                else if (cnt[v] == 1) singles.push_back(v);
                else if (cnt[v] != 0) return false;
            }
            if ((int)triples.size() != k || (int)singles.size() != k) return false;
            // 检查三张部分是否连续
            bool cont = true;
            for (int i = 1; i < k; ++i) {
                if (triples[i] != triples[i - 1] + 1) { cont = false; break; }
            }
            if (!cont) return false;
            // 单牌不能与三张点数重复（已经通过 cnt 检查，因为三张部分占3张，单牌占1张，不会重复）
            pws.start = triples[0];
            pws.len = k;
            pws.singles = singles;
            sort(pws.singles.begin(), pws.singles.end()); // 确保有序
            return true;
        }
        else {
            int maxlen = 0, beststart = -1;
            for (int start = 3; start <= 14; ++start) {
                int len = 0;
                while (start + len <= 14 && cnt[start + len] >= 3) len++;
                if (len >= 2 && len > maxlen) {
                    maxlen = len;
                    beststart = start;
                }
                start += len;
            }
            if (beststart == -1) return false;
            if (!lastMovePatterns.empty()) {
                int lastlen = lastMovePatterns.size() / 4;
                int laststart = lastMovePatterns[0];
                if (maxlen != lastlen || beststart <= laststart) return false;
            }
            int tmpCnt[18];
            memcpy(tmpCnt, cnt, sizeof(tmpCnt));
            for (int j = 0; j < maxlen; ++j) tmpCnt[beststart + j] -= 3;
            vector<int> singles;
            for (int v = 3; v <= 17; ++v) {
                for (int k = 0; k < tmpCnt[v]; ++k) singles.push_back(v);
            }
            if ((int)singles.size() < maxlen) return false;
            sort(singles.begin(), singles.end());
            pws.start = beststart;
            pws.len = maxlen;
            pws.singles.assign(singles.begin(), singles.begin() + maxlen);
            return true;
        }
    }

    // PlanWithPairs (飞机带对子)
    bool check(const int cnt[18], const vector<int>& lastMovePatterns, PlanWithPairs& pwp, bool exact = false) {
        if (exact) {
            int total = 0;
            for (int i = 3; i <= 17; ++i) total += cnt[i];
            if (total % 5 != 0 || total < 10) return false;
            int k = total / 5;
            vector<int> triples, pairs;
            for (int v = 3; v <= 17; ++v) {
                if (cnt[v] == 3) triples.push_back(v);
                else if (cnt[v] == 2) pairs.push_back(v);
                else if (cnt[v] != 0) return false;
            }
            if ((int)triples.size() != k || (int)pairs.size() != k) return false;
            bool cont = true;
            for (int i = 1; i < k; ++i) {
                if (triples[i] != triples[i - 1] + 1) { cont = false; break; }
            }
            if (!cont) return false;
            pwp.start = triples[0];
            pwp.len = k;
            pwp.pairs = pairs;
            sort(pwp.pairs.begin(), pwp.pairs.end());
            return true;
        }
        else {
            int maxlen = 0, beststart = -1;
            for (int start = 3; start <= 14; ++start) {
                int len = 0;
                while (start + len <= 14 && cnt[start + len] >= 3) len++;
                if (len >= 2 && len > maxlen) {
                    maxlen = len;
                    beststart = start;
                }
                start += len;
            }
            if (beststart == -1) return false;
            if (!lastMovePatterns.empty()) {
                int lastlen = lastMovePatterns.size() / 5;
                int laststart = lastMovePatterns[0];
                if (maxlen != lastlen || beststart <= laststart) return false;
            }
            int tmpCnt[18];
            memcpy(tmpCnt, cnt, sizeof(tmpCnt));
            for (int j = 0; j < maxlen; ++j) tmpCnt[beststart + j] -= 3;
            vector<int> pairs;
            for (int v = 3; v <= 17; ++v) {
                for (int k = 0; k < tmpCnt[v] / 2; ++k) pairs.push_back(v);
            }
            if ((int)pairs.size() < maxlen) return false;
            sort(pairs.begin(), pairs.end());
            pwp.start = beststart;
            pwp.len = maxlen;
            pwp.pairs.assign(pairs.begin(), pairs.begin() + maxlen);
            return true;
        }
    }
    int getMainValueOfLastMove(const vector<int>& lastMovePatterns) {
        if (lastMovePatterns.empty()) return 0;
        int cnt[18] = { 0 };
        for (int v : lastMovePatterns) cnt[v]++;

        int sz = lastMovePatterns.size();

        // 单张/对子/三条/炸弹：首元素就是主值
        if (sz == 1 || sz == 2 || sz == 3 || sz == 4) {
            return lastMovePatterns[0];
        }

        // 火箭：大小王，也正确
        if (sz == 2 && cnt[16] == 1 && cnt[17] == 1) return 16; // 大小王比较时16<17，但火箭最大，用16也行

        // 顺子：sz张，都是1张且连续
        bool isStraight = true;
        int startStraight = -1, lenStraight = 0;
        for (int v = 3; v <= 14; ++v) {
            if (cnt[v] == 1) {
                if (startStraight == -1) startStraight = v;
                lenStraight++;
            }
            else if (cnt[v] > 1) { isStraight = false; break; }
        }
        if (isStraight && lenStraight == sz) return startStraight;

        // 连对：sz是偶数，每张2且连续
        if (sz % 2 == 0) {
            bool isPairSeq = true;
            int startPS = -1, lenPS = 0;
            for (int v = 3; v <= 14; ++v) {
                if (cnt[v] == 2) {
                    if (startPS == -1) startPS = v;
                    lenPS++;
                }
                else if (cnt[v] != 0) { isPairSeq = false; break; }
            }
            if (isPairSeq && lenPS == sz / 2) return startPS;
        }

        // 飞机无翼：sz是3的倍数，每张3且连续
        if (sz % 3 == 0) {
            bool isTriSeq = true;
            int startTS = -1, lenTS = 0;
            for (int v = 3; v <= 14; ++v) {
                if (cnt[v] == 3) {
                    if (startTS == -1) startTS = v;
                    lenTS++;
                }
                else if (cnt[v] != 0) { isTriSeq = false; break; }
            }
            int k = sz / 3;
            if (isTriSeq && lenTS == k) return startTS;
        }

        // 三带一 (sz=4) 或 三带二 (sz=5)：找到cnt==3或>=3的值
        if (sz == 4 || sz == 5) {
            for (int v = 3; v <= 17; ++v)
                if (cnt[v] >= 3) return v;
        }

        // 四带二单 (sz=6) 或 四带二对 (sz=8)：找到cnt>=4的值
        if (sz == 6 || sz == 8) {
            for (int v = 3; v <= 17; ++v)
                if (cnt[v] >= 4) return v;
        }

        // 飞机带单 (sz = 4k, k>=2) 或飞机带对 (sz = 5k, k>=2)
        if ((sz >= 8 && sz % 4 == 0) || (sz >= 10 && sz % 5 == 0)) {
            int seqStart = -1, seqLen = 0;
            for (int v = 3; v <= 14; ++v) {
                if (cnt[v] >= 3) {
                    if (seqStart == -1) seqStart = v;
                    seqLen++;
                }
                else {
                    if (seqLen >= 2) break;
                    seqStart = -1;
                    seqLen = 0;
                }
            }
            if (seqStart != -1 && seqLen >= 2) return seqStart;
        }

        // fallback
        return lastMovePatterns[0];
    }
    inline vector<vector<int>> enumerateRocket(const int cnt[18]) {
        if (cnt[16] >= 1 && cnt[17] >= 1) return { {16, 17} };
        return {};
    }
    inline vector<vector<int>> enumerateBombs(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minVal = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 4)
            minVal = getMainValueOfLastMove(lastMovePatterns)+1;
        for (int v = minVal; v <= 17; ++v) {
            if (cnt[v] >= 4) result.push_back({ v, v, v, v });
        }
        return result;
    }
    inline vector<vector<int>> enumerateSingles(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minVal = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 1)
            minVal = getMainValueOfLastMove(lastMovePatterns)+1;
        for (int v = minVal; v <= 17; ++v) {
            if (cnt[v] >= 1) result.push_back({ v });
        }
        return result;
    }
    inline vector<vector<int>> enumeratePairs(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minVal = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 2)
            minVal = getMainValueOfLastMove(lastMovePatterns)+1;
        for (int v = minVal; v <= 17; ++v) {
            if (cnt[v] >= 2) result.push_back({ v, v });
        }
        return result;
    }
    inline vector<vector<int>> enumerateTriples(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minVal = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 3)
            minVal = lastMovePatterns[0] + 1;
        for (int v = minVal; v <= 17; ++v) {
            if (cnt[v] >= 3) result.push_back({ v, v, v });
        }
        return result;
    }
    inline vector<vector<int>> enumerateStraights(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int lastLen = 0, lastStart = 0;
        if (!lastMovePatterns.empty()) {
            lastLen = lastMovePatterns.size();
            lastStart = getMainValueOfLastMove(lastMovePatterns)+1;
        }
        for (int len = 5; len <= 12; ++len) {
            if (lastLen != 0 && len != lastLen) continue;
            for (int start = 3; start + len - 1 <= 14; ++start) {
                if (lastLen != 0 && start <= lastStart) continue;
                bool ok = true;
                for (int i = 0; i < len; ++i) {
                    if (cnt[start + i] < 1) { ok = false; break; }
                }
                if (ok) {
                    vector<int> straight;
                    for (int i = 0; i < len; ++i) straight.push_back(start + i);
                    result.push_back(straight);
                }
            }
        }
        return result;
    }
    inline vector<vector<int>> enumeratePairSequences(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int lastLen = 0, lastStart = 0;
        if (!lastMovePatterns.empty()) {
            lastLen = lastMovePatterns.size() / 2;
            lastStart = getMainValueOfLastMove(lastMovePatterns)+1;
        }
        for (int len = 3; len <= 12; ++len) {
            if (lastLen != 0 && len != lastLen) continue;
            for (int start = 3; start + len - 1 <= 14; ++start) {
                if (lastLen != 0 && start <= lastStart) continue;
                bool ok = true;
                for (int i = 0; i < len; ++i) {
                    if (cnt[start + i] < 2) { ok = false; break; }
                }
                if (ok) {
                    vector<int> seq;
                    for (int i = 0; i < len; ++i) {
                        seq.push_back(start + i);
                        seq.push_back(start + i);
                    }
                    result.push_back(seq);
                }
            }
        }
        return result;
    }
    inline vector<vector<int>> enumerateTripleSequence(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int lastLen = 0, lastStart = 0;
        if (!lastMovePatterns.empty()) {
            lastLen = lastMovePatterns.size() / 3;
            lastStart = getMainValueOfLastMove(lastMovePatterns)+1;
        }
        for (int len = 2; len <= 6; ++len) {
            if (lastLen != 0 && len != lastLen) continue;
            for (int start = 3; start + len - 1 <= 14; ++start) {
                if (lastLen != 0 && start <= lastStart) continue;
                bool ok = true;
                for (int i = 0; i < len; ++i) {
                    if (cnt[start + i] < 3) { ok = false; break; }
                }
                if (ok) {
                    vector<int> plane;
                    for (int i = 0; i < len; ++i) {
                        for (int k = 0; k < 3; ++k) plane.push_back(start + i);
                    }
                    result.push_back(plane);
                }
            }
        }
        return result;
    }
    inline vector<vector<int>> enumerateTriplesWithOne(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minTriple = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 4)
            minTriple = getMainValueOfLastMove(lastMovePatterns)+1;
        for (int triple = minTriple; triple <= 17; ++triple) {
            if (cnt[triple] < 3) continue;
            for (int single = 3; single <= 17; ++single) {
                if (single == triple) continue;
                if (cnt[single] < 1) continue;
                result.push_back({ triple, triple, triple, single });
            }
        }
        return result;
    }
    inline vector<vector<int>> enumerateTriplesWithTwo(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minTriple = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 5)
            minTriple = getMainValueOfLastMove(lastMovePatterns)+1;
        for (int triple = minTriple; triple <= 17; ++triple) {
            if (cnt[triple] < 3) continue;
            for (int pair = 3; pair <= 17; ++pair) {
                if (pair == triple) continue;
                if (cnt[pair] < 2) continue;
                result.push_back({ triple, triple, triple, pair, pair });
            }
        }
        return result;
    }
    inline vector<vector<int>> enumeratePlanesWithSingles(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int lastLen = 0, lastStart = 0;
        if (!lastMovePatterns.empty()) {
            lastLen = lastMovePatterns.size() / 4;
            lastStart = getMainValueOfLastMove(lastMovePatterns);
        }
        for (int len = 2; len <= 4; ++len) {
            if (lastLen != 0 && len != lastLen) continue;
            for (int start = 3; start + len - 1 <= 14; ++start) {
                if (lastLen != 0 && start <= lastStart) continue;
                bool ok = true;
                for (int i = 0; i < len; ++i) {
                    if (cnt[start + i] < 3) { ok = false; break; }
                }
                if (!ok) continue;
                // 收集可用单牌（不能与飞机点数重复）
                vector<int> singles;
                for (int v = 3; v <= 17; ++v) {
                    bool used = false;
                    for (int i = 0; i < len; ++i) if (start + i == v) { used = true; break; }
                    if (used) continue;
                    if (cnt[v] >= 1) singles.push_back(v);
                }
                if ((int)singles.size() < len) continue;
                // 枚举所有组合（C(singles.size(), len)），为了性能，只取最小的len个单牌作为代表
                // 但为了完整性，这里实现简单的递归枚举（实际手牌中单牌数量有限，开销不大）
                vector<int> planeBody;
                for (int i = 0; i < len; ++i)
                    for (int k = 0; k < 3; ++k) planeBody.push_back(start + i);
                if (singles.size() >= len) {
                    std::sort(singles.begin(), singles.end());
                    singles.resize(len);
                    vector<int> action = planeBody;
                    action.insert(action.end(), singles.begin(), singles.end());
                    result.push_back(action);
                }
            }
        }
        return result;
    }
    inline vector<vector<int>> enumeratePlanesWithPairs(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int lastLen = 0, lastStart = 0;
        if (!lastMovePatterns.empty()) {
            lastLen = lastMovePatterns.size() / 5;
            lastStart = getMainValueOfLastMove(lastMovePatterns);
        }
        for (int len = 2; len <= 4; ++len) {
            if (lastLen != 0 && len != lastLen) continue;
            for (int start = 3; start + len - 1 <= 14; ++start) {
                if (lastLen != 0 && start <= lastStart) continue;
                bool ok = true;
                for (int i = 0; i < len; ++i) {
                    if (cnt[start + i] < 3) { ok = false; break; }
                }
                if (!ok) continue;
                // 收集可用对子（不能与飞机点数重复）
                vector<int> pairs;
                for (int v = 3; v <= 17; ++v) {
                    bool used = false;
                    for (int i = 0; i < len; ++i) if (start + i == v) { used = true; break; }
                    if (used) continue;
                    if (cnt[v] >= 2) pairs.push_back(v);
                }
                if ((int)pairs.size() < len) continue;
                // 枚举所有组合
                vector<int> planeBody;
                for (int i = 0; i < len; ++i)
                    for (int k = 0; k < 3; ++k) planeBody.push_back(start + i);
                if (pairs.size() >= len) {
                    std::sort(pairs.begin(), pairs.end());
                    pairs.resize(len);   // 只保留最小的 len 个对子

                    vector<int> action = planeBody;
                    for (int v : pairs) {
                        action.push_back(v);
                        action.push_back(v);
                    }
                    result.push_back(action);
                }
            }
        }
        return result;
    }
    inline vector<vector<int>> enumerateQuadWithSingles(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minQuad = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 6) {
            minQuad = getMainValueOfLastMove(lastMovePatterns)+1;
        }
        for (int quad = minQuad; quad <= 17; ++quad) {
            if (cnt[quad] < 4) continue;
            vector<int> singles;
            for (int s = 3; s <= 17; ++s) {
                if (s != quad && cnt[s] >= 1) singles.push_back(s);
            }
            if (singles.size() < 2) continue;
            if (singles.size() >= 2) {
                std::sort(singles.begin(), singles.end());
                vector<int> action(4, quad);
                action.push_back(singles[0]);
                action.push_back(singles[1]);
                result.push_back(action);
            }
        }
        return result;
    }
    inline vector<vector<int>> enumerateQuadWithPairs(const int cnt[18], const vector<int>& lastMovePatterns) {
        vector<vector<int>> result;
        int minQuad = 3;
        if (!lastMovePatterns.empty() && lastMovePatterns.size() == 8) {
            minQuad = getMainValueOfLastMove(lastMovePatterns)+1;
        }
        for (int quad = minQuad; quad <= 17; ++quad) {
            if (cnt[quad] < 4) continue;
            vector<int> pairs;
            for (int p = 3; p <= 17; ++p) {
                if (p != quad && cnt[p] >= 2) pairs.push_back(p);
            }
            if (pairs.size() < 2) continue;
            if (pairs.size() >= 2) {
                std::sort(pairs.begin(), pairs.end());
                vector<int> action(4, quad);
                action.push_back(pairs[0]); action.push_back(pairs[0]);
                action.push_back(pairs[1]); action.push_back(pairs[1]);
                result.push_back(action);
            }
        }
        return result;
    }
     
}
class CardPatternAnalysis {
private:
    static const  vector<int> reserve_value_table[18];
public:
    static const int SINGLE = 0;
    static const int PAIR = 1;
    static const int TRIPLE = 2;
    static const int THREE_WITH_ONE = 3;
    static const int THREE_WITH_TWO = 4;
    static const int TRIPLE_SEQUENCE = 7;
    static const int TRIPLE_SEQUENCE_WITH_TWO_PAIRS = 6;
    static const int TRIPLE_SEQUENCE_WITH_ONE = 5;
    static const int STRAIGHT = 9;
    static const int PAIR_SEQUENCE = 8;
    static const int BOMB = 10;
    static const int ROCKET = 11;
    static const int QUAD_WITH_SINGLES = 12;
    static const int QUAD_WITH_PAIRS = 13;
    static int getCardType(const vector<int>& cards) {
        vector<int> patterns = divideIntoPatterns(cards);
        int cnt[18] = { 0 };
        for (int v : patterns) cnt[v]++;
        vector<int> emptyLast;  // 主动出牌时没有上家牌
        Rocket r;
        if (PatternCheck::check(cnt, emptyLast, r, true)) return ROCKET;
        Bomb b;
        if (PatternCheck::check(cnt, emptyLast, b, true)) return BOMB;
        Single s;
        if (PatternCheck::check(cnt, emptyLast, s, true)) return SINGLE;
        Pair p;
        if (PatternCheck::check(cnt, emptyLast, p, true)) return PAIR;
        Triple t;
        if (PatternCheck::check(cnt, emptyLast, t, true)) return TRIPLE;
        Straight st;
        if (PatternCheck::check(cnt, emptyLast, st, true)) return STRAIGHT;
        PairSequence ps;
        if (PatternCheck::check(cnt, emptyLast, ps, true)) return PAIR_SEQUENCE;
        TripleSequence ts;
        if (PatternCheck::check(cnt, emptyLast, ts, true)) return TRIPLE_SEQUENCE;
        TripleWithOne two1;
        if (PatternCheck::check(cnt, emptyLast, two1, true)) return THREE_WITH_ONE;
        TripleWithTwo two2;
        if (PatternCheck::check(cnt, emptyLast, two2, true)) return THREE_WITH_TWO;
        QuadWithSingles qws;
        if (PatternCheck::check(cnt, emptyLast, qws, true)) return QUAD_WITH_SINGLES;
        QuadWithPairs qwp;
        if (PatternCheck::check(cnt, emptyLast, qwp, true)) return QUAD_WITH_PAIRS;
        PlanWithSingles pws;
        if (PatternCheck::check(cnt, emptyLast, pws, true)) return TRIPLE_SEQUENCE_WITH_ONE;
        PlanWithPairs pwp;
        if (PatternCheck::check(cnt, emptyLast, pwp, true)) return TRIPLE_SEQUENCE_WITH_TWO_PAIRS;
        return -1;
    }
    static int getCardTypeFromPoints(const vector<int>& cards) {
         
        int cnt[18] = { 0 };
        for (int v : cards) cnt[v]++;
        vector<int> emptyLast;  // 主动出牌时没有上家牌
        Rocket r;
        if (PatternCheck::check(cnt, emptyLast, r, true)) return ROCKET;
        Bomb b;
        if (PatternCheck::check(cnt, emptyLast, b, true)) return BOMB;
        Single s;
        if (PatternCheck::check(cnt, emptyLast, s, true)) return SINGLE;
        Pair p;
        if (PatternCheck::check(cnt, emptyLast, p, true)) return PAIR;
        Triple t;
        if (PatternCheck::check(cnt, emptyLast, t, true)) return TRIPLE;
        Straight st;
        if (PatternCheck::check(cnt, emptyLast, st, true)) return STRAIGHT;
        PairSequence ps;
        if (PatternCheck::check(cnt, emptyLast, ps, true)) return PAIR_SEQUENCE;
        TripleSequence ts;
        if (PatternCheck::check(cnt, emptyLast, ts, true)) return TRIPLE_SEQUENCE;
        TripleWithOne two1;
        if (PatternCheck::check(cnt, emptyLast, two1, true)) return THREE_WITH_ONE;
        TripleWithTwo two2;
        if (PatternCheck::check(cnt, emptyLast, two2, true)) return THREE_WITH_TWO;
        QuadWithSingles qws;
        if (PatternCheck::check(cnt, emptyLast, qws, true)) return QUAD_WITH_SINGLES;
        QuadWithPairs qwp;
        if (PatternCheck::check(cnt, emptyLast, qwp, true)) return QUAD_WITH_PAIRS;
        PlanWithSingles pws;
        if (PatternCheck::check(cnt, emptyLast, pws, true)) return TRIPLE_SEQUENCE_WITH_ONE;
        PlanWithPairs pwp;
        if (PatternCheck::check(cnt, emptyLast, pwp, true)) return TRIPLE_SEQUENCE_WITH_TWO_PAIRS;
        return -1;
    }
    static vector<int> findCardValue(const vector<int>& handcards, const vector<int>& values) {
        vector<int> result;
        vector<int> tempHand = handcards; // 拷贝一份，用于移除已取牌
        for (int val : values) {
            // 在 tempHand 中找第一张点数为 val 的牌
            auto it = find_if(tempHand.begin(), tempHand.end(), [val](int card) {
                return getCardValue(card) == val;
                });
            if (it == tempHand.end()) {
                // 如果找不到任何一张点数为 val 的牌，返回空（表示取牌失败）
                return {};
            }
            result.push_back(*it);
            tempHand.erase(it); // 移除已使用的牌，避免后续重复
        }
        return result;
    }
    //是否出完牌
    static bool isallout(const vector<int>& cards) {
        return cards.empty();
    }
    // 获取牌的数值，0-51分别对应3-2，52是小王，53是大王
    static  int getCardValue(int card) {
        int res = 0;
        if (card >= 0 && card <= 3)res = 3;
        else if (card >= 4 && card <= 7)res = 4;
        else if (card >= 8 && card <= 11)res = 5;
        else if (card >= 12 && card <= 15)res = 6;
        else if (card >= 16 && card <= 19)res = 7;
        else if (card >= 20 && card <= 23)res = 8;
        else if (card >= 24 && card <= 27)res = 9;
        else if (card >= 28 && card <= 31)res = 10;
        else if (card >= 32 && card <= 35)res = 11;
        else if (card >= 36 && card <= 39)res = 12;
        else if (card >= 40 && card <= 43)res = 13;
        else if (card >= 44 && card <= 47)res = 14;
        else if (card >= 48 && card <= 51)res = 15;
        else if (card == 52)res = 16; // 小王
        else if (card == 53)res = 17; // 大王
        return res;
    }
    // 将牌转换为数值并排序，方便后续分析
    static vector<int> divideIntoPatterns(const vector<int>& cards) {
        vector<int> patterns;
        for (int card : cards) {
            patterns.push_back(getCardValue(card));
        }
        sort(patterns.begin(), patterns.end());
        return patterns;
    }
    // 分析手牌，统计每个点数的数量，并记录出现过的点数
    static void analyzeHand(const vector<int>& hand, int cnt[18], vector<int>& uniqueVals) {
        memset(cnt, 0, sizeof(int) * 18);
        for (int card : hand) {
            int val = getCardValue(card);
            cnt[val]++;
        }
        uniqueVals.clear();
        for (int v = 3; v <= 17; ++v) {
            if (cnt[v] > 0) uniqueVals.push_back(v);
        }
    }
    static int evaluateHand(const vector<int>& hand) {
        int cnt[18] = { 0 };
        double score = 0;
        for (int card : hand) {
            int val = CardPatternAnalysis::getCardValue(card);
            cnt[val]++;

        }
        return evaluateHand(cnt);
    }
    static int evaluateHand(const int* cnt) {

        double score = 0;

        if (cnt[17]) score += 10;
        if (cnt[16]) score += 8;
        if (cnt[15]) score += 5 * cnt[15];
        if (cnt[14]) score += 4 * cnt[14];
        if (cnt[13]) score += 3 * cnt[13];
        if (cnt[12]) score += 2 * cnt[12];
        if (cnt[11]) score += 1 * cnt[11];
        if (cnt[10]) score += 1 * cnt[10];

        // 炸弹
        for (int i = 3; i <= 17; ++i) {
            if (cnt[i] == 4) score += 15;
        }
        // 顺子（连续5个不同点数）
        int straight = 0, maxStraight = 0;
        for (int i = 3; i <= 14; ++i) {
            if (cnt[i] > 0) straight++;
            else {
                if (straight >= 5) maxStraight = max(maxStraight, straight);
                straight = 0;
            }
        }
        if (straight >= 5) maxStraight = max(maxStraight, straight);
        score += maxStraight * 8;
        // 单牌数量
        int singles = 0;
        for (int i = 3; i <= 17; ++i) if (cnt[i] == 1) singles++;
        if (singles <= 3) score += 5;
        return (int)score;
    }
    static int decideBid(const vector<int>& hand, bool isFirst, bool lastTwoPassed) {
        int score = evaluateHand(hand);
        if (isFirst) score -= 5;
        if (lastTwoPassed) score += 5;
        if (score >= 35) return 3;
        if (score >= 20) return 2;
        if (score >= 10) return 1;
        return 0;
    }
};
const vector<int> CardPatternAnalysis::reserve_value_table[18] = {
    {},//0
    {},//1
    {},//2
    {0, 1, 2, 3}, // 3
    {4, 5, 6, 7}, // 4
    {8, 9, 10, 11}, // 5
    {12, 13, 14, 15}, // 6
    {16, 17, 18, 19}, // 7
    {20, 21, 22, 23}, // 8
    {24, 25, 26, 27}, // 9
    {28, 29, 30, 31}, // 10
    {32, 33, 34, 35}, // J
    {36, 37, 38, 39}, // Q
    {40, 41, 42, 43}, // K
    {44, 45, 46, 47}, // A
    {48, 49, 50, 51}, // 2
    {52}, // 小王
    {53} // 大王
};
// 手牌索引类，用于快速查询和操作玩家的手牌，支持根据点数快速取牌和检查牌的数量
class HandIndex {
    array<vector<int>, 18> valueToCards;  // 点数 -> 牌号列表
    array<int, 18> valueCount;            // 点数 -> 数量
public:
    // 从手牌构建索引
    HandIndex(const vector<int>& hand) {
        valueCount.fill(0);
        for (int card : hand) {
            int v = CardPatternAnalysis::getCardValue(card);
            valueToCards[v].push_back(card);
            valueCount[v]++;
        }
    }

    // 检查某点数是否有至少 count 张
    bool canTake(int value, int count) const {
        if (value < 3 || value > 17) return false; // 无效点数
        return valueCount[value] >= count;
    }

    // 取出 count 张指定点数的牌（返回具体的牌号）
    vector<int> takeCards(int value, int count) {
        if (!canTake(value, count)) return {};
        auto& vec = valueToCards[value];
        vector<int> res(vec.end() - count, vec.end());
        vec.erase(vec.end() - count, vec.end());
        valueCount[value] -= count;
        return res;
    }

    // 按点数序列取牌（每个点数取对应次数）
    vector<int> takeCardsByValues(const vector<int>& values) {
        int need[18] = { 0 };
        for (int v : values) {
            if (v >= 3 && v <= 17) {
                need[v]++;
            }
        }
        for (int i = 3; i <= 17; ++i) {
            if (need[i] > valueCount[i] && need[i] > 0) return {};
        }
        vector<int> res;
        for (int v : values) {
            auto cards = takeCards(v, 1);
            if (cards.empty()) return {}; // 保守起见，虽然理论上不应该出现这种情况
            res.insert(res.end(), cards.begin(), cards.end());
        }
        return res;
    }

    // 放回牌（用于撤销操作，但 MCTS 中常用状态复制，故可省略）
    void putCards(const vector<int>& cards) {
        for (int c : cards) {
            int v = CardPatternAnalysis::getCardValue(c);
            valueToCards[v].push_back(c);
            valueCount[v]++;
        }
    }

    // 获取当前手牌总数（可选）
    int totalCards() const {
        int total = 0;
        for (int i = 3; i <= 17; ++i) {
            total += valueCount[i];
        }
        return total;
    }
};
// 游戏状态结构体，用于在博弈搜索（如Minimax）中传递和记录当前游戏状态
class GameState {
public:
    // 三个玩家的手牌，索引0为地主，1为农民甲，2为农民乙
    int myhand[3][18];
    int totalCards[3];
    vector<int> publiccard;// 底牌
    vector<vector<int>> history;//使用轻量级数据结构，防止过多复制造成性能问题
    int myRole; // 当前玩家角色 (0:地主, 1:农民甲, 2:农民乙)
    int landlordRole; // 地主角色索引（0, 1, 2），用于判断队友和对手
    int currentPassCount; // 当前连续过牌次数，用于判断是否需要重置跟牌压力
    int currentPlayer; // 当前玩家索引（0, 1, 2），用于轮流出牌
    int lastActionPlayer; // 上一个真正的出牌玩家索引（0, 1, 2），用于判断跟牌压力是否需要重置
    bool isGameOver; // 游戏是否结束
    int winner; // 0:地主胜, 1:农民甲胜, 2:农民乙胜
    mutable bool Actions_cached = false;// 是否已经缓存过当前状态的合法动作
    mutable vector<vector<int>> Cache_Actions;// 缓存当前状态的合法动作，避免重复计算
    // 构造函数，用于初始化状态
    GameState(const vector<int> hands_[3], const vector<int>& publiccards_, const vector<vector<int>>& history_, int landlord_role_, int my_role_)
        : publiccard(publiccards_),
        history(history_),
        myRole(my_role_),
        landlordRole(landlord_role_),
        currentPassCount(0),
        currentPlayer(landlord_role_),
        lastActionPlayer(landlord_role_),
        isGameOver(false),
        winner(-1) {//需要优化初始化函数，暂且搁置
        // 1. 初始化手牌和底牌 (直接使用传入的参数)
        vector<int> numduy;
        for (int i = 0; i < 3; ++i) {
            CardPatternAnalysis::analyzeHand(hands_[i], myhand[i], numduy);
            totalCards[i] = hands_[i].size();
        }
        
    }
    // 状态复制构造函数，用于递归搜索
    GameState(const GameState& other)
        :  publiccard(other.publiccard),      // 1
      history(other.history),            // 2
      myRole(other.myRole),              // 3
      landlordRole(other.landlordRole),  // 4
      currentPassCount(other.currentPassCount), // 5
      currentPlayer(other.currentPlayer),// 6
      lastActionPlayer(other.lastActionPlayer), // 7
      isGameOver(other.isGameOver),      // 8
      winner(other.winner),              // 9
      Actions_cached(false)  {
        for (int i = 0; i < 3; ++i) {
            memcpy(myhand[i], other.myhand[i], sizeof(myhand[i]));
            totalCards[i] = other.totalCards[i];
        }
    }
    GameState(GameState&& other) noexcept
        :
        publiccard(std::move(other.publiccard))
        , history(std::move(other.history))
        , myRole(other.myRole)
        , landlordRole(other.landlordRole)
        , currentPassCount(other.currentPassCount)
        , currentPlayer(other.currentPlayer)
        , lastActionPlayer(other.lastActionPlayer)
        , isGameOver(other.isGameOver)
        , winner(other.winner)
        , Actions_cached(false)      // 移动后缓存失效，安全
        , Cache_Actions()            // 清空缓存向量
    {
        for (int i = 0; i < 3; ++i) {
            memcpy(myhand[i], other.myhand[i], sizeof(myhand[i]));
            totalCards[i] = other.totalCards[i];
        }
    }
    GameState& operator=(GameState&& other) noexcept {
        if (this != &other) {
            publiccard = std::move(other.publiccard);
            history = std::move(other.history);
            myRole = other.myRole;
            landlordRole = other.landlordRole;
            currentPassCount = other.currentPassCount;
            currentPlayer = other.currentPlayer;
            lastActionPlayer = other.lastActionPlayer;
            isGameOver = other.isGameOver;
            winner = other.winner;
            Actions_cached = false;
            Cache_Actions.clear();
            for (int i = 0; i < 3; ++i) {
                memcpy(myhand[i], other.myhand[i], sizeof(myhand[i]));
                totalCards[i] = other.totalCards[i];
            }
        }
        return *this;
    }
    void applyActionInPlace(const vector<int>& action) {//状态转移函数，根据玩家的出牌动作，生成新的游戏状态
        if (action.empty()) {// 1. 从玩家手牌中移除出牌
            this->currentPassCount++;// 过牌，增加连续过牌计数
            if (this->currentPassCount >= 2) {
                this->currentPlayer = this->lastActionPlayer;// 如果连续两人过牌，重置跟牌压力
                this->currentPassCount = 0;
            }
            else {
                this->currentPlayer = (this->currentPlayer + 1) % 3;
                 
            }
        }
        else {
            //出牌从当前的手牌中移除出牌
            for (int val : action) {
                this->myhand[this->currentPlayer][val]--;
                this->totalCards[this->currentPlayer]--;
            }
            //更新历史
            this->history.push_back(action);
            //重置连续过牌计数，记录出牌者
            this->currentPassCount = 0;
            this->lastActionPlayer = this->currentPlayer;
            //下一家出牌
            this->currentPlayer = (this->currentPlayer + 1) % 3;
            if (this->totalCards[this->lastActionPlayer] == 0) {
                this->isGameOver = true;
                this->winner = this->lastActionPlayer;
            }
            
        }
    }
    GameState applyActionCopy(const vector<int>& action) const {
        GameState newState = *this;
        newState.applyActionInPlace(action);
        return newState;
    }
    vector<int> getPublicCard() const { return publiccard; }
    vector<int> getLastMove() const {
        if (!history.empty()) return history.back();
        else return {};
    }
    int* getCurrentPlayerHand() {
        return this->myhand[this->currentPlayer];
    }
    bool isLeading() const {
        if (history.empty()) return true;
        else return !history.empty() && lastActionPlayer == currentPlayer;
    }
    void copyCurrentCnt(int cnt[18]) const {
        memcpy(cnt, myhand[currentPlayer], 18 * sizeof(int));
    }
    vector<vector<int>> getAllActions() {
        if (Actions_cached) return Cache_Actions;
        int cnt[18];
        copyCurrentCnt(cnt);
        vector<int> lastMovePatterns;
        int lastType = -1;
        if (!isLeading()) {
            vector<int> lastMove = getLastMove();
            if (!lastMove.empty()) {
                lastMovePatterns = lastMove;
                sort(lastMovePatterns.begin(), lastMovePatterns.end());
                lastType = CardPatternAnalysis::getCardTypeFromPoints(lastMove);
                // 上家火箭，只能过牌
                if (lastType == CardPatternAnalysis::ROCKET) {
                    Cache_Actions = { {} };
                    Actions_cached = true;
                    return { {} };
                }
            }
        }

        vector<vector<int>> allActions;
        using EnumFunc = function<vector<vector<int>>(const int[18], const vector<int>&)>;
        auto addEnum = [&](EnumFunc func) {
            auto combos = func(cnt, lastMovePatterns);
            for (const auto& c : combos) allActions.push_back(c);
            };

        if (isLeading()) {
            // 主动出牌：全部牌型枚举
            auto rockets = PatternCheck::enumerateRocket(cnt);
            if (!rockets.empty()) allActions.push_back(rockets[0]);
            addEnum(PatternCheck::enumerateBombs);
            addEnum(PatternCheck::enumerateSingles);
            addEnum(PatternCheck::enumeratePairs);
            addEnum(PatternCheck::enumerateTriples);
            addEnum(PatternCheck::enumerateStraights);
            addEnum(PatternCheck::enumeratePairSequences);
            addEnum(PatternCheck::enumerateTripleSequence);
            addEnum(PatternCheck::enumerateTriplesWithOne);
            addEnum(PatternCheck::enumerateTriplesWithTwo);
            addEnum(PatternCheck::enumeratePlanesWithSingles);
            addEnum(PatternCheck::enumeratePlanesWithPairs);
            addEnum(PatternCheck::enumerateQuadWithSingles);
            addEnum(PatternCheck::enumerateQuadWithPairs);
        }
        else {
            // 跟牌：先加炸弹/火箭（可跨类型）
            auto rockets = PatternCheck::enumerateRocket(cnt);
            if (!rockets.empty()) allActions.push_back(rockets[0]);
            if (lastType == CardPatternAnalysis::BOMB) {
                addEnum(PatternCheck::enumerateBombs); // 只出更大的炸弹
            }
            else {
                // 上家非炸弹，可出任意炸弹
                auto bombs = PatternCheck::enumerateBombs(cnt, {});  // 空向量表示主动出炸弹
                for (auto& b : bombs) allActions.push_back(b);
            }

            // 同类型动作
            switch (lastType) {
            case CardPatternAnalysis::SINGLE:
                addEnum(PatternCheck::enumerateSingles); break;
            case CardPatternAnalysis::PAIR:
                addEnum(PatternCheck::enumeratePairs); break;
            case CardPatternAnalysis::TRIPLE:
                addEnum(PatternCheck::enumerateTriples); break;
            case CardPatternAnalysis::STRAIGHT:
                addEnum(PatternCheck::enumerateStraights); break;
            case CardPatternAnalysis::PAIR_SEQUENCE:
                addEnum(PatternCheck::enumeratePairSequences); break;
            case CardPatternAnalysis::TRIPLE_SEQUENCE:
                addEnum(PatternCheck::enumerateTripleSequence); break;
            case CardPatternAnalysis::THREE_WITH_ONE:
                addEnum(PatternCheck::enumerateTriplesWithOne); break;
            case CardPatternAnalysis::THREE_WITH_TWO:
                addEnum(PatternCheck::enumerateTriplesWithTwo); break;
            case CardPatternAnalysis::QUAD_WITH_SINGLES:
                addEnum(PatternCheck::enumerateQuadWithSingles); break;
            case CardPatternAnalysis::QUAD_WITH_PAIRS:
                addEnum(PatternCheck::enumerateQuadWithPairs); break;
            case CardPatternAnalysis::TRIPLE_SEQUENCE_WITH_ONE:
                addEnum(PatternCheck::enumeratePlanesWithSingles); break;
            case CardPatternAnalysis::TRIPLE_SEQUENCE_WITH_TWO_PAIRS:
                addEnum(PatternCheck::enumeratePlanesWithPairs); break;
            default: break;
            }
            allActions.push_back({}); // 过牌
        }

        Cache_Actions = allActions;
        Actions_cached = true;
        return allActions;
    }
    static int exactsearch(GameState& state, int depth, int myId, int alpha, int beta) {
        if (state.isGameOver) {
            bool landlordWin = (state.winner == state.landlordRole);
            bool iAmLandlord = (myId == state.landlordRole);
            return (landlordWin == iAmLandlord) ? 1 : -1;
        }
        if (depth <= 0)return 0;
        auto actions = state.getAllActions();
        sort(actions.begin(), actions.end(),
            [](const vector<int>& a, const vector<int>& b) {
                return a.size() < b.size();
            });
        if (actions.empty())actions.push_back({});
        if (myId == state.currentPlayer) {
            int value = -2;
            for (const auto& act : actions) {
                GameState next = state.applyActionCopy(act);
                int child = exactsearch(next, depth - 1, myId, alpha, beta);
                if (child > value)value = child;
                if (value > alpha)alpha = value;
                if (alpha >= beta)break;
            }
            return value;
        }
        else {
            int value = 2;
            for (const auto& act : actions) {
                GameState next = state.applyActionCopy(act);
                int child = exactsearch(next, depth - 1, myId, alpha, beta);
                if (child < value)value = child;
                if (value < beta)beta = value;
                if (alpha >= beta)break;
            }
            return value;
        }
    }
};
//====按需生成器====//
vector<int> getBestActionByPriority(const int* hand, const vector<int>& lastMove = {}) {
    int cnt[18];
    memcpy(cnt, hand, 18 * sizeof(int));
    vector<int> lastMovePatterns;
    if (!lastMove.empty()) {
        lastMovePatterns = lastMove;
        sort(lastMovePatterns.begin(), lastMovePatterns.end());
    }
    using CheckFunc = function<bool(CardPattern&)>;
    vector<CheckFunc> checkers = {
        // 长套优先（不拆牌，高效跑牌）
  [&](CardPattern& p) ->bool {Straight st; if (PatternCheck::check(cnt, lastMovePatterns, st)) { p = st; return true; } return false; },
  [&](CardPattern& p) ->bool {PairSequence ps; if (PatternCheck::check(cnt, lastMovePatterns, ps)) { p = ps; return true; } return false; },
  [&](CardPattern& p) ->bool {TripleSequence ts; if (PatternCheck::check(cnt, lastMovePatterns, ts)) { p = ts; return true; } return false; },
  [&](CardPattern& p) ->bool {PlanWithSingles pws; if (PatternCheck::check(cnt, lastMovePatterns, pws)) { p = pws; return true; } return false; },
  [&](CardPattern& p) ->bool {PlanWithPairs pwp; if (PatternCheck::check(cnt, lastMovePatterns, pwp)) { p = pwp; return true; } return false; },
  // 带牌型
  [&](CardPattern& p) ->bool {TripleWithOne two1; if (PatternCheck::check(cnt, lastMovePatterns, two1)) { p = two1; return true; } return false; },
  [&](CardPattern& p) ->bool {TripleWithTwo two2; if (PatternCheck::check(cnt, lastMovePatterns, two2)) { p = two2; return true; } return false; },
  [&](CardPattern& p) ->bool {QuadWithSingles qws; if (PatternCheck::check(cnt, lastMovePatterns, qws)) { p = qws; return true; } return false; },
  [&](CardPattern& p) ->bool {QuadWithPairs qwp; if (PatternCheck::check(cnt, lastMovePatterns, qwp)) { p = qwp; return true; } return false; },
  // 短牌（此时才出单/对/三）
  [&](CardPattern& p) ->bool {Triple t; if (PatternCheck::check(cnt, lastMovePatterns, t)) { p = t; return true; } return false; },
  [&](CardPattern& p) ->bool {Pair pr; if (PatternCheck::check(cnt, lastMovePatterns, pr)) { p = pr; return true; } return false; },
  [&](CardPattern& p) ->bool {Single s; if (PatternCheck::check(cnt, lastMovePatterns, s)) { p = s; return true; } return false; },
  // 保留武器（绝不轻易出）
  [&](CardPattern& p) ->bool {Bomb b; if (PatternCheck::check(cnt, lastMovePatterns, b)) { p = b; return true; } return false; },
  [&](CardPattern& p) ->bool {Rocket r; if (PatternCheck::check(cnt, lastMovePatterns, r)) { p = r; return true; } return false; }
    };
    for (auto& check : checkers) {
        CardPattern pattern;
        if (check(pattern)) {
            return visit([&](auto&& p) -> vector<int> {
                using T = decay_t< decltype(p) >;
                if constexpr (is_same_v<T, Rocket>) {

                    return { 16,17 };
                }
                else if constexpr (is_same_v<T, Bomb>) {

                    vector<int> res;
                    for (int i = 0; i < 4; i++) {
                        res.push_back(p.value);
                    }
                    return res;
                }
                else if constexpr (is_same_v<T, Single>) {

                    vector<int>res;
                    res.push_back(p.value);
                    return res;
                }
                else if constexpr (is_same_v<T, Pair>) {

                    vector<int>res;
                    for (int i = 0; i < 2; i++)res.push_back(p.value);
                    return res;
                }
                else if constexpr (is_same_v<T, Triple>) {

                    vector<int>res;
                    for (int i = 0; i < 3; i++)res.push_back(p.value);
                    return res;
                }
                else if constexpr (is_same_v<T, Straight>) {
                    vector<int> targetVals;
                    for (int i = 0; i < p.len; i++) targetVals.push_back(p.start + i);
                    return  targetVals;
                }
                else if constexpr (is_same_v<T, PairSequence>) {
                    vector<int> targetVals;
                    for (int i = 0; i < p.len; i++) {
                        targetVals.push_back(p.start + i);
                        targetVals.push_back(p.start + i);
                    }
                    return  targetVals;
                }
                else if constexpr (is_same_v<T, TripleSequence>) {
                    vector<int> targetVals;
                    for (int i = 0; i < p.len; i++) {
                        targetVals.push_back(p.start + i);
                        targetVals.push_back(p.start + i);
                        targetVals.push_back(p.start + i);
                    }
                    return  targetVals;
                }
                else if constexpr (is_same_v<T, TripleWithOne>) {
                    vector<int> targetvals(3, p.triple);
                    targetvals.push_back(p.single);
                    return targetvals;
                }
                else if constexpr (is_same_v<T, TripleWithTwo>) {
                    vector<int> targetvals(3, p.triple);
                    targetvals.push_back(p.pair);
                    targetvals.push_back(p.pair);
                    return targetvals;
                }
                else if constexpr (is_same_v<T, QuadWithSingles>) {
                    vector<int> targetvals(4, p.quad);
                    targetvals.push_back(p.single1);
                    targetvals.push_back(p.single2);
                    return  targetvals;
                }
                else if constexpr (is_same_v<T, QuadWithPairs>) {
                    vector<int> targetvals(4, p.quad);
                    targetvals.push_back(p.pair1);
                    targetvals.push_back(p.pair1);
                    targetvals.push_back(p.pair2);
                    targetvals.push_back(p.pair2);
                    return targetvals;
                }
                else if constexpr (is_same_v<T, PlanWithSingles>) {
                    vector<int> targetvals;
                    for (int i = 0; i < p.len; i++) {
                        targetvals.push_back(p.start + i);
                        targetvals.push_back(p.start + i);
                        targetvals.push_back(p.start + i);
                    }
                    targetvals.insert(targetvals.end(), p.singles.begin(), p.singles.end());
                    return  targetvals;
                }
                else if constexpr (is_same_v<T, PlanWithPairs>) {
                    vector<int> targetvals;
                    for (int i = 0; i < p.len; i++) {
                        targetvals.push_back(p.start + i);
                        targetvals.push_back(p.start + i);
                        targetvals.push_back(p.start + i);
                    }
                    for (int v : p.pairs) {
                        targetvals.push_back(v);
                        targetvals.push_back(v);
                    }
                    return  targetvals;
                }
                return {};
                }, pattern);
        }
    }
    return {};
}
struct OpponentParticle {
    int hand1[18];
    int hand2[18];
};
class ParticleFilter {
public:
    static constexpr int k = 100;
    static int myRole;
    static vector<OpponentParticle> particles;
    static void initialize(const GameState& rootState);
    static void update(const vector<int>& opponentAction, int opponentId);
    static OpponentParticle sample();
};
void ParticleFilter::initialize(const GameState& rootState) {
    int usedCnt[18] = { 0 };
    // 1. 统计我方当前手牌（地主已含底牌）
    for (int v = 3; v <= 17; v++)
        usedCnt[v] += rootState.myhand[rootState.myRole][v];
    //统计历史出牌
    int history[18] = { 0 };
    for (const auto& action : rootState.history) {
        for (int v : action) {
            history[v]++;
        }
    }
    // 2. 计算剩余牌的数量（总数4张，减去已知的）
    vector<int> pool;
    for (int v = 3; v <= 15; v++)
        for (int i = 0; i < 4 - usedCnt[v]; i++) pool.push_back(v);
    if (usedCnt[16] < 1) pool.push_back(16);
    if (usedCnt[17] < 1) pool.push_back(17);
    //扣除历史出牌
    for (int v = 3; v <= 17; v++) {
        for (int i = 0; i < history[v]; i++) {
            auto it = find(pool.begin(), pool.end(), v);
            if (it != pool.end()) pool.erase(it);
        }
    }
    int need1 = rootState.totalCards[(rootState.myRole + 1) % 3];
    int need2 = rootState.totalCards[(rootState.myRole + 2) % 3];

    // 安全保护：若池子不够，强制补足（仅用于调试，实际不应发生）
    while ((int)pool.size() < need1 + need2) {
        pool.push_back(3); // 补一张最小的牌
    }

    static mt19937 rng(random_device{}());
    particles.clear();
    for (int j = 0; j < k; j++) {
        shuffle(pool.begin(), pool.end(), rng);
        OpponentParticle p{};
        int idx = 0;
        for (int i = 0; i < need1; ++i) p.hand1[pool[idx++]]++;
        for (int i = 0; i < need2; ++i) p.hand2[pool[idx++]]++;
        particles.push_back(p);
    }
}
void ParticleFilter::update(const vector<int>& opponentAction, int opponentId) {
    int playedCnt[18] = { 0 };
    for (int v : opponentAction)playedCnt[v]++;
    vector<OpponentParticle> survivors;
    for (auto& p : particles) {
        int* hand = (opponentId == 1) ? p.hand1 : p.hand2;
        bool vaild = true;
        for (int v = 3; v <= 17; v++)
            if (hand[v] < playedCnt[v]) { vaild = false; break; }
        if (vaild)survivors.push_back(p);
    }
    if (survivors.empty()) {
        return;
    }
    for (auto& p : survivors) {
        int oppIdx = (opponentId - myRole + 3) % 3;
        int* hand = (oppIdx == 1) ? p.hand1 : p.hand2;
        for (int v = 3; v <= 17; v++)hand[v] -= playedCnt[v];
    }
    static mt19937 rng;
    uniform_int_distribution<int> dist(0, survivors.size() - 1);
    particles.clear();
    for (int j = 0; j < k; j++) {
        particles.push_back(survivors[dist(rng)]);
    }
}
OpponentParticle ParticleFilter::sample() {
    static mt19937 rng;
    uniform_int_distribution<int> dist(0, k - 1);
    return particles[dist(rng)];
}
//蒙特卡洛的实现
//1.选择基于3个标准牌的好坏程度，历史次数最多，胜负次数，公式：score = wins/visits + C*sqrt(ln(parent_visits)/visits)+a*prior，C为调节探索程度的常数，通常取1.4
//2.扩展：在选择的节点上随机选择一个未访问过的子节点进行扩展，添加到树中
//3.模拟：从新扩展的节点开始，依托现有贪心策略模拟游戏直到结束，记录结果（胜利或失败）
//4.反向传播：将模拟结果反向传播到树的节点上，更新每个节点的访问次数和胜利次数
//5.重复以上步骤，直到达到预设的迭代次数或时间限制，最终选择访问次数最多的子节点作为决策结果(最多1000次迭代)
class MCTSNode {
public:
    GameState* state;
    MCTSNode* parent;
    vector<MCTSNode*> children;
    vector<int> action; // 从父节点到当前节点的动作
    int visits;//历史场数
    int wins;//胜利场数
    int prior; // 权重
    int max_score; // 用于归一化先验概率的动态常数
    // 先验概率，可以根据启发式评估函数计算得到,evaluateHand函数可以用来评估当前手牌的好坏程度，作为先验概率的一部分
    //先验概率的归一化可以通过将评估分数除以一个动态常数即每个子动作的评估分数中最大的来实现，使得先验概率在0到1之间。
    MCTSNode(GameState& otherstate, MCTSNode* parent = nullptr, const vector<int>& action = {})
        : parent(parent), visits(0), wins(0), max_score(0) {
        this->state = new GameState(otherstate);
        this->action = action;
        // 计算先验分数（未归一化）
        int raw_prior = CardPatternAnalysis::evaluateHand(state->myhand[state->myRole]);
        if (parent) {
            max_score = max(parent->max_score, raw_prior);
            prior = (max_score > 0) ? (raw_prior * 100) / max_score : 0;//------权重待调整；问题：max_score整棵树共享一个全局最大值
        }
        else {
            max_score = raw_prior;
            prior = (max_score > 0) ? (raw_prior * 100) / max_score : 0;
        }
    }

    ~MCTSNode() {
        delete state;
        for (MCTSNode* child : children) {
            delete child;
        }
    }

    // 选择子节点，基于UCB1.0公式 UBC =  exploit + explore + a*prior
    MCTSNode* selectChild(double C = 1.4) {
        MCTSNode* bestChild = nullptr;
        double bestScore = -1e9;
        for (MCTSNode* child : children) {
            double exploit = (child->visits > 0) ? (double)child->wins / child->visits : 0.0;
            double explore = C * sqrt(log(visits + 1) / (child->visits + 1e-6));//待修改explore和C取值，防止除以0
            double UCB = exploit + explore + 0.1 * child->prior; // 待修改0.1权重
            if (UCB > bestScore) {
                bestScore = UCB;
                bestChild = child;
            }
        }
        return bestChild;
    }

    // 扩展子节点
    //贪心策略和随机策略结合，优先扩展评估分数较高的动作，但也保留一定的随机性以增加探索
    void expand() {
        vector<vector<int>> legalActions = state->getAllActions();//待优化，getAllActions函数内部完整枚举，多次调用导致重复计算
        //剔除已经扩展过的动作
        vector<vector<int>> untried;
        for (const auto& action : legalActions) {
            bool tried = false;
            for (MCTSNode* child : children) {
                if (child->action == action) {
                    tried = true;
                    break;
                }
            }
            if (!tried) untried.push_back(action);
        }
        if (untried.empty()) return; // 没有未尝试的动作了,隐藏问题：没有合法动作时调用方不知
        //随机选择一个未尝试的动作进行扩展
        int idx = rand() % untried.size();//问题：untried动作的选择随机效率低，预计策略：对untried动作进行评分，选择评分较小的动作优先选择（添加新参量）
        vector<int> action = untried[idx];
        GameState newState = state->applyActionCopy(action);
        MCTSNode* child = new MCTSNode(newState, this, action);
        children.push_back(child);

    }


    // 模拟游戏直到结束，返回结果（胜利或失败）
    bool simulate() {
        GameState simState = *state;
        if (isUseExactSearch(simState)) {
            int maxDepth = simState.totalCards[simState.currentPlayer];
            int result = GameState::exactsearch(simState, maxDepth, state->myRole, -2, 2);
            return result == 1;
        }
        const double EPSILON = 0.1; // 10% 概率随机
        static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        uniform_real_distribution<double> dist(0.0, 1.0);
        const int MAX_SIM_STEPS = 200;
        int stepCount = 0;
        while (!simState.isGameOver && stepCount < MAX_SIM_STEPS) {
            stepCount++;
            vector<int> action;
            if (dist(rng) < EPSILON) {
                // 以一定概率选择一个随机合法动作，增加探索
                vector<vector<int>> legalActions = simState.getAllActions();
                if (!legalActions.empty()) {
                    int randomIdx = rng() % legalActions.size();
                    simState.applyActionInPlace(legalActions[randomIdx]);
                }
            }
            else {
                if (simState.isLeading()) {
                    action = getBestActionByPriority(simState.getCurrentPlayerHand());
                }
                else {
                    action = getBestActionByPriority(simState.getCurrentPlayerHand(), simState.getLastMove());
                }
                if (action.empty())action = {};
                simState.applyActionInPlace(action);
            }
        }
        if (stepCount >= MAX_SIM_STEPS) {
            // 平局处理：返回 0.5 或根据当前手牌数判断
            return false; // 保守视为失败
        }
        bool landlordWin = (simState.winner == simState.landlordRole);
        bool iAmLandlord = (simState.myRole == simState.landlordRole);
        return landlordWin == iAmLandlord;
    }

    // 反向传播结果
    void backpropagate(double result) {
        visits++;
        wins += result;
        if (parent) parent->backpropagate(result);
    }

    // ADDED: 判断节点是否已经完全扩展（所有合法动作都已生成子节点）
    bool isFullyExpanded() const {
        if (state->isGameOver) return true;   // 终局节点无需扩展
        vector<vector<int>> legalActions = state->getAllActions();
        if (legalActions.empty()) return true;
        for (const auto& act : legalActions) {
            bool hasChild = false;
            for (MCTSNode* child : children) {
                if (child->action == act) {
                    hasChild = true;
                    break;
                }
            }
            if (!hasChild) return false;
        }
        return true;
    }

    // ADDED: 获取胜率最高的子节点（用于最终决策）
    MCTSNode* bestChild() const {
        MCTSNode* best = nullptr;
        double bestWinRate = -1.0;
        for (MCTSNode* child : children) {
            if (child->visits == 0) continue;
            double winRate = (double)child->wins / child->visits;
            if (winRate > bestWinRate) {
                bestWinRate = winRate;
                best = child;
            }
        }
        if (!best && !children.empty()) best = children[0];
        return best;
    }

    // ADDED: 获取最优动作（最终返回给外部的出牌）
    vector<int> getBestAction() const {
        MCTSNode* best = bestChild();
        if (best) return best->action;
        return {};   // 无合法动作时返回空（过牌）
    }

    // ADDED: 执行一次完整的 MCTS 迭代（选择->扩展->模拟->回溯）
    void iterate() {
        MCTSNode* node = this;
        while (!node->state->isGameOver && node->isFullyExpanded()) {
            node = node->selectChild();
            if (!node) break;   // 安全保护
        }

        if (!node->state->isGameOver) {
            node->expand();

            if (!node->children.empty())
                node = node->children.back();
        }
        bool win = node->simulate();
        node->backpropagate(win ? 1.0 : 0.0);
    }
    bool isUseExactSearch(const GameState& s) {
        int total = s.totalCards[0] + s.totalCards[1] + s.totalCards[2];
        return total <= 6 && s.totalCards[s.currentPlayer] <= 3;
    }

};
vector<int> decideWithParticleFilter(GameState& rootState, int timeMs = 900) {
    for(int i=0;i<3;i++){
        cerr << "My hand for player " << i << ": ";
        for (int v = 3; v <= 17; v++) {
            for (int c = 0; c < rootState.myhand[i][v]; c++) {
                cerr << v << " ";
            }
        }
        cerr << endl;
    }
    for(int i=0;i<rootState.history.size();i++){
        cerr << "History action " << i << ": ";
        for (int v : rootState.history[i]) {
            cerr << v << " ";
        }
        cerr << endl;
    }
    for(int i=0;i<3;i++){
        cerr << "Total cards for player " << i << ": " << rootState.totalCards[i] << endl;
    }
    cerr << "Current player: " << rootState.currentPlayer << endl;
    cerr << "Landlord player: " << rootState.landlordRole << endl;
    cerr << "Is leading: " << rootState.isLeading() << endl;
    cerr << "Last move: ";
    for (int v : rootState.getLastMove()) {
        cerr << v << " ";
    }
    cerr << endl;
    
    int N_Worlds = 10;
    int itersPerWorld = 1500;
    vector<int> bestAction;
    double bestWinRate = -1.0;
    map<vector<int>, int> actionVisits;
    map<vector<int>, int> actionWins;
    int totalSims = 0;
    auto globalDeadline = chrono::steady_clock::now() + chrono::milliseconds(timeMs);
    for (int w = 0; w < N_Worlds; w++) {
        if (chrono::steady_clock::now() > globalDeadline)break;
        OpponentParticle p = ParticleFilter::sample();
        GameState worldState = rootState;
        int opp1 = (worldState.myRole + 1) % 3;
        int opp2 = (worldState.myRole + 2) % 3;
        memcpy(worldState.myhand[opp1], p.hand1, 18 * sizeof(int));
        memcpy(worldState.myhand[opp2], p.hand2, 18 * sizeof(int));
        worldState.totalCards[opp1] = std::accumulate(p.hand1 + 3, p.hand1 + 18, 0);
        worldState.totalCards[opp2] = std::accumulate(p.hand2 + 3, p.hand2 + 18, 0);
        MCTSNode rootNode(worldState);
        // for(int i=0;i<3;i++){
        // cerr << "My hand for player " << i << ": ";
        // for (int v = 3; v <= 17; v++) {
        //     for (int c = 0; c < worldState.myhand[i][v]; c++) {
        //         cerr << v << " ";
        //     }
        // }
        // cerr << endl;
    
        auto worldDeadline = chrono::steady_clock::now() + chrono::milliseconds(timeMs / N_Worlds);
        while (chrono::steady_clock::now() < worldDeadline) {
            rootNode.iterate();
            ++totalSims;
        }
        for (MCTSNode* child : rootNode.children) {
            actionVisits[child->action] += child->visits;
            actionWins[child->action] += child->wins;
        }
    }
    //   cerr << "Total simulations: " << totalSims << endl;
    for (auto& [act, visits] : actionVisits) {
        if (visits == 0) continue;
        double rate = actionWins[act] / (double)visits;
        if (rate > bestWinRate) {
            bestWinRate = rate;
            bestAction = act;
        }
    }
    return bestAction;   // 点数序列
}
int ParticleFilter::myRole = 0;              // 定义（分配内存）
vector<OpponentParticle> ParticleFilter::particles;
int main() {
    string line, all;
    while (getline(cin, line)) all += line;
    Json::Reader reader;
    Json::Value input;
    if (!reader.parse(all, input)) {
        cerr << "Failed to parse JSON" << endl;
        return 1;
    }

    int turnID = input["responses"].size();
    Json::Value request = input["requests"][turnID];

    // ---------- 叫牌阶段 ----------
    if (request.isMember("bid")) {
        // 获取当前手牌（叫牌阶段 own 字段存在）
        vector<int> hand;
        if (request.isMember("own")) {
            for (Json::UInt i = 0; i < request["own"].size(); ++i)
                hand.push_back(request["own"][i].asInt());
        }
        // 获取叫牌历史
        vector<int> bidHistory;
        if (request["bid"].isArray()) {
            for (Json::UInt i = 0; i < request["bid"].size(); ++i)
                bidHistory.push_back(request["bid"][i].asInt());
        }
        bool isFirst = bidHistory.empty();
        bool lastTwoPassed = (bidHistory.size() >= 2 && bidHistory[0] == 0 && bidHistory[1] == 0);
        int bid = CardPatternAnalysis::decideBid(hand, isFirst, lastTwoPassed);
        Json::Value ret;
        ret["response"] = bid;
        ret["data"] = input["data"];
        Json::FastWriter writer;
        cout << writer.write(ret) << endl;
        return 0;
    }

// ---------- 出牌阶段 ----------
vector<int> fullHand;            // 自己初始完整手牌（含底牌）
vector<int> publicCard;
int myPosition = -1, landlordPosition = -1;

// 1. 提取公共信息
for (int i = 0; i <= turnID; ++i) {
    Json::Value req = input["requests"][i];
    if (req.isMember("publiccard") && req.isMember("landlord")) {
        landlordPosition = req["landlord"].asInt();
        myPosition = req["pos"].asInt();
        publicCard.clear();
        for (Json::UInt j = 0; j < req["publiccard"].size(); ++j)
            publicCard.push_back(req["publiccard"][j].asInt());
        fullHand.clear();
        for (Json::UInt j = 0; j < req["own"].size(); ++j)
            fullHand.push_back(req["own"][j].asInt());
        if (landlordPosition == myPosition) {
            for (int c : publicCard) fullHand.push_back(c);
        }
        break;
    }
}
 
 
 
// 2. 收集所有真实动作序列 (allMoves) 与 自己已出的牌 (myPlayed)
vector<vector<int>> allMoves;   // 按实际出牌顺序的点数序列（空数组为过牌）
vector<int> myPlayed;           // 自己已出的牌ID列表

for (int i = 1; i < turnID; ++i) {
    Json::Value req = input["requests"][i];
    Json::Value hist = req["history"];
    bool skip = req.isMember("publiccard") && hist[0u].empty() && hist[1u].empty();
    if (!skip) {
         
        for (int k = 0; k <2; k++) {
            vector<int> move;
            for (Json::UInt j = 0; j < hist[k].size(); ++j) {
                int card = hist[k][j].asInt();
                move.push_back(CardPatternAnalysis::getCardValue(card));
            }
            
            allMoves.push_back(move);
        }
    }
    // 自己的响应
    Json::Value resp = input["responses"][i];
    if (resp.isArray() && resp.size() > 0) {
        vector<int> move;
        for (Json::UInt j = 0; j < resp.size(); ++j) {
            int card = resp[j].asInt();
            move.push_back(CardPatternAnalysis::getCardValue(card));
            myPlayed.push_back(card);   // 记录自己打出的牌
        }
        allMoves.push_back(move);
    } else {
        allMoves.push_back({});
    }
}

// 当前请求的 history（尚未执行自己的响应）
bool curPlaceholder = request.isMember("publiccard") &&
                      request["history"][0u].empty() &&
                      request["history"][1u].empty();
if (!curPlaceholder) {
    Json::Value hist = request["history"];
    // 时间顺序：先 history[1] 后 history[0]
    // 但 turnID==0 时 history[0] 是占位符，此时只有 history[1] 真实
    if (turnID == 0 && hist[0u].empty() && !hist[1u].empty()) {
        vector<int> move;
        for (Json::UInt j = 0; j < hist[1u].size(); ++j) {
            int card = hist[1u][j].asInt();
            move.push_back(CardPatternAnalysis::getCardValue(card));
        }
        allMoves.push_back(move);
    } else {
        for (int k = 0; k <2; k++) {
            vector<int> move;
            for (Json::UInt j = 0; j < hist[k].size(); ++j) {
                int card = hist[k][j].asInt();
                move.push_back(CardPatternAnalysis::getCardValue(card));
            }
            allMoves.push_back(move);
        }
    }
}
 // 农民首次请求时，删除 allMoves 中的占位空动作
if (myPosition == 1&& !allMoves.empty() && allMoves[0].empty())
    allMoves.erase(allMoves.begin());
// 3. 生成自己当前手牌（用于构造我的初始手牌，传给 GameState）
vector<int> myCurrentHand = fullHand;
for (int c : myPlayed) {
    auto it = find(myCurrentHand.begin(), myCurrentHand.end(), c);
    if (it != myCurrentHand.end()) myCurrentHand.erase(it);
}
int k=1;
for(auto& move:allMoves){
    cerr << "Move"<<k++<<":";
    for(int c:move){
        cerr << c << " ";
    }
    cerr << endl;
}

// 4. 构造 GameState 并重放历史
// 对手手牌未知，暂时给空
vector<int> initHands[3];
initHands[myPosition] = fullHand;
initHands[(myPosition + 1) % 3] = {};
initHands[(myPosition + 2) % 3] = {};
vector<vector<int>> emptyHistory;
GameState rootState(initHands, publicCard, emptyHistory, landlordPosition, myPosition);

// 设置对手总牌数（初始17/20，后续 apply 会扣除）
for (int p = 0; p < 3; ++p) {
    if (p == myPosition) continue;
    rootState.totalCards[p] = (p == landlordPosition) ? 20 : 17;
}
 
for(const auto& move : allMoves) {
    rootState.applyActionInPlace(move);
}
cerr<<rootState.currentPlayer<<endl;
cerr<<"Last move in rootState: ";
for(int c:rootState.getLastMove()){
    cerr << c << " ";
}cerr << endl;

// 5. 粒子滤波初始化（它会根据 rootState 的已知信息生成粒子）
ParticleFilter::myRole = myPosition;
ParticleFilter::initialize(rootState);

if (rootState.currentPlayer != myPosition) {
    Json::Value ret;
    ret["response"] = Json::arrayValue;
    ret["data"] = input["data"];
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
    return 0;
}

vector<int> bestAction = decideWithParticleFilter(rootState, 900);
vector<int> cardMove;
vector<int> tempHand = myCurrentHand;
for (int val : bestAction) {
    auto it = find_if(tempHand.begin(), tempHand.end(), [val](int c) {
        return CardPatternAnalysis::getCardValue(c) == val;
    });
    if (it != tempHand.end()) {
        cardMove.push_back(*it);
        tempHand.erase(it);
    } else {
        cardMove.clear();
        break;
    }
}

Json::Value ret;
Json::Value output(Json::arrayValue);
for (int c : cardMove) output.append(c);
ret["response"] = output;
ret["data"] = input["data"];
Json::FastWriter writer;
cout << writer.write(ret) << endl;
return 0;
}
//// 根据点数返回该点数的第一张牌ID（0-53）
//int cardIdByValue(int val) {
//    for (int c = 0; c <= 53; ++c) {
//        if (CardPatternAnalysis::getCardValue(c) == val)
//            return c;
//    }
//    return -1;
//}
//int main() {
//    int testCase = 1;   // 修改这里测试不同场景：1、2、3、4
//
//    // 通用变量
//    int myPos, landlordPos, opp1, opp2;
//    vector<int> initialHand;
//    vector<int> publicCard;
//    struct HistoryStep { int player; vector<int> pointMove; };
//    vector<HistoryStep> steps;
//    string caseDesc;
//
//    // 根据测试用例设置
//    switch (testCase) {
//    case 1: {
//        // 农民（非地主），手中有炸弹 4444，还有一些散牌
//        caseDesc = "农民，手中有炸弹，上家（地主）出顺子 34567";
//        myPos = 0;          // 假设我是玩家0
//        landlordPos = 1;    // 地主是玩家1
//        opp1 = 1;           // 地主（上家）
//        opp2 = 2;           // 另一个农民
//        // 我的初始手牌（17张）：炸弹4444，其他单张
//        initialHand = {
//            4,5,6,7,         // 4张4，构成炸弹
//            8,9,10,11,12,13,14, // 一些单牌
//            // 实际需要17张，补齐
//        };
//        // 为了简化，我们只放重点牌，并让 totalCards 正确
//        // 真正的17张：四个4，五个5,6,7,8,9各一张，剩6张随便填小牌
//        initialHand = {
//            4,5,6,7,        // 炸弹4444
//            3,3,3,          // 三个3（三条）
//            8,9,10,11,12,13,14, // 其他单张
//        };
//        // 需要补到17张，可以重复填充（但实际对局只能4张同点数，我们这里只测试逻辑，手牌内容不必完美）
//        publicCard = { 15,16,17 }; // 底牌（但不重要）
//        // 历史：地主先出顺子 3,4,5,6,7 （点数）
//        steps.push_back({ opp1, {3,4,5,6,7} }); // 地主出顺子
//        steps.push_back({ myPos, {} }); // 另一个农民过牌（这里假设轮到我们前，另一农民已过）
//        // 现在轮到我出牌
//        break;
//    }
//    case 2: {
//        caseDesc = "地主主动出牌，有顺子、炸弹、单张";
//        myPos = 0; landlordPos = 0; opp1 = 1; opp2 = 2;
//        // 20张合法手牌：
//        // 炸弹4444：4,5,6,7
//        // 顺子5~10：8,12,16,20,24,28
//        // 单3：0
//        // 火箭：52,53
//        // 对子J：32,33
//        // 对子Q：36,37
//        // 对子K：40,41
//        // 单A：44
//        initialHand = { 4,5,6,7, 8,12,16,20,24,28, 0, 52,53, 32,33, 36,37, 40,41, 44 };
//        publicCard = {};
//        break;
//    }
//    case 3: {
//        // 任何人，上家出火箭
//        caseDesc = "上家出火箭，只能过牌";
//        myPos = 1;
//        landlordPos = 1;
//        opp1 = 2;
//        opp2 = 0;
//        initialHand = { 30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46 }; // 随便
//        publicCard = {};
//        steps.push_back({ opp2, {16,17} }); // 上上家出火箭
//        steps.push_back({ opp1, {} });      // 上家过牌
//        break;
//    }
//    case 4: {
//        caseDesc = "跟牌：三带一 999带4 压 888带3";
//        myPos = 0; landlordPos = 1; opp1 = 1; opp2 = 2;
//        // 17张手牌（农民），包含三条9和单4，其余牌合法
//        // 三条9：24,25,26
//        // 单4：4
//        // 其余13张：3,5,6,7,8,10,J,Q,K,A 各一张，再加33(J方块),37(Q方块),41(K方块)凑对子
//        initialHand = { 24,25,26, 4, 0, 8,12,16,20,28, 32,36,40,44, 33,37,41 };
//        publicCard = {};
//        steps.push_back({ opp1, {8,8,8,3} }); // 上家出888带3
//        steps.push_back({ opp2, {} });
//        break;
//    }
//    }
//
//    // 初始化状态（同之前）
//    vector<int> initHands[3];
//    initHands[myPos] = initialHand;
//    initHands[opp1] = {};
//    initHands[opp2] = {};
//    vector<vector<int>> emptyHistory;
//    GameState rootState(initHands, publicCard, emptyHistory, landlordPos, myPos);
//    // 设置总牌数
//    rootState.totalCards[myPos] = initialHand.size();
//    rootState.totalCards[opp1] = 17;
//    rootState.totalCards[opp2] = 17;
//    for (const auto& step : steps) rootState.applyActionInPlace(step.pointMove);
//
//    ParticleFilter::myRole = myPos;
//    ParticleFilter::initialize(rootState);
//
//    cout << "===== 测试用例 " << testCase << ": " << caseDesc << " =====" << endl;
//
//    // 打印当前手牌
//    int curHand[18];
//    rootState.copyCurrentCnt(curHand);
//    cout << "手牌点数分布: ";
//    for (int v = 3; v <= 17; ++v) if (curHand[v] > 0) cout << v << "x" << curHand[v] << " ";
//    cout << endl;
//
//    // 打印合法动作
//    auto legalMoves = rootState.getAllActions();
//    cout << "合法动作数量: " << legalMoves.size() << endl;
//    for (size_t i = 0; i < legalMoves.size(); ++i) {
//        cout << "  " << i << ": ";
//        for (int v : legalMoves[i]) cout << v << " ";
//        if (legalMoves[i].empty()) cout << "(过牌)";
//        cout << endl;
//    }
//
//    // 执行决策
//    auto start = chrono::steady_clock::now();
//    vector<int> best = decideWithParticleFilter(rootState, 900);
//    auto end = chrono::steady_clock::now();
//    cout << "决策耗时: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
//    cout << "最佳动作点数序列: ";
//    for (int v : best) cout << v << " ";
//    cout << endl;
//
//    // 转换牌面id
//    vector<int> cardMove;
//    vector<int> tempHand = initialHand;
//    for (const auto& step : steps) {
//        if (step.player != myPos) continue;
//        for (int val : step.pointMove) {
//            auto it = find_if(tempHand.begin(), tempHand.end(), [val](int c) {
//                return CardPatternAnalysis::getCardValue(c) == val;
//                });
//            if (it != tempHand.end()) tempHand.erase(it);
//        }
//    }
//    for (int val : best) {
//        auto it = find_if(tempHand.begin(), tempHand.end(), [val](int c) {
//            return CardPatternAnalysis::getCardValue(c) == val;
//            });
//        if (it != tempHand.end()) {
//            cardMove.push_back(*it);
//            tempHand.erase(it);
//        }
//        else {
//            cardMove.clear();
//            break;
//        }
//    }
//    cout << "出牌牌面ID: ";
//    for (int c : cardMove) cout << c << " ";
//    cout << endl << endl;
//
//    return 0;
//}