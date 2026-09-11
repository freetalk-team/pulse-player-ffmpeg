
add_sources_if (CONFIG_OPUS_DECODER
    dec.c
    dec_celt.c
    celt.c
    frame_duration_tab.c
    pvq.c
    silk.c
    tab.c
    dsp.c
    parse.c
    rc.c
)

add_sources_if (CONFIG_OPUS_PARSER
    frame_duration_tab.c
    parser.c
    parse.c
)

add_sources_if (CONFIG_OPUS_ENCODER
    enc.c
    enc_psy.c
    celt.c
    pvq.c
    rc.c
    tab.c
)

add_sources_if (CONFIG_MATROSKA_MUXER frame_duration_tab.c)
add_sources_if (CONFIG_WEBM_MUXER frame_duration_tab.c)
