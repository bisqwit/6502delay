; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Y, S, and Z+N
; Will preserve A, X, C, D, V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 30
; - 7 bytes or less when n <= 33
; - 8 bytes or less when n <= 37
; - 9 bytes or less when n <= 296
; -10 bytes or less when n <= 299
; -11 bytes or less when n <= 303
; -12 bytes or less when n <= 311
; -13 bytes or less when n <= 314
; -14 bytes or less when n <= 496
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=3.9, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=6
; - When 31 <= n <= 50: min=7, avg=8.5, max=9
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
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 16 || n = 17 || n = 20 || n = 31 || n = 32 || n = 34 || n = 297 || n = 298 || n = 300 || n = 304 || n = 305)
	; 2..12 bytes
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
.elseif((n > 8 && n < 12) || n = 13 || n = 19 || (n > 20 && n < 24) || n = 25 || (n > 34 && n < 38))
	; 3..8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 27 || n = 29 || n = 30 || n = 33 || (n > 308 && n < 312))
	; 4..12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || (n > 305 && n < 309))
	; 6..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n > 37 && n < 41))
	; 9 bytes
	php
	tay
	lda #n-38
	jsr delay_a_25_clocks
	tya
	plp
.elseif((n > 40 && n < 297))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 299)
	; 10 bytes
	pha
	delay_n (n-3)
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
	delay_n 5
	lda #172
	jsr delay_a_25_clocks
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 498)
	; 15 bytes
	php
	delay_n 5
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
.if((n > 546 && n < 568) || n = 569 || (n > 574 && n < 626))
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
.elseif(n = 568)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #2
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 570 || n = 573)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 572)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #2
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 574)
	; 14 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.endif
.else
; 626 to 1250
.if((n > 625 && n < 825) || n = 826 || (n > 831 && n < 1082) || n = 1083 || (n > 1088 && n < 1251))
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
.elseif(n = 825)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #3
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 827 || n = 830 || n = 1084 || n = 1087)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 829)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 831 || n = 1088)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 1082)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #4
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
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
.elseif(n = 1086)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.else
; 1251 to 2500
.if((n > 1250 && n < 1339) || n = 1340 || (n > 1345 && n < 1596) || n = 1597 || (n > 1602 && n < 1853) || n = 1854 || (n > 1859 && n < 2110) || n = 2111 || (n > 2116 && n < 2367) || n = 2368 || (n > 2373 && n < 2501))
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
.elseif(n = 1339)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #5
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 1341 || n = 1344 || n = 1598 || n = 1601 || n = 1855 || n = 1858 || n = 2112 || n = 2115 || n = 2369 || n = 2372)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1343)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #5
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 1345 || n = 1602 || n = 1859 || n = 2116 || n = 2373)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 1596)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #6
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
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
.elseif(n = 1600)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #6
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 1853)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #7
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 1857)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 2110)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #8
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 2114)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 2367)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #9
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 2371)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.else
.if((n >= 2501) && (n < 3751))
; 2501 to 3750
.if((n > 2500 && n < 2624) || n = 2625 || (n > 2630 && n < 2881) || n = 2882 || (n > 2887 && n < 3138) || n = 3139 || (n > 3144 && n < 3395) || n = 3396 || (n > 3401 && n < 3652) || n = 3653 || (n > 3658 && n < 3751))
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
.elseif(n = 2624)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #10
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 2626 || n = 2629 || n = 2883 || n = 2886 || n = 3140 || n = 3143 || n = 3397 || n = 3400 || n = 3654 || n = 3657)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2628)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 2630 || n = 2887 || n = 3144 || n = 3401 || n = 3658)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2881)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #11
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
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
.elseif(n = 2885)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 3138)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #12
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 3142)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 3395)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #13
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 3399)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 3652)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #14
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 3656)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 3751 to 5000
.if((n > 3750 && n < 3909) || n = 3910 || (n > 3915 && n < 4166) || n = 4167 || (n > 4172 && n < 4423) || n = 4424 || (n > 4429 && n < 4680) || n = 4681 || (n > 4686 && n < 4937) || n = 4938 || (n > 4943 && n < 5001))
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
.elseif(n = 3909)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #15
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 3911 || n = 3914 || n = 4168 || n = 4171 || n = 4425 || n = 4428 || n = 4682 || n = 4685 || n = 4939 || n = 4942)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3913)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 3915 || n = 4172 || n = 4429 || n = 4686 || n = 4943)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4166)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #16
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 4170)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 4423)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #17
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 4427)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 4680)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #18
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 4684)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 4937)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #19
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 4941)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
; 5001 to 6250
.if((n > 5000 && n < 5194) || n = 5195 || (n > 5200 && n < 5451) || n = 5452 || (n > 5457 && n < 5708) || n = 5709 || (n > 5714 && n < 5965) || n = 5966 || (n > 5971 && n < 6222) || n = 6223 || (n > 6228 && n < 6251))
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
.elseif(n = 5194)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #20
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 5196 || n = 5199 || n = 5453 || n = 5456 || n = 5710 || n = 5713 || n = 5967 || n = 5970 || n = 6224 || n = 6227)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5198)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 5200 || n = 5457 || n = 5714 || n = 5971 || n = 6228)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5451)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #21
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
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
.elseif(n = 5455)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 5708)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #22
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 5712)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 5965)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #23
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 5969)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 6222)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #24
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 6226)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6479) || n = 6480 || (n > 6485 && n < 6736) || n = 6737 || (n > 6742 && n < 6993) || n = 6994 || (n > 6999 && n < 7250) || n = 7251 || (n > 7256 && n < 7501))
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
.elseif(n = 6479)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #25
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 6481 || n = 6484 || n = 6738 || n = 6741 || n = 6995 || n = 6998 || n = 7252 || n = 7255)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6483)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 6485 || n = 6742 || n = 6999 || n = 7256)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6736)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #26
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 6740)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 6993)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #27
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 6997)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 7250)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #28
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 7254)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
; 7501 to 8750
.if((n > 7500 && n < 7507) || n = 7508 || (n > 7513 && n < 7764) || n = 7765 || (n > 7770 && n < 8021) || n = 8022 || (n > 8027 && n < 8278) || n = 8279 || (n > 8284 && n < 8535) || n = 8536 || (n > 8541 && n < 8751))
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
.elseif(n = 7507)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #29
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 7509 || n = 7512 || n = 7766 || n = 7769 || n = 8023 || n = 8026 || n = 8280 || n = 8283 || n = 8537 || n = 8540)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7511)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 7513 || n = 7770 || n = 8027 || n = 8284 || n = 8541)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7764)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #30
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 7768)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 8021)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #31
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 8025)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 8278)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #32
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 8282)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 8535)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #33
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 8539)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8792) || n = 8793 || (n > 8798 && n < 9049) || n = 9050 || (n > 9055 && n < 9306) || n = 9307 || (n > 9312 && n < 9563) || n = 9564 || (n > 9569 && n < 9820) || n = 9821 || (n > 9826 && n < 10001))
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
.elseif(n = 8792)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #34
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 8794 || n = 8797 || n = 9051 || n = 9054 || n = 9308 || n = 9311 || n = 9565 || n = 9568 || n = 9822 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8796)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 8798 || n = 9055 || n = 9312 || n = 9569 || n = 9826)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 9049)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #35
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 9053)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 9306)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #36
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 9310)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 9563)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #37
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 9567)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 9820)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #38
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 9824)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.if((n > 10000 && n < 10077) || n = 10078 || (n > 10083 && n < 10334) || n = 10335 || (n > 10340 && n < 10591) || n = 10592 || (n > 10597 && n < 10848) || n = 10849 || (n > 10854 && n < 11105) || n = 11106 || (n > 11111 && n < 11251))
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
.elseif(n = 10077)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #39
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 10079 || n = 10082 || n = 10336 || n = 10339 || n = 10593 || n = 10596 || n = 10850 || n = 10853 || n = 11107 || n = 11110)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 10081)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 10083 || n = 10340 || n = 10597 || n = 10854 || n = 11111)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 10334)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #40
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
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
.elseif(n = 10338)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 10591)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #41
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 10595)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 10848)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #42
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 10852)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 11105)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #43
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 11109)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 11251 to 12500
.if((n > 11250 && n < 11362) || n = 11363 || (n > 11368 && n < 11619) || n = 11620 || (n > 11625 && n < 11876) || n = 11877 || (n > 11882 && n < 12133) || n = 12134 || (n > 12139 && n < 12390) || n = 12391 || (n > 12396 && n < 12501))
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
.elseif(n = 11362)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #44
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 11364 || n = 11367 || n = 11621 || n = 11624 || n = 11878 || n = 11881 || n = 12135 || n = 12138 || n = 12392 || n = 12395)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 11366)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 11368 || n = 11625 || n = 11882 || n = 12139 || n = 12396)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 11619)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #45
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 11623)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 11876)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #46
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 11880)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 12133)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #47
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 12137)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 12390)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #48
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 12394)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
; 12501 to 13750
.if((n > 12500 && n < 12647) || n = 12648 || (n > 12653 && n < 12904) || n = 12905 || (n > 12910 && n < 13161) || n = 13162 || (n > 13167 && n < 13418) || n = 13419 || (n > 13424 && n < 13675) || n = 13676 || (n > 13681 && n < 13751))
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
.elseif(n = 12647)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #49
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 12649 || n = 12652 || n = 12906 || n = 12909 || n = 13163 || n = 13166 || n = 13420 || n = 13423 || n = 13677 || n = 13680)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 12651)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 12653 || n = 12910 || n = 13167 || n = 13424 || n = 13681)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 12904)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #50
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 12908)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 13161)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #51
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 13165)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 13418)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #52
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 13422)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 13675)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #53
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 13679)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 13751 to 15000
.if((n > 13750 && n < 13932) || n = 13933 || (n > 13938 && n < 14189) || n = 14190 || (n > 14195 && n < 14446) || n = 14447 || (n > 14452 && n < 14703) || n = 14704 || (n > 14709 && n < 14960) || n = 14961 || (n > 14966 && n < 15001))
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
.elseif(n = 13932)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #54
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 13934 || n = 13937 || n = 14191 || n = 14194 || n = 14448 || n = 14451 || n = 14705 || n = 14708 || n = 14962 || n = 14965)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 13936)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 13938 || n = 14195 || n = 14452 || n = 14709 || n = 14966)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 14189)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #55
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 14193)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 14446)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #56
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 14450)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 14703)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #57
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 14707)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 14960)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #58
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 14964)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
; 15001 to 16250
.if((n > 15000 && n < 15217) || n = 15218 || (n > 15223 && n < 15474) || n = 15475 || (n > 15480 && n < 15731) || n = 15732 || (n > 15737 && n < 15988) || n = 15989 || (n > 15994 && n < 16245) || n = 16246)
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
.elseif(n = 15217)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #59
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 15219 || n = 15222 || n = 15476 || n = 15479 || n = 15733 || n = 15736 || n = 15990 || n = 15993 || n = 16247 || n = 16250)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 15221)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 15223 || n = 15480 || n = 15737 || n = 15994)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 15474)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #60
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
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
.elseif(n = 15478)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 15731)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #61
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 15735)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 15988)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #62
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 15992)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 16245)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #63
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 16249)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 16251 to 17500
.if(n = 16251 || n = 16508 || n = 16765 || n = 17022 || n = 17279)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n > 16251 && n < 16502) || n = 16503 || (n > 16508 && n < 16759) || n = 16760 || (n > 16765 && n < 17016) || n = 17017 || (n > 17022 && n < 17273) || n = 17274 || (n > 17279 && n < 17501))
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
.elseif(n = 16502)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #64
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 16504 || n = 16507 || n = 16761 || n = 16764 || n = 17018 || n = 17021 || n = 17275 || n = 17278)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 16506)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 16759)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #65
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 16763)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 17016)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #66
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 17020)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 17273)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #67
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 17277)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
; 17501 to 18750
.if((n > 17500 && n < 17530) || n = 17531 || (n > 17536 && n < 17787) || n = 17788 || (n > 17793 && n < 18044) || n = 18045 || (n > 18050 && n < 18301) || n = 18302 || (n > 18307 && n < 18558) || n = 18559 || (n > 18564 && n < 18751))
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
.elseif(n = 17530)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #68
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 17532 || n = 17535 || n = 17789 || n = 17792 || n = 18046 || n = 18049 || n = 18303 || n = 18306 || n = 18560 || n = 18563)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 17534)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 17536 || n = 17793 || n = 18050 || n = 18307 || n = 18564)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 17787)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #69
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 17791)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 18044)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #70
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 18048)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 18301)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #71
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 18305)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 18558)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #72
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 18562)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.endif
.else
; 18751 to 20000
.if((n > 18750 && n < 18815) || n = 18816 || (n > 18821 && n < 19072) || n = 19073 || (n > 19078 && n < 19329) || n = 19330 || (n > 19335 && n < 19586) || n = 19587 || (n > 19592 && n < 19843) || n = 19844 || (n > 19849 && n < 20001))
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
.elseif(n = 18815)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #73
	jsr delay_256x_a_30_clocks
	tya
	tax
	pla
	plp
.elseif(n = 18817 || n = 18820 || n = 19074 || n = 19077 || n = 19331 || n = 19334 || n = 19588 || n = 19591 || n = 19845 || n = 19848)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 18819)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 18821 || n = 19078 || n = 19335 || n = 19592 || n = 19849)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 19072)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #74
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 19076)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 19329)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #75
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 19333)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 19586)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #76
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 19590)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
	tya
.elseif(n = 19843)
	; 13 bytes
	php
	pha
	txa
	tay
	lax #77
	jsr delay_256x_a_30_clocks
	tya
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
.elseif(n = 19847)
	; 14 bytes
	tay
	pla
	php
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
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
