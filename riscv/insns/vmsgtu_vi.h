// vsgtu.vi  vd, vd2, simm5
VI_VI_ULOOP_CMP
({
  uint64_t imm = (int8_t)(zimm5 << 3) >> 3;
  res = vs2 > (imm & (UINT64_MAX >> (64 - P.VU.vsew)));
})
