# RISC-V Arch Test Environment

这个仓库用于生成可在裸机环境、Spike/NEMU、以及 LinkNan/NanHuV5 RTL
仿真环境中运行的 RISC-V 架构验证 case。它继承了
`riscv-hyp-tests-nhv5` 中较好的组织方式，但目标不是只验证 H 扩展，而是逐步覆盖
CPU 核验证的各个方面：特权级、异常、中断、页表翻译、访存、原子、CMO、向量访存、
前后端 corner case，以及 NanHuV5 这类具体处理器目标上的回归刺激。

核心设计原则是解耦：

| 层次 | 位置 | 职责 |
| --- | --- | --- |
| Framework | `src/`, `asm/`, `inc/`, `linker.ld` | 裸机启动、异常入口、测试注册、日志、页表和指令 helper、链接布局 |
| Cases | `test_cases/manual_test_cases/`, `test_cases/ai_test_cases/` | 真正的验证内容，按功能主题组织 |
| Platform | `platform/<plat>/` | ELF 和外部世界交互的方式，例如 Spike tohost 或 LinkNan UART |
| Target | `targets/<target>/target.mk` | 某个具体 CPU/SoC 的默认平台、ISA、ABI、页表模式 |

NanHuV5 是一个具体 target，不是 framework 概念。LinkNan 是当前用于运行
NanHuV5 RTL `simv` 的 platform。通用代码应尽量放在 framework 层，NanHuV5 默认值应放在
`targets/nanhuv5/target.mk`，LinkNan 的运行时差异应放在 `platform/linknan/`。

## 目录说明

```text
arch-test/
├── asm/
│   ├── boot.S                  # 裸机入口，设置栈、清 BSS、跳到 C main
│   └── handlers.S              # trap/interrupt 汇编入口，保存恢复上下文
├── inc/
│   ├── rvh_test.h              # TEST_START/TEST_ASSERT/TEST_END 等测试宏
│   ├── instructions.h          # CSR、访存、H、CBO、RVV 等指令封装
│   ├── csrs.h                  # CSR bit 定义
│   ├── encoding.h              # CSR 编号和指令 encoding
│   ├── page_tables.h           # 静态页表接口
│   └── dynamic_page_tables.h   # 动态页表接口
├── src/
│   ├── main.c                  # 测试调度入口
│   ├── rvh_test.c              # 特权级切换、异常状态、测试运行时
│   ├── page_tables.c           # 静态页表实现
│   ├── dynamic_page_tables.c   # 动态页表实现
│   └── instruction.c           # 额外指令/随机指令辅助函数
├── test_cases/
│   ├── manual_test_cases/      # 手写 case，按功能主题分类
│   └── ai_test_cases/          # 生成或批量维护的 case
├── platform/
│   ├── spike/                  # Spike syscall/tohost 支持
│   ├── nemu/                   # NEMU 运行支持
│   └── linknan/                # LinkNan/NanHuV5 simv 运行支持
├── targets/
│   └── nanhuv5/target.mk       # NanHuV5 默认配置
├── scripts/
│   ├── archtest_common.py      # Python 工具公共逻辑
│   ├── list_registered_sources.py
│   ├── compile_elf.py          # 单 case 编译工具
│   └── get_result.py           # Spike/NEMU/LinkNan 运行工具
├── docs/
│   └── guide-docs/             # AI 生成用例时的参考指导文档
│       ├── Manual_Reference.md         # 人工确认规则、Spike gate 口径
│       ├── CRITICAL_ISSUES_LOG.md      # 历史关键问题记录
│       ├── bug_tickets/                # AP bug ticket 分析与测试点
│       ├── reference_tables/           # 中断、内存等测试点参考表
│       └── suspected/                  # 疑似 bug 的待验证场景
├── test_register.c             # 默认注册的 case 列表
├── linker.ld                   # 裸机链接脚本
└── Makefile
```

`manual_test_cases/` 和 `ai_test_cases/` 旧根目录仍被 Makefile 兼容扫描，但新增内容建议放在
`test_cases/manual_test_cases/` 或 `test_cases/ai_test_cases/`。

## docs/guide-docs/ 参考文档

`docs/guide-docs/` 目录包含 AI 生成测试用例时的参考资料和历史经验：

| 文档/目录 | 用途 |
|-----------|------|
| `Manual_Reference.md` | 人工确认的测试规则、Spike gate 口径、PMA/PBMT 组合规则 |
| `CRITICAL_ISSUES_LOG.md` | 历史关键问题记录，包含死循环、异常处理等教训 |
| `bug_tickets/` | AP bug ticket 分析与衍生测试点（按项目分目录） |
| `reference_tables/` | 中断、内存等模块的测试点参考表 |
| `suspected/` | 疑似 bug 的 corner 场景，按模块分类 |

**使用建议**：
- AI 生成用例时，应参考 `Manual_Reference.md` 了解当前测试规则和平台限制
- 遇到 Spike 行为异常时，先查 `CRITICAL_ISSUES_LOG.md` 是否有类似问题记录
- 针对特定 bug 编写回归测试时，参考 `bug_tickets/` 下的分析文档
- 设计新测试场景时，可参考 `reference_tables/` 和 `suspected/` 获取灵感

生成目录：

```text
build/<plat>/                  # make 输出
case_elf_asm/<plat>/           # scripts/compile_elf.py 导出的单 case ELF/asm
.tmp/hyptest_compile/          # 临时 test_register.c
.tmp/result_log/<plat>/        # scripts/get_result.py 运行日志
```

## 构建链路

一次裸机构建大致经过下面几步：

1. 选择平台和目标配置。
   `PLAT=spike|nemu|linknan` 决定运行时 I/O 和退出方式。
   `TARGET_PROFILE=nanhuv5` 会默认选择 `PLAT=linknan`、Sv48 和 NanHuV5 需要的 ISA 字符串。

2. 选择页表后端。
   `PAGE_TABLE_BACKEND=static` 链接 `src/page_tables.c`。
   `PAGE_TABLE_BACKEND=dynamic` 链接 `src/dynamic_page_tables.c`。
   两者不能同时链接，因为会提供同名页表接口。

3. 选择 case 源文件。
   默认 `CASE_LINK_MODE=registered`，Makefile 读取 `test_register.c` 中的
   `TEST_REGISTER(name)`，再通过 `scripts/list_registered_sources.py` 找到包含
   `bool name(...)` 的 C 文件。

4. 编译 framework、case、platform 源码。
   所有 C/汇编文件使用裸机工具链 `riscv64-unknown-elf-gcc` 编译，不依赖 Linux 用户态 glibc。

5. 用 `linker.ld` 链接 ELF。
   `asm/boot.S` 提供入口符号，`linker.ld` 决定代码、数据、BSS、栈和测试专用段的位置。

6. 可选生成 `.bin`、反汇编和 readelf 文本。

最终常见输出：

```text
build/linknan/rvh_test.elf
build/linknan/rvh_test.bin
build/linknan/rvh_test.asm
build/linknan/rvh_test.dump
build/linknan/rvh_test.elf.txt
```

## 基本构建

优先使用裸机工具链：

```bash
make PLAT=spike CROSS_COMPILE=/nfs/share/opt/riscv/bin/riscv64-unknown-elf-
```

不要使用 `riscv64-unknown-linux-gnu-` 编译裸机程序。Linux toolchain 会走 sysroot/glibc，
容易出现 `gnu/stubs-lp64.h` 缺失，也会把程序假设成 Linux 用户态程序。

NanHuV5 默认构建：

```bash
make TARGET_PROFILE=nanhuv5 \
  CROSS_COMPILE=/nfs/share/opt/riscv/bin/riscv64-unknown-elf-
```

只验证 ELF 链接、跳过 bin/dump/readelf：

```bash
make TARGET_PROFILE=nanhuv5 \
  CROSS_COMPILE=/nfs/share/opt/riscv/bin/riscv64-unknown-elf- \
  GENERATE_BIN=0 GENERATE_DUMP=0 GENERATE_READELF=0
```

常用 Make 变量：

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `PLAT` | target 可设置 | 运行平台，例如 `spike`、`nemu`、`linknan` |
| `TARGET_PROFILE` | `generic` | 具体 CPU/SoC 配置，例如 `nanhuv5` |
| `CROSS_COMPILE` | `riscv64-unknown-elf-` | 裸机工具链前缀 |
| `TEST_MARCH` | 自动或 target 设置 | GCC `-march` |
| `TEST_MABI` | `lp64` | GCC `-mabi` |
| `PAGE_TABLE_MODE` | `sv39`，NanHuV5 为 `sv48` | 静态页表编译模式 |
| `PAGE_TABLE_BACKEND` | `static` | `static` 或 `dynamic` |
| `CASE_LINK_MODE` | `registered` | `registered`、`selected` 或 full 扫描 |
| `TEST_REGISTER_SRC` | `test_register.c` | 可替换的注册文件 |
| `CASE_C_SRCS` | 空 | `CASE_LINK_MODE=selected` 时手动指定 C case |
| `CASE_ASM_SRCS` | 空 | `CASE_LINK_MODE=selected` 时手动指定 asm case |
| `GENERATE_BIN` | `1` | 是否生成 `.bin` |
| `GENERATE_ASM` | `1` | 是否生成源码混合反汇编 |
| `GENERATE_DUMP` | `1` | 是否生成完整 dump |

## 单 case 编译

`scripts/compile_elf.py` 用来在不修改 `test_register.c` 的情况下生成单个 case 的 ELF。
它会生成临时注册文件，调用 Makefile，然后把结果导出到 `case_elf_asm/<plat>/`。

列出当前注册 case：

```bash
python3 scripts/compile_elf.py --list-cases
```

列出仓库中发现的全部 case 函数：

```bash
python3 scripts/compile_elf.py --discover-all --list-cases
```

查看单 case 编译计划：

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --cross-compile /nfs/share/opt/riscv/bin/riscv64-unknown-elf- \
  --name manual_ebreak_m_ebreak_breakpoint \
  --plan-only
```

实际生成 NanHuV5/LinkNan ELF：

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --cross-compile /nfs/share/opt/riscv/bin/riscv64-unknown-elf- \
  --name manual_ebreak_m_ebreak_breakpoint
```

动态页表 case 会自动选择 `PAGE_TABLE_BACKEND=dynamic`：

```bash
python3 scripts/compile_elf.py \
  --discover-all \
  --target-profile nanhuv5 \
  --cross-compile /nfs/share/opt/riscv/bin/riscv64-unknown-elf- \
  --name manual_dynamic_pt_sv39_mode_basic_rw \
  --plan-only
```

也可以手动指定：

```bash
python3 scripts/compile_elf.py --target-profile nanhuv5 \
  --page-table-backend dynamic \
  --name manual_dynamic_pt_sv39_mode_basic_rw
```

批量选择也支持 `riscv-hyp-tests-nhv5` 风格的常用参数：

```bash
python3 scripts/compile_elf.py --target-profile nanhuv5 all --plan-only
python3 scripts/compile_elf.py --target-profile nanhuv5 4 20 --plan-only
python3 scripts/compile_elf.py --target-profile nanhuv5 --match ebreak --exclude compressed
python3 scripts/compile_elf.py --target-profile nanhuv5 --name-file case_names.txt -j8 --clean
```

如果临时增加新的 case 根目录，可以用：

```bash
python3 scripts/compile_elf.py --target-profile nanhuv5 --case-root path/to/cases --discover-all --list-cases
```

## 运行 case

先用 `scripts/compile_elf.py` 生成 case ELF，再用 `scripts/get_result.py` 运行。

Spike：

```bash
HYPTEST_SPIKE_BIN=/path/to/spike \
python3 scripts/get_result.py --platform spike --case manual_ebreak_m_ebreak_breakpoint
```

NEMU：

```bash
HYPTEST_NEMU_BIN=/path/to/nemu \
python3 scripts/get_result.py --platform nemu --case manual_ebreak_m_ebreak_breakpoint
```

LinkNan/NanHuV5 `simv`：

```bash
HYPTEST_LINKNAN_HOME=/path/to/LinkNan \
python3 scripts/get_result.py \
  --target-profile nanhuv5 \
  --case manual_ebreak_m_ebreak_breakpoint \
  --no-diff
```

`scripts/get_result.py` 默认查找：

```text
$HYPTEST_LINKNAN_HOME/sim/simv/comp/simv
$HYPTEST_LINKNAN_HOME/sim/simv/comp/simv.daidir
```

运行时会在 LinkNan 仓库下创建：

```text
$HYPTEST_LINKNAN_HOME/sim/simv/<case_name>/
```

并把 case ELF 链接为：

```text
workload.ELF
```

传给 `simv` 的关键参数是：

```text
+workload=workload.ELF
+max-cycles=<cycles>
+no-diff
```

如果要打开 difftest，去掉 `--no-diff`，并设置：

```bash
export HYPTEST_DIFFTEST_REF_SO=/path/to/riscv64-spike-so
```

运行日志会写入：

```text
.tmp/result_log/<platform>/<case>.log
```

## 添加新 case

建议路径：

```text
test_cases/manual_test_cases/<topic>/<file>.c
test_cases/ai_test_cases/<topic>/<file>.c
```

case 函数格式：

```c
#include <rvh_test.h>

bool my_case_name(void)
{
    TEST_START();

    TEST_ASSERT("scenario description", condition);

    TEST_END();
}
```

然后在 `test_register.c` 中注册：

```c
TEST_REGISTER(my_case_name);
```

如果 case 需要配套汇编文件，把 `.S` 放在同一目录。registered 模式会把同目录汇编源一起链接。

如果 case 是 NanHuV5 特有行为，仍然按功能主题放到 `test_cases/...` 下，并在 case 注释中说明
NanHuV5 假设。不要把 NanHuV5 写死进 `src/main.c`、`src/rvh_test.c`、`inc/` 这类公共 framework 文件；
确实需要目标差异时，优先通过 `targets/nanhuv5/target.mk` 或 `platform/linknan/` 注入。

## 页表后端

静态页表：

```bash
PAGE_TABLE_BACKEND=static PAGE_TABLE_MODE=sv48 make TARGET_PROFILE=nanhuv5
```

适合大多数固定映射 case，页表内容由 `src/page_tables.c` 和 `inc/page_tables.h` 中的枚举/数组描述。

动态页表：

```bash
PAGE_TABLE_BACKEND=dynamic make TARGET_PROFILE=nanhuv5
```

适合在 case 中动态创建/修改映射的场景。动态页表 case 应包含：

```c
#include <dynamic_page_tables.h>
```

`scripts/compile_elf.py` 会据此自动选择 dynamic 后端。

## 快速开始

### 前置条件

1. **裸机工具链**（必需）：
   ```bash
   export PATH=/nfs/share/opt/riscv/bin:$PATH
   # 验证
   riscv64-unknown-elf-gcc --version
   ```

2. **LinkNan 环境**（用于 NanHuV5 RTL 仿真）：
   ```bash
   export HYPTEST_LINKNAN_HOME=/path/to/LinkNan
   # 验证 simv 存在
   ls $HYPTEST_LINKNAN_HOME/sim/simv/comp/simv
   ```

### 30 秒快速验证

```bash
# 1. 列出所有可用测试
python3 scripts/compile_elf.py --list-cases | head -20

# 2. 编译一个简单的测试
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name manual_csr_mstatus_mie_set_1_success

# 3. 在 LinkNan simv 上运行
python3 scripts/get_result.py \
  --target-profile nanhuv5 \
  --case manual_csr_mstatus_mie_set_1_success \
  --no-diff

# 查看结果
cat .tmp/result_log/linknan/manual_csr_mstatus_mie_set_1_success.log
```

### 完整工作流程示例

#### 场景 1：编写并运行新测试

**步骤 1：编写测试用例**

在 `test_cases/manual_test_cases/csr_priv/my_test.c`:

```c
#include <rvh_test.h>

bool manual_my_custom_csr_test(void)
{
    TEST_START();

    // 读取 mstatus
    uint64_t mstatus = read_csr(mstatus);

    // 验证初始状态
    TEST_ASSERT("mstatus.MIE should be 0", (mstatus & MSTATUS_MIE) == 0);

    // 设置 MIE bit
    write_csr(mstatus, mstatus | MSTATUS_MIE);

    // 验证设置成功
    mstatus = read_csr(mstatus);
    TEST_ASSERT("mstatus.MIE should be 1", (mstatus & MSTATUS_MIE) != 0);

    TEST_END();
}
```

**步骤 2：注册测试**

在 `test_register.c` 的适当位置添加：

```c
/* -------------------- CSR_PRIV -------------------- */
TEST_REGISTER(manual_my_custom_csr_test);
```

或者用发现模式重新生成（会覆盖现有文件）：

```bash
python3 scripts/compile_elf.py --discover-all --list-cases > /tmp/all.txt
# 手动编辑 test_register.c 或使用脚本
```

**步骤 3：验证编译计划**

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name manual_my_custom_csr_test \
  --plan-only
```

输出示例：
```
Case: manual_my_custom_csr_test
  C sources:
    test_cases/manual_test_cases/csr_priv/my_test.c
  ASM sources: (none)
  Page table backend: static
```

**步骤 4：编译**

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name manual_my_custom_csr_test
```

成功输出：
```
[build] manual_my_custom_csr_test ... ok
Output: case_elf_asm/linknan/manual_my_custom_csr_test.ELF
```

**步骤 5：运行**

```bash
python3 scripts/get_result.py \
  --target-profile nanhuv5 \
  --case manual_my_custom_csr_test \
  --no-diff
```

检查结果：
```bash
grep -E "PASSED|FAILED" .tmp/result_log/linknan/manual_my_custom_csr_test.log
```

#### 场景 2：批量编译和回归测试

**编译所有 CSR 相关测试**：

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --match "^manual_csr_" \
  -j8 \
  --skip-asm
```

**编译指定范围**（test_register.c 的行号）：

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --range 10-50 \
  -j4
```

**编译文件列表**：

```bash
# 创建 case_list.txt
cat > /tmp/my_cases.txt << 'EOF'
manual_csr_mstatus_mie_set_1_success
manual_ebreak_m_ebreak_breakpoint
manual_misaligned_m_load_byte_addr_aligned_success
EOF

python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name-file /tmp/my_cases.txt \
  -j8
```

**批量运行**：

```bash
# 运行所有已编译的 case
for elf in case_elf_asm/linknan/*.ELF; do
  case_name=$(basename "$elf" .ELF)
  python3 scripts/get_result.py \
    --target-profile nanhuv5 \
    --case "$case_name" \
    --no-diff
done

# 统计结果
grep -rh "PASSED\|FAILED" .tmp/result_log/linknan/ | sort | uniq -c
```

## test_register.c 管理

### 重新生成完整注册表

当添加大量新测试后，可以重新生成整个 `test_register.c`：

```bash
# 备份当前版本
cp test_register.c test_register.c.backup

# 发现所有测试并生成新注册表
python3 << 'EOPY'
import subprocess
from pathlib import Path
from collections import defaultdict

result = subprocess.run(
    ["python3", "scripts/compile_elf.py", "--discover-all", "--list-cases"],
    capture_output=True, text=True, cwd="."
)

discovered = defaultdict(list)
for line in result.stdout.splitlines():
    parts = line.split()
    if len(parts) >= 4 and parts[1] == "discovered":
        test_name = parts[2].rstrip(":")
        source_path = parts[3]
        path_parts = Path(source_path).parts
        if "manual_test_cases" in path_parts:
            idx = path_parts.index("manual_test_cases")
            topic = path_parts[idx + 1] if idx + 1 < len(path_parts) else "misc"
        else:
            topic = "other"
        discovered[topic].append(test_name)

lines = ["#include <rvh_test.h>", "#include <page_tables.h>", ""]
lines.append("/* Auto-generated test registry */")
lines.append("")

for topic in sorted(discovered.keys()):
    lines.append(f"/* ---- {topic.upper()} ({len(discovered[topic])} cases) ---- */")
    for test_name in sorted(discovered[topic]):
        lines.append(f"TEST_REGISTER({test_name});")
    lines.append("")

Path("test_register.c").write_text("\n".join(lines), encoding="utf-8")
print(f"Generated {sum(len(v) for v in discovered.values())} entries")
EOPY
```

### 启用/禁用测试

注释掉不想运行的测试：

```c
TEST_REGISTER(my_test);           // 启用
// TEST_REGISTER(flaky_test);     // 禁用
```

`scripts/compile_elf.py --list-cases` 只显示启用的测试，加 `--include-commented` 可显示全部。

## scripts/compile_elf.py 详细用法

### 常用选项

| 选项 | 说明 | 示例 |
|------|------|------|
| `--target-profile` | 目标配置（nanhuv5/generic） | `--target-profile nanhuv5` |
| `--name` | 指定测试名称 | `--name my_test` |
| `--match` | 正则匹配测试名 | `--match "^manual_csr_"` |
| `--exclude` | 正则排除测试名 | `--exclude "disabled"` |
| `--range` | 行号范围 | `--range 10-50` |
| `--name-file` | 从文件读取测试列表 | `--name-file cases.txt` |
| `-j N` | 并行编译数 | `-j8` |
| `--plan-only` | 只显示计划不编译 | `--plan-only` |
| `--discover-all` | 发现所有测试（包括未注册的） | `--discover-all` |
| `--list-cases` | 列出测试不编译 | `--list-cases` |
| `--clean` | 每次编译前清理 | `--clean` |
| `--skip-asm` | 不生成反汇编 | `--skip-asm` |
| `--page-table-backend` | 指定页表后端 | `--page-table-backend dynamic` |
| `--make-arg` | 传递给 make 的参数 | `--make-arg LOG_LEVEL=DEBUG` |

### 高级用法示例

**组合过滤**：

```bash
# 编译所有 exception 相关，但排除 illegal
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --match "exception" \
  --exclude "illegal" \
  -j8
```

**调试模式编译**：

```bash
# 保留预处理文件，生成详细 dump
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name my_test \
  --keep-preprocessed \
  --keep-readelf \
  --make-arg "LOG_LEVEL=DEBUG"
```

**多范围选择**（逗号分隔）：

```bash
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --range "1-10,50-60,100-110" \
  -j4
```

## scripts/get_result.py 详细用法

### 基本选项

| 选项 | 说明 | 示例 |
|------|------|------|
| `--target-profile` | 目标配置 | `--target-profile nanhuv5` |
| `--platform` | 运行平台（spike/nemu/linknan） | `--platform linknan` |
| `--case` | 测试名称 | `--case my_test` |
| `--elf-dir` | ELF 目录 | `--elf-dir case_elf_asm/linknan` |
| `--max-cycles` | 最大周期数 | `--max-cycles 2000000` |
| `--no-diff` | 禁用 difftest | `--no-diff` |
| `--spike-home` | Spike 二进制路径 | `--spike-home /path/to/spike` |
| `--linknan-home` | LinkNan 路径 | `--linknan-home /path/to/LinkNan` |

### 平台特定配置

**Spike**：

```bash
export HYPTEST_SPIKE_BIN=/nfs/share/opt/spike/bin/spike
python3 scripts/get_result.py \
  --platform spike \
  --case my_test \
  --max-cycles 1000000
```

**LinkNan + Difftest**：

```bash
export HYPTEST_LINKNAN_HOME=/path/to/LinkNan
export HYPTEST_DIFFTEST_REF_SO=/path/to/riscv64-spike-so
python3 scripts/get_result.py \
  --target-profile nanhuv5 \
  --case my_test \
  --max-cycles 2000000
# 不加 --no-diff，默认启用 difftest
```

### 批量运行脚本示例

```bash
#!/bin/bash
# run_batch.sh

TARGET_PROFILE=nanhuv5
PLATFORM=linknan
ELF_DIR=case_elf_asm/$PLATFORM
LOG_DIR=.tmp/result_log/$PLATFORM

mkdir -p "$LOG_DIR"

passed=0
failed=0
error=0

for elf in "$ELF_DIR"/*.ELF; do
  case_name=$(basename "$elf" .ELF)
  echo "Running $case_name..."

  python3 scripts/get_result.py \
    --target-profile "$TARGET_PROFILE" \
    --case "$case_name" \
    --no-diff \
    > /dev/null 2>&1

  if grep -q "PASSED" "$LOG_DIR/$case_name.log"; then
    ((passed++))
    echo "  ✓ PASSED"
  elif grep -q "FAILED" "$LOG_DIR/$case_name.log"; then
    ((failed++))
    echo "  ✗ FAILED"
  else
    ((error++))
    echo "  ⚠ ERROR"
  fi
done

echo ""
echo "Summary: $passed passed, $failed failed, $error error"
```

## 性能优化

### 编译性能

1. **并行编译**：`-j` 参数匹配 CPU 核数
   ```bash
   python3 scripts/compile_elf.py --target-profile nanhuv5 --range 1-100 -j$(nproc)
   ```

2. **跳过不需要的输出**：
   ```bash
   python3 scripts/compile_elf.py \
     --target-profile nanhuv5 \
     --name my_test \
     --skip-asm \
     --make-arg "GENERATE_BIN=0"
   ```

3. **避免 clean**：默认不清理，只在链接错误时用 `--clean`

### 运行性能

1. **调整 max-cycles**：
   ```bash
   # 简单测试用更少周期
   --max-cycles 100000

   # 复杂测试需要更多
   --max-cycles 5000000
   ```

2. **禁用 difftest**（调试通过后）：
   ```bash
   --no-diff
   ```

## 故障排查

### 编译错误

#### 错误：`gnu/stubs-lp64.h: No such file or directory`

**原因**：使用了 Linux toolchain 而非裸机 toolchain。

**解决**：
```bash
# 错误的
export CROSS_COMPILE=riscv64-unknown-linux-gnu-

# 正确的
export CROSS_COMPILE=riscv64-unknown-elf-
# 或
python3 scripts/compile_elf.py --cross-compile /nfs/share/opt/riscv/bin/riscv64-unknown-elf-
```

#### 错误：`undefined reference to 'my_case_name'`

**原因**：`test_register.c` 中注册了函数，但源文件不存在或函数名拼写错误。

**排查**：
```bash
# 检查注册的名称
grep "TEST_REGISTER(my_case_name)" test_register.c

# 查找函数定义
grep -r "bool my_case_name" test_cases/

# 查看编译计划
python3 scripts/compile_elf.py --name my_case_name --plan-only
```

#### 错误：`multiple definition of 'my_case'`

**原因**：同一个函数在多个文件中定义。

**排查**：
```bash
grep -rn "bool my_case" test_cases/
```

### 运行错误

#### 错误：`simv: command not found`

**原因**：`HYPTEST_LINKNAN_HOME` 未设置或路径错误。

**解决**：
```bash
export HYPTEST_LINKNAN_HOME=/path/to/LinkNan
ls $HYPTEST_LINKNAN_HOME/sim/simv/comp/simv  # 验证
```

#### 测试超时（达到 max-cycles）

**原因**：测试逻辑进入死循环或 cycles 设置太小。

**排查**：
1. 检查测试日志最后的 PC 和指令
2. 增加 max-cycles：`--max-cycles 10000000`
3. 查看反汇编确认逻辑：`less case_elf_asm/linknan/my_test.asm`

#### 测试 FAILED 但预期应该 PASSED

**排查流程**：
1. 查看详细日志：
   ```bash
   cat .tmp/result_log/linknan/my_test.log
   ```

2. 检查 ASSERT 失败位置：
   ```bash
   grep "ASSERT FAILED" .tmp/result_log/linknan/my_test.log
   ```

3. 对比反汇编和源码：
   ```bash
   less case_elf_asm/linknan/my_test.asm
   ```

4. 在 Spike 上运行对比（如果可用）：
   ```bash
   python3 scripts/get_result.py --platform spike --case my_test
   ```

### Python 工具错误

#### 错误：`ModuleNotFoundError: No module named 'archtest_common'`

**原因**：通常是只移动或复制了单个脚本，导致 `scripts/archtest_common.py` 不在同一目录。

**解决**：
```bash
cd /path/to/arch-test
python3 scripts/compile_elf.py ...
```

#### 错误：编译进度卡住不动

**原因**：可能是 make 进程死锁或等待输入。

**解决**：
1. Ctrl+C 中断
2. 检查是否有 zombie 进程：`ps aux | grep make`
3. 清理后重试：`make PLAT=linknan clean`

## 常见问题

### 为什么不能用 linux-gnu toolchain？

裸机程序没有 Linux 进程环境，也没有 glibc syscall ABI。应使用
`riscv64-unknown-elf-`。如果用了 `riscv64-unknown-linux-gnu-`，常见错误是：

```text
fatal error: gnu/stubs-lp64.h: No such file or directory
```

### `hypervisor extensions not present` 是什么？

这通常是运行平台或 ISA 配置没有真正提供 H 扩展。手动写 CSR 或设置状态位不能凭空让硬件实现 H。
在 Spike 上需要用支持 H 的 Spike，并设置合适 ISA；在 RTL 上需要确认 DUT 本身实现了对应扩展。

### ELF 和 BIN 有什么区别？

ELF 带符号表、段信息、入口地址等元数据，适合仿真器加载和调试。
BIN 是按 loadable 段抽出来的裸二进制镜像，很多 RTL testbench 也可以加载，但通常还需要额外告诉入口地址和内存装载地址。

LinkNan `simv` 当前推荐使用 ELF，通过 `+workload=workload.ELF` 传入。

### 为什么链接脚本和 boot.S 能互相看到符号？

它们不是 C 语言 include 关系，而是链接阶段统一解析符号。
`boot.S` 可以引用 `linker.ld` 中定义的 `_stack_top`、`_bss_start` 等符号；
`linker.ld` 也可以用 `ENTRY(_start)` 指定 `boot.S` 导出的入口。所有目标文件和链接脚本最终都交给 linker，linker 建立全局符号表。

### `rvh_test.elf has a LOAD segment with RWX permissions`

这是当前裸机链接脚本把可执行段和可写段放在同一 load segment 后产生的 warning。
它不阻止 ELF 生成。后续如果需要更严格的段权限，可以把 text/rodata/data/bss 拆成不同 program header。

### 如何查看当前有多少测试？

```bash
# 注册的测试（启用的）
python3 scripts/compile_elf.py --list-cases | wc -l

# 所有发现的测试（包括未注册的）
python3 scripts/compile_elf.py --discover-all --list-cases | wc -l

# 按主题分组统计
python3 scripts/compile_elf.py --discover-all --list-cases | \
  awk '{print $4}' | \
  xargs -I{} dirname {} | \
  sort | uniq -c
```

### 为什么有些测试编译很慢？

可能原因：
1. 测试依赖很多其他源文件（通过函数调用）
2. 启用了 `GENERATE_DUMP=1`（默认已禁用）
3. 页表后端不匹配（dynamic case 用 static 后端会编译两次）

查看编译计划：
```bash
python3 scripts/compile_elf.py --name slow_test --plan-only
```

### test_register.c 应该手动维护还是自动生成？

**推荐混合方式**：
1. 首次生成时用自动发现：`--discover-all`
2. 日常添加少量测试时手动编辑
3. 大规模重组时重新自动生成，然后手动调整启用/禁用

自动生成会覆盖所有注释和分组，所以重要的分组信息应该通过目录结构（`test_cases/manual_test_cases/<topic>/`）来体现，而不是依赖注释。
