#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

#include "lo/lo.h"
#include "fofs.h"

// extern int osc_setup();


int done = 0;

void error(int num, const char *m, const char *path);

int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);

int foo_handler(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data);

int quit_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data);

int fof_handler(const char *path, const char *types, lo_arg ** argv,
		int argc, void *data, void *user_data);
  
extern int osc_setup()
{
  const int sample_rate = 48000;
  const FofMode fof_mode = FOF_AMB1D;
  FofBank* fof_bank;
  
  const char*  port = "8800";
  int lo_fd;
  fd_set rfds;
  int retval;

  lo_server s = lo_server_new(port, error);
  fof_bank = fof_newBank(sample_rate, fof_mode);

  lo_server_add_method(s, NULL, NULL, generic_handler, NULL);
  lo_server_add_method(s, "/foo/bar", "fi", foo_handler, NULL);
  lo_server_add_method(s, "/quit", "", quit_handler, NULL);
  lo_server_add_method(s, "/fof", "dfffffffffff", fof_handler, fof_bank);

  lo_fd = lo_server_get_socket_fd(s);

  if (lo_fd > 0)
  {
    do
    {
      FD_ZERO(&rfds);
      FD_SET(lo_fd, &rfds);
      retval = select(lo_fd + 1, &rfds, NULL, NULL, NULL);
      if (retval == -1)
      {
	printf("select() error\n");
	exit(1);
	
      } else if (retval > 0) {
	
	if (FD_ISSET(0, &rfds)) {
	  
	  //read_stdin();
	  
	}
	if (FD_ISSET(lo_fd, &rfds))
	{
	  
	  lo_server_recv_noblock(s, 0);
	  
	}
      }
    } while (!done);
  }
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i;

    printf("path: <%s>\n", path);
    for (i = 0; i < argc; i++) {
        printf("arg %d '%c' ", i, types[i]);
        lo_arg_pp((lo_type)types[i], argv[i]);
        printf("\n");
    }
    printf("\n");
    fflush(stdout);

    return 1;
}

int foo_handler(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data)
{
    /* example showing pulling the argument values out of the argv array */
    printf("%s <- f:%f, i:%d\n\n", path, argv[0]->f, argv[1]->i);
    fflush(stdout);

    return 0;
}

int quit_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data)
{
    done = 1;
    printf("quiting\n\n");

    return 0;
}

int fof_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *fof_bank)
{
  float fargs[11];
  double t = *argv[0];
  
  for (char i = 1; i < argc; ++i)
    fargs[i-1] = *argv[i]->f;

  printf("%s <- f:%f, i:%d\n\n", path, argv[0]->f, argv[1]->i);
  fflush(stdout);
  fof_add_v(fof_bank, t, fargs);
  return 0;
}
