

add_sources_if (CONFIG_HEVC_DECODER x86/hevc/dsp_init.c
        x86/hevc/add_res.asm
        x86/hevc/deblock.asm
        x86/hevc/dequant.asm
        x86/hevc/idct.asm
        x86/hevc/mc.asm
        x86/hevc/sao.asm
        x86/hevc/sao_10bit.asm
        x86/h26x/h2656dsp.asm
        x86/h26x/h2656_inter.asm)
