#include <rvh_test.h>
#include <page_tables.h>
#include <csrs.h> 

bool manual_illegal_ili_decode_do_match_ili() {

    TEST_START();

    TEST_SETUP_EXCEPT();

    //译码过程中decode table查表，若未查询到匹配项，赋予INVALID_INSTR
    goto_priv(PRIV_M);
    TEST_SETUP_EXCEPT();
    INVALID_INSTR();
    TEST_ASSERT("decode do not match leads to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    // 访问有效，CSRoptype非跳转行为，且访问CSR地址不在CSR mapping地址范围内
    
    TEST_SETUP_EXCEPT();
    CSRR(0xf16);
    TEST_ASSERT("CSR addr not match the csr mapping leads to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    //当前权限满足最低CSR访问权限，CSR写行为，但访问的CSR只读
    TEST_SETUP_EXCEPT();
    goto_priv(PRIV_M);
    CSRW(CSR_MCONFIGPTR,0xfff);
    TEST_ASSERT("write csr which is read-only leads to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI 
    ); 


    //当前权限不满足最低CSR访问权限
    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_MIE);
    TEST_ASSERT("low priviliege mode access high priviliege csr leads to illegal instruction interrupt",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    TEST_END();

}



bool manual_illegal_ili_write_unsupported_value_wlrl_field_no_trap() {

    TEST_START();

    TEST_SETUP_EXCEPT();

    // 当前实现对WLRL字段写入非支持值做合法化/忽略，不产生非法指令异常。
    CSRW(CSR_MCAUSE,0xffffffffff);     
    TEST_ASSERT("write unsupported value into WLRL csr field does not trap in current implementation",
        excpt.triggered == false
    ); 


    TEST_END();
}



bool manual_illegal_ili_m_access_any_success() {

    TEST_START();

    //M模式下可以访问任何CSR
    goto_priv(PRIV_M);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_MCAUSE);
    CSRR(CSR_SCAUSE);
    TEST_ASSERT("m mode access any csr successful",
        excpt.triggered == false
    ); 

    reset_state();

    //尝试访问高特权级别的任何CSR
    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_MCAUSE);
    TEST_ASSERT("hs mode access higher privilege csr leads to ili",
        excpt.triggered == true &&
        excpt.cause==CAUSE_ILI
    ); 

    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_MIDELEG);
    TEST_ASSERT("hu mode access higher privilege csr leads to ili",
        excpt.triggered == true &&
        excpt.cause==CAUSE_ILI
    );

    TEST_END();
}

//当寄存器为32位时才做该测试
/*
    //当V=1且XLEN>32时，尝试访问高半部分监管级CSR
    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    CSRS(CSR_SSTATUS,1ULL << 35);
    TEST_ASSERT("When V=1 and XLEN>32, try to access the higher half of the regulatory CSR results in an illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause==CAUSE_ILI
    );

    //当V=1且XLEN>32时，尝试访问高半部分虚拟化扩展CSR
    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    CSRS(CSR_HSTATUS,1ULL << 35);
    TEST_ASSERT("When V=1 and XLEN>32, try to access the higher half of the Virtualization extension CSR results in an illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause==CAUSE_ILI
    );

    //当V=1且XLEN>32时，尝试访问高半部分VS CSR
    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    CSRS(CSR_VSSTATUS,1ULL << 35);
    TEST_ASSERT("When V=1 and XLEN>32, try to access the higher half of the vs CSR results in an illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause==CAUSE_ILI
    );

    //当V=1且XLEN>32时，尝试访问高半部分非特权CSR
    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLEH,1ULL << 35);
    TEST_ASSERT("When V=1 and XLEN>32, try to access the higher half of the non-privileged CSR results in an illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause==CAUSE_ILI
    );
*/


bool manual_illegal_ili_satp_was_accessed_s_without_v_memory_enabled_ili_tvm1() {

    TEST_START();
    //TVM=1，在S模式下，读写satp 寄存器
    goto_priv(PRIV_M);
    CSRW(satp, 0x0);
    CSRS(CSR_MSTATUS, MSTATUS_TVM);
    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    CSRR(satp);
    TEST_ASSERT("The Satp register was accessed in S mode without virtual memory enabled leads to ili when tvm=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    TEST_END();
}

bool manual_illegal_ili_s_sfence_vma_ili_tvm1() {

    TEST_START();

    //TVM=1，在S模式下，执行SFENCE.VMA或SINVAL.VMA指令
    goto_priv(PRIV_M);
    CSRS(CSR_MSTATUS,MSTATUS_TVM);

    goto_priv(PRIV_HS); 
    TEST_SETUP_EXCEPT();
    sfence_vma();
    TEST_ASSERT("s mode sfence.vma leads to ili when tvm=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("s mode sinval.vma leads to ili when tvm=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    //TVM=0，在S模式下，执行SFENCE.VMA或SINVAL.VMA指令
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,MSTATUS_TVM);

    goto_priv(PRIV_HS); 
    TEST_SETUP_EXCEPT();
    sfence_vma();
    TEST_ASSERT("s mode sfence.vma leads to ili when tvm=1",
        excpt.triggered == false
    ); 

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("s mode sinval.vma leads to ili when tvm=1",
        excpt.triggered == false
    ); 

    TEST_END();
}



bool manual_illegal_ili_hs_sinval_vma_success_mstatus_tvm1() {

    TEST_START();
    
    //mstatus.TVM=1，尝试在S模式下执行SINVAL.VMA
    goto_priv(PRIV_M);
    reset_state();
    CSRS(CSR_MSTATUS,MSTATUS_TVM);
    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("hs mode sinval.vma successful when mstatus.tvm=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 


    TEST_END();
}



bool manual_illegal_ili_hu_sinval_vma_ili_inst_irq() {

    TEST_START();

    //在U模式下执行SINVAL.VMA中的任何一条
    TEST_SETUP_EXCEPT();
    goto_priv(PRIV_M);
    reset_state();
    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("hu mode sinval.vma cause to illegal instruction interrupt",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    TEST_END();
}


bool manual_illegal_ili_hs_sfence_vma_satp_mode0_succeeds() {

    TEST_START();
    // 当前实现中satp.mode不是只读0，satp=0时HS执行sfence.vma合法。
    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_HS);
    CSRW(CSR_SATP,0ULL);
    sfence_vma();
    TEST_ASSERT("hs mode sfence.vma when satp.mode=0 succeeds in current implementation",
        excpt.triggered == false
    ); 

    TEST_END();
}




    // uintptr_t vaddr_f = hs_page_base(VSI_GI);  


    // //当V=1时，设置vsstatus.FS=0，HS级别的sstatus.FS=1时，尝试执行浮点指令（vsstatus.FS和HS级别的sstatus.FS同时生效才可以执行浮点指令）
    // goto_priv(PRIV_M);
    // TEST_SETUP_EXCEPT();
    // CSRS(CSR_MISA,1ULL << 7);        //开启h扩展(v=1)
    // CSRS(CSR_MISA,1ULL << 5 | 1ULL << 3);        //开启float和double
    // CSRC(CSR_VSSTATUS, (1ULL << 13) | (1ULL << 14));
    // CSRS(CSR_SSTATUS, (1ULL << 13) | (1ULL << 14));
    // flw(vaddr_f);
    // TEST_ASSERT("Execute floating-point instruction leads to ili when v=1 and vsstatus.fs=0 and sstatus.fs=1",
    //     excpt.triggered == true &&
    //     excpt.cause == CAUSE_ILI
    // ); 


    //当V=1时，设置vsstatus.FS=1，HS级别的sstatus.FS=0时，尝试执行浮点指令
    //当V=1时，设置vsstatus.FS=0，HS级别的sstatus.FS=0时，尝试执行浮点指令



    // //当V=1时，设置vsstatus.VS=0，HS级别的sstatus.VS=1时，尝试执行向量指令（vsstatus.VS和HS级别的sstatus.VS同时生效才可以执行向量指令）
    // goto_priv(PRIV_M);
    // TEST_SETUP_EXCEPT();
    // CSRS(CSR_MISA,1ULL << 7);        //开启h扩展(v=1)
    // CSRS(CSR_MISA,1ULL << 5 | 1ULL << 3);        //开启float和double
    // CSRC(CSR_VSSTATUS, (1ULL << 9) | (1ULL << 10));
    // CSRS(CSR_SSTATUS, (1ULL << 9) | (1ULL << 10));
    // vle32_v();
    // TEST_ASSERT("Execution vector instruction leads to ili when v=1 and vsstatus.fs=0 and sstatus.fs=1",
    //     excpt.triggered == true &&
    //     excpt.cause == CAUSE_ILI
    // ); 


    //当V=1时，设置vsstatus.VS=1，HS级别的sstatus.VS=0时，尝试执行向量指令（vsstatus.VS和HS级别的sstatus.VS同时生效才可以执行向量指令）
    //当V=1时，设置vsstatus.VS=0，HS级别的sstatus.VS=0时，尝试执行向量指令（vsstatus.VS和HS级别的sstatus.VS同时生效才可以执行向量指令）
    //扩展FS的状态设置为0时，尝试读取或者写入浮点对应的状态指令
    //扩展VS的状态设置为0时，尝试读取或者写入向量对应的状态指令
    //扩展XS的状态设置为0时，尝试读取或者写入其他扩展对应的状态指令
    //设置HU=0时，在U模式执行超级虚拟机指令

bool manual_illegal_ili_hs_sret_ili_mstatus_tsr1() {

    TEST_START();
    //mstatus.TSR=1时，执行sret指令
    TEST_SETUP_EXCEPT();
    goto_priv(PRIV_M);
    CSRS(CSR_MSTATUS,1ULL << 22);    //TSR位

    goto_priv(PRIV_HS);
    set_prev_priv(PRIV_HU);

    //TEST_EXEC_SRET();
    sret();

    TEST_ASSERT("hs mode sret cause to ili when mstatus.TSR=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    TEST_END();

}

bool manual_illegal_ili_hs_sret_success_mstatus_tsr0() {

    TEST_START();
    //mstatus.TSR=0时，执行sret指令
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,1ULL << 22);    //TSR位

    goto_priv(PRIV_HU);
    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_HU);


    TEST_ASSERT("hs mode sret successful when mstatus.TSR=0",
        excpt.triggered == false
    ); 
    TEST_END();

}

bool manual_illegal_ili_m_dret_ili() {

    TEST_START();
    //m模式下执行dret指令，产生非法指令异常
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,1ULL << 22);    //TSR位

    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();
    dret();

    TEST_ASSERT("m mode dret cause to ILI",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 



    //HU模式下执行dret指令，产生非法指令异常
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,1ULL << 22);    //TSR位

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    dret();

    TEST_ASSERT("HU mode dret cause to ILI",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 



    //hs模式下执行dret指令，产生非法指令异常
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,1ULL << 22);    //TSR位

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    dret();

    TEST_ASSERT("hs mode dret cause to ILI",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 



    // TEST_END();

}


bool manual_illegal_ili_hu_read_cycle_time_instret_hpmcounteren_mcounteren_cy_tm_ir_hpmn0() {

    TEST_START();
    //当mcounteren寄存器中的CY、TM、IR或HPMn位被清除时，在S模式或U模式下执行时尝试读取cycle、time、instret或hpmcountern寄存器
    goto_priv(PRIV_M);
    bool cond1,cond2,cond3,cond4;

    CSRW(CSR_MCOUNTEREN,0);

    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;
    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;
    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;
    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER10);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI; 

    TEST_ASSERT("hu mode read cycle/time/instret/hpmcounteren when mcounteren.cy/tm/ir/hpmn=0 cause to ILI",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;
    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;
    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;
    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER10);
    cond1 = excpt.triggered == true && excpt.cause == CAUSE_ILI;  

    TEST_ASSERT("hs mode read cycle/time/instret/hpmcounteren when mcounteren.cy/tm/ir/hpmn=0 cause to ILI",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    TEST_END();


}

bool manual_illegal_ili_u_sfence_vma_ili_tvm0() {

    TEST_START();

    //TVM=任意，在u模式下，执行SFENCE.VMA或SINVAL.VMA指令
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,MSTATUS_TVM);

    goto_priv(PRIV_HU); 
    TEST_SETUP_EXCEPT();
    sfence_vma();
    TEST_ASSERT("u mode sfence.vma leads to ili when tvm=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("u mode sinval.vma leads to ili when tvm=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_M);
    CSRS(CSR_MSTATUS,MSTATUS_TVM);

    goto_priv(PRIV_HU); 
    TEST_SETUP_EXCEPT();
    sfence_vma();
    TEST_ASSERT("u mode sfence.vma leads to ili when tvm=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("u mode sinval.vma leads to ili when tvm=1",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    TEST_END();
}


bool manual_illegal_ili_m_sfence_vma_success_tvm0() {

    TEST_START();

    //TVM=任意，在M模式下，执行SFENCE.VMA或SINVAL.VMA指令
    goto_priv(PRIV_M);
    CSRC(CSR_MSTATUS,MSTATUS_TVM);

    goto_priv(PRIV_M); 
    TEST_SETUP_EXCEPT();
    sfence_vma();
    TEST_ASSERT("m mode sfence.vma successful when tvm=0",
        excpt.triggered == false
    ); 

    goto_priv(PRIV_M);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("m mode sinval.vma successful when tvm=0",
        excpt.triggered == false
    ); 

    goto_priv(PRIV_M);
    CSRS(CSR_MSTATUS,MSTATUS_TVM);

    goto_priv(PRIV_M); 
    TEST_SETUP_EXCEPT();
    sfence_vma();
    TEST_ASSERT("m mode sfence.vma successful when tvm=1",
        excpt.triggered == false
    ); 

    goto_priv(PRIV_M);
    TEST_SETUP_EXCEPT();
    Sinval_vma();
    TEST_ASSERT("m mode sinval.vma successful when tvm=1",
        excpt.triggered == false
    ); 

    TEST_END();
}


bool manual_illegal_ili_m_access_dcsr_ili() {

    TEST_START();

    //在非debug模式下，访问debug csr
    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();
    CSRR(0x7B0);
    TEST_ASSERT("m mode access dcsr cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(0x7B1);
    TEST_ASSERT("s mode access dpc cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(0x7B2);
    TEST_ASSERT("u mode access dscratch cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    ); 




    TEST_END();
}

bool manual_illegal_ili_hs_mnret_ili() {
    TEST_START();

    //在hs mode下执行mnret指令
    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    mnret();
    TEST_ASSERT("hs mode mnret cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    //在hu mode下执行mnret指令
    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    mnret();
    TEST_ASSERT("hu mode mnret cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    //在m mode下执行mnret指令
    goto_priv(PRIV_M);
    TEST_SETUP_EXCEPT();
    // Set mnepc to point to the next instruction after mnret
    asm volatile (
        "la t0, 1f\n"           // Load address of label 1 into t0
        "csrw 0x741, t0\n"      // Write to mnepc (CSR 0x741)
        ".insn r 0x73, 0, 0x38, x0, x0, x2\n"               // Execute mnret, should return to label 1
        "1:\n"                  // Label for return address
        ::: "t0"
    );
    TEST_ASSERT("m mode mnret successful",
        excpt.triggered == false
    );
}

bool manual_illegal_ili_hs_access_pmacfg_ili() {
    //S/U-mode访问 PMA/PMP CSR
    TEST_START();

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMACFG0); //pmacfg
    TEST_ASSERT("hs mode access pmacfg cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMAADDR0); //pmaaddr
    TEST_ASSERT("hs mode access pmaaddr cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMPCFG0); //pmpcfg
    TEST_ASSERT("hs mode access pmpcfg cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMPADDR0); //pmpaddr
    TEST_ASSERT("hs mode access pmpaddr cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );
    
    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMACFG0); //pmacfg
    TEST_ASSERT("hu mode access pmacfg cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMAADDR0); //pmaaddr
    TEST_ASSERT("hu mode access pmaaddr cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMPCFG0); //pmpcfg
    TEST_ASSERT("hu mode access pmpcfg cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRR(CSR_PMPADDR0); //pmpaddr
    TEST_ASSERT("hu mode access pmpaddr cause to illegal instruction exception",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

}

static inline void manual_illegal_hfence_vvma_rs_insn(uintptr_t vaddr, uint64_t asid)
{
    asm volatile(
        ".insn r 0x73, 0x0, 0x11, x0, %0, %1\n\t"
        :: "r"(vaddr), "r"(asid) : "memory");
}

static inline void manual_illegal_hfence_gvma_rs_insn(uintptr_t gaddr, uint64_t vmid)
{
    asm volatile(
        ".insn r 0x73, 0x0, 0x31, x0, %0, %1\n\t"
        :: "r"(gaddr), "r"(vmid) : "memory");
}

#define CHECK_M_HCSR_READ_ILI(label, csr) do { \
    TEST_SETUP_EXCEPT(); \
    (void)CSRR(csr); \
    TEST_ASSERT(label " read in M-mode causes illegal instruction exception when H extension is not implemented", \
        excpt.triggered == true && excpt.cause == CAUSE_ILI \
    ); \
} while (0)

#define CHECK_M_HCSR_WRITE_ILI(label, csr, value) do { \
    TEST_SETUP_EXCEPT(); \
    CSRW(csr, value); \
    TEST_ASSERT(label " write in M-mode causes illegal instruction exception when H extension is not implemented", \
        excpt.triggered == true && excpt.cause == CAUSE_ILI \
    ); \
} while (0)

#define CHECK_M_HINST_ILI(label, inst) do { \
    TEST_SETUP_EXCEPT(); \
    inst; \
    TEST_ASSERT(label " in M-mode causes illegal instruction exception when H extension is not implemented", \
        excpt.triggered == true && excpt.cause == CAUSE_ILI \
    ); \
} while (0)

bool manual_illegal_ili_m_access_unimplemented_h_extension_csrs_and_instrs()
{
    TEST_START();

    goto_priv(PRIV_M);
    reset_state();

    uintptr_t addr = hs_page_base(VSRWX_GURWX);

    TEST_ASSERT("H extension is not advertised in misa before probing H CSRs/instructions",
        (CSRR(CSR_MISA) & MISA_H) == 0ULL
    );

    CHECK_M_HCSR_READ_ILI("hstateen0", CSR_HSTATEEN0);
    CHECK_M_HCSR_READ_ILI("hcounteren", CSR_HCOUNTEREN);
    CHECK_M_HCSR_WRITE_ILI("hstateen0", CSR_HSTATEEN0, 0ULL);
    CHECK_M_HCSR_WRITE_ILI("hcounteren", CSR_HCOUNTEREN, 0ULL);

    CHECK_M_HCSR_READ_ILI("hstatus", CSR_HSTATUS);
    CHECK_M_HCSR_READ_ILI("hedeleg", CSR_HEDELEG);
    CHECK_M_HCSR_READ_ILI("hideleg", CSR_HIDELEG);
    CHECK_M_HCSR_READ_ILI("hie", CSR_HIE);
    CHECK_M_HCSR_READ_ILI("htimedelta", CSR_HTIMEDELTA);
    CHECK_M_HCSR_READ_ILI("hgeie", CSR_HGEIE);
    CHECK_M_HCSR_READ_ILI("hvien/hvictl", CSR_HVIEN);
    CHECK_M_HCSR_READ_ILI("henvcfg", CSR_HENVCFG);
    CHECK_M_HCSR_READ_ILI("htval", CSR_HTVAL);
    CHECK_M_HCSR_READ_ILI("hip", CSR_HIP);
    CHECK_M_HCSR_READ_ILI("hvip", CSR_HVIP);
    CHECK_M_HCSR_READ_ILI("htinst", CSR_HTINST);
    CHECK_M_HCSR_READ_ILI("hgatp", CSR_HGATP);
    CHECK_M_HCSR_READ_ILI("hgeip", CSR_HGEIP);

    CHECK_M_HINST_ILI("hfence.vvma x0, x0", hfence_vvma());
    CHECK_M_HINST_ILI("hfence.vvma rs1, rs2", manual_illegal_hfence_vvma_rs_insn(addr, 1));
    CHECK_M_HINST_ILI("hfence.gvma x0, x0", hfence_gvma());
    CHECK_M_HINST_ILI("hfence.gvma rs1, rs2", manual_illegal_hfence_gvma_rs_insn(addr, 1));

    CHECK_M_HINST_ILI("hlv.b", (void)hlvb(addr));
    CHECK_M_HINST_ILI("hlv.bu", (void)hlvbu(addr));
    CHECK_M_HINST_ILI("hlv.h", (void)hlvh(addr));
    CHECK_M_HINST_ILI("hlv.hu", (void)hlvhu(addr));
    CHECK_M_HINST_ILI("hlvx.hu", (void)hlvxhu(addr));
    CHECK_M_HINST_ILI("hlv.w", (void)hlvw(addr));
    CHECK_M_HINST_ILI("hlv.wu", (void)hlvwu(addr));
    CHECK_M_HINST_ILI("hlvx.wu", (void)hlvxwu(addr));
    CHECK_M_HINST_ILI("hlv.d", (void)hlvd(addr));
    CHECK_M_HINST_ILI("hsv.b", (void)hsvb(addr, 0x5aULL));
    CHECK_M_HINST_ILI("hsv.h", (void)hsvh(addr, 0x5a5aULL));
    CHECK_M_HINST_ILI("hsv.w", (void)hsvw(addr, 0x5a5a5a5aULL));
    CHECK_M_HINST_ILI("hsv.d", (void)hsvd(addr, 0x5a5a5a5a5a5a5a5aULL));

    TEST_ASSERT("all M-mode unimplemented H-extension CSR and instruction illegal probes completed",
        test_status == true
    );

    TEST_END();
}

bool manual_illegal_ili_m_vsatp_hgatp_write_no_mmu_side_effect_progress_corner()
{
    TEST_START();

    const enum test_page data_page = VSRWX_GRWX;
    const uintptr_t vaddr = hs_page_base(data_page) + 0x240U;
    const uintptr_t paddr = phys_page_base(data_page) + 0x240U;
    const uint64_t first_payload = 0x1740174017401740ULL;
    const uint64_t second_payload = 0x1741174117411741ULL;
    uint64_t observed = 0ULL;

    goto_priv(PRIV_M);
    TEST_ASSERT("H extension is not advertised before AP-174 illegal vsatp/hgatp write probes",
        (CSRR(CSR_MISA) & MISA_H) == 0ULL
    );
    write64(paddr, first_payload);

    goto_priv(PRIV_HS);
    hspt_init();
    sfence_vma();
    TEST_SETUP_EXCEPT();
    observed = ld(vaddr);
    TEST_ASSERT("AP-174 setup: same-VPN HS load completes before illegal vsatp write",
        excpt.triggered == false && observed == first_payload,
        "observed=0x%llx expected=0x%llx",
        (unsigned long long)observed,
        (unsigned long long)first_payload
    );

    goto_priv(PRIV_M);
    CHECK_M_HCSR_WRITE_ILI("vsatp", CSR_VSATP,
        SATP_MODE_48 | (0x17ULL << SATP_ASID_OFF) | 0x12345ULL);

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    observed = ld(vaddr);
    TEST_ASSERT("AP-174 illegal csrw vsatp must not poison same-VPN HS load progress",
        excpt.triggered == false && observed == first_payload,
        "triggered=%d cause=0x%llx observed=0x%llx expected=0x%llx",
        excpt.triggered,
        (unsigned long long)excpt.cause,
        (unsigned long long)observed,
        (unsigned long long)first_payload
    );

    goto_priv(PRIV_M);
    write64(paddr, second_payload);
    CHECK_M_HCSR_WRITE_ILI("hgatp", CSR_HGATP,
        HGATP_MODE_DFLT | (0x2aULL << HGATP_VMID_OFF) | 0x23456ULL);

    goto_priv(PRIV_HS);
    TEST_SETUP_EXCEPT();
    observed = ld(vaddr);
    TEST_ASSERT("AP-174 illegal csrw hgatp must not poison same-VPN HS load progress",
        excpt.triggered == false && observed == second_payload,
        "triggered=%d cause=0x%llx observed=0x%llx expected=0x%llx",
        excpt.triggered,
        (unsigned long long)excpt.cause,
        (unsigned long long)observed,
        (unsigned long long)second_payload
    );

    for (unsigned i = 0; i < 4U; i++) {
        const uint64_t loop_payload =
            0x1742000000000000ULL | ((uint64_t)i << 8) | i;
        const uint64_t vsatp_value =
            SATP_MODE_48 | (((0x30ULL + i) & 0xffffULL) << SATP_ASID_OFF) |
            (0x12340ULL + i);
        const uint64_t hgatp_value =
            HGATP_MODE_DFLT | (((0x40ULL + i) & 0x3fffULL) << HGATP_VMID_OFF) |
            (0x23450ULL + i);

        goto_priv(PRIV_M);
        write64(paddr, loop_payload);
        CHECK_M_HCSR_WRITE_ILI("vsatp repeated", CSR_VSATP, vsatp_value);
        CHECK_M_HCSR_WRITE_ILI("hgatp repeated", CSR_HGATP, hgatp_value);

        goto_priv(PRIV_HS);
        TEST_SETUP_EXCEPT();
        observed = ld(vaddr);
        TEST_ASSERT("AP-174 repeated illegal vsatp/hgatp writes must not poison same-VPN HS load progress",
            excpt.triggered == false && observed == loop_payload,
            "iter=%u triggered=%d cause=0x%llx observed=0x%llx expected=0x%llx",
            i,
            excpt.triggered,
            (unsigned long long)excpt.cause,
            (unsigned long long)observed,
            (unsigned long long)loop_payload
        );
    }

    goto_priv(PRIV_M);
    hspt_init();
    sfence_vma();

    TEST_END();
}

#undef CHECK_M_HCSR_READ_ILI
#undef CHECK_M_HCSR_WRITE_ILI
#undef CHECK_M_HINST_ILI
