
add_sources(constants.c)

if ((HAVE_MMX_INLINE OR HAVE_MMX_EXTERNAL) AND HAVE_MM_EMPTY)
    add_sources(emms.c)
endif ()

# subsystems
add_sources_if (CONFIG_AC3DSP ac3dsp_init.c)
add_sources_if (CONFIG_AUDIODSP audiodsp_init.c)
add_sources_if (CONFIG_BLOCKDSP blockdsp_init.c)
add_sources_if (CONFIG_BSWAPDSP bswapdsp_init.c)
add_sources_if (CONFIG_DIRAC_DECODER diracdsp_init.c dirac_dwt_init.c)
add_sources_if (CONFIG_FDCTDSP fdctdsp_init.c fdct.c)
add_sources_if (CONFIG_FMTCONVERT fmtconvert_init.c)
add_sources_if (CONFIG_H263DSP h263dsp_init.c)
add_sources_if (CONFIG_H264CHROMA h264chroma_init.c)
add_sources_if (CONFIG_H264DSP h264dsp_init.c)
add_sources_if (CONFIG_H264PRED h264_intrapred_init.c)
add_sources_if (CONFIG_H264QPEL h264_qpel.c)
add_sources_if (CONFIG_HPELDSP hpeldsp_init.c)
add_sources_if (CONFIG_HUFFYUVDSP huffyuvdsp_init.c)
add_sources_if (CONFIG_HUFFYUVENCDSP huffyuvencdsp_init.c)
add_sources_if (CONFIG_IDCTDSP idctdsp_init.c)
add_sources_if (CONFIG_LLAUDDSP lossless_audiodsp_init.c)
add_sources_if (CONFIG_LLVIDDSP lossless_videodsp_init.c)
add_sources_if (CONFIG_LLVIDENCDSP lossless_videoencdsp_init.c)
add_sources_if (CONFIG_LPC lpc_init.c)
add_sources_if (CONFIG_ME_CMP me_cmp_init.c)
add_sources_if (CONFIG_MPEGAUDIODSP mpegaudiodsp.c)
add_sources_if (CONFIG_MPEGVIDEO mpegvideo.c)
add_sources_if (CONFIG_MPEGVIDEOENC mpegvideoenc.c)
add_sources_if (CONFIG_MPEGVIDEOENCDSP mpegvideoencdsp_init.c)
add_sources_if (CONFIG_PIXBLOCKDSP pixblockdsp_init.c)
add_sources_if (CONFIG_QPELDSP qpeldsp_init.c)
add_sources_if (CONFIG_RV34DSP rv34dsp_init.c)
add_sources_if (CONFIG_VC1DSP vc1dsp_init.c vc1dsp_mmx.c)
add_sources_if (CONFIG_VIDEODSP videodsp_init.c)
add_sources_if (CONFIG_VP3DSP vp3dsp_init.c)
add_sources_if (CONFIG_VP8DSP vp8dsp_init.c)
add_sources_if (CONFIG_XMM_CLOBBER_TEST w64xmmtest.c)

# decoders/encoders
add_sources_if (CONFIG_AAC_DECODER aacpsdsp_init.c sbrdsp_init.c)
add_sources_if (CONFIG_AAC_ENCODER aacencdsp_init.c)
add_sources_if (CONFIG_ADPCM_G722_DECODER g722dsp_init.c)
add_sources_if (CONFIG_ADPCM_G722_ENCODER g722dsp_init.c)
add_sources_if (CONFIG_ALAC_DECODER alacdsp_init.c)
add_sources_if (CONFIG_APNG_DECODER pngdsp_init.c)
add_sources_if (CONFIG_APV_DECODER apv_dsp_init.c)
add_sources_if (CONFIG_CAVS_DECODER cavsdsp.c)
add_sources_if (CONFIG_CFHD_DECODER cfhddsp_init.c)
add_sources_if (CONFIG_CFHD_ENCODER cfhdencdsp_init.c)
add_sources_if (CONFIG_DCA_DECODER dcadsp_init.c synth_filter_init.c)
add_sources_if (CONFIG_DNXHD_ENCODER dnxhdenc_init.c)
add_sources_if (CONFIG_EXR_DECODER exrdsp_init.c)
add_sources_if (CONFIG_FLAC_DECODER flacdsp_init.c)
add_sources_if (CONFIG_FLAC_ENCODER flacencdsp_init.c)
add_sources_if (CONFIG_OPUS_DECODER opusdsp_init.c)
add_sources_if (CONFIG_OPUS_ENCODER celt_pvq_init.c)
add_sources_if (CONFIG_JPEG2000_DECODER jpeg2000dsp_init.c)
add_sources_if (CONFIG_LSCR_DECODER pngdsp_init.c)
add_sources_if (CONFIG_MLP_DECODER mlpdsp_init.c)
add_sources_if (CONFIG_MPEG4_DECODER mpeg4videodsp.c)
add_sources_if (CONFIG_MPEG4_DECODER xvididct_init.c)
add_sources_if (CONFIG_PNG_DECODER pngdsp_init.c)
add_sources_if (CONFIG_PRORES_DECODER proresdsp_init.c)
add_sources_if (CONFIG_PRORES_RAW_DECODER proresdsp_init.c)
add_sources_if (CONFIG_RV40_DECODER rv40dsp_init.c)
add_sources_if (CONFIG_SBC_ENCODER sbcdsp_init.c)
add_sources_if (CONFIG_SNOW_DECODER snowdsp_init.c)
add_sources_if (CONFIG_SNOW_DECODER snowdsp.c)
add_sources_if (CONFIG_SNOW_ENCODER snowdsp_init.c)
add_sources_if (CONFIG_SNOW_ENCODER snowdsp.c)
add_sources_if (CONFIG_SVQ1_ENCODER svq1enc_init.c)
add_sources_if (CONFIG_TAK_DECODER takdsp_init.c)
add_sources_if (CONFIG_TRUEHD_DECODER mlpdsp_init.c)
add_sources_if (CONFIG_TTA_DECODER ttadsp_init.c)
add_sources_if (CONFIG_TTA_ENCODER ttaencdsp_init.c)
add_sources_if (CONFIG_UTVIDEO_DECODER utvideodsp_init.c)
add_sources_if (CONFIG_V210_DECODER v210-init.c)
add_sources_if (CONFIG_V210_ENCODER v210enc_init.c)
add_sources_if (CONFIG_VORBIS_DECODER vorbisdsp_init.c)
add_sources_if (CONFIG_VP6_DECODER vp6dsp_init.c)
add_sources_if (CONFIG_VP9_DECODER vp9dsp_init.c
        vp9dsp_init_10bpp.c
        vp9dsp_init_12bpp.c
        vp9dsp_init_16bpp.c)


# subsystems
add_sources_if (CONFIG_AC3DSP ac3dsp.asm ac3dsp_downmix.asm)
add_sources_if (CONFIG_AUDIODSP audiodsp.asm)
add_sources_if (CONFIG_BLOCKDSP blockdsp.asm)
add_sources_if (CONFIG_BSWAPDSP bswapdsp.asm)
add_sources_if (CONFIG_FMTCONVERT fmtconvert.asm)
add_sources_if (CONFIG_H263DSP h263_loopfilter.asm)
add_sources_if (CONFIG_H264CHROMA h264_chromamc.asm h264_chromamc_10bit.asm)
add_sources_if (CONFIG_H264DSP h264_deblock.asm
        h264_deblock_10bit.asm
        h264_idct.asm
        h264_idct_10bit.asm
        h264_weight.asm
        h264_weight_10bit.asm)
add_sources_if (CONFIG_H264PRED h264_intrapred.asm h264_intrapred_10bit.asm)
add_sources_if (CONFIG_H264QPEL h264_qpel_8bit.asm
        h264_qpel_10bit.asm
        fpel.asm
        qpel.asm)
add_sources_if (CONFIG_HPELDSP fpel.asm hpeldsp.asm)
add_sources_if (CONFIG_HUFFYUVDSP huffyuvdsp.asm)
add_sources_if (CONFIG_HUFFYUVENCDSP huffyuvencdsp.asm)
add_sources_if (CONFIG_IDCTDSP idctdsp.asm)
add_sources_if (CONFIG_LLAUDDSP lossless_audiodsp.asm)
add_sources_if (CONFIG_LLVIDDSP lossless_videodsp.asm)
add_sources_if (CONFIG_LLVIDENCDSP lossless_videoencdsp.asm)
add_sources_if (CONFIG_LPC lpc.asm)
add_sources_if (CONFIG_ME_CMP me_cmp.asm)
add_sources_if (CONFIG_MPEGAUDIODSP dct32.asm imdct36.asm)
add_sources_if (CONFIG_MPEGVIDEOENCDSP mpegvideoencdsp.asm)
add_sources_if (CONFIG_OPUS_DECODER opusdsp.asm)
add_sources_if (CONFIG_OPUS_ENCODER celt_pvq_search.asm)
add_sources_if (CONFIG_PIXBLOCKDSP pixblockdsp.asm)
add_sources_if (CONFIG_QPELDSP qpeldsp.asm fpel.asm qpel.asm)
add_sources_if (CONFIG_RV34DSP rv34dsp.asm)
add_sources_if (CONFIG_VC1DSP vc1dsp_loopfilter.asm vc1dsp_mc.asm fpel.asm)

if (ARCH_X86_64)
    add_sources_if (CONFIG_IDCTDSP simple_idct10.asm)
else ()
    add_sources_if (CONFIG_IDCTDSP simple_idct.asm)
endif ()

add_sources_if (CONFIG_VIDEODSP videodsp.asm)
add_sources_if (CONFIG_VP3DSP vp3dsp.asm)
add_sources_if (CONFIG_VP8DSP vp8dsp.asm vp8dsp_loopfilter.asm)

# decoders/encoders
add_sources_if (CONFIG_AAC_DECODER aacpsdsp.asm sbrdsp.asm)
add_sources_if (CONFIG_AAC_ENCODER aacencdsp.asm)
add_sources_if (CONFIG_ADPCM_G722_DECODER g722dsp.asm)
add_sources_if (CONFIG_ADPCM_G722_ENCODER g722dsp.asm)
add_sources_if (CONFIG_ALAC_DECODER alacdsp.asm)
add_sources_if (CONFIG_APNG_DECODER pngdsp.asm)
add_sources_if (CONFIG_APV_DECODER apv_dsp.asm)
add_sources_if (CONFIG_CAVS_DECODER cavsidct.asm cavs_qpel.asm)
add_sources_if (CONFIG_CFHD_ENCODER cfhdencdsp.asm)
add_sources_if (CONFIG_CFHD_DECODER cfhddsp.asm)
add_sources_if (CONFIG_DCA_DECODER dcadsp.asm synth_filter.asm)
add_sources_if (CONFIG_DIRAC_DECODER diracdsp.asm dirac_dwt.asm)
add_sources_if (CONFIG_DNXHD_ENCODER dnxhdenc.asm)
add_sources_if (CONFIG_EXR_DECODER exrdsp.asm)
add_sources_if (CONFIG_FLAC_DECODER flacdsp.asm)

if (CONFIG_GPL)
    add_sources_if (CONFIG_FLAC_ENCODER flac_dsp_gpl.asm)
endif ()

add_sources_if (CONFIG_JPEG2000_DECODER jpeg2000dsp.asm)
add_sources_if (CONFIG_LSCR_DECODER pngdsp.asm)
add_sources_if (CONFIG_MLP_DECODER mlpdsp.asm)
add_sources_if (CONFIG_MPEG4_DECODER xvididct.asm)
add_sources_if (CONFIG_PNG_DECODER pngdsp.asm)
add_sources_if (CONFIG_PRORES_DECODER proresdsp.asm)
add_sources_if (CONFIG_PRORES_RAW_DECODER proresdsp.asm)
add_sources_if (CONFIG_RV40_DECODER rv40dsp.asm)
add_sources_if (CONFIG_SBC_ENCODER sbcdsp.asm)
add_sources_if (CONFIG_SVQ1_ENCODER svq1enc.asm)
add_sources_if (CONFIG_TAK_DECODER takdsp.asm)
add_sources_if (CONFIG_TRUEHD_DECODER mlpdsp.asm)
add_sources_if (CONFIG_TTA_DECODER ttadsp.asm)
add_sources_if (CONFIG_TTA_ENCODER ttaencdsp.asm)
add_sources_if (CONFIG_UTVIDEO_DECODER utvideodsp.asm)
add_sources_if (CONFIG_V210_ENCODER v210enc.asm)
add_sources_if (CONFIG_V210_DECODER v210.asm)
add_sources_if (CONFIG_VORBIS_DECODER vorbisdsp.asm)
add_sources_if (CONFIG_VP6_DECODER vp6dsp.asm)
add_sources_if (CONFIG_VP9_DECODER vp9intrapred.asm
        vp9intrapred_16bpp.asm
        vp9itxfm.asm
        vp9itxfm_avx2.asm
        vp9itxfm_avx512.asm
        vp9itxfm_16bpp.asm
        vp9itxfm_16bpp_avx512.asm
        vp9lpf.asm
        vp9lpf_16bpp.asm
        vp9mc.asm
        vp9mc_16bpp.asm)

add_subdirectory (hevc)
add_subdirectory (vvc)
add_subdirectory (h26x)

target_compile_options(${TARGET} PRIVATE
    "$<$<COMPILE_LANGUAGE:ASM_NASM>:-I${CMAKE_CURRENT_SOURCE_DIR}>"
)

foreach(def ${NASM_DEFINES})
    target_compile_options(${TARGET} PRIVATE
        "$<$<COMPILE_LANGUAGE:ASM_NASM>:-D${def}>"
    )
endforeach()