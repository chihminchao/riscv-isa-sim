// vs1r.v vs3, (rs1)
const reg_t baseAddr = RS1;
const reg_t vs3 = insn.rd();
for (reg_t i = 0; i < P.VU.vlenb; ++i) {
  auto val = P.VU.elt<uint8_t>(vs3, i);
  MMU.store_uint8(baseAddr + i, val, g_vector_mistrap);
}
P.VU.vstart = 0; 
