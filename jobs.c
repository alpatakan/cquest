#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// A5 part 4
#include <pthread.h>
// A5 part 4 end

#include "debug.h"
#include "jobs.h"

#define DEBUG_ID "jobs"

struct jobs jobs;

static void jobs_destroy(job_t* job) {
  if (job == NULL) {
    log_helper("job is null");
    return;
  }

  if (job->destroy != NULL) {
    job->destroy((void*)job);
  } else {
    free(job);
  }
}

uint64_t get_time_ms(clockid_t clockid) {
  struct timespec ts;

  if (clock_gettime(clockid, &ts) != 0) {
    log_helper("clock_gettime error: %u", errno);
    return 0;
  }
  return (((uint64_t)ts.tv_sec) * 1000) + ((uint64_t)ts.tv_nsec / (1000000));
}

job_t* jobs_add(job_e (*handler)(void* ctx1, void* ctx2),
                void (*destroy)(void* jobctx),
                void* ctx1,
                void* ctx2,
                uint32_t timeout) {
  job_t* job;
  uint64_t timestamp;

  job = (job_t*)calloc(1, sizeof(job_t));
  if (job == NULL) {
    log_helper("cannot allocate memory");
    goto out;
  }

  timestamp = get_time_ms(CLOCK_REALTIME);

  job->handler = handler;
  job->destroy = destroy;
  job->ctx1 = ctx1;
  job->ctx2 = ctx2;
  job->tsms = timestamp + timeout;

  TAILQ_INSERT_TAIL(&jobs, job, jobs);
out:
  return job;
}

void jobs_remove(job_t* job) {
  jobs_destroy(job);
  TAILQ_REMOVE(&jobs, job, jobs);
}

void jobs_handle(void) {
  job_e job_ret;
  job_t* job;
  job_t* njob;
  uint64_t timestamp;

  timestamp = get_time_ms(CLOCK_REALTIME);

  TAILQ_FOREACH_SAFE(job, &jobs, jobs, njob) {
    if (timestamp > job->tsms) {
      job_ret = job->handler(job->ctx1, job->ctx2);
      if (job_ret == JOB_ERR) {
        log_helper("job error!");
      }
      if (job_ret != JOB_RUN) {
        jobs_remove(job);
      }
    }
  }
}

// A5 part 4
static job_t* get_next_job(void) {
  job_t* job;
  if (!TAILQ_EMPTY(&jobs)) {
    job = TAILQ_FIRST(&jobs);
    return job;
  }
  return NULL;
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void* jobs_handle_mt(void* thread_ctx) {
  (void)thread_ctx;

  while (true) {
    job_t* job;
    job_e job_ret;
    uint64_t timestamp = get_time_ms(CLOCK_REALTIME);

    pthread_mutex_lock(&lock);
    job = get_next_job();
    if (!job) {
      pthread_mutex_unlock(&lock);
      continue;
    }

    if (timestamp > job->tsms) {
      TAILQ_REMOVE(&jobs, job, jobs);
      pthread_mutex_unlock(&lock);
    } else {
      pthread_mutex_unlock(&lock);
      continue;
    }

    job_ret = job->handler(job->ctx1, job->ctx2);
    if (job_ret == JOB_ERR) {
      log_helper("job error!");
    }
    if (job_ret != JOB_RUN) {
      jobs_destroy(job);
    }
  }
}
// A5 part 4 end

void jobs_init(void) {
  TAILQ_INIT(&jobs);
}

void jobs_uninit(void) {
  job_t* job;
  job_t* njob;

  TAILQ_FOREACH_SAFE(job, &jobs, jobs, njob) {
    jobs_remove(job);
  }
}
