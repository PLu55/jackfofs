#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//extern int osc_setup();

int main(int argc, char* argv[])
{
  int opt;
  int port;
  
  while ((opt = getopt(argc, argv, "ilw")) != -1)
  {
        switch (opt)
	{
	case 'p':
	  break;
	}
  }
  
  osc_setup();
  
}
