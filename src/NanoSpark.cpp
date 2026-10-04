#include "NanoSpark.h"

void spark::SparkManager::commandInit()
{
    // 简单栈操作
    // 弹出栈顶
    commands["pop"] = [](NanoSpark &spark)
    {
        spark.pop();
    };
    // 清空栈
    commands["clear"] = [](NanoSpark &spark)
    {
        spark.clear();
    };

    // 运算相关操作
    // 加法
    commands["+"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(std::get<int>(a) + std::get<int>(b));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(std::get<double>(a) + std::get<double>(b));
        }
        else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b))
        {
            spark.push(std::get<std::string>(a) + std::get<std::string>(b));
        }
        else
        {
            error("Unsupported types for addition");
        }
    };
    // 减法
    commands["-"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(std::get<int>(a) - std::get<int>(b));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(std::get<double>(a) - std::get<double>(b));
        }
        else
        {
            error("Unsupported types for subtraction");
        }
    };
    // 乘法
    commands["*"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(std::get<int>(a) * std::get<int>(b));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(std::get<double>(a) * std::get<double>(b));
        }
        else
        {
            error("Unsupported types for multiplication");
        }
    };
    // 除法
    commands["/"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            if (std::get<int>(b) == 0)
            {
                error("Division by zero");
                return;
            }
            spark.push(std::get<int>(a) / std::get<int>(b));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            if (std::get<double>(b) == 0.0)
            {
                error("Division by zero");
                return;
            }
            spark.push(std::get<double>(a) / std::get<double>(b));
        }
        else
        {
            error("Unsupported types for division");
        }
    };
    // 取模
    commands["%"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            if (std::get<int>(b) == 0)
            {
                error("Modulo by zero");
                return;
            }
            spark.push(std::get<int>(a) % std::get<int>(b));
        }
        else
        {
            error("Unsupported types for modulo (requires integers)");
        }
    };
    // 相等
    commands["="] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(int(std::get<int>(a) == std::get<int>(b)));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(int(std::get<double>(a) == std::get<double>(b)));
        }
        else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b))
        {
            spark.push(int(std::get<std::string>(a) == std::get<std::string>(b)));
        }
        else
        {
            spark.push(0); // 不同类型视为不相等
        }
    };
    // 不等
    commands["!="] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(int(std::get<int>(a) != std::get<int>(b)));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(int(std::get<double>(a) != std::get<double>(b)));
        }
        else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b))
        {
            spark.push(int(std::get<std::string>(a) != std::get<std::string>(b)));
        }
        else
        {
            spark.push(1); // 不同类型视为不相等
        }
    };
    // 小于
    commands["<"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(int(std::get<int>(a) < std::get<int>(b)));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(int(std::get<double>(a) < std::get<double>(b)));
        }
        else
        {
            error("Unsupported types for less than comparison");
        }
    };
    // 大于
    commands[">"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(int(std::get<int>(a) > std::get<int>(b)));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(int(std::get<double>(a) > std::get<double>(b)));
        }
        else
        {
            error("Unsupported types for greater than comparison");
        }
    };
    // 小于等于
    commands["<="] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(int(std::get<int>(a) <= std::get<int>(b)));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(int(std::get<double>(a) <= std::get<double>(b)));
        }
        else
        {
            error("Unsupported types for less than or equal comparison");
        }
    };
    // 大于等于
    commands[">="] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();
        if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b))
        {
            spark.push(int(std::get<int>(a) >= std::get<int>(b)));
        }
        else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
        {
            spark.push(int(std::get<double>(a) >= std::get<double>(b)));
        }
        else
        {
            error("Unsupported types for greater than or equal comparison");
        }
    };
    // 逻辑与（两个操作数都必须是 int，0 为 false）
    commands["and"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();

        auto to_bool = [](const value &v) -> int
        {
            if (std::holds_alternative<int>(v))
                return std::get<int>(v) != 0;
            if (std::holds_alternative<double>(v))
                return std::get<double>(v) != 0.0;
            return 1; // 非空字符串视为 true
        };

        spark.push(int(to_bool(a) && to_bool(b)));
    };
    // 逻辑或
    commands["or"] = [](NanoSpark &spark)
    {
        auto b = spark.pop();
        auto a = spark.pop();

        auto to_bool = [](const value &v) -> int
        {
            if (std::holds_alternative<int>(v))
                return std::get<int>(v) != 0;
            if (std::holds_alternative<double>(v))
                return std::get<double>(v) != 0.0;
            return 1;
        };

        spark.push(int(to_bool(a) || to_bool(b)));
    };
    // 逻辑非
    commands["not"] = [](NanoSpark &spark)
    {
        auto a = spark.pop();

        auto to_bool = [](const value &v) -> int
        {
            if (std::holds_alternative<int>(v))
                return std::get<int>(v) != 0;
            if (std::holds_alternative<double>(v))
                return std::get<double>(v) != 0.0;
            return 1;
        };

        spark.push(int(!to_bool(a)));
    };

    // 字符串缓存栈（SC）操作
    // 单个栈顶元素压入SC
    commands["PUSHSC"] = [](NanoSpark &spark)
    {
        auto str = spark.pop();
        if (!std::holds_alternative<std::string>(str))
        {
            error("The stack pop is not string");
        }
        else
        {
            spark.stack.StringCache.push_back(std::get<std::string>(str));
        }
    };
    // 弹出SC顶端内容
    commands["POPSC"] = [](NanoSpark &spark)
    {
        if (spark.stack.StringCache.empty())
        {
            error("POPSC: StringCache is empty");
            return;
        }
        spark.stack.StringCache.pop_back();
    };
    // 清空SC
    commands["CLEARSC"] = [](NanoSpark &spark)
    {
        spark.stack.StringCache.clear();
    };
    // 将接下来的注入栈转到SC
    commands["_TOSCACHE"] = [](NanoSpark &spark)
    {
        spark.pointStackType = spark::PointStackType::SCache;
    };
    commands["_ADDSC_TOSTACK"] = [](NanoSpark &spark)
    {
        spark.stack.StringCache.push_back("_TOSTACK");
    };

    // Map栈操作（MS栈）
    // 向Map添加值（注：默认初始化）
    commands["addMAP"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        auto key = spark.pop();
        if (!std::holds_alternative<std::string>(key))
        {
            error("The stack pop is not string");
        }
        else
        {
            spark.stack.MapStack[std::get<std::string>(key)] = val;
        }
    };
    commands["deletedMAP"] = [](NanoSpark &spark)
    {
        auto key = spark.pop();
        if (!std::holds_alternative<std::string>(key))
        {
            error("The stack pop is not string");
        }
        else
        {
            spark.stack.MapStack.erase(std::get<std::string>(key));
        }
    };
    commands["getMAP"] = [](NanoSpark &spark)
    {
        auto key = spark.pop();
        if (!std::holds_alternative<std::string>(key))
        {
            error("The stack pop is not string");
        }
        else
        {
            auto &map = spark.stack.MapStack;
            auto it = map.find(std::get<std::string>(key));
            if (it == map.end())
            {
                error("Key not found in MapStack");
            }
            else
            {
                spark.push(it->second);
            }
        }
    };

    // 数字（IS）栈操作
    //  从主栈弹出一个 int，压入 IntStack
    commands["PUSHI"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<int>(val))
        {
            error("PUSHI: expected int");
            return;
        }
        spark.stack.IntStack.push_back(std::get<int>(val));
    };

    // 从 IntStack 弹出栈顶，压入主栈
    commands["POPI"] = [](NanoSpark &spark)
    {
        if (spark.stack.IntStack.empty())
        {
            error("POPI: IntStack is empty");
            return;
        }
        int val = spark.stack.IntStack.back();
        spark.stack.IntStack.pop_back();
        spark.push(val);
    };
    // 清空 IntStack
    commands["CLEARI"] = [](NanoSpark &spark)
    {
        spark.stack.IntStack.clear();
    };

    // 通用缓存 cache操作
    //  从主栈弹出一个值，压入 Cache
    commands["PUSHC"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        spark.stack.Cache.push_back(val);
    };
    // 从 Cache 弹出栈顶，压入主栈
    commands["POPC"] = [](NanoSpark &spark)
    {
        if (spark.stack.Cache.empty())
        {
            error("POPC: Cache is empty");
            return;
        }
        auto val = spark.stack.Cache.back();
        spark.stack.Cache.pop_back();
        spark.push(val);
    };
    // 清空 Cache
    commands["CLEARC"] = [](NanoSpark &spark)
    {
        spark.stack.Cache.clear();
    };

    // 宏操作
    // 添加宏
    commands["_ADDMACRO"] = [this](NanoSpark &spark)
    {
        auto name = spark.pop();
        if (!std::holds_alternative<std::string>(name))
        {
            error("The stack pop is not string");
        }
        else
        {
            spark::strings macro = spark.stack.StringCache;
            this->words[std::get<std::string>(name)] = macro;
        }
    };

    // 简单判断
    commands["IF"] = [this](NanoSpark &spark)
    {
        auto cond = spark.pop();
        if (!std::holds_alternative<int>(cond))
        {
            error("The stack pop is not bool");
        }
        else
        {
            if (std::get<int>(cond) == 0)
                spark.pointStackType = spark::PointStackType::SCache;
        }
    };

    // runtimes操作
    commands["runtimes_push"] = [this](NanoSpark &spark)
    {
        spark.push(this->runtimes);
    };
    commands["runtimes_pop"] = [this](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<int>(val))
        {
            error("runtimes_pop: expected int");
            return;
        }
        this->runtimes = std::get<int>(val);
    };

    // 栈互译操作
    commands["ISTACK_PUSH"] = [](NanoSpark &spark)
    {
        auto ptr = std::make_shared<std::vector<Value>>();
        for (int val : spark.stack.IntStack)
        {
            ptr->push_back(Value(val)); // int 转成 Value
        }
        spark.push(ptr);
    };
    commands["SCACHE_PUSH"] = [](NanoSpark &spark)
    {
        auto ptr = std::make_shared<std::vector<Value>>();
        for (const auto &str : spark.stack.StringCache)
        {
            ptr->push_back(Value(str)); // string 转成 Value
        }
        spark.push(ptr);
    };
    commands["CACHE_PUSH"] = [](NanoSpark &spark)
    {
        // 手动构造 vector<Value>，逐个转换
        auto vec = std::make_shared<std::vector<Value>>();
        vec->reserve(spark.stack.Cache.size());
        for (const auto &v : spark.stack.Cache)
        {
            vec->push_back(Value(v)); // value → Value
        }
        spark.push(vec);
    };
    commands["MAPSTACK_PUSH"] = [](NanoSpark &spark)
    {
        // 手动构造 map<string, Value>，逐个转换
        auto map_ptr = std::make_shared<std::unordered_map<std::string, Value>>();
        for (const auto &[key, val] : spark.stack.MapStack)
        {
            (*map_ptr)[key] = Value(val); // value → Value
        }
        spark.push(map_ptr);
    };
    commands["ISTACK_REPLACE"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<std::shared_ptr<std::vector<Value>>>(val))
        {
            error("ISTACK_REPLACE: expected array on stack top");
            return;
        }
        auto arr = std::get<std::shared_ptr<std::vector<Value>>>(val);
        spark.stack.IntStack.clear();
        for (const auto &v : *arr)
        {
            if (std::holds_alternative<int>(v.data))
            {
                spark.stack.IntStack.push_back(std::get<int>(v.data));
            }
            // 非 int 忽略
        }
    };
    commands["ISTACK_APPEND"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<std::shared_ptr<std::vector<Value>>>(val))
        {
            error("ISTACK_APPEND: expected array on stack top");
            return;
        }
        auto arr = std::get<std::shared_ptr<std::vector<Value>>>(val);
        for (const auto &v : *arr)
        {
            if (std::holds_alternative<int>(v.data))
            {
                spark.stack.IntStack.push_back(std::get<int>(v.data));
            }
        }
    };
    commands["SCACHE_REPLACE"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<std::shared_ptr<std::vector<Value>>>(val))
        {
            error("SCACHE_REPLACE: expected array on stack top");
            return;
        }
        auto arr = std::get<std::shared_ptr<std::vector<Value>>>(val);
        spark.stack.StringCache.clear();
        for (const auto &v : *arr)
        {
            if (std::holds_alternative<std::string>(v.data))
            {
                spark.stack.StringCache.push_back(std::get<std::string>(v.data));
            }
        }
    };

    commands["CACHE_REPLACE"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<std::shared_ptr<std::vector<Value>>>(val))
        {
            error("CACHE_REPLACE: expected array on stack top");
            return;
        }
        auto arr = std::get<std::shared_ptr<std::vector<Value>>>(val);
        spark.stack.Cache.clear();
        for (const auto &v : *arr)
        {
            spark.stack.Cache.push_back(v.data);
        }
    };

    commands["CACHE_APPEND"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<std::shared_ptr<std::vector<Value>>>(val))
        {
            error("CACHE_APPEND: expected array on stack top");
            return;
        }
        auto arr = std::get<std::shared_ptr<std::vector<Value>>>(val);
        for (const auto &v : *arr)
        {
            spark.stack.Cache.push_back(v.data);
        }
    };
    commands["MAPSTACK_REPLACE"] = [](NanoSpark &spark)
    {
        auto val = spark.pop();
        if (!std::holds_alternative<std::shared_ptr<std::unordered_map<std::string, Value>>>(val))
        {
            error("MAPSTACK_REPLACE: expected map on stack top");
            return;
        }
        auto map_ptr = std::get<std::shared_ptr<std::unordered_map<std::string, Value>>>(val);
        spark.stack.MapStack.clear();
        for (const auto &[key, val_obj] : *map_ptr)
        {
            spark.stack.MapStack[key] = val_obj.data;
        }
    };
}
