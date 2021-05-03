/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
	int startaddr;
	void* addr;
	int free;
	int free_index;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/
int buddy(void *addr, int o){
	//int addr1 = (unsigned long)(((void*)addr - (void*)g_memory) ^ (1 << o)) + (unsigned long)g_memory;
	int num = (int)addr - ((int)g_memory);
	num = num/PAGE_SIZE;
	int addr1 = BUDDY_ADDR(addr, o);
	//addr1 = num ^ (1 << o);
	//addr1 = addr1 + (int)g_memory;
	// printf("Buddy: %d\n", addr1);
	int p_addr1 = (int)((void*)addr1 - (void*)g_memory);
	
	p_addr1 = p_addr1/PAGE_SIZE;
	//p_addr1 = num + ((1<<o)/PAGE_SIZE);
	//printf("addr: %d\n", p_addr1);
	return p_addr1;

}

int leftbuddy(void *addr, int o){
	int num = (int)addr - (int)g_memory;
	num = num/PAGE_SIZE;
	int p_addr1 = num - (((1 << o))/PAGE_SIZE);
	return p_addr1;
}

void *split(int size, int index){
	if(size == PAGE_SIZE){
		return NULL;
	}
	for(int i = index; i < 21; i++){
		if (list_empty(&free_area[i]) == 0)
		{
			// printf("big index: %d\n", index);
			// printf("Index: %d\n", i);
			page_t *ptr = list_entry(free_area[i].next, page_t, list); //gets the base address of the first list
			int offset = ptr->startaddr;	
			int addr = ptr ->addr;							  //the offset is necessary for if we're breaking apart something that doesn't start at 0
			int addr2 = offset;											  //address of the base of the piece we're breaking
			int p_addr1 = buddy(addr, i-1);
			int p_addr2 = (void*)addr - (void*)g_memory;
			p_addr2 = p_addr2 >> MIN_ORDER;

			//struct list_head *track = &g_pages[p_addr1].list; //hold the reference to the second half of the split
			//printf("address: %d, other: %d\n", p_addr1, p_addr2);
			// if(i == 20){
			// 		list_del_init(&free_area[i]);
			// }

			if (i == index)
			{
				//printf("LINE 110\n");
				//g_pages[p_addr2].free = 0;
				//g_pages[p_addr2].free_index = i;
				//g_pages[p_addr1].free_index = i;
				//printf("p_addr1: %d\n", p_addr1);
				struct list_head *pos1 = free_area[i].next;
				struct list_head *pos = free_area[i].next->next;
				page_t *ptr2 = list_entry(pos, page_t, list);
				page_t *ting = list_entry(pos1, page_t, list);

				ting -> free_index = i;
				ting -> free = 0;
				
				list_del_init(&free_area[i]);
				//printf("IN SPLIT: Page: %d, bud: %d, size: %d\n", p_addr2, p_addr1, i);
				ptr2 = list_entry(pos, page_t, list);
				//pos = g_pages[p_addr1].list.next;

				
				if (i < 20){
					list_add(pos, &free_area[i]);
					ptr2 = list_entry(free_area[i].prev, page_t, list);
					ptr2 -> free_index = i;
					//printf("Poggers 2: %d\n", ptr2 -> startaddr);
				}
				//printf("Yeet! Page: %d\n", p_addr2);
				// printf("Buddy dump start: \n");
				// buddy_dump();
				// printf("Budy dump end: \n\n");
				return ting -> addr;							  //returns the address
			}
			else
			{
				
				//printf("else: Page: %d, bud: %d, size: %d\n", p_addr2, p_addr1, i);
				
				page_t *ptr2 = list_entry(&g_pages[p_addr1], page_t, list);
				
				struct list_head *pos = free_area[i].next -> next;
				list_del_init(&free_area[i]);

				
				
				
				g_pages[p_addr1].free_index = i-1;
				list_add(&g_pages[p_addr1].list, &free_area[i - 1]); //add this to the list head
					
					//list_add(&g_pages[p_addr2].list, &free_area[i-1]);
				
				g_pages[p_addr2].free_index = i-1;
				if(i == 20){
					g_pages[p_addr2].free_index = 19;
					g_pages[p_addr1].free_index = 19;
				}
				list_add(&g_pages[p_addr2].list, &free_area[i - 1]);
				if(i < 20){
					list_add(pos, &free_area[i]);
				}
				
				// printf("Buddy dump start else: \n");
				// buddy_dump();
				// printf("Budy dump end else: \n\n");
				return split(size, index);
			}
		}
	}
	
	return NULL;
	
}

/**
 * @param page the page number of this page
 * @param size the index inside the free area it would go
*/
void merge(int page, int size){
	int bud = buddy(g_pages[page].addr, size);
	

	
	//	printf("Page: %d, bud: %d, size: %d\n", page, bud, size);
	//	printf("Buddy's free index: %d; buddy's free: %d\n\n", g_pages[bud].free_index, g_pages[bud].free);
	
	// printf("\nInside urge\n\n");
	//  printf("8's buddy %d\n", buddy(PAGE_TO_ADDR(8), size));
	// buddy_dump();
	//printf("leaving urge\n\n");
	
	int yessir = g_pages[bud].free_index == size && g_pages[bud].free == 1;
	
	list_add(&g_pages[page].list, &free_area[size]);

	// struct list_head * pos;
	// list_for_each(pos, &free_area[size]){
	// 	page_t *ptr = list_entry(pos, page_t, list);
	// 	if(ptr -> startaddr == bud){
	// 		//printf("bud: %d\n", bud);
	// 		yessir = 1;
	// 	}
	// 	if(ptr -> startaddr == lbud){
	// 		yam = 1;
	// 	}
	// }
	
	// printf("\nInside purge\n\n");
	// buddy_dump();
	// printf("leaving purge\n\n");
	if(bud < page){
		int temp = bud;
		bud = page;
		page = temp;
	}
	if(yessir == 1 && size < 20){
		
		//printf("innit\n");
		struct list_head *temp = &g_pages[bud].list;
		// __list_del(temp -> prev, temp -> next);
		// __list_del(g_pages[page].list.prev, g_pages[page].list.next);

		// printf("Inside line 217\n\n");
		// buddy_dump();
		// printf("leaving...\n\n");
		
		

		list_del_init(&g_pages[page].list);
		// printf("Inside line 223\n\n");
		// buddy_dump();
		// printf("leaving...\n\n");

		list_del_init(&g_pages[bud].list);
		
		
		
		

		//g_pages[bud].free_index = 0;
		g_pages[page].free_index++;
		
		// printf("Inside merge\n\n");
		// buddy_dump();
		// printf("leaving merge\n\n");
		merge(page, size+1);
	} 
}

void nr_merge(){

}

int find(int index){
	if(list_empty(&free_area[index]) == 0){
		return 0;
	} else {
		return 1;
	}
}
/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		g_pages[i].startaddr = i;
		g_pages[i].addr = PAGE_TO_ADDR(i);
		g_pages[i].free = 1;
		g_pages[i].free_index = 0;
		//printf("Addr: %d\n", g_pages[i].addr);
		
		
		INIT_LIST_HEAD(&g_pages[i].list);
		//list_add(&g_pages[i].list, &g_pages[0].list);
	}
	

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}
	//struct list_head *position;

	

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);

	
	
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	for(int i = 12; i < 21; i++){
		int num = 1 << i;
		if(size <= num){
			int index = find(i);
			if(index == 0){
				
				struct list_head *temporary = free_area[i].next;
				page_t *ptr = list_entry(free_area[i].next, page_t, list);
				int addr = ptr -> addr;
				int p_addr1 = buddy(addr, i);
				int p_addr2 = ptr -> addr;
				
				list_del_init(&free_area[i]);
				
				
				return ptr -> addr;
			} else {
				
				void* map = split(num, i);
				
				return map;
			}
		}

	}

	return NULL;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	int page = (void*)addr - (void*)g_memory;
	page = page >> MIN_ORDER;

	g_pages[page].free = 1;
	//list_add(&g_pages[page].list, &free_area[g_pages[page].free_index]);
	merge(page, g_pages[page].free_index);
	
	
	

}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		
		struct list_head *pos;
		
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
