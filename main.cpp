#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<algorithm>
#include<unordered_map>
#include"jsoncpp/json.h"
using namespace std;
/*斗地主bot --botzone作业
规则：
1. 游戏开始时，每位玩家会得到17张牌，剩余3张牌
作为底牌，暂时不公开。
2. 游戏分为三个阶段：叫牌阶段、出牌阶段和结算阶段。
3. 叫牌阶段：玩家按照顺时针方向依次叫牌，叫牌的内容包括叫地主、抢地主和不叫。叫牌结束后，叫地主的玩家成为地主，获得底牌，并且在出牌阶段先出牌。
4. 出牌阶段：玩家按照顺时针方向依次出牌，出牌
的规则如下：
    - 玩家可以出单张牌、对子、三张牌、顺子、连对、飞机等组合。
    - 出牌必须比上一个玩家出的牌大，除非上一个玩家出的牌是炸弹或者王炸。
    - 炸弹可以打任何牌，王炸可以打任何牌，包括炸弹。
5. 结算阶段：当有玩家出完牌后，游戏结束。地主获胜的条件是所有农民都出完牌；农民获胜的条件是地主出完牌或者所有农民都出完牌。
规定：
每回合只有一个Bot会收到request。Bot收到的request是一个JSON对象，表示之前的出牌情况。格式如下：
输入：
第一个request

{
	"history": [[0, 1, 2] 上上家 , [] 上家 ],  总是两项，每一项都是数组，分别表示上上家和上家出的牌，空数组表示跳过回合或者还没轮到他。
	"publiccard": [29, 8, 14], // 地主被公开的三张牌
	"own": [0, 1, 2, 3, 4] // 自己最初拥有哪些牌
}
输出
对于history，扮演不同角色的Bot首次运行时收到的分别是：

地主收到[[], []]
农民甲收到[[], 非空数组]
农民乙收到[非空数组, [...]]
此后的request
{
	"history": [[0, 1, 2] 上上家 , [] 上家 ],  总是两项，每一项都是数组，分别表示上上家和上家出的牌，空数组表示跳过回合。
}
空数组表示跳过回合或者还没轮到他。
 
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
                int ismin = 17;
                for(int card : handPatterns) {
                    if(card > targetPatterns[0]) {
                        if(card < ismin) {
                            ismin = card;
                        }
                    }
                }
                if(ismin != 17) result=findCardValue(hand, ismin, 1);
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
            }else if(targetPatterns.size()==4){
                //三带一
                int ismin = 17;
                for(size_t i = 0; i < handPatterns.size() - 2; ++i) {
                    if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i + 1] == handPatterns[i + 2] && handPatterns[i] > targetPatterns[0]) {
                        if(handPatterns[i] < ismin) {
                            ismin = handPatterns[i];
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
                int ismin = 17;
                for(size_t i = 0; i < handPatterns.size() - 2; ++i) {
                    if(handPatterns[i] == handPatterns[i + 1] && handPatterns[i + 1] == handPatterns[i + 2] && handPatterns[i] > targetPatterns[0]) {
                        if(handPatterns[i] < ismin) {
                            ismin = handPatterns[i];
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

    // 修正构造函数：使用 Json::UInt 类型索引，避免歧义；只添加一次手牌
    GameAI(const Json::Value& history, const Json::Value& publiccard, const Json::Value& own) 
    {
        // 解析手牌
        for (Json::UInt i = 0; i < own.size(); ++i) {
            myhand.push_back(own[i].asInt());
        }
        // 解析底牌
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
        if (history.size() > 1 && history[1u].isArray()) {
            for (Json::UInt i = 0; i < history[1u].size(); ++i) {
                lastMove.push_back(history[1u][i].asInt());
            }
        }

        isleading = lastMove.empty();
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
     // 决策函数，根据当前的牌型和游戏状态，决定下一步的出牌策略
    vector<int> decideMove() {
        if(isleading) {
            return getLeadCards();
        } else {
            return getFollowCards();
        }
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
    
    // 注意：只传三个参数，不再传多余的 request
    GameAI bot(request["history"], request["publiccard"], request["own"]);
    vector<int> move = bot.decideMove();
    
    Json::Value ret;
    Json::Value output(Json::arrayValue);
    for (int card : move) {
        output.append(card);
    }
    ret["response"] = output;
    ret["data"] = input["data"];
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
    
    return 0;
}