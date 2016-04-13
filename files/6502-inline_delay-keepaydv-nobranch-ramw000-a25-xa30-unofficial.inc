; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Z+N, and C
; Will preserve A, Y, S, D, V, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
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
; Requires a function called "delay_256x_a_30_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays X*256 clocks + overhead
;; Clobbers A,X. Preserves Y.
;; Depends on delay_a_25_clocks within short branch distance
;; Time: X*256+16 clocks (including JSR)
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_256x_a_30_clocks:
;	cpx #0
;	beq delay_a_25_clocks
;	;4 cycles done. Loop is 256 cycles
;:       pha
;	 lda #(256-7-2-2-3)
;	 jsr delay_a_25_clocks
;	pla
;	dex
;	beq delay_a_25_clocks ; count as 2
;	bne :-
;	;Loop end is -1+1 = 0 cycles. Total: 4+JSR+RTS = 16
;
; Requires a function called "delay_a_25_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A+25 clocks (including JSR)
;;;;;;;;;;;;;;;;;;;;;;;;;
;                  ;       Cycles              Accumulator         Carry flag
;                  ; 0  1  2  3  4  5  6          (hex)           0 1 2 3 4 5 6
;                  ;
;                  ; 6  6  6  6  6  6  6   00 01 02 03 04 05 06
;:      sbc #7     ; carry set by CMP
;delay_a_25_clocks:
;       cmp #7     ; 2  2  2  2  2  2  2   00 01 02 03 04 05 06   0 0 0 0 0 0 0
;       bcs :-     ; 2  2  2  2  2  2  2   00 01 02 03 04 05 06   0 0 0 0 0 0 0
;       lsr        ; 2  2  2  2  2  2  2   00 00 01 01 02 02 03   0 1 0 1 0 1 0
;       bcs *+2    ; 2  3  2  3  2  3  2   00 00 01 01 02 02 03   0 1 0 1 0 1 0
;       beq :+     ; 3  3  2  2  2  2  2   00 00 01 01 02 02 03   0 1 0 1 0 1 0
;       lsr        ;       2  2  2  2  2         00 00 01 01 01       1 1 0 0 1
;       beq @rts   ;       3  3  2  2  2         00 00 01 01 01       1 1 0 0 1
;       bcc @rts   ;             3  3  2               01 01 01           0 0 1
;:      bne @rts   ; 2  2              3   00 00             01   0 1         0
;@rts:  rts        ; 6  6  6  6  6  6  6   00 00 00 00 01 01 01   0 1 1 1 0 0 1
;; Total cycles:    25 26 27 28 29 30 31
;
; Generated code is:
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 32
; - 8 bytes or less when n <= 37
; - 9 bytes or less when n <= 296
; -10 bytes or less when n <= 298
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=3, avg=4.0, max=5
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=8.6, max=9
; - When 51 <= n <= 75: min=9, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.1, max=11
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
; 1 to 1250
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 31 || n = 32 || n = 34 || (n > 296 && n < 301) || n = 305 || n = 562 || n = 819 || n = 1076)
	; 2..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	inc $F6,X
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 9 || n = 11 || n = 19 || (n > 20 && n < 24) || n = 33 || n = 35 || n = 37)
	; 3..8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10)
	; 4 bytes
	php
	cmp $C5
	plp
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 4 bytes
	php
	inc $F6,X
	plp
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 24 || n = 25)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 20 || n = 27 || n = 29 || n = 30)
	; 5..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 36)
	; 8 bytes
	inc $F6,X
	delay_n (n-6)
.elseif((n > 37 && n < 41))
	; 9 bytes
	php
	tax
	lda #n-38
	jsr delay_a_25_clocks
	txa
	plp
.elseif((n > 40 && n < 297))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 301 || n = 302)
	; 11 bytes
	php
	pha
	php
	lda #n-48
	jsr delay_a_25_clocks
	plp
	pla
	plp
.elseif(n = 303)
	; 9 bytes
	php
	pha
	lax #1
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 304 || (n > 305 && n < 560) || n = 561 || (n > 562 && n < 817) || n = 818 || (n > 819 && n < 1074) || n = 1075 || (n > 1076 && n < 1251))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 560)
	; 9 bytes
	php
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 817)
	; 9 bytes
	php
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1074)
	; 9 bytes
	php
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1331) || n = 1332 || (n > 1333 && n < 1588) || n = 1589 || (n > 1590 && n < 1845) || n = 1846 || (n > 1847 && n < 2102) || n = 2103 || (n > 2104 && n < 2359) || n = 2360 || (n > 2361 && n < 2501))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1331)
	; 9 bytes
	php
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1333 || n = 1590 || n = 1847 || n = 2104 || n = 2361)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1588)
	; 9 bytes
	php
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 1845)
	; 9 bytes
	php
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2102)
	; 9 bytes
	php
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2359)
	; 9 bytes
	php
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2616) || n = 2617 || (n > 2618 && n < 2873) || n = 2874 || (n > 2875 && n < 3130) || n = 3131 || (n > 3132 && n < 3387) || n = 3388 || (n > 3389 && n < 3644) || n = 3645 || (n > 3646 && n < 3901) || n = 3902 || (n > 3903 && n < 4158) || n = 4159 || (n > 4160 && n < 4415) || n = 4416 || (n > 4417 && n < 4672) || n = 4673 || (n > 4674 && n < 4929) || n = 4930 || (n > 4931 && n < 5001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2616)
	; 9 bytes
	php
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 2618 || n = 2875 || n = 3132 || n = 3389 || n = 3646 || n = 3903 || n = 4160 || n = 4417 || n = 4674 || n = 4931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2873)
	; 9 bytes
	php
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3130)
	; 9 bytes
	php
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3387)
	; 9 bytes
	php
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3644)
	; 9 bytes
	php
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 3901)
	; 9 bytes
	php
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4158)
	; 9 bytes
	php
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4415)
	; 9 bytes
	php
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4672)
	; 9 bytes
	php
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 4929)
	; 9 bytes
	php
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5186) || n = 5187 || (n > 5188 && n < 5443) || n = 5444 || (n > 5445 && n < 5700) || n = 5701 || (n > 5702 && n < 5957) || n = 5958 || (n > 5959 && n < 6214) || n = 6215 || (n > 6216 && n < 6471) || n = 6472 || (n > 6473 && n < 6728) || n = 6729 || (n > 6730 && n < 6985) || n = 6986 || (n > 6987 && n < 7242) || n = 7243 || (n > 7244 && n < 7499) || n = 7500 || (n > 7501 && n < 7756) || n = 7757 || (n > 7758 && n < 8013) || n = 8014 || (n > 8015 && n < 8270) || n = 8271 || (n > 8272 && n < 8527) || n = 8528 || (n > 8529 && n < 8784) || n = 8785 || (n > 8786 && n < 9041) || n = 9042 || (n > 9043 && n < 9298) || n = 9299 || (n > 9300 && n < 9555) || n = 9556 || (n > 9557 && n < 9812) || n = 9813 || (n > 9814 && n < 10001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5186)
	; 9 bytes
	php
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5188 || n = 5445 || n = 5702 || n = 5959 || n = 6216 || n = 6473 || n = 6730 || n = 6987 || n = 7244 || n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5443)
	; 9 bytes
	php
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5700)
	; 9 bytes
	php
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 5957)
	; 9 bytes
	php
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6214)
	; 9 bytes
	php
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6471)
	; 9 bytes
	php
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6728)
	; 9 bytes
	php
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 6985)
	; 9 bytes
	php
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7242)
	; 9 bytes
	php
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7499)
	; 9 bytes
	php
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 7756)
	; 9 bytes
	php
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8013)
	; 9 bytes
	php
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8270)
	; 9 bytes
	php
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8527)
	; 9 bytes
	php
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 8784)
	; 9 bytes
	php
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9041)
	; 9 bytes
	php
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9298)
	; 9 bytes
	php
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9555)
	; 9 bytes
	php
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 9812)
	; 9 bytes
	php
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10071 && n < 10326) || n = 10327 || (n > 10328 && n < 10583) || n = 10584 || (n > 10585 && n < 10840) || n = 10841 || (n > 10842 && n < 11097) || n = 11098 || (n > 11099 && n < 11354) || n = 11355 || (n > 11356 && n < 11611) || n = 11612 || (n > 11613 && n < 11868) || n = 11869 || (n > 11870 && n < 12125) || n = 12126 || (n > 12127 && n < 12382) || n = 12383 || (n > 12384 && n < 12639) || n = 12640 || (n > 12641 && n < 12896) || n = 12897 || (n > 12898 && n < 13153) || n = 13154 || (n > 13155 && n < 13410) || n = 13411 || (n > 13412 && n < 13667) || n = 13668 || (n > 13669 && n < 13924) || n = 13925 || (n > 13926 && n < 14181) || n = 14182 || (n > 14183 && n < 14438) || n = 14439 || (n > 14440 && n < 14695) || n = 14696 || (n > 14697 && n < 14952) || n = 14953 || (n > 14954 && n < 15001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10069)
	; 9 bytes
	php
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10071 || n = 10328 || n = 10585 || n = 10842 || n = 11099 || n = 11356 || n = 11613 || n = 11870 || n = 12127 || n = 12384 || n = 12641 || n = 12898 || n = 13155 || n = 13412 || n = 13669 || n = 13926 || n = 14183 || n = 14440 || n = 14697 || n = 14954)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10326)
	; 9 bytes
	php
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10583)
	; 9 bytes
	php
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 10840)
	; 9 bytes
	php
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11097)
	; 9 bytes
	php
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11354)
	; 9 bytes
	php
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11611)
	; 9 bytes
	php
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 11868)
	; 9 bytes
	php
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12125)
	; 9 bytes
	php
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12382)
	; 9 bytes
	php
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12639)
	; 9 bytes
	php
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 12896)
	; 9 bytes
	php
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13153)
	; 9 bytes
	php
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13410)
	; 9 bytes
	php
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13667)
	; 9 bytes
	php
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 13924)
	; 9 bytes
	php
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14181)
	; 9 bytes
	php
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14438)
	; 9 bytes
	php
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14695)
	; 9 bytes
	php
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 14952)
	; 9 bytes
	php
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15211 && n < 15466) || n = 15467 || (n > 15468 && n < 15723) || n = 15724 || (n > 15725 && n < 15980) || n = 15981 || (n > 15982 && n < 16237) || n = 16238 || (n > 16239 && n < 16494) || n = 16495 || (n > 16496 && n < 16751) || n = 16752 || (n > 16753 && n < 17008) || n = 17009 || (n > 17010 && n < 17265) || n = 17266 || (n > 17267 && n < 17522) || n = 17523 || (n > 17524 && n < 17779) || n = 17780 || (n > 17781 && n < 18036) || n = 18037 || (n > 18038 && n < 18293) || n = 18294 || (n > 18295 && n < 18550) || n = 18551 || (n > 18552 && n < 18807) || n = 18808 || (n > 18809 && n < 19064) || n = 19065 || (n > 19066 && n < 19321) || n = 19322 || (n > 19323 && n < 19578) || n = 19579 || (n > 19580 && n < 19835) || n = 19836 || (n > 19837 && n < 20001))
	; 11 bytes
	php
	pha
	ldx #>(n-48)
	lda #<(n-48)
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15209)
	; 9 bytes
	php
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15211 || n = 15468 || n = 15725 || n = 15982 || n = 16239 || n = 16496 || n = 16753 || n = 17010 || n = 17267 || n = 17524 || n = 17781 || n = 18038 || n = 18295 || n = 18552 || n = 18809 || n = 19066 || n = 19323 || n = 19580 || n = 19837)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15466)
	; 9 bytes
	php
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15723)
	; 9 bytes
	php
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 15980)
	; 9 bytes
	php
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16237)
	; 9 bytes
	php
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16494)
	; 9 bytes
	php
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 16751)
	; 9 bytes
	php
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17008)
	; 9 bytes
	php
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17265)
	; 9 bytes
	php
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17522)
	; 9 bytes
	php
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 17779)
	; 9 bytes
	php
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18036)
	; 9 bytes
	php
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18293)
	; 9 bytes
	php
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18550)
	; 9 bytes
	php
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 18807)
	; 9 bytes
	php
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19064)
	; 9 bytes
	php
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19321)
	; 9 bytes
	php
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19578)
	; 9 bytes
	php
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	plp
.elseif(n = 19835)
	; 9 bytes
	php
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
