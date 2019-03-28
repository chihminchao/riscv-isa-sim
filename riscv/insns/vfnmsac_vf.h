// vfnmsac: vd[i] = -(f[rs1] * vs2[i]) + vd[i]
require_fp;
softfloat_roundingMode = STATE.VU.vxrm;

VFP_VF_LOOP
({
 switch(STATE.VU.vsew){
 case e32:
    vd = f32_mulAdd(rs1, f32(vs2.v ^ F32_SIGN), vd);
    break;
 case e16:
 case e8:
 default:
     softfloat_exceptionFlags = 1;
 };
})
set_fp_exceptions;
