
set_target (swscale)

add_library (${TARGET} OBJECT
    alphablend.c
    cms.c
    csputils.c
    hscale.c
    hscale_fast_bilinear.c
    filters.c
    format.c
    framepool.c
    gamma.c
    graph.c
    input.c
    lut3d.c
    options.c
    output.c
    rgb2rgb.c
    slice.c
    swscale.c
    swscale_unscaled.c
    utils.c
    version.c
    yuv2rgb.c
    vscale.c
)

add_sources_if (CONFIG_UNSTABLE
    ops.c
    ops_chain.c
    ops_dispatch.c
    ops_memcpy.c
    ops_optimizer.c
    rational64.c
    uops.c
    uops_backend.c
)

if (CONFIG_PIC)
    set_target_properties(${TARGET} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(${TARGET} PRIVATE PIC)
endif ()

if (ARCH_X86)
    add_subdirectory (x86)
elseif (ARCH_AARCH64)
    add_subdirectory (aarch64)
endif ()

