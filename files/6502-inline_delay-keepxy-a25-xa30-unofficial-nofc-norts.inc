; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, Z+N, C, D, and V
; Will preserve X, Y, S, and I (interrupt-disable flag)
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
; - 5 bytes or less when n <= 14
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 290
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 1296
; -10 bytes or less when n <= 1305
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=4, avg=5.1, max=6
; - When 21 <= n <= 30: min=4, avg=5.3, max=7
; - When 31 <= n <= 50: min=4, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.4, max=9
; - When 301 <= n <= 500: min=6, avg=7.8, max=9
; - When 501 <= n <= 750: min=6, avg=7.8, max=9
; - When 751 <= n <= 1000: min=7, avg=7.8, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.7, max=11
; - When 2001 <= n <= 3000: min=8, avg=10.1, max=11
; - When 3001 <= n <= 5000: min=9, avg=10.9, max=11
; - When 5001 <= n <= 7500: min=9, avg=11.0, max=11
; - When 7501 <= n <= 10000: min=9, avg=11.0, max=11
; - When 10001 <= n <= 20000: min=9, avg=11.0, max=11
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
; 1 to 156
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif((n > 3 && n < 7))
	; 2..3 bytes
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
.elseif(n = 9 || n = 11 || n = 13 || n = 15)
	; 3..6 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10)
	; 4 bytes
	php
	cmp $C5
	plp
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 16 || n = 18)
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 17)
	; 6 bytes
	php
	rol $26
	ror $26
	plp
.elseif(n = 19)
	; 6 bytes
	php
	delay_n 12
	plp
.elseif(n = 20)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
.elseif(n = 21)
	; 5 bytes
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
.elseif(n = 22 || n = 26)
	; 5..6 bytes
	sec
	delay_n (n-2)
.elseif(n = 23)
	; 6 bytes
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
.elseif((n > 26 && n < 39) || (n > 39 && n < 157))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.endif
.else
; 157 to 312
.if((n > 156 && n < 283))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 283 || n = 284 || n = 291 || n = 297 || n = 302 || n = 307 || n = 312)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif((n > 284 && n < 288) || n = 289)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 292)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 293 || n = 303 || n = 308)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 294 || n = 296)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 298)
	; 6 bytes
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
.elseif(n = 299 || n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 301)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 304)
	; 7 bytes
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	nop
	Jne *-3
.elseif(n = 309)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
.elseif(n = 311)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 318 || n = 323 || n = 338 || n = 353 || n = 378)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 314 || n = 317 || n = 319 || n = 322 || n = 324 || n = 332 || n = 336 || n = 337 || n = 342 || n = 344 || n = 347 || n = 349 || n = 352 || n = 357 || n = 359 || n = 367 || n = 371 || n = 372 || n = 377 || n = 379 || n = 382 || n = 384 || n = 387 || n = 389)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316 || n = 321 || n = 326 || n = 351 || n = 366 || n = 386)
	; 8..9 bytes
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
.elseif(n = 328 || n = 333 || n = 343 || n = 348 || n = 358 || n = 363 || n = 368 || n = 373 || n = 388)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 329 || n = 364)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 331)
	; 9 bytes
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
.elseif(n = 339)
	; 7 bytes
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
.elseif(n = 346)
	; 7 bytes
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
.elseif(n = 354)
	; 8 bytes
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
.elseif(n = 356)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 361)
	; 8 bytes
	clc
	lda #216 ;hides 'cld'
	nop
	adc #1
	Jne *-4
.elseif(n = 362 || n = 369)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 374)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.if(n = 391 || n = 396 || n = 426 || n = 431 || n = 436 || n = 456 || n = 466)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 392 || n = 394 || n = 406 || n = 412 || n = 417 || n = 419 || n = 422 || n = 424 || n = 427 || n = 429 || n = 437 || n = 441 || n = 442 || n = 447 || n = 452 || n = 454 || n = 457 || n = 462 || n = 464)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 392 && n < 419) || (n > 432 && n < 449) || n = 458 || n = 468))
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif((n .mod 5 = 0) && (n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 397 || n = 411 || n = 467)
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
.elseif(n = 407)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif(n = 409)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 414)
	; 7 bytes
	sec
	lda #58 ;hides 'nop'
	sbc #1
	Jpl *-3
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
.elseif(n = 423 || n = 428 || n = 453 || n = 463)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 434)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 444 || n = 451)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 449)
	; 8 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	delay_n 9
	Jne *-5
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
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 472 || n = 477 || n = 482 || n = 487 || n = 492 || n = 494 || n = 497 || n = 499 || n = 511 || n = 512 || n = 517 || n = 522 || n = 524 || n = 527 || n = 529 || n = 532 || n = 539 || n = 542)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 473 || n = 483 || n = 503 || n = 508 || n = 523 || n = 538 || n = 543)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 474 || n = 502 || n = 516 || n = 544)
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
.elseif(n = 478 || n = 488 || n = 493 || n = 498 || n = 513 || n = 518 || n = 528 || n = 533)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 479 || n = 486 || n = 514 || n = 521)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 481 || n = 509 || n = 537)
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
	; 7 bytes
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jpl *-3
.elseif(n = 496 || n = 506 || n = 526)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 501)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 504)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
.elseif(n = 519)
	; 8 bytes
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
.elseif(n = 531)
	; 9 bytes
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
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
.elseif(n = 546)
	; 8 bytes
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jne *-4
.endif
.else
; 547 to 625
.if(n = 547 || n = 552 || n = 557 || n = 559 || n = 562 || n = 566 || n = 567 || n = 569 || n = 574 || n = 577 || n = 582 || n = 587 || n = 589 || n = 592 || n = 594 || n = 597 || n = 602 || n = 604 || n = 609 || n = 612 || n = 616 || n = 617 || n = 622)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && (n = 548 || n = 553 || n = 568 || (n > 582 && n < 594) || (n > 602 && n < 624)))
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 549 || n = 584 || n = 619)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
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
.elseif(n = 554)
	; 8 bytes
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jpl *-4
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 558 || n = 563 || n = 573 || n = 578 || n = 598)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
.elseif(n = 572 || n = 579)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 576 || n = 596 || n = 601)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 581 || n = 599)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 591)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 624)
	; 7 bytes
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626)
	; 7 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
.elseif(n = 627 || n = 629 || n = 632 || n = 634 || n = 637 || n = 639 || n = 647 || n = 652 || n = 657 || n = 659 || n = 662 || n = 664 || n = 667 || n = 669 || n = 672 || n = 674 || n = 679 || n = 682 || n = 686 || n = 687 || n = 692 || n = 694 || n = 697 || n = 699 || n = 702)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 633 || n = 638 || n = 648)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 630 || n = 640 || n = 645 || n = 650)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 631)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 701)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636 || n = 646 || n = 666 || n = 676 || n = 681)
	; 9 bytes
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
.elseif((n .mod 5 = 3) && (n = 643 || (n > 652 && n < 684) || (n > 692 && n < 704)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 644 || n = 671)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 649 || n = 684)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 654 || n = 661)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 689 || n = 696)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.endif
.else
; 704 to 781
.if(n = 704 || n = 707 || n = 709 || n = 721 || n = 722 || n = 727 || n = 732 || n = 737 || n = 739 || n = 742 || n = 744 || n = 756 || n = 757 || n = 762 || n = 764 || n = 767 || n = 769 || n = 772 || n = 774 || n = 777)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 706 || n = 746 || n = 751 || n = 776)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 3) && ((n > 707 && n < 779)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 711)
	; 8 bytes
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
.elseif(n = 712 || n = 719 || n = 747 || n = 754 || n = 761)
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
.elseif(n = 724)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 726)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 729)
	; 7 bytes
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
.elseif(n = 731)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
.elseif(n = 736)
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
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 759 || n = 766)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 771)
	; 9 bytes
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
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
; 782 to 937
.if(n = 782 || n = 789 || n = 824 || n = 852 || n = 887 || n = 922 || n = 929 || n = 936)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 782 && n < 934)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 784 || n = 819 || n = 854 || n = 889 || n = 924)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 0) && ((n > 784 && n < 936)))
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
.elseif(n = 787 || n = 791 || n = 792 || n = 797 || n = 799 || n = 802 || n = 807 || n = 809 || n = 812 || n = 822 || n = 826 || n = 827 || n = 832 || n = 834 || n = 837 || n = 839 || n = 842 || n = 844 || n = 847 || n = 849 || n = 861 || n = 867 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 892 || n = 897 || n = 902 || n = 904 || n = 907 || n = 909 || n = 912 || n = 914 || n = 917 || n = 927 || n = 931 || n = 932 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 794 || n = 829 || n = 871 || n = 906)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 796 || n = 817 || n = 831 || n = 859 || n = 866 || n = 894 || n = 901)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 801 || n = 836 || n = 899 || n = 934)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 804)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 806 || n = 851 || n = 896)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 811)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
.elseif(n = 814)
	; 9 bytes
	txa
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 816 || n = 841 || n = 846 || n = 856 || n = 876 || n = 886 || n = 911 || n = 916 || n = 926)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 821)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 857)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 862)
	; 7 bytes
	sec
	lda #250 ;hides 'nop'
	sbc #1
	Jmi *-3
.elseif(n = 864)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
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
.elseif(n = 881)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 891)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 919)
	; 8 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jpl *-4
.elseif(n = 921)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if((n .mod 5 = 3) && ((n > 937 && n < 1014) || (n > 1022 && n < 1094)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 939)
	; 7 bytes
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1091)))
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
.elseif(n = 942 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 967 || n = 972 || n = 974 || n = 977 || n = 979 || n = 982 || n = 987 || n = 989 || n = 997 || n = 1001 || n = 1002 || n = 1007 || n = 1009 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1024 || n = 1032 || n = 1036 || n = 1037 || n = 1042 || n = 1044 || n = 1047 || n = 1049 || n = 1052 || n = 1057 || n = 1059 || n = 1067 || n = 1072 || n = 1077 || n = 1079 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1092)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 945 && n < 962) || n = 991 || n = 996 || (n > 1015 && n < 1027) || n = 1066 || n = 1086))
	; 8..9 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 7 = 5) && ((n > 956 && n < 972) || (n > 991 && n < 1007) || (n > 1026 && n < 1042) || (n > 1061 && n < 1077)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 959 || n = 994 || n = 1029 || n = 1064)
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
.elseif(n = 969 || n = 976 || n = 1004 || n = 1011 || n = 1039 || n = 1046 || n = 1081)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1018)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
.elseif(n = 1031)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1051 || n = 1091)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1054)
	; 8 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	sbc #1
	Jmi *-4
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
.elseif(n = 1071)
	; 9 bytes
	txa
	pha
	lax #4
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1074)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1102 || n = 1106 || n = 1107 || n = 1112 || n = 1114 || n = 1117 || n = 1122 || n = 1124 || n = 1127 || n = 1129 || n = 1137 || n = 1141 || n = 1142 || n = 1147 || n = 1149 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1162 || n = 1164 || n = 1172 || n = 1176 || n = 1177 || n = 1182 || n = 1184 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1197 || n = 1199 || n = 1207 || n = 1212 || n = 1217 || n = 1219 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1232 || n = 1234 || n = 1242 || n = 1246 || n = 1247)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1251)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && (n = 1096 || n = 1126 || n = 1136 || n = 1156 || n = 1161 || (n > 1190 && n < 1207) || n = 1226 || n = 1231))
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 7 = 5) && ((n > 1096 && n < 1112) || (n > 1131 && n < 1147) || (n > 1166 && n < 1182) || (n > 1201 && n < 1217) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1249)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
.elseif(n = 1109 || n = 1116 || n = 1144 || n = 1151 || n = 1179 || n = 1186 || n = 1214 || n = 1221 || n = 1249)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1119)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1131 || n = 1171)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1134 || n = 1169 || n = 1204 || n = 1239)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
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
.if((n .mod 7 = 5) && (n = 1251 || (n > 1271 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1257 || n = 1259 || n = 1262 || n = 1264 || n = 1267 || n = 1269 || n = 1277 || n = 1281 || n = 1282 || n = 1287 || n = 1295 || n = 1297 || n = 1302 || n = 1304 || n = 1309 || n = 1316 || n = 1318 || n = 1323 || n = 1327)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 1252 && n < 1279)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
.elseif((n .mod 7 = 3) && (n = 1256 || n = 1284 || (n > 1297 && n < 1327)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1261)
	; 8 bytes
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 1266 || n = 1271 || n = 1276 || n = 1288 || n = 1294 || n = 1303 || n = 1322 || n = 1324)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1274)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1283)
	; 7 bytes
	sec
	lda #0
	sbc #1
	Jne *-2
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.elseif(n = 1289)
	; 7 bytes
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
.elseif(n = 1290 || n = 1292 || n = 1296)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1291)
	; 7 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
.elseif(n = 1299 || n = 1315)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1301 || n = 1310)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1306 || n = 1320)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1308)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1311)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
.elseif(n = 1313 || n = 1325)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1317)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1338 || n = 1350 || n = 1352 || n = 1366)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1330 || n = 1344 || n = 1348 || n = 1351 || n = 1358 || n = 1360 || n = 1365)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1339)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1332)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 4
	sbc #1
	Jmi *-5
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1334)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
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
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
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
.elseif(n = 1341 || n = 1357)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
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
	ldy #(n-3)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 1353 || n = 1362)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1359)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
.elseif(n = 1367)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375 || n = 1382 || n = 1396)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1369)
	; 10 bytes
	tya
	ldy #123
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
.elseif(n = 1371 || n = 1395)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1388 || n = 1393 || n = 1399)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373 || n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1376)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 1383)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1385 || n = 1397)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1387)
	; 7 bytes
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
.elseif(n = 1389)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1390 || n = 1392)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1394)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1401 || n = 1406)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
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
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jpl *-6
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1414 || n = 1420 || n = 1421 || n = 1423 || n = 1428 || n = 1430 || n = 1435 || n = 1439 || n = 1442 || n = 1449 || n = 1456 || n = 1458 || n = 1470 || n = 1474 || n = 1477 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408 || n = 1415 || n = 1422 || n = 1441 || n = 1448 || n = 1450 || n = 1462 || n = 1464 || n = 1471 || n = 1476 || n = 1478)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 9 = 5) && ((n > 1408 && n < 1437) || (n > 1462 && n < 1482)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 3) && (n = 1410 || n = 1417 || (n > 1430 && n < 1481)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1411 || n = 1443 || n = 1451 || n = 1467 || n = 1483)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1413 || n = 1429 || n = 1437 || n = 1453)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1416 || n = 1425 || n = 1434 || n = 1479)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1432)
	; 10 bytes
	sec
	lda #(n-2)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 1444)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1446)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1455)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
.elseif(n = 1457)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1460)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1465)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1469)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
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
.elseif(n = 1486 || n = 1491 || n = 1498 || n = 1502 || n = 1505 || n = 1512 || n = 1514 || n = 1519 || n = 1521)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1509) || n = 1522))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1488)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1492 || n = 1504 || n = 1518)
	; 9..10 bytes
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
.elseif(n = 1495)
	; 8 bytes
	clc
	lda #90 ;hides 'nop'
	nop
	adc #1
	Jne *-4
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
.elseif(n = 1506)
	; 8 bytes
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
.elseif(n = 1507 || n = 1523)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1511)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1513)
	; 7 bytes
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
.elseif(n = 1515)
	; 7 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
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
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 1525)
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
.elseif(n = 1527)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
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
.elseif(n = 1530 || n = 1532)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1533 || n = 1537 || n = 1540 || n = 1546 || n = 1547 || n = 1554 || n = 1556 || n = 1561)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1534)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 7 = 3) && ((n > 1535 && n < 1558)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1539 || n = 1555)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1548)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1549)
	; 8 bytes
	clc
	lda #84 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
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
; 1563 to 1640
.if(n = 1563 || n = 1579 || n = 1595 || n = 1619 || n = 1635)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1607) || n = 1620 || n = 1634))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1565 || n = 1577 || n = 1637)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1637)))
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
.elseif(n = 1568 || n = 1570 || n = 1574 || n = 1575 || n = 1582 || n = 1584 || n = 1590 || n = 1596 || n = 1603 || n = 1609 || n = 1610 || n = 1612 || n = 1617 || n = 1624 || n = 1626 || n = 1631 || n = 1633 || n = 1638 || n = 1640)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569 || n = 1623)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1572)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1576 || n = 1583 || n = 1602 || n = 1604 || n = 1628 || n = 1630)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1581 || n = 1597 || n = 1605 || n = 1621)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1586)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1588)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1588 && n < 1617)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1591)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1593)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 1611)
	; 7 bytes
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
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
.elseif(n = 1618 || n = 1632)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1625)
	; 7 bytes
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1639)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.endif
.else
; 1641 to 1718
.if(n = 1641)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642 || n = 1644 || n = 1658 || n = 1660 || n = 1674 || n = 1681 || n = 1700 || n = 1702 || n = 1714 || n = 1716)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1645 || n = 1666 || n = 1672 || n = 1673 || n = 1680 || n = 1682 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1708 || n = 1710)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1654)
	; 9..10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1647)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
.elseif((n .mod 7 = 3) && ((n > 1647 && n < 1719)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1649 || n = 1709)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1651 || n = 1667 || n = 1675 || n = 1691 || n = 1707)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif((n .mod 9 = 5) && ((n > 1651 && n < 1689) || n = 1715))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1653 || n = 1677 || n = 1693 || n = 1717)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1656 || n = 1684 || n = 1698 || n = 1712)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1659)
	; 8 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1663)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
.elseif(n = 1665 || n = 1705)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1668 || n = 1695)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1686)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 1722 || n = 1726 || n = 1729 || n = 1735 || n = 1736 || n = 1738 || n = 1743 || n = 1750 || n = 1752 || n = 1757)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1723 || n = 1731 || n = 1747)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1724 || n = 1733 || n = 1742)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1732 || n = 1746)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1728 || n = 1730)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1737)
	; 7 bytes
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
.elseif(n = 1739)
	; 7 bytes
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
.elseif(n = 1740)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1744)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1749)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1751)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif(n = 1753)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1754)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1756)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.endif
.else
; 1758 to 1796
.if(n = 1758 || n = 1759 || n = 1766 || n = 1768 || n = 1771 || n = 1792)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1759 && n < 1789)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1761 || n = 1782)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif((n .mod 7 = 5) && ((n > 1761 && n < 1791)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1763 || n = 1779)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1764)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1765 || n = 1773 || n = 1789)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1770 || n = 1777 || n = 1786)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1772)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1775)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1780)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1791)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
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
	; 8 bytes
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1798 || n = 1809 || n = 1831)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1799 || n = 1807 || n = 1816 || n = 1822 || n = 1824 || n = 1825 || n = 1834)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1833)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1801 || n = 1802 || n = 1804 || n = 1810)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1803)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1804 && n < 1833)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1808 || n = 1826)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif((n .mod 8 = 3) && ((n > 1810 && n < 1836)))
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1812)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1813 || n = 1821)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1815)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 1820)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1828)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1830)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1837)
	; 8 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 1838 || n = 1844 || n = 1847 || n = 1852 || n = 1855 || n = 1870 || n = 1871)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1839)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1840 || n = 1862)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif((n .mod 9 = 5) && ((n > 1840 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1842)
	; 9 bytes
	txa
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1843)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	tay
.elseif(n = 1845 || n = 1861 || n = 1869)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
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
.elseif(n = 1848)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1849 || n = 1854 || n = 1856 || n = 1858 || n = 1860 || n = 1872)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1851 || n = 1867 || n = 1875)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1853 || n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1857 || n = 1866)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1863)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1864)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1873)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 1874)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
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
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif((n .mod 9 = 5) && ((n > 1876 && n < 1914)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1878 || n = 1880 || n = 1890 || n = 1898 || n = 1900 || n = 1910)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1879)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 1881)
	; 9 bytes
	clc
	lda #68 ;hides 'nop zp'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
.elseif(n = 1882)
	; 9 bytes
	clc
	lda #68 ;hides 'nop zp'
	delay_n 4
	adc #1
	Jne *-5
.elseif((n .mod 8 = 3) && ((n > 1882 && n < 1908)))
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1884)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1885 || n = 1893)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1887)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1888 || n = 1896 || n = 1903)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1889 || n = 1901)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1892)
	; 9 bytes
	clc
	lda #84 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
.elseif(n = 1894)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1897 || n = 1908)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1902)
	; 8 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1905)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
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
.elseif(n = 1909)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
.elseif(n = 1911)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1912)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1914)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1923 || n = 1939)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1916 || n = 1918 || n = 1927 || n = 1936 || n = 1952)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1917 || n = 1933 || n = 1941)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1919 || n = 1930)
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
.elseif(n = 1924 || n = 1942 || n = 1943 || n = 1951)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1925 || n = 1937)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1926 || n = 1934 || n = 1944 || n = 1946)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1928 || n = 1950)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 1929)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1932 || n = 1948)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1935)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1938)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1945)
	; 8 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	sbc #1
	Jne *-4
.elseif(n = 1947)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1953)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.endif
.endif
.else
; 1954 to 2031
.if(n = 1954)
	; 9 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif((n .mod 8 = 3) && (n = 1955 || (n > 1970 && n < 1996) || n = 2011 || n = 2019))
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif((n .mod 9 = 3) && (n = 1956 || (n > 1982 && n < 2011) || n = 2028))
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1957 || n = 1981 || n = 1989 || n = 2005 || n = 2013 || n = 2029)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 2031)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 2007 || n = 2031)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1960 || n = 1966 || n = 1969 || n = 1975 || n = 1978 || n = 1986 || n = 1991 || n = 1999 || n = 2014 || n = 2022 || n = 2023)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1961 || n = 1973 || n = 1997 || n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1962 || n = 1980 || n = 1982 || n = 1988 || n = 1990 || n = 1998 || n = 2000 || n = 2002 || n = 2004 || n = 2006 || n = 2008 || n = 2017 || n = 2024 || n = 2026)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 1963)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 1968)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1970)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 1977)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 1984)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1993)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1996 || n = 2018)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2015)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2016)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2020)
	; 9 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2025)
	; 9 bytes
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2027)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2041 || n = 2047 || n = 2050 || n = 2059 || n = 2068)
	; 10 bytes
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
.elseif(n = 2034 || n = 2042 || n = 2044 || n = 2052 || n = 2054 || n = 2056)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2035 || n = 2043)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 2036)
	; 9 bytes
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
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
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	dey
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
.elseif(n = 2040 || n = 2062)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2046 || n = 2055 || n = 2064)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 2049 || n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2051)
	; 9 bytes
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 2053)
	; 9 bytes
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
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
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2061)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2065)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2067)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2070)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	nop
	nop $04
	adc #1
	Jne *-6
.endif
.else
; 2071 to 2109
.if(n = 2071)
	; 8 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
.elseif(n = 2072)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	nop $04
	iny
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
.elseif(n = 2074 || n = 2076)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif((n .mod 9 = 5) && ((n > 2074 && n < 2103)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2077 || n = 2083 || n = 2086 || n = 2101 || n = 2104)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2078 || n = 2080 || n = 2096 || n = 2098)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2079 || n = 2103)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2085 || n = 2092 || n = 2094)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2087 || n = 2088 || n = 2090 || n = 2097 || n = 2108)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2089)
	; 8 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
.elseif(n = 2091)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2095 || n = 2106)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2099)
	; 9 bytes
	txa
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2100)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 2107)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
; 2110 to 2187
.if(n = 2110 || n = 2112)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2184)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2113 || n = 2122 || n = 2131 || n = 2140 || n = 2143 || n = 2149 || n = 2155 || n = 2158 || n = 2167 || n = 2176 || n = 2185)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2114 || n = 2116 || n = 2132)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2115 || n = 2139 || n = 2151 || n = 2175 || n = 2187)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2117 || n = 2141 || n = 2153 || n = 2177)
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
.elseif(n = 2119 || n = 2133)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2121 || n = 2130)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2123 || n = 2173)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2124 || n = 2142 || n = 2144 || n = 2146 || n = 2152 || n = 2157 || n = 2162 || n = 2164 || n = 2166 || n = 2168 || n = 2169 || n = 2178 || n = 2180 || n = 2184 || n = 2186)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2125)
	; 8 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif((n .mod 11 = 3) && ((n > 2125 && n < 2171)))
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2127)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2128 || n = 2150)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
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
.elseif(n = 2135)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2136)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif((n .mod 9 = 3) && ((n > 2144 && n < 2182)))
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 2160)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2161)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2171)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2179)
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 2182)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188 || n = 2207 || n = 2218 || n = 2220 || n = 2222)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 2191 || n = 2196 || n = 2206 || n = 2212 || n = 2215 || n = 2221)
	; 10 bytes
	delay_n (n-2)
	nop
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
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2194)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2195)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 11
	Jne *-5
	tay
.elseif(n = 2197)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
.elseif(n = 2198)
	; 9 bytes
	clc
	lda #12 ;hides 'nop abs'
	nop $0
	adc #1
	Jne *-5
.elseif(n = 2199)
	; 8 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2200)
	; 9 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	iny
	Jne *-4
	tay
.elseif(n = 2202)
	; 9 bytes
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 2203)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2204)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2205 || n = 2216)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
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
	; 10 bytes
	sec
	lda #184 ;hides 'clv'
	nop
	bit $24
	sbc #1
	Jne *-6
.elseif(n = 2211 || n = 2223)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2214)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2217 || n = 2226)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2238 || n = 2260)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
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
.elseif(n = 2229)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2230 || n = 2239 || n = 2248 || n = 2256 || n = 2257)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2231)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2232 || n = 2234 || n = 2250)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2233)
	; 8 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
.elseif(n = 2235)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2236)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2240 || n = 2242)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2241)
	; 10 bytes
	clc
	lda #116 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 2243)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif(n = 2247 || n = 2259)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2249 || n = 2261)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2251)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2275 || n = 2276 || n = 2286 || n = 2292 || n = 2296 || n = 2302)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267 || n = 2268 || n = 2270 || (n > 2276 && n < 2280) || n = 2288)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2269)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2274)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 2283)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2284)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
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
.elseif(n = 2287)
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
.elseif(n = 2294)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2295)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2301)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	nop $04
	adc #1
	Jne *-5
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
; 2305 to 2343
.if(n = 2305)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2306 || n = 2312 || n = 2314 || (n > 2315 && n < 2319) || n = 2320 || n = 2322 || n = 2328 || (n > 2337 && n < 2341))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 2310 || n = 2311 || n = 2336)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2313)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2315 || n = 2326 || n = 2337)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2319 || n = 2331)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2321 || n = 2333)
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
.elseif(n = 2324)
	; 9 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2325)
	; 9 bytes
	clc
	lda #90 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
.elseif(n = 2327)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
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
.elseif(n = 2332)
	; 9 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
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
.elseif(n = 2341)
	; 9 bytes
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2342)
	; 10 bytes
	clc
	lda #100 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
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
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344)
	; 10 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2418)))
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
.elseif(n = 2347 || n = 2358 || n = 2383 || n = 2395 || n = 2407 || n = 2419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2348 || n = 2359 || n = 2370 || n = 2392 || n = 2414)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n > 2348 && n < 2353) || n = 2354 || (n > 2359 && n < 2363) || n = 2364 || n = 2366 || n = 2372 || n = 2374 || n = 2376 || n = 2378 || n = 2382 || n = 2384 || (n > 2385 && n < 2390) || n = 2394 || n = 2396 || n = 2398 || n = 2400 || n = 2402 || n = 2404 || n = 2406 || (n > 2407 && n < 2411) || n = 2416 || n = 2418 || n = 2420 || n = 2421)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2353 || n = 2363 || n = 2373)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif((n .mod 12 = 3) && ((n > 2354 && n < 2416)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2356)
	; 9 bytes
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2365 || n = 2375)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2368)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 2371 || n = 2385 || n = 2413)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2377)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2380)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2390 || n = 2412)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2397 || n = 2411)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2399)
	; 9 bytes
	sec
	lda #(n-1)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2401)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.else
; 2422 to 2500
.if(n = 2422)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
.elseif(n = 2423 || n = 2433 || n = 2473 || n = 2493)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2424)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	bit $24
	iny
	Jne *-5
	tay
.elseif((n .mod 11 = 5) && ((n > 2424 && n < 2492)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2426 || n = 2428 || n = 2430 || n = 2432 || n = 2437 || n = 2438 || n = 2440 || n = 2444 || n = 2448 || n = 2450 || n = 2452 || n = 2454 || n = 2457 || (n > 2458 && n < 2463) || n = 2464 || n = 2466 || (n > 2469 && n < 2473) || n = 2474 || n = 2476 || n = 2482 || n = 2484 || n = 2486 || n = 2492 || n = 2494 || n = 2496 || n = 2498)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif((n .mod 12 = 3) && (n = 2427 || (n > 2450 && n < 2500)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif((n .mod 12 = 5) && ((n > 2428 && n < 2490)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2431 || n = 2443 || n = 2467 || n = 2479)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2434 || n = 2456 || n = 2478 || n = 2500)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2435 || n = 2445 || n = 2485 || n = 2495)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2442)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2446)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2449)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	nop $04
	adc #1
	Jne *-6
.elseif(n = 2455 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2468)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2481)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2483)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2488)
	; 10 bytes
	sec
	lda #(n-2)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 2490)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
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
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
; 2501 to 2578
.if((n .mod 12 = 5) && ((n > 2500 && n < 2574)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2502 || n = 2524 || n = 2546 || n = 2557 || n = 2568)
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
.elseif(n = 2504 || (n > 2505 && n < 2509) || n = 2510 || n = 2514 || (n > 2515 && n < 2522) || n = 2526 || (n > 2527 && n < 2531) || n = 2532 || n = 2534 || n = 2536 || n = 2538 || (n > 2539 && n < 2543) || n = 2548 || n = 2550 || n = 2552 || n = 2554 || n = 2556 || n = 2558 || n = 2560 || n = 2562 || n = 2564 || n = 2569 || n = 2570 || n = 2572 || n = 2574 || n = 2576 || n = 2578)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2505 || n = 2545)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2509)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 12 = 3) && ((n > 2510 && n < 2572)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2512)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2515 || n = 2527 || n = 2551 || n = 2563)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2522 || n = 2566)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2531)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 2533)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2539 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2543)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2544)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
.elseif(n = 2553)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 2555)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2565)
	; 10 bytes
	tya
	ldy #0
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2575)
	; 9 bytes
	sec
	lda #(n-1)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2577)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.else
; 2579 to 2656
.if(n = 2579)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2582 || n = 2584 || n = 2586 || n = 2589 || n = 2591 || n = 2592 || n = 2594 || n = 2602 || n = 2604 || n = 2605 || n = 2608 || n = 2614 || n = 2616 || n = 2618 || n = 2620 || n = 2622 || (n > 2623 && n < 2631) || n = 2636 || (n > 2637 && n < 2641) || n = 2646 || n = 2648 || n = 2650)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2581 || n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 3) && ((n > 2582 && n < 2656)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif((n .mod 12 = 5) && ((n > 2584 && n < 2646)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2587 || n = 2600 || n = 2611 || n = 2615 || n = 2635 || n = 2647)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2588)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif((n .mod 11 = 5) && ((n > 2589 && n < 2635) || n = 2656))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2593 || n = 2649)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2596)
	; 9 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
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
.elseif(n = 2603)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2606)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
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
.elseif(n = 2613)
	; 9 bytes
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2617)
	; 10 bytes
	sec
	lda #218 ;hides 'nop'
	nop
	bit $24
	sbc #1
	Jne *-6
.elseif(n = 2632)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 2641)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 2642)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	tax
.elseif(n = 2644)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2652)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 2653)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	delay_n 6
	adc #1
	Jne *-6
.elseif(n = 2654)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if((n .mod 12 = 5) && ((n > 2656 && n < 2730)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2658 || (n > 2659 && n < 2663) || n = 2664 || n = 2666 || n = 2670 || (n > 2671 && n < 2676) || n = 2680 || n = 2682 || n = 2690 || n = 2692 || n = 2694 || n = 2696 || n = 2697 || n = 2699 || n = 2701 || n = 2702 || n = 2704 || n = 2706 || n = 2708 || n = 2710 || (n > 2711 && n < 2715) || n = 2716 || n = 2718 || (n > 2722 && n < 2727) || n = 2730 || n = 2732 || n = 2734)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2696) || n = 2719))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2663 || n = 2677)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2665 || n = 2707 || n = 2721)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 3) && ((n > 2666 && n < 2728)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2668)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
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
.elseif((n .mod 11 = 5) && ((n > 2677 && n < 2734)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2684)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
.elseif(n = 2685)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	sbc #1
	Jne *-5
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
	ldy #12 ;hides 'nop abs'
	nop $0
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2709 || n = 2720)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2728)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2731)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735 || n = 2749)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif((n > 2735 && n < 2739) || n = 2745 || n = 2748 || n = 2750 || n = 2760 || n = 2762 || (n > 2767 && n < 2771))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2739 || n = 2751 || n = 2763)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2740)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 2741 || n = 2753 || n = 2765)
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
.elseif(n = 2743 || n = 2767)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2744 || n = 2755 || n = 2766)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
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
.elseif(n = 2747 || n = 2761)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2752)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2754)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2756)
	; 10 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 2757)
	; 10 bytes
	sec
	lda #(n-1)/13 ;hides 'nop zp,x'
	delay_n 6
	sbc #1
	Jne *-6
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
.elseif(n = 2764)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	iny
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
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2778 || (n > 2779 && n < 2785) || n = 2790 || (n > 2791 && n < 2797) || n = 2800 || n = 2802 || n = 2804 || n = 2806 || n = 2807 || n = 2812)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif((n .mod 12 = 3) && ((n > 2774 && n < 2812)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2779)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2788 || n = 2810)
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
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if((n .mod 12 = 5) && ((n > 2812 && n < 2958)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2814 || n = 2816 || n = 2818 || n = 2826 || (n > 2827 && n < 2831) || n = 2832 || n = 2834 || n = 2836 || n = 2838 || (n > 2839 && n < 2845) || n = 2846 || n = 2848 || n = 2850 || (n > 2851 && n < 2859) || n = 2860 || n = 2862 || (n > 2863 && n < 2867) || n = 2868 || n = 2869 || n = 2874 || (n > 2875 && n < 2883) || n = 2884 || n = 2886 || n = 2888 || (n > 2889 && n < 2895) || n = 2896 || n = 2898 || n = 2900 || n = 2902 || (n > 2903 && n < 2907) || n = 2908 || n = 2910 || (n > 2911 && n < 2915) || n = 2916 || n = 2918 || n = 2920 || n = 2922 || (n > 2923 && n < 2929) || n = 2930 || n = 2932 || n = 2934 || (n > 2935 && n < 2943) || n = 2946 || (n > 2947 && n < 2955) || n = 2956 || n = 2958 || (n > 2959 && n < 2967) || n = 2968)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2815 || n = 2827 || n = 2839 || n = 2851 || n = 2863 || n = 2872 || n = 2887 || n = 2899 || n = 2911 || n = 2923 || n = 2935 || n = 2947)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2817 || n = 2831 || n = 2901 || n = 2915 || n = 2929)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && (n = 2819 || n = 2833 || (n > 2874 && n < 2918) || n = 2959))
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
.elseif(n = 2822)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
.elseif((n .mod 12 = 3) && ((n > 2822 && n < 2968)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2824)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2845)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
.elseif(n = 2867)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2870)
	; 9 bytes
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 2944)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.endif
.else
; 2969 to 3125
.if((n .mod 12 = 5) && ((n > 2968 && n < 3066)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2970 || n = 2972 || n = 2975 || n = 2976 || n = 2978 || n = 2980 || n = 2982 || n = 2984 || n = 2986 || (n > 2987 && n < 2991) || n = 2992 || n = 2994 || (n > 2995 && n < 2999) || n = 3000 || n = 3002 || n = 3004 || (n > 3007 && n < 3013) || n = 3014 || n = 3016 || n = 3018 || (n > 3019 && n < 3025) || n = 3026 || n = 3028 || n = 3030 || (n > 3031 && n < 3039) || n = 3040 || n = 3042 || n = 3044 || n = 3046 || n = 3047 || n = 3049 || n = 3050 || n = 3052 || n = 3054 || n = 3056 || (n > 3058 && n < 3063) || n = 3064 || n = 3066 || (n > 3071 && n < 3075) || n = 3076 || n = 3078 || (n > 3079 && n < 3083) || (n > 3085 && n < 3097) || n = 3098 || (n > 3099 && n < 3111) || n = 3112 || (n > 3113 && n < 3125))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3008) || n = 3031 || n = 3067 || n = 3079))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 5) && ((n > 2972 && n < 3002) || (n > 3042 && n < 3114)))
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
.elseif(n = 2977)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	bit $24
	sbc #1
	Jne *-6
.elseif((n .mod 12 = 3) && ((n > 2978 && n < 3064)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif((n .mod 14 = 3) && ((n > 2984 && n < 3014) || (n > 3068 && n < 3126)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3006)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3019)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3025)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3045)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
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
.elseif(n = 3055)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
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
.elseif(n = 3068)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
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
	; 9 bytes
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
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
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3128 || (n > 3129 && n < 3139) || n = 3140 || (n > 3141 && n < 3153) || n = 3154 || (n > 3155 && n < 3162) || (n > 3162 && n < 3167) || n = 3168 || n = 3170 || (n > 3175 && n < 3181) || n = 3182 || (n > 3183 && n < 3187) || (n > 3188 && n < 3195) || n = 3196 || (n > 3197 && n < 3203) || (n > 3203 && n < 3209) || n = 3210 || (n > 3211 && n < 3221) || n = 3222 || n = 3224 || n = 3226 || (n > 3227 && n < 3237) || n = 3238 || (n > 3239 && n < 3249) || n = 3250 || n = 3252 || (n > 3253 && n < 3265) || n = 3266 || (n > 3267 && n < 3277) || n = 3278 || n = 3280)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3127)
	; 9 bytes
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3129)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 3) && ((n > 3138 && n < 3210) || n = 3237 || n = 3265))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3140 && n < 3282)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3171)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3172)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 3173)
	; 10 bytes
	sec
	lda #(n-1)/13 ;hides 'nop zp,x'
	delay_n 6
	sbc #1
	Jne *-6
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
.elseif(n = 3203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3221)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
.elseif(n = 3223)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3227)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3249)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3277)
	; 9 bytes
	sec
	lda #(n-1)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.endif
.else
; 3282 to 3437
.if(n = 3282 || (n > 3283 && n < 3289) || (n > 3289 && n < 3293) || n = 3294 || (n > 3295 && n < 3302) || n = 3303 || n = 3305 || n = 3306 || n = 3308 || (n > 3309 && n < 3319) || n = 3320 || n = 3322 || (n > 3323 && n < 3335) || n = 3336 || (n > 3337 && n < 3349) || n = 3350 || (n > 3351 && n < 3363) || n = 3364 || (n > 3365 && n < 3377) || n = 3378 || (n > 3379 && n < 3384) || n = 3385 || (n > 3386 && n < 3391) || n = 3392 || n = 3394 || (n > 3395 && n < 3405) || n = 3406 || n = 3408 || n = 3410 || (n > 3411 && n < 3419) || n = 3420 || n = 3422 || n = 3423 || n = 3425 || n = 3426 || (n > 3427 && n < 3433) || n = 3434 || n = 3436 || n = 3437)
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3283)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3289)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif((n .mod 14 = 3) && ((n > 3292 && n < 3434)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3294 && n < 3436)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3302)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3304)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3319)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3384)
	; 9 bytes
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3386)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3409)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
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
.elseif(n = 3424)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3427)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 3438 to 3750
.if(n = 3438 || (n > 3439 && n < 3447) || n = 3448 || (n > 3449 && n < 3457) || n = 3458 || n = 3460 || n = 3462 || (n > 3463 && n < 3473) || n = 3474 || n = 3476 || (n > 3477 && n < 3489) || n = 3490 || (n > 3491 && n < 3501) || n = 3502 || n = 3504 || (n > 3505 && n < 3517) || n = 3518 || (n > 3519 && n < 3531) || n = 3532 || (n > 3533 && n < 3545) || n = 3546 || (n > 3547 && n < 3559) || n = 3560 || (n > 3561 && n < 3573) || n = 3574 || (n > 3575 && n < 3587) || n = 3588 || (n > 3589 && n < 3641) || n = 3642 || (n > 3643 && n < 3681) || n = 3682 || (n > 3683 && n < 3699) || (n > 3699 && n < 3713) || n = 3714 || (n > 3715 && n < 3745) || n = 3746 || (n > 3747 && n < 3751))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3439)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif((n .mod 14 = 3) && (n = 3447 || n = 3461 || n = 3489 || (n > 3516 && n < 3574)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3448 && n < 3576)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3457)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 3459)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3473)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3475)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3501)
	; 9 bytes
	sec
	lda #(n-1)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3503)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3587)
	; 10 bytes
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3589)
	; 10 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3641)
	; 9 bytes
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3643)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3681)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3683)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3699)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3713)
	; 10 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3745)
	; 10 bytes
	sec
	lda #(n-1)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
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
.else
; 3751 to 5000
.if((n > 3750 && n < 3763) || (n > 3763 && n < 3777) || n = 3778 || (n > 3779 && n < 3795) || (n > 3795 && n < 3860) || (n > 3860 && n < 3891) || (n > 3891 && n < 3898) || n = 3899 || (n > 3900 && n < 3905) || n = 3906 || (n > 3907 && n < 3923) || (n > 3923 && n < 3969) || n = 3970 || (n > 3971 && n < 4001) || n = 4002 || (n > 4003 && n < 4155) || n = 4156 || (n > 4157 && n < 4412) || n = 4413 || (n > 4414 && n < 4669) || n = 4670 || (n > 4671 && n < 4926) || n = 4927 || (n > 4928 && n < 5001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3763)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3777)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3779)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3795)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3860)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
.elseif(n = 3891)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3898)
	; 9 bytes
	txa
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 3900 || n = 4157 || n = 4414 || n = 4671 || n = 4928)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3905)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 3907)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
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
.elseif(n = 3969)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 3971)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4001)
	; 10 bytes
	sec
	lda #(n-1)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 4003)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4155)
	; 9 bytes
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4412)
	; 9 bytes
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4669)
	; 9 bytes
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 4926)
	; 9 bytes
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5183) || n = 5184 || (n > 5185 && n < 5440) || n = 5441 || (n > 5442 && n < 5697) || n = 5698 || (n > 5699 && n < 5954) || n = 5955 || (n > 5956 && n < 6211) || n = 6212 || (n > 6213 && n < 6468) || n = 6469 || (n > 6470 && n < 6725) || n = 6726 || (n > 6727 && n < 6982) || n = 6983 || (n > 6984 && n < 7239) || n = 7240 || (n > 7241 && n < 7496) || n = 7497 || (n > 7498 && n < 7753) || n = 7754 || (n > 7755 && n < 8010) || n = 8011 || (n > 8012 && n < 8267) || n = 8268 || (n > 8269 && n < 8524) || n = 8525 || (n > 8526 && n < 8781) || n = 8782 || (n > 8783 && n < 9038) || n = 9039 || (n > 9040 && n < 9295) || n = 9296 || (n > 9297 && n < 9552) || n = 9553 || (n > 9554 && n < 9809) || n = 9810 || (n > 9811 && n < 10001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5183)
	; 9 bytes
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5185 || n = 5442 || n = 5699 || n = 5956 || n = 6213 || n = 6470 || n = 6727 || n = 6984 || n = 7241 || n = 7498 || n = 7755 || n = 8012 || n = 8269 || n = 8526 || n = 8783 || n = 9040 || n = 9297 || n = 9554 || n = 9811)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5440)
	; 9 bytes
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5697)
	; 9 bytes
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 5954)
	; 9 bytes
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6211)
	; 9 bytes
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6468)
	; 9 bytes
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6725)
	; 9 bytes
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 6982)
	; 9 bytes
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7239)
	; 9 bytes
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7496)
	; 9 bytes
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 7753)
	; 9 bytes
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8010)
	; 9 bytes
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8267)
	; 9 bytes
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8524)
	; 9 bytes
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 8781)
	; 9 bytes
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9038)
	; 9 bytes
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9295)
	; 9 bytes
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9552)
	; 9 bytes
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 9809)
	; 9 bytes
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10066) || n = 10067 || (n > 10068 && n < 10323) || n = 10324 || (n > 10325 && n < 10580) || n = 10581 || (n > 10582 && n < 10837) || n = 10838 || (n > 10839 && n < 11094) || n = 11095 || (n > 11096 && n < 11351) || n = 11352 || (n > 11353 && n < 11608) || n = 11609 || (n > 11610 && n < 11865) || n = 11866 || (n > 11867 && n < 12122) || n = 12123 || (n > 12124 && n < 12379) || n = 12380 || (n > 12381 && n < 12636) || n = 12637 || (n > 12638 && n < 12893) || n = 12894 || (n > 12895 && n < 13150) || n = 13151 || (n > 13152 && n < 13407) || n = 13408 || (n > 13409 && n < 13664) || n = 13665 || (n > 13666 && n < 13921) || n = 13922 || (n > 13923 && n < 14178) || n = 14179 || (n > 14180 && n < 14435) || n = 14436 || (n > 14437 && n < 14692) || n = 14693 || (n > 14694 && n < 14949) || n = 14950 || (n > 14951 && n < 15001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10066)
	; 9 bytes
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10068 || n = 10325 || n = 10582 || n = 10839 || n = 11096 || n = 11353 || n = 11610 || n = 11867 || n = 12124 || n = 12381 || n = 12638 || n = 12895 || n = 13152 || n = 13409 || n = 13666 || n = 13923 || n = 14180 || n = 14437 || n = 14694 || n = 14951)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10323)
	; 9 bytes
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10580)
	; 9 bytes
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 10837)
	; 9 bytes
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11094)
	; 9 bytes
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11351)
	; 9 bytes
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11608)
	; 9 bytes
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 11865)
	; 9 bytes
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12122)
	; 9 bytes
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12379)
	; 9 bytes
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12636)
	; 9 bytes
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 12893)
	; 9 bytes
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13150)
	; 9 bytes
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13407)
	; 9 bytes
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13664)
	; 9 bytes
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 13921)
	; 9 bytes
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14178)
	; 9 bytes
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14435)
	; 9 bytes
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14692)
	; 9 bytes
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 14949)
	; 9 bytes
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15206) || n = 15207 || (n > 15208 && n < 15463) || n = 15464 || (n > 15465 && n < 15720) || n = 15721 || (n > 15722 && n < 15977) || n = 15978 || (n > 15979 && n < 16234) || n = 16235 || (n > 16236 && n < 16491) || n = 16492 || (n > 16493 && n < 16748) || n = 16749 || (n > 16750 && n < 17005) || n = 17006 || (n > 17007 && n < 17262) || n = 17263 || (n > 17264 && n < 17519) || n = 17520 || (n > 17521 && n < 17776) || n = 17777 || (n > 17778 && n < 18033) || n = 18034 || (n > 18035 && n < 18290) || n = 18291 || (n > 18292 && n < 18547) || n = 18548 || (n > 18549 && n < 18804) || n = 18805 || (n > 18806 && n < 19061) || n = 19062 || (n > 19063 && n < 19318) || n = 19319 || (n > 19320 && n < 19575) || n = 19576 || (n > 19577 && n < 19832) || n = 19833 || (n > 19834 && n < 20001))
	; 11 bytes
	txa
	pha
	ldx #>(n-45)
	lda #<(n-45)
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15206)
	; 9 bytes
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15208 || n = 15465 || n = 15722 || n = 15979 || n = 16236 || n = 16493 || n = 16750 || n = 17007 || n = 17264 || n = 17521 || n = 17778 || n = 18035 || n = 18292 || n = 18549 || n = 18806 || n = 19063 || n = 19320 || n = 19577 || n = 19834)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15463)
	; 9 bytes
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15720)
	; 9 bytes
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 15977)
	; 9 bytes
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 16234)
	; 9 bytes
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 16491)
	; 9 bytes
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 16748)
	; 9 bytes
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17005)
	; 9 bytes
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17262)
	; 9 bytes
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17519)
	; 9 bytes
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 17776)
	; 9 bytes
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18033)
	; 9 bytes
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18290)
	; 9 bytes
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18547)
	; 9 bytes
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 18804)
	; 9 bytes
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19061)
	; 9 bytes
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19318)
	; 9 bytes
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19575)
	; 9 bytes
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
.elseif(n = 19832)
	; 9 bytes
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
.endif
.endif
.endif

.endif
	rest
.endmacro
