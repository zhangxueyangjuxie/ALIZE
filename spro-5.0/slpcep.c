/******************************************************************************/
/*                                                                            */
/*                                 slpcep.c                                   */
/*                                                                            */
/*                               SPro Sources                                 */
/*                                                                            */
/* Guig                                                             Apr. 1997 */
/* -------------------------------------------------------------------------- */
/*
   $Author: guig $
   $Date: 2010-11-09 16:57:22 +0100 (Tue, 09 Nov 2010) $
   $Revision: 151 $
*/
/*  
   Copyright (C) 1997-2010 Guillaume Gravier (guig@irisa.fr)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#define _slpcep_c_

#include <spro.h>
#include <gggetopt.h>

static char *cvsid = "$Id: slpcep.c 151 2010-11-09 15:57:22Z guig $";

/* ----------------------------------------------- */
/* ----- global variables set by read_args() ----- */
/* ----------------------------------------------- */
int format = SPRO_SIG_PCM16_FORMAT; /* signal file format                     */
float Fs = 8000.0;                /* input signal sample rate                 */
int channel = 1;                  /* channel to process                       */
int swap = 0;                     /* change input sample byte order           */
size_t ibs = 10000000;            /* input buffer size (in bytes)             */
size_t obs = 10000000;            /* output buffer size (in bytes)            */
int with_header = 0;              /* output variable length header            */

float emphco = 0.95;              /* pre-emphasis coefficient                 */
float fm_l = 20.0;                /* frame length in ms                       */
float fm_d = 10.0;                /* frame shift in ms                        */
int win = SPRO_HAMMING_WINDOW;    /* weighting window                         */

unsigned short nlpc = 12;         /* LPC analysis order                       */
float alpha = 0.0;                /* frequency deformation parameter          */

unsigned short numceps = 12;      /* number of cepstral coefficients          */
int lifter = 0;                   /* liftering value                          */

int flag = 0;                     /* output stream description                */
unsigned long winlen = 0;         /* length in frames of the CMS window       */
float escale = 0.0;               /* energy scale factor                      */

int trace = 0;                    /* trace level                              */


/* ---------------------------- */
/* ----- void usage(void) ----- */
/* ---------------------------- */
void usage(void)
{
  fprintf(stdout, "\nUsage:\n");
  fprintf(stdout, "    slpcep [options] ifile ofile\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Synopsis:\n");
  fprintf(stdout, "    Linear prediction based cepstral analysis of the input signal.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -F, --format=s            input signal file format (pcm16)\n");
  fprintf(stdout, "  -f, --sample-rate=n       signal sample rate for pcm16 input (%.1f Hz)\n", Fs);
  fprintf(stdout, "  -x, --channel=n           channel number (%d)\n", channel);
  fprintf(stdout, "  -B, --swap                swap sample byte order (don't)\n");
  fprintf(stdout, "  -I, --input-bufsize=n     input buffer size in kbytes (10000)\n");
  fprintf(stdout, "  -O, --output-bufsize=n    output buffer size in kbytes (10000)\n");
  fprintf(stdout, "  -H, --header              output variable length header (don't)\n");  
  fprintf(stdout, "\n");
  fprintf(stdout, "  -k, --pre-emphasis=f      pre-emphasis coefficient (%.2f)\n", emphco);
  fprintf(stdout, "  -l, --length=f            frame length in ms (%.1f ms)\n", fm_l);
  fprintf(stdout, "  -d, --shift=f             frame shift in ms (%.1f ms)\n", fm_d);
  fprintf(stdout, "  -w, --window=s            weighting window (HAMMING)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -n, --order=n             LPC analysis order (%u)\n", nlpc);
  fprintf(stdout, "  -a, --alpha=f             frequency warping parameter (%.1f)\n", alpha);
  fprintf(stdout, "\n");
  fprintf(stdout, "  -p, --num-ceps=n          number of cepstral coefficients (%u)\n", numceps);
  fprintf(stdout, "  -r, --lifter=n            liftering value (%d)\n", lifter);
  fprintf(stdout, "\n");
  fprintf(stdout, "  -e, --energy              add log-energy (off)\n");
  fprintf(stdout, "  -s, --scale-energy=f      scale and normalize log-energy (off)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -Z, --cms                 cepstral mean normalization\n");
  fprintf(stdout, "  -R, --normalize           variance normalization\n");
  fprintf(stdout, "  -L, --segment-length=n    segment length in frames for normalization (whole data)\n");
  fprintf(stdout, "  -D, --delta               add first order derivatives\n");
  fprintf(stdout, "  -A, --acceleration        add second order derivatives\n");
  fprintf(stdout, "  -N, --no-static-energy    remove static energy\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -v, --verbose             verbose mode\n");
  fprintf(stdout, "  -V, --version             print version number and exit\n");
  fprintf(stdout, "  -h, --help                this help message\n");
}

/* ---------------------------------- */
/* ----- int main(int, char **) ----- */
/* ---------------------------------- */
int main(int argc, char **argv)
{
  char *ifn, *ofn;                /* I/O filenames                            */
  sigstream_t *is;
  spfstream_t *os;
  float frate;
  unsigned short dim;
  struct spf_header_field * vh = NULL;
  int status;                     /* error status                             */

  int read_args(int, char **);
  struct spf_header_field * set_header(const char *);
  void free_header(struct spf_header_field *);
  int cepstral_analysis(sigstream_t *, spfstream_t *);

  /* ----- process command line ----- */
  if (read_args(argc, argv))
    return(1);

  if (optind < argc)
    ifn = argv[optind++];
  else {
    fprintf(stderr, "slpcep error -- no input filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc)
    ofn = argv[optind++];
  else {
    fprintf(stderr, "slpcep error -- no output filename specified (use --help to get usage)\n");
    return(1);
  }
  
  if (optind < argc) {
    fprintf(stderr, "slpcep error -- invalid number of arguments (use --help to get usage)\n");
    return(1);
  }
  
  /* ----- show what was asked to do ----- */
  if (trace) {
    fprintf(stdout, "%s --> %s\n", ifn, ofn);
    fflush(stdout);
  }
  
  /* ----- open input stream ----- */
  if ((is = sig_stream_open(ifn, format, Fs, ibs, swap)) == NULL) {
    fprintf(stderr, "slpcep error -- cannot open input stream %s\n", ifn);
    return(SPRO_STREAM_OPEN_ERR);
  }

  /* ----- open output stream ----- */
  frate = (unsigned long)(fm_d * is->Fs / 1000.0) / is->Fs; /* real frame period */
  dim = (flag & WITHE) ? numceps + 1 : numceps;

  if (with_header)
    if ((vh = set_header(ifn)) == NULL) {
      fprintf(stderr, "slpcep error -- cannot allocate memory\n");
      sig_stream_close(is);
      return(SPRO_ALLOC_ERR);
    }

  if ((os = spf_output_stream_open(ofn, dim, flag & WITHE, flag, 1.0 / frate, vh, obs)) == NULL) {
    fprintf(stderr, "slpcep error -- cannot open output stream %s\n", ofn);
    sig_stream_close(is); free_header(vh);
    return(SPRO_STREAM_OPEN_ERR);
  }
  
  free_header(vh);
  
  if (winlen)
    set_stream_seg_length(os, winlen);
  if (escale != 0.0)
    set_stream_energy_scale(os, escale);

  /* ----- run LPCC analysis ----- */
  if ((status = cepstral_analysis(is, os)) != 0) {
    fprintf(stderr, "slpcep error -- error processing stream %s\n", ifn);
    sig_stream_close(is); spf_stream_close(os);
    return(status);
  }

  /* ----- clean ----- */
  sig_stream_close(is);
  spf_stream_close(os);

  return(0);
}

/* --------------------------------------------------------------- */
/* ----- int cepstral_analysis(sigstream_t *, spfstream_t *) ----- */
/* --------------------------------------------------------------- */
/*
 * Main standard loop for LPC analysis.
 */
int cepstral_analysis(sigstream_t *is, spfstream_t *os)
{
  unsigned long l, d, j;
  float *w = NULL, *r, *lift = NULL;
  sample_t *buf;
  spsig_t *s;
  spf_t *a, *k, *c;
  float sigma;
  int status;

  l = (unsigned long)(fm_l * is->Fs / 1000.0); /* frame length in samples */
  d = (unsigned long)(fm_d * is->Fs / 1000.0); /* frame shift in samples */

  /* ----- initialize some more stuff ----- */
  if ((s = sig_alloc(l)) == NULL) /* frame signal */
    return(SPRO_ALLOC_ERR);

  if (win) {
    if ((buf = (sample_t *)malloc(l * sizeof(sample_t))) == NULL) /* frame buffer */
      return(SPRO_ALLOC_ERR);
    if ((w = set_sig_win(l, win)) == NULL) {
      free(buf); sig_free(s);
      return(SPRO_ALLOC_ERR);    
    }
  }
  else
    buf = s->s;
  
  if ((r = (spf_t *)malloc((nlpc + 1) * sizeof(spf_t))) == NULL) {
    if (win) free(buf); sig_free(s); if (win) free(w);
    return(SPRO_ALLOC_ERR);
  }

  if ((a = (spf_t *)malloc(nlpc * sizeof(spf_t))) == NULL) {
    if (win) free(buf); sig_free(s); if (win) free(w);
    free(r);
    return(SPRO_ALLOC_ERR);
  }

  if ((k = (spf_t *)malloc(nlpc * sizeof(spf_t))) == NULL) {
    if (win) free(buf); sig_free(s); if (win) free(w);
    free(r); free(a);
    return(SPRO_ALLOC_ERR);
  }
  
  if ((c = (spf_t *)malloc((numceps + 1) * sizeof(spf_t))) == NULL) {
    if (win) free(buf); sig_free(s); if (win) free(w);
    free(r); free(a); free(k);
    return(SPRO_ALLOC_ERR);
  }

  if (lifter)
    if ((lift = set_lifter(lifter, numceps)) == NULL) {
      if (win) free(buf); sig_free(s); if (win) free(w);
      free(r); free(a); free(k); free(c);
      return(SPRO_ALLOC_ERR);      
    }

  /* ----- loop on each frame ----- */
  while (get_next_sig_frame(is, channel, l, d, emphco, buf)) {

    /* weight signal */
    if (win)
      sig_weight(s, buf, w);

    /* run LPC analysis */
    if ((status = sig_correl(s, alpha, r, nlpc)) != 0) {
      if (win) free(buf); sig_free(s); if (win) free(w);
      free(a); free(k); free(r); free(c); if (lift) free(lift);
      return(status);
    }

    lpc(r, nlpc, a, k, &sigma);    
    lpc_to_cep(a, nlpc, numceps, c);

    if (lifter)
      for (j = 0; j < numceps; j++)
	*(c+j) *= *(lift+j);

    if (flag & WITHE) {
      if (sigma < SPRO_ENERGY_FLOOR)
	sigma = SPRO_ENERGY_FLOOR;
      *(c+numceps) = (spf_t)(2.0 * log(sigma));
    }
    
    /* write vector to stream */
    if (spf_stream_write(os, c, 1) != 1) { 
      if (win) free(buf); sig_free(s); if (win) free(w);
      free(a); free(k); free(r); free(c); if (lift) free(lift);
      return(SPRO_FEATURE_WRITE_ERR);
    }
  }
    
  /* ----- reset memory ----- */
  if (win) {
    free(buf); 
    free(w);
  }
  sig_free(s); 
  free(r);
  free(a); 
  free(k); 
  free(c);
  if (lift)
    free(lift);

  return(0);
}

/* -------------------------------------------------------------- */
/* ----- struct spf_header_field * set_header(const char *) ----- */
/* -------------------------------------------------------------- */
/*
 * Create variable length header.
 */
#define MAX_NUM_HEADER_FIELDS 9

struct spf_header_field * set_header(const char *ifn)
{
  char str[2048];
  struct spf_header_field * p;
  void free_header(struct spf_header_field *);
  int i = 0;

  if ((p = (struct spf_header_field *)malloc((MAX_NUM_HEADER_FIELDS + 1) * sizeof(struct spf_header_field))) == NULL)
    return(NULL);

  for (i = 0; i <= MAX_NUM_HEADER_FIELDS; i++)
    p[i].name = p[i].value = NULL;

  i = 0;

  /* source file */
  sprintf(str, "%s:%1d", ifn, channel);
  p[i].name = strdup("source_filename"); p[i++].value = strdup(str);

  /* frame stuff:_length, shift, rate and pre-emphasis */
  sprintf(str, "%f", fm_l / 1000.0);
  p[i].name = strdup("frame_length"); p[i++].value = strdup(str);

  sprintf(str, "%f", fm_d / 1000.0);
  p[i].name = strdup("frame_shift"); p[i++].value = strdup(str);

  sprintf(str, "%f", emphco);
  p[i].name = strdup("pre_emphasis"); p[i++].value = strdup(str);

  /* analysis type */
  sprintf(str, "lpc(%d)+cep(%d)", nlpc, numceps);
  p[i].name = strdup("feature_type"); p[i++].value = strdup(str);

  /* -- already in mandatory header --
    if (flag) {
    sp_flag_to_str(flag, str);
    p[i].name = strdup("feature_flag"); p[i++].value = strdup(str);
    }
  */

  /* miscellaneous parameters */
  if (alpha != 0.0) {
    sprintf(str, "%f", alpha);
    p[i].name = strdup("frequency_warping"); p[i++].value = strdup(str);
  }
  else {
    p[i].name = strdup("frequency_warping"); p[i++].value = strdup("linear");
  }

  if ((flag & WITHE) && escale != 0.0) {
    sprintf(str, "%f", escale);
    p[i].name = strdup("energy_scaling"); p[i++].value = strdup(str);
  }

  if (((flag & WITHE) || (flag & WITHZ)) && winlen) {
    sprintf(str, "%lu", winlen);
    p[i].name = strdup("segment_length"); p[i++].value = strdup(str);
  }

  if (lifter) {
    sprintf(str, "%d", lifter);
    p[i].name = strdup("liftering"); p[i++].value = strdup(str);
  }

  /* check all fields were set */
  while (i) {

    i--;

    if (p[i].name == NULL || p[i].value == NULL) {
      free_header(p);
      return(NULL);
    }
  }

  return(p);
}

/* ------------------------------------------------------- */
/* ----- void free_header(struct spf_header_field *) ----- */
/* ------------------------------------------------------- */
void free_header(struct spf_header_field *p)
{
  int i;

  if (p) {

    for (i = 0; i < MAX_NUM_HEADER_FIELDS; i++) {
      if (p[i].name)
	free(p[i].name);

      if (p[i].value)
	free(p[i].value);
    }

    free(p);
  }
}

/* --------------------------------------- */
/* ----- int read_args(int, cahr **) ----- */
/* --------------------------------------- */
/*
 *  -F, --format=s            signal file format (pcm)
 *  -f, --sample-rate=f       signal sample rate (8000 Hz)
 *  -x, --channel=n           use channel number (n) (1)
 *  -B, --swap                swap input stream samples
 *  -M, --input-bufsize=n     input buffer size (in bytes)
 *  -S, --output-bufsize=n    output buffer size (in bytes)
 *  -k, --pre-emphasis=f      pre-emphasis coefficient (0.95)
 *  -l, --length=f            frame length in ms (20 ms)
 *  -d, --shift=f             frame shift in ms (10 ms)
 *  -w, --window=s            weighting window (hamming)
 *  -n, --order=n             LPC order (16)
 *  -a, --alpha=n             frequency warping parameter (0.0)
 *  -p, --num-ceps=n          number of cepstral coefficients (16)
 *  -r, --lifter=n            liftering value (0)
 *  -e, --energy              add log-energy (off)
 *  -s, --scale-energy=f      scale and normalize log-energy (off)
 *  -W, --segment-length=n    segment length in frames for normalization (whole data)
 *  -Z, --cms                 mean normalization
 *  -R, --var-norm            variance normalization
 *  -D, --delta               add first order derivatives
 *  -A, --acceleration        add second order derivatives
 *  -N, --no-static-energy    remove static energy
 *  -v, --verbose             verbose mode
 *  -V, --version             print version number
 *  -h, --help
 */
int read_args(int argc, char **argv)
{
  int c, i;
  char *p;

  struct option longopts[] = {
    {"format", required_argument, NULL, 'F'},
    {"sample-rate", required_argument, NULL, 'f'},
    {"channel", required_argument, NULL, 'x'},
    {"swap", no_argument, NULL, 'B'},
    {"input-bufsize", required_argument, NULL, 'I'},
    {"output-bufsize", required_argument, NULL, 'O'},
    {"header", no_argument, NULL, 'H'},
    {"pre-emphasis", required_argument, NULL, 'k'},
    {"length", required_argument, NULL, 'l'},
    {"shift", required_argument, NULL, 'd'},
    {"window", required_argument, NULL, 'w'},
    {"order", required_argument, NULL, 'n'},
    {"alpha", required_argument, NULL, 'a'},
    {"num-cep", required_argument, NULL, 'p'},
    {"lifter", required_argument, NULL, 'r'},
    {"energy", no_argument, NULL, 'e'},
    {"scale-energy", required_argument, NULL, 's'},
    {"segment-length", required_argument, NULL, 'L'},
    {"cms", no_argument, NULL, 'Z'},
    {"normalize", no_argument, NULL, 'R'},
    {"delta", no_argument, NULL, 'D'},
    {"acceleration", no_argument, NULL, 'A'},
    {"no-static-energy", no_argument, NULL, 'N'},
    {"verbose", no_argument, NULL, 'v'},
    {"version", no_argument, NULL, 'V'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
  };

  opterr = 0;
  optopt = '!';

  while ((c = getopt_long(argc, argv, "F:f:x:BI:O:k:l:d:w:n:a:p:r:es:L:ZRDANvVh", longopts, &i)) != EOF)

    switch (c) {

    case 'F':
      if (strcasecmp(optarg, "pcm16") == 0)
	format = SPRO_SIG_PCM16_FORMAT;
      else if(strcasecmp(optarg, "alaw") == 0)
	format = SPRO_SIG_ALAW_FORMAT;
      else if(strcasecmp(optarg, "ulaw") == 0)
	format = SPRO_SIG_ULAW_FORMAT;
      else if(strcasecmp(optarg, "wave") == 0)
	format = SPRO_SIG_WAVE_FORMAT;
      else {
#ifdef SPHERE
	if(strcasecmp(optarg, "sphere") == 0)
	  format = SPRO_SIG_SPHERE_FORMAT;
	else {
#endif
	  fprintf(stderr, "slpcep error -- unknown input signal format %s with --format\n", optarg);
	  fprintf(stderr, "                must be one of PCM16, ALAW, ULAW, WAVE or SPHERE.\n");
	  return(1);
#ifdef SPHERE
	}
#endif
      }
      break;

    case 'f':
      Fs = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --sample-rate (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'x':
      channel = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --channel (use --help to get usage)\n", optarg);
	return(1);
      }
      if (channel < 1 || channel > 2) {
	fprintf(stderr, "slpcep error -- invalid channel number %d (not in [1,2])\n", channel);
	return(1);
      }
      break;

    case 'B':
      swap = 1;
      break;

    case 'I':
      ibs = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --input-bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;      

    case 'O':
      obs = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --output-bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;      

    case 'k':
      emphco = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --pre-emphasis (use --help to get usage)\n", optarg);
	return(1);
      }
      if (emphco < 0 || emphco >= 1.0) {
	fprintf(stderr, "slpcep error -- invalid pre-emphasis coefficient %f (not in [0,1[)\n", emphco);
	return(1);
      }
      break;

    case 'l':
      fm_l = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --length (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'd':
      fm_d = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --shift (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'w':
      if (strcasecmp(optarg, "none") == 0)
	win = SPRO_NULL_WINDOW;
      else if (strcasecmp(optarg, "hamming") == 0)
	win = SPRO_HAMMING_WINDOW;
      else if (strcasecmp(optarg, "hanning") == 0)
	win = SPRO_HANNING_WINDOW;
      else if (strcasecmp(optarg, "blackman") == 0)
	win = SPRO_BLACKMAN_WINDOW;
      else {
	fprintf(stderr, "slpcep error -- unknown window type %s with --window\n", optarg);
	fprintf(stderr, "                must be one of NONE, HAMMING, HANNING or BLACKMAN.\n");
	return(1);
      }
      break;

    case 'n':
      nlpc = (unsigned short)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --order (use --help to get usage)\n", optarg);
	return(1);
      }
      if (nlpc < 1) {
	fprintf(stderr, "slpcep error -- null prediction order\n");
	return(1);
      }
      break;

    case 'a':
      alpha = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --alpha (use --help to get usage)\n", optarg);
	return(1);
      }
      if (alpha <= -1 || alpha >= 1) {
	fprintf(stderr, "slpcep error -- invalid spectral resolution coefficient %f (not in ]-1,1[)\n", alpha);
	return(1);
      }
      break;

    case 'p':
      numceps = (unsigned short)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --num-ceps (use --help to get usage)\n", optarg);
	return(1);
      }
      if (numceps < 1) {
	fprintf(stderr, "slpcep error -- null number of cepstral coefficients\n");
	return(1);
      }
      break;

    case 'r':
      lifter = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --lifter (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'e':
      flag |= WITHE;
      break;

    case 's':
      escale = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --scale-energy (use --help to get usage)\n", optarg);
	return(1);
      }
      if (escale <= 0.0) {
	fprintf(stderr, "slpcep error -- invalid energy scale factor %.2f (not > 0)\n", escale);
	return(1);
      }
      break;

    case 'L':
      winlen = (unsigned long)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "slpcep error -- invalid argument %s to --cms (use --help to get usage)\n", optarg);
	return(1);
      }
      break;
      
    case 'Z':
      flag |= WITHZ;
      break;

    case 'R':
      flag |= WITHR;
      break;

    case 'D':
      flag |= WITHD;
      break;

    case 'A':
      flag |= WITHA;
      break;

    case 'N':
      flag |= WITHN;
      break;

    case 'v':
      trace = 1;
      break;

    case 'V':
      fprintf(stdout, "\nSPro %s -- %s\n\n", VERSION, cvsid);
      exit(0);
      break;

    case 'h':
      usage();
      exit(0);
      break;

    default:
      fprintf(stderr, "slpcep error -- unrecognized option %s (use --help to get usage)\n", argv[optind-1]);
      return(1);
    }

  /* ----- basic argument sanity check ----- */
  if (numceps > nlpc) {
    fprintf(stderr, "slpcep error -- number of cepstral coefficients %d > prediction order %d\n", numceps, nlpc);
    return(1);
  }

  if ((flag & WITHN) && !(flag & WITHE)) {
    fprintf(stderr, "slpcep error -- cannot suppress static energy without energy\n");
    return(1);
  }

  if ((flag & WITHA) && !(flag & WITHD)) {
    fprintf(stderr, "slpcep error -- cannot have accelerations without delta\n");
    return(1);
  }

  if ((flag & WITHN) && ! (flag & WITHD)) {
    fprintf(stderr, "slpcep error -- cannot suppress static energy without delta\n");
    return(1);
  }

  if ((flag & WITHR) && ! (flag & WITHZ)) {
    fprintf(stderr, "slpcep error -- cannot have variance normalization without mean removal\n");
    return(1);
  }

  if (fm_l < fm_d)
    fprintf(stderr, "slpcep warning -- using non overlapping frames\n");

  return(0);
}

#undef _slpcep_c_
