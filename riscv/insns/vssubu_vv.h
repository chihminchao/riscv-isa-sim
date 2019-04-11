// vssubu.vv vd, vs2, vs1
VI_LOOP_BASE

  V_LOOP_ELEMENT_SKIP;
  bool sat = false;

  switch (sew) {
  case e8: {
    VV_U_PARAMS(e8);
    vd = sat_subu<uint8_t>(vs2, vs1, sat);
    printf("%x   %x   %x   %x\n", vs2, vs1, vd, sat);
    break;
    }
  case e16: {
    VV_U_PARAMS(e16);
    vd = sat_subu<uint16_t>(vs2, vs1, sat);
    break;
    }
  case e32: {
    VV_U_PARAMS(e32);
    vd = sat_subu<uint32_t>(vs2, vs1, sat);
    break;
    }
  default: {
    VV_U_PARAMS(e64);
    vd = sat_subu<uint64_t>(vs2, vs1, sat);
    break;
    }
  }
  STATE.VU.vxsat |= sat;

VI_LOOP_END
