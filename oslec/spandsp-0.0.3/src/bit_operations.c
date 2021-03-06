/*
 * SpanDSP - a series of DSP components for telephony
 *
 * bit_operations.c - Various bit level operations, such as bit reversal
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2006 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: bit_operations.c,v 1.5 2006/11/30 15:41:47 steveu Exp $
 */

/*! \file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#include "spandsp/telephony.h"
#include "spandsp/bit_operations.h"

uint16_t bit_reverse16(uint16_t x)
{
    x = (x >> 8) | (x << 8);
    x = ((x & 0xF0F0) >> 4) | ((x & 0x0F0F) << 4);
    x = ((x & 0xCCCC) >> 2) | ((x & 0x3333) << 2);
    return ((x & 0xAAAA) >> 1) | ((x & 0x5555) << 1);
}
/*- End of function --------------------------------------------------------*/

uint32_t bit_reverse32(uint32_t x)
{
    x = (x >> 16) | (x << 16);
    x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
    x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
    x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
    return ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
}
/*- End of function --------------------------------------------------------*/

uint32_t bit_reverse_4bytes(uint32_t x)
{
    x = (x >> 4) | (x << 4);
    x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
    return ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
}
/*- End of function --------------------------------------------------------*/

int one_bits32(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555);
    /* We now have 16 2-bit counts */
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    /* We now have 8 4-bit counts */
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    /* We now have 4 8-bit counts */
#if defined(__i386__)  ||  defined(__x86_64__)
    /* If multiply is fast */
    return (x*0x01010101) >> 24;
#else
    /* If multiply is slow */
    x += (x >> 8);
    x += (x >> 16);
    return (x & 0x0000003F);
#endif
}
/*- End of function --------------------------------------------------------*/

uint32_t make_mask32(uint32_t x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x;
}
/*- End of function --------------------------------------------------------*/

uint16_t make_mask16(uint16_t x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    return x;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
