#define _CRT_SECURE_NO_WARNINGS

#include <specbleach_adenoiser.h>

#include "comm.h"

#if defined(linux) || defined(__APPLE__)
#include "calculus.h"
#endif

void setBuffers_sbnr (SBNR a, double* in, double* out)
{
	a->in = in;
	a->out = out;
}

SBNR create_sbnr (int run, int position, double *in, double *out)
{
    SBNR a = (SBNR) malloc0 (sizeof (sbnr));

    a->run = run;
    a->position = position;
    a->st = specbleach_adaptive_initialize(48000, 20);
    a->in = in;
    a->out = out;
    a->reduction_amount = 10.F;
    a->smoothing_factor = 0.F;
    a->whitening_factor = 0.F;
    a->noise_rescale = 2.F;
    a->post_filter_threshold = -10.F;

    return a;
}

void xsbnr (SBNR a, int pos)
{
    if (a->run && pos == a->position)
    {
        // initialize and process
        SpectralBleachParameters parameters =
              (SpectralBleachParameters){.residual_listen = false,
                                 .reduction_amount = a->reduction_amount,
                                 .smoothing_factor = a->smoothing_factor,
                                 .whitening_factor = a->whitening_factor,
                                 .noise_scaling_type = 0,
                                 .noise_rescale = a->noise_rescale,
                                 .post_filter_threshold = a->post_filter_threshold};

        // Load the parameters before doing the denoising.
        specbleach_adaptive_load_parameters(a->st, parameters);

        // complex input to real input
        float input[2048];
        float output[2048];

        for (size_t i = 0; i < 2048; i++) {
            input[i] = (float) a->in[2*i];
        }
        specbleach_adaptive_process(a->st, (uint32_t)2048,
                                    (float *)input, (float *)output);

        for (size_t i = 0; i < 2048; i++) {
            a->out[2*i] = (double) output[i];
            a->out[2*i+1] = 0.0;
        }
    }
    else if (a->out != a->in) {
        printf("xsbnr: memcpy");
        memcpy (a->out, a->in, 2048 * sizeof (complex));
    }
}

void destroy_sbnr (SBNR a)
{
    specbleach_adaptive_free(a->st);
    _aligned_free (a);
}

PORT
void SetRXASBNRRun (int channel, int run)
{
	SBNR a = rxa[channel].sbnr.p;
	if (a->run != run)
	{
		RXAbp1Check (channel, rxa[channel].amd.p->run, rxa[channel].snba.p->run, 
                             rxa[channel].emnr.p->run, rxa[channel].anf.p->run, rxa[channel].anr.p->run,
                             rxa[channel].rnnr.p->run, run);
		EnterCriticalSection (&ch[channel].csDSP);
		a->run = run;
		RXAbp1Set (channel);
		LeaveCriticalSection (&ch[channel].csDSP);
	}
}

// reduction amount is from 0db to 20db
PORT
void SetRXASBNRreductionAmount (int channel, float amount)
{
	EnterCriticalSection (&ch[channel].csDSP);
	rxa[channel].sbnr.p->reduction_amount = amount;
	LeaveCriticalSection (&ch[channel].csDSP);
}

// percentage smoothing factor - 0 to 100.
PORT
void SetRXASBNRsmoothingFactor (int channel, float factor)
{
	EnterCriticalSection (&ch[channel].csDSP);
	rxa[channel].sbnr.p->smoothing_factor = factor;
	LeaveCriticalSection (&ch[channel].csDSP);
}

// percentage of whitening - 0 to 100.
PORT
void SetRXASBNRwhiteningFactor (int channel, float factor)
{
	EnterCriticalSection (&ch[channel].csDSP);
	rxa[channel].sbnr.p->whitening_factor = factor;
	LeaveCriticalSection (&ch[channel].csDSP);
}

// 0 to 12db
PORT
void SetRXASBNRnoiseRescale (int channel, float factor)
{
	EnterCriticalSection (&ch[channel].csDSP);
	rxa[channel].sbnr.p->noise_rescale = factor;
	LeaveCriticalSection (&ch[channel].csDSP);
}

// sets snr threshold in db in which postfilter would blur the musical noise.
// varies from -10 to +10 db.
PORT
void SetRXASBNRpostFilterThreshold (int channel, float threshold)
{
	EnterCriticalSection (&ch[channel].csDSP);
	rxa[channel].sbnr.p->post_filter_threshold = threshold;
	LeaveCriticalSection (&ch[channel].csDSP);
}
