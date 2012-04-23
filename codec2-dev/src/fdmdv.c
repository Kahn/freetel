/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: fdmdv.c
  AUTHOR......: David Rowe
  DATE CREATED: April 14 2012
                                                                             
  Functions that implement a Frequency Divison Multiplexed Modem for
  Digital Voice (FDMDV) over HF channels.
                                                                             
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2012 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

/*---------------------------------------------------------------------------*\
                                                                             
                               INCLUDES

\*---------------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "fdmdv_internal.h"
#include "fdmdv.h"
#include "rn.h"
#include "test_bits.h"
#include "pilot_coeff.h"
#include "fft.h"
#include "hanning.h"

/*---------------------------------------------------------------------------*\
                                                                             
                               FUNCTIONS

\*---------------------------------------------------------------------------*/

static COMP cneg(COMP a)
{
    COMP res;

    res.real = -a.real;
    res.imag = -a.imag;

    return res;
}

static COMP cconj(COMP a)
{
    COMP res;

    res.real = a.real;
    res.imag = -a.imag;

    return res;
}

static COMP cmult(COMP a, COMP b)
{
    COMP res;

    res.real = a.real*b.real - a.imag*b.imag;
    res.imag = a.real*b.imag + a.imag*b.real;

    return res;
}

static COMP fcmult(float a, COMP b)
{
    COMP res;

    res.real = a*b.real;
    res.imag = a*b.imag;

    return res;
}

static COMP cadd(COMP a, COMP b)
{
    COMP res;

    res.real = a.real + b.real;
    res.imag = a.imag + b.imag;

    return res;
}

static COMP cdot(COMP a[], COMP b[], int n)
{
    COMP res;
    int  i;
    
    for(i=0; i<n; i++) 
	res = cadd(res, cmult(a[i], b[i]));

    return res;
}

static void cbuf_shift_update(COMP buf[], COMP update[], int buflen, int updatelen)
{
    int  i,j;
    
    for(i=0; i<buflen-updatelen; i++) 
	buf[i] = buf[updatelen+i];
    for(j=0; j<updatelen; j++) 
	buf[i] = update[j];
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: fdmdv_create	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 16/4/2012 

  Create and initialise an instance of the modem.  Returns a pointer
  to the modem states or NULL on failure.  One set of states is
  sufficient for a full duuplex modem.

\*---------------------------------------------------------------------------*/

struct FDMDV *fdmdv_create(void)
{
    struct FDMDV *f;
    int           c, i, k;
    float         carrier_freq;

    assert(FDMDV_BITS_PER_FRAME == NC*NB);
    assert(FDMDV_SAMPLES_PER_FRAME == M);

    f = (struct FDMDV*)malloc(sizeof(struct FDMDV));
    if (f == NULL)
	return NULL;
    
    f->current_test_bit = 0;
    f->tx_pilot_bit = 0;
    for(c=0; c<NC+1; c++) {
	f->prev_tx_symbols[c].real = 1.0;
	f->prev_tx_symbols[c].imag = 0.0;
	for(k=0; k<NFILTER; k++) {
	    f->tx_filter_memory[c][k].real = 0.0;
	    f->tx_filter_memory[c][k].imag = 0.0;
	    f->rx_filter_memory[c][k].real = 0.0;
	    f->rx_filter_memory[c][k].imag = 0.0;
	}

	/* Spread initial FDM carrier phase out as far as possible.
           This helped PAPR for a few dB.  We don't need to adjust rx
           phase as DQPSK takes care of that. */
	
	f->phase_tx[c].real = cos(2.0*PI*c/(NC+1));
 	f->phase_tx[c].imag = sin(2.0*PI*c/(NC+1));

	f->phase_rx[c].real = 1.0;
 	f->phase_rx[c].imag = 0.0;

	for(k=0; k<NT*P; k++) {
	    f->rx_filter_mem_timing[c][k].real = 0.0;
	    f->rx_filter_mem_timing[c][k].imag = 0.0;
	}
 	for(k=0; k<NFILTERTIMING; k++) {
	    f->rx_baseband_mem_timing[c][k].real = 0.0;
	    f->rx_baseband_mem_timing[c][k].imag = 0.0;
	}
  }
    
    /* Set up frequency of each carrier */

    for(c=0; c<NC/2; c++) {
	carrier_freq = (-NC/2 + c)*FSEP + FCENTRE;
	f->freq[c].real = cos(2.0*PI*carrier_freq/FS);
 	f->freq[c].imag = sin(2.0*PI*carrier_freq/FS);
    }

    for(c=NC/2; c<NC; c++) {
	carrier_freq = (-NC/2 + c + 1)*FSEP + FCENTRE;
	f->freq[c].real = cos(2.0*PI*carrier_freq/FS);
 	f->freq[c].imag = sin(2.0*PI*carrier_freq/FS);
    }
	
    f->freq[NC].real = cos(2.0*PI*FCENTRE/FS);
    f->freq[NC].imag = sin(2.0*PI*FCENTRE/FS);

    /* Generate DBPSK pilot Look Up Table (LUT) */

    generate_pilot_lut(f->pilot_lut, &f->freq[NC]);

    /* Freq Offset estimation */

    for(i=0; i<NPILOTBASEBAND; i++) {
	f->pilot_baseband1[i].real = f->pilot_baseband2[i].real = 0.0;
	f->pilot_baseband1[i].imag = f->pilot_baseband2[i].imag = 0.0;
    }
    f->pilot_lut_index = 0;
    f->prev_pilot_lut_index = 3*M;
    
    for(i=0; i<NPILOTLPF; i++) {
	f->pilot_lpf1[i].real = f->pilot_lpf2[i].real = 0.0;
	f->pilot_lpf1[i].imag = f->pilot_lpf2[i].imag = 0.0;
    }

    return f;
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: fdmdv_destroy	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 16/4/2012

  Destroy an instance of the modem.

\*---------------------------------------------------------------------------*/

void codec2_destroy(struct FDMDV *fdmdv)
{
    assert(fdmdv != NULL);
    free(fdmdv);
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: fdmdv_get_test_bits()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 16/4/2012

  Generate a frame of bits from a repeating sequence of random data.  OK so
  it's not very random if it repeats but it makes syncing at the demod easier
  for test purposes.

\*---------------------------------------------------------------------------*/

void fdmdv_get_test_bits(struct FDMDV *f, int tx_bits[])
{
    int i;

    for(i=0; i<FDMDV_BITS_PER_FRAME; i++) {
	tx_bits[i] = test_bits[f->current_test_bit];
	f->current_test_bit++;
	if (f->current_test_bit > (NTEST_BITS-1))
	    f->current_test_bit = 0;
    }
 }

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: bits_to_dqpsk_symbols()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 16/4/2012

  Generate Nc+1 QPSK symbols from vector of (1,Nc*Nb) input tx_bits.
  The Nc+1 symbol is the +1 -1 +1 .... BPSK sync carrier.

\*---------------------------------------------------------------------------*/

void bits_to_dqpsk_symbols(COMP tx_symbols[], COMP prev_tx_symbols[], int tx_bits[], int *pilot_bit)
{
    int c, msb, lsb;
    COMP j = {0.0,1.0};

    /* map tx_bits to to Nc DQPSK symbols */

    for(c=0; c<NC; c++) {
	msb = tx_bits[2*c]; 
	lsb = tx_bits[2*c+1];
	if ((msb == 0) && (lsb == 0))
	    tx_symbols[c] = prev_tx_symbols[c];
	if ((msb == 0) && (lsb == 1))
	    tx_symbols[c] = cmult(j, prev_tx_symbols[c]);
	if ((msb == 1) && (lsb == 0))
	    tx_symbols[c] = cneg(prev_tx_symbols[c]);
	if ((msb == 1) && (lsb == 1))
	    tx_symbols[c] = cmult(cneg(j),prev_tx_symbols[c]);
    }

    /* +1 -1 +1 -1 BPSK sync carrier, once filtered becomes (roughly)
       two spectral lines at +/- Rs/2 */
 
    if (*pilot_bit)
	tx_symbols[NC] = cneg(prev_tx_symbols[NC]);
    else
	tx_symbols[NC] = prev_tx_symbols[NC];

    if (*pilot_bit) 
	*pilot_bit = 0;
    else
	*pilot_bit = 1;
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: tx_filter()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 17/4/2012

  Given NC*NB bits construct M samples (1 symbol) of NC filtered
  symbols streams

\*---------------------------------------------------------------------------*/

void tx_filter(COMP tx_baseband[NC+1][M], COMP tx_symbols[], COMP tx_filter_memory[NC+1][NFILTER])
{
    int     c;
    int     i,j,k;
    float   acc;
    COMP    gain;

    gain.real = sqrt(2.0)/2.0;
    gain.imag = 0.0;

    for(c=0; c<NC+1; c++)
	tx_filter_memory[c][NFILTER-1] = cmult(tx_symbols[c], gain);

    /* 
       tx filter each symbol, generate M filtered output samples for each symbol.
       Efficient polyphase filter techniques used as tx_filter_memory is sparse
    */

    for(i=0; i<M; i++) {
	for(c=0; c<NC+1; c++) {

	    /* filter real sample of symbol for carrier c */

	    acc = 0.0;
	    for(j=M-1,k=M-i-1; j<NFILTER; j+=M,k+=M)
		acc += M * tx_filter_memory[c][j].real * gt_alpha5_root[k];
	    tx_baseband[c][i].real = acc;	

	    /* filter imag sample of symbol for carrier c */

	    acc = 0.0;
	    for(j=M-1,k=M-i-1; j<NFILTER; j+=M,k+=M)
		acc += M * tx_filter_memory[c][j].imag * gt_alpha5_root[k];
	    tx_baseband[c][i].imag = acc;

	}
    }

    /* shift memory, inserting zeros at end */

    for(i=0; i<NFILTER-M; i++)
	for(c=0; c<NC+1; c++)
	    tx_filter_memory[c][i] = tx_filter_memory[c][i+M];

    for(i=NFILTER-M; i<NFILTER; i++)
	for(c=0; c<NC+1; c++) {
	    tx_filter_memory[c][i].real = 0.0;
	    tx_filter_memory[c][i].imag = 0.0;
	}
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: fdm_upconvert()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 17/4/2012

  Construct FDM signal by frequency shifting each filtered symbol
  stream.  Returns complex signal so we can apply frequency offsets
  easily.

\*---------------------------------------------------------------------------*/

void fdm_upconvert(COMP tx_fdm[], COMP tx_baseband[NC+1][M], COMP phase_tx[], COMP freq[])
{
    int  i,c;
    COMP two = {2.0, 0.0};
    COMP pilot;

    for(i=0; i<M; i++) {
	tx_fdm[i].real = 0.0;
	tx_fdm[i].imag = 0.0;
    }

    /* Nc/2 tones below centre freq */
  
    for (c=0; c<NC/2; c++) 
	for (i=0; i<M; i++) {
	    phase_tx[c] = cmult(phase_tx[c], freq[c]);
	    tx_fdm[i] = cadd(tx_fdm[i], cmult(tx_baseband[c][i], phase_tx[c]));
	}

    /* Nc/2 tones above centre freq */

    for (c=NC/2; c<NC; c++) 
	for (i=0; i<M; i++) {
	    phase_tx[c] = cmult(phase_tx[c], freq[c]);
	    tx_fdm[i] = cadd(tx_fdm[i], cmult(tx_baseband[c][i], phase_tx[c]));
	}

    /* add centre pilot tone  */

    c = NC;
    for (i=0; i<M; i++) {
	phase_tx[c] = cmult(phase_tx[c],  freq[c]);
	pilot = cmult(cmult(two, tx_baseband[c][i]), phase_tx[c]);
	tx_fdm[i] = cadd(tx_fdm[i], pilot);
    }

    /*
      Scale such that total Carrier power C of real(tx_fdm) = Nc.  This
      excludes the power of the pilot tone.
      We return the complex (single sided) signal to make frequency
      shifting for the purpose of testing easier
    */

    for (i=0; i<M; i++) 
	tx_fdm[i] = cmult(two, tx_fdm[i]);

}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: generate_pilot_fdm()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 19/4/2012

  Generate M samples of DBPSK pilot signal for Freq offset estimation

\*---------------------------------------------------------------------------*/

void generate_pilot_fdm(COMP *pilot_fdm, int *bit, float *symbol, 
			float *filter_mem, COMP *phase, COMP *freq)
{
    int   i,j,k;
    float tx_baseband[M];

    /* +1 -1 +1 -1 DBPSK sync carrier, once filtered becomes (roughly)
       two spectral lines at +/- RS/2 */
 
    if (*bit)
	*symbol = -*symbol;
    else
	*symbol = *symbol;
    if (*bit) 
	*bit = 0;
    else
	*bit = 1;

    /* filter DPSK symbol to create M baseband samples */

    filter_mem[NFILTER-1] = (sqrt(2)/2) * *symbol;
    for(i=0; i<M; i++) {
	tx_baseband[i] = 0.0; 
	for(j=M-1,k=M-i-1; j<NFILTER; j+=M,k+=M)
	    tx_baseband[i] += M * filter_mem[j] * gt_alpha5_root[k];
    }

    /* shift memory, inserting zeros at end */

    for(i=0; i<NFILTER-M; i++)
	filter_mem[i] = filter_mem[i+M];

    for(i=NFILTER-M; i<NFILTER; i++)
	filter_mem[i] = 0.0;

    /* upconvert */

    for(i=0; i<M; i++) {
	*phase = cmult(*phase, *freq);
	pilot_fdm[i].real = sqrt(2)*2*tx_baseband[i] * phase->real;
	pilot_fdm[i].imag = sqrt(2)*2*tx_baseband[i] * phase->imag;
    }
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: generate_pilot_lut()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 19/4/2012

  Generate a 4M sample vector of DBPSK pilot signal.  As the pilot signal
  is periodic in 4M samples we can then use this vector as a look up table
  for pilot signal generation in the demod.

\*---------------------------------------------------------------------------*/

void generate_pilot_lut(COMP pilot_lut[], COMP *pilot_freq)
{
    int   pilot_rx_bit = 0;
    float pilot_symbol = sqrt(2.0);
    COMP  pilot_phase  = {1.0, 0.0};
    float pilot_filter_mem[NFILTER];
    COMP  pilot[M];
    int   i,f;

    for(i=0; i<NFILTER; i++)
	pilot_filter_mem[i] = 0.0;

    /* discard first 4 symbols as filter memory is filling, just keep
       last four symbols */

    for(f=0; f<8; f++) {
	generate_pilot_fdm(pilot, &pilot_rx_bit, &pilot_symbol, pilot_filter_mem, &pilot_phase, pilot_freq);
	if (f >= 4)
	    memcpy(&pilot_lut[M*(f-4)], pilot, M*sizeof(COMP));
    }

}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: lpf_peak_pick()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 20/4/2012

  LPF and peak pick part of freq est, put in a function as we call it twice.

\*---------------------------------------------------------------------------*/

void lpf_peak_pick(float *foff, float *max, COMP pilot_baseband[], COMP pilot_lpf[], COMP S[], int nin)
{
    int   i,j,k;
    int   mpilot;
    float mag, imax;
    int   ix;
    float r;

    /* LPF cutoff 200Hz, so we can handle max +/- 200 Hz freq offset */

    for(i=0; i<NPILOTLPF-nin; i++)
	pilot_lpf[i] = pilot_lpf[nin+i];
    for(i=NPILOTLPF-nin, j=0; i<NPILOTLPF; i++,j++) {
	pilot_lpf[i].real = 0.0; pilot_lpf[i].imag = 0.0;
	for(k=0; k<NPILOTCOEFF; k++)
	    pilot_lpf[i] = cadd(pilot_lpf[i], fcmult(pilot_coeff[k], pilot_baseband[j+k]));
    }

    /* decimate to improve DFT resolution, window and DFT */

    mpilot = FS/(2*200);  /* calc decimation rate given new sample rate is twice LPF freq */
    for(i=0; i<MPILOTFFT; i++) {
	S[i].real = 0.0; S[i].imag = 0.0;
    }
    for(i=0,j=0; i<NPILOTLPF; i+=mpilot,j++) {
	S[j] = fcmult(hanning[i], pilot_lpf[i]);
    }

    fft(&S[0].real, MPILOTFFT, -1);

    /* peak pick and convert to Hz */

    imax = 0.0;
    ix = 0;
    for(i=0; i<MPILOTFFT; i++) {
	mag = S[i].real*S[i].real + S[i].imag*S[i].imag;
	if (mag > imax) {
	    imax = mag;
	    ix = i;
	}
    }
    r = 2.0*200.0/MPILOTFFT;     /* maps FFT bin to frequency in Hz */
  
    if (ix >= MPILOTFFT/2)
	*foff = (ix - MPILOTFFT)*r;
    else
	*foff = (ix)*r;
    *max = imax;

}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: rx_est_freq_offset()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 19/4/2012

  Estimate frequency offset of FDM signal using BPSK pilot.  Note that
  this algorithm is quite sensitive to pilot tone level wrt other
  carriers, so test variations to the pilot amplitude carefully.

\*---------------------------------------------------------------------------*/

float rx_est_freq_offset(struct FDMDV *f, float rx_fdm[], int nin)
{
    int  i,j;
    COMP pilot[M+M/P];
    COMP prev_pilot[M+M/P];
    float foff, foff1, foff2;
    float   max1, max2;

    assert(nin <= M+M/P);

    /* get pilot samples used for correlation/down conversion of rx signal */

    for (i=0; i<nin; i++) {
	pilot[i] = f->pilot_lut[f->pilot_lut_index];
	f->pilot_lut_index++;
	if (f->pilot_lut_index >= 4*M)
	    f->pilot_lut_index = 0;
	
	prev_pilot[i] = f->pilot_lut[f->prev_pilot_lut_index];
	f->prev_pilot_lut_index++;
	if (f->prev_pilot_lut_index >= 4*M)
	    f->prev_pilot_lut_index = 0;
    }

    /*
      Down convert latest M samples of pilot by multiplying by ideal
      BPSK pilot signal we have generated locally.  The peak of the
      resulting signal is sensitive to the time shift between the
      received and local version of the pilot, so we do it twice at
      different time shifts and choose the maximum.
    */

    for(i=0; i<NPILOTBASEBAND-nin; i++) {
	f->pilot_baseband1[i] = f->pilot_baseband1[i+nin];
	f->pilot_baseband2[i] = f->pilot_baseband2[i+nin];
    }

    for(i=0,j=NPILOTBASEBAND-nin; i<nin; i++,j++) {
       	f->pilot_baseband1[j] = fcmult(rx_fdm[i], cconj(pilot[i]));
	f->pilot_baseband2[j] = fcmult(rx_fdm[i], cconj(prev_pilot[i]));
    }

    lpf_peak_pick(&foff1, &max1, f->pilot_baseband1, f->pilot_lpf1, f->S1, nin);
    lpf_peak_pick(&foff2, &max2, f->pilot_baseband2, f->pilot_lpf2, f->S2, nin);

    if (max1 > max2)
	foff = foff1;
    else
	foff = foff2;
	
    return foff;
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: fdm_downconvert()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22/4/2012

  Frequency shift each modem carrier down to Nc+1 baseband signals.

\*---------------------------------------------------------------------------*/

void fdm_downconvert(COMP rx_baseband[NC+1][M+M/P], COMP rx_fdm[], COMP phase_rx[], COMP freq[], int nin)
{
    int  i,c;
    COMP pilot;

    /* maximum number of input samples to demod */

    assert(nin < (M+M/P));

    /* Nc/2 tones below centre freq */
  
    for (c=0; c<NC/2; c++) 
	for (i=0; i<nin; i++) {
	    phase_rx[c] = cmult(phase_rx[c], freq[c]);
	    rx_baseband[c][i] = cmult(rx_fdm[i], cconj(phase_rx[c]));
	}

    /* Nc/2 tones above centre freq */

    for (c=NC/2; c<NC; c++) 
	for (i=0; i<M; i++) {
	    phase_rx[c] = cmult(phase_rx[c], freq[c]);
	    rx_baseband[c][i] = cmult(rx_fdm[i], cconj(phase_rx[c]));
	}

    /* add centre pilot tone  */

    c = NC;
    for (i=0; i<M; i++) {
	phase_rx[c] = cmult(phase_rx[c],  freq[c]);
	rx_baseband[c][i] = cmult(rx_fdm[i], cconj(phase_rx[c]));
    }

}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: rx_filter()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22/4/2012

  Receive filter each baseband signal at oversample rate P.  Filtering at
  rate P lowers CPU compared to rate M.

  Depending on the number of input samples to the demod nin, we
  produce P-1, P (usually), or P+1 filtered samples at rate P.  nin is
  occasionally adjusted to compensate for timing slips due to
  different tx and rx sample clocks.

\*---------------------------------------------------------------------------*/

void rx_filter(COMP rx_filt[NC+1][P+1], COMP rx_baseband[NC+1][M+M/P], COMP rx_filter_memory[NC+1][NFILTER], int nin)
{
    int c, i,j,k,l;
    int n=M/P;

    /* rx filter each symbol, generate P filtered output samples for
       each symbol.  Note we keep filter memory at rate M, it's just
       the filter output at rate P */

    for(i=0, j=0; i<nin; i+=n,j++) {

	/* latest input sample */
	
	for(c=0; c<NC+1; c++)
	    for(k=NFILTER-n,l=i; k<NFILTER; k++,l++)	
		rx_filter_memory[c][k] = rx_baseband[c][l];
	
	/* convolution (filtering) */

	for(c=0; c<NC+1; c++) {
	    rx_filt[c][j].real = 0.0; rx_filt[c][j].imag = 0.0;
	    for(k=0; k<NFILTER; k++) 
		rx_filt[c][j] = cadd(rx_filt[c][j], fcmult(gt_alpha5_root[k], rx_filter_memory[c][k]));
	}

	/* make room for next input sample */
	
	for(c=0; c<NC+1; c++)
	    for(k=0,l=n; k<NFILTER-n; k++,l++)	
		rx_filter_memory[c][k] = rx_filter_memory[c][l];
    }

    assert(j <= (P+1)); /* check for any over runs */
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: rx_est_timing()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 23/4/2012

  Estimate optimum timing offset, re-filter receive symbols at optimum
  timing estimate.

\*---------------------------------------------------------------------------*/

float rx_est_timing(COMP rx_symbols[], 
		    COMP rx_filt[NC+1][P+1], 
		    COMP rx_baseband[NC+1][M+M/P], 
		    COMP rx_filter_mem_timing[NC+1][NT*P], 
		    float env[],
		    COMP rx_baseband_mem_timing[NC+1][NFILTERTIMING], 
		    int nin)	 
{
    int   c,i,j,k;
    int   adjust, s;
    COMP  x, phase, freq;
    float rx_timing;

    /*
      nin  adjust 
      --------------------------------
      120  -1 (one less rate P sample)
      160   0 (nominal)
      200   1 (one more rate P sample)
    */

    adjust = P - nin*P/M;

    /* update buffer of NT rate P filtered symbols */
    
    for(c=0; c<NC+1; c++) 
	for(i=0,j=P-adjust; i<(NT-1)*P+adjust; i++,j++)
	    rx_filter_mem_timing[c][i] = rx_filter_mem_timing[c][j];
    for(c=0; c<NC+1; c++) 
	for(i=(NT-1)*P+adjust,j=0; i<NT*P; i++,j++)
	    rx_filter_mem_timing[c][i] = rx_filt[c][j];
	    
    /* sum envelopes of all carriers */

    for(i=0; i<NT*P; i++) {
	env[i] = 0.0;
	for(c=0; c<NC+1; c++)
	    env[i] += sqrt(pow(rx_filter_mem_timing[c][i].real,2.0) + pow(rx_filter_mem_timing[c][i].imag,2.0));
    }

    /* The envelope has a frequency component at the symbol rate.  The
       phase of this frequency component indicates the timing.  So work
       out single DFT at frequency 2*pi/P */

    x.real = 0.0; x.imag = 0.0;
    freq.real = cos(2*PI/P);
    freq.imag = sin(2*PI/P);
    phase.real = 1.0;
    phase.imag = 0.0;

    for(i=0; i<NT*P; i++) {
	x = cadd(x, fcmult(env[i], phase));
	phase = cmult(phase, freq);
    }

    /* Map phase to estimated optimum timing instant at rate M.  The
       M/4 part was adjusted by experiment, I know not why.... */
    
    rx_timing = atan2(x.imag, x.real)*M/(2*PI) + M/4;
    if (rx_timing > M)
	rx_timing -= M;
    if (rx_timing < -M)
	rx_timing += M;
   
    /* rx_filt_mem_timing contains M + Nfilter + M samples of the
       baseband signal at rate M this enables us to resample the
       filtered rx symbol with M sample precision once we have
       rx_timing */

    for(c=0; c<NC+1; c++) 
	for(i=0,j=nin; i<NFILTERTIMING-nin; i++,j++)
	    rx_baseband_mem_timing[c][i] = rx_filter_mem_timing[c][j];
    for(c=0; c<NC+1; c++) 
	for(i=NFILTERTIMING-nin,j=0; i<NFILTERTIMING; i++,j++)
	    rx_baseband_mem_timing[c][i] = rx_baseband[c][j];

    /* rx filter to get symbol for each carrier at estimated optimum
       timing instant.  We use rate M filter memory to get fine timing
       resolution. */

    s = round(rx_timing) + M;
    for(c=0; c<NC+1; c++) {
	rx_symbols[c].real = 0.0;
	rx_symbols[c].imag = 0.0;
	for(k=s,j=0; k<s+NFILTER; k++,j++)
	    rx_symbols[c] = cadd(rx_symbols[c], fcmult(gt_alpha5_root[j], rx_baseband_mem_timing[c][k]));
    }
    printf("rx_symbols[0] = %f %f\n", rx_symbols[0].real, rx_symbols[0].imag);
	
    return rx_timing;
}
