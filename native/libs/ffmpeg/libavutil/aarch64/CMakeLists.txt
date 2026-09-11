add_sources (
	cpu.c
	float_dsp_init.c
	tx_float_init.c
)

add_sources_if (HAVE_ARMV8 crc.S)
add_sources_if (HAVE_NEON float_dsp_neon.S tx_float_neon.S)

if (CONFIG_PIXELUTILS)
	add_sources_if (HAVE_NEON pixelutils_neon.S)
endif ()

add_sources_if (HAVE_SVE cpu_sve.S)
add_sources_if (HAVE_SME cpu_sme.S)

# if(ENABLE_SVE)
#     target_sources(ffmpeg PRIVATE
#         aarch64/cpu_sve.c
#     )

#     target_compile_options(ffmpeg PRIVATE
#         -march=armv8.2-a+sve
#     )
# endif()

# target_compile_options(ffmpeg PRIVATE
#     -march=armv8.2-a+sve
# )
