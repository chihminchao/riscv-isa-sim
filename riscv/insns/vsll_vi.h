// vsll
VI_VI_LOOP
({
    WRITE_RD(sext_xlen(simm5 << (vs2 & (xlen-1))));
})