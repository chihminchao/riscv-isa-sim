// vsra
VI_VV_LOOP
({
    WRITE_RD(sext_xlen(sext_xlen(vs1) >> (vs2 & (xlen-1))));
})
