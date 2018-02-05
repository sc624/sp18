
.data					# section declaration

        # Useful offset constants for accessing members of a
        # struct mp1_blink_struct structure
        LOCATION   = 0     #offset
        ON_CHAR    = 2
        OFF_CHAR   = 3
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12     #pointer

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:

	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret


                            /*---------------------------------TASKLET------------------------------------*/
/*
ebx - head pointer
edi - temp register
ecx - video reg
*/
mp1_rtc_tasklet:
  pushl %ebp                    #save old frame Pointer
  movl %esp, %ebp              #point to new frame
  pushl %edi                   #preserve registers
  pushl %ecx
  pushl %ebx
  movl mp1_list_head, %ebx          #ebx <- mp1_list_head
  cmpl $0, %ebx                     #check if mp1_list_head isn't empty
  je error

while:
  cmpl $0, %ebx                     #empty list check
  je end                            #leave
  decw COUNTDOWN(%ebx)              #countdown decrement
  cmpw $0, COUNTDOWN(%ebx)          #check if countdown = 0
  jg step                           #go to next struct

check_status:
  cmpw $0, STATUS(%ebx)             #check if 2 bytes status = zero
  je off                            #if off, turn on/if on, turn off

on:
  movb OFF_CHAR(%ebx), %cl          #display opposite char
  movzwl LOCATION(%ebx), %eax       #put location in eax for poke parameter
  imulw $2, %ax                     #apply offset *2
  call mp1_poke                     #call mp1_poke
  movw OFF_LENGTH(%ebx), %dx        #replace countdown with off length
  movw %dx, COUNTDOWN(%ebx)
  movw $0, STATUS(%ebx)             #set STATUS as off
  jmp step                          #go to next struct


off:
  movb ON_CHAR(%ebx), %cl           #display opposite char
  movzwl LOCATION(%ebx), %eax       #put location in eax for poke parameter
  imulw $2, %ax                     #apply offset *2
  call mp1_poke                     #call mp1_poke
  movw ON_LENGTH(%ebx), %dx         #replace countdown with on length
  movw %dx, COUNTDOWN(%ebx)
  movw $1, STATUS(%ebx)             #set STATUS to on
  jmp step                          #go to next struct

step:
  movl NEXT(%ebx), %ebx             #set next ptr
  jmp while                         #jump to main loop



end:
  popl %ebx                         #restore registers
  popl %ecx
  popl %edi
  leave                             #restore stack vars
  ret                               #return

error:
  movl $-1, %eax                    #set eax to return -1 when error
  jmp end


                          /*---------------------------------DISPATCHER------------------------------------*/


ioctl_jump:
	.long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync        #ioctl jump table reference

mp1_ioctl:
    movl 8(%esp), %edx              #edx <- cmd
    cmpl $4, %edx                   #check if cmd is valid (between 0 & 3)
    jae return                      #otherwise return -1
    cmpl $0, %edx
    jb return
    jmp *ioctl_jump(, %edx, 4)      #go to jump table

  return:
    movl $-1, %eax                  #return -1
    leave
    ret

                            /*---------------------------------ADD------------------------------------*/
/*
int mp1_ioctl_add(unsigned long arg)
  %eax - param/return register
  %ebx - new LOCATION
  %edx - old location
  %ecx - arg register
*/
mp1_ioctl_add:
  pushl	%ebp                    #see TASKLET
  movl	%esp, %ebp
  pushl	%edi
  pushl	%ecx
  pushl	%ebx
  movl	8(%ebp), %ecx           #arg stored in %ecx
  cmpl	$0, %ecx	              #check if valid arg stored
  je negone

  pushl	$16                     #push struct size 16
  call mp1_malloc               #call malloc
  addl $4, %esp                 #restore stack
  cmpl $0, %eax                 #malloc NULL check
  je negone                     #jump to fail

  movl %eax, %ebx               #ebx <- eax
  cmpl $0, %ebx                 #check if pointer is NULL
  je negone                     #fail

  pushl	%eax                    #save eax register
  pushl	$16                     #push struct size for copy
  pushl	8(%ebp)                 #push arg parameter
  pushl	%ebx                    #push newly allocated struct ptr
  call mp1_copy_from_user       #call copy
  addl $12, %esp                #restore values
  cmpl $0, %eax                 #check if copy worked
  jne	free                      #free allocated mem

  cmpw $0, LOCATION(%ebx)       #check LOCATION to see if valid
  jb negone                     #outside of range (below)
  cmpw $1999, LOCATION(%ebx)
  ja negone                     #outside of range (above)

  movw ON_LENGTH(%ebx), %dx	    #replace COUNTDOWN with ON_LENGTH
  movw %dx, COUNTDOWN(%ebx)
  movw $1, STATUS(%ebx)         #set STATUS to ON

  popl %edx                     #restore edx register
  movl mp1_list_head, %eax      #mp1_list_head stored into eax
  movl %eax, NEXT(%edx)         #edx holds address of next struct
  movl %edx, mp1_list_head      #mp1_list_head holds address of next struct

  movzwl LOCATION(%edx), %eax   #load LOCATION in eax parameter
  imulw $2, %ax                 #set LOCATION offset *2
  movb ON_CHAR(%edx), %cl       #low bytes in cl reg to display
  call mp1_poke                 #display char
  jmp worked                    #return 0

free:
  pushl %eax
  call 	mp1_free                #free malloc'ed mem
  popl %eax
  jmp negone                    #return -1


                          /*---------------------------------REMOVE------------------------------------*/

/*
int mp1ioctlremove(unsigned long arg)
  %edi - arg
  %edx - previous struct
  %ecx - next struct
 */
 mp1_ioctl_remove:
  pushl %ebp                      #see TASKLET
  movl %esp, %ebp
  pushl %edi
  pushl %ecx
  pushl %ebx
  pushl 8(%ebp)                   #push arg for traverse to search with
  call traverse                   #traverse LL
  addl $4, %esp                   #restore value
  cmpl $0, %eax                   #check if found struct
  je negone                       #return -1
  movl mp1_list_head, %edx        #edx <- mp1_list_head

remove_loop:
  cmpl $0, %edx                   #check if mp1_list_head NULL
  je negone                       #return -1
  cmpl %eax, %edx                 #check if desired struct head matches current struct head
  je remove_found                 #remove if equal
  movl NEXT(%edx), %edx           #set NEXT ptr as head
  jmp remove_loop                 #loop

remove_found:
  movl NEXT(%eax), %ecx        #set current NEXT ptr to next NEXT ptr
  movl %ecx, (%edx)            #set current ptr as previous ptr

free_remove:
  pushl %eax                   #preserve eax register
  call mp1_free                #free removed struct
  popl %eax                    #restore vals
  jmp worked                   #return after removing


                              /*---------------------------------FIND------------------------------------*/

/*
int mp1ioctlfind(unsigned long arg)
  %edi - arg
  %edx - previous struct
  %ecx - next struct
*/
 mp1_ioctl_find:
  pushl %ebp                  #see TASKLET
  movl %esp, %ebp
  pushl %edi
  pushl %ecx
  pushl %ebx
  movl 8(%ebp), %edi          #store arg into edi
  cmpl $0, %edi               #check if arg present
  je negone                   #return -1
  pushl %eax                  #set args for traverse
  call traverse
  addl $4, %esp               #restore stack
  cmpl $0, %eax               #check if found in traverse
  je negone                   #return -1 if not

  pushl $16			              #push mp1 copy parameters
  pushl %eax                  #save caller registers
  pushl 8(%ebp)               #pointers for struct
  call mp1_copy_to_user       #call mp1_list_head
  addl $8, %esp               #restore stack ptr
  popl %eax                   #restore eax register
  cmpl $0, %eax               #check if copy successful
  je worked                   #return 0


                              /*--------------------------------SYNC------------------------------------*/
/*
int mp1ioctlsync(unsigned long arg)
  %esi - first struct
  %edi - second struct
*/
mp1_ioctl_sync:
  pushl %ebp                  #see TASKLET
  movl %esp, %ebp
  pushl %edi
  pushl %ecx
  pushl %ebx
  movl 8(%ebp), %eax          #arg stored in eax


  movw %ax, %dx               #grab bottom two bytes & store into dx
  pushl %edx                  #push edx parameter
  call traverse               #search for struct
  addl $4, %esp               #restore vals
  cmpl $0, %eax               #check if desired struct present
  je negone                   #return -1
  movl %eax, %esi             #copy struct head into esi

  movl 8(%ebp), %eax          #refresh arg in eax
  shrl $16, %eax              #shift high 2 bytes to bottom 2 bytes
  pushl %eax                  #push eax param
  call traverse               #search for struct
  addl $4, %esp               #restore vals
  cmpl $0, %eax               #check if desired struct present
  je negone
  movl %eax, %edi             #copy struct head into edi


  movw ON_LENGTH(%edi), %ax   #copy all elements of first struct to second struct
  movw %ax, ON_LENGTH(%esi)
  movw OFF_LENGTH(%edi), %ax
  movw %ax, OFF_LENGTH(%esi)
  movw COUNTDOWN(%edi), %ax
  movw %ax, COUNTDOWN(%esi)
  movw STATUS(%edi), %ax
  movw %ax, STATUS(%esi)


check_status_sync:
  movw STATUS(%esi), %cx      #check STATUS of second struct
  cmpw $0, %cx                #if 0, turn on/if 1, turn off
  jne sync_on
  jmp sync_off

sync_on:
  movb ON_CHAR(%esi), %cl     #display opposite char
  movzwl (%esi), %eax         #store LOCATION into eax param
  imulw $2, %ax               #LOCATION *2 offset
  call mp1_poke               #display char
  jmp worked                  #return 0 success

sync_off:
  movb OFF_CHAR(%esi), %cl    #get opposite char
  movzwl (%esi), %eax         #store LOCATION into eax param
  imulw $2, %ax               #LOCATION *2 offset
  call mp1_poke               #display char
  jmp worked                  #return 0 success

                          /*---------------------------------TRAVERSE------------------------------------*/
/*
  %ax - arg/return val
  %ecx - head pointer
  %ebx - prev pointer
*/
traverse:						# list traversal used by ioctl functions
	pushl %ebp
	movl %esp, %ebp
  movw 8(%ebp), %ax           #store location arg into eax low bytes
	movl mp1_list_head, %ecx    #head pointer
  movl %ecx, %ebx             #copy ecx into ebx
  cmpw %di, LOCATION(%ecx)    #check if desired location is at head of LL
  je head

loop:
	cmpl $0, %ecx		            #NULL check
	je nada                     #return 0 for none found
	cmpw %ax, LOCATION(%ecx)    #check if found
	je found
  movl %ecx, %ebx             #set previous ptr
  movl NEXT(%ecx), %ecx       #set next ptr
  jmp loop                    #loop
head:
  movl $1, %eax               #return 1 for head of list
  jmp search_return           #leave traverse

found:
  movl %ecx, %eax             #set found head ptr as return val
  jmp search_return           #leave func
nada:
  movl $0, %eax               #return 0 for struct not found
  jmp search_return           #leave
search_return:
  leave                       #leave
  ret                         #return


/*Used across all functions for returning -1/0 with memory errors*/
worked:
  movl $0, %eax
  popl %ebx
  popl %ecx
  popl %edi
  leave
  ret

negone:
  movl $-1, %eax
  popl %ebx
  popl %ecx
  popl %edi
  leave
  ret


.end
