#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<algorithm>
#include<unordered_map>
#include <chrono>
#include <random>
#include <cmath>
#include<variant>
#include<array>
#include<functional>
#include<cstring>
#include<map>
#include<numeric>
// #include"jsoncpp/json.h"
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
struct Rocket{};
struct Bomb{int value;};
struct Single{int value;};
struct Pair{int value;};
struct Triple{int value;};
struct Straight{int start; int len;};
struct PairSequence{int start; int len;};
struct TripleSequence{int start; int len;};
struct TripleWithOne{int triple; int single;};
struct TripleWithTwo{int triple; int pair;};
struct QuadWithSingles{int quad;int single1; int single2;};
struct QuadWithPairs{int quad; int pair1; int pair2;};
struct PlanWithSingles{int start; int len; vector<int> singles;};
struct PlanWithPairs{int start; int len; vector<int> pairs;};
using CardPattern = variant<
Rocket, Bomb, Single, Pair
, Triple, Straight, PairSequence,
 TripleSequence, TripleWithOne
, TripleWithTwo, QuadWithSingles, 
QuadWithPairs, PlanWithSingles
, PlanWithPairs>;
//====牌型检测命名空间===//
namespace PatternCheck{
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
        inline vector<vector<int>> enumerateRocket(const int cnt[18]) {
            if (cnt[16] >= 1 && cnt[17] >= 1) return {{16, 17}};
            return {};
        }
        inline vector<vector<int>> enumerateBombs(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int minVal = 3;
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 4)
                minVal = lastMovePatterns[0] + 1;
            for (int v = minVal; v <= 17; ++v) {
                if (cnt[v] >= 4) result.push_back({v, v, v, v});
            }
            return result;
        }
        inline vector<vector<int>> enumerateSingles(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int minVal = 3;
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 1)
                minVal = lastMovePatterns[0] + 1;
            for (int v = minVal; v <= 17; ++v) {
                if (cnt[v] >= 1) result.push_back({v});
            }
            return result;
        }
        inline vector<vector<int>> enumeratePairs(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int minVal = 3;
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 2)
                minVal = lastMovePatterns[0] + 1;
            for (int v = minVal; v <= 17; ++v) {
                if (cnt[v] >= 2) result.push_back({v, v});
            }
            return result;
        }
        inline vector<vector<int>> enumerateTriples(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int minVal = 3;
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 3)
                minVal = lastMovePatterns[0] + 1;
            for (int v = minVal; v <= 17; ++v) {
                if (cnt[v] >= 3) result.push_back({v, v, v});
            }
            return result;
        }
        inline vector<vector<int>> enumerateStraights(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int lastLen = 0, lastStart = 0;
            if (!lastMovePatterns.empty()) {
                lastLen = lastMovePatterns.size();
                lastStart = lastMovePatterns[0];
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
                lastStart = lastMovePatterns[0];
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
                lastStart = lastMovePatterns[0];
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
                minTriple = lastMovePatterns[0] + 1;
            for (int triple = minTriple; triple <= 17; ++triple) {
                if (cnt[triple] < 3) continue;
                for (int single = 3; single <= 17; ++single) {
                    if (single == triple) continue;
                    if (cnt[single] < 1) continue;
                    result.push_back({triple, triple, triple, single});
                }
            }
            return result;
        }
        inline vector<vector<int>> enumerateTriplesWithTwo(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int minTriple = 3;
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 5)
                minTriple = lastMovePatterns[0] + 1;
            for (int triple = minTriple; triple <= 17; ++triple) {
                if (cnt[triple] < 3) continue;
                for (int pair = 3; pair <= 17; ++pair) {
                    if (pair == triple) continue;
                    if (cnt[pair] < 2) continue;
                    result.push_back({triple, triple, triple, pair, pair});
                }
            }
            return result;
        }
        inline vector<vector<int>> enumeratePlanesWithSingles(const int cnt[18], const vector<int>& lastMovePatterns) {
            vector<vector<int>> result;
            int lastLen = 0, lastStart = 0;
            if (!lastMovePatterns.empty()) {
                lastLen = lastMovePatterns.size() / 4;
                lastStart = lastMovePatterns[0];
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
                lastStart = lastMovePatterns[0];
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
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 6)
                minQuad = lastMovePatterns[0] + 1;
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
            if (!lastMovePatterns.empty() && lastMovePatterns.size() == 8)
                minQuad = lastMovePatterns[0] + 1;
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
    static const int SINGLE=0;
    static const int PAIR=1;
    static const int TRIPLE=2;
    static const int THREE_WITH_ONE=3;
    static const int THREE_WITH_TWO=4;
    static const int TRIPLE_SEQUENCE=7;
    static const int TRIPLE_SEQUENCE_WITH_TWO_PAIRS=6;
    static const int TRIPLE_SEQUENCE_WITH_ONE=5;
    static const int STRAIGHT=9;
    static const int PAIR_SEQUENCE=8;
    static const int BOMB=10;
    static const int ROCKET=11;
    static const int QUAD_WITH_SINGLES = 12;
    static const int QUAD_WITH_PAIRS = 13;
     static int getCardType(const vector<int>& cards) {
         vector<int> patterns = divideIntoPatterns(cards);
            int cnt[18] = {0};
            for (int v : patterns) cnt[v]++;
            vector<int> emptyLast;  // 主动出牌时没有上家牌
            Rocket r;
            if (PatternCheck::check(cnt, emptyLast, r,true)) return ROCKET;
            Bomb b;
            if (PatternCheck::check(cnt, emptyLast, b,true)) return BOMB;
            Single s;
            if (PatternCheck::check(cnt, emptyLast, s,true)) return SINGLE;
            Pair p;
            if (PatternCheck::check(cnt, emptyLast, p,true)) return PAIR;
            Triple t;
            if (PatternCheck::check(cnt, emptyLast, t,true)) return TRIPLE;
            Straight st;
            if (PatternCheck::check(cnt, emptyLast, st,true)) return STRAIGHT;
            PairSequence ps;
            if (PatternCheck::check(cnt, emptyLast, ps,true)) return PAIR_SEQUENCE;
            TripleSequence ts;
            if (PatternCheck::check(cnt, emptyLast, ts,true)) return TRIPLE_SEQUENCE;
            TripleWithOne two1;
            if (PatternCheck::check(cnt, emptyLast, two1,true)) return THREE_WITH_ONE;
            TripleWithTwo two2;
            if (PatternCheck::check(cnt, emptyLast, two2,true)) return THREE_WITH_TWO;
            QuadWithSingles qws;
            if (PatternCheck::check(cnt, emptyLast, qws,true)) return QUAD_WITH_SINGLES;
            QuadWithPairs qwp;
            if (PatternCheck::check(cnt, emptyLast, qwp,true)) return QUAD_WITH_PAIRS;
            PlanWithSingles pws;
            if (PatternCheck::check(cnt, emptyLast, pws,true)) return TRIPLE_SEQUENCE_WITH_ONE;
            PlanWithPairs pwp;
            if (PatternCheck::check(cnt, emptyLast, pwp,true)) return TRIPLE_SEQUENCE_WITH_TWO_PAIRS;
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
        int res=0;
        if(card>=0&&card<=3)res=3;
        else if(card>=4&&card<=7)res=4;
        else if(card>=8&&card<=11)res=5;
        else if(card>=12&&card<=15)res=6;
        else if(card>=16&&card<=19)res=7;
        else if(card>=20&&card<=23)res=8;
        else if(card>=24&&card<=27)res=9;
        else if(card>=28&&card<=31)res=10;
        else if(card>=32&&card<=35)res=11;
        else if(card>=36&&card<=39)res=12;
        else if(card>=40&&card<=43)res=13;
        else if(card>=44&&card<=47)res=14;
        else if(card>=48&&card<=51)res=15;
        else if(card==52)res=16; // 小王
        else if(card==53)res=17; // 大王
        return res;
    }
    // 将牌转换为数值并排序，方便后续分析
    static vector<int> divideIntoPatterns(const vector<int>& cards) {
        vector<int> patterns;
        for(int card : cards) {
            patterns.push_back(getCardValue(card));
        }
        sort(patterns.begin(), patterns.end());
        return patterns;
    }
    // 分析手牌，统计每个点数的数量，并记录出现过的点数
    static void analyzeHand(const vector<int>&hand,int cnt[18],vector<int>& uniqueVals){
        memset(cnt,0,sizeof(int)*18);
        for(int card : hand) {
            int val = getCardValue(card);
            cnt[val]++;
        }
        uniqueVals.clear();
        for(int v = 3; v <= 17; ++v) {
            if(cnt[v] > 0) uniqueVals.push_back(v);
        }
    }
     static int evaluateHand(const vector<int>& hand) {
        int cnt[18] = {0};
        double score = 0;
        for (int card : hand) {
            int val = CardPatternAnalysis::getCardValue(card);
            cnt[val]++;
            if (val == 17) score += 10;
            else if (val == 16) score += 8;
            else if (val == 15) score += 5;
            else if (val == 14) score += 4;
            else if (val == 13) score += 3;
            else if (val == 12) score += 2;
            else if (val == 11) score += 1;
            else if (val == 10) score += 1;
        }
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
         if(value < 3 || value > 17) return false; // 无效点数
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
        int need[18] = {0};
        for (int v : values) {
            if (v >= 3 && v <= 17) {
                need[v]++;
            }
        }
        for (int i = 3; i <= 17; ++i) {
            if (need[i] > valueCount[i]&&need[i]>0) return {};
        }
        vector<int> res;
        for (int v : values) {
            auto cards = takeCards(v, 1);
            if(cards.empty()) return {}; // 保守起见，虽然理论上不应该出现这种情况
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
         for(int i = 3; i <= 17; ++i) {
            total += valueCount[i]; 
         }
        return total;
    }
};
// 游戏状态结构体，用于在博弈搜索（如Minimax）中传递和记录当前游戏状态
class GameState {
    public:
    vector<int> myhand[3];// 三个玩家的手牌，索引0为地主，1为农民甲，2为农民乙
    vector<int> publiccard;// 底牌
    vector<vector<int>> history;//使用轻量级数据结构，防止过多复制造成性能问题
    int myRole; // 当前玩家角色 (0:地主, 1:农民甲, 2:农民乙)
    int landlordRole; // 地主角色索引（0, 1, 2），用于判断队友和对手
    int currentPassCount; // 当前连续过牌次数，用于判断是否需要重置跟牌压力
    int currentPlayer; // 当前玩家索引（0, 1, 2），用于轮流出牌
    int lastActionPlayer; // 上一个真正的出牌玩家索引（0, 1, 2），用于判断跟牌压力是否需要重置
    bool isGameOver; // 游戏是否结束
    int winner; // 0:地主胜, 1:农民甲胜, 2:农民乙胜
    mutable bool Actions_cached=false;// 是否已经缓存过当前状态的合法动作
    mutable vector<vector<int>> Cache_Actions;// 缓存当前状态的合法动作，避免重复计算
    // 构造函数，用于初始化状态
    GameState(const vector<int> hands_[3],const vector<int>& publiccards_,const vector<vector<int>>& history_,int landlord_role_,int my_role_)
        : history(history_),publiccard(publiccards_),currentPlayer(landlord_role_),myRole(my_role_),landlordRole(landlord_role_),
        isGameOver(false), winner(-1), currentPassCount(0), lastActionPlayer(-1) {//需要优化初始化函数，暂且搁置
        // 1. 初始化手牌和底牌 (直接使用传入的参数)
        for (int i = 0; i < 3; ++i) {
            myhand[i] = hands_[i];
        }
    }
    // 状态复制构造函数，用于递归搜索
        GameState(const GameState& other)
                : history(other.history), publiccard(other.publiccard), currentPlayer(other.currentPlayer),
                myRole(other.myRole), landlordRole(other.landlordRole),
                currentPassCount(other.currentPassCount), lastActionPlayer(other.lastActionPlayer),
                isGameOver(other.isGameOver), winner(other.winner),Actions_cached(false) {
                for (int i = 0; i < 3; ++i) {
                    myhand[i] = other.myhand[i];
                }
            }
        GameState(GameState&& other) noexcept
            : myhand{ std::move(other.myhand[0]), std::move(other.myhand[1]), std::move(other.myhand[2]) }
            , publiccard(std::move(other.publiccard))
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
            
        }
        GameState& operator=(GameState&& other) noexcept {
            if (this != &other) {
                // 释放当前资源（vector 会自动释放）
                myhand[0] = std::move(other.myhand[0]);
                myhand[1] = std::move(other.myhand[1]);
                myhand[2] = std::move(other.myhand[2]);
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
                for (int card : action) {
                    auto it = find(this->myhand[this->currentPlayer].begin(), this->myhand[this->currentPlayer].end(), card);
                    if (it != this->myhand[this->currentPlayer].end()) {
                        this->myhand[this->currentPlayer].erase(it);
                    }
                }
                //更新历史
                this->history.push_back(action);
                //重置连续过牌计数，记录出牌者
                this->currentPassCount = 0;
                this->lastActionPlayer = this->currentPlayer;
                //下一家出牌
                this->currentPlayer = (this->currentPlayer + 1) % 3;
                if (this->myhand[this->lastActionPlayer].empty()) {
                    this->isGameOver = true;
                    this->winner = this->lastActionPlayer;
                }
            }
        }
	GameState applyActionCopy(const vector<int>& action) const {
            GameState newState=*this;
            newState.applyActionInPlace(action);
            return newState;
        }
    vector<int> getCurrentPlayerHand() const {return myhand[currentPlayer];}
    vector<int> getPublicCard() const {return publiccard;}
    vector<int> getLastMove() const{if(!history.empty()) return history.back();
        else return {};
    }
    bool isLeading() const {
        if(history.empty()) return true;
        else return !history.empty() && lastActionPlayer == currentPlayer;
    }
     
    vector<vector<int>> getAllActions(){
        if (Actions_cached) return Cache_Actions;
            vector<int> hand = getCurrentPlayerHand();
            HandIndex idx(hand);
            int cnt[18];
            vector<int> vals;
            CardPatternAnalysis::analyzeHand(hand, cnt, vals);
            vector<int> lastMovePatterns;
            if (!isLeading()) {
                vector<int> lastMove = getLastMove();
                if (!lastMove.empty())
                    lastMovePatterns = CardPatternAnalysis::divideIntoPatterns(lastMove);
                // 如果上家出火箭，则只能过牌
                if (lastMovePatterns.size() == 2 && lastMovePatterns[0] == 16 && lastMovePatterns[1] == 17) {
                    Cache_Actions = {{}};
                    Actions_cached = true;
                    return {{}};
                }
            }
            vector<vector<int>> allActions;
            // 定义枚举函数指针数组（顺序可任意，但建议火箭炸弹优先，不过主动出牌时全部生成即可）
            using EnumFunc = function<vector<vector<int>>(const int[18], const vector<int>&)>;
            vector<EnumFunc> funcs = {
                PatternCheck::enumerateBombs,
                PatternCheck::enumerateSingles,
                PatternCheck::enumeratePairs,
                PatternCheck::enumerateTriples,
                PatternCheck::enumerateStraights,
                PatternCheck::enumeratePairSequences,
                PatternCheck::enumerateTripleSequence,
                PatternCheck::enumerateTriplesWithOne,
                PatternCheck::enumerateTriplesWithTwo,
                PatternCheck::enumeratePlanesWithSingles,
                PatternCheck::enumeratePlanesWithPairs,
                PatternCheck::enumerateQuadWithSingles,
                PatternCheck::enumerateQuadWithPairs
            };
            // 火箭单独处理（因为不需要 lastMovePatterns）
            auto rocketCombos = PatternCheck::enumerateRocket(cnt);
            for (auto& combo : rocketCombos) {
                auto cards = idx.takeCardsByValues(combo);
                if (!cards.empty()) {allActions.push_back(cards);
                idx.putCards(cards); // 取牌后立即放回，保持索引状态不变
                }
            }
            for (auto& func : funcs) {
                if (func == nullptr) continue;
                auto combos = func(cnt, lastMovePatterns);
                for (auto& combo : combos) {
                    auto cards = idx.takeCardsByValues(combo);
                    if (!cards.empty()) {allActions.push_back(cards);
                    idx.putCards(cards); // 取牌后立即放回，保持索引状态不变
                    }
                }
            }
            if (!isLeading()) {
                allActions.push_back({}); // 过牌
            }
            // 不再去重
            Cache_Actions = allActions;
            Actions_cached = true;
            return allActions;
        }
}; 
//====按需生成器====//
vector<int> getBestActionByPriority(const vector<int>&hand,const vector<int>&lastMove={}){
    int cnt[18];
    vector<int> uniqueVals;
    CardPatternAnalysis::analyzeHand(hand, cnt, uniqueVals);
    vector<int> lastMovePatterns;
    if(!lastMove.empty()){
        lastMovePatterns = CardPatternAnalysis::divideIntoPatterns(lastMove);
    }
    using CheckFunc=function<bool(CardPattern&)>;
    vector<CheckFunc> checkers={
        [&](CardPattern& p) ->bool {Rocket r; if (PatternCheck::check(cnt, lastMovePatterns, r)) { p = r; return true; } return false;},
 [&](CardPattern& p) ->bool {Bomb b; if (PatternCheck::check(cnt, lastMovePatterns, b)) { p = b; return true; } return false;  },
 [&](CardPattern& p) ->bool {Single s; if (PatternCheck::check(cnt, lastMovePatterns, s)) { p = s; return true; } return false; },
 [&](CardPattern& p) ->bool {Pair pr; if (PatternCheck::check(cnt, lastMovePatterns, pr)) { p = pr; return true; } return false; },
 [&](CardPattern& p) ->bool {Triple t; if (PatternCheck::check(cnt, lastMovePatterns, t)) { p = t; return true; } return false; },
 [&](CardPattern& p) ->bool {Straight st; if (PatternCheck::check(cnt, lastMovePatterns, st)) { p = st; return true; } return false; },
 [&](CardPattern& p) ->bool {PairSequence ps; if (PatternCheck::check(cnt, lastMovePatterns, ps)) { p = ps; return true; } return false; },
 [&](CardPattern& p) ->bool {TripleSequence ts; if (PatternCheck::check(cnt, lastMovePatterns, ts)) { p = ts; return true; } return false; },
 [&](CardPattern& p) ->bool {TripleWithOne two1; if (PatternCheck::check(cnt, lastMovePatterns, two1)) { p = two1; return true; } return false; },
 [&](CardPattern& p) ->bool {TripleWithTwo two2; if (PatternCheck::check(cnt, lastMovePatterns, two2)) { p = two2; return true; } return false;  },
 [&](CardPattern& p) ->bool {QuadWithSingles qws; if (PatternCheck::check(cnt, lastMovePatterns, qws)) { p = qws; return true; } return false; },
 [&](CardPattern& p) ->bool {QuadWithPairs qwp; if (PatternCheck::check(cnt, lastMovePatterns, qwp)) { p = qwp; return true; } return false; },
 [&](CardPattern& p) ->bool { PlanWithSingles pws; if (PatternCheck::check(cnt, lastMovePatterns, pws)) { p = pws; return true; } return false;  },
 [&](CardPattern& p) ->bool {PlanWithPairs pwp; if (PatternCheck::check(cnt, lastMovePatterns, pwp)) { p = pwp; return true; } return false; }
    };
    for(auto& check:checkers){
        CardPattern pattern;
        if(check(pattern)){
            HandIndex idx(hand);
            return visit([&](auto&& p) -> vector<int> {
                using T = decay_t< decltype(p) >;
                if constexpr (is_same_v<T,Rocket>){
                    auto small = idx.takeCards(16,1);
                    auto big = idx.takeCards(17,1);
                    if(small.empty()||big.empty())return {};
                    return {small[0], big[0]};
                }else if constexpr (is_same_v<T,Bomb>){
                    return idx.takeCards(p.value,4);
                }else if constexpr (is_same_v<T,Single>){
                    return idx.takeCards(p.value,1);
                }else if constexpr (is_same_v<T,Pair>){
                    return idx.takeCards(p.value,2);
                }else if constexpr (is_same_v<T,Triple>){
                    return idx.takeCards(p.value,3);
                }else if constexpr (is_same_v<T,Straight>){
                    vector<int> targetVals;
                    for(int i=0;i<p.len;i++) targetVals.push_back(p.start+i);
                    return idx.takeCardsByValues(targetVals);
                }else if constexpr (is_same_v<T,PairSequence>){
                    vector<int> targetVals;
                    for(int i=0;i<p.len;i++){
                        targetVals.push_back(p.start+i);
                        targetVals.push_back(p.start+i);
                    }
                    return idx.takeCardsByValues(targetVals);
                }else if constexpr(is_same_v<T,TripleSequence>){
                    vector<int> targetVals;
                    for(int i=0;i<p.len;i++){
                        targetVals.push_back(p.start+i);
                        targetVals.push_back(p.start+i);
                        targetVals.push_back(p.start+i);
                    }
                    return idx.takeCardsByValues(targetVals);
                }else if constexpr(is_same_v<T,TripleWithOne>){
                    vector<int> targetvals(3,p.triple);
                    targetvals.push_back(p.single);
                    return idx.takeCardsByValues(targetvals);
                }else if constexpr(is_same_v<T,TripleWithTwo>){
                    vector<int> targetvals(3,p.triple);
                    targetvals.push_back(p.pair);
                    targetvals.push_back(p.pair);
                    return idx.takeCardsByValues(targetvals);
                }else if constexpr(is_same_v<T,QuadWithSingles>){
                    vector<int> targetvals(4,p.quad);
                    targetvals.push_back(p.single1);
                    targetvals.push_back(p.single2);
                    return idx.takeCardsByValues(targetvals);
                }else if constexpr(is_same_v<T,QuadWithPairs>){
                    vector<int> targetvals(4,p.quad);
                    targetvals.push_back(p.pair1);
                    targetvals.push_back(p.pair1);
                    targetvals.push_back(p.pair2);
                    targetvals.push_back(p.pair2);
                    return idx.takeCardsByValues(targetvals);
                }else if constexpr(is_same_v<T,PlanWithSingles>){
                    vector<int> targetvals;
                    for(int i=0;i<p.len;i++){
                        targetvals.push_back(p.start+i);
                        targetvals.push_back(p.start+i);
                        targetvals.push_back(p.start+i);
                    }
                    targetvals.insert(targetvals.end(), p.singles.begin(), p.singles.end());
                    return idx.takeCardsByValues(targetvals);
                }else if constexpr(is_same_v<T,PlanWithPairs>){
                    vector<int> targetvals;
                    for(int i=0;i<p.len;i++){
                        targetvals.push_back(p.start+i);
                        targetvals.push_back(p.start+i);
                        targetvals.push_back(p.start+i);
                    }
                    for(int v : p.pairs){
                        targetvals.push_back(v);
                        targetvals.push_back(v);
                    }
                    return idx.takeCardsByValues(targetvals);
                }              
                 return {};
            },pattern);
        }
    }
    return {};
}
//蒙特卡洛的实现
//1.选择基于3个标准牌的好坏程度，历史次数最多，胜负次数，公式：score = wins/visits + C*sqrt(ln(parent_visits)/visits)+a*prior，C为调节探索程度的常数，通常取1.4
//2.扩展：在选择的节点上随机选择一个未访问过的子节点进行扩展，添加到树中
//3.模拟：从新扩展的节点开始，依托现有贪心策略模拟游戏直到结束，记录结果（胜利或失败）
//4.反向传播：将模拟结果反向传播到树的节点上，更新每个节点的访问次数和胜利次数
//5.重复以上步骤，直到达到预设的迭代次数或时间限制，最终选择访问次数最多的子节点作为决策结果(最多1000次迭代)
class MCTSNode {
public:
    GameState *state;
    MCTSNode* parent;
    vector<MCTSNode*> children;
    vector<int> action; // 从父节点到当前节点的动作
    int visits;
    int wins;
    int prior; 
    int max_score; // 用于归一化先验概率的动态常数
    // 先验概率，可以根据启发式评估函数计算得到,evaluateHand函数可以用来评估当前手牌的好坏程度，作为先验概率的一部分
    //先验概率的归一化可以通过将评估分数除以一个动态常数即每个子动作的评估分数中最大的来实现，使得先验概率在0到1之间。
    MCTSNode( GameState& otherstate, MCTSNode* parent = nullptr,const vector<int>& action={}) 
        : parent(parent), visits(0), wins(0.0), max_score(0) {
            this->state = new GameState(otherstate);
            this->action = action;
        // 计算先验分数（未归一化）
        int raw_prior = CardPatternAnalysis::evaluateHand(state->myhand[state->myRole]);
        if (parent) {
            max_score = max(parent->max_score, raw_prior);
            prior = (max_score > 0) ? (raw_prior * 100) / max_score : 0;
        } else {
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
    
    // 选择子节点，基于UCB1公式
    MCTSNode* selectChild(double C = 1.4) {
        MCTSNode* bestChild = nullptr;
        double bestScore = -1e9;
        for (MCTSNode* child : children) {
           double exploit = (child->visits > 0) ? (double)child->wins / child->visits : 0.0;
            double explore = C * sqrt(log(visits + 1) / (child->visits + 1e-6));
            double UCB = exploit + explore + 0.1 * child->prior; // 加入先验概率的影响
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
          vector<vector<int>> legalActions = state->getAllActions();
          //剔除已经扩展过的动作
        vector<vector<int>> untried;
        for(const auto& action : legalActions) {
            bool tried = false;
            for(MCTSNode* child : children) {
                if(child->action == action) {
                    tried = true;
                    break;
                }
            }
            if(!tried) untried.push_back(action);
        }
        if(untried.empty()) return; // 没有未尝试的动作了
        //随机选择一个未尝试的动作进行扩展
        int idx = rand() % untried.size();
        vector<int> action = untried[idx];
        GameState newState = state->applyActionCopy(action);
        MCTSNode* child = new MCTSNode(newState, this, action);
        children.push_back(child);

    }
    
    // 模拟游戏直到结束，返回结果（胜利或失败）
   bool simulate() {
        GameState simState = *state;
        const double EPSILON = 0.1; // 10% 概率随机
        static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        uniform_real_distribution<double> dist(0.0, 1.0);
        while (!simState.isGameOver) {
            vector<int> action;
            if(simState.isLeading()){
                action=getBestActionByPriority(simState.getCurrentPlayerHand());
            }else{
                action =getBestActionByPriority(simState.getCurrentPlayerHand(), simState.getLastMove());
            }
            if(action.empty())action ={};
            simState.applyActionInPlace(action);  
            //  if (dist(rng) < EPSILON) {
            //     // 以一定概率选择一个随机合法动作，增加探索
            //     vector<vector<int>> legalActions = simState.getAllActions();
            //     if (!legalActions.empty()) {
            //         int randomIdx = rng() % legalActions.size();
            //         simState = simState.applyAction(legalActions[randomIdx]);
            //     }
            //  }
        }
        return simState.winner == state->currentPlayer;
        }
    // 反向传播结果
    void backpropagate(double result) {
        visits++;
        wins += result;
        if (parent) parent->backpropagate(result);
    }
};
#include <iomanip>
//蒙特卡洛树搜索算法的主函数
void testGetAllActions() {
    cout << "========== 斗地主动作生成性能测试 ==========" << endl;
    
    // 构造一个典型的地主手牌（21张）：包含多种牌型
    vector<int> testHand;
    // 两个3
    testHand.push_back(0); testHand.push_back(1);
    // 两个4
    testHand.push_back(4); testHand.push_back(5);
    // 两个5
    testHand.push_back(8); testHand.push_back(9);
    // 两个6
    testHand.push_back(12); testHand.push_back(13);
    // 两个7
    testHand.push_back(16); testHand.push_back(17);
    // 两个8
    testHand.push_back(20); testHand.push_back(21);
    // 9,10,J,Q,K,A,2 各一张
    testHand.push_back(24); // 9
    testHand.push_back(28); // 10
    testHand.push_back(32); // J
    testHand.push_back(36); // Q
    testHand.push_back(40); // K
    testHand.push_back(44); // A
    testHand.push_back(48); // 2
    // 小王、大王
    testHand.push_back(52);
    testHand.push_back(53);
    
    // 农民手牌（简单填充）
    vector<int> testHand2;
    for (int i = 0; i < 17; ++i) testHand2.push_back(i);
    
    vector<int> hands[3] = {testHand, testHand2, testHand2};
    vector<vector<int>> history;
    GameState state(hands, {}, history, 0, 0);
    
    auto start = chrono::high_resolution_clock::now();
    auto actions = state.getAllActions();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    
    cout << "【主动出牌测试】" << endl;
    cout << "手牌张数: " << testHand.size() << endl;
    cout << "生成合法动作数量: " << actions.size() << endl;
    cout << "耗时: " << duration.count() << " 微秒 (" << fixed << setprecision(2) << duration.count() / 1000.0 << " 毫秒)" << endl;
    
    // 统计每种牌型的数量
    map<string, int> typeCount;
    for (auto& act : actions) {
        int type = CardPatternAnalysis::getCardType(act);
       cout << "动作点数: ";
        for (int c : act) cout << CardPatternAnalysis::getCardValue(c) << " ";
        cout << "类型: " << type << endl;
        switch (type) {
            case CardPatternAnalysis::ROCKET: typeCount["火箭"]++; break;
            case CardPatternAnalysis::BOMB: typeCount["炸弹"]++; break;
            case CardPatternAnalysis::SINGLE: typeCount["单张"]++; break;
            case CardPatternAnalysis::PAIR: typeCount["对子"]++; break;
            case CardPatternAnalysis::TRIPLE: typeCount["三张"]++; break;
            case CardPatternAnalysis::STRAIGHT: typeCount["顺子"]++; break;
            case CardPatternAnalysis::PAIR_SEQUENCE: typeCount["连对"]++; break;
            case CardPatternAnalysis::TRIPLE_SEQUENCE: typeCount["飞机"]++; break;
            case CardPatternAnalysis::THREE_WITH_ONE: typeCount["三带一"]++; break;
            case CardPatternAnalysis::THREE_WITH_TWO: typeCount["三带二"]++; break;
            case CardPatternAnalysis::TRIPLE_SEQUENCE_WITH_ONE: typeCount["飞机带单"]++; break;
            case CardPatternAnalysis::TRIPLE_SEQUENCE_WITH_TWO_PAIRS: typeCount["飞机带对"]++; break;
            case CardPatternAnalysis::QUAD_WITH_SINGLES: typeCount["四带两单"]++; break;
            case CardPatternAnalysis::QUAD_WITH_PAIRS: typeCount["四带两对"]++; break;
            default: typeCount["未知"]++; break;
        }
    }
    cout << "牌型分布:" << endl;
    for (auto& p : typeCount) {
        cout << "  " << p.first << ": " << p.second << endl;
    }
    
    // 输出一些示例动作（点数表示）
    cout << "动作示例（随机5个）:" << endl;
    vector<int> indices(actions.size());
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), mt19937{random_device{}()});
    for (int i = 0; i < min(5, (int)actions.size()); ++i) {
        auto& act = actions[indices[i]];
        cout << "  ";
        for (int card : act) {
            cout << CardPatternAnalysis::getCardValue(card) << " ";
        }
        cout << endl;
    }
    // 测试跟牌情况：假设上家出了一个单张5
    cout << "\n【跟牌测试】上家出单张5（点数5）" << endl;
    vector<vector<int>> historyWithLast = { {8} }; // 假设上一轮出的是8号牌（黑桃5）
    GameState state2(hands, {}, historyWithLast, 0, 1); // 当前玩家是农民甲
    // 手动设置 lastActionPlayer 和 currentPassCount 模拟跟牌场景
    state2.lastActionPlayer = 0; // 地主出的
    state2.currentPlayer = 1;
    // 注意：state2 的构造函数中 lastActionPlayer 默认为 -1，这里需要修正，但为了测试简便，直接调用 getAllActions 会自动根据 history 判断
    // 重新构造更准确：让 history 包含上家的出牌
    GameState state3(hands, {}, {{8}}, 0, 1); // history 里有一个出牌记录
    // 但 state3 的 lastActionPlayer 仍为 -1，需要手动调整或依赖 isLeading 逻辑（history非空且 lastActionPlayer != currentPlayer）
    // 为了简单，直接调用 state3.getAllActions()，它会根据 history.back() 作为上家牌
    start = chrono::high_resolution_clock::now();
    auto actions2 = state3.getAllActions();
    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "合法动作数量: " << actions2.size() << endl;
    cout << "耗时: " << duration.count() << " 微秒 (" << duration.count() / 1000.0 << " 毫秒)" << endl;
    cout << "动作示例（前10个）:" << endl;
    for (size_t i = 0; i < min(actions2.size(), (size_t)10); ++i) {
        cout << "  ";
        for (int card : actions2[i]) {
            cout << CardPatternAnalysis::getCardValue(card) << " ";
        }
        cout << endl;
    }
    
    // 测试炸弹和火箭的情况
    cout << "\n【特殊牌型测试】手牌包含炸弹和火箭" << endl;
    vector<int> bombHand;
    // 四个3
    for (int i = 0; i < 4; ++i) bombHand.push_back(i);
    // 大小王
    bombHand.push_back(52); bombHand.push_back(53);
    // 再加一些单牌
    bombHand.push_back(4); bombHand.push_back(5);
    vector<int> handsBomb[3] = {bombHand, {}, {}};
    GameState stateBomb(handsBomb, {}, history, 0, 0);
    start = chrono::high_resolution_clock::now();
    auto actionsBomb = stateBomb.getAllActions();
    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "手牌张数: " << bombHand.size() << endl;
    cout << "合法动作数量: " << actionsBomb.size() << endl;
    cout << "耗时: " << duration.count() << " 微秒 (" << duration.count() / 1000.0 << " 毫秒)" << endl;
    bool hasRocket = false, hasBomb = false;
    for (auto& act : actionsBomb) {
        if (act.size() == 2 && CardPatternAnalysis::getCardValue(act[0]) == 16 && CardPatternAnalysis::getCardValue(act[1]) == 17)
            hasRocket = true;
        if (act.size() == 4 && CardPatternAnalysis::getCardValue(act[0]) == 3)
            hasBomb = true;
    }
    cout << "是否包含火箭: " << (hasRocket ? "是" : "否") << endl;
    cout << "是否包含炸弹(四个3): " << (hasBomb ? "是" : "否") << endl;
    
    cout << "========== 测试结束 ==========" << endl;
    exit(0); // 测试完成后直接退出，不进行正常的 Bot 交互
}
int main() {
    testGetAllActions();
    // string line, all;
    // while (getline(cin, line)) all += line;
    // Json::Reader reader;
    // Json::Value input;
    // if (!reader.parse(all, input)) {
    //     cerr << "Failed to parse JSON" << endl;
    //     return 1;
    // }

    // int turnID = input["responses"].size();
    // Json::Value request = input["requests"][turnID];

    // // ---------- 叫牌阶段 ----------
    // if (request.isMember("bid")) {
    //     // 获取当前手牌（叫牌阶段 own 字段存在）
    //     vector<int> hand;
    //     if (request.isMember("own")) {
    //         for (Json::UInt i = 0; i < request["own"].size(); ++i)
    //             hand.push_back(request["own"][i].asInt());
    //     }
    //     // 获取叫牌历史
    //     vector<int> bidHistory;
    //     if (request["bid"].isArray()) {
    //         for (Json::UInt i = 0; i < request["bid"].size(); ++i)
    //             bidHistory.push_back(request["bid"][i].asInt());
    //     }
    //     bool isFirst = bidHistory.empty();
    //     bool lastTwoPassed = (bidHistory.size() >= 2 && bidHistory[0] == 0 && bidHistory[1] == 0);
    //     int bid = CardPatternAnalysis::decideBid(hand, isFirst, lastTwoPassed);
    //     Json::Value ret;
    //     ret["response"] = bid;
    //     ret["data"] = input["data"];
    //     Json::FastWriter writer;
    //     cout << writer.write(ret) << endl;
    //     return 0;
    // }

    // // ---------- 出牌阶段 ----------
    // // 1. 恢复当前手牌和底牌（参考官方样例的维护方式）
    // vector<int> currentHand;
    // Json::Value currentPublicCard;
    // int myPosition = -1, landlordPosition = -1, landlordBid = -1;

    // // 遍历所有历史回合，恢复状态
    // for (int i = 0; i <= turnID; ++i) {
    //     Json::Value req = input["requests"][i];

    //     // 当请求包含 publiccard 时，说明这是第一回合的出牌请求（叫分结束后）
    //     if (req.isMember("publiccard") && req.isMember("landlord")) {
    //         landlordPosition = req["landlord"].asInt();
    //         landlordBid = req["finalbid"].asInt();
    //         myPosition = req["pos"].asInt();
    //         // 底牌
    //         currentPublicCard = req["publiccard"];
    //         // 手牌初始化（含底牌）
    //         if (req.isMember("own") && req["own"].size() > 0) {
    //             currentHand.clear();
    //             for (Json::UInt j = 0; j < req["own"].size(); ++j)
    //                 currentHand.push_back(req["own"][j].asInt());
    //         }
    //          if (landlordPosition == myPosition) {
    //         for (Json::UInt j = 0; j < currentPublicCard.size(); ++j)
    //             currentHand.push_back(currentPublicCard[j].asInt());
    //     }
    //     }

    //     // 如果请求包含 own 且当前手牌为空（兜底），也进行初始化
    //     if (req.isMember("own") && req["own"].size() > 0 && currentHand.empty()) {
    //         for (Json::UInt j = 0; j < req["own"].size(); ++j)
    //             currentHand.push_back(req["own"][j].asInt());
    //     }

    //     // 如果不是最后一回合，根据历史响应（自己出的牌）更新手牌
    //     if (i < turnID) {
    //         Json::Value resp = input["responses"][i];
    //         if (resp.isArray()) {
    //             for (Json::UInt j = 0; j < resp.size(); ++j) {
    //                 int card = resp[j].asInt();
    //                 auto it = find(currentHand.begin(), currentHand.end(), card);
    //                 if (it != currentHand.end()) currentHand.erase(it);
    //             }
    //         }
    //     }
    // }

    // // 2. 获取当前回合的请求（用于决策）
    // Json::Value currentRequest = input["requests"][turnID];
    // // 注意：当前请求可能没有 own 字段，但 history 和 publiccard 可能存在
    // // GameAI bot(currentRequest["history"], currentPublicCard, currentHand);
    //  vector<int> move;// = bot.decideMove();

    // // 3. 输出决策
    // Json::Value ret;
    // Json::Value output(Json::arrayValue);
    // for (int card : move) output.append(card);
    // ret["response"] = output;
    // ret["data"] = input["data"];
    // Json::FastWriter writer;
    // cout << writer.write(ret) << endl;

    return 0;
}