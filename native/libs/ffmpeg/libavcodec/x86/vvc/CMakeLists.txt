
add_sources_if (CONFIG_VVC_DECODER x86/vvc/dsp_init.c
        x86/vvc/alf.asm
        x86/vvc/dmvr.asm
        x86/vvc/mc.asm
        x86/vvc/of.asm
        x86/vvc/sad.asm
        x86/vvc/sao.asm
        x86/vvc/sao_10bit.asm)
