; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Z+N, C, and V
; Will preserve Y, S, D, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
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
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 284
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=5, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.3, max=7
; - When 301 <= n <= 500: min=7, avg=7.0, max=7
; - When 501 <= n <= 750: min=5, avg=7.0, max=7
; - When 751 <= n <= 1000: min=5, avg=7.0, max=7
; - When 1001 <= n <= 2000: min=5, avg=7.0, max=7
; - When 2001 <= n <= 3000: min=5, avg=7.0, max=7
; - When 3001 <= n <= 5000: min=5, avg=7.0, max=7
; - When 5001 <= n <= 7500: min=5, avg=7.0, max=7
; - When 7501 <= n <= 10000: min=5, avg=7.0, max=7
; - When 10001 <= n <= 20000: min=5, avg=7.0, max=7
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
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 283 || n = 284 || n = 291 || n = 548 || n = 805 || n = 1062)
	; 2..6 bytes
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
.elseif(n = 9 || n = 11 || (n > 19 && n < 24))
	; 3..6 bytes
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
	; 2 bytes
	brk
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18)
	; 4 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 19 || n = 26)
	; 4 bytes
	delay_n (n-13)
	brk
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif((n > 26 && n < 283))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif((n > 284 && n < 289))
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 289)
	; 5 bytes
	lax #1
	jsr delay_256x_a_30_clocks
.elseif(n = 290 || (n > 291 && n < 546) || n = 547 || (n > 548 && n < 803) || n = 804 || (n > 805 && n < 1060) || n = 1061 || (n > 1062 && n < 1251))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 546)
	; 5 bytes
	lax #2
	jsr delay_256x_a_30_clocks
.elseif(n = 803)
	; 5 bytes
	lax #3
	jsr delay_256x_a_30_clocks
.elseif(n = 1060)
	; 5 bytes
	lax #4
	jsr delay_256x_a_30_clocks
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1317) || n = 1318 || (n > 1319 && n < 1574) || n = 1575 || (n > 1576 && n < 1831) || n = 1832 || (n > 1833 && n < 2088) || n = 2089 || (n > 2090 && n < 2345) || n = 2346 || (n > 2347 && n < 2501))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1317)
	; 5 bytes
	lax #5
	jsr delay_256x_a_30_clocks
.elseif(n = 1319 || n = 1576 || n = 1833 || n = 2090 || n = 2347)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 1574)
	; 5 bytes
	lax #6
	jsr delay_256x_a_30_clocks
.elseif(n = 1831)
	; 5 bytes
	lax #7
	jsr delay_256x_a_30_clocks
.elseif(n = 2088)
	; 5 bytes
	lax #8
	jsr delay_256x_a_30_clocks
.elseif(n = 2345)
	; 5 bytes
	lax #9
	jsr delay_256x_a_30_clocks
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2602) || n = 2603 || (n > 2604 && n < 2859) || n = 2860 || (n > 2861 && n < 3116) || n = 3117 || (n > 3118 && n < 3373) || n = 3374 || (n > 3375 && n < 3630) || n = 3631 || (n > 3632 && n < 3887) || n = 3888 || (n > 3889 && n < 4144) || n = 4145 || (n > 4146 && n < 4401) || n = 4402 || (n > 4403 && n < 4658) || n = 4659 || (n > 4660 && n < 4915) || n = 4916 || (n > 4917 && n < 5001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 2602)
	; 5 bytes
	lax #10
	jsr delay_256x_a_30_clocks
.elseif(n = 2604 || n = 2861 || n = 3118 || n = 3375 || n = 3632 || n = 3889 || n = 4146 || n = 4403 || n = 4660 || n = 4917)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2859)
	; 5 bytes
	lax #11
	jsr delay_256x_a_30_clocks
.elseif(n = 3116)
	; 5 bytes
	lax #12
	jsr delay_256x_a_30_clocks
.elseif(n = 3373)
	; 5 bytes
	lax #13
	jsr delay_256x_a_30_clocks
.elseif(n = 3630)
	; 5 bytes
	lax #14
	jsr delay_256x_a_30_clocks
.elseif(n = 3887)
	; 5 bytes
	lax #15
	jsr delay_256x_a_30_clocks
.elseif(n = 4144)
	; 5 bytes
	lax #16
	jsr delay_256x_a_30_clocks
.elseif(n = 4401)
	; 5 bytes
	lax #17
	jsr delay_256x_a_30_clocks
.elseif(n = 4658)
	; 5 bytes
	lax #18
	jsr delay_256x_a_30_clocks
.elseif(n = 4915)
	; 5 bytes
	lax #19
	jsr delay_256x_a_30_clocks
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5172) || n = 5173 || (n > 5174 && n < 5429) || n = 5430 || (n > 5431 && n < 5686) || n = 5687 || (n > 5688 && n < 5943) || n = 5944 || (n > 5945 && n < 6200) || n = 6201 || (n > 6202 && n < 6457) || n = 6458 || (n > 6459 && n < 6714) || n = 6715 || (n > 6716 && n < 6971) || n = 6972 || (n > 6973 && n < 7228) || n = 7229 || (n > 7230 && n < 7485) || n = 7486 || (n > 7487 && n < 7742) || n = 7743 || (n > 7744 && n < 7999) || n = 8000 || (n > 8001 && n < 8256) || n = 8257 || (n > 8258 && n < 8513) || n = 8514 || (n > 8515 && n < 8770) || n = 8771 || (n > 8772 && n < 9027) || n = 9028 || (n > 9029 && n < 9284) || n = 9285 || (n > 9286 && n < 9541) || n = 9542 || (n > 9543 && n < 9798) || n = 9799 || (n > 9800 && n < 10001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 5172)
	; 5 bytes
	lax #20
	jsr delay_256x_a_30_clocks
.elseif(n = 5174 || n = 5431 || n = 5688 || n = 5945 || n = 6202 || n = 6459 || n = 6716 || n = 6973 || n = 7230 || n = 7487 || n = 7744 || n = 8001 || n = 8258 || n = 8515 || n = 8772 || n = 9029 || n = 9286 || n = 9543 || n = 9800)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 5429)
	; 5 bytes
	lax #21
	jsr delay_256x_a_30_clocks
.elseif(n = 5686)
	; 5 bytes
	lax #22
	jsr delay_256x_a_30_clocks
.elseif(n = 5943)
	; 5 bytes
	lax #23
	jsr delay_256x_a_30_clocks
.elseif(n = 6200)
	; 5 bytes
	lax #24
	jsr delay_256x_a_30_clocks
.elseif(n = 6457)
	; 5 bytes
	lax #25
	jsr delay_256x_a_30_clocks
.elseif(n = 6714)
	; 5 bytes
	lax #26
	jsr delay_256x_a_30_clocks
.elseif(n = 6971)
	; 5 bytes
	lax #27
	jsr delay_256x_a_30_clocks
.elseif(n = 7228)
	; 5 bytes
	lax #28
	jsr delay_256x_a_30_clocks
.elseif(n = 7485)
	; 5 bytes
	lax #29
	jsr delay_256x_a_30_clocks
.elseif(n = 7742)
	; 5 bytes
	lax #30
	jsr delay_256x_a_30_clocks
.elseif(n = 7999)
	; 5 bytes
	lax #31
	jsr delay_256x_a_30_clocks
.elseif(n = 8256)
	; 5 bytes
	lax #32
	jsr delay_256x_a_30_clocks
.elseif(n = 8513)
	; 5 bytes
	lax #33
	jsr delay_256x_a_30_clocks
.elseif(n = 8770)
	; 5 bytes
	lax #34
	jsr delay_256x_a_30_clocks
.elseif(n = 9027)
	; 5 bytes
	lax #35
	jsr delay_256x_a_30_clocks
.elseif(n = 9284)
	; 5 bytes
	lax #36
	jsr delay_256x_a_30_clocks
.elseif(n = 9541)
	; 5 bytes
	lax #37
	jsr delay_256x_a_30_clocks
.elseif(n = 9798)
	; 5 bytes
	lax #38
	jsr delay_256x_a_30_clocks
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10055) || n = 10056 || (n > 10057 && n < 10312) || n = 10313 || (n > 10314 && n < 10569) || n = 10570 || (n > 10571 && n < 10826) || n = 10827 || (n > 10828 && n < 11083) || n = 11084 || (n > 11085 && n < 11340) || n = 11341 || (n > 11342 && n < 11597) || n = 11598 || (n > 11599 && n < 11854) || n = 11855 || (n > 11856 && n < 12111) || n = 12112 || (n > 12113 && n < 12368) || n = 12369 || (n > 12370 && n < 12625) || n = 12626 || (n > 12627 && n < 12882) || n = 12883 || (n > 12884 && n < 13139) || n = 13140 || (n > 13141 && n < 13396) || n = 13397 || (n > 13398 && n < 13653) || n = 13654 || (n > 13655 && n < 13910) || n = 13911 || (n > 13912 && n < 14167) || n = 14168 || (n > 14169 && n < 14424) || n = 14425 || (n > 14426 && n < 14681) || n = 14682 || (n > 14683 && n < 14938) || n = 14939 || (n > 14940 && n < 15001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 10055)
	; 5 bytes
	lax #39
	jsr delay_256x_a_30_clocks
.elseif(n = 10057 || n = 10314 || n = 10571 || n = 10828 || n = 11085 || n = 11342 || n = 11599 || n = 11856 || n = 12113 || n = 12370 || n = 12627 || n = 12884 || n = 13141 || n = 13398 || n = 13655 || n = 13912 || n = 14169 || n = 14426 || n = 14683 || n = 14940)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 10312)
	; 5 bytes
	lax #40
	jsr delay_256x_a_30_clocks
.elseif(n = 10569)
	; 5 bytes
	lax #41
	jsr delay_256x_a_30_clocks
.elseif(n = 10826)
	; 5 bytes
	lax #42
	jsr delay_256x_a_30_clocks
.elseif(n = 11083)
	; 5 bytes
	lax #43
	jsr delay_256x_a_30_clocks
.elseif(n = 11340)
	; 5 bytes
	lax #44
	jsr delay_256x_a_30_clocks
.elseif(n = 11597)
	; 5 bytes
	lax #45
	jsr delay_256x_a_30_clocks
.elseif(n = 11854)
	; 5 bytes
	lax #46
	jsr delay_256x_a_30_clocks
.elseif(n = 12111)
	; 5 bytes
	lax #47
	jsr delay_256x_a_30_clocks
.elseif(n = 12368)
	; 5 bytes
	lax #48
	jsr delay_256x_a_30_clocks
.elseif(n = 12625)
	; 5 bytes
	lax #49
	jsr delay_256x_a_30_clocks
.elseif(n = 12882)
	; 5 bytes
	lax #50
	jsr delay_256x_a_30_clocks
.elseif(n = 13139)
	; 5 bytes
	lax #51
	jsr delay_256x_a_30_clocks
.elseif(n = 13396)
	; 5 bytes
	lax #52
	jsr delay_256x_a_30_clocks
.elseif(n = 13653)
	; 5 bytes
	lax #53
	jsr delay_256x_a_30_clocks
.elseif(n = 13910)
	; 5 bytes
	lax #54
	jsr delay_256x_a_30_clocks
.elseif(n = 14167)
	; 5 bytes
	lax #55
	jsr delay_256x_a_30_clocks
.elseif(n = 14424)
	; 5 bytes
	lax #56
	jsr delay_256x_a_30_clocks
.elseif(n = 14681)
	; 5 bytes
	lax #57
	jsr delay_256x_a_30_clocks
.elseif(n = 14938)
	; 5 bytes
	lax #58
	jsr delay_256x_a_30_clocks
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15195) || n = 15196 || (n > 15197 && n < 15452) || n = 15453 || (n > 15454 && n < 15709) || n = 15710 || (n > 15711 && n < 15966) || n = 15967 || (n > 15968 && n < 16223) || n = 16224 || (n > 16225 && n < 16480) || n = 16481 || (n > 16482 && n < 16737) || n = 16738 || (n > 16739 && n < 16994) || n = 16995 || (n > 16996 && n < 17251) || n = 17252 || (n > 17253 && n < 17508) || n = 17509 || (n > 17510 && n < 17765) || n = 17766 || (n > 17767 && n < 18022) || n = 18023 || (n > 18024 && n < 18279) || n = 18280 || (n > 18281 && n < 18536) || n = 18537 || (n > 18538 && n < 18793) || n = 18794 || (n > 18795 && n < 19050) || n = 19051 || (n > 19052 && n < 19307) || n = 19308 || (n > 19309 && n < 19564) || n = 19565 || (n > 19566 && n < 19821) || n = 19822 || (n > 19823 && n < 20001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 15195)
	; 5 bytes
	lax #59
	jsr delay_256x_a_30_clocks
.elseif(n = 15197 || n = 15454 || n = 15711 || n = 15968 || n = 16225 || n = 16482 || n = 16739 || n = 16996 || n = 17253 || n = 17510 || n = 17767 || n = 18024 || n = 18281 || n = 18538 || n = 18795 || n = 19052 || n = 19309 || n = 19566 || n = 19823)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 15452)
	; 5 bytes
	lax #60
	jsr delay_256x_a_30_clocks
.elseif(n = 15709)
	; 5 bytes
	lax #61
	jsr delay_256x_a_30_clocks
.elseif(n = 15966)
	; 5 bytes
	lax #62
	jsr delay_256x_a_30_clocks
.elseif(n = 16223)
	; 5 bytes
	lax #63
	jsr delay_256x_a_30_clocks
.elseif(n = 16480)
	; 5 bytes
	lax #64
	jsr delay_256x_a_30_clocks
.elseif(n = 16737)
	; 5 bytes
	lax #65
	jsr delay_256x_a_30_clocks
.elseif(n = 16994)
	; 5 bytes
	lax #66
	jsr delay_256x_a_30_clocks
.elseif(n = 17251)
	; 5 bytes
	lax #67
	jsr delay_256x_a_30_clocks
.elseif(n = 17508)
	; 5 bytes
	lax #68
	jsr delay_256x_a_30_clocks
.elseif(n = 17765)
	; 5 bytes
	lax #69
	jsr delay_256x_a_30_clocks
.elseif(n = 18022)
	; 5 bytes
	lax #70
	jsr delay_256x_a_30_clocks
.elseif(n = 18279)
	; 5 bytes
	lax #71
	jsr delay_256x_a_30_clocks
.elseif(n = 18536)
	; 5 bytes
	lax #72
	jsr delay_256x_a_30_clocks
.elseif(n = 18793)
	; 5 bytes
	lax #73
	jsr delay_256x_a_30_clocks
.elseif(n = 19050)
	; 5 bytes
	lax #74
	jsr delay_256x_a_30_clocks
.elseif(n = 19307)
	; 5 bytes
	lax #75
	jsr delay_256x_a_30_clocks
.elseif(n = 19564)
	; 5 bytes
	lax #76
	jsr delay_256x_a_30_clocks
.elseif(n = 19821)
	; 5 bytes
	lax #77
	jsr delay_256x_a_30_clocks
.endif
.endif
.endif

.endif
	rest
.endmacro
