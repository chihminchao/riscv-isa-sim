// vlb.v and vlseg[2-8]b.v
require(STATE.VU.vsew >= e8);
reg_t nf = insn.v_nf() + 1;
require((nf >= 2 && STATE.VU.vlmul == 1) || nf == 1);
reg_t vl = STATE.VU.vl;
reg_t baseAddr = RS1;
reg_t vd = insn.rd();
for (reg_t i = STATE.VU.vstart; i < vl; ++i) {
  V_LOOP_ELEMENT_SKIP;

  for (reg_t fn = 0; fn < nf; ++fn) {
    int64_t val = MMU.load_int8(baseAddr + (i * nf + fn) * 1);
    if (STATE.VU.vsew == e8) {
      STATE.VU.elt<uint8_t>(vd + fn, i) = val;
    } else if (STATE.VU.vsew == e16) {
      STATE.VU.elt<uint16_t>(vd + fn, i) = val;
    } else if (STATE.VU.vsew == e32) {
      STATE.VU.elt<uint32_t>(vd + fn, i) = val;
    } else {
      STATE.VU.elt<uint64_t>(vd + fn, i) = val;
    }
  }
}

//zero unfilled part
for (reg_t i = vl; i < STATE.VU.vlmax; ++i) {
  for (reg_t fn = 0; fn < nf; ++fn) {
    if (STATE.VU.vsew == e8) {
      STATE.VU.elt<uint8_t>(vd + fn, i) = 0;
    } else if (STATE.VU.vsew == e16) {
      STATE.VU.elt<uint16_t>(vd + fn, i) = 0;
    } else if (STATE.VU.vsew == e32) {
      STATE.VU.elt<uint32_t>(vd + fn, i) = 0;
    } else {
      STATE.VU.elt<uint64_t>(vd + fn, i) = 0;
    }
  }
}
STATE.VU.vstart = 0;
