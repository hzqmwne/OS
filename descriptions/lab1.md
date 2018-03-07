<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD><TITLE>Jos Lab 1: PC Bootstrap</TITLE>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Jos Lab 1: Booting a PC</H1>
<P><B>Hand out: Mar. 6th
<BR>Deadline: Mar. 19th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<P>
<H2>Introduction</H2>
<P>This lab is split into three parts. The first part concentrates on getting
familiarized with x86 assembly language, the QEMU x86 emulator, and the PC's
power-on bootstrap procedure. The second part examines the boot loader for our
course kernel, which resides in the <TT>boot</TT> directory of the <TT>lab</TT>
tree. Finally, the third part delves into the initial template for our course
kernel itself, named JOS, which resides in the <TT>kernel</TT> directory.
<P>
<a name="setup"></a>
<H3>Software Setup</H3>
<P>Since in the this and subseqent labs, we will use Git as the version control system, highly recommend learning how to use Git.
</P>
<P>To get the lab in your PC, you need to <i>clone</i> the
course repository, by running the commands below.
You must use an x86 architecture;
that is, <TT>uname -a</TT> should mention <TT>i386 GNU/Linux</TT> or <TT>i686
GNU/Linux</TT>. </p>
<P style="color:red;font-weight:bold;">Please use the virtual machine provided by us.
<a href="./tools.html#vm">Virtual Machine</a></P>
<PRE>
shell% <B>cd oslabs</B>
shell% <B>git clone -b lab1 http://ipads.se.sjtu.edu.cn:1312/lab/jos-2018-spring.git</B>
shell% <B>cd jos-2018-spring</B>
...
shell%
</PRE>
<p>
Git allows you to keep track of the changes you make to the code.
For example, if you are finished with one of the exercises, and want
to checkpoint your progress, you can <i>commit</i> your changes
by running:</p>
<pre>
shell% <b>git commit -am 'my solution for lab1 exercise9'</b>
Created commit 60d2135: my solution for lab1 exercise9
 1 files changed, 1 insertions(+), 0 deletions(-)
shell%
</pre>

<p>
You can keep track of your changes by using the <kbd>git diff</kbd> command.
Running <kbd>git diff</kbd> will display the changes to your code since your
last commit, and <kbd>git diff origin/lab1</kbd> will display the changes
relative to the initial code supplied for this lab.
Here, <tt>origin/lab1</tt> is the name of the git branch with the
initial code you downloaded from our server for this assignment.
</p>
<P>To help you set up running environment, <A
href="./tools.html">tools page</A> has
  directions on how to set up <TT>gcc</TT> <TT>gdb</TT> and <TT>QEMU</TT> for use with JOS.
We also have set up the appropriate compilers and simulators for you in the course supplied Debian VMware image.
<P>

<H3>Grading and Hand-In Procedure</H3>

<P><b>Grading:</b> coding<b>(70%)</b>, document<b>(15%)</b>, interview<b>(15%)</b>.

<p>
When you are ready to hand in your lab,
run <kbd>make handin</kbd> in the your <TT> lab </TT>
directory. This will make a tar file for you, which you can
then upload to the ta's public ftp, before you hand in your source code, you should rename the tar file with your student id, the name of the final file you turnin should be <b>{your student id}.tar.gz</b>.  <kbd>make handin</kbd>
provides more specific directions.
</p>

<p>
Generally speaking, you can <b>ONLY</b> submit your code once to our ftp,
because you cannot modify or replace any files already existing on our server.
However, there may have been some emergencies which you need to improve your handin.
Please add a version number with '_' after your student id, e.g. 123456_1.tar.gz,
and submit again.
</p>
<p>
  <b>
    Keep the submitted file carefully since you might need it to check your grades.
  </b>
<P>For the lab1, you need to hand in your souce code and a document to describe
the design of lab1(<b>The more detail, the more credits you may get!</b>). Note that you should name the document "lab1.pdf" and add it in <b>lab1/</b> directory:
<pre>
ftp:<b> ftp://public.sjtu.edu.cn/upload/os_2018/lab1/</b>
user:<b> Dd_nirvana</b>
password:<b> public</b>
</pre>
<P>You do not need to turn in answers to any of the questions in the text of the
lab. But we will ask the questions about them in the interview, and we believe
that they will help with the rest of the lab.

<P>We will be grading your solutions with a grading program. You can run
<TT>make grade</TT> to test your solutions with the grading program.

<h2>Part 1: PC Bootstrap</h2>

The purpose of the first exercise
is to introduce you to x86 assembly language
and the PC bootstrap process,
and to get you started with QEMU and QEMU/GDB debugging.
You will not have to write any code for this part of the lab,
but you should go through it anyway for your own understanding
and be prepared to answer the questions posed below.

<H3>Getting Started with x86 assembly</H3>
<P>If you are not already familiar with x86 assembly language, you will quickly
become familiar with it during this course! The <A
href="../2017/readings/pcasm-book.pdf">PC Assembly
Language Book</A> is an excellent place to start. Hopefully, the book contains
mixture of new and old material for you.
<P><I>Warning:</I> Unfortunately the examples in the book are written for the
NASM assembler, whereas we will be using the GNU assembler. NASM uses the
so-called <I>Intel</I> syntax while GNU uses the <I>AT&amp;T</I> syntax. While
semantically equivalent, an assembly file will differ quite a lot, at least
superficially, depending on which syntax is used. Luckily the conversion between
the two is pretty simple, and is covered in <A
href="http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html">Brennan's
Guide to Inline Assembly</A>. </P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 1.</SPAN> Read or at least carefully scan the
entire <A href="../2017/readings/pcasm-book.pdf">PC
Assembly Language</A> book, except that you should skip all sections after 1.3.5
in chapter 1, which talk about features of the NASM assembler that do not apply
directly to the GNU assembler. You may also skip chapters 5 and 6, and all
sections under 7.2, which deal with processor and language features we won't use
in JOS. </P>
<P>Also read the section "The Syntax" in <A
href="http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html">Brennan's
Guide to Inline Assembly</A> to familiarize yourself with the most important
features of GNU assembler syntax. </P></DIV>
<P>
<P>Certainly the definitive reference for x86 assembly language programming is
Intel's instruction set architecture reference, which you can find on <A
href="../reference.html">the reference
page</A> in two flavors: an HTML edition of the old <A
href="../2017/readings/i386/toc.htm">80386
Programmer's Reference Manual</A>, which is much shorter and easier to navigate
than more recent manuals but describes all of the x86 processor features that we
will make use of in JOS; and the full, latest and greatest <A
href="https://software.intel.com/en-us/articles/intel-sdm">IA-32
Intel Architecture Software Developer's Manuals</A> from Intel, covering all the
features of the most recent processors that we won't need in class but you may
be interested in learning about. An equivalent (but even longer) set of manuals
is <A
href="http://developer.amd.com/resources/developer-guides-manuals/">available
from AMD</A>, which also covers the new 64-bit extensions now appearing in both
AMD and Intel processors.
<P>You should skim the recommended chapters of the PC Assembly book, and "The
Syntax" section in Brennan's Guide now. Save the Intel/AMD architecture manuals
for later or use them for reference when you want to look up the definitive
explanation of a particular processor feature or instruction.</P>
<H3>Simulating the x86</H3>
Instead of developing the operating system on a real,
physical personal computer (PC), we use a simulator, which emulates a complete
PC faithfully: the code you write for the simulator will boot on a real PC too.
Using a simulator simplifies debugging; you can, for example, set break points
inside of the simulated x86, which is difficult to do with the silicon-version
of an x86.
<P>In JOS we will use the <A href="http://wiki.qemu.org/Index.html">QEMU Emulator</A>, a modern and relatively fast emulator. While QEMU's built-in monitor provides only limited debugging support, QEMU can act as a remote debugging target for the <a href="http://www.gnu.org/software/gdb/">GNU debugger</a> (GDB), which we'll use in this lab to step through the early boot process.  <o:p></o:p></span></p>

<P>To get started, checkout the Lab 1 files into your own directory as
described above in "<a href="#setup">Software Setup</a>", then type <SAMP>make</SAMP> in the
<SAMP>lab</SAMP> directory to build the minimal JOS boot loader and kernel you
will start with. (It's a little generous to call the code we're running here a
"kernel," but we'll flesh it out throughout the semester.)
<PRE>
shell% <B>cd jos-2018-spring</B>
shell% <B>make</B>
+ as kern/entry.S
+ cc kern/init.c
+ cc kern/console.c
+ cc kern/monitor.c
+ cc kern/printf.c
+ cc lib/printfmt.c
+ cc lib/readline.c
+ cc lib/string.c
+ ld obj/kern/kernel
+ as boot/boot.S
+ cc -Os boot/main.c
+ ld boot/boot
boot block is 414 bytes (max 510)
+ mk obj/kern/kernel.img
</pre>

<p><b>(If you get errors like <code>undefined reference to `__udivdi3'</code>, you are probably on x86_64 Linux and don't have the 32-bit gcc multilib. If you're running Debian or Ubuntu, try installing the gcc-multilib package.)</b></p>

<P>Now you're ready to run QEMU, supplying the file <TT>obj/kern/kernel.img</TT>, created above, as the contents of the emulated PC's "virtual hard disk." This hard disk image contains both our boot loader (<TT>obj/boot/boot</TT>) and our kernel (<TT>obj/kern/kernel</TT>).

 <PRE>shell% <B>make qemu</B></PRE>

This executes QEMU with the options required to set the hard disk and direct serial port output to the terminal. Some text should appear in the QEMU window:

<PRE>Booting from Hard Disk...
6828 decimal is XXX octal!
chnum1: 0 chnum2: 0
chnum1: 0
entering test_backtrace 5
entering test_backtrace 4
entering test_backtrace 3
entering test_backtrace 2
entering test_backtrace 1
entering test_backtrace 0
leaving test_backtrace 0
leaving test_backtrace 1
leaving test_backtrace 2
leaving test_backtrace 3
leaving test_backtrace 4
leaving test_backtrace 5
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K&gt;
</PRE>

<P>Everything after '<TT>Booting from Hard Disk...</TT>' was printed by our
skeletal JOS kernel; the <TT>K&gt;</TT> is the prompt printed by the small
<I>monitor</I>, or interactive control program, that we've included in the
kernel. These lines printed by the kernel will also appear in the regular shell
window from which you ran QEMU. This is because for testing and lab grading
purposes we have set up the JOS kernel to write its console output not only to
the virtual VGA display (as seen in the QEMU window), but also to the simulated
PC's virtual parallel port, which QEMU outputs to its own standard output
because of the -serial argument. Likewise, the JOS kernel will take input from
both the keyboard and the serial port, so you can give it commands in either the
VGA display window or the terminal running QEMU. Alternatively, you can use the serial console without the virtual VGA
by running <kbd>make qemu-nox</kbd>.</p>

<P>There are only two commands you can give to the kernel monitor, <TT>help</TT>
and <TT>kerninfo</TT>.
<pre>
K&gt; <kbd>help</kbd>
help - display this list of commands
kerninfo - display information about the kernel
K&gt; <kbd>kerninfo</kbd>
Special kernel symbols:
  _start f010000c (virt)  0010000c (phys)
  etext  f0101a75 (virt)  00101a75 (phys)
  edata  f010f320 (virt)  0010f320 (phys)
  end    f010f980 (virt)  0010f980 (phys)
Kernel executable memory footprint: 63KB
K&gt;
</pre>


<P>The <TT>help</TT> command is obvious, and we will shortly discuss the meaning
of what the <TT>kerninfo</TT> command prints. Although simple, it's important to
note that this kernel monitor is running "directly" on the "raw (virtual)
hardware" of the simulated PC. This means that you should be able to copy the
contents of <TT>obj/kern/kernel.img</TT> onto the first few sectors of a
<I>real</I> hard disk, insert that hard disk into a real PC, turn it on, and see
exactly the same thing on the PC's real screen as you did above in the QEMU
window. (We don't recommend you do this on a real machine with useful
information on its hard disk, though, because copying <TT>kernel.img</TT> onto
the beginning of its hard disk will trash the master boot record and the
beginning of the first partition, effectively causing everything previously on
the hard disk to be lost!) </P>

<H3>The PC's Physical Address Space</H3>We will now dive into a bit more detail
about how a PC starts up. A PC's physical address space is hard-wired to have
the following general layout:
<P>
<TABLE align=center>
  <TBODY>
  <TR>
    <TD><PRE>+------------------+  &lt;- 0xFFFFFFFF (4GB)
|      32-bit      |
|  memory mapped   |
|     devices      |
|                  |
/\/\/\/\/\/\/\/\/\/\

/\/\/\/\/\/\/\/\/\/\
|                  |
|      Unused      |
|                  |
+------------------+  &lt;- depends on amount of RAM
|                  |
|                  |
| Extended Memory  |
|                  |
|                  |
+------------------+  &lt;- 0x00100000 (1MB)
|     BIOS ROM     |
+------------------+  &lt;- 0x000F0000 (960KB)
|  16-bit devices, |
|  expansion ROMs  |
+------------------+  &lt;- 0x000C0000 (768KB)
|   VGA Display    |
+------------------+  &lt;- 0x000A0000 (640KB)
|                  |
|    Low Memory    |
|                  |
+------------------+  &lt;- 0x00000000
</PRE></TR></TBODY></TABLE>The first PCs, which were based on the 16-bit Intel
8088 processor, were only capable of addressing 1MB of physical memory. The
physical address space of an early PC would therefore start at 0x00000000 but
end at 0x000FFFFF instead of 0xFFFFFFFF. The 640KB area marked "Low Memory" was
the <I>only</I> random-access memory (RAM) that an early PC could use; in fact
the very earliest PCs only could be configured with 16KB, 32KB, or 64KB of RAM!
<P>The 384KB area from 0x000A0000 through 0x000FFFFF was reserved by the
hardware for special uses such as video display buffers and firmware held in
non-volatile memory. The most important part of this reserved area is the Basic
Input/Output System (BIOS), which occupies the 64KB region from 0x000F0000
through 0x000FFFFF. In early PCs the BIOS was held in true read-only memory
(ROM), but current PCs store the BIOS in updateable flash memory. The BIOS is
responsible for performing basic system initialization such as activating the
video card and checking the amount of memory installed. After performing this
initialization, the BIOS loads the operating system from some appropriate
location such as floppy disk, hard disk, CD-ROM, or the network, and passes
control of the machine to the operating system.
<P>When Intel finally "broke the one megabyte barrier" with the 80286 and 80386
processors, which supported 16MB and 4GB physical address spaces respectively,
the PC architects nevertheless preserved the original layout for the low 1MB of
physical address space in order to ensure backward compatibility with existing
software. Modern PCs therefore have a "hole" in physical memory from 0x000A0000
to 0x00100000, dividing RAM into "low" or "conventional memory" (the first
640KB) and "extended memory" (everything else). In addition, some space at the
the very top of the PC's 32-bit physical address space, above all physical RAM,
is now commonly reserved by the BIOS for use by 32-bit PCI devices.
<P>Recent x86 processors can support <I>more</I> than 4GB of physical RAM, so
RAM can extend further above 0xFFFFFFFF. In this case the BIOS must arrange to
leave a <I>second</I> hole in the system's RAM at the top of the 32-bit
addressable region, to leave room for these 32-bit devices to be mapped. Because
of design limitations JOS will use only the first 256MB of a PC's physical
memory anyway, so for now we will pretend that all PCs have "only" a 32-bit
physical address space. But dealing with complicated physical address spaces and
other aspects of hardware organization that evolved over many years is one of
the important practical challenges of OS development.

<H3>The ROM BIOS</H3>
In this portion of the lab, you'll use QEMU's debugging facilities to investigate
how an IA-32 compatible computer boots.

Open two terminal windows. In one, enter <B>make qemu-gdb</B>. This starts up QEMU, but QEMU stops just before the processor executes the first instruction and waits for a debugging connection from GDB. In the second terminal, from the same directory you ran <B>make</B>, run <B>gdb</B>. You should see something like this,

<PRE>K&gt; <B>gdb</B>
GNU gdb (GDB) 6.8-debian
Copyright (C) 2008 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later &lt;http://gnu.org/licenses/gpl.html&gt;
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "i486-linux-gnu".
+ target remote localhost:1234
The target architecture is assumed to be i8086
[f000:fff0] 0xffff0:	ljmp   $0xf000,$0xe05b
0x0000fff0 in ?? ()
(gdb)
</PRE>
<p>We provided a <tt>.gdbinit</tt> file that set up GDB to debug the
16-bit code used during early boot and directed it to attach to the
listening QEMU.
<p>The following line:
<pre>
[f000:fff0] 0xffff0:	ljmp   $0xf000,$0xe05b
</pre>
<p>is GDB's disassembly of the first instruction to be executed.
From this output you can conclude a few things:
<ul>
<li>	The IBM PC starts executing at physical address 0x000ffff0,
	which is at the very top of the 64KB area reserved for the ROM BIOS.</li>
 <li>	The PC starts executing
	with <tt>CS = 0xf000</tt> and <tt>IP = 0xfff0</tt>.</li>
 <li>	The first instruction to be executed is a <tt>jmp</tt> instruction,
	which jumps to the segmented address
	<tt>CS = 0xf000</tt> and <tt>IP = 0xe05b</tt>.</li>
</ul>
<p>
Why does QEMU start like this?
This is how Intel designed the 8088 processor,
which IBM used in their original PC.
Because the BIOS in a PC is "hard-wired"
to the physical address range 0x000f0000-0x000fffff,
this design ensures that the BIOS always gets control of the machine first
after power-up or any system restart -
which is crucial because on power-up
there <i>is</i> no other software anywhere in the machine's RAM
that the processor could execute.
The QEMU emulator comes with its own BIOS,
which it places at this location
in the processor's simulated physical address space.
On processor reset,
the (simulated) processor enters real mode
and sets CS to 0xf000 and the IP to 0xfff0, so that
execution begins at that (CS:IP) segment address.
How does the segmented address 0xf000:fff0
turn into a physical address?
</p>

<p>To answer that we need to know a bit about real mode addressing.
In real mode (the mode that PC starts off in),
address translation works according to the formula:
<i>physical address</i> = 16 * <i>segment</i> + <i>offset</i>.
So, when the PC sets CS to 0xf000 and IP to
0xfff0, the physical address referenced is:</p>

<pre>
   16 * 0xf000 + 0xfff0   # in hex multiplication by 16 is
   = 0xf0000 + 0xfff0     # easy--just append a 0.
   = 0xffff0
</pre>

<p>
<tt>0xffff0</tt> is 16 bytes before the end of the BIOS (<tt>0x100000</tt>).
Therefore we shouldn't be surprised that the
first thing that the BIOS does is <tt>jmp</tt> backwards
to an earlier location in the BIOS; after all
how much could it accomplish in just 16 bytes?
</p>

<div class="required">
	<span class="header">Exercise 2.</span>
	Use GDB's <kbd>si</kbd> (Step Instruction) command
	to trace into the ROM BIOS
	for a few more instructions,
	and try to guess what it might be doing.
	You might want to look at
	<a href="http://web.archive.org/web/20040404164813/members.iweb.net.au/~pstorr/pcbook/book2/book2.htm">Phil Storrs I/O Ports Description</a>,
	as well as other materials on the
	<a href="../reference.html">Jos reference materials page</a>.
	No need to figure out all the details -
	just the general idea of what the BIOS is doing first.
</div>


<P>When the BIOS runs, it sets up an interrupt descriptor table and initializes
various devices such as the VGA display. This is where the "<TT>Plex86/Bochs
VGABios</TT>" messages you see in the QEMU window come from. </P>

<P>After initializing the PCI bus and all the important devices the BIOS knows
about, it searches for a bootable device such as a floppy, hard drive, or
CD-ROM. Eventually, when it finds a bootable disk, the BIOS reads the <I>boot
loader</I> from the disk and transfers control to it. </P>


<H2>Part 2: The Boot Loader</H2>
<P>Floppy and hard disks for PCs are divided into 512 byte regions called
<I>sectors</I>. A sector is the disk's minimum transfer granularity: each read
or write operation must be one or more sectors in size and aligned on a sector
boundary. If the disk is bootable, the first sector is called the <I>boot
sector</I>, since this is where the boot loader code resides. When the BIOS
finds a bootable floppy or hard disk, it loads the 512-byte boot sector into
memory at physical addresses 0x7c00 through 0x7dff, and then uses a <TT>jmp</TT>
instruction to set the CS:IP to <TT>0000:7c00</TT>, passing control to the boot
loader. Like the BIOS load address, these addresses are fairly arbitrary - but
they are fixed and standardized for PCs. </P>

<P>The ability to boot from a CD-ROM came much later during the evolution of the
PC, and as a result the PC architects took the opportunity to rethink the boot
process slightly. As a result, the way a modern BIOS boots from a CD-ROM is a
bit more complicated (and more powerful). CD-ROMs use a sector size of 2048
bytes instead of 512, and the BIOS can load a much larger boot image from the
disk into memory (not just one sector) before transferring control to it. For
more information, see the <A
href="http://download.intel.com/support/motherboards/desktop/sb/specscdrom.pdf">"El
Torito" Bootable CD-ROM Format Specification</A>. </P>


<P>For JOS, however, we will use the conventional hard drive boot mechanism,
which means that our boot loader must fit into a measly 512 bytes. The boot
loader consists of one assembly language source file, <TT>boot/boot.S</TT>, and
one C source file, <TT>boot/main.c</TT> Look through these source files
carefully and make sure you understand what's going on. The boot loader must
perform two main functions:
<OL>
  <LI>First, the boot loader switches the processor from real mode to <I>32-bit
  protected mode</I>, because it is only in this mode that software can access
  all the memory above 1MB in the processor's physical address space. Protected
  mode is described briefly in sections 1.2.7 and 1.2.8 of <A
  href="../2017/readings/pcasm-book.pdf">PC
  Assembly Language</A>, and in great detail in the Intel architecture manuals.
  At this point you only have to understand that translation of segmented
  addresses (segment:offset pairs) into physical addresses happens differently
  in protected mode, and that after the transition offsets are 32 bits instead
  of 16.
  <LI>Second, the boot loader reads the kernel from the hard disk by directly
  accessing the IDE disk device registers via the x86's special I/O
  instructions. If you would like to understand better what the particular I/O
  instructions here mean, check out the "IDE hard drive controller" section on
  <A href="../reference.html">the JOS
  reference page</A>. You will not need to learn much about programming specific
  devices in this class: writing device drivers is in practice a very important
  part of OS development, but from a conceptual or architectural viewpoint it is
  also one of the least interesting. </LI></OL>

<P>After you understand the boot loader source code, look at the file
<TT>obj/boot/boot.asm</TT>. This file is a disassembly of the boot loader that
our GNUmakefile creates <I>after</I> compiling the boot loader. This disassembly
file makes it easy to see exactly where in physical memory all of the boot
loader's code resides, and makes it easier to track what's happening while
stepping through the boot loader in GDB.
<P>You can set breakpoints in GDB with the <TT>b</TT> command. For example,
<TT>b *0x7c00 </TT>, sets a breakpoint at address 0x7C00. Once at a breakpoint,
you can continue execution using the <TT>c</TT> and <TT>si</TT> commands: <TT>c</TT>
causes QEMU to continue execution until the next breakpoint (or until you press
<TT>Ctrl-C</TT> in GDB), and <TT>si N</TT> steps through the instructions N at a time.
<P>To examine instructions in memory (besides the immediate next one to be executed,
which GDB prints automatically), you use the <TT>x/i</TT> command. This command has the syntax
<TT>x/Ni ADDR</TT>, where N is the number of consecutive instructions to disassemble and
<TT>ADDR</TT> is the memory address at which to start disassembling.

<P></P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 3.</SPAN> Set a breakpoint at address 0x7c00,
which is where the boot sector will be loaded. Continue execution until that
break point. Trace through the code in <TT>boot/boot.S</TT>, using the source
code and the disassembly file <TT>obj/boot/boot.asm</TT> to keep track of where
you are. Also use the <TT>x/i</TT> command in GDB to disassemble sequences of
instructions in the boot loader, and compare the original boot loader source
code with both the GNU disassembly in <TT>obj/boot/boot.asm</TT> and the GDB</P>

<P>Trace into <TT>bootmain()</TT> in <TT>boot/main.c</TT>, and then into
<TT>readsect()</TT>. Identify the exact assembly instructions that correspond to
each of the statements in <TT>readsect()</TT>. Trace through the rest of
<TT>readsect()</TT> and back out into <TT>bootmain()</TT>, and identify the
begin and end of the <TT>for</TT> loop that reads the remaining sectors of the
kernel from the disk. Find out what code will run when the loop is finished, set
a breakpoint there, and continue to that breakpoint. Then step through the
remainder of the boot loader. </P></DIV>
<P>Be able to answer the following questions:
<UL>
  <LI>At what point does the processor start executing 32-bit code? What exactly
  causes the switch from 16- to 32-bit mode?
  <LI>What is the <I>last</I> instruction of the boot loader executed, and what
  is the <I>first</I> instruction of the kernel it just loaded?
  <LI>How does the boot loader decide how many sectors it must read in order to
  fetch the entire kernel from disk? Where does it find this information?
</LI></UL>

<H3>Loading the Kernel </H3>
We will now look in further detail at the C language
portion of the boot loader, in <TT>boot/main.c</TT>. But before doing so, this
is a good time to stop and review some of the basics of C programming.
<P></P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 4.</SPAN> Read about programming with pointers in
C. The best reference for the C language is <I>The C Programming Language</I> by
Brian Kernighan and Dennis Ritchie (known as 'K&amp;R'). We recommend that
students purchase this book (here is an <A
href="http://www.amazon.cn/C%E7%A8%8B%E5%BA%8F%E8%AE%BE%E8%AE%A1%E8%AF%AD%E8%A8%80-%E5%85%8B%E5%B0%BC%E6%B1%89/dp/B0011C9OMG/ref=sr_1_1?ie=UTF8&qid=1362365327&sr=8-1">Amazon
Link</A>).</P>

<P>Read 5.1 (Pointers and Addresses) through 5.5 (Character Pointers and
Functions) in K&amp;R. Then download the code for <A
href="../2017/readings/lab1/pointers.c">pointers.c</A>,
run it, and make sure you understand where all of the printed values come from.
In particular, make sure you understand where the pointer addresses in lines 1
and 6 come from, how all the values in lines 2 through 4 get there, and why the
values printed in line 5 are seemingly corrupted. </P>
<P>There are other references on pointers in C, though not as strongly
recommended. <A
href="../2017/readings/lab1/pointers.pdf">A tutorial by
Ted Jensen</A> that cites K&amp;R heavily is available in the course readings.
</P>
<P><I>Warning:</I> Unless you are already thoroughly versed in C, do not skip or
even skim this reading exercise. If you do not really understand pointers in C,
you will suffer untold pain and misery in subsequent labs, and then eventually
come to understand them the hard way. Trust us; you don't want to find out what
"the hard way" is. </P></DIV>
<P>To make sense out of <TT>boot/main.c</TT> you'll need to know what an ELF
binary is. When you compile and link a C program such as the JOS kernel, the
compiler transforms each C source ('<TT>.c</TT>') file into an <I>object</I>
('<TT>.o</TT>') file containing assembly language instructions encoded in the
binary format expected by the hardware. The linker then combines all of the
compiled object files into a single <I>binary image</I> such as
<TT>obj/kern/kernel</TT>, which in this case is a binary in the ELF format,
which stands for "Executable and Linkable Format". </P>
<P>Full information about this format is available in <A
href="../2017/readings/elf.pdf">the ELF
specification</A> on <A
href="../reference.html">our reference
page</A>, but you will not need to delve very deeply into the details of this
format in this class. Although as a whole the format is quite powerful and
complex, most of the complex parts are for supporting dynamic loading of shared
libraries, which we will not do in this class.
<P>For purposes of JOS, you can consider an ELF executable to be a header with
loading information, followed by several <I>program sections</I>, each of which
is a contiguous chunk of code or data intended to be loaded into memory at a
specified address. The boot loader does not modify the code or data; it loads it
into memory and starts executing it.
<P>An ELF binary starts with a fixed-length <I>ELF header</I>, followed by a
variable-length <I>program header</I> listing each of the program sections to be
loaded. The C definitions for these ELF headers are in <TT>inc/elf.h</TT>. The
program sections we're interested in are:
<UL>
  <LI><TT>.text</TT>: The program's executable instructions.
  <LI><TT>.rodata</TT>: Read-only data, such as ASCII string constants produced
  by the C compiler. (We will not bother setting up the hardware to prohibit
  writing, however.)
  <LI><TT>.data</TT>: The data section holds the program's initialized data,
  such as global variables declared with initializers like <TT>int x = 5;</TT>.
  </LI></UL>
<P>When the linker computes the memory layout of a program, it reserves space
for <I>uninitialized</I> global variables, such as <TT>int x;</TT>, in a section
called <TT>.bss</TT> that immediately follows <TT>.data</TT> in memory. C
requires that "uninitialized" global variables start with a value of zero. Thus
there is no need to store contents for <TT>.bss</TT> in the ELF binary; instead,
the linker records just the address and size of the <TT>.bss</TT> section. The
loader or the program itself must arrange to zero the <TT>.bss</TT> section.
<P>You can display a full list of the names, sizes, and link addresses of all
the sections in the kernel executable by typing:
<PRE>shell% <B>i386-jos-elf-objdump -h obj/kern/kernel</B>
</PRE>
<P>You can substitute objdump for i386-jos-elf-objdump if your computer uses an ELF
toolchain by default like most modern Linuxen and BSDs.</P>
<P>You will see many more sections than the ones we listed above, but the others
are not important for our purposes. Most of the others are to hold debugging
information, which is typically included in the program's executable file but
not loaded into memory by the program loader.
<P>Besides the section information, there is one more field in the ELF header
that is important to us, named <TT>e_entry</TT>. This field holds the link
address of the <I>entry point</I> in the program: the memory address in the
program's text section at which the program should begin executing. You can see
the entry point:
<PRE>shell% <B>i386-jos-elf-objdump -f obj/kern/kernel</B>
</PRE>
<P>To examine memory in GDB, you use the <TT>x</TT> command with different arguments.
The <A href="http://sourceware.org/gdb/current/onlinedocs/gdb/">GDB Manual</A>
has full details. For now, it is enough to know that the recipe <TT>x/Nx ADDR</TT> prints
<I>N</I> words of memory at <I>ADDR</I>. (Note that both 'x's in the command are lowercase.)
<P><I>Warning</I>: The size of a word is not a universal standard. In GNU assembly, a word is
two bytes (the 'w' in xorw, which stands for word, means 2 bytes).</P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 5.</SPAN> Reset the machine (exit QEMU/GDB and start
them again). Examine the 8 words of memory at 0x00100000 at the point the BIOS
enters the boot loader, and then again at the point the boot loader enters the
kernel. Why are they different? What is there at the second breakpoint? (You do
not really need to use QEMU to answer this question. Just think.) </P></DIV>
<P>
<H3>Link vs. Load Address</H3>
<P>The <I>load address</I> of a binary is the memory address at which a binary
is <I>actually</I> loaded. For example, the BIOS is loaded by the PC hardware at
address 0xf0000. So this is the BIOS's load address. Similarly, the BIOS loads
the boot sector at address 0x7c00. So this is the boot sector's load
address.</P>
<P>The <I>link address</I> of a binary is the memory address for which the
binary is linked. Linking a binary for a given link address prepares it to be
loaded at that address. The linker encodes the link address in the binary in
various ways, for example when the code needs the address of a global variable,
with the result that a binary usually won't work if it is not loaded at the
address that it is linked for.
<P>In one sentence: the link address is the location where a binary
<I>assumes</I> it is going to be loaded, while the load address is the location
where a binary <I>is</I> loaded. It's up to us to make sure that they turn out
to be the same.
<P>Look at the <TT>-Ttext</TT> linker command in <TT>boot/Makefrag</TT>, and at
the address mentioned early in the linker script in <TT>kern/kernel.ld</TT>.
These set the link address for the boot loader and kernel respectively.
<P></P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 6.</SPAN> Trace through the first few
instructions of the boot loader again and identify the first instruction that
would "break" or otherwise do the wrong thing if you were to get the boot
loader's link address wrong. Then change the link address in
<TT>boot/Makefrag</TT> to something wrong, run <TT>make clean</TT>, recompile
the lab with <TT>make</TT>, and trace into the boot loader again to see what
happens. Don't forget to change the link address back and <TT>make clean</TT>
afterwards! </P></DIV>
<P>When object code contains no absolute addresses that encode the link address
in this fashion, we say that the code is <I>position-independent</I>: it will
behave correctly no matter where it is loaded. GCC can generate
position-independent code using the <TT>-fpic</TT> option, and this feature is
used extensively in modern shared libraries that use the ELF executable format.
Position independence typically has some performance cost, however, because it
restricts the ways in which the compiler may choose instructions to access the
program's data. We will not use <TT>-fpic</TT> in JOS.

<H2>Part 3: The Kernel</H2>
We will now start to examine the minimal JOS kernel
in a bit more detail. (And you will finally get to write some code!) Like the
boot loader, the kernel begins with some assembly language code that sets things
up so that C language code can execute properly.
<H3>Using segmentation to work around position dependence</H3>
<P>Did you notice above that while the boot loader's link and load addresses
match perfectly, there appears to be a (rather large) disparity between the
<I>kernel's</I> link and load addresses? Go back and check both and make sure
you can see what we're talking about.
<P>Operating system kernels often like to be linked and run at very high
<I>virtual address</I>, such as 0xf0100000, in order to leave the lower part of
the processor's virtual address space for user programs to use. The reason for
this arrangement will become clearer in the next lab.
<P>Many machines don't have any physical memory at address 0xf0100000, so we
can't count on being able to store the kernel there. Instead, we will use the
processor's memory management hardware to map virtual address 0xf0100000 - the
link address at which the kernel code <I>expects</I> to run - to physical
address 0x00100000 - where the boot loader loaded the kernel. This way, although
the kernel's virtual address is high enough to leave plenty of address space for
user processes, it will be loaded in physical memory at the 1MB point in the
PC's RAM, just above the BIOS ROM. This approach requires that the PC have at
least a few megabytes of physical memory (so that address 0x00100000 works), but
this is likely to be true of any PC built after about 1990.
<P>We will map the <I>entire</I> bottom 256MB of the PC's physical address
space, from 0x00000000 through 0x0fffffff, to virtual addresses 0xf0000000
through 0xffffffff respectively. You should now see why JOS can only use the
first 256MB of physical memory.
<P>The x86 processor has two distinct memory management mechanisms that JOS
could use to achieve this mapping: <I>segmentation</I> and <I>paging</I>. Both
are described in full detail in the <A
href="../2017/readings/i386/toc.htm">80386
Programmer's Reference Manual</A> and the <A
href="../2017/readings/ia32/IA32-3A.pdf">IA-32
Developer's Manual, Volume 3</A>. When the JOS kernel first starts up, it
initially uses segmentation to establish the desired virtual-to-physical
mapping, because it is quick and easy - and the x86 processor requires us to set
up the segmentation hardware in any case, because it can't be disabled!
<P></P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 7.</SPAN> Use QEMU and GDB to trace into the JOS kernel
and find where the new virtual-to-physical mapping takes effect. Then examine
the Global Descriptor Table (GDT) that the code uses to achieve this effect, and
make sure you understand what's going on. </P>
<P>What is the first instruction <I>after</I> the new mapping is established
that would fail to work properly if the old mapping were still in place? Comment
out or otherwise intentionally break the segmentation setup code in
<TT>kern/entry.S</TT>, trace into it, and see if you were right.
</P></DIV>
<H3>Formatted Printing to the Console</H3>
Most people take functions like
<TT>printf()</TT> for granted, sometimes even thinking of them as "primitives"
of the C language. But in an OS kernel, we have to implement all I/O ourselves.
<!--
<p>
<center><table border=1 width=80%><tr><td bgcolor=#e0e0ff>
	<b>Exercise 8.</b>
	Read chapter five of the Lions book, "Two Files".
	Although it is about UNIX on the PDP11,
	you may nevertheless find it highly relevant and useful
	in understanding this section of the lab.
</table></center>
</p>
-->

<p>
Read through <tt>kern/printf.c</tt>, <tt>lib/printfmt.c</tt>,and <tt>kern/console.c</tt>,
and make sure you understand their relationship.It will become clear in later labs why
<tt>printfmt.c</tt> is located in the separate <tt>lib</tt> directory.</p>

<div class="required">
<p><span class="header">Exercise 8.</span>
	We have omitted a small fragment of code -
	the code necessary to print octal numbers
	using patterns of the form "%o".
	Find and fill in this code fragment. Remember the octal number should begin with '0'.
</p></div>
<div class="required">
<p><span class="header">Exercise 9.</span>
    You need also to add support for the "+" flag, which forces
    to precede the result with a plus or minus sign (+ or -) even for positive numbers.
</p></div>
<p>
Be able to answer the following questions:
</p>
<OL>
  <LI>Explain the interface between <CODE>printf.c</CODE> and
  <CODE>console.c</CODE>. Specifically, what function does
  <CODE>console.c</CODE> export? How is this function used by
  <CODE>printf.c</CODE>?
  <LI>Explain the following from <CODE>console.c</CODE>:
<PRE>1      if (crt_pos &gt;= CRT_SIZE) {
2              int i;
3              memcpy(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
4              for (i = CRT_SIZE - CRT_COLS; i &lt; CRT_SIZE; i++)
5                      crt_buf[i] = 0x0700 | ' ';
6              crt_pos -= CRT_COLS;
7      }
</PRE>
  <LI>For the following questions you might wish to consult the notes for
  Lecture 2. These notes cover GCC's calling convention on the x86.
  <P>Trace the execution of the following code step-by-step:
<PRE>int x = 1, y = 3, z = 4;
cprintf("x %d, y %x, z %d\n", x, y, z);
</PRE>
  <UL>
    <LI>In the call to <CODE>cprintf()</CODE>,
    to what does <CODE>fmt</CODE> point?
    To what does <CODE>ap</CODE> point?
    <li> List (in order of execution) each call to
<code>cons_putc</code>, <code>va_arg</code>, and <code>vcprintf</code>.
For <code>cons_putc</code>, list its argument as well.  For
<code>va_arg</code>, list what <code>ap</code> points to before and
after the call.  For <code>vcprintf</code> list the values of its
two arguments.</li>

</UL>
  <LI>Run the following code.
<PRE>unsigned int i = 0x00646c72;
    cprintf("H%x Wo%s", 57616, &amp;i);
</PRE>
What is the output? Explain how this output is arrived out in the
  step-by-step manner of the previous exercise.
<a href="http://www.ascii-code.com/">Here's an ASCII
  table</a> that maps bytes to characters.

  <P>The output depends on that fact that the x86 is little-endian. If the x86
  were instead big-endian what would you set <CODE>i</CODE> to in order to yield
  the same output? Would you need to change <CODE>57616</CODE> to a different
  value?
  <P><A href="http://www.webopedia.com/TERM/b/big_endian.html">Here's a
  description of little- and big-endian</A> and <A
  href="http://www.networksorcery.com/enp/ien/ien137.txt">a more whimsical
  description</A>. </P></li>
  <LI>In the following code, what is going to be printed after
  <CODE>'y='</CODE>? (note: the answer is not a specific value.) Why does this
  happen?
<PRE>    cprintf("x=%d y=%d", 3);
</PRE></li>
  <LI>Let's say that GCC changed its calling convention so that it pushed
  arguments on the stack in declaration order, so that the last argument is
  pushed last. How would you have to change <CODE>cprintf</CODE> or its
  interface so that it would still be possible to pass it a variable number of
  arguments? </LI></OL>

<div class="todo required">
  <P><SPAN class="header">Exercise 10.</SPAN>
    Modify the function <code>printnum()</code> in <code>lib/printfmt.c</code>
to support <code>"%-"</code> when printing numbers.
With the directives starting with "%-", the printed number should be left adjusted. (i.e., paddings are on the right side.)
For example, the following function call: <pre>cprintf("test:[%-5d]", 3)</pre>, should give a result as <pre>"test:[3    ]"</pre>(4 spaces after '3').
Before modifying <code>printnum()</code>, make sure you know what happened in function <code>vprintffmt()</code>.
  </p>
</div>

<H3>The Stack</H3>
In the final exercise of this lab, we will explore in more
detail the way the C language uses the stack on the x86, and in the process
write a useful new kernel monitor function that prints a <I>backtrace</I> of the
stack: a list of the saved Instruction Pointer (IP) values from the nested
<TT>call</TT> instructions that led to the current point of execution.
<P></P>
<DIV class="todo required">
<P><SPAN class=header>Exercise 11.</SPAN> Determine where the kernel initializes
its stack, and exactly where in memory its stack is located. How does the kernel
reserve space for its stack? And at which "end" of this reserved area is the
stack pointer initialized to point to? </P></DIV>
<P>The x86 stack pointer (<TT>esp</TT> register) points to the lowest location
on the stack that is currently in use. Everything <I>below</I> that location in
the region reserved for the stack is free.
Pushing a value onto the stack
involves decreasing the stack pointer and then writing the value to the place
the stack pointer points to. Popping a value from the stack involves reading the
value the stack pointer points to and then increasing the stack pointer. In
32-bit mode, the stack can only hold 32-bit values, and esp is always divisible
by four. Various x86 instructions, such as <TT>call</TT>, are "hard-wired" to
use the stack pointer register.
<p>
The <tt>ebp</tt> (base pointer) register, in contrast,
is associated with the stack primarily by software convention.
On entry to a C function,
the function's <i>prologue</i> code normally
saves the previous function's base pointer by pushing it onto the stack,
and then copies the current <tt>esp</tt> value into <tt>ebp</tt>
for the duration of the function.
If all the functions in a program obey this convention,
then at any given point during the program's execution,
it is possible to trace back through the stack
by following the chain of saved <tt>ebp</tt> pointers
and determining exactly what nested sequence of function calls
caused this particular point in the program to be reached.
This capability can be particularly useful, for example,
when a particular function causes an <tt>assert</tt> failure or <tt>panic</tt>
because bad arguments were passed to it,
but you aren't sure <i>who</i> passed the bad arguments.
A stack backtrace lets you find the offending function.
</p>

<DIV class="todo required">
<P><SPAN class="header">Exercise 12.</SPAN>
To become familiar with the C calling conventions on the x86,
	find the address of the <code>test_backtrace</code> function
	in <tt>obj/kern/kernel.asm</tt>,
	set a breakpoint there,
	and examine what happens each time it gets called
	after the kernel starts.
	How many 32-bit words does each recursive nesting level
	of <code>test_backtrace</code> push on the stack,
	and what are those words?</p>
</p></div>

<p>
The above exercise should give you the information you need
to implement a stack backtrace function,
which you should call <code>mon_backtrace()</code>.
A prototype for this function is already waiting for you
in <tt>kern/monitor.c</tt>.
You can do it entirely in C,
but you may find the <code>read_ebp()</code> function in <tt>inc/x86.h</tt> useful.
You'll also have to hook this new function
into the kernel monitor's command list
so that it can be invoked interactively by the user.
</p>
<P>The backtrace function should display a listing of function call frames in
the following format:
<PRE>Stack backtrace:
Stack backtrace:
  eip f0100a62  ebp f0109e58  args 00000001 f0109e80 f0109e98 f0100ed2 00000031
  eip f01000d6  ebp f0109ed8  args 00000000 00000000 f0100058 f0109f28 00000061
  ...
</PRE>
<p>
The first line printed reflects the <i>currently executing</i> function,
namely <code>mon_backtrace</code> itself,
the second line reflects the function that called <code>mon_backtrace</code>,
the third line reflects the function that called that one, and so on.
You should print <i>all</i> the outstanding stack frames.
By studying <tt>kern/entry.S</tt>
you'll find that there is an easy way to tell when to stop.
</p>
<P>Within each line, the <TT>ebp</TT> value indicates the base pointer into the
stack used by that function: i.e., the position of the stack pointer just after
the function was entered and the function prologue code set up the base pointer.
The listed <TT>eip</TT> value is the function's <I>return instruction
pointer</I>: the instruction address to which control will return when the
function returns. The return instruction pointer typically points to the
instruction after the <TT>call</TT> instruction (why?). Finally, the five hex
values listed after <TT>args</TT> are the first five arguments to the function
in question, which would have been pushed on the stack just before the function
was called. If the function was called with fewer than five arguments, of
course, then not all five of these values will be useful. (Why can't the
backtrace code detect how many arguments there actually are? How could this
limitation be fixed?)

<P>Here are a few specific points you read about in K&amp;R Chapter 5 that are
worth remembering for the following exercise and for future labs.
<ul>
<li>If <code>int *p = (int*)100</code>, then
    <code>(int)p + 1</code> and <code>(int)(p + 1)</code>
    are different numbers: the first is <code>101</code> but
    the second is <code>104</code>.
    When adding an integer to a pointer, as in the second case,
    the integer is implicitly multiplied by the size of the object
    the pointer points to.</li>
<li><code>p[i]</code> is defined to be the same as <code>*(p+i)</code>,
referring to the i'th object in the memory pointed to by p.
The above rule for addition helps this definition work
when the objects are larger than one byte.</li>
<li> <code>&amp;p[i]</code> is the same as <code>(p+i)</code>, yielding
the address of the i'th object in the memory pointed to by p.</li>
</ul>
Although most C programs never need to cast between pointers and integers, operating
systems frequently do. Whenever you see an addition involving a memory address,
ask yourself whether it is an integer addition or pointer addition and make sure
the value being added is appropriately multiplied or not.

<div class="todo required">
<p><span class="header">Exercise 13.</span>
	Implement the backtrace function as specified above.
	Use the same format as in the example, since otherwise the
	grading script will be confused.
	When you think you have it working right,
	run <kbd>make grade</kbd> to see if its output
	conforms to what our grading script expects,
	and fix it if it doesn't.
	<i>After</i> you have handed in your Lab 1 code,
	you are welcome to change the output format of the backtrace function
	any way you like.
</p></div>

<p>
At this point, your backtrace function should give you the addresses of
the function callers on the stack that lead to <code>mon_backtrace()</code>
being executed.  However, in practice you often want to know the function
names corresponding to those addresses.  For instance, you may want to know
which functions could contain a bug that's causing your kernel to crash.
</p>

<p>
To help you implement this functionality, we have provided the function
<code>debuginfo_eip()</code>, which looks up <tt>eip</tt> in the symbol table
and returns the debugging information for that address.  This function is
defined in <tt>kern/kdebug.c</tt>.
</p>

<DIV class="todo required">
<P><SPAN class="header">Exercise 14.</SPAN> Modify your stack backtrace function to display, for each <tt>eip</tt>,
	the function name, source file name, and line number corresponding
	to that <tt>eip</tt>.</p>

	<p>In <code>debuginfo_eip</code>, where do <tt>__STAB_*</tt> come
	from? This question has a long answer; to help you to
	discover the answer, here are some things you might want to
	do:</p>
	<ul>
	  <li> look in the file <tt>kern/kernel.ld</tt> for <tt>__STAB_*</tt></li>
	  <li> run <kbd>i386-jos-elf-objdump -h obj/kern/kernel</kbd></li>
	  <li> run <kbd>i386-jos-elf-objdump -G obj/kern/kernel</kbd></li>
	  <li> run <kbd>i386-jos-elf-gcc -pipe -nostdinc -O2
	  -fno-builtin -I. -MD -Wall -Wno-format -DJOS_KERNEL -gstabs
	  -c -S kern/init.c</kbd>, and look at init.s.</li>
	  <li> see if the bootloader loads the symbol table in memory as part of
	  loading the kernel binary</li>
	</ul>
	<p>Complete the implementation of <code>debuginfo_eip</code> by
	inserting the call to <code>stab_binsearch</code> to find the line
	number for an address.</p>
	<p>Add a <tt>backtrace</tt> command to the kernel monitor, and
	extend your implementation of <code>mon_backtrace</code> to
	call <code>debuginfo_eip</code> and print a line for each
	stack frame of the form:</p>
	<pre>
K&gt; backtrace
Stack backtrace:
  eip f01008ae  ebp f010ff78  args 00000001 f010ff8c 00000000 f0110580 00000000
	 kern/monitor.c:143: monitor+106
  eip f0100193  ebp f010ffd8  args 00000000 00001aac 00000660 00000000 00000000
	 kern/init.c:49: i386_init+59
  eip f010003d  ebp f010fff8  args 00000000 00000000 0000ffff 10cf9a00 0000ffff
	 kern/entry.S:70: &lt;unknown&gt;+0
K&gt;
</pre>
	<p>Each line gives the file name and line within that file of
	the stack frame's <tt>eip</tt>, followed by the name of the
	function and the offset of the <tt>eip</tt> from the first
	instruction of the function (e.g., <tt>monitor+106</tt> means
	the return <tt>eip</tt> is 106 bytes past the beginning of
	<tt>monitor</tt>).</p>
	<p>Be sure to print the file and function names on a separate
	line, to avoid confusing the grading script.</p>
	<p>
	You may find that the some functions are missing from the
	backtrace. For example, you will probably see a call to
	<code>monitor()</code> but not to <code>runcmd()</code>. This is
	because the compiler in-lines some function calls.
	Other optimizations may cause you to see unexpected line
	numbers. If you get rid of the <tt>-O2</tt> from
	<tt>GNUMakefile</tt>, the backtraces may make more sense
	(but your kernel will run more slowly).
</p></div>

<div class="todo required">
  <P><SPAN class="header">Exercise 15.</SPAN>
	There is a "time" command in Linux. The command counts the program's running time.
<PRE>
$time ls
a.file b.file ...

real	0m0.002s
user	0m0.001s
sys	0m0.001s
</PRE>
	In this exercise, you need to implement a rather easy "time" command. 
	The output of the "time" is the running time (in clocks cycles) of the command.
	The usage of this command is like this: "time [command]". 
<PRE>
K> time kerninfo
Special kernel symbols:
 _start f010000c (virt)  0010000c (phys)
 etext  f0101a75 (virt)  00101a75 (phys)
 edata  f010f320 (virt)  0010f320 (phys)
 end    f010f980 (virt)  0010f980 (phys)
Kernel executable memory footprint: 63KB
kerninfo cycles: 23199409
K>
</PRE>
	Here, 23199409 is the running time of the program in cycles.
	As JOS has no support for time system, we could use CPU time stamp counter to measure the time.

	<p><b>Hint: You can refer to instruction "rdtsc" in Intel Mannual for measuring time stamp.
	("rdtsc" may not be very accurate in virtual machine environment. But it's not a problem in this exercise.)</b></p>
  </p>
</div>
<p>
<B>This completes the lab.</B> Type <TT>make grade</TT> in the <TT>lab</TT>
directory for test, then type <kbd>make handin</kbd> to pack the files,
rename the lab1-handin.tar.gz file to <b>{your student id}.tar.gz</b>,  and
follow the directions for uploading your lab tar file onto ta's ftp.
</p>

<br><br>
<table border=0 cellspacing=0 cellpadding=0 width="100%">
<tbody>
<tr bgcolor="#999999" height="20px">
<td style="padding-left:10px; padding-top: 10px; padding-bottom: 10px">

<font style="font-size: 14px;">

<p><b><a href="#top">Top</a></b>

</font>

</td></tr>
</tbody>
</table>
</BODY></HTML>

