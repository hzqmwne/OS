#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define VENDER_ID_82540EM 0x8086
#define DEVIDE_ID_82540EM 0x100e

#define ETHERNET_PACKET_MAX_SIZE 1518

extern volatile uint32_t *e1000_mmio;

int pci_82540em_vendor(struct pci_func *pcif);
int transmit_packet(char *buf, unsigned len);
int receive_packet(char *buf);


uint32_t get_mac_address_low();
uint16_t get_mac_address_high();

#endif	// JOS_KERN_E1000_H
