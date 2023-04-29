/** @file fofs_jack_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include <fofs.h>
#include <fofs_client.h>

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

int fof_process(jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in, *out;
	
	in = jack_port_get_buffer (input_port, nframes);
	out = jack_port_get_buffer (output_port, nframes);
	memcpy (out, in,
		sizeof (jack_default_audio_sample_t) * nframes);

	return 0;      
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
	exit (1);
}

int
main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = "simple";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	
	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. 
	 */

	printf ("engine sample rate: %" PRIu32 "\n",
		jack_get_sample_rate (client));

	/* create two ports */

	int nchans = 8;
	for (int i=0; i < nchans; 
	port[i] = jack_port_register (client, "output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);

	if ((input_port == NULL) || (output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (1);
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}

	if (jack_connect (client, jack_port_name (output_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free (ports);

	/* keep running until stopped by the user */

	sleep (-1);

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/

	jack_client_close (client);
	exit (0);
}


/* 
 * Major problem in fof implementation is that there is memory allocation
 * for the fofs, how to get rid of this probmem? No, this is not a problem
 * as the allocation is made outside the realtime thread.
 */


int fofs_jack_mode_to_nchans(FofMode mode)
{
  int nchans;
  switch (mode)
  {
    case FOF_MONO:
      nchans = 1;
      break;
    case FOF_STEREO:
      nchans = 2;
    case FOF_QUAD:
    case FOF_AMB1:
      nchans = 4;
    case FOF_AMB1D:
      nchans = 8;
    default:
      return NULL;
  }
  return nchans;
}


fofs_jack_client* fofs_jack_client_new(int sample_rate, FofMode mode)
{
  fofs_jack_client* client = (fofs_jack_client*) malloc(sizeof(fofs_jack_client_s));
  const char *client_name = "fofs_jack";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client->mode = mode;
  client->nchans = fofs_jack_mode_to_nchans(mode);
  if (client->nchans == NULL)
    return NULL;
  client->fof_bank = fof_newBank(sample_rate, mode);
  client->j_client = jack_client_open (client_name, options, &status, server_name);
  
  for (int i; i < nchans; ++i)
  {
    char str[80];
    char nstr[2];
    itoa(i, nstr, 10);
    strcpy(str, "output_");
    strcat(str, nstr);
    port[i] = jack_port_register (client->j_client, str,
				  JACK_DEFAULT_AUDIO_TYPE,
				  JackPortIsOutput, 0);
  }
  return client;
}
