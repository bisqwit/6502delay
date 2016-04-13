; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Z+N, and C
; Will preserve A, X, Y, S, D, V, and I (interrupt-disable flag)
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
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 32
; - 8 bytes or less when n <= 37
; - 9 bytes or less when n <= 296
; -10 bytes or less when n <= 298
; -11 bytes or less when n <= 303
; -12 bytes or less when n <= 311
; -13 bytes or less when n <= 314
; -14 bytes or less when n <= 496
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.2, max=6
; - When 21 <= n <= 30: min=5, avg=5.9, max=7
; - When 31 <= n <= 50: min=7, avg=8.6, max=9
; - When 51 <= n <= 75: min=9, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.1, max=11
; - When 301 <= n <= 500: min=11, avg=13.9, max=15
; - When 501 <= n <= 750: min=13, avg=15.0, max=15
; - When 751 <= n <= 1000: min=13, avg=15.0, max=15
; - When 1001 <= n <= 2000: min=13, avg=15.0, max=15
; - When 2001 <= n <= 3000: min=13, avg=15.0, max=15
; - When 3001 <= n <= 5000: min=13, avg=15.0, max=15
; - When 5001 <= n <= 7500: min=13, avg=15.0, max=15
; - When 7501 <= n <= 10000: min=13, avg=15.0, max=15
; - When 10001 <= n <= 20000: min=13, avg=15.0, max=15
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
.elseif((n > 3 && n < 7) || (n > 15 && n < 19) || n = 20 || n = 31 || n = 32 || n = 34 || (n > 296 && n < 301) || n = 304 || n = 305)
	; 2..12 bytes
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
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || (n > 20 && n < 24) || n = 25 || n = 33 || (n > 34 && n < 38))
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
.elseif(n = 24 || (n > 305 && n < 309))
	; 6..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28 || n = 38 || n = 40)
	; 6..9 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 27 || n = 29 || n = 30 || n = 39 || (n > 308 && n < 312))
	; 6..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif((n > 40 && n < 297))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
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
.elseif(n = 314)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #1
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 315)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #247
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 316)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #248
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 317)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #249
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 318)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #250
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 319)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #251
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 320)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #252
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 321)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #253
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 322)
	; 14 bytes
	php
	pha
	lda #0
	jsr delay_a_25_clocks
	lda #254
	jsr delay_a_25_clocks
	pla
	plp
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
.if((n > 546 && n < 571) || n = 572 || (n > 573 && n < 626))
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
.elseif(n = 571)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 573)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 828) || n = 829 || (n > 830 && n < 1085) || n = 1086 || (n > 1087 && n < 1251))
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
.elseif(n = 828)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 830 || n = 1087)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1085)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1342) || n = 1343 || (n > 1344 && n < 1599) || n = 1600 || (n > 1601 && n < 1856) || n = 1857 || (n > 1858 && n < 2113) || n = 2114 || (n > 2115 && n < 2370) || n = 2371 || (n > 2372 && n < 2501))
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
.elseif(n = 1342)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 1344 || n = 1601 || n = 1858 || n = 2115 || n = 2372)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1599)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 1856)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 2113)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 2370)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2627) || n = 2628 || (n > 2629 && n < 2884) || n = 2885 || (n > 2886 && n < 3141) || n = 3142 || (n > 3143 && n < 3398) || n = 3399 || (n > 3400 && n < 3655) || n = 3656 || (n > 3657 && n < 3912) || n = 3913 || (n > 3914 && n < 4169) || n = 4170 || (n > 4171 && n < 4426) || n = 4427 || (n > 4428 && n < 4683) || n = 4684 || (n > 4685 && n < 4940) || n = 4941 || (n > 4942 && n < 5001))
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
.elseif(n = 2627)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 2629 || n = 2886 || n = 3143 || n = 3400 || n = 3657 || n = 3914 || n = 4171 || n = 4428 || n = 4685 || n = 4942)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2884)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 3141)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 3398)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 3655)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 3912)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4169)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4426)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4683)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4940)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5197) || n = 5198 || (n > 5199 && n < 5454) || n = 5455 || (n > 5456 && n < 5711) || n = 5712 || (n > 5713 && n < 5968) || n = 5969 || (n > 5970 && n < 6225) || n = 6226 || (n > 6227 && n < 6482) || n = 6483 || (n > 6484 && n < 6739) || n = 6740 || (n > 6741 && n < 6996) || n = 6997 || (n > 6998 && n < 7253) || n = 7254 || (n > 7255 && n < 7510) || n = 7511 || (n > 7512 && n < 7767) || n = 7768 || (n > 7769 && n < 8024) || n = 8025 || (n > 8026 && n < 8281) || n = 8282 || (n > 8283 && n < 8538) || n = 8539 || (n > 8540 && n < 8795) || n = 8796 || (n > 8797 && n < 9052) || n = 9053 || (n > 9054 && n < 9309) || n = 9310 || (n > 9311 && n < 9566) || n = 9567 || (n > 9568 && n < 9823) || n = 9824 || (n > 9825 && n < 10001))
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
.elseif(n = 5197)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 5199 || n = 5456 || n = 5713 || n = 5970 || n = 6227 || n = 6484 || n = 6741 || n = 6998 || n = 7255 || n = 7512 || n = 7769 || n = 8026 || n = 8283 || n = 8540 || n = 8797 || n = 9054 || n = 9311 || n = 9568 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5454)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 5711)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 5968)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6225)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6482)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6739)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6996)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 7253)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 7510)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 7767)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8024)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8281)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8538)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8795)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9052)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9309)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9566)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9823)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10080) || n = 10081 || (n > 10082 && n < 10337) || n = 10338 || (n > 10339 && n < 10594) || n = 10595 || (n > 10596 && n < 10851) || n = 10852 || (n > 10853 && n < 11108) || n = 11109 || (n > 11110 && n < 11365) || n = 11366 || (n > 11367 && n < 11622) || n = 11623 || (n > 11624 && n < 11879) || n = 11880 || (n > 11881 && n < 12136) || n = 12137 || (n > 12138 && n < 12393) || n = 12394 || (n > 12395 && n < 12650) || n = 12651 || (n > 12652 && n < 12907) || n = 12908 || (n > 12909 && n < 13164) || n = 13165 || (n > 13166 && n < 13421) || n = 13422 || (n > 13423 && n < 13678) || n = 13679 || (n > 13680 && n < 13935) || n = 13936 || (n > 13937 && n < 14192) || n = 14193 || (n > 14194 && n < 14449) || n = 14450 || (n > 14451 && n < 14706) || n = 14707 || (n > 14708 && n < 14963) || n = 14964 || (n > 14965 && n < 15001))
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
.elseif(n = 10080)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 10082 || n = 10339 || n = 10596 || n = 10853 || n = 11110 || n = 11367 || n = 11624 || n = 11881 || n = 12138 || n = 12395 || n = 12652 || n = 12909 || n = 13166 || n = 13423 || n = 13680 || n = 13937 || n = 14194 || n = 14451 || n = 14708 || n = 14965)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10337)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 10594)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 10851)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11108)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11365)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11622)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11879)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12136)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12393)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12650)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12907)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13164)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13421)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13678)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13935)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14192)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14449)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14706)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14963)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15220) || n = 15221 || (n > 15222 && n < 15477) || n = 15478 || (n > 15479 && n < 15734) || n = 15735 || (n > 15736 && n < 15991) || n = 15992 || (n > 15993 && n < 16248) || n = 16249 || (n > 16250 && n < 16505) || n = 16506 || (n > 16507 && n < 16762) || n = 16763 || (n > 16764 && n < 17019) || n = 17020 || (n > 17021 && n < 17276) || n = 17277 || (n > 17278 && n < 17533) || n = 17534 || (n > 17535 && n < 17790) || n = 17791 || (n > 17792 && n < 18047) || n = 18048 || (n > 18049 && n < 18304) || n = 18305 || (n > 18306 && n < 18561) || n = 18562 || (n > 18563 && n < 18818) || n = 18819 || (n > 18820 && n < 19075) || n = 19076 || (n > 19077 && n < 19332) || n = 19333 || (n > 19334 && n < 19589) || n = 19590 || (n > 19591 && n < 19846) || n = 19847 || (n > 19848 && n < 20001))
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
.elseif(n = 15220)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 15222 || n = 15479 || n = 15736 || n = 15993 || n = 16250 || n = 16507 || n = 16764 || n = 17021 || n = 17278 || n = 17535 || n = 17792 || n = 18049 || n = 18306 || n = 18563 || n = 18820 || n = 19077 || n = 19334 || n = 19591 || n = 19848)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15477)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 15734)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 15991)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 16248)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 16505)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 16762)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17019)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17276)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17533)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17790)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18047)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18304)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18561)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18818)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19075)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19332)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19589)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19846)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
