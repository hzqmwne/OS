// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>
#include <inc/x86.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

#define USER_STACK_SIZE 2048*PGSIZE

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
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if((uint32_t)addr >= USTACKTOP-USER_STACK_SIZE && (uint32_t)addr < USTACKTOP && !(vpt[PGNUM(addr)] & PTE_P)) {
		// user stack is not present
		envid_t thisid = sys_getenvid();
		r = sys_page_alloc(thisid, (void *)PTE_ADDR(addr), PTE_W|PTE_U|PTE_P);
		if(r < 0) {
			cprintf("%e\n", r);
			panic("pgfault user stack sys_page_alloc error");
		}
		return;
	}
	if(!((err & FEC_WR) && (vpt[PGNUM(addr)] & PTE_COW))) {
		panic("pgfault check error");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
	envid_t thisid = sys_getenvid();    // can't use thisenv->env_id now! After this, child's stack is fine, and then thisenv->env_id can be changed. see lib/fork.c, fork()
	r = sys_page_alloc(thisid, (void *)PFTEMP, PTE_W|PTE_U|PTE_P);
	if(r < 0) {
		cprintf("%e\n", r);
		panic("pgfault sys_page_alloc error");
	}
	memmove((void *)PFTEMP, (void *)PTE_ADDR(addr), PGSIZE);
	r = sys_page_map(thisid, (void *)PFTEMP, thisid, (void *)PTE_ADDR(addr), PTE_W|PTE_U|PTE_P);
	if(r < 0) {
		cprintf("%e\n", r);
		panic("pgfault sys_page_map error");
	}

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
	int r;

	// LAB 4: Your code here.
	envid_t parent = thisenv->env_id;
	uint32_t addr = pn<<PTXSHIFT;
	uint32_t pde = vpd[PDX(addr)];
	if((pde & PTE_P) && (pde & PTE_U)) {
		uint32_t pte = vpt[pn];
		if((pte & PTE_P) && (pte & PTE_U)) {
			if(pte & (PTE_W|PTE_COW)) {
				r = sys_page_map(parent, (void *)addr, envid, (void *)addr, PTE_COW|PTE_U|PTE_P);
				if(r < 0) {
					return r;
				}
				r = sys_page_map(parent, (void *)addr, parent, (void *)addr, PTE_COW|PTE_U|PTE_P);
				if(r < 0) {
					return r;
				}
			}
			else {
				r = sys_page_map(parent, (void *)addr, envid, (void *)addr, PTE_U|PTE_P);
				if(r < 0) {
					return r;
				}
			}
		}
	}
	//panic("duppage not implemented");
	return 0;
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
//   Use vpd, vpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	int r;
	set_pgfault_handler(pgfault);
	envid_t envid = sys_exofork();
	if(envid < 0) {
		return envid;
	}
	else if(envid > 0) {    // now in parent, and envid is child's
		uint32_t pn;
		for(pn = 0; pn < PGNUM(UTOP); ++pn) {
			if(pn == PGNUM(UXSTACKTOP-PGSIZE)) {
				r = sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_W|PTE_U|PTE_P);
				if(r < 0) {
					return r;
				}
			}
			else {
				r = duppage(envid, pn);
				if(r < 0) {
					return r;
				}
			}
		}
		extern void _pgfault_upcall(void);
		sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
		r = sys_env_set_status(envid, ENV_RUNNABLE);
		if(r < 0) {
			return r;
		}
	}
	else {    // child. if can reach here, sys_env_set_status must be called by parent and virtual memory is prepared
		thisenv = (struct Env *)envs + ENVX(sys_getenvid());
	}
	return envid;
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	int r;
	set_pgfault_handler(pgfault);
	envid_t envid = sys_exofork();
	if(envid < 0) {
		return envid;
	}
	else if(envid > 0) {    // now in parent, and envid is child's
		uint32_t pn;
		uint32_t esp = read_esp();
		for(pn = 0; pn < PGNUM(UTOP); ++pn) {
			if(pn == PGNUM(UXSTACKTOP-PGSIZE)) {
				r = sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_W|PTE_U|PTE_P);
				if(r < 0) {
					return r;
				}
			}
			else if(pn == PGNUM(&thisenv)) {    // thisenv
				assert((uint32_t)&thisenv % PGSIZE == 0);
				r = duppage(envid, pn);
				if(r < 0) {
					return r;
				}				
			}
			else if(pn >= PGNUM(USTACKTOP-USER_STACK_SIZE) && pn < PGNUM(USTACKTOP)) {    // user stack
				r = duppage(envid, pn);
				if(r < 0) {
					return r;
				}
			}
			else {
				envid_t parent = thisenv->env_id;
				uint32_t addr = pn<<PTXSHIFT;
				uint32_t pde = vpd[PDX(addr)];
				if((pde & PTE_P) && (pde & PTE_U)) {
					uint32_t pte = vpt[pn];
					if((pte & PTE_P) && (pte & PTE_U)) {
						r = sys_page_map(parent, (void *)addr, envid, (void *)addr, pte & (PTE_W|PTE_U|PTE_P));
						// notice if parent is PTE_COW, it may not share memory
						// or when the child fork(not sfork) again, how to do about share memory ?
						if(r < 0) {
							return r;
						}
					}
				}
			}
		}
		extern void _pgfault_upcall(void);
		sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
		r = sys_env_set_status(envid, ENV_RUNNABLE);
		if(r < 0) {
			return r;
		}
	}
	else {    // child. if can reach here, sys_env_set_status must be called by parent and virtual memory is prepared
		thisenv = (struct Env *)envs + ENVX(sys_getenvid());
	}
	//panic("sfork not implemented");
	return envid;
}
