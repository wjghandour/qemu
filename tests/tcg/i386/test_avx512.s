	.file	"test_avx512.c"
	.text
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	""
.LC1:
	.string	"%s%.2f, "
.LC2:
	.string	", "
.LC3:
	.string	"]\n"
	.text
	.p2align 4
	.globl	fprint_vec
	.type	fprint_vec, @function
fprint_vec:
.LFB5710:
	.cfi_startproc
	endbr64
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	leaq	.LC1(%rip), %r13
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	movq	%rdi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	leaq	.LC2(%rip), %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	movq	%rsi, %rbx
	movq	%rdi, %rsi
	movl	$91, %edi
	call	fputc@PLT
	leaq	64(%rbx), %r14
	leaq	.LC0(%rip), %rcx
	.p2align 4,,10
	.p2align 3
.L2:
	vxorpd	%xmm1, %xmm1, %xmm1
	vcvtss2sd	(%rbx), %xmm1, %xmm0
	movq	%r13, %rdx
	movl	$1, %esi
	movq	%r12, %rdi
	movl	$1, %eax
	addq	$4, %rbx
	call	__fprintf_chk@PLT
	movq	%rbp, %rcx
	cmpq	%r14, %rbx
	jne	.L2
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	movq	%r12, %rcx
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	movl	$2, %edx
	movl	$1, %esi
	leaq	.LC3(%rip), %rdi
	popq	%r14
	.cfi_def_cfa_offset 8
	jmp	fwrite@PLT
	.cfi_endproc
.LFE5710:
	.size	fprint_vec, .-fprint_vec
	.p2align 4
	.globl	eval_fmadd_parallel_16
	.type	eval_fmadd_parallel_16, @function
eval_fmadd_parallel_16:
.LFB5711:
	.cfi_startproc
	endbr64
	movq	%rsi, %rax
	testq	%rsi, %rsi
	jle	.L9
	vbroadcastss	.LC33(%rip), %zmm1
	vbroadcastss	.LC34(%rip), %zmm0
	vbroadcastss	.LC20(%rip), %zmm2
	vbroadcastss	.LC21(%rip), %zmm3
	vbroadcastss	.LC22(%rip), %zmm4
	vbroadcastss	.LC23(%rip), %zmm5
	vbroadcastss	.LC24(%rip), %zmm6
	vbroadcastss	.LC25(%rip), %zmm7
	vbroadcastss	.LC26(%rip), %zmm8
	vbroadcastss	.LC27(%rip), %zmm9
	vbroadcastss	.LC28(%rip), %zmm10
	vbroadcastss	.LC29(%rip), %zmm11
	vbroadcastss	.LC30(%rip), %zmm12
	vbroadcastss	.LC31(%rip), %zmm13
	vbroadcastss	.LC32(%rip), %zmm14
	vbroadcastss	.LC35(%rip), %zmm17
	xorl	%edx, %edx
	vmovaps	%zmm1, %zmm15
	vmovaps	%zmm0, %zmm16
	.p2align 4,,10
	.p2align 3
.L8:
	incq	%rdx
	vfmadd231ps	%zmm1, %zmm0, %zmm17
	vfmadd231ps	%zmm1, %zmm0, %zmm16
	vfmadd231ps	%zmm1, %zmm0, %zmm15
	vfmadd231ps	%zmm1, %zmm0, %zmm14
	vfmadd231ps	%zmm1, %zmm0, %zmm13
	vfmadd231ps	%zmm1, %zmm0, %zmm12
	vfmadd231ps	%zmm1, %zmm0, %zmm11
	vfmadd231ps	%zmm1, %zmm0, %zmm10
	vfmadd231ps	%zmm1, %zmm0, %zmm9
	vfmadd231ps	%zmm1, %zmm0, %zmm8
	vfmadd231ps	%zmm1, %zmm0, %zmm7
	vfmadd231ps	%zmm1, %zmm0, %zmm6
	vfmadd231ps	%zmm1, %zmm0, %zmm5
	vfmadd231ps	%zmm1, %zmm0, %zmm4
	vfmadd231ps	%zmm1, %zmm0, %zmm3
	vfmadd231ps	%zmm1, %zmm0, %zmm2
	cmpq	%rdx, %rax
	jne	.L8
.L7:
	movq	(%rdi), %rdx
	salq	$4, %rax
	vmovaps	%zmm17, (%rdx)
	vmovaps	%zmm16, 64(%rdx)
	vmovaps	%zmm15, 128(%rdx)
	vmovaps	%zmm14, 192(%rdx)
	vmovaps	%zmm13, 256(%rdx)
	vmovaps	%zmm12, 320(%rdx)
	vmovaps	%zmm11, 384(%rdx)
	vmovaps	%zmm10, 448(%rdx)
	vmovaps	%zmm9, 512(%rdx)
	vmovaps	%zmm8, 576(%rdx)
	vmovaps	%zmm7, 640(%rdx)
	vmovaps	%zmm6, 704(%rdx)
	vmovaps	%zmm5, 768(%rdx)
	vmovaps	%zmm4, 832(%rdx)
	vmovaps	%zmm3, 896(%rdx)
	vmovaps	%zmm2, 960(%rdx)
	vzeroupper
	ret
	.p2align 4,,10
	.p2align 3
.L9:
	vbroadcastss	.LC20(%rip), %zmm2
	vbroadcastss	.LC21(%rip), %zmm3
	vbroadcastss	.LC22(%rip), %zmm4
	vbroadcastss	.LC23(%rip), %zmm5
	vbroadcastss	.LC24(%rip), %zmm6
	vbroadcastss	.LC25(%rip), %zmm7
	vbroadcastss	.LC26(%rip), %zmm8
	vbroadcastss	.LC27(%rip), %zmm9
	vbroadcastss	.LC28(%rip), %zmm10
	vbroadcastss	.LC29(%rip), %zmm11
	vbroadcastss	.LC30(%rip), %zmm12
	vbroadcastss	.LC31(%rip), %zmm13
	vbroadcastss	.LC32(%rip), %zmm14
	vbroadcastss	.LC33(%rip), %zmm15
	vbroadcastss	.LC34(%rip), %zmm16
	vbroadcastss	.LC35(%rip), %zmm17
	jmp	.L7
	.cfi_endproc
.LFE5711:
	.size	eval_fmadd_parallel_16, .-eval_fmadd_parallel_16
	.section	.rodata.str1.1
.LC36:
	.string	"Num fmadd: %d\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB5712:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movl	$1024, %esi
	movl	$32, %edi
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	movq	%fs:40, %rax
	movq	%rax, 8(%rsp)
	xorl	%eax, %eax
	call	aligned_alloc@PLT
	movq	%rsp, %rdi
	movl	$10, %esi
	movq	%rax, (%rsp)
	call	eval_fmadd_parallel_16
	movl	%eax, %edx
	leaq	.LC36(%rip), %rsi
	movl	$1, %edi
	xorl	%eax, %eax
	call	__printf_chk@PLT
	xorl	%ebx, %ebx
	.p2align 4,,10
	.p2align 3
.L12:
	movq	(%rsp), %rsi
	movq	stdout(%rip), %rdi
	addq	%rbx, %rsi
	addq	$64, %rbx
	call	fprint_vec
	cmpq	$1024, %rbx
	jne	.L12
	movq	(%rsp), %rdi
	call	free@PLT
	movq	8(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L16
	addq	$16, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
.L16:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5712:
	.size	main, .-main
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC20:
	.long	1098907648
	.align 4
.LC21:
	.long	1097859072
	.align 4
.LC22:
	.long	1096810496
	.align 4
.LC23:
	.long	1095761920
	.align 4
.LC24:
	.long	1094713344
	.align 4
.LC25:
	.long	1093664768
	.align 4
.LC26:
	.long	1092616192
	.align 4
.LC27:
	.long	1091567616
	.align 4
.LC28:
	.long	1090519040
	.align 4
.LC29:
	.long	1088421888
	.align 4
.LC30:
	.long	1086324736
	.align 4
.LC31:
	.long	1084227584
	.align 4
.LC32:
	.long	1082130432
	.align 4
.LC33:
	.long	1077936128
	.align 4
.LC34:
	.long	1073741824
	.align 4
.LC35:
	.long	1065353216
	.ident	"GCC: (Ubuntu 11.3.0-1ubuntu1~22.04) 11.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
