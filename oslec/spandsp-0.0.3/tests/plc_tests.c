/*
 * SpanDSP - a series of DSP components for telephony
 *
 * plc_tests.c
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2004 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
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
 * $Id: plc_tests.c,v 1.18 2006/11/19 14:07:27 steveu Exp $
 */

/*! \page plc_tests_page Packet loss concealment tests
\section plc_tests_page_sec_1 What does it do?
These tests run a speech file through the packet loss concealment routines.
The loss rate, in percent, and the packet size, in samples, may be specified
on the command line.

\section plc_tests_page_sec_2 How are the tests run?
These tests process a speech file called pre_plc.wav. This file should contain
8000 sample/second 16 bits/sample linear audio. The tests read this file in
blocks, of a size specified on the command line. Some of these blocks are
dropped, to simulate packet loss. The rate of loss is also specified on the
command line. The PLC module is then used to reconstruct an acceptable
approximation to the original signal. The resulting audio is written to a new
audio file, called post_plc.wav. This file contains 8000 sample/second
16 bits/sample linear audio.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_TGMATH_H)
#include <tgmath.h>
#endif
#if defined(HAVE_MATH_H)
#include <math.h>
#endif
#include <tiffio.h>

#include <audiofile.h>

#include "spandsp.h"

#define INPUT_FILE_NAME     "../localtests/short_nb_voice.wav"
#define OUTPUT_FILE_NAME    "post_plc.wav"

int main(int argc, char *argv[])
{
    AFfilehandle inhandle;
    AFfilehandle outhandle;
    AFfilesetup filesetup;
    plc_state_t plc;
    int inframes;
    int outframes;
    int16_t amp[1024];
    int block_no;
    int lost_blocks;
    int block_len;
    int loss_rate;
    int dropit;
    int block_real;
    int block_synthetic;
    int tone;
    int i;
    uint32_t phase_acc;
    int32_t phase_rate;

    loss_rate = 25;
    block_len = 160;
    block_real = FALSE;
    block_synthetic = FALSE;
    tone = -1;
    for (i = 1;  i < argc;  i++)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            loss_rate = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-b") == 0)
        {
            block_len = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-t") == 0)
        {
            tone = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-r") == 0)
            block_real = TRUE;
        if (strcmp(argv[i], "-s") == 0)
            block_synthetic = TRUE;
    }
    if ((filesetup = afNewFileSetup()) == AF_NULL_FILESETUP)
    {
        fprintf(stderr, "    Failed to create file setup\n");
        exit(2);
    }
    afInitSampleFormat(filesetup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
    afInitRate(filesetup, AF_DEFAULT_TRACK, (float) SAMPLE_RATE);
    afInitFileFormat(filesetup, AF_FILE_WAVE);
    afInitChannels(filesetup, AF_DEFAULT_TRACK, 1);

    phase_rate = 0;
    inhandle = NULL;
    if (tone < 0)
    {
        if ((inhandle = afOpenFile(INPUT_FILE_NAME, "r", NULL)) == AF_NULL_FILEHANDLE)
        {
            fprintf(stderr, "    Failed to open wave file '%s'\n", INPUT_FILE_NAME);
            exit(2);
        }
    }
    else
    {
        phase_rate = dds_phase_ratef((float) tone);
    }
    if ((outhandle = afOpenFile(OUTPUT_FILE_NAME, "w", filesetup)) == AF_NULL_FILEHANDLE)
    {
        fprintf(stderr, "    Failed to open wave file '%s'\n", OUTPUT_FILE_NAME);
        exit(2);
    }
    plc_init(&plc);
    lost_blocks = 0;
    for (block_no = 0;  ;  block_no++)
    {
        if (tone < 0)
        {
            inframes = afReadFrames(inhandle,
                                    AF_DEFAULT_TRACK,
                                    amp,
                                    block_len);
            if (inframes != block_len)
                break;
        }
        else
        {
            if (block_no > 10000)
                break;
            for (i = 0;  i < block_len;  i++)
                amp[i] = (int16_t) dds_modf(&phase_acc, phase_rate, 10000.0, 0);
            inframes = block_len;
        }
        dropit = rand()/(RAND_MAX/100);
        if (dropit > loss_rate)
        {
            plc_rx(&plc, amp, inframes);
            if (block_real)
                memset(amp, 0, sizeof(int16_t)*inframes);
        }
        else
        {
            lost_blocks++;
            plc_fillin(&plc, amp, inframes);
            if (block_synthetic)
                memset(amp, 0, sizeof(int16_t)*inframes);
        }
        outframes = afWriteFrames(outhandle,
                                  AF_DEFAULT_TRACK,
                                  amp,
                                  inframes);
        if (outframes != inframes)
        {
            fprintf(stderr, "    Error writing out sound\n");
            exit(2);
        }
    }
    printf("Dropped %d of %d blocks\n", lost_blocks, block_no);
    if (tone < 0)
    {
        if (afCloseFile(inhandle) != 0)
        {
            fprintf(stderr, "    Cannot close wave file '%s'\n", INPUT_FILE_NAME);
            exit(2);
        }
    }
    if (afCloseFile(outhandle) != 0)
    {
        fprintf(stderr, "    Cannot close wave file '%s'\n", OUTPUT_FILE_NAME);
        exit(2);
    }
    afFreeFileSetup(filesetup);
    return 0;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
