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
	int free;
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
void *split(int size, int i){
	if(size == PAGE_SIZE){
		return NULL;
	}
	
		if(list_empty(&free_area[i]) == 0){
			list_move(&free_area[i], &free_area[i-1]);
			int offset = 0;//the offset is necessary for if we're breaking apart something that doesn't start at 0
			//the offset will be something we need to figure out but for now we'll just set it 0
			int addr1 = (1 << (i - 1)) - 1 + offset;//the index of its physical address e.g. for i = 13, we would get 4 KB then - 1 and let's say offset = 0 we convert to page address which would be 1
			int addr2 = addr1 - (1 << (i - 1));//address of the base of the piece we're breaking
			int p_addr1 = ADDR_TO_PAGE(addr1); //gets the page address for the break point of the list
			int p_addr2 = ADDR_TO_PAGE(addr2);
			if(addr1 + 1 - offset == size){
				g_pages[p_addr2].free = 0;
				struct list_head *track = &g_pages[p_addr1]; //hold the reference to the second half of the split
				move(track, &free_area[i-1]);//links this to the head of this index
				return &g_pages[p_addr2];//returns the list
			} else {
				return split(size/2, i-1);
			}
			
		}
	
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
		INIT_LIST_HEAD(&g_pages[i].list);
		if(i != 0){
			list_add(&g_pages[i].list, &g_pages[i-1].list);
		}
		g_pages[i].free = 1;
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

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
		if(size < num){
			int index = find(i);
			if(index == 1){
				//find the page address to set this to not free
				//break off the correct size piece from memory
			} else {
				return split(num, i);
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
