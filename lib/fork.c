// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
    int thisid = sys_getenvid();

	volatile pde_t *pde = &uvpd[PDX(addr)];
	if(!(*pde & PTE_P))
        panic("pgfault: not available pde");
    volatile pte_t *pte = &uvpt[(uint32_t)addr / PGSIZE];
    if(!(*pte & PTE_P))
        panic("pgfault: not available pte");

	if(!(err & FEC_WR) || !(*pte & PTE_COW))
        panic("pgfault: wrong perm");


	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	r = sys_page_alloc(thisid, (void *)PFTEMP, PTE_P | PTE_U | PTE_W);
	if(r < 0)
        panic("pgfault: %e", r);
    memmove((void *)PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
    r = sys_page_map(thisid, (void *)PFTEMP, thisid, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W);
    if(r < 0)
        panic("pgfault: %e", r);
    r = sys_page_unmap(thisid, (void *)PFTEMP);
    if(r < 0)
        panic("pgfault: %e", r);

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r = 0;
//cprintf("duppage %d\n", pn);
	// LAB 4: Your code here.
	//panic("duppage not implemented");
	uintptr_t va = pn * PGSIZE;

	volatile pde_t *pde = &uvpd[PDX(va)];
	if(!(*pde & PTE_P))
        return 0;

    volatile pte_t *pte = &uvpt[pn];

	if(*pte & PTE_P) {
        if((*pte & PTE_W) || (*pte & PTE_COW)) {
            r = sys_page_map(thisenv->env_id, (void *)va, envid, (void *)va, PTE_P | PTE_U | PTE_COW);
            if(r < 0)
                return r;
            r = sys_page_map(thisenv->env_id, (void *)va, thisenv->env_id, (void *)va, PTE_P | PTE_U | PTE_COW);
        }
        else
            r = sys_page_map(thisenv->env_id, (void *)va, envid, (void *)va, PTE_P | PTE_U);
	}
	else
        return 0;

	return r;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//

extern void _pgfault_upcall(void);
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");
    int err;

	set_pgfault_handler(pgfault);
	envid_t envid = sys_exofork();
	if(envid == 0) {
        cprintf("at child\n");
        thisenv = &envs[ENVX(sys_getenvid())];
        return 0;
	}
	else if(envid < 0)
        panic("fork: error %e", envid);
    cprintf("at parent\n");

    sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_P | PTE_U | PTE_W);

    uint8_t *addr;
    for (addr = (uint8_t*) UTEXT; addr < (uint8_t *)(UXSTACKTOP-PGSIZE); addr += PGSIZE) {
        err = duppage(envid, ((unsigned)addr) / PGSIZE);
        if(err < 0)
            panic("fork: error %e", err);
    }

    //err = sys_env_set_pgfault_upcall(envid, (void *)_pgfault_upcall);
    //if(err < 0) panic("fork: error %e", err);
    err = sys_env_set_status(envid, ENV_RUNNABLE);
    if(err < 0) panic("fork: error %e", err);

    return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
