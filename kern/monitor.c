// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display backtrace", mon_backtrace },
	{ "time", "Display the running time (in clocks cycles) of the command", mon_time },
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

unsigned read_eip();

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		(end-entry+1023)/1024);
	return 0;
}

// Lab1 only
// read the pointer to the retaddr on the stack
static uint32_t
read_pretaddr() {
    uint32_t pretaddr;
    __asm __volatile("leal 4(%%ebp), %0" : "=r" (pretaddr)); 
    return pretaddr;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	//cprintf("Backtrace success\n");
	unsigned *ebp, eip;
	int i;
	struct Eipdebuginfo info;

	cprintf("Stack backtrace:\n");
	ebp = (unsigned *)read_ebp();
	while(ebp) {    // see entry.S, line 73
		eip = ebp[1];

		cprintf("  eip %08x  ebp %08x  args %08x %08x %08x %08x %08x\n", eip, ebp, ebp[2], ebp[3], ebp[4], ebp[5], ebp[6]);
		debuginfo_eip(eip, &info);
		cprintf("         %s:%d ", info.eip_file, info.eip_line);
		for(i = 0; i < info.eip_fn_namelen; ++i) {
			cprintf("%c", info.eip_fn_name[i]);
		}
		cprintf("+%d\n", eip - info.eip_fn_addr);

		ebp = (unsigned *)ebp[0];
	}
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int invokecmd(int argc, char *argv[MAXARGS], struct Trapframe *tf) {
	// Lookup and invoke the command
	int i;
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	return invokecmd(argc, argv, tf);
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

// return EIP of caller.
// does not work if inlined.
// putting at the end of the file seems to prevent inlining.
unsigned
read_eip()
{
	uint32_t callerpc;
	__asm __volatile("movl 4(%%ebp), %0" : "=r" (callerpc));
	return callerpc;
}



int
mon_time(int argc, char **argv, struct Trapframe *tf) {
	long long start, end;
	start = read_tsc();
	invokecmd(argc - 1, argv + 1, tf);
	end = read_tsc();
	cprintf("%s cycles: %d\n", argv[1], end - start);
	return 0;
}


