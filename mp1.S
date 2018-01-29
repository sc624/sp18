
.data					# section declaration

        # Useful offset constants for accessing members of a
        # struct mp1_blink_struct structure
        LOCATION   = 0
        ON_CHAR    = 2
        OFF_CHAR   = 3
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

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

mp1_rtc_tasklet:
    subl $1, %eax
    cmpl 8(mp1_list_head), $0
    je count
  count:
    cmpl 10(mp1_list_head), $1
    je off
    jne on
  off:
    movl $1000, %eax
    movw 3(mp1_list_head), %cl
    pushl %eax
    pushl %cl
    call mp1_poke
    movl 6(mp1_list_head), 8(mp1_list_head)
    step
  on:
    movl $1000, %eax
    movw 2(mp1_list_head), %cl
    pushl %eax
    pushl %cl
    call mp1_poke
    movl 4(mp1_list_head), 8(mp1_list_head)
    step
  step:
    cmp 16(mp1_list_head), $0
    je fin
    jne mp1_rtc_tasklet
    fin
  fin:
    ret


mp1_ioctl:

	ret

mp1_ioctl_add:
	ret

mp1_ioctl_remove:
	ret

mp1_ioctl_find:
	ret

mp1_ioctl_sync:
	ret

.end
