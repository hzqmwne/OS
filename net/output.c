#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	int message;
	envid_t server_id;
	struct jif_pkt *pkt = &nsipcbuf.pkt;
	while(1) {
		message = ipc_recv(&server_id, pkt, NULL);
		if(message == NSREQ_OUTPUT && server_id == ns_envid) {
			int r;
			while((r = sys_network_transmit_packet(pkt->jp_data, pkt->jp_len)) < 0) {
				sys_yield();
			}
		}
	}
}
