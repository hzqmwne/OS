#include <kern/e1000.h>

// LAB 6: Your driver code here

#include <inc/assert.h>
#include <inc/string.h>
#include <kern/pmap.h>

#define EERD_OFFSET 0x14

#define TDBAL_OFFSET 0x3800
#define TDBAH_OFFSET 0x3804
#define TDLEN_OFFSET 0x3808
#define TDH_OFFSET 0x3810
#define TDT_OFFSET 0x3818
#define TCTL_OFFSET 0x400
#define TIPG_OFFSET 0x410

#define RAL_OFFSET 0x5400
#define RAH_OFFSET 0x5404
#define MTA_OFFSET 0x5200
#define IMS_OFFSET 0xd0
#define RDBAL_OFFSET 0x2800
#define RDBAH_OFFSET 0x2804
#define RDLEN_OFFSET 0x2808
#define RDH_OFFSET 0x2810
#define RDT_OFFSET 0x2818
#define RCTL_OFFSET 0x100

#define TRANSMIT_BUFFER_LEN 64
#define RECEIVE_BUFFER_LEN 128

#define MTA_LEN 128

#define TDESC_CMD_EOP 0x1
#define TDESC_CMD_RS 0x8
#define TDESC_STATUS_DD 0x1

#define RDESC_STATUS_DD 0x1
#define RDESC_STATUS_EOP 0x2

struct EERD {
	unsigned start : 1;
	unsigned  : 3;
	unsigned done : 1;
	unsigned  : 3;
	unsigned addr : 8;
	unsigned data : 16;
};

struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

struct rx_desc
{
	uint64_t addr;
	uint16_t length;
	uint16_t checksum;
	uint8_t status;
	uint8_t errors;
	uint16_t special;
};

struct TCTL {
	unsigned  : 1;    // reserved
	unsigned EN : 1;
	unsigned  : 1;
	unsigned PSP : 1;
	unsigned CT : 8;
	unsigned COLD : 10;
	unsigned SWXOFF : 1;
	unsigned  : 1;
	unsigned RTLC : 1;
	unsigned NRTU : 1;
	unsigned  : 6;
};

struct RCTL {
	unsigned  : 1;
	unsigned EN : 1;
	unsigned SBP : 1;
	unsigned UPE : 1;
	unsigned MPE : 1;
	unsigned LPE : 1;
	unsigned LBM : 2;
	unsigned RDMTS : 2;
	unsigned  : 2;
	unsigned MO : 2;
	unsigned  : 1;
	unsigned BAM : 1;
	unsigned BSIZE : 2;
	unsigned VFE : 1;
	unsigned CFIEN : 1;
	unsigned CFI : 1;
	unsigned  : 1;
	unsigned DPF : 1;
	unsigned PMCF : 1;
	unsigned  : 1;
	unsigned BSEX : 1;
	unsigned SECRC : 1;
	unsigned  : 5;
};

struct TIPG {
	unsigned IPGT : 10;
	unsigned IPGR1 : 10;
	unsigned IPGR2 : 10;
	unsigned  : 2;
};

struct RAH {
	unsigned RAH : 16;
	unsigned AS : 2;
	unsigned  : 13;
	unsigned AV : 1;
};

volatile uint32_t *e1000_mmio;

static uint32_t mac_address_low = 0;
static uint32_t mac_address_high = 0;

__attribute__((__aligned__(16)))
static struct tx_desc transmit_descriptor_buffer[TRANSMIT_BUFFER_LEN];
static char transmit_data_buffer[TRANSMIT_BUFFER_LEN][ETHERNET_PACKET_MAX_SIZE];

__attribute__((__aligned__(16)))
static struct rx_desc receive_descriptor_buffer[RECEIVE_BUFFER_LEN];
static char receive_data_buffer[RECEIVE_BUFFER_LEN][2048];


static uint16_t read_eeprom(uint8_t addr) {
	struct EERD eerd = { .start=1, .addr=addr, };
	e1000_mmio[EERD_OFFSET >> 2] = *(uint32_t *)&eerd;
	do {
		eerd = *(struct EERD *)&e1000_mmio[EERD_OFFSET >> 2];
	} while(!eerd.done);
	return eerd.data;
}

static void set_mac_address() {
	uint32_t word0 = read_eeprom(0);
	uint32_t word1 = read_eeprom(1);
	uint32_t word2 = read_eeprom(2);
	mac_address_low = (word1 << 16) | word0;
	mac_address_high = word2;
}

static void e1000_mem_init(struct pci_func *pcif) {
	uint32_t mmio_base = pcif->reg_base[0];
	uint32_t mmio_size = pcif->reg_size[0];
	uint32_t base_down = ROUNDDOWN(mmio_base, PGSIZE);
	uint32_t size_up = ROUNDUP(mmio_size+PGOFF(mmio_base), PGSIZE);
	uint32_t i;
	for(i = 0; i < size_up; i += PGSIZE) {
		pte_t *pte = pgdir_walk(kern_pgdir, (void *)KSTACKTOP+i, 1);
		*pte = (base_down+i) | PTE_PCD|PTE_PWT|PTE_W|PTE_P;
	}
	e1000_mmio =  (void *)(KSTACKTOP | PGOFF(mmio_base));
	//assert(e1000_mmio[8 >> 2] == 0x80080783);
}

static void e1000_transmit_init() {
	e1000_mmio[TDBAL_OFFSET >> 2] = (uint32_t)PADDR(transmit_descriptor_buffer);
	e1000_mmio[TDBAH_OFFSET >> 2] = 0;
	e1000_mmio[TDLEN_OFFSET >> 2] = sizeof(transmit_descriptor_buffer);
	e1000_mmio[TDH_OFFSET >> 2] = 0;
	e1000_mmio[TDT_OFFSET >> 2] = 0;
	struct TCTL tctl = { .EN=1, .PSP=1, .CT=0x10, .COLD=0x200, };
	e1000_mmio[TCTL_OFFSET >> 2] = *(uint32_t *)&tctl;
	struct TIPG tipg = { .IPGT=10, .IPGR1=4, .IPGR2=6, };
	e1000_mmio[TIPG_OFFSET >> 2] = *(uint32_t *)&tipg;
}

static void e1000_receive_init() {
	int i;
	for(i = 0; i < RECEIVE_BUFFER_LEN; ++i) {
		receive_descriptor_buffer[i].addr = PADDR(&receive_data_buffer[i]);
	}

	e1000_mmio[RAL_OFFSET >> 2] = mac_address_low;
	struct RAH rah = { .RAH=mac_address_high, .AS=0, .AV=1, };
	e1000_mmio[RAH_OFFSET >> 2] = *(uint32_t *)&rah;

	volatile uint32_t *mta = &e1000_mmio[MTA_OFFSET >> 2];
	for(i = 0; i < MTA_LEN; ++i) {
		mta[i] = 0;
	}
	e1000_mmio[IMS_OFFSET >> 2] = 0;
	e1000_mmio[RDBAL_OFFSET >> 2] = (uint32_t)PADDR(receive_descriptor_buffer);
	e1000_mmio[RDBAH_OFFSET >> 2] = 0;
	e1000_mmio[RDLEN_OFFSET >> 2] = sizeof(receive_descriptor_buffer);
	e1000_mmio[RDH_OFFSET >> 2] = 0;
	e1000_mmio[RDT_OFFSET >> 2] = RECEIVE_BUFFER_LEN - 1;    // RDT and RDH must be different !!!!!

	struct RCTL rctl = {
		.EN=1, 
		.LPE=0, 
		.LBM=0, 
		.BAM=1, 
		.BSIZE=0, 
		.BSEX=0, 
		.SECRC=1,
	};
	e1000_mmio[RCTL_OFFSET >> 2] = *(uint32_t *)&rctl;
}

int pci_82540em_vendor(struct pci_func *pcif) {
	pci_func_enable(pcif);
	e1000_mem_init(pcif);
	set_mac_address();
	e1000_transmit_init();
	e1000_receive_init();
	return 0;
}

int transmit_packet(char *buf, unsigned len) {
	if(len > ETHERNET_PACKET_MAX_SIZE) {
		return -2;    // packet is too long
	}
	unsigned tdt = e1000_mmio[TDT_OFFSET >> 2];
	struct tx_desc *desc = &transmit_descriptor_buffer[tdt];
	if(desc->addr != 0 && !(desc->status & TDESC_STATUS_DD)) {
		return -1;    // transmit queue is full
	}
	memmove(transmit_data_buffer[tdt], buf, len);
	desc->addr = PADDR(&transmit_data_buffer[tdt]);
	desc->length = len;
	desc->cmd = TDESC_CMD_EOP | TDESC_CMD_RS;
	desc->status = 0;
	e1000_mmio[TDT_OFFSET >> 2] = (tdt + 1) % TRANSMIT_BUFFER_LEN;
	return 0;
}

int receive_packet(char *buf) {
	unsigned rdt = e1000_mmio[RDT_OFFSET >> 2];
	rdt = (rdt + 1) % RECEIVE_BUFFER_LEN;
	struct rx_desc *desc = &receive_descriptor_buffer[rdt];
	if(!(desc->status & RDESC_STATUS_DD)) {
		return -1;    // no packet
	}
	unsigned len = desc->length;
	memmove(buf, receive_data_buffer[rdt], len);
	desc->status = 0;
	e1000_mmio[RDT_OFFSET >> 2] = rdt;
	return len;
}

uint32_t get_mac_address_low() {
	return mac_address_low;
}

uint16_t get_mac_address_high() {
	return mac_address_high;
}
