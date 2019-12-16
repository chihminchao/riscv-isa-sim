// vl1r.v vd, (rs1)
require_vector;
const reg_t baseAddr = RS1;
const reg_t vd = insn.rd();
for (reg_t i = 0; i < P.VU.vlenb; ++i) {
  
  auto val = MMU.load_uint8(baseAddr + i, g_vector_mistrap);
  fprintf(stderr, "here: %ld: %x\n", i, val);
  P.VU.elt<uint8_t>(vd, i) = val;
}
P.VU.vstart = 0;