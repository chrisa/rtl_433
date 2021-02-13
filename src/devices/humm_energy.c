/** @file
    Decoder for humm energy monitor

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "decoder.h"

/**
Decoder for humm energy monitor

Preamble:

    aa aa aa aa aa 2d d4
*/

static int humm_energy_decode(r_device *decoder, bitbuffer_t *bitbuffer)
{
    uint8_t const preamble_pattern[] = {0xaa, 0xaa, 0xaa, 0x2d, 0xd4};

    data_t *data;
    uint8_t msg[11];

    if (bitbuffer->num_rows != 1 || bitbuffer->bits_per_row[0] < 85) {
        if (decoder->verbose > 1)
            fprintf(stderr, "%s: too few bits (%u)\n", __func__, bitbuffer->bits_per_row[0]);
        return DECODE_ABORT_LENGTH; // unrecognized
    }

    unsigned start_pos = bitbuffer_search(bitbuffer, 0, 0,
            preamble_pattern, sizeof(preamble_pattern) * 8);
    start_pos += sizeof(preamble_pattern) * 8;

    if (start_pos >= bitbuffer->bits_per_row[0]) {
        if (decoder->verbose > 1)
            fprintf(stderr, "%s: preamble not found\n", __func__);
        return DECODE_ABORT_EARLY; // no preamble found
    }

    if (start_pos + 45 > bitbuffer->bits_per_row[0]) {
        if (decoder->verbose > 1)
            fprintf(stderr, "%s: message too short (%u)\n", __func__, bitbuffer->bits_per_row[0] - start_pos);
        return DECODE_ABORT_LENGTH; // message too short
    }

    bitbuffer_extract_bytes(bitbuffer, 0, start_pos, msg, sizeof (msg) * 8);
    if (decoder->verbose > 1) {
        decoder_log_bitrow(decoder, 0, __func__, msg, sizeof(msg) * 8, "%s: MSG: ");
    }

    if (msg[0] != 0x13 && msg[1] != 0xa7) {
            fprintf(stderr, "failed sanity check\n");
        return DECODE_FAIL_SANITY;
    }

    int power = (msg[2] & 0x01 ? 2560 : 0) + (msg[3] * 10) + (msg[4] & 0x40 ? 5 : 0);

    /* clang-format off */
    data = data_make(
            "model",            "",             DATA_STRING, "humm energy monitor",
            "power",            "",             DATA_INT,    power,
            NULL);
    /* clang-format on */

    decoder_output_data(decoder, data);
    return 1;
}

static const char *output_fields[] = {
        "power",
        NULL,
};

r_device humm_energy = {
        .name        = "humm energy monitor",
        .modulation  = FSK_PULSE_PCM,
        .short_width = 25,
        .long_width  = 25,
        .reset_limit = 9000,
        .decode_fn   = &humm_energy_decode,
        .disabled    = 0,
        .fields      = output_fields,
};
