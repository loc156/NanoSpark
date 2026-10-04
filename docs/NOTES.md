# 已知问题与限制

> ⚠️ 本文由 AI 生成，作者未逐行核对，内容仅供参考，不保证与代码完全一致。
>
> 本文是对当前代码状态的记录，不是待办清单——要不要改由设计者决定。
> 结论来自**逐行阅读源码**与**原工程的构建记录**，作者未复核。

---

## 1. 工具链版本要求：浮点 `std::from_chars`

> **这不是代码缺陷，是环境差异。** 这是据构建记录与编译对照得出的结论，作者未复核；
> 换用较老的 libstdc++ 时，会卡在下面这一行。

`include/NanoSpark.h:119` 用 `std::from_chars` 识别浮点字面量：

```cpp
double double_val;
if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), double_val); ...)
```

浮点重载**不是所有 libstdc++ 都有**。两个工具链的对照：

| 工具链 | `std::from_chars` → `double` |
| --- | --- |
| **GCC 16.1.0**（MSYS2 UCRT64，原工程使用） | ✅ 正常 |
| MinGW-w64 **GCC 11.5.0** | ❌ `no matching function for call to 'from_chars(const char*, const char*, double&)'`（`-std=c++17`、`-std=c++20` 均失败） |
| 同一 GCC 11.5.0 的 **`int` 重载** | ✅ 正常（所以只有浮点分支会出问题） |

### 原工程的完整环境

来源：原 `build/` 目录中的 `CMakeCache.txt`、`.cmake/api/v1/reply/toolchains-v1-*.json`、`compile_commands.json`。

| 项 | 值 |
| --- | --- |
| 编译器 | GCC / G++ **16.1.0**，`D:/msys64/ucrt64/bin/g++.exe`（MSYS2 UCRT64 工具链） |
| C 编译器 | `D:/msys64/ucrt64/bin/gcc.exe` 16.1.0 |
| CMake | **4.4.2**，`D:/cmake/bin/cmake.exe` |
| 生成器 | `MinGW Makefiles` |
| make | `D:/msys64/ucrt64/bin/mingw32-make.exe` |
| 资源编译器 | `D:/msys64/ucrt64/bin/windres.exe` |
| 构建类型 | `Debug` |
| 编译选项 | `-g -std=c++20 -DNANOSPARK_SOURCE_DIR=\"D:/c++/Spark\"` |
| 警告开关 | 无 |
| 头文件搜索路径 | `D:/msys64/ucrt64/include/c++/16.1.0`、`.../x86_64-w64-mingw32`、`.../backward`、`D:/msys64/ucrt64/include` 等 |

`D:/msys64` 与 `D:/cmake` 这两个目录在**当前机器上已不存在**（所以旧的 `build/` 缓存一打开就报错，工程也无法直接构建）。

### 两条出路

目前**未改动任何代码**，需要时可以二选一：

1. **换工具链**：装一个提供浮点 `from_chars` 的较新 GCC（原环境用的是 16.1.0）或 LLVM。
2. **改 5 行**：把浮点分支换成 `std::strtod` 或 `try { std::stod(s); } catch (...)`，行为等价、可移植性更好。

## 2. `runtimes` 是共享状态，且每层 `run()` 退出时都会自减

`run()` 的结尾是 `while (runtimes-- > 0)`。而宏展开（`run(words[code])`）和 `run_string` / `run_file` 都是**在同一个 `SparkManager` 上再开一层 `run()`**。

后果：

- 每一层 `run()` 退出时都执行一次 `runtimes--`，**即使那一层根本没有循环语义**；
- 所以嵌套深度本身就在消耗燃料。`while.spark` 的设计是靠循环体每轮 `1 runtimes_pop` **重新装弹**来维持循环；
- 当 `runtimes` 已经是 0 时退出，它会被减到 **−1**；层层退出会继续往下减。文档里"可能溢出 int"的说法偏轻——真实情况是**计数被多层共享并各自自减**。

## 3. `_ADDMACRO` 不清空 `StringCache`

宏体取的是 SC 的**整体快照**，但 `_ADDMACRO` 不负责清理。所有定义点都得手写 `CLEARSC`，**漏一次不会报错**，只会让下一个宏体悄悄继承上一段残渣。

## 4. 命名依赖很脆

`nanospark/while.spark` 定义循环体时，那行裸 token `while_body` 同时扮演两个角色：

- 它是新宏的**名字**；
- 但它**已经是**先前定义的"名字生成器"宏，于是会先展开，压出动态名 `while_body_1`——真正注册的名字是这个。

一旦把生成器宏改名，语义会**静默**改变（不报错，只是宏名对不上）。

## 5. `_TOSTACK` 是元指令，宏替代不了

因为 `_TOSTACK` 的判定排在 SCache 捕获之前，它**永远无法被捕获进宏体**。想在宏体末尾放一个 `_TOSTACK`，只能靠 C++ 侧的 `_ADDSC_TOSTACK` 追加。这是 `while.spark` 里那个看起来莫名其妙的 `_ADDSC_TOSTACK` 存在的唯一原因。

## 6. 全局命名空间完全共享

所有 `.spark` 文件共用同一份 `MapStack` / `words` / `StringCache` / `Cache` / `IntStack` / `runtimes` / 输入方向。**没有作用域，没有模块**，多个文件互相污染是必然的，只能靠命名约定（`if_cond_1`、`while_body_2` 这类前缀）避开。

## 7. 路径依赖 CWD

`main.cpp:31` 用的是字面量相对路径：

```cpp
auto tokens = loader.LoadTokens("../nanospark/main.spark");
```

所以**必须在 `build/` 目录里运行 exe**。

`CMakeLists.txt` 定义了 `NANOSPARK_SOURCE_DIR` 宏，但 `main.cpp` 只在 catch 块里把它打印出来，**从没用于路径解析**——这个宏目前等于白定义。

## 8. `main.cpp` 里的冗余与循环

- 第 16 行有个**全局** `spark::SparkManager sm;`，被第 25 行的局部同名变量完全遮蔽，**从未被使用**（唯一副作用是静态初始化时跑了一次 `commandInit()`）。
- 主体是 `while (1)`：每轮**重新构造** `SparkManager`、重新注册所有库、重新加载 `main.spark`，执行完停在 `std::cin.get()`。所以它更像"按回车重跑一遍"的演示壳，而不是 REPL。

## 9. 类型策略不统一

| 情形 | 行为 |
| --- | --- |
| `=` / `!=` 两侧类型不同 | 返回 0 / 1，**不报错** |
| `<` `>` `<=` `>=` 两侧类型不同 | **抛异常** |
| `IF` 的条件 | **只接受 int**，其他类型抛异常 |
| `and` / `or` / `not` 的操作数 | 接受 int / double / string（字符串恒真） |

同一门语言里三套规则，用起来需要记。

## 10. 容器桥接口不对称

- 有 `SCACHE_REPLACE`，**没有 `SCACHE_APPEND`**；
- 有 `MAPSTACK_PUSH` / `MAPSTACK_REPLACE`，**没有 `MAPSTACK_APPEND`**；
- `ISTACK_*` 静默忽略非 int 元素，`SCACHE_*` 静默忽略非 string 元素——**不报错**，只丢数据。

## 11. 没有 `eval`（但有替代品）

宏名是运行期拼出来的字符串，没法在词法阶段当 token 调用。`run_string` 承担了这个角色——它是 `eval` 的替代品，代价是多一层 `run()`（见第 2 条）。

---
