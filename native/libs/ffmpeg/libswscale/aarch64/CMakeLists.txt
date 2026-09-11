
set(SWS_GENERATED_OPS_ASM ${CMAKE_CURRENT_BINARY_DIR}/ops.S)
set(SWS_GENERATED_LOOKUP_ASM ${CMAKE_CURRENT_BINARY_DIR}/lookup.c)

# Build generator for HOST system (your build machine)
# Use the native clang from llvm-mingw (which runs on your host)
if (MINGW)
	
	set(GENERATOR_HOST_EXE
		${CMAKE_CURRENT_BINARY_DIR}/sws_ops_asmgen
	)

	# Build the generator for the host system using the host compiler
	add_custom_command(
		OUTPUT ${GENERATOR_HOST_EXE}
		COMMAND ${HOST_CC} ${CMAKE_CURRENT_SOURCE_DIR}/ops_asmgen.c -I${PROJECT_SOURCE_DIR} -o ${GENERATOR_HOST_EXE}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ops_asmgen.c
		COMMENT "Building ops_asmgen for host (Linux)"
		VERBATIM
	)

	# Use the host-built generator to create ops.S
	add_custom_command(
		OUTPUT ${SWS_GENERATED_OPS_ASM}
		COMMAND ${GENERATOR_HOST_EXE} -ops > ${SWS_GENERATED_OPS_ASM}
		DEPENDS ${GENERATOR_HOST_EXE}
		COMMENT "Generating ops.S from ops_asmgen"
		VERBATIM
	)

	add_custom_command(
		OUTPUT ${SWS_GENERATED_LOOKUP_ASM}
		COMMAND ${GENERATOR_HOST_EXE} -lookup > ${SWS_GENERATED_LOOKUP_ASM}
		DEPENDS ${GENERATOR_HOST_EXE}
		COMMENT "Generating lookup.c from ops_asmgen"
		VERBATIM
	)
else ()
	add_executable(sws_ops_asmgen
		ops_asmgen.c
	)

	add_custom_command(
		OUTPUT ${SWS_GENERATED_OPS_ASM}

		COMMAND
			/bin/sh -c
			"\"$<TARGET_FILE:sws_ops_asmgen>\" -ops > \"${SWS_GENERATED_OPS_ASM}\""
		DEPENDS
			sws_ops_asmgen
		VERBATIM
	)

	add_custom_command(
		OUTPUT ${SWS_GENERATED_LOOKUP_ASM}

		COMMAND
			/bin/sh -c
			"\"$<TARGET_FILE:sws_ops_asmgen>\" -lookup > \"${SWS_GENERATED_LOOKUP_ASM}\""
		DEPENDS
			sws_ops_asmgen
		VERBATIM
	)
endif ()

add_custom_target(sws_generate_ops
    DEPENDS "${SWS_GENERATED_OPS_ASM}"
    DEPENDS "${SWS_GENERATED_LOOKUP_ASM}"
)

set_source_files_properties(
    ${SWS_GENERATED_OPS_ASM} 
	${SWS_GENERATED_LOOKUP_ASM} 
    PROPERTIES
        GENERATED TRUE
)

add_dependencies(${TARGET} sws_generate_ops)

add_sources (
    rgb2rgb.c
    swscale.c
    swscale_unscaled.c
	${SWS_GENERATED_OPS_ASM}
	${SWS_GENERATED_LOOKUP_ASM}
)

add_sources_if (HAVE_NEON 
    hscale.S 
    input.S 
    output.S 
    range_convert_neon.S 
    rgb2rgb_neon.S 
    swscale_unscaled_neon.S 
    xyz2rgb_neon.S 
    yuv2rgb_neon.S)

if (CONFIG_UNSTABLE)
    # add_sources_if (HAVE_NEON ops.c ops_neon.gen.S ops_lookup.gen.S)
    add_sources_if (HAVE_NEON ops.c)
endif ()


# $(SUBDIR)ops_neon.gen.S: $(SUBDIR)ops_asmgen$(HOSTEXESUF)
# 	$(M)$< -ops > $@.tmp
# 	$(CP) $@.tmp $@
# 	$(RM) $@.tmp

# $(SUBDIR)ops_lookup.gen.c: $(SUBDIR)ops_asmgen$(HOSTEXESUF)
# 	$(M)$< -lookup > $@.tmp
# 	$(CP) $@.tmp $@
# 	$(RM) $@.tmp

# clean::
# 	$(RM) $(CLEANSUFFIXES:%=libswscale/%)

# HOSTPROGS = ops_asmgen
