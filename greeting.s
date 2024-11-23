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
	.file	"greeting.c"
	.text
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"Hello World!\012\000"
	.align	2
.LC1:
	.ascii	"\012\000"
	.text
	.align	2
	.global	_start
	.arch armv6
	.syntax unified
	.arm
	.fpu vfp
	.type	_start, %function
_start:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r8, r1
	ldr	r10, .L22
	sub	sp, sp, #20
.LPIC0:
	add	r10, pc, r10
	mov	r6, #0
	str	r0, [sp, #8]
.L3:
	mov	r3, #0
	mov	r7, #4
	mov	r0, #1
	add	r1, r10, r6
	rsb	r2, r6, #13
	mov	r4, r3
	mov	r5, r3
	.syntax divided
@ 57 "greeting.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmp	r0, #0
	add	r6, r6, r0
	ble	.L2
	cmp	r6, #12
	ble	.L3
.L2:
	ldr	r3, [sp, #8]
	cmp	r3, #0
	ble	.L1
	ldr	r3, .L22+4
	mov	fp, #0
.LPIC1:
	add	r3, pc, r3
	str	r3, [sp, #12]
	sub	r3, r8, #4
	str	r3, [sp, #4]
.L6:
	ldr	r3, [sp, #4]
	ldr	r10, [r3, #4]!
	cmp	r10, #0
	str	r3, [sp, #4]
	beq	.L8
	mov	r3, r10
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L8
	mov	r8, #0
.L9:
	ldrb	r6, [r3], #1	@ zero_extendqisi2
	add	r8, r8, #1
	cmp	r6, #0
	bne	.L9
	b	.L10
.L21:
	cmp	r8, r6
	ble	.L8
.L10:
	mov	r3, #0
	mov	r7, #4
	mov	r0, #1
	add	r1, r10, r6
	sub	r2, r8, r6
	mov	r4, r3
	mov	r5, r3
	.syntax divided
@ 57 "greeting.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmp	r0, #0
	add	r6, r6, r0
	bgt	.L21
.L8:
	mov	r3, #0
	mov	r0, #1
	ldr	r1, [sp, #12]
	mov	r7, #4
	mov	r2, r0
	mov	r4, r3
	mov	r5, r3
	.syntax divided
@ 57 "greeting.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r3, [sp, #8]
	add	fp, fp, r2
	cmp	r3, fp
	bne	.L6
.L1:
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L23:
	.align	2
.L22:
	.word	.LC0-(.LPIC0+8)
	.word	.LC1-(.LPIC1+8)
	.size	_start, .-_start
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
