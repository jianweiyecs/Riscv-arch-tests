#include <rvh_test.h>
#include <csrs.h> 

bool manual_zicntr_time_hu_access_time_ili_mcounteren_tm0() {


    //当mcounteren.TM=0时，hu/hs 不可访问time寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN_TM);


    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hu access to time casuses illegal instruction exception when mcounteren.tm=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hs access to time casuses illegal instruction exception when mcounteren.tm=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );
    

}

bool manual_zicntr_time_m_access_time_success_mcounteren_tm1() {

    //当mcounteren.TM=1时，m、hs 可访问time寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_TM);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("m access to time successful when mcounteren.tm=1",
        excpt.triggered == false
    );


    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hs access to time successful when mcounteren.tm=1",
        excpt.triggered == false
    );
    TEST_END(); 
}

bool manual_zicntr_time_hu_access_time_ili_mcounteren_tm1_scounteren_tm0() {

    //当mcounteren.TM=1,scounteren=0时，hu 不可访问time寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_TM); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_TM); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hu access to time casuses illegal instruction exception when mcounteren.TM=1,scounteren.tm=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}

bool manual_zicntr_time_m_access_time_success_mcounteren_tm1_scounteren_tm1() {


    //当mcounteren.TM=1,scounteren=1时，m、hu、hs 可访问time寄存器
    
    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SCOUNTEREN, HCOUNTEREN_TM); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_TM); 

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("m access to time successful when mcounteren.tm=1 scounteren.tm=1",
        excpt.triggered == false
    );


    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hu access to time successful when mcounteren.tm=1 scounteren.tm=1",
        excpt.triggered == false
    );

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hs access to time successful when mcounteren.tm=1 scounteren.tm=1",
        excpt.triggered == false
    );

    TEST_END(); 
}


bool manual_zicntr_time_hu_read_time_ili_after_mcounteren_set_scounteren_clear() {

    //当mcounteren.TM=1,scounteren=0时，hu不可访问time寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_TM); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_TM); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("hu access to time casuses illegal instruction exception when mcounteren.tm=1 scounteren.tm=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}

bool manual_zicntr_time_m_access_time_success_except_mcounteren_tm0_scounteren_tm0() {

    //当mcounteren.TM=0,scounteren=0时，m可访问time寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN_TM); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_TM); 


    TEST_SETUP_EXCEPT();
    CSRR(CSR_TIME);

    TEST_ASSERT("M access to time successful exception when mcounteren.tm=0 scounteren.tm=0",
        excpt.triggered == false
    );

}

bool manual_zicntr_cycle_hu_csrr_cycle_ili_mcounteren_cy0() {


    //当mcounteren.CY=0时，hu/hs 不可访问cycle寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN_CY);


    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hu CSRR cycle casuses illegal instruction exception when mcounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hu CSRS(imm=0) cycle casuses illegal instruction exception when mcounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );


    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hs CSRR cycle casuses illegal instruction exception when mcounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );
    
    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hs CSRS(imm=0) cycle casuses illegal instruction exception when mcounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );


}

bool manual_zicntr_cycle_m_csrr_cycle_success_mcounteren_cy1() {

    //当mcounteren.CY=1时，m、hs 可访问cycle寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_CY);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("m CSRR cycle successful when mcounteren.CY=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("m CSRS(imm=0) cycle successful when mcounteren.CY=1",
        excpt.triggered == false
    );



    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hs CSRR cycle successful when mcounteren.CY=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hs CSRS(imm=0) cycle successful when mcounteren.CY=1",
        excpt.triggered == false
    );


    TEST_END(); 
}

bool manual_zicntr_cycle_hu_csrr_cycle_ili_mcounteren_cy1_scounteren_cy0() {

    //当mcounteren.CY=1,scounteren=0时，hu 不可访问cycle寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_CY); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_CY); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hu CSRR cycle casuses illegal instruction exception when mcounteren.CY=1,scounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hu CSRS(imm=0) cycle casuses illegal instruction exception when mcounteren.CY=1,scounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );


    TEST_END(); 
}

bool manual_zicntr_cycle_m_csrr_cycle_success_mcounteren_cy1_scounteren_cy1() {


    //当mcounteren.CY=1,scounteren=1时，m、hu、hs可访问cycle寄存器
    
    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SCOUNTEREN, HCOUNTEREN_CY); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_CY); 

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("m CSRR cycle successful when mcounteren.CY=1 scounteren.CY=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("m CSRS(imm=0) cycle successful when mcounteren.CY=1 scounteren.CY=1",
        excpt.triggered == false
    );

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hu CSRR cycle successful when mcounteren.CY=1 scounteren.CY=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hu CSRS(imm=0) cycle successful when mcounteren.CY=1 scounteren.CY=1",
        excpt.triggered == false
    );


    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hs CSRR cycle successful when mcounteren.CY=1 scounteren.CY=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hs CSRS(imm=0) cycle successful when mcounteren.CY=1 scounteren.CY=1",
        excpt.triggered == false
    );

    TEST_END(); 
}

bool manual_zicntr_cycle_hu_csrr_csrs_ili_after_mcounteren_set_scounteren_clear() {

    //当mcounteren.CY=1,scounteren=0时，hu不可访问cycle寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_CY); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_CY); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("hu CSRR cycle casuses illegal instruction exception when mcounteren.CY=1 scounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("hu CSRS(imm=0) cycle casuses illegal instruction exception when mcounteren.CY=1 scounteren.CY=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}

bool manual_zicntr_cycle_m_csrr_cycle_success_except_mcounteren_cy0_scounteren_cy0() {

    //当mcounteren.CY=0,scounteren=0时，m可访问cycle寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN_CY); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_CY); 


    TEST_SETUP_EXCEPT();
    CSRR(CSR_CYCLE);

    TEST_ASSERT("M CSRR cycle successful exception when mcounteren.cy=0 scounteren.cy=0",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_CYCLE, 0);

    TEST_ASSERT("M CSRS(imm=0) cycle successful exception when mcounteren.cy=0 scounteren.cy=0",
        excpt.triggered == false
    );

}

bool manual_zicntr_instret_hu_csrr_instret_ili_mcounteren_ir0() {


    //当mcounteren.IR=0时，hu/hs 不可访问instret寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN_IR);


    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("hu CSRR instret casuses illegal instruction exception when mcounteren.IR=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("hs CSRS(imm=0) instret casuses illegal instruction exception when mcounteren.IR=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    

}

bool manual_zicntr_instret_m_csrr_instret_success_mcounteren_ir1() {

    //当mcounteren.IR=1时，m、hs 可访问instret寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_IR);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("m CSRR instret successful when mcounteren.IR=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("m CSRS(imm=0) instret successful when mcounteren.IR=1",
        excpt.triggered == false
    );


    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("hs CSRR instret successful when mcounteren.IR=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("hs CSRS(imm=0) instret successful when mcounteren.IR=1",
        excpt.triggered == false
    );

    TEST_END(); 
}

bool manual_zicntr_instret_hu_csrr_instret_ili_mcounteren_ir1_scounteren_ir0() {

    //当mcounteren.IR=1,scounteren=0时，hu 不可访问instret寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_IR); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_IR); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("hu CSRR instret casuses illegal instruction exception when mcounteren.IR=1,scounteren.IR=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET,0);

    TEST_ASSERT("hu CSRS(imm=0) instret casuses illegal instruction exception when mcounteren.IR=1,scounteren.IR=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}

bool manual_zicntr_instret_m_csrr_instret_success_mcounteren_ir1_scounteren_ir1() {


    //当mcounteren.IR=1,scounteren=1时，m、hu、hs 可访问instret寄存器
    
    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SCOUNTEREN, HCOUNTEREN_IR); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_IR); 

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("m CSRR instret successful when mcounteren.IR=1 scounteren.IR=1",
        excpt.triggered == false
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("m CSRS(imm=0) instret successful when mcounteren.IR=1 scounteren.IR=1",
        excpt.triggered == false
    );

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("hu CSRR instret successful when mcounteren.IR=1 scounteren.IR=1",
        excpt.triggered == false
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("hu CSRS(imm=0) instret successful when mcounteren.IR=1 scounteren.IR=1",
        excpt.triggered == false
    );


    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("hs CSRR instret successful when mcounteren.IR=1 scounteren.IR=1",
        excpt.triggered == false
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("hs CSRS(imm=0) instret successful when mcounteren.IR=1 scounteren.IR=1",
        excpt.triggered == false
    );


    TEST_END(); 
}


bool manual_zicntr_instret_hu_csrr_csrs_ili_after_mcounteren_set_scounteren_clear() {

    //当mcounteren.IR=1,scounteren=0时，hu不可访问instret寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN_IR); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_IR); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("hu CSRR instret casuses illegal instruction exception when mcounteren.IR=1 scounteren.IR=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("hu CSRS(imm=0) instret casuses illegal instruction exception when mcounteren.IR=1 scounteren.IR=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}


bool manual_zicntr_instret_m_csrr_instret_success_except_mcounteren_ir0_scounteren_ir0() {

    //当mcounteren.IR=0,hcounteren=0,scounteren=0时，m可访问INSTRET寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN_IR); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN_IR); 


    TEST_SETUP_EXCEPT();
    CSRR(CSR_INSTRET);

    TEST_ASSERT("M CSRR INSTRET successful exception when mcounteren.IR=0 scounteren.IR=0",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_INSTRET, 0);

    TEST_ASSERT("M CSRS(imm=0) INSTRET successful exception when mcounteren.IR=0 scounteren.IR=0",
        excpt.triggered == false
    );

}



bool manual_zicntr_hpm_hu_csrr_hpmcounter3_ili_mcounteren_hpm30() {


    //当mcounteren.HPM3=0时，hu/hs 不可访问hpmcounter3寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN(3)); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hu CSRR hpmcounter3 casuses illegal instruction exception when mcounteren.hpm3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hu CSRS(imm=0) hpmcounter3 casuses illegal instruction exception when mcounteren.hpm3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hs CSRR hpmcounter3 casuses illegal instruction exception when mcounteren.hpm3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hs CSRS(imm=0) hpmcounter3 casuses illegal instruction exception when mcounteren.hpm3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    

}

bool manual_zicntr_hpm_m_csrr_hpmcounter3_success_mcounteren_hpm31() {

    //当mcounteren.HPM3=1时，m 可访问hpmcounter3寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN(3));

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("m CSRR hpmcounter3 successful when mcounteren.hpm3=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("m CSRS(imm=0) hpmcounter3 successful when mcounteren.hpm3=1",
        excpt.triggered == false
    );

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hs CSRR hpmcounter3 successful when mcounteren.hpm3=1",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hs CSRS(imm=0) hpmcounter3 successful when mcounteren.hpm3=1",
        excpt.triggered == false
    );
    TEST_END(); 
}

bool manual_zicntr_hpm_hu_csrr_hpmcounter3_ili_mcounteren_hpm31_scounteren_hpm30() {

    //当mcounteren.HPM3=1,scounteren=0时，hu 不可访问hpmcounter3寄存器

    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN(3)); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN(3)); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hu CSRR hpmcounter3 casuses illegal instruction exception when mcounteren.hpm3=1,scounteren.hpm3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hu CSRS(imm=0) hpmcounter3 casuses illegal instruction exception when mcounteren.hpm3=1,scounteren.hpm3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}

bool manual_zicntr_hpm_m_csrr_hpmcounter3_success_mcounteren_hpm31_scounteren_hpm31() {


    //当mcounteren.HPM3=1,scounteren=1时，m、hu、hs 可访问hpmcounter3寄存器
    
    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_SCOUNTEREN, HCOUNTEREN(3)); 
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN(3)); 

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("m CSRR hpmcounter3 successful when mcounteren.hpm3=1 scounteren.hpm3=1",
        excpt.triggered == false
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("m CSRS(imm=0) hpmcounter3 successful when mcounteren.hpm3=1 scounteren.hpm3=1",
        excpt.triggered == false
    );


    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hu CSRR hpmcounter3 successful when mcounteren.hpm3=1 scounteren.hpm3=1",
        excpt.triggered == false
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hu CSRS(imm=0) hpmcounter3 successful when mcounteren.hpm3=1 scounteren.hpm3=1",
        excpt.triggered == false
    );

    goto_priv(PRIV_HS);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hs CSRR hpmcounter3 successful when mcounteren.hpm3=1 scounteren.hpm3=1",
        excpt.triggered == false
    );


    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hs CSRS(imm=0) hpmcounter3 successful when mcounteren.hpm3=1 scounteren.hpm3=1",
        excpt.triggered == false
    );

    TEST_END(); 
}


bool manual_zicntr_hpm_hu_csrr_csrs_hpmcounter3_ili_after_mcounteren_set_scounteren_clear() {

    //当mcounteren.HPM3=1,scounteren=0时，hu不可访问hpmcounter3寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRS(CSR_MCOUNTEREN, HCOUNTEREN(3)); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN(3)); 

    goto_priv(PRIV_HU);

    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("hu CSRR hpmcounter3 casuses illegal instruction exception when mcounteren.HPM3=1 scounteren.HPM3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("hu CSRS(imm=0) hpmcounter3 casuses illegal instruction exception when mcounteren.HPM3=1 scounteren.HPM3=0",
        excpt.triggered == true &&
        excpt.cause == CAUSE_ILI
    );

    TEST_END(); 
}


bool manual_zicntr_hpm_m_csrr_hpmcounter3_success_except_mcounteren_hpm30_scounteren_hpm30() {

    //当mcounteren.HPM3=0,scounteren=0时，m可访问hpmcounter3寄存器


    TEST_START();    
    goto_priv(PRIV_M);
    CSRC(CSR_MCOUNTEREN, HCOUNTEREN(3)); 
    CSRC(CSR_SCOUNTEREN, HCOUNTEREN(3)); 


    TEST_SETUP_EXCEPT();
    CSRR(CSR_HPMCOUNTER3);

    TEST_ASSERT("M CSRR hpmcounter3 successful exception when mcounteren.HPM3=0 scounteren.HPM3=0",
        excpt.triggered == false
    );

    TEST_SETUP_EXCEPT();
    CSRS(CSR_HPMCOUNTER3, 0);

    TEST_ASSERT("M CSRS(imm=0) hpmcounter3 successful exception when mcounteren.HPM3=0 scounteren.HPM3=0",
        excpt.triggered == false
    );

}