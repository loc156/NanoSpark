# 设计文档

> ⚠️ 本文由 AI 生成，作者未逐行核对，内容仅供参考，不保证与代码完全一致。
>
> 本文只讲**机制**：一个 token 进来之后发生了什么，以及控制流是怎么"长"出来的。
> 指令清单见 [REFERENCE.md](REFERENCE.md)，已知缺陷见 [NOTES.md](NOTES.md)。

---

## 1. 执行模型

全部执行逻辑浓缩在 `SparkManager::run()` 里。每读到一个 token，按**固定优先级**依次判断：

| 顺序 | 条件 | 行为 |
| --- | --- | --- |
| 1 | token == `_RUNEND` | 结束**当前这一层** `run()` |
| 2 | token == `_RUNNEXT` | 跳过本 token |
| 3 | token == `_TOSTACK` | 输入方向切回主栈，**本 token 自身不入缓存** |
| 4 | 当前方向是 `SCache` | token 原样进 `StringCache`，不执行 |
| 5 | token 是已注册命令 | 执行该命令 |
| 6 | token 是已定义宏 | **递归调用 `run(宏体)`** |
| 7 | 其他 | `parse_value()` 解析后压主栈 |

这张表就是这门语言的"元语义"，有三个结论直接决定写法：

**(a) 1–3 排在 4 前面** ⇒ `_TOSTACK` / `_RUNEND` / `_RUNNEXT` **永远无法被捕获成数据**。
这正是 `_ADDSC_TOSTACK` 存在的原因：想在宏体末尾放一个 `_TOSTACK`，只能让 C++ 侧把字符串 `"_TOSTACK"` **追加**进 `StringCache`，而不能在 `.spark` 里直接写。

**(b) `_TOSCACHE` 是命令（第 5 档），不是元 token** ⇒ 它在 SCache 模式下会被当成数据吞掉。
这个"不对称"是好事：它让**引用的嵌套**成为可能。

**(c) 命令优先于宏（5 先于 6）** ⇒ 宏不能覆盖同名命令。给宏起名 `print` 永远不会被调用。

### 宏展开是递归，不是文本替换

第 6 档执行的是 `run(words[code])`——**新开一层 `run()`**，不是把 token 拼进当前流。后果：

- `_RUNEND` 只结束那一层，不是全局停机；
- `pointStackType` 挂在 `spark` 上，**跨层共享**：宏若在 SCache 状态下返回，会把这个状态漏给调用者；
- `runtimes` 挂在 `SparkManager` 上，同样跨层共享（见 §5）。

## 2. 主栈的数值语义

裸 token 走 `parse_value()`：先试整数，再试浮点，都不成就是字符串。
带引号的 token 是字符串字面量。

因此 `123` 是 int，`1.5` 是 double，`hello` 是字符串——**一个没定义过的名字不会报错，它会变成字符串压栈**。这是宏名字生成机制能工作的前提。

`value` 是一个 `variant<int, double, string, shared_ptr<vector<Value>>, shared_ptr<map<string,Value>>>`。后两者是容器桥用来整体搬运容器的。

## 3. 引用：`_TOSCACHE` … `_TOSTACK`

```spark
my_name
_TOSCACHE
1 2 + print
_TOSTACK
_ADDMACRO
```

读法：`_TOSCACHE` 之后、`_TOSTACK` 之前的 token **不执行**，全部进 `StringCache`；`_TOSTACK` 把输入方向切回来。`_ADDMACRO` 从主栈弹一个字符串当**宏名**，把 `StringCache` 的**全部内容**注册成宏体。

这就是这门语言的"引用"语法。由于 `_TOSCACHE` 在 SCache 下会被当数据吞掉（§1(b)），**引用可以嵌套**。

几个必须记住的点：

- `_ADDMACRO` **不负责清空 `StringCache`**。每个定义点后面都要自己写 `CLEARSC`，漏一次就会让下一个宏体继承上一段残渣，而且**不报错**。
- `StringCache` 全局唯一，所以同一时刻只能有一个"待定义的宏体"。
- `_ADDMACRO` 取的是 SC 的**整体快照**，不支持"只取某一段"。

## 4. 条件跳过：`IF` 的"吞噬"

```spark
<条件>
IF
  <这段代码在条件为 0 时被吞掉>
_TOSTACK
```

`IF` 弹主栈顶（必须是 int）。若为 0，就把输入方向切到 `SCache`——于是**调用者后续的 token 一个一个被吞进 `StringCache`**，直到某个 `_TOSTACK` 把方向切回来。

关键在于：**这不是跳转**。没有 PC，没有地址，没有回填。只是"输入方向变了"，于是同一段文本从"代码"变成了"数据"（顺便被丢弃）。

这也解释了设计者那句话：**代码和数据没有界限**。

> 副作用：被吞掉的 token 会**留在 `StringCache` 里**。用 `IF` 做条件跳过时，如果不打算随后 `_ADDMACRO`，需要自己 `CLEARSC` 收拾。

## 5. `runtimes`：循环开关

`run()` 的循环条件是 `do { ... } while (runtimes-- > 0)`：

- 每层 `run()` **至少执行一遍**宏体/token 序列；
- 执行完之后若 `runtimes > 0`，**再跑一轮**；
- 循环体里写 `1 runtimes_pop` 相当于"再续一次燃料"；
- 条件为假时不续燃料，`runtimes` 保持 0，`run()` 退出。

`runtimes_push` / `runtimes_pop` **走的是主栈**：`runtimes_push` 把当前值压主栈，`runtimes_pop` 从主栈弹一个 int 写回去。

> ⚠️ 这里有个实现细节值得注意：`runtimes` 是 `SparkManager` 级的共享状态，而 `while (runtimes-- > 0)` **在每一层 `run()` 退出时都会执行一次自减**。所以嵌套的宏展开 / `run_string` 本身就在消耗燃料。`while.spark` 的设计是每轮靠 `1 runtimes_pop` 重新装弹来维持循环。详见 [NOTES.md](NOTES.md) 第 2 节。

## 6. 层号机制：动态 key

嵌套 `if` 会出现"条件 key 互相覆盖"的问题。解法是给每层分配一个**层号**，用字符串拼接出每层专属的 key：

```spark
if_depth -1 addMAP                 // 初始化层号
if_cond                            // ── 名字生成器宏
_TOSCACHE
if_cond_ if_depth getMAP int2str concat
_TOSTACK
_ADDMACRO
CLEARSC
```

这里 `if_cond` 被定义成一个**"运行时会往栈上压一个字符串"的宏**：它把 `"if_cond_"` 和当前层号拼起来，压出一个 `if_cond_-1`、`if_cond_1` 这样的**动态 key**。

调用 `if(` 时，`add_if_depth` 先把层号 +1，`if_cond` 再压出本层的 key；随后用户写条件表达式，`)if` 负责把条件 `addMAP` 进这个 key 并检查。

### 缺 eval，用 `run_string` 补

宏名是**运行期拼出来的字符串**，没法在词法阶段当 token 调用。`run_string` 就是补这个缺口：它把主栈弹出来的字符串**重新分词并执行**。

**`run_string` ≈ 这门语言缺失的 `eval`。**

## 7. `while` 是怎么长出来的

`nanospark/while.spark` 的循环体（简化）：

```spark
while_body
_TOSCACHE
  <循环体>
  <更新条件>
  while_cond getMAP
  IF
    1 runtimes_pop
  _TOSTACK              // 由 _ADDSC_TOSTACK 追加
_ADDMACRO
CLEARSC
```

展开过程：

1. `_TOSCACHE` … `_TOSTACK` 把循环体捕获成 `StringCache`；
2. `_ADDSC_TOSTACK` 往 SC 末尾追加字面量 `_TOSTACK`——因为 `_TOSTACK` 没法在源码里被捕获（§1(a)）；
3. `_ADDMACRO` 把这段（连同结尾的 `_TOSTACK`）注册成宏；
4. 执行时，条件为真就 `1 runtimes_pop` 续一次燃料，`run()` 的 `do-while` 于是再跑一轮；
5. 条件为假就不续燃料，`run()` 退出，循环结束。

一套循环，用到的是：**宏捕获 + 动态名字 + `runtimes` + `run_string`**，没有一条跳转指令。

> ⚠️ `while.spark` 里有一处极易被误读的写法：定义循环体时那行裸 token `while_body` **既是新宏的名字，又是先前定义的"名字生成器"宏**——它先展开、压出动态名 `while_body_1`，这个字符串随即被 `_ADDMACRO` 当作宏名弹走。改名会**静默**改变语义。

## 8. `if` / `else` 是怎么长出来的

`nanospark/if.spark` 定义了四个宏：`if(` / `)if` / `else` / `end_if`，配三个辅助宏（名字生成器 `if_cond`、层号 `add_if_depth` / `sub_if_depth`）。

机制是**双向吞噬**：

| 宏 | 宏体 | 作用 |
| --- | --- | --- |
| `if(` | `add_if_depth if_cond` | 层号 +1，把本层条件 key 压上主栈 |
| `)if` | `addMAP if_cond getMAP IF` | 把条件存进 key，再取出来交给 `IF` |
| `else` | `if_cond getMAP not IF` | 取条件、取反、交给 `IF` |
| `end_if` | `if_cond deletedMAP sub_if_depth` | 收掉本层 key，层号 −1 |

因为 `IF` 的条件是"取反"过的，所以：

- 条件为真 → `)if` 放行，**then 分支正常执行**；走到 `else` 时取反为 0 → **else 分支被吞掉**。
- 条件为假 → `)if` 立刻切 SCache，**then 分支被吞掉**；走到 `else` 时取反为 1 → **else 分支正常执行**。

嵌套靠层号隔离：每层有自己的 `if_cond_<n>`，互不干扰。

> ⚠️ 注意 `end_if` 的宏体里**没有** `_TOSTACK`（它进不了 `StringCache`）。所以"吞噬"只能由源码里手写的 `_TOSTACK` 或 `_ADDSC_TOSTACK` 制造的 `_TOSTACK` 来结束。
> 另外，**`if.spark` 目前没有被 `main.spark` 加载，仓库里也没有使用它的示例程序**，因此这套用法缺少可运行的证据。
