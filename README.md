# RISC-V Tomasulo CPU Simulator

用 C++20 实现的 RV32I 乱序执行 CPU 模拟器，采用 Tomasulo 算法 + 1-bit 分支预测。

---

## 1. 作业目标

- 实现**乱序执行（out-of-order）** 的 Tomasulo 算法：指令可以不等前序指令执行完，只要操作数就绪就执行；但**取指与提交仍然是顺序的**。
- 指令集为 **RV32I**（可参考 `reference/reference-card.pdf`，实现除 `ebreak`、`ecall`之外的指令）。
- 需要实现**分支预测**并统计预测准确率。
- 访存指令需模拟**硬件延迟返回**（不能直接立即使用内存/全局变量的瞬时值）。
- 统计花费的**时钟周期数**。
- 数据内存与指令内存的读写可以**同时进行**。

---

## 2. 目录结构

```
├── main.cpp                    # 主入口
├── naive-main.cpp              # 单周期参考实现入口
├── test.sh                     # 批量测试脚本
├── CMakeLists.txt
├── src/                        # Tomasulo 核心源码
│   ├── tomasulo.hpp            # 顶层控制器，整合流水线阶段
│   ├── alu.hpp                 # ALU 计算单元
│   ├── branch_predictor.hpp    # 1-bit 分支预测器
│   ├── cdb.hpp                 # 公共数据总线 (CDB)
│   ├── decoder.hpp             # RV32I 指令译码器
│   ├── fetch.hpp               # 取指单元
│   ├── load_store_queue.hpp    # Load/Store 队列 (含 store-to-load forwarding)
│   ├── memory.hpp              # 内存 (std::map 模拟)
│   ├── regfile.hpp             # 寄存器文件 + 重命名状态表
│   ├── reorder_buffer.hpp      # 重排序缓冲区 (ROB)
│   └── reservation_station.hpp # 保留站 (RS)
├── naive-src/                  # 单周期参考源码
├── data/testcases/             # 18 个测试用例 (.c / .data / .dump)
├── data/sample/                # 示例用例
├── doc/                        # 设计文档
├── reference/                  # RISC-V 参考资料
└── ppt/                        # 课程讲义
```

---

## 3. 下发数据格式

每个测试程序都包含**三个同名文件**：

| 文件 | 含义 | 用途 |
|------|------|------|
| `.c`   | 人类可读的 C 源码 | 理解程序在算什么（如求 pi、gcd、快排…） |
| `.data`| 机器码的**文本十六进制**表示 | **你的 simulator 的实际输入** |
| `.dump`| `objdump` 反汇编 | 便于对照每条机器码对应的汇编指令 |

### `.data` 格式

以 `@` 开头的行给出一个**加载地址**（十六进制），其后的若干行是**空格分隔的十六进制字节**，依次写入从该地址开始的连续内存。例如：

```
@00000000
37 01 02 00 EF 10 00 04 13 05 F0 0F B7 06 03 00
23 82 A6 00 6F F0 9F FF
@00001000
37 17 00 00 83 27 C7 06 ...
```

表示：从 `0x00000000` 写入 `37 01 02 00 …`，从 `0x00001000` 写入后续字节。

---
---

## 4. 元件清单

| 模块 | 文件 | 职责 |
|------|------|------|
| Decoder | `src/decoder.hpp` | RV32I 指令译码 |
| ALU | `src/alu.hpp` | 执行所有计算指令 |
| RegFile | `src/regfile.hpp` | 32 寄存器 + 重命名状态表 |
| Reservation Station | `src/reservation_station.hpp` | 8 条保留站，双缓冲 |
| Reorder Buffer | `src/reorder_buffer.hpp` | 8 条 ROB，环形队列 |
| Load/Store Queue | `src/load_store_queue.hpp` | 8 条 LSQ，含 store-to-load forwarding |
| Common Data Bus | `src/cdb.hpp` | 每周期一条广播 |
| Fetch | `src/fetch.hpp` | 从内存取指 |
| Branch Predictor | `src/branch_predictor.hpp` | 1024 条目 1-bit 预测器 |
| Memory | `src/memory.hpp` | `std::map` 模拟内存 |

## 5. 流水线阶段（每周期执行顺序可交换）

```
commit → writeback → cdb_listen → execute → issue → tick(old←new)
```

各模块维护 `old_*` / `new_*` 双缓冲，阶段间只读写本周期状态，`tick()` 统一更新，模拟硬件并行。

## 6. 构建方法

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_STANDARD=20
make code        # Tomasulo 版本
make naive-code  # 单周期参考实现
```

## 7. 批量测试

```bash
./test.sh
```

输出四列表格：

```
Testcase             Cycles       Predictions  Accuracy     Status
--------             ------       -----------  --------     ------
array_test1          217          44           50.00%       PASS
multiarray           1613         261          68.97%       PASS
...
```

## 测试结果

18/18 全过：

| 测试用例 | Cycles | 预测数 | 准确率 |
|---------|--------|--------|--------|
| array_test1 | 217 | 44 | 50.00% |
| array_test2 | 243 | 50 | 54.00% |
| basicopt1 | 590,140 | 190,750 | 66.00% |
| bulgarian | 391,721 | 91,458 | 82.71% |
| expr | 559 | 123 | 80.49% |
| gcd | 570 | 171 | 57.89% |
| hanoi | 185,379 | 28,207 | 65.47% |
| lvalue2 | 57 | 14 | 42.86% |
| magic | 625,926 | 89,279 | 71.03% |
| manyarguments | 71 | 18 | 33.33% |
| multiarray | 1,613 | 261 | 68.97% |
| naive | 32 | 4 | 50.00% |
| pi | 114,152,952 | 42,208,788 | 76.84% |
| qsort | 1,455,706 | 268,671 | 77.93% |
| queens | 643,635 | 99,701 | 60.43% |
| statement_test | 1,217 | 280 | 52.14% |
| superloop | 550,372 | 445,087 | 91.36% |
| tak | 1,964,545 | 197,071 | 63.08% |
| sample | 32 | 4 | 50.00% |