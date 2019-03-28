// vsxh.v and vsxseg[2-8]h.v
require(STATE.VU.vsew >= e16);
reg_t nf = insn.v_nf() + 1;
require((nf >= 2 && STATE.VU.vlmul == 1) || nf == 1);
reg_t vl = STATE.VU.vl;
reg_t baseAddr = RS1;
reg_t stride = RS2;
reg_t vs3 = insn.rd();
for (reg_t i = STATE.VU.vstart; i < vl; ++i) {
  V_LOOP_ELEMENT_SKIP;

  reg_t index = STATE.VU.elt<int16_t>(stride, i);
  for (reg_t fn = 0; fn < nf; ++fn) {
    MMU.store_uint16(baseAddr + i * index + fn * 2, STATE.VU.elt<uint16_t>(vs3 + fn, i));
  }
}
STATE.VU.vstart = 0;
