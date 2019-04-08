// vsxb.v and vsxseg[2-8]b.v
require(STATE.VU.vsew >= e8);
reg_t nf = insn.v_nf() + 1;
require((nf >= 2 && STATE.VU.vlmul == 1) || nf == 1);
reg_t vl = STATE.VU.vl;
reg_t baseAddr = RS1;
reg_t stride = RS2;
reg_t vs3 = insn.rd();
for (reg_t i = STATE.VU.vstart; i < vl; ++i) {
  V_LOOP_ELEMENT_SKIP;

  reg_t index = STATE.VU.elt<int8_t>(stride, i);
  for (reg_t fn = 0; fn < nf; ++fn) {
    uint8_t val = 0;
    switch (STATE.VU.vsew) {
    case e8:
      val = STATE.VU.elt<uint8_t>(vs3 + fn, i);
      break;
    case e16:
      val = STATE.VU.elt<uint16_t>(vs3 + fn, i);
      break;
    case e32:
      val = STATE.VU.elt<uint32_t>(vs3 + fn, i);
      break;
    defaualt:
      val = STATE.VU.elt<uint64_t>(vs3 + fn, i);
      break;
    }
    MMU.store_uint8(baseAddr + i * index + fn * 1, val);
  }
}
STATE.VU.vstart = 0;
