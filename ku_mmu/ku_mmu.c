#include <stdio.h>
#include <stdlib.h>

#define OFFSET_MASK 0x03
#define PFN_MASK 0xFC
#define SWAP_MASK 0xFE

void *memory;
void *swap;
void *page;

int memory_size;
int sw_size;

struct ku_pte {
	char pte;
};

char *free_list;
char *swap_list;

struct ku_pcb {
	struct ku_pcb *next;
	struct ku_pte *adress;
	char pid;
};

struct page {
	struct page *next;
	struct ku_pte *adress;
	char pid;
	char va;
};

struct ku_pcb *head;
struct page *p_head;

struct page* swap_page;

void *ku_mmu_init(unsigned int mem_size, unsigned int swap_size){
	memory = malloc(mem_size);
	swap = malloc(swap_size);
	
	head = malloc(sizeof(struct ku_pcb));
	head->next = NULL;
	
	p_head = malloc(sizeof(struct page));
	p_head->next = NULL;

	free_list = (char *)malloc(mem_size / 4);
	swap_list = (char *)malloc(swap_size / 4);

	swap_page = malloc(sizeof(struct page) * swap_size / 4);

	memory_size = mem_size;
	sw_size = swap_size;

	return memory;	
}

int ku_run_proc(char pid, struct ku_pte **ku_cr3){
	struct ku_pcb *temp = head;
	
	while(temp->next != NULL){
		if(temp->pid == pid){
			*ku_cr3 = temp->adress;
			return 0;
		}
	}

	struct ku_pcb *new = malloc(sizeof(struct ku_pcb));
	temp->next = new;
	new->pid = pid;
	new->adress = malloc(64);

	*ku_cr3 = new->adress;

	return 0;
}

int ku_page_fault(char pid, char va){
	struct ku_pcb *temp = head;
	char *entry;

	while (temp->next != NULL) {
		if (temp->pid == pid) {
			entry = (char*)(temp->adress + ((va & PFN_MASK) >> 2));

			if (*entry == 0) {
				for (int i = 0; i < memory_size / 4; i++) {
					if (free_list[i] == 0) {
						char pfn;
						struct page* p_temp = p_head;
						struct page* p_new = malloc(sizeof(struct page));

						while (p_temp->next != NULL) {
							p_temp = p_temp->next;
						}

						p_temp->next = p_new;
						p_new->pid = pid;
						p_new->va = va;
						p_new->adress = temp->adress;

						pfn = (4 * i) & PFN_MASK;
						pfn = pfn | 0x01;

						*entry = pfn;

						free_list[i] = 1;

						return 0;
					}

					struct page* out = p_head->next;
					struct page* in = malloc(sizeof(struct page));
					struct page* p_temp = p_head;
					char* p_entry;

					p_entry = (char *)(out->adress + ((out->va & PFN_MASK) >> 2));

					while (p_temp->next != NULL) {
						p_temp = p_temp->next;
					}

					p_temp->next = in;
					in->pid = pid;
					in->va = va;
					in->adress = temp->adress;

					*entry = *p_entry;

					for (int i = 1; i < sw_size / 4; i++) {
						if (swap_list[i] == 0) {
							char pfn;

							pfn = (2 * i) & SWAP_MASK;

							*p_entry = pfn;
							
							p_head->next = out->next;
							return 0;
						}
					}

					return -1;
				}
			}

			if (!(*entry & 0x01)) {
				struct page* out = p_head->next;
				struct page* in = malloc(sizeof(struct page));
				struct page* p_temp = p_head;
				char* p_entry;
				char c_temp;

				p_entry = (char *)(out->adress + ((out->va & PFN_MASK) >> 2));

				while (p_temp->next != NULL) {
					p_temp = p_temp->next;
				}

				p_temp->next = in;
				in->pid = pid;
				in->va = va;
				in->adress = temp->adress;

				c_temp = *entry;
				*entry = *p_entry;
				*p_entry = c_temp;

				return 0;
			}
		}
	}
}
