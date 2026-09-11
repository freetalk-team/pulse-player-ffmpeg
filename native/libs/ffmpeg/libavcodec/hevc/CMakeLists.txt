
add_sources_if (CONFIG_HEVC_DECODER
    hevc/cabac.c.c
    hevc/data.c.c
    hevc/dsp.c.c
    hevc/filter.c.c
    hevc/hevcdec.c.c
    hevc/mvs.c.c
    hevc/pred.c.c
    hevc/refs.c.c
)

add_sources_if (CONFIG_HEVC_PARSER
    hevc/parser.c.c
)


add_sources_if (CONFIG_HEVCPARSE
    hevc/data.c.c
    hevc/parse.c.c
    hevc/ps.c.c
)


add_sources_if (CONFIG_HEVC_SEI
    hevc/sei.c.c
)
