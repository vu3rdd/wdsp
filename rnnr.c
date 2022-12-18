#define _CRT_SECURE_NO_WARNINGS
#include "comm.h"

#include "rnnoise.h"

#if defined(linux) || defined(__APPLE__)
#include "calculus.h"
#endif

PORT
void SetRXARNNRRun (int channel, int run)
{
	RNNR a = rxa[channel].rnnr.p;
	if (a->run != run)
	{
		RXAbp1Check (channel, rxa[channel].amd.p->run, rxa[channel].snba.p->run, 
                             rxa[channel].emnr.p->run, rxa[channel].anf.p->run, rxa[channel].anr.p->run,
                             run);
		EnterCriticalSection (&ch[channel].csDSP);
		a->run = run;
		RXAbp1Set (channel);
		LeaveCriticalSection (&ch[channel].csDSP);
	}
}

void setBuffers_rnnr (RNNR a, double* in, double* out)
{
	a->in = in;
	a->out = out;
}

RNNR create_rnnr (int run, int position, double *in, double *out)
{
    RNNR a = (RNNR) malloc0 (sizeof (rnnr));

    a->run = run;
    a->position = position;
    a->st = rnnoise_create(NULL);
    a->frame_size = 2048;
    a->in = in;
    a->out = out;
    return a;
}

void xrnnr (RNNR a, int pos)
{
    if (a->run && pos == a->position)
    {
        // XXX how to get input samples?
        float input[2048];
        float output[2048];

        for (size_t i = 0; i < 2048; i++) {
            input[i] = (float) a->in[2*i];
        }

        buffered_rnnoise_process_frame(a->st, (float *)output, (float *)input);

        for (size_t i = 0; i < 2048; i++) {
            a->out[2*i] = (double) output[i];
            a->out[2*i+1] = 0.0;
        }
    }
    else if (a->out != a->in) {
        printf("xrnnr: memcpy");
        memcpy (a->out, a->in, 2048 * sizeof (complex));
    }
}

void destroy_rnnr (RNNR a)
{
    rnnoise_destroy(a->st);
    _aligned_free (a);
}
