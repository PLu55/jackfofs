#include "jfofs.h"
#include "jfofs_private.h"
#include "test_util.h"

void fof_default(fof_t* fof)
{
  fof->argv[FOF_ARG_ampl]   =     1.0f;  
  fof->argv[FOF_ARG_freq]   =   100.0f;
  fof->argv[FOF_ARG_gliss]  =    0.0f;
  fof->argv[FOF_ARG_phi]    =     0.0f;
  fof->argv[FOF_ARG_beta]   =     0.1f;
  fof->argv[FOF_ARG_alpha]  =  13.816f;  /* one second duration */
  fof->argv[FOF_ARG_amin]   =   0.001f;
  fof->argv[FOF_ARG_cutoff] =   0.002f;
  fof->argv[FOF_ARG_pan1]   =     0.0f;
  fof->argv[FOF_ARG_pan2]   =     0.0f;
  fof->argv[FOF_ARG_pan3]   =     0.0f;
}

char* fof_arg_name[] = { "ampl", "freq","gliss", "phi", "beta",
  "alpha", "amin", "cutoff", "pan1", "pan2", "pan3" };

void fof_print(fof_t* fof)
{
  printf("fof @%p:\n", fof);
  printf("     time: %8ld\n", fof->time_us);
  for (int i = 0; i < FOF_NUMARGS; i++)
    printf("   %6s: %8.4f\n", fof_arg_name[i], fof->argv[i]);
  printf("\n");
}

int fof_equal(fof_t* fof1, fof_t* fof2)
{
  int r = 1;
  for (int i = 0; i < FOF_NUMARGS; i++)
  {
    r = r && fof1->argv[i] == fof2->argv[i];
  }
  
  return
    r && fof1->time_us == fof2->time_us;
}
