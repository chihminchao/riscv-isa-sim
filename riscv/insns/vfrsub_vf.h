// vfsub.vf vd, vs2, rs1
VFP_VF_LOOP
({
 switch(P.VU.vsew){
 case e32:
    vd = f32_sub(rs1, vs2);
    break;
 case e16:
 case e8:
 default:
     softfloat_exceptionFlags = 1;
 };
})