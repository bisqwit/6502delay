; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Y, S, Z+N, C, and V
; Will preserve A, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
;
; Requires that a label @nearby_rts_14cyc is defined,
; that points _somewhere_ that contains a dummy 2-cycle
; opcode, such as NOP or LDA #1, followed by a $60 byte (RTS).
;
; Requires that a label @nearby_rts_15cyc is defined,
; that points _somewhere_ that contains a JMP to a location that does RTS.
;
; Requires a function called "delay_256a_x_33_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A:X clocks+overhead
;; Time: 256*A+X+33 clocks (including JSR)
;; Clobbers A. Preserves X,Y. Has relocations.
;;;;;;;;;;;;;;;;;;;;;;;;;
;:	; do 256-5 cycles.
;	sbc #1			; 2 cycles - Carry was set from cmp
;	pha
;	 lda #(256-5 - 27-7-2)
;	 jsr :+
;	pla
;delay_256a_x_33_clocks:
;	cmp #1			; +2
;	bcs :-			; +3 (-1)
;	; 0-255 cycles remain, overhead = 4
;	txa 			; +2; 6; +27 = 33
;:	;passthru
;<<Place the function delay_a_27_clocks immediately following here>>
;
; Requires a function called "delay_a_27_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A+27 clocks (including JSR)
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_a_27_clocks:
;        ;          ;    Cycles        Accumulator     Carry flag
;        ;          ; 0  1  2  3  4       (hex)        0 1 2 3 4
;        sec        ; 0  0  0  0  0   00 01 02 03 04   1 1 1 1 1
;:       sbc #5     ; 2  2  2  2  2   FB FC FD FE FF   0 0 0 0 0
;        bcs :-     ; 4  4  4  4  4   FB FC FD FE FF   0 0 0 0 0
;        lsr a      ; 6  6  6  6  6   7D 7E 7E 7F 7F   1 0 1 0 1
;        bcc :+     ; 8  8  8  8  8   7D 7E 7E 7F 7F   1 0 1 0 1
;:       sbc #$7E   ;10 11 10 11 10   FF FF 00 00 01   0 0 1 1 1
;        bcc :+     ;12 13 12 13 12   FF FF 00 00 01   0 0 1 1 1
;        beq :+     ;      14 15 14         00 00 01       1 1 1
;        bne :+     ;            16               01           1
;:       rts        ;15 16 17 18 19
;
; Generated code is:
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 291
; - 8 bytes or less when n <= 294
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.1, max=9
; - When 301 <= n <= 500: min=8, avg=9.0, max=9
; - When 501 <= n <= 750: min=7, avg=9.0, max=9
; - When 751 <= n <= 1000: min=7, avg=9.0, max=9
; - When 1001 <= n <= 2000: min=7, avg=9.0, max=9
; - When 2001 <= n <= 3000: min=7, avg=9.0, max=9
; - When 3001 <= n <= 5000: min=7, avg=9.0, max=9
; - When 5001 <= n <= 7500: min=7, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=7, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=7, avg=9.0, max=9
; Written by Joel Yliluoma


.macro delay_n n, rest
.assert (n = 0 || n >= 2), error, "delay_n: delay must be 0 or >= 2"
.if(n = 0)
.elseif(n < 0)
	.error .sprintf("delay_n: n cannot be < 0, you gave %d", n)
.elseif(n > 20000)
	delay_n (n/2), { delay_n (n-(n/2)) }
.else
.if((n >= 1) && (n < 10001))
.if((n >= 1) && (n < 5001))
.if((n >= 1) && (n < 2501))
.if((n >= 1) && (n < 1251))
.if((n >= 1) && (n < 626))
.if((n >= 1) && (n < 313))
; 1 to 312
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 17 || n = 28 || n = 31 || n = 292 || n = 293 || n = 298 || n = 301)
	; 2..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6)
	; 2 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif((n > 8 && n < 12) || (n > 19 && n < 23) || n = 33)
	; 3..6 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 2 bytes
	brk
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 34 || n = 35)
	; 7 bytes
	tax
	lda #n-33
	jsr delay_a_27_clocks
	txa
.elseif((n > 35 && n < 39) || (n > 39 && n < 292))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 294 || n = 302)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 295 || n = 297)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 296)
	; 7 bytes
	tay
	lax #1
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 299)
	; 7 bytes
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 300)
	; 8 bytes
	tay
	pla
	lax #1
	jsr delay_256a_x_33_clocks
	tya
.elseif((n > 302 && n < 313))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.endif
.else
; 313 to 625
.if((n > 312 && n < 553) || n = 554 || (n > 559 && n < 626))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 553)
	; 7 bytes
	tay
	lax #2
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 555 || n = 558)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 556)
	; 7 bytes
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 557)
	; 8 bytes
	tay
	pla
	lax #2
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 559)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 810) || n = 811 || (n > 816 && n < 1067) || n = 1068 || (n > 1073 && n < 1251))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 810)
	; 7 bytes
	tay
	lax #3
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 812 || n = 815 || n = 1069 || n = 1072)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 813)
	; 7 bytes
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 814)
	; 8 bytes
	tay
	pla
	lax #3
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 816 || n = 1073)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1067)
	; 7 bytes
	tay
	lax #4
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1070)
	; 7 bytes
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1071)
	; 8 bytes
	tay
	pla
	lax #4
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1324) || n = 1325 || (n > 1330 && n < 1581) || n = 1582 || (n > 1587 && n < 1838) || n = 1839 || (n > 1844 && n < 2095) || n = 2096 || (n > 2101 && n < 2352) || n = 2353 || (n > 2358 && n < 2501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1324)
	; 7 bytes
	tay
	lax #5
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1326 || n = 1329 || n = 1583 || n = 1586 || n = 1840 || n = 1843 || n = 2097 || n = 2100 || n = 2354 || n = 2357)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1327)
	; 7 bytes
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1328)
	; 8 bytes
	tay
	pla
	lax #5
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1330 || n = 1587 || n = 1844 || n = 2101 || n = 2358)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1581)
	; 7 bytes
	tay
	lax #6
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1584)
	; 7 bytes
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1585)
	; 8 bytes
	tay
	pla
	lax #6
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1838)
	; 7 bytes
	tay
	lax #7
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 1841)
	; 7 bytes
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 1842)
	; 8 bytes
	tay
	pla
	lax #7
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2095)
	; 7 bytes
	tay
	lax #8
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2098)
	; 7 bytes
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2099)
	; 8 bytes
	tay
	pla
	lax #8
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2352)
	; 7 bytes
	tay
	lax #9
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2355)
	; 7 bytes
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2356)
	; 8 bytes
	tay
	pla
	lax #9
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
; 2501 to 3750
.if((n > 2500 && n < 2609) || n = 2610 || (n > 2615 && n < 2866) || n = 2867 || (n > 2872 && n < 3123) || n = 3124 || (n > 3129 && n < 3380) || n = 3381 || (n > 3386 && n < 3637) || n = 3638 || (n > 3643 && n < 3751))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2609)
	; 7 bytes
	tay
	lax #10
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2611 || n = 2614 || n = 2868 || n = 2871 || n = 3125 || n = 3128 || n = 3382 || n = 3385 || n = 3639 || n = 3642)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2612)
	; 7 bytes
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2613)
	; 8 bytes
	tay
	pla
	lax #10
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2615 || n = 2872 || n = 3129 || n = 3386 || n = 3643)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2866)
	; 7 bytes
	tay
	lax #11
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 2869)
	; 7 bytes
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 2870)
	; 8 bytes
	tay
	pla
	lax #11
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3123)
	; 7 bytes
	tay
	lax #12
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3126)
	; 7 bytes
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3127)
	; 8 bytes
	tay
	pla
	lax #12
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3380)
	; 7 bytes
	tay
	lax #13
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3383)
	; 7 bytes
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3384)
	; 8 bytes
	tay
	pla
	lax #13
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3637)
	; 7 bytes
	tay
	lax #14
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3640)
	; 7 bytes
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3641)
	; 8 bytes
	tay
	pla
	lax #14
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 3751 to 5000
.if((n > 3750 && n < 3894) || n = 3895 || (n > 3900 && n < 4151) || n = 4152 || (n > 4157 && n < 4408) || n = 4409 || (n > 4414 && n < 4665) || n = 4666 || (n > 4671 && n < 4922) || n = 4923 || (n > 4928 && n < 5001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3894)
	; 7 bytes
	tay
	lax #15
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3896 || n = 3899 || n = 4153 || n = 4156 || n = 4410 || n = 4413 || n = 4667 || n = 4670 || n = 4924 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3897)
	; 7 bytes
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 3898)
	; 8 bytes
	tay
	pla
	lax #15
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 3900 || n = 4157 || n = 4414 || n = 4671 || n = 4928)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4151)
	; 7 bytes
	tay
	lax #16
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4154)
	; 7 bytes
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4155)
	; 8 bytes
	tay
	pla
	lax #16
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4408)
	; 7 bytes
	tay
	lax #17
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4411)
	; 7 bytes
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4412)
	; 8 bytes
	tay
	pla
	lax #17
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4665)
	; 7 bytes
	tay
	lax #18
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4668)
	; 7 bytes
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4669)
	; 8 bytes
	tay
	pla
	lax #18
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4922)
	; 7 bytes
	tay
	lax #19
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 4925)
	; 7 bytes
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 4926)
	; 8 bytes
	tay
	pla
	lax #19
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
; 5001 to 6250
.if((n > 5000 && n < 5179) || n = 5180 || (n > 5185 && n < 5436) || n = 5437 || (n > 5442 && n < 5693) || n = 5694 || (n > 5699 && n < 5950) || n = 5951 || (n > 5956 && n < 6207) || n = 6208 || (n > 6213 && n < 6251))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5179)
	; 7 bytes
	tay
	lax #20
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5181 || n = 5184 || n = 5438 || n = 5441 || n = 5695 || n = 5698 || n = 5952 || n = 5955 || n = 6209 || n = 6212)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5182)
	; 7 bytes
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5183)
	; 8 bytes
	tay
	pla
	lax #20
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5185 || n = 5442 || n = 5699 || n = 5956 || n = 6213)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5436)
	; 7 bytes
	tay
	lax #21
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5439)
	; 7 bytes
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5440)
	; 8 bytes
	tay
	pla
	lax #21
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5693)
	; 7 bytes
	tay
	lax #22
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5696)
	; 7 bytes
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5697)
	; 8 bytes
	tay
	pla
	lax #22
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5950)
	; 7 bytes
	tay
	lax #23
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 5953)
	; 7 bytes
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 5954)
	; 8 bytes
	tay
	pla
	lax #23
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6207)
	; 7 bytes
	tay
	lax #24
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6210)
	; 7 bytes
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6211)
	; 8 bytes
	tay
	pla
	lax #24
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6464) || n = 6465 || (n > 6470 && n < 6721) || n = 6722 || (n > 6727 && n < 6978) || n = 6979 || (n > 6984 && n < 7235) || n = 7236 || (n > 7241 && n < 7492) || n = 7493 || n = 7499 || n = 7500)
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6464)
	; 7 bytes
	tay
	lax #25
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6466 || n = 6469 || n = 6723 || n = 6726 || n = 6980 || n = 6983 || n = 7237 || n = 7240 || n = 7494 || n = 7497)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6467)
	; 7 bytes
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6468)
	; 8 bytes
	tay
	pla
	lax #25
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6470 || n = 6727 || n = 6984 || n = 7241 || n = 7498)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6721)
	; 7 bytes
	tay
	lax #26
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6724)
	; 7 bytes
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6725)
	; 8 bytes
	tay
	pla
	lax #26
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6978)
	; 7 bytes
	tay
	lax #27
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 6981)
	; 7 bytes
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 6982)
	; 8 bytes
	tay
	pla
	lax #27
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7235)
	; 7 bytes
	tay
	lax #28
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7238)
	; 7 bytes
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7239)
	; 8 bytes
	tay
	pla
	lax #28
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7492)
	; 7 bytes
	tay
	lax #29
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7495)
	; 7 bytes
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7496)
	; 8 bytes
	tay
	pla
	lax #29
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
; 7501 to 8750
.if((n > 7500 && n < 7749) || n = 7750 || (n > 7755 && n < 8006) || n = 8007 || (n > 8012 && n < 8263) || n = 8264 || (n > 8269 && n < 8520) || n = 8521 || (n > 8526 && n < 8751))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7749)
	; 7 bytes
	tay
	lax #30
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7751 || n = 7754 || n = 8008 || n = 8011 || n = 8265 || n = 8268 || n = 8522 || n = 8525)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 7752)
	; 7 bytes
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 7753)
	; 8 bytes
	tay
	pla
	lax #30
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 7755 || n = 8012 || n = 8269 || n = 8526)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8006)
	; 7 bytes
	tay
	lax #31
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8009)
	; 7 bytes
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8010)
	; 8 bytes
	tay
	pla
	lax #31
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8263)
	; 7 bytes
	tay
	lax #32
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8266)
	; 7 bytes
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8267)
	; 8 bytes
	tay
	pla
	lax #32
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8520)
	; 7 bytes
	tay
	lax #33
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8523)
	; 7 bytes
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8524)
	; 8 bytes
	tay
	pla
	lax #33
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8777) || n = 8778 || (n > 8783 && n < 9034) || n = 9035 || (n > 9040 && n < 9291) || n = 9292 || (n > 9297 && n < 9548) || n = 9549 || (n > 9554 && n < 9805) || n = 9806 || (n > 9811 && n < 10001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8777)
	; 7 bytes
	tay
	lax #34
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8779 || n = 8782 || n = 9036 || n = 9039 || n = 9293 || n = 9296 || n = 9550 || n = 9553 || n = 9807 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8780)
	; 7 bytes
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 8781)
	; 8 bytes
	tay
	pla
	lax #34
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 8783 || n = 9040 || n = 9297 || n = 9554 || n = 9811)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9034)
	; 7 bytes
	tay
	lax #35
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9037)
	; 7 bytes
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9038)
	; 8 bytes
	tay
	pla
	lax #35
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9291)
	; 7 bytes
	tay
	lax #36
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9294)
	; 7 bytes
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9295)
	; 8 bytes
	tay
	pla
	lax #36
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9548)
	; 7 bytes
	tay
	lax #37
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9551)
	; 7 bytes
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9552)
	; 8 bytes
	tay
	pla
	lax #37
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9805)
	; 7 bytes
	tay
	lax #38
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 9808)
	; 7 bytes
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 9809)
	; 8 bytes
	tay
	pla
	lax #38
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
.if((n >= 10001) && (n < 11251))
; 10001 to 11250
.if((n > 10000 && n < 10062) || n = 10063 || (n > 10068 && n < 10319) || n = 10320 || (n > 10325 && n < 10576) || n = 10577 || (n > 10582 && n < 10833) || n = 10834 || (n > 10839 && n < 11090) || n = 11091 || (n > 11096 && n < 11251))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10062)
	; 7 bytes
	tay
	lax #39
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10064 || n = 10067 || n = 10321 || n = 10324 || n = 10578 || n = 10581 || n = 10835 || n = 10838 || n = 11092 || n = 11095)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10065)
	; 7 bytes
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10066)
	; 8 bytes
	tay
	pla
	lax #39
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10068 || n = 10325 || n = 10582 || n = 10839 || n = 11096)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10319)
	; 7 bytes
	tay
	lax #40
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10322)
	; 7 bytes
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10323)
	; 8 bytes
	tay
	pla
	lax #40
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10576)
	; 7 bytes
	tay
	lax #41
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10579)
	; 7 bytes
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10580)
	; 8 bytes
	tay
	pla
	lax #41
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10833)
	; 7 bytes
	tay
	lax #42
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 10836)
	; 7 bytes
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 10837)
	; 8 bytes
	tay
	pla
	lax #42
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11090)
	; 7 bytes
	tay
	lax #43
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11093)
	; 7 bytes
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11094)
	; 8 bytes
	tay
	pla
	lax #43
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 11251 to 12500
.if((n > 11250 && n < 11347) || n = 11348 || (n > 11353 && n < 11604) || n = 11605 || (n > 11610 && n < 11861) || n = 11862 || (n > 11867 && n < 12118) || n = 12119 || (n > 12124 && n < 12375) || n = 12376 || (n > 12381 && n < 12501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11347)
	; 7 bytes
	tay
	lax #44
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11349 || n = 11352 || n = 11606 || n = 11609 || n = 11863 || n = 11866 || n = 12120 || n = 12123 || n = 12377 || n = 12380)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11350)
	; 7 bytes
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11351)
	; 8 bytes
	tay
	pla
	lax #44
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11353 || n = 11610 || n = 11867 || n = 12124 || n = 12381)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11604)
	; 7 bytes
	tay
	lax #45
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11607)
	; 7 bytes
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11608)
	; 8 bytes
	tay
	pla
	lax #45
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11861)
	; 7 bytes
	tay
	lax #46
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 11864)
	; 7 bytes
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 11865)
	; 8 bytes
	tay
	pla
	lax #46
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12118)
	; 7 bytes
	tay
	lax #47
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12121)
	; 7 bytes
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12122)
	; 8 bytes
	tay
	pla
	lax #47
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12375)
	; 7 bytes
	tay
	lax #48
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12378)
	; 7 bytes
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12379)
	; 8 bytes
	tay
	pla
	lax #48
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
; 12501 to 13750
.if((n > 12500 && n < 12632) || n = 12633 || (n > 12638 && n < 12889) || n = 12890 || (n > 12895 && n < 13146) || n = 13147 || (n > 13152 && n < 13403) || n = 13404 || (n > 13409 && n < 13660) || n = 13661 || (n > 13666 && n < 13751))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12632)
	; 7 bytes
	tay
	lax #49
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12634 || n = 12637 || n = 12891 || n = 12894 || n = 13148 || n = 13151 || n = 13405 || n = 13408 || n = 13662 || n = 13665)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 12635)
	; 7 bytes
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12636)
	; 8 bytes
	tay
	pla
	lax #49
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12638 || n = 12895 || n = 13152 || n = 13409 || n = 13666)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12889)
	; 7 bytes
	tay
	lax #50
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 12892)
	; 7 bytes
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 12893)
	; 8 bytes
	tay
	pla
	lax #50
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13146)
	; 7 bytes
	tay
	lax #51
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13149)
	; 7 bytes
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13150)
	; 8 bytes
	tay
	pla
	lax #51
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13403)
	; 7 bytes
	tay
	lax #52
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13406)
	; 7 bytes
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13407)
	; 8 bytes
	tay
	pla
	lax #52
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13660)
	; 7 bytes
	tay
	lax #53
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13663)
	; 7 bytes
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13664)
	; 8 bytes
	tay
	pla
	lax #53
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 13751 to 15000
.if((n > 13750 && n < 13917) || n = 13918 || (n > 13923 && n < 14174) || n = 14175 || (n > 14180 && n < 14431) || n = 14432 || (n > 14437 && n < 14688) || n = 14689 || (n > 14694 && n < 14945) || n = 14946 || (n > 14951 && n < 15001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13917)
	; 7 bytes
	tay
	lax #54
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13919 || n = 13922 || n = 14176 || n = 14179 || n = 14433 || n = 14436 || n = 14690 || n = 14693 || n = 14947 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13920)
	; 7 bytes
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 13921)
	; 8 bytes
	tay
	pla
	lax #54
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 13923 || n = 14180 || n = 14437 || n = 14694 || n = 14951)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14174)
	; 7 bytes
	tay
	lax #55
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14177)
	; 7 bytes
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14178)
	; 8 bytes
	tay
	pla
	lax #55
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14431)
	; 7 bytes
	tay
	lax #56
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14434)
	; 7 bytes
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14435)
	; 8 bytes
	tay
	pla
	lax #56
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14688)
	; 7 bytes
	tay
	lax #57
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14691)
	; 7 bytes
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14692)
	; 8 bytes
	tay
	pla
	lax #57
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14945)
	; 7 bytes
	tay
	lax #58
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 14948)
	; 7 bytes
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 14949)
	; 8 bytes
	tay
	pla
	lax #58
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
; 15001 to 16250
.if((n > 15000 && n < 15202) || n = 15203 || (n > 15208 && n < 15459) || n = 15460 || (n > 15465 && n < 15716) || n = 15717 || (n > 15722 && n < 15973) || n = 15974 || (n > 15979 && n < 16230) || n = 16231 || (n > 16236 && n < 16251))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15202)
	; 7 bytes
	tay
	lax #59
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15204 || n = 15207 || n = 15461 || n = 15464 || n = 15718 || n = 15721 || n = 15975 || n = 15978 || n = 16232 || n = 16235)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15205)
	; 7 bytes
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15206)
	; 8 bytes
	tay
	pla
	lax #59
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15208 || n = 15465 || n = 15722 || n = 15979 || n = 16236)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15459)
	; 7 bytes
	tay
	lax #60
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15462)
	; 7 bytes
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15463)
	; 8 bytes
	tay
	pla
	lax #60
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15716)
	; 7 bytes
	tay
	lax #61
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15719)
	; 7 bytes
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15720)
	; 8 bytes
	tay
	pla
	lax #61
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15973)
	; 7 bytes
	tay
	lax #62
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 15976)
	; 7 bytes
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 15977)
	; 8 bytes
	tay
	pla
	lax #62
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16230)
	; 7 bytes
	tay
	lax #63
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16233)
	; 7 bytes
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16234)
	; 8 bytes
	tay
	pla
	lax #63
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 16251 to 17500
.if((n > 16250 && n < 16487) || n = 16488 || (n > 16493 && n < 16744) || n = 16745 || (n > 16750 && n < 17001) || n = 17002 || (n > 17007 && n < 17258) || n = 17259 || (n > 17264 && n < 17501))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16487)
	; 7 bytes
	tay
	lax #64
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16489 || n = 16492 || n = 16746 || n = 16749 || n = 17003 || n = 17006 || n = 17260 || n = 17263)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16490)
	; 7 bytes
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16491)
	; 8 bytes
	tay
	pla
	lax #64
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16493 || n = 16750 || n = 17007 || n = 17264)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16744)
	; 7 bytes
	tay
	lax #65
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 16747)
	; 7 bytes
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 16748)
	; 8 bytes
	tay
	pla
	lax #65
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17001)
	; 7 bytes
	tay
	lax #66
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17004)
	; 7 bytes
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17005)
	; 8 bytes
	tay
	pla
	lax #66
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17258)
	; 7 bytes
	tay
	lax #67
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17261)
	; 7 bytes
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17262)
	; 8 bytes
	tay
	pla
	lax #67
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
; 17501 to 18750
.if((n > 17500 && n < 17515) || n = 17516 || (n > 17521 && n < 17772) || n = 17773 || (n > 17778 && n < 18029) || n = 18030 || (n > 18035 && n < 18286) || n = 18287 || (n > 18292 && n < 18543) || n = 18544 || (n > 18549 && n < 18751))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17515)
	; 7 bytes
	tay
	lax #68
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17517 || n = 17520 || n = 17774 || n = 17777 || n = 18031 || n = 18034 || n = 18288 || n = 18291 || n = 18545 || n = 18548)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 17518)
	; 7 bytes
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17519)
	; 8 bytes
	tay
	pla
	lax #68
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17521 || n = 17778 || n = 18035 || n = 18292 || n = 18549)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17772)
	; 7 bytes
	tay
	lax #69
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 17775)
	; 7 bytes
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 17776)
	; 8 bytes
	tay
	pla
	lax #69
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18029)
	; 7 bytes
	tay
	lax #70
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18032)
	; 7 bytes
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18033)
	; 8 bytes
	tay
	pla
	lax #70
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18286)
	; 7 bytes
	tay
	lax #71
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18289)
	; 7 bytes
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18290)
	; 8 bytes
	tay
	pla
	lax #71
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18543)
	; 7 bytes
	tay
	lax #72
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18546)
	; 7 bytes
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18547)
	; 8 bytes
	tay
	pla
	lax #72
	jsr delay_256a_x_33_clocks
	tya
.endif
.else
; 18751 to 20000
.if((n > 18750 && n < 18800) || n = 18801 || (n > 18806 && n < 19057) || n = 19058 || (n > 19063 && n < 19314) || n = 19315 || (n > 19320 && n < 19571) || n = 19572 || (n > 19577 && n < 19828) || n = 19829 || (n > 19834 && n < 20001))
	; 9 bytes
	pha
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18800)
	; 7 bytes
	tay
	lax #73
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18802 || n = 18805 || n = 19059 || n = 19062 || n = 19316 || n = 19319 || n = 19573 || n = 19576 || n = 19830 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18803)
	; 7 bytes
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 18804)
	; 8 bytes
	tay
	pla
	lax #73
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 18806 || n = 19063 || n = 19320 || n = 19577 || n = 19834)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19057)
	; 7 bytes
	tay
	lax #74
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19060)
	; 7 bytes
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19061)
	; 8 bytes
	tay
	pla
	lax #74
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19314)
	; 7 bytes
	tay
	lax #75
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19317)
	; 7 bytes
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19318)
	; 8 bytes
	tay
	pla
	lax #75
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19571)
	; 7 bytes
	tay
	lax #76
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19574)
	; 7 bytes
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19575)
	; 8 bytes
	tay
	pla
	lax #76
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19828)
	; 7 bytes
	tay
	lax #77
	jsr delay_256a_x_33_clocks
	tya
.elseif(n = 19831)
	; 7 bytes
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
.elseif(n = 19832)
	; 8 bytes
	tay
	pla
	lax #77
	jsr delay_256a_x_33_clocks
	tya
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
