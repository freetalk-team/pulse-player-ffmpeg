
# replace 
# %s/\v^\s*OBJS-\$\((CONFIG_[^)]+)\)\s*\+\=\s*(.*)$/add_sources_if (\1 \2)/
# %s/\v^\s*STLIBOBJS-\$\((CONFIG_[^)]+)\)\s*\+\=\s*(.*)$/add_sources_if (\1 \2)/
# %s/\.o\s*\\$/.c/

set (AV_HAVE_BIGENDIAN ${HAVE_BIGENDIAN})
set (AV_HAVE_FAST_UNALIGNED ${HAVE_FAST_UNALIGNED})

configure_file (avconfig.h.in avconfig.h)

set_target (avutil)

add_library (${TARGET} OBJECT
    adler32.c
    aes.c
    aes_ctr.c
    ambient_viewing_environment.c
    audio_fifo.c
    avstring.c
    avsscanf.c
    base64.c
    blowfish.c
    bprint.c
    buffer.c
    cast5.c
    camellia.c
    channel_layout.c
    container_fifo.c
    cpu.c
    crc.c
    csp.c
    des.c
    detection_bbox.c
    dict.c
    display.c
    dovi_meta.c
    downmix_info.c
    encryption_info.c
    error.c
    eval.c
    executor.c
    fifo.c
    file.c
    file_open.c
    float_dsp.c
    float_scalarproduct.c
    film_grain_params.c
    fixed_dsp.c
    frame.c
    hash.c
    hdr_dynamic_metadata.c
    hdr_dynamic_vivid_metadata.c
    hmac.c
    hwcontext.c
    iamf.c
    imgutils.c
    integer.c
    intmath.c
    lfg.c
    lls.c
    log.c
    log2_tab.c
    lzo.c
    mathematics.c
    mastering_display_metadata.c
    md5.c
    mem.c
    murmur3.c
    opt.c
    parseutils.c
    pixdesc.c
    pixelutils.c
    random_seed.c
    raw_color_params.c
    rational.c
    refstruct.c
    reverse.c
    rc4.c
    ripemd.c
    samplefmt.c
    side_data.c
    sha.c
    sha512.c
    slicethread.c
    spherical.c
    stereo3d.c
    tdrdi.c
    threadmessage.c
    time.c
    timecode.c
    timecode_internal.c
    timestamp.c
    tree.c
    twofish.c
    utils.c
    xga_font_data.c
    xtea.c
    tea.c
    tx.c
    tx_float.c
    tx_double.c
    tx_int32.c
    uuid.c
    version.c
    video_enc_params.c
    video_hint.c
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

if (CONFIG_PIC)
    set_target_properties(${TARGET} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
 
    target_compile_definitions(${TARGET} PRIVATE PIC)
endif ()

add_sources_if (CONFIG_CUDA hwcontext_cuda.c)
add_sources_if (CONFIG_D3D11VA hwcontext_d3d11va.c)
add_sources_if (CONFIG_D3D12VA hwcontext_d3d12va.c)
add_sources_if (CONFIG_DXVA2 hwcontext_dxva2.c)
add_sources_if (CONFIG_AMF hwcontext_amf.c)
add_sources_if (CONFIG_LIBDRM hwcontext_drm.c)
add_sources_if (CONFIG_MACOS_KPERF macos_kperf.c)
add_sources_if (CONFIG_MEDIACODEC hwcontext_mediacodec.c)
add_sources_if (CONFIG_OHCODEC hwcontext_oh.c)
add_sources_if (CONFIG_OPENCL hwcontext_opencl.c)
add_sources_if (CONFIG_QSV hwcontext_qsv.c)
add_sources_if (CONFIG_VAAPI hwcontext_vaapi.c)
add_sources_if (CONFIG_VIDEOTOOLBOX hwcontext_videotoolbox.c)
add_sources_if (CONFIG_VDPAU hwcontext_vdpau.c)
add_sources_if (CONFIG_VULKAN hwcontext_vulkan.c vulkan.c)

add_sources_if_not (CONFIG_VULKAN hwcontext_stub.c)

#OBJS += $(COMPAT_OBJS:%=../compat/%)

# Object files to satisfy dependencies of other libraries for static builds
add_sources_if (CONFIG_EXR_DECODER half2float.c float2half.c)
add_sources_if (CONFIG_EXR_ENCODER float2half.c)
add_sources_if (CONFIG_PHM_DECODER half2float.c)
add_sources_if (CONFIG_PHM_ENCODER float2half.c)
add_sources_if (CONFIG_SWSCALE half2float.c)

if (ARCH_X86)
    add_subdirectory (x86)
elseif (ARCH_AARCH64)
    add_subdirectory (aarch64)
endif ()

# Windows resource file
#SHLIBOBJS-$(HAVE_GNU_WINDRES)           += avutilres.c

# target_compile_definitions(${TARGET} PRIVATE
#     HAVE_ALIGNED_STACK=1
# )



# target_compile_options(${TARGET} PRIVATE
#     "$<$<COMPILE_LANGUAGE:ASM_NASM>:-DHAVE_ALIGNED_STACK=1>"
# )

target_link_libraries (${TARGET} PRIVATE OpenSSL::Crypto)
