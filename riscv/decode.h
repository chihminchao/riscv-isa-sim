// See LICENSE for license details.

#ifndef _RISCV_DECODE_H
#define _RISCV_DECODE_H

#if (-1 != ~0) || ((-1 >> 1) != -1)
# error spike requires a two''s-complement c++ implementation
#endif

#ifdef WORDS_BIGENDIAN
# error spike requires a little-endian host
#endif

#include <cstdint>
#include <string.h>
#include <strings.h>
#include "encoding.h"
#include "config.h"
#include "common.h"
#include "softfloat_types.h"
#include "specialize.h"
#include <cinttypes>

typedef int64_t sreg_t;
typedef uint64_t reg_t;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

const int NXPR = 32;
const int NFPR = 32;
const int NVPR = 32;
const int NCSR = 4096;

#define X_RA 1
#define X_SP 2

#define FP_RD_NE  0
#define FP_RD_0   1
#define FP_RD_DN  2
#define FP_RD_UP  3
#define FP_RD_NMM 4

#define FSR_RD_SHIFT 5
#define FSR_RD   (0x7 << FSR_RD_SHIFT)

#define FPEXC_NX 0x01
#define FPEXC_UF 0x02
#define FPEXC_OF 0x04
#define FPEXC_DZ 0x08
#define FPEXC_NV 0x10

#define FSR_AEXC_SHIFT 0
#define FSR_NVA  (FPEXC_NV << FSR_AEXC_SHIFT)
#define FSR_OFA  (FPEXC_OF << FSR_AEXC_SHIFT)
#define FSR_UFA  (FPEXC_UF << FSR_AEXC_SHIFT)
#define FSR_DZA  (FPEXC_DZ << FSR_AEXC_SHIFT)
#define FSR_NXA  (FPEXC_NX << FSR_AEXC_SHIFT)
#define FSR_AEXC (FSR_NVA | FSR_OFA | FSR_UFA | FSR_DZA | FSR_NXA)

#define insn_length(x) \
  (((x) & 0x03) < 0x03 ? 2 : \
   ((x) & 0x1f) < 0x1f ? 4 : \
   ((x) & 0x3f) < 0x3f ? 6 : \
   8)
#define MAX_INSN_LENGTH 8
#define PC_ALIGN 2

#define TAIL_ZEROING true

typedef uint64_t insn_bits_t;
class insn_t
{
public:
  insn_t() = default;
  insn_t(insn_bits_t bits) : b(bits) {}
  insn_bits_t bits() { return b; }
  int length() { return insn_length(b); }
  int64_t i_imm() { return int64_t(b) >> 20; }
  int64_t shamt() { return x(20, 6); }
  int64_t s_imm() { return x(7, 5) + (xs(25, 7) << 5); }
  int64_t sb_imm() { return (x(8, 4) << 1) + (x(25,6) << 5) + (x(7,1) << 11) + (imm_sign() << 12); }
  int64_t u_imm() { return int64_t(b) >> 12 << 12; }
  int64_t uj_imm() { return (x(21, 10) << 1) + (x(20, 1) << 11) + (x(12, 8) << 12) + (imm_sign() << 20); }
  uint64_t rd() { return x(7, 5); }
  uint64_t rs1() { return x(15, 5); }
  uint64_t rs2() { return x(20, 5); }
  uint64_t rs3() { return x(27, 5); }
  uint64_t rm() { return x(12, 3); }
  uint64_t csr() { return x(20, 12); }

  int64_t rvc_imm() { return x(2, 5) + (xs(12, 1) << 5); }
  int64_t rvc_zimm() { return x(2, 5) + (x(12, 1) << 5); }
  int64_t rvc_addi4spn_imm() { return (x(6, 1) << 2) + (x(5, 1) << 3) + (x(11, 2) << 4) + (x(7, 4) << 6); }
  int64_t rvc_addi16sp_imm() { return (x(6, 1) << 4) + (x(2, 1) << 5) + (x(5, 1) << 6) + (x(3, 2) << 7) + (xs(12, 1) << 9); }
  int64_t rvc_lwsp_imm() { return (x(4, 3) << 2) + (x(12, 1) << 5) + (x(2, 2) << 6); }
  int64_t rvc_ldsp_imm() { return (x(5, 2) << 3) + (x(12, 1) << 5) + (x(2, 3) << 6); }
  int64_t rvc_swsp_imm() { return (x(9, 4) << 2) + (x(7, 2) << 6); }
  int64_t rvc_sdsp_imm() { return (x(10, 3) << 3) + (x(7, 3) << 6); }
  int64_t rvc_lw_imm() { return (x(6, 1) << 2) + (x(10, 3) << 3) + (x(5, 1) << 6); }
  int64_t rvc_ld_imm() { return (x(10, 3) << 3) + (x(5, 2) << 6); }
  int64_t rvc_j_imm() { return (x(3, 3) << 1) + (x(11, 1) << 4) + (x(2, 1) << 5) + (x(7, 1) << 6) + (x(6, 1) << 7) + (x(9, 2) << 8) + (x(8, 1) << 10) + (xs(12, 1) << 11); }
  int64_t rvc_b_imm() { return (x(3, 2) << 1) + (x(10, 2) << 3) + (x(2, 1) << 5) + (x(5, 2) << 6) + (xs(12, 1) << 8); }
  int64_t rvc_simm3() { return x(10, 3); }
  uint64_t rvc_rd() { return rd(); }
  uint64_t rvc_rs1() { return rd(); }
  uint64_t rvc_rs2() { return x(2, 5); }
  uint64_t rvc_rs1s() { return 8 + x(7, 3); }
  uint64_t rvc_rs2s() { return 8 + x(2, 3); }

  uint64_t v_vm() { return x(25, 1); }
  uint64_t v_nf() { return x(29, 3); }
  uint64_t v_simm5() { return xs(15, 5); }
  uint64_t v_zimm5() { return x(15, 5); }
  uint64_t v_zimm11() { return x(20, 11); }
  uint64_t v_lmul() { return 1 << x(20, 2); }
  uint64_t v_sew() { return 1 << (x(22, 3) + 3); }

private:
  insn_bits_t b;
  uint64_t x(int lo, int len) { return (b >> lo) & ((insn_bits_t(1) << len)-1); }
  uint64_t xs(int lo, int len) { return int64_t(b) << (64-lo-len) >> (64-len); }
  uint64_t imm_sign() { return xs(63, 1); }
};

template <class T, size_t N, bool zero_reg>
class regfile_t
{
public:
  void write(size_t i, T value)
  {
    if (!zero_reg || i != 0)
      data[i] = value;
  }
  const T& operator [] (size_t i) const
  {
    return data[i];
  }
private:
  T data[N];
};

// helpful macros, etc
#define MMU (*p->get_mmu())
#define STATE (*p->get_state())
#define P (*p)
#define FLEN (p->get_flen())
#define READ_REG(reg) STATE.XPR[reg]
#define READ_FREG(reg) STATE.FPR[reg]
#define RD READ_REG(insn.rd())
#define RS1 READ_REG(insn.rs1())
#define RS2 READ_REG(insn.rs2())
#define RS3 READ_REG(insn.rs3())
#define WRITE_RD(value) WRITE_REG(insn.rd(), value)

#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) STATE.XPR.write(reg, value)
# define WRITE_FREG(reg, value) DO_WRITE_FREG(reg, freg(value))
#else
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){(reg) << 1, {wdata, 0}}; \
    STATE.XPR.write(reg, wdata); \
  })
# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){((reg) << 1) | 1, wdata}; \
    DO_WRITE_FREG(reg, wdata); \
  })
#endif

// RVC macros
#define WRITE_RVC_RS1S(value) WRITE_REG(insn.rvc_rs1s(), value)
#define WRITE_RVC_RS2S(value) WRITE_REG(insn.rvc_rs2s(), value)
#define WRITE_RVC_FRS2S(value) WRITE_FREG(insn.rvc_rs2s(), value)
#define RVC_RS1 READ_REG(insn.rvc_rs1())
#define RVC_RS2 READ_REG(insn.rvc_rs2())
#define RVC_RS1S READ_REG(insn.rvc_rs1s())
#define RVC_RS2S READ_REG(insn.rvc_rs2s())
#define RVC_FRS2 READ_FREG(insn.rvc_rs2())
#define RVC_FRS2S READ_FREG(insn.rvc_rs2s())
#define RVC_SP READ_REG(X_SP)

// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())
#define dirty_fp_state (STATE.mstatus |= MSTATUS_FS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_ext_state (STATE.mstatus |= MSTATUS_XS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define DO_WRITE_FREG(reg, value) (STATE.FPR.write(reg, value), dirty_fp_state)
#define WRITE_FRD(value) WRITE_FREG(insn.rd(), value)
 
#define SHAMT (insn.i_imm() & 0x3F)
#define BRANCH_TARGET (pc + insn.sb_imm())
#define JUMP_TARGET (pc + insn.uj_imm())
#define RM ({ int rm = insn.rm(); \
              if(rm == 7) rm = STATE.frm; \
              if(rm > 4) throw trap_illegal_instruction(0); \
              rm; })

#define get_field(reg, mask) (((reg) & (decltype(reg))(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(decltype(reg))(mask)) | (((decltype(reg))(val) * ((mask) & ~((mask) << 1))) & (decltype(reg))(mask)))

#define require(x) if (unlikely(!(x))) throw trap_illegal_instruction(0)
#define require_privilege(p) require(STATE.prv >= (p))
#define require_rv64 require(xlen == 64)
#define require_rv32 require(xlen == 32)
#define require_extension(s) require(p->supports_extension(s))
#define require_fp require((STATE.mstatus & MSTATUS_FS) != 0)
#define require_accelerator require((STATE.mstatus & MSTATUS_XS) != 0)

#define set_fp_exceptions ({ if (softfloat_exceptionFlags) { \
                               dirty_fp_state; \
                               STATE.fflags |= softfloat_exceptionFlags; \
                             } \
                             softfloat_exceptionFlags = 0; })

#define sext32(x) ((sreg_t)(int32_t)(x))
#define zext32(x) ((reg_t)(uint32_t)(x))
#define sext_xlen(x) (((sreg_t)(x) << (64-xlen)) >> (64-xlen))
#define zext_xlen(x) (((reg_t)(x) << (64-xlen)) >> (64-xlen))

#define set_pc(x) \
  do { p->check_pc_alignment(x); \
       npc = sext_xlen(x); \
     } while(0)

#define set_pc_and_serialize(x) \
  do { reg_t __npc = (x) & p->pc_alignment_mask(); \
       npc = PC_SERIALIZE_AFTER; \
       STATE.pc = __npc; \
     } while(0)

class wait_for_interrupt_t {};

#define wfi() \
  do { set_pc_and_serialize(npc); \
       npc = PC_SERIALIZE_WFI; \
       throw wait_for_interrupt_t(); \
     } while(0)

#define serialize() set_pc_and_serialize(npc)

/* Sentinel PC values to serialize simulator pipeline */
#define PC_SERIALIZE_BEFORE 3
#define PC_SERIALIZE_AFTER 5
#define PC_SERIALIZE_WFI 7
#define invalid_pc(pc) ((pc) & 1)

/* Convenience wrappers to simplify softfloat code sequences */
#define isBoxedF32(r) (isBoxedF64(r) && ((uint32_t)((r.v[0] >> 32) + 1) == 0))
#define unboxF32(r) (isBoxedF32(r) ? (uint32_t)r.v[0] : defaultNaNF32UI)
#define isBoxedF64(r) ((r.v[1] + 1) == 0)
#define unboxF64(r) (isBoxedF64(r) ? r.v[0] : defaultNaNF64UI)
typedef float128_t freg_t;
inline float32_t f32(uint32_t v) { return { v }; }
inline float64_t f64(uint64_t v) { return { v }; }
inline float32_t f32(freg_t r) { return f32(unboxF32(r)); }
inline float64_t f64(freg_t r) { return f64(unboxF64(r)); }
inline float128_t f128(freg_t r) { return r; }
inline freg_t freg(float32_t f) { return { ((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float64_t f) { return { f.v, (uint64_t)-1 }; }
inline freg_t freg(float128_t f) { return f; }
#define F32_SIGN ((uint32_t)1 << 31)
#define F64_SIGN ((uint64_t)1 << 63)
#define fsgnj32(a, b, n, x) \
  f32((f32(a).v & ~F32_SIGN) | ((((x) ? f32(a).v : (n) ? F32_SIGN : 0) ^ f32(b).v) & F32_SIGN))
#define fsgnj64(a, b, n, x) \
  f64((f64(a).v & ~F64_SIGN) | ((((x) ? f64(a).v : (n) ? F64_SIGN : 0) ^ f64(b).v) & F64_SIGN))

#define isNaNF128(x) isNaNF128UI(x.v[1], x.v[0])
inline float128_t defaultNaNF128()
{
  float128_t nan;
  nan.v[1] = defaultNaNF128UI64;
  nan.v[0] = defaultNaNF128UI0;
  return nan;
}
inline freg_t fsgnj128(freg_t a, freg_t b, bool n, bool x)
{
  a.v[1] = (a.v[1] & ~F64_SIGN) | (((x ? a.v[1] : n ? F64_SIGN : 0) ^ b.v[1]) & F64_SIGN);
  return a;
}
inline freg_t f128_negate(freg_t a)
{
  a.v[1] ^= F64_SIGN;
  return a;
}

#define check_vcsr(csr_num) ({ \
  csr_num == CSR_VSTART or csr_num == CSR_VXSAT or csr_num == CSR_VXRM or \
  csr_num == CSR_VL or csr_num == CSR_VTYPE; \
})

#define validate_csr(which, write) ({ \
  if (!STATE.serialized) return PC_SERIALIZE_BEFORE; \
  STATE.serialized = false; \
  unsigned csr_priv = get_field((which), 0x300); \
  unsigned csr_read_only = get_field((which), 0xC00) == 3; \
  if (((write) && csr_read_only) || STATE.prv < csr_priv) \
    throw trap_illegal_instruction(0); \
  (which); })

/* For debug only. This will fail if the native machine's float types are not IEEE */
inline float to_f(float32_t f){float r; memcpy(&r, &f, sizeof(r)); return r;}
inline double to_f(float64_t f){double r; memcpy(&r, &f, sizeof(r)); return r;}
inline long double to_f(float128_t f){long double r; memcpy(&r, &f, sizeof(r)); return r;}

// Vector enums
enum VFUNARY0{
    VFCVT_XU_F_V  = 0b00000,
    VFCVT_X_F_V   = 0b00001,
    VFCVT_F_XU_V  = 0b00010,
    VFCVT_F_X_V   = 0b00011,
    VFWCVT_XU_F_V = 0b01000,
    VFWCVT_X_F_V  = 0b01001,
    VFWCVT_F_XU_V = 0b01010,
    VFWCVT_F_X_V  = 0b01011,
    VFWCVT_F_F_V  = 0b01100,
    VFNCVT_XU_F_V = 0b10000,
    VFNCVT_X_F_V  = 0b10001,
    VFNCVT_F_XU_V = 0b10010,
    VFNCVT_F_X_V  = 0b10011,
    VFNCVT_F_F_V  = 0b10100
};

enum VFUNARY1{
    VFSQRT_V  = 0b00000,
    VFCLASS_V = 0b10000
};

enum VMUNARY0{
    VMSBF  = 0b00001,
    VMSOF  = 0b00010,
    VMSIF  = 0b00011,
    VMIOTA = 0b10000,
    VID    = 0b10001
};

// Vector macros
#define e8 8   //   8b elements
#define e16 16  //  16b elements
#define e32 32  //  32b elements
#define e64 64  //  64b elements
#define e128 128 // 128b elements
#define m1 1   // Vlmul x1, assumed if m setting absent
#define m2 2   // Vlmul x2
#define m4 4   // Vlmul x4
#define m8 8   // Vlmul x8
#define d1 1   // EDIV 1, assumed if d setting absent
#define d2 2   // EDIV 2
#define d4 4   // EDIV 4
#define d8 8   // EDIV 8

#define vsext(x, sew) (((sreg_t)(x) << (64-sew)) >> (64-sew))
#define vzext(x, sew) (((reg_t)(x) << (64-sew)) >> (64-sew))

#define DEBUG_RVV 0

#if DEBUG_RVV
#define DEBUG_RVV_FP_VV \
  printf("vfp(%lu) vd=%f vs1=%f vs2=%f\n", i, to_f(vd), to_f(vs1), to_f(vs2));
#define DEBUG_RVV_FP_VF \
  printf("vfp(%lu) vd=%f vs1=%f vs2=%f\n", i, to_f(vd), to_f(rs1), to_f(vs2));
#define DEBUG_RVV_FMA_VV \
  printf("vfma(%lu) vd=%f vs1=%f vs2=%f vd_old=%f\n", i, to_f(vd), to_f(vs1), to_f(vs2), to_f(vd_old));
#define DEBUG_RVV_FMA_VF \
  printf("vfma(%lu) vd=%f vs1=%f vs2=%f vd_old=%f\n", i, to_f(vd), to_f(rs1), to_f(vs2), to_f(vd_old));
#else
#define DEBUG_RVV_FP_VV 0
#define DEBUG_RVV_FP_VF 0
#define DEBUG_RVV_FMA_VV 0
#define DEBUG_RVV_FMA_VF 0
#endif

#define STRIP(inx) \
    reg_t elems_per_strip = P.VU.get_slen()/P.VU.vsew; \
    reg_t elems_per_vreg = P.VU.get_vlen()/P.VU.vsew; \
    assert(inx>=0 && inx<=P.VU.vlmul * elems_per_vreg); \
    reg_t elems_per_lane = P.VU.vlmul * elems_per_strip; \
    reg_t strip_index = (inx % elems_per_vreg) / elems_per_strip; \
    reg_t index_in_strip = (inx % elems_per_vreg) % elems_per_strip; \
    reg_t mmu_inx = index_in_strip + strip_index * elems_per_lane;

#ifdef RISCV_ENABLE_1905_CHECK
#define VI_CHECK_1905 \
  extern bool g_check_1905; \
  if (g_check_1905) { \
    fprintf(stderr, "unsupported in 1905\n"); \
  };
#else
#define VI_CHECK_1905 \
  while (0);
#endif

#define V_CHECK_MASK(do_mask) \
  if (insn.v_vm() == 0) { \
    int midx = (P.VU.vmlen * i) / 64; \
    int mpos = (P.VU.vmlen * i) % 64; \
    do_mask = (P.VU.elt<uint64_t>(0, midx) >> mpos) & 0x1; \
  }

#define V_LOOP_ELEMENT_SKIP \
  const int mlen = P.VU.vmlen; \
  const int midx = (mlen * i) / 64; \
  const int mpos = (mlen * i) % 64; \
  if (insn.v_vm() == 0) { \
    bool skip = ((P.VU.elt<uint64_t>(0, midx) >> mpos) & 0x1) == 0; \
    if (skip) \
      continue; \
  } 

#define VI_NARROW_CHECK_COMMON \
  require(P.VU.vlmul <= 4); \
  require(P.VU.vsew * 2 <= P.VU.ELEN); \
  require(insn.rs2() + P.VU.vlmul * 2 <= 31);

#define VI_WIDE_CHECK_COMMON \
  require(P.VU.vlmul <= 4); \
  require(P.VU.vsew * 2 <= P.VU.ELEN); \
  require(insn.rd() + P.VU.vlmul * 2 <= 31); \
  if (insn.v_vm() == 0) \
    require(insn.rd() != 0);

#define VI_WIDE_CHECK_ONE \
  VI_WIDE_CHECK_COMMON; \
  require((insn.rd() & ~(P.VU.vlmul - 1)) != (insn.rs2() & ~(P.VU.vlmul - 1)));

#define VI_WIDE_CHECK_TWO \
  VI_WIDE_CHECK_COMMON; \
  require((insn.rd() & ~(P.VU.vlmul - 1)) != (insn.rs2() & ~(P.VU.vlmul - 1)) && \
          (insn.rd() & ~(P.VU.vlmul - 1)) != (insn.rs1() & ~(P.VU.vlmul - 1))); \

#define V_WIDE_OP_AND_ASSIGN(var0, var1, var2, op0, op1, sign) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign##16_t vd_w = P.VU.elt<sign##16_t>(rd_num, i); \
    P.VU.elt<uint16_t>(rd_num, i) = \
      op1((sign##16_t)(sign##8_t)var0 op0 (sign##16_t)(sign##8_t)var1) + var2; \
    } \
    break; \
  case e16: { \
    sign##32_t vd_w = P.VU.elt<sign##32_t>(rd_num, i); \
    P.VU.elt<uint32_t>(rd_num, i) = \
      op1((sign##32_t)(sign##16_t)var0 op0 (sign##32_t)(sign##16_t)var1) + var2; \
    } \
    break; \
  default: { \
    sign##64_t vd_w = P.VU.elt<sign##64_t>(rd_num, i); \
    P.VU.elt<uint64_t>(rd_num, i) = \
      op1((sign##64_t)(sign##32_t)var0 op0 (sign##64_t)(sign##32_t)var1) + var2; \
    } \
    break; \
  }

#define VI_WIDE_WVX_OP(var0, op0, sign) \
  switch(P.VU.vsew) { \
  case e8: { \
    sign##16_t &vd_w = P.VU.elt<sign##16_t>(rd_num, i); \
    sign##16_t vs2_w = P.VU.elt<sign##16_t>(rs2_num, i); \
    vd_w = vs2_w op0 (sign##16_t)(sign##8_t)var0; \
    } \
    break; \
  case e16: { \
    sign##32_t &vd_w = P.VU.elt<sign##32_t>(rd_num, i); \
    sign##32_t vs2_w = P.VU.elt<sign##32_t>(rs2_num, i); \
    vd_w = vs2_w op0 (sign##32_t)(sign##16_t)var0; \
    } \
    break; \
  default: { \
    sign##64_t &vd_w = P.VU.elt<sign##64_t>(rd_num, i); \
    sign##64_t vs2_w = P.VU.elt<sign##64_t>(rs2_num, i); \
    vd_w = vs2_w op0 (sign##64_t)(sign##32_t)var0; \
    } \
    break; \
  }

#define VI_GENERAL_LOOP_BASE \
  require(P.VU.vsew == e8 || P.VU.vsew == e16 || P.VU.vsew == e32 || P.VU.vsew == e64); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t sew = P.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ 

#define VI_TAIL_ZERO(elm) \
  if (vl != 0 && vl < P.VU.vlmax && TAIL_ZEROING) { \
    uint8_t *tail = &P.VU.elt<uint8_t>(rd_num, vl * ((sew >> 3) * elm)); \
    memset(tail, 0, (P.VU.vlmax - vl) * ((sew >> 3) * elm)); \
  }

#define VI_TAIL_ZERO_MASK(dst) \
  if (vl != 0 && TAIL_ZEROING){ \
    for (reg_t i=vl; i<P.VU.vlmax; ++i){ \
      const int mlen = P.VU.vmlen; \
      const int midx = (mlen * i) / 64; \
      const int mpos = (mlen * i) % 64; \
      uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
      uint64_t &vdi = P.VU.elt<uint64_t>(dst, midx); \
      vdi = (vdi & ~mmask);\
    }\
  }\

#define VI_LOOP_BASE \
    VI_GENERAL_LOOP_BASE \
    V_LOOP_ELEMENT_SKIP; 

#define VI_LOOP_END \
  } \
  if (vl != 0 && vl < P.VU.vlmax && TAIL_ZEROING){ \
    uint8_t *tail = &P.VU.elt<uint8_t>(rd_num, vl * ((sew >> 3) * 1)); \
    memset(tail, 0, (P.VU.vlmax - vl) * ((sew >> 3) * 1)); \
  }\
  P.VU.vstart = 0;

#define VI_LOOP_SLIDE_END(x) \
  } \
  if (vl != 0 && (vl + x) < P.VU.vlmax && TAIL_ZEROING){ \
    uint8_t *tail = &P.VU.elt<uint8_t>(rd_num, (vl + x) * ((sew >> 3) * 1)); \
    memset(tail, 0, (P.VU.vlmax - (vl + x)) * ((sew >> 3) * 1)); \
  }\
  P.VU.vstart = 0;

#define VI_LOOP_END_NO_TAIL_ZERO \
  } \
  P.VU.vstart = 0;

#define VI_LOOP_WIDEN_END \
  } \
  if (vl != 0 && vl < P.VU.vlmax && TAIL_ZEROING){ \
    uint8_t *tail = &P.VU.elt<uint8_t>(rd_num, vl * ((sew >> 3) * 2)); \
    memset(tail, 0, (P.VU.vlmax - vl) * ((sew >> 3) * 2)); \
  }\
  P.VU.vstart = 0;

#define VI_LOOP_REDUCTION_END(x) \
  } \
  if (vl > 0) { \
    vd_0_des = vd_0_res; \
    for (reg_t i = 1; i < P.VU.get_vlen()/sew; ++i) { \
      P.VU.elt<type_sew_t<x>::type>(rd_num, i) = 0; \
    } \
  } \
  P.VU.vstart = 0; 

#define VI_LOOP_CMP_BASE \
  require(P.VU.vsew == e8 || P.VU.vsew == e16 || P.VU.vsew == e32 || P.VU.vsew == e64); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t sew = P.VU.vsew; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    V_LOOP_ELEMENT_SKIP; \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t &vdi = P.VU.elt<uint64_t>(insn.rd(), midx); \
    uint64_t res = 0;

#define VI_LOOP_CMP_END \
    vdi = (vdi & ~mmask) | (((res) << mpos) & mmask); \
  } \
  VI_TAIL_ZERO_MASK(rd_num); \
  P.VU.vstart = 0;

#define VI_LOOP_MASK(op) \
  require(P.VU.vsew <= e64); \
  reg_t vl = P.VU.vl; \
  for (reg_t i = P.VU.vstart; i < vl; ++i) { \
    int mlen = P.VU.vmlen; \
    int midx = (mlen * i) / 64; \
    int mpos = (mlen * i) % 64; \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t vs2 = P.VU.elt<uint64_t>(insn.rs2(), midx); \
    uint64_t vs1 = P.VU.elt<uint64_t>(insn.rs1(), midx); \
    uint64_t &res = P.VU.elt<uint64_t>(insn.rd(), midx); \
    res = (res & ~mmask) | ((op) & mmask); \
  } \
  \
  for (reg_t i = vl; i < P.VU.vlmax && i > 0; ++i) { \
    int mlen = P.VU.vmlen; \
    int midx = (mlen * i) / 64; \
    int mpos = (mlen * i) % 64; \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t &res = P.VU.elt<uint64_t>(insn.rd(), midx); \
    res = (res & ~mmask); \
  } \
  P.VU.vstart = 0;

#define VXI_PARAMS(x) \
    type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
    type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \
    type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
    type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5(); 

#define VV_U_PARAMS(x) \
    type_usew_t<x>::type &vd = P.VU.elt<type_usew_t<x>::type>(rd_num, i); \
    type_usew_t<x>::type vs1 = P.VU.elt<type_usew_t<x>::type>(rs1_num, i); \
    type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i); 

#define VX_U_PARAMS(x) \
    type_usew_t<x>::type &vd = P.VU.elt<type_usew_t<x>::type>(rd_num, i); \
    type_usew_t<x>::type rs1 = (type_usew_t<x>::type)RS1; \
    type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i); 

#define VI_U_PARAMS(x) \
    type_usew_t<x>::type &vd = P.VU.elt<type_usew_t<x>::type>(rd_num, i); \
    type_usew_t<x>::type simm5 = (type_usew_t<x>::type)insn.v_zimm5(); \
    type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i); 

#define VV_PARAMS(x) \
    type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    type_sew_t<x>::type vs1 = P.VU.elt<type_sew_t<x>::type>(rs1_num, i); \
    type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); 

#define VX_PARAMS(x) \
    type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    type_sew_t<x>::type rs1 = (type_sew_t<x>::type)RS1; \
    type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); 

#define VI_PARAMS(x) \
    type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    type_sew_t<x>::type simm5 = (type_sew_t<x>::type)insn.v_simm5(); \
    type_sew_t<x>::type vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); 

#define XV_PARAMS(x) \
    type_sew_t<x>::type &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    type_usew_t<x>::type vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, RS1);

#define VI_XI_SLIDEDOWN_PARAMS(x, off) \
    auto &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i + off);

#define VI_XI_SLIDEUP_PARAMS(x, offset) \
    auto &vd = P.VU.elt<type_sew_t<x>::type>(rd_num, i); \
    auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i - offset);

#define VI_VV_LOOP_NARROW(BODY) \
VI_NARROW_CHECK_COMMON; \
VI_LOOP_BASE \
if (sew == e8){ \
  VI_NARROW_SHIFT(e8, e16) \
  BODY; \
}else if(sew == e16){ \
  VI_NARROW_SHIFT(e16, e32) \
  BODY; \
}else if(sew == e32){ \
  VI_NARROW_SHIFT(e32, e64) \
  BODY; \
} \
VI_LOOP_END 


#define VI_VV_ULOOP(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VV_U_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VV_U_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VV_U_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VV_U_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 

// comparision result to masking register
#define VI_VV_LOOP_CMP(BODY) \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
            VV_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VV_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VV_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VV_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VX_LOOP_CMP(BODY) \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
            VX_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VX_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VX_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VX_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VI_LOOP_CMP(BODY) \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
            VI_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VI_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VI_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VI_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VV_ULOOP_CMP(BODY) \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
            VV_U_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VV_U_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VV_U_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VV_U_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VX_ULOOP_CMP(BODY) \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
            VX_U_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VX_U_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VX_U_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VX_U_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VI_ULOOP_CMP(BODY) \
  VI_LOOP_CMP_BASE \
  if (sew == e8){ \
            VI_U_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VI_U_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VI_U_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VI_U_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_CMP_END

#define VI_VVXI_MERGE_LOOP(BODY) \
  VI_GENERAL_LOOP_BASE \
  if (sew == e8){ \
            VXI_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VXI_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VXI_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VXI_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 

#define VI_VV_LOOP(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VV_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VV_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VV_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VV_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 

#define VI_VV_LOOP_WIDEN(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VV_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VV_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VV_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VV_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_WIDEN_END 

// reduction loop
#define VI_LOOP_REDUCTION_BASE(x) \
  require(x == e8 || x == e16 || x == e32 || x == e64); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_sew_t<x>::type>(rd_num, 0); \
  auto vd_0_res = P.VU.elt<type_sew_t<x>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    V_LOOP_ELEMENT_SKIP; \
    auto vs2 = P.VU.elt<type_sew_t<x>::type>(rs2_num, i); \

#define REDUCTION_LOOP(x, BODY) \
  VI_LOOP_REDUCTION_BASE(x) \
  BODY; \
  VI_LOOP_REDUCTION_END(x)

#define VI_VV_LOOP_REDUCTION(BODY) \
  reg_t sew = P.VU.vsew; \
  if (sew == e8) { \
    REDUCTION_LOOP(e8, BODY) \
  } else if(sew == e16) { \
    REDUCTION_LOOP(e16, BODY) \
  } else if(sew == e32) { \
    REDUCTION_LOOP(e32, BODY) \
  } else if(sew == e64) { \
    REDUCTION_LOOP(e64, BODY) \
  }

// reduction unsiged loop
#define VI_ULOOP_REDUCTION_BASE(x) \
  require(x == e8 || x == e16 || x == e32 || x == e64); \
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_usew_t<x>::type>(rd_num, 0); \
  auto vd_0_res = P.VU.elt<type_usew_t<x>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    V_LOOP_ELEMENT_SKIP; \
    auto vs2 = P.VU.elt<type_usew_t<x>::type>(rs2_num, i);

#define REDUCTION_ULOOP(x, BODY) \
  VI_ULOOP_REDUCTION_BASE(x) \
  BODY; \
  VI_LOOP_REDUCTION_END(x)

#define VI_VV_ULOOP_REDUCTION(BODY) \
  reg_t sew = P.VU.vsew; \
  if (sew == e8){ \
    REDUCTION_ULOOP(e8, BODY) \
  } else if(sew == e16) { \
    REDUCTION_ULOOP(e16, BODY) \
  } else if(sew == e32) { \
    REDUCTION_ULOOP(e32, BODY) \
  } else if(sew == e64) { \
    REDUCTION_ULOOP(e64, BODY) \
  }

#define VI_VX_ULOOP(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VX_U_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VX_U_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VX_U_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VX_U_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 

#define VI_VX_LOOP_WIDEN(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VX_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VX_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VX_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VX_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_WIDEN_END 

#define VI_VX_LOOP(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VX_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VX_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VX_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VX_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 

#define VI_VI_ULOOP(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VI_U_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VI_U_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VI_U_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VI_U_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 


#define VI_VI_LOOP(BODY) \
  VI_LOOP_BASE \
  if (sew == e8){ \
            VI_PARAMS(e8); \
            BODY; \
  }else if(sew == e16){ \
            VI_PARAMS(e16); \
            BODY; \
  }else if(sew == e32){ \
            VI_PARAMS(e32); \
            BODY; \
  }else if(sew == e64){ \
            VI_PARAMS(e64); \
            BODY; \
  } \
  VI_LOOP_END 

#define VI_WIDE_SSMA(sew1, sew2, add, opd) \
  type_sew_t<sew2>::type &vd = P.VU.elt<type_sew_t<sew2>::type>(rd_num, i); \
  type_sew_t<sew1>::type vs1 = P.VU.elt<type_sew_t<sew1>::type>(rs1_num, i); \
  type_sew_t<sew1>::type vs2 = P.VU.elt<type_sew_t<sew1>::type>(rs2_num, i); \
  type_sew_t<sew1>::type rs1 = (type_sew_t<sew1>::type)RS1; \
  int##sew2##_t res; \
  bool sat = false; \
  const int gb = sew1 / 2; \
  VRM vrm = P.VU.get_vround_mode(); \
  res = (int##sew2##_t)(int##sew1##_t)vs2 * (int##sew2##_t)(int##sew1##_t)opd; \
  INT_ROUNDING(res, vrm, gb); \
  res = res >> gb; \
  if (add) \
    vd = sat_add<int##sew2##_t, uint##sew2##_t>(vd, res, sat); \
  else \
    vd = sat_sub<int##sew2##_t, uint##sew2##_t>(vd, res, sat); \
  P.VU.vxsat |= sat;

#define VI_VVX_LOOP_WIDE_SSMA(add, opd) \
  require(P.VU.vlmul <= 4); \
  require(P.VU.vsew * 2 <= P.VU.ELEN); \
  require(insn.rd() + P.VU.vlmul <= 32); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_WIDE_SSMA(8, 16, add, opd); \
  } else if(sew == e16){ \
    VI_WIDE_SSMA(16, 32, add, opd); \
  } else if(sew == e32){ \
    VI_WIDE_SSMA(32, 64, add, opd); \
  } \
  VI_LOOP_WIDEN_END

#define VI_WIDE_USSMA(sew1, sew2, add, opd) \
  type_usew_t<sew2>::type &vd = P.VU.elt<type_usew_t<sew2>::type>(rd_num, i); \
  type_usew_t<sew1>::type vs1 = P.VU.elt<type_usew_t<sew1>::type>(rs1_num, i); \
  type_usew_t<sew1>::type vs2 = P.VU.elt<type_usew_t<sew1>::type>(rs2_num, i); \
  type_usew_t<sew1>::type rs1 = (type_usew_t<sew1>::type)RS1; \
  uint##sew2##_t res; \
  bool sat = false; \
  const int gb = sew1 / 2; \
  VRM vrm = P.VU.get_vround_mode(); \
  res = (uint##sew2##_t)(uint##sew1##_t)vs2 * (uint##sew2##_t)(uint##sew1##_t)opd; \
  INT_ROUNDING(res, vrm, gb); \
  \
  res = res >> gb; \
  if (add) \
    vd = sat_addu<uint##sew2##_t>(vd, res, sat); \
  else \
    vd = sat_subu<uint##sew2##_t>(vd, res, sat); \
  P.VU.vxsat |= sat;

#define VI_VVX_LOOP_WIDE_USSMA(add, opd) \
  require(P.VU.vlmul <= 4); \
  require(P.VU.vsew * 2 <= P.VU.ELEN); \
  require(insn.rd() + P.VU.vlmul <= 32); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_WIDE_USSMA(8, 16, add, opd); \
  } else if(sew == e16){ \
    VI_WIDE_USSMA(16, 32, add, opd); \
  } else if(sew == e32){ \
    VI_WIDE_USSMA(32, 64, add, opd); \
  } \
  VI_LOOP_WIDEN_END

#define VI_NARROW_SHIFT(sew1, sew2) \
  type_usew_t<sew1>::type &vd = P.VU.elt<type_usew_t<sew1>::type>(rd_num, i); \
  type_usew_t<sew2>::type vs2_u = P.VU.elt<type_usew_t<sew2>::type>(rs2_num, i); \
  type_usew_t<sew1>::type zimm5 = (type_usew_t<sew1>::type)insn.v_zimm5(); \
  type_sew_t<sew2>::type vs2 = P.VU.elt<type_sew_t<sew2>::type>(rs2_num, i); \
  type_sew_t<sew1>::type vs1 = P.VU.elt<type_sew_t<sew1>::type>(rs1_num, i); \
  type_sew_t<sew1>::type rs1 = (type_sew_t<sew1>::type)RS1; 

#define VI_VVXI_LOOP_NARROW(BODY) \
  require(P.VU.vsew <= e32); \
  VI_LOOP_BASE \
  if (sew == e8){ \
    VI_NARROW_SHIFT(e8, e16) \
    BODY; \
  } else if (sew == e16) { \
    VI_NARROW_SHIFT(e16, e32) \
    BODY; \
  } else if (sew == e32) { \
    VI_NARROW_SHIFT(e32, e64) \
    BODY; \
  } \
  VI_LOOP_END

// wide reduction loop - signed
#define VI_LOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  VI_WIDE_CHECK_ONE; \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_sew_t<sew2>::type>(rd_num, 0); \
  auto vd_0_res = P.VU.elt<type_sew_t<sew2>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    V_LOOP_ELEMENT_SKIP; \
    auto vs2 = P.VU.elt<type_sew_t<sew1>::type>(rs2_num, i);

#define WIDE_REDUCTION_LOOP(sew1, sew2, BODY) \
  VI_LOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  BODY; \
  VI_LOOP_REDUCTION_END(sew2)

#define VI_VV_LOOP_WIDE_REDUCTION(BODY) \
  require(!P.VU.vill);\
  reg_t sew = P.VU.vsew; \
  if (sew == e8){ \
    WIDE_REDUCTION_LOOP(e8, e16, BODY) \
  } else if(sew == e16){ \
    WIDE_REDUCTION_LOOP(e16, e32, BODY) \
  } else if(sew == e32){ \
    WIDE_REDUCTION_LOOP(e32, e64, BODY) \
  }

// wide reduction loop - unsigned
#define VI_ULOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  VI_WIDE_CHECK_ONE; \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  auto &vd_0_des = P.VU.elt<type_usew_t<sew2>::type>(rd_num, 0); \
  auto vd_0_res = P.VU.elt<type_usew_t<sew2>::type>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i) { \
    V_LOOP_ELEMENT_SKIP; \
    auto vs2 = P.VU.elt<type_usew_t<sew1>::type>(rs2_num, i);

#define WIDE_REDUCTION_ULOOP(sew1, sew2, BODY) \
  VI_ULOOP_WIDE_REDUCTION_BASE(sew1, sew2) \
  BODY; \
  VI_LOOP_REDUCTION_END(sew2)

#define VI_VV_ULOOP_WIDE_REDUCTION(BODY) \
  require(!P.VU.vill);\
  reg_t sew = P.VU.vsew; \
  if (sew == e8){ \
    WIDE_REDUCTION_ULOOP(e8, e16, BODY) \
  } else if(sew == e16){ \
    WIDE_REDUCTION_ULOOP(e16, e32, BODY) \
  } else if(sew == e32){ \
    WIDE_REDUCTION_ULOOP(e32, e64, BODY) \
  }

#define VI_VVX_LOOP_AVG(opd, op) \
VRM xrm = p->VU.get_vround_mode(); \
VI_LOOP_BASE \
  switch(sew) { \
    case e8: { \
     VV_PARAMS(e8); \
     type_sew_t<e8>::type rs1 = RS1; \
     auto res = (int32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e16: { \
     VV_PARAMS(e16); \
     type_sew_t<e16>::type rs1 = RS1; \
     auto res = (int32_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    case e32: { \
     VV_PARAMS(e32); \
     type_sew_t<e32>::type rs1 = RS1; \
     auto res = (int64_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
    default: { \
     VV_PARAMS(e64); \
     type_sew_t<e64>::type rs1 = RS1; \
     auto res = (int128_t)vs2 op opd; \
     INT_ROUNDING(res, xrm, 1); \
     vd = res >> 1; \
     break; \
    } \
  } \
VI_LOOP_END

#define VF_LOOP_BASE \
  require_extension('F'); \
  require_fp; \
  require(P.VU.vsew == 32); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm; \
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    V_LOOP_ELEMENT_SKIP;

#define VF_LOOP_CMP_BASE \
  require_extension('F'); \
  require_fp; \
  require(P.VU.vsew == 32); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm; \
  for (reg_t i = P.VU.vstart; i < vl; ++i) { \
    float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
    float32_t vs1 = P.VU.elt<float32_t>(rs1_num, i); \
    float32_t rs1 = f32(READ_FREG(rs1_num)); \
    V_LOOP_ELEMENT_SKIP; \
    uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
    uint64_t &vdi = P.VU.elt<uint64_t>(rd_num, midx); \
    uint64_t res = 0;

#define VF_LOOP_REDUCTION_BASE \
  require_extension('F'); \
  require_fp; \
  require(P.VU.vsew == 32); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm; \
  float32_t vd_0 = P.VU.elt<float32_t>(rd_num, 0); \
  float32_t vs1_0 = P.VU.elt<float32_t>(rs1_num, 0); \
  vd_0 = vs1_0;\
  for (reg_t i=P.VU.vstart; i<vl; ++i){ \
    V_LOOP_ELEMENT_SKIP; \
    int32_t &vd = P.VU.elt<int32_t>(rd_num, i); \

#define VF_LOOP_WIDE_REDUCTION_BASE \
  require_extension('F'); \
  require_fp; \
  require(P.VU.vsew == 32); \
  require(!P.VU.vill);\
  reg_t vl = P.VU.vl; \
  reg_t rd_num = insn.rd(); \
  reg_t rs1_num = insn.rs1(); \
  reg_t rs2_num = insn.rs2(); \
  softfloat_roundingMode = STATE.frm; \
  float64_t vd_0 = P.VU.elt<float64_t>(rs1_num, 0); \
  for (reg_t i=P.VU.vstart; i<vl; ++i) { \
    V_LOOP_ELEMENT_SKIP;

#define VF_LOOP_END \
  } \
  if (vl != 0 && vl < P.VU.vlmax && TAIL_ZEROING){ \
    uint8_t *tail = &P.VU.elt<uint8_t>(rd_num, vl * ((P.VU.vsew >> 3) * 1)); \
    memset(tail, 0, (P.VU.vlmax - vl) * ((P.VU.vsew >> 3) * 1)); \
  }\
  P.VU.vstart = 0; \
  set_fp_exceptions;

#define VF_LOOP_REDUCTION_END(x) \
  } \
  P.VU.vstart = 0; \
  set_fp_exceptions; \
  if (vl > 0 && TAIL_ZEROING) { \
    P.VU.elt<type_sew_t<x>::type>(rd_num, 0) = vd_0.v; \
    for (reg_t i = 1; i < (P.VU.VLEN / P.VU.vsew); ++i) { \
       P.VU.elt<type_sew_t<x>::type>(rd_num, i) = 0; \
    } \
  }

#define VF_LOOP_CMP_END \
    switch(P.VU.vsew) { \
    case 32: { \
      vdi = (vdi & ~mmask) | (((res) << mpos) & mmask); \
      break; \
    } \
    case 16: \
    case 8: \
    default: \
      softfloat_exceptionFlags = 1; \
      break; \
    }; \
  } \
  if (vl != 0 && TAIL_ZEROING){ \
    for (reg_t i=vl; i<P.VU.vlmax; ++i){ \
      const int mlen = P.VU.vmlen; \
      const int midx = (mlen * i) / 64; \
      const int mpos = (mlen * i) % 64; \
      uint64_t mmask = (UINT64_MAX << (64 - mlen)) >> (64 - mlen - mpos); \
      uint64_t &vdi = P.VU.elt<uint64_t>(insn.rd(), midx); \
      vdi = (vdi & ~mmask);\
    }\
  }\
  P.VU.vstart = 0; \
  set_fp_exceptions;


#define VFMA_VV_LOOP(BODY)                      \
  VF_LOOP_BASE \
  float32_t &vd = P.VU.elt<float32_t>(rd_num, i); \
  float32_t vs1 = P.VU.elt<float32_t>(rs1_num, i); \
  float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
  float32_t vd_old = vd; \
  BODY; \
  DEBUG_RVV_FMA_VV; \
  VF_LOOP_END

#define VFMA_VF_LOOP(BODY) \
  VF_LOOP_BASE \
  float32_t &vd = P.VU.elt<float32_t>(rd_num, i); \
  float32_t rs1 = f32(READ_FREG(rs1_num)); \
  float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
  float32_t vd_old = vd; \
  BODY; \
  DEBUG_RVV_FMA_VF; \
  VF_LOOP_END

#define VFP_VV_LOOP(BODY) \
  VF_LOOP_BASE \
  float32_t &vd = P.VU.elt<float32_t>(rd_num, i); \
  float32_t vs1 = P.VU.elt<float32_t>(rs1_num, i); \
  float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
  BODY; \
  DEBUG_RVV_FP_VV; \
  VF_LOOP_END

#define VFP_VV_LOOP_REDUCTION(BODY) \
  VF_LOOP_REDUCTION_BASE \
  float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
  BODY; \
  DEBUG_RVV_FP_VV; \
  VF_LOOP_REDUCTION_END(e32)

#define VFP_VV_LOOP_WIDE_REDUCTION(BODY) \
  VF_LOOP_WIDE_REDUCTION_BASE \
  float64_t vs2 = f64(P.VU.elt<float32_t>(rs2_num, i).v); \
  BODY; \
  DEBUG_RVV_FP_VV; \
  VF_LOOP_REDUCTION_END(e64)

#define VFP_VF_LOOP(BODY) \
  VF_LOOP_BASE \
  float32_t &vd = P.VU.elt<float32_t>(rd_num, i); \
  float32_t rs1 = f32(READ_FREG(rs1_num)); \
  float32_t vs2 = P.VU.elt<float32_t>(rs2_num, i); \
  BODY; \
  DEBUG_RVV_FP_VF; \
  VF_LOOP_END

#define VFP_LOOP_CMP(BODY) \
  VF_LOOP_CMP_BASE \
  BODY; \
  DEBUG_RVV_FP_VV; \
  VF_LOOP_CMP_END \

#define VFP_VVF_LOOP_WIDE(BODY) \
  VF_LOOP_BASE \
  float64_t &vd = P.VU.elt<float64_t>(rd_num, i); \
  float64_t vs1 = f32_to_f64(P.VU.elt<float32_t>(rs1_num, i)); \
  float64_t vs2 = f32_to_f64(P.VU.elt<float32_t>(rs2_num, i)); \
  float64_t rs1 = f64(READ_FREG(rs1_num)); \
  BODY; \
  DEBUG_RVV_FP_VV; \
  VF_LOOP_END

#define INT_ROUNDING(result, xrm, gb) \
  switch(xrm){\
    case VRM::RNU:\
      result += ((uint64_t)1 << (gb - 1));\
      break;\
    case VRM::RNE:\
      if ((result & ((uint64_t)0x3 << (gb - 1))) == 0x1){\
          result -= ((uint64_t)1 << (gb - 1));\
	  }else if ((result & ((uint64_t)0x3 << (gb - 1))) == 0x3){\
          result += ((uint64_t)1 << (gb - 1));\
      }\
      break;\
    case VRM::RDN:\
      result = (result >> (gb - 1)) << (gb - 1);\
      break;\
    case VRM::ROD:\
      if ((result & ((uint64_t)0x3 << (gb - 1))) == 0x1){\
          result += ((uint64_t)1 << (gb - 1));\
	  }else if ((result & ((uint64_t)0x3 << (gb - 1))) == 0x3){\
          result -= ((uint64_t)1 << (gb - 1));\
      }\
      break;\
    case VRM::INVALID_RM:\
      assert(true);\
  }
// Seems that 0x0 doesn't work.
#define DEBUG_START             0x100
#define DEBUG_END                 (0x1000 - 1)

#endif
