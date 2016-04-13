; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Y, Z+N, C, and V
; Will preserve S, D, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=4, avg=5.4, max=7
; - When 31 <= n <= 50: min=4, avg=4.9, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=4, avg=5.0, max=5
; - When 201 <= n <= 300: min=4, avg=5.1, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=4, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.2, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.6, max=7
; - When 2001 <= n <= 3000: min=5, avg=7.0, max=7
; - When 3001 <= n <= 5000: min=5, avg=7.0, max=7
; - When 5001 <= n <= 7500: min=5, avg=7.0, max=7
; - When 7501 <= n <= 10000: min=5, avg=7.0, max=7
; - When 10001 <= n <= 20000: min=5, avg=7.0, max=7
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
.elseif((n > 3 && n < 7) || (n > 15 && n < 19))
	; 2..5 bytes
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
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || n = 22)
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
	ldy #4
	dey
	Jne *-1
	tay
.elseif(n = 26)
	; 5 bytes
	sec
	delay_n (n-2)
.elseif(n = 27)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 28)
	; 6 bytes
	clc
	clc
	delay_n (n-4)
.elseif((n > 28 && n < 39) || (n > 39 && n < 49) || (n > 49 && n < 124) || (n > 124 && n < 157))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.elseif(n = 49)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jmi *-1
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.endif
.else
; 157 to 312
.if((n > 156 && n < 284))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 284)
	; 4 bytes
	ldy #200 ;hides 'iny'
	Jmi *-1
.elseif(n = 285 || n = 294 || n = 298 || n = 303 || n = 308)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 287)
	; 6 bytes
	delay_n (n-3)
	ldy $A4
.elseif(n = 288)
	; 6 bytes
	tya
	delay_n (n-4)
	tay
.elseif(n = 289)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 290)
	; 6 bytes
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
.elseif(n = 292)
	; 5 bytes
	lax #1
	jsr delay_256a_x_33_clocks
.elseif(n = 293)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
.elseif(n = 295 || n = 302 || n = 309)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 297 || n = 305 || n = 307 || n = 310)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 299)
	; 6 bytes
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
.elseif(n = 300)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jpl *-2
.elseif(n = 301)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 304)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 312)
	; 6 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	dex
	Jpl *-3
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if((n .mod 5 = 3) && (n = 313 || n = 318 || (n > 327 && n < 354) || (n > 362 && n < 379) || n = 388 || (n > 402 && n < 424) || (n > 432 && n < 459) || n = 468))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
.elseif(n = 315 || n = 317 || n = 319 || n = 322 || n = 324 || n = 325 || n = 327 || n = 329 || n = 334 || n = 335 || n = 339 || n = 340 || n = 342 || n = 345 || n = 347 || n = 349 || n = 350 || n = 352 || n = 354 || n = 355 || n = 357 || n = 359 || n = 360 || n = 362 || n = 364 || n = 367 || n = 369 || n = 370 || n = 374 || n = 375 || n = 380 || n = 384 || n = 385 || n = 387 || n = 389 || n = 390 || n = 392 || n = 394 || n = 397 || n = 399 || n = 402 || n = 409 || n = 410 || n = 415 || n = 417 || n = 419 || n = 420 || n = 422 || n = 424 || n = 425 || n = 427 || n = 429 || n = 430 || n = 432 || n = 434 || n = 437 || n = 439 || n = 440 || n = 444 || n = 445 || n = 447 || n = 450 || n = 452 || n = 454 || n = 457 || n = 459 || n = 460 || n = 462 || n = 465)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 320)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
.elseif(n = 321 || n = 326 || n = 336 || n = 356 || n = 406 || n = 416 || n = 421 || n = 431 || n = 436 || n = 446 || n = 466)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 323 || n = 330 || n = 344 || n = 365 || n = 400 || n = 407 || n = 414 || n = 428 || n = 435 || n = 442 || n = 463)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 332)
	; 6 bytes
	ldx #74 ;hides 'lsr a'
	dex
	dex
	Jne *-3
.elseif(n = 337 || n = 358 || n = 372 || n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 377)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 382)
	; 6 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 383)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 395)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 404)
	; 6 bytes
	ldx #90 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 405)
	; 5 bytes
	ldx #58 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 412)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
.elseif(n = 455)
	; 6 bytes
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
.elseif(n = 464)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 467)
	; 6 bytes
	ldx #24 ;hides 'clc'
	inx
	inx
	Jpl *-3
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 475 || n = 479 || n = 482 || n = 487 || n = 490 || n = 492 || n = 495 || n = 497 || n = 499 || n = 500 || n = 502 || n = 504 || n = 507 || n = 509 || n = 514 || n = 515 || n = 520 || n = 522 || n = 525 || n = 527 || n = 529 || n = 532 || n = 534 || n = 535 || n = 537 || n = 542 || n = 545)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 470 || n = 498)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 471 || n = 501 || n = 521)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 472)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	inx
	Jne *-3
.elseif((n .mod 5 = 3) && ((n > 472 && n < 494) || (n > 507 && n < 529) || n = 538 || n = 543))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 477 || n = 484 || n = 505 || n = 512 || n = 519 || n = 533 || n = 540)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 480)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 489)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	iny
	Jne *-2
.elseif(n = 494)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
.elseif(n = 510)
	; 5 bytes
	ldx #200 ;hides 'iny'
	dex
	Jmi *-2
.elseif(n = 517)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	dex
	Jne *-2
.elseif(n = 524)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	dex
	Jpl *-2
.elseif(n = 530)
	; 6 bytes
	ldx #138 ;hides 'txa'
	inx
	inx
	Jne *-3
.elseif(n = 539)
	; 6 bytes
	ldx #136 ;hides 'dey'
	inx
	inx
	Jne *-3
.elseif(n = 544)
	; 6 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jmi *-3
.endif
.else
; 547 to 625
.if(n = 547 || n = 554 || n = 575 || n = 617)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 3) && ((n > 547 && n < 564) || (n > 572 && n < 599) || (n > 607 && n < 624)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 549)
	; 5 bytes
	lax #2
	jsr delay_256a_x_33_clocks
.elseif(n = 550 || n = 555 || n = 559 || n = 562 || n = 564 || n = 565 || n = 567 || n = 569 || n = 570 || n = 572 || n = 574 || n = 577 || n = 579 || n = 580 || n = 584 || n = 585 || n = 587 || n = 590 || n = 592 || n = 594 || n = 595 || n = 597 || n = 599 || n = 600 || n = 604 || n = 605 || n = 609 || n = 612 || n = 614 || n = 619 || n = 622 || n = 625)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 552)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 560)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
.elseif(n = 615)
	; 5 bytes
	ldx #168 ;hides 'tay'
	inx
	Jne *-2
.elseif(n = 620)
	; 6 bytes
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 630 || n = 632 || n = 634 || n = 635 || n = 637 || n = 639 || n = 640 || n = 642 || n = 644 || n = 649 || n = 650 || n = 654 || n = 655 || n = 657 || n = 660 || n = 662 || n = 664 || n = 667 || n = 669 || n = 670 || n = 672 || n = 675 || n = 677 || n = 679 || n = 682 || n = 685 || n = 689 || n = 695 || n = 697 || n = 699 || n = 700 || n = 702 || n = 705 || n = 707 || n = 709 || n = 710 || n = 714 || n = 717 || n = 719 || n = 724 || n = 725 || n = 730 || n = 732 || n = 735 || n = 737 || n = 739 || n = 740 || n = 742 || n = 745 || n = 747 || n = 749 || n = 752 || n = 754 || n = 759 || n = 760 || n = 762 || n = 765 || n = 767 || n = 769 || n = 770 || n = 772 || n = 774 || n = 775 || n = 777 || n = 779 || n = 780)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || (n > 642 && n < 669) || n = 678 || n = 683 || (n > 692 && n < 704) || (n > 717 && n < 739) || (n > 752 && n < 774)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 5 bytes
	ldx #90 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 652 || n = 659 || n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 674)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 684)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jne *-1
.elseif(n = 688)
	; 6 bytes
	tya
	delay_n (n-4)
	tay
.elseif(n = 690)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
.elseif(n = 692)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 704)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 712)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
.elseif(n = 720)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 727)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jpl *-2
.elseif(n = 734)
	; 5 bytes
	ldy #232 ;hides 'inx'
	dey
	Jmi *-2
.elseif(n = 744)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 748)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 755)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	dex
	Jne *-3
.endif
.else
; 782 to 937
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif((n .mod 5 = 3) && ((n > 782 && n < 809) || (n > 817 && n < 844) || (n > 857 && n < 879) || (n > 887 && n < 914) || (n > 922 && n < 934)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 787 || n = 789 || n = 790 || n = 794 || n = 795 || n = 797 || n = 802 || n = 804 || n = 805 || n = 807 || n = 810 || n = 812 || n = 814 || n = 815 || n = 817 || n = 819 || n = 822 || n = 829 || n = 830 || n = 832 || n = 835 || n = 837 || n = 840 || n = 842 || n = 844 || n = 845 || n = 847 || n = 849 || n = 850 || n = 852 || n = 854 || n = 857 || n = 859 || n = 864 || n = 865 || n = 867 || n = 870 || n = 872 || n = 874 || n = 875 || n = 877 || n = 879 || n = 880 || n = 882 || n = 884 || n = 885 || n = 887 || n = 889 || n = 892 || n = 894 || n = 895 || n = 900 || n = 902 || n = 905 || n = 907 || n = 909 || n = 910 || n = 912 || n = 914 || n = 915 || n = 919 || n = 920 || n = 922 || n = 924 || n = 927 || n = 929 || n = 930 || n = 934)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 785 || n = 799 || n = 813 || n = 834 || n = 848 || n = 855 || n = 862 || n = 883 || n = 904)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || (n > 810 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 800)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 806)
	; 5 bytes
	lax #3
	jsr delay_256a_x_33_clocks
.elseif(n = 809)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 824)
	; 6 bytes
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 825)
	; 5 bytes
	ldx #138 ;hides 'txa'
	inx
	Jne *-2
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
.elseif(n = 853)
	; 5 bytes
	ldx #122 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 860)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 917)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.elseif(n = 937)
	; 5 bytes
	ldx #122 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 943 || n = 948 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1013 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1065 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || n = 1058 || n = 1072 || n = 1079 || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 942 || n = 945 || n = 947 || n = 949 || n = 950 || n = 952 || n = 954 || n = 955 || n = 957 || n = 959 || n = 964 || n = 969 || n = 970 || n = 972 || n = 975 || n = 977 || n = 979 || n = 982 || n = 985 || n = 987 || n = 989 || n = 990 || n = 994 || n = 997 || n = 999 || n = 1000 || n = 1004 || n = 1005 || n = 1007 || n = 1010 || n = 1012 || n = 1015 || n = 1019 || n = 1024 || n = 1025 || n = 1027 || n = 1029 || n = 1032 || n = 1035 || n = 1039 || n = 1040 || n = 1042 || n = 1045 || n = 1047 || n = 1050 || n = 1054 || n = 1055 || n = 1057 || n = 1059 || n = 1060 || n = 1062 || n = 1064 || n = 1067 || n = 1069 || n = 1074 || n = 1075 || n = 1077 || n = 1080 || n = 1082 || n = 1084 || n = 1085 || n = 1087 || n = 1089 || n = 1090 || n = 1092)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 958)
	; 5 bytes
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
.elseif(n = 962)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jmi *-3
.elseif(n = 965)
	; 5 bytes
	ldx #138 ;hides 'txa'
	dex
	Jne *-2
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 984)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jpl *-3
.elseif(n = 992)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jpl *-3
.elseif(n = 1014)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jne *-1
.elseif(n = 1017)
	; 6 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1018)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 1020)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1022)
	; 6 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 1034)
	; 6 bytes
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 1049)
	; 5 bytes
	ldx #106 ;hides 'ror a'
	inx
	Jne *-2
.elseif(n = 1052)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1063)
	; 5 bytes
	lax #4
	jsr delay_256a_x_33_clocks
.elseif(n = 1070)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1102 || n = 1104 || n = 1105 || n = 1109 || n = 1110 || n = 1112 || n = 1115 || n = 1117 || n = 1119 || n = 1120 || n = 1122 || n = 1125 || n = 1127 || n = 1129 || n = 1130 || n = 1132 || n = 1134 || n = 1137 || n = 1139 || n = 1140 || n = 1144 || n = 1145 || n = 1147 || n = 1150 || n = 1152 || n = 1154 || n = 1155 || n = 1157 || n = 1159 || n = 1160 || n = 1162 || n = 1164 || n = 1165 || n = 1167 || n = 1169 || n = 1172 || n = 1174 || n = 1179 || n = 1180 || n = 1182 || n = 1185 || n = 1187 || n = 1190 || n = 1192 || n = 1194 || n = 1195 || n = 1197 || n = 1199 || n = 1200 || n = 1202 || n = 1204 || n = 1207 || n = 1209 || n = 1210 || n = 1214 || n = 1215 || n = 1217 || n = 1220 || n = 1222 || n = 1224 || n = 1225 || n = 1227 || n = 1229 || n = 1230 || n = 1234 || n = 1235 || n = 1237 || n = 1239 || n = 1242 || n = 1244 || n = 1245 || n = 1249)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1095)
	; 6 bytes
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1124) || (n > 1132 && n < 1159) || (n > 1167 && n < 1194) || (n > 1202 && n < 1229) || (n > 1237 && n < 1249)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1124)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1175)
	; 5 bytes
	ldx #168 ;hides 'tay'
	dex
	Jne *-2
.elseif(n = 1189)
	; 5 bytes
	ldy #170 ;hides 'tax'
	dey
	Jne *-2
.elseif(n = 1232)
	; 6 bytes
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1250)
	; 6 bytes
	ldx #117 ;hides 'adc zp,x'
	nop
	inx
	Jne *-3
.endif
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
; 1251 to 1406
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1252 || n = 1255 || n = 1257 || n = 1260 || n = 1262 || n = 1264 || n = 1265 || n = 1267 || n = 1269 || n = 1270 || n = 1272 || n = 1274 || n = 1277 || n = 1279 || n = 1280 || (n > 1283 && n < 1287) || n = 1288 || (n > 1289 && n < 1296) || (n > 1296 && n < 1301) || n = 1302 || (n > 1304 && n < 1310) || n = 1311 || (n > 1312 && n < 1317) || n = 1318 || n = 1319 || n = 1321 || n = 1323 || (n > 1324 && n < 1328) || n = 1329 || n = 1330 || (n > 1331 && n < 1336) || n = 1337 || (n > 1338 && n < 1344) || (n > 1345 && n < 1349) || n = 1350 || n = 1351 || (n > 1352 && n < 1359) || (n > 1359 && n < 1366) || (n > 1367 && n < 1373) || n = 1374 || n = 1375 || (n > 1376 && n < 1380) || (n > 1380 && n < 1385) || n = 1386 || (n > 1387 && n < 1394) || (n > 1394 && n < 1399) || n = 1400 || (n > 1401 && n < 1407))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1278 || n = 1283 || n = 1322)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1259)
	; 6 bytes
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1273)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jne *-2
.elseif(n = 1281)
	; 5 bytes
	ldx #0
	dex
	Jne *-1
.elseif(n = 1287)
	; 5 bytes
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
.elseif(n = 1301)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jne *-2
.elseif(n = 1304)
	; 6 bytes
	ldx #164 ;hides 'ldy zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1312)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1320)
	; 5 bytes
	lax #5
	jsr delay_256a_x_33_clocks
.elseif(n = 1328)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1336)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1344)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1349)
	; 6 bytes
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
.elseif(n = 1367)
	; 6 bytes
	ldx #152 ;hides 'tya'
	nop
	dex
	Jne *-3
.elseif(n = 1376)
	; 6 bytes
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
.elseif(n = 1385)
	; 5 bytes
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.else
; 1407 to 1562
.if(n = 1407 || (n > 1408 && n < 1413) || n = 1414 || (n > 1415 && n < 1420) || n = 1421 || (n > 1422 && n < 1429) || (n > 1429 && n < 1436) || (n > 1436 && n < 1443) || (n > 1443 && n < 1450) || (n > 1450 && n < 1457) || (n > 1457 && n < 1464) || (n > 1464 && n < 1471) || (n > 1471 && n < 1478) || (n > 1478 && n < 1485) || n = 1486 || n = 1487 || (n > 1488 && n < 1492) || n = 1494 || n = 1495 || n = 1498 || (n > 1499 && n < 1504) || n = 1505 || (n > 1506 && n < 1511) || (n > 1513 && n < 1520) || (n > 1520 && n < 1525) || n = 1526 || n = 1528 || (n > 1529 && n < 1534) || (n > 1534 && n < 1538) || n = 1539 || n = 1540 || (n > 1541 && n < 1547) || (n > 1548 && n < 1555) || (n > 1556 && n < 1560) || n = 1561)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1413)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1488)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1493)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1496)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	nop
	inx
	Jne *-3
.elseif(n = 1497)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	inx
	Jne *-2
.elseif(n = 1504)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1511)
	; 6 bytes
	ldx #168 ;hides 'tay'
	nop
	dex
	Jne *-3
.elseif(n = 1512)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1529)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1538)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 1547)
	; 6 bytes
	ldx #85 ;hides 'eor zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1556)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1560)
	; 6 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	dex
	Jne *-3
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if((n > 1562 && n < 1568) || (n > 1569 && n < 1576) || n = 1578 || (n > 1579 && n < 1583) || (n > 1584 && n < 1590) || (n > 1590 && n < 1597) || (n > 1597 && n < 1604) || (n > 1604 && n < 1609) || (n > 1611 && n < 1618) || (n > 1619 && n < 1623) || n = 1624 || n = 1626 || n = 1627 || (n > 1628 && n < 1632) || (n > 1632 && n < 1637) || n = 1638 || (n > 1639 && n < 1643) || n = 1644 || n = 1645 || (n > 1646 && n < 1653) || n = 1654 || (n > 1655 && n < 1660) || (n > 1660 && n < 1667) || (n > 1667 && n < 1673) || (n > 1674 && n < 1681) || (n > 1681 && n < 1688) || (n > 1688 && n < 1695) || (n > 1695 && n < 1702) || (n > 1702 && n < 1709) || (n > 1709 && n < 1716) || n = 1717 || n = 1718)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1568)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && ((n > 1568 && n < 1717)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1577)
	; 5 bytes
	lax #6
	jsr delay_256a_x_33_clocks
.elseif(n = 1579)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1609)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
.elseif(n = 1610)
	; 6 bytes
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
.elseif(n = 1619)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1623)
	; 5 bytes
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
.elseif(n = 1628)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1643)
	; 6 bytes
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1673)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
.endif
.else
; 1719 to 1875
.if(n = 1719 || n = 1720 || n = 1722 || (n > 1723 && n < 1730) || (n > 1730 && n < 1737) || (n > 1737 && n < 1744) || (n > 1744 && n < 1749) || n = 1750 || (n > 1752 && n < 1758) || n = 1759 || (n > 1760 && n < 1765) || n = 1766 || n = 1767 || (n > 1768 && n < 1772) || (n > 1772 && n < 1779) || n = 1780 || (n > 1781 && n < 1786) || (n > 1786 && n < 1790) || n = 1791 || n = 1792 || (n > 1793 && n < 1799) || (n > 1799 && n < 1816) || (n > 1817 && n < 1824) || n = 1825 || (n > 1826 && n < 1832) || n = 1833 || (n > 1836 && n < 1840) || (n > 1840 && n < 1844) || (n > 1844 && n < 1876))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1721)
	; 5 bytes
	ldx #10 ;hides 'asl a'
	inx
	Jne *-2
.elseif((n .mod 7 = 1) && ((n > 1722 && n < 1787)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1752)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1760)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	nop
	inx
	Jne *-3
.elseif(n = 1768)
	; 6 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 1790)
	; 6 bytes
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
.elseif(n = 1793)
	; 6 bytes
	ldx #0
	nop
	dex
	Jne *-2
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif(n = 1816)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1817)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1824)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
.elseif(n = 1832)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1834)
	; 5 bytes
	lax #7
	jsr delay_256a_x_33_clocks
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1836)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 1840)
	; 6 bytes
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.endif
.endif
.endif
.else
; 1876 to 2500
.if((n > 1875 && n < 1898) || (n > 1898 && n < 1907) || (n > 1907 && n < 1916) || (n > 1916 && n < 1925) || (n > 1925 && n < 1961) || (n > 1961 && n < 2000) || (n > 2000 && n < 2008) || (n > 2008 && n < 2016) || (n > 2016 && n < 2069) || (n > 2069 && n < 2078) || (n > 2078 && n < 2087) || (n > 2087 && n < 2091) || n = 2092 || (n > 2093 && n < 2105) || (n > 2105 && n < 2114) || (n > 2114 && n < 2123) || (n > 2123 && n < 2132) || (n > 2132 && n < 2186) || (n > 2186 && n < 2195) || (n > 2195 && n < 2204) || (n > 2204 && n < 2213) || (n > 2213 && n < 2249) || (n > 2249 && n < 2348) || n = 2349 || (n > 2350 && n < 2435) || (n > 2435 && n < 2501))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1907)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 1916)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1925)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1961)
	; 6 bytes
	ldx #218 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2000)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 2008)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	nop
	inx
	Jne *-3
.elseif(n = 2016)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2069)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jne *-3
.elseif(n = 2078)
	; 6 bytes
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
.elseif(n = 2087)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jne *-3
.elseif(n = 2091)
	; 5 bytes
	lax #8
	jsr delay_256a_x_33_clocks
.elseif(n = 2093 || n = 2350)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2105)
	; 6 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2114)
	; 6 bytes
	ldx #21 ;hides 'ora zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 2123)
	; 6 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2132)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2186)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 2195)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 2204)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 2213)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
.elseif(n = 2249)
	; 6 bytes
	ldx #250 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2348)
	; 5 bytes
	lax #9
	jsr delay_256a_x_33_clocks
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.endif
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2605) || n = 2606 || (n > 2607 && n < 2862) || n = 2863 || (n > 2864 && n < 3119) || n = 3120 || (n > 3121 && n < 3376) || n = 3377 || (n > 3378 && n < 3633) || n = 3634 || (n > 3635 && n < 3890) || n = 3891 || (n > 3892 && n < 4147) || n = 4148 || (n > 4149 && n < 4404) || n = 4405 || (n > 4406 && n < 4661) || n = 4662 || (n > 4663 && n < 4918) || n = 4919 || (n > 4920 && n < 5001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 2605)
	; 5 bytes
	lax #10
	jsr delay_256a_x_33_clocks
.elseif(n = 2607 || n = 2864 || n = 3121 || n = 3378 || n = 3635 || n = 3892 || n = 4149 || n = 4406 || n = 4663 || n = 4920)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2862)
	; 5 bytes
	lax #11
	jsr delay_256a_x_33_clocks
.elseif(n = 3119)
	; 5 bytes
	lax #12
	jsr delay_256a_x_33_clocks
.elseif(n = 3376)
	; 5 bytes
	lax #13
	jsr delay_256a_x_33_clocks
.elseif(n = 3633)
	; 5 bytes
	lax #14
	jsr delay_256a_x_33_clocks
.elseif(n = 3890)
	; 5 bytes
	lax #15
	jsr delay_256a_x_33_clocks
.elseif(n = 4147)
	; 5 bytes
	lax #16
	jsr delay_256a_x_33_clocks
.elseif(n = 4404)
	; 5 bytes
	lax #17
	jsr delay_256a_x_33_clocks
.elseif(n = 4661)
	; 5 bytes
	lax #18
	jsr delay_256a_x_33_clocks
.elseif(n = 4918)
	; 5 bytes
	lax #19
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5175) || n = 5176 || (n > 5177 && n < 5432) || n = 5433 || (n > 5434 && n < 5689) || n = 5690 || (n > 5691 && n < 5946) || n = 5947 || (n > 5948 && n < 6203) || n = 6204 || (n > 6205 && n < 6460) || n = 6461 || (n > 6462 && n < 6717) || n = 6718 || (n > 6719 && n < 6974) || n = 6975 || (n > 6976 && n < 7231) || n = 7232 || (n > 7233 && n < 7488) || n = 7489 || (n > 7490 && n < 7745) || n = 7746 || (n > 7747 && n < 8002) || n = 8003 || (n > 8004 && n < 8259) || n = 8260 || (n > 8261 && n < 8516) || n = 8517 || (n > 8518 && n < 8773) || n = 8774 || (n > 8775 && n < 9030) || n = 9031 || (n > 9032 && n < 9287) || n = 9288 || (n > 9289 && n < 9544) || n = 9545 || (n > 9546 && n < 9801) || n = 9802 || (n > 9803 && n < 10001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 5175)
	; 5 bytes
	lax #20
	jsr delay_256a_x_33_clocks
.elseif(n = 5177 || n = 5434 || n = 5691 || n = 5948 || n = 6205 || n = 6462 || n = 6719 || n = 6976 || n = 7233 || n = 7490 || n = 7747 || n = 8004 || n = 8261 || n = 8518 || n = 8775 || n = 9032 || n = 9289 || n = 9546 || n = 9803)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 5432)
	; 5 bytes
	lax #21
	jsr delay_256a_x_33_clocks
.elseif(n = 5689)
	; 5 bytes
	lax #22
	jsr delay_256a_x_33_clocks
.elseif(n = 5946)
	; 5 bytes
	lax #23
	jsr delay_256a_x_33_clocks
.elseif(n = 6203)
	; 5 bytes
	lax #24
	jsr delay_256a_x_33_clocks
.elseif(n = 6460)
	; 5 bytes
	lax #25
	jsr delay_256a_x_33_clocks
.elseif(n = 6717)
	; 5 bytes
	lax #26
	jsr delay_256a_x_33_clocks
.elseif(n = 6974)
	; 5 bytes
	lax #27
	jsr delay_256a_x_33_clocks
.elseif(n = 7231)
	; 5 bytes
	lax #28
	jsr delay_256a_x_33_clocks
.elseif(n = 7488)
	; 5 bytes
	lax #29
	jsr delay_256a_x_33_clocks
.elseif(n = 7745)
	; 5 bytes
	lax #30
	jsr delay_256a_x_33_clocks
.elseif(n = 8002)
	; 5 bytes
	lax #31
	jsr delay_256a_x_33_clocks
.elseif(n = 8259)
	; 5 bytes
	lax #32
	jsr delay_256a_x_33_clocks
.elseif(n = 8516)
	; 5 bytes
	lax #33
	jsr delay_256a_x_33_clocks
.elseif(n = 8773)
	; 5 bytes
	lax #34
	jsr delay_256a_x_33_clocks
.elseif(n = 9030)
	; 5 bytes
	lax #35
	jsr delay_256a_x_33_clocks
.elseif(n = 9287)
	; 5 bytes
	lax #36
	jsr delay_256a_x_33_clocks
.elseif(n = 9544)
	; 5 bytes
	lax #37
	jsr delay_256a_x_33_clocks
.elseif(n = 9801)
	; 5 bytes
	lax #38
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10058) || n = 10059 || (n > 10060 && n < 10315) || n = 10316 || (n > 10317 && n < 10572) || n = 10573 || (n > 10574 && n < 10829) || n = 10830 || (n > 10831 && n < 11086) || n = 11087 || (n > 11088 && n < 11343) || n = 11344 || (n > 11345 && n < 11600) || n = 11601 || (n > 11602 && n < 11857) || n = 11858 || (n > 11859 && n < 12114) || n = 12115 || (n > 12116 && n < 12371) || n = 12372 || (n > 12373 && n < 12628) || n = 12629 || (n > 12630 && n < 12885) || n = 12886 || (n > 12887 && n < 13142) || n = 13143 || (n > 13144 && n < 13399) || n = 13400 || (n > 13401 && n < 13656) || n = 13657 || (n > 13658 && n < 13913) || n = 13914 || (n > 13915 && n < 14170) || n = 14171 || (n > 14172 && n < 14427) || n = 14428 || (n > 14429 && n < 14684) || n = 14685 || (n > 14686 && n < 14941) || n = 14942 || (n > 14943 && n < 15001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 10058)
	; 5 bytes
	lax #39
	jsr delay_256a_x_33_clocks
.elseif(n = 10060 || n = 10317 || n = 10574 || n = 10831 || n = 11088 || n = 11345 || n = 11602 || n = 11859 || n = 12116 || n = 12373 || n = 12630 || n = 12887 || n = 13144 || n = 13401 || n = 13658 || n = 13915 || n = 14172 || n = 14429 || n = 14686 || n = 14943)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 10315)
	; 5 bytes
	lax #40
	jsr delay_256a_x_33_clocks
.elseif(n = 10572)
	; 5 bytes
	lax #41
	jsr delay_256a_x_33_clocks
.elseif(n = 10829)
	; 5 bytes
	lax #42
	jsr delay_256a_x_33_clocks
.elseif(n = 11086)
	; 5 bytes
	lax #43
	jsr delay_256a_x_33_clocks
.elseif(n = 11343)
	; 5 bytes
	lax #44
	jsr delay_256a_x_33_clocks
.elseif(n = 11600)
	; 5 bytes
	lax #45
	jsr delay_256a_x_33_clocks
.elseif(n = 11857)
	; 5 bytes
	lax #46
	jsr delay_256a_x_33_clocks
.elseif(n = 12114)
	; 5 bytes
	lax #47
	jsr delay_256a_x_33_clocks
.elseif(n = 12371)
	; 5 bytes
	lax #48
	jsr delay_256a_x_33_clocks
.elseif(n = 12628)
	; 5 bytes
	lax #49
	jsr delay_256a_x_33_clocks
.elseif(n = 12885)
	; 5 bytes
	lax #50
	jsr delay_256a_x_33_clocks
.elseif(n = 13142)
	; 5 bytes
	lax #51
	jsr delay_256a_x_33_clocks
.elseif(n = 13399)
	; 5 bytes
	lax #52
	jsr delay_256a_x_33_clocks
.elseif(n = 13656)
	; 5 bytes
	lax #53
	jsr delay_256a_x_33_clocks
.elseif(n = 13913)
	; 5 bytes
	lax #54
	jsr delay_256a_x_33_clocks
.elseif(n = 14170)
	; 5 bytes
	lax #55
	jsr delay_256a_x_33_clocks
.elseif(n = 14427)
	; 5 bytes
	lax #56
	jsr delay_256a_x_33_clocks
.elseif(n = 14684)
	; 5 bytes
	lax #57
	jsr delay_256a_x_33_clocks
.elseif(n = 14941)
	; 5 bytes
	lax #58
	jsr delay_256a_x_33_clocks
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15198) || n = 15199 || (n > 15200 && n < 15455) || n = 15456 || (n > 15457 && n < 15712) || n = 15713 || (n > 15714 && n < 15969) || n = 15970 || (n > 15971 && n < 16226) || n = 16227 || (n > 16228 && n < 16483) || n = 16484 || (n > 16485 && n < 16740) || n = 16741 || (n > 16742 && n < 16997) || n = 16998 || (n > 16999 && n < 17254) || n = 17255 || (n > 17256 && n < 17511) || n = 17512 || (n > 17513 && n < 17768) || n = 17769 || (n > 17770 && n < 18025) || n = 18026 || (n > 18027 && n < 18282) || n = 18283 || (n > 18284 && n < 18539) || n = 18540 || (n > 18541 && n < 18796) || n = 18797 || (n > 18798 && n < 19053) || n = 19054 || (n > 19055 && n < 19310) || n = 19311 || (n > 19312 && n < 19567) || n = 19568 || (n > 19569 && n < 19824) || n = 19825 || (n > 19826 && n < 20001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 15198)
	; 5 bytes
	lax #59
	jsr delay_256a_x_33_clocks
.elseif(n = 15200 || n = 15457 || n = 15714 || n = 15971 || n = 16228 || n = 16485 || n = 16742 || n = 16999 || n = 17256 || n = 17513 || n = 17770 || n = 18027 || n = 18284 || n = 18541 || n = 18798 || n = 19055 || n = 19312 || n = 19569 || n = 19826)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 15455)
	; 5 bytes
	lax #60
	jsr delay_256a_x_33_clocks
.elseif(n = 15712)
	; 5 bytes
	lax #61
	jsr delay_256a_x_33_clocks
.elseif(n = 15969)
	; 5 bytes
	lax #62
	jsr delay_256a_x_33_clocks
.elseif(n = 16226)
	; 5 bytes
	lax #63
	jsr delay_256a_x_33_clocks
.elseif(n = 16483)
	; 5 bytes
	lax #64
	jsr delay_256a_x_33_clocks
.elseif(n = 16740)
	; 5 bytes
	lax #65
	jsr delay_256a_x_33_clocks
.elseif(n = 16997)
	; 5 bytes
	lax #66
	jsr delay_256a_x_33_clocks
.elseif(n = 17254)
	; 5 bytes
	lax #67
	jsr delay_256a_x_33_clocks
.elseif(n = 17511)
	; 5 bytes
	lax #68
	jsr delay_256a_x_33_clocks
.elseif(n = 17768)
	; 5 bytes
	lax #69
	jsr delay_256a_x_33_clocks
.elseif(n = 18025)
	; 5 bytes
	lax #70
	jsr delay_256a_x_33_clocks
.elseif(n = 18282)
	; 5 bytes
	lax #71
	jsr delay_256a_x_33_clocks
.elseif(n = 18539)
	; 5 bytes
	lax #72
	jsr delay_256a_x_33_clocks
.elseif(n = 18796)
	; 5 bytes
	lax #73
	jsr delay_256a_x_33_clocks
.elseif(n = 19053)
	; 5 bytes
	lax #74
	jsr delay_256a_x_33_clocks
.elseif(n = 19310)
	; 5 bytes
	lax #75
	jsr delay_256a_x_33_clocks
.elseif(n = 19567)
	; 5 bytes
	lax #76
	jsr delay_256a_x_33_clocks
.elseif(n = 19824)
	; 5 bytes
	lax #77
	jsr delay_256a_x_33_clocks
.endif
.endif
.endif

.endif
	rest
.endmacro
