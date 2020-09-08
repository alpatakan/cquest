# C interview questions

Simple and functional C interview questions. Essential concepts like signal handling, job queue, logging and multi-threading knowledge are required for implementing answers.

## Structure

Answers are wrapped between `// AX part N` and `// AX part N end` where X is question number. All files are given to a interviewee with stripping answers.

Files:

- `main.c`
- `jobs.c`
- `jobs.h`
- `debug.h`
- `Makefile`

## Questions

1. Add `job_myjob_destroy` function to free job context & `rand_id` in `ctx1`
2. If `rand_is` is greater than `UINT8 / 2` then replace it with another
   `rand_id` and let it run again.
3. Add signal handler: when `USR1` receive print all queue'd jobs, when
   `SIGTERM` OR `SIGINT` received free all queue and gradually exit. All
   unhandled signals should exit immediately.
4. `log_helper` should print `DEBUG_ID`, filename and line info to `LOG_FILE`.
5. Create multiple threads, pull and run jobs from the queue. Properly lock
   the queue, process jobs concurrently.
6. Create a Makefile which will define a flag for printing logs to
   `stdout` instead of a file. Also add debug and release build options.

## Build

`LOG_STDOUT=y make`
