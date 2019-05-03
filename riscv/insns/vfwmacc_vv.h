// vfwmacc.vv vd, vs2, vs1
VFP_VVF_LOOP_WIDE
({
  switch(p->VU.vsew){
  case e32:
    vd = f64_mulAdd(vs1, vs2, vd);
    break;
  case e16:
  case e8:
  default:
    softfloat_exceptionFlags = 1;
    break;
  };
})
