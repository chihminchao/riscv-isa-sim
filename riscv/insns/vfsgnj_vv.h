// vfsgnj
VFP_VV_LOOP
({
  switch(STATE.VU.vsew) {
  case e32:
    vd = fsgnj32(vs1.v, vs2.v, false, false);
    break;
  case e16:
  case e8:
  default:
    softfloat_exceptionFlags = 1;
  };
})
