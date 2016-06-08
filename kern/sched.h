/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

// This function does not return.
void sched_yield(void) __attribute__((noreturn));

// Lab 4 Challenge Scheduling
void sched_init(void);
void sched_first_in_que(struct Env *e);
void sched_change_priority(struct Env *e, int p);
void sched_recycle(struct Env *e);

#endif	// !JOS_KERN_SCHED_H
