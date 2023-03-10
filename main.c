// Explanation: There is a main loop which adds some job to a job_queue with
//  predefined (JOB_SPAWN_INTERVAL_MS) intervals.
//  Jobs have timoeut, 2 data contexes(ctx1, ctx2) for any purpose, job handler
//  (job->handler) & optional destroy (job->destroy) handlers. jobs_handle() in
//  main loop looks for jobs in the queue, if timeout is expires calls job
//  handler to do stuff, then free if JOB is not needed anymore.

// Questions:
// Q1: add job_myjob_destroy function to free job context & rand_id in ctx1
// Q2: if rand_is is greater than UINT8/2 then replace it with another rand_id &
//     do not free job, let it run again.
// Q3: add signal handler: when USR1 receive print all queue'd jobs, when
//     SIGTERM OR SIGINT received free all queue and gradually exit. All
//     unhandled signals should exit immediately.
// Q4: log_helper should print DEBUG_ID, filename and line info of the caller to
//     a file.
// Q5: create multiple threads, pull and run jobs from the queue with properly
//     locking the queue, process jobs concurrently.
// Q6: create a Makefile which will define a flag for printing logs to
//     stdout instead of a file. Also add debug and release build options.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// A3 part 3:
#include <signal.h>
// A3 part 3 end

#include "debug.h"
#include "jobs.h"

#define DEBUG_ID "main"

#define JOB_SPAWN_INTERVAL_MS 1000
#define JOB_HANDLE_INTERVAL_MS 500
#define JOB_DELAY_MS 3000

// A5 part 1
#include <pthread.h>
#include <sys/syscall.h>
#define NUM_THREADS 4
// A5 part 1 end

// A3 part 1:
static int g_signal_exit = 0;

static void dump_jobs(void) {
  uint8_t c = 0;
  job_t* job;
  job_t* njob;

  log_helper("dump jobs:");
  TAILQ_FOREACH_SAFE(job, &jobs, jobs, njob) {
    log_helper("job %u: ctx1=%u tsms=%lu", c++,
               job->ctx1 == NULL ? 0 : (uint8_t) * ((uint8_t*)job->ctx1),
               job->tsms);
  }
}

static void signal_handler(int signum) {
  log_helper("signal %d received! count: %d", signum, g_signal_exit);
  switch (signum) {
    case SIGINT:
    case SIGTERM:
      g_signal_exit++;
      // optional bonus point:
      if (g_signal_exit > 10) {
        exit(EXIT_FAILURE);
      }
      break;
    case SIGUSR1:
      dump_jobs();
      break;
    default:
      log_helper("unhandled signal: %d", signum);
      exit(EXIT_FAILURE);
  }
}

static int signal_init(void) {
  struct sigaction act = {0};

  act.sa_handler = &signal_handler;

  if (sigaction(SIGINT, &act, NULL) < 0) {
    log_helper("can't catch SIGINT");
    return -1;
  }
  if (sigaction(SIGTERM, &act, NULL) < 0) {
    log_helper("can't catch SIGTERM");
    return -1;
  }
  if (sigaction(SIGUSR1, &act, NULL) < 0) {
    log_helper("can't catch SIGTERM");
    return -1;
  }
  return 0;
}
// A3 part 1 end

job_e job_myjob_handle(void* ctx1, void* ctx2) {
  (void)ctx2;
  job_e ret = JOB_DONE;
  uint8_t* rand_ptr = (uint8_t*)ctx1;
  log_helper("handle job %u", *rand_ptr);
  // do stuff

  // A5 part 5
  pid_t tid = syscall(__NR_gettid);
  log_helper("handle job from thread %d ", tid);
  // A5 part 5 end

  // A2:
  if (*rand_ptr > (UINT8_MAX >> 1)) {
    uint8_t rand_new = rand() % UINT8_MAX;

    log_helper("%u: rand_id is greater than %u, change to %u", *rand_ptr,
               (UINT8_MAX >> 1), rand_new);
    *rand_ptr = rand_new;
    ret = JOB_RUN;
  }
  // A2 end

  return ret;
}

// A1 part 1:
void job_myjob_destroy(void* jobctx) {
  job_t* job = jobctx;
  uint8_t* rand_id = NULL;

  if (job == NULL) {
    return;
  }

  rand_id = job->ctx1;
  if (rand_id != NULL) {
    log_helper("free rand_id %u", *rand_id);
    free(rand_id);
  }

  TAILQ_REMOVE(&jobs, job, jobs);
  free(job);
}
// A1 part 1 end;

// A5 part 2
static int create_thread_pool(void) {
  pthread_attr_t attr;
  pthread_t thread_list[NUM_THREADS] = {};

  if (pthread_attr_init(&attr)) {
    log_helper("pthread_attr_init failed!");
    return -1;
  }

  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
    log_helper("pthread_attr_setdetachstate failed!");
    return -1;
  }

  for (size_t i = 0; i < NUM_THREADS; i++) {
    if (pthread_create(&thread_list[i], &attr, jobs_handle_mt, (void*)NULL)) {
      log_helper("pthread_create failed when creating thread #%zu", i);
      return -1;
    }
    log_helper("pthread_attr_setdetachstate failed!");
    log_helper("starting thread %zu (%lu)", i, thread_list[i]);
  }

  return 0;
}
// A5 part 2 end

int main(void) {
  int ret = 0;

  log_helper("main start");
  jobs_init();
  // A3 part 4:
  signal_init();
  // A3 part 4 end

  // A5 part 3
  bool use_threads = true;
  log_helper("creating %u threads", NUM_THREADS);
  if (create_thread_pool()) {
    log_helper("could not create thread pool!");
    ret = -1;
    goto out;
  }
  log_helper("created %u thread(s)", NUM_THREADS);
  // A5 part 3 end

  // main loop
  while (g_signal_exit == 0) {
    uint64_t timestamp = get_time_ms(CLOCK_REALTIME);
    static uint64_t last_timestamp = 0, last_handle_timestamp = 0;

    if (timestamp - last_timestamp > JOB_SPAWN_INTERVAL_MS) {
      uint8_t* rand_id = calloc(0, sizeof(uint8_t));
      *rand_id = rand() % UINT8_MAX;

      log_helper("add jobq with rand_id=%u", *rand_id);
      // A1 part 2:
      jobs_add(job_myjob_handle, job_myjob_destroy, rand_id, NULL,
               JOB_DELAY_MS);
      // A1 part 2 end;
      // jobs_add(job_myjob_handle, NULL, rand_id, NULL, JOB_DELAY_MS);
      last_timestamp = timestamp;
    }

    // A5 part 5
    if (use_threads) {
      continue;
    }
    // A5 part 5 end
    if (timestamp - last_handle_timestamp > JOB_HANDLE_INTERVAL_MS) {
      log_helper("jobs_handle run");
      jobs_handle();
      last_handle_timestamp = timestamp;
    }
  }

out:
  // A3 part 2:
  jobs_uninit();
  // A3 part 2 end
  return ret;
}
