// vfsub.vv vd, vs2, vs1
VI_VFP_VV_LOOP
({
 switch(P.VU.vsew){
 case e32:
    vd = f32_sub(vs2, vs1);
    break;
 case e16:
 case e8:
 default:
     softfloat_exceptionFlags = 1;
 };
})
