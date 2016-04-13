; delay_n, delays n cycles with small code
; Will clobber A, X, Y, and C
; Will preserve S, Z+N, D, V, and I (interrupt-disable flag)
;
; WARNING: If an interrupt happens in the middle of this code, data corruption can occur.
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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 24
; - 8 bytes or less when n <= 31
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=0, avg=2.4, max=4
; - When 11 <= n <= 20: min=3, avg=5.0, max=7
; - When 21 <= n <= 30: min=6, avg=7.1, max=8
; - When 31 <= n <= 50: min=6, avg=7.0, max=9
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.2, max=9
; - When 501 <= n <= 750: min=6, avg=8.2, max=9
; - When 751 <= n <= 1000: min=7, avg=8.2, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.6, max=9
; - When 2001 <= n <= 3000: min=8, avg=9.0, max=9
; - When 3001 <= n <= 5000: min=9, avg=9.0, max=9
; - When 5001 <= n <= 7500: min=9, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=9, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=9, avg=9.0, max=9
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
.error .sprintf("delay_n: INTERNAL ERROR: UNABLE TO DEVISE DELAY CODE FOR %d CYCLES", n)
.elseif(n = 4 || n = 6)
	; 2..3 bytes
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
	; 4 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 5 bytes
	php
	tsx
	plp
	txs
	plp
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
.endif
.else
; 20 to 39
.if(n = 20)
	; 7 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 21)
	; 6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 22)
	; 7 bytes
	php
	tsx
	delay_n 7
	plp
	txs
	plp
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 25)
	; 8 bytes
	delay_n 4
	delay_n (n-4)
.elseif(n = 26)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 27)
	; 7 bytes
	php
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	plp
.elseif(n = 28)
	; 7 bytes
	php
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	plp
.elseif(n = 29)
	; 8 bytes
	php
	sec
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	plp
.elseif(n = 30)
	; 8 bytes
	php
	clc
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	plp
.elseif(n = 31)
	; 6 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 32)
	; 9 bytes
	php
	tya
	ldy #4
	dey
	Jne *-1
	tay
	plp
.elseif(n = 33)
	; 7 bytes
	php
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif((n > 33 && n < 40))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.endif
.endif
.else
; 40 to 78
.if((n > 39 && n < 46) || (n > 46 && n < 56) || (n > 56 && n < 79))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 46)
	; 6 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	plp
.elseif(n = 56)
	; 6 bytes
	php
	ldy #136 ;hides 'dey'
	Jmi *-1
	plp
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 131) || (n > 131 && n < 157))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 131)
	; 6 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 290))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 290 || n = 305 || n = 310)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 291)
	; 6 bytes
	php
	ldy #200 ;hides 'iny'
	Jmi *-1
	plp
.elseif(n = 292 || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 293 || n = 298 || n = 303)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 294)
	; 8 bytes
	php
	ldy $A4
	ldy #200 ;hides 'iny'
	Jmi *-1
	plp
.elseif(n = 295)
	; 8 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	plp
.elseif(n = 297 || n = 299 || n = 301 || n = 304 || n = 312)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 300)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
	plp
.elseif(n = 302 || n = 309)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 306)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 307)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	dex
	Jpl *-2
	plp
.elseif(n = 308)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 311)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if((n .mod 5 = 3) && ((n > 312 && n < 324) || n = 338 || (n > 347 && n < 359) || (n > 367 && n < 409) || n = 418 || n = 433 || n = 448 || (n > 457 && n < 469)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 314 || n = 317 || n = 322 || n = 324 || n = 326 || n = 329 || n = 331 || n = 332 || n = 334 || n = 336 || n = 341 || n = 342 || n = 346 || n = 347 || n = 349 || n = 352 || n = 354 || n = 356 || n = 357 || n = 359 || n = 361 || n = 362 || n = 364 || n = 367 || n = 369 || n = 371 || n = 374 || n = 376 || n = 377 || n = 381 || n = 382 || n = 387 || n = 391 || n = 392 || n = 394 || n = 396 || n = 397 || n = 399 || n = 401 || n = 404 || n = 406 || n = 409 || n = 412 || n = 416 || n = 417 || n = 422 || n = 424 || n = 426 || n = 427 || n = 429 || n = 431 || n = 432 || n = 434 || n = 436 || n = 437 || n = 439 || n = 441 || n = 444 || n = 446 || n = 447 || n = 451 || n = 452 || n = 454 || n = 457 || n = 459 || n = 461 || n = 462 || n = 464 || n = 466 || n = 467)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif((n .mod 5 = 0) && ((n > 314 && n < 326) || (n > 334 && n < 361) || (n > 369 && n < 386) || n = 395 || (n > 409 && n < 431) || (n > 439 && n < 466)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 316 || n = 330 || n = 337 || n = 351 || n = 372 || n = 407 || n = 414 || n = 421 || n = 435 || n = 442 || n = 449)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 319)
	; 8 bytes
	php
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.elseif(n = 321)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
	plp
.elseif(n = 327)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
	plp
.elseif(n = 328 || n = 333 || n = 343 || n = 363 || n = 413 || n = 423 || n = 428 || n = 438 || n = 443 || n = 453)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 339)
	; 8 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 344 || n = 365 || n = 379 || n = 400 || n = 456)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 366)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 384)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
	plp
.elseif(n = 386)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif(n = 389)
	; 8 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	Jmi *-1
	plp
.elseif(n = 390)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	plp
.elseif(n = 402)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 405)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
	plp
.elseif(n = 411)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 419)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
	plp
.endif
.else
; 469 to 625
.if(n = 469 || n = 472 || n = 476 || n = 481 || n = 482 || n = 486 || n = 489 || n = 494 || n = 497 || n = 499 || n = 502 || n = 504 || n = 506 || n = 507 || n = 509 || n = 511 || n = 514 || n = 516 || n = 521 || n = 522 || n = 527 || n = 529 || n = 532 || n = 534 || n = 536 || n = 539 || n = 541 || n = 542 || n = 544 || n = 549 || n = 552 || n = 556 || n = 557 || n = 562 || n = 564 || n = 566 || n = 569 || n = 571 || n = 572 || n = 574 || n = 576 || n = 577 || n = 579 || n = 581 || n = 584 || n = 586 || n = 587 || n = 591 || n = 592 || n = 594 || n = 597 || n = 599 || n = 601 || n = 602 || n = 604 || n = 606 || n = 607 || n = 609 || n = 611 || n = 612 || n = 614 || n = 616 || n = 619 || n = 621)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif((n .mod 7 = 1) && (n = 470 || n = 484 || n = 491 || (n > 511 && n < 527) || (n > 539 && n < 562) || n = 582 || n = 624))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 471)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 473 || n = 478 || n = 508 || n = 528 || n = 558 || n = 573 || n = 593 || n = 598 || n = 608)
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 474)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
	plp
.elseif((n .mod 5 = 0) && ((n > 474 && n < 501) || (n > 514 && n < 536) || (n > 544 && n < 571) || (n > 579 && n < 606) || (n > 614 && n < 626)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 477 || n = 505 || n = 575 || n = 589 || n = 596 || n = 610 || n = 617)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 479)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && ((n > 482 && n < 504) || (n > 512 && n < 524) || (n > 532 && n < 554) || n = 563 || n = 568 || (n > 577 && n < 589) || n = 603 || (n > 612 && n < 624)))
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 487)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 492)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
	plp
.elseif(n = 496)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
	plp
.elseif(n = 501)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.elseif(n = 510)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
	plp
.elseif(n = 517)
	; 7 bytes
	php
	ldx #200 ;hides 'iny'
	dex
	Jmi *-2
	plp
.elseif(n = 524)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	Jne *-2
	plp
.elseif(n = 531)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
	plp
.elseif(n = 537)
	; 8 bytes
	php
	ldx #138 ;hides 'txa'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 546)
	; 8 bytes
	php
	ldx #136 ;hides 'dey'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 551)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 559)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 567)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 622)
	; 7 bytes
	php
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
	plp
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626 || n = 632 || n = 634 || n = 637 || n = 639 || n = 641 || n = 642 || n = 644 || n = 646 || n = 647 || n = 649 || n = 651 || n = 656 || n = 657 || n = 661 || n = 662 || n = 664 || n = 667 || n = 669 || n = 671 || n = 674 || n = 676 || n = 677 || n = 679 || n = 682 || n = 684 || n = 686 || n = 689 || n = 692 || n = 696 || n = 697 || n = 702 || n = 704 || n = 706 || n = 707 || n = 709 || n = 711 || n = 712 || n = 714 || n = 716 || n = 717 || n = 721 || n = 724 || n = 726 || n = 731 || n = 732 || n = 737 || n = 739 || n = 742 || n = 744 || n = 746 || n = 747 || n = 749 || n = 752 || n = 754 || n = 756 || n = 759 || n = 761 || n = 766 || n = 767 || n = 769 || n = 772 || n = 774 || n = 776 || n = 777 || n = 779 || n = 781)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 627)
	; 8 bytes
	php
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
	plp
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || n = 643 || n = 648 || (n > 657 && n < 779)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 629)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
	plp
.elseif((n .mod 5 = 0) && ((n > 629 && n < 641) || (n > 649 && n < 676) || n = 685 || n = 690 || (n > 699 && n < 711) || (n > 719 && n < 746) || (n > 759 && n < 781)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 631 || n = 659 || n = 666 || n = 736)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 636)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	iny
	Jne *-3
	plp
.elseif(n = 638 || n = 653)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif((n .mod 7 = 1) && (n = 645 || n = 652 || (n > 679 && n < 702) || (n > 714 && n < 730) || (n > 749 && n < 772)))
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 654)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 672)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 681)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 691)
	; 6 bytes
	php
	ldy #136 ;hides 'dey'
	Jne *-1
	plp
.elseif(n = 695)
	; 8 bytes
	php
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	plp
.elseif(n = 699)
	; 8 bytes
	php
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 719)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
	plp
.elseif(n = 727)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 734)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
	plp
.elseif(n = 741)
	; 7 bytes
	php
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
	plp
.elseif(n = 751)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 755)
	; 7 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
	plp
.elseif(n = 762)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.endif
.else
; 782 to 937
.if(n = 782 || n = 784 || n = 786 || n = 787 || n = 791 || n = 794 || n = 796 || n = 797 || n = 801 || n = 802 || n = 804 || n = 809 || n = 811 || n = 812 || n = 814 || n = 816 || n = 817 || n = 819 || n = 821 || n = 822 || n = 824 || n = 826 || n = 829 || n = 836 || n = 837 || n = 839 || n = 842 || n = 844 || n = 847 || n = 849 || n = 851 || n = 852 || n = 854 || n = 856 || n = 857 || n = 859 || n = 861 || n = 864 || n = 866 || n = 867 || n = 871 || n = 872 || n = 874 || n = 877 || n = 879 || n = 881 || n = 882 || n = 884 || n = 886 || n = 887 || n = 889 || n = 891 || n = 892 || n = 894 || n = 896 || n = 899 || n = 901 || n = 902 || n = 907 || n = 909 || n = 912 || n = 914 || n = 916 || n = 917 || n = 919 || n = 921 || n = 922 || n = 924 || n = 926 || n = 927 || n = 929 || n = 931 || n = 934 || n = 936 || n = 937)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif((n .mod 5 = 3) && ((n > 782 && n < 934)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif(n = 785 || n = 792 || n = 806 || n = 820 || n = 841 || n = 855 || n = 862 || n = 869 || n = 890 || n = 911)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
	plp
.elseif(n = 789)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif((n .mod 5 = 0) && ((n > 789 && n < 816) || (n > 824 && n < 851) || (n > 859 && n < 886) || (n > 894 && n < 921) || n = 930 || n = 935))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 799 || n = 827 || n = 834 || n = 876 || n = 897 || n = 904 || n = 925 || n = 932)
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 807)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 831)
	; 8 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	dex
	Jpl *-3
	plp
.elseif(n = 832)
	; 7 bytes
	php
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
	plp
.elseif(n = 846)
	; 7 bytes
	php
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
	plp
.elseif(n = 906)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
	plp
.endif
.endif
.else
; 938 to 1250
.if((n .mod 5 = 3) && ((n > 937 && n < 1249)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 939 || n = 946 || (n > 959 && n < 982) || (n > 994 && n < 1017) || (n > 1029 && n < 1052) || (n > 1064 && n < 1087) || (n > 1099 && n < 1122) || (n > 1134 && n < 1157) || (n > 1169 && n < 1192) || (n > 1204 && n < 1227) || n = 1240 || n = 1247))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 0) && ((n > 939 && n < 956) || (n > 969 && n < 991) || (n > 999 && n < 1021) || (n > 1034 && n < 1061) || (n > 1074 && n < 1096) || (n > 1104 && n < 1131) || (n > 1139 && n < 1166) || (n > 1174 && n < 1201) || (n > 1209 && n < 1236) || n = 1245 || n = 1250))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 941 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 956 || n = 957 || n = 959 || n = 961 || n = 962 || n = 964 || n = 966 || n = 971 || n = 976 || n = 977 || n = 979 || n = 982 || n = 984 || n = 986 || n = 987 || n = 989 || n = 992 || n = 994 || n = 996 || n = 997 || n = 1001 || n = 1004 || n = 1006 || n = 1007 || n = 1011 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1026 || n = 1027 || n = 1029 || n = 1031 || n = 1032 || n = 1034 || n = 1036 || n = 1039 || n = 1041 || n = 1042 || n = 1046 || n = 1047 || n = 1049 || n = 1052 || n = 1054 || n = 1057 || n = 1061 || n = 1062 || n = 1064 || n = 1066 || n = 1067 || n = 1069 || n = 1071 || n = 1074 || n = 1076 || n = 1081 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1091 || n = 1092 || n = 1094 || n = 1096 || n = 1097 || n = 1099 || n = 1101 || n = 1102 || n = 1104 || n = 1106 || n = 1109 || n = 1111 || n = 1112 || n = 1116 || n = 1117 || n = 1119 || n = 1124 || n = 1126 || n = 1127 || n = 1129 || n = 1131 || n = 1132 || n = 1134 || n = 1136 || n = 1137 || n = 1139 || n = 1141 || n = 1144 || n = 1146 || n = 1147 || n = 1151 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1161 || n = 1162 || n = 1164 || n = 1166 || n = 1167 || n = 1169 || n = 1171 || n = 1172 || n = 1174 || n = 1176 || n = 1179 || n = 1181 || n = 1186 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1197 || n = 1199 || n = 1201 || n = 1202 || n = 1204 || n = 1206 || n = 1207 || n = 1209 || n = 1211 || n = 1214 || n = 1216 || n = 1217 || n = 1221 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1231 || n = 1232 || n = 1234 || n = 1236 || n = 1237 || n = 1241 || n = 1242 || n = 1244 || n = 1246 || n = 1249)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 942)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 951)
	; 8 bytes
	php
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
	plp
.elseif(n = 965)
	; 7 bytes
	php
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	plp
.elseif(n = 969)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
	plp
.elseif(n = 972)
	; 7 bytes
	php
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
	plp
.elseif(n = 991)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 999)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 1021)
	; 6 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1024)
	; 8 bytes
	php
	ldx $A6
	ldx #202 ;hides 'dex'
	Jne *-1
	plp
.elseif(n = 1025)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	plp
.elseif(n = 1056)
	; 7 bytes
	php
	ldx #106 ;hides 'ror a'
	inx
	Jne *-2
	plp
.elseif(n = 1059)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1070)
	; 7 bytes
	php
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
	plp
.elseif(n = 1077)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 1122)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1182)
	; 7 bytes
	php
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
	plp
.elseif(n = 1196)
	; 7 bytes
	php
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
	plp
.elseif(n = 1239)
	; 8 bytes
	php
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
; 1251 to 1406
.if(n = 1251 || n = 1252 || n = 1256 || n = 1259 || n = 1262 || n = 1264 || n = 1267 || n = 1269 || n = 1271 || n = 1272 || n = 1274 || n = 1276 || n = 1277 || n = 1279 || n = 1281 || n = 1284 || n = 1286 || n = 1287 || (n > 1290 && n < 1294) || n = 1295 || (n > 1296 && n < 1303) || (n > 1303 && n < 1308) || n = 1309 || (n > 1311 && n < 1317) || n = 1318 || (n > 1319 && n < 1324) || n = 1325 || n = 1326 || (n > 1327 && n < 1331) || (n > 1331 && n < 1335) || n = 1336 || n = 1337 || (n > 1338 && n < 1343) || n = 1344 || (n > 1345 && n < 1352) || (n > 1352 && n < 1356) || n = 1357 || n = 1358 || (n > 1359 && n < 1366) || (n > 1366 && n < 1373) || (n > 1374 && n < 1380) || n = 1381 || n = 1382 || (n > 1383 && n < 1387) || (n > 1387 && n < 1394) || (n > 1394 && n < 1401) || (n > 1401 && n < 1406))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif((n .mod 5 = 3) && ((n > 1252 && n < 1284)))
	; 7 bytes
	php
	ldx #(n-8)/5
	dex
	Jne *-1
	plp
.elseif((n .mod 7 = 1) && (n = 1254 || n = 1261 || (n > 1274 && n < 1402)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1271) || n = 1285 || n = 1290))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1257)
	; 8 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1266)
	; 8 bytes
	php
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1280)
	; 7 bytes
	php
	ldx #74 ;hides 'lsr a'
	inx
	Jne *-2
	plp
.elseif(n = 1288)
	; 7 bytes
	php
	ldx #0
	dex
	Jne *-1
	plp
.elseif(n = 1294)
	; 7 bytes
	php
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
	plp
.elseif(n = 1308)
	; 7 bytes
	php
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
	plp
.elseif(n = 1311)
	; 8 bytes
	php
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1319)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1327)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1335)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1343)
	; 8 bytes
	php
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
	plp
.elseif(n = 1356)
	; 8 bytes
	php
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1374)
	; 8 bytes
	php
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1383)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1406)
	; 7 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	plp
.endif
.else
; 1407 to 1562
.if(n = 1407 || (n > 1408 && n < 1415) || (n > 1415 && n < 1420) || n = 1421 || (n > 1422 && n < 1427) || n = 1428 || (n > 1429 && n < 1436) || (n > 1436 && n < 1443) || (n > 1443 && n < 1450) || (n > 1450 && n < 1457) || (n > 1457 && n < 1464) || (n > 1464 && n < 1471) || (n > 1471 && n < 1478) || (n > 1478 && n < 1485) || (n > 1485 && n < 1492) || n = 1493 || n = 1494 || (n > 1495 && n < 1499) || (n > 1499 && n < 1503) || n = 1505 || (n > 1506 && n < 1511) || n = 1512 || (n > 1513 && n < 1518) || n = 1519 || (n > 1520 && n < 1527) || (n > 1527 && n < 1534) || n = 1535 || (n > 1536 && n < 1541) || (n > 1541 && n < 1545) || n = 1546 || n = 1547 || (n > 1548 && n < 1554) || (n > 1555 && n < 1562))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1420)
	; 7 bytes
	php
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
	plp
.elseif(n = 1427)
	; 7 bytes
	php
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	plp
.elseif(n = 1495)
	; 8 bytes
	php
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1503)
	; 8 bytes
	php
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1504)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	inx
	Jne *-2
	plp
.elseif(n = 1511)
	; 8 bytes
	php
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1536)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1545)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1554)
	; 8 bytes
	php
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if((n > 1562 && n < 1567) || n = 1568 || (n > 1569 && n < 1575) || (n > 1576 && n < 1583) || (n > 1583 && n < 1590) || (n > 1591 && n < 1597) || (n > 1597 && n < 1604) || (n > 1604 && n < 1611) || (n > 1611 && n < 1617) || (n > 1618 && n < 1625) || (n > 1626 && n < 1630) || n = 1631 || n = 1633 || n = 1634 || (n > 1635 && n < 1639) || (n > 1639 && n < 1644) || n = 1645 || (n > 1646 && n < 1650) || n = 1651 || n = 1652 || (n > 1653 && n < 1660) || n = 1661 || (n > 1662 && n < 1667) || (n > 1667 && n < 1674) || (n > 1674 && n < 1680) || (n > 1681 && n < 1688) || (n > 1688 && n < 1695) || (n > 1695 && n < 1702) || (n > 1702 && n < 1709) || (n > 1709 && n < 1716) || n = 1717 || n = 1718)
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 1567)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1575)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1617)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 1626)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1630)
	; 7 bytes
	php
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	plp
.elseif(n = 1635)
	; 8 bytes
	php
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1644)
	; 7 bytes
	php
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1650)
	; 8 bytes
	php
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	plp
.elseif(n = 1662)
	; 8 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1680)
	; 8 bytes
	php
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
	plp
.endif
.else
; 1719 to 1875
.if((n > 1718 && n < 1723) || (n > 1723 && n < 1728) || n = 1729 || (n > 1730 && n < 1737) || (n > 1737 && n < 1742) || n = 1743 || (n > 1744 && n < 1751) || (n > 1751 && n < 1758) || (n > 1759 && n < 1765) || n = 1766 || (n > 1767 && n < 1772) || n = 1773 || n = 1774 || (n > 1775 && n < 1779) || (n > 1779 && n < 1786) || (n > 1786 && n < 1793) || (n > 1793 && n < 1797) || n = 1798 || n = 1799 || (n > 1800 && n < 1806) || (n > 1806 && n < 1823) || (n > 1824 && n < 1831) || n = 1832 || (n > 1833 && n < 1839) || n = 1840 || n = 1841 || (n > 1842 && n < 1847) || (n > 1847 && n < 1876))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1794)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1728)
	; 7 bytes
	php
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
	plp
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1759)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1767)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1775)
	; 8 bytes
	php
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1797)
	; 8 bytes
	php
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	plp
.elseif(n = 1800)
	; 8 bytes
	php
	ldx #0
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1806)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 1823)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1824)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1831)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1833)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	plp
.elseif(n = 1839)
	; 8 bytes
	php
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1842)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1847)
	; 8 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.endif
.endif
.endif
.else
; 1876 to 2500
.if((n > 1875 && n < 1914) || (n > 1914 && n < 1923) || (n > 1923 && n < 1932) || (n > 1932 && n < 1950) || (n > 1950 && n < 2007) || (n > 2007 && n < 2015) || (n > 2015 && n < 2023) || (n > 2023 && n < 2085) || (n > 2085 && n < 2094) || (n > 2094 && n < 2112) || (n > 2112 && n < 2121) || (n > 2121 && n < 2130) || (n > 2130 && n < 2202) || (n > 2202 && n < 2211) || (n > 2211 && n < 2220) || (n > 2220 && n < 2238) || (n > 2238 && n < 2442) || (n > 2442 && n < 2501))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.elseif(n = 1914)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1923)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1932)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1950)
	; 8 bytes
	php
	ldx #216 ;hides 'cld'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2007)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2015)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2023)
	; 8 bytes
	php
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2085)
	; 8 bytes
	php
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 2094)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2112)
	; 8 bytes
	php
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2121)
	; 8 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2130)
	; 8 bytes
	php
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 2202)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 2211)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2220)
	; 8 bytes
	php
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
	plp
.elseif(n = 2238)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2442)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	plp
.endif
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 5001))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 9 bytes
	php
	ldx #>(n-41)
	lda #<(n-41)
	jsr delay_256x_a_30_clocks
	plp
.endif
.endif

.endif
	rest
.endmacro
