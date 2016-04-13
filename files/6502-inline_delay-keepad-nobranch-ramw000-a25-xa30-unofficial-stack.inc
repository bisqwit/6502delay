; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber X, Y, S, Z+N, C, and V
; Will preserve A, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 30
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=3.9, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=6
; - When 31 <= n <= 50: min=7, avg=7.0, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.1, max=9
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
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 290 || n = 291 || n = 295 || n = 298)
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
.elseif((n > 8 && n < 12) || n = 19 || (n > 20 && n < 24) || n = 25)
	; 3..6 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 18 || n = 20 || n = 27 || n = 29 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 30 && n < 34))
	; 7 bytes
	tax
	lda #n-31
	jsr delay_a_25_clocks
	txa
.elseif((n > 33 && n < 290))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 292 || n = 299)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 293)
	; 7 bytes
	tay
	lax #1
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 294)
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
.elseif(n = 297)
	; 8 bytes
	tay
	pla
	lax #1
	jsr delay_256x_a_30_clocks
	tya
.elseif((n > 299 && n < 313))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.endif
.else
; 313 to 625
.if((n > 312 && n < 550) || n = 551 || (n > 556 && n < 626))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 550)
	; 7 bytes
	tay
	lax #2
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 552 || n = 555)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 553)
	; 7 bytes
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 554)
	; 8 bytes
	tay
	pla
	lax #2
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 556)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 807) || n = 808 || (n > 813 && n < 1064) || n = 1065 || (n > 1070 && n < 1251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 807)
	; 7 bytes
	tay
	lax #3
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 809 || n = 812 || n = 1066 || n = 1069)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 810)
	; 7 bytes
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 811)
	; 8 bytes
	tay
	pla
	lax #3
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 813 || n = 1070)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1064)
	; 7 bytes
	tay
	lax #4
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1067)
	; 7 bytes
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1068)
	; 8 bytes
	tay
	pla
	lax #4
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1321) || n = 1322 || (n > 1327 && n < 1578) || n = 1579 || (n > 1584 && n < 1835) || n = 1836 || (n > 1841 && n < 2092) || n = 2093 || (n > 2098 && n < 2349) || n = 2350 || (n > 2355 && n < 2501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1321)
	; 7 bytes
	tay
	lax #5
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1323 || n = 1326 || n = 1580 || n = 1583 || n = 1837 || n = 1840 || n = 2094 || n = 2097 || n = 2351 || n = 2354)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1324)
	; 7 bytes
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1325)
	; 8 bytes
	tay
	pla
	lax #5
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1327 || n = 1584 || n = 1841 || n = 2098 || n = 2355)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1578)
	; 7 bytes
	tay
	lax #6
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1581)
	; 7 bytes
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1582)
	; 8 bytes
	tay
	pla
	lax #6
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1835)
	; 7 bytes
	tay
	lax #7
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 1838)
	; 7 bytes
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 1839)
	; 8 bytes
	tay
	pla
	lax #7
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2092)
	; 7 bytes
	tay
	lax #8
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2095)
	; 7 bytes
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2096)
	; 8 bytes
	tay
	pla
	lax #8
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2349)
	; 7 bytes
	tay
	lax #9
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2352)
	; 7 bytes
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2353)
	; 8 bytes
	tay
	pla
	lax #9
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
; 2501 to 3750
.if((n > 2500 && n < 2606) || n = 2607 || (n > 2612 && n < 2863) || n = 2864 || (n > 2869 && n < 3120) || n = 3121 || (n > 3126 && n < 3377) || n = 3378 || (n > 3383 && n < 3634) || n = 3635 || (n > 3640 && n < 3751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2606)
	; 7 bytes
	tay
	lax #10
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2608 || n = 2611 || n = 2865 || n = 2868 || n = 3122 || n = 3125 || n = 3379 || n = 3382 || n = 3636 || n = 3639)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2609)
	; 7 bytes
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2610)
	; 8 bytes
	tay
	pla
	lax #10
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2612 || n = 2869 || n = 3126 || n = 3383 || n = 3640)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2863)
	; 7 bytes
	tay
	lax #11
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 2866)
	; 7 bytes
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 2867)
	; 8 bytes
	tay
	pla
	lax #11
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3120)
	; 7 bytes
	tay
	lax #12
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3123)
	; 7 bytes
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3124)
	; 8 bytes
	tay
	pla
	lax #12
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3377)
	; 7 bytes
	tay
	lax #13
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3380)
	; 7 bytes
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3381)
	; 8 bytes
	tay
	pla
	lax #13
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3634)
	; 7 bytes
	tay
	lax #14
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3637)
	; 7 bytes
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3638)
	; 8 bytes
	tay
	pla
	lax #14
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 3751 to 5000
.if((n > 3750 && n < 3891) || n = 3892 || (n > 3897 && n < 4148) || n = 4149 || (n > 4154 && n < 4405) || n = 4406 || (n > 4411 && n < 4662) || n = 4663 || (n > 4668 && n < 4919) || n = 4920 || (n > 4925 && n < 5001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3891)
	; 7 bytes
	tay
	lax #15
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3893 || n = 3896 || n = 4150 || n = 4153 || n = 4407 || n = 4410 || n = 4664 || n = 4667 || n = 4921 || n = 4924)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3894)
	; 7 bytes
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 3895)
	; 8 bytes
	tay
	pla
	lax #15
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 3897 || n = 4154 || n = 4411 || n = 4668 || n = 4925)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4148)
	; 7 bytes
	tay
	lax #16
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4151)
	; 7 bytes
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4152)
	; 8 bytes
	tay
	pla
	lax #16
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4405)
	; 7 bytes
	tay
	lax #17
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4408)
	; 7 bytes
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4409)
	; 8 bytes
	tay
	pla
	lax #17
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4662)
	; 7 bytes
	tay
	lax #18
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4665)
	; 7 bytes
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4666)
	; 8 bytes
	tay
	pla
	lax #18
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4919)
	; 7 bytes
	tay
	lax #19
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 4922)
	; 7 bytes
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 4923)
	; 8 bytes
	tay
	pla
	lax #19
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
; 5001 to 6250
.if((n > 5000 && n < 5176) || n = 5177 || (n > 5182 && n < 5433) || n = 5434 || (n > 5439 && n < 5690) || n = 5691 || (n > 5696 && n < 5947) || n = 5948 || (n > 5953 && n < 6204) || n = 6205 || (n > 6210 && n < 6251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5176)
	; 7 bytes
	tay
	lax #20
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5178 || n = 5181 || n = 5435 || n = 5438 || n = 5692 || n = 5695 || n = 5949 || n = 5952 || n = 6206 || n = 6209)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5179)
	; 7 bytes
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5180)
	; 8 bytes
	tay
	pla
	lax #20
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5182 || n = 5439 || n = 5696 || n = 5953 || n = 6210)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5433)
	; 7 bytes
	tay
	lax #21
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5436)
	; 7 bytes
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5437)
	; 8 bytes
	tay
	pla
	lax #21
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5690)
	; 7 bytes
	tay
	lax #22
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5693)
	; 7 bytes
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5694)
	; 8 bytes
	tay
	pla
	lax #22
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5947)
	; 7 bytes
	tay
	lax #23
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 5950)
	; 7 bytes
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 5951)
	; 8 bytes
	tay
	pla
	lax #23
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6204)
	; 7 bytes
	tay
	lax #24
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6207)
	; 7 bytes
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6208)
	; 8 bytes
	tay
	pla
	lax #24
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6461) || n = 6462 || (n > 6467 && n < 6718) || n = 6719 || (n > 6724 && n < 6975) || n = 6976 || (n > 6981 && n < 7232) || n = 7233 || (n > 7238 && n < 7489) || n = 7490 || (n > 7495 && n < 7501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6461)
	; 7 bytes
	tay
	lax #25
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6463 || n = 6466 || n = 6720 || n = 6723 || n = 6977 || n = 6980 || n = 7234 || n = 7237 || n = 7491 || n = 7494)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6464)
	; 7 bytes
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6465)
	; 8 bytes
	tay
	pla
	lax #25
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6467 || n = 6724 || n = 6981 || n = 7238 || n = 7495)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6718)
	; 7 bytes
	tay
	lax #26
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6721)
	; 7 bytes
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6722)
	; 8 bytes
	tay
	pla
	lax #26
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6975)
	; 7 bytes
	tay
	lax #27
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 6978)
	; 7 bytes
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 6979)
	; 8 bytes
	tay
	pla
	lax #27
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7232)
	; 7 bytes
	tay
	lax #28
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7235)
	; 7 bytes
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7236)
	; 8 bytes
	tay
	pla
	lax #28
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7489)
	; 7 bytes
	tay
	lax #29
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7492)
	; 7 bytes
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7493)
	; 8 bytes
	tay
	pla
	lax #29
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
; 7501 to 8750
.if((n > 7500 && n < 7746) || n = 7747 || (n > 7752 && n < 8003) || n = 8004 || (n > 8009 && n < 8260) || n = 8261 || (n > 8266 && n < 8517) || n = 8518 || (n > 8523 && n < 8751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7746)
	; 7 bytes
	tay
	lax #30
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7748 || n = 7751 || n = 8005 || n = 8008 || n = 8262 || n = 8265 || n = 8519 || n = 8522)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 7749)
	; 7 bytes
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 7750)
	; 8 bytes
	tay
	pla
	lax #30
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 7752 || n = 8009 || n = 8266 || n = 8523)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8003)
	; 7 bytes
	tay
	lax #31
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8006)
	; 7 bytes
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8007)
	; 8 bytes
	tay
	pla
	lax #31
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8260)
	; 7 bytes
	tay
	lax #32
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8263)
	; 7 bytes
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8264)
	; 8 bytes
	tay
	pla
	lax #32
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8517)
	; 7 bytes
	tay
	lax #33
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8520)
	; 7 bytes
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8521)
	; 8 bytes
	tay
	pla
	lax #33
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8774) || n = 8775 || (n > 8780 && n < 9031) || n = 9032 || (n > 9037 && n < 9288) || n = 9289 || (n > 9294 && n < 9545) || n = 9546 || (n > 9551 && n < 9802) || n = 9803 || (n > 9808 && n < 10001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8774)
	; 7 bytes
	tay
	lax #34
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8776 || n = 8779 || n = 9033 || n = 9036 || n = 9290 || n = 9293 || n = 9547 || n = 9550 || n = 9804 || n = 9807)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8777)
	; 7 bytes
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 8778)
	; 8 bytes
	tay
	pla
	lax #34
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 8780 || n = 9037 || n = 9294 || n = 9551 || n = 9808)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9031)
	; 7 bytes
	tay
	lax #35
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9034)
	; 7 bytes
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9035)
	; 8 bytes
	tay
	pla
	lax #35
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9288)
	; 7 bytes
	tay
	lax #36
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9291)
	; 7 bytes
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9292)
	; 8 bytes
	tay
	pla
	lax #36
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9545)
	; 7 bytes
	tay
	lax #37
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9548)
	; 7 bytes
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9549)
	; 8 bytes
	tay
	pla
	lax #37
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9802)
	; 7 bytes
	tay
	lax #38
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 9805)
	; 7 bytes
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 9806)
	; 8 bytes
	tay
	pla
	lax #38
	jsr delay_256x_a_30_clocks
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
.if((n > 10000 && n < 10059) || n = 10060 || (n > 10065 && n < 10316) || n = 10317 || (n > 10322 && n < 10573) || n = 10574 || (n > 10579 && n < 10830) || n = 10831 || (n > 10836 && n < 11087) || n = 11088 || (n > 11093 && n < 11251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10059)
	; 7 bytes
	tay
	lax #39
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10061 || n = 10064 || n = 10318 || n = 10321 || n = 10575 || n = 10578 || n = 10832 || n = 10835 || n = 11089 || n = 11092)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10062)
	; 7 bytes
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10063)
	; 8 bytes
	tay
	pla
	lax #39
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10065 || n = 10322 || n = 10579 || n = 10836 || n = 11093)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10316)
	; 7 bytes
	tay
	lax #40
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10319)
	; 7 bytes
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10320)
	; 8 bytes
	tay
	pla
	lax #40
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10573)
	; 7 bytes
	tay
	lax #41
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10576)
	; 7 bytes
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10577)
	; 8 bytes
	tay
	pla
	lax #41
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10830)
	; 7 bytes
	tay
	lax #42
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 10833)
	; 7 bytes
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 10834)
	; 8 bytes
	tay
	pla
	lax #42
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11087)
	; 7 bytes
	tay
	lax #43
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11090)
	; 7 bytes
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11091)
	; 8 bytes
	tay
	pla
	lax #43
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 11251 to 12500
.if((n > 11250 && n < 11344) || n = 11345 || (n > 11350 && n < 11601) || n = 11602 || (n > 11607 && n < 11858) || n = 11859 || (n > 11864 && n < 12115) || n = 12116 || (n > 12121 && n < 12372) || n = 12373 || (n > 12378 && n < 12501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11344)
	; 7 bytes
	tay
	lax #44
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11346 || n = 11349 || n = 11603 || n = 11606 || n = 11860 || n = 11863 || n = 12117 || n = 12120 || n = 12374 || n = 12377)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11347)
	; 7 bytes
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11348)
	; 8 bytes
	tay
	pla
	lax #44
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11350 || n = 11607 || n = 11864 || n = 12121 || n = 12378)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11601)
	; 7 bytes
	tay
	lax #45
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11604)
	; 7 bytes
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11605)
	; 8 bytes
	tay
	pla
	lax #45
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11858)
	; 7 bytes
	tay
	lax #46
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 11861)
	; 7 bytes
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 11862)
	; 8 bytes
	tay
	pla
	lax #46
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12115)
	; 7 bytes
	tay
	lax #47
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12118)
	; 7 bytes
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12119)
	; 8 bytes
	tay
	pla
	lax #47
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12372)
	; 7 bytes
	tay
	lax #48
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12375)
	; 7 bytes
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12376)
	; 8 bytes
	tay
	pla
	lax #48
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
; 12501 to 13750
.if((n > 12500 && n < 12629) || n = 12630 || (n > 12635 && n < 12886) || n = 12887 || (n > 12892 && n < 13143) || n = 13144 || (n > 13149 && n < 13400) || n = 13401 || (n > 13406 && n < 13657) || n = 13658 || (n > 13663 && n < 13751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12629)
	; 7 bytes
	tay
	lax #49
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12631 || n = 12634 || n = 12888 || n = 12891 || n = 13145 || n = 13148 || n = 13402 || n = 13405 || n = 13659 || n = 13662)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 12632)
	; 7 bytes
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12633)
	; 8 bytes
	tay
	pla
	lax #49
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12635 || n = 12892 || n = 13149 || n = 13406 || n = 13663)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12886)
	; 7 bytes
	tay
	lax #50
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 12889)
	; 7 bytes
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 12890)
	; 8 bytes
	tay
	pla
	lax #50
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13143)
	; 7 bytes
	tay
	lax #51
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13146)
	; 7 bytes
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13147)
	; 8 bytes
	tay
	pla
	lax #51
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13400)
	; 7 bytes
	tay
	lax #52
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13403)
	; 7 bytes
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13404)
	; 8 bytes
	tay
	pla
	lax #52
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13657)
	; 7 bytes
	tay
	lax #53
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13660)
	; 7 bytes
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13661)
	; 8 bytes
	tay
	pla
	lax #53
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 13751 to 15000
.if((n > 13750 && n < 13914) || n = 13915 || (n > 13920 && n < 14171) || n = 14172 || (n > 14177 && n < 14428) || n = 14429 || (n > 14434 && n < 14685) || n = 14686 || (n > 14691 && n < 14942) || n = 14943 || (n > 14948 && n < 15001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13914)
	; 7 bytes
	tay
	lax #54
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13916 || n = 13919 || n = 14173 || n = 14176 || n = 14430 || n = 14433 || n = 14687 || n = 14690 || n = 14944 || n = 14947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13917)
	; 7 bytes
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 13918)
	; 8 bytes
	tay
	pla
	lax #54
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 13920 || n = 14177 || n = 14434 || n = 14691 || n = 14948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14171)
	; 7 bytes
	tay
	lax #55
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14174)
	; 7 bytes
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14175)
	; 8 bytes
	tay
	pla
	lax #55
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14428)
	; 7 bytes
	tay
	lax #56
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14431)
	; 7 bytes
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14432)
	; 8 bytes
	tay
	pla
	lax #56
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14685)
	; 7 bytes
	tay
	lax #57
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14688)
	; 7 bytes
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14689)
	; 8 bytes
	tay
	pla
	lax #57
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14942)
	; 7 bytes
	tay
	lax #58
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 14945)
	; 7 bytes
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 14946)
	; 8 bytes
	tay
	pla
	lax #58
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
; 15001 to 16250
.if((n > 15000 && n < 15199) || n = 15200 || (n > 15205 && n < 15456) || n = 15457 || (n > 15462 && n < 15713) || n = 15714 || (n > 15719 && n < 15970) || n = 15971 || (n > 15976 && n < 16227) || n = 16228 || (n > 16233 && n < 16251))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15199)
	; 7 bytes
	tay
	lax #59
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15201 || n = 15204 || n = 15458 || n = 15461 || n = 15715 || n = 15718 || n = 15972 || n = 15975 || n = 16229 || n = 16232)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15202)
	; 7 bytes
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15203)
	; 8 bytes
	tay
	pla
	lax #59
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15205 || n = 15462 || n = 15719 || n = 15976 || n = 16233)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15456)
	; 7 bytes
	tay
	lax #60
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15459)
	; 7 bytes
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15460)
	; 8 bytes
	tay
	pla
	lax #60
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15713)
	; 7 bytes
	tay
	lax #61
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15716)
	; 7 bytes
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15717)
	; 8 bytes
	tay
	pla
	lax #61
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15970)
	; 7 bytes
	tay
	lax #62
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 15973)
	; 7 bytes
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 15974)
	; 8 bytes
	tay
	pla
	lax #62
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16227)
	; 7 bytes
	tay
	lax #63
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16230)
	; 7 bytes
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16231)
	; 8 bytes
	tay
	pla
	lax #63
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 16251 to 17500
.if((n > 16250 && n < 16484) || n = 16485 || (n > 16490 && n < 16741) || n = 16742 || (n > 16747 && n < 16998) || n = 16999 || (n > 17004 && n < 17255) || n = 17256 || (n > 17261 && n < 17501))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16484)
	; 7 bytes
	tay
	lax #64
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16486 || n = 16489 || n = 16743 || n = 16746 || n = 17000 || n = 17003 || n = 17257 || n = 17260)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16487)
	; 7 bytes
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16488)
	; 8 bytes
	tay
	pla
	lax #64
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16490 || n = 16747 || n = 17004 || n = 17261)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16741)
	; 7 bytes
	tay
	lax #65
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16744)
	; 7 bytes
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 16745)
	; 8 bytes
	tay
	pla
	lax #65
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 16998)
	; 7 bytes
	tay
	lax #66
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17001)
	; 7 bytes
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17002)
	; 8 bytes
	tay
	pla
	lax #66
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17255)
	; 7 bytes
	tay
	lax #67
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17258)
	; 7 bytes
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17259)
	; 8 bytes
	tay
	pla
	lax #67
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
; 17501 to 18750
.if((n > 17500 && n < 17512) || n = 17513 || (n > 17518 && n < 17769) || n = 17770 || (n > 17775 && n < 18026) || n = 18027 || (n > 18032 && n < 18283) || n = 18284 || (n > 18289 && n < 18540) || n = 18541 || (n > 18546 && n < 18751))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17512)
	; 7 bytes
	tay
	lax #68
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17514 || n = 17517 || n = 17771 || n = 17774 || n = 18028 || n = 18031 || n = 18285 || n = 18288 || n = 18542 || n = 18545)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 17515)
	; 7 bytes
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17516)
	; 8 bytes
	tay
	pla
	lax #68
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17518 || n = 17775 || n = 18032 || n = 18289 || n = 18546)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17769)
	; 7 bytes
	tay
	lax #69
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 17772)
	; 7 bytes
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 17773)
	; 8 bytes
	tay
	pla
	lax #69
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18026)
	; 7 bytes
	tay
	lax #70
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18029)
	; 7 bytes
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18030)
	; 8 bytes
	tay
	pla
	lax #70
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18283)
	; 7 bytes
	tay
	lax #71
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18286)
	; 7 bytes
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18287)
	; 8 bytes
	tay
	pla
	lax #71
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18540)
	; 7 bytes
	tay
	lax #72
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18543)
	; 7 bytes
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18544)
	; 8 bytes
	tay
	pla
	lax #72
	jsr delay_256x_a_30_clocks
	tya
.endif
.else
; 18751 to 20000
.if((n > 18750 && n < 18797) || n = 18798 || (n > 18803 && n < 19054) || n = 19055 || (n > 19060 && n < 19311) || n = 19312 || (n > 19317 && n < 19568) || n = 19569 || (n > 19574 && n < 19825) || n = 19826 || (n > 19831 && n < 20001))
	; 9 bytes
	pha
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18797)
	; 7 bytes
	tay
	lax #73
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18799 || n = 18802 || n = 19056 || n = 19059 || n = 19313 || n = 19316 || n = 19570 || n = 19573 || n = 19827 || n = 19830)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18800)
	; 7 bytes
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 18801)
	; 8 bytes
	tay
	pla
	lax #73
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 18803 || n = 19060 || n = 19317 || n = 19574 || n = 19831)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19054)
	; 7 bytes
	tay
	lax #74
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19057)
	; 7 bytes
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19058)
	; 8 bytes
	tay
	pla
	lax #74
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19311)
	; 7 bytes
	tay
	lax #75
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19314)
	; 7 bytes
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19315)
	; 8 bytes
	tay
	pla
	lax #75
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19568)
	; 7 bytes
	tay
	lax #76
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19571)
	; 7 bytes
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19572)
	; 8 bytes
	tay
	pla
	lax #76
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19825)
	; 7 bytes
	tay
	lax #77
	jsr delay_256x_a_30_clocks
	tya
.elseif(n = 19828)
	; 7 bytes
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
.elseif(n = 19829)
	; 8 bytes
	tay
	pla
	lax #77
	jsr delay_256x_a_30_clocks
	tya
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
