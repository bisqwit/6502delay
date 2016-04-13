; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, S, Z+N, C, and V
; Will preserve A, Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=9, avg=9.0, max=9
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
; 1 to 1250
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 8 || n = 17 || n = 28 || n = 290 || n = 291 || n = 298 || n = 555 || n = 812 || n = 1069)
	; 2..8 bytes
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
.elseif(n = 18 || n = 31)
	; 4..6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 33 && n < 39) || (n > 39 && n < 290))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 292 || n = 299 || n = 556 || n = 813 || n = 1070)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n > 292 && n < 296))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 296)
	; 7 bytes
	pha
	lax #1
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 297 || (n > 299 && n < 553) || n = 554 || (n > 556 && n < 810) || n = 811 || (n > 813 && n < 1067) || n = 1068 || (n > 1070 && n < 1251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 553)
	; 7 bytes
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 810)
	; 7 bytes
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1067)
	; 7 bytes
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1324) || n = 1325 || (n > 1327 && n < 1581) || n = 1582 || (n > 1584 && n < 1838) || n = 1839 || (n > 1841 && n < 2095) || n = 2096 || (n > 2098 && n < 2352) || n = 2353 || (n > 2355 && n < 2501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1324)
	; 7 bytes
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1326 || n = 1583 || n = 1840 || n = 2097 || n = 2354)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1327 || n = 1584 || n = 1841 || n = 2098 || n = 2355)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1581)
	; 7 bytes
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1838)
	; 7 bytes
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2095)
	; 7 bytes
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2352)
	; 7 bytes
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2609) || n = 2610 || (n > 2612 && n < 2866) || n = 2867 || (n > 2869 && n < 3123) || n = 3124 || (n > 3126 && n < 3380) || n = 3381 || (n > 3383 && n < 3637) || n = 3638 || (n > 3640 && n < 3894) || n = 3895 || (n > 3897 && n < 4151) || n = 4152 || (n > 4154 && n < 4408) || n = 4409 || (n > 4411 && n < 4665) || n = 4666 || (n > 4668 && n < 4922) || n = 4923 || (n > 4925 && n < 5001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2609)
	; 7 bytes
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2611 || n = 2868 || n = 3125 || n = 3382 || n = 3639 || n = 3896 || n = 4153 || n = 4410 || n = 4667 || n = 4924)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2612 || n = 2869 || n = 3126 || n = 3383 || n = 3640 || n = 3897 || n = 4154 || n = 4411 || n = 4668 || n = 4925)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2866)
	; 7 bytes
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3123)
	; 7 bytes
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3380)
	; 7 bytes
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3637)
	; 7 bytes
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3894)
	; 7 bytes
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4151)
	; 7 bytes
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4408)
	; 7 bytes
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4665)
	; 7 bytes
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4922)
	; 7 bytes
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5179) || n = 5180 || (n > 5182 && n < 5436) || n = 5437 || (n > 5439 && n < 5693) || n = 5694 || (n > 5696 && n < 5950) || n = 5951 || (n > 5953 && n < 6207) || n = 6208 || (n > 6210 && n < 6464) || n = 6465 || (n > 6467 && n < 6721) || n = 6722 || (n > 6724 && n < 6978) || n = 6979 || (n > 6981 && n < 7235) || n = 7236 || (n > 7238 && n < 7492) || n = 7493 || (n > 7495 && n < 7749) || n = 7750 || (n > 7752 && n < 8006) || n = 8007 || (n > 8009 && n < 8263) || n = 8264 || (n > 8266 && n < 8520) || n = 8521 || (n > 8523 && n < 8777) || n = 8778 || (n > 8780 && n < 9034) || n = 9035 || (n > 9037 && n < 9291) || n = 9292 || (n > 9294 && n < 9548) || n = 9549 || (n > 9551 && n < 9805) || n = 9806 || (n > 9808 && n < 10001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5179)
	; 7 bytes
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5181 || n = 5438 || n = 5695 || n = 5952 || n = 6209 || n = 6466 || n = 6723 || n = 6980 || n = 7237 || n = 7494 || n = 7751 || n = 8008 || n = 8265 || n = 8522 || n = 8779 || n = 9036 || n = 9293 || n = 9550 || n = 9807)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5182 || n = 5439 || n = 5696 || n = 5953 || n = 6210 || n = 6467 || n = 6724 || n = 6981 || n = 7238 || n = 7495 || n = 7752 || n = 8009 || n = 8266 || n = 8523 || n = 8780 || n = 9037 || n = 9294 || n = 9551 || n = 9808)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5436)
	; 7 bytes
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5693)
	; 7 bytes
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5950)
	; 7 bytes
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6207)
	; 7 bytes
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6464)
	; 7 bytes
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6721)
	; 7 bytes
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6978)
	; 7 bytes
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7235)
	; 7 bytes
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7492)
	; 7 bytes
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7749)
	; 7 bytes
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8006)
	; 7 bytes
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8263)
	; 7 bytes
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8520)
	; 7 bytes
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8777)
	; 7 bytes
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9034)
	; 7 bytes
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9291)
	; 7 bytes
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9548)
	; 7 bytes
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9805)
	; 7 bytes
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10062) || n = 10063 || (n > 10065 && n < 10319) || n = 10320 || (n > 10322 && n < 10576) || n = 10577 || (n > 10579 && n < 10833) || n = 10834 || (n > 10836 && n < 11090) || n = 11091 || (n > 11093 && n < 11347) || n = 11348 || (n > 11350 && n < 11604) || n = 11605 || (n > 11607 && n < 11861) || n = 11862 || (n > 11864 && n < 12118) || n = 12119 || (n > 12121 && n < 12375) || n = 12376 || (n > 12378 && n < 12632) || n = 12633 || (n > 12635 && n < 12889) || n = 12890 || (n > 12892 && n < 13146) || n = 13147 || (n > 13149 && n < 13403) || n = 13404 || (n > 13406 && n < 13660) || n = 13661 || (n > 13663 && n < 13917) || n = 13918 || (n > 13920 && n < 14174) || n = 14175 || (n > 14177 && n < 14431) || n = 14432 || (n > 14434 && n < 14688) || n = 14689 || (n > 14691 && n < 14945) || n = 14946 || (n > 14948 && n < 15001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10062)
	; 7 bytes
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10064 || n = 10321 || n = 10578 || n = 10835 || n = 11092 || n = 11349 || n = 11606 || n = 11863 || n = 12120 || n = 12377 || n = 12634 || n = 12891 || n = 13148 || n = 13405 || n = 13662 || n = 13919 || n = 14176 || n = 14433 || n = 14690 || n = 14947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10065 || n = 10322 || n = 10579 || n = 10836 || n = 11093 || n = 11350 || n = 11607 || n = 11864 || n = 12121 || n = 12378 || n = 12635 || n = 12892 || n = 13149 || n = 13406 || n = 13663 || n = 13920 || n = 14177 || n = 14434 || n = 14691 || n = 14948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10319)
	; 7 bytes
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10576)
	; 7 bytes
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10833)
	; 7 bytes
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11090)
	; 7 bytes
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11347)
	; 7 bytes
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11604)
	; 7 bytes
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11861)
	; 7 bytes
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12118)
	; 7 bytes
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12375)
	; 7 bytes
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12632)
	; 7 bytes
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12889)
	; 7 bytes
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13146)
	; 7 bytes
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13403)
	; 7 bytes
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13660)
	; 7 bytes
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13917)
	; 7 bytes
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14174)
	; 7 bytes
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14431)
	; 7 bytes
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14688)
	; 7 bytes
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14945)
	; 7 bytes
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15202) || n = 15203 || (n > 15205 && n < 15459) || n = 15460 || (n > 15462 && n < 15716) || n = 15717 || (n > 15719 && n < 15973) || n = 15974 || (n > 15976 && n < 16230) || n = 16231 || (n > 16233 && n < 16487) || n = 16488 || (n > 16490 && n < 16744) || n = 16745 || (n > 16747 && n < 17001) || n = 17002 || (n > 17004 && n < 17258) || n = 17259 || (n > 17261 && n < 17515) || n = 17516 || (n > 17518 && n < 17772) || n = 17773 || (n > 17775 && n < 18029) || n = 18030 || (n > 18032 && n < 18286) || n = 18287 || (n > 18289 && n < 18543) || n = 18544 || (n > 18546 && n < 18800) || n = 18801 || (n > 18803 && n < 19057) || n = 19058 || (n > 19060 && n < 19314) || n = 19315 || (n > 19317 && n < 19571) || n = 19572 || (n > 19574 && n < 19828) || n = 19829 || (n > 19831 && n < 20001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15202)
	; 7 bytes
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15204 || n = 15461 || n = 15718 || n = 15975 || n = 16232 || n = 16489 || n = 16746 || n = 17003 || n = 17260 || n = 17517 || n = 17774 || n = 18031 || n = 18288 || n = 18545 || n = 18802 || n = 19059 || n = 19316 || n = 19573 || n = 19830)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15205 || n = 15462 || n = 15719 || n = 15976 || n = 16233 || n = 16490 || n = 16747 || n = 17004 || n = 17261 || n = 17518 || n = 17775 || n = 18032 || n = 18289 || n = 18546 || n = 18803 || n = 19060 || n = 19317 || n = 19574 || n = 19831)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15459)
	; 7 bytes
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15716)
	; 7 bytes
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15973)
	; 7 bytes
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16230)
	; 7 bytes
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16487)
	; 7 bytes
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16744)
	; 7 bytes
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17001)
	; 7 bytes
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17258)
	; 7 bytes
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17515)
	; 7 bytes
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17772)
	; 7 bytes
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18029)
	; 7 bytes
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18286)
	; 7 bytes
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18543)
	; 7 bytes
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18800)
	; 7 bytes
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19057)
	; 7 bytes
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19314)
	; 7 bytes
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19571)
	; 7 bytes
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19828)
	; 7 bytes
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
.endif
.endif
.endif

.endif
	rest
.endmacro
