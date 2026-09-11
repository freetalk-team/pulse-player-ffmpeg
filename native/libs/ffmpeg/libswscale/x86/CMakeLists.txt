
add_sources (
    rgb2rgb.c
    swscale.c
    yuv2rgb.c
    input.asm
    output.asm
    scale.asm
    scale_avx2.asm
    range_convert.asm
    rgb_2_rgb.asm
    yuv_2_rgb.asm
    yuv2yuvX.asm
)

if ((HAVE_MMX_INLINE OR HAVE_MMX_EXTERNAL) AND HAVE_MM_EMPTY)
    add_sources(emms.asm)
endif ()

if (CONFIG_UNSTABLE)
    add_sources_if (ARCH_X86_64 
        ops.c
        ops_common.asm
        ops_int.asm
        ops_float.asm)
endif ()

foreach(def ${NASM_DEFINES})
    target_compile_options(${TARGET} PRIVATE
        "$<$<COMPILE_LANGUAGE:ASM_NASM>:-D${def}>"
    )
endforeach()

target_compile_options(${TARGET} PRIVATE
    "$<$<COMPILE_LANGUAGE:ASM_NASM>:-I${CMAKE_CURRENT_SOURCE_DIR}/>"
)
