#include "paging.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));
uint32_t user_vidmem_page_table[1024] __attribute__((aligned(4096)));

/*
 * paging_init
 *   DESCRIPTION: Initializes paging w/
                0 to 4-MB: 4-kB pages marked not present (except VIDEO_MEMORY)
                4 to 8-MB: 4-MB page for Kernel
                8 to 4-GB: 4-MB pages marked not present
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Paging initialized
 */
void paging_init() {
    blank_page_directory();
    /* Holds the physical address to map pages to */
    unsigned int i;

    /* Attributes: supervisor level, read/write, present */
    page_directory[0] = ((uint32_t) first_page_table) | 3;

    /* Divide 0 to 4-MB into 4-kB pages
     * Fill all 1024 entries in the table, mapping 4 megabytes
     * Mark all of them not present
     */
    for(i = 0; i < 1024; i++)   {
        first_page_table[i] = 0x0;
    }

    /* Map 4-kB page (within 0 to 4-MB) to Video Memory
     * attributes: supervisor level, read/write, present
     * 184 == 0xB8 == Linear Address[21:12]
     */
    first_page_table[184] = (uint32_t) VIDEO_MEMORY | 0x3;

    /* 4-MB to 8-MB for the Kernel to physical memory at 4-MB to 8-MB
     * attributes: page size, supervisor level, read/write, present
     */
    page_directory[1] = (uint32_t) KERNEL_PAGE | 0x83;

    /* 8-MB to 4-GB marked not present */
    for(i = 2; i < 1024; i++) {
        page_directory[i] = 0x0;
    }

    /* Set up page directory and page table for user's vidmap at 2-GB
     * attributes: user level, read/write, present
     */
    page_directory[512] = ((uint32_t) user_vidmem_page_table) | 0x7;
    blank_table(user_vidmem_page_table);

    loadPageDirectory((uint32_t) page_directory);
    enablePaging();
}

/* Sets all page directory entries to not present */
void blank_page_directory() {
    int i;
    for(i = 0; i < 1024; i++)   {
        page_directory[i] = 0x0;
    }
}

/* Set all entries in table to not present */
void blank_table(uint32_t* table) {
    int i;
    for(i = 0; i < 1024; i++)   {
        table[i] = 0x0;
    }
}
