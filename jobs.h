#ifndef __JOBS_H__
#define __JOBS_H__

#include <stdint.h>
#include <sys/queue.h>
#include <time.h>

#if !defined(TAILQ_FOREACH_SAFE)
#define TAILQ_FOREACH_SAFE(var, head, field, next) \
  for ((var) = ((head)->tqh_first);                \
       (var) && ((next) = ((var)->field.tqe_next), 1); (var) = (next))
#endif

typedef enum { JOB_INIT = 0, JOB_RUN, JOB_DONE, JOB_ERR } job_e;

TAILQ_HEAD(jobs, job);
typedef struct job {
  TAILQ_ENTRY(job)
  jobs;
  job_e (*handler)(void* ctx1, void* ctx2);
  void (*destroy)(void* jobctx);
  void* ctx1;
  void* ctx2;
  uint64_t tsms;
} job_t;

extern struct jobs jobs;

uint64_t get_time_ms(clockid_t clockid);

job_e job_myjob_handle(void* ctx1, void* ctx2);
// A1 part 3:
void job_myjob_destroy(void* jobctx);
// A1 part 3 end;

job_t* jobs_add(job_e (*handler)(void* ctx1, void* ctx2),
                void (*destroy)(void* jobctx),
                void* ctx1,
                void* ctx2,
                uint32_t timeout);
void jobs_remove(job_t* job);
void jobs_handle(void);
void jobs_init(void);
void jobs_uninit(void);

#endif /* __JOBS_H__ */
