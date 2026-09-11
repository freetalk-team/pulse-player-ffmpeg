
add_sources_if (CONFIG_APV_VULKAN_HWACCEL vulkan/apv_decode.comp.spv.c
                                     vulkan/apv_idct.comp.spv.c)

add_sources_if (CONFIG_FFV1_VULKAN_ENCODER vulkan/ffv1_enc_setup.comp.spv.c
                                      vulkan/ffv1_enc_reset.comp.spv.c
                                      vulkan/ffv1_enc_reset_golomb.comp.spv.c
                                      vulkan/ffv1_enc.comp.spv.c
                                      vulkan/ffv1_enc_golomb.comp.spv.c
                                      vulkan/ffv1_enc_rgb.comp.spv.c
                                      vulkan/ffv1_enc_rgb_golomb.comp.spv.c
                                      vulkan/ffv1_enc_rct_search.comp.spv.c
                                      vulkan/ffv1_enc_remap.comp.spv.c
                                      vulkan/ffv1_enc_rgb_float.comp.spv.c
                                      vulkan/ffv1_enc_rgb_float_golomb.comp.spv.c
                                      vulkan/ffv1_enc_sort32.comp.spv.c
                                      vulkan/ffv1_enc_bayer.comp.spv.c
                                      vulkan/ffv1_enc_bayer_golomb.comp.spv.c)

add_sources_if (CONFIG_FFV1_VULKAN_HWACCEL vulkan/ffv1_dec_setup.comp.spv.c
                                      vulkan/ffv1_dec_reset.comp.spv.c
                                      vulkan/ffv1_dec_reset_golomb.comp.spv.c
                                      vulkan/ffv1_dec.comp.spv.c
                                      vulkan/ffv1_dec_golomb.comp.spv.c
                                      vulkan/ffv1_dec_rgb.comp.spv.c
                                      vulkan/ffv1_dec_rgb_golomb.comp.spv.c
                                      vulkan/ffv1_dec_rgb_float.comp.spv.c
                                      vulkan/ffv1_dec_rgb_float_golomb.comp.spv.c
                                      vulkan/ffv1_dec_bayer.comp.spv.c
                                      vulkan/ffv1_dec_bayer_golomb.comp.spv.c)

add_sources_if (CONFIG_PRORES_KS_VULKAN_ENCODER vulkan/prores_ks_alpha_data.comp.spv.c
                                           vulkan/prores_ks_slice_data.comp.spv.c
                                           vulkan/prores_ks_estimate_slice.comp.spv.c
                                           vulkan/prores_ks_encode_slice.comp.spv.c
                                           vulkan/prores_ks_trellis_node.comp.spv.c)

add_sources_if (CONFIG_PRORES_RAW_VULKAN_HWACCEL vulkan/prores_raw_decode.comp.spv.c
                                            vulkan/prores_raw_idct.comp.spv.c)

add_sources_if (CONFIG_PRORES_VULKAN_HWACCEL vulkan/prores_vld.comp.spv.c
                                        vulkan/prores_idct.comp.spv.c)

add_sources_if (CONFIG_DPX_VULKAN_HWACCEL vulkan/dpx_unpack.comp.spv.c
                                     vulkan/dpx_copy.comp.spv.c)
