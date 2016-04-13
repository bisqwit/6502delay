; delay_n, delays n cycles with small code
; Will clobber Z+N, and V
; Will preserve A, X, Y, S, C, D, and I (interrupt-disable flag)
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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 30
; - 7 bytes or less when n <= 35
; - 8 bytes or less when n <= 43
; - 9 bytes or less when n <= 296
; -10 bytes or less when n <= 298
; -11 bytes or less when n <= 309
; -12 bytes or less when n <= 311
; -13 bytes or less when n <= 322
; -14 bytes or less when n <= 496
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=2, avg=3.7, max=5
; - When 21 <= n <= 30: min=4, avg=5.3, max=6
; - When 31 <= n <= 50: min=6, avg=7.8, max=9
; - When 51 <= n <= 75: min=8, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.1, max=11
; - When 301 <= n <= 500: min=11, avg=13.8, max=15
; - When 501 <= n <= 750: min=15, avg=15.0, max=15
; - When 751 <= n <= 1000: min=15, avg=15.0, max=15
; - When 1001 <= n <= 2000: min=15, avg=15.0, max=15
; - When 2001 <= n <= 3000: min=15, avg=15.0, max=15
; - When 3001 <= n <= 5000: min=15, avg=15.0, max=15
; - When 5001 <= n <= 7500: min=15, avg=15.0, max=15
; - When 7501 <= n <= 10000: min=15, avg=15.0, max=15
; - When 10001 <= n <= 20000: min=15, avg=15.0, max=15
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
	; 2 bytes
	bit $24
.elseif((n > 3 && n < 7) || (n > 15 && n < 19) || n = 28 || n = 31 || n = 32 || n = 41 || n = 43 || (n > 296 && n < 301))
	; 2..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || (n > 18 && n < 24) || (n > 32 && n < 38) || n = 46)
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
	; 2 bytes
	brk
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 39)
	; 4..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 27 || n = 40)
	; 5..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 29 || n = 30 || n = 310 || n = 311)
	; 6..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 42)
	; 8 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 44 || n = 45 || (n > 46 && n < 52) || (n > 52 && n < 297))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk
	.endrepeat
.elseif((n > 300 && n < 304))
	; 11 bytes
	php
	pha
	php
	lda #n-48
	jsr delay_a_25_clocks
	plp
	pla
	plp
.elseif((n > 303 && n < 310))
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 312)
	; 13 bytes
	delay_n (n-16)
	delay_n 16
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if(n = 313)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif((n > 313 && n < 323))
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 323)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 324)
	; 14 bytes
	php
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 325)
	; 14 bytes
	php
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 326)
	; 14 bytes
	php
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 327)
	; 14 bytes
	php
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 328)
	; 14 bytes
	php
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 329)
	; 14 bytes
	php
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 330)
	; 14 bytes
	php
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 331)
	; 14 bytes
	php
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.else
; 332 to 351
.if(n = 332)
	; 14 bytes
	php
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 333)
	; 14 bytes
	php
	pha
	lda #10
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 334)
	; 14 bytes
	php
	pha
	lda #11
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 335)
	; 14 bytes
	php
	pha
	lda #12
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 336)
	; 14 bytes
	php
	pha
	lda #13
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 337)
	; 14 bytes
	php
	pha
	lda #14
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 338)
	; 14 bytes
	php
	pha
	lda #15
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 339)
	; 14 bytes
	php
	pha
	lda #16
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 340)
	; 14 bytes
	php
	pha
	lda #17
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 341)
	; 14 bytes
	php
	pha
	lda #18
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 342)
	; 14 bytes
	php
	pha
	lda #19
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 343)
	; 14 bytes
	php
	pha
	lda #20
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 344)
	; 14 bytes
	php
	pha
	lda #21
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 345)
	; 14 bytes
	php
	pha
	lda #22
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 346)
	; 14 bytes
	php
	pha
	lda #23
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 347)
	; 14 bytes
	php
	pha
	lda #24
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 348)
	; 14 bytes
	php
	pha
	lda #25
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 349)
	; 14 bytes
	php
	pha
	lda #26
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 350)
	; 14 bytes
	php
	pha
	lda #27
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 351)
	; 14 bytes
	php
	pha
	lda #28
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 14 bytes
	php
	pha
	lda #29
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 353)
	; 14 bytes
	php
	pha
	lda #30
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 354)
	; 14 bytes
	php
	pha
	lda #31
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 355)
	; 14 bytes
	php
	pha
	lda #32
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 356)
	; 14 bytes
	php
	pha
	lda #33
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 357)
	; 14 bytes
	php
	pha
	lda #34
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 358)
	; 14 bytes
	php
	pha
	lda #35
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 359)
	; 14 bytes
	php
	pha
	lda #36
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 360)
	; 14 bytes
	php
	pha
	lda #37
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 361)
	; 14 bytes
	php
	pha
	lda #38
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 362)
	; 14 bytes
	php
	pha
	lda #39
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 363)
	; 14 bytes
	php
	pha
	lda #40
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 364)
	; 14 bytes
	php
	pha
	lda #41
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 365)
	; 14 bytes
	php
	pha
	lda #42
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 366)
	; 14 bytes
	php
	pha
	lda #43
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 367)
	; 14 bytes
	php
	pha
	lda #44
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 368)
	; 14 bytes
	php
	pha
	lda #45
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 369)
	; 14 bytes
	php
	pha
	lda #46
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 370)
	; 14 bytes
	php
	pha
	lda #47
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.else
; 371 to 390
.if(n = 371)
	; 14 bytes
	php
	pha
	lda #48
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 372)
	; 14 bytes
	php
	pha
	lda #49
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 373)
	; 14 bytes
	php
	pha
	lda #50
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 374)
	; 14 bytes
	php
	pha
	lda #51
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 375)
	; 14 bytes
	php
	pha
	lda #52
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 376)
	; 14 bytes
	php
	pha
	lda #53
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 377)
	; 14 bytes
	php
	pha
	lda #54
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 378)
	; 14 bytes
	php
	pha
	lda #55
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 379)
	; 14 bytes
	php
	pha
	lda #56
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 380)
	; 14 bytes
	php
	pha
	lda #57
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 381)
	; 14 bytes
	php
	pha
	lda #58
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 382)
	; 14 bytes
	php
	pha
	lda #59
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 383)
	; 14 bytes
	php
	pha
	lda #60
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 384)
	; 14 bytes
	php
	pha
	lda #61
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 385)
	; 14 bytes
	php
	pha
	lda #62
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 386)
	; 14 bytes
	php
	pha
	lda #63
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 387)
	; 14 bytes
	php
	pha
	lda #64
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 388)
	; 14 bytes
	php
	pha
	lda #65
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 389)
	; 14 bytes
	php
	pha
	lda #66
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 390)
	; 14 bytes
	php
	pha
	lda #67
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 14 bytes
	php
	pha
	lda #68
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 392)
	; 14 bytes
	php
	pha
	lda #69
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 393)
	; 14 bytes
	php
	pha
	lda #70
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 394)
	; 14 bytes
	php
	pha
	lda #71
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 395)
	; 14 bytes
	php
	pha
	lda #72
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 396)
	; 14 bytes
	php
	pha
	lda #73
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 397)
	; 14 bytes
	php
	pha
	lda #74
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 398)
	; 14 bytes
	php
	pha
	lda #75
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 399)
	; 14 bytes
	php
	pha
	lda #76
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 400)
	; 14 bytes
	php
	pha
	lda #77
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 401)
	; 14 bytes
	php
	pha
	lda #78
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 402)
	; 14 bytes
	php
	pha
	lda #79
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 403)
	; 14 bytes
	php
	pha
	lda #80
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 404)
	; 14 bytes
	php
	pha
	lda #81
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 405)
	; 14 bytes
	php
	pha
	lda #82
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 406)
	; 14 bytes
	php
	pha
	lda #83
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 407)
	; 14 bytes
	php
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 408)
	; 14 bytes
	php
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 409)
	; 14 bytes
	php
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.else
; 410 to 429
.if(n = 410)
	; 14 bytes
	php
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 411)
	; 14 bytes
	php
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 412)
	; 14 bytes
	php
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 413)
	; 14 bytes
	php
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 414)
	; 14 bytes
	php
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 415)
	; 14 bytes
	php
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 416)
	; 14 bytes
	php
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 417)
	; 14 bytes
	php
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 418)
	; 14 bytes
	php
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 419)
	; 14 bytes
	php
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 420)
	; 14 bytes
	php
	pha
	lda #97
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 421)
	; 14 bytes
	php
	pha
	lda #98
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 422)
	; 14 bytes
	php
	pha
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 423)
	; 14 bytes
	php
	pha
	lda #100
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 424)
	; 14 bytes
	php
	pha
	lda #101
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 425)
	; 14 bytes
	php
	pha
	lda #102
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 426)
	; 14 bytes
	php
	pha
	lda #103
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 427)
	; 14 bytes
	php
	pha
	lda #104
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 428)
	; 14 bytes
	php
	pha
	lda #105
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 429)
	; 14 bytes
	php
	pha
	lda #106
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 14 bytes
	php
	pha
	lda #107
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 431)
	; 14 bytes
	php
	pha
	lda #108
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 432)
	; 14 bytes
	php
	pha
	lda #109
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 433)
	; 14 bytes
	php
	pha
	lda #110
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 434)
	; 14 bytes
	php
	pha
	lda #111
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 435)
	; 14 bytes
	php
	pha
	lda #112
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 436)
	; 14 bytes
	php
	pha
	lda #113
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 437)
	; 14 bytes
	php
	pha
	lda #114
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 438)
	; 14 bytes
	php
	pha
	lda #115
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 439)
	; 14 bytes
	php
	pha
	lda #116
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 440)
	; 14 bytes
	php
	pha
	lda #117
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 441)
	; 14 bytes
	php
	pha
	lda #118
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 442)
	; 14 bytes
	php
	pha
	lda #119
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 443)
	; 14 bytes
	php
	pha
	lda #120
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 444)
	; 14 bytes
	php
	pha
	lda #121
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 445)
	; 14 bytes
	php
	pha
	lda #122
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 446)
	; 14 bytes
	php
	pha
	lda #123
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 447)
	; 14 bytes
	php
	pha
	lda #124
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 448)
	; 14 bytes
	php
	pha
	lda #125
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.else
; 449 to 468
.if(n = 449)
	; 14 bytes
	php
	pha
	lda #126
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 450)
	; 14 bytes
	php
	pha
	lda #127
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 451)
	; 14 bytes
	php
	pha
	lda #128
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 452)
	; 14 bytes
	php
	pha
	lda #129
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 453)
	; 14 bytes
	php
	pha
	lda #130
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 454)
	; 14 bytes
	php
	pha
	lda #131
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 455)
	; 14 bytes
	php
	pha
	lda #132
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 456)
	; 14 bytes
	php
	pha
	lda #133
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 457)
	; 14 bytes
	php
	pha
	lda #134
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 458)
	; 14 bytes
	php
	pha
	lda #135
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 459)
	; 14 bytes
	php
	pha
	lda #136
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 460)
	; 14 bytes
	php
	pha
	lda #137
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 461)
	; 14 bytes
	php
	pha
	lda #138
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 462)
	; 14 bytes
	php
	pha
	lda #139
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 463)
	; 14 bytes
	php
	pha
	lda #140
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 464)
	; 14 bytes
	php
	pha
	lda #141
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 465)
	; 14 bytes
	php
	pha
	lda #142
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 466)
	; 14 bytes
	php
	pha
	lda #143
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 467)
	; 14 bytes
	php
	pha
	lda #144
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 468)
	; 14 bytes
	php
	pha
	lda #145
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 469) && (n < 547))
.if((n >= 469) && (n < 508))
.if((n >= 469) && (n < 488))
; 469 to 487
.if(n = 469)
	; 14 bytes
	php
	pha
	lda #146
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 470)
	; 14 bytes
	php
	pha
	lda #147
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 471)
	; 14 bytes
	php
	pha
	lda #148
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 472)
	; 14 bytes
	php
	pha
	lda #149
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 473)
	; 14 bytes
	php
	pha
	lda #150
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 474)
	; 14 bytes
	php
	pha
	lda #151
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 475)
	; 14 bytes
	php
	pha
	lda #152
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 476)
	; 14 bytes
	php
	pha
	lda #153
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 477)
	; 14 bytes
	php
	pha
	lda #154
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 478)
	; 14 bytes
	php
	pha
	lda #155
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 479)
	; 14 bytes
	php
	pha
	lda #156
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 480)
	; 14 bytes
	php
	pha
	lda #157
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 481)
	; 14 bytes
	php
	pha
	lda #158
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 482)
	; 14 bytes
	php
	pha
	lda #159
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 483)
	; 14 bytes
	php
	pha
	lda #160
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 484)
	; 14 bytes
	php
	pha
	lda #161
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 485)
	; 14 bytes
	php
	pha
	lda #162
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 486)
	; 14 bytes
	php
	pha
	lda #163
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 487)
	; 14 bytes
	php
	pha
	lda #164
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.endif
.else
; 488 to 507
.if(n = 488)
	; 14 bytes
	php
	pha
	lda #165
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 489)
	; 14 bytes
	php
	pha
	lda #166
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 490)
	; 14 bytes
	php
	pha
	lda #167
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 491)
	; 14 bytes
	php
	pha
	lda #168
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 492)
	; 14 bytes
	php
	pha
	lda #169
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 493)
	; 14 bytes
	php
	pha
	lda #170
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 494)
	; 14 bytes
	php
	pha
	lda #171
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 495)
	; 14 bytes
	php
	pha
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 496)
	; 14 bytes
	php
	pha
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 497)
	; 15 bytes
	php
	pha
	nop
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 498)
	; 15 bytes
	php
	pha
	nop
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif((n > 498 && n < 508))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 508 to 546
.if((n > 507 && n < 547))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 547 to 625
.if((n > 546 && n < 626))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 1251))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 2501))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 5001))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 15 bytes
	php
	pha
	txa
	pha
	ldx #>(n-59)
	lda #<(n-59)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif

.endif
	rest
.endmacro
