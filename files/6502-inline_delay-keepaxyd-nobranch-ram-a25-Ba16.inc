; delay_n, delays n cycles with small code
; Will clobber Z+N, C, and V
; Will preserve A, X, Y, S, D, and I (interrupt-disable flag)
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
; Requires a function called "delay_256a_16_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A*256 clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A*256+16 clocks (including JSR)
;; Depends on delay_a_25_clocks
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_256a_16_clocks:
;	cmp #0
;	bne :+
;	rts
;delay_256a_11_clocks_:
;:       pha
;	 lda #(256-25-7-2-2-3)
;	 jsr delay_a_25_clocks
;	pla
;	sec
;	sbc #1
;	bne :-
;	rts
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
; - 2 bytes or less when n <= 5
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 32
; - 8 bytes or less when n <= 291
; - 9 bytes or less when n <= 296
; -10 bytes or less when n <= 304
; -11 bytes or less when n <= 306
; -12 bytes or less when n <= 489
; -13 bytes or less when n <= 491
; -14 bytes or less when n <= 496
; -15 bytes or less when n <= 504
; -16 bytes or less when n <= 506
; -17 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.6, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=7.0, max=8
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=10
; - When 301 <= n <= 500: min=10, avg=12.1, max=15
; - When 501 <= n <= 750: min=7, avg=12.4, max=17
; - When 751 <= n <= 1000: min=7, avg=12.5, max=17
; - When 1001 <= n <= 2000: min=7, avg=12.6, max=17
; - When 2001 <= n <= 3000: min=7, avg=12.6, max=17
; - When 3001 <= n <= 5000: min=7, avg=12.6, max=17
; - When 5001 <= n <= 7500: min=7, avg=12.6, max=17
; - When 7501 <= n <= 10000: min=7, avg=12.6, max=17
; - When 10001 <= n <= 20000: min=7, avg=12.6, max=17
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
.elseif(n = 4 || n = 6 || (n > 15 && n < 19) || n = 31 || n = 32 || n = 290 || n = 291 || n = 297 || n = 298 || n = 305 || n = 306)
	; 2..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || (n > 20 && n < 24) || n = 33)
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
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 20 || n = 27 || n = 29 || n = 30 || n = 303 || n = 304)
	; 5..10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || (n > 298 && n < 302))
	; 6..10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 25)
	; 7 bytes
	pha
	lda #0
	jsr delay_256a_16_clocks
	pla
.elseif(n = 26 || n = 28 || n = 302)
	; 6..10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 33 && n < 290))
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
.elseif((n > 306 && n < 313))
	; 12 bytes
	pha
	lda #>(n-61)
	jsr delay_a_25_clocks
	lda #<(n-61)
	jsr delay_a_25_clocks
	pla
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
.if((n >= 313) && (n < 352))
.if((n >= 313) && (n < 332))
; 313 to 331
.if((n > 312 && n < 317))
	; 12 bytes
	pha
	lda #>(n-61)
	jsr delay_a_25_clocks
	lda #<(n-61)
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
.if((n >= 469) && (n < 508))
.if((n >= 469) && (n < 488))
; 469 to 487
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
.endif
.else
; 488 to 507
.if(n = 488)
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
.elseif((n > 491 && n < 499))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 499)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 500)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 501)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 502)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 503)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 504)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 505)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 506)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 507)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.endif
.else
.if((n >= 508) && (n < 527))
; 508 to 526
.if(n = 508)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 509)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 510)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 511)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 512)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #(n-5)/3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 513)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #n/3-1
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 514)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #(n-1)/3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 515)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 516)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 517)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 518)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 519)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 520)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #(n-1)/3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 521)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 522)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #n/3-1
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 523)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 524)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #(n-5)/3
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 525)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 526)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.else
; 527 to 546
.if(n = 527)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 528)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 529)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 530)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 531)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 532)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 533)
	; 10 bytes
	lda #2
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 534 || n = 536 || n = 538 || n = 539 || n = 541 || n = 543 || n = 546)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 535 || n = 540)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 537)
	; 7 bytes
	pha
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 542)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 544)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 545)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.endif
.else
; 547 to 625
.if(n = 547 || n = 552 || n = 557)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 548 || n = 550 || n = 553 || n = 554)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 549 || n = 556)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 551 || n = 558)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 555)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 558 && n < 564))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 563 && n < 626))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if((n > 625 && n < 704))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 704) && (n < 743))
; 704 to 742
.if((n > 703 && n < 738))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 738 || n = 739)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 739 && n < 743))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 743) && (n < 762))
; 743 to 761
.if((n > 742 && n < 747))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 747)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 748)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 749)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 750)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 751)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 752)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 753)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 754)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 755)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 756)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 757)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 758)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 759)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 760)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 761)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 762 to 781
.if(n = 762)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 763)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 764)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 765)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 766)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 767)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 768)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 769)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 770)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 771)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 772)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 773)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 774)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 775)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 776)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 777)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 778)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 779)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 780)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 781)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
; 782 to 937
.if(n = 782)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 783)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 784)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 785)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 786)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 787)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 788)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 789)
	; 10 bytes
	lda #3
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 790 || n = 792 || n = 794 || n = 795 || n = 797 || n = 799 || n = 802 || n = 804 || n = 806 || n = 809 || n = 810)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 791 || n = 796)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 793)
	; 7 bytes
	pha
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 798)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 800)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 801)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 803 || n = 808 || n = 813)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 805 || n = 812)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 807 || n = 814)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 811)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 814 && n < 820))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 819 && n < 938))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if((n > 937 && n < 994))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 994 || n = 995)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 995 && n < 1003))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1003)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1004)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1005)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1006)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1007)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1008)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1009)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1010)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1011)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1012)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1013)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1014)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1015)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 1016) && (n < 1055))
.if((n >= 1016) && (n < 1035))
; 1016 to 1034
.if(n = 1016)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1017)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1018)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1019)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1020)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1021)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1022)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1023)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1024)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1025)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1026)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1027)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1028)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1029)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1030)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1031)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1032)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1033)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1034)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 1035 to 1054
.if(n = 1035)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1036)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1037)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1038)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1039)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1040)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1041)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1042)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1043)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 1044)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1045)
	; 10 bytes
	lda #4
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 1046 || n = 1048 || n = 1050 || n = 1051 || n = 1053)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1047 || n = 1052)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1049)
	; 7 bytes
	pha
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1054)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.endif
.else
; 1055 to 1093
.if(n = 1055 || n = 1058 || n = 1060 || n = 1062 || n = 1065 || n = 1066)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1056)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1057)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1059 || n = 1064 || n = 1069)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1061 || n = 1068)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1063 || n = 1070)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1067)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 1070 && n < 1076))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 1075 && n < 1094))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 1094 to 1250
.if((n > 1093 && n < 1250))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1250)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
.if((n >= 1251) && (n < 1290))
.if((n >= 1251) && (n < 1270))
; 1251 to 1269
.if(n = 1251)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 1251 && n < 1259))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1259)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1260)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1261)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1262)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1263)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1264)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1265)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1266)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1267)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1268)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1269)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 1270 to 1289
.if(n = 1270)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1271)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1272)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1273)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1274)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1275)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1276)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1277)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1278)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1279)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1280)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1281)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1282)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1283)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1284)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1285)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1286)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1287)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1288)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1289)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 1290 to 1328
.if(n = 1290)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1291)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1292)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1293)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1294)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1295)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1296)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1297)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1298)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1299)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 1300)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1301)
	; 10 bytes
	lda #5
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 1302 || n = 1304 || n = 1306 || n = 1307 || n = 1309 || n = 1311 || n = 1314 || n = 1316 || n = 1318 || n = 1321 || n = 1322)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1303 || n = 1308)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1305)
	; 7 bytes
	pha
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1310)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1312)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1313)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1315 || n = 1320 || n = 1325)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1317 || n = 1324)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1319 || n = 1326)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1323)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1327 || n = 1328)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.else
; 1329 to 1406
.if((n > 1328 && n < 1332))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 1331 && n < 1407))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if((n > 1406 && n < 1485))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n > 1484 && n < 1506))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1506 || n = 1507)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 1507 && n < 1515))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1515)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1516)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1517)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1518)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1519)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1520)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1521)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1522)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1523)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 1524) && (n < 1543))
; 1524 to 1542
.if(n = 1524)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1525)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1526)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1527)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1528)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1529)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1530)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1531)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1532)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1533)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1534)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1535)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1536)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1537)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1538)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1539)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1540)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1541)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1542)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 1543 to 1562
.if(n = 1543)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1544)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1545)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1546)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1547)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1548)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1549)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1550)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1551)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1552)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1553)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1554)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1555)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 1556)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1557)
	; 10 bytes
	lda #6
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 1558 || n = 1560 || n = 1562)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1559)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1561)
	; 7 bytes
	pha
	lda #6
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1565 || n = 1567 || n = 1570 || n = 1572 || n = 1574 || n = 1577 || n = 1578)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1566)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1568)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1569)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1571 || n = 1576 || n = 1581)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1573 || n = 1580)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1575 || n = 1582)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1579)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 1582 && n < 1588))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 1587 && n < 1719))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if((n > 1718 && n < 1758))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 1758) && (n < 1777))
; 1758 to 1776
.if((n > 1757 && n < 1762))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1762 || n = 1763)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 1763 && n < 1771))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1771)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1772)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1773)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1774)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1775)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1776)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 1777 to 1796
.if(n = 1777)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1778)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1779)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1780)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1781)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1782)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1783)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1784)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1785)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1786)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1787)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #(n-67)/10
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1788)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1789)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1790)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1791)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1792)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1793)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1794)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1795)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1796)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #(n-67)/10
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1798)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1799)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1800)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1801)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1802)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1803)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1804)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1805)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1806)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1807)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1808)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1809)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1810)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1811)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 1812)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1813)
	; 10 bytes
	lda #7
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 1814)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1815)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.else
; 1816 to 1835
.if(n = 1816 || n = 1818 || n = 1819 || n = 1821 || n = 1823 || n = 1826 || n = 1828 || n = 1830 || n = 1833 || n = 1834)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1817)
	; 7 bytes
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1820)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1822)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1824)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1825)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1827 || n = 1832)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1829)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1831)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1835)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1837)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1838)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 1838 && n < 1844))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 1843 && n < 1876))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if((n > 1875 && n < 2018))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2018 || n = 2019)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2019 && n < 2027))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2027)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2028)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2029)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2030)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2031)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2033)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2034)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2035)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2036)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2037)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2038)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2039)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2040)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2041)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2042)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2043)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2044)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2045)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2046)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2047)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2048)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2049)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2050)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 2051 to 2070
.if(n = 2051)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2052)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2053)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2054)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2055)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2056)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2057)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2058)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2059)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2060)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2061)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2062)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2063)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2064)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2065)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2066)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2067)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 2068)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2069)
	; 10 bytes
	lda #8
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 2070)
	; 8 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2076)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 2072 || n = 2074 || n = 2075 || n = 2077 || n = 2079 || n = 2082 || n = 2084 || n = 2086 || n = 2089 || n = 2090)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2073)
	; 7 bytes
	pha
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2078)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2080)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2081)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2083 || n = 2088 || n = 2093)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2085 || n = 2092)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2087 || n = 2094)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2091)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2094 && n < 2100))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 2099 && n < 2110))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 2110 to 2187
.if((n > 2109 && n < 2188))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n > 2187 && n < 2266))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if((n > 2265 && n < 2274))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2274 || n = 2275)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2275 && n < 2283))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2283)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2284)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2285)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2286)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2287)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2288)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2289)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2290)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2291)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2292)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2293)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2294)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2295)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2296)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2297)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2298)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2299)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2300)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2301)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2302)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2303)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2304)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2306)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2307)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2308)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2309)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2310)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2311)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2312)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2313)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2314)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2315)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2316)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2317)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2318)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2319)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2320)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2321)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2322)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2323)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2325)
	; 10 bytes
	lda #9
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 2326 || n = 2328 || n = 2330 || n = 2331 || n = 2333 || n = 2335 || n = 2338 || n = 2340 || n = 2342)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2327 || n = 2332)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 2329)
	; 7 bytes
	pha
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2334)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2336)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2337)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2339)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2341)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2343)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.else
; 2344 to 2500
.if(n = 2344 || n = 2349)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2345 || n = 2346)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2348)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2350)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 2350 && n < 2356))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 2355 && n < 2501))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
.if((n >= 2501) && (n < 2540))
; 2501 to 2539
.if((n > 2500 && n < 2530))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2530 || n = 2531)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2531 && n < 2539))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2539)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2541)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2542)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2543)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2544)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2545)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2546)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2547)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2548)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2549)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2550)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2551)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2552)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2553)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2554)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2555)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2556)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2557)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2558)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2560)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2561)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2562)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2563)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2564)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2565)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2566)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2567)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2568)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2569)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2570)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2571)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2572)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2573)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2574)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2575)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2576)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2577)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2578)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 2579 to 2656
.if(n = 2579)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 2580)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2581)
	; 10 bytes
	lda #10
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 2582 || n = 2584 || n = 2586 || n = 2587 || n = 2589 || n = 2591 || n = 2594 || n = 2596 || n = 2598 || n = 2601 || n = 2602)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2588)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 2585)
	; 7 bytes
	pha
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2590)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2592)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2593)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2595 || n = 2600 || n = 2605)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2597 || n = 2604)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2599 || n = 2606)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2603)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2606 && n < 2612))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 2611 && n < 2657))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 2657 to 2812
.if((n > 2656 && n < 2786))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2786 || n = 2787)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2787 && n < 2795))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2795)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2796)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2797)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2798)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2799)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2800)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2801)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2802)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2803)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2804)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2805)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2806)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2807)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2808)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2809)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2810)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2811)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2812)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
.if((n >= 2813) && (n < 2832))
; 2813 to 2831
.if(n = 2813)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2814)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2815)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2816)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2817)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2818)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2819)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2820)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2821)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2822)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2823)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2824)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2825)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2826)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2827)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2828)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2829)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2830)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2831)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 2832 to 2851
.if(n = 2832)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2833)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2834)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2835)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 2836)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2837)
	; 10 bytes
	lda #11
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 2838 || n = 2840 || n = 2842 || n = 2843 || n = 2845 || n = 2847 || n = 2850)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2839 || n = 2844)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 2841)
	; 7 bytes
	pha
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2846)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2848)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2849)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2851)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2854 || n = 2857 || n = 2858)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853 || n = 2860)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2855 || n = 2862)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2856 || n = 2861)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2859)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2862 && n < 2868))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 2867 && n < 2891))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 2891 to 2968
.if((n > 2890 && n < 2969))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n > 2968 && n < 3042))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3042 || n = 3043)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3043 && n < 3047))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if((n > 3046 && n < 3051))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3051)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3052)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3053)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3054)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3055)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3056)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3057)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3058)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3059)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3060)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3061)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3062)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3063)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3064)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3065)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3067)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3068)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3069)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3070)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3071)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3072)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3073)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3074)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3075)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3076)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3077)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3078)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3079)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3080)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3081)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3082)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3083)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3084)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3085)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 3086 to 3125
.if(n = 3086)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3087)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3088)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3089)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3090)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3091)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 3092)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3093)
	; 10 bytes
	lda #12
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 3094 || n = 3096 || n = 3098 || n = 3099 || n = 3101 || n = 3103 || n = 3106 || n = 3108 || n = 3110 || n = 3113 || n = 3114)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3095 || n = 3100)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 3097)
	; 7 bytes
	pha
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3102)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3104)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3105)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3107 || n = 3112 || n = 3117)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3109 || n = 3116)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3111 || n = 3118)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3115)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3118 && n < 3124))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3124 || n = 3125)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if((n > 3125 && n < 3282))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if((n > 3281 && n < 3298))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3298 || n = 3299)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3299 && n < 3307))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3307)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3308)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3309)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3310)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3311)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3312)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3313)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3314)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3315)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3316)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3317)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3318)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3319)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3320)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 3321) && (n < 3340))
; 3321 to 3339
.if(n = 3321)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3322)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3323)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3324)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3325)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3326)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3327)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3328)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3329)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3330)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3331)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3332)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3333)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3334)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3335)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3336)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3337)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3338)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3339)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 3340 to 3359
.if(n = 3340)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3341)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3342)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3343)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3344)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3345)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3346)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3347)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 3348)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3349)
	; 10 bytes
	lda #13
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 3350 || n = 3352 || n = 3354 || n = 3355 || n = 3357 || n = 3359)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3351 || n = 3356)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 3353)
	; 7 bytes
	pha
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3358)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.endif
.endif
.else
; 3360 to 3437
.if(n = 3360)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3361)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3362 || n = 3364 || n = 3366 || n = 3369 || n = 3370)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3368 || n = 3373)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3365 || n = 3372)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3367 || n = 3374)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3371)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3374 && n < 3380))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 3379 && n < 3438))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if((n > 3437 && n < 3516))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if((n > 3515 && n < 3554))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3554)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if(n = 3555)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3555 && n < 3563))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3563)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3564)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3565)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3566)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3567)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3568)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3569)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3570)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3571)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3572)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3573)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3575)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3576)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3577)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3578)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3579)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3580)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3581)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3582)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3583)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3584)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3585)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3586)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3587)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3588)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3589)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3590)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3591)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3592)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3593)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
; 3594 to 3750
.if(n = 3594)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3595)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3596)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3597)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3598)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3599)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3600)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3601)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3602)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3603)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 3604)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3605)
	; 10 bytes
	lda #14
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 3606 || n = 3608 || n = 3610 || n = 3611 || n = 3613 || n = 3615 || n = 3618 || n = 3620 || n = 3622 || n = 3625 || n = 3626)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3607 || n = 3612)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 3609)
	; 7 bytes
	pha
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3614)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3616)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3617)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3619 || n = 3624 || n = 3629)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3621 || n = 3628)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3623 || n = 3630)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3627)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3630 && n < 3636))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 3635 && n < 3751))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
; 3751 to 3828
.if((n > 3750 && n < 3810))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3810 || n = 3811)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3811 && n < 3819))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3819)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3820)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3821)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3822)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3823)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3824)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3825)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3826)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3827)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3828)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3830)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3831)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3832)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3833)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3834)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3835)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3836)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3837)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3838)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3839)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3840)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3841)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3842)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3843)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3844)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3845)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3846)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3847)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 3848 to 3867
.if(n = 3848)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3849)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3850)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3851)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3852)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3853)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3854)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3855)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3856)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3857)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3858)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3859)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 3860)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3861)
	; 10 bytes
	lda #15
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 3862 || n = 3864 || n = 3866 || n = 3867)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3863)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 3865)
	; 7 bytes
	pha
	lda #15
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 3868 to 3906
.if(n = 3868)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 3869 || n = 3871 || n = 3874 || n = 3876 || n = 3878 || n = 3881 || n = 3882)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3872)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3873)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3875 || n = 3880 || n = 3885)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3877 || n = 3884)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3879 || n = 3886)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3883)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 3886 && n < 3892))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 3891 && n < 3907))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 3907 to 4062
.if((n > 3906 && n < 4063))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
.if((n >= 4063) && (n < 4082))
; 4063 to 4081
.if((n > 4062 && n < 4066))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4066 || n = 4067)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4067 && n < 4075))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4075)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4076)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4077)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4078)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4079)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4080)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4081)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 4082 to 4101
.if(n = 4082)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4083)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4084)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4085)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4086)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4087)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4088)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4089)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4090)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4091)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4092)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4093)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4094)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4095)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4096)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4097)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4098)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4099)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4100)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4101)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 4102) && (n < 4121))
; 4102 to 4120
.if(n = 4102)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4103)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4104)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4105)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4106)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4107)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4108)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4109)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4110)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4111)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4112)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4113)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4114)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4115)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 4116)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 4117)
	; 10 bytes
	lda #16
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 4118 || n = 4120)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 4119)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.else
; 4121 to 4140
.if(n = 4121)
	; 7 bytes
	pha
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4122 || n = 4123 || n = 4125 || n = 4127 || n = 4130 || n = 4132 || n = 4134 || n = 4137 || n = 4138)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4124)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 4126)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4128)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4129)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4131 || n = 4136)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4133 || n = 4140)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4135)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4139)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4142)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 4142 && n < 4148))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 4147 && n < 4219))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if((n > 4218 && n < 4297))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if((n > 4296 && n < 4322))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4322 || n = 4323)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4323 && n < 4331))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4331)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4332)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4333)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4334)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4335)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 4336) && (n < 4356))
; 4336 to 4355
.if(n = 4336)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4337)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4338)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4339)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4340)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4341)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4342)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4343)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4344)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4345)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4346)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4347)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4348)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4349)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4350)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4351)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4352)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4353)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4354)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4355)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 4356 to 4375
.if(n = 4356)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4357)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4358)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4359)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4360)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4361)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4362)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4363)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4364)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4365)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4366)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4367)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4368)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4369)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4370)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4371)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 4372)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 4373)
	; 10 bytes
	lda #17
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 4374)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4375)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4378 || n = 4379 || n = 4381 || n = 4383 || n = 4386 || n = 4388 || n = 4390 || n = 4393 || n = 4394)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 7 bytes
	pha
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4380)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 4382)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4384)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4385)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4387 || n = 4392 || n = 4397)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4389 || n = 4396)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4391 || n = 4398)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4395)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4398 && n < 4404))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 4403 && n < 4532))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if((n > 4531 && n < 4571))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 4571) && (n < 4590))
; 4571 to 4589
.if((n > 4570 && n < 4578))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4578 || n = 4579)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4579 && n < 4587))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4587)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4588)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4589)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 4590 to 4609
.if(n = 4590)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4591)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4592)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4593)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4594)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4595)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4596)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4597)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4598)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4599)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4600)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4601)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4602)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4603)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4604)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4605)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4606)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4607)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4608)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4609)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if(n = 4610)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4611)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4612)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4613)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4614)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4615)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4616)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4617)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4618)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4619)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4620)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4621)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4622)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4623)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4624)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4625)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4626)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4627)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 4628)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.endif
.else
; 4629 to 4648
.if(n = 4629)
	; 10 bytes
	lda #18
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 4630 || n = 4632 || n = 4634 || n = 4635 || n = 4637 || n = 4639 || n = 4642 || n = 4644 || n = 4646)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4631 || n = 4636)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 4633)
	; 7 bytes
	pha
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4638)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4640)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4641)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4643 || n = 4648)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4645)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4647)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4650)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4651)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4652)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4653)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4654)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 4654 && n < 4660))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 4659 && n < 4688))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if((n > 4687 && n < 4834))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4834 || n = 4835)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4835 && n < 4843))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4843)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if(n = 4844)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4845)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4846)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4847)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4848)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4849)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4850)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4851)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4852)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4853)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4854)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4855)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4856)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4857)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4858)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4859)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4860)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4861)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4862)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 4863 to 4882
.if(n = 4863)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4864)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4865)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4866)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4867)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4868)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4869)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4870)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4871)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4872)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4873)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4874)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4875)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4876)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4877)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4878)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4879)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4880)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4881)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4882)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 4884)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 4885)
	; 10 bytes
	lda #19
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 4886 || n = 4888 || n = 4890 || n = 4891 || n = 4893 || n = 4895 || n = 4898 || n = 4900 || n = 4902 || n = 4905 || n = 4906)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4887 || n = 4892)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 4889)
	; 7 bytes
	pha
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4894)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4896)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4897)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4899 || n = 4904 || n = 4909)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4901 || n = 4908)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4903 || n = 4910)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4907)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4910 && n < 4916))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 4915 && n < 4922))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 4922 to 5000
.if((n > 4921 && n < 5001))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
.if((n >= 5001) && (n < 5313))
.if((n >= 5001) && (n < 5157))
.if((n >= 5001) && (n < 5079))
; 5001 to 5078
.if((n > 5000 && n < 5079))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n > 5078 && n < 5090))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5090 || n = 5091)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5091 && n < 5099))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5099)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5100)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5101)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5102)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5103)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5104)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5105)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5106)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5107)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5108)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5109)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5110)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5111)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5112)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5113)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5114)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5115)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5116)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5117)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5119)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5120)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5121)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5122)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5123)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5124)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5125)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5126)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5127)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5128)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5129)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5130)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5131)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5132)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5133)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5134)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5135)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5136)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 5137 to 5156
.if(n = 5137)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5138)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5139)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 5140)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5141)
	; 10 bytes
	lda #20
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 5142 || n = 5144 || n = 5146 || n = 5147 || n = 5149 || n = 5151 || n = 5154 || n = 5156)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5143 || n = 5148)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 5145)
	; 7 bytes
	pha
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5150)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5152)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5153)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5155)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157 || n = 5164)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5158 || n = 5161 || n = 5162)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5159 || n = 5166)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5160 || n = 5165)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5163)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5166 && n < 5172))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 5171 && n < 5313))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if((n > 5312 && n < 5346))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5346 || n = 5347)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5347 && n < 5352))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if((n > 5351 && n < 5355))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5355)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5356)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5357)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5358)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5359)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5360)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5361)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5362)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5363)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5364)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5365)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5366)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5367)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5368)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5369)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5370)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5372)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5373)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5374)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5375)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5376)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5377)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5378)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5379)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5380)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5381)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5382)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5383)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5384)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5385)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5386)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5387)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5388)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5389)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5390)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 5391 to 5468
.if(n = 5391)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5392)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5393)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5394)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5395)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 5396)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5397)
	; 10 bytes
	lda #21
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 5398 || n = 5400 || n = 5402 || n = 5403 || n = 5405 || n = 5407 || n = 5410 || n = 5412 || n = 5414 || n = 5417 || n = 5418)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5399 || n = 5404)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 5401)
	; 7 bytes
	pha
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5406)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5408)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5409)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5411 || n = 5416 || n = 5421)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5413 || n = 5420)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5415 || n = 5422)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5419)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5422 && n < 5428))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 5427 && n < 5469))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5602))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5602 || n = 5603)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5603 && n < 5611))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5611)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5612)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5613)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5614)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5615)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5616)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5617)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5618)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5619)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5620)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5621)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5622)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5623)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5624)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5625)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
.if((n >= 5626) && (n < 5665))
.if((n >= 5626) && (n < 5645))
; 5626 to 5644
.if(n = 5626)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5627)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5628)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5629)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5630)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5631)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5632)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5633)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5634)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5635)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5636)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5637)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5638)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5639)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5640)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5641)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5642)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5643)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5644)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5646)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5647)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5648)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5649)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5650)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5651)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 5652)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5653)
	; 10 bytes
	lda #22
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 5654 || n = 5656 || n = 5658 || n = 5659 || n = 5661 || n = 5663)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5655 || n = 5660)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 5657)
	; 7 bytes
	pha
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5662)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5664)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 5665 to 5703
.if(n = 5665)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5666 || n = 5668 || n = 5670 || n = 5673 || n = 5674)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667 || n = 5672 || n = 5677)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5669 || n = 5676)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5671 || n = 5678)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5675)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5678 && n < 5684))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 5683 && n < 5704))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 5704 to 5781
.if((n > 5703 && n < 5782))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5858))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5858 || n = 5859)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if((n > 5859 && n < 5867))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5867)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5868)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5869)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5870)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5871)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5872)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5873)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5874)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5875)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5876)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5877)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5878)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 5879 to 5898
.if(n = 5879)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5880)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5881)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5882)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5883)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5884)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5885)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5886)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5887)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5888)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5889)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5890)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5891)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5892)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5893)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5894)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5895)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5896)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5897)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5898)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5900)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5901)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5902)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5903)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5904)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5905)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5906)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5907)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 5908)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5909)
	; 10 bytes
	lda #23
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 5910 || n = 5912 || n = 5914 || n = 5915 || n = 5917 || n = 5919 || n = 5922 || n = 5924 || n = 5926 || n = 5929 || n = 5930)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5911 || n = 5916)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 5913)
	; 7 bytes
	pha
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5918)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5920)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5921)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5923 || n = 5928 || n = 5933)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5925 || n = 5932)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5927 || n = 5934)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5931)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 5934 && n < 5938))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938 || n = 5939)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 5939 && n < 6094))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6114))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6114 || n = 6115)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6115 && n < 6123))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6123)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6124)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6125)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6126)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6127)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6128)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6129)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6130)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6131)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6132)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6134)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6135)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6136)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6137)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6138)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6139)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6140)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6141)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6142)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6143)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6144)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6145)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6146)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6147)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6148)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6149)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6150)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6151)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 6152 to 6171
.if(n = 6152)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6153)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6154)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6155)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6156)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6157)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6158)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6159)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6160)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6161)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6162)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6163)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 6164)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6165)
	; 10 bytes
	lda #24
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 6166 || n = 6168 || n = 6170 || n = 6171)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6167)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 6169)
	; 7 bytes
	pha
	lda #24
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 6173 || n = 6175 || n = 6178 || n = 6180 || n = 6182 || n = 6185 || n = 6186)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6176)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6177)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6179 || n = 6184 || n = 6189)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6181 || n = 6188)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6183 || n = 6190)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6187)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6190 && n < 6196))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 6195 && n < 6251))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
.if((n >= 6251) && (n < 6407))
.if((n >= 6251) && (n < 6329))
; 6251 to 6328
.if((n > 6250 && n < 6329))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if((n > 6328 && n < 6368))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6368) && (n < 6387))
; 6368 to 6386
.if(n = 6368 || n = 6369)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6370 || n = 6371)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6371 && n < 6379))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6379)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6380)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6381)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6382)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6383)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6384)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6385)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6386)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6388)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6389)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6390)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6391)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6392)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6393)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6394)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6395)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6396)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6397)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6398)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6399)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6400)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6401)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6402)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6403)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6404)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6405)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6406)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
.if(n = 6407)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6408)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6409)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6410)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6411)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6412)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6413)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6414)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6415)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6416)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6417)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6418)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6419)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 6420)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6421)
	; 10 bytes
	lda #25
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 6422 || n = 6424 || n = 6426 || n = 6427 || n = 6429 || n = 6431 || n = 6434 || n = 6436 || n = 6438 || n = 6441 || n = 6442)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6423 || n = 6428)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 6425)
	; 7 bytes
	pha
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6430)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6432)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6433)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6435 || n = 6440 || n = 6445)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6437 || n = 6444)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6439)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6443)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 6446 to 6484
.if(n = 6446)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 6446 && n < 6452))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 6451 && n < 6485))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6563))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6626))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6626 || n = 6627)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6627 && n < 6635))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6635)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6636)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6637)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6638)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6639)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6640)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6642)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6643)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6644)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6645)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6646)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6647)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6648)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6649)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6650)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6651)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6652)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6653)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6654)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6655)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6656)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6657)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6658)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6659)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6661)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6662)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6663)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6664)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6665)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6666)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6667)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6668)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6669)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6670)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6671)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6672)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6673)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6674)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6675)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 6676)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6677)
	; 10 bytes
	lda #26
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 6678)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6679)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6682 || n = 6683 || n = 6685 || n = 6687 || n = 6690 || n = 6692 || n = 6694 || n = 6697 || n = 6698)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6681)
	; 7 bytes
	pha
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6684)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 6686)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6688)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6689)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6691 || n = 6696 || n = 6701)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6693 || n = 6700)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6695 || n = 6702)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6699)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6702 && n < 6708))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 6707 && n < 6719))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 6719 to 6875
.if((n > 6718 && n < 6876))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
.if((n >= 6876) && (n < 6895))
; 6876 to 6894
.if((n > 6875 && n < 6882))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6882 || n = 6883)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6883 && n < 6891))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6891)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6892)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6893)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6894)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 6895 to 6914
.if(n = 6895)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6896)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6897)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6898)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6899)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6900)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6901)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6902)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6903)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6904)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6905)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6906)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6907)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6908)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6909)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6910)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6911)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6912)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6913)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6914)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 6915) && (n < 6934))
; 6915 to 6933
.if(n = 6915)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6916)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6917)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6918)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6919)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6920)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6921)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6922)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6923)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6924)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6925)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6926)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6927)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6928)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6929)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6930)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6931)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 6932)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6933)
	; 10 bytes
	lda #27
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.endif
.else
; 6934 to 6953
.if(n = 6934 || n = 6936 || n = 6938 || n = 6939 || n = 6941 || n = 6943 || n = 6946 || n = 6948 || n = 6950 || n = 6953)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6935 || n = 6940)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 6937)
	; 7 bytes
	pha
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6942)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6944)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6945)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6947 || n = 6952)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6949)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6951)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6955)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6956)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6957)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6958)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 6958 && n < 6964))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 6963 && n < 7032))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7110))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if((n > 7109 && n < 7138))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7138 || n = 7139)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7139 && n < 7147))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7147)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7148)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7150)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7151)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7152)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7153)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7154)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7155)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7156)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7157)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7158)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7159)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7160)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7161)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7162)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7163)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7164)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7165)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7166)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7167)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7169)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7170)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7171)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7172)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7173)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7174)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7175)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7176)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7177)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7178)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7179)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7180)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7181)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7182)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7183)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7184)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7185)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7186)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7187)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if(n = 7188)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7189)
	; 10 bytes
	lda #28
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 7190 || n = 7192 || n = 7194 || n = 7195 || n = 7197 || n = 7199 || n = 7202 || n = 7204 || n = 7206 || n = 7209 || n = 7210)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7191 || n = 7196)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 7193)
	; 7 bytes
	pha
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7198)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7200)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7201)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7203 || n = 7208 || n = 7213)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7205 || n = 7212)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7207 || n = 7214)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7211)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7214 && n < 7220))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 7219 && n < 7344))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7394))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7394 || n = 7395)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7395 && n < 7403))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7403)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7404)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7405)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7406)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7407)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7408)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7409)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7410)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7411)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7412)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7413)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7414)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7415)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7416)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7417)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7418)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7419)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7420)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7421)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7423)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7424)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7425)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7426)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7427)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7428)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7429)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7430)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7431)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7432)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7433)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7434)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7435)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7436)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7437)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7438)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7439)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7440)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7442)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7443)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 7444)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7445)
	; 10 bytes
	lda #29
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 7446 || n = 7448 || n = 7450 || n = 7451 || n = 7453 || n = 7455 || n = 7458 || n = 7460)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7447 || n = 7452)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 7449)
	; 7 bytes
	pha
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7454)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7456)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7457)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7459)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461 || n = 7468)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7462 || n = 7465 || n = 7466)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7463 || n = 7470)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7464 || n = 7469)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7467)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7470 && n < 7476))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 7475 && n < 7501))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
; 7501 to 7656
.if((n > 7500 && n < 7650))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7650 || n = 7651)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7651 && n < 7657))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657 || n = 7658)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7659)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7660)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7661)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7662)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7663)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7664)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7665)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7666)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7667)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7668)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7669)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7670)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7671)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7672)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7673)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7674)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7675)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7677)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7678)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7679)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7680)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7681)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7682)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7683)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7684)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7685)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7686)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7687)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7688)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7689)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7690)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7691)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7692)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7693)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7694)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7695)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 7696 to 7734
.if(n = 7696)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7697)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7698)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7699)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 7700)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7701)
	; 10 bytes
	lda #30
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 7702 || n = 7704 || n = 7706 || n = 7707 || n = 7709 || n = 7711 || n = 7714 || n = 7716 || n = 7718 || n = 7721 || n = 7722)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7703 || n = 7708)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 7705)
	; 7 bytes
	pha
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7710)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7712)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7713)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7715 || n = 7720 || n = 7725)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7717 || n = 7724)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7719 || n = 7726)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7723)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7726 && n < 7732))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 7731 && n < 7735))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 7735 to 7812
.if((n > 7734 && n < 7813))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7891))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7906))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7906 || n = 7907)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7907 && n < 7915))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7915)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7916)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7917)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7918)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7919)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7920)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7921)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7922)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7923)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7924)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7925)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7926)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7927)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7928)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7929)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7931)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7932)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7933)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7934)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7935)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7936)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7937)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7938)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7939)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7940)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7941)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7942)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7943)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7944)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7945)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7946)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7947)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7948)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 7949 to 7968
.if(n = 7949)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7950)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7951)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7952)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7953)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7954)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7955)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 7956)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7957)
	; 10 bytes
	lda #31
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 7958 || n = 7960 || n = 7962 || n = 7963 || n = 7965 || n = 7967)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7959 || n = 7964)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 7961)
	; 7 bytes
	pha
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7966)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7968)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.endif
.else
; 7969 to 8125
.if(n = 7969)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7970 || n = 7972 || n = 7974 || n = 7977 || n = 7978)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971 || n = 7976 || n = 7981)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7973 || n = 7980)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7975 || n = 7982)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7979)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7982 && n < 7988))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 7987 && n < 8126))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
.if((n >= 8126) && (n < 8165))
; 8126 to 8164
.if((n > 8125 && n < 8162))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8162 || n = 8163)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8164)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 8165) && (n < 8184))
; 8165 to 8183
.if((n > 8164 && n < 8171))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8171)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8172)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8173)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8174)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8175)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8176)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8177)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8178)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8179)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8180)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8181)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8182)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8183)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8185)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8186)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8187)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8188)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8189)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8190)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8191)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8192)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8193)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8194)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8195)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8196)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8197)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8198)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8199)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8200)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8201)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8202)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8203)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 8204 to 8281
.if(n = 8204)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8205)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8206)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8207)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8208)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8209)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8210)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8211)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 8212)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8213)
	; 10 bytes
	lda #32
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 8214 || n = 8216 || n = 8218 || n = 8219 || n = 8221 || n = 8223 || n = 8226 || n = 8228 || n = 8230 || n = 8233 || n = 8234)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8215 || n = 8220)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 8217)
	; 7 bytes
	pha
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8222)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 8224)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8225)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8227 || n = 8232 || n = 8237)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8229 || n = 8236)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8231 || n = 8238)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8235)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 8238 && n < 8244))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 8243 && n < 8282))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8418))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8418 || n = 8419)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 8419 && n < 8427))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8427)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8428)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8429)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8430)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8431)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8432)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8433)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8434)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8435)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8436)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8437)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
.if((n >= 8438) && (n < 8457))
; 8438 to 8456
.if(n = 8438)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8439)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8440)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8441)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8442)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8443)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8444)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8445)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8446)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8447)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8448)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8449)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8450)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8451)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8452)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8453)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8454)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8455)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8456)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 8457 to 8476
.if(n = 8457)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8458)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8459)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8460)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8461)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8462)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8463)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8464)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8465)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8466)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8467)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 8468)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8469)
	; 10 bytes
	lda #33
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 8470 || n = 8472 || n = 8474 || n = 8475)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8471 || n = 8476)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 8473)
	; 7 bytes
	pha
	lda #33
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8479 || n = 8482 || n = 8484 || n = 8486 || n = 8489 || n = 8490)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 8480)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8481)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8483 || n = 8488 || n = 8493)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8485 || n = 8492)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8487 || n = 8494)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8491)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 8494 && n < 8500))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 8499 && n < 8516))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 8516 to 8593
.if((n > 8515 && n < 8594))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if((n > 8593 && n < 8672))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 8672) && (n < 8711))
.if((n >= 8672) && (n < 8691))
; 8672 to 8690
.if(n = 8672 || n = 8673)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8674 || n = 8675)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 8675 && n < 8683))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8683)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8684)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8685)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8686)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8687)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8688)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8689)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8690)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8692)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8693)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8694)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8695)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8696)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8697)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8698)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8699)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8700)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8701)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8702)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8703)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8704)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8705)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8706)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8707)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8708)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8709)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8710)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 8711 to 8750
.if(n = 8711)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8712)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8713)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8714)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8715)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8716)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8717)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8718)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8719)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8720)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8721)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8722)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8723)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 8724)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8725)
	; 10 bytes
	lda #34
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 8726 || n = 8728 || n = 8730 || n = 8731 || n = 8733 || n = 8735 || n = 8738 || n = 8740 || n = 8742 || n = 8745 || n = 8746)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8727 || n = 8732)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 8729)
	; 7 bytes
	pha
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8734)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 8736)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8737)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8739 || n = 8744 || n = 8749)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8741 || n = 8748)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8743 || n = 8750)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8747)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
.if((n >= 8751) && (n < 9063))
.if((n >= 8751) && (n < 8907))
; 8751 to 8906
.if((n > 8750 && n < 8756))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 8755 && n < 8907))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if((n > 8906 && n < 8930))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8930 || n = 8931)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 8931 && n < 8939))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8939)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8940)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8941)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8942)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8943)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8944)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8945)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8947)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8948)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8949)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8950)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8951)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8952)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8953)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8954)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8955)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8956)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8957)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8958)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8959)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8960)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8961)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8962)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8963)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8964)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8966)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8967)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8968)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8969)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8970)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8971)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8972)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8973)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8974)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8975)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8976)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8977)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8978)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8979)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 8980)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8981)
	; 10 bytes
	lda #35
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 8982 || n = 8984)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 8983)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.endif
.endif
.else
; 8985 to 9062
.if(n = 8985)
	; 7 bytes
	pha
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8986 || n = 8987 || n = 8989 || n = 8991 || n = 8994 || n = 8996 || n = 8998 || n = 9001 || n = 9002)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 8990)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 8992)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8993)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8995 || n = 9000 || n = 9005)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8997 || n = 9004)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8999 || n = 9006)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9003)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9006 && n < 9012))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 9011 && n < 9063))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
; 9063 to 9140
.if((n > 9062 && n < 9141))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if((n > 9140 && n < 9180))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 9180) && (n < 9199))
; 9180 to 9198
.if((n > 9179 && n < 9186))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9186 || n = 9187)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9187 && n < 9195))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9195)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9196)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9197)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9198)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 9199 to 9218
.if(n = 9199)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9200)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9201)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9202)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9203)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9204)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9205)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9206)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9207)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9208)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9209)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9210)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9211)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9212)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9213)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9214)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9215)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9216)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9217)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9218)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
.if((n >= 9219) && (n < 9238))
; 9219 to 9237
.if(n = 9219)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9220)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9221)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9222)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9223)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9224)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9225)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9226)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9227)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9228)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9229)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9230)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9231)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9232)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9233)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9234)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9235)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 9236)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 9237)
	; 10 bytes
	lda #36
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.endif
.else
; 9238 to 9257
.if(n = 9238 || n = 9240 || n = 9242 || n = 9243 || n = 9245 || n = 9247 || n = 9250 || n = 9252 || n = 9254 || n = 9257)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9239 || n = 9244)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 9241)
	; 7 bytes
	pha
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9246)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 9248)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9249)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9251 || n = 9256)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9253)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9255)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9259)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9260)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9261)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9262)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 9262 && n < 9268))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 9267 && n < 9297))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 9297 to 9375
.if((n > 9296 && n < 9376))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 9376) && (n < 9688))
.if((n >= 9376) && (n < 9532))
.if((n >= 9376) && (n < 9454))
; 9376 to 9453
.if((n > 9375 && n < 9442))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9442 || n = 9443)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9443 && n < 9451))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9451)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9452)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9453)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9455)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9456)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9457)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9458)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9459)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9460)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9461)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9462)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9463)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9464)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9465)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9466)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9467)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9468)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9469)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9470)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9471)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9472)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9474)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9475)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9476)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9477)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9478)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9479)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9480)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9481)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9482)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9483)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9484)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9485)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9486)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9487)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9488)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9489)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9490)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9491)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 9492)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.else
; 9493 to 9531
.if(n = 9493)
	; 10 bytes
	lda #37
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 9494 || n = 9496 || n = 9498 || n = 9499 || n = 9501 || n = 9503 || n = 9506 || n = 9508 || n = 9510 || n = 9513 || n = 9514)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495 || n = 9500)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 9497)
	; 7 bytes
	pha
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9502)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 9504)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9505)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9507 || n = 9512 || n = 9517)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9509 || n = 9516)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9511 || n = 9518)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9515)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9518 && n < 9524))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 9523 && n < 9532))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 9532 to 9687
.if((n > 9531 && n < 9688))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if((n > 9687 && n < 9698))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9698 || n = 9699)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9699 && n < 9707))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9707)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9708)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9709)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9710)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9711)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9712)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9713)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9714)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9715)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9716)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9717)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9718)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9719)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9720)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9721)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9722)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9723)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9724)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9725)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9726)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9728)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9729)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9730)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9731)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9732)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9733)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9734)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9735)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9736)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9737)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9738)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9739)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9740)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9741)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9742)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9743)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9744)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9745)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9747)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 9748)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 9749)
	; 10 bytes
	lda #38
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 9750 || n = 9752 || n = 9754 || n = 9755 || n = 9757 || n = 9759 || n = 9762 || n = 9764)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9751 || n = 9756)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 9753)
	; 7 bytes
	pha
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9758)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 9760)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9761)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9763)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9765)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766 || n = 9769 || n = 9770)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9767 || n = 9774)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9768 || n = 9773)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9771)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9772)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif((n > 9774 && n < 9780))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 9779 && n < 9844))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
; 9844 to 9921
.if((n > 9843 && n < 9922))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if((n > 9921 && n < 9954))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9954 || n = 9955)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9955 && n < 9961))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 9961) && (n < 9981))
; 9961 to 9980
.if(n = 9961 || n = 9962)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9963)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9964)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9965)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9966)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9967)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9968)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9969)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9970)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9971)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9972)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9973)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9974)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9975)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9976)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9977)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9978)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9979)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9980)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9982)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9983)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9984)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9985)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9986)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9987)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9988)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9989)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9990)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9991)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9992)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9993)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9994)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9995)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9996)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9997)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9998)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9999)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10000)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
.if((n >= 10001) && (n < 11251))
.if((n >= 10001) && (n < 10626))
.if((n >= 10001) && (n < 10313))
.if((n >= 10001) && (n < 10157))
; 10001 to 10156
.if(n = 10001)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10002)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10003)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 10004)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10005)
	; 10 bytes
	lda #39
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 10006 || n = 10008 || n = 10010 || n = 10011 || n = 10013 || n = 10015 || n = 10018 || n = 10020 || n = 10022 || n = 10025 || n = 10026)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10007 || n = 10012)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 10009)
	; 7 bytes
	pha
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10014)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 10016)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10017)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10019 || n = 10024 || n = 10029)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10021 || n = 10028)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10023 || n = 10030)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10027)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10030 && n < 10036))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 10035 && n < 10157))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10210))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10210 || n = 10211)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10211 && n < 10219))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10219)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10220)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10221)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10222)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10223)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10224)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10225)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10226)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10227)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10228)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10229)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10230)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10231)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10232)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10233)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10234)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if(n = 10235)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10236)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10237)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10238)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10239)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10240)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10241)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10242)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10243)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10244)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10245)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10246)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10247)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10248)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10249)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10250)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10251)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10252)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10253)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 10254 to 10273
.if(n = 10254)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10255)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10256)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10257)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10258)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10259)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 10260)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10261)
	; 10 bytes
	lda #40
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 10262 || n = 10264 || n = 10266 || n = 10267 || n = 10269 || n = 10271)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10263 || n = 10268)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 10265)
	; 7 bytes
	pha
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10270)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 10272)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10273)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10276 || n = 10278 || n = 10281 || n = 10282)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275 || n = 10280 || n = 10285)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10277 || n = 10284)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10279 || n = 10286)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10283)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10286 && n < 10292))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 10291 && n < 10313))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
; 10313 to 10468
.if((n > 10312 && n < 10466))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10466 || n = 10467)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10468)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
.if((n >= 10469) && (n < 10488))
; 10469 to 10487
.if((n > 10468 && n < 10475))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10475)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10476)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10477)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10478)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10479)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10480)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10481)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10482)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10483)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10484)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10485)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10486)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10487)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 10488 to 10507
.if(n = 10488)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10489)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10490)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10491)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10492)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10493)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10494)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10495)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10496)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10497)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10498)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10499)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10500)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10501)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10502)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10503)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10504)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10505)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10506)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10507)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 10508 to 10546
.if(n = 10508)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10509)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10510)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10511)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10512)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10513)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10514)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10515)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 10516)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10517)
	; 10 bytes
	lda #41
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 10518 || n = 10520 || n = 10522 || n = 10523 || n = 10525 || n = 10527 || n = 10530 || n = 10532 || n = 10534 || n = 10537 || n = 10538)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10519 || n = 10524)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 10521)
	; 7 bytes
	pha
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10526)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 10528)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10529)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10531 || n = 10536 || n = 10541)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10533 || n = 10540)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10535 || n = 10542)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10539)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10542 && n < 10547))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.else
; 10547 to 10625
.if(n = 10547)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 10547 && n < 10626))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 10626) && (n < 10938))
.if((n >= 10626) && (n < 10782))
.if((n >= 10626) && (n < 10704))
; 10626 to 10703
.if((n > 10625 && n < 10704))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if((n > 10703 && n < 10722))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10722 || n = 10723)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10723 && n < 10731))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10731)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10732)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10733)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10734)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10735)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10736)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10737)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10738)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10739)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10740)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10741)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10742)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10744)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10745)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10746)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10747)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10748)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10749)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10750)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10751)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10752)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10753)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10754)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10755)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10756)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10757)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10758)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10759)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10760)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10761)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10763)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10764)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10765)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10766)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10767)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10768)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10769)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10770)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10771)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 10772)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10773)
	; 10 bytes
	lda #42
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 10774 || n = 10776 || n = 10778 || n = 10779 || n = 10781)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10775 || n = 10780)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 10777)
	; 7 bytes
	pha
	lda #42
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 10783 || n = 10786 || n = 10788 || n = 10790 || n = 10793 || n = 10794)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10784)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10785)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10787 || n = 10792 || n = 10797)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10789 || n = 10796)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10791 || n = 10798)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10795)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10798 && n < 10804))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 10803 && n < 10938))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if((n > 10937 && n < 10977))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 10977) && (n < 10996))
; 10977 to 10995
.if(n = 10977)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10978 || n = 10979)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 10979 && n < 10987))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10987)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10988)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10989)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10990)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10991)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10992)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10993)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10994)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10995)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10997)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10998)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10999)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11000)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11001)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11002)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11003)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11004)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11005)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11006)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11007)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11008)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11009)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11010)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11011)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11012)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11013)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11014)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11015)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
; 11016 to 11054
.if(n = 11016)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11017)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11018)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11019)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11020)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11021)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11022)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11023)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11024)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11025)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11026)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11027)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 11028)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11029)
	; 10 bytes
	lda #43
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 11030 || n = 11032 || n = 11034 || n = 11035 || n = 11037 || n = 11039 || n = 11042 || n = 11044 || n = 11046 || n = 11049 || n = 11050)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11031 || n = 11036)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 11033)
	; 7 bytes
	pha
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11038)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 11040)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11041)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11043 || n = 11048 || n = 11053)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11045 || n = 11052)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11047 || n = 11054)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11051)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 11055 to 11093
.if((n > 11054 && n < 11060))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 11059 && n < 11094))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 11094 to 11250
.if((n > 11093 && n < 11234))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11234 || n = 11235)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 11235 && n < 11243))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11243)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11244)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11245)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11246)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11247)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11248)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11249)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11250)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 11251) && (n < 11876))
.if((n >= 11251) && (n < 11563))
.if((n >= 11251) && (n < 11407))
.if((n >= 11251) && (n < 11329))
.if((n >= 11251) && (n < 11290))
.if((n >= 11251) && (n < 11270))
; 11251 to 11269
.if(n = 11251)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11252)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11253)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11254)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11255)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11256)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11257)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11258)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11259)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11260)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11261)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11262)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11263)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11264)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11265)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11266)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11267)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11268)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11269)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 11270 to 11289
.if(n = 11270)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11271)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11272)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11273)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11274)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11275)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11276)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11277)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11278)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11279)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11280)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11281)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11282)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11283)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 11284)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11285)
	; 10 bytes
	lda #44
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 11286 || n = 11288)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 11287)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 11289)
	; 7 bytes
	pha
	lda #44
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290 || n = 11291 || n = 11293 || n = 11295 || n = 11298 || n = 11300 || n = 11302 || n = 11305 || n = 11306)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 11294)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 11296)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11297)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11299 || n = 11304 || n = 11309)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11301 || n = 11308)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11303 || n = 11310)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11307)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 11310 && n < 11316))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 11315 && n < 11329))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 11329 to 11406
.if((n > 11328 && n < 11407))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if((n > 11406 && n < 11485))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 11485) && (n < 11524))
.if((n >= 11485) && (n < 11504))
; 11485 to 11503
.if((n > 11484 && n < 11490))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11490 || n = 11491)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 11491 && n < 11499))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11499)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11500)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11501)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11502)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11503)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 11504 to 11523
.if(n = 11504)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11505)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11506)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11507)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11508)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11509)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11510)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11511)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11512)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11513)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11514)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11515)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11516)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11517)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11518)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11519)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11520)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11521)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11522)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11523)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11525)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11526)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11527)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11528)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11529)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11530)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11531)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11532)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11533)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11534)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11535)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11536)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11537)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11538)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11539)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 11540)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11541)
	; 10 bytes
	lda #45
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 11542)
	; 8 bytes
	delay_n (n-2)
	nop
.endif
.else
; 11543 to 11562
.if(n = 11543 || n = 11548)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 11544 || n = 11546 || n = 11547 || n = 11549 || n = 11551 || n = 11554 || n = 11556 || n = 11558 || n = 11561 || n = 11562)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11545)
	; 7 bytes
	pha
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11550)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 11552)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11553)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11555 || n = 11560)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11557)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11559)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11564)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11565)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11566)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 11566 && n < 11572))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 11571 && n < 11719))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if((n > 11718 && n < 11746))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11746 || n = 11747)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 11747 && n < 11755))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11755)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11756)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11757)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11759)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11760)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11761)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11762)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11763)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11764)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11765)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11766)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11767)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11768)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11769)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11770)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11771)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11772)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11773)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11774)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11775)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11776)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11778)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11779)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11780)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11781)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11782)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11783)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11784)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11785)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11786)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11787)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11788)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11789)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11790)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11791)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11792)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11793)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11794)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11795)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 11796)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.endif
.else
; 11797 to 11875
.if(n = 11797)
	; 10 bytes
	lda #46
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 11798 || n = 11800 || n = 11802 || n = 11803 || n = 11805 || n = 11807 || n = 11810 || n = 11812 || n = 11814 || n = 11817 || n = 11818)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11799 || n = 11804)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 11801)
	; 7 bytes
	pha
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11806)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 11808)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11809)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11811 || n = 11816 || n = 11821)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11813 || n = 11820)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11815 || n = 11822)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11819)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 11822 && n < 11828))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 11827 && n < 11876))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
; 11876 to 12031
.if((n > 11875 && n < 12002))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12002 || n = 12003)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12003 && n < 12011))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12011)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12012)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12013)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12014)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12015)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12016)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12017)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12018)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12019)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12020)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12021)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12022)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12023)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12024)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12025)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12026)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12027)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12028)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12029)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12030)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12031)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if(n = 12032)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12033)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12034)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12035)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12036)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12037)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12038)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12039)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12040)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12041)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12042)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12043)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12044)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12045)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12046)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12047)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12048)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12049)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12050)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 12051 to 12070
.if(n = 12051)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 12052)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12053)
	; 10 bytes
	lda #47
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 12054 || n = 12056 || n = 12058 || n = 12059 || n = 12061 || n = 12063 || n = 12066 || n = 12068 || n = 12070)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12055 || n = 12060)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 12057)
	; 7 bytes
	pha
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12062)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 12064)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12065)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12067)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12069)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 12071 to 12109
.if(n = 12071 || n = 12078)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12072 || n = 12077)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12073 || n = 12074)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12075)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12076)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif((n > 12078 && n < 12084))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 12083 && n < 12110))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 12110 to 12187
.if((n > 12109 && n < 12188))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12258))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12258 || n = 12259)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12259 && n < 12266))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 12266) && (n < 12305))
.if((n >= 12266) && (n < 12285))
; 12266 to 12284
.if(n = 12266)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12267)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12268)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12269)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12270)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12271)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12272)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12273)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12274)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12275)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12276)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12277)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12278)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12279)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12280)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12281)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12282)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12283)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12284)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12286)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12287)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12288)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12289)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12290)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12291)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12292)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12293)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12294)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12295)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12296)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12297)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12298)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12299)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12300)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12301)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12302)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12303)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12304)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 12305 to 12343
.if(n = 12305)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12306)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12307)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 12308)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12309)
	; 10 bytes
	lda #48
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 12310 || n = 12312 || n = 12314 || n = 12315 || n = 12317 || n = 12319 || n = 12322 || n = 12324 || n = 12326 || n = 12329 || n = 12330)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12311 || n = 12316)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 12313)
	; 7 bytes
	pha
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12318)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 12320)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12321)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12323 || n = 12328 || n = 12333)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12325 || n = 12332)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12327 || n = 12334)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12331)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12334 && n < 12340))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 12339 && n < 12344))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 12344 to 12500
.if((n > 12343 && n < 12501))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
.if((n >= 12501) && (n < 13126))
.if((n >= 12501) && (n < 12813))
.if((n >= 12501) && (n < 12657))
.if((n >= 12501) && (n < 12579))
.if((n >= 12501) && (n < 12540))
; 12501 to 12539
.if((n > 12500 && n < 12514))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12514 || n = 12515)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12515 && n < 12523))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12523)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12524)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12525)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12526)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12527)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12528)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12529)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12530)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12531)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12532)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12533)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12534)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12535)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12536)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12537)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12538)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12539)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 12540) && (n < 12559))
; 12540 to 12558
.if(n = 12540)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12541)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12542)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12543)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12544)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12545)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12546)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12547)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12548)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12549)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12550)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12551)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12552)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12553)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12554)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12555)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12556)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12557)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12558)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12560)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12561)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12562)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12563)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 12564)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12565)
	; 10 bytes
	lda #49
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 12566 || n = 12568 || n = 12570 || n = 12571 || n = 12573 || n = 12575 || n = 12578)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12567 || n = 12572)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 12569)
	; 7 bytes
	pha
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12574)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 12576)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12577)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579 || n = 12584 || n = 12589)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12580 || n = 12582 || n = 12585 || n = 12586)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12581 || n = 12588)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12583 || n = 12590)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12587)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12590 && n < 12596))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 12595 && n < 12657))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
; 12657 to 12734
.if((n > 12656 && n < 12735))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if((n > 12734 && n < 12770))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12770 || n = 12771)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12772 || n = 12773)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 12774) && (n < 12793))
; 12774 to 12792
.if((n > 12773 && n < 12779))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12779)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12780)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12781)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12782)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12783)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12784)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12785)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12786)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12787)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12788)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12789)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12790)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12791)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12792)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12794)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12795)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12796)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12797)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12798)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12799)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12800)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12801)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12802)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12803)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12804)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12805)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12806)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12807)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12808)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12809)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12810)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12811)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12812)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12813) && (n < 12969))
; 12813 to 12968
.if(n = 12813)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12814)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12815)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12816)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12817)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12818)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12819)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 12820)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12821)
	; 10 bytes
	lda #50
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 12822 || n = 12824 || n = 12826 || n = 12827 || n = 12829 || n = 12831 || n = 12834 || n = 12836 || n = 12838 || n = 12841 || n = 12842)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12823 || n = 12828)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 12825)
	; 7 bytes
	pha
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12830)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 12832)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12833)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12835 || n = 12840 || n = 12845)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12837 || n = 12844)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12839 || n = 12846)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12843)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12846 && n < 12852))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 12851 && n < 12969))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 13026))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13026 || n = 13027)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 13027 && n < 13035))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13035)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13036)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13037)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13038)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13039)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13040)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13041)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13042)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13043)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13044)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13045)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13046)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13048)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13049)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13050)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13051)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13052)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13053)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13054)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13055)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13056)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13057)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13058)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13059)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13060)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13061)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13062)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13063)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13064)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13065)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13067)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13068)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13069)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13070)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13071)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13072)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13073)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13074)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13075)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 13076)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13077)
	; 10 bytes
	lda #51
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 13078 || n = 13080 || n = 13082 || n = 13083 || n = 13085)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13079 || n = 13084)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 13081)
	; 7 bytes
	pha
	lda #51
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 13087 || n = 13090 || n = 13092 || n = 13094 || n = 13097 || n = 13098)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13088)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13089)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13091 || n = 13096 || n = 13101)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13093 || n = 13100)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13095 || n = 13102)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13099)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 13102 && n < 13108))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 13107 && n < 13126))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13126) && (n < 13438))
.if((n >= 13126) && (n < 13282))
; 13126 to 13281
.if((n > 13125 && n < 13282))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
.if((n >= 13282) && (n < 13301))
; 13282 to 13300
.if(n = 13282 || n = 13283)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 13283 && n < 13291))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13291)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13292)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13293)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13294)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13295)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13296)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13297)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13298)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13299)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13300)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13302)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13303)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13304)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13305)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13306)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13307)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13308)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13309)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13310)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13311)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13312)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13313)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13314)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13315)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13316)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13317)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13318)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13319)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13320)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 13321 to 13359
.if(n = 13321)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13322)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13323)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13324)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13325)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13326)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13327)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13328)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13329)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13330)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13331)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 13332)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13333)
	; 10 bytes
	lda #52
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 13334 || n = 13336 || n = 13338 || n = 13339 || n = 13341 || n = 13343 || n = 13346 || n = 13348 || n = 13350 || n = 13353 || n = 13354)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13335 || n = 13340)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 13337)
	; 7 bytes
	pha
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13342)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 13344)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13345)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13347 || n = 13352 || n = 13357)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13349 || n = 13356)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13351 || n = 13358)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13355)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13359)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.else
; 13360 to 13437
.if((n > 13359 && n < 13364))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 13363 && n < 13438))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
; 13438 to 13515
.if((n > 13437 && n < 13516))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if((n > 13515 && n < 13538))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13538 || n = 13539)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 13539 && n < 13547))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13547)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13548)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13549)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13550)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13551)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13552)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13553)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13554)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 13555) && (n < 13574))
; 13555 to 13573
.if(n = 13555)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13556)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13557)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13558)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13559)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13560)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13561)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13562)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13563)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13564)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13565)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13566)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13567)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13568)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13569)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13570)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13571)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13572)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13573)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 13574 to 13593
.if(n = 13574)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13575)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13576)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13577)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13578)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13579)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13580)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13581)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13582)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13583)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13584)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13585)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13586)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13587)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 13588)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13589)
	; 10 bytes
	lda #53
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 13590 || n = 13592)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 13591)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 13593)
	; 7 bytes
	pha
	lda #53
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
; 13594 to 13750
.if(n = 13594 || n = 13595 || n = 13597 || n = 13599 || n = 13602 || n = 13604 || n = 13606 || n = 13609 || n = 13610)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 13598)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 13600)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13601)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13603 || n = 13608 || n = 13613)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13605 || n = 13612)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13607 || n = 13614)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13611)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 13614 && n < 13620))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 13619 && n < 13751))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 13751) && (n < 14376))
.if((n >= 13751) && (n < 14063))
.if((n >= 13751) && (n < 13907))
.if((n >= 13751) && (n < 13829))
.if((n >= 13751) && (n < 13790))
; 13751 to 13789
.if((n > 13750 && n < 13790))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 13790) && (n < 13809))
; 13790 to 13808
.if((n > 13789 && n < 13794))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13794 || n = 13795)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 13795 && n < 13803))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13803)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13804)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13805)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13806)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13807)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13808)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13810)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13811)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13812)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13813)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13814)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13815)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13816)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13817)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13818)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13819)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13820)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13821)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13822)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13823)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13824)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13825)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13826)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13827)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13828)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
.if((n >= 13829) && (n < 13848))
; 13829 to 13847
.if(n = 13829)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13830)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13831)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13832)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13833)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13834)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13835)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13836)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13837)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13838)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13839)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13840)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13841)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13842)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13843)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 13844)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13845)
	; 10 bytes
	lda #54
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 13846)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13847)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.else
; 13848 to 13867
.if(n = 13848 || n = 13850 || n = 13851 || n = 13853 || n = 13855 || n = 13858 || n = 13860 || n = 13862 || n = 13865 || n = 13866)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13849)
	; 7 bytes
	pha
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13852)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 13854)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 13856)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13857)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13859 || n = 13864)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13861)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13863)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13867)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13869)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13870)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 13870 && n < 13876))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 13875 && n < 13907))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 13907 to 14062
.if((n > 13906 && n < 14050))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14050 || n = 14051)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14051 && n < 14059))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14059)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14060)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14061)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14062)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
.if((n >= 14063) && (n < 14082))
; 14063 to 14081
.if(n = 14063)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14064)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14065)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14066)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14067)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14068)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14069)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14070)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14071)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14072)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14073)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14074)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14075)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14076)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14077)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14078)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14079)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14080)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14081)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 14082 to 14101
.if(n = 14082)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14083)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14084)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14085)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14086)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14087)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14088)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14089)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14090)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14091)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14092)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14093)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14094)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14095)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14096)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14097)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14098)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14099)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 14100)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14101)
	; 10 bytes
	lda #55
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.endif
.endif
.else
; 14102 to 14140
.if(n = 14102 || n = 14104 || n = 14106 || n = 14107 || n = 14109 || n = 14111 || n = 14114 || n = 14116 || n = 14118 || n = 14121 || n = 14122)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14103 || n = 14108)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 14105)
	; 7 bytes
	pha
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14110)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 14112)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14113)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14115 || n = 14120 || n = 14125)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14117 || n = 14124)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14119 || n = 14126)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14123)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14126 && n < 14132))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 14131 && n < 14141))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 14141 to 14218
.if((n > 14140 && n < 14219))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if((n > 14218 && n < 14297))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 14297) && (n < 14336))
; 14297 to 14335
.if((n > 14296 && n < 14306))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14306 || n = 14307)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14307 && n < 14315))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14315)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14316)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14317)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14318)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14319)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14320)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14321)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14322)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14323)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14324)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14325)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14326)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14327)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14328)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14329)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14330)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14331)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14332)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14333)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14334)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14335)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 14336) && (n < 14356))
; 14336 to 14355
.if(n = 14336)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14337)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14338)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14339)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14340)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14341)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14342)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14343)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14344)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14345)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14346)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14347)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14348)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14349)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14350)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14351)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14352)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14353)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14354)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14355)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.endif
.else
; 14356 to 14375
.if(n = 14356)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14357)
	; 10 bytes
	lda #56
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 14358 || n = 14360 || n = 14362 || n = 14363 || n = 14365 || n = 14367 || n = 14370 || n = 14372 || n = 14374)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14359 || n = 14364)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 14361)
	; 7 bytes
	pha
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14366)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 14368)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14369)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14371)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14373)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14375)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14376) && (n < 14688))
.if((n >= 14376) && (n < 14532))
; 14376 to 14531
.if(n = 14376 || n = 14381)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14377 || n = 14378)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14380)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14382)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 14382 && n < 14388))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 14387 && n < 14532))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if((n > 14531 && n < 14562))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14562 || n = 14563)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14563 && n < 14571))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 14571) && (n < 14590))
; 14571 to 14589
.if(n = 14571)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14572)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14573)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14574)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14575)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14576)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14577)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14578)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14579)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14580)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14581)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14582)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14583)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14584)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14585)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14586)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14587)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14588)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14589)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14591)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14592)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14593)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14594)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14595)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14596)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14597)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14598)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14599)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14600)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14601)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14602)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14603)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14604)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14605)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14606)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14607)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14608)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14609)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 14610 to 14687
.if(n = 14610)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14611)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 14612)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14613)
	; 10 bytes
	lda #57
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 14614 || n = 14616 || n = 14618 || n = 14619 || n = 14621 || n = 14623 || n = 14626 || n = 14628 || n = 14630 || n = 14633 || n = 14634)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14615 || n = 14620)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 14617)
	; 7 bytes
	pha
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14622)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 14624)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14625)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14627 || n = 14632 || n = 14637)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14629 || n = 14636)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14631 || n = 14638)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14635)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14638 && n < 14644))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 14643 && n < 14688))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
; 14688 to 14843
.if((n > 14687 && n < 14818))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14818 || n = 14819)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14819 && n < 14827))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14827)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14828)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14829)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14830)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14831)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14832)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14833)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14834)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14835)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14836)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14837)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14838)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14839)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14840)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14841)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14842)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14843)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14845)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14846)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14847)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14848)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14849)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14850)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14851)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14852)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14853)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14854)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14855)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14856)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14857)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14858)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14859)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14860)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14861)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14862)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14864)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14865)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14866)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14867)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 14868)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14869)
	; 10 bytes
	lda #58
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 14870 || n = 14872 || n = 14874 || n = 14875 || n = 14877 || n = 14879 || n = 14882)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14871 || n = 14876)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 14873)
	; 7 bytes
	pha
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14878)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 14880)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14881)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883 || n = 14888 || n = 14893)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14884 || n = 14886 || n = 14889 || n = 14890)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14885 || n = 14892)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14887 || n = 14894)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14891)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14894 && n < 14900))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 14899 && n < 14922))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 14922 to 15000
.if((n > 14921 && n < 15001))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
.if((n >= 15001) && (n < 15626))
.if((n >= 15001) && (n < 15313))
.if((n >= 15001) && (n < 15157))
.if((n >= 15001) && (n < 15079))
; 15001 to 15078
.if((n > 15000 && n < 15074))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15074 || n = 15075)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15075 && n < 15079))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 15079) && (n < 15118))
.if((n >= 15079) && (n < 15098))
; 15079 to 15097
.if((n > 15078 && n < 15083))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15083)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15084)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15085)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15086)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15087)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15088)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15089)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15090)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15091)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15092)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15093)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15094)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15095)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15096)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15097)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 15098 to 15117
.if(n = 15098)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15099)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15100)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15101)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15102)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15103)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15104)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15105)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15106)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15107)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15108)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15109)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15110)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15111)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15112)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15113)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15114)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15115)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15116)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15117)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 15118 to 15156
.if(n = 15118)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15119)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15120)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15121)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15122)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15123)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 15124)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15125)
	; 10 bytes
	lda #59
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 15126 || n = 15128 || n = 15130 || n = 15131 || n = 15133 || n = 15135 || n = 15138 || n = 15140 || n = 15142 || n = 15145 || n = 15146)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15127 || n = 15132)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 15129)
	; 7 bytes
	pha
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15134)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 15136)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15137)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15139 || n = 15144 || n = 15149)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15141 || n = 15148)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15143 || n = 15150)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15147)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15150 && n < 15156))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15156)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 15157 to 15312
.if((n > 15156 && n < 15313))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
; 15313 to 15351
.if((n > 15312 && n < 15330))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15330 || n = 15331)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15331 && n < 15339))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15339)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15340)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15341)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15342)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15343)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15344)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15345)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15346)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15347)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15348)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15349)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15350)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15351)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15353)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15354)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15355)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15356)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15357)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15358)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15359)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15360)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15361)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15362)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15363)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15364)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15365)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15366)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15367)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15368)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15369)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15370)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 15371 to 15390
.if(n = 15371)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15372)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15373)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15374)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15375)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15376)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15377)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15378)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15379)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 15380)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15381)
	; 10 bytes
	lda #60
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 15382 || n = 15384 || n = 15386 || n = 15387 || n = 15389)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15383 || n = 15388)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 15385)
	; 7 bytes
	pha
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15390)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15394 || n = 15396 || n = 15398 || n = 15401 || n = 15402)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15392)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15393)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15395 || n = 15400 || n = 15405)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15397 || n = 15404)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15399 || n = 15406)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15403)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15406 && n < 15412))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 15411 && n < 15469))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
; 15469 to 15546
.if((n > 15468 && n < 15547))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if((n > 15546 && n < 15586))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15586) && (n < 15606))
; 15586 to 15605
.if(n = 15586 || n = 15587)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15587 && n < 15595))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15595)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15596)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15597)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15598)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15599)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15600)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15601)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15602)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15603)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15604)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15605)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 15606 to 15625
.if(n = 15606)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15607)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15608)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15609)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15610)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15611)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15612)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15613)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15614)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15615)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15616)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15617)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15618)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15619)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15620)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15621)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15622)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15623)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15624)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15625)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15626) && (n < 15938))
.if((n >= 15626) && (n < 15782))
; 15626 to 15781
.if(n = 15626)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15627)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15628)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15629)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15630)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15631)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15632)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15633)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15634)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15635)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 15636)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15637)
	; 10 bytes
	lda #61
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 15638 || n = 15640 || n = 15642 || n = 15643 || n = 15645 || n = 15647 || n = 15650 || n = 15652 || n = 15654 || n = 15657 || n = 15658)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15639 || n = 15644)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 15641)
	; 7 bytes
	pha
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15646)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 15648)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15649)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15651 || n = 15656 || n = 15661)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15653 || n = 15660)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15655 || n = 15662)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15659)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15662 && n < 15668))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 15667 && n < 15782))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15842))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15842 || n = 15843)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15843 && n < 15851))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15851)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15852)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15853)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15854)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15855)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15856)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15857)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15858)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15859)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15860) && (n < 15899))
.if((n >= 15860) && (n < 15879))
; 15860 to 15878
.if(n = 15860)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15861)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15862)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15863)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15864)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15865)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15866)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15867)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15868)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15869)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15870)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15871)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15872)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15873)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15874)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15875)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15876)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15877)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15878)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 15879 to 15898
.if(n = 15879)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15880)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15881)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15882)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15883)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15884)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15885)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15886)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15887)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15888)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15889)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15890)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15891)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 15892)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15893)
	; 10 bytes
	lda #62
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 15894 || n = 15896 || n = 15898)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15895)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 15897)
	; 7 bytes
	pha
	lda #62
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15901 || n = 15903 || n = 15906 || n = 15908 || n = 15910 || n = 15913 || n = 15914)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 15902)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 15904)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15905)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15907 || n = 15912 || n = 15917)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15909 || n = 15916)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15911 || n = 15918)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15915)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 15918 && n < 15924))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 15923 && n < 15938))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
; 15938 to 16093
.if((n > 15937 && n < 16094))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
.if((n >= 16094) && (n < 16113))
; 16094 to 16112
.if((n > 16093 && n < 16098))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16098 || n = 16099)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16099 && n < 16107))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16107)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16108)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16109)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16110)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16111)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16112)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16114)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16115)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16116)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16117)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16118)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16119)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16120)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16121)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16122)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16123)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16124)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16125)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16126)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16127)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16128)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16129)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16130)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16131)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16132)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 16133) && (n < 16152))
; 16133 to 16151
.if(n = 16133)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16134)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16135)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16136)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16137)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16138)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16139)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16140)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16141)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16142)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16143)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16144)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16145)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16146)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16147)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 16148)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16149)
	; 10 bytes
	lda #63
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 16150)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16151)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.else
; 16152 to 16171
.if(n = 16152 || n = 16154 || n = 16155 || n = 16157 || n = 16159 || n = 16162 || n = 16164 || n = 16166 || n = 16169 || n = 16170)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16153)
	; 7 bytes
	pha
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16156)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 16158)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 16160)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16161)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16163 || n = 16168)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16165)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16167)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16171)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16173)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16174)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 16174 && n < 16180))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 16179 && n < 16251))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 16251) && (n < 16876))
.if((n >= 16251) && (n < 16563))
.if((n >= 16251) && (n < 16407))
.if((n >= 16251) && (n < 16329))
; 16251 to 16328
.if((n > 16250 && n < 16329))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if((n > 16328 && n < 16354))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16354 || n = 16355)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16355 && n < 16363))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16363)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16364)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16365)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16366)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16367)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16369)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16370)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16371)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16372)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16373)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16374)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16375)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16376)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16377)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16378)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16379)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16380)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16381)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16382)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16383)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16384)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16385)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16386)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 16387 to 16406
.if(n = 16387)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16388)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16389)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16390)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16391)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16392)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16393)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16394)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16395)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16396)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16397)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16398)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16399)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16400)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16401)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16402)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16403)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 16404)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16405)
	; 10 bytes
	lda #64
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 16406)
	; 8 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
; 16407 to 16562
.if(n = 16407 || n = 16412)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 16408 || n = 16410 || n = 16411 || n = 16413 || n = 16415 || n = 16418 || n = 16420 || n = 16422 || n = 16425 || n = 16426)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16409)
	; 7 bytes
	pha
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16414)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 16416)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16417)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16419 || n = 16424 || n = 16429)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16421 || n = 16428)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16423 || n = 16430)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16427)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16430 && n < 16436))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 16435 && n < 16563))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
; 16563 to 16640
.if((n > 16562 && n < 16610))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16610 || n = 16611)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16611 && n < 16619))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16619)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16620)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16621)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16622)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16623)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16624)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16625)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16626)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16627)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16628)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16629)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16630)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16631)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16632)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16633)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16634)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16635)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16636)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16637)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16638)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16639)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16640)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 16641) && (n < 16680))
.if((n >= 16641) && (n < 16660))
; 16641 to 16659
.if(n = 16641)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16642)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16643)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16644)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16645)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16646)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16647)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16648)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16649)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16650)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16651)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16652)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16653)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16654)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16655)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16656)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16657)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16658)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16659)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.endif
.else
; 16660 to 16679
.if(n = 16660)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16661)
	; 10 bytes
	lda #65
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 16662 || n = 16664 || n = 16666 || n = 16667 || n = 16669 || n = 16671 || n = 16674 || n = 16676 || n = 16678)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16663 || n = 16668)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 16665)
	; 7 bytes
	pha
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16670)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 16672)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16673)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16675)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16677)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16679)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680 || n = 16685)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16681 || n = 16682)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16683)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16684)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16686)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 16686 && n < 16692))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 16691 && n < 16719))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 16719 to 16875
.if((n > 16718 && n < 16866))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16866 || n = 16867)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16867 && n < 16875))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16875)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 16876) && (n < 17188))
.if((n >= 16876) && (n < 17032))
.if((n >= 16876) && (n < 16954))
.if((n >= 16876) && (n < 16915))
.if((n >= 16876) && (n < 16895))
; 16876 to 16894
.if(n = 16876)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16877)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16878)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16879)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16880)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16881)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16882)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16883)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16884)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16885)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16886)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16887)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16888)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16889)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16890)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16891)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16892)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16893)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16894)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 16895 to 16914
.if(n = 16895)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16896)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16897)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16898)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16899)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16900)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16901)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16902)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16903)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16904)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16905)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16906)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16907)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16908)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16909)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16910)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16911)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16912)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16913)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16914)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 16915 to 16953
.if(n = 16915)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 16916)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16917)
	; 10 bytes
	lda #66
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 16918 || n = 16920 || n = 16922 || n = 16923 || n = 16925 || n = 16927 || n = 16930 || n = 16932 || n = 16934 || n = 16937 || n = 16938)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16919 || n = 16924)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 16921)
	; 7 bytes
	pha
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16926)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 16928)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16929)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16931 || n = 16936 || n = 16941)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16933 || n = 16940)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16935 || n = 16942)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16939)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16942 && n < 16948))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 16947 && n < 16954))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 16954 to 17031
.if((n > 16953 && n < 17032))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
; 17032 to 17109
.if((n > 17031 && n < 17110))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if((n > 17109 && n < 17122))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17122 || n = 17123)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17123 && n < 17131))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17131)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17132)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17133)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17134)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17135)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17136)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17137)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17138)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17139)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17140)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17141)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17142)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17143)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17144)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17145)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17146)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17147)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17148)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17150)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17151)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17152)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17153)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17154)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17155)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17156)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17157)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17158)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17159)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17160)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17161)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17162)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17163)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17164)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17165)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17166)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17167)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 17168 to 17187
.if(n = 17168)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17169)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17170)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17171)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 17172)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17173)
	; 10 bytes
	lda #67
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 17174 || n = 17176 || n = 17178 || n = 17179 || n = 17181 || n = 17183 || n = 17186)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17175 || n = 17180)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 17177)
	; 7 bytes
	pha
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17182)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 17184)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17185)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17187)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17190 || n = 17193 || n = 17194)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17189 || n = 17196)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17191 || n = 17198)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17192 || n = 17197)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17195)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17198 && n < 17204))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 17203 && n < 17344))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if((n > 17343 && n < 17378))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17378 || n = 17379)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17379 && n < 17383))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 17383) && (n < 17402))
; 17383 to 17401
.if((n > 17382 && n < 17387))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17387)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17388)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17389)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17390)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17391)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17392)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17393)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17394)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17395)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17396)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17397)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17398)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17399)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17400)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17401)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 17402 to 17421
.if(n = 17402)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17403)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17404)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17405)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17406)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17407)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17408)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17409)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17410)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17411)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17412)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17413)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17414)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17415)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17416)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17417)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17418)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17419)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17420)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17421)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 17422 to 17500
.if(n = 17422)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17423)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17424)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17425)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17426)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17427)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 17428)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17429)
	; 10 bytes
	lda #68
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 17430 || n = 17432 || n = 17434 || n = 17435 || n = 17437 || n = 17439 || n = 17442 || n = 17444 || n = 17446 || n = 17449 || n = 17450)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17431 || n = 17436)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 17433)
	; 7 bytes
	pha
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17438)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 17440)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17441)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17443 || n = 17448 || n = 17453)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17445 || n = 17452)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17447 || n = 17454)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17451)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17454 && n < 17460))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 17459 && n < 17501))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
.if((n >= 17501) && (n < 18126))
.if((n >= 17501) && (n < 17813))
.if((n >= 17501) && (n < 17657))
; 17501 to 17656
.if((n > 17500 && n < 17634))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17634 || n = 17635)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17635 && n < 17643))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17643)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17644)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17645)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17646)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17647)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17648)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17649)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17650)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17651)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17652)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17653)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17654)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17655)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17656)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
.if((n >= 17657) && (n < 17676))
; 17657 to 17675
.if(n = 17657)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17658)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17659)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17660)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17661)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17662)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17663)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17664)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17665)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17666)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17667)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17668)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17669)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17670)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17671)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17672)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17673)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17674)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17675)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 17676 to 17695
.if(n = 17676)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17677)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17678)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17679)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17680)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17681)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17682)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17683)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 17684)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17685)
	; 10 bytes
	lda #69
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 17686 || n = 17688 || n = 17690 || n = 17691 || n = 17693 || n = 17695)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17687 || n = 17692)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 17689)
	; 7 bytes
	pha
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17694)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.endif
.else
; 17696 to 17734
.if(n = 17696)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17697)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17698 || n = 17700 || n = 17702 || n = 17705 || n = 17706)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699 || n = 17704 || n = 17709)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17701 || n = 17708)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17703 || n = 17710)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17707)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17710 && n < 17716))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 17715 && n < 17735))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 17735 to 17812
.if((n > 17734 && n < 17813))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17890))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17890)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 17891) && (n < 17930))
.if((n >= 17891) && (n < 17910))
; 17891 to 17909
.if(n = 17891)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 17891 && n < 17899))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17899)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17900)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17901)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17902)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17903)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17904)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17905)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17906)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17907)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17908)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17909)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 17910 to 17929
.if(n = 17910)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17911)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17912)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17913)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17914)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17915)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17916)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17917)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17918)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17919)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17920)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17921)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17922)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17923)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17924)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17925)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17926)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17927)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17928)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17929)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 17930 to 17968
.if(n = 17930)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17931)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17932)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17933)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17934)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17935)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17936)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17937)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17938)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17939)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 17940)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17941)
	; 10 bytes
	lda #70
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 17942 || n = 17944 || n = 17946 || n = 17947 || n = 17949 || n = 17951 || n = 17954 || n = 17956 || n = 17958 || n = 17961 || n = 17962)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17943 || n = 17948)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 17945)
	; 7 bytes
	pha
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17950)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 17952)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17953)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17955 || n = 17960 || n = 17965)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17957 || n = 17964)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17959 || n = 17966)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17963)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17967 || n = 17968)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.endif
.endif
.endif
.else
; 17969 to 18125
.if((n > 17968 && n < 17972))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 17971 && n < 18126))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 18126) && (n < 18438))
.if((n >= 18126) && (n < 18282))
.if((n >= 18126) && (n < 18204))
.if((n >= 18126) && (n < 18165))
; 18126 to 18164
.if((n > 18125 && n < 18146))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18146 || n = 18147)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18147 && n < 18155))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18155)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18156)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18157)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18158)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18159)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18160)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18161)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18162)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18163)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18164)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18166)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18167)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18168)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18169)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18170)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18171)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18172)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18173)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18174)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18175)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18176)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18177)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18178)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18179)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18180)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18181)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18182)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18183)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 18184 to 18203
.if(n = 18184)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18185)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18186)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18187)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18188)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18189)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18190)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18191)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18192)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18193)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18194)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18195)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 18196)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 18197)
	; 10 bytes
	lda #71
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 18198 || n = 18200 || n = 18202 || n = 18203)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18199)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 18201)
	; 7 bytes
	pha
	lda #71
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 18205 || n = 18207 || n = 18210 || n = 18212 || n = 18214 || n = 18217 || n = 18218)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18206)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 18208)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18209)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18211 || n = 18216 || n = 18221)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18213 || n = 18220)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18215 || n = 18222)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18219)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18222 && n < 18228))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 18227 && n < 18282))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
; 18282 to 18359
.if((n > 18281 && n < 18360))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18399))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18399) && (n < 18418))
; 18399 to 18417
.if((n > 18398 && n < 18402))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18402 || n = 18403)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18403 && n < 18411))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18411)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18412)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18413)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18414)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18415)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18416)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18417)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18419)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18420)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18421)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18422)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18423)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18424)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18425)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18426)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18427)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18428)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18429)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18430)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18431)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18432)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18433)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18434)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18435)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18436)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18437)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18438) && (n < 18594))
.if((n >= 18438) && (n < 18516))
.if((n >= 18438) && (n < 18477))
.if((n >= 18438) && (n < 18457))
; 18438 to 18456
.if(n = 18438)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18439)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18440)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18441)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18442)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18443)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18444)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18445)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18446)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18447)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18448)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18449)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18450)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18451)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 18452)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 18453)
	; 10 bytes
	lda #72
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 18454 || n = 18456)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 18455)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 7 bytes
	pha
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18458 || n = 18459 || n = 18461 || n = 18463 || n = 18466 || n = 18468 || n = 18470 || n = 18473 || n = 18474)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18460)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 18462)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 18464)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18465)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18467 || n = 18472)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18469 || n = 18476)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18471)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18475)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18478)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 18478 && n < 18484))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 18483 && n < 18516))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 18516 to 18593
.if((n > 18515 && n < 18594))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18658))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18658 || n = 18659)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18659 && n < 18667))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18667)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18668)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18669)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18670)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18671)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18673)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18674)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18675)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18676)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18677)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18678)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18679)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18680)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18681)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18682)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18683)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18684)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18685)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18686)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18687)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18688)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18689)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18690)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18692)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18693)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18694)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18695)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18696)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18697)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18698)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18699)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18700)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18701)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18702)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18703)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18704)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18705)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18706)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18707)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 18708)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 18709)
	; 10 bytes
	lda #73
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 18710)
	; 8 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711 || n = 18716)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 18712 || n = 18714 || n = 18715 || n = 18717 || n = 18719 || n = 18722 || n = 18724 || n = 18726 || n = 18729 || n = 18730)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18713)
	; 7 bytes
	pha
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18718)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 18720)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18721)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18723 || n = 18728 || n = 18733)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18725 || n = 18732)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18727 || n = 18734)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18731)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18734 && n < 18740))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 18739 && n < 18751))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
; 18751 to 18906
.if((n > 18750 && n < 18907))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
.if((n >= 18907) && (n < 18926))
; 18907 to 18925
.if((n > 18906 && n < 18914))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18914 || n = 18915)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18915 && n < 18923))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18923)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18924)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18925)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 18926 to 18945
.if(n = 18926)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18927)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18928)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18929)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18930)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18931)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18932)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18933)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18934)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18935)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18936)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18937)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18938)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18939)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18940)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18941)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18942)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18943)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18944)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18945)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 18946) && (n < 18965))
; 18946 to 18964
.if(n = 18946)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18947)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18948)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18949)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18950)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18951)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18952)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18953)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18954)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18955)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18956)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18957)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18958)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18959)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18960)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18961)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18962)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18963)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 18964)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.endif
.else
; 18965 to 18984
.if(n = 18965)
	; 10 bytes
	lda #74
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 18966 || n = 18968 || n = 18970 || n = 18971 || n = 18973 || n = 18975 || n = 18978 || n = 18980 || n = 18982)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18967 || n = 18972)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 18969)
	; 7 bytes
	pha
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18974)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 18976)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18977)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18979 || n = 18984)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18981)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18983)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985 || n = 18986)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18987)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18988)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18989)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18990)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 18990 && n < 18996))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 18995 && n < 19063))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
; 19063 to 19140
.if((n > 19062 && n < 19141))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if((n > 19140 && n < 19170))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19170 || n = 19171)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19171 && n < 19179))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19179)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19181)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19182)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19183)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19184)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19185)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19186)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19187)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19188)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19189)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19190)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19191)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19192)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19193)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19194)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19195)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19196)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19197)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19198)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 19199 to 19218
.if(n = 19199)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19200)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19201)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19202)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19203)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19204)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19205)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19206)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19207)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19208)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19209)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19210)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19211)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19212)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19213)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19214)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19215)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19216)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19217)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19218)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.endif
.else
; 19219 to 19375
.if(n = 19219)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 19220)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19221)
	; 10 bytes
	lda #75
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 19222 || n = 19224 || n = 19226 || n = 19227 || n = 19229 || n = 19231 || n = 19234 || n = 19236 || n = 19238 || n = 19241 || n = 19242)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19223 || n = 19228)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 19225)
	; 7 bytes
	pha
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19230)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 19232)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19233)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19235 || n = 19240 || n = 19245)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19237 || n = 19244)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19239 || n = 19246)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19243)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19246 && n < 19252))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 19251 && n < 19376))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
; 19376 to 19453
.if((n > 19375 && n < 19426))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19426 || n = 19427)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19427 && n < 19435))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19435)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19436)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19437)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19438)
	; 15 bytes
	pha
	jsr @nearby_rts_14cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19439)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19440)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19441)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19442)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19443)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19444)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19445)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19446)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19447)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19448)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19449)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19450)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19451)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19452)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19453)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19454) && (n < 19493))
.if((n >= 19454) && (n < 19473))
; 19454 to 19472
.if(n = 19454)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19455)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19456)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19457)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19458)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19459)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19460)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19461)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19462)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19463)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19464)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19465)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19466)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19467)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19468)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19469)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19470)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19471)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19472)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 19473 to 19492
.if(n = 19473)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19474)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19475)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 19476)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19477)
	; 10 bytes
	lda #76
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 19478 || n = 19480 || n = 19482 || n = 19483 || n = 19485 || n = 19487 || n = 19490 || n = 19492)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19479 || n = 19484)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 19481)
	; 7 bytes
	pha
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19486)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 19488)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19489)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19491)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493 || n = 19500)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19494 || n = 19497 || n = 19498)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19495 || n = 19502)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19496 || n = 19501)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19499)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19502 && n < 19508))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif((n > 19507 && n < 19532))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.endif
.else
; 19532 to 19687
.if((n > 19531 && n < 19682))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19682 || n = 19683)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19683 && n < 19688))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
.if((n >= 19688) && (n < 19707))
; 19688 to 19706
.if((n > 19687 && n < 19691))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19691)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19692)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19693)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19694)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19695)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19696)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19697)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19698)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19699)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19700)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19701)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19702)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19703)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19704)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19705)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19706)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19708)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19709)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19710)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19711)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19712)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19713)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19714)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19715)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19716)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19717)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19718)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19719)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19720)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19721)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19722)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19723)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19724)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19725)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19726)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 19727 to 19765
.if(n = 19727)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19728)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19729)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19730)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19731)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 19732)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19733)
	; 10 bytes
	lda #77
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 19734 || n = 19736 || n = 19738 || n = 19739 || n = 19741 || n = 19743 || n = 19746 || n = 19748 || n = 19750 || n = 19753 || n = 19754)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19735 || n = 19740)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 19737)
	; 7 bytes
	pha
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19742)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 19744)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19745)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19747 || n = 19752 || n = 19757)
	; 10..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19749 || n = 19756)
	; 10..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19751 || n = 19758)
	; 10..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19755)
	; 12 bytes
	pha
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19758 && n < 19764))
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19764 || n = 19765)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
; 19766 to 19843
.if((n > 19765 && n < 19844))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
; 19844 to 19921
.if((n > 19843 && n < 19922))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if((n > 19921 && n < 19938))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19938 || n = 19939)
	; 13 bytes
	pha
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19939 && n < 19947))
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19947)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19948)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19949)
	; 15 bytes
	pha
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19950)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19951)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #172
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19952)
	; 15 bytes
	pha
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19953)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19954)
	; 16 bytes
	pha
	nop
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19955)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #164
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19956)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #165
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19957)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #166
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19958)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #167
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19959)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #168
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19960)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #169
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if(n = 19961)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #170
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19962)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19963)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #172
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19964)
	; 17 bytes
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19965)
	; 17 bytes
	pha
	lda #1
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19966)
	; 17 bytes
	pha
	lda #2
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19967)
	; 17 bytes
	pha
	lda #3
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19968)
	; 17 bytes
	pha
	lda #4
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19969)
	; 17 bytes
	pha
	lda #5
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19970)
	; 17 bytes
	pha
	lda #6
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19971)
	; 17 bytes
	pha
	lda #7
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19972)
	; 17 bytes
	pha
	lda #8
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19973)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19974)
	; 17 bytes
	pha
	lda #84
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19975)
	; 17 bytes
	pha
	lda #85
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19976)
	; 17 bytes
	pha
	lda #86
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19977)
	; 17 bytes
	pha
	lda #87
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19978)
	; 17 bytes
	pha
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19979)
	; 17 bytes
	pha
	lda #89
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19980)
	; 17 bytes
	pha
	lda #90
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 19981 to 20000
.if(n = 19981)
	; 17 bytes
	pha
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19982)
	; 17 bytes
	pha
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19983)
	; 17 bytes
	pha
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19984)
	; 17 bytes
	pha
	lda #94
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19985)
	; 17 bytes
	pha
	lda #95
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19986)
	; 17 bytes
	pha
	lda #96
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19987)
	; 9 bytes
	lda #(n-19)/256
	jsr delay_256a_11_clocks_
	lda @zptemp
	sta @zptemp
.elseif(n = 19988)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19989)
	; 10 bytes
	lda #78
	jsr delay_256a_11_clocks_
	lda @zptemp
	nop
	sta @zptemp
.elseif(n = 19990 || n = 19992 || n = 19994 || n = 19995 || n = 19997 || n = 19999)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19991 || n = 19996)
	; 9 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 19993)
	; 7 bytes
	pha
	lda #78
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19998)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 20000)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
