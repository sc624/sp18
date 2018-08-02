#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "debug.h"
#include "types.h"
#include "syscalls.h"
#include "process.h"

#define STATS_SIZE             64
#define INODE_OFFSET            4
#define DATA_OFFSET             8
#define FILENAME_LEN           32
#define BLOCK_SIZE           4096
#define MAX_FILE_SIZE       36164
#define METADATA_SIZE          30
#define IMAGE_ADDR     0x08048000

/* File types */
#define RTC_FILE        0
#define DIR_FILE        1
#define REG_FILE        2

/* From lecture notes -- Lecture 16 pg 26 */
typedef struct directory_entry{
    int8_t filename[FILENAME_LEN];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[24];
} dentry_t;

typedef struct boot_block {
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[52];
    dentry_t direntries[63];
} boot_t;

typedef struct inode{
    int32_t length;
    int32_t data_block_num [1023];
} inode_t;

extern uint32_t boot_addr;
extern boot_t boot_block;
extern dentry_t* dentries;
extern inode_t* inodes;

/* File System Utilities */

/* Initializes file system*/
extern void fs_init(uint32_t boot_addr);

/* File System Driver Functions*/
extern int32_t file_open(const uint8_t* filename, int fd );
extern int32_t file_close(int32_t fd);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/* Directory Functions */
extern int32_t dir_open(const uint8_t* filename, int fd);
extern int32_t dir_close(int32_t fd);
extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

/* Helper functions */
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buffer, uint32_t length);
int32_t isExe(const uint8_t* filename);
uint32_t program_load(const uint8_t* filename);

#endif
