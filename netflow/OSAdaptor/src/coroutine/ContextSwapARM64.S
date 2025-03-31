/*!
    \brief ARM/ARM64 context switch， 保存和恢复寄存器状态
    \file coctx_swap_arm64.S
    \details 这个要与coctx_t匹配，libco只切换了寄存器和栈顶指针，对比ucontext少了浮点数上下文与sigmask（信号屏蔽掩码）
*/

.globl coctx_swap
.type  coctx_swap, @function
coctx_swap:

#if defined(__arm__)  /** copied from https://swtch.com/libtask/asm.S */
.globl setmcontext
setmcontext:
	str	r1, [r0,#4]
	str	r2, [r0,#8]
	str	r3, [r0,#12]
	str	r4, [r0,#16]
	str	r5, [r0,#20]
	str	r6, [r0,#24]
	str	r7, [r0,#28]
	str	r8, [r0,#32]
	str	r9, [r0,#36]
	str	r10, [r0,#40]
	str	r11, [r0,#44]
	str	r12, [r0,#48]
	str	r13, [r0,#52]
	str	r14, [r0,#56]
	/* store 1 as r0-to-restore */
	mov	r1, #1
	str	r1, [r0]
	/* return 0 */
	mov	r0, #0
	mov	pc, lr

.globl getmcontext
getmcontext:
	ldr	r1, [r0,#4]
	ldr	r2, [r0,#8]
	ldr	r3, [r0,#12]
	ldr	r4, [r0,#16]
	ldr	r5, [r0,#20]
	ldr	r6, [r0,#24]
	ldr	r7, [r0,#28]
	ldr	r8, [r0,#32]
	ldr	r9, [r0,#36]
	ldr	r10, [r0,#40]
	ldr	r11, [r0,#44]
	ldr	r12, [r0,#48]
	ldr	r13, [r0,#52]
	ldr	r14, [r0,#56]
	ldr	r0, [r0]
	mov	pc, lr
//    // 保存当前协程的寄存器到原结构体
//    push {r4-r11, lr}
//    str sp, [r0]          // 保存sp到原协程结构体
//
//    // 加载目标协程的寄存器值
//    ldr sp, [r1]          // 加载目标协程的sp
//    ldmia sp!, {r4-r11, lr}
//
//    // 设置目标协程返回地址
//    bx lr                 // 跳转到目标执行流


/** copied from： https://github.com/kurocha/coroutine-arm64/blob/master/source/Coroutine/Context.s */
#elif defined(__aarch64__)
# Make space on the stack for caller registers
	sub sp, sp, 0xb0

	# Save caller registers
	stp d8, d9, [sp, 0x00]
	stp d10, d11, [sp, 0x10]
	stp d12, d13, [sp, 0x20]
	stp d14, d15, [sp, 0x30]
	stp x19, x20, [sp, 0x40]
	stp x21, x22, [sp, 0x50]
	stp x23, x24, [sp, 0x60]
	stp x25, x26, [sp, 0x70]
	stp x27, x28, [sp, 0x80]
	stp x29, x30, [sp, 0x90]

	# Save return address
	str x30, [sp, 0xa0]

	# Save stack pointer to x0 (first argument)
	mov x2, sp
	str x2, [x0, 0]

	# Load stack pointer from x1 (second argument)
	ldr x3, [x1, 0]
	mov sp, x3

	# Restore caller registers
	ldp d8, d9, [sp, 0x00]
	ldp d10, d11, [sp, 0x10]
	ldp d12, d13, [sp, 0x20]
	ldp d14, d15, [sp, 0x30]
	ldp x19, x20, [sp, 0x40]
	ldp x21, x22, [sp, 0x50]
	ldp x23, x24, [sp, 0x60]
	ldp x25, x26, [sp, 0x70]
	ldp x27, x28, [sp, 0x80]
	ldp x29, x30, [sp, 0x90]

	# Load return address into x4
	ldr x4, [sp, 0xa0]

	# Pop stack frame
	add sp, sp, 0xb0

	# Jump to return address (in x4)
	ret x4
//    // 保存当前协程的寄存器到原结构体
//    stp x19, x20, [x0]
//    stp x21, x22, [x0, #16]
//    stp x23, x24, [x0, #32]
//    stp x25, x26, [x0, #48]
//    stp x27, x28, [x0, #64]
//    stp x29, lr, [x0, #80]
//    mov x9, sp
//    str x9, [x0, #96]
//
//    // 加载目标协程的寄存器值
//    ldr x9, [x1, #96]
//    mov sp, x9
//    ldp x19, x20, [x1]
//    ldp x21, x22, [x1, #16]
//    ldp x23, x24, [x1, #32]
//    ldp x25, x26, [x1, #48]
//    ldp x27, x28, [x1, #64]
//    ldp x29, lr, [x1, #80]
//
//    // 设置目标协程返回地址
//    ret
#endif
