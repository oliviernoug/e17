/*
cdecode.h - c header for a base64 decoding algorithm

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#ifndef BASE64_CDECODE_H
#define BASE64_CDECODE_H

typedef enum
{
        step_a, step_b, step_c, step_d
} base64_decodestep;

typedef struct
{
        base64_decodestep step;
        unsigned char plainchar;
} base64_decodestate;

void base64_init_decodestate(base64_decodestate* state_in);

int base64_decode_value(char value_in);

int base64_decode_block(const char* code_in, const int length_in, unsigned char* plaintext_out, base64_decodestate* state_in);

#endif /* BASE64_CDECODE_H */
