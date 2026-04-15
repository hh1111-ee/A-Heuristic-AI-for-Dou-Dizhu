#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<algorithm>
#include<unordered_map>
#include <chrono>
#include <random>
#include <cmath>
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
class CardPatternAnalysis {
private:
    static const  vector<int> reserve_value_table[18];
public:
    static const vector<int> findCardValue(const vector<int>& handcards,int value,int count) {
        vector<int> result;
        const vector<int>& candidates = reserve_value_table[value];
        for(int card : candidates) {
            if(find(handcards.begin(), handcards.end(), card) != handcards.end()) {
                result.push_back(card);
                // 在 findCardValue 中
                if(result.size() == (size_t)count) break;   
                 
            }
        }
        return result;
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
    //单牌
    static bool isSingle(const vector<int>& cards) {
        return cards.size() == 1;
    }
    //对子
    static bool  isPair(const vector<int>& cards) {
        return cards.size() == 2 && cards[0] == cards[1];
    }
    //三张牌
    static bool isTriple(const vector<int>& cards) {
        return cards.size() == 3 && cards[0] == cards[1] && cards[1] == cards[2];
    }
    static bool isThreeWithOne(const vector<int>& cards) {
        if (cards.size() != 4) return false;
        for (size_t i = 0; i < cards.size(); ++i) {
            vector<int> temp(cards);
            temp.erase(temp.begin() + i); // 去掉第i张牌
            if (isTriple(temp)) return true; // 如果剩下的三张牌是三条，则满足三带一
        }
        return false;
    }
    static bool isThreeWithTwo(const vector<int>& cards) {
        if (cards.size() != 5) return false;
        for (size_t i = 0; i < cards.size(); ++i) {
            vector<int> temp(cards);
            temp.erase(temp.begin() + i); // 去掉第i张牌
            if (isTriple(temp)) { // 如果剩下的三张牌是三条
                // 检查剩下的两张牌是否是对子
                for (size_t j = 0; j < temp.size() - 1; ++j) {
                    if (temp[j] == temp[j + 1]) return true; // 如果是对子，则满足三带二
                }
            }
        }
        return false;
    }
    //顺子
    static bool isStraight(const vector<int>& cards) {
        if (cards.size()<5) return false;
        for (size_t i = 1; i < cards.size(); ++i) {
            if (cards[i] != cards[i - 1] + 1 || cards[i] >= 15) return false; // 顺子不能包含2和王
        }
        return true;
    }
    //连对
    static bool  isPairSequence(const vector<int>& cards){
        if(cards.size() < 6 || cards.size() % 2 != 0) return false;
        for(size_t i = 0; i < cards.size(); i += 2)
        { 
            if(cards[i] != cards[i + 1]) return false;
            if(i > 0 && cards[i] != cards[i - 2] + 1) return false;
        }
        return true;
    }
    //飞机
    static bool isTripleSequence(const vector<int>& cards) {
        if (cards.size() < 6 || cards.size() % 3 != 0) return false;
        for (size_t i = 0; i < cards.size(); i += 3) {
            if (cards[i] != cards[i + 1] || cards[i + 1] != cards[i + 2]) return false;
            if(cards[i] >= 15) return false; // 飞机不能包含2和王
            if (i > 0 && cards[i] != cards[i - 3] + 1) return false;
        }
        return true;
    }
    static bool isTripleSequenceWithTwoPairs(const vector<int>& cards) {
        if (cards.size() != 8) return false;
        // 提取三条部分
        vector<int> triplePart = {cards[0], cards[3], cards[6]};
        if (!isTripleSequence(triplePart)) return false;
        // 提取剩余的两张牌
        vector<int> remaining;
        for (size_t i = 0; i < cards.size(); ++i) {
            if (i != 0 && i != 3 && i != 6) remaining.push_back(cards[i]);
        }
        // 检查剩余的两张牌是否是对子
        return remaining.size() == 2 && remaining[0] == remaining[1];
    }
    //炸弹和王炸
    static bool isBomb(const vector<int>& cards) {
        return cards.size() == 4 && cards[0] == cards[1] && cards[1] == cards[2] && cards[2] == cards[3];
    }
    static bool isRocket(const vector<int>& cards) {
        return cards.size() == 2 && ((cards[0] == 16 && cards[1] == 17) || (cards[0] == 17 && cards[1] == 16));
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
    // 根据当前牌型和目标牌型，找出可以出的更大的牌型
    static vector<int> findHigherSameType(const vector<int>& hand, const vector<int>& target) {
            vector<int> result;
            vector<int> handPatterns = divideIntoPatterns(hand);
            vector<int> targetPatterns = divideIntoPatterns(target);
            if(isSingle(targetPatterns)) {// 单牌
                int ismin = 18;
                for(int card : handPatterns) {
                    if(card > targetPatterns[0]) {
                        if(card < ismin) {
                            ismin = card;
                        }
                    }
                }
                if(ismin != 18) result=findCardValue(hand, ismin, 1);
            } else if(isPair(targetPatterns)) {// 对子  
                int ismin = 17;
                for(size_t i = 0; i < handPatterns.size() - 1; ++i) {
                    if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i] > targetPatterns[0]) {
                        if(handPatterns[i] < ismin) {
                            ismin = handPatterns[i];
                        }
                    }
                }
                if(ismin != 17) result=findCardValue(hand, {ismin, ismin});
            } else if(isTriple(targetPatterns)) {// 三张牌
                int ismin = 17;
                for(size_t i = 0; i < handPatterns.size() - 2; ++i) {
                    if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i + 1] == handPatterns[i + 2] && handPatterns[i] > targetPatterns[0]) {
                        if(handPatterns[i] < ismin) {
                            ismin = handPatterns[i];
                        }
                    }
                }
                if(ismin != 17) result=findCardValue(hand, ismin, 3);
            }else if(targetPatterns.size()==4&&!isBomb(targetPatterns)){
                //三带一
                int ismin = 17;
                int tripleVal = -1;
                for(int v : targetPatterns) {
                    if(count(targetPatterns.begin(), targetPatterns.end(), v) == 3) {
                        tripleVal = v;
                        break;
                    }
                }
                if(tripleVal == -1) return {}; // 不是合法的三带一
                for (size_t i = 0; i < handPatterns.size()-2; i++)
                {
                     if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i + 1] == handPatterns[i + 2] && handPatterns[i] > tripleVal) {
                        if(handPatterns[i] < ismin) {
                            ismin = handPatterns[i];
                            break; // 找到最小满足的三张
                        }
                    }
                }
                
                if(ismin != 17) {
                    vector<int> targetValues = {ismin, ismin, ismin};
                    for(int val : handPatterns) {
                        if(val != ismin && CardPatternAnalysis::findCardValue(hand, val, 1).size() == 1) {
                            targetValues.push_back(val);
                            break;
                        }
                    }
                    vector<int> res = findCardValue(hand, targetValues);
                    if (!res.empty()) result = res;
                }
                

            }else if(targetPatterns.size()==5&&!isStraight(targetPatterns)){
                //三带二
                 // 提取上家三带二中的三张点数
                int tripleVal = -1;
                for (int v : targetPatterns) {
                    if (count(targetPatterns.begin(), targetPatterns.end(), v) == 3) {
                        tripleVal = v;
                        break;
                    }
                }
                if (tripleVal == -1) return {}; // 不是合法的三带二
                
                // 在手牌中找比 tripleVal 大的最小三张
                int ismin = 17;
                for (size_t i = 0; i < handPatterns.size() - 2; ++i) {
                    if (handPatterns[i] == handPatterns[i+1] && handPatterns[i+1] == handPatterns[i+2] && handPatterns[i] > tripleVal) {
                        if (handPatterns[i] < ismin){ ismin = handPatterns[i];
                        break; // 找到最小满足的三张
                        }
                    }
                }
                if(ismin != 17) {
                    vector<int> targetValues = {ismin, ismin, ismin};
                    for(size_t i = 0; i < handPatterns.size() - 1; ++i) {
                        if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i] != ismin) {
                            targetValues.push_back(handPatterns[i]);
                            targetValues.push_back(handPatterns[i]);
                            break;
                        }
                    }
                    vector<int> res = findCardValue(hand, targetValues);
                    if (!res.empty()) result = res;
                }

            } else if(isStraight(targetPatterns)) {// 顺子
               int len = targetPatterns.size();   // 获取顺子长度
                int ismin = 17;
                // 在手牌中搜索长度为 len 的更大顺子
                for (size_t i = 0; i + len <= handPatterns.size(); ++i) {
                    vector<int> candidate(handPatterns.begin() + i, handPatterns.begin() + i + len);
                    if (isStraight(candidate) && candidate[0] > targetPatterns[0]) {
                        if (candidate[0] < ismin) ismin = candidate[0];
                    }
                }
                if (ismin != 17) {
                    vector<int> targetValues;
                    for (int j = 0; j < len; ++j) targetValues.push_back(ismin + j);
                    vector<int> res = findCardValue(hand, targetValues);
                    if (!res.empty()) result = res;
                }
            }else if(isPairSequence(targetPatterns)) {// 连对
                int length = targetPatterns.size();
                int ismin = 17;
                for(size_t i = 0; i <= handPatterns.size() - length + 1; ++i) {
                    vector<int> candidate(handPatterns.begin() + i, handPatterns.begin() + i + length);
                    if(isPairSequence(candidate) && candidate[0] > targetPatterns[0]) {
                        if(candidate[0] < ismin) {
                            ismin = candidate[0];
                        }
                    }
                }
                if(ismin != 17) {
                vector<int> targetValues;
                for (int j = 0; j < length / 2; ++j) {
                    targetValues.push_back(ismin + j);
                    targetValues.push_back(ismin + j);
                }
                vector<int> res = findCardValue(hand, targetValues);
                if (!res.empty()) result = res;
                }
            } else if(isTripleSequence(targetPatterns)) {// 飞机
                int length = targetPatterns.size();
                int ismin = 17;
                for(size_t i = 0; i <= handPatterns.size() - length + 1; ++i) {
                    vector<int> candidate(handPatterns.begin() + i, handPatterns.begin() + i + length);
                    if(isTripleSequence(candidate) && candidate[0] > targetPatterns[0]) {
                        if(candidate[0] < ismin) {
                            ismin = candidate[0];
                        }
                    }
                }
                if(ismin != 17) {
                  int groups = length / 3;
                    vector<int> targetValues;
                    for (int j = 0; j < groups; ++j) {
                        for (int k = 0; k < 3; ++k) {
                            targetValues.push_back(ismin + j);
                        }
                    }
                    vector<int> res = findCardValue(hand, targetValues);
                    if (!res.empty()) result = res;
                }
            }else if(targetPatterns.size()==8&&isTripleSequence({targetPatterns[0],targetPatterns[1],targetPatterns[2], targetPatterns[3],targetPatterns[4],targetPatterns[5], targetPatterns[6]})) {
                // 飞机带两对
                int length = targetPatterns.size();
                int ismin = 17;
                for(size_t i = 0; i <= handPatterns.size() - length + 1; ++i) {
                    vector<int> candidate(handPatterns.begin() + i, handPatterns.begin() + i + length);
                    if(isTripleSequence({candidate[0], candidate[3], candidate[6]}) && candidate[0] > targetPatterns[0]) {
                        if(candidate[0] < ismin) {
                            ismin = candidate[0];
                        }
                    }
                }
                if(ismin != 17) {
                    vector<int> targetValues;
                    for (int j = 0; j < 2; ++j) {
                        for (int k = 0; k < 3; ++k) {
                            targetValues.push_back(ismin + j);
                        }
                    }
                    int pairCount = 0;
                    for(size_t i = 0; i < handPatterns.size() - 1; ++i) {
                        if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i] != ismin && handPatterns[i] != ismin + 1) {
                            targetValues.push_back(handPatterns[i]);
                            targetValues.push_back(handPatterns[i]);
                            pairCount++;
                            if(pairCount == 2) break;
                        }
                    }
                    vector<int> res = findCardValue(hand, targetValues);
                    if (!res.empty()) result = res;
                }


            } else if(isBomb(targetPatterns)) {// 炸弹
                int ismin = 17;
                
                for(size_t i = 0; i < handPatterns.size() - 3; ++i) {
                    vector<int> candidate(handPatterns.begin() + i, handPatterns.begin() + i + 4);
                    if(isBomb(candidate) && candidate[0] > targetPatterns[0]) {
                        if(candidate[0] < ismin) {
                            ismin = candidate[0];
                        }
                    }
                }
                if(ismin != 17) result=findCardValue(hand, {ismin, ismin, ismin, ismin});
            } 
            
        return result.empty() ? vector<int>() : result;
    }
    static int getCardType(const vector<int>& cards) {
        vector<int> patterns = divideIntoPatterns(cards);
        if (isRocket(patterns)) return 10; // 火箭
        if (isBomb(patterns)) return 9; // 炸弹
        if (isStraight(patterns)) return 8; // 顺子
        if (isPairSequence(patterns)) return 7; // 连对
        if (isTripleSequence(patterns)) return 6; // 飞机
        if (isTripleSequenceWithTwoPairs(patterns)) return 5; // 飞机带两对
        if (isThreeWithTwo(patterns)) return 4; // 三带二
        if (isThreeWithOne(patterns)) return 3; // 三带一
        if (isTriple(patterns)) return 2; // 三张牌
        if (isPair(patterns)) return 1; // 对子
        if (isSingle(patterns)) return 0; // 单牌
        return -1; // 非法牌型
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
// Bot的核心逻辑类，可以在这里实现各种牌型的分析和出牌策略
class GameAI{
public:
    vector<int> myhand;
    vector<int> publiccard;
    CardPatternAnalysis patternAnalyzer;
    int role; // 0:地主, 1:农民甲, 2:农民乙
    vector<int> lastMove;
    bool isleading;
    //默认构造函数
    GameAI() : role(0), isleading(true) {}
    //适配蒙特卡洛树搜索的构造函数，接受 `own` 为 vector<int>，避免在调用处进行重复转换
    GameAI(const vector<vector<int>>&history,vector<int> publiccard,const vector<int>& own) 
    {
        // 解析手牌（直接使用传入的向量）
        myhand = own;
        // 解析底牌
        this->publiccard.clear();
        for (int i = 0; i < publiccard.size(); ++i) {
            this->publiccard.push_back(publiccard[i]);
        }

        // 判断角色
        bool prvePrevEmpty = (history.size() > 0 && !history[0].empty());
        bool prevEmpty = (history.size() > 1 && !history[1].empty());
        if (prvePrevEmpty && prevEmpty) role = 0;
        else if (prvePrevEmpty && !prevEmpty) role = 1;
        else role = 2;

        // 记录上家出的牌
        // 记录需要压的牌（上家优先，上家没出则压上上家）
        if (history.size() > 1 && !history[1].empty()) {
            // 上家出了牌，压上家
            for (int i = 0; i < history[1u].size(); ++i) {
                lastMove.push_back(history[1][i]);
            }
        } else if (history.size() > 0 && !history[0].empty()) {
            // 上家过牌，但上上家出了牌，压上上家
            for (int i = 0; i < history[0u].size(); ++i) {
                lastMove.push_back(history[0][i]);
            }
        }
        isleading = lastMove.empty();
    }
    // 修正构造函数：接受 `own` 为 vector<int>，避免在调用处进行重复转换
    GameAI(const Json::Value& history, const Json::Value& publiccard, const vector<int>& own) 
    {
        // 解析手牌（直接使用传入的向量）
        myhand = own;
        // 解析底牌
        this->publiccard.clear();
        for (Json::UInt i = 0; i < publiccard.size(); ++i) {
            this->publiccard.push_back(publiccard[i].asInt());
        }

        // 判断角色
        bool prvePrevEmpty = (history.size() > 0 && history[0u].isArray() && history[0u].size() == 0);
        bool prevEmpty = (history.size() > 1 && history[1u].isArray() && history[1u].size() == 0);
        if (prvePrevEmpty && prevEmpty) role = 0;
        else if (prvePrevEmpty && !prevEmpty) role = 1;
        else role = 2;

        // 记录上家出的牌
        // 记录需要压的牌（上家优先，上家没出则压上上家）
        if (history.size() > 1 && history[1u].isArray() && history[1u].size() > 0) {
            // 上家出了牌，压上家
            for (Json::UInt i = 0; i < history[1u].size(); ++i) {
                lastMove.push_back(history[1u][i].asInt());
            }
        } else if (history.size() > 0 && history[0u].isArray() && history[0u].size() > 0) {
            // 上家过牌，但上上家出了牌，压上上家
            for (Json::UInt i = 0; i < history[0u].size(); ++i) {
                lastMove.push_back(history[0u][i].asInt());
            }
        }
        isleading = lastMove.empty();
    }
    int getRole() const {
        return role;
    }
 vector<int> getLeadCards(){
            // 0. 尝试出火箭
            auto smallJoker = CardPatternAnalysis::findCardValue(myhand, 16, 1);
            if (!smallJoker.empty()) {
                auto bigJoker = CardPatternAnalysis::findCardValue(myhand, 17, 1);
                if (!bigJoker.empty()) {
                    return {smallJoker[0], bigJoker[0]};
                }
            }
            vector<int> handvals = CardPatternAnalysis::divideIntoPatterns(myhand);
            // 1. 尝试出飞机（优先带翅膀，否则纯飞机）
                for (size_t i = 0; i + 5 < handvals.size(); ++i) {
                    // 尝试不同长度（从最长开始）
                    for (int len = (handvals.size() - i) / 3 * 3; len >= 6; len -= 3) {
                        if (i + len > handvals.size()) continue;
                        vector<int> candidate(handvals.begin() + i, handvals.begin() + i + len);
                        if (CardPatternAnalysis::isTripleSequence(candidate)) {
                            // 飞机主体点数列表（如 [5,5,5,6,6,6]）
                            vector<int> bodyVals = candidate;
                            vector<int> bodyCards = CardPatternAnalysis::findCardValue(myhand, bodyVals);
                            if (bodyCards.size() != bodyVals.size()) continue;

                            // 拷贝手牌并移除主体牌
                            vector<int> tempHand = myhand;
                            for (int c : bodyCards) {
                                auto it = find(tempHand.begin(), tempHand.end(), c);
                                if (it != tempHand.end()) tempHand.erase(it);
                            }

                            int groupCount = len / 3;   // 飞机有几组（例如 333444 为2组）
                            // 优先尝试带对子（三带二）
                            vector<int> wingPairs;
                            vector<int> tempVals = CardPatternAnalysis::divideIntoPatterns(tempHand);
                            for (int j = 0; j + 1 < tempVals.size(); ++j) {
                                if (tempVals[j] == tempVals[j+1]) {
                                    vector<int> aPair = CardPatternAnalysis::findCardValue(tempHand, tempVals[j], 2);
                                    if (aPair.size() == 2) {
                                        wingPairs.insert(wingPairs.end(), aPair.begin(), aPair.end());
                                        // 从 tempHand 中移除这对牌
                                        for (int c : aPair) {
                                            auto it = find(tempHand.begin(), tempHand.end(), c);
                                            if (it != tempHand.end()) tempHand.erase(it);
                                        }
                                        j += 2; // 跳过已使用的对子
                                        if (wingPairs.size() / 2 == groupCount) break;
                                    }
                                }
                            }
                            if (wingPairs.size() / 2 == groupCount) {
                                vector<int> result = bodyCards;
                                result.insert(result.end(), wingPairs.begin(), wingPairs.end());
                                return result;
                            }

                            // 若对子不够，尝试带单牌（三带一）
                            vector<int> wingSingles;
                            tempVals = CardPatternAnalysis::divideIntoPatterns(tempHand);
                            for (int v : tempVals) {
                                vector<int> aSingle = CardPatternAnalysis::findCardValue(tempHand, v, 1);
                                if (!aSingle.empty()) {
                                    wingSingles.push_back(aSingle[0]);
                                    // 从 tempHand 中移除这张牌
                                    auto it = find(tempHand.begin(), tempHand.end(), aSingle[0]);
                                    if (it != tempHand.end()) tempHand.erase(it);
                                    if (wingSingles.size() == groupCount) break;
                                }
                            }
                            if (wingSingles.size() == groupCount) {
                                vector<int> result = bodyCards;
                                result.insert(result.end(), wingSingles.begin(), wingSingles.end());
                                return result;
                            }

                            // 若无法配齐翅膀，则出纯飞机（不带翼）
                            return bodyCards;
                        }
                    }
                }
            //2.尝试出炸弹
            for(size_t i=0;i+3<handvals.size();i++){
                if(handvals[i]==handvals[i+3]){
                    vector<int> res;
                    res=CardPatternAnalysis::findCardValue(myhand, {handvals[i], handvals[i], handvals[i], handvals[i]});
                    
                    if(!res.empty()) return res;
                }
            }
            //3.尝试出顺子
            int max_len=12;// 顺子最长只能到A
            for(int len=max_len;len>=5;len--){
                 for(size_t i=0;i+len<=handvals.size();i++){
                vector<int> candidate(handvals.begin() + i, handvals.begin() + i + len);
                if(CardPatternAnalysis::isStraight(candidate)){
                    vector<int> res;
                    res=CardPatternAnalysis::findCardValue(myhand, candidate); 
                    if(res.size()==candidate.size())
                    return res;
                }
                
                }
            }
            //尝试三张牌，包括三带一和三带二
            for(size_t i=0;i+2<handvals.size();i++){
                vector<int> candidate(handvals.begin() + i, handvals.begin() + i + 3);
                if(CardPatternAnalysis::isTriple(candidate)){
                    // 先尝试三带二'
                    vector<int> pairCandidate;
                   for(size_t i=0;i+1<handvals.size();i++){
                     if(handvals[i]==handvals[i+1]&&handvals[i]!=candidate[0]){
                    pairCandidate={handvals[i], handvals[i]};
                    break;
                    }
                    }
                    //尝试三带一
                    if(pairCandidate.empty())
                        for(int val : handvals){
                            if(val!=candidate[0]&&CardPatternAnalysis::findCardValue(myhand, val, 1).size()==1){
                                pairCandidate={val};
                                break;
                            }
                    }
                    vector<int> res;
                    candidate.insert(candidate.end(), pairCandidate.begin(), pairCandidate.end());
                    res=CardPatternAnalysis::findCardValue(myhand, candidate); 
                    if(res.size()==candidate.size())
                    return res;
                }

            }
            //尝试出连对
            for(int len=max_len;len>=6;len-=2){
                 for(size_t i=0;i+len<=handvals.size();i++){
                vector<int> candidate(handvals.begin() + i, handvals.begin() + i + len);
                if(CardPatternAnalysis::isPairSequence(candidate)){
                    vector<int> res;
                    res=CardPatternAnalysis::findCardValue(myhand, candidate); 
                    if(res.size()==candidate.size())
                    return res;
                }
                }
            }
            //4.尝试出对子
            for(size_t i=0;i+1<handvals.size();i++){
                if(handvals[i]==handvals[i+1]){
                    return CardPatternAnalysis::findCardValue(myhand, {handvals[i], handvals[i]});
                }
            }
            //5.尝试出最小单牌
            if(!handvals.empty()){
                int minval=17;
                for(int val : handvals){
                    if(val<minval) minval=val;
                }
                return CardPatternAnalysis::findCardValue(myhand, minval, 1);

            }
            return {};
        }
    
    //跟牌策略
    vector<int> getFollowCards(){
        // 根据上家的牌型和牌值，尝试出一手比上家更大的牌
         if (CardPatternAnalysis::isRocket(CardPatternAnalysis::divideIntoPatterns(lastMove))) {
        return {};
        }
        vector<int> move = patternAnalyzer.findHigherSameType(myhand, lastMove);
        if(!move.empty()) {
            return move;
        }
        vector<int> handvals = CardPatternAnalysis::divideIntoPatterns(myhand);
        for(size_t i=0;i+3<handvals.size();i++){
            if(handvals[i]==handvals[i+3]){
                return CardPatternAnalysis::findCardValue(myhand, handvals[i], 4);
            }
        }
        auto rocket = CardPatternAnalysis::findCardValue(myhand, 16, 1);
        if(!rocket.empty()){
            auto bigRocket = CardPatternAnalysis::findCardValue(myhand, 17, 1);
            if(!bigRocket.empty()){
                return {rocket[0], bigRocket[0]};
            }
        }
        // 如果实在没有牌可以出，就只能跳过了
        return {};
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
     // 决策函数，根据当前的牌型和游戏状态，决定下一步的出牌策略
    vector<int> decideMove() {
        if(isleading) {
            return getLeadCards();
        } else {
            return getFollowCards();
        }
    }
};
// 游戏状态结构体，用于在博弈搜索（如Minimax）中传递和记录当前游戏状态
class GameState {
    public:
    // 玩家手牌
    vector<int> myhand[3];
    // 地主公开的底牌
    vector<int> publiccard;
    // 完整的出牌历史记录
    vector<vector<int>> history;//使用轻量级数据结构，防止过多复制造成性能问题
    // 当前玩家角色 (0:地主, 1:农民甲, 2:农民乙)
    int myRole;
    int landlordRole; // 地主角色索引（0, 1, 2），用于判断队友和对手
   
     
    int currentPassCount; // 当前连续过牌次数，用于判断是否需要重置跟牌压力
    int currentPlayer; // 当前玩家索引（0, 1, 2），用于轮流出牌
    int lastActionPlayer; // 上一个真正的出牌玩家索引（0, 1, 2），用于判断跟牌压力是否需要重置
     
     
    // 游戏是否结束
    bool isGameOver;
    int winner; // 0:地主胜, 1:农民甲胜, 2:农民乙胜

    // 构造函数，用于初始化状态
    GameState(const vector<int> hands_[3],const vector<int>& publiccards_,const vector<vector<int>>& history_,int landlord_role_,int my_role_)
        : history(history_),publiccard(publiccards_),currentPlayer(landlord_role_),myRole(my_role_),landlordRole(landlord_role_),
        isGameOver(false), winner(-1) {//需要优化初始化函数，暂且搁置
        
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
          isGameOver(other.isGameOver), winner(other.winner) {
        for (int i = 0; i < 3; ++i) {
            myhand[i] = other.myhand[i];
        }
    }
     GameState applyAction(const vector<int>& action) {//状态转移函数，根据玩家的出牌动作，生成新的游戏状态
        GameState newState=*this;
        // 1. 从玩家手牌中移除出牌
        if(action.empty()){
            // 过牌，增加连续过牌计数
            newState.currentPassCount++;
            if(newState.currentPassCount >= 2) {
                // 如果连续两人过牌，重置跟牌压力
                newState.currentPlayer=newState.lastActionPlayer;
                newState.currentPassCount=0;
            }else{
                newState.currentPlayer=(newState.currentPlayer+1)%3;
            }
        }else{
            //出牌从当前的手牌中移除出牌
            for(int card : action){
                auto it = find(newState.myhand[newState.currentPlayer].begin(), newState.myhand[newState.currentPlayer].end(), card);
                if(it != newState.myhand[newState.currentPlayer].end()) {
                    newState.myhand[newState.currentPlayer].erase(it);
                }
            }
            //更新历史
            newState.history.push_back(action);
            //重置连续过牌计数，记录出牌者
            newState.currentPassCount=0;
            newState.lastActionPlayer=newState.currentPlayer;
            //下一家出牌
            newState.currentPlayer=(newState.currentPlayer+1)%3;
            if(newState.myhand[newState.lastActionPlayer].empty()) {
                newState.isGameOver=true;
                newState.winner=(newState.lastActionPlayer==newState.landlordRole) ? 0 : 1;
            }
        }
        return newState;
    }
    vector<int> getCurrentPlayerHand() const {
        return myhand[currentPlayer];
    }
    vector<int> getPublicCard() const {
        return publiccard;
    }
    vector<int> getLastMove() const{
        if(!history.empty()) return history.back();
        else return {};
    }
    bool isLeading() const {
        if(history.empty()) return true;
        else return !history.empty() && lastActionPlayer == currentPlayer;
    }
    // 生成当前手牌的所有单张
        vector<vector<int>> generateSingles(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            for (int v : vals) {
                auto cards = CardPatternAnalysis::findCardValue(hand, v, 1);
                if (!cards.empty()) result.push_back(cards);
            }
            return result;
        }

        // 生成所有对子
        vector<vector<int>> generatePairs(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            for (size_t i = 0; i + 1 < vals.size(); ++i) {
                if (vals[i] == vals[i+1]) {
                    auto cards = CardPatternAnalysis::findCardValue(hand, vals[i], 2);
                    if (!cards.empty()) result.push_back(cards);
                    i++; // 跳过下一个
                }
            }
            return result;
        }

        // 生成所有三张
        vector<vector<int>> generateTriples(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            for (size_t i = 0; i + 2 < vals.size(); ++i) {
                if (vals[i] == vals[i+2]) {
                    auto cards = CardPatternAnalysis::findCardValue(hand, vals[i], 3);
                    if (!cards.empty()) result.push_back(cards);
                    i += 2;
                }
            }
            return result;
        }

        // 生成所有炸弹
        vector<vector<int>> generateBombs(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            for (size_t i = 0; i + 3 < vals.size(); ++i) {
                if (vals[i] == vals[i+3]) {
                    auto cards = CardPatternAnalysis::findCardValue(hand, vals[i], 4);
                    if (!cards.empty()) result.push_back(cards);
                    i += 3;
                }
            }
            return result;
        }

        // 生成所有顺子（长度5~12）
        vector<vector<int>> generateStraights(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            // 去除重复点数
            vector<int> uniqueVals;
            for (size_t i = 0; i < vals.size(); ++i) {
                if (i == 0 || vals[i] != vals[i-1]) uniqueVals.push_back(vals[i]);
            }
            for (int len = 12; len >= 5; --len) {
                for (size_t i = 0; i + len <= uniqueVals.size(); ++i) {
                    bool isStraight = true;
                    for (int j = 0; j < len; ++j) {
                        if (uniqueVals[i+j] != uniqueVals[i] + j) {
                            isStraight = false;
                            break;
                        }
                    }
                    if (isStraight) {
                        vector<int> targetVals;
                        for (int j = 0; j < len; ++j) targetVals.push_back(uniqueVals[i] + j);
                        auto cards = CardPatternAnalysis::findCardValue(hand, targetVals);
                        if (cards.size() == targetVals.size()) result.push_back(cards);
                    }
                }
            }
            return result;
        }

        // 生成所有连对（至少3对）
        vector<vector<int>> generatePairSequences(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            // 统计每种点数的个数
            int cnt[18] = {0};
            for (int v : vals) cnt[v]++;
            for (int len = 6; len <= 12; len += 2) { // 长度6,8,10,12 对应3,4,5,6对
                for (int start = 3; start + len/2 - 1 <= 14; ++start) {
                    bool ok = true;
                    for (int j = 0; j < len/2; ++j) {
                        if (cnt[start + j] < 2) { ok = false; break; }
                    }
                    if (ok) {
                        vector<int> targetVals;
                        for (int j = 0; j < len/2; ++j) {
                            targetVals.push_back(start + j);
                            targetVals.push_back(start + j);
                        }
                        auto cards = CardPatternAnalysis::findCardValue(hand, targetVals);
                        if (cards.size() == targetVals.size()) result.push_back(cards);
                    }
                }
            }
            return result;
        }

        // 生成所有飞机（至少2个连续三张）
        vector<vector<int>> generatePlanes(const vector<int>& hand) {
            vector<vector<int>> result;
            vector<int> vals = CardPatternAnalysis::divideIntoPatterns(hand);
            int cnt[18] = {0};
            for (int v : vals) cnt[v]++;
            for (int len = 6; len <= 12; len += 3) { // 长度6,9,12 对应2,3,4组
                for (int start = 3; start + len/3 - 1 <= 14; ++start) {
                    bool ok = true;
                    for (int j = 0; j < len/3; ++j) {
                        if (cnt[start + j] < 3) { ok = false; break; }
                    }
                    if (ok) {
                        vector<int> targetVals;
                        for (int j = 0; j < len/3; ++j) {
                            for (int k = 0; k < 3; ++k) targetVals.push_back(start + j);
                        }
                        auto cards = CardPatternAnalysis::findCardValue(hand, targetVals);
                        if (cards.size() == targetVals.size()) result.push_back(cards);
                    }
                }
            }
            return result;
        }

        // 生成火箭
        vector<vector<int>> generateRocket(const vector<int>& hand) {
            auto small = CardPatternAnalysis::findCardValue(hand, 16, 1);
            if (!small.empty()) {
                auto big = CardPatternAnalysis::findCardValue(hand, 17, 1);
                if (!big.empty()) return {{small[0], big[0]}};
            }
            return {};
        }
    vector<vector<vector<int>>> getAllActions(){
        vector<vector<vector<int>>> allActions;
        if(isLeading()){
            allActions.push_back(generateRocket(getCurrentPlayerHand()));
            allActions.push_back(generateBombs(getCurrentPlayerHand()));
            allActions.push_back(generatePlanes(getCurrentPlayerHand()));
            allActions.push_back(generatePairSequences(getCurrentPlayerHand()));
            allActions.push_back(generateStraights(getCurrentPlayerHand()));
            allActions.push_back(generateTriples(getCurrentPlayerHand()));
            allActions.push_back(generatePairs(getCurrentPlayerHand()));
            allActions.push_back(generateSingles(getCurrentPlayerHand()));
            if(getCurrentPlayerHand().empty()) allActions.push_back({}); // 过牌也是合法动作
        }else{
            // 跟牌时只考虑能压过上家的牌型
            vector<int> lastMove = getLastMove();
            int lastType = CardPatternAnalysis::getCardType(lastMove);
            // 根据lastType调用相应的生成函数，并过滤出能压过lastMove的动作
             // 这里需要实现一个过滤函数，判断生成的动作是否能压过lastMove
        }
         return allActions;
    }
    static vector<vector<int>> getLegalActions(const GameState& state) {
        // 根据当前状态，生成所有合法的出牌动作
        if(state.isleading){}
        vector<vector<int>> legalActions;
         
        return legalActions;
    }
    static vector<vector<int>> greedyPolicy(const GameState& state) {
        // 生成基于启发式评估的贪心合法动作列表，优先考虑评估分数较高的动作
        GameAI tempAI(state.history, state.publiccard, state.myhand);
        vector<int> leadCards = tempAI.getLeadCards();
        vector<int> followCards = tempAI.getFollowCards();
        vector<vector<int>> legalActions;
        if (state.isleading) {
            if (!leadCards.empty()) legalActions.push_back(leadCards);
        } else {
            if (!followCards.empty()) legalActions.push_back(followCards);
            legalActions.push_back({}); // 过牌也是合法动作
        }
        return legalActions;
    }

};
//蒙特卡洛的实现
//1.选择基于3个标准牌的好坏程度，历史次数最多，胜负次数，公式：score = wins/visits + C*sqrt(ln(parent_visits)/visits)+a*prior，C为调节探索程度的常数，通常取1.4
//2.扩展：在选择的节点上随机选择一个未访问过的子节点进行扩展，添加到树中
//3.模拟：从新扩展的节点开始，依托现有贪心策略模拟游戏直到结束，记录结果（胜利或失败）
//4.反向传播：将模拟结果反向传播到树的节点上，更新每个节点的访问次数和胜利次数
//5.重复以上步骤，直到达到预设的迭代次数或时间限制，最终选择访问次数最多的子节点作为决策结果(最多1000次迭代)
class MCTSNode {
public:
    GameState state;
    GameAI* ai; // 每个节点维护一个GameAI实例，用于评估当前状态和生成子状态
    MCTSNode* parent;
    vector<MCTSNode*> children;
    int visits;
    int wins;
    int prior; 
    int max_score; // 用于归一化先验概率的动态常数
    // 先验概率，可以根据启发式评估函数计算得到,evaluateHand函数可以用来评估当前手牌的好坏程度，作为先验概率的一部分
    //先验概率的归一化可以通过将评估分数除以一个动态常数即每个子动作的评估分数中最大的来实现，使得先验概率在0到1之间。
    MCTSNode(const GameState& state, MCTSNode* parent = nullptr) 
        : state(state), parent(parent), visits(0), wins(0), max_score(0) {
        // 计算先验概率
        if(parent) {
            prior = GameAI::evaluateHand(state.myhand);
            max_score = max(parent->max_score,max_score, prior);
            if (max_score > 0) {
                prior = (prior * 100) / max_score; // 归一化到0-100
            } else {
                prior = 0;
            }
        } else {
            prior = GameAI::evaluateHand(state.myhand);
            max_score = prior;
             if (max_score > 0) {
                prior = (prior * 100) / max_score; // 归一化到0-100
            } else {
                prior = 0;
        }
        }
        ai = new GameAI(state.history, state.publiccard, state.myhand);
    }
    
    ~MCTSNode() {
        for (MCTSNode* child : children) {
            delete child;
        }
    }
    
    // 选择子节点，基于UCB1公式
    MCTSNode* selectChild(double C = 1.4) {
        MCTSNode* bestChild = nullptr;
        double bestScore = -1.0;
        for (MCTSNode* child : children) {
            double score = (child->wins / (child->visits + 1e-6)) + C * sqrt(log(visits + 1) / (child->visits + 1e-6))+ 0.1 * child->prior; // 加入先验概率
            if (score > bestScore) {
                bestScore = score;
                bestChild = child;
            }
        }
        return bestChild;
    }
    
    // 扩展子节点
    //贪心策略和随机策略结合，优先扩展评估分数较高的动作，但也保留一定的随机性以增加探索
    void expand() {
         vector<int> moves = ai->decideMove(); // 生成当前状态的合法动作
        // 对每个合法动作，生成一个新的GameState并创建一个新的MCTS
    }
    
    // 模拟游戏直到结束，返回结果（胜利或失败）
    bool simulate() {
            
    }
    
    // 反向传播结果
    void backpropagate(bool win) {
        visits++;
        if (win) wins++;
        if (parent) parent->backpropagate(win);
    }
};
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
        int bid = GameAI::decideBid(hand, isFirst, lastTwoPassed);
        Json::Value ret;
        ret["response"] = bid;
        ret["data"] = input["data"];
        Json::FastWriter writer;
        cout << writer.write(ret) << endl;
        return 0;
    }

    // ---------- 出牌阶段 ----------
    // 1. 恢复当前手牌和底牌（参考官方样例的维护方式）
    vector<int> currentHand;
    Json::Value currentPublicCard;
    int myPosition = -1, landlordPosition = -1, landlordBid = -1;

    // 遍历所有历史回合，恢复状态
    for (int i = 0; i <= turnID; ++i) {
        Json::Value req = input["requests"][i];

        // 当请求包含 publiccard 时，说明这是第一回合的出牌请求（叫分结束后）
        if (req.isMember("publiccard") && req.isMember("landlord")) {
            landlordPosition = req["landlord"].asInt();
            landlordBid = req["finalbid"].asInt();
            myPosition = req["pos"].asInt();
            // 底牌
            currentPublicCard = req["publiccard"];
            // 手牌初始化（含底牌）
            if (req.isMember("own") && req["own"].size() > 0) {
                currentHand.clear();
                for (Json::UInt j = 0; j < req["own"].size(); ++j)
                    currentHand.push_back(req["own"][j].asInt());
            }
             if (landlordPosition == myPosition) {
            for (Json::UInt j = 0; j < currentPublicCard.size(); ++j)
                currentHand.push_back(currentPublicCard[j].asInt());
        }
        }

        // 如果请求包含 own 且当前手牌为空（兜底），也进行初始化
        if (req.isMember("own") && req["own"].size() > 0 && currentHand.empty()) {
            for (Json::UInt j = 0; j < req["own"].size(); ++j)
                currentHand.push_back(req["own"][j].asInt());
        }

        // 如果不是最后一回合，根据历史响应（自己出的牌）更新手牌
        if (i < turnID) {
            Json::Value resp = input["responses"][i];
            if (resp.isArray()) {
                for (Json::UInt j = 0; j < resp.size(); ++j) {
                    int card = resp[j].asInt();
                    auto it = find(currentHand.begin(), currentHand.end(), card);
                    if (it != currentHand.end()) currentHand.erase(it);
                }
            }
        }
    }

    // 2. 获取当前回合的请求（用于决策）
    Json::Value currentRequest = input["requests"][turnID];
    // 注意：当前请求可能没有 own 字段，但 history 和 publiccard 可能存在
    GameAI bot(currentRequest["history"], currentPublicCard, currentHand);
    vector<int> move = bot.decideMove();

    // 3. 输出决策
    Json::Value ret;
    Json::Value output(Json::arrayValue);
    for (int card : move) output.append(card);
    ret["response"] = output;
    ret["data"] = input["data"];
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;

    return 0;
}