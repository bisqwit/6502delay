; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Y, S, and Z+N
; Will preserve A, C, D, V, and I (interrupt-disable flag)
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

; Requires a disposable zeropage variable @zptemp.
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
; - 7 bytes or less when n <= 36
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 301
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.0, max=10
; - When 301 <= n <= 500: min=9, avg=11.0, max=11
; - When 501 <= n <= 750: min=9, avg=11.0, max=11
; - When 751 <= n <= 1000: min=9, avg=11.0, max=11
; - When 1001 <= n <= 2000: min=9, avg=11.0, max=11
; - When 2001 <= n <= 3000: min=9, avg=11.0, max=11
; - When 3001 <= n <= 5000: min=9, avg=11.0, max=11
; - When 5001 <= n <= 7500: min=9, avg=11.0, max=11
; - When 7501 <= n <= 10000: min=9, avg=11.0, max=11
; - When 10001 <= n <= 20000: min=9, avg=11.0, max=11
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
.elseif(n = 4 || n = 8 || n = 17 || n = 28 || n = 41 || n = 43 || n = 299 || n = 300 || n = 302 || n = 305 || n = 308)
	; 2..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif((n > 8 && n < 12) || (n > 19 && n < 23) || (n > 32 && n < 36) || n = 37 || n = 46)
	; 3..8 bytes
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
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 36 || n = 39 || n = 42 || n = 45)
	; 3..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 31 || n = 44)
	; 4..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 40)
	; 5..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 46 && n < 52) || (n > 52 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk
	.endrepeat
.elseif(n = 301 || n = 309)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 303)
	; 9 bytes
	php
	tay
	lax #1
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 304)
	; 11 bytes
	php
	pha
	php
	lda #254
	jsr delay_a_27_clocks
	plp
	pla
	plp
.elseif(n = 306)
	; 9 bytes
	php
	pha
	lax #1
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 307)
	; 10 bytes
	tay
	pla
	php
	lax #1
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif((n > 309 && n < 313))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.endif
.else
; 313 to 625
.if((n > 312 && n < 560) || n = 561 || (n > 566 && n < 626))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 560)
	; 9 bytes
	php
	tay
	lax #2
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 562 || n = 565)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 563)
	; 9 bytes
	php
	pha
	lax #2
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 564)
	; 10 bytes
	tay
	pla
	php
	lax #2
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 566)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 817) || n = 818 || (n > 823 && n < 1074) || n = 1075 || (n > 1080 && n < 1251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 817)
	; 9 bytes
	php
	tay
	lax #3
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 819 || n = 822 || n = 1076 || n = 1079)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 820)
	; 9 bytes
	php
	pha
	lax #3
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 821)
	; 10 bytes
	tay
	pla
	php
	lax #3
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 823 || n = 1080)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1074)
	; 9 bytes
	php
	tay
	lax #4
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 1077)
	; 9 bytes
	php
	pha
	lax #4
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1078)
	; 10 bytes
	tay
	pla
	php
	lax #4
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1331) || n = 1332 || (n > 1337 && n < 1588) || n = 1589 || (n > 1594 && n < 1845) || n = 1846 || (n > 1851 && n < 2102) || n = 2103 || (n > 2108 && n < 2359) || n = 2360 || (n > 2365 && n < 2501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1331)
	; 9 bytes
	php
	tay
	lax #5
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 1333 || n = 1336 || n = 1590 || n = 1593 || n = 1847 || n = 1850 || n = 2104 || n = 2107 || n = 2361 || n = 2364)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1334)
	; 9 bytes
	php
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1335)
	; 10 bytes
	tay
	pla
	php
	lax #5
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 1337 || n = 1594 || n = 1851 || n = 2108 || n = 2365)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1588)
	; 9 bytes
	php
	tay
	lax #6
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 1591)
	; 9 bytes
	php
	pha
	lax #6
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1592)
	; 10 bytes
	tay
	pla
	php
	lax #6
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 1845)
	; 9 bytes
	php
	tay
	lax #7
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 1848)
	; 9 bytes
	php
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 1849)
	; 10 bytes
	tay
	pla
	php
	lax #7
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 2102)
	; 9 bytes
	php
	tay
	lax #8
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 2105)
	; 9 bytes
	php
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2106)
	; 10 bytes
	tay
	pla
	php
	lax #8
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 2359)
	; 9 bytes
	php
	tay
	lax #9
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 2362)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2363)
	; 10 bytes
	tay
	pla
	php
	lax #9
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
; 2501 to 3750
.if((n > 2500 && n < 2616) || n = 2617 || (n > 2622 && n < 2873) || n = 2874 || (n > 2879 && n < 3130) || n = 3131 || (n > 3136 && n < 3387) || n = 3388 || (n > 3393 && n < 3644) || n = 3645 || (n > 3650 && n < 3751))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2616)
	; 9 bytes
	php
	tay
	lax #10
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 2618 || n = 2621 || n = 2875 || n = 2878 || n = 3132 || n = 3135 || n = 3389 || n = 3392 || n = 3646 || n = 3649)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619)
	; 9 bytes
	php
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2620)
	; 10 bytes
	tay
	pla
	php
	lax #10
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 2622 || n = 2879 || n = 3136 || n = 3393 || n = 3650)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2873)
	; 9 bytes
	php
	tay
	lax #11
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 2876)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 2877)
	; 10 bytes
	tay
	pla
	php
	lax #11
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 3130)
	; 9 bytes
	php
	tay
	lax #12
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3133)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3134)
	; 10 bytes
	tay
	pla
	php
	lax #12
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 3387)
	; 9 bytes
	php
	tay
	lax #13
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3390)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3391)
	; 10 bytes
	tay
	pla
	php
	lax #13
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 3644)
	; 9 bytes
	php
	tay
	lax #14
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3647)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3648)
	; 10 bytes
	tay
	pla
	php
	lax #14
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 3751 to 5000
.if((n > 3750 && n < 3901) || n = 3902 || (n > 3907 && n < 4158) || n = 4159 || (n > 4164 && n < 4415) || n = 4416 || (n > 4421 && n < 4672) || n = 4673 || (n > 4678 && n < 4929) || n = 4930 || (n > 4935 && n < 5001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3901)
	; 9 bytes
	php
	tay
	lax #15
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 3903 || n = 3906 || n = 4160 || n = 4163 || n = 4417 || n = 4420 || n = 4674 || n = 4677 || n = 4931 || n = 4934)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3904)
	; 9 bytes
	php
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 3905)
	; 10 bytes
	tay
	pla
	php
	lax #15
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 3907 || n = 4164 || n = 4421 || n = 4678 || n = 4935)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4158)
	; 9 bytes
	php
	tay
	lax #16
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4161)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4162)
	; 10 bytes
	tay
	pla
	php
	lax #16
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 4415)
	; 9 bytes
	php
	tay
	lax #17
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4418)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4419)
	; 10 bytes
	tay
	pla
	php
	lax #17
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 4672)
	; 9 bytes
	php
	tay
	lax #18
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4675)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4676)
	; 10 bytes
	tay
	pla
	php
	lax #18
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 4929)
	; 9 bytes
	php
	tay
	lax #19
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 4932)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 4933)
	; 10 bytes
	tay
	pla
	php
	lax #19
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
; 5001 to 6250
.if((n > 5000 && n < 5186) || n = 5187 || (n > 5192 && n < 5443) || n = 5444 || (n > 5449 && n < 5700) || n = 5701 || (n > 5706 && n < 5957) || n = 5958 || (n > 5963 && n < 6214) || n = 6215 || (n > 6220 && n < 6251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5186)
	; 9 bytes
	php
	tay
	lax #20
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5188 || n = 5191 || n = 5445 || n = 5448 || n = 5702 || n = 5705 || n = 5959 || n = 5962 || n = 6216 || n = 6219)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5189)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5190)
	; 10 bytes
	tay
	pla
	php
	lax #20
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 5192 || n = 5449 || n = 5706 || n = 5963 || n = 6220)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5443)
	; 9 bytes
	php
	tay
	lax #21
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5446)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5447)
	; 10 bytes
	tay
	pla
	php
	lax #21
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 5700)
	; 9 bytes
	php
	tay
	lax #22
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5703)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5704)
	; 10 bytes
	tay
	pla
	php
	lax #22
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 5957)
	; 9 bytes
	php
	tay
	lax #23
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 5960)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 5961)
	; 10 bytes
	tay
	pla
	php
	lax #23
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 6214)
	; 9 bytes
	php
	tay
	lax #24
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6217)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6218)
	; 10 bytes
	tay
	pla
	php
	lax #24
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6471) || n = 6472 || (n > 6477 && n < 6728) || n = 6729 || (n > 6734 && n < 6985) || n = 6986 || (n > 6991 && n < 7242) || n = 7243 || (n > 7248 && n < 7499) || n = 7500)
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6471)
	; 9 bytes
	php
	tay
	lax #25
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6473 || n = 6476 || n = 6730 || n = 6733 || n = 6987 || n = 6990 || n = 7244 || n = 7247)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6474)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6475)
	; 10 bytes
	tay
	pla
	php
	lax #25
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 6477 || n = 6734 || n = 6991 || n = 7248)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6728)
	; 9 bytes
	php
	tay
	lax #26
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6731)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6732)
	; 10 bytes
	tay
	pla
	php
	lax #26
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 6985)
	; 9 bytes
	php
	tay
	lax #27
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 6988)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 6989)
	; 10 bytes
	tay
	pla
	php
	lax #27
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 7242)
	; 9 bytes
	php
	tay
	lax #28
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 7245)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7246)
	; 10 bytes
	tay
	pla
	php
	lax #28
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 7499)
	; 9 bytes
	php
	tay
	lax #29
	jsr delay_256a_x_33_clocks
	tya
	plp
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
; 7501 to 8750
.if(n = 7501 || n = 7504 || n = 7758 || n = 7761 || n = 8015 || n = 8018 || n = 8272 || n = 8275 || n = 8529 || n = 8532)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7502)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7503)
	; 10 bytes
	tay
	pla
	php
	lax #29
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 7505 || n = 7762 || n = 8019 || n = 8276 || n = 8533)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n > 7505 && n < 7756) || n = 7757 || (n > 7762 && n < 8013) || n = 8014 || (n > 8019 && n < 8270) || n = 8271 || (n > 8276 && n < 8527) || n = 8528 || (n > 8533 && n < 8751))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7756)
	; 9 bytes
	php
	tay
	lax #30
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 7759)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 7760)
	; 10 bytes
	tay
	pla
	php
	lax #30
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 8013)
	; 9 bytes
	php
	tay
	lax #31
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8016)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8017)
	; 10 bytes
	tay
	pla
	php
	lax #31
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 8270)
	; 9 bytes
	php
	tay
	lax #32
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8273)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8274)
	; 10 bytes
	tay
	pla
	php
	lax #32
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 8527)
	; 9 bytes
	php
	tay
	lax #33
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8530)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8531)
	; 10 bytes
	tay
	pla
	php
	lax #33
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8784) || n = 8785 || (n > 8790 && n < 9041) || n = 9042 || (n > 9047 && n < 9298) || n = 9299 || (n > 9304 && n < 9555) || n = 9556 || (n > 9561 && n < 9812) || n = 9813 || (n > 9818 && n < 10001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8784)
	; 9 bytes
	php
	tay
	lax #34
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 8786 || n = 8789 || n = 9043 || n = 9046 || n = 9300 || n = 9303 || n = 9557 || n = 9560 || n = 9814 || n = 9817)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8787)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 8788)
	; 10 bytes
	tay
	pla
	php
	lax #34
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 8790 || n = 9047 || n = 9304 || n = 9561 || n = 9818)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 9041)
	; 9 bytes
	php
	tay
	lax #35
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9044)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9045)
	; 10 bytes
	tay
	pla
	php
	lax #35
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 9298)
	; 9 bytes
	php
	tay
	lax #36
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9301)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9302)
	; 10 bytes
	tay
	pla
	php
	lax #36
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 9555)
	; 9 bytes
	php
	tay
	lax #37
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9558)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9559)
	; 10 bytes
	tay
	pla
	php
	lax #37
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 9812)
	; 9 bytes
	php
	tay
	lax #38
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 9815)
	; 9 bytes
	php
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 9816)
	; 10 bytes
	tay
	pla
	php
	lax #38
	jsr delay_256a_x_33_clocks
	plp
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
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10075 && n < 10326) || n = 10327 || (n > 10332 && n < 10583) || n = 10584 || (n > 10589 && n < 10840) || n = 10841 || (n > 10846 && n < 11097) || n = 11098 || (n > 11103 && n < 11251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10069)
	; 9 bytes
	php
	tay
	lax #39
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10071 || n = 10074 || n = 10328 || n = 10331 || n = 10585 || n = 10588 || n = 10842 || n = 10845 || n = 11099 || n = 11102)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10072)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10073)
	; 10 bytes
	tay
	pla
	php
	lax #39
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 10075 || n = 10332 || n = 10589 || n = 10846 || n = 11103)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10326)
	; 9 bytes
	php
	tay
	lax #40
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10329)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10330)
	; 10 bytes
	tay
	pla
	php
	lax #40
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 10583)
	; 9 bytes
	php
	tay
	lax #41
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10586)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10587)
	; 10 bytes
	tay
	pla
	php
	lax #41
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 10840)
	; 9 bytes
	php
	tay
	lax #42
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 10843)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 10844)
	; 10 bytes
	tay
	pla
	php
	lax #42
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 11097)
	; 9 bytes
	php
	tay
	lax #43
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11100)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11101)
	; 10 bytes
	tay
	pla
	php
	lax #43
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 11251 to 12500
.if((n > 11250 && n < 11354) || n = 11355 || (n > 11360 && n < 11611) || n = 11612 || (n > 11617 && n < 11868) || n = 11869 || (n > 11874 && n < 12125) || n = 12126 || (n > 12131 && n < 12382) || n = 12383 || (n > 12388 && n < 12501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11354)
	; 9 bytes
	php
	tay
	lax #44
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11356 || n = 11359 || n = 11613 || n = 11616 || n = 11870 || n = 11873 || n = 12127 || n = 12130 || n = 12384 || n = 12387)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11357)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11358)
	; 10 bytes
	tay
	pla
	php
	lax #44
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 11360 || n = 11617 || n = 11874 || n = 12131 || n = 12388)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 11611)
	; 9 bytes
	php
	tay
	lax #45
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11614)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11615)
	; 10 bytes
	tay
	pla
	php
	lax #45
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 11868)
	; 9 bytes
	php
	tay
	lax #46
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 11871)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 11872)
	; 10 bytes
	tay
	pla
	php
	lax #46
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 12125)
	; 9 bytes
	php
	tay
	lax #47
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12128)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12129)
	; 10 bytes
	tay
	pla
	php
	lax #47
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 12382)
	; 9 bytes
	php
	tay
	lax #48
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12385)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12386)
	; 10 bytes
	tay
	pla
	php
	lax #48
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
; 12501 to 13750
.if((n > 12500 && n < 12639) || n = 12640 || (n > 12645 && n < 12896) || n = 12897 || (n > 12902 && n < 13153) || n = 13154 || (n > 13159 && n < 13410) || n = 13411 || (n > 13416 && n < 13667) || n = 13668 || (n > 13673 && n < 13751))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12639)
	; 9 bytes
	php
	tay
	lax #49
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12641 || n = 12644 || n = 12898 || n = 12901 || n = 13155 || n = 13158 || n = 13412 || n = 13415 || n = 13669 || n = 13672)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12642)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12643)
	; 10 bytes
	tay
	pla
	php
	lax #49
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 12645 || n = 12902 || n = 13159 || n = 13416 || n = 13673)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12896)
	; 9 bytes
	php
	tay
	lax #50
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 12899)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 12900)
	; 10 bytes
	tay
	pla
	php
	lax #50
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 13153)
	; 9 bytes
	php
	tay
	lax #51
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13156)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13157)
	; 10 bytes
	tay
	pla
	php
	lax #51
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 13410)
	; 9 bytes
	php
	tay
	lax #52
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13413)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13414)
	; 10 bytes
	tay
	pla
	php
	lax #52
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 13667)
	; 9 bytes
	php
	tay
	lax #53
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13670)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13671)
	; 10 bytes
	tay
	pla
	php
	lax #53
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 13751 to 15000
.if((n > 13750 && n < 13924) || n = 13925 || (n > 13930 && n < 14181) || n = 14182 || (n > 14187 && n < 14438) || n = 14439 || (n > 14444 && n < 14695) || n = 14696 || (n > 14701 && n < 14952) || n = 14953 || (n > 14958 && n < 15001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13924)
	; 9 bytes
	php
	tay
	lax #54
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 13926 || n = 13929 || n = 14183 || n = 14186 || n = 14440 || n = 14443 || n = 14697 || n = 14700 || n = 14954 || n = 14957)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13927)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 13928)
	; 10 bytes
	tay
	pla
	php
	lax #54
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 13930 || n = 14187 || n = 14444 || n = 14701 || n = 14958)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14181)
	; 9 bytes
	php
	tay
	lax #55
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14184)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14185)
	; 10 bytes
	tay
	pla
	php
	lax #55
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 14438)
	; 9 bytes
	php
	tay
	lax #56
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14441)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14442)
	; 10 bytes
	tay
	pla
	php
	lax #56
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 14695)
	; 9 bytes
	php
	tay
	lax #57
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14698)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14699)
	; 10 bytes
	tay
	pla
	php
	lax #57
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 14952)
	; 9 bytes
	php
	tay
	lax #58
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 14955)
	; 9 bytes
	php
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 14956)
	; 10 bytes
	tay
	pla
	php
	lax #58
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
; 15001 to 16250
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15215 && n < 15466) || n = 15467 || (n > 15472 && n < 15723) || n = 15724 || (n > 15729 && n < 15980) || n = 15981 || (n > 15986 && n < 16237) || n = 16238 || (n > 16243 && n < 16251))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15209)
	; 9 bytes
	php
	tay
	lax #59
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15211 || n = 15214 || n = 15468 || n = 15471 || n = 15725 || n = 15728 || n = 15982 || n = 15985 || n = 16239 || n = 16242)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15212)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15213)
	; 10 bytes
	tay
	pla
	php
	lax #59
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 15215 || n = 15472 || n = 15729 || n = 15986 || n = 16243)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15466)
	; 9 bytes
	php
	tay
	lax #60
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15469)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15470)
	; 10 bytes
	tay
	pla
	php
	lax #60
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 15723)
	; 9 bytes
	php
	tay
	lax #61
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15726)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15727)
	; 10 bytes
	tay
	pla
	php
	lax #61
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 15980)
	; 9 bytes
	php
	tay
	lax #62
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 15983)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 15984)
	; 10 bytes
	tay
	pla
	php
	lax #62
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 16237)
	; 9 bytes
	php
	tay
	lax #63
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 16240)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16241)
	; 10 bytes
	tay
	pla
	php
	lax #63
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 16251 to 17500
.if((n > 16250 && n < 16494) || n = 16495 || (n > 16500 && n < 16751) || n = 16752 || (n > 16757 && n < 17008) || n = 17009 || (n > 17014 && n < 17265) || n = 17266 || (n > 17271 && n < 17501))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16494)
	; 9 bytes
	php
	tay
	lax #64
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 16496 || n = 16499 || n = 16753 || n = 16756 || n = 17010 || n = 17013 || n = 17267 || n = 17270)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16497)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16498)
	; 10 bytes
	tay
	pla
	php
	lax #64
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 16500 || n = 16757 || n = 17014 || n = 17271)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16751)
	; 9 bytes
	php
	tay
	lax #65
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 16754)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 16755)
	; 10 bytes
	tay
	pla
	php
	lax #65
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 17008)
	; 9 bytes
	php
	tay
	lax #66
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17011)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17012)
	; 10 bytes
	tay
	pla
	php
	lax #66
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 17265)
	; 9 bytes
	php
	tay
	lax #67
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17268)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17269)
	; 10 bytes
	tay
	pla
	php
	lax #67
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
; 17501 to 18750
.if((n > 17500 && n < 17522) || n = 17523 || (n > 17528 && n < 17779) || n = 17780 || (n > 17785 && n < 18036) || n = 18037 || (n > 18042 && n < 18293) || n = 18294 || (n > 18299 && n < 18550) || n = 18551 || (n > 18556 && n < 18751))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17522)
	; 9 bytes
	php
	tay
	lax #68
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17524 || n = 17527 || n = 17781 || n = 17784 || n = 18038 || n = 18041 || n = 18295 || n = 18298 || n = 18552 || n = 18555)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17525)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17526)
	; 10 bytes
	tay
	pla
	php
	lax #68
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 17528 || n = 17785 || n = 18042 || n = 18299 || n = 18556)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17779)
	; 9 bytes
	php
	tay
	lax #69
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 17782)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 17783)
	; 10 bytes
	tay
	pla
	php
	lax #69
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 18036)
	; 9 bytes
	php
	tay
	lax #70
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18039)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18040)
	; 10 bytes
	tay
	pla
	php
	lax #70
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 18293)
	; 9 bytes
	php
	tay
	lax #71
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18296)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18297)
	; 10 bytes
	tay
	pla
	php
	lax #71
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 18550)
	; 9 bytes
	php
	tay
	lax #72
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18553)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18554)
	; 10 bytes
	tay
	pla
	php
	lax #72
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.else
; 18751 to 20000
.if((n > 18750 && n < 18807) || n = 18808 || (n > 18813 && n < 19064) || n = 19065 || (n > 19070 && n < 19321) || n = 19322 || (n > 19327 && n < 19578) || n = 19579 || (n > 19584 && n < 19835) || n = 19836 || (n > 19841 && n < 20001))
	; 11 bytes
	php
	pha
	lda #>(n-51)
	ldx #<(n-51)
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18807)
	; 9 bytes
	php
	tay
	lax #73
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 18809 || n = 18812 || n = 19066 || n = 19069 || n = 19323 || n = 19326 || n = 19580 || n = 19583 || n = 19837 || n = 19840)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18810)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 18811)
	; 10 bytes
	tay
	pla
	php
	lax #73
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 18813 || n = 19070 || n = 19327 || n = 19584 || n = 19841)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19064)
	; 9 bytes
	php
	tay
	lax #74
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19067)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19068)
	; 10 bytes
	tay
	pla
	php
	lax #74
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 19321)
	; 9 bytes
	php
	tay
	lax #75
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19324)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19325)
	; 10 bytes
	tay
	pla
	php
	lax #75
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 19578)
	; 9 bytes
	php
	tay
	lax #76
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19581)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19582)
	; 10 bytes
	tay
	pla
	php
	lax #76
	jsr delay_256a_x_33_clocks
	plp
	tya
.elseif(n = 19835)
	; 9 bytes
	php
	tay
	lax #77
	jsr delay_256a_x_33_clocks
	tya
	plp
.elseif(n = 19838)
	; 9 bytes
	php
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	plp
.elseif(n = 19839)
	; 10 bytes
	tay
	pla
	php
	lax #77
	jsr delay_256a_x_33_clocks
	plp
	tya
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
