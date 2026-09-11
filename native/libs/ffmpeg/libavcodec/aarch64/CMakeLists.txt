# subsystems
add_sources_if (CONFIG_AC3DSP ac3dsp_init_aarch64.c)
add_sources_if (CONFIG_FDCTDSP fdctdsp_init_aarch64.c)
add_sources_if (CONFIG_FMTCONVERT fmtconvert_init.c)
add_sources_if (CONFIG_H264CHROMA h264chroma_init_aarch64.c)
add_sources_if (CONFIG_H264DSP h264dsp_init_aarch64.c)
add_sources_if (CONFIG_HUFFYUVDSP huffyuvdsp_init_aarch64.c)
add_sources_if (CONFIG_H264PRED h264pred_init.c)
add_sources_if (CONFIG_H264QPEL h264qpel_init_aarch64.c)
add_sources_if (CONFIG_HPELDSP hpeldsp_init_aarch64.c)
add_sources_if (CONFIG_IDCTDSP idctdsp_init_aarch64.c)
add_sources_if (CONFIG_ME_CMP me_cmp_init_aarch64.c)
add_sources_if (CONFIG_MPEGAUDIODSP mpegaudiodsp_init.c)
add_sources_if (CONFIG_MPEGVIDEOENCDSP mpegvideoencdsp_init.c)
add_sources_if (CONFIG_NEON_CLOBBER_TEST neontest.c)
add_sources_if (CONFIG_PIXBLOCKDSP pixblockdsp_init_aarch64.c)
add_sources_if (CONFIG_PNG_DECODER pngdsp_init.c)
add_sources_if (CONFIG_VIDEODSP videodsp_init.c)
add_sources_if (CONFIG_VP8DSP vp8dsp_init_aarch64.c)

# decoders/encoders
add_sources_if (CONFIG_AAC_DECODER aacpsdsp_init_aarch64.c sbrdsp_init_aarch64.c)
add_sources_if (CONFIG_AAC_ENCODER aacencdsp_init.c)
add_sources_if (CONFIG_DCA_DECODER dcadsp_init_aarch64.c synth_filter_init.c)
add_sources_if (CONFIG_OPUS_DECODER opusdsp_init.c)
add_sources_if (CONFIG_RV40_DECODER rv40dsp_init_aarch64.c)
add_sources_if (CONFIG_VC1DSP vc1dsp_init_aarch64.c)
add_sources_if (CONFIG_VORBIS_DECODER vorbisdsp_init.c)
add_sources_if (CONFIG_VP9_DECODER vp9dsp_init_10bpp_aarch64.c vp9dsp_init_12bpp_aarch64.c vp9mc_aarch64.S vp9dsp_init_aarch64.c)

# ARMv8 optimizations

# subsystems
if (HAVE_ARMV8)
add_sources_if (CONFIG_VIDEODSP videodsp.S)
endif ()

# NEON optimizations

# subsystems
if (HAVE_NEON)

add_sources_if (CONFIG_AAC_DECODER sbrdsp_neon.S)
add_sources_if (CONFIG_AAC_ENCODER aacencdsp_neon.S)
add_sources_if (CONFIG_AC3DSP ac3dsp_neon.S)
add_sources_if (CONFIG_FDCTDSP fdctdsp_neon.S)
add_sources_if (CONFIG_FMTCONVERT fmtconvert_neon.S)
add_sources_if (CONFIG_H264CHROMA h264cmc_neon.S)
add_sources_if (CONFIG_H264DSP h264dsp_neon.S h264idct_neon.S)
add_sources_if (CONFIG_H264PRED h264pred_neon.S)
add_sources_if (CONFIG_H264QPEL h264qpel_neon.S hpeldsp_neon.S)
add_sources_if (CONFIG_HPELDSP hpeldsp_neon.S)
add_sources_if (CONFIG_HUFFYUVDSP huffyuvdsp_neon.S)
add_sources_if (CONFIG_IDCTDSP idctdsp_neon.S simple_idct_neon.S)
add_sources_if (CONFIG_ME_CMP me_cmp_neon.S)
add_sources_if (CONFIG_MPEGAUDIODSP mpegaudiodsp_neon.S)
add_sources_if (CONFIG_MPEGVIDEOENCDSP mpegvideoencdsp_neon.S)
add_sources_if (CONFIG_PIXBLOCKDSP pixblockdsp_neon.S)
add_sources_if (CONFIG_PNG_DECODER pngdsp_neon.S)
add_sources_if (CONFIG_VC1DSP vc1dsp_neon.S)
add_sources_if (CONFIG_VP8DSP vp8dsp_neon.S)

# decoders/encoders
add_sources_if (CONFIG_AAC_DECODER aacpsdsp_neon.S)
add_sources_if (CONFIG_DCA_DECODER dcadsp_neon.S synth_filter_neon.S)
add_sources_if (CONFIG_OPUS_DECODER opusdsp_neon.S)
add_sources_if (CONFIG_VORBIS_DECODER vorbisdsp_neon.S)
add_sources_if (CONFIG_VP9_DECODER vp9itxfm_16bpp_neon.S
                                        vp9itxfm_neon.S
                                        vp9lpf_16bpp_neon.S
                                        vp9lpf_neon.S
                                        vp9mc_16bpp_neon.S
                                        vp9mc_neon.S)
add_sources_if (CONFIG_HEVC_DECODER hevcdsp_deblock_neon.S
                                        hevcdsp_dequant_neon.S
                                        hevcdsp_idct_neon.S
                                        hevcdsp_init_aarch64.S
                                        hevcpred_neon.S
                                        hevcpred_init_aarch64.S
                                        h26x/epel_neon.S
                                        h26x/qpel_neon.S
                                        h26x/sao_neon.S)

endif ()
