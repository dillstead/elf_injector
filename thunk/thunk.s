	.arch armv6
	.eabi_attribute 28, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"thunk.c"
	.text
	.align	2
	.global	start
	.arch armv6
	.syntax unified
	.arm
	.fpu vfp
	.type	start, %function
start:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	sub	sp, sp, #20
	mov	r6, r1
	mov	fp, r0
	mov	r7, #5
	ldr	r0, [r1]
	str	r2, [sp, #12]
	mov	r1, #0
	.syntax divided
@ 55 "thunk.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmn	r0, #1
	mov	r4, r0
	beq	.L3
	ldr	r8, .L10
	mov	r10, r3
.LPIC0:
	add	r8, pc, r8
	mov	r0, r1
	mov	r7, #192
	ldm	r8, {r1, r5}
	mov	r2, #7
	mov	r3, #2
	.syntax divided
@ 74 "thunk.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmn	r0, #1
	mov	ip, r0
	beq	.L3
	mov	r1, r6
	ldr	r6, [r8, #8]
	mov	r0, fp
	add	r6, ip, r6
	add	ip, r8, #12
	ldr	r2, [sp, #12]
	mov	r3, r10
	str	ip, [sp]
	blx	r6
	mov	r7, #91
	mov	r0, r6
	ldr	r1, [r8]
	.syntax divided
@ 55 "thunk.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
.L3:
	mov	r7, #6
	mov	r0, r4
	.syntax divided
@ 40 "thunk.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L11:
	.align	2
.L10:
	.word	.LANCHOR0-(.LPIC0+8)
	.size	start, .-start
	.align	2
	.global	_thunk_start
	.syntax unified
	.arm
	.fpu vfp
	.type	_thunk_start, %function
_thunk_start:
	@ Naked Function: prologue and epilogue provided by programmer.
	@ Volatile: function does not return.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	.syntax divided
@ 123 "thunk.c" 1
	push    {r0-r11, lr}
ldr     r0, [sp, #52]
add     r1, sp, #56
add     r2, r1, r0, lsl #2
mov     r3, r2
add     r2, r2, #4
nz:
add     r3, r3, #4
ldr     r4, [r3]
cmp     r4, #0
bne     nz
add     r3, r3, #4
bl      start
pop     {r0-r11, lr}
ldr     ip, host_entry
bx      ip
.align  2
host_entry:
.word   4

@ 0 "" 2
	.arm
	.syntax unified
	.size	_thunk_start, .-_thunk_start
	.global	ii
	.global	chunk_entry_off
	.global	chunk_len
	.global	chunk_off
	.data
	.align	2
	.set	.LANCHOR0,. + 0
	.type	chunk_len, %object
	.size	chunk_len, 4
chunk_len:
	.word	2
	.type	chunk_off, %object
	.size	chunk_off, 4
chunk_off:
	.word	1
	.type	chunk_entry_off, %object
	.size	chunk_entry_off, 4
chunk_entry_off:
	.word	3
	.type	ii, %object
	.size	ii, 32
ii:
	.word	1
	.word	5
	.word	2
	.word	6
	.word	3
	.word	7
	.word	0
	.word	8
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
