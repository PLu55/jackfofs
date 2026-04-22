#ifndef __JFOFS_OFFLINE_H__
#define __JFOFS_OFFLINE_H__

#include <stdint.h>
#include "jfofs_types.h"

/*
 * Offline rendering API
 *
 * Renders FOF synthesis to a WAV file without a running JACK server.
 * Requires libsndfile and libfofs at link time.
 *
 * Typical usage:
 *
 *   int status;
 *   jfofs_offline_job_t *job =
 *       jfofs_offline_create("out.wav", 
 *                            setup.sample_rate,
 *                            setup.mode,
 *                            setup.n_preallocate_fofs,
 *                            setup.fofs_trace_level,
 *                            512,
 *                            &status);
 *   while (need_more_audio) {
 *       // Add some fofs scheduled for next buffer.
 *       jfofs_offline_add(job, time_us, argv);
 *        ...
 *       // Compute the next block of audio and write it to the file.
 *       jfofs_offline_process(job);
 *   }
 *   jfofs_offline_close(job);
 */

typedef struct offline_job_s jfofs_offline_job_t;

/*
 * jfofs_version - return the library version string.
 *
 * The version string is derived from the PROJECT_VERSION CMake variable at
 * build time and follows semantic versioning format, e.g. "1.2.3".
 */
 const char *jfofs_offline_version(void);
 
/*
 * jfofs_offline_create - open a new offline rendering job.
 *
 * @path       - output file path (WAV, 32-bit float)  
 * @sample_rate - samples per second, e.g. 48000
 * @mode - FOF mode (defines channel count, see fofs.h)
 * @n_preallocate_fofs - number of pre-allocated Fofs
 * @fofs_trace_level - trace verbosity level for libfofs
 * @block_size - number of frames per processing block
 * @status     - set to JFOFS_SUCCESS on success, error code on failure
 *
 * Returns a pointer to the new job, or NULL on failure.
 */
jfofs_offline_job_t *jfofs_offline_create(const char *path,
                                          int sample_rate, 
                                          int mode, 
                                          int n_preallocate_fofs, 
                                          int fofs_trace_level,
                                          int block_size, 
                                          int *status);

/*
 * jfofs_offline_add - schedule a FOF for rendering.
 *
 * @job     - the offline job
 * @time_us - activation time in microseconds (relative to job start)
 * @argv    - FOF parameter array of length FOF_NUMARGS
 *
 * Returns JFOFS_SUCCESS or an error code.
 */
int jfofs_offline_add(jfofs_offline_job_t *job, uint64_t time_us,
                      const float *argv);

/*
 * jfofs_offline_process - render one block and write it to the output file.
 *
 * @job - the offline job
 *
 * Returns JFOFS_SUCCESS or JFOFS_FALIURE.
 */
int jfofs_offline_process(jfofs_offline_job_t *job);

/*
 * jfofs_offline_close - flush, close the file and free all resources.
 *
 * @job - the offline job (must not be used after this call)
 */
void jfofs_offline_close(jfofs_offline_job_t *job);

#endif /* __JFOFS_OFFLINE_H__ */
