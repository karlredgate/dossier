
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <sys/mman.h>

typedef unsigned char __u8;
typedef unsigned short int __u16;
typedef unsigned int __u32;


struct slot_entry {
	__u8  bus;
	__u8  device;
	__u8  inta_link;
	__u16 inta_irq;
	__u8  intb_link;
	__u16 intb_irq;
	__u8  intc_link;
	__u16 intc_irq;
	__u8  intd_link;
	__u16 intd_irq;
	__u8  slot;
	__u8  reserved;
} __attribute__((packed));

struct routing_table {
	__u32 signature;
	__u16 version;
	__u16 size;
	__u8  router_bus;
	__u8  router_devfn;
	__u16 exclusive_irqs;
	__u32 compatable_router;
	__u32 miniport_data;
	__u8  reserved[11];
	__u8  checksum;
	struct slot_entry slot[1];
} __attribute__((packed));


#define PCI_DEVICE(devfn)         (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)         ((devfn) & 0x07)
extern struct routing_table * pirq_alloc_read_table(void);
extern void pirq_free_table(struct routing_table *table);
extern int pirq_pci_dev_to_slot(struct routing_table *table, int bus, int dev);

/* If unknown, use INT_MAX so they get sorted last */
int pirq_pci_dev_to_slot(struct routing_table *table, int bus, int dev)
{
	int i, num_slots;
	struct slot_entry *slot;

	if (!table)
		return INT_MAX;

	num_slots = (table->size - 32) / sizeof(*slot);
	for (i=0; i<num_slots; i++) {
		slot = &table->slot[i];
		if (slot->bus == bus &&
		    PCI_DEVICE(slot->device) == dev)
			return slot->slot;
	}
	return INT_MAX;
}



struct routing_table * pirq_alloc_read_table()
{
	struct routing_table *table = NULL;
	uint16_t size = 0;
	uint8_t checksum = 0;
	int i;
	void *mem;
	off_t offset=0L;
	int fd=open("/dev/mem", O_RDONLY);

	if(fd==-1)
	{
		perror("open(/dev/mem)");
		return NULL;
	}

	mem = mmap(0, 64*1024, PROT_READ, MAP_SHARED, fd, 0xF0000L);
	if (!mem) {
		perror("mmap(/dev/mem)");
		goto out;
	}

	while( offset < 0xFFFF)
	{
		if(memcmp(mem+offset, "$PIR", 4)==0)
		{
			fprintf( stderr, "found PIR table\n" );
			table = (struct routing_table *)(mem+offset);
			size = table->size;
			fprintf( stderr, "table size is %d\n", size );
			/* quick sanity checks */
			/* Version must be 1.0 */
			if (! (table->version >> 8)==1 && (table->version && 0xFF) == 0) {
			      fprintf( stderr, "table version not 1.0\n" );
			      break;
			}

			table = malloc(size);
			if (!table) {
			    fprintf( stderr, "malloc failed\n" );
			    break;
			}

			memcpy(table, mem+offset, size);
			for (i=0; i<size; i++)
				checksum +=*(((uint8_t *)table)+i);
			if (checksum) {
				fprintf( stderr, "checksum non-0\n" );
				/* free (table);
				table = NULL; */
			}
			break;
		}
		offset += 16;
	}
	munmap(mem, 64*1024);
out:
	close(fd);
	fprintf( stderr, "made it to end of table read\n" );
	return table;
}

void pirq_free_table(struct routing_table *table)
{
	free(table);
}

static void
pirq_unparse_slot(struct slot_entry *slot)
{
	printf("Slot %d: PCI %02x:%02x. ",
	       slot->slot, slot->bus, PCI_DEVICE(slot->device));
	printf("INTA link %x irq %x ", slot->inta_link, slot->inta_irq);
	printf("B link %x irq %x ", slot->intb_link, slot->intb_irq);
	printf("C link %x irq %x ", slot->intc_link, slot->intc_irq);
	printf("D link %x irq %x ", slot->intd_link, slot->intd_irq);
	printf("\n");
}

static void
pirq_unparse_routing_table(struct routing_table *table)
{
	int i, num_slots;
	struct slot_entry *slot;
	char buf[5]; buf[4] = 0;
	memcpy(buf, &table->signature, 4);

	printf("PCI IRQ Routing Table\n");
	printf("Signature: %s\n", buf);
	printf("Version  : %x\n", table->version);
	printf("Size     : %xh\n", table->size);
	printf("Bus      : %x\n", table->router_bus);
	printf("DevFn    : %x\n", table->router_devfn);
	printf("Exclusive IRQs : %x\n", table->exclusive_irqs);
	printf("Compatable Router: %x\n", table->compatable_router);

	num_slots = (table->size - 32) / sizeof(*slot);
	slot = &table->slot[0];
	for (i=0; i<num_slots; i++) {
		pirq_unparse_slot(&slot[i]);
	}
}

int main(int argc, char *argv[])
{
	struct routing_table *table;
	table = pirq_alloc_read_table();
	if (!table) {
		fprintf( stderr, "NULL table\n" );
		return 1;
	}
	pirq_unparse_routing_table(table);
	pirq_free_table(table);
	return 0;
}
