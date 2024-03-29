; delay_n, delays n cycles with small code
; Will clobber A, X, Y, Z+N, C, D, and V
; Will preserve S, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a disposable zeropage variable @zptemp.
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
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 14
; - 5 bytes or less when n <= 21
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=3, avg=4.5, max=5
; - When 21 <= n <= 30: min=4, avg=5.5, max=7
; - When 31 <= n <= 50: min=4, avg=4.9, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=4, avg=5.0, max=5
; - When 201 <= n <= 300: min=4, avg=5.1, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=4, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.2, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.6, max=7
; - When 2001 <= n <= 3000: min=6, avg=7.0, max=7
; - When 3001 <= n <= 5000: min=7, avg=7.0, max=7
; - When 5001 <= n <= 7500: min=7, avg=7.0, max=7
; - When 7501 <= n <= 10000: min=7, avg=7.0, max=7
; - When 10001 <= n <= 20000: min=7, avg=7.0, max=7
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
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 8 || n = 15)
	; 2..5 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	inc $F6,X
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 9 || n = 11 || n = 19)
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
.elseif(n = 13)
	; 4 bytes
	php
	inc $F6,X
	plp
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 16)
	; 5 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 17 || n = 18)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
	ldy #4
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
.elseif((n > 28 && n < 39) || (n > 39 && n < 49) || (n > 49 && n < 79))
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
.endif
.else
; 79 to 156
.if((n > 78 && n < 124) || (n > 124 && n < 157))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.endif
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
.elseif(n = 285 || n = 298 || n = 303 || n = 308)
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
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 290)
	; 6 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 292 || n = 294 || n = 297 || n = 305 || n = 307 || n = 310)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
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
.elseif(n = 315 || n = 317 || n = 319 || n = 322 || n = 324 || n = 325 || n = 327 || n = 329 || n = 334 || n = 335 || n = 339 || n = 340 || n = 342 || n = 345 || n = 347 || n = 349 || n = 350 || n = 352 || n = 354 || n = 355 || n = 357 || n = 360 || n = 362 || n = 364 || n = 367 || n = 369 || n = 370 || n = 374 || n = 375 || n = 380 || n = 387 || n = 389 || n = 390 || n = 392 || n = 394 || n = 397 || n = 399 || n = 402 || n = 405 || n = 409 || n = 410 || n = 415 || n = 417 || n = 419 || n = 420 || n = 422 || n = 424 || n = 425 || n = 427 || n = 429 || n = 430 || n = 432 || n = 434 || n = 437 || n = 439 || n = 440 || n = 444 || n = 445 || n = 447 || n = 450 || n = 452 || n = 454 || n = 457 || n = 459 || n = 460 || n = 462 || n = 465)
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
.elseif(n = 359)
	; 6 bytes
	ldx #216 ;hides 'cld'
	nop
	inx
	Jne *-3
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
.elseif(n = 384)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 385)
	; 6 bytes
	inc $F6,X
	delay_n (n-6)
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
	ldx #216 ;hides 'cld'
	dex
	dex
	Jmi *-3
.elseif(n = 412)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
.elseif(n = 455)
	; 6 bytes
	ldy #200 ;hides 'iny'
	sta @zptemp
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
; 469 to 625
.if(n = 469 || n = 474 || n = 475 || n = 479 || n = 482 || n = 487 || n = 490 || n = 492 || n = 495 || n = 497 || n = 499 || n = 500 || n = 502 || n = 504 || n = 507 || n = 509 || n = 514 || n = 515 || n = 520 || n = 522 || n = 525 || n = 527 || n = 529 || n = 532 || n = 534 || n = 535 || n = 537 || n = 542 || n = 545 || n = 549 || n = 550 || n = 555 || n = 557 || n = 559 || n = 562 || n = 564 || n = 565 || n = 567 || n = 569 || n = 570 || n = 572 || n = 574 || n = 577 || n = 579 || n = 580 || n = 584 || n = 585 || n = 587 || n = 590 || n = 592 || n = 594 || n = 595 || n = 597 || n = 599 || n = 600 || n = 602 || n = 604 || n = 605 || n = 609 || n = 612 || n = 614 || n = 619 || n = 625)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 470 || n = 498 || n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 471 || n = 501 || n = 521 || n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
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
.elseif((n .mod 5 = 3) && ((n > 472 && n < 494) || (n > 507 && n < 529) || (n > 537 && n < 564) || (n > 572 && n < 599) || (n > 607 && n < 624)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547) || n = 556 || n = 561 || (n > 570 && n < 582) || n = 596 || (n > 605 && n < 617)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 7 = 1) && (n = 477 || n = 484 || (n > 504 && n < 520) || (n > 532 && n < 555) || n = 575 || n = 617))
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
.elseif(n = 552)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 560)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 607)
	; 6 bytes
	ldx #202 ;hides 'dex'
	sta @zptemp
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
.elseif(n = 622)
	; 5 bytes
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
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
.elseif(n = 627 || n = 630 || n = 632 || n = 634 || n = 635 || n = 637 || n = 639 || n = 640 || n = 642 || n = 644 || n = 649 || n = 650 || n = 654 || n = 655 || n = 657 || n = 660 || n = 662 || n = 664 || n = 667 || n = 669 || n = 670 || n = 672 || n = 675 || n = 677 || n = 679 || n = 682 || n = 685 || n = 695 || n = 697 || n = 699 || n = 700 || n = 702 || n = 704 || n = 705 || n = 707 || n = 709 || n = 710 || n = 714 || n = 717 || n = 719 || n = 724 || n = 725 || n = 730 || n = 732 || n = 735 || n = 737 || n = 739 || n = 740 || n = 742 || n = 745 || n = 747 || n = 749 || n = 752 || n = 754 || n = 759 || n = 760 || n = 762 || n = 765 || n = 767 || n = 769 || n = 770 || n = 772 || n = 774 || n = 775 || n = 777 || n = 779 || n = 780)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || (n > 642 && n < 669) || n = 678 || n = 683 || (n > 692 && n < 704) || (n > 712 && n < 739) || (n > 752 && n < 774)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	iny
	Jne *-3
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
.elseif(n = 689)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 690)
	; 6 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 692)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 712)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif((n .mod 5 = 3) && ((n > 782 && n < 809) || (n > 817 && n < 844) || (n > 852 && n < 879) || (n > 887 && n < 914) || (n > 922 && n < 934)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 787 || n = 789 || n = 790 || n = 794 || n = 795 || n = 797 || n = 802 || n = 804 || n = 805 || n = 807 || n = 809 || n = 810 || n = 812 || n = 814 || n = 815 || n = 817 || n = 819 || n = 822 || n = 829 || n = 830 || n = 832 || n = 835 || n = 837 || n = 840 || n = 842 || n = 844 || n = 845 || n = 847 || n = 849 || n = 850 || n = 852 || n = 854 || n = 857 || n = 859 || n = 860 || n = 864 || n = 865 || n = 867 || n = 870 || n = 872 || n = 874 || n = 875 || n = 877 || n = 879 || n = 880 || n = 882 || n = 884 || n = 885 || n = 887 || n = 889 || n = 892 || n = 894 || n = 895 || n = 900 || n = 902 || n = 905 || n = 907 || n = 909 || n = 910 || n = 912 || n = 914 || n = 915 || n = 917 || n = 919 || n = 920 || n = 922 || n = 924 || n = 927 || n = 929 || n = 930 || n = 934 || n = 937)
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
.elseif((n .mod 5 = 1) && ((n > 785 && n < 937)))
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
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.endif
.endif
.else
; 938 to 1250
.if((n .mod 5 = 3) && ((n > 937 && n < 949) || (n > 962 && n < 984) || (n > 992 && n < 1014) || (n > 1027 && n < 1054) || (n > 1067 && n < 1089) || (n > 1097 && n < 1124) || (n > 1132 && n < 1159) || (n > 1167 && n < 1194) || (n > 1202 && n < 1229) || (n > 1237 && n < 1249)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 942 || n = 945 || n = 947 || n = 949 || n = 950 || n = 952 || n = 954 || n = 955 || n = 957 || n = 959 || n = 964 || n = 969 || n = 970 || n = 972 || n = 975 || n = 977 || n = 979 || n = 980 || n = 982 || n = 985 || n = 987 || n = 989 || n = 990 || n = 994 || n = 997 || n = 999 || n = 1000 || n = 1004 || n = 1005 || n = 1007 || n = 1010 || n = 1012 || n = 1015 || n = 1022 || n = 1024 || n = 1025 || n = 1027 || n = 1029 || n = 1032 || n = 1034 || n = 1035 || n = 1039 || n = 1040 || n = 1042 || n = 1045 || n = 1047 || n = 1050 || n = 1054 || n = 1055 || n = 1057 || n = 1059 || n = 1060 || n = 1062 || n = 1064 || n = 1067 || n = 1069 || n = 1074 || n = 1075 || n = 1077 || n = 1080 || n = 1082 || n = 1084 || n = 1085 || n = 1087 || n = 1089 || n = 1090 || n = 1092 || n = 1094 || n = 1097 || n = 1099 || n = 1102 || n = 1104 || n = 1105 || n = 1109 || n = 1110 || n = 1112 || n = 1117 || n = 1119 || n = 1120 || n = 1122 || n = 1124 || n = 1125 || n = 1127 || n = 1129 || n = 1130 || n = 1132 || n = 1134 || n = 1137 || n = 1139 || n = 1140 || n = 1144 || n = 1145 || n = 1147 || n = 1150 || n = 1152 || n = 1154 || n = 1155 || n = 1157 || n = 1159 || n = 1160 || n = 1162 || n = 1164 || n = 1165 || n = 1167 || n = 1169 || n = 1172 || n = 1174 || n = 1179 || n = 1180 || n = 1182 || n = 1185 || n = 1187 || n = 1190 || n = 1192 || n = 1194 || n = 1195 || n = 1197 || n = 1199 || n = 1200 || n = 1202 || n = 1204 || n = 1207 || n = 1209 || n = 1210 || n = 1214 || n = 1215 || n = 1217 || n = 1220 || n = 1222 || n = 1224 || n = 1225 || n = 1227 || n = 1229 || n = 1230 || n = 1234 || n = 1235 || n = 1237 || n = 1239 || n = 1242 || n = 1244 || n = 1245 || n = 1249)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
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
.elseif(n = 1019)
	; 6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1020)
	; 6 bytes
	delay_n (n-6)
	inc @zptemp,x
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
	ldx #152 ;hides 'tya'
	dex
	Jne *-2
.elseif(n = 1070)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif(n = 1095)
	; 6 bytes
	ldy #136 ;hides 'dey'
	sta @zptemp
	Jne *-3
.elseif(n = 1115)
	; 6 bytes
	ldx #248 ;hides 'sed'
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
.elseif(n = 1252 || n = 1255 || n = 1257 || n = 1260 || n = 1262 || n = 1264 || n = 1265 || n = 1267 || n = 1269 || n = 1270 || n = 1272 || n = 1274 || n = 1277 || n = 1279 || n = 1280 || (n > 1283 && n < 1287) || n = 1288 || (n > 1289 && n < 1296) || (n > 1296 && n < 1301) || n = 1302 || (n > 1304 && n < 1310) || n = 1311 || (n > 1312 && n < 1317) || n = 1318 || n = 1319 || (n > 1320 && n < 1324) || (n > 1324 && n < 1328) || n = 1329 || n = 1330 || (n > 1331 && n < 1336) || n = 1337 || (n > 1338 && n < 1345) || (n > 1345 && n < 1349) || n = 1350 || n = 1351 || (n > 1352 && n < 1359) || (n > 1359 && n < 1366) || (n > 1367 && n < 1373) || n = 1374 || n = 1375 || (n > 1376 && n < 1380) || (n > 1380 && n < 1387) || (n > 1387 && n < 1394) || (n > 1394 && n < 1399) || n = 1400 || (n > 1401 && n < 1407))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1278 || n = 1283)
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
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	nop
	dex
	Jne *-3
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
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.else
; 1407 to 1562
.if(n = 1407 || (n > 1408 && n < 1413) || n = 1414 || (n > 1415 && n < 1420) || n = 1421 || (n > 1422 && n < 1429) || (n > 1429 && n < 1436) || (n > 1436 && n < 1443) || (n > 1443 && n < 1450) || (n > 1450 && n < 1457) || (n > 1457 && n < 1464) || (n > 1464 && n < 1471) || (n > 1471 && n < 1478) || (n > 1478 && n < 1485) || n = 1486 || n = 1487 || (n > 1488 && n < 1492) || (n > 1492 && n < 1496) || n = 1498 || (n > 1499 && n < 1504) || n = 1505 || (n > 1506 && n < 1511) || n = 1512 || (n > 1513 && n < 1520) || (n > 1520 && n < 1527) || n = 1528 || (n > 1529 && n < 1534) || (n > 1534 && n < 1538) || n = 1539 || n = 1540 || (n > 1541 && n < 1547) || (n > 1548 && n < 1555) || (n > 1555 && n < 1560) || n = 1561)
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
	; 5 bytes
	ldx #216 ;hides 'cld'
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
.if((n > 1562 && n < 1568) || (n > 1569 && n < 1576) || (n > 1576 && n < 1583) || (n > 1584 && n < 1590) || (n > 1590 && n < 1597) || (n > 1597 && n < 1604) || (n > 1604 && n < 1610) || (n > 1611 && n < 1618) || (n > 1619 && n < 1623) || n = 1624 || n = 1626 || n = 1627 || (n > 1628 && n < 1632) || (n > 1632 && n < 1637) || n = 1638 || (n > 1639 && n < 1643) || n = 1644 || n = 1645 || (n > 1646 && n < 1653) || n = 1654 || (n > 1655 && n < 1660) || (n > 1660 && n < 1667) || (n > 1667 && n < 1673) || (n > 1674 && n < 1681) || (n > 1681 && n < 1688) || (n > 1688 && n < 1695) || (n > 1695 && n < 1702) || (n > 1702 && n < 1709) || (n > 1709 && n < 1716) || n = 1717 || n = 1718)
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
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
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
.if(n = 1719 || n = 1720 || n = 1722 || (n > 1723 && n < 1730) || (n > 1730 && n < 1735) || n = 1736 || (n > 1737 && n < 1744) || (n > 1744 && n < 1751) || (n > 1752 && n < 1758) || n = 1759 || (n > 1760 && n < 1765) || n = 1766 || n = 1767 || (n > 1768 && n < 1772) || (n > 1772 && n < 1779) || (n > 1779 && n < 1786) || (n > 1786 && n < 1790) || n = 1791 || n = 1792 || (n > 1793 && n < 1799) || (n > 1799 && n < 1816) || (n > 1817 && n < 1824) || n = 1825 || (n > 1826 && n < 1832) || n = 1833 || n = 1834 || (n > 1835 && n < 1840) || (n > 1840 && n < 1876))
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
.elseif(n = 1735)
	; 5 bytes
	ldx #248 ;hides 'sed'
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
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1840)
	; 6 bytes
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
.endif
.endif
.endif
.else
; 1876 to 2500
.if((n > 1875 && n < 1907) || (n > 1907 && n < 1916) || (n > 1916 && n < 1925) || (n > 1925 && n < 1943) || (n > 1943 && n < 2000) || (n > 2000 && n < 2008) || (n > 2008 && n < 2016) || (n > 2016 && n < 2078) || (n > 2078 && n < 2087) || (n > 2087 && n < 2105) || (n > 2105 && n < 2114) || (n > 2114 && n < 2123) || (n > 2123 && n < 2195) || (n > 2195 && n < 2204) || (n > 2204 && n < 2213) || (n > 2213 && n < 2231) || (n > 2231 && n < 2435) || (n > 2435 && n < 2501))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 1943)
	; 6 bytes
	ldx #216 ;hides 'cld'
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
.elseif(n = 2231)
	; 6 bytes
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
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
.if((n > 2500 && n < 5001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.endif
.endif

.endif
	rest
.endmacro
