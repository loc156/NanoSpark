

# 最前面的提醒
> 事先说明，介绍文档（.md文件）全部来自ai生成，并且我没有检查。
> 这是一个我很早之前随便写出来玩的玩具，整体看了眼是个半废品。简单分享一下
> 代码印象中可以运行，但是因为环境的改变目前无法运行，没法保证代码的正确性。（人话就是我没跑）
> 总的来说你可以理解为我从箱子底下翻出来一个我自己也不知道怎么启动的玩具，但是看了眼有点意思就发出来了。

# NanoSpark

> 一门**无跳转、多栈、靠宏扩展**的实验性栈语言。
> 核心只有 5 个容器和一条顺序执行的 token 流；控制流不靠跳转指令，而是靠**切换输入方向**和**宏递归**长出来。

**文档导航**

| 文档 | 内容 |
| --- | --- |
| [docs/DESIGN.md](docs/DESIGN.md) | 设计公理、执行模型、`IF` 的吞噬、层号机制、`runtimes` 循环、宏递归 |
| [docs/REFERENCE.md](docs/REFERENCE.md) | 全部 65 条指令、3 个元 token、宏库用法 |
| [docs/NOTES.md](docs/NOTES.md) | 已知问题、限制、陷阱，以及编译环境要求 |

---

## 这是什么

NanoSpark 最初是想写一门"能用的语言"，动手过程中方向锚定了：**底层凝固，上层用宏无限扩展**。

所以它现在**不是**一门开箱即用的实用语言，而是一个**元语言实验场**——核心给足原语，结构由上层用宏和库自己搭。

一句话概括它的形态：**有 5 个通用栈、没有 PC、没有跳转的"汇编变体"**。

## 五条设计公理

1. **顺序执行** —— 按 token 流顺序读取，没有跳转，没有 PC。
2. **多工作区** —— 主栈 + `IntStack` / `MapStack` / `StringCache` / `Cache` 四个通用容器。
3. **模式切换** —— `_TOSCACHE` / `_TOSTACK` 改变"输入方向"：切到 SCache 后，后续 token 不再执行，而是被**吞进字符串缓存**。
4. **宏** —— `words` 表存"名字 → token 数组"，宏体是**捕获下来的 token 片段**。
5. **无跳转** —— 控制流靠"条件性吞噬代码"和"宏递归"实现，不靠跳转。

其中第 3 条是整个语言的地基：**它同时提供了"引用"和"条件跳过"两种能力**，而这两件事在别的语言里分别叫"字面量捕获"和"if"。

## 开发环境

下表取自原工程的 CMake 构建记录（`CMakeCache.txt` / `toolchains-v1.json` / `compile_commands.json`）：

| 项 | 值 |
| --- | --- |
| 操作系统 | Windows |
| 编译器 | **GCC / G++ 16.1.0**（MSYS2 **UCRT64** 工具链）—— `D:\msys64\ucrt64\bin\g++.exe` |
| 语言标准 | **C++20**（`CMAKE_CXX_STANDARD 20` + `CXX_EXTENSIONS OFF` + `CXX_STANDARD_REQUIRED ON`） |
| 构建系统 | **CMake 4.4.2** —— `D:\cmake\bin\cmake.exe` |
| 生成器 | **MinGW Makefiles**，`mingw32-make` 取自同一套 UCRT64 —— `D:\msys64\ucrt64\bin\mingw32-make.exe` |
| 资源编译器 | `windres.exe`（同上） |
| 构建类型 | **Debug** |
| 实际编译选项 | `-g -std=c++20`，外加 `-DNANOSPARK_SOURCE_DIR=\"<工程路径>\"` |
| 警告开关 | **无**（`-Wall -Wextra` 之类的都没开） |
| 链接库 | `stdc++ mingw32 gcc_s gcc mingwex kernel32 pthread advapi32 shell32 user32` |

> 这是构建记录里**唯一出现过**的组合。项目在 MSVC、clang 或 Linux 下**没有**构建记录。

### ⚠️ 编译器版本是硬要求，不是建议

核心 `SparkManager::parse_value()` 用**浮点 `std::from_chars`** 来识别浮点字面量，这个重载在较老的 libstdc++ 里**不存在**：

| 工具链 | `std::from_chars` → `double` |
| --- | --- |
| GCC 16.1.0（原工程使用） | ✅ |
| MinGW-w64 GCC 11.5.0 | ❌ `no matching function for call to 'from_chars(const char*, const char*, double&)'`（`-std=c++17`、`-std=c++20` 均失败） |

这只是**工具链版本的差异**，不是代码缺陷——但请注意上面的提醒：作者没有复核过这些结论。详见 [docs/NOTES.md](docs/NOTES.md) 第 1 节。

## 快速上手

### 构建

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

编译产物是 `build/NanoSpark.exe`。

> ⚠️ 程序用 `../nanospark/main.spark` 这个**相对路径**加载脚本，所以**必须在 `build/` 目录里运行**，CWD 不能随便换。

### 运行

```bash
cd build
./NanoSpark.exe
```

启动后加载 `nanospark/main.spark`，执行完停在 `std::cin.get()`，按回车重跑一轮。

## 目录结构

```
NanoSpark/
├─ include/
│   ├─ NanoSpark.h        核心：NanoStack / value / NanoSpark / SparkManager
│   ├─ NanoPrintLib.h     print / peek / putstr / newline / dump
│   ├─ NanoStringLib.h    int2str / dbl2str / str2int / concat / strlen / streq / substr
│   ├─ NanoStackLib.h     dup / swap / over / rot / drop / depth / select
│   ├─ NanoSource.h       词法：把 .spark 文本切成 token
│   └─ NanoRunLib.h       run_file / run_string
├─ src/
│   └─ NanoSpark.cpp      commandInit()，注册全部核心指令
├─ nanospark/             用 NanoSpark 自己写的宏库
│   ├─ main.spark         入口脚本
│   ├─ while.spark        while 的宏实现
│   └─ if.spark           if / else / end_if 的宏实现（当前未被 main.spark 加载）
├─ main.cpp
└─ CMakeLists.txt
```

## 实现了什么

以下**据源码阅读**整理，作者未逐条复核：

算术、比较、逻辑、变量（`MapStack`）、输出、栈操作、宏定义（`_ADDMACRO`）、库加载（`run_file`）、动态执行（`run_string`）。

控制流在语言层也有对应实现：`while`（[`nanospark/while.spark`](nanospark/while.spark)）、`if/else` 的宏与层号嵌套机制（[`nanospark/if.spark`](nanospark/if.spark)）。二者都不是语法糖，而是**用宏和 `runtimes` 装出来的**——机制说明见 [docs/DESIGN.md](docs/DESIGN.md)。

## 它想证明什么

- **可理解性**：整个核心可以被完全理解，没有黑箱。
- **生长性**：极简原语能不能长出复杂结构。
- **认知价值**：无跳转也能实现控制流——**栈状态切换可以替代跳转**。
