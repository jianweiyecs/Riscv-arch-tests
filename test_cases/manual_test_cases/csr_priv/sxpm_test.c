#include <rvh_test.h>
#include <csrs.h> 
#include <page_tables.h>

//需要提前修改pma的相关寄存器

bool manual_sxpm_ssnpm_vu_senvcfg_pmm_00() {

    //VU 模式访问地址，senvcfg.PMM设置00，禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SENVCFG, SENVCFG_PMM3);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    
    goto_priv(PRIV_VU);

    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VU 模式访问地址,senvcfg.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );


    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VU 模式访问地址,senvcfg.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    TEST_END();
}

bool manual_sxpm_ssnpm_hu_senvcfg_pmm_00() {

    //HU 模式访问地址，senvcfg.PMM设置00，禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SENVCFG, SENVCFG_PMM3);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    
    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HU 模式访问地址,senvcfg.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HU 模式访问地址,senvcfg.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );


}


bool manual_sxpm_ssnpm_vu_senvcfg_pmm_10_pmlen_7_rv64() {

    //VU 模式访问地址，senvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    // vspt_init();
    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_VU);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VU模式访问地址,senvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFF00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VU模式访问地址,senvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 != 0xdeadbeef
    );

    TEST_END();
}




bool manual_sxpm_ssnpm_vs_henvcfg_pmm_10_pmlen_7_rv64() {

    //VS 模式访问地址，henvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, SENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef); excpt_info();
    // sd(vaddr2, 0xdeadbeef);

    // val1 = ld(vaddr1);
    val2 = ld(vaddr2);

    TEST_ASSERT("VS 模式访问地址,henvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        excpt.triggered == false &&
        val2 == 0xdeadbeef
    );

    TEST_END(); 
}


bool manual_sxpm_ssnpm_hu_senvcfg_pmm_10_pmlen_7_rv64() {

    //HU 模式访问地址，senvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;


    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HU模式访问地址,senvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );



    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFF00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HU模式访问地址,senvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 != 0xdeadbeef
    );


}



bool manual_sxpm_ssnpm_vu_senvcfg_pmm_11_pmlen_16_rv64() {

    //VU 模式访问地址，senvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    // vspt_init();
    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_VU);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VU 模式访问地址,senvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF800000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VU 模式访问地址,senvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 != 0xdeadbeef
    );

    TEST_END();
}



bool manual_sxpm_ssnpm_hu_senvcfg_pmm_11_pmlen_16_rv64() {

    //HU 模式访问地址，senvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;


    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HU 模式访问地址,senvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );



    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF800000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HU 模式访问地址,senvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 != 0xdeadbeef
    );


}


bool manual_sxpm_ssnpm_vu_x_only_senvcfg_pmm_10_mxr1() {

    //VU 模式访问X-only页面，senvcfg.PMM设置10，MXR=1,禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    
    goto_priv(PRIV_VU);
    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("VU 模式访问X-only页面,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );


    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("VU 模式访问X-only页面,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_hu_x_only_senvcfg_pmm_10_mxr1() {

    //HU 模式访问X-only页面，senvcfg.PMM设置10，MXR=1,禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;
    
    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val1 = ld(vaddr1);


    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("HU 模式访问X-only页面,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        val2 != val1
    );

    TEST_END();
}



bool manual_sxpm_ssnpm_vu_x_only_senvcfg_pmm_11_mxr1() {

    //VU 模式访问X-only页面，senvcfg.PMM设置11，MXR=1,禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    
    goto_priv(PRIV_VU);
    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("VU 模式访问X-only页面,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );


    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("VU 模式访问X-only页面,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_hu_x_only_senvcfg_pmm_11_mxr1() {

    //HU 模式访问X-only页面，senvcfg.PMM设置11，MXR=1,禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;
    
    goto_priv(PRIV_HU);
    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val1 = ld(vaddr1);


    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("HU 模式访问X-only页面,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        val2 != val1
    );

    TEST_END();
}



bool manual_sxpm_ssnpm_hs_vu_senvcfg_pmm_00() {

    //HS模式按照VU模式显示内存访问，senvcfg.PMM设置00

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_HSTATUS,HSTATUS_SPV);
    CSRC(CSR_HSTATUS,HSTATUS_SPVP);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    vaddr2 = vs_page_base(SWITCH1); 
    hsvd(vaddr2, 0xdeadbeef);
    excpt_info();


    TEST_SETUP_EXCEPT();
    val2 = hlvd(vaddr1);

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2); 

    hsvd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = hlvd(vaddr1);

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_m_vu_senvcfg_pmm_00() {

    //M模式按照VU模式显示内存访问，senvcfg.PMM设置00

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS,MSTATUS_MPRV);
    CSRS(CSR_MSTATUS,1ULL << 11);
    CSRC(CSR_MSTATUS,1ULL << 12);
    CSRS(CSR_HSTATUS,HSTATUS_SPV);
    CSRC(CSR_HSTATUS,HSTATUS_SPVP);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_hs_vu_senvcfg_pmm_10_pmlen_7_rv64() {

    //HS模式按照VU模式显示内存访问，senvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRS(CSR_HSTATUS,HSTATUS_SPV);
    CSRC(CSR_HSTATUS,HSTATUS_SPVP);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);excpt_info();

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_m_vu_senvcfg_pmm_10_mxr0_pmlen_7_rv64() {

    //M模式按照VU模式显示内存访问，senvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS,MSTATUS_MPRV);
    CSRS(CSR_MSTATUS,1ULL << 11);
    CSRC(CSR_MSTATUS,1ULL << 12);
    CSRS(CSR_HSTATUS,HSTATUS_SPV);
    CSRC(CSR_HSTATUS,HSTATUS_SPVP);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置10,MXR=0,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置10,MXR=0,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    TEST_END();
}





bool manual_sxpm_ssnpm_hs_vu_senvcfg_pmm_11_pmlen_16_rv64() {

    //HS模式按照VU模式显示内存访问，senvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_HSTATUS,HSTATUS_SPV);
    CSRC(CSR_HSTATUS,HSTATUS_SPVP);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);excpt_info();

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_m_vu_senvcfg_pmm_11_mxr0_pmlen_16_rv64() {

    //M模式按照VU模式显示内存访问，senvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS,MSTATUS_MPRV);
    CSRS(CSR_MSTATUS,1ULL << 11);
    CSRC(CSR_MSTATUS,1ULL << 12);
    CSRS(CSR_HSTATUS,HSTATUS_SPV);
    CSRC(CSR_HSTATUS,HSTATUS_SPVP);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置11,MXR=0,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2); 

    sd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置11,MXR=0,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    TEST_END();
}



bool manual_sxpm_ssnpm_hs_vu_senvcfg_pmm_10_mxr1() {

    //HS模式按照VU模式显示内存访问，senvcfg.PMM设置10，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvwu(vaddr1);
    val2 = hlvwu(vaddr2);

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_m_vu_senvcfg_pmm_10_mxr1() {

    //M模式按照VU模式显示内存访问，senvcfg.PMM设置10，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvwu(vaddr1);
    val2 = hlvwu(vaddr2);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_hs_vu_senvcfg_pmm_11_mxr1() {

    //HS模式按照VU模式显示内存访问，senvcfg.PMM设置11，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvwu(vaddr1);
    val2 = hlvwu(vaddr2);

    TEST_ASSERT("HS模式按照VU模式显示内存访问,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_m_vu_senvcfg_pmm_11_mxr1() {

    //M模式按照VU模式显示内存访问，senvcfg.PMM设置11，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvwu(vaddr1);
    val2 = hlvwu(vaddr2);

    TEST_ASSERT("M模式按照VU模式显示内存访问,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}


bool manual_sxpm_ssnpm_hs_vs_henvcfg_pmm_00() {

    //HS模式按照VS模式显示内存访问，henvcfg.PMM设置00

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_HENVCFG, HENVCFG_PMM3);
    printf("HENVCFG=%llx\n",CSRR(CSR_HENVCFG));

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xaaffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("HS模式按照VS模式显示内存访问,henvcfg.PMM设置00,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END();
}

bool manual_sxpm_ssnpm_m_vs_henvcfg_pmm_00() {

    //M模式按照VS模式显示内存访问，henvcfg.PMM设置00

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_HENVCFG, HENVCFG_PMM3);
    printf("HENVCFG=%llx\n",CSRR(CSR_HENVCFG));

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xaaffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("M模式按照VS模式显示内存访问,henvcfg.PMM设置00,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END();
}




bool manual_sxpm_ssnpm_hs_vs_henvcfg_pmm_10_pmlen_7_rv64() {

    //HS模式按照VS模式显示内存访问，henvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("HS模式按照VS模式显示内存访问,henvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        excpt.triggered == false &&
        val2 == val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_m_vs_henvcfg_pmm_10_mxr0_pmlen_7_rv64() {

    //M模式按照VS模式显示内存访问，henvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("M模式按照VS模式显示内存访问,henvcfg.PMM设置10,MXR=0,开启指针掩码,PMLEN=7(RV64)",
        excpt.triggered == false &&
        val2 == val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_hs_vs_henvcfg_pmm_11_mxr0_pmlen_16_rv64() {

    //HS模式按照VS模式显示内存访问，henvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xabcdffffffffffff;
    vaddr2 = 0xacbeffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);
    TEST_ASSERT("HS模式按照VS模式显示内存访问,henvcfg.PMM设置11,MXR=0,开启指针掩码,PMLEN=16(RV64)",
        excpt.triggered == false &&
        val2 == val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_m_vs_henvcfg_pmm_11_mxr0_pmlen_16_rv64() {

    //M模式按照VS模式显示内存访问，henvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xabcdffffffffffff;
    vaddr2 = 0xacbeffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("M模式按照VS模式显示内存访问,henvcfg.PMM设置11,MXR=0,开启指针掩码,PMLEN=16(RV64)",
        excpt.triggered == false &&
        val2 == val1
    );

    TEST_END(); 
}


bool manual_sxpm_ssnpm_hs_vs_henvcfg_pmm_10_mxr1() {

    //HS模式按照VS模式显示内存访问，henvcfg.PMM设置10，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("HS模式按照VS模式显示内存访问,henvcfg.PMM设置10,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_m_vs_henvcfg_pmm_10_mxr1() {

    //M模式按照VS模式显示内存访问，henvcfg.PMM设置10，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("M模式按照VS模式显示内存访问,henvcfg.PMM设置10,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_hs_vs_henvcfg_pmm_11_mxr1() {

    //HS模式按照VS模式显示内存访问，henvcfg.PMM设置11，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("HS模式按照VS模式显示内存访问,henvcfg.PMM设置11,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_m_vs_henvcfg_pmm_11_mxr1() {

    //M模式按照VS模式显示内存访问，henvcfg.PMM设置11，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    vaddr1 = 0xafffffffffffffff;
    vaddr2 = 0xabffffffffffffff;
    hsvw(vaddr1, 0xdeadbeef);
    hsvw(vaddr2, 0xdeadbeef);

    val1 = hlvw(vaddr1);
    val2 = hlvw(vaddr2);

    TEST_ASSERT("M模式按照VS模式显示内存访问,henvcfg.PMM设置11,MXR=1,禁用指针掩码",
        excpt.triggered == false &&
        val2 != val1
    );

    TEST_END(); 
}




bool manual_sxpm_ssnpm_hu_hlv_hsv_hstatus_hupmm_00() {

    //HU 模式执行hlv、hsv，hstatus.HUPMM设置00

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_HSTATUS, HSTATUS_HUPMM3);
    CSRS(CSR_HSTATUS,HSTATUS_SPVP);
    CSRS(CSR_HSTATUS,HSTATUS_HU);
    
    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;
    
    goto_priv(PRIV_HU);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    hsvw(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val1 = hlvw(vaddr1);

    TEST_ASSERT("HU 模式执行hlv、hsv,hstatus.HUPMM设置00,禁用指针掩码",
        val1 != 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    hsvd(vaddr2, 0xdeadbeef);

    val1 = hlvd(vaddr1);
    TEST_ASSERT("HU 模式执行hlv、hsv,hstatus.HUPMM设置00,禁用指针掩码",
        val1 == 0xdeadbeef
    );

    TEST_END();
}



bool manual_sxpm_ssnpm_hu_hlv_hsv_hstatus_hupmm_10_mxr0_pmlen_7_rv64() {

    //HU 模式执行hlv、hsv，hstatus.HUPMM设置10,MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HSTATUS, HSTATUS_HUPMM2);
    CSRS(CSR_MENVCFG, MENVCFG_PMM2);
    CSRS(CSR_SENVCFG, SENVCFG_PMM2);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);

    CSRS(CSR_HSTATUS, HSTATUS_SPVP);
    CSRS(CSR_HSTATUS, HSTATUS_HU);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;
    
    goto_priv(PRIV_HU);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    hsvd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val1 = hlvd(vaddr1);

    TEST_ASSERT("HU 模式执行hlv、hsv,hstatus.HUPMM设置10,MXR=0,开启指针掩码,PMLEN=7(RV64)",
        val1 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    hsvd(vaddr2, 0xdeadbeef);

    val1 = hlvd(vaddr1);
    TEST_ASSERT("HU 模式执行hlv、hsv,hstatus.HUPMM设置10,MXR=0,开启指针掩码,PMLEN=7(RV64)",
        val1 == 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_hu_hlv_hsv_hstatus_hupmm_11_mxr0_pmlen_16_rv64() {

    //HU 模式执行hlv、hsv，hstatus.HUPMM设置11,MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HSTATUS, HSTATUS_HUPMM3);
    CSRS(CSR_HSTATUS, HSTATUS_SPVP);
    CSRS(CSR_HSTATUS, HSTATUS_HU);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HU);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    hsvd(vaddr2, 0xdeadbeef);

    TEST_SETUP_EXCEPT();
    val1 = hlvd(vaddr1);
    printf("val1=%lx\n",val1);

    TEST_ASSERT("HU 模式执行hlv、hsv,hstatus.HUPMM设置11,MXR=0,开启指针掩码,PMLEN=16(RV64)",
        val1 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    hsvd(vaddr2, 0xdeadbeef);

    val1 = hlvd(vaddr1);
    TEST_ASSERT("HU 模式执行hlv、hsv,hstatus.HUPMM设置11,MXR=0,开启指针掩码,PMLEN=16(RV64)",
        val1 == 0xdeadbeef
    );

    TEST_END();
}

bool manual_sxpm_ssnpm_hu_hlv_hsv_x_only_hstatus_hupmm_10_mxr0() {

    //HU 模式执行hlv、hsv，hstatus.HUPMM设置10,显示访问X-only页面，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HSTATUS, HSTATUS_HUPMM2);
    CSRS(CSR_HSTATUS, HSTATUS_SPVP);
    CSRS(CSR_HSTATUS, HSTATUS_HU);

    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);
    
    goto_priv(PRIV_HU);

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(VSX_GUX);
    printf("vaddr2 = 0x%lx\n", vaddr2);

    val2 = hlvd(vaddr2);
    TEST_SETUP_EXCEPT();
    val1 = hlvd(vaddr1);

    TEST_ASSERT("HU 模式执行hlv、hsv显示访问X-only页面,hstatus.HUPMM设置10,MXR=0,禁用指针掩码",
        val2 != val1
    );

    TEST_END(); 
}


bool manual_sxpm_ssnpm_hu_hlv_hsv_x_only_hstatus_hupmm_11_mxr0() {

    //HU 模式执行hlv、hsv，hstatus.HUPMM设置11,显示访问X-only页面，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HSTATUS, HSTATUS_HUPMM2);
    CSRS(CSR_HSTATUS, HSTATUS_SPVP);
    CSRS(CSR_HSTATUS, HSTATUS_HU);

    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR); 

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);
    
    goto_priv(PRIV_HU);

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(VSX_GUX);
    printf("vaddr2 = 0x%lx\n", vaddr2);

    val2 = hlvd(vaddr2);
    TEST_SETUP_EXCEPT();
    val1 = hlvd(vaddr1);

    TEST_ASSERT("HU 模式执行hlv、hsv显示访问X-only页面,hstatus.HUPMM设置11,MXR=0,禁用指针掩码",
        val2 != val1
    );

    TEST_END(); 
}

bool manual_sxpm_ssnpm_hs_hlvwxu_henvcfg_pmm_10_mxr0() {

    //HS模式执行hlvxwu，henvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    printf("vaddr2 = 0x%lx\n", vaddr2);
    TEST_SETUP_EXCEPT();

    hsvw(vaddr2, 0xdeadbeef);excpt_info();
    TEST_SETUP_EXCEPT();

    val2 = hlvxwu(vaddr2);excpt_info();
    TEST_SETUP_EXCEPT();
    val1 = hlvxwu(vaddr1);excpt_info();

    TEST_ASSERT("HS模式执行hlvwxu,henvcfg.PMM设置10,MXR=0,禁用指针掩码",
        val2 != val1
    );

    TEST_END(); 
}


bool manual_sxpm_ssnpm_vs_henvcfg_pmm_00() {

    //VS 模式访问地址，henvcfg.PMM设置00，禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_HENVCFG, HENVCFG_PMM3);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();

    goto_priv(PRIV_VS);

    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VS 模式访问地址,henvcfg.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );


    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VS 模式访问地址,henvcfg.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    TEST_END();
}



bool manual_sxpm_ssnpm_vs_henvcfg_pmm_10_explicit_pmlen_7_rv64() {

    //VS 模式访问地址，henvcfg.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();

    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VS模式访问地址,henvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFF00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VS模式访问地址,henvcfg.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 != 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_vs_henvcfg_pmm_11_pmlen_16_rv64() {

    //VS 模式访问地址，henvcfg.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();

    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VS 模式访问地址,henvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF800000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("VS 模式访问地址,henvcfg.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 != 0xdeadbeef
    );

    TEST_END();
}




bool manual_sxpm_ssnpm_vs_x_only_senvcfg_pmm_10_mxr1() {

    //VS 模式访问X-only页面，henvcfg.PMM设置10，MXR=1,禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();

    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("VS 模式访问X-only页面,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );


    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("VS 模式访问X-only页面,senvcfg.PMM设置10,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_vs_x_only_senvcfg_pmm_11_mxr1() {

    //VS 模式访问X-only页面，henvcfg.PMM设置11，MXR=1,禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);


    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();

    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("VS 模式访问X-only页面,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );


    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("VS 模式访问X-only页面,senvcfg.PMM设置11,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_ssnpm_vu_senvcfg_pmm_10_mxr0() {

    //VU 模式访问地址，senvcfg.PMM设置10，MXR=0 

    TEST_START();    
    goto_priv(PRIV_M); 
    CSRS(CSR_MENVCFG, MENVCFG_PMM3);
    CSRS(CSR_HENVCFG, HENVCFG_PMM3);
    CSRS(CSR_SENVCFG, SENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, HSTATUS_HUPMM);
    printf("MENVCFG=%llx\n",CSRR(CSR_MENVCFG));  
    printf("SENVCFG=%llx\n",CSRR(CSR_SENVCFG));  
    printf("HENVCFG=%llx\n",CSRR(CSR_HENVCFG));  
    printf("mstatus=%llx\n",CSRR(CSR_MSTATUS));
    printf("hstatus=%llx\n",CSRR(CSR_HSTATUS));

    uintptr_t vaddr,vaddr1;
    uint64_t val,val2;
    TEST_SETUP_EXCEPT();

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);
    // TEST_SETUP_EXCEPT();

    printf("nihao\n");
    vaddr = 0x80000000000;
    vaddr = 0x70000000000;

    sd(vaddr, 0xdeadbeef);
    printf("vaddr=%llx\n",vaddr);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr-0x2);
    printf("ld = %llx\n",ld(vaddr));
    excpt_info();

    printf("ld = %llx\n",ld(vaddr1));

    TEST_SETUP_EXCEPT();
    excpt_info();


    TEST_ASSERT("VU 模式访问地址,senvcfg.PMM设置10,MXR=0",
        excpt.triggered == false &&
        val2 == 0xdeadbeef
    );

    TEST_END(); 
}










bool manual_sxpm_smnpm_hs_menvcfg_pmm_00() {

    //HS 模式访问地址，MENVCFG.PMM设置00，禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MENVCFG, MENVCFG_PMM3);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HS 模式访问地址,MENVCFG.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );


    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HS 模式访问地址,MENVCFG.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    TEST_END();
}


bool manual_sxpm_smnpm_hs_menvcfg_pmm_10_pmlen_7_rv64() {

    //HS 模式访问地址，MENVCFG.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MENVCFG, MENVCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HS 模式访问地址,MENVCFG.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFF00000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HS 模式访问地址,MENVCFG.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 != 0xdeadbeef
    );

    TEST_END(); 
}




bool manual_sxpm_smnpm_hs_menvcfg_pmm_11_pmlen_16_rv64() {

    //HS 模式访问地址，MENVCFG.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MENVCFG, MENVCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    vaddr1 = vs_page_base(SWITCH1);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH1);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HS 模式访问地址,MENVCFG.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(SWITCH2);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF800000000000ULL);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    vaddr2 = vs_page_base(SWITCH2);
    sd(vaddr1, 0xdeadbeef);

    val2 = ld(vaddr2);

    TEST_ASSERT("HS 模式访问地址,MENVCFG.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 != 0xdeadbeef
    );


    TEST_END(); 
}


bool manual_sxpm_smnpm_hs_x_only_menvcfg_pmm_10_mxr1() {

    //HS模式显示访问X-only页面，menvcfg.PMM设置10，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MENVCFG, MENVCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("HS 模式访问X-only页面,MENVCFG.PMM设置10,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFE00000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("HS 模式访问X-only页面,MENVCFG.PMM设置10,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END(); 
}


bool manual_sxpm_smnpm_hs_x_only_menvcfg_pmm_11_mxr1() {

    //HS模式显示访问X-only页面，menvcfg.PMM设置11，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MENVCFG, MENVCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);

    uintptr_t vaddr1,vaddr2;
    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(vaddr1);

    TEST_ASSERT("HS 模式访问X-only页面,MENVCFG.PMM设置11,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    vaddr1 = vs_page_base(VSX_GUX);
    printf("vaddr1 = 0x%lx\n", vaddr1);
    vaddr1 = vaddr1 | (0xFFFF000000000000ULL);

    val2 = ld(vaddr1);

    TEST_ASSERT("HS 模式访问X-only页面,MENVCFG.PMM设置11,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END(); 
}








bool manual_sxpm_smmpm_m_mseccfg_pmm_00() {

    //M 模式访问地址，MSECCFG.PMM设置00，禁用指针掩码

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MSECCFG, MSECCFG_PMM3);

    uintptr_t addr1,addr2;
    uint64_t val1,val2;

    hspt_init();
    
    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();
    addr1 = phys_page_base(SWITCH1);
    printf("addr1 = 0x%lx\n", addr1);
    addr1 = addr1 | (0xFE00000000000000ULL);
    printf("addr1 = 0x%lx\n", addr1);

    addr2 = phys_page_base(SWITCH1);
    sd(addr1, 0xdeadbeef);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问地址,MSECCFG.PMM设置00,禁用指针掩码",
        val2 != 0xdeadbeef
    );


    addr1 = phys_page_base(SWITCH2);
    printf("addr1 = 0x%lx\n", addr1);

    addr2 = phys_page_base(SWITCH2);
    sd(addr1, 0xdeadbeef);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问地址,MSECCFG.PMM设置00,禁用指针掩码",
        val2 == 0xdeadbeef
    );

}


bool manual_sxpm_smmpm_m_mseccfg_pmm_10_pmlen_7_rv64() {

    //M 模式访问地址，MSECCFG.PMM设置10，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MSECCFG, MSECCFG_PMM2);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t addr1,addr2;
    uint64_t val1,val2;

    hspt_init();

    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();
    addr1 = phys_page_base(SWITCH1);
    printf("addr1 = 0x%lx\n", addr1);
    addr1 = addr1 | (0xFE00000000000000ULL);
    printf("addr1 = 0x%lx\n", addr1);

    addr2 = phys_page_base(SWITCH1);
    sd(addr1, 0xdeadbeef);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问地址,MSECCFG.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 == 0xdeadbeef
    );

    addr1 = phys_page_base(SWITCH2);
    printf("addr1 = 0x%lx\n", addr1);
    addr1 = addr1 | (0xFF00000000000000ULL);
    printf("addr1 = 0x%lx\n", addr1);

    addr2 = phys_page_base(SWITCH2);
    sd(addr1, 0xdeadbeef);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问地址,MSECCFG.PMM设置10,开启指针掩码,PMLEN=7(RV64)",
        val2 != 0xdeadbeef
    );

    TEST_END(); 
}




bool manual_sxpm_smmpm_m_mseccfg_pmm_11_pmlen_16_rv64() {

    //M 模式访问地址，MSECCFG.PMM设置11，MXR=0

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MSECCFG, MSECCFG_PMM3);
    CSRC(CSR_MSTATUS, MSTATUS_MXR);

    uintptr_t addr1,addr2;
    uint64_t val1,val2;

    hspt_init();

    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();
    addr1 = phys_page_base(SWITCH1);
    printf("addr1 = 0x%lx\n", addr1);
    addr1 = addr1 | (0xFFFF000000000000ULL);
    printf("addr1 = 0x%lx\n", addr1);

    addr2 = phys_page_base(SWITCH1);
    sd(addr1, 0xdeadbeef);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问地址,MSECCFG.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 == 0xdeadbeef
    );

    addr1 = phys_page_base(SWITCH2);
    printf("addr1 = 0x%lx\n", addr1);
    addr1 = addr1 | (0xFFFF800000000000ULL);
    printf("addr1 = 0x%lx\n", addr1);

    addr2 = phys_page_base(SWITCH2);
    sd(addr1, 0xdeadbeef);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问地址,MSECCFG.PMM设置11,开启指针掩码,PMLEN=16(RV64)",
        val2 != 0xdeadbeef
    );


    TEST_END(); 
}


bool manual_sxpm_smmpm_m_x_only_mseccfg_pmm_10_mxr1() {

    //M模式显示访问X-only页面，MSECCFG.PMM设置10，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MSECCFG, MSECCFG_PMM2);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1 = phys_page_base(VSX_GUX);
    uintptr_t addr2 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);

    uint64_t val1,val2;

    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();
    vspt_init();
    
    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();

    addr1 = phys_page_base(VSX_GUX);
    printf("addr1 = 0x%lx\n", addr1);

    TEST_SETUP_EXCEPT();
    val2 = ld(addr1);

    TEST_ASSERT("M 模式访问X-only页面,MSECCFG.PMM设置10,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    addr1 = phys_page_base(VSX_GUX);
    printf("addr1 = 0x%lx\n", addr1);
    addr1 = addr1 | (0xFE00000000000000ULL);

    val2 = ld(addr1);

    TEST_ASSERT("M 模式访问X-only页面,MSECCFG.PMM设置10,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END(); 
}


bool manual_sxpm_smmpm_m_x_only_mseccfg_pmm_11_mxr1() {

    //M模式显示访问X-only页面，MSECCFG.PMM设置11，MXR=1

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MSECCFG, MSECCFG_PMM3);
    CSRS(CSR_MSTATUS, MSTATUS_MXR);
    CSRS(CSR_HSTATUS, MSTATUS_MXR);
    CSRS(CSR_SSTATUS, MSTATUS_MXR);

    uintptr_t addr1,addr2;
    addr1 = phys_page_base(VSX_GUX);
    addr2 = phys_page_base(VSX_GUX);
    write64(addr1, 0xdeadbeef);

    uint64_t val1,val2;

    hspt_init();
    
    goto_priv(PRIV_M);

    TEST_SETUP_EXCEPT();
    printf("addr1 = 0x%lx\n", addr1);
    val2 = ld(addr1);
    TEST_ASSERT("M 模式访问X-only页面,MSECCFG.PMM设置11,MXR=1,禁用指针掩码",
        val2 == 0xdeadbeef
    );

    addr2 = addr2 | (0xFFFF000000000000ULL);
    printf("addr2 = 0x%lx\n", addr2);

    val2 = ld(addr2);

    TEST_ASSERT("M 模式访问X-only页面,MSECCFG.PMM设置11,MXR=1,禁用指针掩码",
        val2 != 0xdeadbeef
    );

    TEST_END(); 
}