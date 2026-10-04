# 指令参考

> ⚠️ 本文由 AI 生成，作者未逐行核对，内容仅供参考，不保证与代码完全一致。

共 **65 条已注册命令**，外加 **3 个元 token**（不是命令，在 `run()` 里被拦截）。

约定：栈效应一律按"栈底在左"写成 `[a][b] → [结果]`，即 **b 是栈顶**。

---

## 元 token

这 3 个名字由 `run()` 直接拦截，**注册不了、也捕获不到**（判定优先级高于 SCache 捕获）：

| Token | 作用 |
| --- | --- |
| `_TOSTACK` | 输入方向切回主栈。自身不进入 `StringCache` |
| `_RUNEND` | 结束**当前这一层** `run()`（宏展开是新的一层，所以它不等于全局停机） |
| `_RUNNEXT` | 跳过本 token |

> `_TOSTACK` 因为捕获不到，需要出现在宏体里时只能用 `_ADDSC_TOSTACK` 从 C++ 侧追加。

## 核心指令（`src/NanoSpark.cpp`，44 条）

### 栈

| 指令 | 栈效应 | 说明 |
| --- | --- | --- |
| `pop` | `[a] →` | **弹出并丢弃**主栈顶 |
| `clear` | — | 清空主栈 |

### 算术

| 指令 | 栈效应 | 说明 |
| --- | --- | --- |
| `+` | `[a][b] → [a+b]` | 额外支持 `string + string`（拼接） |
| `-` | `[a][b] → [a-b]` | 仅 int / double |
| `*` | `[a][b] → [a*b]` | 仅 int / double |
| `/` | `[a][b] → [a/b]` | 仅 int / double；除零抛异常 |
| `%` | `[a][b] → [a%b]` | **仅 int**；除零抛异常 |

两侧类型不一致一律抛 `Unsupported types for ...`。

### 比较

| 指令 | 栈效应 | 说明 |
| --- | --- | --- |
| `=` | `[a][b] → [0/1]` | 支持 int / double / string；**类型不同返回 0**（不报错） |
| `!=` | `[a][b] → [0/1]` | 同上，类型不同返回 1 |
| `<` `>` `<=` `>=` | `[a][b] → [0/1]` | 仅 int / double；类型不匹配**抛异常** |

> 类型策略不统一：`=` 宽松、`<` 严格。见 [NOTES.md](NOTES.md)。

### 逻辑

| 指令 | 栈效应 | 说明 |
| --- | --- | --- |
| `and` | `[a][b] → [0/1]` | 真值判定：int≠0 真、double≠0 真、**字符串恒真** |
| `or` | `[a][b] → [0/1]` | 同上 |
| `not` | `[a] → [0/1]` | 同上 |

> `IF` 只接受 int，而这三个接受任意类型——又一处不一致。

### 字符串缓存（StringCache）

| 指令 | 说明 |
| --- | --- |
| `PUSHSC` | `[str] →`，把主栈顶字符串压入 SC 尾部（非 string 抛异常） |
| `POPSC` | 弹出 SC 尾部（为空抛异常） |
| `CLEARSC` | 清空 SC |
| `_TOSCACHE` | 输入方向切到 SCache，后续 token 被吞为数据 |
| `_ADDSC_TOSTACK` | 往 SC 尾部追加字面量 `"_TOSTACK"` |

### Map 栈（MapStack）

| 指令 | 说明 |
| --- | --- |
| `addMAP` | `[key:string][val] →`，`Map[key] = val` |
| `deletedMAP` | `[key:string] →`，删除该键（键不存在**不报错**） |
| `getMAP` | `[key:string] → [val]`，**键不存在直接抛异常** |

### Int 栈（IntStack）

| 指令 | 说明 |
| --- | --- |
| `PUSHI` | `[int] →`，主栈 → IS |
| `POPI` | IS 顶 → 主栈（为空抛异常） |
| `CLEARI` | 清空 IS |

### 通用缓存（Cache）

| 指令 | 说明 |
| --- | --- |
| `PUSHC` | `[任意值] →`，主栈 → Cache |
| `POPC` | Cache 顶 → 主栈（为空抛异常） |
| `CLEARC` | 清空 Cache |

### 宏与条件

| 指令 | 说明 |
| --- | --- |
| `_ADDMACRO` | `[name:string] →`，把 **StringCache 的现有全部内容**注册为名为 `name` 的宏体。**不清空 SC**，调用方需自行 `CLEARSC` |
| `IF` | `[cond:int] →`。为 0 时把输入方向切到 SCache（开始吞噬）；非 0 什么都不做。非 int 抛异常 |

### 循环燃料

| 指令 | 说明 |
| --- | --- |
| `runtimes_push` | 把当前 `runtimes` 压入主栈 |
| `runtimes_pop` | 弹主栈 int 写入 `runtimes`（非 int 抛异常） |

### 容器桥

把整个容器转成一个 `shared_ptr<vector<Value>>`（或 map）压上主栈，或反向覆盖 / 追加回去。

| 系列 | 指令 |
| --- | --- |
| 压出（容器 → 主栈） | `ISTACK_PUSH` `SCACHE_PUSH` `CACHE_PUSH` `MAPSTACK_PUSH` |
| 覆盖（主栈 → 容器） | `ISTACK_REPLACE` `SCACHE_REPLACE` `CACHE_REPLACE` `MAPSTACK_REPLACE` |
| 追加（主栈 → 容器） | `ISTACK_APPEND` `CACHE_APPEND` |

> **接口不对称**：只有 `ISTACK` 和 `CACHE` 有 APPEND，没有 `SCACHE_APPEND`，也没有 `MAPSTACK_APPEND`。
> 另外 `ISTACK_*` 只接受 int 元素、`SCACHE_*` 只接受 string 元素，**其余元素被静默忽略**。

## 库指令

### `NanoPrintLib.h`（5 条）

| 指令 | 说明 |
| --- | --- |
| `print` | 弹栈顶并打印 + 换行（int / double / string，其他打印 `[unsupported]`） |
| `peek` | 打印栈顶**但不弹出**，带换行 |
| `putstr` | 弹栈顶，只接受 string，**不换行** |
| `newline` | 输出一个换行 |
| `dump` | 打印整个主栈（调试用），不弹出；容器类型显示为 `[?]` |

### `NanoStringLib.h`（7 条）

| 指令 | 说明 |
| --- | --- |
| `int2str` | `[int] → [str]` |
| `dbl2str` | `[double] → [str]` |
| `str2int` | `[str] → [int]`，非数字抛异常 |
| `concat` | `[a][b] → [a+b]`，仅两个 string |
| `strlen` | `[str] → [len]` |
| `streq` | `[a][b] → [0/1]`，仅两个 string |
| `substr` | `[str][start][len] → [sub]`，越界抛异常 |

### `NanoStackLib.h`（7 条）

| 指令 | 说明 |
| --- | --- |
| `dup` | `[a] → [a][a]` |
| `swap` | `[a][b] → [b][a]` |
| `over` | `[a][b] → [a][b][a]`，至少需要 2 个元素 |
| `rot` | `[a][b][c] → [b][c][a]` |
| `drop` | `[a] →`（与核心的 `pop` 等价） |
| `depth` | `→ [n]`，当前主栈深度 |
| `select` | `[cond:int][a][b] → [cond ? a : b]` |

### `NanoRunLib.h`（2 条）

| 指令 | 说明 |
| --- | --- |
| `run_file` | `[path:str] →`，从文件读 token 并 `run()`；路径**相对于进程 CWD** |
| `run_string` | `[code:str] →`，把字符串分词后 `run()`——**这门语言缺失的 `eval`** |

> 两者执行的都是**嵌套的一层 `run()`**，会共享 `MapStack` / `words` / `runtimes` / 输入方向。

## 词法（`NanoSource.h`）

- 空白分割，**不区分换行**；
- `"..."` 双引号包住的内容是一个完整 token，引号本身不进 token；
- `//` 到行尾是注释；
- 整数 / 浮点会被自动识别成对应类型，其余为字符串。

## 宏库（`nanospark/`）

| 文件 | 内容 | 状态 |
| --- | --- | --- |
| `main.spark` | 入口：`run_file while.spark` + `123 dump` | 被 `main.cpp` 加载 |
| `while.spark` | `while` 的宏实现，含层号与 `runtimes` 燃料机制 | 由 `main.spark` 加载 |
| `if.spark` | `if(` / `)if` / `else` / `end_if` 四个宏 | **当前无人加载，也无示例** |

> 这些 `.spark` 文件之间会**共享**同一份 `MapStack` / `words` / `StringCache` / `runtimes`，命名冲突会互相污染。
