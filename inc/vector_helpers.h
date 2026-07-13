#ifndef VECTOR_HELPERS_H
#define VECTOR_HELPERS_H

#include <stdint.h>

#define VTYPE(SEW, LMUL) (((SEW) << 3) | (LMUL))
#define VSEW_FROM_BITS(BITS) \
    ((BITS) == 8 ? 0 : \
     (BITS) == 16 ? 1 : \
     (BITS) == 32 ? 2 : 3)

#define LOAD_VECTOR_TO_REGISTER(name, instruction, type, sew_data) \
    static inline void name(const type *src, int vl){ \
        int vtype = VTYPE(VSEW_FROM_BITS(sew_data), 0); \
        asm volatile( \
            ".option push\n\t" \
            ".option norvc\n\t" \
            "vsetvl t0, %1, %2\n\t" \
            instruction " v6, (%0)\n\t" \
            ".option pop\n\t" \
            :: "r"(src), "r"(vl), "r"(vtype) : "t0", "memory"); \
    }

#define STORE_VECTOR_FROM_REGISTER(name, instruction, type, sew_data) \
    static inline void name(type *dest, int vl){ \
        int vtype = VTYPE(VSEW_FROM_BITS(sew_data), 0); \
        asm volatile( \
            ".option push\n\t" \
            ".option norvc\n\t" \
            "vsetvl t0, %1, %2\n\t" \
            instruction " v6, (%0)\n\t" \
            ".option pop\n\t" \
            :: "r"(dest), "r"(vl), "r"(vtype) : "t0", "memory"); \
    }

LOAD_VECTOR_TO_REGISTER(vle8_to_v6, "vle8.v", uint8_t, 8);
LOAD_VECTOR_TO_REGISTER(vle16_to_v6, "vle16.v", uint16_t, 16);
LOAD_VECTOR_TO_REGISTER(vle32_to_v6, "vle32.v", uint32_t, 32);
LOAD_VECTOR_TO_REGISTER(vle64_to_v6, "vle64.v", uint64_t, 64);

STORE_VECTOR_FROM_REGISTER(vse8_from_v6, "vse8.v", uint8_t, 8);
STORE_VECTOR_FROM_REGISTER(vse16_from_v6, "vse16.v", uint16_t, 16);
STORE_VECTOR_FROM_REGISTER(vse32_from_v6, "vse32.v", uint32_t, 32);
STORE_VECTOR_FROM_REGISTER(vse64_from_v6, "vse64.v", uint64_t, 64);

static inline void set_vredsum_vs_conditions(int sew, int lmull, int vl, int vs1_init, int vs2_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvli t0, %0, %1\n\t"
        "vmv.v.i v0, 1\n\t"
        "vmv.v.i v4, %2\n\t"
        "vmv.v.i v6, %3\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "i"(vtype), "I"(vs1_init), "I"(vs2_init)
        : "t0", "v0", "v4", "v6", "memory");
}

static inline void execute_vredsum_vs() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vredsum.vs v2, v2, v4, v0.t\n\t"
        ".option pop\n\t"
        :
        :
        : "v2", "v4", "v0", "memory");
}

static inline void set_vcpop_conditions(int vl, int sew, int lmull, int v0_init, int v2_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v2, %3\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init), "I"(v2_init)
        : "t0", "v0", "v2", "memory");
}

#define execute_vcpop_v2()                          \
    ({                                              \
        uint32_t _temp;                             \
        asm volatile(                               \
            ".option push\n\t"                      \
            ".option norvc\n\t"                     \
            "vcpop.m %0, v2, v0.t\n\t"              \
            ".option pop\n\t"                       \
            : "=r"(_temp)                           \
            :                                       \
            : "v2", "v0", "memory");                \
        _temp;                                      \
    })

static inline void set_vfirst_m_conditions(int sew, int lmull, int vl, int v0_init, int v4_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v4, %3\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init), "I"(v4_init)
        : "t0", "v4", "v0", "memory");
}

#define EXECUTE_VFIRST_M_V3()                       \
    ({                                              \
        uint32_t _temp;                             \
        asm volatile(                               \
            ".option push\n\t"                      \
            ".option norvc\n\t"                     \
            "vfirst.m %0, v3, v0.t\n\t"             \
            ".option pop\n\t"                       \
            : "=r"(_temp)                           \
            :                                       \
            : "v3", "v0", "memory");                \
        _temp;                                      \
    })

static inline void set_vmsbfm_conditions(int vl, int sew, int lmull, int v0_init, int v4_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v4, %3\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init), "I"(v4_init)
        : "t0", "v4", "v0", "memory");
}

static inline void execute_vmsbf_m() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vmsbf.m v3, v4, v0.t\n\t"
        ".option pop\n\t"
        :
        :
        : "v3", "v4", "v0", "memory");
}

static inline void set_vmsifm_conditions(int vl, int sew, int lmull, int v0_init, int v4_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v4, %3\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init), "I"(v4_init)
        : "t0", "v4", "v0", "memory");
}

static inline void execute_vmsif_m() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vmsif.m v5, v4, v0.t\n\t"
        ".option pop\n\t"
        :
        :
        : "v5", "v4", "v0", "memory");
}

static inline void set_vmsofm_conditions(int vl, int sew, int lmull, int v0_init, int v4_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v4, %3\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init), "I"(v4_init)
        : "t0", "v4", "v0", "memory");
}

static inline void execute_vmsof_m() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vmsof.m v5, v4, v0.t\n\t"
        ".option pop\n\t"
        :
        :
        : "v5", "v4", "v0", "memory");
}

static inline void set_vadd_conditions(int sew, int lmull, int vl, int v0_init, int v4_init, int v6_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvli t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v4, %3\n\t"
        "vmv.v.i v6, %4\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "rI"(vtype), "I"(v0_init), "I"(v4_init), "I"(v6_init)
        : "t0", "v0", "v4", "v6", "memory");
}

static inline void execute_vadd_vv() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vadd.vv v8, v4, v6\n\t"
        ".option pop\n\t"
        :
        :
        : "v8", "v4", "v6", "memory");
}

static inline void set_vfadd_conditions(int sew, int lmull, int vl, float v0_init, float v4_init, float v6_init) {
    int vtype = (sew << 3) | lmull;
    int v0_value = (int)v0_init;
    int v4_value = (int)v4_init;
    int v6_value = (int)v6_init;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvli t0, %0, %1\n\t"
        "vmv.v.x v0, %2\n\t"
        "vmv.v.x v4, %3\n\t"
        "vmv.v.x v6, %4\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "rI"(vtype), "r"(v0_value), "r"(v4_value), "r"(v6_value)
        : "t0", "v0", "v4", "v6", "memory");
}

static inline void execute_vfadd() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        ".word 0x02431457\n\t"
        ".option pop\n\t"
        :
        :
        : "v8", "v4", "v6", "memory");
}

static inline void set_viota_m_conditions(int vl, int sew, int lmull, int v0_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init)
        : "t0", "v0", "memory");
}

static inline void execute_viota_m() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "viota.m v6, v0\n\t"
        ".option pop\n\t"
        :
        :
        : "v6", "v0", "memory");
}

static inline void set_vcompress_conditions(int vl, int sew, int lmull, int v0_init, int v4_init, int v8_init) {
    int vtype = (sew << 3) | lmull;

    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vsetvl t0, %0, %1\n\t"
        "vmv.v.i v0, %2\n\t"
        "vmv.v.i v4, %3\n\t"
        "vmv.v.i v8, %4\n\t"
        ".option pop\n\t"
        :
        : "r"(vl), "r"(vtype), "I"(v0_init), "I"(v4_init), "I"(v8_init)
        : "t0", "v0", "v4", "v8", "memory");
}

static inline void execute_vcompress() {
    asm volatile(
        ".option push\n\t"
        ".option norvc\n\t"
        "vcompress.vm v8, v4, v0\n\t"
        ".option pop\n\t"
        :
        :
        : "v8", "v4", "v0", "memory");
}

#endif /* VECTOR_HELPERS_H */
