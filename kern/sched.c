#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

// Lab 4 Challenge Scheduling
#include <inc/string.h>
struct Env* sched_head[SCHED_MAX_USER_PRIORITY];
struct Env* sched_tail[SCHED_MAX_USER_PRIORITY];
struct Env* sched_lastrun[SCHED_MAX_USER_PRIORITY];
struct Env fake_head[SCHED_MAX_USER_PRIORITY];

#define SCHED_HEAD(priority) (sched_head[priority - 1])
#define SCHED_TAIL(priority) (sched_tail[priority - 1])
#define SCHED_LASTRUN(priority) (sched_lastrun[priority - 1])

void
sched_pushback(struct Env *e, int p)
{
    if(p > SCHED_MAX_USER_PRIORITY || p < 1)
        panic("sched_pushback: wrong priority!");
    e->env_sched_next = NULL;
    SCHED_TAIL(p)->env_sched_next = e;
    SCHED_TAIL(p) = e;
}

void
sched_erase(struct Env *e, int p)
{
    if(p > SCHED_MAX_USER_PRIORITY || p < 1)
        panic("sched_erase: wrong priority! %d", p);
    struct Env *cur = SCHED_HEAD(p);
    while(cur != SCHED_TAIL(p))
    {
        if(cur->env_sched_next == e) {
            cur->env_sched_next = e->env_sched_next;
            if(e == SCHED_TAIL(p))
                SCHED_TAIL(p) = cur;
            e->env_sched_next = NULL;
            return;
        }
        cur = cur->env_sched_next;
    }
    panic("sched_erase: fail!");
}

void
sched_recycle(struct Env *e)
{
    if(SCHED_LASTRUN(e->env_sched_priority) == e)
        SCHED_LASTRUN(e->env_sched_priority) = SCHED_HEAD(e->env_sched_priority);
    sched_erase(e, e->env_sched_priority);
}

void
sched_first_in_que(struct Env *e)
{
//cprintf("frist in que: %x\n", e->env_id);
    sched_pushback(e, e->env_sched_priority);
}
void
sched_change_priority(struct Env *e, int p)
{
cprintf("change priority envid: %x to %d\n", e->env_id, p);
    if(e->env_sched_priority == p)
        return;
    sched_erase(e, e->env_sched_priority);
    e->env_sched_priority = p;
    sched_pushback(e, p);
}

void
sched_init(void)
{
cprintf("sched_init\n");
    memset(sched_head, 0, sizeof(sched_head));
    memset(sched_tail, 0, sizeof(sched_tail));
    memset(fake_head, 0, sizeof(fake_head));
    memset(sched_lastrun, 0, sizeof(sched_lastrun));
    int i;
    for(i = 1; i <= SCHED_MAX_USER_PRIORITY; i++) {
        SCHED_LASTRUN(i) = SCHED_HEAD(i) = SCHED_TAIL(i) = fake_head + i - 1;
    }
}

bool
sched_is_que_empty(int p)
{
    return SCHED_HEAD(p) == SCHED_TAIL(p);
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;


	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// LAB 4: Your code here.

//cprintf("yield_start\n");
	//int i, ori_envx;
    //if(curenv) {
    //    ori_envx = (ENVX(curenv->env_id)) % NENV;
    //    i = (ori_envx + 1) % NENV;
    //}
    //else {
    //    ori_envx = NENV - 1;
    //    i = 0;
    //}

    //while(i != ori_envx) {
    //    if(envs[i].env_status == ENV_RUNNABLE)
    //        break;
    //    i = (i + 1) % NENV;
    //}

    //if(envs[i].env_status == ENV_RUNNABLE) {
//cprintf("yield 1: env_cpu: %d cur_cpu: %d envid: %x\n", envs[i].env_cpunum, cpunum(), envs[i].env_id);
    //    env_run(&envs[i]);
    //}
    //else if(envs[i].env_status == ENV_RUNNING
    //    && envs[i].env_cpunum == cpunum()) {
//cprintf("yield 2: env_cpu: %d cur_cpu: %d envid: %x\n", envs[i].env_cpunum, cpunum(), envs[i].env_id);
    //    env_run(&envs[i]);
    //}

    // Lab 4 Challenge Scheduling
    int p;
    struct Env *cur;
    for(p = SCHED_MAX_USER_PRIORITY; p >= 1; p--) {
        if(sched_is_que_empty(p))
            continue;
        cur = SCHED_LASTRUN(p);

        while(cur != SCHED_TAIL(p)) {
            if(cur->env_sched_next->env_status == ENV_RUNNABLE) {
            //cprintf("yield 1: env_cpu: %d cur_cpu: %d envid: %x priority: %d \n",
             //   cur->env_sched_next->env_cpunum, cpunum(), cur->env_sched_next->env_id, cur->env_sched_next->env_sched_priority);

                SCHED_LASTRUN(p) = cur->env_sched_next;
                env_run(cur->env_sched_next);
            }
            cur = cur->env_sched_next;
        }
        cur = SCHED_HEAD(p);
        while(cur != SCHED_LASTRUN(p)) {
            if(cur->env_sched_next->env_status == ENV_RUNNABLE) {
            //cprintf("yield 2: env_cpu: %d cur_cpu: %d envid: %x priority: %d \n",
            //    cur->env_sched_next->env_cpunum, cpunum(), cur->env_sched_next->env_id, cur->env_sched_next->env_sched_priority);

                SCHED_LASTRUN(p) = cur->env_sched_next;
                env_run(cur->env_sched_next);
            }
            cur = cur->env_sched_next;
        }
    }
    if(curenv
    && curenv->env_status == ENV_RUNNING
    && curenv->env_cpunum == cpunum()) {
        env_run(curenv);
    }



//panic("yield_halt\n");

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;
	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;

	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile (
		"movl $0, %%ebp\n"
		"movl %0, %%esp\n"
		"pushl $0\n"
		"pushl $0\n"
		"sti\n"
		"1:\n"
		"hlt\n"
		"jmp 1b\n"
	: : "a" (thiscpu->cpu_ts.ts_esp0));
}

