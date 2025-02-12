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
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r3, #0
	ldr	r4, .L22
	mov	r10, r0
.LPIC0:
	add	r4, pc, r4
	mov	r8, r1
.L3:
	mov	r7, #4
	mov	r0, #1
	add	r1, r4, r3
	rsb	r2, r3, #13
	.syntax divided
@ 43 "greeting.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmp	r0, #0
	add	r3, r3, r0
	ble	.L2
	cmp	r3, #12
	ble	.L3
.L2:
	cmp	r10, #0
	pople	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
	ldr	fp, .L22+4
	sub	r8, r8, #4
.LPIC1:
	add	fp, pc, fp
	mov	r6, #0
.L6:
	ldr	r5, [r8, #4]!
	cmp	r5, #0
	beq	.L8
	mov	r2, r5
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L8
	mov	r4, #0
.L9:
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	add	r4, r4, #1
	cmp	r3, #0
	bne	.L9
	b	.L10
.L21:
	cmp	r4, r3
	ble	.L8
.L10:
	mov	r7, #4
	mov	r0, #1
	add	r1, r5, r3
	sub	r2, r4, r3
	.syntax divided
@ 43 "greeting.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmp	r0, #0
	add	r3, r3, r0
	bgt	.L21
.L8:
	mov	r0, #1
	mov	r7, #4
	mov	r1, fp
	mov	r2, r0
	.syntax divided
@ 43 "greeting.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	add	r6, r6, r2
	cmp	r10, r6
	bne	.L6
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L23:
	.align	2
.L22:
	.word	.LC0-(.LPIC0+8)
	.word	.LC1-(.LPIC1+8)
	.size	_start, .-_start
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
