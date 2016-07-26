/**
University of Illinois/NCSA
Open Source License

Copyright (c) 2009-2014 by the contributors listed in CREDITS.TXT

All rights reserved.

Developed by:

    LLVM Team

    University of Illinois at Urbana-Champaign

    http://llvm.org

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal with
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimers.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimers in the
      documentation and/or other materials provided with the distribution.

    * Neither the names of the LLVM Team, University of Illinois at
      Urbana-Champaign, nor the names of its contributors may be used to
      endorse or promote products derived from this Software without specific
      prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
SOFTWARE.
**/

#include "int_lib.h"

ARM_EABI_FNALIAS(ul2f, floatundisf)

COMPILER_RT_ABI float
__floatundisf(du_int a)
{
    if (a == 0)
        return 0.0F;
    const unsigned N = sizeof(du_int) * CHAR_BIT;
    int sd = N - __builtin_clzll(a);  /* number of significant digits */
    int e = sd - 1;             /* 8 exponent */
    if (sd > FLT_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                                12345678901234567890123456
         *  1 = msb 1 bit
         *  P = bit FLT_MANT_DIG-1 bits to the right of 1
         *  Q = bit FLT_MANT_DIG bits to the right of 1
         *  R = "or" of all bits to the right of Q
         */
        switch (sd)
        {
        case FLT_MANT_DIG + 1:
            a <<= 1;
            break;
        case FLT_MANT_DIG + 2:
            break;
        default:
            a = (a >> (sd - (FLT_MANT_DIG+2))) |
                ((a & ((du_int)(-1) >> ((N + FLT_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to FLT_MANT_DIG or FLT_MANT_DIG+1 bits */
        if (a & ((du_int)1 << FLT_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    else
    {
        a <<= (FLT_MANT_DIG - sd);
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    float_bits fb;
    fb.u = ((e + 127) << 23)       |  /* exponent */
           ((su_int)a & 0x007FFFFF);  /* mantissa */
    return fb.f;
}
