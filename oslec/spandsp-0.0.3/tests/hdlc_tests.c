/*
 * SpanDSP - a series of DSP components for telephony
 *
 * hdlc_tests.c
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2003 Steve Underwood
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
 * $Id: hdlc_tests.c,v 1.30 2006/11/19 14:07:27 steveu Exp $
 */

/*! \file */

/*! \page hdlc_tests_page HDLC tests
\section hdlc_tests_page_sec_1 What does it do?
The HDLC tests exercise the HDLC module, and verifies correct operation
using both 16 and 32 bit CRCs.
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

#include "spandsp.h"

int ref_len;
uint8_t buf[1000];

int frame_handled;
int frame_len_errors;
int frame_data_errors;
int underflow_reported;

hdlc_rx_state_t rx;
hdlc_tx_state_t tx;

int frames_sent;
int bytes_sent;
uint64_t start;
uint64_t end;

/* Use a local random generator, so the results are consistent across platforms. We have hard coded
   correct results for a message sequence generated by this particular PRNG. */
static int my_rand(void)
{
    static int rndnum = 1234567;

    return (rndnum = 1664525U*rndnum + 1013904223U) >> 8;
}
/*- End of function --------------------------------------------------------*/

static int cook_up_msg(uint8_t *buf)
{
    int i;
    int len;

    /* Use medium length messages, with some randomised length variation. */
    /* TODO: this doesn't exercise extremely short or long messages. */
    len = (my_rand() & 0x3F) + 100;
    for (i = 0;  i < len;  i++)
        buf[i] = my_rand();
    return len;
}
/*- End of function --------------------------------------------------------*/

static void frame_handler(void *user_data, int ok, const uint8_t *pkt, int len)
{
    if (len < 0)
    {
        /* Special conditions */
        switch (len)
        {
        case PUTBIT_TRAINING_FAILED:
            printf("Training failed\n");
            break;
        case PUTBIT_TRAINING_SUCCEEDED:
            printf("Training succeeded\n");
            break;
        case PUTBIT_CARRIER_UP:
            printf("Carrier up\n");
            break;
        case PUTBIT_CARRIER_DOWN:
            printf("Carrier down\n");
            break;
        case PUTBIT_FRAMING_OK:
            //printf("Framing OK\n");
            break;
        case PUTBIT_ABORT:
            printf("Abort\n");
            break;
        default:
            printf("Eh!\n");
            break;
        }
        return;
    }
    if (len != ref_len)
    {
        printf("Len error - %d %d\n", len, ref_len);
        frame_len_errors++;
        return;
    }
    if (memcmp(pkt, buf, len))
    {
        printf("Frame data error\n");
        frame_data_errors++;
        return;
    }
    frame_handled = TRUE;
}
/*- End of function --------------------------------------------------------*/

static void underflow_handler(void *user_data)
{
    //printf("Underflow reported\n");
    underflow_reported = TRUE;
}
/*- End of function --------------------------------------------------------*/

static void check_result(void)
{
    hdlc_rx_stats_t rx_stats;

    hdlc_rx_get_stats(&rx, &rx_stats);
    printf("%lu bytes\n", rx_stats.bytes);
    printf("%lu good frames\n", rx_stats.good_frames);
    printf("%lu CRC errors\n", rx_stats.crc_errors);
    printf("%lu length errors\n", rx_stats.length_errors);
    printf("%lu aborts\n", rx_stats.aborts);
    printf("%d frame length errors\n", frame_len_errors);
    printf("%d frame data errors\n", frame_data_errors);
    printf("Test duration %" PRIu64 " ticks\n", end - start);
    if (rx_stats.bytes != bytes_sent
        ||
        rx_stats.good_frames != frames_sent
        ||
        rx_stats.crc_errors != 0
        ||
        rx_stats.length_errors != 0
        ||
        rx_stats.aborts != 0
        ||
        frame_len_errors != 0
        ||
        frame_data_errors != 0)
    {
        printf("Tests failed.\n");
        exit(2);
    }
    printf("Test passed.\n\n");
}
/*- End of function --------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int i;
    int j;
    int len;
    int nextbyte;
    int progress;
    int progress_delay;

    printf("HDLC module tests\n");

    /* Try a few random messages through the CRC logic. */
    printf("Testing the CRC-16 routines\n");
    for (i = 0;  i < 100;  i++)
    {
        ref_len = cook_up_msg(buf);
        len = crc_itu16_append(buf, ref_len);
        if (!crc_itu16_check(buf, len))
        {
            printf("CRC-16 failure\n");
            exit(2);
        }
    }
    printf("Test passed.\n\n");
    
    printf("Testing the CRC-32 routines\n");
    for (i = 0;  i < 100;  i++)
    {
        ref_len = cook_up_msg(buf);
        len = crc_itu32_append(buf, ref_len);
        if (!crc_itu32_check(buf, len))
        {
            printf("CRC-32 failure\n");
            exit(2);
        }
    }
    printf("Test passed.\n\n");

    /* Now try sending HDLC messages with CRC-16 */
    printf("Testing with CRC-16 (byte by byte)\n");
    frame_len_errors = 0;
    frame_data_errors = 0;
    hdlc_tx_init(&tx, FALSE, 1, FALSE, underflow_handler, NULL);
    hdlc_rx_init(&rx, FALSE, FALSE, 5, frame_handler, NULL);
    underflow_reported = FALSE;

    start = rdtscll();
    hdlc_tx_preamble(&tx, 40);
    /* Push an initial message so we should NOT get an underflow after the preamble. */
    ref_len = cook_up_msg(buf);
    hdlc_tx_frame(&tx, buf, ref_len);
    frame_handled = FALSE;
    frames_sent = 0;
    bytes_sent = 0;
    for (i = 0;  i < 1000000;  i++)
    {
        nextbyte = hdlc_tx_get_byte(&tx);
        hdlc_rx_put_byte(&rx, nextbyte);
        if (underflow_reported)
        {
            underflow_reported = FALSE;
            nextbyte = hdlc_tx_get_byte(&tx);
            hdlc_rx_put_byte(&rx, nextbyte);
            frames_sent++;
            bytes_sent += ref_len;
            if (!frame_handled)
            {
                printf("Frame not received.\n");
                printf("Tests failed.\n");
                exit(2);
            }
            ref_len = cook_up_msg(buf);
            hdlc_tx_frame(&tx, buf, ref_len);
            frame_handled = FALSE;
        }
    }
    end = rdtscll();
    check_result();

    /* Now try sending HDLC messages with CRC-16 bit by bit */
    printf("Testing with CRC-16 (bit by bit)\n");
    frame_len_errors = 0;
    frame_data_errors = 0;
    hdlc_tx_init(&tx, FALSE, 2, FALSE, underflow_handler, NULL);
    hdlc_rx_init(&rx, FALSE, FALSE, 5, frame_handler, NULL);
    underflow_reported = FALSE;

    start = rdtscll();
    hdlc_tx_preamble(&tx, 40);
    /* Don't push an initial message so we should get an underflow after the preamble. */
    /* Lie for the first message, as there isn't really one */
    frame_handled = TRUE;
    frames_sent = 0;
    bytes_sent = 0;
    ref_len = 0;
    for (i = 0;  i < 8*1000000;  i++)
    {
        nextbyte = hdlc_tx_get_bit(&tx);
        hdlc_rx_put_bit(&rx, nextbyte);
        if (underflow_reported)
        {
            underflow_reported = FALSE;
            for (j = 0;  j < 20;  j++)
            {
                nextbyte = hdlc_tx_get_bit(&tx);
                hdlc_rx_put_bit(&rx, nextbyte);
            }
            if (ref_len)
            {
                frames_sent++;
                bytes_sent += ref_len;
            }
            if (!frame_handled)
            {
                printf("Frame not received.\n");
                printf("Tests failed.\n");
                exit(2);
            }
            ref_len = cook_up_msg(buf);
            hdlc_tx_frame(&tx, buf, ref_len);
            frame_handled = FALSE;
        }
    }
    end = rdtscll();
    check_result();

    /* Now try sending HDLC messages with CRC-32 */
    printf("Testing with CRC-32 (byte by byte)\n");
    frame_len_errors = 0;
    frame_data_errors = 0;
    hdlc_tx_init(&tx, TRUE, 1, FALSE, underflow_handler, NULL);
    hdlc_rx_init(&rx, TRUE, FALSE, 1, frame_handler, NULL);
    underflow_reported = FALSE;

    start = rdtscll();
    hdlc_tx_preamble(&tx, 40);
    /* Don't push an initial message so we should get an underflow after the preamble. */
    /* Lie for the first message, as there isn't really one */
    frame_handled = TRUE;
    frames_sent = 0;
    bytes_sent = 0;
    ref_len = 0;
    for (i = 0;  i < 1000000;  i++)
    {
        nextbyte = hdlc_tx_get_byte(&tx);
        hdlc_rx_put_byte(&rx, nextbyte);
        if (underflow_reported)
        {
            underflow_reported = FALSE;
            nextbyte = hdlc_tx_get_byte(&tx);
            hdlc_rx_put_byte(&rx, nextbyte);
            if (ref_len)
            {
                frames_sent++;
                bytes_sent += ref_len;
            }
            if (!frame_handled)
            {
                printf("Frame not received.\n");
                printf("Tests failed.\n");
                exit(2);
            }
            ref_len = cook_up_msg(buf);
            hdlc_tx_frame(&tx, buf, ref_len);
            frame_handled = FALSE;
        }
    }
    end = rdtscll();
    check_result();

    /* Now try progressive mode with CRC-16 */
    printf("Testing progressive mode with CRC-16 (byte by byte)\n");
    frame_len_errors = 0;
    frame_data_errors = 0;
    hdlc_tx_init(&tx, TRUE, 1, TRUE, underflow_handler, NULL);
    hdlc_rx_init(&rx, TRUE, FALSE, 1, frame_handler, NULL);
    underflow_reported = FALSE;

    start = rdtscll();
    hdlc_tx_preamble(&tx, 40);
    /* Don't push an initial message so we should get an underflow after the preamble. */
    /* Lie for the first message, as there isn't really one */
    frame_handled = TRUE;
    progress = 9999;
    progress_delay = 9999;
    frames_sent = 0;
    bytes_sent = 0;
    ref_len = 0;
    for (i = 0;  i < 1000000;  i++)
    {
        nextbyte = hdlc_tx_get_byte(&tx);
        hdlc_rx_put_byte(&rx, nextbyte);
        if (underflow_reported)
        {
            underflow_reported = FALSE;
            nextbyte = hdlc_tx_get_byte(&tx);
            hdlc_rx_put_byte(&rx, nextbyte);
            if (ref_len)
            {
                frames_sent++;
                bytes_sent += ref_len;
            }
            if (!frame_handled)
            {
                printf("Frame not received.\n");
                printf("Tests failed.\n");
                exit(2);
            }
            ref_len = cook_up_msg(buf);
            hdlc_tx_frame(&tx, buf, 10);
            progress = 10;
            progress_delay = 8;
            frame_handled = FALSE;
        }
        if (progress < ref_len  &&  progress_delay-- <= 0)
        {
            if (hdlc_tx_frame(&tx, buf + progress, (progress + 10 <= ref_len)  ?  10  :  ref_len - progress) < 0)
            {
                printf("Failed to add progressively\n");
                exit(2);
            }
            progress += 10;
            progress_delay = 8;
        }
    }
    end = rdtscll();
    check_result();

    printf("Tests passed.\n");
    return  0;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
