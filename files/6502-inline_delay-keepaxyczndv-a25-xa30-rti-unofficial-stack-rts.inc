; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber S
; Will preserve A, X, Y, and P
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
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
; - 7 bytes or less when n <= 36
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 296
; -10 bytes or less when n <= 299
; -11 bytes or less when n <= 309
; -12 bytes or less when n <= 327
; -13 bytes or less when n <= 1319
; -14 bytes or less when n <= 1913
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=8.9, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.1, max=11
; - When 301 <= n <= 500: min=10, avg=11.6, max=13
; - When 501 <= n <= 750: min=10, avg=11.6, max=13
; - When 751 <= n <= 1000: min=11, avg=11.7, max=13
; - When 1001 <= n <= 2000: min=10, avg=12.5, max=15
; - When 2001 <= n <= 3000: min=12, avg=13.7, max=15
; - When 3001 <= n <= 5000: min=12, avg=14.4, max=15
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
.elseif(n = 4 || n = 5 || n = 8 || n = 15 || n = 17 || n = 18 || n = 21 || n = 28 || n = 30 || n = 31 || n = 34 || n = 41 || n = 43 || n = 44 || n = 297 || n = 298 || n = 300 || n = 310 || n = 311)
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
.elseif((n > 8 && n < 12) || n = 20 || n = 22 || n = 33 || n = 35 || n = 37 || n = 46)
	; 3..8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 2 bytes
	brk
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 36 || n = 39 || n = 42 || n = 45)
	; 3..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 40)
	; 6..8 bytes
	delay_n (n-14)
	delay_n 14
.elseif((n > 46 && n < 52) || (n > 53 && n < 297))
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
.elseif(n = 53)
	; 8 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
.elseif(n = 299)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 301 || n = 303)
	; 11 bytes
	php
	pha
	php
	lda #n-48
	jsr delay_a_25_clocks
	plp
	pla
	plp
.elseif(n = 302)
	; 10 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
	plp
.elseif((n > 303 && n < 310))
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 312)
	; 10 bytes
	php
	pha
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
	pla
	plp
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 320 || n = 376 || n = 383)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 314 || n = 339 || n = 344 || n = 349 || n = 354 || n = 369 || n = 374 || n = 384 || n = 389)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 315 || n = 330 || n = 335 || n = 340 || n = 345 || n = 350 || n = 363 || n = 365 || n = 370 || n = 380 || n = 385)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 316 || n = 321 || n = 326 || n = 328 || n = 331 || n = 333 || n = 336 || n = 338 || n = 351 || n = 356 || n = 358 || n = 361 || n = 366 || n = 371 || n = 373 || n = 386)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 317 || n = 322 || n = 342 || n = 347 || n = 357 || n = 362 || n = 372 || n = 377 || n = 382 || n = 387)
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 318)
	; 11 bytes
	php
	pha
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	nop
	Jne *-3
	pla
	plp
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 359 || n = 364 || n = 379)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 323)
	; 12 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	pla
	plp
.elseif(n = 325)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 327 || n = 332 || n = 337 || n = 352 || n = 367)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 341 || n = 348 || n = 355 || n = 390)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 343 || n = 378)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 346 || n = 381 || n = 388)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 353)
	; 11 bytes
	php
	pha
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	pla
	plp
.elseif(n = 360)
	; 11 bytes
	php
	pha
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
	pla
	plp
.elseif(n = 368)
	; 12 bytes
	php
	pha
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	pla
	plp
.elseif(n = 375)
	; 12 bytes
	php
	pha
	clc
	lda #216 ;hides 'cld'
	nop
	adc #1
	Jne *-4
	pla
	plp
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 398 || n = 401 || n = 406 || n = 408 || n = 426 || n = 431 || n = 436 || n = 438 || n = 441 || n = 456 || n = 461 || n = 466 || n = 468)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 392 || n = 437 || n = 442 || n = 467)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 393)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 5 = 4) && (n = 394 || n = 404 || n = 409 || n = 419 || n = 434 || (n > 443 && n < 465)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 395)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 397)
	; 10 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
	plp
.elseif(n = 399 || n = 414 || n = 424 || n = 429 || n = 439)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 400 || n = 405 || n = 415 || n = 420 || n = 433 || n = 435 || n = 440 || n = 445 || n = 450)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 401 && n < 433) || (n > 446 && n < 463)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 403)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 410)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 411)
	; 11 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	brk
	Jne *-4
	pla
	plp
.elseif(n = 413)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 416)
	; 11 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 418 || n = 446 || n = 453 || n = 460)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 421)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 423)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 425)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 428)
	; 11 bytes
	php
	pha
	sec
	lda #58 ;hides 'nop'
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 430)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 443)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 448)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 451 || n = 458 || n = 465)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 455)
	; 12 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	nop
	brk
	Jne *-5
	pla
	plp
.elseif(n = 463)
	; 12 bytes
	php
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	delay_n 9
	Jne *-5
	pla
	plp
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 484 || n = 494 || n = 499 || n = 509 || n = 519 || n = 524 || n = 529 || n = 539 || n = 544)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 470 || n = 480 || n = 508 || n = 510 || n = 515 || n = 520 || n = 525 || n = 540 || n = 545)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 471 || n = 476 || n = 478 || n = 491 || n = 496 || n = 501 || n = 506 || n = 511 || n = 513 || n = 526 || n = 531 || n = 536 || n = 538 || n = 541 || n = 543 || n = 546)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 472 || n = 482 || n = 487 || n = 497 || n = 517 || n = 522 || n = 537)
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 473)
	; 12 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 475)
	; 12 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	nop
	brk
	Jcs *-5
	pla
	plp
.elseif(n = 477 || n = 492 || n = 502 || n = 507 || n = 512 || n = 527 || n = 532 || n = 542)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 479 || n = 489 || n = 504 || n = 514 || n = 534)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 481 || n = 488 || n = 516 || n = 530)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 483)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 485)
	; 12 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 486)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 490)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 493 || n = 500 || n = 528 || n = 535)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 495 || n = 523)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 498)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 503)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 505)
	; 11 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jpl *-3
	pla
	plp
.elseif(n = 518)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 521)
	; 11 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 533)
	; 12 bytes
	php
	pha
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
	pla
	plp
.endif
.else
; 547 to 625
.if(n = 547 || n = 572 || n = 577 || n = 587 || n = 592 || n = 612)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 548)
	; 12 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif((n .mod 5 = 4) && ((n > 548 && n < 580) || n = 604 || (n > 613 && n < 625)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 550 || n = 555 || n = 580 || n = 585 || n = 590 || n = 595 || n = 610 || n = 615)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 551 || n = 586 || n = 593)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 551 && n < 568) || n = 582 || (n > 596 && n < 608) || n = 617 || n = 622))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 553)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 556 || n = 563 || n = 598)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 558 || n = 565 || n = 600 || n = 621)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 560)
	; 12 bytes
	php
	pha
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 561 || n = 566 || n = 571 || n = 576 || n = 581 || n = 588 || n = 596 || n = 601 || n = 606 || n = 608 || n = 611 || n = 616 || n = 618 || n = 623)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 568)
	; 12 bytes
	php
	pha
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 570)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 573)
	; 11 bytes
	php
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	brk
	Jne *-4
	pla
	plp
.elseif(n = 575)
	; 12 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	dey
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 578)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 583)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 584 || n = 589 || n = 594 || n = 599 || n = 609)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 591)
	; 11 bytes
	php
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	brk
	Jne *-4
	pla
	plp
.elseif(n = 603)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 605)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 613)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-28)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 620)
	; 12 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 625)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
	tax
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626 || n = 633 || n = 668 || n = 675)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 626 && n < 653) || n = 662))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 628 || n = 635 || n = 663 || n = 698)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 4) && ((n > 628 && n < 645) || (n > 653 && n < 665)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 630 || n = 643 || n = 648 || n = 650 || n = 660 || n = 680 || n = 685 || n = 690 || n = 695 || n = 700)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 631 || n = 636 || n = 641 || n = 646 || n = 651 || n = 666 || n = 671 || n = 676 || n = 678 || n = 681 || n = 686 || n = 688 || n = 693 || n = 701)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 638)
	; 11 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
	pla
	plp
.elseif(n = 640)
	; 11 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	pla
	plp
.elseif(n = 645)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif((n .mod 5 = 4) && (n = 649 || (n > 668 && n < 700)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 653)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 655)
	; 12 bytes
	php
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	nop
	brk
	Jne *-5
	pla
	plp
.elseif(n = 656 || n = 670 || n = 691)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && (n = 657 || (n > 666 && n < 698)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 658)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-28)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 661 || n = 696 || n = 703)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 665)
	; 12 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 673)
	; 13 bytes
	php
	pha
	sec
	lda #(n-25)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 683)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 702)
	; 10 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	pla
	plp
.endif
.else
; 704 to 781
.if((n .mod 5 = 4) && ((n > 703 && n < 780)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 705 || n = 720 || n = 730 || n = 735 || n = 748 || n = 750 || n = 755 || n = 760 || n = 765 || n = 770)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 706 || n = 711 || n = 716 || n = 718 || n = 721 || n = 736 || n = 741 || n = 746 || n = 751 || n = 756 || n = 758 || n = 771 || n = 776 || n = 778 || n = 781)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 706 && n < 778)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 708)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 710 || n = 738)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 713)
	; 13 bytes
	php
	pha
	sec
	lda #(n-25)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 715)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 723)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 725)
	; 12 bytes
	php
	pha
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
	pla
	plp
.elseif(n = 726 || n = 733 || n = 761 || n = 768 || n = 775)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 728)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 731)
	; 11 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 740)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 743)
	; 11 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
	pla
	plp
.elseif(n = 745)
	; 11 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 753)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 763)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 766)
	; 11 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
	pla
	plp
.elseif(n = 773 || n = 780)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.endif
.endif
.else
; 782 to 937
.if((n .mod 5 = 2) && ((n > 781 && n < 938)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 783 || n = 786 || n = 788 || n = 791 || n = 806 || n = 811 || n = 813 || n = 816 || n = 821 || n = 826 || n = 841 || n = 846 || n = 848 || n = 851 || n = 853 || n = 856 || n = 858 || n = 861 || n = 881 || n = 886 || n = 888 || n = 891 || n = 896 || n = 898 || n = 911 || n = 916 || n = 918 || n = 921 || n = 926 || n = 928 || n = 931)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 935)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif((n .mod 5 = 0) && ((n > 784 && n < 796) || n = 805 || n = 820 || (n > 829 && n < 841) || (n > 854 && n < 876) || (n > 889 && n < 911) || n = 925 || n = 930))
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 793)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-28)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 796 || n = 803 || n = 838 || n = 866 || n = 901 || n = 936)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 798 || n = 868 || n = 903)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 800)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 801 || n = 815 || n = 850 || n = 913)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 808 || n = 836 || n = 843 || n = 885 || n = 906 || n = 920)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 810 || n = 831 || n = 845 || n = 873 || n = 880 || n = 908 || n = 915)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 818)
	; 12 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 823)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 825)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 833)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 863)
	; 13 bytes
	php
	pha
	sec
	lda #(n-26)/9
	delay_n 4
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 871)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 876)
	; 11 bytes
	php
	pha
	sec
	lda #250 ;hides 'nop'
	sbc #1
	Jmi *-3
	pla
	plp
.elseif(n = 878)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 883)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 893)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 923)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 933)
	; 12 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jpl *-4
	pla
	plp
.elseif(n = 935)
	; 12 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 973 || n = 1008 || n = 1078)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 4) && ((n > 938 && n < 1090)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 940 || n = 945 || n = 958 || n = 960 || n = 965 || n = 970 || n = 975 || n = 995 || n = 1000 || n = 1005 || n = 1010 || n = 1015 || n = 1030 || n = 1035 || n = 1040 || n = 1050 || n = 1065 || n = 1075 || n = 1080 || n = 1085)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 941 || n = 948 || n = 983 || n = 990 || n = 1011 || n = 1018 || n = 1025 || n = 1046 || n = 1053 || n = 1060 || n = 1081)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1028) || (n > 1036 && n < 1093)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif((n .mod 7 = 5) && (n = 943 || n = 950 || (n > 970 && n < 986) || (n > 1005 && n < 1021) || (n > 1040 && n < 1056) || (n > 1075 && n < 1091)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 946 || n = 951 || n = 956 || n = 961 || n = 966 || n = 968 || n = 981 || n = 986 || n = 988 || n = 991 || n = 996 || n = 1001 || n = 1016 || n = 1021 || n = 1023 || n = 1026 || n = 1028 || n = 1031 || n = 1036 || n = 1051 || n = 1056 || n = 1058 || n = 1061 || n = 1063 || n = 1066 || n = 1071 || n = 1086 || n = 1091 || n = 1093)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 953)
	; 11 bytes
	php
	pha
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 955)
	; 11 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 963 || n = 1003)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 976)
	; 11 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	tay
	pla
	plp
.elseif(n = 980)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 993 || n = 1033)
	; 13 bytes
	php
	pha
	sec
	lda #(n-25)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 998)
	; 12 bytes
	php
	pha
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1032)
	; 10 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
	plp
.elseif(n = 1038)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1043)
	; 12 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	brk
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 1045)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 1068)
	; 12 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	sbc #1
	Jmi *-4
	pla
	plp
.elseif(n = 1070)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1073)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1088)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && ((n > 1093 && n < 1250)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && (n = 1095 || (n > 1115 && n < 1131) || (n > 1150 && n < 1166) || (n > 1185 && n < 1201) || (n > 1220 && n < 1236)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1096 || n = 1098 || n = 1101 || n = 1103 || n = 1106 || n = 1108 || n = 1121 || n = 1126 || n = 1128 || n = 1131 || n = 1136 || n = 1138 || n = 1141 || n = 1143 || n = 1156 || n = 1161 || n = 1166 || n = 1168 || n = 1171 || n = 1173 || n = 1176 || n = 1191 || n = 1196 || n = 1198 || n = 1201 || n = 1206 || n = 1208 || n = 1211 || n = 1213 || n = 1226 || n = 1231 || n = 1236 || n = 1238 || n = 1241 || n = 1243 || n = 1246 || n = 1248)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 1096 && n < 1248)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif((n .mod 5 = 0) && ((n > 1099 && n < 1111) || n = 1120 || (n > 1134 && n < 1156) || (n > 1169 && n < 1191) || (n > 1204 && n < 1226) || n = 1240 || n = 1245))
	; 12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1110 && n < 1126) || (n > 1145 && n < 1161) || (n > 1180 && n < 1196) || (n > 1215 && n < 1231)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1113)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1115)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1133)
	; 12 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1148 || n = 1218)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1163 || n = 1203)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1178)
	; 13 bytes
	php
	pha
	sec
	lda #(n-26)/9
	delay_n 4
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1183)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 1233)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1250)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	tay
	pla
	plp
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
.if((n .mod 7 = 5) && ((n > 1250 && n < 1266) || (n > 1285 && n < 1329)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 1251 && n < 1293)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1253)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 5 = 4) && ((n > 1253 && n < 1295)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 1255 || n = 1260 || n = 1280 || n = 1285 || n = 1290 || n = 1295 || n = 1302 || n = 1306 || n = 1308 || n = 1317)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1256 || n = 1263 || n = 1270 || n = 1291 || n = 1298 || n = 1312 || n = 1319 || n = 1326)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1261 || n = 1266 || n = 1271 || n = 1276 || n = 1278 || n = 1281 || n = 1296 || n = 1301 || n = 1309 || n = 1316)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1268)
	; 12 bytes
	php
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1273 || n = 1313)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1275)
	; 12 bytes
	php
	pha
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 1283 || n = 1323)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1288)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1297)
	; 11 bytes
	php
	pha
	sec
	lda #0
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1299)
	; 11 bytes
	php
	pha
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 1303)
	; 11 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1304)
	; 13 bytes
	php
	pha
	php
	sec
	lda #0
	sbc #1
	Jne *-2
	plp
	pla
	plp
.elseif(n = 1305)
	; 11 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1310)
	; 13 bytes
	php
	pha
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1311)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1315 || n = 1324)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1318 || n = 1320)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 1322)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1325)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1327)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if((n .mod 8 = 1) && ((n > 1328 && n < 1354)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1330 || n = 1334 || n = 1344 || n = 1362 || n = 1365)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1339 || n = 1355)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1332)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1364)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1336 || n = 1338 || n = 1343 || n = 1352 || n = 1364 || n = 1366)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1341)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1346)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 4
	sbc #1
	Jmi *-5
	pla
	plp
.elseif(n = 1348)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
	pla
	plp
.elseif(n = 1350)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1351 || n = 1360)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1357)
	; 13 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1358 || n = 1367)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1359)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 1368 to 1406
.if((n .mod 7 = 3) && ((n > 1367 && n < 1397)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1369 || n = 1378 || n = 1387)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1406)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1371 || n = 1379 || n = 1395)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1372 || n = 1374 || n = 1386 || n = 1400 || n = 1402)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1376)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1380 || n = 1388 || n = 1404 || n = 1406)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1381)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1383)
	; 14 bytes
	php
	pha
	tya
	ldy #123
	delay_n 4
	nop
	dey
	Jpl *-4
	tay
	pla
	plp
.elseif(n = 1385)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
	tax
	pla
	plp
.elseif(n = 1390)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 1392)
	; 13 bytes
	php
	pha
	tya
	ldy #250 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1393)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1394)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
	pla
	plp
.elseif(n = 1397)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1399)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 1401)
	; 11 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1403)
	; 11 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1428 || n = 1442)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408)
	; 13 bytes
	php
	pha
	php
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1409 || n = 1425)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1410 || n = 1424 || n = 1431 || n = 1445)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1411 || n = 1427 || n = 1435 || n = 1443)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1441)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1413)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1414 || n = 1423 || n = 1432)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1415)
	; 13 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	brk
	adc #1
	Jpl *-5
	pla
	plp
.elseif(n = 1416)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 1417)
	; 11 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1418)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jpl *-6
	pla
	plp
.elseif(n = 1420 || n = 1429 || n = 1434 || n = 1436 || n = 1441)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1421 || n = 1430 || n = 1439)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1422)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1437)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1438)
	; 11 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
	pla
	plp
.elseif(n = 1444)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 14 bytes
	php
	pha
	sec
	lda #(n-16)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1446 && n < 1483)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1448 || n = 1484)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1449 || n = 1457 || n = 1465 || n = 1481)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1450 || n = 1477)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1451 || n = 1467 || n = 1483)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1451 && n < 1481)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1453 || n = 1456 || n = 1463 || n = 1470 || n = 1472)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1455)
	; 13 bytes
	php
	pha
	clc
	lda #184 ;hides 'clv'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1458)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1460)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1462 || n = 1464 || n = 1478)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1469)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1471)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1474)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 1476)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1479)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
; 1485 to 1562
.if(n = 1485)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1485 && n < 1514) || (n > 1539 && n < 1559)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1523) || n = 1536 || n = 1550 || n = 1557))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1488 || n = 1498 || n = 1500 || n = 1512 || n = 1516 || n = 1526 || n = 1528 || n = 1533 || n = 1551 || n = 1554)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1490 || n = 1492 || n = 1506 || n = 1518 || n = 1525 || n = 1532 || n = 1544 || n = 1546 || n = 1560 || n = 1562)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1491 || n = 1499 || n = 1507 || n = 1523 || n = 1539 || n = 1547 || n = 1555)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1493 || n = 1502)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1497 || n = 1505 || n = 1521 || n = 1537 || n = 1553 || n = 1561)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1509)
	; 12 bytes
	php
	pha
	clc
	lda #90 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 1511)
	; 12 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1514)
	; 12 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1519)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 1520)
	; 12 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 1527)
	; 11 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1529)
	; 11 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1530)
	; 12 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1534)
	; 13 bytes
	php
	pha
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1535)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1541)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1542)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 1543)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1548)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1556)
	; 12 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563)
	; 12 bytes
	php
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1600)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1565)
	; 12 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1567 || n = 1576)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1568 || n = 1570 || n = 1575 || n = 1582 || n = 1588 || n = 1596 || n = 1598)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569 || n = 1577 || n = 1593)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1572)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 1574)
	; 12 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1579 || n = 1595)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1581 || n = 1590 || n = 1597)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1583)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1584)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1586)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1589)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1591)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1600)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1603 || n = 1612 || n = 1621)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1604 || n = 1624 || n = 1626 || n = 1631 || n = 1638)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1605 || n = 1616 || n = 1618 || n = 1623 || n = 1630)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1606 || n = 1613 || n = 1620 || n = 1634)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1607)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1607 && n < 1637)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1609 || n = 1617 || n = 1633)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1610 || n = 1637)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1611 || n = 1619 || n = 1635)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1614)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1625)
	; 11 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1627)
	; 11 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1628)
	; 12 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
	pla
	plp
.elseif(n = 1632)
	; 13 bytes
	php
	pha
	php
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1639)
	; 11 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1640)
	; 13 bytes
	brk
	delay_n (n-13)
.endif
.endif
.else
; 1641 to 1718
.if(n = 1641)
	; 11 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1642 || n = 1644 || n = 1656 || n = 1658 || n = 1672 || n = 1679 || n = 1686 || n = 1688 || n = 1695 || n = 1714 || n = 1716)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1645 || n = 1652 || n = 1680 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1708)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1682 || n = 1709)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1647)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B5A0'
	ldy #181
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && (n = 1648 || (n > 1661 && n < 1719)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1649 || n = 1665 || n = 1681 || n = 1689 || n = 1705)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1651 || n = 1659 || n = 1667 || n = 1691 || n = 1707 || n = 1715)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1653)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1654 || n = 1668 || n = 1670 || n = 1698)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 1655)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1660)
	; 13 bytes
	php
	pha
	php
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1661)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1663 || n = 1687)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1665 && n < 1703)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1673)
	; 12 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1674)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1677)
	; 13 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1700)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
	pla
	plp
.elseif(n = 1710)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1712)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 1717)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1791)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1721 || n = 1737 || n = 1745 || n = 1761 || n = 1777 || n = 1785 || n = 1793)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1722 || n = 1724 || n = 1740 || n = 1743 || n = 1750 || n = 1752 || n = 1764 || n = 1775 || n = 1780)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 3) && (n = 1723 || n = 1731 || (n > 1762 && n < 1788)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1747) || n = 1760 || (n > 1773 && n < 1796)))
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1726 || n = 1766 || n = 1796)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 1728)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1729 || n = 1738 || n = 1747 || n = 1792)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1730 || n = 1742 || n = 1744 || n = 1749 || n = 1754 || n = 1756 || n = 1768 || n = 1784 || n = 1789)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1733 || n = 1757)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1735 || n = 1759)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1736 || n = 1772)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1751)
	; 11 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1753)
	; 11 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1758)
	; 13 bytes
	php
	pha
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1765)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1767)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1770)
	; 12 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1773)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1778)
	; 12 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1782)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1786)
	; 12 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1791)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1794)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797 || n = 1804)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1798 || n = 1812 || n = 1814 || n = 1820)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1799)
	; 12 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1800)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1829)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1802)
	; 12 bytes
	php
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1803 || n = 1827 || n = 1835)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1805 || n = 1829)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1806 || n = 1813 || n = 1830)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1807 || n = 1831)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1808)
	; 12 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1809)
	; 12 bytes
	php
	pha
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1811)
	; 12 bytes
	php
	pha
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1815)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1816)
	; 14 bytes
	php
	pha
	php
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1817)
	; 12 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1818)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1821 || n = 1832)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1822)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 1823)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1824)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 1825 || n = 1833)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1826)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1834)
	; 12 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1838 || n = 1848 || n = 1852 || n = 1858 || n = 1861 || n = 1866 || n = 1874)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1874)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1839 || n = 1869)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1840 || n = 1847)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841 || n = 1849 || n = 1865)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1842)
	; 12 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1843 || n = 1859 || n = 1867 || n = 1875)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1844)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
	pla
	plp
.elseif(n = 1845)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1850)
	; 12 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1851)
	; 12 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 1853)
	; 12 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1854)
	; 13 bytes
	php
	pha
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
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
	; 13 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1860)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 1862)
	; 12 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1863)
	; 13 bytes
	php
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1868)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-12)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 1870)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-2
	jsr @nearby_rts
	sbc #1
	Jpl *-5
	pla
	plp
.elseif(n = 1871)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1872)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	pla
	plp
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
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1877 || n = 1901)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1878 || n = 1911)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1879 || n = 1903)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1880 || n = 1898)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1914)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1883 || n = 1899 || n = 1907)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1884 || n = 1885 || n = 1892 || n = 1902 || n = 1910)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1886)
	; 14 bytes
	php
	pha
	clc
	lda #100 ;hides 'nop zp'
	nop $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 1887)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1888)
	; 13 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1890)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1893)
	; 13 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1894 || n = 1912)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 1895)
	; 13 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1896)
	; 13 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1904)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1906)
	; 13 bytes
	php
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1908)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1914)
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
.else
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915 || n = 1931)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1916)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-8)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1917)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1918 || n = 1927)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1919 || n = 1928)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1920)
	; 13 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1921 || n = 1929)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1922)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1923)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1924)
	; 13 bytes
	php
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1925)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1926)
	; 13 bytes
	php
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 1930)
	; 14 bytes
	php
	pha
	php
	sec
	lda #212 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 1932)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 1933)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.else
; 1934 to 1953
.if(n = 1934)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1935)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1936 || n = 1945)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1937 || n = 1953)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1938 || n = 1941)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1939 || n = 1947)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1940)
	; 14 bytes
	php
	pha
	tya
	ldy #112
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
	plp
.elseif(n = 1942)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1943)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1944)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1946)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1948)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	brk
	adc #1
	Jpl *-6
	pla
	plp
.elseif(n = 1949)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1950)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 1951)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1952)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
.if((n >= 1954) && (n < 1973))
; 1954 to 1972
.if(n = 1954 || n = 1963 || n = 1972)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1955 || n = 1971)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1956)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1957 || n = 1967)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1958)
	; 14 bytes
	php
	pha
	tya
	ldy #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 1959)
	; 12 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1960)
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
.elseif(n = 1961)
	; 12 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1962)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1964)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1965)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1966)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1968)
	; 13 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1969)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1970)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.endif
.else
; 1973 to 1992
.if(n = 1973)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1974 || n = 1983 || n = 1992)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1975)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1976)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 1977)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1978)
	; 13 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1979)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1980)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1981 || n = 1990)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1982)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1984)
	; 14 bytes
	php
	pha
	php
	sec
	lda #(n-22)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 1985)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1986)
	; 13 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 1987)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1988)
	; 13 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1989)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1991)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2001 || n = 2009 || n = 2025)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1994)
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
.elseif((n .mod 8 = 3) && ((n > 1994 && n < 2028)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1996 || n = 2002 || n = 2004 || n = 2012 || n = 2014 || n = 2022)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 8) && ((n > 1996 && n < 2025)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1998)
	; 13 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1998 && n < 2027)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2000 || n = 2013 || n = 2018 || n = 2028)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2005)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2007)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2010)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2016)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2020)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2021)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2023)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2029)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2030)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2031)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2054)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2033 || n = 2049 || n = 2057)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/8
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2034)
	; 13 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2063)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2036 || n = 2046 || n = 2064)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2037)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2038 || n = 2056 || n = 2058 || n = 2068)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2039)
	; 13 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2040)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2041)
	; 13 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2042 || n = 2060 || n = 2069)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2043 || n = 2051 || n = 2059)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2045)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2047)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2048)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	pla
	plp
.elseif(n = 2050)
	; 13 bytes
	php
	pha
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2052)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2055)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	brk
	adc #1
	Jpl *-5
	pla
	plp
.elseif(n = 2061)
	; 13 bytes
	php
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2063)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2065)
	; 13 bytes
	php
	pha
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2066)
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
.elseif(n = 2067)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2070)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071 || n = 2080 || n = 2089)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2072)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	php
	cmp $C5
	plp
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2073 || n = 2082)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2075)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2076)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2077)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2078)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2079)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2081)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2083)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2084)
	; 14 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	nop
	nop $04
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2085)
	; 12 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 2086)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2087)
	; 12 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2088)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	tax
	pla
	plp
.endif
.else
; 2090 to 2109
.if(n = 2090 || n = 2099 || n = 2101 || n = 2108)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2091)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2092)
	; 14 bytes
	php
	pha
	php
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 2093)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2094)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2095)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2096)
	; 12 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
	pla
	plp
.elseif(n = 2097)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2098 || n = 2107)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2100)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2102)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2103)
	; 12 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 2104)
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
.elseif(n = 2105)
	; 12 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2106)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2109)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
.if((n >= 2110) && (n < 2129))
; 2110 to 2128
.if(n = 2110)
	; 14 bytes
	php
	pha
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 2111)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2112)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	pla
	tay
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
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2115)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2116 || n = 2125)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2117)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2118)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2119)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2120)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2121)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2122 || n = 2126)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2123)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2124)
	; 13 bytes
	php
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2127)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2128)
	; 14 bytes
	php
	pha
	php
	sec
	lda #(n-22)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
	pla
	plp
.endif
.else
; 2129 to 2148
.if(n = 2129)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2130)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2131)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2132)
	; 12 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2133)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2134 || n = 2143)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2135 || n = 2144)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2136)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2137 || n = 2147)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2138)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2139)
	; 12 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 2140)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2141)
	; 12 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2142)
	; 13 bytes
	php
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2145)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2146)
	; 14 bytes
	php
	pha
	php
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 2148)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.endif
.endif
.else
; 2149 to 2187
.if(n = 2149)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2150)
	; 12 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2151)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2153 || n = 2165)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2154 || n = 2163 || n = 2171 || n = 2172 || n = 2180 || n = 2181)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2155 || n = 2167)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2156 || n = 2158 || n = 2182)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2157)
	; 13 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jmi *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 8) && ((n > 2158 && n < 2187)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2160)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2162 || n = 2173 || n = 2184)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2164)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2166 || n = 2183)
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
.elseif(n = 2169)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2174)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2175)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2176)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-2
	jsr @nearby_rts
	sbc #1
	Jpl *-5
	pla
	plp
.elseif(n = 2178)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2185)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2187)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if((n .mod 9 = 1) && ((n > 2187 && n < 2225)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2189 || n = 2201 || n = 2225)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2190 || n = 2198 || n = 2210 || n = 2220 || n = 2226)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2191 || n = 2203)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2192 || n = 2194 || n = 2200 || n = 2207 || n = 2221)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2193)
	; 13 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2195)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2196)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2199 || n = 2209)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2202)
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
.elseif(n = 2204)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-8)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2205)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2208)
	; 13 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2211)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2212)
	; 13 bytes
	php
	pha
	clc
	lda #12 ;hides 'nop abs'
	nop $0
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2213)
	; 12 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2214)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2216)
	; 13 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2217)
	; 13 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2218)
	; 13 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2219)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2222)
	; 12 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2223)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227 || n = 2239)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2228)
	; 13 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2229)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2230)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2231 || n = 2240)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2232)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2233 || n = 2242)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2234 || n = 2235 || n = 2244)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2236)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2237)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2238)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2241)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2243)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2245)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 2246 to 2265
.if(n = 2246)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2247)
	; 12 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2248)
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
.elseif(n = 2249)
	; 12 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2250)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2251 || n = 2260)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2252 || n = 2264)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2253 || n = 2262)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2254)
	; 14 bytes
	php
	pha
	php
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 2255)
	; 14 bytes
	php
	pha
	clc
	lda #116 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2256)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2257)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2258)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2259)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2261)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2263)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2265)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266)
	; 14 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2267)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2268)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2269 || n = 2278)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2270 || n = 2281)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2271)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2272)
	; 13 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2273)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2274)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2275)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2276)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2277)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2279)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2280)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2282)
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
.elseif(n = 2283)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2284)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 2285 to 2304
.if(n = 2285)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2286)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2287 || n = 2296)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2288)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2289)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2290 || n = 2300)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2291 || n = 2302)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2292 || n = 2293)
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
.elseif(n = 2294)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2295)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2297)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2298)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2299)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2301)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2303)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2304)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305 || n = 2314)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2306 || n = 2310)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2307 || n = 2318)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2308)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2309)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2311)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2312)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2313)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2315)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop $04
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2316)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2317)
	; 13 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2319)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2320)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2321)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2322)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2323)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 2324 to 2343
.if(n = 2324 || n = 2325)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2326 || n = 2336 || n = 2342)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2327)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2328)
	; 14 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2329 || n = 2340)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2330 || n = 2332 || n = 2334)
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
.elseif(n = 2331)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2333)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2335)
	; 13 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	bit $24
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2337)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	cmp $C5
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2338)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-6)/11 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2339)
	; 13 bytes
	php
	pha
	clc
	lda #90 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2341)
	; 13 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2343)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
.if((n >= 2344) && (n < 2363))
; 2344 to 2362
.if(n = 2344)
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2345)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2346)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2347 || n = 2359)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2348)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-16)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2349)
	; 13 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2350)
	; 14 bytes
	php
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2351 || n = 2362)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2352)
	; 14 bytes
	php
	pha
	sec
	lda #250 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	pla
	plp
.elseif(n = 2353)
	; 14 bytes
	php
	pha
	tya
	ldy #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2354)
	; 14 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jpl *-5
	tay
	pla
	plp
.elseif(n = 2355)
	; 13 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2356)
	; 14 bytes
	php
	pha
	clc
	lda #100 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2357)
	; 13 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2358)
	; 14 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2360)
	; 13 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2361)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.else
; 2363 to 2382
.if(n = 2363)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2364 || n = 2366 || n = 2376)
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
.elseif(n = 2365)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2367)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2368)
	; 14 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2369 || n = 2381)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2372)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2373 || n = 2374 || n = 2378)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2375)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2377)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2379)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2380)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2382)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
; 2383 to 2421
.if((n .mod 12 = 7) && ((n > 2382 && n < 2420)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2384 || n = 2406)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2385)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2386)
	; 14 bytes
	php
	pha
	clc
	lda #137 ;hides 'nop #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2387 || n = 2397)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2388)
	; 14 bytes
	php
	pha
	tya
	ldy #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2389 || n = 2399 || n = 2409)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2390 || n = 2392 || (n > 2399 && n < 2404) || n = 2412)
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
.elseif(n = 2391)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2393 || n = 2405 || n = 2417)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2394)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2396 || n = 2398 || n = 2408 || n = 2410 || n = 2418 || n = 2420)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2404)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2411)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2413)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2414)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2415)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2416)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2421)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2432 || n = 2434 || n = 2440 || n = 2444 || n = 2446 || n = 2452 || n = 2456 || n = 2458)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2423 || n = 2424 || n = 2430 || n = 2442 || n = 2454)
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
.elseif(n = 2425)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2426 || n = 2448)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2427)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2428 || n = 2439 || n = 2450)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2429 || n = 2441)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2431 || n = 2443 || n = 2455)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2433 || n = 2445 || n = 2451)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2435)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	brk
	sbc #1
	Jmi *-5
	pla
	plp
.elseif(n = 2436)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2437)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jmi *-4
	tay
	pla
	plp
.elseif(n = 2438)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	bit $24
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2447)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2449)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2453)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
	plp
.elseif(n = 2457)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2459)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2460)
	; 14 bytes
	php
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	pla
	tax
	pla
	plp
.endif
.else
; 2461 to 2500
.if(n = 2461 || n = 2472 || n = 2494)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2462)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2463)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	nop $04
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2464)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2465 || n = 2477 || n = 2489)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2466 || n = 2496)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 2467 || n = 2479 || n = 2491)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2468 || n = 2478 || n = 2480 || n = 2486 || n = 2488 || n = 2498 || n = 2500)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2469 || n = 2499)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2470 || n = 2481 || n = 2492)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2471)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2473 || n = 2474 || n = 2476)
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
.elseif(n = 2475)
	; 13 bytes
	php
	pha
	sec
	lda #250 ;hides 'nop'
	brk
	sbc #1
	Jmi *-5
	pla
	plp
.elseif(n = 2482)
	; 14 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2483)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2484)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2485)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2487)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2490)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2493)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2495)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2497)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
	plp
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
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2502)
	; 14 bytes
	php
	pha
	sec
	lda #(n-16)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2502 && n < 2540)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2504)
	; 14 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2505 || n = 2516 || n = 2538)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2506 || n = 2520 || n = 2522 || n = 2524 || n = 2530)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2507)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2508 || n = 2510 || n = 2512 || n = 2531 || n = 2532 || n = 2534)
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
.elseif(n = 2509 || n = 2529)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2511)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2514 || n = 2536)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2517)
	; 13 bytes
	php
	pha
	tya
	ldy #250 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2518 || n = 2535)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2519)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2521)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2523)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2526)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2528)
	; 14 bytes
	php
	pha
	tya
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2533)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540 || n = 2542 || n = 2554)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2541)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2543 || n = 2544 || n = 2546)
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
.elseif(n = 2545)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2547)
	; 13 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2548)
	; 14 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2549)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2550)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2551)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2552)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2553)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2555)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2556)
	; 14 bytes
	php
	pha
	sec
	lda #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2557)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2558)
	; 13 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
	pla
	plp
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2560 || n = 2571)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2561 || n = 2573)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2562 || n = 2574)
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
.elseif(n = 2563 || n = 2575)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2564 || n = 2576)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2565)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2566)
	; 14 bytes
	php
	pha
	sec
	lda #(n-6)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2567)
	; 13 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2568)
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	iny
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2569)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2570)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2572)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2577)
	; 14 bytes
	php
	pha
	sec
	lda #0
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2578)
	; 14 bytes
	php
	pha
	sec
	lda #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jpl *-6
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
.if((n >= 2579) && (n < 2598))
; 2579 to 2597
.if(n = 2579)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2580)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2581 || n = 2595)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2582)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2583)
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
.elseif(n = 2584)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2585 || n = 2597)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2586)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2587)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2588)
	; 14 bytes
	php
	pha
	tya
	ldy #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jpl *-5
	tay
	pla
	plp
.elseif(n = 2589)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2590 || n = 2594 || n = 2596)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2591)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2592)
	; 14 bytes
	php
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2593)
	; 13 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 2598 to 2617
.if(n = 2598)
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
.elseif(n = 2599 || n = 2611)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2600 || n = 2605 || n = 2616)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2601)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2602)
	; 13 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2603)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2604 || n = 2615)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2606)
	; 14 bytes
	php
	pha
	sec
	lda #(n-6)/20 ;hides 'nop #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2607)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2608)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-8)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2609)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2610)
	; 13 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2612)
	; 13 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2613)
	; 13 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2614)
	; 14 bytes
	php
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2617)
	; 14 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2652)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2619 || n = 2639)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2620)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2621 || n = 2633 || n = 2645)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2622 || n = 2628 || n = 2634 || (n > 2639 && n < 2645) || n = 2653)
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
.elseif(n = 2623 || n = 2635 || n = 2647)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2624)
	; 13 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2625)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2626 || n = 2637 || n = 2648)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
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
.elseif(n = 2629)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2630 || n = 2632 || n = 2638 || n = 2650)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2631)
	; 14 bytes
	php
	pha
	sec
	lda #218 ;hides 'nop'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2636)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2646)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2649)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2651)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2654)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2655)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2656)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	tax
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if((n .mod 12 = 5) && ((n > 2656 && n < 2694)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2658)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2696)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2660 || n = 2662 || n = 2672 || n = 2674 || n = 2684)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2661 || n = 2673 || n = 2685)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2663)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2664 || n = 2675 || n = 2676 || n = 2678 || n = 2687 || n = 2689 || n = 2694)
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
.elseif(n = 2665)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2666)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2667)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	delay_n 4
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2668 || n = 2679)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2670 || n = 2692)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2677 || n = 2691)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2680)
	; 14 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2682)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2686)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2688)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2690)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-6)/11 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 2696 to 2734
.if(n = 2696 || n = 2704 || n = 2708 || n = 2710 || n = 2715 || n = 2732)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2697)
	; 13 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2698)
	; 13 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2699)
	; 13 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2700)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-16)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2701)
	; 13 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2702)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2703 || n = 2714 || n = 2725)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2705 || n = 2717 || n = 2729)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2706 || n = 2711 || n = 2716 || n = 2718 || n = 2726 || n = 2727)
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
.elseif(n = 2707 || n = 2719 || n = 2731)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2709)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2712)
	; 13 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2713)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2720)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2721)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2722)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2723 || n = 2734)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2724)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2728)
	; 14 bytes
	php
	pha
	sec
	lda #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2730)
	; 14 bytes
	php
	pha
	tya
	ldy #n/14-1 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2733)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2736 || n = 2747)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2737)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2738 || n = 2740 || n = 2744)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2739)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2741 || n = 2753)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2742)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2743)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2745)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2746)
	; 14 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2748)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-8)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2749)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2750 || n = 2751)
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
.elseif(n = 2752)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2755 || n = 2767)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2756)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2757)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	brk
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2758 || n = 2769)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2759)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2760)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-4)/13 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2761)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2762)
	; 14 bytes
	php
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2763)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2764)
	; 14 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2765)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2766)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2768)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2770)
	; 14 bytes
	php
	pha
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2771)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2772)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-16)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2773)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2776 || n = 2782 || n = 2792 || n = 2794 || n = 2804 || n = 2806 || n = 2810)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2775)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2777 || n = 2789 || n = 2801)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2778)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2779 || n = 2791 || n = 2803)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2780 || n = 2802)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2781 || n = 2793)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2783 || n = 2795 || n = 2796 || n = 2798 || n = 2808)
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
.elseif(n = 2784)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2785)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2786)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2787)
	; 13 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2788)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2790)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2797)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2799)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2800)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2805)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2807)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2809)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2811)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2812)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if((n .mod 12 = 5) && ((n > 2812 && n < 2886)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2814 || n = 2821 || n = 2826 || n = 2832 || n = 2844 || n = 2850 || n = 2855 || n = 2860 || (n > 2866 && n < 2873) || n = 2874 || n = 2880)
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
.elseif((n .mod 12 = 7) && ((n > 2814 && n < 2888)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2816 || n = 2818 || n = 2820 || n = 2830 || n = 2840 || n = 2842 || n = 2852 || n = 2854 || n = 2862 || n = 2864 || n = 2866 || n = 2876 || n = 2878 || n = 2882 || n = 2888)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2817)
	; 13 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2819 || n = 2833 || n = 2847 || n = 2889)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2822)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2823)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2824)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2828)
	; 14 bytes
	php
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2829 || n = 2841 || n = 2853 || n = 2865 || n = 2877 || n = 2883 || n = 2886)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2831)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2834)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2835)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2836)
	; 14 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2838)
	; 14 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2843 || n = 2879)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2845 || n = 2881)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2846)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2848)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2856 || n = 2890)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2857)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2858)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2859)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
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
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2893 || n = 2895 || n = 2896 || n = 2898 || (n > 2903 && n < 2907) || n = 2908 || n = 2910 || n = 2916 || n = 2922 || n = 2927 || n = 2928 || n = 2930 || n = 2932 || n = 2934 || n = 2940 || n = 2946 || n = 2952 || n = 2964 || n = 2966 || n = 2967)
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
.elseif(n = 2892 || n = 2926 || n = 2960)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2894)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop $04
	brk
	Jne *-5
	tay
	pla
	plp
.elseif((n .mod 12 = 5) && ((n > 2896 && n < 2958)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2898 && n < 2960)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2900 || n = 2902 || n = 2912 || n = 2914 || n = 2918 || n = 2920 || n = 2936 || n = 2938 || n = 2948 || n = 2950 || n = 2954 || n = 2956 || n = 2962)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2901 || n = 2929 || n = 2943)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2903 || n = 2931)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2907 || n = 2924 || n = 2941)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2913 || n = 2925 || n = 2937 || n = 2949 || n = 2955 || n = 2961)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2915 || n = 2951)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2917 || n = 2953)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2919 || n = 2939)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2942)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	bit $FF24 ;hides 'bit zp'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2944)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2958)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2963)
	; 14 bytes
	php
	pha
	clc
	lda #122 ;hides 'nop'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2965)
	; 14 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2968)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n .mod 12 = 5) && ((n > 2968 && n < 3042)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2970 || n = 2976 || n = 2978 || n = 2980 || n = 2982 || n = 3000 || (n > 3001 && n < 3005) || n = 3006 || n = 3012 || n = 3014 || n = 3016 || n = 3018 || n = 3024 || n = 3030 || n = 3035 || n = 3036 || n = 3038 || n = 3040)
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
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3044)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2972 || n = 2974 || n = 2984 || n = 2986 || n = 2990 || n = 2996 || n = 2998 || n = 3008 || n = 3010 || n = 3022 || n = 3032 || n = 3034 || n = 3042 || n = 3044 || n = 3046)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2973 || n = 3001 || n = 3015)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2975 || n = 2992 || n = 3026)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2977 || n = 2994 || n = 3011 || n = 3028)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2979)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 14 = 3) && ((n > 2984 && n < 3028)))
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2987 || n = 3023)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2988)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2989 || n = 3025)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2991)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
	plp
.elseif((n .mod 12 = 9) && ((n > 2996 && n < 3046)))
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3020)
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3037)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3039)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 3047 to 3125
.if((n > 3046 && n < 3053) || n = 3054 || n = 3066 || n = 3073 || n = 3074 || n = 3076 || n = 3078 || n = 3087 || n = 3088 || n = 3090 || (n > 3100 && n < 3111) || n = 3112 || n = 3114 || (n > 3119 && n < 3125))
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
.elseif(n = 3053 || n = 3065 || n = 3077)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3055 || n = 3067 || n = 3079)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3056 || n = 3058 || n = 3064 || n = 3068 || n = 3070 || n = 3080 || n = 3092 || n = 3100 || n = 3116 || n = 3118)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 14 = 5) && ((n > 3056 && n < 3100)))
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3059 || n = 3095 || n = 3113)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3060 || n = 3094)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3061 || n = 3097 || n = 3115)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3062)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	php
	cmp $C5
	plp
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3063 || n = 3081 || n = 3093)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3069)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3072)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3075)
	; 14 bytes
	php
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3082)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3083 || n = 3111 || n = 3125)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3084)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3086)
	; 14 bytes
	php
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3089)
	; 13 bytes
	php
	pha
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3091)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3096)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3098)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3117)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3119)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3129 || n = 3132 || n = 3138 || n = 3140 || n = 3142 || n = 3146 || n = 3148 || n = 3156 || n = 3158 || n = 3160 || n = 3161 || n = 3163 || n = 3165 || n = 3166 || n = 3168 || (n > 3172 && n < 3176) || n = 3180 || n = 3182 || n = 3184 || (n > 3190 && n < 3195) || n = 3200)
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
.elseif(n = 3127 || n = 3155 || n = 3183 || n = 3197)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3128 || n = 3145 || n = 3162 || n = 3179 || n = 3196)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3130 || n = 3147 || n = 3164 || n = 3198)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3131 || n = 3149 || n = 3167 || n = 3185 || n = 3203)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3133 || n = 3151 || n = 3169 || n = 3187)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3134 || n = 3136 || n = 3144 || n = 3152 || n = 3154 || n = 3170 || n = 3172 || n = 3190)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3135 || n = 3143)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3137 || n = 3157 || n = 3177)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3139 || n = 3153 || n = 3181 || n = 3195)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3150)
	; 14 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3159 || n = 3199)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3171)
	; 14 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3176)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-4)/13 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3178)
	; 14 bytes
	php
	pha
	tya
	ldy #n/14-1 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3186)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3188)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-16)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3189)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3201)
	; 14 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3202)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 3204 to 3281
.if(n = 3204 || n = 3210 || n = 3212 || n = 3214 || n = 3216 || n = 3218 || n = 3220 || n = 3222 || (n > 3226 && n < 3230) || n = 3231 || n = 3233 || n = 3234 || n = 3236 || n = 3245 || n = 3246 || n = 3248 || n = 3250 || n = 3252 || (n > 3253 && n < 3257) || n = 3258 || (n > 3268 && n < 3273))
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
.elseif(n = 3205 || n = 3223 || n = 3241 || n = 3259 || n = 3277)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 2 = 0) && (n = 3206 || n = 3208 || n = 3224 || n = 3226 || (n > 3237 && n < 3245) || n = 3260 || n = 3262 || n = 3268 || n = 3278 || n = 3280))
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3207 || n = 3243 || n = 3261)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3209 || n = 3251 || n = 3279)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3211 || n = 3225 || n = 3253 || n = 3267 || n = 3281)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3213 || n = 3230 || n = 3247 || n = 3264)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3215 || n = 3232 || n = 3249 || n = 3266)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3217)
	; 14 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3219)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3221 || n = 3239 || n = 3257 || n = 3275)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3235)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3237)
	; 13 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3263)
	; 13 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3265)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3273)
	; 14 bytes
	php
	pha
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3274)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3276)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
; 3282 to 3359
.if(n = 3282 || (n > 3283 && n < 3291) || n = 3301 || n = 3302 || (n > 3303 && n < 3307) || n = 3308 || n = 3310 || n = 3312 || n = 3320 || n = 3322 || (n > 3323 && n < 3329) || n = 3330 || n = 3336 || (n > 3340 && n < 3347) || n = 3348 || (n > 3352 && n < 3357) || n = 3358)
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
.elseif(n = 3283 || n = 3300 || n = 3317 || n = 3334)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3291)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3292)
	; 14 bytes
	php
	pha
	clc
	lda #100 ;hides 'nop zp'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3293)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3294)
	; 14 bytes
	php
	pha
	tya
	ldy #100 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3295 || n = 3313 || n = 3331 || n = 3349)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3296 || n = 3314 || n = 3338 || n = 3340 || n = 3350 || n = 3352)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3297)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3298 || n = 3315 || n = 3332)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3299 || n = 3319 || n = 3339 || n = 3359)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3303)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3307 || n = 3321)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3309 || n = 3323 || n = 3351)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3311 || n = 3329 || n = 3347)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3316)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3318)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3333)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3335)
	; 13 bytes
	php
	pha
	clc
	lda #90 ;hides 'nop'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3337)
	; 13 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3357)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 3360 to 3437
.if((n > 3359 && n < 3363) || n = 3364 || (n > 3370 && n < 3377) || n = 3378 || (n > 3379 && n < 3383) || n = 3384 || n = 3389 || n = 3390 || n = 3392 || (n > 3393 && n < 3397) || n = 3408 || (n > 3409 && n < 3417) || n = 3418 || (n > 3425 && n < 3433))
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
.elseif((n .mod 14 = 3) && ((n > 3362 && n < 3406) || n = 3433))
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3365 || n = 3383 || n = 3401 || n = 3419 || n = 3437)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3366 || n = 3417 || n = 3434)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3367 || n = 3385 || n = 3403 || n = 3421)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3368 || n = 3402 || n = 3436)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3369 || n = 3387 || n = 3400)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3370 || n = 3386 || n = 3388 || n = 3406 || n = 3422 || n = 3424)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3379 || n = 3393 || n = 3407 || n = 3435)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3397)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
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
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3404)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-12)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3409)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3420)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3423)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3425)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if(n = 3438)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3439 || n = 3457 || n = 3475 || n = 3493 || n = 3511 || n = 3529 || n = 3547 || n = 3565 || n = 3583)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 2 = 0) && (n = 3440 || n = 3458 || n = 3460 || n = 3476 || n = 3478 || (n > 3489 && n < 3497) || n = 3512 || n = 3514 || n = 3518 || n = 3520 || n = 3530 || n = 3532 || n = 3548 || n = 3550 || n = 3566 || n = 3568 || n = 3584 || n = 3586))
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3441)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3442)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
	tay
	pla
	plp
.elseif((n > 3442 && n < 3447) || n = 3448 || n = 3450 || n = 3452 || n = 3454 || n = 3456 || n = 3462 || (n > 3463 && n < 3468) || n = 3469 || n = 3472 || n = 3474 || (n > 3479 && n < 3485) || n = 3486 || n = 3488 || n = 3498 || n = 3500 || n = 3501 || (n > 3505 && n < 3509) || n = 3510 || n = 3516 || (n > 3521 && n < 3527) || n = 3528 || n = 3534 || n = 3535 || (n > 3539 && n < 3545) || n = 3546 || n = 3551 || n = 3552 || n = 3554 || n = 3556 || n = 3558 || n = 3560 || n = 3562 || n = 3564 || n = 3569 || n = 3571 || n = 3574 || n = 3576 || n = 3578 || n = 3580 || n = 3582 || n = 3588 || (n > 3589 && n < 3594))
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
.elseif(n = 3447 || n = 3461 || n = 3503 || n = 3531 || n = 3559 || n = 3573 || n = 3587)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif((n .mod 14 = 5) && ((n > 3448 && n < 3478) || (n > 3504 && n < 3534) || (n > 3560 && n < 3590)))
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3451 || n = 3485 || n = 3502 || n = 3536 || n = 3553 || n = 3570)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3453)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3455 || n = 3473 || n = 3491 || n = 3509 || n = 3527 || n = 3545 || n = 3563 || n = 3581)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3459 || n = 3479 || n = 3499 || n = 3539 || n = 3579)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3468)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
	plp
.elseif(n = 3470 || n = 3504 || n = 3521 || n = 3538 || n = 3555 || n = 3572)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3471)
	; 14 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3487)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3489)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3495 || n = 3549 || n = 3567 || n = 3585)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3497 || n = 3537 || n = 3557 || n = 3577)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3513)
	; 14 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3515)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3517)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 3594 to 3750
.if((n > 3593 && n < 3597) || n = 3598 || n = 3600 || n = 3605 || (n > 3606 && n < 3617) || n = 3618 || (n > 3623 && n < 3635) || n = 3636 || (n > 3640 && n < 3653) || n = 3654 || (n > 3659 && n < 3667) || n = 3668 || n = 3670 || (n > 3679 && n < 3684) || (n > 3684 && n < 3689) || n = 3690 || n = 3696 || (n > 3700 && n < 3706) || (n > 3713 && n < 3717) || n = 3718 || (n > 3719 && n < 3723) || n = 3724 || n = 3726 || (n > 3730 && n < 3737) || n = 3738 || n = 3741 || n = 3744 || n = 3749 || n = 3750)
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
.elseif(n = 3597 || n = 3677 || n = 3717 || n = 3737)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3599 || n = 3617 || n = 3635 || n = 3653 || n = 3689 || n = 3707 || n = 3725 || n = 3743)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3601 || n = 3619 || n = 3637 || n = 3673 || n = 3691 || n = 3709 || n = 3727 || n = 3745)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3602 || n = 3620 || n = 3622 || n = 3656 || n = 3658 || n = 3674 || n = 3676 || n = 3692 || n = 3694 || n = 3698 || n = 3700 || n = 3710 || n = 3712 || n = 3728 || n = 3730 || n = 3746 || n = 3748)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3603)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3604 || n = 3621 || n = 3638 || n = 3672 || n = 3706 || n = 3723 || n = 3740)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3606 || n = 3623 || n = 3640 || n = 3708 || n = 3742)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3639 || n = 3659 || n = 3679 || n = 3699 || n = 3719 || n = 3739)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
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
.elseif(n = 3657 || n = 3675 || n = 3693 || n = 3711 || n = 3747)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3667)
	; 14 bytes
	php
	pha
	clc
	lda #90 ;hides 'nop'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3669)
	; 14 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3671)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
	pla
	plp
.elseif(n = 3678)
	; 14 bytes
	php
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	pla
	tax
	pla
	plp
.elseif(n = 3684)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3695)
	; 13 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3697)
	; 13 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3713)
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3729)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
; 3751 to 3906
.if((n > 3750 && n < 3757) || n = 3758 || n = 3760 || n = 3762 || (n > 3766 && n < 3774) || n = 3775 || n = 3778 || n = 3780 || n = 3785 || n = 3787 || n = 3789 || n = 3792 || (n > 3793 && n < 3797) || n = 3798 || (n > 3802 && n < 3808) || (n > 3810 && n < 3815) || n = 3816 || n = 3821 || n = 3822 || n = 3824 || n = 3826 || (n > 3827 && n < 3833) || n = 3834 || n = 3840 || n = 3841 || n = 3843 || (n > 3844 && n < 3848) || n = 3849 || n = 3850 || n = 3852 || n = 3858 || n = 3860 || (n > 3861 && n < 3869) || n = 3870 || n = 3875 || (n > 3879 && n < 3886) || n = 3894 || n = 3896 || n = 3898 || (n > 3899 && n < 3905) || n = 3906)
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
.elseif(n = 3757 || n = 3774 || n = 3791 || n = 3808 || n = 3825 || n = 3842 || n = 3859 || n = 3876 || n = 3893)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3759)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3761 || n = 3779 || n = 3797 || n = 3815 || n = 3833 || n = 3851 || n = 3869 || n = 3887 || n = 3905)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3763 || n = 3781 || n = 3799 || n = 3817 || n = 3835 || n = 3853 || n = 3871 || n = 3889)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3764 || n = 3766 || n = 3782 || n = 3784 || n = 3800 || n = 3802 || n = 3818 || n = 3820 || n = 3836 || n = 3838 || n = 3854 || n = 3856 || n = 3872 || n = 3890 || n = 3892)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3765 || n = 3783 || n = 3801 || n = 3855 || n = 3873 || n = 3891)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3776 || n = 3793 || n = 3810 || n = 3827 || n = 3844 || n = 3861 || n = 3878 || n = 3895)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3777)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3786)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3788)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3790)
	; 14 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3809)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3819 || n = 3839 || n = 3879 || n = 3899)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3823)
	; 14 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3837 || n = 3857 || n = 3877 || n = 3897)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3848)
	; 14 bytes
	php
	pha
	tya
	ldy #192 ;hides 'cpy #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3874)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3886)
	; 14 bytes
	php
	pha
	sec
	lda #(n-6)/20 ;hides 'nop #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3888)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-8)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 3907 to 4062
.if(n = 3907 || n = 3925 || n = 3943 || n = 3961 || n = 3979 || n = 3997 || n = 4015 || n = 4033 || n = 4051)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3908 || n = 3915 || n = 3926 || n = 3928 || n = 3962 || n = 3982 || n = 3998 || n = 4000 || n = 4016 || n = 4018 || n = 4020 || n = 4034 || n = 4036 || n = 4052 || n = 4054)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3909 || n = 3914 || n = 3945 || n = 3981 || n = 4035 || n = 4053)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3927 || n = 3944 || n = 3978 || n = 4012 || n = 4029 || n = 4046)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3911 || n = 3913 || n = 3918 || n = 3920 || n = 3922 || n = 3924 || (n > 3929 && n < 3937) || n = 3938 || n = 3940 || n = 3942 || (n > 3946 && n < 3953) || n = 3954 || n = 3956 || n = 3958 || n = 3960 || n = 3965 || (n > 3966 && n < 3975) || n = 3976 || n = 3984 || n = 3986 || (n > 3987 && n < 3995) || n = 3996 || (n > 4000 && n < 4012) || (n > 4020 && n < 4028) || n = 4030 || n = 4032 || n = 4038 || (n > 4039 && n < 4046) || n = 4047 || n = 4050 || n = 4055 || n = 4056 || n = 4058 || (n > 4059 && n < 4063))
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
.elseif(n = 3916)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-12)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3917 || n = 3957 || n = 4037 || n = 4057)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3919)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3921)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3923 || n = 3941 || n = 3959 || n = 3977 || n = 3995 || n = 4013 || n = 4031 || n = 4049)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3929 || n = 3946 || n = 3963 || n = 3980 || n = 4014 || n = 4048)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3937)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3939 || n = 3999 || n = 4019 || n = 4039 || n = 4059)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3953)
	; 14 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3955)
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3964)
	; 14 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3966)
	; 14 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3975)
	; 13 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3983)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3985)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3987)
	; 14 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4017)
	; 13 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	brk
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4028)
	; 14 bytes
	php
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4080 || n = 4097 || n = 4114 || n = 4131 || n = 4148 || n = 4165 || n = 4182 || n = 4199 || n = 4216 || n = 4233 || n = 4250 || n = 4284 || n = 4318 || n = 4352)
	; 14 bytes
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4064 || n = 4066 || n = 4068 || (n > 4072 && n < 4077) || n = 4078 || n = 4081 || n = 4083 || n = 4084 || n = 4086 || (n > 4090 && n < 4097) || n = 4098 || (n > 4099 && n < 4103) || n = 4104 || (n > 4108 && n < 4113) || n = 4115 || n = 4118 || n = 4120 || n = 4122 || (n > 4126 && n < 4131) || n = 4132 || n = 4135 || n = 4136 || n = 4138 || n = 4140 || (n > 4144 && n < 4148) || n = 4149 || (n > 4150 && n < 4155) || n = 4156 || n = 4158 || n = 4163 || n = 4164 || n = 4166 || n = 4168 || n = 4170 || n = 4173 || n = 4174 || n = 4181 || n = 4183 || (n > 4184 && n < 4193) || n = 4194 || n = 4200 || (n > 4201 && n < 4211) || n = 4212 || (n > 4219 && n < 4229) || n = 4230 || n = 4236 || n = 4238 || (n > 4239 && n < 4247) || n = 4248 || (n > 4252 && n < 4257) || n = 4258 || (n > 4259 && n < 4264) || n = 4266 || (n > 4270 && n < 4277) || n = 4278 || n = 4281 || n = 4282 || (n > 4288 && n < 4297) || n = 4298 || n = 4300 || n = 4302 || (n > 4306 && n < 4317) || (n > 4324 && n < 4335) || n = 4336 || (n > 4342 && n < 4352) || n = 4353 || n = 4356 || (n > 4360 && n < 4369) || n = 4370 || n = 4372 || n = 4374)
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
.elseif(n = 4065 || n = 4082 || n = 4099 || n = 4116 || n = 4133 || n = 4150 || n = 4167 || n = 4184 || n = 4201 || n = 4218 || n = 4235 || n = 4252 || n = 4269 || n = 4286 || n = 4320 || n = 4354)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4067 || n = 4085 || n = 4103 || n = 4121 || n = 4139 || n = 4157 || n = 4175 || n = 4193 || n = 4211 || n = 4229 || n = 4247 || n = 4265 || n = 4283 || n = 4301 || n = 4319 || n = 4337 || n = 4355 || n = 4373)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4069 || n = 4087 || n = 4105 || n = 4123 || n = 4141 || n = 4159 || n = 4177 || n = 4195 || n = 4213 || n = 4231 || n = 4249 || n = 4267 || n = 4285 || n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4070 || n = 4072 || n = 4088 || n = 4090 || n = 4106 || n = 4108 || n = 4124 || n = 4126 || n = 4142 || n = 4160 || n = 4162 || n = 4172 || n = 4178 || n = 4180 || n = 4196 || n = 4198 || n = 4214 || n = 4232 || n = 4234 || n = 4268 || n = 4270 || n = 4288 || n = 4304 || n = 4306 || n = 4322 || n = 4324 || n = 4338 || n = 4340 || n = 4342 || n = 4358 || n = 4360)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4071 || n = 4089 || n = 4107 || n = 4125 || n = 4143 || n = 4161 || n = 4171 || n = 4215 || n = 4251 || n = 4287 || n = 4305 || n = 4323 || n = 4341)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4077)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
	tax
	pla
	plp
.elseif(n = 4079 || n = 4179 || n = 4219 || n = 4239 || n = 4259 || n = 4279 || n = 4299 || n = 4359)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4113)
	; 14 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4117 || n = 4137 || n = 4197 || n = 4217 || n = 4237 || n = 4257 || n = 4277 || n = 4297 || n = 4317)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4119)
	; 14 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4134)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4144)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4155)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 4176)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4264)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4280)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop $04
	brk
	Jne *-5
	tax
	pla
	plp
.elseif(n = 4335)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4369)
	; 14 bytes
	php
	pha
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4371)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4378 || n = 4380 || n = 4394 || n = 4396 || n = 4412 || n = 4414 || n = 4430 || n = 4432 || n = 4448 || n = 4450 || n = 4466 || n = 4468 || n = 4484 || n = 4486 || n = 4502 || n = 4504 || n = 4520 || n = 4522)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4377)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4379 || n = 4399 || n = 4419 || n = 4439 || n = 4459 || n = 4479)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n > 4380 && n < 4385) || n = 4386 || (n > 4387 && n < 4391) || n = 4392 || n = 4398 || (n > 4399 && n < 4409) || n = 4410 || n = 4415 || n = 4416 || n = 4418 || (n > 4419 && n < 4423) || n = 4424 || (n > 4432 && n < 4437) || n = 4438 || (n > 4439 && n < 4445) || n = 4446 || (n > 4450 && n < 4457) || n = 4458 || n = 4460 || n = 4462 || n = 4464 || (n > 4468 && n < 4477) || n = 4478 || n = 4480 || n = 4482 || n = 4487 || (n > 4488 && n < 4492) || n = 4493 || n = 4495 || n = 4496 || n = 4498 || n = 4500 || (n > 4505 && n < 4517) || n = 4518 || (n > 4522 && n < 4526) || (n > 4528 && n < 4532))
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
.elseif(n = 4385)
	; 14 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4387)
	; 14 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4391 || n = 4409 || n = 4427 || n = 4445 || n = 4463 || n = 4481 || n = 4499 || n = 4517)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4393 || n = 4411 || n = 4429 || n = 4447 || n = 4465 || n = 4483 || n = 4501 || n = 4519)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4395 || n = 4413 || n = 4428 || n = 4431 || n = 4449 || n = 4467 || n = 4485 || n = 4503 || n = 4521)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4397 || n = 4437 || n = 4457 || n = 4477 || n = 4497)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4417)
	; 14 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4423)
	; 14 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4425)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
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
.elseif(n = 4461)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4488)
	; 14 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4492)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4494)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4505)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4526)
	; 14 bytes
	php
	pha
	sec
	lda #(n-6)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4527)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4528)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-8)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4535) || n = 4536 || (n > 4540 && n < 4553) || n = 4554 || (n > 4559 && n < 4571) || n = 4572 || n = 4578 || (n > 4579 && n < 4589) || n = 4590 || n = 4595 || n = 4596 || n = 4606 || n = 4608)
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
.elseif(n = 4535 || n = 4553 || n = 4571 || n = 4589 || n = 4607)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4591 || n = 4609)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4538 || n = 4540 || n = 4556 || n = 4558 || n = 4574 || n = 4576 || n = 4592 || n = 4594 || n = 4601)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4539 || n = 4559 || n = 4579 || n = 4599)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4557 || n = 4577 || n = 4597)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4575 || n = 4593 || n = 4600 || n = 4602 || n = 4603)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4598)
	; 12 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4604)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4605)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.endif
.else
; 4610 to 4687
.if(n = 4610 || n = 4612 || n = 4619 || n = 4628 || n = 4630 || n = 4658 || n = 4660 || n = 4686)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4611 || n = 4618 || n = 4620 || n = 4629 || n = 4685)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4613 || n = 4624 || n = 4626 || (n > 4630 && n < 4636) || (n > 4639 && n < 4655) || n = 4656 || (n > 4660 && n < 4668) || n = 4669 || (n > 4670 && n < 4677) || n = 4678 || n = 4680 || n = 4682 || n = 4684 || n = 4687)
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
.elseif(n = 4614)
	; 12 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4615)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 4616)
	; 12 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4617)
	; 13 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4621)
	; 14 bytes
	php
	pha
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 4622)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4623)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 4625)
	; 13 bytes
	php
	pha
	sec
	lda #0
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4627)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	brk
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 4636)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4637 || n = 4677)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4638)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4639 || n = 4659 || n = 4679)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4655)
	; 13 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 4657)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4668)
	; 14 bytes
	php
	pha
	sec
	lda #(n-4)/22 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4670)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-6)/22 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4681)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
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
.endif
.endif
.endif
.else
; 4688 to 5000
.if((n > 4687 && n < 4692) || n = 4693 || n = 4694 || n = 4696 || n = 4701 || n = 4702 || (n > 4703 && n < 4717) || n = 4718 || (n > 4719 && n < 4725) || n = 4726 || n = 4728 || (n > 4729 && n < 4737) || n = 4738 || (n > 4739 && n < 4757) || n = 4758 || (n > 4759 && n < 4765) || n = 4766 || n = 4768 || (n > 4769 && n < 4777) || n = 4778 || (n > 4779 && n < 4785) || (n > 4785 && n < 4795) || n = 4796 || n = 4798 || (n > 4799 && n < 4806) || (n > 4806 && n < 4811) || n = 4812 || (n > 4813 && n < 4817) || n = 4818 || (n > 4819 && n < 4827) || (n > 4827 && n < 4837) || n = 4838 || (n > 4839 && n < 4857) || n = 4858 || (n > 4859 && n < 4877) || n = 4878 || (n > 4879 && n < 4897) || n = 4898 || (n > 4899 && n < 4917) || n = 4918 || (n > 4919 && n < 4937) || n = 4938 || n = 4941 || (n > 4943 && n < 4957) || n = 4958 || (n > 4959 && n < 4975) || n = 4976 || (n > 4980 && n < 4997) || n = 4998 || n = 5000)
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
.elseif(n = 4692)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4695)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4697)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4698 || n = 4700 || n = 4943 || n = 4978 || n = 4980)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4699 || n = 4719 || n = 4739 || n = 4759 || n = 4779 || n = 4799 || n = 4819 || n = 4839 || n = 4859 || n = 4879 || n = 4899 || n = 4919 || n = 4939 || n = 4959 || n = 4979 || n = 4999)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4703)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4717 || n = 4737 || n = 4757 || n = 4777 || n = 4797 || n = 4817 || n = 4837 || n = 4857 || n = 4877 || n = 4897 || n = 4917 || n = 4937 || n = 4957 || n = 4997)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4725)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4727)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4729)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4765)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4767)
	; 14 bytes
	php
	pha
	tya
	ldy #250 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4769)
	; 14 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4785)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4795)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4806)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4811)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/22 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4813)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/22 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4827)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 4942)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4975)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4977)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
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
; 5001 to 5156
.if((n > 5000 && n < 5015) || n = 5016 || (n > 5020 && n < 5037) || n = 5038 || (n > 5039 && n < 5057) || n = 5058 || (n > 5059 && n < 5075) || n = 5076 || n = 5078 || (n > 5079 && n < 5092) || (n > 5092 && n < 5097) || n = 5098 || (n > 5099 && n < 5108) || n = 5109 || (n > 5111 && n < 5117) || n = 5118 || n = 5120 || (n > 5121 && n < 5125) || (n > 5131 && n < 5137) || n = 5138 || (n > 5139 && n < 5157))
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
.elseif(n = 5015)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5017)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5018 || n = 5020 || n = 5111 || n = 5131)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5019 || n = 5039 || n = 5059 || n = 5079 || n = 5099 || n = 5119)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5037 || n = 5057 || n = 5077 || n = 5097 || n = 5117)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5075)
	; 14 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 5092)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	tax
	pla
	plp
.elseif(n = 5108)
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5110 || n = 5129 || n = 5130)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5121)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5125)
	; 13 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5126)
	; 13 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5127)
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5128)
	; 13 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 5137)
	; 14 bytes
	php
	pha
	sec
	lda #0
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5139)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	nop
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 5157 to 5312
.if((n > 5156 && n < 5163) || n = 5164 || (n > 5165 && n < 5187) || (n > 5187 && n < 5196) || (n > 5200 && n < 5207) || n = 5208 || (n > 5209 && n < 5231) || (n > 5231 && n < 5313))
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
.elseif(n = 5163)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/22 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5165)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/22 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5187)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5196)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	adc #1
	Jne *-6
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
.elseif(n = 5198)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5199)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5200)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5207)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 5209)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5231)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
; 5313 to 5625
.if((n > 5312 && n < 5363) || (n > 5363 && n < 5372) || n = 5373 || (n > 5374 && n < 5381) || (n > 5385 && n < 5407) || (n > 5407 && n < 5454) || n = 5455 || (n > 5457 && n < 5471) || n = 5472 || (n > 5473 && n < 5515) || n = 5516 || (n > 5517 && n < 5618) || (n > 5618 && n < 5626))
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
.elseif(n = 5363)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	cmp $C5
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5372)
	; 14 bytes
	php
	pha
	sec
	lda #(n-4)/22 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5374)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-6)/22 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5381)
	; 14 bytes
	php
	pha
	clc
	lda #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 5382)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5383)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5384)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5385)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5407)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 5456)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5457)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5471)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5473)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5515)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/22 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5517)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/22 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5618)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5636) || (n > 5640 && n < 5711) || n = 5712 || (n > 5714 && n < 5968) || n = 5969 || (n > 5971 && n < 6225) || n = 6226 || (n > 6228 && n < 6251))
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
.elseif(n = 5636)
	; 14 bytes
	php
	pha
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 5637)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5638)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5639)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5640)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
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
.elseif(n = 5713 || n = 5970 || n = 6227)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5714 || n = 5971 || n = 6228)
	; 14 bytes
	pha
	delay_n (n-3)
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
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6310) || (n > 6310 && n < 6383) || (n > 6383 && n < 6401) || n = 6402 || n = 6404 || n = 6405 || n = 6407 || (n > 6408 && n < 6482) || n = 6483 || (n > 6485 && n < 6739) || n = 6740 || (n > 6742 && n < 6996) || n = 6997 || (n > 6999 && n < 7253) || n = 7254 || (n > 7256 && n < 7501))
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
.elseif(n = 6310)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
	pla
	plp
.elseif(n = 6383)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 6401)
	; 14 bytes
	php
	pha
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 6403)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 6406)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 6408)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	tay
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
.elseif(n = 6484 || n = 6741 || n = 6998 || n = 7255)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6485 || n = 6742 || n = 6999 || n = 7256)
	; 14 bytes
	pha
	delay_n (n-3)
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
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7510) || n = 7511 || (n > 7513 && n < 7767) || n = 7768 || (n > 7770 && n < 7913) || (n > 7913 && n < 7931) || n = 7932 || (n > 7933 && n < 7942) || n = 7943 || (n > 7944 && n < 8024) || n = 8025 || (n > 8027 && n < 8281) || n = 8282 || (n > 8284 && n < 8538) || n = 8539 || (n > 8541 && n < 8795) || n = 8796 || (n > 8798 && n < 9052) || n = 9053 || (n > 9055 && n < 9309) || n = 9310 || (n > 9312 && n < 9566) || n = 9567 || (n > 9569 && n < 9823) || n = 9824 || (n > 9826 && n < 10001))
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
.elseif(n = 7512 || n = 7769 || n = 8026 || n = 8283 || n = 8540 || n = 8797 || n = 9054 || n = 9311 || n = 9568 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7513 || n = 7770 || n = 8027 || n = 8284 || n = 8541 || n = 8798 || n = 9055 || n = 9312 || n = 9569 || n = 9826)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 7913)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7931)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7933)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7942)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7944)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	tay
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
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10080) || n = 10081 || (n > 10083 && n < 10337) || n = 10338 || (n > 10340 && n < 10594) || n = 10595 || (n > 10597 && n < 10851) || n = 10852 || (n > 10854 && n < 11108) || n = 11109 || (n > 11111 && n < 11365) || n = 11366 || (n > 11368 && n < 11622) || n = 11623 || (n > 11625 && n < 11879) || n = 11880 || (n > 11882 && n < 12136) || n = 12137 || (n > 12139 && n < 12393) || n = 12394 || (n > 12396 && n < 12650) || n = 12651 || (n > 12653 && n < 12907) || n = 12908 || (n > 12910 && n < 13164) || n = 13165 || (n > 13167 && n < 13421) || n = 13422 || (n > 13424 && n < 13678) || n = 13679 || (n > 13681 && n < 13935) || n = 13936 || (n > 13938 && n < 14192) || n = 14193 || (n > 14195 && n < 14449) || n = 14450 || (n > 14452 && n < 14706) || n = 14707 || (n > 14709 && n < 14963) || n = 14964 || (n > 14966 && n < 15001))
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
.elseif(n = 10083 || n = 10340 || n = 10597 || n = 10854 || n = 11111 || n = 11368 || n = 11625 || n = 11882 || n = 12139 || n = 12396 || n = 12653 || n = 12910 || n = 13167 || n = 13424 || n = 13681 || n = 13938 || n = 14195 || n = 14452 || n = 14709 || n = 14966)
	; 14 bytes
	pha
	delay_n (n-3)
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
.if((n > 15000 && n < 15220) || n = 15221 || (n > 15223 && n < 15477) || n = 15478 || (n > 15480 && n < 15734) || n = 15735 || (n > 15737 && n < 15991) || n = 15992 || (n > 15994 && n < 16248) || n = 16249 || (n > 16251 && n < 16505) || n = 16506 || (n > 16508 && n < 16762) || n = 16763 || (n > 16765 && n < 17019) || n = 17020 || (n > 17022 && n < 17276) || n = 17277 || (n > 17279 && n < 17533) || n = 17534 || (n > 17536 && n < 17790) || n = 17791 || (n > 17793 && n < 18047) || n = 18048 || (n > 18050 && n < 18304) || n = 18305 || (n > 18307 && n < 18561) || n = 18562 || (n > 18564 && n < 18818) || n = 18819 || (n > 18821 && n < 19075) || n = 19076 || (n > 19078 && n < 19332) || n = 19333 || (n > 19335 && n < 19589) || n = 19590 || (n > 19592 && n < 19846) || n = 19847 || (n > 19849 && n < 20001))
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
.elseif(n = 15223 || n = 15480 || n = 15737 || n = 15994 || n = 16251 || n = 16508 || n = 16765 || n = 17022 || n = 17279 || n = 17536 || n = 17793 || n = 18050 || n = 18307 || n = 18564 || n = 18821 || n = 19078 || n = 19335 || n = 19592 || n = 19849)
	; 14 bytes
	pha
	delay_n (n-3)
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
