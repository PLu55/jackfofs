#ifndef __OFFLINE_H__
#define __OFFLINE_H__

#include <stdint.h>
#include "setup.h"
#include "jfofs_types.h"

/*
 * Offline rendering API
 *
 * Renders FOF synthesis to a sound file without a running JACK server.
 * Requires libsndfile for file I/O.
 *
 * Typical usage:
 *
 *   int status;
 *   offline_job_t *job = offline_job_create("out.wav", &setup, 512, &status);
 *   while (need_more_audio) {
 *       offline_job_add(job, time_us, argv);   // schedule FOFs as needed
 *       offline_job_process(job);              // render one block
 *   }
 *   offline_job_close(job);
 */

typedef struct offline_job_s offline_job_t;

/*
 * offline_job_create - open a new offline rendering job.
 *
 * @path       - output file path (WAV, 32-bit float)
 * @setup      - pointer to a setup_t describing sample_rate, mode, etc.
 * @block_size - number of frames per processing block
 * @status     - set to JFOFS_SUCCESS on success, error code on failure
 *
 * Returns a pointer to the new job, or NULL on failure.
 */
offline_job_t *offline_job_create(const char *path, const setup_t *setup,
                                  int block_size, int *status);

/*
 * offline_job_add - schedule a FOF for rendering.
 *
 * @job     - the offline job
 * @time_us - activation time in microseconds (relative to job start)
 * @argv    - FOF parameter array of length FOF_NUMARGS
 *
 * Returns JFOFS_SUCCESS or an error code.
 */
int offline_job_add(offline_job_t *job, uint64_t time_us, const float *argv);

/*
 * offline_job_process - render one block and write it to the output file.
 *
 * @job - the offline job
 *
 * Returns JFOFS_SUCCESS or JFOFS_FALIURE.
 */
int offline_job_process(offline_job_t *job);

/*
 * offline_job_close - flush, close the file and free all resources.
 *
 * @job - the offline job (must not be used after this call)
 */
void offline_job_close(offline_job_t *job);

#endif /* __OFFLINE_H__ */
