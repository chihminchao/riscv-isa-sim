// vfmin:
require_fp;
softfloat_roundingMode = STATE.VU.vxrm;
bool less;

VFP_VV_LOOP
({
 switch(STATE.VU.vsew){
 case e32:
    less = f32_lt_quiet(vs1, vs2) || (f32_eq(vs1, vs2) && (vs1.v & F32_SIGN));

    if(isNaNF32UI(vs1.v) && isNaNF32UI(vs2.v)){
      vd = f32(defaultNaNF32UI);
    }else{
      vd = (less || isNaNF32UI(vs2.v) ? vs1 : vs2);
    }
    break;
 case e16:
 case e8:
 default:
     softfloat_exceptionFlags = 1;
 };
})
set_fp_exceptions;