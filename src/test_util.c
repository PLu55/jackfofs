#include "test_util.h"

void fof_default(fof* _fof)
{
  _fof->argv[FOF_ARG_ampl]   =     1.0f;  
  _fof->argv[FOF_ARG_freq]   =   100.0f;
  _fof->argv[FOF_ARG_gliss]  =    0.0f;
  _fof->argv[FOF_ARG_phi]    =     0.0f;
  _fof->argv[FOF_ARG_beta]   =     0.1f;
  _fof->argv[FOF_ARG_alpha]  =  13.816f;  /* one second duration */
  _fof->argv[FOF_ARG_amin]   =   0.001f;
  _fof->argv[FOF_ARG_cutoff] =   0.002f;
  _fof->argv[FOF_ARG_pan1]   =     0.0f;
  _fof->argv[FOF_ARG_pan2]   =     0.0f;
  _fof->argv[FOF_ARG_pan3]   =     0.0f;
}


char* fof_arg_name[] = { "ampl", "freq","gliss", "phi", "beta",
  "alpha", "amin", "cutoff", "pan1", "pan2", "pan3" };

void print_fof(fof* f)
{
  printf("fof@%p:\n", f);
  printf("     time: %8ld\n", f->time_us);
  for (int i = 0; i < FOF_NUMARGS; i++)
    printf("   %6s: %8.4f\n", fof_arg_name[i], f->argv[i]);
  printf("\n");
}
