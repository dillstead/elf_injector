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
	.file	"startup_dump.c"
	.text
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"Arguments:\012\000"
	.align	2
.LC1:
	.ascii	"Environment:\012\000"
	.align	2
.LC2:
	.ascii	"\012\000"
	.align	2
.LC3:
	.ascii	"Auxiliary Vector:\012\000"
	.align	2
.LC4:
	.ascii	":\000"
	.align	2
.LC5:
	.ascii	"(0x\000"
	.align	2
.LC6:
	.ascii	")\012\000"
	.align	2
.LC7:
	.ascii	"0123456789\000"
	.align	2
.LC8:
	.ascii	"0123456789abcdef\000"
	.align	2
.LC9:
	.ascii	"Inject Info:\012\000"
	.text
	.align	2
	.global	start
	.arch armv6
	.syntax unified
	.arm
	.fpu vfp
	.type	start, %function
start:
	@ args = 4, pretend = 0, frame = 72
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r5, r1
	ldr	r1, .L99
	mov	r6, r0
	mov	r4, r2
	sub	sp, sp, #76
	mov	r8, r3
.LPIC0:
	add	r1, pc, r1
	mov	r7, #4
	mov	r0, #1
	mov	r2, #11
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	cmp	r6, #0
	ble	.L8
	ldr	r10, .L99+4
	sub	r5, r5, #4
.LPIC2:
	add	r10, pc, r10
	mov	r3, #0
.L7:
	ldr	r1, [r5, #4]!
	cmp	r1, #0
	moveq	r2, r1
	beq	.L5
	mov	r0, r1
	ldrb	r2, [r0], #1	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L5
	mov	r2, #0
.L6:
	ldrb	ip, [r0], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	ip, #0
	bne	.L6
.L5:
	mov	r7, #4
	mov	r0, #1
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	mov	r1, r10
	mov	r2, r0
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	add	r3, r3, r2
	cmp	r6, r3
	bne	.L7
.L8:
	ldr	r1, .L99+8
	mov	r7, #4
	mov	r0, #1
.LPIC1:
	add	r1, pc, r1
	mov	r2, #13
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r1, [r4]
	ldr	r5, .L99+12
	cmp	r1, #0
.LPIC4:
	add	r5, pc, r5
	beq	.L4
.L13:
	mov	r3, r1
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L11
	mov	r2, #0
.L12:
	ldrb	r0, [r3], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L12
.L11:
	mov	r7, #4
	mov	r0, #1
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	mov	r1, r5
	mov	r2, r0
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r1, [r4, #4]!
	cmp	r1, #0
	bne	.L13
.L4:
	ldr	r1, .L99+16
	mov	r7, #4
.LPIC3:
	add	r1, pc, r1
	mov	r0, #1
	mov	r2, #18
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r1, [r8]
	cmp	r1, #0
	beq	.L10
	ldr	r3, .L99+20
	ldr	fp, .L99+24
.LPIC6:
	add	r3, pc, r3
	str	r3, [sp]
	ldr	r3, .L99+28
	ldr	r4, .L99+32
.LPIC7:
	add	r3, pc, r3
	ldr	r6, .L99+36
	str	r3, [sp, #4]
	ldr	r3, .L99+40
.LPIC5:
	add	fp, pc, fp
	add	r5, sp, #8
.LPIC8:
	add	r4, pc, r4
.LPIC10:
	add	r6, pc, r6
	mov	r10, #0
.L14:
	umull	r0, r2, r3, r1
	cmp	r1, #9
	mov	r0, #1
	lsr	r2, r2, #3
	bls	.L29
.L15:
	umull	lr, ip, r3, r2
	cmp	r2, #9
	add	r0, r0, #1
	lsr	r2, ip, #3
	bhi	.L15
.L29:
	add	ip, r5, r0
	strb	r10, [r5, r0]
.L16:
	umull	r0, r2, r3, r1
	cmp	r1, #9
	lsr	r2, r2, #3
	add	r0, r2, r2, lsl #2
	sub	r0, r1, r0, lsl #1
	mov	r1, r2
	ldrb	r2, [r4, r0]	@ zero_extendqisi2
	strb	r2, [ip, #-1]!
	bhi	.L16
	ldrb	r2, [sp, #8]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L17
	add	r1, sp, #9
	mov	r2, #0
.L18:
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L18
.L17:
	mov	r7, #4
	mov	r0, #1
	mov	r1, r5
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	mov	r1, fp
	mov	r2, r0
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r0, [r8, #4]
	cmp	r0, #9
	umull	ip, r1, r3, r0
	lsr	r1, r1, #3
	bls	.L19
.L20:
	umull	lr, ip, r3, r1
	cmp	r1, #9
	add	r2, r2, #1
	lsr	r1, ip, #3
	bhi	.L20
.L19:
	add	ip, r5, r2
	strb	r10, [r5, r2]
.L21:
	umull	r1, r2, r3, r0
	cmp	r0, #9
	lsr	r2, r2, #3
	add	r1, r2, r2, lsl #2
	sub	r1, r0, r1, lsl #1
	mov	r0, r2
	ldrb	r2, [r4, r1]	@ zero_extendqisi2
	strb	r2, [ip, #-1]!
	bhi	.L21
	ldrb	r2, [sp, #8]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L22
	add	r1, sp, #9
	mov	r2, #0
.L23:
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L23
.L22:
	mov	r7, #4
	mov	r0, #1
	mov	r1, r5
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	ldr	r1, [sp]
	mov	r2, #3
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r2, [r8, #4]
	mov	r1, #1
	cmp	r2, #15
	lsr	r0, r2, r7
	mov	ip, r0
	bls	.L24
.L25:
	cmp	ip, #15
	add	r1, r1, #1
	lsr	ip, ip, #4
	bhi	.L25
.L24:
	strb	r10, [r5, r1]
	add	r1, r5, r1
	b	.L26
.L97:
	lsr	r0, r0, #4
.L26:
	and	ip, r2, #15
	cmp	r2, #15
	ldrb	ip, [r6, ip]	@ zero_extendqisi2
	mov	r2, r0
	strb	ip, [r1, #-1]!
	bhi	.L97
	ldrb	r2, [sp, #8]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L27
	add	r1, sp, #9
	mov	r2, #0
.L28:
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L28
.L27:
	mov	r7, #4
	mov	r0, #1
	mov	r1, r5
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	ldr	r1, [sp, #4]
	mov	r2, #2
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r1, [r8, #8]!
	cmp	r1, #0
	bne	.L14
.L10:
	ldr	r1, .L99+44
	mov	r7, #4
.LPIC11:
	add	r1, pc, r1
	mov	r0, #1
	mov	r2, #13
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r3, [sp, #112]
	ldr	r1, [r3]
	cmp	r1, #0
	beq	.L1
	ldr	r3, .L99+48
	ldr	fp, .L99+52
.LPIC13:
	add	r3, pc, r3
	str	r3, [sp]
	ldr	r3, .L99+56
	ldr	r4, .L99+60
.LPIC14:
	add	r3, pc, r3
	ldr	r6, .L99+64
	str	r3, [sp, #4]
	ldr	r8, [sp, #112]
	ldr	r3, .L99+40
.LPIC12:
	add	fp, pc, fp
.LPIC15:
	add	r4, pc, r4
.LPIC17:
	add	r6, pc, r6
	add	r5, sp, #8
	mov	r10, #0
.L31:
	umull	r0, r2, r3, r1
	cmp	r1, #9
	mov	r0, #1
	lsr	r2, r2, #3
	bls	.L46
.L32:
	cmp	r2, #9
	umull	ip, r2, r3, r2
	add	r0, r0, #1
	lsr	r2, r2, #3
	bhi	.L32
.L46:
	add	ip, r5, r0
	strb	r10, [r5, r0]
.L33:
	umull	r0, r2, r3, r1
	cmp	r1, #9
	lsr	r2, r2, #3
	add	r0, r2, r2, lsl #2
	sub	r0, r1, r0, lsl #1
	mov	r1, r2
	ldrb	r0, [r4, r0]	@ zero_extendqisi2
	strb	r0, [ip, #-1]!
	bhi	.L33
	ldrb	r2, [sp, #8]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L34
	add	r1, sp, #9
	mov	r2, #0
.L35:
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L35
.L34:
	mov	r7, #4
	mov	r0, #1
	mov	r1, r5
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	mov	r1, fp
	mov	r2, r0
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r0, [r8, #4]
	cmp	r0, #9
	umull	ip, r1, r3, r0
	lsr	r1, r1, #3
	bls	.L36
.L37:
	cmp	r1, #9
	umull	ip, r1, r3, r1
	add	r2, r2, #1
	lsr	r1, r1, #3
	bhi	.L37
.L36:
	add	ip, r5, r2
	strb	r10, [r5, r2]
.L38:
	umull	r1, r2, r3, r0
	cmp	r0, #9
	lsr	r2, r2, #3
	add	r1, r2, r2, lsl #2
	sub	r1, r0, r1, lsl #1
	mov	r0, r2
	ldrb	r1, [r4, r1]	@ zero_extendqisi2
	strb	r1, [ip, #-1]!
	bhi	.L38
	ldrb	r2, [sp, #8]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L39
	add	r1, sp, #9
	mov	r2, #0
.L40:
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L40
.L39:
	mov	r7, #4
	mov	r0, #1
	mov	r1, r5
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	ldr	r1, [sp]
	mov	r2, #3
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r2, [r8, #4]
	mov	ip, #1
	cmp	r2, #15
	lsr	r1, r2, r7
	mov	r0, r1
	bls	.L41
.L42:
	cmp	r0, #15
	add	ip, ip, #1
	lsr	r0, r0, #4
	bhi	.L42
.L41:
	add	r0, r5, ip
	strb	r10, [r5, ip]
	b	.L43
.L98:
	lsr	r1, r1, #4
.L43:
	and	ip, r2, #15
	cmp	r2, #15
	ldrb	ip, [r6, ip]	@ zero_extendqisi2
	mov	r2, r1
	strb	ip, [r0, #-1]!
	bhi	.L98
	ldrb	r2, [sp, #8]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L44
	add	r1, sp, #9
	mov	r2, #0
.L45:
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	add	r2, r2, #1
	cmp	r0, #0
	bne	.L45
.L44:
	mov	r7, #4
	mov	r0, #1
	mov	r1, r5
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	mov	r0, #1
	ldr	r1, [sp, #4]
	mov	r2, #2
	.syntax divided
@ 51 "startup_dump.c" 1
	swi #0

@ 0 "" 2
	.arm
	.syntax unified
	ldr	r1, [r8, #8]!
	cmp	r1, #0
	bne	.L31
.L1:
	add	sp, sp, #76
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L100:
	.align	2
.L99:
	.word	.LC0-(.LPIC0+8)
	.word	.LC2-(.LPIC2+8)
	.word	.LC1-(.LPIC1+8)
	.word	.LC2-(.LPIC4+8)
	.word	.LC3-(.LPIC3+8)
	.word	.LC5-(.LPIC6+8)
	.word	.LC4-(.LPIC5+8)
	.word	.LC6-(.LPIC7+8)
	.word	.LC7-(.LPIC8+8)
	.word	.LC8-(.LPIC10+8)
	.word	-858993459
	.word	.LC9-(.LPIC11+8)
	.word	.LC5-(.LPIC13+8)
	.word	.LC4-(.LPIC12+8)
	.word	.LC6-(.LPIC14+8)
	.word	.LC7-(.LPIC15+8)
	.word	.LC8-(.LPIC17+8)
	.size	start, .-start
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
