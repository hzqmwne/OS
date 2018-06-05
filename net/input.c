#include "ns.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	int r;
	struct jif_pkt *pkt = &nsipcbuf.pkt;
	r = sys_page_alloc(thisenv->env_id, &nsipcbuf, PTE_U|PTE_W|PTE_P);
	assert(r == 0);
	while(1) {
		r = sys_network_receive_packet(pkt->jp_data);
		if(r >= 0) {
			pkt->jp_len = r;
			ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_P|PTE_W|PTE_U);
			r = sys_page_alloc(thisenv->env_id, &nsipcbuf, PTE_U|PTE_W|PTE_P);
			assert(r == 0);
		}
		else {
			sys_yield();
		}
	}
}
