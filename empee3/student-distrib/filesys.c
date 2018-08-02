#include "filesys.h"

uint32_t boot_addr;
boot_t boot_block;
dentry_t* dentries;
inode_t* inodes;

/*
 * fs_init
 *   DESCRIPTION: Initializes file system driver
 *   INPUTS: boot - Address of boot block of the file system
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Copies status bytes to global variables,
 *                 set dentries/inodes pointers, and initializes fd_arr[]
 */
void fs_init(uint32_t boot) {
    boot_addr = boot;

    /* Get status bytes */
    memcpy(&boot_block.dir_count, (unsigned int*)boot_addr, 4);
    memcpy(&boot_block.inode_count, (unsigned int*)(boot_addr + INODE_OFFSET), 4);
    memcpy(&boot_block.data_count, (unsigned int*)(boot_addr + DATA_OFFSET), 4);

    /* Dentries start after 64B stats field */
    dentries = (dentry_t*)(boot_addr + STATS_SIZE);

    /* Inodes start after boot block */
    inodes = (inode_t*)(boot_addr + BLOCK_SIZE);
}

/*
 * read_dentry_by_name
 *   DESCRIPTION: Fills in dentry with the file name file type, and inodue number for the file
 *   INPUTS: fname  - Name of the file
 *           dentry - Pointer to dentry block to copy fields to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If succesful, modifies dentry block passed in
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    uint32_t idx;
    int nameFound = 0;

    /* Piazza @864: bounds check */
    if(strlen((const int8_t*) fname) > FILENAME_LEN)
        return -1;

    /* Find dentry index in boot block  */
    for(idx = 0; idx < boot_block.dir_count; idx++) {
        int8_t byte_size = (strlen((int8_t*)(dentries+idx)->filename) > strlen((const int8_t*) fname)) ?
        strlen((int8_t*)(dentries+idx)->filename) : strlen((const int8_t*) fname);
        byte_size = (byte_size > 32) ? 32 : byte_size;
        if(strncmp((const int8_t*) fname, (int8_t*) (dentries+idx)->filename, byte_size) == 0) {
            nameFound = 1;
            break;
        }
    }

    if(nameFound)
        return read_dentry_by_index(idx, dentry);
    else
        return -1;
}

/*
 * read_dentry_by_index
 *   DESCRIPTION: Fills in dentry with the file name file type, and inodue number for the dentry at index
 *   INPUTS: index  - Index of dentry in the boot block
 *           dentry - Pointer to dentry block to copy fields to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If succesful, modifies dentry block passed in
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if(index > boot_block.dir_count)
        return -1;

    /* Use memcpy because name does not necessarily include a terminal EOS */
    memcpy(&dentry->filename, (int8_t*)(dentries+index)->filename, FILENAME_LEN);
    memcpy(&dentry->filetype, &(dentries+index)->filetype,4);
    memcpy(&dentry->inode_num, &(dentries+index)->inode_num, 4);

    return 0;
}


/*
 * read_data
 *   DESCRIPTION: populates buffer (via memcpy) with data from data blocks which are taken from the inode struct
 *   INPUTS: uint32_t inode -- inode number to read from, uint32_t offset -- start reading from this offset, uint8_t* buffer -- buffer to fill, uint32_t length -- size of file
 *   OUTPUTS: none
 *   RETURN VALUE: length -- size of the file
 *   SIDE EFFECTS: none
 */
 int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buffer, uint32_t length) {
     int idx;
     int copy_length = BLOCK_SIZE - offset;  //1st block copy size
     int data_idx = offset / BLOCK_SIZE;
     uint32_t data_blocks = (uint32_t)(boot_addr + (boot_block.inode_count) * BLOCK_SIZE + BLOCK_SIZE);  //beginning of data blocks

     //checks if inode & data block are both valid
     //inode check
     if(inode >= boot_block.inode_count)   //check if current inode is less than max inodes
        return -1;

     //inode element stored in vars
     int32_t file_length = (inodes + inode)->length;

     if(offset >= file_length)           //check if offset param is past the length of file
        return -1;

     if((length + offset) > file_length)
        length = file_length - offset;

     //data check
     if((inodes + inode)->data_block_num[data_idx] > boot_block.data_count)
        return -1;

     //set up data blocks to copy: data blocks beginning address plus data_idx of current inode * size of block plus offset
     int copy_data = (data_blocks + ((inodes + inode)->data_block_num[data_idx]) * BLOCK_SIZE + offset % BLOCK_SIZE);
     // copy_length = BLOCK_SIZE - offset;
     copy_length = (copy_length > length) ? length : copy_length;      //if copy length is larger than length, set copy length equal to length; otherwise, if it's smaller, leave copy_length alone
     copy_length = (copy_length <= 0) ? length : copy_length;          //if copy length is 0 or less, use length as copy size

     int bytes_copied = 0;

     uint8_t* og = buffer;

     for(idx = 0; idx < length; ){
         if(buffer > (og+MAX_FILE_SIZE))
            break;

         memcpy(buffer, (uint8_t*)copy_data, copy_length);   //copy file data into a buffer
         buffer = buffer + copy_length;                      //update buffer, bytes copied, & index with new lengths
         bytes_copied += copy_length;
         idx += copy_length;

         if((inodes + inode)->data_block_num[data_idx] > boot_block.data_count)
            return -1;        //data check again

         data_idx++;     //continue to next data index
         copy_data = (data_blocks + ((inodes + inode)->data_block_num[data_idx]) * BLOCK_SIZE);  //in inode param, interate through the data_block array and set equal to pointer

         if(length - bytes_copied > BLOCK_SIZE)        //if the total length is bigger than one block, we only want to copy one data block worth
            copy_length = BLOCK_SIZE;
         else
            copy_length = length - bytes_copied;        //otherwise if its smaller, we subtract the length from the number of bytes copied to get remaining number of bytes needed to copy
         if(copy_length > BLOCK_SIZE)                  //don't want to copy more than block size
            copy_length = BLOCK_SIZE;
     }
     return length;
 }


/*
 * file_open
 *   DESCRIPTION: Opens a regular file
 *   INPUTS: filename - Name of the file to be opened
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If succesful, update fd_arr[fd] fields
 */
int32_t file_open(const uint8_t* filename, int fd ) {
    pcb_t* pcb_ptr = get_pcb();
    dentry_t open_dentry;
    if(read_dentry_by_name(filename, &open_dentry) == -1)
    {
        return -1;
    }
    /* Set file object fields */
    pcb_ptr->fd_arr[fd].inode_num = open_dentry.inode_num;
    pcb_ptr->fd_arr[fd].fpos = 0;
    pcb_ptr->fd_arr[fd].flags = USED;

    return 0;
}

/*
 * file_close
 *   DESCRIPTION: Regular file closing implemented in do_close
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:
 */
int32_t file_close(int32_t fd) {
    return 0;
}

/*
 * file_read
 *   DESCRIPTION: Takes in char buffer with the file descriptor and the length of the file to print. Calls read_data to popluate the printed buffer and read_dentry_by_name
 *   INPUTS: char* buffer -- buffer to be populated, int32_t fd -- file descriptor to get position offset in file, int32_t length -- size of file to be printed
 *   OUTPUTS: none
 *   RETURN VALUE: Number of bytes read -- success, 0 -- fpos beyond or at end of file
 *   SIDE EFFECTS: none
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    int32_t bytesRead;
    int32_t readUntil = nbytes;
    uint32_t f_length;
    pcb_t* pcb_ptr = get_pcb();

    /* Check if file position is at or beyond the end of file */
    f_length = (inodes + pcb_ptr->fd_arr[fd].inode_num)->length;
    if(pcb_ptr->fd_arr[fd].fpos >= f_length)
        return 0;

    /* Read to end of the file or the end of the buf (whichever ends sooner) */
    if(f_length < nbytes)
        readUntil = f_length;

    bytesRead = read_data(pcb_ptr->fd_arr[fd].inode_num, pcb_ptr->fd_arr[fd].fpos, buf, readUntil);
    pcb_ptr->fd_arr[fd].fpos += bytesRead;

    return bytesRead;
}

/* Read-only filesystem */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/*
 * dir_open
 *   DESCRIPTION: Opens a directory file
 *   INPUTS: filename - Name of the file to be opened
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If succesful, update fd_arr[fd] fields
 */
int32_t dir_open(const uint8_t* filename, int fd) {
    pcb_t* pcb_ptr = get_pcb();
    dentry_t open_dentry;

    /* Array is full or name does not exist */
    if((read_dentry_by_name(filename, &open_dentry) == -1))
        return -1;

     /* Set file object fields */
    pcb_ptr->fd_arr[fd].inode_num = 0;
    pcb_ptr->fd_arr[fd].flags = USED;

    return 0;

}


int32_t dir_close(int32_t fd) {
    return 0;
}

/* Read-only filesystem */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/*
 * dir_read
 *   DESCRIPTION:  Reads filename in directory associated with fd into buf
 *   INPUTS: fd  - file descriptor
 *           buf - buffer to put filename in
 *        nbytes - length of filename
 *   OUTPUTS: nonex
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Modifies buf
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t * pcb_ptr = get_pcb();
    int8_t tmp[FILENAME_LEN+1];

    /* Invalid fd/buf */
    if((fd < 0) || (fd >= MAX_OPEN_FILES) || (buf == NULL))
        return -1;

    /* Check if dir is open */
    if(pcb_ptr->fd_arr[fd].flags == NOT_USED)
        return -1;

    /* Check if file position is at or beyond the end of file */
    if(pcb_ptr->fd_arr[fd].fpos >= boot_block.dir_count)
        return 0;

    /* Read files name by name in directory */
    strncpy(buf, (dentries+pcb_ptr->fd_arr[fd].fpos)->filename, nbytes);
    pcb_ptr->fd_arr[fd].fpos++;

    /* Find out how many bytes were read into the buffer */
    strncpy(tmp,buf,FILENAME_LEN+1);
    tmp[FILENAME_LEN] = '\0';

    return strlen(tmp);
}

/*
 * isExe
 *   DESCRIPTION:  Checks if the file is a valid executable
 *   INPUTS: filename  - Name of file
 *   OUTPUTS: none
 *   RETURN VALUE: If file is a valid executable return 1, else return 0
 *   SIDE EFFECTS:
 */
int32_t isExe(const uint8_t* filename) {
    dentry_t my_dentry;
    /* 4 bytes required to determine if exec */
    uint8_t buffer[4];

    /* filename not available */
    if(read_dentry_by_name(filename, &my_dentry) == -1)
        return 0;
    read_data(my_dentry.inode_num, 0, buffer, 4);

    /* First 4 bytes of an executable file represent a "magic number" @ Appendix C */
    if((buffer[0] == 0x7F) && (buffer[1] == 0x45) && (buffer[2] == 0x4c)
        && (buffer[3] == 0x46))
        return 1;

    return 0;
}

/*
 * program_load
 *   DESCRIPTION:  Loads program image to memory and finds entry point of program
 *   INPUTS: filename  - Name of executable file
 *   OUTPUTS: none
 *   RETURN VALUE: address of entry point on success, -1 on failure
 *   SIDE EFFECTS:
 */
uint32_t program_load(const uint8_t* filename) {
    dentry_t Mydentry;
    uint8_t buffer[METADATA_SIZE];

    if(read_dentry_by_name(filename, &Mydentry) == -1)
        return -1;

    int size = (inodes + Mydentry.inode_num)->length;

    /* Get program meta-data (entry-point and ELF bytes) */
    read_data(Mydentry.inode_num, 0, buffer, METADATA_SIZE);

    /* Memcpy user image to 0x080840800 */
    read_data(Mydentry.inode_num, 0,(uint8_t*) IMAGE_ADDR, size);

    /* shell entry address in hex file (little endian) = e8 82 04 08
     *                                                   24 25 26 27
     */
     // Entry point is at bytes 24 - 27 @ Docs 6.3.4
    uint8_t a = buffer[24];
    uint8_t b = buffer[25];
    uint8_t c = buffer[26];
    uint8_t d = buffer[27];

    /* Store 8 bytes into variable */
    uint32_t entry = (a<<24) | (b<<16)| (c<<8) | d;
    /* Convert little to big endian */
    entry = (entry >> 24) | ((entry << 8) & 0x00ff0000) |
            ((entry >> 8) & 0x0000ff00) | (entry << 24);

    return entry;
}
