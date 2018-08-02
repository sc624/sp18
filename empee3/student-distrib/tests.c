#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"
#include "filesys.h"
#include "terminal.h"
#include "syscalls.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

int idt_test1(){
	TEST_HEADER;

	int result = PASS;

	//    0 - Division by zero exception
	//    1 - Debug exception
	//    2 - Non maskable interrupt
	//    3 - Breakpoint exception
	//    4 - 'Into detected overflow'
	//    5 - Out of bounds exception
	//    6 - Invalid opcode exception
	//    7 - No coprocessor exception
	//    8 - Double fault (pushes an error code)
	//    9 - Coprocessor segment overrun
	//    10 - Bad TSS (pushes an error code)
	//    11 - Segment not present (pushes an error code)
	//    12 - Stack fault (pushes an error code)
	//    13 - General protection fault (pushes an error code)
	//    14 - Page fault (pushes an error code)
	//    15 - Unknown interrupt exception
	//    16 - Coprocessor fault
	//    17 - Alignment check exception
	//    18 - Machine check exception
	//    19-31 - Reserved
	asm("int $0");
	asm("int $4");
	asm("int $8");
	asm("int $12");
	asm("int $16");
	asm("int $18");


	return result;
}

/*
 * rtc_test
 *   DESCRIPTION: Tests if RTC (and PIC) is properly set
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: PASS
 *   SIDE EFFECTS: Screen flickers with garbage values at RTC freq
 */
int rtc_test() {
	RTC_init();
	return PASS;
}

/*
 * page_test
 *   DESCRIPTION: Tests if paging was properly initialized
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: PASS
 *   SIDE EFFECTS: If good dereference, print to screen. Else page fault.
 */
int page_test() {
	TEST_HEADER;

	int* ptr;
	int val;
	int result = PASS;

	// Good dereferences
    // Video Memory at 0x000B8000
    ptr = (int*) first_page_table[184];
	val = *ptr;

    printf("Succesful Dereference at: 0x%x\n",ptr);
    // Kernel at 4-MB
	ptr = (int*) page_directory[1];
	val = *ptr;
	printf("Succesful Dereference at: 0x%x\n",ptr);

	// Bad dereferences
    // Before Video Memory at 0-MB
	// ptr = NULL;
	// val = *ptr;

    // After Video Memory at 3-MB
    // ptr = (int*) 0x300000;
    // val = *ptr;

    // This points to a page at 8-MB (marked not present)
    // ptr = (int*) 0x800000;
    // val = *ptr;

	return result;
}

/*
 * syscall_test
 *   DESCRIPTION: Tests if syscall handler is properly set
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: If succesful, print "System Call!"
 */
int syscall_test() {
	TEST_HEADER;
	int result = PASS;
	asm("int $0x80");
	return result;
}

// add more tests here

/* Checkpoint 2 tests */
file_t fd_arr[MAX_OPEN_FILES];

/*
 * dir_read_test
 *   DESCRIPTION: Tests dir_open, and dir_read
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: If succesful, print a list of all files in the FS
 */
int dir_read_test() {
    int8_t buf[FILENAME_LEN+1];
    int idx;
    int fd = 2;

    //dir_open((const uint8_t*)".");
    printf("\n\n");
    clear();

    for(idx = 0; idx < boot_block.dir_count; idx++) {
        dir_read(fd,(void*) buf,FILENAME_LEN);
        buf[FILENAME_LEN] = '\0';
        printf("file_name: %s, file_type: %d, file_size: %d B \n",
                buf, (dentries+fd_arr[fd].fpos)->filetype,
                (inodes + (dentries+fd_arr[fd].fpos)->inode_num)->length);
    }

    return PASS;
}

/*
 * terminal_test
 *   DESCRIPTION: reads input from keyboard, stores in buffer, writes to terminal
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints to screen with amount of bytes written and read
 */
int terminal_test()
{
	char text[500];
	int32_t echo = terminal_read(0, &text, 500);
	printf("Read %d bytes from keyboard\n", echo);
	int32_t wr = terminal_write(0, &text, 500);
	printf("Wrote %d bytes to terminal\n", wr);
	return PASS;
}

/*
 * file_print_test
 *   DESCRIPTION: Tests file_open, file_read, and file_close
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: If succesful, print particular file to screen
 */
int file_print_test(){
    int fd = 2;
    uint32_t f_length;

    /*text file test*/
    // char filename[FILENAME_LEN] = "frame0.txt";

    /*NON text file test*/
    // char filename[FILENAME_LEN] = "pingpong";

    /* large text file test*/
    char filename[FILENAME_LEN+1] = "verylargetextwithverylongname.tx";

    clear();        //clear screen

    /* Print the whole file */
    file_open((const uint8_t*)filename, fd);        //open file to load file struct
    f_length = (inodes + fd_arr[fd].inode_num)->length;     //set file length

    // printf("file_name: %s \n", filename);
    // printf("file_length: %d B \n", f_length);

    // int8_t buffer[f_length];                                //initialize buffer array with size f_length
    //
    // printf("\n\n");
    //
    // file_read(fd, buffer, f_length);            //populate buffer with file
    // int idx;
    // for(idx = 0; idx < f_length; idx++)
    //     printf("%c", buffer[idx]);              //print all characters in buffer
    //
    // file_close(fd);             //close file fd

    /* Print part by part (half by half) */
    int8_t buffer1[40000];

    printf("\n\n");

    int idx;
    file_read(fd, buffer1, f_length/2);     //print 1st half
    for(idx = 0; idx < f_length/2; idx++)
        printf("%c", buffer1[idx]);

    file_read(fd, buffer1, f_length/2);     //print second half
    for(idx = 0; idx < f_length/2; idx++)
        printf("%c", buffer1[idx]);
    file_close(fd);

    printf("\n");

    return PASS;
}

/*
 * read_by_index_test
 *   DESCRIPTION: Read and print file by file using an index to the boot block
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: If succesful, print particular file to screen
 */
int read_by_index_test() {
    dentry_t tmp_dentry;
    int32_t file_length;
    uint8_t buf[40000];
    uint32_t idx = 10;
    int bytesRead;
    int j;

    clear();

    read_dentry_by_index(idx, &tmp_dentry);
    file_length = (inodes + tmp_dentry.inode_num)->length;

    printf("\n\n");

    printf("file_name: %s \n", tmp_dentry.filename);
    printf("file_length: %d B \n", file_length);

    /* Read entire file */
    bytesRead = read_data(tmp_dentry.inode_num, 0, buf, file_length);
    for(j = 0; j < file_length; j++)
        printf("%c", buf[j]);
    printf("\n");
    printf("Bytes Read: %d B \n",bytesRead);

    return PASS;
}

int rtc_test2() {
	RTC_open(2);
	//int32_t result;
	int i=0;
	int counter=0;
	int32_t new_frequency=2;
	printf("the  frequency   is : 2 \n");              //this test should work
	//printf("the  frequency   is : 5 \n");               //this test doesn't work
	printf("counter: %d\n",counter);
	if(new_frequency % 2 == 0){
		for(i = 0; i < 100; i++){
			const void* buf[1];         //buffer containing frequencies
			int32_t nbytes;
			int32_t fd;
			printf("the frequency is : %d\n", new_frequency/2); //print out current frequency
				counter++;
			printf("counter: %d\n",counter);
			if(i % 10 == 0){            //change frequency every 10 ticks
				nbytes= 1;
				fd = 0;
				new_frequency= new_frequency << 1;          //multiply by 2
				*(int32_t*)buf=new_frequency;               //set new frequency

				RTC_write(fd, buf, nbytes);                 //the new RTC
			}
			RTC_read();
		}
	}
	else{
	printf("\n\n");
		clear();
		printf("Frequency entered is not a power of 2.\n");
		return FAIL;
	}

	printf("counter: %d\n",counter);
	return PASS;
}

/* Checkpoint 3 tests */
int syscall_linker_test() {
	TEST_HEADER;
	int result = PASS;
	halt(1);
	return result;
}

int is_exe_test() {
    return isExe((const uint8_t*) "pingpong");
}

int do_execute_test() {
    if(execute((const uint8_t*) "shell") == -1)
        return FAIL;

    // Check if it page faults
    // int* ptr;
    // ptr = (int*) 0x08000000;
    // ptr = (int*) 0x080482E4;
    // printf("Check dereference: 0x%x \n", ptr);
    // *ptr = 1;

    return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
    /* Checkpoint 1 */
	// TEST_OUTPUT("page_test", page_test());
	// TEST_OUTPUT("idt_test1",idt_test1());
	//TEST_OUTPUT("RTC_init",rtc_test());
	// TEST_OUTPUT("syscall_test",syscall_test());

    /* Chekpoint 2 */
    // TEST_OUTPUT("RTC_init",rtc_test2());
    // TEST_OUTPUT("file_print_test", file_print_test());
    // TEST_OUTPUT("dir_read_test", dir_read_test());
    // TEST_OUTPUT("read_by_index_test", read_by_index_test());
    // TEST_OUTPUT("Terminal Test", terminal_test());

    /* Checkpoint 3 */
    // TEST_OUTPUT("Syscall Linker Test", syscall_linker_test());
    // TEST_OUTPUT("is_exe_test", is_exe_test());
    // TEST_OUTPUT("do_execute_test", do_execute_test());
}
