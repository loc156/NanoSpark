#pragma once
#include "NanoSpark.h"

namespace spark
{
    class StackLib
    {
    public:
        void Register(SparkManager& sm)
        {
            // 复制栈顶
            sm.commands["dup"] = [](NanoSpark& spark) {
                if (spark.stack.stack.empty()) {
                    error("dup: stack is empty");
                }
                spark.push(spark.top());
            };

            // 交换栈顶两个
            sm.commands["swap"] = [](NanoSpark& spark) {
                auto b = spark.pop();
                auto a = spark.pop();
                spark.push(b);
                spark.push(a);
            };

            // 复制第二个到栈顶
            sm.commands["over"] = [](NanoSpark& spark) {
                auto& s = spark.stack.stack;
                if (s.size() < 2) {
                    error("over: need at least 2 elements");
                }
                spark.push(s[s.size() - 2]);
            };

            // 旋转栈顶三个：[a][b][c] → [b][c][a]
            sm.commands["rot"] = [](NanoSpark& spark) {
                auto c = spark.pop();
                auto b = spark.pop();
                auto a = spark.pop();
                spark.push(b);
                spark.push(c);
                spark.push(a);
            };

            // 丢弃栈顶（等价 pop，但语义更清晰）
            sm.commands["drop"] = [](NanoSpark& spark) {
                spark.pop();
            };

            // 栈深度（int）压栈
            sm.commands["depth"] = [](NanoSpark& spark) {
                spark.push((int)spark.stack.stack.size());
            };

            // 条件选择：[cond][a][b] → cond ? a : b
            sm.commands["select"] = [](NanoSpark& spark) {
                auto b = spark.pop();
                auto a = spark.pop();
                auto cond = spark.pop();
                if (!std::holds_alternative<int>(cond)) {
                    error("select: condition must be int");
                }
                spark.push(std::get<int>(cond) ? a : b);
            };
        }
    };
}