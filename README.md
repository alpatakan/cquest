# C interview questions

Simple and functional C interview questions. Essential concepts like signal handling, job queue, logging and multi-threading knowledge are required for implementing answers.

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

## Answers

Embedded into source code.
