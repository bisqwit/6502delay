; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, and S
; Will preserve Y, and P
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a zeropage two-byte variable @ptrtemp, that points
; into a memory address that can be safely read and overwritten
; with random data.
;
; Requires a function called "delay_256a_x_33_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A:X clocks+overhead
;; Time: 256*A+X+33 clocks (including JSR)
;; Clobbers A. Preserves X,Y. Has relocations.
;;;;;;;;;;;;;;;;;;;;;;;;;
;:	; do 256-5 cycles.
;	sbc #1			; 2 cycles - Carry was set from cmp
;	pha
;	 lda #(256-5 - 27-7-2)
;	 jsr :+
;	pla
;delay_256a_x_33_clocks:
;	cmp #1			; +2
;	bcs :-			; +3 (-1)
;	; 0-255 cycles remain, overhead = 4
;	txa 			; +2; 6; +27 = 33
;:	;passthru
;<<Place the function delay_a_27_clocks immediately following here>>
;
; Requires a function called "delay_a_27_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A+27 clocks (including JSR)
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_a_27_clocks:
;        ;          ;    Cycles        Accumulator     Carry flag
;        ;          ; 0  1  2  3  4       (hex)        0 1 2 3 4
;        sec        ; 0  0  0  0  0   00 01 02 03 04   1 1 1 1 1
;:       sbc #5     ; 2  2  2  2  2   FB FC FD FE FF   0 0 0 0 0
;        bcs :-     ; 4  4  4  4  4   FB FC FD FE FF   0 0 0 0 0
;        lsr a      ; 6  6  6  6  6   7D 7E 7E 7F 7F   1 0 1 0 1
;        bcc :+     ; 8  8  8  8  8   7D 7E 7E 7F 7F   1 0 1 0 1
;:       sbc #$7E   ;10 11 10 11 10   FF FF 00 00 01   0 0 1 1 1
;        bcc :+     ;12 13 12 13 12   FF FF 00 00 01   0 0 1 1 1
;        beq :+     ;      14 15 14         00 00 01       1 1 1
;        bne :+     ;            16               01           1
;:       rts        ;15 16 17 18 19
;
; Generated code is:
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 15
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 22
; - 7 bytes or less when n <= 28
; - 8 bytes or less when n <= 31
; - 9 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=4.5, max=6
; - When 21 <= n <= 30: min=6, avg=6.9, max=8
; - When 31 <= n <= 50: min=6, avg=7.0, max=9
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.0, max=9
; - When 501 <= n <= 750: min=7, avg=8.0, max=9
; - When 751 <= n <= 1000: min=7, avg=8.1, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.6, max=9
; - When 2001 <= n <= 3000: min=7, avg=9.0, max=9
; - When 3001 <= n <= 5000: min=7, avg=9.0, max=9
; - When 5001 <= n <= 7500: min=7, avg=9.0, max=9
; - When 7501 <= n <= 10000: min=7, avg=9.0, max=9
; - When 10001 <= n <= 20000: min=7, avg=9.0, max=9
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
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 26)
	; 2..7 bytes
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
.elseif((n > 8 && n < 12) || n = 13 || n = 19 || n = 22 || n = 25)
	; 3..7 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15 || n = 18 || n = 24)
	; 4..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16 || n = 17 || n = 20)
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
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
.elseif(n = 34)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 35)
	; 8 bytes
	php
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif((n > 35 && n < 46) || (n > 46 && n < 131) || (n > 131 && n < 157))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 46)
	; 6 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	plp
.elseif(n = 131)
	; 6 bytes
	php
	ldx #232 ;hides 'inx'
	Jne *-1
	plp
.endif
.else
; 157 to 312
.if((n > 156 && n < 292))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 292 || n = 301 || n = 305 || n = 310)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 293 || n = 298 || n = 303)
	; 7 bytes
	php
	ldx #(n-13)/5
	dex
	Jpl *-1
	plp
.elseif(n = 294 || n = 296 || n = 302)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 295)
	; 8 bytes
	php
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	plp
.elseif(n = 297)
	; 9 bytes
	php
	pha
	lda #254
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 299)
	; 7 bytes
	php
	lax #1
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 300)
	; 7 bytes
	php
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
	plp
.elseif(n = 304 || n = 312)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 309)
	; 8 bytes
	php
	ldx #(n-15)/7
	nop
	dex
	Jpl *-2
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
.elseif(n = 314 || n = 317 || n = 322 || n = 324 || n = 327 || n = 329 || n = 332 || n = 334 || n = 342 || n = 347 || n = 349 || n = 352 || n = 354 || n = 357 || n = 359 || n = 362 || n = 364 || n = 367 || n = 369 || n = 374 || n = 377 || n = 382 || n = 394 || n = 397 || n = 399 || n = 404 || n = 409 || n = 417 || n = 424 || n = 427 || n = 429 || n = 432 || n = 434 || n = 437 || n = 439 || n = 444 || n = 447 || n = 452 || n = 454 || n = 457 || n = 459 || n = 462 || n = 464 || n = 467)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
	ldx #58 ;hides 'nop'
	inx
	inx
	Jpl *-3
	plp
.elseif(n = 326 || n = 331 || n = 336 || n = 341 || n = 346 || n = 356 || n = 361 || n = 371 || n = 376 || n = 381 || n = 387 || n = 389 || n = 391 || n = 396 || n = 401 || n = 406 || n = 416 || n = 422 || n = 426 || n = 431 || n = 436 || n = 441 || n = 446 || n = 451 || n = 461 || n = 466)
	; 7..8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 390)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	plp
.elseif(n = 392)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 412)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 419)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	dex
	Jpl *-2
	plp
.endif
.else
; 469 to 625
.if(n = 469 || n = 472 || n = 482 || n = 489 || n = 494 || n = 497 || n = 502 || n = 504 || n = 507 || n = 509 || n = 514 || n = 517 || n = 522 || n = 529 || n = 532 || n = 539 || n = 542 || n = 544 || n = 549 || n = 552 || n = 557 || n = 562 || n = 569 || n = 572 || n = 574 || n = 577 || n = 579 || n = 584 || n = 587 || n = 592 || n = 594 || n = 597 || n = 599 || n = 602 || n = 604 || n = 607 || n = 612 || n = 619 || n = 622)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 476 || n = 481 || n = 486 || n = 499 || n = 506 || n = 511 || n = 516 || n = 521 || n = 527 || n = 534 || n = 536 || n = 541 || n = 546 || n = 559 || n = 566 || n = 571 || n = 576 || n = 581 || n = 586 || n = 591 || n = 601 || n = 606 || n = 611 || n = 616 || n = 621)
	; 8 bytes
	pha
	delay_n (n-3)
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
	ldx #58 ;hides 'nop'
	inx
	Jpl *-2
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
.elseif(n = 551)
	; 8 bytes
	php
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 556)
	; 7 bytes
	php
	lax #2
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 564)
	; 8 bytes
	php
	ldx #250 ;hides 'nop'
	dex
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
.elseif(n = 609)
	; 8 bytes
	php
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 614)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
	plp
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626 || n = 632 || n = 639 || n = 641 || n = 646 || n = 651 || n = 656 || n = 661 || n = 671 || n = 676 || n = 686 || n = 691 || n = 696 || n = 706 || n = 711 || n = 716 || n = 721 || n = 726 || n = 731 || n = 737 || n = 741 || n = 746 || n = 756 || n = 761 || n = 766 || n = 776 || n = 781)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 5 = 0) && ((n > 629 && n < 641) || (n > 649 && n < 676) || n = 685 || n = 690 || (n > 699 && n < 711) || (n > 724 && n < 746) || (n > 759 && n < 781)))
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
.elseif(n = 634 || n = 637 || n = 642 || n = 644 || n = 647 || n = 649 || n = 657 || n = 662 || n = 664 || n = 667 || n = 669 || n = 674 || n = 677 || n = 679 || n = 682 || n = 684 || n = 689 || n = 692 || n = 697 || n = 702 || n = 704 || n = 707 || n = 709 || n = 712 || n = 714 || n = 717 || n = 719 || n = 724 || n = 732 || n = 739 || n = 742 || n = 744 || n = 747 || n = 749 || n = 752 || n = 754 || n = 759 || n = 767 || n = 769 || n = 772 || n = 774 || n = 777 || n = 779)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 636)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
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
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jpl *-3
	plp
.elseif(n = 720)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
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
.if(n = 782 || n = 784 || n = 787 || n = 794 || n = 797 || n = 802 || n = 804 || n = 809 || n = 812 || n = 814 || n = 817 || n = 819 || n = 822 || n = 824 || n = 829 || n = 837 || n = 839 || n = 842 || n = 844 || n = 847 || n = 849 || n = 852 || n = 854 || n = 857 || n = 859 || n = 864 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 887 || n = 889 || n = 892 || n = 894 || n = 899 || n = 902 || n = 907 || n = 909 || n = 912 || n = 914 || n = 917 || n = 919 || n = 922 || n = 927 || n = 929 || n = 934 || n = 937)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 5 = 3) && ((n > 782 && n < 809) || (n > 817 && n < 934)))
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
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || (n > 810 && n < 827) || n = 836 || (n > 845 && n < 872) || (n > 880 && n < 902) || (n > 915 && n < 937)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 789)
	; 8 bytes
	php
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif((n .mod 5 = 0) && ((n > 789 && n < 816) || (n > 824 && n < 851) || (n > 864 && n < 886) || (n > 894 && n < 921) || n = 930 || n = 935))
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
.elseif(n = 813)
	; 7 bytes
	php
	lax #3
	jsr delay_256a_x_33_clocks
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
.elseif(n = 860)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 867)
	; 7 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
	plp
.elseif(n = 906)
	; 8 bytes
	php
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
	plp
.elseif(n = 924)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
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
.elseif((n .mod 7 = 1) && (n = 939 || n = 946 || (n > 959 && n < 982) || (n > 994 && n < 1017) || (n > 1029 && n < 1052) || n = 1065 || n = 1079 || n = 1086 || (n > 1099 && n < 1122) || (n > 1134 && n < 1157) || (n > 1169 && n < 1192) || (n > 1204 && n < 1227) || n = 1240 || n = 1247))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 940 || n = 945 || n = 950 || n = 955 || n = 965 || n = 970 || n = 975 || n = 980 || n = 985 || n = 990 || n = 1000 || n = 1005 || n = 1010 || n = 1015 || n = 1020 || n = 1035 || n = 1040 || n = 1045 || n = 1050 || n = 1055 || n = 1060 || n = 1072 || n = 1075 || n = 1080 || n = 1085 || n = 1090 || n = 1095 || n = 1105 || n = 1110 || n = 1115 || n = 1120 || n = 1125 || n = 1130 || n = 1140 || n = 1145 || n = 1150 || n = 1155 || n = 1160 || n = 1165 || n = 1175 || n = 1180 || n = 1185 || n = 1190 || n = 1195 || n = 1200 || n = 1210 || n = 1215 || n = 1220 || n = 1225 || n = 1230 || n = 1235 || n = 1245 || n = 1250)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 941 || n = 947 || n = 951 || n = 956 || n = 961 || n = 966 || n = 971 || n = 976 || n = 986 || n = 996 || n = 1001 || n = 1006 || n = 1011 || n = 1024 || n = 1026 || n = 1031 || n = 1036 || n = 1046 || n = 1061 || n = 1066 || n = 1071 || n = 1076 || n = 1081 || n = 1091 || n = 1096 || n = 1101 || n = 1106 || n = 1111 || n = 1116 || n = 1126 || n = 1136 || n = 1141 || n = 1146 || n = 1151 || n = 1161 || n = 1166 || n = 1171 || n = 1176 || n = 1181 || n = 1186 || n = 1196 || n = 1201 || n = 1206 || n = 1211 || n = 1216 || n = 1221 || n = 1231 || n = 1236 || n = 1241 || n = 1246)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 942)
	; 8 bytes
	php
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
	plp
.elseif(n = 944)
	; 7 bytes
	php
	ldx #122 ;hides 'nop'
	inx
	Jne *-2
	plp
.elseif(n = 949 || n = 952 || n = 954 || n = 957 || n = 959 || n = 962 || n = 964 || n = 977 || n = 979 || n = 982 || n = 984 || n = 989 || n = 992 || n = 994 || n = 997 || n = 1004 || n = 1007 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1032 || n = 1034 || n = 1039 || n = 1042 || n = 1047 || n = 1049 || n = 1052 || n = 1054 || n = 1057 || n = 1062 || n = 1064 || n = 1067 || n = 1069 || n = 1074 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1092 || n = 1094 || n = 1097 || n = 1099 || n = 1102 || n = 1109 || n = 1112 || n = 1117 || n = 1119 || n = 1124 || n = 1127 || n = 1129 || n = 1132 || n = 1134 || n = 1137 || n = 1139 || n = 1144 || n = 1147 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1162 || n = 1164 || n = 1167 || n = 1169 || n = 1172 || n = 1174 || n = 1179 || n = 1182 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1197 || n = 1199 || n = 1202 || n = 1204 || n = 1207 || n = 1209 || n = 1214 || n = 1217 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1232 || n = 1234 || n = 1237 || n = 1242 || n = 1244 || n = 1249)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 987)
	; 8 bytes
	php
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
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
.elseif(n = 1025)
	; 8 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	plp
.elseif(n = 1027)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	plp
.elseif(n = 1029)
	; 8 bytes
	php
	ldx #202 ;hides 'dex'
	Jne *-1
	slo (@ptrtemp,x)
	plp
.elseif(n = 1041)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
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
	lax #4
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1077)
	; 8 bytes
	php
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	dex
	Jmi *-3
	plp
.elseif(n = 1104)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 1122)
	; 8 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	dex
	Jne *-3
	plp
.elseif(n = 1131)
	; 8 bytes
	php
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
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
.if(n = 1251 || n = 1256 || n = 1271 || n = 1276 || n = 1281 || n = 1286 || n = 1291 || n = 1297 || n = 1330 || n = 1395)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1252 || n = 1259 || n = 1262 || n = 1264 || n = 1267 || n = 1269 || n = 1272 || n = 1274 || n = 1277 || n = 1279 || n = 1284 || n = 1287 || n = 1292 || n = 1293 || n = 1295 || (n > 1297 && n < 1303) || (n > 1303 && n < 1310) || (n > 1310 && n < 1317) || n = 1318 || (n > 1319 && n < 1324) || n = 1325 || n = 1326 || n = 1328 || (n > 1331 && n < 1335) || n = 1336 || n = 1337 || (n > 1338 && n < 1345) || (n > 1345 && n < 1352) || (n > 1352 && n < 1356) || n = 1357 || n = 1358 || (n > 1359 && n < 1366) || (n > 1366 && n < 1373) || (n > 1374 && n < 1380) || n = 1381 || n = 1382 || (n > 1383 && n < 1387) || (n > 1387 && n < 1392) || n = 1393 || (n > 1395 && n < 1401) || (n > 1401 && n < 1406))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 1255 || n = 1260 || n = 1265 || n = 1270 || n = 1285 || n = 1290 || n = 1329)
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
.elseif(n = 1319)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1327)
	; 7 bytes
	php
	lax #5
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1335)
	; 8 bytes
	php
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
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
.elseif(n = 1392)
	; 7 bytes
	php
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
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
.if(n = 1407 || (n > 1409 && n < 1415) || (n > 1415 && n < 1422) || (n > 1422 && n < 1427) || n = 1428 || (n > 1430 && n < 1436) || (n > 1436 && n < 1443) || (n > 1443 && n < 1450) || (n > 1450 && n < 1457) || (n > 1457 && n < 1464) || (n > 1464 && n < 1471) || (n > 1471 && n < 1478) || (n > 1478 && n < 1485) || (n > 1485 && n < 1492) || n = 1493 || n = 1494 || (n > 1495 && n < 1499) || n = 1501 || n = 1502 || n = 1505 || (n > 1507 && n < 1511) || n = 1512 || (n > 1513 && n < 1518) || (n > 1521 && n < 1527) || (n > 1527 && n < 1532) || n = 1533 || (n > 1536 && n < 1541) || (n > 1541 && n < 1545) || n = 1546 || n = 1547 || (n > 1548 && n < 1554) || (n > 1555 && n < 1562))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 8 bytes
	php
	ldx #(n-8)/7
	nop
	dex
	Jne *-2
	plp
.elseif(n = 1409 || n = 1430 || n = 1507 || n = 1521 || n = 1535)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1500)
	; 8 bytes
	php
	ldx #90 ;hides 'nop'
	nop
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
.elseif(n = 1519)
	; 8 bytes
	php
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1532)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
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
.if(n = 1563)
	; 8 bytes
	php
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif((n > 1563 && n < 1567) || n = 1568 || (n > 1569 && n < 1575) || (n > 1576 && n < 1583) || n = 1585 || n = 1588 || n = 1589 || (n > 1591 && n < 1597) || (n > 1597 && n < 1604) || (n > 1604 && n < 1611) || (n > 1611 && n < 1616) || (n > 1619 && n < 1625) || (n > 1626 && n < 1630) || n = 1631 || n = 1634 || (n > 1635 && n < 1639) || (n > 1639 && n < 1644) || n = 1645 || (n > 1647 && n < 1653) || (n > 1653 && n < 1660) || n = 1661 || (n > 1662 && n < 1667) || (n > 1667 && n < 1674) || (n > 1674 && n < 1681) || (n > 1681 && n < 1688) || (n > 1688 && n < 1695) || (n > 1695 && n < 1702) || (n > 1702 && n < 1709) || (n > 1709 && n < 1716) || n = 1717 || n = 1718)
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 1584)
	; 7 bytes
	php
	lax #6
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1586)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1587 || n = 1619 || n = 1633 || n = 1647)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1591)
	; 8 bytes
	php
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1616)
	; 7 bytes
	php
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
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
	ldx #(n-6)/7 ;hides 'nop'
	dex
	Jne *-2
	plp
.elseif(n = 1662)
	; 8 bytes
	php
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
	plp
.endif
.else
; 1719 to 1875
.if((n > 1718 && n < 1723) || (n > 1723 && n < 1728) || n = 1729 || (n > 1731 && n < 1737) || (n > 1737 && n < 1742) || n = 1743 || (n > 1745 && n < 1751) || (n > 1751 && n < 1756) || n = 1757 || (n > 1759 && n < 1765) || n = 1766 || (n > 1767 && n < 1772) || n = 1773 || n = 1774 || (n > 1775 && n < 1779) || (n > 1779 && n < 1786) || n = 1787 || (n > 1788 && n < 1793) || (n > 1793 && n < 1800) || (n > 1800 && n < 1806) || (n > 1806 && n < 1823) || (n > 1824 && n < 1831) || n = 1832 || (n > 1833 && n < 1839) || n = 1840 || n = 1845 || n = 1846 || (n > 1847 && n < 1851) || (n > 1851 && n < 1876))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 1731 || n = 1745 || n = 1844)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1756)
	; 7 bytes
	php
	ldx #(n-6)/7 ;hides 'nop'
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
.elseif(n = 1788)
	; 8 bytes
	php
	ldx #58 ;hides 'nop'
	nop
	inx
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
.elseif(n = 1841)
	; 7 bytes
	php
	lax #7
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1842)
	; 8 bytes
	php
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.elseif(n = 1843)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1847)
	; 8 bytes
	php
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
	plp
.elseif(n = 1851)
	; 8 bytes
	php
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
; 1876 to 2187
.if((n > 1875 && n < 1905) || (n > 1905 && n < 1914) || (n > 1914 && n < 1923) || (n > 1923 && n < 1932) || (n > 1932 && n < 1950) || (n > 1950 && n < 1968) || (n > 1968 && n < 2007) || (n > 2007 && n < 2015) || (n > 2015 && n < 2023) || (n > 2023 && n < 2076) || (n > 2076 && n < 2085) || (n > 2085 && n < 2094) || (n > 2094 && n < 2098) || n = 2099 || (n > 2101 && n < 2112) || (n > 2112 && n < 2121) || (n > 2121 && n < 2130) || (n > 2130 && n < 2139) || (n > 2139 && n < 2188))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 1905)
	; 8 bytes
	php
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
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
.elseif(n = 1968 || n = 2112)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
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
.elseif(n = 2076)
	; 8 bytes
	php
	ldx #26 ;hides 'nop'
	nop
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
.elseif(n = 2098)
	; 7 bytes
	php
	lax #8
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2100)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2101)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2139)
	; 8 bytes
	php
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
	plp
.endif
.else
; 2188 to 2500
.if((n > 2187 && n < 2193) || (n > 2193 && n < 2202) || (n > 2202 && n < 2211) || (n > 2211 && n < 2220) || (n > 2220 && n < 2238) || (n > 2238 && n < 2256) || (n > 2256 && n < 2355) || n = 2356 || (n > 2358 && n < 2442) || (n > 2442 && n < 2501))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2193)
	; 8 bytes
	php
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
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
.elseif(n = 2256)
	; 8 bytes
	php
	ldx #(n-6)/9 ;hides 'nop'
	nop
	dex
	Jne *-3
	plp
.elseif(n = 2355)
	; 7 bytes
	php
	lax #9
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2357)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2358)
	; 8 bytes
	pha
	delay_n (n-3)
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
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2612) || n = 2613 || (n > 2615 && n < 2869) || n = 2870 || (n > 2872 && n < 3126) || n = 3127 || (n > 3129 && n < 3383) || n = 3384 || (n > 3386 && n < 3640) || n = 3641 || (n > 3643 && n < 3897) || n = 3898 || (n > 3900 && n < 4154) || n = 4155 || (n > 4157 && n < 4411) || n = 4412 || (n > 4414 && n < 4668) || n = 4669 || (n > 4671 && n < 4925) || n = 4926 || (n > 4928 && n < 5001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2612)
	; 7 bytes
	php
	lax #10
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 2614 || n = 2871 || n = 3128 || n = 3385 || n = 3642 || n = 3899 || n = 4156 || n = 4413 || n = 4670 || n = 4927)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 2615 || n = 2872 || n = 3129 || n = 3386 || n = 3643 || n = 3900 || n = 4157 || n = 4414 || n = 4671 || n = 4928)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 2869)
	; 7 bytes
	php
	lax #11
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3126)
	; 7 bytes
	php
	lax #12
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3383)
	; 7 bytes
	php
	lax #13
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3640)
	; 7 bytes
	php
	lax #14
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 3897)
	; 7 bytes
	php
	lax #15
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4154)
	; 7 bytes
	php
	lax #16
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4411)
	; 7 bytes
	php
	lax #17
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4668)
	; 7 bytes
	php
	lax #18
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 4925)
	; 7 bytes
	php
	lax #19
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5182) || n = 5183 || (n > 5185 && n < 5439) || n = 5440 || (n > 5442 && n < 5696) || n = 5697 || (n > 5699 && n < 5953) || n = 5954 || (n > 5956 && n < 6210) || n = 6211 || (n > 6213 && n < 6467) || n = 6468 || (n > 6470 && n < 6724) || n = 6725 || (n > 6727 && n < 6981) || n = 6982 || (n > 6984 && n < 7238) || n = 7239 || (n > 7241 && n < 7495) || n = 7496 || (n > 7498 && n < 7752) || n = 7753 || (n > 7755 && n < 8009) || n = 8010 || (n > 8012 && n < 8266) || n = 8267 || (n > 8269 && n < 8523) || n = 8524 || (n > 8526 && n < 8780) || n = 8781 || (n > 8783 && n < 9037) || n = 9038 || (n > 9040 && n < 9294) || n = 9295 || (n > 9297 && n < 9551) || n = 9552 || (n > 9554 && n < 9808) || n = 9809 || (n > 9811 && n < 10001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5182)
	; 7 bytes
	php
	lax #20
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5184 || n = 5441 || n = 5698 || n = 5955 || n = 6212 || n = 6469 || n = 6726 || n = 6983 || n = 7240 || n = 7497 || n = 7754 || n = 8011 || n = 8268 || n = 8525 || n = 8782 || n = 9039 || n = 9296 || n = 9553 || n = 9810)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5185 || n = 5442 || n = 5699 || n = 5956 || n = 6213 || n = 6470 || n = 6727 || n = 6984 || n = 7241 || n = 7498 || n = 7755 || n = 8012 || n = 8269 || n = 8526 || n = 8783 || n = 9040 || n = 9297 || n = 9554 || n = 9811)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5439)
	; 7 bytes
	php
	lax #21
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5696)
	; 7 bytes
	php
	lax #22
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 5953)
	; 7 bytes
	php
	lax #23
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6210)
	; 7 bytes
	php
	lax #24
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6467)
	; 7 bytes
	php
	lax #25
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6724)
	; 7 bytes
	php
	lax #26
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 6981)
	; 7 bytes
	php
	lax #27
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7238)
	; 7 bytes
	php
	lax #28
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7495)
	; 7 bytes
	php
	lax #29
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 7752)
	; 7 bytes
	php
	lax #30
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8009)
	; 7 bytes
	php
	lax #31
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8266)
	; 7 bytes
	php
	lax #32
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8523)
	; 7 bytes
	php
	lax #33
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 8780)
	; 7 bytes
	php
	lax #34
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9037)
	; 7 bytes
	php
	lax #35
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9294)
	; 7 bytes
	php
	lax #36
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9551)
	; 7 bytes
	php
	lax #37
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 9808)
	; 7 bytes
	php
	lax #38
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10065) || n = 10066 || (n > 10068 && n < 10322) || n = 10323 || (n > 10325 && n < 10579) || n = 10580 || (n > 10582 && n < 10836) || n = 10837 || (n > 10839 && n < 11093) || n = 11094 || (n > 11096 && n < 11350) || n = 11351 || (n > 11353 && n < 11607) || n = 11608 || (n > 11610 && n < 11864) || n = 11865 || (n > 11867 && n < 12121) || n = 12122 || (n > 12124 && n < 12378) || n = 12379 || (n > 12381 && n < 12635) || n = 12636 || (n > 12638 && n < 12892) || n = 12893 || (n > 12895 && n < 13149) || n = 13150 || (n > 13152 && n < 13406) || n = 13407 || (n > 13409 && n < 13663) || n = 13664 || (n > 13666 && n < 13920) || n = 13921 || (n > 13923 && n < 14177) || n = 14178 || (n > 14180 && n < 14434) || n = 14435 || (n > 14437 && n < 14691) || n = 14692 || (n > 14694 && n < 14948) || n = 14949 || (n > 14951 && n < 15001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10065)
	; 7 bytes
	php
	lax #39
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10067 || n = 10324 || n = 10581 || n = 10838 || n = 11095 || n = 11352 || n = 11609 || n = 11866 || n = 12123 || n = 12380 || n = 12637 || n = 12894 || n = 13151 || n = 13408 || n = 13665 || n = 13922 || n = 14179 || n = 14436 || n = 14693 || n = 14950)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10068 || n = 10325 || n = 10582 || n = 10839 || n = 11096 || n = 11353 || n = 11610 || n = 11867 || n = 12124 || n = 12381 || n = 12638 || n = 12895 || n = 13152 || n = 13409 || n = 13666 || n = 13923 || n = 14180 || n = 14437 || n = 14694 || n = 14951)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10322)
	; 7 bytes
	php
	lax #40
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10579)
	; 7 bytes
	php
	lax #41
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 10836)
	; 7 bytes
	php
	lax #42
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11093)
	; 7 bytes
	php
	lax #43
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11350)
	; 7 bytes
	php
	lax #44
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11607)
	; 7 bytes
	php
	lax #45
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 11864)
	; 7 bytes
	php
	lax #46
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12121)
	; 7 bytes
	php
	lax #47
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12378)
	; 7 bytes
	php
	lax #48
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12635)
	; 7 bytes
	php
	lax #49
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 12892)
	; 7 bytes
	php
	lax #50
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13149)
	; 7 bytes
	php
	lax #51
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13406)
	; 7 bytes
	php
	lax #52
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13663)
	; 7 bytes
	php
	lax #53
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 13920)
	; 7 bytes
	php
	lax #54
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14177)
	; 7 bytes
	php
	lax #55
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14434)
	; 7 bytes
	php
	lax #56
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14691)
	; 7 bytes
	php
	lax #57
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 14948)
	; 7 bytes
	php
	lax #58
	jsr delay_256a_x_33_clocks
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15205) || n = 15206 || (n > 15208 && n < 15462) || n = 15463 || (n > 15465 && n < 15719) || n = 15720 || (n > 15722 && n < 15976) || n = 15977 || (n > 15979 && n < 16233) || n = 16234 || (n > 16236 && n < 16490) || n = 16491 || (n > 16493 && n < 16747) || n = 16748 || (n > 16750 && n < 17004) || n = 17005 || (n > 17007 && n < 17261) || n = 17262 || (n > 17264 && n < 17518) || n = 17519 || (n > 17521 && n < 17775) || n = 17776 || (n > 17778 && n < 18032) || n = 18033 || (n > 18035 && n < 18289) || n = 18290 || (n > 18292 && n < 18546) || n = 18547 || (n > 18549 && n < 18803) || n = 18804 || (n > 18806 && n < 19060) || n = 19061 || (n > 19063 && n < 19317) || n = 19318 || (n > 19320 && n < 19574) || n = 19575 || (n > 19577 && n < 19831) || n = 19832 || (n > 19834 && n < 20001))
	; 9 bytes
	php
	lda #>(n-44)
	ldx #<(n-44)
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15205)
	; 7 bytes
	php
	lax #59
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15207 || n = 15464 || n = 15721 || n = 15978 || n = 16235 || n = 16492 || n = 16749 || n = 17006 || n = 17263 || n = 17520 || n = 17777 || n = 18034 || n = 18291 || n = 18548 || n = 18805 || n = 19062 || n = 19319 || n = 19576 || n = 19833)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15208 || n = 15465 || n = 15722 || n = 15979 || n = 16236 || n = 16493 || n = 16750 || n = 17007 || n = 17264 || n = 17521 || n = 17778 || n = 18035 || n = 18292 || n = 18549 || n = 18806 || n = 19063 || n = 19320 || n = 19577 || n = 19834)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15462)
	; 7 bytes
	php
	lax #60
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15719)
	; 7 bytes
	php
	lax #61
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 15976)
	; 7 bytes
	php
	lax #62
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16233)
	; 7 bytes
	php
	lax #63
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16490)
	; 7 bytes
	php
	lax #64
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 16747)
	; 7 bytes
	php
	lax #65
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17004)
	; 7 bytes
	php
	lax #66
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17261)
	; 7 bytes
	php
	lax #67
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17518)
	; 7 bytes
	php
	lax #68
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 17775)
	; 7 bytes
	php
	lax #69
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18032)
	; 7 bytes
	php
	lax #70
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18289)
	; 7 bytes
	php
	lax #71
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18546)
	; 7 bytes
	php
	lax #72
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 18803)
	; 7 bytes
	php
	lax #73
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19060)
	; 7 bytes
	php
	lax #74
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19317)
	; 7 bytes
	php
	lax #75
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19574)
	; 7 bytes
	php
	lax #76
	jsr delay_256a_x_33_clocks
	plp
.elseif(n = 19831)
	; 7 bytes
	php
	lax #77
	jsr delay_256a_x_33_clocks
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
