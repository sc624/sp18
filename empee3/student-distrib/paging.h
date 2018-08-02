#ifndef PAGING_H
#define PAGING_H

#include "x86_desc.h"
#include "types.h"
#include "lib.h"

/* Physical memory addresses to map to */
#define VIDEO_MEMORY    0x000B8000  // 4-kB page within 0-MB to 4-MB range
#define KERNEL_PAGE     0x00400000  // 4-MB to 8-MB (4-MB page)
#define NOT_PRESENT     0x00800000  // 8-MB to 4-GB

#define FIRST_USER      0x00800000

#define FOUR_MB         0x00400000
#define FOUR_KB         0x00001000
#define EIGHT_MB        0x00800000
#define EIGHT_KB        0x00002000
#define TWO_GB          0x80000000

#define USER_STACK      0x083FFFFC
#define USER_PAGE_START 0x08000000
#define USER_PAGE_END   0x08400000

/* Initializes paging  */
extern void paging_init();

/* Sets all page directory entries to not present */
extern void blank_page_directory();

/* Set all entries in table to not present */
extern void blank_table(uint32_t* table);

/* Loads page directory into CR3 (PDBR) */
extern void loadPageDirectory(uint32_t ptr);

/* Enables 4-MB pages and then enables paging  */
extern void enablePaging();

/* Flush the TLB */
extern void flushTLB();

/* Both are 4kB aligned (Temporary Solution)
 * Write a page allocator for proper solution
 */
extern uint32_t page_directory[1024] __attribute__((aligned(4096)));
extern uint32_t first_page_table[1024] __attribute__((aligned(4096)));
extern uint32_t user_vidmem_page_table[1024] __attribute__((aligned(4096)));

#endif
