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
#define MIN_ORDER 12 //min 4KB
#define MAX_ORDER 20 //max 1024 KB

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
	int free;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1]; //free list

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/
void *split(int size, int index){
	if(size == PAGE_SIZE){
		return NULL;
	}
	for(int i = 13; i < 21; i++){
		if (list_empty(&free_area[i]) == 0)
		{
			printf("big index: %d\n", index);
			printf("Index: %d\n", i);
			page_t *ptr = list_entry(free_area[i].next, page_t, list); //gets the base address of the first list
			int offset = ptr->startaddr;								  //the offset is necessary for if we're breaking apart something that doesn't start at 0
			int addr2 = offset;											  //address of the base of the piece we're breaking
			printf("Start addr: %d\n", addr2);
			int addr1 = (1 << (i - 1)) + PAGE_SIZE * offset; //the index of its physical address e.g. for i = 13, we would get 4 KB then - 1 and let's say offset = 0 we convert to page address which would be 1

			int p_addr1 = addr1 / PAGE_SIZE - 1; //gets the page address for the break point of the list
			int p_addr2 = addr2;

			//struct list_head *track = &g_pages[p_addr1].list; //hold the reference to the second half of the split
			printf("address: %d, other: %d\n", p_addr1, p_addr2);

			if (i == index + 1)
			{
				g_pages[p_addr2].free = 0;
				
				list_add(&g_pages[p_addr1].list, &free_area[i - 1]); //add this to the list head
				//list_add(&g_pages[p_addr1].list, &free_area[i]);
				printf("made it here, %d\n", g_pages[p_addr2].startaddr);

				return &g_pages[p_addr2];							  //returns the list
			}
			else
			{
				
				page_t *ptr2 = list_entry(free_area[i].next, page_t, list);
				printf("ln 102: start addr: %d\n", ptr2 -> startaddr);
				//list_move((free_area[i].next), &free_area[i - 1]);
				list_move(&free_area[i], &free_area[i - 2]);
				list_move(&g_pages[p_addr1].list, free_area[i - 1].next);
				return split(size, index);
			}
		}
	}
	
	return NULL;
	
}

void merge(void *addr){

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
		g_pages[i].free = 1;
		
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

	for (i = 0; i < n_pages; i++)
	{
		list_add(&g_pages[i].list, &g_pages[0].list);
	}
	//printf("Address of free area next vs address of g pages, %p, %p\n", free_area[MAX_ORDER].next, &g_pages[0].list);

	// list_for_each(position, free_area[MAX_ORDER].next)
	// {
	// 	page_t *tempptr = list_entry(position, page_t, list);
	// 	printf("Address: %d\n", tempptr->startaddr);
	// 	printf("%p self %p ting %p bink\n", position, position->prev, position->next);
	// }
	
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
		// printf("size: %d\n", size);
		// printf("%d\n", num);
		if(size < num){
			int index = find(i);
			if(index == 0){
				//find the page address to set this to not free
				//break off the correct size piece from memory

				//printf("wrong place\n");
				struct list_head *temporary = free_area[i].next;
				list_move(free_area[i].next, &g_pages[0].list);
				return temporary;
			} else {
				//printf("we in it: %d\n", i);
				void* map = split(num, i);
				// for(int i = 12; i < 21; i++){
				// 	if(find(i) == 0){
				// 		struct list_head *pointer;
				// 		list_for_each(pointer, free_area[i].next){
				// 			printf("pointer: %p\n", pointer);
				// 		}
				// 		printf("Yeet %d\n", i);
				// 	}
				// }
				// printf("here?\n");
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
