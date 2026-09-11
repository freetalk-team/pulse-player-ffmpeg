
add_sources_if (CONFIG_AAC_DECODER 
    aacdec.c 
    aacdec_tab.c 
    aacdec_float.c 
    aacdec_usac.c
    aacdec_ac.c 
    aacdec_lpd.c
    aacdec_usac_mps212.c
)

add_sources_if (CONFIG_AAC_FIXED_DECODER 
    aacdec.c 
    aacdec_tab.c
    aacdec_fixed.c
)
