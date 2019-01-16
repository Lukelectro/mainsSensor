	.file	"test_uart.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"String stored in SRAM\n"
	.section	.text.startup,"ax",@progbits
.global	main
	.type	main, @function
main:
.LFB5:
	.file 1 "test_uart.c"
	.loc 1 30 0
	.cfi_startproc
	push r28
.LCFI0:
	.cfi_def_cfa_offset 3
	.cfi_offset 28, -2
	push r29
.LCFI1:
	.cfi_def_cfa_offset 4
	.cfi_offset 29, -3
	in r28,__SP_L__
	in r29,__SP_H__
.LCFI2:
	.cfi_def_cfa_register 28
	sbiw r28,7
.LCFI3:
	.cfi_def_cfa_offset 11
	in __tmp_reg__,__SREG__
	cli
	out __SP_H__,r29
	out __SREG__,__tmp_reg__
	out __SP_L__,r28
/* prologue: function */
/* frame size = 7 */
/* stack size = 9 */
.L__stack_usage = 9
.LVL0:
	.loc 1 43 0
	ldi r24,lo8(103)
	ldi r25,0
	call uart_init
.LVL1:
	.loc 1 48 0
/* #APP */
 ;  48 "test_uart.c" 1
	sei
 ;  0 "" 2
	.loc 1 57 0
/* #NOAPP */
	ldi r24,lo8(.LC0)
	ldi r25,hi8(.LC0)
	call uart_puts
.LVL2:
	.loc 1 62 0
	ldi r24,lo8(__c.1849)
	ldi r25,hi8(__c.1849)
	call uart_puts_p
.LVL3:
.LBB10:
.LBB11:
	.file 2 "/usr/lib/avr/include/stdlib.h"
	.loc 2 428 0
	ldi r20,lo8(10)
	movw r22,r28
	subi r22,-1
	sbci r23,-1
	ldi r24,lo8(-122)
	ldi r25,0
	call __itoa_ncheck
.LVL4:
.LBE11:
.LBE10:
	.loc 1 70 0
	movw r24,r28
	adiw r24,1
	call uart_puts
.LVL5:
	.loc 1 76 0
	ldi r24,lo8(13)
.L17:
	call uart_putc
.LVL6:
.L2:
	.loc 1 87 0
	call uart_getc
.LVL7:
	movw r16,r24
.LVL8:
	.loc 1 88 0
	sbrc r25,0
	rjmp .L2
	.loc 1 100 0
	sbrs r25,4
	rjmp .L3
	.loc 1 103 0
	ldi r24,lo8(__c.1851)
	ldi r25,hi8(__c.1851)
	call uart_puts_p
.LVL9:
.L3:
	.loc 1 105 0
	sbrs r17,3
	rjmp .L4
	.loc 1 112 0
	ldi r24,lo8(__c.1853)
	ldi r25,hi8(__c.1853)
	call uart_puts_p
.LVL10:
.L4:
	.loc 1 114 0
	sbrs r17,1
	rjmp .L5
	.loc 1 120 0
	ldi r24,lo8(__c.1855)
	ldi r25,hi8(__c.1855)
	call uart_puts_p
.LVL11:
.L5:
	.loc 1 125 0
	mov r24,r16
	rjmp .L17
	.cfi_endproc
.LFE5:
	.size	main, .-main
	.section	.progmem.data,"a",@progbits
	.type	__c.1855, @object
	.size	__c.1855, 24
__c.1855:
	.string	"Buffer overflow error: "
	.type	__c.1853, @object
	.size	__c.1853, 21
__c.1853:
	.string	"UART Overrun Error: "
	.type	__c.1851, @object
	.size	__c.1851, 19
__c.1851:
	.string	"UART Frame Error: "
	.type	__c.1849, @object
	.size	__c.1849, 24
__c.1849:
	.string	"String stored in FLASH\n"
	.text
.Letext0:
	.file 3 "uart.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x2e9
	.word	0x2
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF20
	.byte	0x1
	.long	.LASF21
	.long	.LASF22
	.long	.Ldebug_ranges0+0
	.long	0
	.long	0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF0
	.uleb128 0x3
	.byte	0x2
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.long	.LASF1
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF2
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF3
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF4
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF5
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF6
	.uleb128 0x4
	.byte	0x1
	.long	.LASF12
	.byte	0x2
	.word	0x1a2
	.byte	0x1
	.long	0xcf
	.byte	0x3
	.long	0xcf
	.uleb128 0x5
	.long	.LASF7
	.byte	0x2
	.word	0x1a2
	.long	0x30
	.uleb128 0x6
	.string	"__s"
	.byte	0x2
	.word	0x1a2
	.long	0xcf
	.uleb128 0x5
	.long	.LASF8
	.byte	0x2
	.word	0x1a2
	.long	0x30
	.uleb128 0x7
	.long	0xbd
	.uleb128 0x8
	.byte	0x1
	.long	.LASF14
	.byte	0x2
	.word	0x1a5
	.byte	0x1
	.long	0xcf
	.byte	0x1
	.uleb128 0x9
	.long	0x30
	.uleb128 0x9
	.long	0xcf
	.uleb128 0x9
	.long	0x30
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xb
	.byte	0x1
	.long	.LASF13
	.byte	0x2
	.word	0x1ab
	.byte	0x1
	.long	0xcf
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xc
	.byte	0x2
	.long	0xd5
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF9
	.uleb128 0xd
	.byte	0x1
	.long	.LASF23
	.byte	0x1
	.byte	0x1d
	.byte	0x1
	.long	0x30
	.long	.LFB5
	.long	.LFE5
	.long	.LLST0
	.byte	0x1
	.long	0x249
	.uleb128 0xe
	.string	"c"
	.byte	0x1
	.byte	0x1f
	.long	0x29
	.byte	0x6
	.byte	0x60
	.byte	0x93
	.uleb128 0x1
	.byte	0x61
	.byte	0x93
	.uleb128 0x1
	.uleb128 0xf
	.long	.LASF10
	.byte	0x1
	.byte	0x20
	.long	0x249
	.byte	0x2
	.byte	0x8c
	.sleb128 1
	.uleb128 0x10
	.string	"num"
	.byte	0x1
	.byte	0x21
	.long	0x30
	.byte	0x86
	.uleb128 0x11
	.long	0x61
	.long	.LBB10
	.long	.LBE10
	.byte	0x1
	.byte	0x45
	.long	0x17c
	.uleb128 0x12
	.long	0x8c
	.byte	0xa
	.uleb128 0x13
	.long	0x80
	.byte	0x3
	.byte	0x91
	.sleb128 -10
	.byte	0x9f
	.uleb128 0x12
	.long	0x74
	.byte	0x86
	.uleb128 0x14
	.long	.LBB11
	.long	.LBE11
	.uleb128 0x15
	.long	.LVL4
	.long	0x260
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x2
	.byte	0x8
	.byte	0x86
	.uleb128 0x16
	.byte	0x6
	.byte	0x66
	.byte	0x93
	.uleb128 0x1
	.byte	0x67
	.byte	0x93
	.uleb128 0x1
	.byte	0x2
	.byte	0x91
	.sleb128 -10
	.uleb128 0x16
	.byte	0x1
	.byte	0x64
	.byte	0x1
	.byte	0x3a
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x17
	.long	.LVL1
	.long	0x283
	.long	0x195
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x2
	.byte	0x8
	.byte	0x67
	.byte	0
	.uleb128 0x17
	.long	.LVL2
	.long	0x297
	.long	0x1b1
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x5
	.byte	0x3
	.long	.LC0
	.byte	0
	.uleb128 0x17
	.long	.LVL3
	.long	0x2b6
	.long	0x1cd
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x5
	.byte	0x3
	.long	__c.1849
	.byte	0
	.uleb128 0x17
	.long	.LVL5
	.long	0x297
	.long	0x1e6
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x2
	.byte	0x91
	.sleb128 -10
	.byte	0
	.uleb128 0x18
	.long	.LVL6
	.long	0x2ca
	.uleb128 0x18
	.long	.LVL7
	.long	0x2de
	.uleb128 0x17
	.long	.LVL9
	.long	0x2b6
	.long	0x214
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x5
	.byte	0x3
	.long	__c.1851
	.byte	0
	.uleb128 0x17
	.long	.LVL10
	.long	0x2b6
	.long	0x230
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x5
	.byte	0x3
	.long	__c.1853
	.byte	0
	.uleb128 0x15
	.long	.LVL11
	.long	0x2b6
	.uleb128 0x16
	.byte	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.byte	0x5
	.byte	0x3
	.long	__c.1855
	.byte	0
	.byte	0
	.uleb128 0x19
	.long	0xd5
	.long	0x259
	.uleb128 0x1a
	.long	0x259
	.byte	0x6
	.byte	0
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF11
	.uleb128 0x1b
	.byte	0x1
	.long	.LASF13
	.byte	0x2
	.word	0x1ab
	.byte	0x1
	.long	0xcf
	.byte	0x1
	.long	0x283
	.uleb128 0x9
	.long	0x30
	.uleb128 0x9
	.long	0xcf
	.uleb128 0x9
	.long	0x45
	.byte	0
	.uleb128 0x1c
	.byte	0x1
	.long	.LASF15
	.byte	0x3
	.byte	0x77
	.byte	0x1
	.byte	0x1
	.long	0x297
	.uleb128 0x9
	.long	0x29
	.byte	0
	.uleb128 0x1c
	.byte	0x1
	.long	.LASF16
	.byte	0x3
	.byte	0xa7
	.byte	0x1
	.byte	0x1
	.long	0x2ab
	.uleb128 0x9
	.long	0x2ab
	.byte	0
	.uleb128 0xc
	.byte	0x2
	.long	0x2b1
	.uleb128 0x1d
	.long	0xd5
	.uleb128 0x1c
	.byte	0x1
	.long	.LASF17
	.byte	0x3
	.byte	0xb5
	.byte	0x1
	.byte	0x1
	.long	0x2ca
	.uleb128 0x9
	.long	0x2ab
	.byte	0
	.uleb128 0x1c
	.byte	0x1
	.long	.LASF18
	.byte	0x3
	.byte	0x9a
	.byte	0x1
	.byte	0x1
	.long	0x2de
	.uleb128 0x9
	.long	0x45
	.byte	0
	.uleb128 0x1e
	.byte	0x1
	.long	.LASF19
	.byte	0x3
	.byte	0x92
	.byte	0x1
	.long	0x29
	.byte	0x1
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xb
	.byte	0x1
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x20
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0xa
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LFB5
	.long	.LCFI0
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 2
	.long	.LCFI0
	.long	.LCFI1
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 3
	.long	.LCFI1
	.long	.LCFI2
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 4
	.long	.LCFI2
	.long	.LCFI3
	.word	0x2
	.byte	0x8c
	.sleb128 4
	.long	.LCFI3
	.long	.LFE5
	.word	0x2
	.byte	0x8c
	.sleb128 11
	.long	0
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.word	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.word	0
	.word	0
	.long	.LFB5
	.long	.LFE5-.LFB5
	.long	0
	.long	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.LFB5
	.long	.LFE5
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF14:
	.string	"__itoa"
.LASF7:
	.string	"__val"
.LASF22:
	.string	"/home/lucas/Opslag/AVRonLinux/mainssensor/receiver/uartlibrary"
.LASF16:
	.string	"uart_puts"
.LASF20:
	.string	"GNU C 4.9.2 -fpreprocessed -mn-flash=1 -mno-skip-bug -mmcu=avr5 -gdwarf-2 -Os -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums"
.LASF15:
	.string	"uart_init"
.LASF6:
	.string	"long long unsigned int"
.LASF3:
	.string	"unsigned char"
.LASF4:
	.string	"long unsigned int"
.LASF12:
	.string	"itoa"
.LASF19:
	.string	"uart_getc"
.LASF23:
	.string	"main"
.LASF0:
	.string	"unsigned int"
.LASF21:
	.string	"test_uart.c"
.LASF11:
	.string	"sizetype"
.LASF5:
	.string	"long long int"
.LASF9:
	.string	"char"
.LASF13:
	.string	"__itoa_ncheck"
.LASF17:
	.string	"uart_puts_p"
.LASF10:
	.string	"buffer"
.LASF18:
	.string	"uart_putc"
.LASF1:
	.string	"long int"
.LASF2:
	.string	"signed char"
.LASF8:
	.string	"__radix"
	.ident	"GCC: (GNU) 4.9.2"
.global __do_copy_data
