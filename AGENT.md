# AGENT.md

这个文件写给后续维护这个仓库的 agent 或开发者。目标是保持仓库可构建、可理解、可扩展，同时避免把 NanHuV5 这种具体目标写死进通用 framework。

## 工作目标

本仓库用于生成 RISC-V 裸机架构验证 case。当前重点是让 case 能在 NanHuV5/LinkNan RTL 环境中构建和运行，但 framework 需要继续支持 Spike、NEMU 和未来其他 CPU target。

不要把仓库理解成 H-extension-only。H 扩展 case 只是其中一类测试内容。

## docs/guide-docs/ 使用指南

`docs/guide-docs/` 存放了 AI 生成测试用例时必须参考的规则文档和历史经验库。

### 接手必读顺序

1. **`docs/guide-docs/Manual_Reference.md`** — 当前口径优先，包含：
   - 哪些测试可以直接 Spike gate，哪些必须 compile-only 或 manual
   - PMA / PBMT / 地址区域组合的已确认规则
   - 平台行为不可靠时的处理策略

2. **`docs/guide-docs/CRITICAL_ISSUES_LOG.md`** — 历史问题记录，遇到编译/运行异常先查此处

3. **`docs/guide-docs/bug_tickets/`** — AP bug ticket 分析与测试点（当前主要是 `nhv51/` 目录）
   - 每个 AP ticket 包含：触发条件、相关模块、衍生测试点 PxxA/PxxB
   - `BUG_TEST_POINT_BACKLOG.md` 是各项目 bug 测试点总账

4. **`docs/guide-docs/reference_tables/`** — 测试点分解参考表
   - `interrupt/` — 中断与 breakpoint 的交叉测试点
   - `memblock/` — MemBlock 模块测试点和疑似 bug

5. **`docs/guide-docs/suspected/`** — 疑似 bug 的 corner 场景（待验证）
   - `backend/` — 后端新 CSR、PMP 相关
   - `frontend/` — 前端 suspected corner
   - `memblock/` — MemBlock 疑似 bug 场景（分批次记录）
   - `mix/` — 跨模块组合场景（breakpoint、vmodule）

### AI 生成用例时的参考流程

1. 确定测试目标（功能点 / bug 回归 / corner 覆盖）
2. 查 `Manual_Reference.md` 确认当前口径：是否需要 Spike gate？PMA/PBMT 有无限制？
3. 如果是 bug 回归，找对应 `bug_tickets/<project>/AP_xxx.md` 了解触发条件和已有测试点
4. 如果是 suspected 场景，参考 `suspected/<module>/` 下的点位描述
5. 生成代码后在 `CRITICAL_ISSUES_LOG.md` 中搜索类似历史问题，避免重踩
6. 注册到 `test_register.c`，更新 `docs/guide-docs/Manual_Reference.md` 中的相关说明（如适用）

### docs 文档的优先级冲突

当 `Manual_Reference.md` 与 `CRITICAL_ISSUES_LOG.md` 历史章节冲突时，以 `Manual_Reference.md` 为准。
`CRITICAL_ISSUES_LOG.md` 中 `2026-04` 部分有专门的口径优先级说明，新 agent 接手时必须阅读。

## 分层边界

| 层次 | 允许放什么 | 不应放什么 |
| --- | --- | --- |
| `src/`, `asm/`, `inc/`, `linker.ld` | 通用裸机启动、trap、日志、指令 helper、页表 helper | 某个 CPU 的固定假设 |
| `platform/<plat>/` | UART/tohost/exit/syscall/MMIO 运行环境差异 | 架构测试语义 |
| `targets/<target>/target.mk` | 具体 target 的默认 `PLAT`、`TEST_MARCH`、`PAGE_TABLE_MODE` | 大量 C 逻辑或 case 内容 |
| `test_cases/...` | 验证场景和检查逻辑 | 平台运行时实现 |

NanHuV5 相关默认值放在 `targets/nanhuv5/target.mk`。
LinkNan `simv` 运行适配放在 `platform/linknan/` 和 `scripts/get_result.py`。

## 构建规则

优先验证这些命令：

```bash
python3 -m py_compile scripts/archtest_common.py scripts/list_registered_sources.py scripts/compile_elf.py scripts/get_result.py

python3 scripts/compile_elf.py --list-cases

make TARGET_PROFILE=nanhuv5 \
  CROSS_COMPILE=/nfs/share/opt/riscv/bin/riscv64-unknown-elf- \
  GENERATE_BIN=0 GENERATE_DUMP=0 GENERATE_READELF=0
```

必须使用裸机 toolchain，例如：

```text
riscv64-unknown-elf-
```

不要默认使用：

```text
riscv64-unknown-linux-gnu-
```

Linux toolchain 会引入 glibc/sysroot 假设，不适合这个裸机环境。

## Case 组织

新增手写 case：

```text
test_cases/manual_test_cases/<topic>/<file>.c
```

新增生成类 case：

```text
test_cases/ai_test_cases/<topic>/<file>.c
```

旧根目录 `manual_test_cases/` 和 `ai_test_cases/` 只是兼容路径，不建议继续新增。

case 函数必须和注册名一致：

```c
bool my_case(void)
{
    TEST_START();
    TEST_ASSERT("scenario", condition);
    TEST_END();
}
```

默认构建使用 `CASE_LINK_MODE=registered`。如果一个 `.c` 文件中包含多个 case，即使只注册其中一个，整个文件也会被编译。因此公共 header 必须提供该文件中所有函数会用到的声明和 inline helper。

## 页表后端

`PAGE_TABLE_BACKEND=static`：

- 链接 `src/page_tables.c`
- 使用 `inc/page_tables.h`
- 适合固定映射和多数普通 case

`PAGE_TABLE_BACKEND=dynamic`：

- 链接 `src/dynamic_page_tables.c`
- 使用 `inc/dynamic_page_tables.h`
- 适合 case 动态创建、删除或修改页表映射

不要同时链接两个页表实现。`scripts/compile_elf.py` 会通过 case 是否包含
`dynamic_page_tables.h` 自动选择 dynamic 后端。

## NanHuV5 目标

`TARGET_PROFILE=nanhuv5` 当前默认：

```text
PLAT=linknan
TEST_MARCH=rv64imac_zicsr_zifencei_zicbom_zicboz_zve64x
TEST_MABI=lp64
PAGE_TABLE_MODE=sv48
PAGE_TABLE_BACKEND=static
```

这些是 NanHuV5 这个具体目标的默认值。不要把这些默认值复制到通用 Makefile 或通用 C 代码中。

## LinkNan 运行

`scripts/get_result.py --target-profile nanhuv5` 默认 platform 是 `linknan`。
它会查找：

```text
$HYPTEST_LINKNAN_HOME/sim/simv/comp/simv
$HYPTEST_LINKNAN_HOME/sim/simv/comp/simv.daidir
```

并在：

```text
$HYPTEST_LINKNAN_HOME/sim/simv/<case_name>/
```

创建运行目录，把 ELF 链接为 `workload.ELF`，再传给 `simv`。

## 添加新测试用例的完整流程

### 1. 确定测试主题和文件位置

按功能领域组织：

```text
test_cases/manual_test_cases/
├── csr_priv/          # CSR 和特权级相关
├── exception/         # 异常处理（ebreak, illegal, ecall）
├── interrupt/         # 中断处理（external, timer, software）
├── memory/            # 访存、页表、PMP/PMA
├── vector/            # 向量扩展
├── special/           # 特殊场景（如动态页表）
└── misc/              # 其他
```

### 2. 编写测试用例（模板）

#### 简单 CSR 测试模板

```c
// test_cases/manual_test_cases/csr_priv/my_csr_test.c
#include <rvh_test.h>

/**
 * @brief 测试 mstatus.MIE bit 读写
 * @requirement RISC-V Priv Spec 3.1.6
 * @privilege M-mode
 */
bool manual_csr_mstatus_mie_read_write(void)
{
    TEST_START();
    
    // 读取初始状态
    uint64_t initial = read_csr(mstatus);
    TEST_ASSERT("initial MIE should be 0", 
                (initial & MSTATUS_MIE) == 0);
    
    // 设置 MIE
    write_csr(mstatus, initial | MSTATUS_MIE);
    uint64_t after_set = read_csr(mstatus);
    TEST_ASSERT("MIE should be 1 after set", 
                (after_set & MSTATUS_MIE) != 0);
    
    // 清除 MIE
    write_csr(mstatus, after_set & ~MSTATUS_MIE);
    uint64_t after_clear = read_csr(mstatus);
    TEST_ASSERT("MIE should be 0 after clear", 
                (after_clear & MSTATUS_MIE) == 0);
    
    TEST_END();
}
```

#### 异常测试模板

```c
// test_cases/manual_test_cases/exception/my_exception_test.c
#include <rvh_test.h>

/**
 * @brief 测试 illegal instruction 异常
 * @expected mcause=2 (Illegal instruction)
 */
bool manual_exception_illegal_inst_basic(void)
{
    TEST_START();
    
    // 设置异常预期
    TEST_SETUP_EXCEPT();
    
    // 触发异常（执行非法指令）
    asm volatile(".word 0x00000000");  // 全0指令为非法
    
    // 验证异常触发
    TEST_ASSERT("exception should be triggered", 
                excpt.triggered);
    TEST_ASSERT("cause should be illegal instruction", 
                excpt.cause == CAUSE_ILLEGAL_INSTRUCTION);
    TEST_ASSERT("PC should point to illegal inst", 
                excpt.epc != 0);
    
    TEST_END();
}
```

#### 内存访问测试模板

```c
// test_cases/manual_test_cases/memory/my_memory_test.c
#include <rvh_test.h>
#include <page_tables.h>

/**
 * @brief 测试页表翻译和权限检查
 * @features MMU, Sv48
 */
bool manual_memory_page_fault_load_no_read_permission(void)
{
    TEST_START();
    
    // 分配测试页面
    uint64_t test_va = 0x8000000000UL;
    uint64_t test_pa = 0x80100000UL;
    
    // 设置页表映射（只有执行权限，无读权限）
    map_page(test_va, test_pa, PTE_V | PTE_X);
    
    // 设置异常预期
    TEST_SETUP_EXCEPT();
    
    // 尝试读取（应触发 page fault）
    volatile uint64_t *ptr = (uint64_t *)test_va;
    uint64_t val = *ptr;
    
    // 验证异常
    TEST_ASSERT("page fault should occur", excpt.triggered);
    TEST_ASSERT("cause should be load page fault", 
                excpt.cause == CAUSE_LOAD_PAGE_FAULT);
    TEST_ASSERT("tval should be faulting address", 
                excpt.tval == test_va);
    
    TEST_END();
}
```

#### 动态页表测试模板

```c
// test_cases/manual_test_cases/special/my_dynamic_pt_test.c
#include <rvh_test.h>
#include <dynamic_page_tables.h>

/**
 * @brief 测试动态页表创建和映射
 * @page_table_backend dynamic
 */
bool manual_dynamic_pt_create_and_map(void)
{
    TEST_START();
    
    // 初始化动态页表系统
    init_dynamic_page_tables();
    
    uint64_t va = 0x9000000000UL;
    uint64_t pa = 0x80200000UL;
    
    // 动态创建映射
    int ret = pt_map(va, pa, PTE_V | PTE_R | PTE_W);
    TEST_ASSERT("mapping should succeed", ret == 0);
    
    // 验证可以访问
    volatile uint64_t *ptr = (uint64_t *)va;
    *ptr = 0xdeadbeefUL;
    TEST_ASSERT("read should match write", *ptr == 0xdeadbeefUL);
    
    // 取消映射
    ret = pt_unmap(va);
    TEST_ASSERT("unmap should succeed", ret == 0);
    
    // 验证访问会触发异常
    TEST_SETUP_EXCEPT();
    uint64_t val = *ptr;
    TEST_ASSERT("page fault after unmap", excpt.triggered);
    
    TEST_END();
}
```

### 3. 注册测试

编辑 `test_register.c`，在对应主题区域添加：

```c
/* -------------------- CSR_PRIV -------------------- */
TEST_REGISTER(manual_csr_mstatus_mie_read_write);
```

或者重新自动发现：

```bash
python3 scripts/compile_elf.py --discover-all --list-cases > /tmp/all_cases.txt
# 然后使用脚本重新生成 test_register.c
```

### 4. 验证编译

```bash
# 查看编译计划
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name manual_csr_mstatus_mie_read_write \
  --plan-only

# 实际编译
python3 scripts/compile_elf.py \
  --target-profile nanhuv5 \
  --name manual_csr_mstatus_mie_read_write
```

### 5. 运行测试

```bash
python3 scripts/get_result.py \
  --target-profile nanhuv5 \
  --case manual_csr_mstatus_mie_read_write \
  --no-diff

# 查看结果
cat .tmp/result_log/linknan/manual_csr_mstatus_mie_read_write.log
```

## 调试技巧

### 1. 编译问题调试

#### 问题：找不到符号或链接失败

```bash
# 检查源文件发现
python3 scripts/compile_elf.py --discover-all --list-cases | grep my_test

# 查看编译计划中的依赖
python3 scripts/compile_elf.py --name my_test --plan-only

# 查看完整 make 输出（不通过 scripts/compile_elf.py）
make TARGET_PROFILE=nanhuv5 \
  TEST_REGISTER_SRC=test_register.c \
  CASE_LINK_MODE=registered \
  GENERATE_BIN=0 GENERATE_DUMP=0 \
  2>&1 | tee build.log
```

#### 问题：页表后端冲突

```bash
# 检查测试是否包含 dynamic_page_tables.h
grep -n "dynamic_page_tables.h" test_cases/manual_test_cases/memory/my_test.c

# 手动指定后端
python3 scripts/compile_elf.py \
  --name my_test \
  --page-table-backend dynamic
```

### 2. 运行时调试

#### 技巧 1：查看反汇编定位问题

```bash
# 生成反汇编
python3 scripts/compile_elf.py --name my_test

# 查看关键函数
less case_elf_asm/linknan/my_test.asm
# 搜索 /my_test_name

# 查看完整 dump（如果需要）
python3 scripts/compile_elf.py --name my_test --keep-readelf
less case_elf_asm/linknan/my_test.elf.txt
```

#### 技巧 2：增加日志级别

在测试代码中添加：

```c
#include <rvh_test.h>

bool my_test(void) {
    TEST_START();
    
    // 打印调试信息
    printf("[DEBUG] Starting test\n");
    
    uint64_t val = read_csr(mstatus);
    printf("[DEBUG] mstatus = 0x%lx\n", val);
    
    // ... 测试逻辑 ...
    
    TEST_END();
}
```

或使用 make 参数：

```bash
python3 scripts/compile_elf.py \
  --name my_test \
  --make-arg "LOG_LEVEL=DEBUG"
```

#### 技巧 3：对比平台行为

```bash
# 在 Spike 上运行
python3 scripts/get_result.py --platform spike --case my_test

# 在 LinkNan 上运行
python3 scripts/get_result.py --platform linknan --case my_test

# 对比日志
diff -u \
  .tmp/result_log/spike/my_test.log \
  .tmp/result_log/linknan/my_test.log
```

#### 技巧 4：使用 difftest 定位差异

```bash
export HYPTEST_DIFFTEST_REF_SO=/path/to/riscv64-spike-so

python3 scripts/get_result.py \
  --target-profile nanhuv5 \
  --case my_test \
  --max-cycles 1000000
# 不加 --no-diff，启用 difftest

# difftest 会在第一个不一致的指令处停止
# 查看 FSDB 波形或日志定位问题
```

### 3. 性能调试

#### 测试运行太慢

```bash
# 检查是否进入死循环
tail -f .tmp/result_log/linknan/my_test.log

# 增加 max-cycles 并观察
python3 scripts/get_result.py \
  --case my_test \
  --max-cycles 10000000

# 检查反汇编中是否有意外的循环
objdump -d case_elf_asm/linknan/my_test.ELF | less
```

#### 编译太慢

```bash
# 查看依赖的源文件数量
python3 scripts/compile_elf.py --name my_test --plan-only

# 禁用不需要的输出
python3 scripts/compile_elf.py \
  --name my_test \
  --skip-asm \
  --make-arg "GENERATE_BIN=0"

# 使用并行编译（批量时）
python3 scripts/compile_elf.py --match "my_tests_" -j8
```

## 高级维护指南

### Framework 层修改

#### 添加新的 CSR

1. 在 `inc/encoding.h` 添加 CSR 编号：
   ```c
   #define CSR_MYNEWCSR  0x7a0
   ```

2. 在 `inc/csrs.h` 添加 bit 定义：
   ```c
   #define MYNEWCSR_BIT1  (1UL << 1)
   #define MYNEWCSR_BIT2  (1UL << 2)
   ```

3. 验证：
   ```bash
   grep -n "MYNEWCSR" inc/*.h
   make TARGET_PROFILE=nanhuv5 clean
   make TARGET_PROFILE=nanhuv5 GENERATE_BIN=0
   ```

#### 添加新的指令封装

在 `inc/instructions.h` 添加：

```c
// CBO.ZERO 指令
static inline void cbo_zero(uint64_t addr)
{
    asm volatile("cbo.zero (%0)" : : "r"(addr) : "memory");
}
```

验证：
```bash
make TARGET_PROFILE=nanhuv5 clean
make TARGET_PROFILE=nanhuv5 GENERATE_BIN=0
```

#### 修改 trap handler

**高风险操作**，影响所有测试。修改 `asm/handlers.S` 前：

1. 备份当前版本
2. 理解现有保存/恢复逻辑
3. 小心修改寄存器使用（x2/sp 不在 SAVE_CONTEXT 中）
4. 全面验证：

```bash
# 编译所有注册的测试
python3 scripts/compile_elf.py --target-profile nanhuv5 all

# 运行冒烟测试集
for case in manual_ebreak_m_ebreak_breakpoint \
            manual_csr_mstatus_mie_set_1_success \
            manual_misaligned_m_load_byte_addr_aligned_success; do
  python3 scripts/get_result.py --case $case --no-diff
done
```

### Platform 层修改

#### 添加新的运行平台

1. 创建目录：
   ```bash
   mkdir -p platform/myplat/inc
   ```

2. 提供必要文件：
   ```text
   platform/myplat/
   ├── inc/platform.h      # MEM_BASE, MEM_SIZE 定义
   ├── syscalls.c          # printf/exit 实现
   └── linker.ld           # 可选，平台特定链接脚本
   ```

3. 在 `scripts/get_result.py` 添加运行逻辑

4. 验证：
   ```bash
   make PLAT=myplat TARGET_PROFILE=generic
   ```

### Target 层修改

#### 添加新的目标配置

1. 创建 `targets/mynewchip/target.mk`：
   ```makefile
   PLAT ?= linknan
   TEST_MARCH ?= rv64imac_zicsr_zifencei
   TEST_MABI ?= lp64
   PAGE_TABLE_MODE ?= sv39
   PAGE_TABLE_BACKEND ?= static
   ```

2. 验证：
   ```bash
   make TARGET_PROFILE=mynewchip GENERATE_BIN=0
   python3 scripts/compile_elf.py --target-profile mynewchip --list-cases
   ```

### test_register.c 维护策略

#### 策略 1：手动维护（推荐用于稳定期）

**优点**：精确控制启用/禁用，保留注释和分组
**缺点**：新增测试需要手动添加

```bash
# 添加新测试
vim test_register.c
# 在适当位置添加：
# TEST_REGISTER(manual_new_test);

# 禁用不稳定测试
vim test_register.c
# 注释掉：
# // TEST_REGISTER(flaky_test);
```

#### 策略 2：自动重新生成（推荐用于大规模重组）

```bash
# 备份
cp test_register.c test_register.c.backup

# 重新发现并生成
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

for topic in sorted(discovered.keys()):
    lines.append(f"/* ---- {topic.upper()} ({len(discovered[topic])} cases) ---- */")
    for test_name in sorted(discovered[topic]):
        lines.append(f"TEST_REGISTER({test_name});")
    lines.append("")

Path("test_register.c").write_text("\n".join(lines), encoding="utf-8")
EOPY

# 对比差异
diff -u test_register.c.backup test_register.c | less

# 手动调整启用/禁用
vim test_register.c
```

#### 策略 3：混合维护（推荐用于迭代期）

1. 新功能开发：手动添加到 test_register.c
2. 季度整理：自动重新生成，然后手动调整
3. 使用注释标记关键测试：
   ```c
   /* SMOKE TEST - 不要禁用 */
   TEST_REGISTER(manual_ebreak_m_ebreak_breakpoint);
   
   /* KNOWN FLAKY - 暂时禁用 */
   // TEST_REGISTER(manual_flaky_interrupt_test);
   ```

## 修改建议

- 改 framework 前先确认是否能用 target/platform/case 层解决。
- 添加新的 CSR 编号放到 `inc/encoding.h`，添加 CSR bit 放到 `inc/csrs.h`。
- 添加指令封装放到 `inc/instructions.h`，尽量使用小而明确的 `static inline`。
- 改 linker/boot/trap 前要重新跑 NanHuV5 构建，因为这类变化影响所有 case。
- 不要随意删除用户已有 case。无法构建时优先修公共接口或缩小注册集合。
- 文档路径要以当前目录结构为准：`test_cases/manual_test_cases` 和 `test_cases/ai_test_cases`。

## 版本管理建议

### 提交前检查清单

```bash
# 1. Python 语法检查
python3 -m py_compile scripts/*.py

# 2. 测试注册表一致性
python3 scripts/compile_elf.py --list-cases > /dev/null

# 3. 基本编译验证
make TARGET_PROFILE=nanhuv5 \
  GENERATE_BIN=0 GENERATE_DUMP=0 GENERATE_READELF=0

# 4. 冒烟测试（选几个关键 case）
python3 scripts/compile_elf.py --name manual_ebreak_m_ebreak_breakpoint
python3 scripts/get_result.py --case manual_ebreak_m_ebreak_breakpoint --no-diff

# 5. 文档更新
# 确保 README.md 和 AGENT.md 与代码同步
```

### Git 提交建议

```bash
# 分类提交
git add test_cases/manual_test_cases/csr_priv/new_test.c
git commit -m "test(csr): add mstatus.MIE read/write test"

git add test_register.c
git commit -m "test: register new CSR tests"

git add inc/instructions.h
git commit -m "feat(framework): add CBO.ZERO instruction wrapper"

# 避免混合 framework 和 case 修改
# 避免在同一次提交中修改 test_register.c 和大量 case 文件
```
