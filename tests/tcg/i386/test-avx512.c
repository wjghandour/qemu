#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef void (*testfn)(void);

typedef struct {
    uint64_t q0, q1, q2, q3, q4, q5, q6, q7;
} __attribute__((aligned(64))) vxdi;

#define N_ZMM 32
#define N_ZMM_F64 8
#define N_ZMM_F32 16
#define N_MEM 4

typedef struct {
    uint64_t mm[8];  /* Offset 0x0. */
    vxdi zmm[N_ZMM]; /* Offset 0x40. */
    uint64_t r[16];  /* Offset 0x840. */
    uint64_t flags;  /* Offset 0x8c0. */
    uint32_t ff;
    vxdi mem[N_MEM];
    vxdi mem0[N_MEM];
} reg_state;

typedef struct {
    int n;
    testfn fn;
    const char *s;
    reg_state *init;
} TestDef;

reg_state initI;
reg_state initF16;
reg_state initF32;
reg_state initF64;

static void dump_zmm(const char *name, int n, const vxdi *r, int ff)
{
    printf("%s%d = %016lx %016lx %016lx %016lx %016lx %016lx %016lx %016lx\n",
           name, n, r->q7, r->q6, r->q5, r->q4, r->q3, r->q2, r->q1, r->q0);
    if (ff == 64) {
        double v[N_ZMM_F64];
        memcpy(v, r, sizeof(v));
        printf("        %16g %16g %16g %16g %16g %16g %16g %16g\n",
               v[7], v[6], v[5], v[4], v[3], v[2], v[1], v[0]);
    } else if (ff == 32) {
        float v[N_ZMM_F32];
        memcpy(v, r, sizeof(v));
        printf(" %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g %8g\n",
               v[15], v[14], v[13], v[12], v[11], v[10], v[9], v[8],
               v[7], v[6], v[5], v[4], v[3], v[2], v[1], v[0]);
    }
}

static void dump_regs(reg_state *s)
{
    int i;

    for (i = 0; i < 32; i++) {
        dump_zmm("zmm", i, &s->zmm[i], s->ff);
    }
    for (i = 0; i < 4; i++) {
        dump_zmm("mem", i, &s->mem0[i], s->ff);
    }
}

static void compare_state(const reg_state *a, const reg_state *b)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (a->mm[i] != b->mm[i]) {
            printf("MM%d = %016lx\n", i, b->mm[i]);
        }
    }
    for (i = 0; i < 16; i++) {
        if (a->r[i] != b->r[i]) {
            printf("r%d = %016lx\n", i, b->r[i]);
        }
    }
    for (i = 0; i < N_ZMM; i++) {
        if (memcmp(&a->zmm[i], &b->zmm[i], sizeof(a->zmm[i]))) {
            dump_zmm("zmm", i, &b->zmm[i], a->ff);
        }
    }
    for (i = 0; i < N_MEM; i++) {
        if (memcmp(&a->mem0[i], &a->mem[i], sizeof(a->mem0[i]))) {
            dump_zmm("mem", i, &a->mem[i], a->ff);
        }
    }
    if (a->flags != b->flags) {
        printf("FLAGS = %016lx\n", b->flags);
    }
}

#define LOADMM(r, o) "movq " #r ", " #o "[%0]\n\t"
#define LOADZMM(r, o) "vmovaps " #r ", " #o "[%0]\n\t"
#define STOREMM(r, o) "movq " #o "[%1], " #r "\n\t"
#define STOREZMM(r, o) "vmovaps " #o "[%1], " #r "\n\t"
/* Values below are offsets of each MM reg in the reg_state type. */
#define MMREG(F) \
    F(mm0, 0x00) \
    F(mm1, 0x08) \
    F(mm2, 0x10) \
    F(mm3, 0x18) \
    F(mm4, 0x20) \
    F(mm5, 0x28) \
    F(mm6, 0x30) \
    F(mm7, 0x38)

#define ALL_ZMM "zmm0", "zmm1", "zmm2","zmm3", "zmm4", "zmm5", "zmm6","zmm7", \
        "zmm8", "zmm9", "zmm10","zmm11", "zmm12", "zmm13", "zmm14","zmm15",   \
        "zmm16", "zmm17", "zmm18","zmm19", "zmm20", "zmm21", "zmm22","zmm23",   \
        "zmm24", "zmm25", "zmm26","zmm27", "zmm28", "zmm29", "zmm30","zmm31"
/* Values below are offsets of each ZMM reg in the reg_state type. */
#define ZMMREG(F) \
    F(zmm0,  0x040) \
    F(zmm1,  0x080) \
    F(zmm2,  0x0c0) \
    F(zmm3,  0x100) \
    F(zmm4,  0x140) \
    F(zmm5,  0x180) \
    F(zmm6,  0x1c0) \
    F(zmm7,  0x200) \
    F(zmm8,  0x240) \
    F(zmm9,  0x280) \
    F(zmm10, 0x2C0) \
    F(zmm11, 0x300) \
    F(zmm12, 0x340) \
    F(zmm13, 0x380) \
    F(zmm14, 0x3c0) \
    F(zmm15, 0x400) \
    F(zmm16, 0x440) \
    F(zmm17, 0x480) \
    F(zmm18, 0x4c0) \
    F(zmm19, 0x500) \
    F(zmm20, 0x540) \
    F(zmm21, 0x580) \
    F(zmm22, 0x5c0) \
    F(zmm23, 0x600) \
    F(zmm24, 0x640) \
    F(zmm25, 0x680) \
    F(zmm26, 0x6c0) \
    F(zmm27, 0x700) \
    F(zmm28, 0x740) \
    F(zmm29, 0x780) \
    F(zmm30, 0x7c0) \
    F(zmm31, 0x800)

#define LOADREG(r, o) "mov " #r ", " #o "[rax]\n\t"
#define STOREREG(r, o) "mov " #o "[rax], " #r "\n\t"
/* Values below are offsets of each reg in the reg_state type. */
/* Note that RAX (reg index 0) and RSP/RBP (reg indexes 6/7)
   are handledd separately below. */
#define REG(F) \
    F(rbx, 0x848) \
    F(rcx, 0x850) \
    F(rdx, 0x858) \
    F(rsi, 0x860) \
    F(rdi, 0x868) \
    F(r8,  0x880) \
    F(r9,  0x888) \
    F(r10, 0x890) \
    F(r11, 0x898) \
    F(r12, 0x8a0) \
    F(r13, 0x8a8) \
    F(r14, 0x8b0) \
    F(r15, 0x8b8)
#define REG_RAX(F) \
    F(0x840)
#define REG_RSP(F) \
    F(0x870)
#define REG_RBP(F) \
    F(0x878)
#define LOADRCX(o) "mov rcx, " #o "[rax]\n\t"
#define LOADRAX(o) "mov rax, " #o "[rax]\n\t"
#define STORERBX(o) "mov " #o "[rax], rbx\n\t"
/* Value below is the offset of the flags field in the reg_state type. */
#define FLAGS(F) \
    F(0x8c0)

static void run_test(const TestDef *t)
{
    reg_state result;
    reg_state *init = t->init;
    memcpy(init->mem, init->mem0, sizeof(init->mem));
    printf("%5d %s\n", t->n, t->s);
    fflush(stdout);
    asm volatile(
            MMREG(LOADMM)
            ZMMREG(LOADZMM)
            "sub rsp, 128\n\t"
            "push rax\n\t"
            "push rbx\n\t"
            "push rcx\n\t"
            "push rdx\n\t"
            "push %1\n\t"
            "push %2\n\t"
            "mov rax, %0\n\t"
            "pushf\n\t"
            "pop rbx\n\t"
            "shr rbx, 8\n\t"
            "shl rbx, 8\n\t"
            FLAGS(LOADRCX)
            "and rcx, 0xff\n\t"
            "or rbx, rcx\n\t"
            "push rbx\n\t"
            "popf\n\t"
            REG(LOADREG)
            REG_RAX(LOADRAX)
            "call [rsp]\n\t"
            "mov [rsp], rax\n\t"
            "mov rax, 8[rsp]\n\t"
            REG(STOREREG)
            "mov rbx, [rsp]\n\t"
            REG_RAX(STORERBX)
            "mov rbx, 0\n\t"
            REG_RSP(STORERBX)
            REG_RBP(STORERBX)
            "pushf\n\t"
            "pop rbx\n\t"
            "and rbx, 0xff\n\t"
            FLAGS(STORERBX)
            "add rsp, 16\n\t"
            "pop rdx\n\t"
            "pop rcx\n\t"
            "pop rbx\n\t"
            "pop rax\n\t"
            "add rsp, 128\n\t"
            MMREG(STOREMM)
            ZMMREG(STOREZMM)
            : : "r"(init), "r"(&result), "r"(t->fn)
            : "memory", "cc",
            "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
            ALL_ZMM
            );
    compare_state(init, &result);
    fflush(stdout);
}

#define TEST(n, cmd, type) \
static void __attribute__((naked)) test_##n(void) \
{ \
    asm volatile(cmd); \
    asm volatile("ret"); \
}
#include "test-avx512.h"
#undef TEST

static const TestDef test_table[] = {
#define TEST(n, cmd, type) {n, test_##n, cmd, &init##type},
#include "test-avx512.h"
    {-1, NULL, "", NULL}
};
#undef TEST


static void __attribute__((naked)) test_nop(void)
{
    asm volatile("nop");
    asm volatile("ret");
}
static const TestDef testdef_nop = {
    -1, test_nop, "nop", &initI
};

static void run_all(void)
{
    const TestDef *t;
    for (t = test_table; t->fn; t++) {
        run_test(t);
    }
}

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))

uint16_t val_f16[] = { 0x4000, 0xbc00, 0x44cd, 0x3a66, 0x4200, 0x7a1a, 0x4780, 0x4826 };
float val_f32[] = {2.0, -1.0, 4.8, 0.8, 3, -42.0, 5e6, 7.5, 8.3};
double val_f64[] = {2.0, -1.0, 4.8, 0.8, 3, -42.0, 5e6, 7.5};
vxdi val_i64[] = {
    {0x3d6b3b6a9e4118f2lu, 0x355ae76d2774d78clu,
     0xac3ff76c4daa4b28lu, 0xe7fabd204cb54083lu,
     0x3d6b3b6a9e4118f3lu, 0x355ae76d2774d78dlu,
     0xac3ff76c4daa4b29lu, 0xe7fabd204cb5408dlu},
    {0xd851c54a56bf1f29lu, 0x4a84d1d50bf4c4fflu,
     0x56621e553d52b56clu, 0xd0069553da8f584alu,
     0xd851c54a56bf1f2alu, 0x4a84d1d50bf4c100lu,
     0x56621e553d52b56dlu, 0xd0069553da8f584blu},
    {0x5826475e2c5fd799lu, 0xfd32edc01243f5e9lu,
     0x738ba2c66d3fe126lu, 0x5707219c6e6c26b4lu,
     0x5826475e2c5fd79alu, 0xfd32edc01243f5ealu,
     0x738ba2c66d3fe127lu, 0x5707219c6e6c26b5lu},
};

vxdi deadbeef = {0xa5a5a5a5deadbeefull, 0xa5a5a5a5deadbeefull,
                 0xa5a5a5a5deadbeefull, 0xa5a5a5a5deadbeefull,
                 0xa5a5a5a5deadbeefull, 0xa5a5a5a5deadbeefull,
                 0xa5a5a5a5deadbeefull, 0xa5a5a5a5deadbeefull};
vxdi indexq = {0x000000000000001full, 0x000000000000008full,
               0xffffffffffffffffull, 0xffffffffffffff5full,
               0x000000000000000eull, 0x000000000000000aull,
               0x00000000000000efull, 0x0000000000000010ull};
vxdi indexd = {0x00000002000000efull, 0xfffffff500000010ull,
               0x0000000afffffff0ull, 0x000000000000000eull,
               0x000000040000001full, 0xfffffff10000001eull,
               0x0000000cfffffff1ull, 0x000000000000000aull};

vxdi gather_mem[0x40];

void init_f16reg(vxdi *r)
{
    memset(r, 0, sizeof(*r));
    memcpy(r, val_f16, sizeof(val_f16));
}

void init_f32reg(vxdi *r)
{
    static int n;
    float v[N_ZMM_F32];
    int i;
    for (i = 0; i < N_ZMM_F32; i++) {
        v[i] = val_f32[n++];
        if (n == ARRAY_LEN(val_f32)) {
            n = 0;
        }
    }
    memcpy(r, v, sizeof(*r));
}

void init_f64reg(vxdi *r)
{
    static int n;
    double v[N_ZMM_F64];
    int i;
    for (i = 0; i < N_ZMM_F64; i++) {
        v[i] = val_f64[n++];
        if (n == ARRAY_LEN(val_f64)) {
            n = 0;
        }
    }
    memcpy(r, v, sizeof(*r));
}

void init_intreg(vxdi *r)
{
    static uint64_t mask;
    static int n;

    r->q0 = val_i64[n].q0 ^ mask;
    r->q1 = val_i64[n].q1 ^ mask;
    r->q2 = val_i64[n].q2 ^ mask;
    r->q3 = val_i64[n].q3 ^ mask;
    r->q4 = val_i64[n].q4 ^ mask;
    r->q5 = val_i64[n].q5 ^ mask;
    r->q6 = val_i64[n].q6 ^ mask;
    r->q7 = val_i64[n].q7 ^ mask;
    n++;
    if (n == ARRAY_LEN(val_i64)) {
        n = 0;
        mask *= 0x104C11DB7;
    }
}

static void init_all(reg_state *s)
{
    int i;

    s->r[3] = (uint64_t)&s->mem[0]; /* rdx */
    s->r[4] = (uint64_t)&gather_mem[ARRAY_LEN(gather_mem) / 2]; /* rsi */
    s->r[5] = (uint64_t)&s->mem[2]; /* rdi */
    s->flags = 2;
    for (i = 0; i < N_ZMM; i++) {
        s->zmm[i] = deadbeef;
    }
    s->zmm[13] = indexd;
    s->zmm[14] = indexq;
    for (i = 0; i < N_MEM; i++) {
        s->mem0[i] = deadbeef;
    }
}

int main(int argc, char *argv[])
{
    int i;

    init_all(&initI);
    init_intreg(&initI.zmm[10]);
    init_intreg(&initI.zmm[11]);
    init_intreg(&initI.zmm[12]);
    init_intreg(&initI.mem0[1]);
    printf("Int:\n");
    dump_regs(&initI);

    init_all(&initF16);
    init_f16reg(&initF16.zmm[10]);
    init_f16reg(&initF16.zmm[11]);
    init_f16reg(&initF16.zmm[12]);
    init_f16reg(&initF16.mem0[1]);
    initF16.ff = 16;
    printf("F16:\n");
    dump_regs(&initF16);

    init_all(&initF32);
    init_f32reg(&initF32.zmm[10]);
    init_f32reg(&initF32.zmm[11]);
    init_f32reg(&initF32.zmm[12]);
    init_f32reg(&initF32.mem0[1]);
    initF32.ff = 32;
    printf("F32:\n");
    dump_regs(&initF32);

    init_all(&initF64);
    init_f64reg(&initF64.zmm[10]);
    init_f64reg(&initF64.zmm[11]);
    init_f64reg(&initF64.zmm[12]);
    init_f64reg(&initF64.mem0[1]);
    initF64.ff = 64;
    printf("F64:\n");
    dump_regs(&initF64);

    for (i = 0; i < ARRAY_LEN(gather_mem); i++) {
        init_intreg(&gather_mem[i]);
    }

    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n == -1) {
            run_test(&testdef_nop);
        } else {
            int num = 0;
            const TestDef *ptr = &test_table[0];
            while (ptr->n !=-1) {
                if (ptr->n == n) {
                    num++;
                    run_test(ptr);
                }
                ptr++;
            }
            if (num == 0) {
                fprintf(stderr, "ERROR: no test with id: %d\n", n);
                exit(1);
            }
        }
    } else {
        run_test(&testdef_nop);
        run_all();
    }
    return 0;
}
