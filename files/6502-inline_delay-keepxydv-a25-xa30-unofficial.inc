; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, Z+N, and C
; Will preserve X, Y, S, D, V, and I (interrupt-disable flag)
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
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 290
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 315
; -10 bytes or less when n <= 1305
; -11 bytes or less when n <= 2321
; -12 bytes or less when n <= 3121
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=4, avg=5.6, max=7
; - When 31 <= n <= 50: min=4, avg=6.2, max=7
; - When 51 <= n <= 75: min=5, avg=6.7, max=7
; - When 76 <= n <= 100: min=6, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.3, max=10
; - When 501 <= n <= 750: min=6, avg=8.3, max=10
; - When 751 <= n <= 1000: min=7, avg=8.3, max=10
; - When 1001 <= n <= 2000: min=6, avg=9.0, max=11
; - When 2001 <= n <= 3000: min=8, avg=10.2, max=12
; - When 3001 <= n <= 5000: min=9, avg=11.4, max=13
; - When 5001 <= n <= 7500: min=10, avg=12.7, max=13
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
.if((n >= 1) && (n < 157))
.if((n >= 1) && (n < 79))
.if((n >= 1) && (n < 40))
.if((n >= 1) && (n < 20))
; 1 to 19
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 6 || (n > 15 && n < 19))
	; 2..5 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	clc
	Jcc *+2
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 13 || n = 19)
	; 3..5 bytes
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
.endif
.else
; 20 to 39
.if(n = 20)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
.elseif(n = 21)
	; 5 bytes
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
.elseif(n = 22)
	; 5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 23 || n = 26 || n = 33)
	; 5..6 bytes
	clc
	delay_n (n-2)
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 27)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 28)
	; 6 bytes
	clc
	clc
	delay_n (n-4)
.elseif(n = 29)
	; 6 bytes
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
.elseif(n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 31)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
.elseif(n = 32)
	; 7 bytes
	lda #$2A ;hides 'rol a'
	delay_n 6
	Jpl *-4
.elseif(n = 34)
	; 5 bytes
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
.elseif(n = 35)
	; 6 bytes
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
.elseif(n = 36)
	; 6 bytes
	clc
	lda #$0A ;hides 'asl a'
	clc
	Jpl *-2
.elseif(n = 37)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 38)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.endif
.endif
.else
; 40 to 78
.if(n = 40 || n = 45 || n = 47 || (n > 48 && n < 53) || n = 54 || n = 56 || n = 60 || n = 61 || (n > 63 && n < 71) || (n > 71 && n < 79))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 41)
	; 5 bytes
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 42)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 43)
	; 6 bytes
	sec
	sec
	delay_n (n-4)
.elseif(n = 44)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 4
	Jpl *-3
.elseif(n = 46 || n = 48 || n = 62)
	; 6..7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.elseif(n = 55)
	; 5 bytes
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
.elseif(n = 57)
	; 6 bytes
	clc
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
.elseif(n = 58)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	plp
.elseif(n = 59)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
.elseif(n = 63)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	nop $04
	Jne *-3
.elseif(n = 71)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 4
	Jne *-3
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 95) || (n > 95 && n < 118) || (n > 118 && n < 123) || (n > 123 && n < 128) || (n > 128 && n < 157))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 95)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
.elseif(n = 118)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	plp
.elseif(n = 123)
	; 7 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	plp
.elseif(n = 128)
	; 6 bytes
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 163) || (n > 163 && n < 288) || n = 289)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 163)
	; 7 bytes
	php
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	plp
.elseif(n = 288)
	; 6 bytes
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
.elseif(n = 290 || n = 305 || n = 310)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 291 || n = 297 || n = 301 || n = 302 || n = 304 || n = 307 || n = 309 || n = 312)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 292)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 293 || n = 294 || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 298 || n = 308)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 299 || n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 303)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 311)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 348 || n = 362 || n = 369)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 314 || n = 317 || n = 319 || n = 321 || n = 322 || n = 324 || n = 326 || n = 331 || n = 332 || n = 336 || n = 337 || n = 339 || n = 342 || n = 344 || n = 347 || n = 349 || n = 352 || n = 354 || n = 357 || n = 359 || n = 366 || n = 367 || n = 371 || n = 372 || n = 374 || n = 377 || n = 379 || n = 382 || n = 384 || n = 387 || n = 389)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316)
	; 10 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	plp
.elseif(n = 318)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	iny
	Jpl *-3
	tay
.elseif(n = 323 || n = 328 || n = 333 || n = 338 || n = 343 || n = 351 || n = 353 || n = 358 || n = 363 || n = 368 || n = 373 || n = 378 || n = 386 || n = 388)
	; 8..10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 0) && ((n > 324 && n < 341) || n = 355 || n = 360 || (n > 369 && n < 381) || n = 390))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 334 || n = 341 || n = 376)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 329 || n = 364)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 346)
	; 9 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 356)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 361)
	; 10 bytes
	php
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	plp
.elseif(n = 381)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 383)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
.endif
.else
; 391 to 468
.if(n = 391)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jmi *-4
	tay
.elseif(n = 392 || n = 394 || n = 396 || n = 406 || n = 407 || n = 412 || n = 414 || n = 417 || n = 419 || n = 422 || n = 424 || n = 427 || n = 429 || n = 431 || n = 437 || n = 441 || n = 442 || n = 444 || n = 447 || n = 449 || n = 452 || n = 454 || n = 457 || n = 462 || n = 464 || n = 466)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 393 || n = 398 || n = 403 || n = 413 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 456 || n = 458 || n = 463)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 0) && (n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 397 || n = 411 || n = 418 || n = 453 || n = 467)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 399)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 401)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 404 || n = 432 || n = 439 || n = 446)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 408)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 409)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 416)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 421)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	tax
.elseif(n = 426)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $2EA0'
	ldy #46
	dey
	Jpl *-4
	tay
.elseif(n = 434)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 436)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D0A0'
	ldy #208
	iny
	Jne *-4
	tay
.elseif(n = 451)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 459)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	tay
.elseif(n = 461)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 468)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 504)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 0) && (n = 470 || n = 480 || n = 485 || n = 495 || (n > 504 && n < 516) || (n > 524 && n < 546)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 471)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
.elseif(n = 472 || n = 477 || n = 479 || n = 482 || n = 486 || n = 487 || n = 492 || n = 494 || n = 497 || n = 499 || n = 506 || n = 511 || n = 512 || n = 517 || n = 519 || n = 522 || n = 524 || n = 527 || n = 529 || n = 531 || n = 532 || n = 539 || n = 542 || n = 546)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 472 && n < 484) || (n > 497 && n < 519) || (n > 527 && n < 544)))
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 474 || n = 502 || n = 516 || n = 523 || n = 544)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 475 || n = 490 || n = 500 || n = 520)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 476)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 481 || n = 488 || n = 509 || n = 537)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 484)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 489)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
.elseif(n = 491)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 493)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jpl *-2
	tay
.elseif(n = 496)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 501)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
.elseif(n = 514)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 521)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 526)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $3AA0'
	ldy #58
	dey
	Jne *-4
	tay
.elseif(n = 534)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 536)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 541)
	; 9 bytes
	tya
	ldy #(n-13)/8
	cmp $C5
	dey
	Jpl *-3
	tay
.endif
.else
; 547 to 625
.if(n = 547 || n = 549 || n = 552 || n = 557 || n = 559 || n = 562 || n = 566 || n = 567 || n = 569 || n = 574 || n = 576 || n = 577 || n = 582 || n = 584 || n = 587 || n = 589 || n = 592 || n = 594 || n = 597 || n = 601 || n = 602 || n = 604 || n = 609 || n = 612 || n = 616 || n = 617 || n = 619 || n = 622 || n = 624)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 548)
	; 8 bytes
	tya
	ldy #(n-4)/8 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 5 = 0) && ((n > 549 && n < 566) || n = 590 || (n > 599 && n < 611)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 551 || n = 586 || n = 607 || n = 614 || n = 621)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 553 || n = 563 || n = 568 || n = 573 || n = 578 || n = 583 || n = 588 || n = 596 || n = 598 || n = 603 || n = 608 || n = 613 || n = 618 || n = 623)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 554)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 561)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	dey
	dey
	Jpl *-3
	tay
.elseif(n = 564)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
.elseif((n .mod 5 = 0) && ((n > 569 && n < 586) || n = 595 || (n > 614 && n < 626)))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 571)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	iny
	Jne *-4
	tay
.elseif(n = 581 || n = 599)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 591)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 606)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 611)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
	tax
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626 || n = 689)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 627 || n = 629 || n = 632 || n = 634 || n = 637 || n = 639 || n = 647 || n = 652 || n = 654 || n = 657 || n = 659 || n = 662 || n = 664 || n = 667 || n = 669 || n = 672 || n = 674 || n = 679 || n = 681 || n = 682 || n = 686 || n = 687 || n = 692 || n = 694 || n = 697 || n = 699 || n = 702)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 649 || n = 663 || n = 684 || n = 698)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 630 || n = 640 || n = 645 || n = 650)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 631)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
	plp
.elseif(n = 633)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 701)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 638 || n = 643 || n = 648 || n = 653 || n = 658 || n = 666 || n = 668 || n = 673 || n = 683 || n = 693 || n = 703)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 641 || n = 701)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 642 || n = 656 || n = 677 || n = 691)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 644 || n = 671)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 646)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 661)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 676)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 678)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 696)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jpl *-3
	tay
.endif
.else
; 704 to 781
.if(n = 704 || n = 707 || n = 709 || n = 711 || n = 721 || n = 722 || n = 724 || n = 727 || n = 729 || n = 732 || n = 737 || n = 739 || n = 742 || n = 744 || n = 746 || n = 756 || n = 757 || n = 762 || n = 764 || n = 767 || n = 769 || n = 772 || n = 774 || n = 776 || n = 777)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 706)
	; 9 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jmi *-4
	tay
.elseif(n = 708 || n = 713 || n = 718 || n = 723 || n = 728 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 771 || n = 773 || n = 778)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 712 || n = 719 || n = 733 || n = 747 || n = 754 || n = 761 || n = 768)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 714 || n = 749)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 716 || n = 734)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 717)
	; 7 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
.elseif(n = 726)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 731)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
.elseif(n = 736)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
	plp
.elseif(n = 738)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 741 || n = 781)
	; 9 bytes
	tya
	ldy #(n-13)/8
	cmp $C5
	dey
	Jpl *-3
	tay
.elseif(n = 751)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jmi *-4
	tay
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 759)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 766)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 779)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if(n = 782 || n = 789 || n = 824 || n = 838 || n = 852)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 782 && n < 799) || n = 808 || (n > 817 && n < 834) || (n > 842 && n < 859)))
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 784 || n = 819 || n = 854)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 0) && ((n > 784 && n < 856)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 786)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 787 || n = 791 || n = 792 || n = 794 || n = 797 || n = 799 || n = 802 || n = 807 || n = 809 || n = 812 || n = 814 || n = 816 || n = 822 || n = 826 || n = 827 || n = 829 || n = 832 || n = 834 || n = 837 || n = 839 || n = 842 || n = 844 || n = 846 || n = 847 || n = 849 || n = 856)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 796 || n = 803 || n = 817 || n = 831 || n = 859)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 801)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	dey
	Jpl *-4
	tay
.elseif(n = 804)
	; 8 bytes
	tya
	ldy #(n-4)/8 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 806 || n = 851)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 811)
	; 10 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 813)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 821)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 836)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	iny
	Jne *-2
	pla
	tay
.elseif(n = 841)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DCA0'
	ldy #220
	dey
	Jmi *-4
	tay
.elseif(n = 857)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.endif
.else
; 860 to 937
.if((n .mod 5 = 0) && ((n > 859 && n < 936)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 861 || n = 862 || n = 867 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 891 || n = 892 || n = 897 || n = 899 || n = 902 || n = 904 || n = 907 || n = 909 || n = 912 || n = 914 || n = 917 || n = 919 || n = 927 || n = 931 || n = 932 || n = 934 || n = 937)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 863 || n = 868 || n = 878 || n = 883 || n = 888 || n = 893 || n = 898 || n = 911 || n = 913 || n = 918 || n = 923 || n = 928 || n = 933)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 864)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 866 || n = 873 || n = 894 || n = 901 || n = 908)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 869)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 871)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 876)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	dex
	Jne *-4
	tax
.elseif(n = 881)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 886)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $62A0'
	ldy #98
	dey
	Jne *-4
	tay
.elseif(n = 887 || n = 922 || n = 929 || n = 936)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 889 || n = 924)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 896)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 903)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 906)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 916)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	iny
	Jne *-4
	tay
.elseif(n = 921)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 926)
	; 9 bytes
	tya
	ldy #(n-2)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if((n .mod 5 = 3) && (n = 938 || (n > 947 && n < 974) || (n > 982 && n < 1009)))
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 939)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
	tay
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1016)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 941)
	; 7 bytes
	tya
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 942 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 956 || n = 961 || n = 967 || n = 972 || n = 974 || n = 977 || n = 979 || n = 982 || n = 987 || n = 989 || n = 991 || n = 996 || n = 997 || n = 1001 || n = 1002 || n = 1004 || n = 1007 || n = 1009 || n = 1012 || n = 1014)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && (n = 943 || (n > 956 && n < 979) || (n > 991 && n < 1014)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 946)
	; 9 bytes
	php
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 951)
	; 10 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 959 || n = 994)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 962)
	; 7 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	tay
.elseif(n = 966)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	tay
.elseif(n = 969)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 976)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	dey
	Jne *-2
	pla
	tay
.elseif(n = 981)
	; 9 bytes
	tya
	ldy #(n-13)/8
	cmp $C5
	dey
	Jpl *-3
	tay
.elseif(n = 984)
	; 8 bytes
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 986)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1011)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jpl *-4
	tay
.endif
.else
; 1016 to 1093
.if(n = 1016 || n = 1021 || n = 1023 || n = 1028 || n = 1033 || n = 1043 || n = 1051 || n = 1053 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1088 || n = 1093)
	; 8..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1017 || n = 1019 || n = 1022 || n = 1024 || n = 1026 || n = 1032 || n = 1036 || n = 1037 || n = 1039 || n = 1042 || n = 1044 || n = 1047 || n = 1049 || n = 1052 || n = 1054 || n = 1057 || n = 1059 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1079 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1091 || n = 1092)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1018)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
.elseif((n .mod 5 = 0) && ((n > 1019 && n < 1091)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 7 = 5) && ((n > 1026 && n < 1049) || (n > 1061 && n < 1084)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1029 || n = 1064)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1031)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1038)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 1046)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1056)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1061)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1066)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $75A0'
	ldy #117
	dey
	Jpl *-4
	tay
.elseif(n = 1074)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 1081)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1086)
	; 9 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	Jmi *-4
	tay
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1102 || n = 1106 || n = 1107 || n = 1109 || n = 1112 || n = 1114 || n = 1117 || n = 1119 || n = 1122 || n = 1124 || n = 1127 || n = 1129 || n = 1131 || n = 1136 || n = 1137 || n = 1141 || n = 1142 || n = 1144 || n = 1147 || n = 1149 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1161 || n = 1162 || n = 1164 || n = 1171 || n = 1172 || n = 1176 || n = 1177 || n = 1179 || n = 1182 || n = 1184 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1196 || n = 1197 || n = 1199 || n = 1206 || n = 1207 || n = 1212 || n = 1214 || n = 1217 || n = 1219 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1231 || n = 1232 || n = 1234 || n = 1242 || n = 1246 || n = 1247 || n = 1249)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1251)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 7 = 5) && ((n > 1096 && n < 1119) || (n > 1131 && n < 1154) || (n > 1166 && n < 1189) || (n > 1201 && n < 1224) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1098 || n = 1103 || n = 1108 || n = 1113 || n = 1123 || n = 1133 || n = 1138 || n = 1143 || n = 1148 || n = 1158 || n = 1163 || n = 1168 || n = 1173 || n = 1178 || n = 1183 || n = 1193 || n = 1198 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1226 || n = 1228 || n = 1233 || n = 1238 || n = 1243 || n = 1248)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1099)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	tay
.elseif(n = 1101)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1116 || n = 1151 || n = 1186)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1126)
	; 10 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1128)
	; 8 bytes
	tya
	ldy #250 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1134 || n = 1169 || n = 1204 || n = 1239)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1156)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $7FA0'
	ldy #127
	dey
	Jpl *-4
	tay
.elseif(n = 1191)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $7CA0'
	ldy #124
	iny
	Jne *-4
	tay
.elseif(n = 1201)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $85A0'
	ldy #133
	dey
	Jne *-4
	tay
.elseif(n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1221)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1236)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	tay
.elseif(n = 1241)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || (n > 1271 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1257 || n = 1259 || n = 1262 || n = 1264 || n = 1266 || n = 1267 || n = 1269 || n = 1271 || n = 1276 || n = 1277 || n = 1281 || n = 1282 || n = 1284 || n = 1287 || n = 1289 || n = 1294 || n = 1295 || n = 1297 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1315 || n = 1316 || n = 1320 || n = 1323)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1253 || n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1303 || n = 1324)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1254)
	; 8 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1281)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 1256 || n = 1301 || n = 1310 || n = 1319)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1261 || n = 1317)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1263)
	; 8 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1274)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.elseif(n = 1290)
	; 9 bytes
	php
	sec
	lda #0
	sbc #1
	Jne *-2
	plp
.elseif(n = 1291)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $71A0'
	ldy #113
	iny
	Jne *-4
	tay
.elseif(n = 1292)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1296)
	; 9 bytes
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1298)
	; 9 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1299)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $70A0'
	ldy #112
	iny
	Jne *-4
	tay
.elseif(n = 1305)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1306 || n = 1322)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1308)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $6FA0'
	ldy #111
	iny
	Jne *-4
	tay
.elseif(n = 1312)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
	tay
.elseif(n = 1313 || n = 1325)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1318)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
.elseif(n = 1326)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.elseif(n = 1327)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1330 || n = 1332 || n = 1344 || n = 1348 || n = 1350 || n = 1351 || n = 1358 || n = 1360 || n = 1365 || n = 1367)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1352 || n = 1359 || n = 1366)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1333 || n = 1341 || n = 1357)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1334)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jpl *-5
	tay
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1364)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1336)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1336 && n < 1365)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1339)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1340 || n = 1347)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1343)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
.elseif(n = 1345)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
.elseif(n = 1353)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $96A0'
	ldy #150
	dey
	Jne *-4
	tay
.elseif(n = 1354)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $96A0'
	ldy #150
	dey
	Jne *-4
	tay
.elseif(n = 1361)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1362)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $69A0'
	ldy #105
	iny
	Jne *-4
	tay
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1369)
	; 10 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1406)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1371)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $98A0'
	ldy #152
	dey
	Jne *-4
	tay
.elseif(n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1388 || n = 1393 || n = 1395)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373 || n = 1382 || n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1376 || n = 1383 || n = 1401 || n = 1406)
	; 9..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1378)
	; 9 bytes
	tya
	ldy #250 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1380)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 1381)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1385 || n = 1397)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1387 || n = 1396)
	; 9..10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1389)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1390)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
.elseif(n = 1392)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1394)
	; 9 bytes
	php
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1399)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
.elseif(n = 1402)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.elseif(n = 1404)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1411 || n = 1414 || n = 1420 || n = 1421 || n = 1423 || n = 1428 || n = 1430 || n = 1435 || n = 1439 || n = 1442)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1408 && n < 1446)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1410 || n = 1431)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1441)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1413 || n = 1429 || n = 1437)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1415 || n = 1422)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1416)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $9DA0'
	ldy #157
	dey
	Jne *-4
	tay
.elseif(n = 1417)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $9DA0'
	ldy #157
	dey
	Jne *-4
	tay
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1425)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $62A0'
	ldy #98
	iny
	Jne *-4
	tay
.elseif(n = 1432 || n = 1441)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1434)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
	tax
.elseif(n = 1438)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1443)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $60A0'
	ldy #96
	iny
	Jne *-4
	tay
.elseif(n = 1444)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1446 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1448 || n = 1450 || n = 1464 || n = 1478)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1449 || n = 1451 || n = 1456 || n = 1458 || n = 1470 || n = 1474 || n = 1477 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1452)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A1A0'
	ldy #161
	dey
	Jne *-4
	tay
.elseif(n = 1453)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1455)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1457)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1459)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jpl *-4
	tay
.elseif(n = 1460 || n = 1476)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1462)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
.elseif(n = 1463 || n = 1472 || n = 1481)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1465)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1466 || n = 1473)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1467)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $85A0'
	ldy #133
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1469)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1471)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
.elseif(n = 1479)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.elseif(n = 1480)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485 || n = 1509)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1486 || n = 1491 || n = 1495 || n = 1498 || n = 1502 || n = 1505 || n = 1512 || n = 1514 || n = 1518 || n = 1519 || n = 1521)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1487 || n = 1494)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1488)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A5A0'
	ldy #165
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1490 || n = 1499)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1492 || n = 1513)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1493)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1497)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1500)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1501)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jmi *-4
	tay
.elseif(n = 1504)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1506)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
.elseif(n = 1507)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
.elseif(n = 1508)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1511)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1515)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A8A0'
	ldy #168
	dey
	Jne *-4
	tay
.elseif(n = 1516)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1520)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1522)
	; 9 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1523)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	tax
.endif
.else
; 1524 to 1562
.if((n .mod 7 = 5) && ((n > 1523 && n < 1560)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1525 || n = 1549 || n = 1557)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1525 && n < 1563)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1527 || n = 1548 || n = 1555)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1528)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1529)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1530)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1532)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1533 || n = 1537 || n = 1539 || n = 1540 || n = 1546 || n = 1547 || n = 1554 || n = 1556 || n = 1561)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1534)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
.elseif(n = 1536)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1541)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1542)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1543)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
.elseif(n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1551)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1558)
	; 10 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
.elseif(n = 1560)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1564)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 1565 || n = 1577)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1567 || n = 1600)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1568 || n = 1574 || n = 1575 || n = 1582 || n = 1584 || n = 1590 || n = 1593 || n = 1596)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1570)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1571 || n = 1589 || n = 1598)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1572)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1576 || n = 1583)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1578)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1579)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1581 || n = 1597)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1585 || n = 1592 || n = 1599)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1586)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1588)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1591)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1595)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 6
	dey
	Jpl *-5
	tay
.elseif(n = 1603 || n = 1609 || n = 1610 || n = 1612 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1638 || n = 1640)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1604 || n = 1611 || n = 1630 || n = 1639)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1605 || n = 1621)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1606)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	dey
	Jne *-4
	tay
.elseif(n = 1607 || n = 1616 || n = 1625)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1607 && n < 1637)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1613)
	; 7 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1614)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
.elseif(n = 1618)
	; 9 bytes
	php
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1620 || n = 1634)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1623)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $B4A0'
	ldy #180
	dey
	Jne *-4
	tay
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1628)
	; 10 bytes
	tya
	ldy #250 ;hides 'nop'
	delay_n 4
	dey
	dey
	Jne *-5
	tay
.elseif(n = 1632)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	plp
.elseif(n = 1633)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B5A0'
	ldy #181
	dey
	Jne *-4
	tay
.elseif(n = 1635)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1637)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1679)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1644 || n = 1667 || n = 1674)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1645 || n = 1659 || n = 1666 || n = 1672 || n = 1673 || n = 1675)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646)
	; 9 bytes
	php
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1647)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
.elseif(n = 1648 || n = 1654)
	; 9..10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1649)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1651)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1651 && n < 1680)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1653 || n = 1669 || n = 1677)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1655 || n = 1662 || n = 1676)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1656)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1658)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 1660)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
.elseif(n = 1663)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
.elseif(n = 1665)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1668)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $B9A0'
	ldy #185
	dey
	Jne *-4
	tay
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1682 || n = 1687 || n = 1689 || n = 1694 || n = 1701 || n = 1703 || n = 1707 || n = 1708 || n = 1710 || n = 1712)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1681 || n = 1691 || n = 1702 || n = 1716)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1683 || n = 1690 || n = 1718)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1684 || n = 1700)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 7 = 5) && ((n > 1684 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1686)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
.elseif(n = 1688 || n = 1697 || n = 1715)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1693 || n = 1717)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1695)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1696)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1698)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	nop
	iny
	Jne *-5
	tay
.elseif(n = 1704)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1705)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1709)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1711)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 1714)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1735 || n = 1736 || n = 1738 || n = 1743 || n = 1747 || n = 1750 || n = 1752 || n = 1757)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1756)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1721 || n = 1745)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1723)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BFA0'
	ldy #191
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1723 && n < 1752)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1749)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1728)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1730 || n = 1737)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1731)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
.elseif(n = 1732)
	; 9 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	iny
	Jne *-2
	pla
	tay
.elseif(n = 1739)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1740)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	iny
	Jne *-4
	tay
.elseif(n = 1744)
	; 9 bytes
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1746)
	; 9 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1753)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1754)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1756)
	; 9 bytes
	delay_n (n-3)
	nop $04
.endif
.else
; 1758 to 1796
.if(n = 1758)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
	tay
.elseif(n = 1759)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
	tay
.elseif(n = 1760 || n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1761 || n = 1764 || n = 1766 || n = 1770 || n = 1771 || n = 1782 || n = 1791 || n = 1792)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1761 && n < 1791)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1763)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1765 || n = 1773 || n = 1789)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1767)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1768)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1772)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1774 || n = 1788)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1775)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1777)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	dey
	Jne *-4
	tay
.elseif(n = 1779)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1780)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1781 || n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1784)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
	tay
.elseif(n = 1785)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1786)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	dey
	Jne *-4
	tay
.elseif(n = 1794)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1795)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	iny
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797)
	; 8 bytes
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1798 || n = 1809 || n = 1820)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1799 || n = 1807 || n = 1816 || n = 1819 || n = 1824 || n = 1825 || n = 1834)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1833)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1801 || n = 1810)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1802)
	; 10 bytes
	php
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1803)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1804)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1804 && n < 1833)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1808)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1811)
	; 10 bytes
	tya
	ldy #128
	delay_n 9
	dey
	Jpl *-4
	tay
.elseif(n = 1812)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	dey
	Jne *-4
	tay
.elseif(n = 1813 || n = 1821)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1815)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1817 || n = 1829)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1818)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $A5A0'
	ldy #165
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1822)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	iny
	Jne *-4
	tay
.elseif(n = 1826)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1827)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1828)
	; 10 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	iny ;first loop only
	iny
	Jne *-3
	pla
	tay
.elseif(n = 1830)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
.elseif(n = 1831)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	iny
	Jne *-4
	tay
.elseif(n = 1835)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	tay
	plp
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 1837 || n = 1845 || n = 1861 || n = 1869)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1838)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 1839)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1840)
	; 9 bytes
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	tax
.elseif((n .mod 9 = 5) && ((n > 1840 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1842 || n = 1864)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1843)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1844)
	; 10 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1846)
	; 10 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
.elseif(n = 1847 || n = 1851 || n = 1852 || n = 1855 || n = 1860 || n = 1863 || n = 1870 || n = 1871)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1848)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1849)
	; 9 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1853 || n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1854)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 1856)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1857)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
.elseif(n = 1858)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
.elseif(n = 1862)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $A9A0'
	ldy #169
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1866)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	dey
	Jne *-4
	tay
.elseif(n = 1867)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	iny
	Jne *-4
	tay
.elseif(n = 1872)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1873)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $AAA0'
	ldy #170
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1874)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1875)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
.if(n = 1876)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1876 && n < 1914)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1878 || n = 1887 || n = 1888 || n = 1891)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 1880 || n = 1910)
	; 11 bytes
	tya
	ldy #(n-20)/15
	rol $26
	ror $26
	dey
	Jpl *-5
	tay
.elseif(n = 1881)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1882 || n = 1890 || n = 1898)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1883)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1884)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	dey
	Jne *-4
	tay
.elseif(n = 1885 || n = 1893 || n = 1909)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1889 || n = 1901)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1892)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1894)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jne *-4
	tay
.elseif(n = 1896)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $ACA0'
	ldy #172
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1897 || n = 1908)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1899)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1900)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1902)
	; 8 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1903)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	dey
	Jne *-4
	tay
.elseif(n = 1905)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1906)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1907)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1911)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1912)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	dey
	Jne *-4
	tay
.elseif(n = 1914)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1924 || n = 1942 || n = 1943 || n = 1950 || n = 1951 || n = 1953)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1916)
	; 10 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1917 || n = 1933 || n = 1941)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1918 || n = 1927 || n = 1936)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1919)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1920)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1921)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1921 && n < 1950)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1923)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1925 || n = 1937)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1926)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 1928)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $AFA0'
	ldy #175
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1929)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1930)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
.elseif(n = 1932 || n = 1934)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1935 || n = 1945)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1938)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	dey
	Jne *-4
	tay
.elseif(n = 1939)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	iny
	Jne *-4
	tay
.elseif(n = 1944)
	; 10 bytes
	tya
	ldy #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	tay
.elseif(n = 1946)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1947)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
	tay
.elseif(n = 1948)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
	tay
.elseif(n = 1952)
	; 10 bytes
	php
	sec
	lda #216 ;hides 'cld'
	nop
	sbc #1
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954)
	; 10 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1955)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1956)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	iny
	Jne *-4
	tay
.elseif(n = 1957 || n = 1981 || n = 1989)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 1986)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 1960 || n = 1968 || n = 1969 || n = 1977 || n = 1978 || n = 1986 || n = 1987 || n = 1991)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1961 || n = 1973)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1962)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1963)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1964)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1965)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1966)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
.elseif(n = 1970)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1971)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 9
	dey
	Jpl *-5
	tay
.elseif(n = 1972)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
.elseif(n = 1974)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 1975)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
.elseif(n = 1979)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1980 || n = 1982 || n = 1988)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1983)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	iny
	Jne *-4
	tay
.elseif(n = 1984)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1990)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1992)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
.endif
.else
; 1993 to 2031
.if(n = 1993)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1993 && n < 2031)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1995 || n = 2015 || n = 2025)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 1996 || n = 2007 || n = 2018)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1997 || n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1998)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1999 || n = 2004 || n = 2014 || n = 2022 || n = 2023 || n = 2031)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2000)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2001)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
.elseif(n = 2002)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
.elseif(n = 2005 || n = 2013 || n = 2029)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 2006)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2008)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2010)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2011)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2016)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2017)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2019)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	dey
	Jne *-4
	tay
.elseif(n = 2020)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	dey
	Jne *-4
	tay
.elseif(n = 2024)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2026)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 2027)
	; 10 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
.elseif(n = 2028)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	iny
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2041 || n = 2043 || n = 2050 || n = 2059 || n = 2067 || n = 2068 || n = 2070)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2033 || n = 2045 || n = 2069)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2034)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2035 || n = 2049 || n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2036)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jmi *-5
	tay
.elseif(n = 2037)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 2038)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2038 && n < 2067)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2040 || n = 2051 || n = 2062)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2042)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 2044 || n = 2052 || n = 2054)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2046)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
.elseif(n = 2047)
	; 9 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
.elseif(n = 2053)
	; 9 bytes
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 2055)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
	tax
.elseif(n = 2056)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
.elseif(n = 2058)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
	tay
.elseif(n = 2060)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2061)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2064)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	tax
.elseif(n = 2065)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2076 || n = 2077 || n = 2086 || n = 2103 || n = 2104 || n = 2107 || n = 2108)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2072)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2073)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2074)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2074 && n < 2103)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2078)
	; 10 bytes
	php
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2079)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 2080 || n = 2088 || n = 2089 || n = 2098)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2081 || n = 2105)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2082)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 2083)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	tay
.elseif(n = 2085 || n = 2087 || n = 2094)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2090)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2091)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2092)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	tay
.elseif(n = 2095 || n = 2106)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2096)
	; 10 bytes
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2097)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2099)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2100)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
.elseif(n = 2101)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
.elseif(n = 2109)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110)
	; 9 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	tax
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2148)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2123 || n = 2144)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2113 || n = 2121 || n = 2122 || n = 2130 || n = 2131 || n = 2140 || n = 2142 || n = 2143 || n = 2148)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2115 || n = 2135)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2116 || n = 2124)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2117 || n = 2141)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2118)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 2119)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
.elseif(n = 2125)
	; 10 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	nop
	iny
	Jne *-3
	pla
	tay
.elseif(n = 2126)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2127)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2128)
	; 9 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2132)
	; 10 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2133)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2134)
	; 10 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
.elseif(n = 2136)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 2137)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
.elseif(n = 2139)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2145)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.elseif(n = 2146)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2157 || n = 2158 || n = 2162 || n = 2166 || n = 2167 || n = 2169 || n = 2176 || n = 2178 || n = 2179 || n = 2184 || n = 2185)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2150 || n = 2161)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2151)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	cmp $C5
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2152)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2153 || n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2154)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	iny
	Jne *-4
	tay
.elseif(n = 2155)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2155 && n < 2184)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2159)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2160)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2163)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
	tax
.elseif(n = 2164)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	tay
.elseif(n = 2168)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2170)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
.elseif(n = 2171)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2172)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2173)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2175)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2180)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2181)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
.elseif(n = 2182)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
.elseif(n = 2186)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2187)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jpl *-5
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2189 || n = 2213 || n = 2225)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2190)
	; 8 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 2191)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2191 && n < 2220)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2193)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2194 || n = 2205 || n = 2216)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2195)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 11
	Jne *-5
	tay
.elseif(n = 2196 || n = 2197 || n = 2206)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2198)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 2199)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2200)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
.elseif(n = 2202 || n = 2207 || n = 2211 || n = 2212 || n = 2215 || n = 2220 || n = 2221 || n = 2223)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2203)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2204)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2208)
	; 8 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 2209)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
.elseif(n = 2214)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2217)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	dey
	Jne *-4
	tay
.elseif(n = 2218)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
.elseif(n = 2222)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2224)
	; 10 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	iny
	Jne *-3
	pla
	tay
.elseif(n = 2226)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	tay
.endif
.else
; 2227 to 2265
.if(n = 2227)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2227 && n < 2265)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2229 || n = 2230 || n = 2239 || n = 2241 || n = 2248 || n = 2251 || n = 2256 || n = 2257)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2231 || n = 2259)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2232)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2233)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2234)
	; 11 bytes
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2235)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	inx
	Jne *-4
	tax
.elseif(n = 2236)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2238 || n = 2260)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2240)
	; 10 bytes
	php
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2242)
	; 10 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2243)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2244)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2245)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2247)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2249 || n = 2261)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2250)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jmi *-5
	tay
.elseif(n = 2252)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 2253)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2254)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
.elseif(n = 2258)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2262)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2263)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2265)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2274 || n = 2275 || n = 2277 || n = 2283 || n = 2284 || n = 2286 || n = 2292 || n = 2302)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267 || n = 2276 || n = 2294)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2268 || n = 2278 || n = 2279 || n = 2288 || n = 2296)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2269)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2270)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2271)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif(n = 2272)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2272 && n < 2301)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2280)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
.elseif(n = 2281)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
.elseif(n = 2285 || n = 2297)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2287 || n = 2301)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2289)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2290)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2293 || n = 2304)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2295)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2298)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2299)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2303)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2306 || n = 2316 || n = 2322)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2307)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
.elseif(n = 2308)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
.elseif(n = 2309)
	; 9 bytes
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2311)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2312)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2313)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2314)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2315)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2317)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2318)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2319)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2320)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2321)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2323)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	cmp $C5
	iny
	Jne *-4
	tay
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 9 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2325)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2326 || n = 2337)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2327)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2328)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2329)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2330)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2331)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2332)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
.elseif(n = 2333)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2334)
	; 9 bytes
	tya
	ldy #(n-2)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2335)
	; 9 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2336)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2338)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2339)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 2340)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jpl *-5
	tay
.elseif(n = 2341)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2342)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2343)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2382)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2346)
	; 9 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2347 || n = 2349 || n = 2354 || n = 2372 || n = 2377 || n = 2382)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2348 || n = 2359 || n = 2370)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2350 || n = 2352 || n = 2353 || n = 2364 || n = 2376)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2351)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2355 || n = 2365 || n = 2375)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2356)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2358)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2360)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2361)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2362 || n = 2374)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2363)
	; 11 bytes
	php
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 2366)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2367)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2368)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 2371)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2373)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2378)
	; 11 bytes
	tya
	ldy #(n-26)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
	tay
.elseif(n = 2379)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2380)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2387 || n = 2394 || n = 2395 || n = 2407 || n = 2409 || n = 2416 || n = 2419 || n = 2421)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2385 || n = 2399 || n = 2413)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2386 || n = 2398 || n = 2410)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2388 || n = 2400)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2389)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2390)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2391)
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	cmp $C5
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2392 || n = 2403 || n = 2414)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2393 || n = 2405 || n = 2417)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2396 || n = 2408)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2397)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2401)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2402)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2404)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2406)
	; 11 bytes
	php
	sec
	lda #(n-8)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2411)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	iny
	Jne *-6
	tay
.elseif(n = 2412)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2415)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2418)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2420)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2423)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2424)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	tay
.elseif((n .mod 11 = 5) && ((n > 2424 && n < 2459)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2426 || n = 2431 || n = 2433 || n = 2438 || n = 2443 || n = 2459)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427 || n = 2455)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2429 || n = 2441 || n = 2453)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2430)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2432 || n = 2442 || n = 2444)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2434)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2435 || n = 2445)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2437)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2440)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2446 || n = 2448 || n = 2460)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2449)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2450)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	tay
.elseif(n = 2451)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 2452)
	; 11 bytes
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	cmp $C5
	dex
	Jne *-6
	tax
.elseif(n = 2454)
	; 11 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2456)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2457)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2462)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2463)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
	iny
	Jne *-5
	tay
.elseif(n = 2464)
	; 11 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2465 || n = 2477 || n = 2489)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2466 || n = 2467 || n = 2471 || n = 2479 || n = 2481 || n = 2493 || n = 2499)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2468)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2469 || n = 2480 || n = 2491)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2470 || n = 2487)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2472 || n = 2484 || n = 2496 || n = 2498)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2473)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $BEA0'
	ldy #190
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2474)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2475 || n = 2485 || n = 2495)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2476)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2478)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2482)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2483 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2486)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $BFA0'
	ldy #191
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2488)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2490)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2492)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2494)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2500)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	nop
	iny
	Jne *-5
	tay
.endif
.endif
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
.if((n .mod 12 = 5) && ((n > 2500 && n < 2538)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2502 || n = 2524 || n = 2535)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2503)
	; 9 bytes
	tya
	ldy #250 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	tay
.elseif(n = 2504 || n = 2521 || n = 2538)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2505)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2506 || n = 2518 || n = 2530)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2507 || n = 2515 || n = 2517 || n = 2526 || n = 2527 || n = 2529)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2508 || n = 2520)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2509)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 5
	inx
	inx
	Jne *-6
	tax
.elseif(n = 2510)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2511 || n = 2539)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2512)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2514)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2516 || n = 2528)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2519)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2522)
	; 10 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2523)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2531)
	; 11 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2532)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2533)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2534)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2536)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	cmp $C5
	dey
	Jne *-6
	tay
.endif
.else
; 2540 to 2578
.if(n = 2540)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2541)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2542 || n = 2554)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2543 || n = 2548 || n = 2551 || n = 2563 || n = 2569 || n = 2574 || n = 2575)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2544)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
.elseif(n = 2545)
	; 10 bytes
	tya
	ldy #(n-5)/10
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2546 || n = 2557 || n = 2568)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2547)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2549 || n = 2561 || n = 2573)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2550)
	; 12 bytes
	php
	sec
	lda #n/10-1
	clv
	Jvc *+2
	sbc #1
	Jne *-5
	plp
.elseif(n = 2552 || n = 2564 || n = 2576)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2553 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2555)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2556)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2558)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
.elseif(n = 2559)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2560)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 2562)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2565)
	; 10 bytes
	tya
	ldy #0
	delay_n 5
	dey
	Jne *-4
	tay
.elseif(n = 2566)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2570)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2571)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2572)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2577)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2578)
	; 10 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	tax
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if((n .mod 11 = 5) && ((n > 2578 && n < 2613)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2584 || n = 2592 || n = 2604 || n = 2616)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2581 || n = 2595)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2582)
	; 11 bytes
	php
	sec
	lda #(n-8)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2583 || n = 2587 || n = 2611 || n = 2613)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2585 || n = 2597 || n = 2609)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2586)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
	tay
.elseif(n = 2588)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2589)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2591)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2593)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2594)
	; 11 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2596)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2598)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 2599)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2600)
	; 10 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2602 || n = 2614)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2603)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2605)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2606)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	tay
.elseif(n = 2607)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2608)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2610)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2615)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2617)
	; 11 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2631)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2619)
	; 10 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	tay
.elseif(n = 2620)
	; 11 bytes
	php
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 2621 || n = 2633 || n = 2645)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2622)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2623 || n = 2634 || n = 2656)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2624 || n = 2648)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2627 || n = 2646)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2628 || n = 2641 || n = 2652)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2629)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2630)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2632)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2635 || n = 2636 || n = 2639 || n = 2647 || n = 2649)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2640)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2642)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	tax
.elseif(n = 2643)
	; 10 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2644)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2653)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 2654)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2655)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if((n .mod 12 = 5) && ((n > 2656 && n < 2694)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2658 || n = 2659 || n = 2661 || n = 2671 || n = 2673 || n = 2680 || n = 2683 || n = 2695)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675 || n = 2690)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2662)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2663 || n = 2664)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2665 || n = 2679)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2666)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2667 || n = 2678 || n = 2689)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2668)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 2670)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2672)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2674 || n = 2691)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2676)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2677)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2682)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2684)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2686)
	; 9 bytes
	tya
	ldy #(n-2)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2687)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2688)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2692)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2694)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	delay_n 4
	dey
	Jne *-6
	tay
.endif
.else
; 2696 to 2734
.if(n = 2696)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2697 || n = 2701 || n = 2702 || n = 2713 || n = 2718 || n = 2719 || n = 2723 || n = 2726 || n = 2731)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2699)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
.elseif((n .mod 11 = 5) && ((n > 2699 && n < 2734)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2703)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2704)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2705 || n = 2717 || n = 2729)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2706 || n = 2712 || n = 2724)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2707 || n = 2721)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2708 || n = 2725)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2709)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2710)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2714)
	; 11 bytes
	tya
	ldy #(n-26)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
	tay
.elseif(n = 2715)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2716)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2720)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2727)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2728)
	; 11 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	cmp $C5
	inx
	Jne *-6
	tax
.elseif(n = 2730)
	; 12 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	iny ;first loop only
	pla
	pha
	iny
	Jne *-5
	pla
	tay
.elseif(n = 2732)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2734)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735 || n = 2749)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2736 || n = 2748)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2737 || n = 2751)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2738)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2739)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 2740)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2741 || n = 2753)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2742)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2743)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	tay
.elseif(n = 2744)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2745)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2746)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2747)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2750)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 2752)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	cmp $C5
	iny
	Jne *-6
	tay
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2755 || n = 2766)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2756)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2757)
	; 10 bytes
	tya
	ldy #(n-1)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2758)
	; 10 bytes
	tya
	ldy #(n-2)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2759)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2760)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2761)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2762)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 2763)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2764)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2765)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n > 2766 && n < 2770))
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2770)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2771)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2772)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2800)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2775)
	; 9 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2776)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2777 || n = 2789 || n = 2801)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2778 || n = 2779 || n = 2781 || n = 2790 || n = 2807 || n = 2812)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2780 || n = 2795)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2782 || n = 2794 || n = 2806)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2783 || n = 2784 || n = 2796)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2785)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2786)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2787)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 5
	iny
	Jne *-5
	tay
.elseif(n = 2788 || n = 2799 || n = 2810)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2791 || n = 2805)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2792 || n = 2802 || n = 2804)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2797)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2798)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2803)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2808)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2809)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2811)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 5
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if((n .mod 12 = 5) && ((n > 2812 && n < 2850)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2814)
	; 10 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2815 || n = 2822 || n = 2827 || n = 2829 || n = 2831 || n = 2839 || n = 2841 || n = 2846 || n = 2851)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2848)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2817 || n = 2836)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2818 || n = 2830 || n = 2842)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2819 || n = 2833 || n = 2847)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2820)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2821)
	; 10 bytes
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2823)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	iny
	Jne *-5
	tay
.elseif(n = 2824)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2826)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2832)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2834)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	tay
.elseif(n = 2835)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop $04
	iny
	Jne *-5
	tay
.elseif(n = 2838)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2840)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2843)
	; 11 bytes
	tya
	ldy #128
	nop
	jsr @nearby_rts_15cyc
	dey
	Jpl *-5
	tay
.elseif(n = 2844)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2845)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
.elseif(n = 2850)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2856 || n = 2868)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2853 || n = 2857 || n = 2862 || n = 2863 || n = 2879 || n = 2886 || n = 2887)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854 || n = 2866 || n = 2890)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2855 || n = 2874)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2858)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	iny
	Jne *-6
	tay
.elseif(n = 2859)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2860)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2861 || n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2864 || n = 2876 || n = 2881 || n = 2888)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2867)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2869)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2870)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2871)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2872)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2875 || n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2877)
	; 11 bytes
	php
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 2878)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2880)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tay
.elseif(n = 2882)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 2883)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	cmp $C5
	inx
	Jne *-6
	tax
.elseif(n = 2884)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	cmp $C5
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2892 || n = 2904 || n = 2910 || n = 2916 || n = 2928)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2893)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2894 || n = 2898 || n = 2899 || n = 2901 || n = 2906 || n = 2911 || n = 2913 || n = 2918 || n = 2923 || n = 2927)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895 || n = 2912 || n = 2929)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2896)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2897 || n = 2909 || n = 2921)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2900 || n = 2915)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2902 || n = 2914 || n = 2926)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2903 || n = 2917)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2905 || n = 2925)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2907)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2908)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2919)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
	dey
	Jne *-5
	tay
.elseif(n = 2920)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2922)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 2924)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 2930 to 2968
.if(n = 2930)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	tay
.elseif(n = 2931 || n = 2959)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2932)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2933 || n = 2945 || n = 2957)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2934 || n = 2936 || n = 2948)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2935 || n = 2937 || n = 2939 || n = 2947 || n = 2949 || n = 2958)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2938 || n = 2962)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2940 || n = 2952 || n = 2964)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2941)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2942)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2943)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	dey
	Jne *-5
	tay
.elseif(n = 2944)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 2946 || n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2950)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2951)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 2953)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 2954)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2955)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2956)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2960)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2961)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2966)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 2967)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2968)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	cmp $C5
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if((n .mod 12 = 5) && ((n > 2968 && n < 3006)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2970)
	; 11 bytes
	tya
	ldy #(n-2)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dey
	Jne *-6
	tay
.elseif(n = 2971 || n = 2977 || n = 2978 || n = 2983 || n = 2989 || n = 2995 || n = 3002 || n = 3007)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2984 || n = 2996)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2973 || n = 2987 || n = 3001)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2974)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 2975 || n = 2990)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2976 || n = 3000)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2979)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	cmp $C5
	inx
	Jne *-6
	tax
.elseif(n = 2980 || n = 2997)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2982)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2986 || n = 2998)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2988)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2991)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2992)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2994)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2999)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 5
	dey
	Jne *-6
	tay
.elseif(n = 3003)
	; 10 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	tay
.elseif(n = 3004)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3006)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3021)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3009 || n = 3023 || n = 3030 || n = 3031 || n = 3033 || n = 3037 || n = 3038 || n = 3042)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010 || n = 3022 || n = 3034 || n = 3046)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3011)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $BCA0'
	ldy #188
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3012 || n = 3024 || n = 3036)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3013)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3014)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3015 || n = 3043)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3016)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3017 || n = 3029 || n = 3041)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3018 || n = 3044)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3019)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3020 || n = 3035)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3025)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3026)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3027)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3028)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3032)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3039)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3040)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3045)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047 || n = 3073)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3048)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
	tax
.elseif(n = 3049 || n = 3054 || n = 3059 || n = 3066 || n = 3067 || n = 3074 || n = 3079 || n = 3081)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3050 || n = 3080)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3051)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3052)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3055)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3056 || n = 3068)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3057 || n = 3071 || n = 3085)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3058)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 3060 || n = 3062 || n = 3072)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3061)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3063)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3064 || n = 3083)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3069)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 3070)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3075)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 3076)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3078)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3082)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3084)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3112)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif((n > 3086 && n < 3090) || n = 3096 || n = 3100 || n = 3101 || n = 3103 || n = 3104 || n = 3114 || n = 3115 || n = 3118 || n = 3122)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3090)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3091)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3092)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3093 || n = 3109)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3094)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3095 || n = 3110)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3097)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3098)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3099 || n = 3113)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3102 || n = 3121)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3105 || n = 3125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3106 || n = 3120)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3107)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $C2A0'
	ldy #194
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3108)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $3EA0'
	ldy #62
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3111)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3117)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3119)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3123)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 4
	inx
	Jne *-6
	tax
.elseif(n = 3124)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
.if((n >= 3126) && (n < 3165))
; 3126 to 3164
.if(n = 3126)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3127 || n = 3141 || n = 3155)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3128 || n = 3148)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3129 || n = 3131 || n = 3135 || n = 3142 || n = 3143 || n = 3147 || n = 3152 || n = 3153 || n = 3161)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3130 || n = 3158)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3132 || n = 3146 || n = 3160)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3133 || n = 3150)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3134)
	; 11 bytes
	php
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3136)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3137)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3138 || n = 3151)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3139)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3140 || n = 3159)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3144)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3145)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3149)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3154)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	tay
.elseif(n = 3156)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3157)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 3162)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 3163)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3164)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3166 || n = 3179 || n = 3180)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3167)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3168)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3169 || n = 3183)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3170)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3171)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3172)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3173)
	; 10 bytes
	tya
	ldy #(n-1)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 3174)
	; 10 bytes
	tya
	ldy #(n-2)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3175)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3176)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3177)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3178)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3181)
	; 11 bytes
	tya
	ldy #(n-1)/15 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3182)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
.endif
.else
; 3184 to 3203
.if(n = 3184 || n = 3201)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3185)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3186 || n = 3192 || n = 3199)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3187)
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
.elseif(n = 3188)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 3189)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3190)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3191)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3193)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3194)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3195)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3196)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3197)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3198)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 3200)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3202)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
.if((n >= 3204) && (n < 3223))
; 3204 to 3222
.if(n = 3204)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3206)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3207 || n = 3213 || n = 3222)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3208)
	; 12 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 6
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3209)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3210 || n = 3212)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3211)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3214)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3215)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3216)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3217)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3218)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3219)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3220)
	; 11 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3221)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.endif
.else
; 3223 to 3242
.if(n = 3223)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3224)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3225 || n = 3239)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3226)
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3227 || n = 3231 || n = 3241)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3228)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3229 || n = 3242)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3230)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3232)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3233)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3234)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3235)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3236)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3237)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3238)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3240)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3247 || n = 3249 || n = 3257 || n = 3264 || n = 3270 || n = 3271 || n = 3277)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244 || n = 3272)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3245 || n = 3265)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3246)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3248)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 3250)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3252 || n = 3269)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3253 || n = 3267 || n = 3281)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3254 || n = 3273)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3255 || n = 3268)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3256)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	plp
.elseif(n = 3258 || n = 3274 || n = 3276)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3259)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3260 || n = 3275)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3261)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3262)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3263)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3266)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3278)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3280)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
.if((n >= 3282) && (n < 3301))
; 3282 to 3300
.if(n = 3282 || n = 3298)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3283)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3284)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3285)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3286)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3287 || n = 3288 || n = 3297)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3289)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 3290)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3291)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts
	Jne *-6
	tay
.elseif(n = 3292)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3293)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3294)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3295)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3296)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3299)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3300)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
; 3301 to 3320
.if(n = 3301 || n = 3317)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3302)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3303 || n = 3320)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3304)
	; 12 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	plp
.elseif(n = 3305)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3306)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3307)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3308 || n = 3313)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3309)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3310)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3311)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3312)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3314)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3315)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3316)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3318)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3319)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.endif
.else
; 3321 to 3359
.if(n = 3321)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop $04
	iny
	Jne *-6
	tay
.elseif(n = 3322 || n = 3324 || n = 3334 || n = 3338 || n = 3339 || n = 3341 || n = 3346 || n = 3353 || n = 3355 || n = 3356)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3323 || n = 3337 || n = 3351)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3325 || n = 3345)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3326)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3327)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3328 || n = 3342)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3329)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3330 || n = 3349)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3331)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3332)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3333)
	; 11 bytes
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3335 || n = 3350)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3336)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3340 || n = 3357)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3343)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $9FA0'
	ldy #159
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3344)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3347)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3348)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3352)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3354)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3358)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3359)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3366 || n = 3367 || n = 3373 || n = 3375 || n = 3382 || n = 3389 || n = 3392)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3361 || n = 3372)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3362)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 3363)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3364)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3365 || n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3368 || n = 3387)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3369)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
.elseif(n = 3370)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3371 || n = 3388)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3374)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3376)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3377)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3378)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3380)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3381 || n = 3397)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3383)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3384 || n = 3398)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3385)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3386)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3390)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3391)
	; 11 bytes
	php
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3394)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3396)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 7
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 3399) && (n < 3418))
; 3399 to 3417
.if(n = 3399 || n = 3408 || n = 3409)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3400 || n = 3402)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3401)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3403)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
.elseif(n = 3404)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	inx
	inx
	Jne *-7
	tax
.elseif(n = 3405)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3406)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3407)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3410)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3411)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3412)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3413)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3414)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3415)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3416)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3417)
	; 12 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	cmp $C5
	dey
	Jne *-7
	tay
.endif
.else
; 3418 to 3437
.if(n = 3418)
	; 11 bytes
	tya
	ldy #(n-2)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dey
	Jne *-6
	tay
.elseif(n = 3419)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	dey
	Jne *-6
	tay
.elseif(n = 3420)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3421 || n = 3435)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3422)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3423 || n = 3424 || n = 3437)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3425)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3426)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3427)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3428)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3429)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3430)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3431)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3432)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3433)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop $04
	dey
	Jne *-6
	tay
.elseif(n = 3434)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3436)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3452 || n = 3457)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3439 || n = 3456 || n = 3473)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3440 || n = 3455 || n = 3470)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3441 || n = 3446 || n = 3447 || n = 3451 || n = 3458 || n = 3467 || n = 3472)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3442)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3443)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3444)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3445 || n = 3465)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3448)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3449 || n = 3463)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3450)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3453)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3454)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
.elseif(n = 3459)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3460)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3461)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3462)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 5
	dey
	Jne *-7
	tay
.elseif(n = 3464)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3466)
	; 12 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3468)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3469)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3471)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
.elseif(n = 3474)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3475)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3476)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3491 || n = 3505)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3478 || n = 3479 || n = 3484 || n = 3486 || n = 3487 || n = 3502 || n = 3511)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3480)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3481)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3482 || n = 3501)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3483)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3488 || n = 3494)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3489)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 3490 || n = 3507)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3492)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3493 || n = 3509)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3495)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3496)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3497)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3498 || n = 3510)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3499)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3500 || n = 3515)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3503)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3504)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3506)
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3508)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3512)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3513)
	; 11 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 3514)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 6
	inx
	Jne *-7
	tax
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3548)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3517 || n = 3521 || n = 3522 || n = 3526 || n = 3527 || n = 3532 || n = 3535 || n = 3537 || n = 3543 || n = 3549 || n = 3551)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3518)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3519 || n = 3533 || n = 3547)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3520 || n = 3539)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3523)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3524 || n = 3541)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3525 || n = 3545)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3528)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3529)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3530)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3531)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3534)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3536 || n = 3550)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3538 || n = 3552)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3540)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3542)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3544)
	; 12 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 3546)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3553)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $A9A0'
	ldy #169
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3554)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.endif
.else
; 3555 to 3593
.if(n = 3555)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3556)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3559 || n = 3562 || n = 3563 || n = 3567 || n = 3579 || n = 3591 || n = 3593)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3560 || n = 3590)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3561 || n = 3575)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3564 || n = 3578)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3565 || n = 3585)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3566 || n = 3580)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3568 || n = 3582 || n = 3584)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3569)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3570)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3571)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3572)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3574)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 5
	iny
	Jne *-7
	tay
.elseif(n = 3576)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 13
	iny
	Jne *-7
	tay
.elseif(n = 3577)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3581)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3583)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
	tax
.elseif(n = 3586)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3587)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3588)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3589)
	; 10 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3598 || n = 3600 || n = 3606 || n = 3607 || n = 3611 || n = 3622 || n = 3627 || n = 3628 || n = 3630)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3608 || n = 3612 || n = 3629)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3596 || n = 3615)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3597 || n = 3614 || n = 3631)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3599 || n = 3632)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3601)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3602)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 11
	iny
	Jne *-6
	tay
.elseif(n = 3604)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3605 || n = 3625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3609 || n = 3626)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3610)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3613)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3616)
	; 11 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3617)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3618)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3619)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3620)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3621)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3623)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3624)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
.endif
.else
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3634)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3635 || n = 3650)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3636)
	; 11 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3637)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3638)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3639 || n = 3647)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3640)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3641)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3642)
	; 12 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 3643)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3644)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3646)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3648)
	; 11 bytes
	php
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3649)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 3651)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
	tax
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3653)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3654 || n = 3662 || n = 3671)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3655)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 3656)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3657)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
	tax
.elseif(n = 3658)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3659)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3660)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3661)
	; 11 bytes
	tya
	ldy #(n-1)/15 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3663)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 3664)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3665)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3666)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3667)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3668)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3669)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3670)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3691 || n = 3710)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3673)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3674 || n = 3679 || n = 3681 || n = 3682 || n = 3686 || n = 3687 || n = 3689 || n = 3693 || (n > 3695 && n < 3699) || n = 3703 || n = 3707)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3676)
	; 12 bytes
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	delay_n 9
	dex
	Jne *-7
	tax
.elseif(n = 3677 || n = 3694)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3678)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3680 || n = 3695)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3683)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3684)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3685 || n = 3705)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3688)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3690)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3692 || n = 3706)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3699)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3700)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3701)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3702)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tay
.elseif(n = 3704)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 3708)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 3709)
	; 11 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.endif
.else
; 3711 to 3750
.if(n = 3711 || n = 3728 || n = 3745)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3712 || n = 3713 || n = 3719 || n = 3727 || n = 3730 || n = 3734 || n = 3735 || n = 3742 || n = 3750)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3714 || n = 3718)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3716)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3717 || n = 3733 || n = 3749)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3720)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 3721)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $B1A0'
	ldy #177
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3722 || n = 3743)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3723)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3724)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3726 || n = 3737)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3729 || n = 3748)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3731)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3732)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3736)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3738)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3739)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3740)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3741)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $B2A0'
	ldy #178
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3744)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3746)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3747)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.endif
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
.if((n >= 3751) && (n < 3790))
.if((n >= 3751) && (n < 3770))
; 3751 to 3769
.if(n = 3751 || n = 3757 || n = 3769)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3752)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3753)
	; 11 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 3754 || n = 3756)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3755)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3758)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 3759)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3760)
	; 13 bytes
	php
	sec
	lda #250
	rol $26
	ror $26
	sbc #1
	Jne *-6
	plp
.elseif(n = 3761)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	pla
	pha
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3762)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3763)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3764)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3766)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 11
	dey
	Jne *-7
	tay
.elseif(n = 3767)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3768)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 6
	iny
	Jne *-7
	tay
.endif
.else
; 3770 to 3789
.if(n = 3770)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3771)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3772 || n = 3776 || n = 3782 || n = 3783 || n = 3787 || n = 3788)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3773)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3774)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3775)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 3777)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 3778)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	iny
	Jne *-7
	tay
.elseif(n = 3779)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3780)
	; 11 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3781)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3784)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3786)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3789)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3791)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	pla
	pha
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3792 || n = 3793)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3794)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3795)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3796)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3797)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3798 || n = 3799 || n = 3802)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3800)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3801)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3803)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 3804)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $B5A0'
	ldy #181
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3805)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3806)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3807)
	; 11 bytes
	tya
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	nop
.elseif(n = 3808)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 3809 to 3828
.if(n = 3809 || n = 3817 || n = 3826)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3810)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3811)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3812)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3813)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3814)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3815)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3816)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3818)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3819)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3820)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3821)
	; 11 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3822)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $A6A0'
	ldy #166
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 3823)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 3824)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3827)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3828)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	delay_n 7
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3861)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3830 || n = 3847 || n = 3864)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3831 || n = 3834 || n = 3838 || n = 3846 || n = 3849 || n = 3851 || n = 3866)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3832)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3833)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3835 || n = 3852)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3836)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3837)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 3839 || n = 3857)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3840 || n = 3841 || n = 3858)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3842)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	tay
.elseif(n = 3843 || n = 3862)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3844)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3845 || n = 3865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3848)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3850 || n = 3863)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3853)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 3854 || n = 3867)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3855)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3856)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3859)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3860)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3878 || n = 3879 || n = 3883 || n = 3887 || n = 3894 || n = 3895 || n = 3904)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3869 || n = 3890)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3870)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3871 || n = 3872 || n = 3888)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3873)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3874)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3875)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3876)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3877 || n = 3893)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3880 || n = 3897)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3881 || n = 3898)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3882)
	; 13 bytes
	php
	sec
	lda #242
	delay_n 11
	sbc #1
	Jne *-6
	plp
.elseif(n = 3884 || n = 3901)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3885 || n = 3905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3886 || n = 3903)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3889)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $B9A0'
	ldy #185
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3891)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3892)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3896)
	; 13 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 11
	sbc #1
	Jne *-7
	plp
.elseif(n = 3899)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3900)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3902)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3906)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
.if((n >= 3907) && (n < 3926))
; 3907 to 3925
.if(n = 3907)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3908)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3909)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3910)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3911)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3912 || n = 3922)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3913)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
.elseif(n = 3914)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 3915)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3916)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $A3A0'
	ldy #163
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 3917)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3918)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3919)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3920)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3921)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3923)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3924)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3925)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.endif
.else
; 3926 to 3945
.if(n = 3926 || n = 3927 || n = 3934 || n = 3942)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3928)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3929)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3930)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
	tax
.elseif(n = 3931)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $BBA0'
	ldy #187
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3932)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3933)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3935)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3936)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3937)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3938)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3939)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3940)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3941)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3943)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3944)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.endif
.endif
.else
; 3946 to 3984
.if(n = 3946)
	; 12 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	pla
	pha
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3947)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
.elseif(n = 3948 || n = 3952 || n = 3960 || n = 3969 || n = 3979)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3949 || n = 3966 || n = 3983)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3950)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3951 || n = 3958 || n = 3959 || n = 3961 || n = 3967 || n = 3968 || n = 3970 || n = 3975 || n = 3978 || n = 3980)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3953 || n = 3974)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3954)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3955)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3956)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3957 || n = 3976)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3962)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 3963)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3964)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3971)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3972)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3973)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3977)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3981)
	; 11 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.elseif(n = 3982)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3984)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985 || n = 4005)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3986 || n = 3998 || n = 4008)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3987)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3988)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3989 || n = 4021)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3990 || n = 3991 || n = 3997 || n = 4002 || n = 4006 || n = 4007 || n = 4011 || n = 4022 || n = 4023)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3992 || n = 3996 || n = 4012)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3993)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3994)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3995 || n = 4014)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3999)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4000 || n = 4017)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4001)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4003)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4004)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4009)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4010)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4013)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4015)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $BFA0'
	ldy #191
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4016)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4018)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4019)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4020)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4032 || (n > 4039 && n < 4045) || n = 4059)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4025 || n = 4045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4026)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	tay
.elseif(n = 4027 || n = 4029 || n = 4038 || n = 4039 || n = 4047 || n = 4054 || n = 4060)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4028 || n = 4048)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4030)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4031)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4033 || n = 4052)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4035)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4036)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4037 || n = 4053)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 4046 || n = 4062)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4049)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4050)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
.elseif(n = 4055)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4056)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4057)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4058)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4061)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
.elseif(n = 4064 || n = 4080 || n = 4096)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4066 || n = 4074 || n = 4088 || n = 4093)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4067)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4068 || n = 4085)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4069)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 4070 || n = 4073 || n = 4081 || n = 4086 || n = 4087 || n = 4092 || n = 4094)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4071 || n = 4090)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4072)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4075 || n = 4097)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4076)
	; 12 bytes
	tya
	pha
	ldy #42 ;hides 'rol a'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
.elseif(n = 4077)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $C2A0'
	ldy #194
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4078)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $3EA0'
	ldy #62
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4079)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4082)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4083)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4084)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4089)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4091)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4095)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4098)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $C3A0'
	ldy #195
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4099)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4100)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4101)
	; 11 bytes
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4119 || n = 4136)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4103 || n = 4104 || n = 4106 || n = 4107 || n = 4111 || n = 4113 || n = 4121 || n = 4123 || n = 4127 || n = 4129 || n = 4130 || n = 4138)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4108)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4109 || n = 4128)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4112)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4114)
	; 12 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4115 || n = 4117 || n = 4137)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4116 || n = 4132 || n = 4135)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4118)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4120)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	tax
.elseif(n = 4122 || n = 4139)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4124)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4126)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4131)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4133)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4134)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 8
	dey
	Jne *-7
	tay
.elseif(n = 4140)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4142 || n = 4163)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4143)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 11
	iny
	Jne *-6
	tay
.elseif(n = 4144)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4145 || n = 4165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4146 || n = 4159 || n = 4176 || n = 4178)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4147 || n = 4166)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4148 || n = 4156 || n = 4173)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4149 || n = 4151 || n = 4155 || n = 4157 || n = 4164 || (n > 4166 && n < 4170) || n = 4172 || n = 4174)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4150)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4152 || n = 4154 || n = 4160 || n = 4177)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4153 || n = 4170)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4158 || n = 4175)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4161)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4162)
	; 11 bytes
	php
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4171)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4179)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 11
	iny
	Jne *-6
	tay
.endif
.else
; 4180 to 4218
.if(n = 4180)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4181 || n = 4199 || n = 4217)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4182)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	tay
.elseif(n = 4183)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4184)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4185 || n = 4205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4186 || n = 4189 || n = 4191 || n = 4206 || n = 4208)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4187 || n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4188 || n = 4190)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4192 || n = 4209)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4193 || n = 4195 || n = 4196 || (n > 4199 && n < 4203) || n = 4218)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4194 || (n > 4209 && n < 4213))
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4197)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4198)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4207)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4213)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4214)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4215)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 4216)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 9
	inx
	Jne *-7
	tax
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4239 || n = 4254 || n = 4256)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4220)
	; 13 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 11
	sbc #1
	Jne *-7
	plp
.elseif(n = 4221 || n = 4238 || n = 4255)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4222 || n = 4228 || n = 4230 || n = 4232 || n = 4249)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4223 || n = 4242)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4224 || n = 4241 || n = 4248)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4225 || n = 4245)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4226 || n = 4247)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4227 || n = 4231 || n = 4240 || n = 4244 || n = 4257)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4229 || n = 4251)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4233)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4234)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4235)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4236)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4237)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4243)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 11
	dey
	Jne *-7
	tay
.elseif(n = 4246)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 4250)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4252)
	; 12 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4253)
	; 11 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.endif
.else
; 4258 to 4296
.if(n = 4258)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dey
	Jne *-7
	tay
.elseif(n = 4259 || n = 4263 || n = 4267 || n = 4274 || n = 4276 || n = 4282 || n = 4284 || n = 4287 || n = 4291 || n = 4293)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4260 || n = 4277 || n = 4294)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4261 || n = 4280)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4262)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4264 || n = 4275 || n = 4292)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4265 || n = 4285)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4266)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tax
.elseif(n = 4268)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4269)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4270)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4271)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4272 || n = 4289)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4273 || n = 4295)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4278)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4279 || n = 4296)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4281)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $BAA0'
	ldy #186
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4283)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 11
	dey
	Jne *-7
	tay
.elseif(n = 4286)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4288)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4290)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297 || n = 4301 || n = 4303 || n = 4307 || n = 4308 || n = 4319 || n = 4320 || n = 4322 || n = 4327)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4298)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4299 || n = 4318)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4300)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $B3A0'
	ldy #179
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4302 || n = 4309 || n = 4321 || n = 4326)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4304)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4305 || n = 4325)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4306 || n = 4323)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4310 || n = 4331)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4311)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4312 || n = 4313 || n = 4330 || n = 4332)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4314 || n = 4315 || n = 4334 || n = 4335)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4316)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4317)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4324)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4328)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4329)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4333)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.endif
.else
; 4336 to 4375
.if(n = 4336 || n = 4346 || n = 4355)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4337 || n = 4356 || n = 4375)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4338)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4339 || n = 4361)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4341)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4342 || n = 4344 || n = 4347 || n = 4353 || n = 4354 || n = 4358 || n = 4367 || n = 4369)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4343)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4345 || n = 4365)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4348 || n = 4368)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4349)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4350)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4351)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4352)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4357)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4359)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4360)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4362)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4363 || n = 4364 || n = 4366)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4370)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4371)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4372)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4373)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 4374)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
.if((n >= 4376) && (n < 4415))
; 4376 to 4414
.if(n = 4376)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 4377 || n = 4387 || n = 4389 || n = 4396 || n = 4398 || n = 4407 || n = 4409)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4379 || n = 4397)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4380 || n = 4399)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4381 || n = 4384 || n = 4386 || n = 4391 || n = 4400 || n = 4402 || n = 4404)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4382 || n = 4401 || n = 4410 || n = 4412)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4383)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4385 || n = 4405)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4388 || n = 4408)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4390)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 4392)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4393)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4394 || n = 4413)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4395)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 4403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4406)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4411)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 4414)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 9
	iny
	Jne *-7
	tay
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4436)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4416)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	plp
.elseif(n = 4417 || n = 4429)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4418 || n = 4420 || n = 4439)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4419)
	; 11 bytes
	php
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4421 || n = 4434 || n = 4438 || n = 4447 || n = 4453)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n > 4421 && n < 4425) || n = 4426 || n = 4440 || n = 4441 || n = 4446)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4425 || n = 4445)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4427)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4428 || n = 4448 || n = 4452)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4430)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4431)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4432 || n = 4451)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4433)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4435)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4437)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4442)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4443)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4444)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4450)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 9
	dey
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4455)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4456 || n = 4458 || n = 4492)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4457 || n = 4478)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4459 || n = 4467 || n = 4472 || n = 4474 || n = 4480 || n = 4487 || n = 4491)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4460)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif((n > 4460 && n < 4465) || n = 4479 || n = 4481 || n = 4483 || n = 4484)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4465 || n = 4485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4466)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $3EA0'
	ldy #62
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4468)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4469)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	tax
.elseif(n = 4470 || n = 4489)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4471)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4473 || n = 4488)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4475)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4476)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4477)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4482)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	iny
	Jne *-7
	tay
.elseif(n = 4486)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4490)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.endif
.else
; 4493 to 4531
.if(n = 4493 || n = 4515)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4494 || n = 4513)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4495 || n = 4501 || n = 4507 || n = 4509 || n = 4510 || n = 4529 || n = 4531)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4496)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4497)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4498)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4499 || n = 4520)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4500 || n = 4502 || n = 4526)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4503)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 4504)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4505 || n = 4525)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4506)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	tay
.elseif(n = 4508 || n = 4527)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4511 || n = 4528)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4512)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4514)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4516)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4517)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4518)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4519)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4521)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4522)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4523)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4524)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_14cyc
	Jne *-7
	tay
.elseif(n = 4530)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4534 || n = 4552 || n = 4553)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4533 || n = 4538 || n = 4542 || n = 4554 || n = 4556 || n = 4566)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4535)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4536)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4537 || n = 4559)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4539)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4540)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4541 || n = 4562)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4543 || n = 4547 || n = 4548 || n = 4564 || n = 4569)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4544)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4545 || n = 4565)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4546)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4549 || n = 4568)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4550 || n = 4570)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4551)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4555)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4557)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4558)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4560)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4561)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4563)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $BEA0'
	ldy #190
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4567)
	; 11 bytes
	tya
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	nop
.endif
.else
; 4571 to 4609
.if(n = 4571 || n = 4573 || n = 4574 || n = 4578 || n = 4579 || (n > 4595 && n < 4602))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4572)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4575)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4576)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4577 || n = 4595)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4580)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
	tay
.elseif(n = 4581)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4582)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4583 || n = 4604)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4584 || n = 4603)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4585 || n = 4605)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4586 || n = 4587 || n = 4607 || n = 4609)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4588 || n = 4606)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4589)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4590)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4591 || n = 4592)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4593)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4594)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4602)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4608)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4629 || n = 4632 || n = 4648)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4611)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4612)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4613)
	; 12 bytes
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-6
	tay
.elseif((n > 4613 && n < 4620) || n = 4621 || n = 4631 || n = 4640 || n = 4642)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4620)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	rol $26
	ror $26
	dey
	Jne *-7
	tay
.elseif(n = 4622 || n = 4641)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4623)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4624 || n = 4626 || n = 4627 || n = 4643)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4625 || n = 4645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4628)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4630)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4633)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4634)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	tay
.elseif(n = 4635)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4636)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4637)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4638)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4639)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
.elseif(n = 4644)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4646)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4647)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4650 || n = 4670)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4651 || n = 4674)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4652)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n > 4652 && n < 4657) || n = 4675 || n = 4677 || n = 4680)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4657)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	inx
	inx
	Jne *-7
	tax
.elseif(n = 4658)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	rol $26
	ror $26
	dey
	Jne *-7
	tay
.elseif(n = 4659)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4660 || n = 4679)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4661)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4662 || n = 4664 || n = 4667 || n = 4671 || n = 4678 || n = 4681 || n = 4687)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4663 || n = 4668 || n = 4682)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4665 || n = 4685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4666)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4669)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4672)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
.elseif(n = 4673)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4676)
	; 11 bytes
	php
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4683)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 4684)
	; 12 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
.elseif(n = 4686)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4709)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4689 || n = 4693 || n = 4700 || n = 4702 || n = 4707 || n = 4715 || n = 4719 || n = 4721)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690 || n = 4710)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4691 || n = 4713)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4692 || n = 4712 || n = 4724)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4694)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4695)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4696)
	; 12 bytes
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	jsr @nearby_rts_14cyc
	dex
	Jne *-7
	tax
.elseif(n = 4697)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4698 || n = 4717)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4699 || n = 4706 || n = 4714 || n = 4716 || n = 4726)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4701)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4703)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4704)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4705 || n = 4725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4708)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4711)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4718)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4720)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4722)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4723)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 13
	iny
	Jne *-7
	tay
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4737 || n = 4738 || n = 4740 || n = 4747 || n = 4757 || n = 4759)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4728)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4729)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4730 || n = 4751)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4731)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4732 || n = 4743 || n = 4752 || n = 4762)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4733)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4734 || n = 4744 || n = 4754 || n = 4761)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4735)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4736 || n = 4755)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4739 || n = 4748 || n = 4758)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4741 || n = 4760)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4742)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4745 || n = 4765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4746)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4749)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4750)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4753)
	; 10 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4756)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4763)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4764)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4789)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4767 || n = 4769 || n = 4776 || n = 4778 || n = 4787 || n = 4795 || n = 4797)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4768 || n = 4777 || n = 4788 || n = 4796)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4770)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4771)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4772)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4773 || n = 4775 || n = 4782 || n = 4783 || n = 4786 || n = 4794)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4774 || n = 4793)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4779 || n = 4801)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4780)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4781)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4784)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4790)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4791)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4792)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4798)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4799)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4800)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4802)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 4803)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4804)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4806 || n = 4819 || n = 4832 || n = 4838 || n = 4840)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4807 || n = 4809 || n = 4827 || n = 4837)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4808 || n = 4815 || n = 4828)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4810 || n = 4830)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4811)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4812 || n = 4831)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4813)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4814 || n = 4835)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4816)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4817 || n = 4836)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4818 || (n > 4819 && n < 4823) || n = 4824 || n = 4826 || n = 4839 || n = 4842 || n = 4843)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4823)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4829)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4833)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4834)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4841)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4846 || (n > 4858 && n < 4863) || n = 4864 || n = 4866)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4845 || n = 4865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4847 || n = 4849 || n = 4852 || n = 4871 || n = 4873)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4848 || n = 4868 || n = 4878)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4850)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4851)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $CAA0'
	ldy #202
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4853)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4854)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4855)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4856 || n = 4877)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4857 || n = 4872 || n = 4882)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4858 || n = 4881)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4863)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 11
	dey
	Jne *-7
	tay
.elseif(n = 4867)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4869)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4870)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4874)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4875)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4876)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4879)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4880)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
	plp
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 4884 || n = 4886 || n = 4894 || n = 4895 || n = 4906 || (n > 4913 && n < 4917))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4885 || n = 4905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4887 || n = 4891 || n = 4900 || n = 4907 || n = 4909 || n = 4913 || n = 4921)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4888 || n = 4908 || n = 4920)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4889 || n = 4911)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4890 || n = 4910)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4892 || n = 4912)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4893)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4896)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4897)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4898 || n = 4919)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4899)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4901)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4902)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4903)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4904)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4917)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4918)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922)
	; 12 bytes
	php
	sec
	lda #(n-8)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 4923)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4924 || n = 4932 || n = 4952)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4925 || n = 4945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4926)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4927 || n = 4929 || n = 4935 || n = 4942 || n = 4947 || n = 4957)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4928 || n = 4948)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4930 || n = 4950)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4931 || n = 4934 || n = 4936 || n = 4937 || n = 4941 || n = 4943 || n = 4944 || n = 4951 || n = 4956 || n = 4958)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4933)
	; 11 bytes
	php
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4938)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4939)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4940)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4946)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4949)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4953)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4954)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4955)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4959)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4960)
	; 12 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.else
; 4961 to 5000
.if(n = 4961 || n = 4982)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4962 || n = 4964 || n = 4966 || (n > 4973 && n < 4977) || n = 4983 || n = 4986 || n = 4991 || n = 4993 || n = 4998)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4963 || n = 4967 || n = 4969 || n = 4979 || n = 4984 || n = 4987 || n = 4989)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4965 || n = 4985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4968 || n = 4988)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4970 || n = 4990)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4971)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4972 || n = 4992)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4973 || n = 4997)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4977 || n = 4999)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4978)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	nop
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-7
	tay
.elseif(n = 4980)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4981)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4994)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4995)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4996)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5000)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.endif
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
.if(n = 5001)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5002)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5003 || n = 5024 || n = 5066)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5004 || n = 5006 || n = 5011 || (n > 5012 && n < 5017) || n = 5031 || (n > 5032 && n < 5040) || n = 5046 || n = 5051 || n = 5053 || (n > 5054 && n < 5063) || n = 5071 || n = 5073 || n = 5074 || n = 5076 || n = 5077)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5005 || n = 5025 || n = 5045 || n = 5065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5007 || n = 5009 || n = 5026 || n = 5027 || n = 5029 || n = 5047 || n = 5049 || n = 5067)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5008 || n = 5028 || n = 5048 || n = 5068)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5010 || n = 5030 || n = 5050 || n = 5070)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5017)
	; 11 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5018)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5019 || n = 5042)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5020)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5021 || n = 5043)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5022)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5023)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5040)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5041)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5044)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5054)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5063)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5064)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5069)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5075)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5078)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	jsr @nearby_rts_14cyc
	Jne *-7
	tax
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5080 || (n > 5081 && n < 5085) || (n > 5093 && n < 5101) || n = 5104 || n = 5113)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5081)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5085 || n = 5105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5086)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5087 || n = 5109)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5088)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5089)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5090 || n = 5110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5091)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5092 || n = 5112 || n = 5114)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5093 || n = 5117)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5101)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5102)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5103)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5106)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5107)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5108)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5111)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5115)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5116)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5118 to 5156
.if((n > 5117 && n < 5125) || n = 5126 || (n > 5135 && n < 5139) || (n > 5141 && n < 5148))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5125)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5127 || n = 5133 || n = 5152 || n = 5155)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5128)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5129 || n = 5150)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5130)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5131 || n = 5153)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5132)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5134)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5135)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5139)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5140)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5141)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5148)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5149)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5151)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5154)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5156)
	; 12 bytes
	php
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || n = 5180)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5158)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n > 5158 && n < 5162) || (n > 5165 && n < 5169) || n = 5172 || n = 5174 || (n > 5180 && n < 5187) || n = 5193 || n = 5195)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5162)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5163)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5164)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5165 || n = 5189)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5169)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5170)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5171 || n = 5192)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5173 || n = 5177 || n = 5194)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5175)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5176)
	; 12 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	pla
	tay
.elseif(n = 5178)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5179)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5187)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5188)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5190)
	; 11 bytes
	php
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5191)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.else
; 5196 to 5234
.if(n = 5196 || n = 5198 || n = 5200 || n = 5204 || (n > 5205 && n < 5211) || n = 5214 || n = 5217 || n = 5220 || n = 5222 || n = 5223 || (n > 5226 && n < 5231))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5197 || n = 5219)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5199 || n = 5215 || n = 5221)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5201)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5202)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5203)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5205 || n = 5231)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5211)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 5212)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5213 || n = 5234)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5216)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5218)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5224)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5225)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5226)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5232)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5233)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5236 || n = 5265)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5237 || n = 5261)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif((n > 5237 && n < 5241) || n = 5242 || (n > 5243 && n < 5247) || (n > 5249 && n < 5253) || n = 5262 || n = 5264 || (n > 5265 && n < 5270) || n = 5273)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5241 || n = 5263)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5243 || n = 5270)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5247)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5248)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5249 || n = 5272)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5253)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5254)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5255)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5256)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5257)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5258)
	; 12 bytes
	php
	sec
	lda #(n-8)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5259)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5260)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5271)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5274 to 5312
.if(n = 5274)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5275)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5276 || n = 5297)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5277 || n = 5279 || n = 5280 || n = 5282 || n = 5286 || (n > 5287 && n < 5293) || n = 5298 || n = 5300 || n = 5301 || n = 5304 || n = 5305 || (n > 5309 && n < 5313))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5278 || n = 5287 || n = 5299)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5281)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
.elseif(n = 5283)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5284)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $24A0'
	ldy #36
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5285 || n = 5307)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5293)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5294)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5295)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5296)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5302)
	; 12 bytes
	tya
	ldy #(n-2)/25 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5303)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5306)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5308)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5309)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if((n > 5312 && n < 5316) || n = 5319 || (n > 5320 && n < 5324) || n = 5325 || n = 5326 || n = 5330 || n = 5334 || n = 5336 || (n > 5341 && n < 5351))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5316)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5317)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5318 || n = 5339)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5320 || n = 5331)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5324)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5327)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5328)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5329 || n = 5351)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5332)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5333)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5335)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5337)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5338)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5340)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5341)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.endif
.else
; 5352 to 5390
.if(n = 5352 || n = 5354 || (n > 5364 && n < 5373) || n = 5374 || n = 5376 || n = 5377 || n = 5382 || n = 5384 || (n > 5387 && n < 5391))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5353 || n = 5362 || n = 5375 || n = 5383)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5355)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5356)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5357)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5358)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5359)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5360)
	; 11 bytes
	tya
	ldy #(n-5)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5361)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5363)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5364 || n = 5387)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5373)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5378)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5379)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5380)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5381)
	; 11 bytes
	tya
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5385)
	; 11 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5386)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.endif
.endif
.else
; 5391 to 5468
.if((n > 5390 && n < 5395) || n = 5396 || (n > 5397 && n < 5403) || n = 5406 || n = 5407 || n = 5411 || n = 5412 || (n > 5413 && n < 5417) || n = 5418 || n = 5420 || n = 5421 || (n > 5422 && n < 5426) || (n > 5433 && n < 5439) || n = 5440 || (n > 5441 && n < 5447) || n = 5448 || n = 5450 || n = 5457 || n = 5458 || n = 5460 || n = 5462 || n = 5467 || n = 5468)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5395 || n = 5417 || n = 5439 || n = 5461)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5397 || n = 5419 || n = 5441 || n = 5449 || n = 5463)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5403)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5404)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5405 || n = 5429 || n = 5453)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5408)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5409)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5410 || n = 5433 || n = 5456)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5413 || n = 5465)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5422)
	; 12 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	pla
	tay
.elseif(n = 5426)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 5427)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5428)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5430)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5431)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5432 || n = 5459)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5447)
	; 11 bytes
	php
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5451)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5452)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5454)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5455)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5464)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5466)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	plp
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
; 5469 to 5546
.if((n > 5468 && n < 5475) || (n > 5479 && n < 5483) || n = 5487 || n = 5488 || n = 5490 || (n > 5491 && n < 5499) || n = 5504 || n = 5509 || n = 5511 || n = 5512 || n = 5516 || (n > 5517 && n < 5523) || n = 5526 || n = 5528 || (n > 5529 && n < 5540) || n = 5542 || n = 5544 || n = 5545)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5475)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5476)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5477 || n = 5501 || n = 5525)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5478)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5479 || n = 5502)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5483 || n = 5505 || n = 5527)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5484)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tax
.elseif(n = 5485 || n = 5507 || n = 5529)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5486 || n = 5513 || n = 5540)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5489)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5491 || n = 5517 || n = 5543)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5499)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5500)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5503)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5506)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5508)
	; 12 bytes
	php
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5510)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5514)
	; 12 bytes
	tya
	ldy #(n-2)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5515)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5523)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5524)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5541)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5546)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5547 to 5625
.if(n = 5547)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5548 || n = 5617)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5549 || n = 5571 || n = 5593 || n = 5615)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5550 || (n > 5551 && n < 5567) || n = 5568 || (n > 5573 && n < 5592) || (n > 5597 && n < 5603) || (n > 5603 && n < 5610) || (n > 5610 && n < 5614) || n = 5618 || (n > 5621 && n < 5626))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5551)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5567 || n = 5594)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5569 || n = 5595)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5570)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5572)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5573 || n = 5597 || n = 5621)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5592)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5596)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5603)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5610)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 5614)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5616)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5619)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5620)
	; 12 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.endif
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
; 5626 to 5703
.if((n > 5625 && n < 5637) || n = 5641 || n = 5642 || n = 5646 || (n > 5648 && n < 5661) || (n > 5663 && n < 5667) || n = 5670 || n = 5672 || n = 5674 || (n > 5675 && n < 5684) || (n > 5687 && n < 5691) || (n > 5693 && n < 5699) || n = 5700 || n = 5701 || n = 5703)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5637)
	; 11 bytes
	tya
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5638)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5639)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5640 || n = 5663 || n = 5686)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5643)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5644)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5645 || n = 5669 || n = 5693)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5647 || n = 5673 || n = 5699)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5648 || n = 5675 || n = 5702)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5661)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5662)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5667)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5668)
	; 12 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5671)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5684)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5685)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5687)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
	tax
.elseif(n = 5691)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5692)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.else
; 5704 to 5781
.if(n = 5704)
	; 11 bytes
	php
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5705 || (n > 5709 && n < 5715) || (n > 5717 && n < 5725) || (n > 5725 && n < 5729) || (n > 5732 && n < 5739) || (n > 5741 && n < 5745) || (n > 5745 && n < 5751) || n = 5752 || (n > 5756 && n < 5763) || (n > 5765 && n < 5776) || (n > 5778 && n < 5782))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5706)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5707)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5708)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5709 || n = 5732 || n = 5755 || n = 5778)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5715)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5716)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5717 || n = 5741 || n = 5765)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5725 || n = 5751 || n = 5777)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5729 || n = 5756)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5730)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5731)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5739)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5740)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5745)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5753)
	; 11 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5754)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5763)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5764)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5776)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if(n = 5782 || (n > 5783 && n < 5787) || (n > 5789 && n < 5799) || n = 5802 || (n > 5803 && n < 5810) || (n > 5813 && n < 5822) || (n > 5824 && n < 5829) || (n > 5829 && n < 5835) || (n > 5837 && n < 5845) || (n > 5847 && n < 5853) || n = 5854 || (n > 5855 && n < 5859))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5783 || n = 5810)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5787)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5788)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5789 || n = 5813 || n = 5837)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5799)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5800)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5801 || n = 5824 || n = 5847)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5803 || n = 5829 || n = 5855)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5811)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5812)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5822)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5823)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5835)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5836)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5845)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5846)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5853)
	; 12 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5859)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.endif
.else
; 5860 to 5937
.if(n = 5860)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5861 || n = 5885 || n = 5909 || n = 5933)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5862 || n = 5863 || (n > 5864 && n < 5868) || (n > 5870 && n < 5878) || n = 5879 || n = 5880 || n = 5882 || (n > 5885 && n < 5890) || (n > 5893 && n < 5902) || (n > 5903 && n < 5907) || (n > 5909 && n < 5918) || (n > 5918 && n < 5928) || n = 5929 || n = 5930 || (n > 5933 && n < 5938))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5864 || n = 5891 || n = 5918)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5868)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5869)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5870)
	; 12 bytes
	tya
	ldy #(n-5)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5878)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5881 || n = 5907)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5883)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5884)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5890)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	jsr @nearby_rts
	Jne *-7
	tax
.elseif(n = 5892)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5893)
	; 12 bytes
	tya
	ldy #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5902)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5903)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5908)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5928)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5931)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5932)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if((n > 5937 && n < 5945) || (n > 5945 && n < 5955) || n = 5958 || n = 5960 || n = 5962 || (n > 5963 && n < 5972) || (n > 5972 && n < 5979) || n = 5982 || n = 5984 || (n > 5985 && n < 5999) || (n > 5999 && n < 6003) || (n > 6005 && n < 6009) || n = 6010 || (n > 6011 && n < 6026) || (n > 6029 && n < 6035) || n = 6036 || (n > 6037 && n < 6051) || (n > 6053 && n < 6063) || (n > 6063 && n < 6075) || n = 6079 || (n > 6080 && n < 6087) || n = 6088 || (n > 6089 && n < 6094))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5945 || n = 5972 || n = 5999 || n = 6026 || n = 6080)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5955)
	; 12 bytes
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5956)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5957 || n = 5981 || n = 6005 || n = 6029 || n = 6053 || n = 6077)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5959 || n = 5985 || n = 6011 || n = 6037 || n = 6063 || n = 6089)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5961)
	; 11 bytes
	php
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5963)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5979)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5980)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5983)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 6003)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6004)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6009)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
	tax
.elseif(n = 6027)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6028)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6035)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 6051)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6052)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6075)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6076)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6078)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6087)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.else
; 6094 to 6250
.if((n > 6093 && n < 6099) || (n > 6103 && n < 6107) || (n > 6107 && n < 6113) || n = 6114 || (n > 6115 && n < 6123) || n = 6126 || n = 6127 || (n > 6128 && n < 6134) || (n > 6134 && n < 6138) || n = 6140 || (n > 6141 && n < 6147) || (n > 6149 && n < 6161) || (n > 6161 && n < 6165) || n = 6166 || (n > 6167 && n < 6188) || (n > 6188 && n < 6193) || (n > 6193 && n < 6215) || n = 6216 || n = 6217 || (n > 6220 && n < 6242) || n = 6243 || n = 6244 || (n > 6245 && n < 6251))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6099)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6100)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6101 || n = 6125)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6102)
	; 12 bytes
	tya
	ldy #(n-2)/25 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6103)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6107 || n = 6134 || n = 6161 || n = 6188 || n = 6215 || n = 6242)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6113)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6115 || n = 6141 || n = 6167 || n = 6193 || n = 6219 || n = 6245)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6123)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6124)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6128)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6138)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6139)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6147)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6148)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6149)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6165)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6218)
	; 11 bytes
	php
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6220)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
; 6251 to 6875
.if(n = 6251 || n = 6252 || (n > 6253 && n < 6269) || n = 6270 || (n > 6271 && n < 6296) || (n > 6297 && n < 6321) || n = 6322 || n = 6324 || (n > 6325 && n < 6346) || n = 6348 || (n > 6350 && n < 6373) || n = 6374 || n = 6376 || (n > 6377 && n < 6401) || n = 6402 || n = 6403 || (n > 6404 && n < 6427) || (n > 6427 && n < 6431) || (n > 6431 && n < 6443) || (n > 6443 && n < 6453) || (n > 6453 && n < 6458) || (n > 6458 && n < 6475) || n = 6476 || n = 6478 || (n > 6479 && n < 6485) || (n > 6485 && n < 6499) || (n > 6499 && n < 6503) || n = 6504 || (n > 6505 && n < 6512) || (n > 6512 && n < 6531) || (n > 6531 && n < 6539) || (n > 6539 && n < 6555) || n = 6556 || (n > 6557 && n < 6566) || (n > 6566 && n < 6583) || (n > 6583 && n < 6593) || (n > 6593 && n < 6609) || (n > 6609 && n < 6620) || (n > 6620 && n < 6635) || (n > 6635 && n < 6647) || (n > 6647 && n < 6661) || (n > 6661 && n < 6673) || (n > 6674 && n < 6701) || (n > 6702 && n < 6728) || n = 6729 || n = 6730 || n = 6733 || (n > 6734 && n < 6755) || (n > 6755 && n < 6782) || (n > 6782 && n < 6789) || (n > 6789 && n < 6809) || (n > 6809 && n < 6836) || (n > 6836 && n < 6863) || (n > 6863 && n < 6876))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6253)
	; 12 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6269 || n = 6296 || n = 6350 || n = 6377 || n = 6404 || n = 6431 || n = 6458 || n = 6485 || n = 6512 || n = 6539 || n = 6566 || n = 6593 || n = 6620 || n = 6647 || n = 6674 || n = 6701 || n = 6728 || n = 6755 || n = 6782 || n = 6809 || n = 6836 || n = 6863)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6271 || n = 6297 || n = 6323 || n = 6349 || n = 6375 || n = 6401 || n = 6427 || n = 6453 || n = 6479 || n = 6505 || n = 6531 || n = 6557 || n = 6583 || n = 6609 || n = 6635)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6321)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6325)
	; 12 bytes
	tya
	ldy #218 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6346)
	; 12 bytes
	tya
	ldy #(n-2)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6347)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6373)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6443)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 6475)
	; 11 bytes
	php
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6477 || n = 6734)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6499)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 6503)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6555)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 6661)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6673)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6702)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_14cyc
	Jne *-7
	tax
.elseif(n = 6731)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6732)
	; 11 bytes
	php
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6789)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.else
; 6876 to 7500
.if((n > 6875 && n < 6890) || (n > 6890 && n < 6905) || (n > 6905 && n < 6917) || (n > 6917 && n < 6989) || n = 6990 || (n > 6991 && n < 7003) || (n > 7003 && n < 7108) || (n > 7108 && n < 7246) || n = 7247 || (n > 7248 && n < 7253) || (n > 7253 && n < 7501))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6890)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6905)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	Jne *-7
	tax
.elseif(n = 6917)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6989)
	; 11 bytes
	php
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6991 || n = 7248)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7003)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 7108)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	tax
.elseif(n = 7246)
	; 11 bytes
	php
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 7253)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.else
; 7501 to 10000
.if(n = 7501 || n = 7502 || n = 7504 || (n > 7505 && n < 7760) || n = 7761 || (n > 7762 && n < 8017) || n = 8018 || (n > 8019 && n < 8274) || n = 8275 || (n > 8276 && n < 8531) || n = 8532 || (n > 8533 && n < 8788) || n = 8789 || (n > 8790 && n < 9045) || n = 9046 || (n > 9047 && n < 9302) || n = 9303 || (n > 9304 && n < 9559) || n = 9560 || (n > 9561 && n < 9816) || n = 9817 || (n > 9818 && n < 10001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 7503)
	; 11 bytes
	php
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 7505 || n = 7762 || n = 8019 || n = 8276 || n = 8533 || n = 8790 || n = 9047 || n = 9304 || n = 9561 || n = 9818)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7760)
	; 11 bytes
	php
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8017)
	; 11 bytes
	php
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8274)
	; 11 bytes
	php
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8531)
	; 11 bytes
	php
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8788)
	; 11 bytes
	php
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 9045)
	; 11 bytes
	php
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 9302)
	; 11 bytes
	php
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 9559)
	; 11 bytes
	php
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 9816)
	; 11 bytes
	php
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10073) || n = 10074 || (n > 10075 && n < 10330) || n = 10331 || (n > 10332 && n < 10587) || n = 10588 || (n > 10589 && n < 10844) || n = 10845 || (n > 10846 && n < 11101) || n = 11102 || (n > 11103 && n < 11358) || n = 11359 || (n > 11360 && n < 11615) || n = 11616 || (n > 11617 && n < 11872) || n = 11873 || (n > 11874 && n < 12129) || n = 12130 || (n > 12131 && n < 12386) || n = 12387 || (n > 12388 && n < 12643) || n = 12644 || (n > 12645 && n < 12900) || n = 12901 || (n > 12902 && n < 13157) || n = 13158 || (n > 13159 && n < 13414) || n = 13415 || (n > 13416 && n < 13671) || n = 13672 || (n > 13673 && n < 13928) || n = 13929 || (n > 13930 && n < 14185) || n = 14186 || (n > 14187 && n < 14442) || n = 14443 || (n > 14444 && n < 14699) || n = 14700 || (n > 14701 && n < 14956) || n = 14957 || (n > 14958 && n < 15001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10073)
	; 11 bytes
	php
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10075 || n = 10332 || n = 10589 || n = 10846 || n = 11103 || n = 11360 || n = 11617 || n = 11874 || n = 12131 || n = 12388 || n = 12645 || n = 12902 || n = 13159 || n = 13416 || n = 13673 || n = 13930 || n = 14187 || n = 14444 || n = 14701 || n = 14958)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10330)
	; 11 bytes
	php
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10587)
	; 11 bytes
	php
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10844)
	; 11 bytes
	php
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11101)
	; 11 bytes
	php
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11358)
	; 11 bytes
	php
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11615)
	; 11 bytes
	php
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11872)
	; 11 bytes
	php
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12129)
	; 11 bytes
	php
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12386)
	; 11 bytes
	php
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12643)
	; 11 bytes
	php
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12900)
	; 11 bytes
	php
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13157)
	; 11 bytes
	php
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13414)
	; 11 bytes
	php
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13671)
	; 11 bytes
	php
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13928)
	; 11 bytes
	php
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14185)
	; 11 bytes
	php
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14442)
	; 11 bytes
	php
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14699)
	; 11 bytes
	php
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14956)
	; 11 bytes
	php
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15213) || n = 15214 || (n > 15215 && n < 15470) || n = 15471 || (n > 15472 && n < 15727) || n = 15728 || (n > 15729 && n < 15984) || n = 15985 || (n > 15986 && n < 16241) || n = 16242 || (n > 16243 && n < 16498) || n = 16499 || (n > 16500 && n < 16755) || n = 16756 || (n > 16757 && n < 17012) || n = 17013 || (n > 17014 && n < 17269) || n = 17270 || (n > 17271 && n < 17526) || n = 17527 || (n > 17528 && n < 17783) || n = 17784 || (n > 17785 && n < 18040) || n = 18041 || (n > 18042 && n < 18297) || n = 18298 || (n > 18299 && n < 18554) || n = 18555 || (n > 18556 && n < 18811) || n = 18812 || (n > 18813 && n < 19068) || n = 19069 || (n > 19070 && n < 19325) || n = 19326 || (n > 19327 && n < 19582) || n = 19583 || (n > 19584 && n < 19839) || n = 19840 || (n > 19841 && n < 20001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15213)
	; 11 bytes
	php
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15215 || n = 15472 || n = 15729 || n = 15986 || n = 16243 || n = 16500 || n = 16757 || n = 17014 || n = 17271 || n = 17528 || n = 17785 || n = 18042 || n = 18299 || n = 18556 || n = 18813 || n = 19070 || n = 19327 || n = 19584 || n = 19841)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15470)
	; 11 bytes
	php
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15727)
	; 11 bytes
	php
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15984)
	; 11 bytes
	php
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 16241)
	; 11 bytes
	php
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 16498)
	; 11 bytes
	php
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 16755)
	; 11 bytes
	php
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17012)
	; 11 bytes
	php
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17269)
	; 11 bytes
	php
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17526)
	; 11 bytes
	php
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17783)
	; 11 bytes
	php
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18040)
	; 11 bytes
	php
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18297)
	; 11 bytes
	php
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18554)
	; 11 bytes
	php
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18811)
	; 11 bytes
	php
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19068)
	; 11 bytes
	php
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19325)
	; 11 bytes
	php
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19582)
	; 11 bytes
	php
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19839)
	; 11 bytes
	php
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
