; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Z+N, C, and V
; Will preserve A, X, Y, S, D, and I (interrupt-disable flag)
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
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 291
; - 9 bytes or less when n <= 302
; -10 bytes or less when n <= 304
; -11 bytes or less when n <= 315
; -12 bytes or less when n <= 489
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=2, avg=3.7, max=5
; - When 21 <= n <= 30: min=4, avg=5.3, max=6
; - When 31 <= n <= 50: min=6, avg=6.9, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=9, avg=11.9, max=13
; - When 501 <= n <= 750: min=11, avg=13.0, max=13
; - When 751 <= n <= 1000: min=11, avg=13.0, max=13
; - When 1001 <= n <= 2000: min=11, avg=13.0, max=13
; - When 2001 <= n <= 3000: min=11, avg=13.0, max=13
; - When 3001 <= n <= 5000: min=11, avg=13.0, max=13
; - When 5001 <= n <= 7500: min=11, avg=13.0, max=13
; - When 7501 <= n <= 10000: min=11, avg=13.0, max=13
; - When 10001 <= n <= 20000: min=11, avg=13.0, max=13
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
	cmp $C5
.elseif((n > 3 && n < 7) || (n > 15 && n < 19) || n = 28 || n = 31 || n = 32 || n = 290 || n = 291 || n = 305 || n = 306)
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
.elseif(n = 9 || n = 11 || (n > 18 && n < 24) || n = 33)
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
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 39)
	; 4..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 29 || n = 30 || n = 303 || n = 304)
	; 6..10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 33 && n < 39) || (n > 39 && n < 290))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif((n > 291 && n < 297))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 296 && n < 303) || n = 308 || n = 309)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 307)
	; 11 bytes
	pha
	txa
	pha
	lax #1
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif((n > 309 && n < 313))
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if((n > 312 && n < 316))
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 316)
	; 12 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 317)
	; 12 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 318)
	; 12 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 319)
	; 12 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 320)
	; 12 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 321)
	; 12 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 322)
	; 12 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 323)
	; 12 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 324)
	; 12 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 325)
	; 12 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 326)
	; 12 bytes
	pha
	lda #10
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 327)
	; 12 bytes
	pha
	lda #11
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 328)
	; 12 bytes
	pha
	lda #12
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 329)
	; 12 bytes
	pha
	lda #13
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 330)
	; 12 bytes
	pha
	lda #14
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 331)
	; 12 bytes
	pha
	lda #15
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.else
; 332 to 351
.if(n = 332)
	; 12 bytes
	pha
	lda #16
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 333)
	; 12 bytes
	pha
	lda #17
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 334)
	; 12 bytes
	pha
	lda #18
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 335)
	; 12 bytes
	pha
	lda #19
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 336)
	; 12 bytes
	pha
	lda #20
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 337)
	; 12 bytes
	pha
	lda #21
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 338)
	; 12 bytes
	pha
	lda #22
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 339)
	; 12 bytes
	pha
	lda #23
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 340)
	; 12 bytes
	pha
	lda #24
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 341)
	; 12 bytes
	pha
	lda #25
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 342)
	; 12 bytes
	pha
	lda #26
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 343)
	; 12 bytes
	pha
	lda #27
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 344)
	; 12 bytes
	pha
	lda #28
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 345)
	; 12 bytes
	pha
	lda #29
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 346)
	; 12 bytes
	pha
	lda #30
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 347)
	; 12 bytes
	pha
	lda #31
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 348)
	; 12 bytes
	pha
	lda #32
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 349)
	; 12 bytes
	pha
	lda #33
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 350)
	; 12 bytes
	pha
	lda #34
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 351)
	; 12 bytes
	pha
	lda #35
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.endif
.else
.if((n >= 352) && (n < 371))
; 352 to 370
.if(n = 352)
	; 12 bytes
	pha
	lda #36
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 353)
	; 12 bytes
	pha
	lda #37
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 354)
	; 12 bytes
	pha
	lda #38
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 355)
	; 12 bytes
	pha
	lda #39
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 356)
	; 12 bytes
	pha
	lda #40
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 357)
	; 12 bytes
	pha
	lda #41
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 358)
	; 12 bytes
	pha
	lda #42
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 359)
	; 12 bytes
	pha
	lda #43
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 360)
	; 12 bytes
	pha
	lda #44
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 361)
	; 12 bytes
	pha
	lda #45
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 362)
	; 12 bytes
	pha
	lda #46
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 363)
	; 12 bytes
	pha
	lda #47
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 364)
	; 12 bytes
	pha
	lda #48
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 365)
	; 12 bytes
	pha
	lda #49
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 366)
	; 12 bytes
	pha
	lda #50
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 367)
	; 12 bytes
	pha
	lda #51
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 368)
	; 12 bytes
	pha
	lda #52
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 369)
	; 12 bytes
	pha
	lda #53
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 370)
	; 12 bytes
	pha
	lda #54
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.else
; 371 to 390
.if(n = 371)
	; 12 bytes
	pha
	lda #55
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 372)
	; 12 bytes
	pha
	lda #56
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 373)
	; 12 bytes
	pha
	lda #57
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 374)
	; 12 bytes
	pha
	lda #58
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 375)
	; 12 bytes
	pha
	lda #59
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 376)
	; 12 bytes
	pha
	lda #60
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 377)
	; 12 bytes
	pha
	lda #61
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 378)
	; 12 bytes
	pha
	lda #62
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 379)
	; 12 bytes
	pha
	lda #63
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 380)
	; 12 bytes
	pha
	lda #64
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 381)
	; 12 bytes
	pha
	lda #65
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 382)
	; 12 bytes
	pha
	lda #66
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 383)
	; 12 bytes
	pha
	lda #67
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 384)
	; 12 bytes
	pha
	lda #68
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 385)
	; 12 bytes
	pha
	lda #69
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 386)
	; 12 bytes
	pha
	lda #70
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 387)
	; 12 bytes
	pha
	lda #71
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 388)
	; 12 bytes
	pha
	lda #72
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 389)
	; 12 bytes
	pha
	lda #73
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 390)
	; 12 bytes
	pha
	lda #74
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 391) && (n < 430))
.if((n >= 391) && (n < 410))
; 391 to 409
.if(n = 391)
	; 12 bytes
	pha
	lda #75
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 392)
	; 12 bytes
	pha
	lda #76
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 393)
	; 12 bytes
	pha
	lda #77
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 394)
	; 12 bytes
	pha
	lda #78
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 395)
	; 12 bytes
	pha
	lda #79
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 396)
	; 12 bytes
	pha
	lda #80
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 397)
	; 12 bytes
	pha
	lda #81
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 398)
	; 12 bytes
	pha
	lda #82
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 399)
	; 12 bytes
	pha
	lda #83
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 400)
	; 12 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 401)
	; 12 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 402)
	; 12 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 403)
	; 12 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 404)
	; 12 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 405)
	; 12 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 406)
	; 12 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 407)
	; 12 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 408)
	; 12 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 409)
	; 12 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.else
; 410 to 429
.if(n = 410)
	; 12 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 411)
	; 12 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 412)
	; 12 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 413)
	; 12 bytes
	pha
	lda #97
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 414)
	; 12 bytes
	pha
	lda #98
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 415)
	; 12 bytes
	pha
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 416)
	; 12 bytes
	pha
	lda #100
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 417)
	; 12 bytes
	pha
	lda #101
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 418)
	; 12 bytes
	pha
	lda #102
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 419)
	; 12 bytes
	pha
	lda #103
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 420)
	; 12 bytes
	pha
	lda #104
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 421)
	; 12 bytes
	pha
	lda #105
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 422)
	; 12 bytes
	pha
	lda #106
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 423)
	; 12 bytes
	pha
	lda #107
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 424)
	; 12 bytes
	pha
	lda #108
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 425)
	; 12 bytes
	pha
	lda #109
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 426)
	; 12 bytes
	pha
	lda #110
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 427)
	; 12 bytes
	pha
	lda #111
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 428)
	; 12 bytes
	pha
	lda #112
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 429)
	; 12 bytes
	pha
	lda #113
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.endif
.else
.if((n >= 430) && (n < 449))
; 430 to 448
.if(n = 430)
	; 12 bytes
	pha
	lda #114
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 431)
	; 12 bytes
	pha
	lda #115
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 432)
	; 12 bytes
	pha
	lda #116
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 433)
	; 12 bytes
	pha
	lda #117
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 434)
	; 12 bytes
	pha
	lda #118
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 435)
	; 12 bytes
	pha
	lda #119
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 436)
	; 12 bytes
	pha
	lda #120
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 437)
	; 12 bytes
	pha
	lda #121
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 438)
	; 12 bytes
	pha
	lda #122
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 439)
	; 12 bytes
	pha
	lda #123
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 440)
	; 12 bytes
	pha
	lda #124
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 441)
	; 12 bytes
	pha
	lda #125
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 442)
	; 12 bytes
	pha
	lda #126
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 443)
	; 12 bytes
	pha
	lda #127
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 444)
	; 12 bytes
	pha
	lda #128
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 445)
	; 12 bytes
	pha
	lda #129
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 446)
	; 12 bytes
	pha
	lda #130
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 447)
	; 12 bytes
	pha
	lda #131
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 448)
	; 12 bytes
	pha
	lda #132
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.else
; 449 to 468
.if(n = 449)
	; 12 bytes
	pha
	lda #133
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 450)
	; 12 bytes
	pha
	lda #134
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 451)
	; 12 bytes
	pha
	lda #135
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 452)
	; 12 bytes
	pha
	lda #136
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 453)
	; 12 bytes
	pha
	lda #137
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 454)
	; 12 bytes
	pha
	lda #138
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 455)
	; 12 bytes
	pha
	lda #139
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 456)
	; 12 bytes
	pha
	lda #140
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 457)
	; 12 bytes
	pha
	lda #141
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 458)
	; 12 bytes
	pha
	lda #142
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 459)
	; 12 bytes
	pha
	lda #143
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 460)
	; 12 bytes
	pha
	lda #144
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 461)
	; 12 bytes
	pha
	lda #145
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 462)
	; 12 bytes
	pha
	lda #146
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 463)
	; 12 bytes
	pha
	lda #147
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 464)
	; 12 bytes
	pha
	lda #148
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 465)
	; 12 bytes
	pha
	lda #149
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 466)
	; 12 bytes
	pha
	lda #150
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 467)
	; 12 bytes
	pha
	lda #151
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 468)
	; 12 bytes
	pha
	lda #152
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469)
	; 12 bytes
	pha
	lda #153
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 470)
	; 12 bytes
	pha
	lda #154
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 471)
	; 12 bytes
	pha
	lda #155
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 472)
	; 12 bytes
	pha
	lda #156
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 473)
	; 12 bytes
	pha
	lda #157
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 474)
	; 12 bytes
	pha
	lda #158
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 475)
	; 12 bytes
	pha
	lda #159
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 476)
	; 12 bytes
	pha
	lda #160
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 477)
	; 12 bytes
	pha
	lda #161
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 478)
	; 12 bytes
	pha
	lda #162
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 479)
	; 12 bytes
	pha
	lda #163
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 480)
	; 12 bytes
	pha
	lda #164
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 481)
	; 12 bytes
	pha
	lda #165
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 482)
	; 12 bytes
	pha
	lda #166
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 483)
	; 12 bytes
	pha
	lda #167
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 484)
	; 12 bytes
	pha
	lda #168
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 485)
	; 12 bytes
	pha
	lda #169
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 486)
	; 12 bytes
	pha
	lda #170
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 487)
	; 12 bytes
	pha
	lda #171
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 488)
	; 12 bytes
	pha
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 489)
	; 12 bytes
	pha
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 490)
	; 13 bytes
	pha
	nop
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 491)
	; 13 bytes
	pha
	nop
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif((n > 491 && n < 547))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.else
; 547 to 625
.if((n > 546 && n < 564) || n = 565 || (n > 566 && n < 626))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 564)
	; 11 bytes
	pha
	txa
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 566)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 821) || n = 822 || (n > 823 && n < 1078) || n = 1079 || (n > 1080 && n < 1251))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 821)
	; 11 bytes
	pha
	txa
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 823 || n = 1080)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1078)
	; 11 bytes
	pha
	txa
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1335) || n = 1336 || (n > 1337 && n < 1592) || n = 1593 || (n > 1594 && n < 1849) || n = 1850 || (n > 1851 && n < 2106) || n = 2107 || (n > 2108 && n < 2363) || n = 2364 || (n > 2365 && n < 2501))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 1335)
	; 11 bytes
	pha
	txa
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 1337 || n = 1594 || n = 1851 || n = 2108 || n = 2365)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1592)
	; 11 bytes
	pha
	txa
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 1849)
	; 11 bytes
	pha
	txa
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2106)
	; 11 bytes
	pha
	txa
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2363)
	; 11 bytes
	pha
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2620) || n = 2621 || (n > 2622 && n < 2877) || n = 2878 || (n > 2879 && n < 3134) || n = 3135 || (n > 3136 && n < 3391) || n = 3392 || (n > 3393 && n < 3648) || n = 3649 || (n > 3650 && n < 3905) || n = 3906 || (n > 3907 && n < 4162) || n = 4163 || (n > 4164 && n < 4419) || n = 4420 || (n > 4421 && n < 4676) || n = 4677 || (n > 4678 && n < 4933) || n = 4934 || (n > 4935 && n < 5001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2620)
	; 11 bytes
	pha
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2622 || n = 2879 || n = 3136 || n = 3393 || n = 3650 || n = 3907 || n = 4164 || n = 4421 || n = 4678 || n = 4935)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2877)
	; 11 bytes
	pha
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3134)
	; 11 bytes
	pha
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3391)
	; 11 bytes
	pha
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3648)
	; 11 bytes
	pha
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3905)
	; 11 bytes
	pha
	txa
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4162)
	; 11 bytes
	pha
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4419)
	; 11 bytes
	pha
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4676)
	; 11 bytes
	pha
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4933)
	; 11 bytes
	pha
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5190) || n = 5191 || (n > 5192 && n < 5447) || n = 5448 || (n > 5449 && n < 5704) || n = 5705 || (n > 5706 && n < 5961) || n = 5962 || (n > 5963 && n < 6218) || n = 6219 || (n > 6220 && n < 6475) || n = 6476 || (n > 6477 && n < 6732) || n = 6733 || (n > 6734 && n < 6989) || n = 6990 || (n > 6991 && n < 7246) || n = 7247 || (n > 7248 && n < 7503) || n = 7504 || (n > 7505 && n < 7760) || n = 7761 || (n > 7762 && n < 8017) || n = 8018 || (n > 8019 && n < 8274) || n = 8275 || (n > 8276 && n < 8531) || n = 8532 || (n > 8533 && n < 8788) || n = 8789 || (n > 8790 && n < 9045) || n = 9046 || (n > 9047 && n < 9302) || n = 9303 || (n > 9304 && n < 9559) || n = 9560 || (n > 9561 && n < 9816) || n = 9817 || (n > 9818 && n < 10001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5190)
	; 11 bytes
	pha
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5192 || n = 5449 || n = 5706 || n = 5963 || n = 6220 || n = 6477 || n = 6734 || n = 6991 || n = 7248 || n = 7505 || n = 7762 || n = 8019 || n = 8276 || n = 8533 || n = 8790 || n = 9047 || n = 9304 || n = 9561 || n = 9818)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5447)
	; 11 bytes
	pha
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5704)
	; 11 bytes
	pha
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5961)
	; 11 bytes
	pha
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 6218)
	; 11 bytes
	pha
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 6475)
	; 11 bytes
	pha
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 6732)
	; 11 bytes
	pha
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 6989)
	; 11 bytes
	pha
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 7246)
	; 11 bytes
	pha
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 7503)
	; 11 bytes
	pha
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 7760)
	; 11 bytes
	pha
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 8017)
	; 11 bytes
	pha
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 8274)
	; 11 bytes
	pha
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 8531)
	; 11 bytes
	pha
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 8788)
	; 11 bytes
	pha
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 9045)
	; 11 bytes
	pha
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 9302)
	; 11 bytes
	pha
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 9559)
	; 11 bytes
	pha
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 9816)
	; 11 bytes
	pha
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10073) || n = 10074 || (n > 10075 && n < 10330) || n = 10331 || (n > 10332 && n < 10587) || n = 10588 || (n > 10589 && n < 10844) || n = 10845 || (n > 10846 && n < 11101) || n = 11102 || (n > 11103 && n < 11358) || n = 11359 || (n > 11360 && n < 11615) || n = 11616 || (n > 11617 && n < 11872) || n = 11873 || (n > 11874 && n < 12129) || n = 12130 || (n > 12131 && n < 12386) || n = 12387 || (n > 12388 && n < 12643) || n = 12644 || (n > 12645 && n < 12900) || n = 12901 || (n > 12902 && n < 13157) || n = 13158 || (n > 13159 && n < 13414) || n = 13415 || (n > 13416 && n < 13671) || n = 13672 || (n > 13673 && n < 13928) || n = 13929 || (n > 13930 && n < 14185) || n = 14186 || (n > 14187 && n < 14442) || n = 14443 || (n > 14444 && n < 14699) || n = 14700 || (n > 14701 && n < 14956) || n = 14957 || (n > 14958 && n < 15001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 10073)
	; 11 bytes
	pha
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 10075 || n = 10332 || n = 10589 || n = 10846 || n = 11103 || n = 11360 || n = 11617 || n = 11874 || n = 12131 || n = 12388 || n = 12645 || n = 12902 || n = 13159 || n = 13416 || n = 13673 || n = 13930 || n = 14187 || n = 14444 || n = 14701 || n = 14958)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10330)
	; 11 bytes
	pha
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 10587)
	; 11 bytes
	pha
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 10844)
	; 11 bytes
	pha
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 11101)
	; 11 bytes
	pha
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 11358)
	; 11 bytes
	pha
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 11615)
	; 11 bytes
	pha
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 11872)
	; 11 bytes
	pha
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 12129)
	; 11 bytes
	pha
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 12386)
	; 11 bytes
	pha
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 12643)
	; 11 bytes
	pha
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 12900)
	; 11 bytes
	pha
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 13157)
	; 11 bytes
	pha
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 13414)
	; 11 bytes
	pha
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 13671)
	; 11 bytes
	pha
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 13928)
	; 11 bytes
	pha
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 14185)
	; 11 bytes
	pha
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 14442)
	; 11 bytes
	pha
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 14699)
	; 11 bytes
	pha
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 14956)
	; 11 bytes
	pha
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15213) || n = 15214 || (n > 15215 && n < 15470) || n = 15471 || (n > 15472 && n < 15727) || n = 15728 || (n > 15729 && n < 15984) || n = 15985 || (n > 15986 && n < 16241) || n = 16242 || (n > 16243 && n < 16498) || n = 16499 || (n > 16500 && n < 16755) || n = 16756 || (n > 16757 && n < 17012) || n = 17013 || (n > 17014 && n < 17269) || n = 17270 || (n > 17271 && n < 17526) || n = 17527 || (n > 17528 && n < 17783) || n = 17784 || (n > 17785 && n < 18040) || n = 18041 || (n > 18042 && n < 18297) || n = 18298 || (n > 18299 && n < 18554) || n = 18555 || (n > 18556 && n < 18811) || n = 18812 || (n > 18813 && n < 19068) || n = 19069 || (n > 19070 && n < 19325) || n = 19326 || (n > 19327 && n < 19582) || n = 19583 || (n > 19584 && n < 19839) || n = 19840 || (n > 19841 && n < 20001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 15213)
	; 11 bytes
	pha
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 15215 || n = 15472 || n = 15729 || n = 15986 || n = 16243 || n = 16500 || n = 16757 || n = 17014 || n = 17271 || n = 17528 || n = 17785 || n = 18042 || n = 18299 || n = 18556 || n = 18813 || n = 19070 || n = 19327 || n = 19584 || n = 19841)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15470)
	; 11 bytes
	pha
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 15727)
	; 11 bytes
	pha
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 15984)
	; 11 bytes
	pha
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 16241)
	; 11 bytes
	pha
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 16498)
	; 11 bytes
	pha
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 16755)
	; 11 bytes
	pha
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 17012)
	; 11 bytes
	pha
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 17269)
	; 11 bytes
	pha
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 17526)
	; 11 bytes
	pha
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 17783)
	; 11 bytes
	pha
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 18040)
	; 11 bytes
	pha
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 18297)
	; 11 bytes
	pha
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 18554)
	; 11 bytes
	pha
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 18811)
	; 11 bytes
	pha
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 19068)
	; 11 bytes
	pha
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 19325)
	; 11 bytes
	pha
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 19582)
	; 11 bytes
	pha
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 19839)
	; 11 bytes
	pha
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.endif

.endif
	rest
.endmacro
