#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "libmseed.h"

#include "min_max.h"

static void
usage ()
{
  printf ("Usage: ./ms2ampmax <mseedfile>\n");
  printf ("\nOutput format: \n");
  printf ("<maximum>\n");
}

int
main (int argc, char **argv)
{
  char *mseedfile    = NULL;
  MS3TraceList *mstl = NULL;
  MS3TraceID *tid    = NULL;
  MS3TraceSeg *seg   = NULL;
  uint32_t flags     = 0;
  int8_t verbose     = 0;
  int rv;

  /* Simple argument parsing */
  if (argc != 2)
  {
    usage ();
    return -1;
  }
  mseedfile = argv[1];

  /* Set flags to validate CRCs while reading */
  flags |= MSF_VALIDATECRC;
  /* Unpack the data */
  flags |= MSF_RECORDLIST;

  /* Read all miniSEED into a trace list */
  rv = ms3_readtracelist (&mstl, mseedfile, NULL, 0, flags, verbose);
  if (rv != MS_NOERROR)
  {
    ms_log (2, "Cannot read miniSEED from file: %s\n", ms_errorstr (rv));
    return -1;
  }

  /* Traverse trace list structures */
  tid = mstl->traces;
  while (tid)
  {
    /* Allocate the data array of every trace */
    double *data;
    uint64_t totalSamples = 0;
    /* Count how many samples of this trace */
    seg = tid->first;
    while (seg)
    {
      totalSamples += seg->samplecnt;
      seg = seg->next;
    }
#ifdef DEBUG
    printf ("estimated samples of this trace: %" PRId64 "\n", totalSamples);
#endif

    /* Get the data of this trace */
    seg            = tid->first;
    uint64_t index = 0;
    data           = (double *)malloc (sizeof (double) * totalSamples);
    if (data == NULL)
    {
      ms_log (2, "something wrong when mallocing data array\n");
      return -1;
    }
    while (seg)
    {
      /* Unpack and get the data */
      if (seg->recordlist && seg->recordlist->first)
      {
        void *sptr;
        size_t i;
        uint8_t samplesize;
        char sampletype;
        /* Determine sample size and type based on encoding of first record */
        ms_encoding_sizetype (seg->recordlist->first->msr->encoding, &samplesize, &sampletype);

        /* Unpack data samples using record list.
         * No data buffer is supplied, so it will be allocated and assigned to the segment.
         * Alternatively, a user-specified data buffer can be provided here. */
        int64_t unpacked = mstl3_unpack_recordlist (tid, seg, NULL, 0, verbose);

        if (unpacked != seg->samplecnt)
        {
          ms_log (2, "Cannot unpack samples for %s\n", tid->sid);
        }
        else
        {
          if (sampletype == 'a')
          {
            printf ("%*s",
                    (seg->numsamples > INT_MAX) ? INT_MAX : (int)seg->numsamples,
                    (char *)seg->datasamples);
          }
          else
          {
            for (i = 0; i < seg->numsamples; i++)
            {
              sptr = (char *)seg->datasamples + (i * samplesize);
              if (sampletype == 'i')
              {
                data[index] = (double)(*(int32_t *)sptr);
              }
              else if (sampletype == 'f')
              {
                data[index] = (double)(*(float *)sptr);
              }
              else if (sampletype == 'd')
              {
                data[index] = (double)(*(double *)sptr);
              }

              index++;
            }
          }
        }
      }

      seg = seg->next;
    }

    /* Calculate the min, max, and maxamp with all */
    double min, max, maxamp;
    getMinMax (data, totalSamples, &min, &max);
    maxamp = (fabs (max) > fabs (min)) ? fabs (max) : fabs (min);
    printf ("%.5E\n", maxamp);

    free (data);
    tid = tid->next;
  }

  /* Make sure everything is cleaned up */
  if (mstl)
    mstl3_free (&mstl, 0);
  return 0;
}
