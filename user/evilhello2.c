// evil hello world -- kernel pointer passed to kernel
// kernel should destroy user environment in response

#include <inc/lib.h>
#include <inc/mmu.h>
#include <inc/x86.h>


// Call this function with ring0 privilege
void evil()
{
	// Kernel memory access
	*(char*)0xf010000a = 0;

	// Out put something via outb
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, ' ');
	outb(0x3f8, 'R');
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, 'G');
	outb(0x3f8, '0');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '\n');
}

static void
sgdt(struct Pseudodesc* gdtd)
{
	__asm __volatile("sgdt %0" :  "=m" (*gdtd));
}

__attribute__((__aligned__(PGSIZE)))
static char emptypage[PGSIZE * 2];

static void callgate() {
	evil();
	__asm __volatile("popl %ebp\n"
			"lret\n");
}

// Invoke a given function pointer with ring0 privilege, then return to ring3
void ring0_call(void (*fun_ptr)(void)) {
    // Here's some hints on how to achieve this.
    // 1. Store the GDT descripter to memory (sgdt instruction)
    // 2. Map GDT in user space (sys_map_kernel_page)
    // 3. Setup a CALLGATE in GDT (SETCALLGATE macro)
    // 4. Enter ring0 (lcall instruction)
    // 5. Call the function pointer
    // 6. Recover GDT entry modified in step 3 (if any)
    // 7. Leave ring0 (lret instruction)

    // Hint : use a wrapper function to call fun_ptr. Feel free
    //        to add any functions or global variables in this 
    //        file if necessary.

    // Lab3 : Your Code Here

	struct Pseudodesc gdtd;
	sgdt(&gdtd);
	uintptr_t gdt_kvstart = gdtd.pd_base;
	uintptr_t gdt_kvend = gdtd.pd_base + sizeof(struct Segdesc)*gdtd.pd_lim;
	struct Segdesc *gdt_vstart = (struct Segdesc *)((PGNUM(emptypage)<<PTXSHIFT) | PGOFF(gdt_kvstart));
	struct Segdesc *gdt_vend = (struct Segdesc *)((uintptr_t)gdt_vstart+(gdt_kvend-gdt_kvstart));
	sys_map_kernel_page((void *)gdt_kvstart, gdt_vstart);
	sys_map_kernel_page((void *)(gdt_kvend - 1), (void *)((uintptr_t)gdt_vend - 1));    // idt is 8byte*256, may cross two physical pages

	struct Segdesc backup;
	int gdt_index = 5;
	backup = gdt_vstart[gdt_index];
	struct Gatedesc *gate = (struct Gatedesc *)&gdt_vstart[gdt_index];
	SETCALLGATE(*gate, GD_KT, callgate, 3);

	__asm __volatile("lcall %0, %1"::"g"((gdt_index<<3)), "g"(callgate));

	//gdt_vstart[gdt_index] = backup;    // why is there a General Protection exception when does this ?
}

void
umain(int argc, char **argv)
{
        // call the evil function in ring0
	ring0_call(&evil);

	// call the evil function in ring3
	evil();
}

