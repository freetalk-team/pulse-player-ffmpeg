

add_sources(
    cpu.c
    cpuid.asm
    crc.asm
    fixed_dsp.asm
    fixed_dsp_init.c
    float_dsp.asm
    float_dsp_init.c
    imgutils.asm
    imgutils_init.c
    lls.asm
    lls_init.c
    tx_float.asm
    tx_float_init.c
)

if ((HAVE_MMX_INLINE OR HAVE_MMX_EXTERNAL) AND HAVE_MM_EMPTY)
    add_sources(emms.asm)
endif ()

add_sources_if (HAVE_AESNI_EXTERNAL
    aes.asm
    aes_init.c
)

add_sources_if (CONFIG_PIXELUTILS pixelutils.asm)

foreach(def ${NASM_DEFINES})
    target_compile_options(${TARGET} PRIVATE
        "$<$<COMPILE_LANGUAGE:ASM_NASM>:-D${def}>"
    )
endforeach()

target_compile_options(${TARGET} PRIVATE
    "$<$<COMPILE_LANGUAGE:ASM_NASM>:-I${CMAKE_CURRENT_SOURCE_DIR}/>"
)
