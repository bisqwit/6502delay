; delay_n, delays n cycles with small code
; Will clobber X, S, Z+N, C, and V
; Will preserve A, Y, D, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 302
; -10 bytes or less when n <= 305
; -11 bytes or less when n <= 315
; -12 bytes or less when n <= 489
; -13 bytes or less when n <= 492
; -14 bytes or less when n <= 502
; -15 bytes or less when n <= 505
; -16 bytes or less when n <= 515
; -17 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=9, avg=12.0, max=14
; - When 501 <= n <= 750: min=7, avg=12.2, max=17
; - When 751 <= n <= 1000: min=7, avg=12.3, max=17
; - When 1001 <= n <= 2000: min=7, avg=12.4, max=17
; - When 2001 <= n <= 3000: min=7, avg=12.4, max=17
; - When 3001 <= n <= 5000: min=7, avg=12.4, max=17
; - When 5001 <= n <= 7500: min=7, avg=12.4, max=17
; - When 7501 <= n <= 10000: min=7, avg=12.4, max=17
; - When 10001 <= n <= 20000: min=7, avg=12.4, max=17
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
.elseif(n = 4 || n = 5 || n = 8 || n = 17 || n = 28 || n = 31 || n = 290 || n = 291 || n = 306)
	; 2..11 bytes
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
.elseif(n = 18 || n = 30 || n = 303 || n = 304)
	; 4..10 bytes
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
.elseif((n > 33 && n < 39) || (n > 39 && n < 290))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 292)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n > 292 && n < 297))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 296 && n < 303) || (n > 306 && n < 310))
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 305)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
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
.elseif(n = 490 || n = 501)
	; 13..14 bytes
	delay_n (n-485)
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 491 || n = 502 || n = 504)
	; 13..15 bytes
	delay_n (n-486)
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 492 || n = 505)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 492 && n < 497) || n = 507)
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 497)
	; 14 bytes
	delay_n (n-481)
	lda #168
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 498)
	; 14 bytes
	delay_n (n-482)
	lda #169
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 499)
	; 14 bytes
	delay_n (n-483)
	lda #170
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 500)
	; 14 bytes
	delay_n (n-484)
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
.elseif(n = 506)
	; 16 bytes
	delay_n (n-501)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.endif
.endif
.else
.if((n >= 508) && (n < 527))
; 508 to 526
.if(n = 508 || n = 509)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 510)
	; 16 bytes
	delay_n (n-481)
	lda #168
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 511)
	; 16 bytes
	delay_n (n-482)
	lda #169
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 512)
	; 16 bytes
	delay_n (n-483)
	lda #170
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 513)
	; 16 bytes
	delay_n (n-484)
	lda #171
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 514)
	; 16 bytes
	delay_n (n-485)
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
.elseif(n = 515)
	; 16 bytes
	delay_n (n-486)
	lda #173
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
	lda #173
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
	lda #173
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
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 531 || n = 536 || n = 539 || n = 541 || n = 546)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 532)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 533)
	; 8 bytes
	tax
	pla
	lda #2
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 534)
	; 7 bytes
	tax
	lda #2
	jsr delay_256a_16_clocks
	txa
.elseif(n = 535 || n = 540)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 537)
	; 7 bytes
	pha
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 538)
	; 8 bytes
	tax
	pla
	lda #2
	jsr delay_256a_16_clocks
	txa
.elseif(n = 542 || n = 545)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 543)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 544)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
; 547 to 625
.if(n = 547 || n = 550 || n = 557)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 548 || n = 551)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 549)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 552)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 553)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 554)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 555 || n = 558 || n = 560 || n = 563)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 556)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 559 || n = 561 || n = 562)
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
.elseif(n = 738)
	; 13 bytes
	delay_n (n-733)
	lda #172
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 739)
	; 13 bytes
	delay_n (n-734)
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 740)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 741 || n = 742)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 743) && (n < 762))
; 743 to 761
.if(n = 743 || n = 744 || (n > 754 && n < 758))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 745 || n = 758)
	; 14..16 bytes
	delay_n (n-729)
	lda #168
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 746 || n = 759)
	; 14..16 bytes
	delay_n (n-730)
	lda #169
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 747 || n = 760)
	; 14..16 bytes
	delay_n (n-731)
	lda #170
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 748 || n = 761)
	; 14..16 bytes
	delay_n (n-732)
	lda #171
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 749)
	; 14 bytes
	delay_n (n-733)
	lda #172
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 750 || n = 752)
	; 14..15 bytes
	delay_n (n-734)
	lda #173
	jsr delay_a_25_clocks
	lda #2
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
.elseif(n = 753)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 754)
	; 16 bytes
	delay_n (n-749)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 762 to 781
.if(n = 762)
	; 16 bytes
	delay_n (n-733)
	lda #172
	jsr delay_a_25_clocks
	lda #2
	jsr delay_256a_16_clocks
	pla
.elseif(n = 763)
	; 16 bytes
	delay_n (n-734)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 787 || n = 792 || n = 795 || n = 797 || n = 802 || n = 810)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 788)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 789)
	; 8 bytes
	tax
	pla
	lda #3
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 790)
	; 7 bytes
	tax
	lda #3
	jsr delay_256a_16_clocks
	txa
.elseif(n = 791 || n = 796)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 793)
	; 7 bytes
	pha
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 794)
	; 8 bytes
	tax
	pla
	lda #3
	jsr delay_256a_16_clocks
	txa
.elseif(n = 798 || n = 801 || n = 803 || n = 806 || n = 813)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 799)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 800)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 804 || n = 807)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 805)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 808)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 809)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 811 || n = 814 || n = 816 || n = 819)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 812)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 815 || n = 817 || n = 818)
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
.elseif(n = 994 || n = 1005)
	; 13..14 bytes
	delay_n (n-989)
	lda #172
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 995 || n = 1006 || n = 1008)
	; 13..15 bytes
	delay_n (n-990)
	lda #173
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 996 || n = 1009)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 996 && n < 1001) || (n > 1010 && n < 1014))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1001 || n = 1014)
	; 14..16 bytes
	delay_n (n-985)
	lda #168
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1002 || n = 1015)
	; 14..16 bytes
	delay_n (n-986)
	lda #169
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1003)
	; 14 bytes
	delay_n (n-987)
	lda #170
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1004)
	; 14 bytes
	delay_n (n-988)
	lda #171
	jsr delay_a_25_clocks
	lda #3
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
.elseif(n = 1010)
	; 16 bytes
	delay_n (n-1005)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	delay_n (n-987)
	lda #170
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1017)
	; 16 bytes
	delay_n (n-988)
	lda #171
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1018)
	; 16 bytes
	delay_n (n-989)
	lda #172
	jsr delay_a_25_clocks
	lda #3
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1019)
	; 16 bytes
	delay_n (n-990)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 1043 || n = 1048 || n = 1051 || n = 1053)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1044)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1045)
	; 8 bytes
	tax
	pla
	lda #4
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 1046)
	; 7 bytes
	tax
	lda #4
	jsr delay_256a_16_clocks
	txa
.elseif(n = 1047 || n = 1052)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1049)
	; 7 bytes
	pha
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1050)
	; 8 bytes
	tax
	pla
	lda #4
	jsr delay_256a_16_clocks
	txa
.elseif(n = 1054)
	; 9 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
; 1055 to 1093
.if(n = 1055)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1056)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1057 || n = 1059 || n = 1062 || n = 1069)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1058 || n = 1066)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1060 || n = 1063)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1061)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1064)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1065)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 1067 || n = 1070 || n = 1072 || n = 1075)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1068)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1071 || n = 1073 || n = 1074)
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
	delay_n (n-1245)
	lda #172
	jsr delay_a_25_clocks
	lda #4
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
; 1251 to 1289
.if(n = 1251 || n = 1262 || n = 1264 || n = 1275)
	; 13..16 bytes
	delay_n (n-1246)
	lda #173
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1252 || n = 1265)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 1252 && n < 1257) || (n > 1266 && n < 1270))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1257 || n = 1270)
	; 14..16 bytes
	delay_n (n-1241)
	lda #168
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1258 || n = 1271)
	; 14..16 bytes
	delay_n (n-1242)
	lda #169
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1259 || n = 1272)
	; 14..16 bytes
	delay_n (n-1243)
	lda #170
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1260 || n = 1273)
	; 14..16 bytes
	delay_n (n-1244)
	lda #171
	jsr delay_a_25_clocks
	lda #4
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1261 || n = 1274)
	; 14..16 bytes
	delay_n (n-1245)
	lda #172
	jsr delay_a_25_clocks
	lda #4
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
.elseif(n = 1266)
	; 16 bytes
	delay_n (n-1261)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 1290) && (n < 1309))
; 1290 to 1308
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 1299 || n = 1304 || n = 1307)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1300)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1301)
	; 8 bytes
	tax
	pla
	lda #5
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 1302)
	; 7 bytes
	tax
	lda #5
	jsr delay_256a_16_clocks
	txa
.elseif(n = 1303 || n = 1308)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1305)
	; 7 bytes
	pha
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1306)
	; 8 bytes
	tax
	pla
	lda #5
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 1309 to 1328
.if(n = 1309 || n = 1314 || n = 1322)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1310 || n = 1313 || n = 1315 || n = 1318 || n = 1325)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1311)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1312)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1316 || n = 1319)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1317)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1320)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1321)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 1323 || n = 1326 || n = 1328)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1324)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1327)
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
; 1329 to 1406
.if(n = 1329 || n = 1330)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1331)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 1506 || n = 1517 || n = 1519)
	; 13..15 bytes
	delay_n (n-1501)
	lda #172
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1507 || n = 1518 || n = 1520)
	; 13..15 bytes
	delay_n (n-1502)
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1508 || n = 1521)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 1508 && n < 1513) || n = 1523)
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1513)
	; 14 bytes
	delay_n (n-1497)
	lda #168
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1514)
	; 14 bytes
	delay_n (n-1498)
	lda #169
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1515)
	; 14 bytes
	delay_n (n-1499)
	lda #170
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1516)
	; 14 bytes
	delay_n (n-1500)
	lda #171
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1522)
	; 16 bytes
	delay_n (n-1517)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 1524) && (n < 1543))
; 1524 to 1542
.if(n = 1524 || n = 1525)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1526)
	; 16 bytes
	delay_n (n-1497)
	lda #168
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1527)
	; 16 bytes
	delay_n (n-1498)
	lda #169
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1528)
	; 16 bytes
	delay_n (n-1499)
	lda #170
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1529)
	; 16 bytes
	delay_n (n-1500)
	lda #171
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1530)
	; 16 bytes
	delay_n (n-1501)
	lda #172
	jsr delay_a_25_clocks
	lda #5
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1531)
	; 16 bytes
	delay_n (n-1502)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 1555 || n = 1560)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1556)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1557)
	; 8 bytes
	tax
	pla
	lda #6
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 1558)
	; 7 bytes
	tax
	lda #6
	jsr delay_256a_16_clocks
	txa
.elseif(n = 1559)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1561)
	; 7 bytes
	pha
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1562)
	; 8 bytes
	tax
	pla
	lda #6
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1565 || n = 1570 || n = 1578)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1566 || n = 1569 || n = 1571 || n = 1574 || n = 1581)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1567)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1568)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1572 || n = 1575)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1573)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1576)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1577)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 1579 || n = 1582 || n = 1584 || n = 1587)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1580)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1583 || n = 1585 || n = 1586)
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
; 1719 to 1796
.if((n > 1718 && n < 1762))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1762 || n = 1773 || n = 1775 || n = 1786)
	; 13..16 bytes
	delay_n (n-1757)
	lda #172
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1763 || n = 1774 || n = 1776 || n = 1787)
	; 13..16 bytes
	delay_n (n-1758)
	lda #173
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1764 || n = 1777)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 1764 && n < 1769) || (n > 1778 && n < 1782))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1769 || n = 1782)
	; 14..16 bytes
	delay_n (n-1753)
	lda #168
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1770 || n = 1783)
	; 14..16 bytes
	delay_n (n-1754)
	lda #169
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1771 || n = 1784)
	; 14..16 bytes
	delay_n (n-1755)
	lda #170
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1772 || n = 1785)
	; 14..16 bytes
	delay_n (n-1756)
	lda #171
	jsr delay_a_25_clocks
	lda #6
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1778)
	; 16 bytes
	delay_n (n-1773)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797)
	; 17 bytes
	pha
	lda #9
	jsr delay_a_25_clocks
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1812)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 1813)
	; 8 bytes
	tax
	pla
	lda #7
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 1814)
	; 7 bytes
	tax
	lda #7
	jsr delay_256a_16_clocks
	txa
.elseif(n = 1815)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 1816 to 1835
.if(n = 1816 || n = 1819 || n = 1821 || n = 1826 || n = 1834)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1817)
	; 7 bytes
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 1818)
	; 8 bytes
	tax
	pla
	lda #7
	jsr delay_256a_16_clocks
	txa
.elseif(n = 1820)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1822 || n = 1825 || n = 1827 || n = 1830)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1823)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1824)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1828 || n = 1831)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1829)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1832)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1833)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 1835)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 1837)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1838 || n = 1840 || n = 1843)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1839 || n = 1841 || n = 1842)
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
.elseif(n = 2018 || n = 2029 || n = 2031)
	; 13..15 bytes
	delay_n (n-2013)
	lda #172
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2019 || n = 2030)
	; 13..14 bytes
	delay_n (n-2014)
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2020)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 2020 && n < 2025))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2025)
	; 14 bytes
	delay_n (n-2009)
	lda #168
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2026)
	; 14 bytes
	delay_n (n-2010)
	lda #169
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2027)
	; 14 bytes
	delay_n (n-2011)
	lda #170
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2028)
	; 14 bytes
	delay_n (n-2012)
	lda #171
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032 || n = 2043)
	; 15..16 bytes
	delay_n (n-2014)
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2033)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 2034)
	; 16 bytes
	delay_n (n-2029)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2034 && n < 2038))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2038)
	; 16 bytes
	delay_n (n-2009)
	lda #168
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2039)
	; 16 bytes
	delay_n (n-2010)
	lda #169
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2040)
	; 16 bytes
	delay_n (n-2011)
	lda #170
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2041)
	; 16 bytes
	delay_n (n-2012)
	lda #171
	jsr delay_a_25_clocks
	lda #7
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2042)
	; 16 bytes
	delay_n (n-2013)
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2068)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2069)
	; 8 bytes
	tax
	pla
	lda #8
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 2070)
	; 7 bytes
	tax
	lda #8
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2076)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2072 || n = 2075 || n = 2077 || n = 2082 || n = 2090)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2073)
	; 7 bytes
	pha
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2074)
	; 8 bytes
	tax
	pla
	lda #8
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2078 || n = 2081 || n = 2083 || n = 2086 || n = 2093)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2079)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2080)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2084 || n = 2087)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2085)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2088)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2089)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2091 || n = 2094 || n = 2096 || n = 2099)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2092)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2095 || n = 2097 || n = 2098)
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
.elseif(n = 2274 || n = 2285 || n = 2287 || n = 2298)
	; 13..16 bytes
	delay_n (n-2269)
	lda #172
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2275 || n = 2286 || n = 2288 || n = 2299)
	; 13..16 bytes
	delay_n (n-2270)
	lda #173
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2276 || n = 2289)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 2276 && n < 2281) || (n > 2290 && n < 2294))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2281 || n = 2294)
	; 14..16 bytes
	delay_n (n-2265)
	lda #168
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2282 || n = 2295)
	; 14..16 bytes
	delay_n (n-2266)
	lda #169
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2283 || n = 2296)
	; 14..16 bytes
	delay_n (n-2267)
	lda #170
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2284 || n = 2297)
	; 14..16 bytes
	delay_n (n-2268)
	lda #171
	jsr delay_a_25_clocks
	lda #8
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2290)
	; 16 bytes
	delay_n (n-2285)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #9
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 2326)
	; 7 bytes
	tax
	lda #9
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2327 || n = 2332)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2328 || n = 2331 || n = 2333 || n = 2338)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2329)
	; 7 bytes
	pha
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2330)
	; 8 bytes
	tax
	pla
	lda #9
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2334 || n = 2337 || n = 2339 || n = 2342)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2335)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2336)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2340 || n = 2343)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2341)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.endif
.else
; 2344 to 2500
.if(n = 2344)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2345)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2346)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2350 || n = 2352 || n = 2355)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2348)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2349)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2351 || n = 2353 || n = 2354)
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
.elseif(n = 2530)
	; 13 bytes
	delay_n (n-2525)
	lda #172
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2531)
	; 13 bytes
	delay_n (n-2526)
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2532)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 2532 && n < 2537))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2537)
	; 14 bytes
	delay_n (n-2521)
	lda #168
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2538)
	; 14 bytes
	delay_n (n-2522)
	lda #169
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2539)
	; 14 bytes
	delay_n (n-2523)
	lda #170
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540 || n = 2553)
	; 14..16 bytes
	delay_n (n-2524)
	lda #171
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2541 || n = 2543 || n = 2554)
	; 14..16 bytes
	delay_n (n-2525)
	lda #172
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2542 || n = 2544 || n = 2555)
	; 14..16 bytes
	delay_n (n-2526)
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2545)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 2546)
	; 16 bytes
	delay_n (n-2541)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif((n > 2546 && n < 2550))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2550)
	; 16 bytes
	delay_n (n-2521)
	lda #168
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2551)
	; 16 bytes
	delay_n (n-2522)
	lda #169
	jsr delay_a_25_clocks
	lda #9
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2552)
	; 16 bytes
	delay_n (n-2523)
	lda #170
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.if(n = 2579 || n = 2584 || n = 2587 || n = 2589 || n = 2594 || n = 2602)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2580)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2581)
	; 8 bytes
	tax
	pla
	lda #10
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 2582)
	; 7 bytes
	tax
	lda #10
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2583 || n = 2588)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2585)
	; 7 bytes
	pha
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2586)
	; 8 bytes
	tax
	pla
	lda #10
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2590 || n = 2593 || n = 2595 || n = 2598 || n = 2605)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2591)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2592)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2596 || n = 2599)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2597)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2600)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2601)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2603 || n = 2606 || n = 2608 || n = 2611)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2604)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2607 || n = 2609 || n = 2610)
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
.elseif(n = 2786 || n = 2797 || n = 2799 || n = 2810)
	; 13..16 bytes
	delay_n (n-2781)
	lda #172
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2787 || n = 2798 || n = 2800 || n = 2811)
	; 13..16 bytes
	delay_n (n-2782)
	lda #173
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2788 || n = 2801)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 2788 && n < 2793) || (n > 2802 && n < 2806))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2793 || n = 2806)
	; 14..16 bytes
	delay_n (n-2777)
	lda #168
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2794 || n = 2807)
	; 14..16 bytes
	delay_n (n-2778)
	lda #169
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2795 || n = 2808)
	; 14..16 bytes
	delay_n (n-2779)
	lda #170
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2796 || n = 2809)
	; 14..16 bytes
	delay_n (n-2780)
	lda #171
	jsr delay_a_25_clocks
	lda #10
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2802)
	; 16 bytes
	delay_n (n-2797)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 2835 || n = 2840 || n = 2843 || n = 2845 || n = 2850)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2836)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 2837)
	; 8 bytes
	tax
	pla
	lda #11
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 2838)
	; 7 bytes
	tax
	lda #11
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2839 || n = 2844)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2841)
	; 7 bytes
	pha
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 2842)
	; 8 bytes
	tax
	pla
	lda #11
	jsr delay_256a_16_clocks
	txa
.elseif(n = 2846 || n = 2849 || n = 2851)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 2847)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2848)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2855)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2853)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2854 || n = 2861)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2856)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2857)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2858)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2859 || n = 2862 || n = 2864 || n = 2867)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2860)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2863 || n = 2865 || n = 2866)
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
.elseif(n = 3042)
	; 13 bytes
	delay_n (n-3037)
	lda #172
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3043)
	; 13 bytes
	delay_n (n-3038)
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3044)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3045 || n = 3046)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 3047) && (n < 3086))
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if(n = 3047 || n = 3048 || (n > 3058 && n < 3062))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3049 || n = 3062)
	; 14..16 bytes
	delay_n (n-3033)
	lda #168
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3050 || n = 3063)
	; 14..16 bytes
	delay_n (n-3034)
	lda #169
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3051 || n = 3064)
	; 14..16 bytes
	delay_n (n-3035)
	lda #170
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3052 || n = 3065)
	; 14..16 bytes
	delay_n (n-3036)
	lda #171
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3053)
	; 14 bytes
	delay_n (n-3037)
	lda #172
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3054 || n = 3056)
	; 14..15 bytes
	delay_n (n-3038)
	lda #173
	jsr delay_a_25_clocks
	lda #11
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
.elseif(n = 3057)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 3058)
	; 16 bytes
	delay_n (n-3053)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 3066 to 3085
.if(n = 3066)
	; 16 bytes
	delay_n (n-3037)
	lda #172
	jsr delay_a_25_clocks
	lda #11
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3067)
	; 16 bytes
	delay_n (n-3038)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 3091 || n = 3096 || n = 3099 || n = 3101 || n = 3106 || n = 3114)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3092)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3093)
	; 8 bytes
	tax
	pla
	lda #12
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 3094)
	; 7 bytes
	tax
	lda #12
	jsr delay_256a_16_clocks
	txa
.elseif(n = 3095 || n = 3100)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3097)
	; 7 bytes
	pha
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3098)
	; 8 bytes
	tax
	pla
	lda #12
	jsr delay_256a_16_clocks
	txa
.elseif(n = 3102 || n = 3105 || n = 3107 || n = 3110 || n = 3117)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3103)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3104)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3108 || n = 3111)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3109)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3112)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3113)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3115 || n = 3118 || n = 3120 || n = 3123)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3116)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3119 || n = 3121 || n = 3122)
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
.elseif(n = 3298 || n = 3309 || n = 3311)
	; 13..15 bytes
	delay_n (n-3293)
	lda #172
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3299 || n = 3310 || n = 3312)
	; 13..15 bytes
	delay_n (n-3294)
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3300 || n = 3313)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 3300 && n < 3305) || (n > 3314 && n < 3318))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3305 || n = 3318)
	; 14..16 bytes
	delay_n (n-3289)
	lda #168
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3306 || n = 3319)
	; 14..16 bytes
	delay_n (n-3290)
	lda #169
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3307 || n = 3320)
	; 14..16 bytes
	delay_n (n-3291)
	lda #170
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3308)
	; 14 bytes
	delay_n (n-3292)
	lda #171
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3314)
	; 16 bytes
	delay_n (n-3309)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 3321) && (n < 3340))
; 3321 to 3339
.if(n = 3321)
	; 16 bytes
	delay_n (n-3292)
	lda #171
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3322)
	; 16 bytes
	delay_n (n-3293)
	lda #172
	jsr delay_a_25_clocks
	lda #12
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3323)
	; 16 bytes
	delay_n (n-3294)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 3347 || n = 3352 || n = 3355 || n = 3357)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 3348)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3349)
	; 8 bytes
	tax
	pla
	lda #13
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 3350)
	; 7 bytes
	tax
	lda #13
	jsr delay_256a_16_clocks
	txa
.elseif(n = 3351 || n = 3356)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3353)
	; 7 bytes
	pha
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3354)
	; 8 bytes
	tax
	pla
	lda #13
	jsr delay_256a_16_clocks
	txa
.elseif(n = 3358)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 3359)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 3361 || n = 3363 || n = 3366 || n = 3373)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3362 || n = 3370)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3364 || n = 3367)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3365)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3368)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3369)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3371 || n = 3374 || n = 3376 || n = 3379)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3372)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3375 || n = 3377 || n = 3378)
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
; 3438 to 3593
.if((n > 3437 && n < 3554))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3554 || n = 3565 || n = 3567 || n = 3578)
	; 13..16 bytes
	delay_n (n-3549)
	lda #172
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3555 || n = 3566 || n = 3568 || n = 3579)
	; 13..16 bytes
	delay_n (n-3550)
	lda #173
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3556 || n = 3569)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 3556 && n < 3561) || (n > 3570 && n < 3574))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3561 || n = 3574)
	; 14..16 bytes
	delay_n (n-3545)
	lda #168
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3562 || n = 3575)
	; 14..16 bytes
	delay_n (n-3546)
	lda #169
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3563 || n = 3576)
	; 14..16 bytes
	delay_n (n-3547)
	lda #170
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3564 || n = 3577)
	; 14..16 bytes
	delay_n (n-3548)
	lda #171
	jsr delay_a_25_clocks
	lda #13
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3570)
	; 16 bytes
	delay_n (n-3565)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
.if((n >= 3594) && (n < 3613))
; 3594 to 3612
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 3603 || n = 3608 || n = 3611)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3604)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3605)
	; 8 bytes
	tax
	pla
	lda #14
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 3606)
	; 7 bytes
	tax
	lda #14
	jsr delay_256a_16_clocks
	txa
.elseif(n = 3607 || n = 3612)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3609)
	; 7 bytes
	pha
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3610)
	; 8 bytes
	tax
	pla
	lda #14
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 3613 to 3632
.if(n = 3613 || n = 3618 || n = 3626)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3614 || n = 3617 || n = 3619 || n = 3622 || n = 3629)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3615)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3616)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3620 || n = 3623)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3621)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3624)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3625)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3627 || n = 3630 || n = 3632)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3628)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3631)
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
; 3633 to 3671
.if(n = 3633 || n = 3634)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3635)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 3635 && n < 3672))
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
; 3672 to 3750
.if((n > 3671 && n < 3751))
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
.elseif(n = 3810 || n = 3821)
	; 13..14 bytes
	delay_n (n-3805)
	lda #172
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3811 || n = 3822 || n = 3824)
	; 13..15 bytes
	delay_n (n-3806)
	lda #173
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3812 || n = 3825)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 3812 && n < 3817) || n = 3827 || n = 3828)
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3817)
	; 14 bytes
	delay_n (n-3801)
	lda #168
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3818)
	; 14 bytes
	delay_n (n-3802)
	lda #169
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3819)
	; 14 bytes
	delay_n (n-3803)
	lda #170
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3820)
	; 14 bytes
	delay_n (n-3804)
	lda #171
	jsr delay_a_25_clocks
	lda #14
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
.elseif(n = 3826)
	; 16 bytes
	delay_n (n-3821)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3830)
	; 16 bytes
	delay_n (n-3801)
	lda #168
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3831)
	; 16 bytes
	delay_n (n-3802)
	lda #169
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3832)
	; 16 bytes
	delay_n (n-3803)
	lda #170
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3833)
	; 16 bytes
	delay_n (n-3804)
	lda #171
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3834)
	; 16 bytes
	delay_n (n-3805)
	lda #172
	jsr delay_a_25_clocks
	lda #14
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3835)
	; 16 bytes
	delay_n (n-3806)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 3859 || n = 3864 || n = 3867)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 3860)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 3861)
	; 8 bytes
	tax
	pla
	lda #15
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 3862)
	; 7 bytes
	tax
	lda #15
	jsr delay_256a_16_clocks
	txa
.elseif(n = 3863)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3865)
	; 7 bytes
	pha
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 3866)
	; 8 bytes
	tax
	pla
	lda #15
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.else
; 3868 to 3906
.if(n = 3868)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 3869 || n = 3874 || n = 3882)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870 || n = 3873 || n = 3875 || n = 3878 || n = 3885)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3871)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3872)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3876 || n = 3879)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3877)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3880)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3881)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3883 || n = 3886 || n = 3888 || n = 3891)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3884)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3887 || n = 3889 || n = 3890)
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
; 4063 to 4101
.if((n > 4062 && n < 4066))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4066 || n = 4077 || n = 4079 || n = 4090)
	; 13..16 bytes
	delay_n (n-4061)
	lda #172
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4067 || n = 4078 || n = 4080 || n = 4091)
	; 13..16 bytes
	delay_n (n-4062)
	lda #173
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4068 || n = 4081)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 4068 && n < 4073) || (n > 4082 && n < 4086))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4073 || n = 4086)
	; 14..16 bytes
	delay_n (n-4057)
	lda #168
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4074 || n = 4087)
	; 14..16 bytes
	delay_n (n-4058)
	lda #169
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4075 || n = 4088)
	; 14..16 bytes
	delay_n (n-4059)
	lda #170
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4076 || n = 4089)
	; 14..16 bytes
	delay_n (n-4060)
	lda #171
	jsr delay_a_25_clocks
	lda #15
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4082)
	; 16 bytes
	delay_n (n-4077)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 4115 || n = 4120)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4116)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 4117)
	; 8 bytes
	tax
	pla
	lda #16
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 4118)
	; 7 bytes
	tax
	lda #16
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4119)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 4121 to 4140
.if(n = 4121)
	; 7 bytes
	pha
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4122)
	; 8 bytes
	tax
	pla
	lda #16
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4123 || n = 4125 || n = 4130 || n = 4138)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4124)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4126 || n = 4129 || n = 4131 || n = 4134)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4127)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4128)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4132 || n = 4135)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4133)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4136)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4137)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4139)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4140)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4142 || n = 4144 || n = 4147)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4143 || n = 4145 || n = 4146)
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
.elseif(n = 4322 || n = 4333)
	; 13..14 bytes
	delay_n (n-4317)
	lda #172
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4323 || n = 4334)
	; 13..14 bytes
	delay_n (n-4318)
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4324)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 4324 && n < 4329))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4329)
	; 14 bytes
	delay_n (n-4313)
	lda #168
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4330)
	; 14 bytes
	delay_n (n-4314)
	lda #169
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4331)
	; 14 bytes
	delay_n (n-4315)
	lda #170
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4332)
	; 14 bytes
	delay_n (n-4316)
	lda #171
	jsr delay_a_25_clocks
	lda #16
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
.if(n = 4336 || n = 4347)
	; 15..16 bytes
	delay_n (n-4318)
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4337)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 4338)
	; 16 bytes
	delay_n (n-4333)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4338 && n < 4342))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4342)
	; 16 bytes
	delay_n (n-4313)
	lda #168
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4343)
	; 16 bytes
	delay_n (n-4314)
	lda #169
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4344)
	; 16 bytes
	delay_n (n-4315)
	lda #170
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4345)
	; 16 bytes
	delay_n (n-4316)
	lda #171
	jsr delay_a_25_clocks
	lda #16
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4346)
	; 16 bytes
	delay_n (n-4317)
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 4372)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 4373)
	; 8 bytes
	tax
	pla
	lda #17
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 4374)
	; 7 bytes
	tax
	lda #17
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4375)
	; 8 bytes
	pha
	delay_n (n-3)
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
.if(n = 4376 || n = 4379 || n = 4381 || n = 4386 || n = 4394)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 7 bytes
	pha
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4378)
	; 8 bytes
	tax
	pla
	lda #17
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4380)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4382 || n = 4385 || n = 4387 || n = 4390 || n = 4397)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4383)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4384)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4388 || n = 4391)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4389)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4392)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4393)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4395 || n = 4398 || n = 4400 || n = 4403)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4396)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4399 || n = 4401 || n = 4402)
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
; 4532 to 4609
.if((n > 4531 && n < 4578))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4578 || n = 4589 || n = 4591 || n = 4602)
	; 13..16 bytes
	delay_n (n-4573)
	lda #172
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4579 || n = 4590 || n = 4592 || n = 4603)
	; 13..16 bytes
	delay_n (n-4574)
	lda #173
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4580 || n = 4593)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 4580 && n < 4585) || (n > 4594 && n < 4598))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4585 || n = 4598)
	; 14..16 bytes
	delay_n (n-4569)
	lda #168
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4586 || n = 4599)
	; 14..16 bytes
	delay_n (n-4570)
	lda #169
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4587 || n = 4600)
	; 14..16 bytes
	delay_n (n-4571)
	lda #170
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4588 || n = 4601)
	; 14..16 bytes
	delay_n (n-4572)
	lda #171
	jsr delay_a_25_clocks
	lda #17
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4594)
	; 16 bytes
	delay_n (n-4589)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #18
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 4630)
	; 7 bytes
	tax
	lda #18
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4631 || n = 4636)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4632 || n = 4635 || n = 4637 || n = 4642)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4633)
	; 7 bytes
	pha
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4634)
	; 8 bytes
	tax
	pla
	lda #18
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4638 || n = 4641 || n = 4643 || n = 4646)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 4639)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4640)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4644 || n = 4647)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4645)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4648)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4650)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4651 || n = 4654 || n = 4656 || n = 4659)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4652)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4653)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4655 || n = 4657 || n = 4658)
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
.elseif(n = 4834)
	; 13 bytes
	delay_n (n-4829)
	lda #172
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4835)
	; 13 bytes
	delay_n (n-4830)
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4836)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 4836 && n < 4841))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4841)
	; 14 bytes
	delay_n (n-4825)
	lda #168
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4842)
	; 14 bytes
	delay_n (n-4826)
	lda #169
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4843)
	; 14 bytes
	delay_n (n-4827)
	lda #170
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
.if(n = 4844 || n = 4857)
	; 14..16 bytes
	delay_n (n-4828)
	lda #171
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4845 || n = 4858)
	; 14..16 bytes
	delay_n (n-4829)
	lda #172
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4846 || n = 4848 || n = 4859)
	; 14..16 bytes
	delay_n (n-4830)
	lda #173
	jsr delay_a_25_clocks
	lda #18
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
.elseif(n = 4849)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 4850)
	; 16 bytes
	delay_n (n-4845)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif((n > 4850 && n < 4854))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4854)
	; 16 bytes
	delay_n (n-4825)
	lda #168
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4855)
	; 16 bytes
	delay_n (n-4826)
	lda #169
	jsr delay_a_25_clocks
	lda #18
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4856)
	; 16 bytes
	delay_n (n-4827)
	lda #170
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.if(n = 4883 || n = 4888 || n = 4891 || n = 4893 || n = 4898 || n = 4906)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4884)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 4885)
	; 8 bytes
	tax
	pla
	lda #19
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 4886)
	; 7 bytes
	tax
	lda #19
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4887 || n = 4892)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 4889)
	; 7 bytes
	pha
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 4890)
	; 8 bytes
	tax
	pla
	lda #19
	jsr delay_256a_16_clocks
	txa
.elseif(n = 4894 || n = 4897 || n = 4899 || n = 4902 || n = 4909)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4895)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4896)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4900 || n = 4903)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4901)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4904)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4905)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4907 || n = 4910 || n = 4912 || n = 4915)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4908)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 4911 || n = 4913 || n = 4914)
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
.elseif(n = 5090 || n = 5101 || n = 5114)
	; 13..16 bytes
	delay_n (n-5085)
	lda #172
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5091 || n = 5102 || n = 5104 || n = 5115)
	; 13..16 bytes
	delay_n (n-5086)
	lda #173
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5092 || n = 5105)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 5092 && n < 5097) || (n > 5106 && n < 5110))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5097 || n = 5110)
	; 14..16 bytes
	delay_n (n-5081)
	lda #168
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5098 || n = 5111)
	; 14..16 bytes
	delay_n (n-5082)
	lda #169
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5099 || n = 5112)
	; 14..16 bytes
	delay_n (n-5083)
	lda #170
	jsr delay_a_25_clocks
	lda #19
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5100 || n = 5113)
	; 14..16 bytes
	delay_n (n-5084)
	lda #171
	jsr delay_a_25_clocks
	lda #19
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
.elseif(n = 5106)
	; 16 bytes
	delay_n (n-5101)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 5139 || n = 5144 || n = 5147 || n = 5149 || n = 5154)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5140)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5141)
	; 8 bytes
	tax
	pla
	lda #20
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 5142)
	; 7 bytes
	tax
	lda #20
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5143 || n = 5148)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5145)
	; 7 bytes
	pha
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5146)
	; 8 bytes
	tax
	pla
	lda #20
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5150 || n = 5153 || n = 5155)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 5151)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5152)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5156)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5158 || n = 5165)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5159)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5160)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5161)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5162)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5163 || n = 5166 || n = 5168 || n = 5171)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5164)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5167 || n = 5169 || n = 5170)
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
.elseif(n = 5346)
	; 13 bytes
	delay_n (n-5341)
	lda #172
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5347)
	; 13 bytes
	delay_n (n-5342)
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5348)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 5348 && n < 5352))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if(n = 5352 || (n > 5362 && n < 5366))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5353 || n = 5366)
	; 14..16 bytes
	delay_n (n-5337)
	lda #168
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5354 || n = 5367)
	; 14..16 bytes
	delay_n (n-5338)
	lda #169
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5355 || n = 5368)
	; 14..16 bytes
	delay_n (n-5339)
	lda #170
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5356 || n = 5369)
	; 14..16 bytes
	delay_n (n-5340)
	lda #171
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5357 || n = 5370)
	; 14..16 bytes
	delay_n (n-5341)
	lda #172
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5358 || n = 5360)
	; 14..15 bytes
	delay_n (n-5342)
	lda #173
	jsr delay_a_25_clocks
	lda #20
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
.elseif(n = 5361)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5362)
	; 16 bytes
	delay_n (n-5357)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #20
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 16 bytes
	delay_n (n-5342)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 5395 || n = 5400 || n = 5403 || n = 5405 || n = 5410 || n = 5418)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5396)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5397)
	; 8 bytes
	tax
	pla
	lda #21
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 5398)
	; 7 bytes
	tax
	lda #21
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5399 || n = 5404)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5401)
	; 7 bytes
	pha
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5402)
	; 8 bytes
	tax
	pla
	lda #21
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5406 || n = 5409 || n = 5411 || n = 5414 || n = 5421)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5407)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5408)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5412 || n = 5415)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5413)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5416)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5417)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5419 || n = 5422 || n = 5424 || n = 5427)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5420)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5423 || n = 5425 || n = 5426)
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
.elseif(n = 5602 || n = 5613)
	; 13..14 bytes
	delay_n (n-5597)
	lda #172
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5603 || n = 5614 || n = 5616)
	; 13..15 bytes
	delay_n (n-5598)
	lda #173
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5604 || n = 5617)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 5604 && n < 5609) || (n > 5618 && n < 5622))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5609 || n = 5622)
	; 14..16 bytes
	delay_n (n-5593)
	lda #168
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5610 || n = 5623)
	; 14..16 bytes
	delay_n (n-5594)
	lda #169
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5611 || n = 5624)
	; 14..16 bytes
	delay_n (n-5595)
	lda #170
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5612 || n = 5625)
	; 14..16 bytes
	delay_n (n-5596)
	lda #171
	jsr delay_a_25_clocks
	lda #21
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
.elseif(n = 5618)
	; 16 bytes
	delay_n (n-5613)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	delay_n (n-5597)
	lda #172
	jsr delay_a_25_clocks
	lda #21
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5627)
	; 16 bytes
	delay_n (n-5598)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 5651 || n = 5656 || n = 5659 || n = 5661)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 5652)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5653)
	; 8 bytes
	tax
	pla
	lda #22
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 5654)
	; 7 bytes
	tax
	lda #22
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5655 || n = 5660)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5657)
	; 7 bytes
	pha
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5658)
	; 8 bytes
	tax
	pla
	lda #22
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5662)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 5663)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5664)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5667 || n = 5670 || n = 5677)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5666 || n = 5674)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5668 || n = 5671)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5669)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5672)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5673)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5675 || n = 5678 || n = 5680 || n = 5683)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5676)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5679 || n = 5681 || n = 5682)
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
.elseif(n = 5858)
	; 13 bytes
	delay_n (n-5853)
	lda #172
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5859)
	; 13 bytes
	delay_n (n-5854)
	lda #173
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860 || n = 5873)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 5860 && n < 5865) || (n > 5874 && n < 5878))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5865 || n = 5878)
	; 14..16 bytes
	delay_n (n-5849)
	lda #168
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5866 || n = 5879)
	; 14..16 bytes
	delay_n (n-5850)
	lda #169
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5867 || n = 5880)
	; 14..16 bytes
	delay_n (n-5851)
	lda #170
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5868 || n = 5881)
	; 14..16 bytes
	delay_n (n-5852)
	lda #171
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5869 || n = 5882)
	; 14..16 bytes
	delay_n (n-5853)
	lda #172
	jsr delay_a_25_clocks
	lda #22
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5870 || n = 5872 || n = 5883)
	; 14..16 bytes
	delay_n (n-5854)
	lda #173
	jsr delay_a_25_clocks
	lda #22
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
.elseif(n = 5874)
	; 16 bytes
	delay_n (n-5869)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 5907 || n = 5912 || n = 5915 || n = 5917 || n = 5922 || n = 5930)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5908)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5909)
	; 8 bytes
	tax
	pla
	lda #23
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 5910)
	; 7 bytes
	tax
	lda #23
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5911 || n = 5916)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5913)
	; 7 bytes
	pha
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 5914)
	; 8 bytes
	tax
	pla
	lda #23
	jsr delay_256a_16_clocks
	txa
.elseif(n = 5918 || n = 5921 || n = 5923 || n = 5926 || n = 5933)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5919)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5920)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5924 || n = 5927)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5925)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5928)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5929)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5931 || n = 5934 || n = 5936)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5932)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 5935 || n = 5937)
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
.if(n = 5938)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 5939)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 6114 || n = 6125 || n = 6127)
	; 13..15 bytes
	delay_n (n-6109)
	lda #172
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6115 || n = 6126 || n = 6128)
	; 13..15 bytes
	delay_n (n-6110)
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6116 || n = 6129)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 6116 && n < 6121) || n = 6131 || n = 6132)
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6121)
	; 14 bytes
	delay_n (n-6105)
	lda #168
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6122)
	; 14 bytes
	delay_n (n-6106)
	lda #169
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6123)
	; 14 bytes
	delay_n (n-6107)
	lda #170
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6124)
	; 14 bytes
	delay_n (n-6108)
	lda #171
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6130)
	; 16 bytes
	delay_n (n-6125)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6134)
	; 16 bytes
	delay_n (n-6105)
	lda #168
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6135)
	; 16 bytes
	delay_n (n-6106)
	lda #169
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6136)
	; 16 bytes
	delay_n (n-6107)
	lda #170
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6137)
	; 16 bytes
	delay_n (n-6108)
	lda #171
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6138)
	; 16 bytes
	delay_n (n-6109)
	lda #172
	jsr delay_a_25_clocks
	lda #23
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6139)
	; 16 bytes
	delay_n (n-6110)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 6163 || n = 6168 || n = 6171)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6164)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6165)
	; 8 bytes
	tax
	pla
	lda #24
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 6166)
	; 7 bytes
	tax
	lda #24
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6167)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6169)
	; 7 bytes
	pha
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6170)
	; 8 bytes
	tax
	pla
	lda #24
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6173 || n = 6178 || n = 6186)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174 || n = 6177 || n = 6179 || n = 6182 || n = 6189)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6175)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6176)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6180 || n = 6183)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6181)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6184)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6185)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6187 || n = 6190 || n = 6192 || n = 6195)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6188)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6191 || n = 6193 || n = 6194)
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
; 6251 to 6406
.if((n > 6250 && n < 6370))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6370 || n = 6381 || n = 6383 || n = 6394)
	; 13..16 bytes
	delay_n (n-6365)
	lda #172
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6371 || n = 6382 || n = 6384 || n = 6395)
	; 13..16 bytes
	delay_n (n-6366)
	lda #173
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6372 || n = 6385)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 6372 && n < 6377) || (n > 6386 && n < 6390))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6377 || n = 6390)
	; 14..16 bytes
	delay_n (n-6361)
	lda #168
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6378 || n = 6391)
	; 14..16 bytes
	delay_n (n-6362)
	lda #169
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6379 || n = 6392)
	; 14..16 bytes
	delay_n (n-6363)
	lda #170
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6380 || n = 6393)
	; 14..16 bytes
	delay_n (n-6364)
	lda #171
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6386)
	; 16 bytes
	delay_n (n-6381)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
.if((n >= 6407) && (n < 6426))
; 6407 to 6425
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 6419 || n = 6424)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6420)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6421)
	; 8 bytes
	tax
	pla
	lda #25
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 6422)
	; 7 bytes
	tax
	lda #25
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6423)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6425)
	; 7 bytes
	pha
	lda #25
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 6426 to 6445
.if(n = 6426)
	; 8 bytes
	tax
	pla
	lda #25
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6427 || n = 6429 || n = 6434 || n = 6442)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6428)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6430 || n = 6433 || n = 6435 || n = 6438 || n = 6445)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6431)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6432)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6436 || n = 6439)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6437)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6440)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6441)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6443)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6444)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
; 6446 to 6484
.if(n = 6446 || n = 6448 || n = 6451)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6447 || n = 6449 || n = 6450)
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
.elseif(n = 6626 || n = 6637 || n = 6639)
	; 13..15 bytes
	delay_n (n-6621)
	lda #172
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6627 || n = 6638 || n = 6640)
	; 13..15 bytes
	delay_n (n-6622)
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6628)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 6628 && n < 6633))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6633)
	; 14 bytes
	delay_n (n-6617)
	lda #168
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6634)
	; 14 bytes
	delay_n (n-6618)
	lda #169
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6635)
	; 14 bytes
	delay_n (n-6619)
	lda #170
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6636)
	; 14 bytes
	delay_n (n-6620)
	lda #171
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6642)
	; 16 bytes
	delay_n (n-6637)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif((n > 6642 && n < 6646))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6646)
	; 16 bytes
	delay_n (n-6617)
	lda #168
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6647)
	; 16 bytes
	delay_n (n-6618)
	lda #169
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6648)
	; 16 bytes
	delay_n (n-6619)
	lda #170
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6649)
	; 16 bytes
	delay_n (n-6620)
	lda #171
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6650)
	; 16 bytes
	delay_n (n-6621)
	lda #172
	jsr delay_a_25_clocks
	lda #25
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6651)
	; 16 bytes
	delay_n (n-6622)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6676)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6677)
	; 8 bytes
	tax
	pla
	lda #26
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 6678)
	; 7 bytes
	tax
	lda #26
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6679)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6683 || n = 6685 || n = 6690 || n = 6698)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6681)
	; 7 bytes
	pha
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6682)
	; 8 bytes
	tax
	pla
	lda #26
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6684)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6686 || n = 6689 || n = 6691 || n = 6694 || n = 6701)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6687)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6688)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6692 || n = 6695)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6693)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6696)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6697)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6699 || n = 6702 || n = 6704 || n = 6707)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6700)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6703 || n = 6705 || n = 6706)
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
; 6876 to 6914
.if((n > 6875 && n < 6882))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6882 || n = 6893 || n = 6895 || n = 6906)
	; 13..16 bytes
	delay_n (n-6877)
	lda #172
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6883 || n = 6894 || n = 6896 || n = 6907)
	; 13..16 bytes
	delay_n (n-6878)
	lda #173
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6884 || n = 6897)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 6884 && n < 6889) || (n > 6898 && n < 6902))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6889 || n = 6902)
	; 14..16 bytes
	delay_n (n-6873)
	lda #168
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6890 || n = 6903)
	; 14..16 bytes
	delay_n (n-6874)
	lda #169
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6891 || n = 6904)
	; 14..16 bytes
	delay_n (n-6875)
	lda #170
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6892 || n = 6905)
	; 14..16 bytes
	delay_n (n-6876)
	lda #171
	jsr delay_a_25_clocks
	lda #26
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6898)
	; 16 bytes
	delay_n (n-6893)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 6932)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 6933)
	; 8 bytes
	tax
	pla
	lda #27
	jsr delay_256a_11_clocks_
	txa
.endif
.else
; 6934 to 6953
.if(n = 6934)
	; 7 bytes
	tax
	lda #27
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6935 || n = 6940)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 6936 || n = 6939 || n = 6941 || n = 6946)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6937)
	; 7 bytes
	pha
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 6938)
	; 8 bytes
	tax
	pla
	lda #27
	jsr delay_256a_16_clocks
	txa
.elseif(n = 6942 || n = 6945 || n = 6947 || n = 6950)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 6943)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6944)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6948 || n = 6951)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6949)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6952)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6953)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 6955 || n = 6958 || n = 6960 || n = 6963)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6956)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 6957)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6959 || n = 6961 || n = 6962)
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
.elseif(n = 7138)
	; 13 bytes
	delay_n (n-7133)
	lda #172
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7139)
	; 13 bytes
	delay_n (n-7134)
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7140)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7140 && n < 7145))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7145)
	; 14 bytes
	delay_n (n-7129)
	lda #168
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7146)
	; 14 bytes
	delay_n (n-7130)
	lda #169
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7147)
	; 14 bytes
	delay_n (n-7131)
	lda #170
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7148)
	; 14 bytes
	delay_n (n-7132)
	lda #171
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149 || n = 7151 || n = 7162)
	; 14..16 bytes
	delay_n (n-7133)
	lda #172
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7150 || n = 7152 || n = 7163)
	; 14..16 bytes
	delay_n (n-7134)
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7153)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7154)
	; 16 bytes
	delay_n (n-7149)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7154 && n < 7158))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7158)
	; 16 bytes
	delay_n (n-7129)
	lda #168
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7159)
	; 16 bytes
	delay_n (n-7130)
	lda #169
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7160)
	; 16 bytes
	delay_n (n-7131)
	lda #170
	jsr delay_a_25_clocks
	lda #27
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7161)
	; 16 bytes
	delay_n (n-7132)
	lda #171
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #28
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 7190)
	; 7 bytes
	tax
	lda #28
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7191 || n = 7196)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7192 || n = 7195 || n = 7197 || n = 7202 || n = 7210)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7193)
	; 7 bytes
	pha
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7194)
	; 8 bytes
	tax
	pla
	lda #28
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7198 || n = 7201 || n = 7203 || n = 7206 || n = 7213)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7199)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7200)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7204 || n = 7207)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7205)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7208)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7209)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7211 || n = 7214 || n = 7216 || n = 7219)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7212)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7215 || n = 7217 || n = 7218)
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
.elseif(n = 7394 || n = 7405 || n = 7407 || n = 7418)
	; 13..16 bytes
	delay_n (n-7389)
	lda #172
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7395 || n = 7406 || n = 7408 || n = 7419)
	; 13..16 bytes
	delay_n (n-7390)
	lda #173
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7396 || n = 7409)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 7396 && n < 7401) || (n > 7410 && n < 7414))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7401 || n = 7414)
	; 14..16 bytes
	delay_n (n-7385)
	lda #168
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7402 || n = 7415)
	; 14..16 bytes
	delay_n (n-7386)
	lda #169
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7403 || n = 7416)
	; 14..16 bytes
	delay_n (n-7387)
	lda #170
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7404 || n = 7417)
	; 14..16 bytes
	delay_n (n-7388)
	lda #171
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7410)
	; 16 bytes
	delay_n (n-7405)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 7443 || n = 7448 || n = 7451 || n = 7453 || n = 7458)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7444)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7445)
	; 8 bytes
	tax
	pla
	lda #29
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 7446)
	; 7 bytes
	tax
	lda #29
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7447 || n = 7452)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7449)
	; 7 bytes
	pha
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7450)
	; 8 bytes
	tax
	pla
	lda #29
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7454 || n = 7457 || n = 7459)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 7455)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7456)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7460)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7462 || n = 7469)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7463)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7464)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7465)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7466)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7467 || n = 7470 || n = 7472 || n = 7475)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7468)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7471 || n = 7473 || n = 7474)
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
.elseif(n = 7650)
	; 13 bytes
	delay_n (n-7645)
	lda #172
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7651)
	; 13 bytes
	delay_n (n-7646)
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7652)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 7652 && n < 7657))
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
.if(n = 7657 || n = 7670)
	; 14..16 bytes
	delay_n (n-7641)
	lda #168
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7658 || n = 7671)
	; 14..16 bytes
	delay_n (n-7642)
	lda #169
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7659 || n = 7672)
	; 14..16 bytes
	delay_n (n-7643)
	lda #170
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7660 || n = 7673)
	; 14..16 bytes
	delay_n (n-7644)
	lda #171
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7661 || n = 7663 || n = 7674)
	; 14..16 bytes
	delay_n (n-7645)
	lda #172
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7662 || n = 7664 || n = 7675)
	; 14..16 bytes
	delay_n (n-7646)
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7665)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7666)
	; 16 bytes
	delay_n (n-7661)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	pla
.elseif((n > 7666 && n < 7670))
	; 16 bytes
	php
	delay_n (n-7)
	plp
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 7699 || n = 7704 || n = 7707 || n = 7709 || n = 7714 || n = 7722)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7700)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7701)
	; 8 bytes
	tax
	pla
	lda #30
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 7702)
	; 7 bytes
	tax
	lda #30
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7703 || n = 7708)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7705)
	; 7 bytes
	pha
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7706)
	; 8 bytes
	tax
	pla
	lda #30
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7710 || n = 7713 || n = 7715 || n = 7718 || n = 7725)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7711)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7712)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7716 || n = 7719)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7717)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7720)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7721)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7723 || n = 7726 || n = 7728 || n = 7731)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7724)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7727 || n = 7729 || n = 7730)
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
.elseif(n = 7906 || n = 7917)
	; 13..14 bytes
	delay_n (n-7901)
	lda #172
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7907 || n = 7918 || n = 7920)
	; 13..15 bytes
	delay_n (n-7902)
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7908 || n = 7921)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 7908 && n < 7913) || (n > 7922 && n < 7926))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7913 || n = 7926)
	; 14..16 bytes
	delay_n (n-7897)
	lda #168
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7914 || n = 7927)
	; 14..16 bytes
	delay_n (n-7898)
	lda #169
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7915 || n = 7928)
	; 14..16 bytes
	delay_n (n-7899)
	lda #170
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7916 || n = 7929)
	; 14..16 bytes
	delay_n (n-7900)
	lda #171
	jsr delay_a_25_clocks
	lda #30
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
.elseif(n = 7922)
	; 16 bytes
	delay_n (n-7917)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 16 bytes
	delay_n (n-7901)
	lda #172
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7931)
	; 16 bytes
	delay_n (n-7902)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 7955 || n = 7960 || n = 7963 || n = 7965)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 7956)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 7957)
	; 8 bytes
	tax
	pla
	lda #31
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 7958)
	; 7 bytes
	tax
	lda #31
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7959 || n = 7964)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 7961)
	; 7 bytes
	pha
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 7962)
	; 8 bytes
	tax
	pla
	lda #31
	jsr delay_256a_16_clocks
	txa
.elseif(n = 7966)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 7967)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 7969 || n = 7971 || n = 7974 || n = 7981)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7970 || n = 7978)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7972 || n = 7975)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7973)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7976)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7977)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7979 || n = 7982 || n = 7984 || n = 7987)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7980)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 7983 || n = 7985 || n = 7986)
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
.elseif(n = 8162)
	; 13 bytes
	delay_n (n-8157)
	lda #172
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8163)
	; 13 bytes
	delay_n (n-8158)
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8164)
	; 13 bytes
	pha
	delay_n (n-3)
.endif
.else
.if((n >= 8165) && (n < 8184))
; 8165 to 8183
.if((n > 8164 && n < 8169) || (n > 8178 && n < 8182))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8169 || n = 8182)
	; 14..16 bytes
	delay_n (n-8153)
	lda #168
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8170 || n = 8183)
	; 14..16 bytes
	delay_n (n-8154)
	lda #169
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8171)
	; 14 bytes
	delay_n (n-8155)
	lda #170
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8172)
	; 14 bytes
	delay_n (n-8156)
	lda #171
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8173)
	; 14 bytes
	delay_n (n-8157)
	lda #172
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8174 || n = 8176)
	; 14..15 bytes
	delay_n (n-8158)
	lda #173
	jsr delay_a_25_clocks
	lda #31
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
.elseif(n = 8177)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8178)
	; 16 bytes
	delay_n (n-8173)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 16 bytes
	delay_n (n-8155)
	lda #170
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8185)
	; 16 bytes
	delay_n (n-8156)
	lda #171
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8186)
	; 16 bytes
	delay_n (n-8157)
	lda #172
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8187)
	; 16 bytes
	delay_n (n-8158)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 8211 || n = 8216 || n = 8219 || n = 8221 || n = 8226 || n = 8234)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8212)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8213)
	; 8 bytes
	tax
	pla
	lda #32
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 8214)
	; 7 bytes
	tax
	lda #32
	jsr delay_256a_16_clocks
	txa
.elseif(n = 8215 || n = 8220)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8217)
	; 7 bytes
	pha
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8218)
	; 8 bytes
	tax
	pla
	lda #32
	jsr delay_256a_16_clocks
	txa
.elseif(n = 8222 || n = 8225 || n = 8227 || n = 8230 || n = 8237)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8223)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8224)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8228 || n = 8231)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8229)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8232)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8233)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8235 || n = 8238 || n = 8240 || n = 8243)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8236)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8239 || n = 8241 || n = 8242)
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
.elseif(n = 8418 || n = 8429)
	; 13..14 bytes
	delay_n (n-8413)
	lda #172
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8419 || n = 8430 || n = 8432)
	; 13..15 bytes
	delay_n (n-8414)
	lda #173
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8420 || n = 8433)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 8420 && n < 8425) || (n > 8434 && n < 8438))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8425)
	; 14 bytes
	delay_n (n-8409)
	lda #168
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8426)
	; 14 bytes
	delay_n (n-8410)
	lda #169
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8427)
	; 14 bytes
	delay_n (n-8411)
	lda #170
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8428)
	; 14 bytes
	delay_n (n-8412)
	lda #171
	jsr delay_a_25_clocks
	lda #32
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
.elseif(n = 8434)
	; 16 bytes
	delay_n (n-8429)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	delay_n (n-8409)
	lda #168
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8439)
	; 16 bytes
	delay_n (n-8410)
	lda #169
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8440)
	; 16 bytes
	delay_n (n-8411)
	lda #170
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8441)
	; 16 bytes
	delay_n (n-8412)
	lda #171
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8442)
	; 16 bytes
	delay_n (n-8413)
	lda #172
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8443)
	; 16 bytes
	delay_n (n-8414)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 8467 || n = 8472 || n = 8475)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8468)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8469)
	; 8 bytes
	tax
	pla
	lda #33
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 8470)
	; 7 bytes
	tax
	lda #33
	jsr delay_256a_16_clocks
	txa
.elseif(n = 8471 || n = 8476)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8473)
	; 7 bytes
	pha
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8474)
	; 8 bytes
	tax
	pla
	lda #33
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8482 || n = 8490)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478 || n = 8481 || n = 8483 || n = 8486 || n = 8493)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8479)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8480)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8484 || n = 8487)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8485)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8488)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8489)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8491 || n = 8494 || n = 8496 || n = 8499)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8492)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 8495 || n = 8497 || n = 8498)
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
; 8672 to 8710
.if(n = 8672 || n = 8673)
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8674 || n = 8685 || n = 8687 || n = 8698)
	; 13..16 bytes
	delay_n (n-8669)
	lda #172
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8675 || n = 8686 || n = 8688 || n = 8699)
	; 13..16 bytes
	delay_n (n-8670)
	lda #173
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8676 || n = 8689)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 8676 && n < 8681) || (n > 8690 && n < 8694))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8681 || n = 8694)
	; 14..16 bytes
	delay_n (n-8665)
	lda #168
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8682 || n = 8695)
	; 14..16 bytes
	delay_n (n-8666)
	lda #169
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8683 || n = 8696)
	; 14..16 bytes
	delay_n (n-8667)
	lda #170
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8684 || n = 8697)
	; 14..16 bytes
	delay_n (n-8668)
	lda #171
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8690)
	; 16 bytes
	delay_n (n-8685)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 8723 || n = 8728)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8724)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8725)
	; 8 bytes
	tax
	pla
	lda #34
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 8726)
	; 7 bytes
	tax
	lda #34
	jsr delay_256a_16_clocks
	txa
.elseif(n = 8727)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8729)
	; 7 bytes
	pha
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8730)
	; 8 bytes
	tax
	pla
	lda #34
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 8731 to 8750
.if(n = 8731 || n = 8733 || n = 8738 || n = 8746)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8732)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8734 || n = 8737 || n = 8739 || n = 8742 || n = 8749)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8735)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8736)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8740 || n = 8743)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8741)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8744)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8745)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8747 || n = 8750)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8748)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.endif
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
.if(n = 8751 || n = 8753 || n = 8754)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8752 || n = 8755)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 8930 || n = 8941)
	; 13..14 bytes
	delay_n (n-8925)
	lda #172
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8931 || n = 8942 || n = 8944)
	; 13..15 bytes
	delay_n (n-8926)
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8932 || n = 8945)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 8932 && n < 8937))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8937)
	; 14 bytes
	delay_n (n-8921)
	lda #168
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8938)
	; 14 bytes
	delay_n (n-8922)
	lda #169
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8939)
	; 14 bytes
	delay_n (n-8923)
	lda #170
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8940)
	; 14 bytes
	delay_n (n-8924)
	lda #171
	jsr delay_a_25_clocks
	lda #34
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
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 16 bytes
	delay_n (n-8941)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif((n > 8946 && n < 8950))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8950)
	; 16 bytes
	delay_n (n-8921)
	lda #168
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8951)
	; 16 bytes
	delay_n (n-8922)
	lda #169
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8952)
	; 16 bytes
	delay_n (n-8923)
	lda #170
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8953)
	; 16 bytes
	delay_n (n-8924)
	lda #171
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8954)
	; 16 bytes
	delay_n (n-8925)
	lda #172
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	pla
.elseif(n = 8955)
	; 16 bytes
	delay_n (n-8926)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 8979 || n = 8984)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8980)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 8981)
	; 8 bytes
	tax
	pla
	lda #35
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 8982)
	; 7 bytes
	tax
	lda #35
	jsr delay_256a_16_clocks
	txa
.elseif(n = 8983)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 8986)
	; 8 bytes
	tax
	pla
	lda #35
	jsr delay_256a_16_clocks
	txa
.elseif(n = 8987 || n = 8989 || n = 8994 || n = 9002)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8990 || n = 8993 || n = 8995 || n = 8998 || n = 9005)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8991)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8992)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8996 || n = 8999)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8997)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9000)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9001)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9003 || n = 9006 || n = 9008 || n = 9011)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9004)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9007 || n = 9009 || n = 9010)
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
; 9063 to 9218
.if((n > 9062 && n < 9186))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9186 || n = 9197 || n = 9199 || n = 9210)
	; 13..16 bytes
	delay_n (n-9181)
	lda #172
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9187 || n = 9198 || n = 9200 || n = 9211)
	; 13..16 bytes
	delay_n (n-9182)
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9188 || n = 9201)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 9188 && n < 9193) || (n > 9202 && n < 9206))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9193 || n = 9206)
	; 14..16 bytes
	delay_n (n-9177)
	lda #168
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9194 || n = 9207)
	; 14..16 bytes
	delay_n (n-9178)
	lda #169
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9195 || n = 9208)
	; 14..16 bytes
	delay_n (n-9179)
	lda #170
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9196 || n = 9209)
	; 14..16 bytes
	delay_n (n-9180)
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9202)
	; 16 bytes
	delay_n (n-9197)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 9236)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 9237)
	; 8 bytes
	tax
	pla
	lda #36
	jsr delay_256a_11_clocks_
	txa
.endif
.else
; 9238 to 9257
.if(n = 9238)
	; 7 bytes
	tax
	lda #36
	jsr delay_256a_16_clocks
	txa
.elseif(n = 9239 || n = 9244)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9240 || n = 9243 || n = 9245 || n = 9250)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9241)
	; 7 bytes
	pha
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9242)
	; 8 bytes
	tax
	pla
	lda #36
	jsr delay_256a_16_clocks
	txa
.elseif(n = 9246 || n = 9249 || n = 9251 || n = 9254)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 9247)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9248)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9252 || n = 9255)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9253)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9256)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9257)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9259 || n = 9262 || n = 9264 || n = 9267)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9260)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9261)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9263 || n = 9265 || n = 9266)
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
.elseif(n = 9442 || n = 9453)
	; 13..14 bytes
	delay_n (n-9437)
	lda #172
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9443)
	; 13 bytes
	delay_n (n-9438)
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9444)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9444 && n < 9449))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9449)
	; 14 bytes
	delay_n (n-9433)
	lda #168
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9450)
	; 14 bytes
	delay_n (n-9434)
	lda #169
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9451)
	; 14 bytes
	delay_n (n-9435)
	lda #170
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9452)
	; 14 bytes
	delay_n (n-9436)
	lda #171
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454 || n = 9456 || n = 9467)
	; 14..16 bytes
	delay_n (n-9438)
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9455 || n = 9466)
	; 15..16 bytes
	delay_n (n-9437)
	lda #172
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9457)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9458)
	; 16 bytes
	delay_n (n-9453)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9458 && n < 9462))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9462)
	; 16 bytes
	delay_n (n-9433)
	lda #168
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9463)
	; 16 bytes
	delay_n (n-9434)
	lda #169
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9464)
	; 16 bytes
	delay_n (n-9435)
	lda #170
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9465)
	; 16 bytes
	delay_n (n-9436)
	lda #171
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #37
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 9494)
	; 7 bytes
	tax
	lda #37
	jsr delay_256a_16_clocks
	txa
.elseif(n = 9495 || n = 9500)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9496 || n = 9499 || n = 9501 || n = 9506 || n = 9514)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9497)
	; 7 bytes
	pha
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9498)
	; 8 bytes
	tax
	pla
	lda #37
	jsr delay_256a_16_clocks
	txa
.elseif(n = 9502 || n = 9505 || n = 9507 || n = 9510 || n = 9517)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9503)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9504)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9508 || n = 9511)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9509)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9512)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9513)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9515 || n = 9518 || n = 9520 || n = 9523)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9516)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9519 || n = 9521 || n = 9522)
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
.elseif(n = 9698 || n = 9709 || n = 9711 || n = 9722)
	; 13..16 bytes
	delay_n (n-9693)
	lda #172
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9699 || n = 9710 || n = 9712 || n = 9723)
	; 13..16 bytes
	delay_n (n-9694)
	lda #173
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9700 || n = 9713)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 9700 && n < 9705) || (n > 9714 && n < 9718))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9705 || n = 9718)
	; 14..16 bytes
	delay_n (n-9689)
	lda #168
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9706 || n = 9719)
	; 14..16 bytes
	delay_n (n-9690)
	lda #169
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9707 || n = 9720)
	; 14..16 bytes
	delay_n (n-9691)
	lda #170
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9708 || n = 9721)
	; 14..16 bytes
	delay_n (n-9692)
	lda #171
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9714)
	; 16 bytes
	delay_n (n-9709)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 9747 || n = 9752 || n = 9755 || n = 9757 || n = 9762)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9748)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 9749)
	; 8 bytes
	tax
	pla
	lda #38
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 9750)
	; 7 bytes
	tax
	lda #38
	jsr delay_256a_16_clocks
	txa
.elseif(n = 9751 || n = 9756)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 9753)
	; 7 bytes
	pha
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9754)
	; 8 bytes
	tax
	pla
	lda #38
	jsr delay_256a_16_clocks
	txa
.elseif(n = 9758 || n = 9761 || n = 9763)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 9759)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9760)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9764)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9765)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766 || n = 9773)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9767)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9768)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9769)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9770)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9771 || n = 9774 || n = 9776 || n = 9779)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9772)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 9775 || n = 9777 || n = 9778)
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
.elseif(n = 9954)
	; 13 bytes
	delay_n (n-9949)
	lda #172
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9955)
	; 13 bytes
	delay_n (n-9950)
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9956)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 9956 && n < 9961))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 9961) && (n < 9981))
; 9961 to 9980
.if(n = 9961 || n = 9974)
	; 14..16 bytes
	delay_n (n-9945)
	lda #168
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9962 || n = 9975)
	; 14..16 bytes
	delay_n (n-9946)
	lda #169
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9963 || n = 9976)
	; 14..16 bytes
	delay_n (n-9947)
	lda #170
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9964 || n = 9977)
	; 14..16 bytes
	delay_n (n-9948)
	lda #171
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9965 || n = 9967 || n = 9978)
	; 14..16 bytes
	delay_n (n-9949)
	lda #172
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9966 || n = 9968 || n = 9979)
	; 14..16 bytes
	delay_n (n-9950)
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif(n = 9969)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9970)
	; 16 bytes
	delay_n (n-9965)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	pla
.elseif((n > 9970 && n < 9974))
	; 16 bytes
	php
	delay_n (n-7)
	plp
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 10003 || n = 10008 || n = 10011 || n = 10013 || n = 10018 || n = 10026)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10004)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10005)
	; 8 bytes
	tax
	pla
	lda #39
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 10006)
	; 7 bytes
	tax
	lda #39
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10007 || n = 10012)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10009)
	; 7 bytes
	pha
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10010)
	; 8 bytes
	tax
	pla
	lda #39
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10014 || n = 10017 || n = 10019 || n = 10022 || n = 10029)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10015)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10016)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10020 || n = 10023)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10021)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10024)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10025)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10027 || n = 10030 || n = 10032 || n = 10035)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10028)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10031 || n = 10033 || n = 10034)
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
.elseif(n = 10210 || n = 10221 || n = 10223 || n = 10234)
	; 13..16 bytes
	delay_n (n-10205)
	lda #172
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10211 || n = 10222 || n = 10224)
	; 13..15 bytes
	delay_n (n-10206)
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10212 || n = 10225)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 10212 && n < 10217) || (n > 10226 && n < 10230))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10217 || n = 10230)
	; 14..16 bytes
	delay_n (n-10201)
	lda #168
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10218 || n = 10231)
	; 14..16 bytes
	delay_n (n-10202)
	lda #169
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10219 || n = 10232)
	; 14..16 bytes
	delay_n (n-10203)
	lda #170
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10220 || n = 10233)
	; 14..16 bytes
	delay_n (n-10204)
	lda #171
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10226)
	; 16 bytes
	delay_n (n-10221)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	delay_n (n-10206)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 10259 || n = 10264 || n = 10267 || n = 10269)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 10260)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10261)
	; 8 bytes
	tax
	pla
	lda #40
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 10262)
	; 7 bytes
	tax
	lda #40
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10263 || n = 10268)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10265)
	; 7 bytes
	pha
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10266)
	; 8 bytes
	tax
	pla
	lda #40
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10270 || n = 10273)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 10271)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10272)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10282)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275 || n = 10278 || n = 10285)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10276 || n = 10279)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10277)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10280)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10281)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10283 || n = 10286 || n = 10288 || n = 10291)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10284)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10287 || n = 10289 || n = 10290)
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
.elseif(n = 10466)
	; 13 bytes
	delay_n (n-10461)
	lda #172
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10467)
	; 13 bytes
	delay_n (n-10462)
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10468)
	; 13 bytes
	pha
	delay_n (n-3)
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
; 10469 to 10507
.if((n > 10468 && n < 10473) || (n > 10482 && n < 10486))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10473 || n = 10486)
	; 14..16 bytes
	delay_n (n-10457)
	lda #168
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10474 || n = 10487)
	; 14..16 bytes
	delay_n (n-10458)
	lda #169
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10475 || n = 10488)
	; 14..16 bytes
	delay_n (n-10459)
	lda #170
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10476 || n = 10489)
	; 14..16 bytes
	delay_n (n-10460)
	lda #171
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10477 || n = 10479 || n = 10490)
	; 14..16 bytes
	delay_n (n-10461)
	lda #172
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10478 || n = 10480 || n = 10491)
	; 14..16 bytes
	delay_n (n-10462)
	lda #173
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10481)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10482)
	; 16 bytes
	delay_n (n-10477)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 10515 || n = 10520 || n = 10523 || n = 10525 || n = 10530 || n = 10538)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10516)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10517)
	; 8 bytes
	tax
	pla
	lda #41
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 10518)
	; 7 bytes
	tax
	lda #41
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10519 || n = 10524)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10521)
	; 7 bytes
	pha
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10522)
	; 8 bytes
	tax
	pla
	lda #41
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10526 || n = 10529 || n = 10531 || n = 10534 || n = 10541)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10527)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10528)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10532 || n = 10535)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10533)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10536)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10537)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10539 || n = 10542 || n = 10544)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10540)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10543 || n = 10545 || n = 10546)
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
	; 11 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 10722 || n = 10733 || n = 10735)
	; 13..15 bytes
	delay_n (n-10717)
	lda #172
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10723 || n = 10734 || n = 10736)
	; 13..15 bytes
	delay_n (n-10718)
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10724 || n = 10737)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 10724 && n < 10729) || (n > 10738 && n < 10742))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10729 || n = 10742)
	; 14..16 bytes
	delay_n (n-10713)
	lda #168
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10730)
	; 14 bytes
	delay_n (n-10714)
	lda #169
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10731)
	; 14 bytes
	delay_n (n-10715)
	lda #170
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10732)
	; 14 bytes
	delay_n (n-10716)
	lda #171
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10738)
	; 16 bytes
	delay_n (n-10733)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 16 bytes
	delay_n (n-10714)
	lda #169
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10744)
	; 16 bytes
	delay_n (n-10715)
	lda #170
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10745)
	; 16 bytes
	delay_n (n-10716)
	lda #171
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10746)
	; 16 bytes
	delay_n (n-10717)
	lda #172
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10747)
	; 16 bytes
	delay_n (n-10718)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 10771 || n = 10776 || n = 10779 || n = 10781)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 10772)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 10773)
	; 8 bytes
	tax
	pla
	lda #42
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 10774)
	; 7 bytes
	tax
	lda #42
	jsr delay_256a_16_clocks
	txa
.elseif(n = 10775 || n = 10780)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10777)
	; 7 bytes
	pha
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10778)
	; 8 bytes
	tax
	pla
	lda #42
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782 || n = 10785 || n = 10787 || n = 10790 || n = 10797)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10783)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10784)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10786 || n = 10794)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 10788 || n = 10791)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10789)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10792)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10793)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10795 || n = 10798 || n = 10800 || n = 10803)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10796)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 10799 || n = 10801 || n = 10802)
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
; 10938 to 11015
.if((n > 10937 && n < 10978))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10978 || n = 10989 || n = 11002)
	; 13..16 bytes
	delay_n (n-10973)
	lda #172
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10979 || n = 10990 || n = 10992 || n = 11003)
	; 13..16 bytes
	delay_n (n-10974)
	lda #173
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10980 || n = 10993)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 10980 && n < 10985) || (n > 10994 && n < 10998))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10985 || n = 10998)
	; 14..16 bytes
	delay_n (n-10969)
	lda #168
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10986 || n = 10999)
	; 14..16 bytes
	delay_n (n-10970)
	lda #169
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10987 || n = 11000)
	; 14..16 bytes
	delay_n (n-10971)
	lda #170
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	pla
.elseif(n = 10988 || n = 11001)
	; 14..16 bytes
	delay_n (n-10972)
	lda #171
	jsr delay_a_25_clocks
	lda #42
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
.elseif(n = 10994)
	; 16 bytes
	delay_n (n-10989)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 11027 || n = 11032)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11028)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11029)
	; 8 bytes
	tax
	pla
	lda #43
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 11030)
	; 7 bytes
	tax
	lda #43
	jsr delay_256a_16_clocks
	txa
.elseif(n = 11031)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11033)
	; 7 bytes
	pha
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11034)
	; 8 bytes
	tax
	pla
	lda #43
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 11035 to 11054
.if(n = 11035 || n = 11037 || n = 11042 || n = 11050)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11036)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11038 || n = 11041 || n = 11043 || n = 11046 || n = 11053)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11039)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11040)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11044 || n = 11047)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11045)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11048)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11049)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11051 || n = 11054)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11052)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
; 11055 to 11093
.if(n = 11055 || n = 11057 || n = 11058)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11056 || n = 11059)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 11234 || n = 11245 || n = 11247)
	; 13..15 bytes
	delay_n (n-11229)
	lda #172
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11235 || n = 11246 || n = 11248)
	; 13..15 bytes
	delay_n (n-11230)
	lda #173
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11236 || n = 11249)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 11236 && n < 11241))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11241)
	; 14 bytes
	delay_n (n-11225)
	lda #168
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11242)
	; 14 bytes
	delay_n (n-11226)
	lda #169
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11243)
	; 14 bytes
	delay_n (n-11227)
	lda #170
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11244)
	; 14 bytes
	delay_n (n-11228)
	lda #171
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11250)
	; 16 bytes
	delay_n (n-11245)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #43
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
.if((n > 11250 && n < 11254))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11254)
	; 16 bytes
	delay_n (n-11225)
	lda #168
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11255)
	; 16 bytes
	delay_n (n-11226)
	lda #169
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11256)
	; 16 bytes
	delay_n (n-11227)
	lda #170
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11257)
	; 16 bytes
	delay_n (n-11228)
	lda #171
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11258)
	; 16 bytes
	delay_n (n-11229)
	lda #172
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11259)
	; 16 bytes
	delay_n (n-11230)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 11283 || n = 11288)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11284)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11285)
	; 8 bytes
	tax
	pla
	lda #44
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 11286)
	; 7 bytes
	tax
	lda #44
	jsr delay_256a_16_clocks
	txa
.elseif(n = 11287)
	; 8 bytes
	pha
	delay_n (n-3)
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
.if(n = 11290)
	; 8 bytes
	tax
	pla
	lda #44
	jsr delay_256a_16_clocks
	txa
.elseif(n = 11291 || n = 11293 || n = 11298 || n = 11306)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11294 || n = 11297 || n = 11299 || n = 11302 || n = 11309)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11295)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11296)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11300 || n = 11303)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11301)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11304)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11305)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11307 || n = 11310 || n = 11312 || n = 11315)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11308)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11311 || n = 11313 || n = 11314)
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
; 11485 to 11523
.if((n > 11484 && n < 11490))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11490 || n = 11501 || n = 11503 || n = 11514)
	; 13..16 bytes
	delay_n (n-11485)
	lda #172
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11491 || n = 11502 || n = 11504 || n = 11515)
	; 13..16 bytes
	delay_n (n-11486)
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11492 || n = 11505)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 11492 && n < 11497) || (n > 11506 && n < 11510))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11497 || n = 11510)
	; 14..16 bytes
	delay_n (n-11481)
	lda #168
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11498 || n = 11511)
	; 14..16 bytes
	delay_n (n-11482)
	lda #169
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11499 || n = 11512)
	; 14..16 bytes
	delay_n (n-11483)
	lda #170
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11500 || n = 11513)
	; 14..16 bytes
	delay_n (n-11484)
	lda #171
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11506)
	; 16 bytes
	delay_n (n-11501)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11540)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 11541)
	; 8 bytes
	tax
	pla
	lda #45
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 11542)
	; 7 bytes
	tax
	lda #45
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 11543 to 11562
.if(n = 11543 || n = 11548)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11544 || n = 11547 || n = 11549 || n = 11554 || n = 11562)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11545)
	; 7 bytes
	pha
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11546)
	; 8 bytes
	tax
	pla
	lda #45
	jsr delay_256a_16_clocks
	txa
.elseif(n = 11550 || n = 11553 || n = 11555 || n = 11558)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 11551)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11552)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11556 || n = 11559)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11557)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11560)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11561)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563 || n = 11566 || n = 11568 || n = 11571)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11564)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11565)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11567 || n = 11569 || n = 11570)
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
.elseif(n = 11746 || n = 11757)
	; 13..14 bytes
	delay_n (n-11741)
	lda #172
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11747)
	; 13 bytes
	delay_n (n-11742)
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11748)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 11748 && n < 11753))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11753)
	; 14 bytes
	delay_n (n-11737)
	lda #168
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11754)
	; 14 bytes
	delay_n (n-11738)
	lda #169
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11755)
	; 14 bytes
	delay_n (n-11739)
	lda #170
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11756)
	; 14 bytes
	delay_n (n-11740)
	lda #171
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758 || n = 11760 || n = 11771)
	; 14..16 bytes
	delay_n (n-11742)
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11759 || n = 11770)
	; 15..16 bytes
	delay_n (n-11741)
	lda #172
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11761)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11762)
	; 16 bytes
	delay_n (n-11757)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif((n > 11762 && n < 11766))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11766)
	; 16 bytes
	delay_n (n-11737)
	lda #168
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11767)
	; 16 bytes
	delay_n (n-11738)
	lda #169
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11768)
	; 16 bytes
	delay_n (n-11739)
	lda #170
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11769)
	; 16 bytes
	delay_n (n-11740)
	lda #171
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #46
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 11798)
	; 7 bytes
	tax
	lda #46
	jsr delay_256a_16_clocks
	txa
.elseif(n = 11799 || n = 11804)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11800 || n = 11803 || n = 11805 || n = 11810 || n = 11818)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 11801)
	; 7 bytes
	pha
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 11802)
	; 8 bytes
	tax
	pla
	lda #46
	jsr delay_256a_16_clocks
	txa
.elseif(n = 11806 || n = 11809 || n = 11811 || n = 11814 || n = 11821)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11807)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11808)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11812 || n = 11815)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11813)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11816)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11817)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11819 || n = 11822 || n = 11824 || n = 11827)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11820)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 11823 || n = 11825 || n = 11826)
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
.elseif(n = 12002 || n = 12013 || n = 12026)
	; 13..16 bytes
	delay_n (n-11997)
	lda #172
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12003 || n = 12014 || n = 12016 || n = 12027)
	; 13..16 bytes
	delay_n (n-11998)
	lda #173
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12004 || n = 12017)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 12004 && n < 12009) || (n > 12018 && n < 12022))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12009 || n = 12022)
	; 14..16 bytes
	delay_n (n-11993)
	lda #168
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12010 || n = 12023)
	; 14..16 bytes
	delay_n (n-11994)
	lda #169
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12011 || n = 12024)
	; 14..16 bytes
	delay_n (n-11995)
	lda #170
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12012 || n = 12025)
	; 14..16 bytes
	delay_n (n-11996)
	lda #171
	jsr delay_a_25_clocks
	lda #46
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
.elseif(n = 12018)
	; 16 bytes
	delay_n (n-12013)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.if(n = 12051 || n = 12056 || n = 12059 || n = 12061 || n = 12066)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12052)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12053)
	; 8 bytes
	tax
	pla
	lda #47
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 12054)
	; 7 bytes
	tax
	lda #47
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12055 || n = 12060)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12057)
	; 7 bytes
	pha
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12058)
	; 8 bytes
	tax
	pla
	lda #47
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12062 || n = 12065 || n = 12067 || n = 12070)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 12063)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12064)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12068)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12069)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12072)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12073)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12074)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12075 || n = 12078 || n = 12080 || n = 12083)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12076)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12077)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12079 || n = 12081 || n = 12082)
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
.elseif(n = 12258)
	; 13 bytes
	delay_n (n-12253)
	lda #172
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12259)
	; 13 bytes
	delay_n (n-12254)
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12260)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 12260 && n < 12265))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12265)
	; 14 bytes
	delay_n (n-12249)
	lda #168
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 12266) && (n < 12305))
.if((n >= 12266) && (n < 12285))
; 12266 to 12284
.if(n = 12266 || n = 12279)
	; 14..16 bytes
	delay_n (n-12250)
	lda #169
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12267 || n = 12280)
	; 14..16 bytes
	delay_n (n-12251)
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12268 || n = 12281)
	; 14..16 bytes
	delay_n (n-12252)
	lda #171
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12269 || n = 12271 || n = 12282)
	; 14..16 bytes
	delay_n (n-12253)
	lda #172
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12270 || n = 12272 || n = 12283)
	; 14..16 bytes
	delay_n (n-12254)
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12273)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12274)
	; 16 bytes
	delay_n (n-12269)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
.elseif((n > 12274 && n < 12278))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12278)
	; 16 bytes
	delay_n (n-12249)
	lda #168
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 12307 || n = 12312 || n = 12315 || n = 12317 || n = 12322 || n = 12330)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12308)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12309)
	; 8 bytes
	tax
	pla
	lda #48
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 12310)
	; 7 bytes
	tax
	lda #48
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12311 || n = 12316)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12313)
	; 7 bytes
	pha
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12314)
	; 8 bytes
	tax
	pla
	lda #48
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12318 || n = 12321 || n = 12323 || n = 12326 || n = 12333)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12319)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12320)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12324 || n = 12327)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12325)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12328)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12329)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12331 || n = 12334 || n = 12336 || n = 12339)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12332)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12335 || n = 12337 || n = 12338)
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
.elseif(n = 12514 || n = 12525 || n = 12538)
	; 13..16 bytes
	delay_n (n-12509)
	lda #172
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12515 || n = 12526 || n = 12528 || n = 12539)
	; 13..16 bytes
	delay_n (n-12510)
	lda #173
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12516 || n = 12529)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 12516 && n < 12521) || (n > 12530 && n < 12534))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12521 || n = 12534)
	; 14..16 bytes
	delay_n (n-12505)
	lda #168
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12522 || n = 12535)
	; 14..16 bytes
	delay_n (n-12506)
	lda #169
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12523 || n = 12536)
	; 14..16 bytes
	delay_n (n-12507)
	lda #170
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12524 || n = 12537)
	; 14..16 bytes
	delay_n (n-12508)
	lda #171
	jsr delay_a_25_clocks
	lda #48
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
.elseif(n = 12530)
	; 16 bytes
	delay_n (n-12525)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 12563 || n = 12568 || n = 12571 || n = 12573 || n = 12578)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12564)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12565)
	; 8 bytes
	tax
	pla
	lda #49
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 12566)
	; 7 bytes
	tax
	lda #49
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12567 || n = 12572)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12569)
	; 7 bytes
	pha
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12570)
	; 8 bytes
	tax
	pla
	lda #49
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12574 || n = 12577)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 12575)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12576)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579 || n = 12582 || n = 12589)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12580 || n = 12583)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12581)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12584)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12585)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12586)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12587 || n = 12590 || n = 12592 || n = 12595)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12588)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12591 || n = 12593 || n = 12594)
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
.elseif(n = 12770)
	; 13 bytes
	delay_n (n-12765)
	lda #172
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12771)
	; 13 bytes
	delay_n (n-12766)
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12772)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 12773)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 12774) && (n < 12793))
; 12774 to 12792
.if((n > 12773 && n < 12777) || (n > 12786 && n < 12790))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12777 || n = 12790)
	; 14..16 bytes
	delay_n (n-12761)
	lda #168
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12778 || n = 12791)
	; 14..16 bytes
	delay_n (n-12762)
	lda #169
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12779 || n = 12792)
	; 14..16 bytes
	delay_n (n-12763)
	lda #170
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12780)
	; 14 bytes
	delay_n (n-12764)
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12781 || n = 12783)
	; 14..15 bytes
	delay_n (n-12765)
	lda #172
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12782 || n = 12784)
	; 14..15 bytes
	delay_n (n-12766)
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12785)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12786)
	; 16 bytes
	delay_n (n-12781)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 16 bytes
	delay_n (n-12764)
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12794)
	; 16 bytes
	delay_n (n-12765)
	lda #172
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12795)
	; 16 bytes
	delay_n (n-12766)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 12819 || n = 12824 || n = 12827 || n = 12829 || n = 12834 || n = 12842)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12820)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 12821)
	; 8 bytes
	tax
	pla
	lda #50
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 12822)
	; 7 bytes
	tax
	lda #50
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12823 || n = 12828)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12825)
	; 7 bytes
	pha
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 12826)
	; 8 bytes
	tax
	pla
	lda #50
	jsr delay_256a_16_clocks
	txa
.elseif(n = 12830 || n = 12833 || n = 12835 || n = 12838 || n = 12845)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12831)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12832)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12836 || n = 12839)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12837)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12840)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12841)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12843 || n = 12846 || n = 12848 || n = 12851)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12844)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 12847 || n = 12849 || n = 12850)
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
.elseif(n = 13026 || n = 13037)
	; 13..14 bytes
	delay_n (n-13021)
	lda #172
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13027 || n = 13038 || n = 13040)
	; 13..15 bytes
	delay_n (n-13022)
	lda #173
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13028 || n = 13041)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13028 && n < 13033) || (n > 13042 && n < 13046))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13033 || n = 13046)
	; 14..16 bytes
	delay_n (n-13017)
	lda #168
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13034)
	; 14 bytes
	delay_n (n-13018)
	lda #169
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13035)
	; 14 bytes
	delay_n (n-13019)
	lda #170
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13036)
	; 14 bytes
	delay_n (n-13020)
	lda #171
	jsr delay_a_25_clocks
	lda #50
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
.elseif(n = 13042)
	; 16 bytes
	delay_n (n-13037)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	delay_n (n-13018)
	lda #169
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13048)
	; 16 bytes
	delay_n (n-13019)
	lda #170
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13049)
	; 16 bytes
	delay_n (n-13020)
	lda #171
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13050)
	; 16 bytes
	delay_n (n-13021)
	lda #172
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13051)
	; 16 bytes
	delay_n (n-13022)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 13075 || n = 13080 || n = 13083 || n = 13085)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 13076)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13077)
	; 8 bytes
	tax
	pla
	lda #51
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 13078)
	; 7 bytes
	tax
	lda #51
	jsr delay_256a_16_clocks
	txa
.elseif(n = 13079 || n = 13084)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13081)
	; 7 bytes
	pha
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13082)
	; 8 bytes
	tax
	pla
	lda #51
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086 || n = 13089 || n = 13091 || n = 13094 || n = 13101)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13087)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13088)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13090 || n = 13098)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13092 || n = 13095)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13093)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13096)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13097)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13099 || n = 13102 || n = 13104 || n = 13107)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13100)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13103 || n = 13105 || n = 13106)
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
; 13282 to 13320
.if(n = 13282 || n = 13293 || n = 13306)
	; 13..16 bytes
	delay_n (n-13277)
	lda #172
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13283 || n = 13294 || n = 13296 || n = 13307)
	; 13..16 bytes
	delay_n (n-13278)
	lda #173
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13284 || n = 13297)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13284 && n < 13289) || (n > 13298 && n < 13302))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13289 || n = 13302)
	; 14..16 bytes
	delay_n (n-13273)
	lda #168
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13290 || n = 13303)
	; 14..16 bytes
	delay_n (n-13274)
	lda #169
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13291 || n = 13304)
	; 14..16 bytes
	delay_n (n-13275)
	lda #170
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13292 || n = 13305)
	; 14..16 bytes
	delay_n (n-13276)
	lda #171
	jsr delay_a_25_clocks
	lda #51
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
.elseif(n = 13298)
	; 16 bytes
	delay_n (n-13293)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 13331 || n = 13336 || n = 13339)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13332)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13333)
	; 8 bytes
	tax
	pla
	lda #52
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 13334)
	; 7 bytes
	tax
	lda #52
	jsr delay_256a_16_clocks
	txa
.elseif(n = 13335)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13337)
	; 7 bytes
	pha
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13338)
	; 8 bytes
	tax
	pla
	lda #52
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 13340 to 13359
.if(n = 13340)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13341 || n = 13346 || n = 13354)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13342 || n = 13345 || n = 13347 || n = 13350 || n = 13357)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13343)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13344)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13348 || n = 13351)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13349)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13352)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13353)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13355 || n = 13358)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13356)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
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
.endif
.else
; 13360 to 13437
.if(n = 13360 || n = 13363)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13361 || n = 13362)
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
.elseif(n = 13538 || n = 13549)
	; 13..14 bytes
	delay_n (n-13533)
	lda #172
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13539 || n = 13550 || n = 13552)
	; 13..15 bytes
	delay_n (n-13534)
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13540 || n = 13553)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13540 && n < 13545))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13545)
	; 14 bytes
	delay_n (n-13529)
	lda #168
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13546)
	; 14 bytes
	delay_n (n-13530)
	lda #169
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13547)
	; 14 bytes
	delay_n (n-13531)
	lda #170
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13548)
	; 14 bytes
	delay_n (n-13532)
	lda #171
	jsr delay_a_25_clocks
	lda #52
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
.elseif(n = 13554)
	; 16 bytes
	delay_n (n-13549)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 13555) && (n < 13574))
; 13555 to 13573
.if((n > 13554 && n < 13558))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13558)
	; 16 bytes
	delay_n (n-13529)
	lda #168
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13559)
	; 16 bytes
	delay_n (n-13530)
	lda #169
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13560)
	; 16 bytes
	delay_n (n-13531)
	lda #170
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13561)
	; 16 bytes
	delay_n (n-13532)
	lda #171
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13562)
	; 16 bytes
	delay_n (n-13533)
	lda #172
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13563)
	; 16 bytes
	delay_n (n-13534)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 13587 || n = 13592)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13588)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13589)
	; 8 bytes
	tax
	pla
	lda #53
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 13590)
	; 7 bytes
	tax
	lda #53
	jsr delay_256a_16_clocks
	txa
.elseif(n = 13591)
	; 8 bytes
	pha
	delay_n (n-3)
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
.if(n = 13594)
	; 8 bytes
	tax
	pla
	lda #53
	jsr delay_256a_16_clocks
	txa
.elseif(n = 13595 || n = 13597 || n = 13602 || n = 13610)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13598 || n = 13601 || n = 13603 || n = 13606 || n = 13613)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13599)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13600)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13604 || n = 13607)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13605)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13608)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13609)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13611 || n = 13614 || n = 13616 || n = 13619)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13612)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13615 || n = 13617 || n = 13618)
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
; 13751 to 13828
.if((n > 13750 && n < 13794))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13794 || n = 13805 || n = 13818)
	; 13..16 bytes
	delay_n (n-13789)
	lda #172
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13795 || n = 13806 || n = 13808 || n = 13819)
	; 13..16 bytes
	delay_n (n-13790)
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13796 || n = 13809)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 13796 && n < 13801) || (n > 13810 && n < 13814))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13801 || n = 13814)
	; 14..16 bytes
	delay_n (n-13785)
	lda #168
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13802 || n = 13815)
	; 14..16 bytes
	delay_n (n-13786)
	lda #169
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13803 || n = 13816)
	; 14..16 bytes
	delay_n (n-13787)
	lda #170
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13804 || n = 13817)
	; 14..16 bytes
	delay_n (n-13788)
	lda #171
	jsr delay_a_25_clocks
	lda #53
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
.elseif(n = 13810)
	; 16 bytes
	delay_n (n-13805)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13844)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 13845)
	; 8 bytes
	tax
	pla
	lda #54
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 13846)
	; 7 bytes
	tax
	lda #54
	jsr delay_256a_16_clocks
	txa
.elseif(n = 13847)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 13848 to 13867
.if(n = 13848 || n = 13851 || n = 13853 || n = 13858 || n = 13866)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 13849)
	; 7 bytes
	pha
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 13850)
	; 8 bytes
	tax
	pla
	lda #54
	jsr delay_256a_16_clocks
	txa
.elseif(n = 13852)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13854 || n = 13857 || n = 13859 || n = 13862)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 13855)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13856)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13860 || n = 13863)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13861)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13864)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13865)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13867)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 13869)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13870 || n = 13872 || n = 13875)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13871 || n = 13873 || n = 13874)
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
.elseif(n = 14050 || n = 14061)
	; 13..14 bytes
	delay_n (n-14045)
	lda #172
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14051 || n = 14062)
	; 13..14 bytes
	delay_n (n-14046)
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14052)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14052 && n < 14057))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14057)
	; 14 bytes
	delay_n (n-14041)
	lda #168
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14058)
	; 14 bytes
	delay_n (n-14042)
	lda #169
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14059)
	; 14 bytes
	delay_n (n-14043)
	lda #170
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14060)
	; 14 bytes
	delay_n (n-14044)
	lda #171
	jsr delay_a_25_clocks
	lda #54
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
.elseif(n = 14064 || n = 14075)
	; 15..16 bytes
	delay_n (n-14046)
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14065)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14066)
	; 16 bytes
	delay_n (n-14061)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14066 && n < 14070))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14070)
	; 16 bytes
	delay_n (n-14041)
	lda #168
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14071)
	; 16 bytes
	delay_n (n-14042)
	lda #169
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14072)
	; 16 bytes
	delay_n (n-14043)
	lda #170
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14073)
	; 16 bytes
	delay_n (n-14044)
	lda #171
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14074)
	; 16 bytes
	delay_n (n-14045)
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 14100)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14101)
	; 8 bytes
	tax
	pla
	lda #55
	jsr delay_256a_11_clocks_
	txa
.endif
.endif
.else
; 14102 to 14140
.if(n = 14102)
	; 7 bytes
	tax
	lda #55
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14103 || n = 14108)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14104 || n = 14107 || n = 14109 || n = 14114 || n = 14122)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14105)
	; 7 bytes
	pha
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14106)
	; 8 bytes
	tax
	pla
	lda #55
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14110 || n = 14113 || n = 14115 || n = 14118 || n = 14125)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14111)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14112)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14116 || n = 14119)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14117)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14120)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14121)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14123 || n = 14126 || n = 14128 || n = 14131)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14124)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14127 || n = 14129 || n = 14130)
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
.elseif(n = 14306 || n = 14317 || n = 14330)
	; 13..16 bytes
	delay_n (n-14301)
	lda #172
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14307 || n = 14318 || n = 14320 || n = 14331)
	; 13..16 bytes
	delay_n (n-14302)
	lda #173
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14308 || n = 14321)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 14308 && n < 14313) || (n > 14322 && n < 14326))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14313 || n = 14326)
	; 14..16 bytes
	delay_n (n-14297)
	lda #168
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14314 || n = 14327)
	; 14..16 bytes
	delay_n (n-14298)
	lda #169
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14315 || n = 14328)
	; 14..16 bytes
	delay_n (n-14299)
	lda #170
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14316 || n = 14329)
	; 14..16 bytes
	delay_n (n-14300)
	lda #171
	jsr delay_a_25_clocks
	lda #55
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
.elseif(n = 14322)
	; 16 bytes
	delay_n (n-14317)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #56
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 14358)
	; 7 bytes
	tax
	lda #56
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14359 || n = 14364)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14360 || n = 14363 || n = 14365 || n = 14370)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14361)
	; 7 bytes
	pha
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14362)
	; 8 bytes
	tax
	pla
	lda #56
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14366 || n = 14369 || n = 14371 || n = 14374)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 14367)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14368)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14372 || n = 14375)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14373)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.if(n = 14376)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14377)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14378)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379 || n = 14382 || n = 14384 || n = 14387)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14380)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14381)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14383 || n = 14385 || n = 14386)
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
.elseif(n = 14562)
	; 13 bytes
	delay_n (n-14557)
	lda #172
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14563)
	; 13 bytes
	delay_n (n-14558)
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14564)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 14564 && n < 14569))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14569)
	; 14 bytes
	delay_n (n-14553)
	lda #168
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14570)
	; 14 bytes
	delay_n (n-14554)
	lda #169
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 14571) && (n < 14590))
; 14571 to 14589
.if(n = 14571 || n = 14584)
	; 14..16 bytes
	delay_n (n-14555)
	lda #170
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14572 || n = 14585)
	; 14..16 bytes
	delay_n (n-14556)
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14573 || n = 14586)
	; 14..16 bytes
	delay_n (n-14557)
	lda #172
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14574 || n = 14576 || n = 14587)
	; 14..16 bytes
	delay_n (n-14558)
	lda #173
	jsr delay_a_25_clocks
	lda #56
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
.elseif(n = 14577)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14578)
	; 16 bytes
	delay_n (n-14573)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif((n > 14578 && n < 14582))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14582)
	; 16 bytes
	delay_n (n-14553)
	lda #168
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14583)
	; 16 bytes
	delay_n (n-14554)
	lda #169
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 14611 || n = 14616 || n = 14619 || n = 14621 || n = 14626 || n = 14634)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14612)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14613)
	; 8 bytes
	tax
	pla
	lda #57
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 14614)
	; 7 bytes
	tax
	lda #57
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14615 || n = 14620)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14617)
	; 7 bytes
	pha
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14618)
	; 8 bytes
	tax
	pla
	lda #57
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14622 || n = 14625 || n = 14627 || n = 14630 || n = 14637)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14623)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14624)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14628 || n = 14631)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14629)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14632)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14633)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14635 || n = 14638 || n = 14640 || n = 14643)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14636)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14639 || n = 14641 || n = 14642)
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
.elseif(n = 14818 || n = 14829 || n = 14842)
	; 13..16 bytes
	delay_n (n-14813)
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14819 || n = 14830 || n = 14832 || n = 14843)
	; 13..16 bytes
	delay_n (n-14814)
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14820 || n = 14833)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 14820 && n < 14825) || (n > 14834 && n < 14838))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14825 || n = 14838)
	; 14..16 bytes
	delay_n (n-14809)
	lda #168
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14826 || n = 14839)
	; 14..16 bytes
	delay_n (n-14810)
	lda #169
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14827 || n = 14840)
	; 14..16 bytes
	delay_n (n-14811)
	lda #170
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14828 || n = 14841)
	; 14..16 bytes
	delay_n (n-14812)
	lda #171
	jsr delay_a_25_clocks
	lda #57
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
.elseif(n = 14834)
	; 16 bytes
	delay_n (n-14829)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 14867 || n = 14872 || n = 14875 || n = 14877 || n = 14882)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14868)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 14869)
	; 8 bytes
	tax
	pla
	lda #58
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 14870)
	; 7 bytes
	tax
	lda #58
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14871 || n = 14876)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14873)
	; 7 bytes
	pha
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 14874)
	; 8 bytes
	tax
	pla
	lda #58
	jsr delay_256a_16_clocks
	txa
.elseif(n = 14878 || n = 14881)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 14879)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14880)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883 || n = 14886 || n = 14893)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14884 || n = 14887)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14885)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14888)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14889)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14890)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14891 || n = 14894 || n = 14896 || n = 14899)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14892)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 14895 || n = 14897 || n = 14898)
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
.elseif(n = 15074)
	; 13 bytes
	delay_n (n-15069)
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15075)
	; 13 bytes
	delay_n (n-15070)
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15076)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 15077 || n = 15078)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 15079) && (n < 15118))
.if((n >= 15079) && (n < 15098))
; 15079 to 15097
.if(n = 15079 || n = 15080 || (n > 15090 && n < 15094))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15081 || n = 15094)
	; 14..16 bytes
	delay_n (n-15065)
	lda #168
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15082 || n = 15095)
	; 14..16 bytes
	delay_n (n-15066)
	lda #169
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15083 || n = 15096)
	; 14..16 bytes
	delay_n (n-15067)
	lda #170
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15084 || n = 15097)
	; 14..16 bytes
	delay_n (n-15068)
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15085)
	; 14 bytes
	delay_n (n-15069)
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15086 || n = 15088)
	; 14..15 bytes
	delay_n (n-15070)
	lda #173
	jsr delay_a_25_clocks
	lda #58
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
.elseif(n = 15089)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15090)
	; 16 bytes
	delay_n (n-15085)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 15098 to 15117
.if(n = 15098)
	; 16 bytes
	delay_n (n-15069)
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15099)
	; 16 bytes
	delay_n (n-15070)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 15123 || n = 15128 || n = 15131 || n = 15133 || n = 15138 || n = 15146)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15124)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15125)
	; 8 bytes
	tax
	pla
	lda #59
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 15126)
	; 7 bytes
	tax
	lda #59
	jsr delay_256a_16_clocks
	txa
.elseif(n = 15127 || n = 15132)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15129)
	; 7 bytes
	pha
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15130)
	; 8 bytes
	tax
	pla
	lda #59
	jsr delay_256a_16_clocks
	txa
.elseif(n = 15134 || n = 15137 || n = 15139 || n = 15142 || n = 15149)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15135)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15136)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15140 || n = 15143)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15141)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15144)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15145)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15147 || n = 15150 || n = 15152 || n = 15155)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15148)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15151 || n = 15153 || n = 15154)
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
.elseif(n = 15330 || n = 15341 || n = 15343)
	; 13..15 bytes
	delay_n (n-15325)
	lda #172
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15331 || n = 15342 || n = 15344)
	; 13..15 bytes
	delay_n (n-15326)
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15332 || n = 15345)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 15332 && n < 15337) || (n > 15346 && n < 15350))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15337 || n = 15350)
	; 14..16 bytes
	delay_n (n-15321)
	lda #168
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15338 || n = 15351)
	; 14..16 bytes
	delay_n (n-15322)
	lda #169
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15339)
	; 14 bytes
	delay_n (n-15323)
	lda #170
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15340)
	; 14 bytes
	delay_n (n-15324)
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15346)
	; 16 bytes
	delay_n (n-15341)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 16 bytes
	delay_n (n-15323)
	lda #170
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15353)
	; 16 bytes
	delay_n (n-15324)
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15354)
	; 16 bytes
	delay_n (n-15325)
	lda #172
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15355)
	; 16 bytes
	delay_n (n-15326)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 15379 || n = 15384 || n = 15387 || n = 15389)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 15380)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15381)
	; 8 bytes
	tax
	pla
	lda #60
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 15382)
	; 7 bytes
	tax
	lda #60
	jsr delay_256a_16_clocks
	txa
.elseif(n = 15383 || n = 15388)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15385)
	; 7 bytes
	pha
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15386)
	; 8 bytes
	tax
	pla
	lda #60
	jsr delay_256a_16_clocks
	txa
.elseif(n = 15390)
	; 9 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15392)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15393 || n = 15395 || n = 15398 || n = 15405)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15394 || n = 15402)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15396 || n = 15399)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15397)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15400)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15401)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15403 || n = 15406 || n = 15408 || n = 15411)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15404)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15407 || n = 15409 || n = 15410)
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
; 15469 to 15625
.if((n > 15468 && n < 15586))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15586 || n = 15597 || n = 15599 || n = 15610)
	; 13..16 bytes
	delay_n (n-15581)
	lda #172
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15587 || n = 15598 || n = 15600 || n = 15611)
	; 13..16 bytes
	delay_n (n-15582)
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15588 || n = 15601)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 15588 && n < 15593) || (n > 15602 && n < 15606))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15593 || n = 15606)
	; 14..16 bytes
	delay_n (n-15577)
	lda #168
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15594 || n = 15607)
	; 14..16 bytes
	delay_n (n-15578)
	lda #169
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15595 || n = 15608)
	; 14..16 bytes
	delay_n (n-15579)
	lda #170
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15596 || n = 15609)
	; 14..16 bytes
	delay_n (n-15580)
	lda #171
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15602)
	; 16 bytes
	delay_n (n-15597)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 15626) && (n < 15938))
.if((n >= 15626) && (n < 15782))
.if((n >= 15626) && (n < 15704))
.if((n >= 15626) && (n < 15665))
.if((n >= 15626) && (n < 15645))
; 15626 to 15644
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 15635 || n = 15640 || n = 15643)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15636)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15637)
	; 8 bytes
	tax
	pla
	lda #61
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 15638)
	; 7 bytes
	tax
	lda #61
	jsr delay_256a_16_clocks
	txa
.elseif(n = 15639 || n = 15644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15641)
	; 7 bytes
	pha
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15642)
	; 8 bytes
	tax
	pla
	lda #61
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 15645 to 15664
.if(n = 15645 || n = 15650 || n = 15658)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15646 || n = 15649 || n = 15651 || n = 15654 || n = 15661)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15647)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15648)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15652 || n = 15655)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15653)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15656)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15657)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15659 || n = 15662 || n = 15664)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15660)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15663)
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
; 15665 to 15703
.if(n = 15665 || n = 15666)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15667)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 15667 && n < 15704))
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
; 15704 to 15781
.if((n > 15703 && n < 15782))
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
.elseif(n = 15842 || n = 15853 || n = 15855)
	; 13..15 bytes
	delay_n (n-15837)
	lda #172
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15843 || n = 15854 || n = 15856)
	; 13..15 bytes
	delay_n (n-15838)
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15844 || n = 15857)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 15844 && n < 15849) || n = 15859)
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15849)
	; 14 bytes
	delay_n (n-15833)
	lda #168
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15850)
	; 14 bytes
	delay_n (n-15834)
	lda #169
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15851)
	; 14 bytes
	delay_n (n-15835)
	lda #170
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15852)
	; 14 bytes
	delay_n (n-15836)
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15858)
	; 16 bytes
	delay_n (n-15853)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 15860) && (n < 15899))
.if((n >= 15860) && (n < 15879))
; 15860 to 15878
.if(n = 15860 || n = 15861)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15862)
	; 16 bytes
	delay_n (n-15833)
	lda #168
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15863)
	; 16 bytes
	delay_n (n-15834)
	lda #169
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15864)
	; 16 bytes
	delay_n (n-15835)
	lda #170
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15865)
	; 16 bytes
	delay_n (n-15836)
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15866)
	; 16 bytes
	delay_n (n-15837)
	lda #172
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15867)
	; 16 bytes
	delay_n (n-15838)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 15891 || n = 15896)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15892)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 15893)
	; 8 bytes
	tax
	pla
	lda #62
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 15894)
	; 7 bytes
	tax
	lda #62
	jsr delay_256a_16_clocks
	txa
.elseif(n = 15895)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15897)
	; 7 bytes
	pha
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 15898)
	; 8 bytes
	tax
	pla
	lda #62
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15901 || n = 15906 || n = 15914)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15902 || n = 15905 || n = 15907 || n = 15910 || n = 15917)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15903)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15904)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15908 || n = 15911)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15909)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15912)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15913)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15915 || n = 15918 || n = 15920 || n = 15923)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15916)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 15919 || n = 15921 || n = 15922)
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
; 16094 to 16132
.if((n > 16093 && n < 16098))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16098 || n = 16109 || n = 16122)
	; 13..16 bytes
	delay_n (n-16093)
	lda #172
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16099 || n = 16110 || n = 16112 || n = 16123)
	; 13..16 bytes
	delay_n (n-16094)
	lda #173
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16100 || n = 16113)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 16100 && n < 16105) || (n > 16114 && n < 16118))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16105 || n = 16118)
	; 14..16 bytes
	delay_n (n-16089)
	lda #168
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16106 || n = 16119)
	; 14..16 bytes
	delay_n (n-16090)
	lda #169
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16107 || n = 16120)
	; 14..16 bytes
	delay_n (n-16091)
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16108 || n = 16121)
	; 14..16 bytes
	delay_n (n-16092)
	lda #171
	jsr delay_a_25_clocks
	lda #62
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
.elseif(n = 16114)
	; 16 bytes
	delay_n (n-16109)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16148)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16149)
	; 8 bytes
	tax
	pla
	lda #63
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 16150)
	; 7 bytes
	tax
	lda #63
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16151)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 16152 to 16171
.if(n = 16152 || n = 16155 || n = 16157 || n = 16162 || n = 16170)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16153)
	; 7 bytes
	pha
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16154)
	; 8 bytes
	tax
	pla
	lda #63
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16156)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16158 || n = 16161 || n = 16163 || n = 16166)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 16159)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16160)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16164 || n = 16167)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16165)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16168)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16169)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16171)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16173)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16174 || n = 16176 || n = 16179)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16175 || n = 16177 || n = 16178)
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
.elseif(n = 16354 || n = 16365 || n = 16367)
	; 13..15 bytes
	delay_n (n-16349)
	lda #172
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16355 || n = 16366)
	; 13..14 bytes
	delay_n (n-16350)
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16356)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16356 && n < 16361))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16361)
	; 14 bytes
	delay_n (n-16345)
	lda #168
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16362)
	; 14 bytes
	delay_n (n-16346)
	lda #169
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16363)
	; 14 bytes
	delay_n (n-16347)
	lda #170
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16364)
	; 14 bytes
	delay_n (n-16348)
	lda #171
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368 || n = 16379)
	; 15..16 bytes
	delay_n (n-16350)
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16369)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16370)
	; 16 bytes
	delay_n (n-16365)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16370 && n < 16374))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16374)
	; 16 bytes
	delay_n (n-16345)
	lda #168
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16375)
	; 16 bytes
	delay_n (n-16346)
	lda #169
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16376)
	; 16 bytes
	delay_n (n-16347)
	lda #170
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16377)
	; 16 bytes
	delay_n (n-16348)
	lda #171
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16378)
	; 16 bytes
	delay_n (n-16349)
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 16404)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16405)
	; 8 bytes
	tax
	pla
	lda #64
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 16406)
	; 7 bytes
	tax
	lda #64
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.endif
.endif
.else
; 16407 to 16562
.if(n = 16407 || n = 16412)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16408 || n = 16411 || n = 16413 || n = 16418 || n = 16426)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16409)
	; 7 bytes
	pha
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16410)
	; 8 bytes
	tax
	pla
	lda #64
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16414 || n = 16417 || n = 16419 || n = 16422 || n = 16429)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16415)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16416)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16420 || n = 16423)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16421)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16424)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16425)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16427 || n = 16430 || n = 16432 || n = 16435)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16428)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16431 || n = 16433 || n = 16434)
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
.elseif(n = 16610 || n = 16621 || n = 16623 || n = 16634)
	; 13..16 bytes
	delay_n (n-16605)
	lda #172
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16611 || n = 16622 || n = 16624 || n = 16635)
	; 13..16 bytes
	delay_n (n-16606)
	lda #173
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16612 || n = 16625)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 16612 && n < 16617) || (n > 16626 && n < 16630))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16617 || n = 16630)
	; 14..16 bytes
	delay_n (n-16601)
	lda #168
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16618 || n = 16631)
	; 14..16 bytes
	delay_n (n-16602)
	lda #169
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16619 || n = 16632)
	; 14..16 bytes
	delay_n (n-16603)
	lda #170
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16620 || n = 16633)
	; 14..16 bytes
	delay_n (n-16604)
	lda #171
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16626)
	; 16 bytes
	delay_n (n-16621)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #65
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 16662)
	; 7 bytes
	tax
	lda #65
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16663 || n = 16668)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16664 || n = 16667 || n = 16669 || n = 16674)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16665)
	; 7 bytes
	pha
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16666)
	; 8 bytes
	tax
	pla
	lda #65
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16670 || n = 16673 || n = 16675 || n = 16678)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 16671)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16672)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16676 || n = 16679)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16677)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16681)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16682)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16683 || n = 16686 || n = 16688 || n = 16691)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16684)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16685)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16687 || n = 16689 || n = 16690)
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
.elseif(n = 16866)
	; 13 bytes
	delay_n (n-16861)
	lda #172
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16867)
	; 13 bytes
	delay_n (n-16862)
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16868)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 16868 && n < 16873))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16873)
	; 14 bytes
	delay_n (n-16857)
	lda #168
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16874)
	; 14 bytes
	delay_n (n-16858)
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16875)
	; 14 bytes
	delay_n (n-16859)
	lda #170
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
.if(n = 16876 || n = 16889)
	; 14..16 bytes
	delay_n (n-16860)
	lda #171
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16877 || n = 16890)
	; 14..16 bytes
	delay_n (n-16861)
	lda #172
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16878 || n = 16880 || n = 16891)
	; 14..16 bytes
	delay_n (n-16862)
	lda #173
	jsr delay_a_25_clocks
	lda #65
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
.elseif(n = 16881)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16882)
	; 16 bytes
	delay_n (n-16877)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif((n > 16882 && n < 16886))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16886)
	; 16 bytes
	delay_n (n-16857)
	lda #168
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16887)
	; 16 bytes
	delay_n (n-16858)
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16888)
	; 16 bytes
	delay_n (n-16859)
	lda #170
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.if(n = 16915 || n = 16920 || n = 16923 || n = 16925 || n = 16930 || n = 16938)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16916)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 16917)
	; 8 bytes
	tax
	pla
	lda #66
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 16918)
	; 7 bytes
	tax
	lda #66
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16919 || n = 16924)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 16921)
	; 7 bytes
	pha
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 16922)
	; 8 bytes
	tax
	pla
	lda #66
	jsr delay_256a_16_clocks
	txa
.elseif(n = 16926 || n = 16929 || n = 16931 || n = 16934 || n = 16941)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16927)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16928)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16932 || n = 16935)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16933)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16936)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16937)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16939 || n = 16942 || n = 16944 || n = 16947)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16940)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 16943 || n = 16945 || n = 16946)
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
.elseif(n = 17122 || n = 17133 || n = 17146)
	; 13..16 bytes
	delay_n (n-17117)
	lda #172
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17123 || n = 17134 || n = 17136 || n = 17147)
	; 13..16 bytes
	delay_n (n-17118)
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17124 || n = 17137)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 17124 && n < 17129) || (n > 17138 && n < 17142))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17129 || n = 17142)
	; 14..16 bytes
	delay_n (n-17113)
	lda #168
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17130 || n = 17143)
	; 14..16 bytes
	delay_n (n-17114)
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17131 || n = 17144)
	; 14..16 bytes
	delay_n (n-17115)
	lda #170
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17132 || n = 17145)
	; 14..16 bytes
	delay_n (n-17116)
	lda #171
	jsr delay_a_25_clocks
	lda #66
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
.elseif(n = 17138)
	; 16 bytes
	delay_n (n-17133)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 17171 || n = 17176 || n = 17179 || n = 17181 || n = 17186)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17172)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17173)
	; 8 bytes
	tax
	pla
	lda #67
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 17174)
	; 7 bytes
	tax
	lda #67
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17175 || n = 17180)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17177)
	; 7 bytes
	pha
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17178)
	; 8 bytes
	tax
	pla
	lda #67
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17182 || n = 17185 || n = 17187)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 17183)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17184)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17191)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17189)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17190 || n = 17197)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17192)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17193)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17194)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17195 || n = 17198 || n = 17200 || n = 17203)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17196)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17199 || n = 17201 || n = 17202)
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
.elseif(n = 17378)
	; 13 bytes
	delay_n (n-17373)
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17379)
	; 13 bytes
	delay_n (n-17374)
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17380)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 17381 || n = 17382)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 17383) && (n < 17402))
; 17383 to 17401
.if(n = 17383 || n = 17384 || (n > 17394 && n < 17398))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17385 || n = 17398)
	; 14..16 bytes
	delay_n (n-17369)
	lda #168
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17386 || n = 17399)
	; 14..16 bytes
	delay_n (n-17370)
	lda #169
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17387 || n = 17400)
	; 14..16 bytes
	delay_n (n-17371)
	lda #170
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17388 || n = 17401)
	; 14..16 bytes
	delay_n (n-17372)
	lda #171
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17389 || n = 17391)
	; 14..15 bytes
	delay_n (n-17373)
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17390 || n = 17392)
	; 14..15 bytes
	delay_n (n-17374)
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17393)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17394)
	; 16 bytes
	delay_n (n-17389)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 17402 to 17421
.if(n = 17402)
	; 16 bytes
	delay_n (n-17373)
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17403)
	; 16 bytes
	delay_n (n-17374)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 17427 || n = 17432 || n = 17435 || n = 17437 || n = 17442 || n = 17450)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17428)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17429)
	; 8 bytes
	tax
	pla
	lda #68
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 17430)
	; 7 bytes
	tax
	lda #68
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17431 || n = 17436)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17433)
	; 7 bytes
	pha
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17434)
	; 8 bytes
	tax
	pla
	lda #68
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17438 || n = 17441 || n = 17443 || n = 17446 || n = 17453)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17439)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17440)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17444 || n = 17447)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17445)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17448)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17449)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17451 || n = 17454 || n = 17456 || n = 17459)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17452)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17455 || n = 17457 || n = 17458)
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
.elseif(n = 17634 || n = 17645)
	; 13..14 bytes
	delay_n (n-17629)
	lda #172
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17635 || n = 17646 || n = 17648)
	; 13..15 bytes
	delay_n (n-17630)
	lda #173
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17636 || n = 17649)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 17636 && n < 17641) || (n > 17650 && n < 17654))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17641 || n = 17654)
	; 14..16 bytes
	delay_n (n-17625)
	lda #168
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17642 || n = 17655)
	; 14..16 bytes
	delay_n (n-17626)
	lda #169
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17643 || n = 17656)
	; 14..16 bytes
	delay_n (n-17627)
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17644)
	; 14 bytes
	delay_n (n-17628)
	lda #171
	jsr delay_a_25_clocks
	lda #68
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
.elseif(n = 17650)
	; 16 bytes
	delay_n (n-17645)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 16 bytes
	delay_n (n-17628)
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17658)
	; 16 bytes
	delay_n (n-17629)
	lda #172
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17659)
	; 16 bytes
	delay_n (n-17630)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 17683 || n = 17688 || n = 17691 || n = 17693)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 17684)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17685)
	; 8 bytes
	tax
	pla
	lda #69
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 17686)
	; 7 bytes
	tax
	lda #69
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17687 || n = 17692)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17689)
	; 7 bytes
	pha
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17690)
	; 8 bytes
	tax
	pla
	lda #69
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17694)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 17695)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 17696 to 17734
.if(n = 17696)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17697 || n = 17699 || n = 17702 || n = 17709)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17698 || n = 17706)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17700 || n = 17703)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17701)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17704)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17705)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17707 || n = 17710 || n = 17712 || n = 17715)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17708)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17711 || n = 17713 || n = 17714)
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
	delay_n (n-17885)
	lda #172
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 17891) && (n < 17930))
; 17891 to 17929
.if(n = 17891 || n = 17902 || n = 17904 || n = 17915)
	; 13..16 bytes
	delay_n (n-17886)
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17892 || n = 17905)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 17892 && n < 17897) || (n > 17906 && n < 17910))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17897 || n = 17910)
	; 14..16 bytes
	delay_n (n-17881)
	lda #168
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17898 || n = 17911)
	; 14..16 bytes
	delay_n (n-17882)
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17899 || n = 17912)
	; 14..16 bytes
	delay_n (n-17883)
	lda #170
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17900 || n = 17913)
	; 14..16 bytes
	delay_n (n-17884)
	lda #171
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17901 || n = 17903 || n = 17914)
	; 14..16 bytes
	delay_n (n-17885)
	lda #172
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17906)
	; 16 bytes
	delay_n (n-17901)
	jsr @nearby_rts_15cyc
	lda #173
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
.else
.if((n >= 17930) && (n < 17949))
; 17930 to 17948
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 17939 || n = 17944 || n = 17947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 17940)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17941)
	; 8 bytes
	tax
	pla
	lda #70
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 17942)
	; 7 bytes
	tax
	lda #70
	jsr delay_256a_16_clocks
	txa
.elseif(n = 17943 || n = 17948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17945)
	; 7 bytes
	pha
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 17946)
	; 8 bytes
	tax
	pla
	lda #70
	jsr delay_256a_16_clocks
	txa
.endif
.else
; 17949 to 17968
.if(n = 17949 || n = 17954 || n = 17962)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17950 || n = 17953 || n = 17955 || n = 17958 || n = 17965)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17951)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17952)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17956 || n = 17959)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17957)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17960)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17961)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17963 || n = 17966 || n = 17968)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17964)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 17967)
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
; 17969 to 18125
.if(n = 17969 || n = 17970)
	; 12 bytes
	pha
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 17971)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 18146 || n = 18157 || n = 18159)
	; 13..15 bytes
	delay_n (n-18141)
	lda #172
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18147 || n = 18158 || n = 18160)
	; 13..15 bytes
	delay_n (n-18142)
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18148 || n = 18161)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 18148 && n < 18153) || n = 18163 || n = 18164)
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18153)
	; 14 bytes
	delay_n (n-18137)
	lda #168
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18154)
	; 14 bytes
	delay_n (n-18138)
	lda #169
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18155)
	; 14 bytes
	delay_n (n-18139)
	lda #170
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18156)
	; 14 bytes
	delay_n (n-18140)
	lda #171
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18162)
	; 16 bytes
	delay_n (n-18157)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18166)
	; 16 bytes
	delay_n (n-18137)
	lda #168
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18167)
	; 16 bytes
	delay_n (n-18138)
	lda #169
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18168)
	; 16 bytes
	delay_n (n-18139)
	lda #170
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18169)
	; 16 bytes
	delay_n (n-18140)
	lda #171
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18170)
	; 16 bytes
	delay_n (n-18141)
	lda #172
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18171)
	; 16 bytes
	delay_n (n-18142)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 18195 || n = 18200 || n = 18203)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18196)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 18197)
	; 8 bytes
	tax
	pla
	lda #71
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 18198)
	; 7 bytes
	tax
	lda #71
	jsr delay_256a_16_clocks
	txa
.elseif(n = 18199)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18201)
	; 7 bytes
	pha
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18202)
	; 8 bytes
	tax
	pla
	lda #71
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18205 || n = 18210 || n = 18218)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18206 || n = 18209 || n = 18211 || n = 18214 || n = 18221)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18207)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18208)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18212 || n = 18215)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18213)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18216)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18217)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18219 || n = 18222 || n = 18224 || n = 18227)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18220)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18223 || n = 18225 || n = 18226)
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
; 18282 to 18437
.if((n > 18281 && n < 18402))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18402 || n = 18413 || n = 18426)
	; 13..16 bytes
	delay_n (n-18397)
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18403 || n = 18414 || n = 18416 || n = 18427)
	; 13..16 bytes
	delay_n (n-18398)
	lda #173
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18404 || n = 18417)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 18404 && n < 18409) || (n > 18418 && n < 18422))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18409 || n = 18422)
	; 14..16 bytes
	delay_n (n-18393)
	lda #168
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18410 || n = 18423)
	; 14..16 bytes
	delay_n (n-18394)
	lda #169
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18411 || n = 18424)
	; 14..16 bytes
	delay_n (n-18395)
	lda #170
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18412 || n = 18425)
	; 14..16 bytes
	delay_n (n-18396)
	lda #171
	jsr delay_a_25_clocks
	lda #71
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
.elseif(n = 18418)
	; 16 bytes
	delay_n (n-18413)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 18451 || n = 18456)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18452)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 18453)
	; 8 bytes
	tax
	pla
	lda #72
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 18454)
	; 7 bytes
	tax
	lda #72
	jsr delay_256a_16_clocks
	txa
.elseif(n = 18455)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 7 bytes
	pha
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18458)
	; 8 bytes
	tax
	pla
	lda #72
	jsr delay_256a_16_clocks
	txa
.elseif(n = 18459 || n = 18461 || n = 18466 || n = 18474)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18460)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18462 || n = 18465 || n = 18467 || n = 18470)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 18463)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18464)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18468 || n = 18471)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18469)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18472)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18473)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18475)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18476)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18478 || n = 18480 || n = 18483)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18479 || n = 18481 || n = 18482)
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
.elseif(n = 18658 || n = 18669 || n = 18671)
	; 13..15 bytes
	delay_n (n-18653)
	lda #172
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18659 || n = 18670)
	; 13..14 bytes
	delay_n (n-18654)
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18660)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 18660 && n < 18665))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18665)
	; 14 bytes
	delay_n (n-18649)
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18666)
	; 14 bytes
	delay_n (n-18650)
	lda #169
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18667)
	; 14 bytes
	delay_n (n-18651)
	lda #170
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18668)
	; 14 bytes
	delay_n (n-18652)
	lda #171
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672 || n = 18683)
	; 15..16 bytes
	delay_n (n-18654)
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18673)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18674)
	; 16 bytes
	delay_n (n-18669)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif((n > 18674 && n < 18678))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18678)
	; 16 bytes
	delay_n (n-18649)
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18679)
	; 16 bytes
	delay_n (n-18650)
	lda #169
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18680)
	; 16 bytes
	delay_n (n-18651)
	lda #170
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18681)
	; 16 bytes
	delay_n (n-18652)
	lda #171
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18682)
	; 16 bytes
	delay_n (n-18653)
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18708)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 18709)
	; 8 bytes
	tax
	pla
	lda #73
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 18710)
	; 7 bytes
	tax
	lda #73
	jsr delay_256a_16_clocks
	txa
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711 || n = 18716)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18712 || n = 18715 || n = 18717 || n = 18722 || n = 18730)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18713)
	; 7 bytes
	pha
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18714)
	; 8 bytes
	tax
	pla
	lda #73
	jsr delay_256a_16_clocks
	txa
.elseif(n = 18718 || n = 18721 || n = 18723 || n = 18726 || n = 18733)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18719)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18720)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18724 || n = 18727)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18725)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18728)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18729)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18731 || n = 18734 || n = 18736 || n = 18739)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18732)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18735 || n = 18737 || n = 18738)
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
; 18907 to 18945
.if((n > 18906 && n < 18914))
	; 12 bytes
	pha
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18914 || n = 18925 || n = 18938)
	; 13..16 bytes
	delay_n (n-18909)
	lda #172
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18915 || n = 18926 || n = 18928 || n = 18939)
	; 13..16 bytes
	delay_n (n-18910)
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18916 || n = 18929)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 18916 && n < 18921) || (n > 18930 && n < 18934))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18921 || n = 18934)
	; 14..16 bytes
	delay_n (n-18905)
	lda #168
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18922 || n = 18935)
	; 14..16 bytes
	delay_n (n-18906)
	lda #169
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18923 || n = 18936)
	; 14..16 bytes
	delay_n (n-18907)
	lda #170
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18924 || n = 18937)
	; 14..16 bytes
	delay_n (n-18908)
	lda #171
	jsr delay_a_25_clocks
	lda #73
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
.elseif(n = 18930)
	; 16 bytes
	delay_n (n-18925)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
	; 8 bytes
	delay_n (n-2)
	nop
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
	; 8 bytes
	tax
	pla
	lda #74
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 18966)
	; 7 bytes
	tax
	lda #74
	jsr delay_256a_16_clocks
	txa
.elseif(n = 18967 || n = 18972)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18968 || n = 18971 || n = 18973 || n = 18978)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18969)
	; 7 bytes
	pha
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 18970)
	; 8 bytes
	tax
	pla
	lda #74
	jsr delay_256a_16_clocks
	txa
.elseif(n = 18974 || n = 18977 || n = 18979 || n = 18982)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 18975)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18976)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18980 || n = 18983)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18981)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18984)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18986)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18987 || n = 18990 || n = 18992 || n = 18995)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18988)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 18989)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18991 || n = 18993 || n = 18994)
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
.elseif(n = 19170)
	; 13 bytes
	delay_n (n-19165)
	lda #172
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19171)
	; 13 bytes
	delay_n (n-19166)
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19172)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19172 && n < 19177))
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19177)
	; 14 bytes
	delay_n (n-19161)
	lda #168
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19178)
	; 14 bytes
	delay_n (n-19162)
	lda #169
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19179)
	; 14 bytes
	delay_n (n-19163)
	lda #170
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180 || n = 19193)
	; 14..16 bytes
	delay_n (n-19164)
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19181 || n = 19183 || n = 19194)
	; 14..16 bytes
	delay_n (n-19165)
	lda #172
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19182 || n = 19184 || n = 19195)
	; 14..16 bytes
	delay_n (n-19166)
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19185)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19186)
	; 16 bytes
	delay_n (n-19181)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif((n > 19186 && n < 19190))
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19190)
	; 16 bytes
	delay_n (n-19161)
	lda #168
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19191)
	; 16 bytes
	delay_n (n-19162)
	lda #169
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19192)
	; 16 bytes
	delay_n (n-19163)
	lda #170
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.if(n = 19219 || n = 19224 || n = 19227 || n = 19229 || n = 19234 || n = 19242)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19220)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19221)
	; 8 bytes
	tax
	pla
	lda #75
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 19222)
	; 7 bytes
	tax
	lda #75
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19223 || n = 19228)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19225)
	; 7 bytes
	pha
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19226)
	; 8 bytes
	tax
	pla
	lda #75
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19230 || n = 19233 || n = 19235 || n = 19238 || n = 19245)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 19231)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19232)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19236 || n = 19239)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19237)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19240)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19241)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19243 || n = 19246 || n = 19248 || n = 19251)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19244)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19247 || n = 19249 || n = 19250)
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
.elseif(n = 19426 || n = 19437 || n = 19439 || n = 19450)
	; 13..16 bytes
	delay_n (n-19421)
	lda #172
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19427 || n = 19438 || n = 19440 || n = 19451)
	; 13..16 bytes
	delay_n (n-19422)
	lda #173
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19428 || n = 19441)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 19428 && n < 19433) || (n > 19442 && n < 19446))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19433 || n = 19446)
	; 14..16 bytes
	delay_n (n-19417)
	lda #168
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19434 || n = 19447)
	; 14..16 bytes
	delay_n (n-19418)
	lda #169
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19435 || n = 19448)
	; 14..16 bytes
	delay_n (n-19419)
	lda #170
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19436 || n = 19449)
	; 14..16 bytes
	delay_n (n-19420)
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19442)
	; 16 bytes
	delay_n (n-19437)
	jsr @nearby_rts_15cyc
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 19475 || n = 19480 || n = 19483 || n = 19485 || n = 19490)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19476)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19477)
	; 8 bytes
	tax
	pla
	lda #76
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 19478)
	; 7 bytes
	tax
	lda #76
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19479 || n = 19484)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19481)
	; 7 bytes
	pha
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19482)
	; 8 bytes
	tax
	pla
	lda #76
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19486 || n = 19489 || n = 19491)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 19487)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19488)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19492)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19494 || n = 19501)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 19495)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19496)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19497)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19498)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19499 || n = 19502 || n = 19504 || n = 19507)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19500)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19503 || n = 19505 || n = 19506)
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
.elseif(n = 19682)
	; 13 bytes
	delay_n (n-19677)
	lda #172
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19683)
	; 13 bytes
	delay_n (n-19678)
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19684)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 19684 && n < 19688))
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
.if(n = 19688 || (n > 19698 && n < 19702))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19689 || n = 19702)
	; 14..16 bytes
	delay_n (n-19673)
	lda #168
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19690 || n = 19703)
	; 14..16 bytes
	delay_n (n-19674)
	lda #169
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19691 || n = 19704)
	; 14..16 bytes
	delay_n (n-19675)
	lda #170
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19692 || n = 19705)
	; 14..16 bytes
	delay_n (n-19676)
	lda #171
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19693 || n = 19695 || n = 19706)
	; 14..16 bytes
	delay_n (n-19677)
	lda #172
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19694 || n = 19696)
	; 14..15 bytes
	delay_n (n-19678)
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19697)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19698)
	; 16 bytes
	delay_n (n-19693)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 16 bytes
	delay_n (n-19678)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 19731 || n = 19736 || n = 19739 || n = 19741 || n = 19746 || n = 19754)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19732)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19733)
	; 8 bytes
	tax
	pla
	lda #77
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 19734)
	; 7 bytes
	tax
	lda #77
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19735 || n = 19740)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19737)
	; 7 bytes
	pha
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19738)
	; 8 bytes
	tax
	pla
	lda #77
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19742 || n = 19745 || n = 19747 || n = 19750 || n = 19757)
	; 9..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 19743)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19744)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19748 || n = 19751)
	; 10 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19749)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19752)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19753)
	; 10 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19755 || n = 19758 || n = 19760 || n = 19763)
	; 11 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19756)
	; 11 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 19759 || n = 19761 || n = 19762)
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
.elseif(n = 19938 || n = 19949 || n = 19951)
	; 13..15 bytes
	delay_n (n-19933)
	lda #172
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19939 || n = 19950 || n = 19952)
	; 13..15 bytes
	delay_n (n-19934)
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19940 || n = 19953)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif((n > 19940 && n < 19945) || (n > 19954 && n < 19958))
	; 14..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 19945 || n = 19958)
	; 14..16 bytes
	delay_n (n-19929)
	lda #168
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19946 || n = 19959)
	; 14..16 bytes
	delay_n (n-19930)
	lda #169
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19947 || n = 19960)
	; 14..16 bytes
	delay_n (n-19931)
	lda #170
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19948)
	; 14 bytes
	delay_n (n-19932)
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19954)
	; 16 bytes
	delay_n (n-19949)
	jsr @nearby_rts_15cyc
	lda #173
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if(n = 19961)
	; 16 bytes
	delay_n (n-19932)
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19962)
	; 16 bytes
	delay_n (n-19933)
	lda #172
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19963)
	; 16 bytes
	delay_n (n-19934)
	lda #173
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
	; 7 bytes
	tax
	lda #(n-17)/256
	jsr delay_256a_11_clocks_
	txa
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
.elseif(n = 19987 || n = 19992 || n = 19995 || n = 19997)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 19988)
	; 7 bytes
	pha
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	pla
.elseif(n = 19989)
	; 8 bytes
	tax
	pla
	lda #78
	jsr delay_256a_11_clocks_
	txa
.elseif(n = 19990)
	; 7 bytes
	tax
	lda #78
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19991 || n = 19996)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19993)
	; 7 bytes
	pha
	lda #78
	jsr delay_256a_16_clocks
	pla
.elseif(n = 19994)
	; 8 bytes
	tax
	pla
	lda #78
	jsr delay_256a_16_clocks
	txa
.elseif(n = 19998)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 19999)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
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
