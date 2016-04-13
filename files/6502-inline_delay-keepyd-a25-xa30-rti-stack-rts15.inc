; delay_n, delays n cycles with small code
; Will clobber A, X, S, Z+N, C, and V
; Will preserve Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 13
; - 4 bytes or less when n <= 21
; - 5 bytes or less when n <= 282
; - 6 bytes or less when n <= 286
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=2, avg=3.3, max=4
; - When 21 <= n <= 30: min=4, avg=4.7, max=5
; - When 31 <= n <= 50: min=4, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=4, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.2, max=7
; - When 301 <= n <= 500: min=4, avg=5.9, max=7
; - When 501 <= n <= 750: min=5, avg=5.9, max=7
; - When 751 <= n <= 1000: min=5, avg=5.9, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.4, max=7
; - When 2001 <= n <= 3000: min=6, avg=6.9, max=7
; - When 3001 <= n <= 5000: min=6, avg=7.0, max=7
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
; 1 to 156
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4)
	; 1 bytes
	pla
.elseif(n = 5)
	; 2 bytes
	delay_n (n-2)
	nop
.elseif(n = 6 || n = 11)
	; 2..3 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 2 bytes
	delay_n (n-4)
	pla
.elseif(n = 9 || n = 10 || n = 20 || n = 22)
	; 3..5 bytes
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
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 17 || n = 21 || n = 26)
	; 3..4 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 19)
	; 4 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 23)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	pha
	Jpl *-2
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif((n > 26 && n < 39) || (n > 39 && n < 124) || (n > 124 && n < 157))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.endif
.else
; 157 to 312
.if((n > 156 && n < 283))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 283 || n = 284 || n = 298 || n = 303 || n = 308)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 285 || n = 294)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 287)
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
.elseif(n = 289 || n = 297 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 290 || n = 310)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 292)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 307)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 314 || n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 404 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 315 || n = 320 || n = 335 || n = 340 || n = 350 || n = 355 || n = 360 || n = 375 || n = 380 || n = 383 || n = 390 || n = 402 || n = 405 || n = 410 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450 || n = 455)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 5 = 2) && ((n > 316 && n < 328) || n = 342 || n = 347 || (n > 356 && n < 368) || n = 412 || (n > 421 && n < 438) || n = 447 || n = 452 || n = 462))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 325 || n = 352 || n = 370 || n = 397 || n = 415)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 329 || n = 345 || n = 369 || n = 417 || n = 457 || n = 465)
	; 6 bytes
	ldx #(n-9)/8
	pha
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
.elseif(n = 385 || n = 409 || n = 425)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 387)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 392)
	; 6 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 460)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.if(n = 469 || n = 474 || n = 479 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 534 || n = 539 || n = 549 || n = 559 || n = 564 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 475 || n = 490 || n = 495 || n = 500 || n = 507 || n = 510 || n = 515 || n = 520 || n = 525 || n = 535 || n = 555 || n = 565 || n = 570 || n = 580 || n = 590 || n = 600 || n = 605 || n = 615)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif((n .mod 5 = 2) && (n = 482 || n = 492 || n = 502 || n = 522 || n = 542 || (n > 556 && n < 573) || (n > 586 && n < 603) || n = 612))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 487)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 489 || n = 497 || n = 529 || n = 545 || n = 577 || n = 585 || n = 609 || n = 625)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 532 || n = 550 || n = 595 || n = 622)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 537 || n = 569)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
	; 5 bytes
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
.elseif(n = 620)
	; 6 bytes
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
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
.elseif((n .mod 5 = 2) && ((n > 626 && n < 643) || n = 662 || (n > 671 && n < 683) || n = 692 || n = 702 || n = 707 || n = 717 || n = 732 || n = 742 || n = 747 || (n > 761 && n < 773)))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || (n > 642 && n < 669) || n = 678 || n = 683 || (n > 692 && n < 704) || (n > 712 && n < 739) || (n > 752 && n < 774)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 654 || n = 664 || n = 669 || n = 679 || n = 684 || n = 699 || n = 704 || n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 734 || n = 739 || n = 749 || n = 754 || n = 759 || n = 774 || n = 779)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 630 || n = 635 || n = 650 || n = 655 || n = 660 || n = 670 || n = 675 || n = 690 || n = 695 || n = 700 || n = 710 || n = 725 || n = 735 || n = 740 || n = 752 || n = 760 || n = 765 || n = 770 || n = 780)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 640 || n = 775)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 649 || n = 689 || n = 697 || n = 745)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 652 || n = 659 || n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 705 || n = 737 || n = 769 || n = 777)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667 || n = 685 || n = 712)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 674)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	inx
	Jne *-3
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
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
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || n = 804 || n = 814 || n = 819 || n = 829 || n = 839 || n = 844 || (n > 853 && n < 865) || (n > 873 && n < 885) || n = 894 || (n > 908 && n < 925) || n = 934))
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 5 = 2) && (n = 787 || n = 797 || n = 807 || n = 812 || n = 822 || (n > 831 && n < 843) || n = 852 || (n > 866 && n < 888) || (n > 901 && n < 928)))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 5 = 0) && (n = 790 || n = 795 || (n > 804 && n < 816) || n = 830 || (n > 839 && n < 851) || n = 860 || (n > 869 && n < 886) || n = 895 || n = 900 || n = 915 || n = 920 || n = 930))
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 802 || n = 847)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 809 || n = 817 || n = 849 || n = 857 || n = 865 || n = 889 || n = 929)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 835)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jmi *-3
.elseif(n = 892 || n = 910)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 905 || n = 937)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.if((n .mod 5 = 3) && ((n > 937 && n < 949) || (n > 957 && n < 984) || (n > 992 && n < 1014) || (n > 1027 && n < 1054) || (n > 1067 && n < 1089) || (n > 1097 && n < 1124) || (n > 1132 && n < 1159) || (n > 1167 && n < 1194) || (n > 1202 && n < 1229) || (n > 1237 && n < 1249)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || (n > 1092 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 950 || n = 970 || n = 975 || n = 980 || n = 990 || n = 1005 || n = 1010 || n = 1015 || n = 1018 || n = 1035 || n = 1040 || n = 1050 || n = 1055 || n = 1060 || n = 1067 || n = 1075 || n = 1080 || n = 1085 || n = 1095 || n = 1110 || n = 1115 || n = 1120 || n = 1125 || n = 1130 || n = 1140 || n = 1150 || n = 1155 || n = 1160 || n = 1165 || n = 1175 || n = 1190 || n = 1195 || n = 1200 || n = 1210 || n = 1215 || n = 1220 || n = 1230 || n = 1235 || n = 1245)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && ((n > 941 && n < 958) || n = 972 || n = 987 || n = 997 || n = 1007 || n = 1012 || n = 1032 || n = 1042 || n = 1047 || n = 1062 || (n > 1076 && n < 1093) || n = 1102 || n = 1112 || (n > 1121 && n < 1133) || (n > 1146 && n < 1158) || n = 1167 || n = 1172 || (n > 1181 && n < 1203) || n = 1222 || n = 1227 || n = 1237 || n = 1242))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1124 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 1) && (n = 945 || n = 1025 || n = 1057 || (n > 1088 && n < 1106) || (n > 1128 && n < 1146) || n = 1169 || n = 1185 || (n > 1208 && n < 1226) || n = 1249))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 955 || n = 982 || n = 1000 || n = 1045 || n = 1090 || n = 1162)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.elseif(n = 969 || n = 977 || n = 985)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 1020)
	; 6 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1022)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1027)
	; 6 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1117 || n = 1180 || n = 1207)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
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
.elseif((n .mod 9 = 1) && (n = 1252 || n = 1270 || n = 1288 || n = 1306 || n = 1315 || (n > 1332 && n < 1361) || n = 1378 || n = 1396 || n = 1405))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1255 || n = 1260 || n = 1277 || n = 1280 || n = 1285 || n = 1291 || n = 1403)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 8 = 1) && (n = 1257 || n = 1265 || (n > 1296 && n < 1338) || (n > 1352 && n < 1394)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1259)
	; 6 bytes
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1286 || (n > 1291 && n < 1296) || (n > 1297 && n < 1303) || n = 1304 || (n > 1306 && n < 1310) || n = 1311 || n = 1314 || n = 1316 || n = 1318 || n = 1319 || n = 1322 || n = 1323 || (n > 1324 && n < 1328) || n = 1330 || n = 1332 || (n > 1333 && n < 1337) || (n > 1338 && n < 1342) || n = 1343 || n = 1344 || (n > 1345 && n < 1349) || n = 1350 || (n > 1353 && n < 1359) || (n > 1361 && n < 1366) || n = 1368 || (n > 1369 && n < 1373) || n = 1374 || n = 1375 || (n > 1380 && n < 1385) || n = 1386 || (n > 1387 && n < 1393) || n = 1395 || n = 1397 || n = 1398 || n = 1400 || n = 1404 || n = 1406)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1402)
	; 6 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1379)
	; 6 bytes
	ldx #138 ;hides 'txa'
	pha
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
.if(n = 1407 || (n > 1409 && n < 1414) || n = 1416 || n = 1418 || n = 1419 || n = 1421 || (n > 1425 && n < 1429) || n = 1430 || n = 1431 || n = 1434 || n = 1435 || (n > 1436 && n < 1441) || n = 1442 || (n > 1443 && n < 1449) || (n > 1450 && n < 1457) || n = 1458 || (n > 1459 && n < 1464) || n = 1466 || n = 1467 || n = 1469 || n = 1470 || n = 1472 || (n > 1473 && n < 1477) || n = 1479 || n = 1480 || (n > 1481 && n < 1485) || n = 1487 || n = 1490 || n = 1491 || n = 1493 || n = 1494 || n = 1498 || n = 1502 || n = 1503 || (n > 1506 && n < 1513) || (n > 1513 && n < 1517) || n = 1518 || (n > 1522 && n < 1527) || n = 1528 || n = 1530 || n = 1532 || n = 1533 || n = 1535 || n = 1536 || n = 1539 || (n > 1541 && n < 1545) || n = 1546 || (n > 1549 && n < 1553) || n = 1554 || n = 1556 || n = 1557 || n = 1559)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1450) || (n > 1464 && n < 1490) || n = 1505 || n = 1521 || (n > 1536 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && (n = 1414 || n = 1432 || (n > 1458 && n < 1496) || (n > 1521 && n < 1559)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1423 || n = 1500)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1424 || n = 1501)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1517)
	; 6 bytes
	ldx #138 ;hides 'txa'
	pla
	dex
	Jne *-3
.elseif(n = 1519)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pha
	dex
	Jne *-3
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
.if((n > 1562 && n < 1567) || (n > 1569 && n < 1576) || (n > 1577 && n < 1583) || (n > 1585 && n < 1590) || n = 1591 || n = 1592 || n = 1595 || n = 1596 || (n > 1597 && n < 1601) || n = 1602 || (n > 1604 && n < 1609) || (n > 1612 && n < 1617) || n = 1620 || n = 1622 || n = 1624 || n = 1629 || n = 1631 || (n > 1633 && n < 1637) || n = 1638 || (n > 1641 && n < 1646) || n = 1647 || (n > 1649 && n < 1653) || n = 1654 || n = 1656 || n = 1658 || n = 1659 || (n > 1660 && n < 1665) || (n > 1667 && n < 1671) || n = 1672 || (n > 1675 && n < 1681) || n = 1682 || n = 1683 || (n > 1684 && n < 1688) || (n > 1689 && n < 1693) || n = 1694 || n = 1696 || (n > 1697 && n < 1702) || n = 1703 || n = 1704 || (n > 1705 && n < 1709) || n = 1710 || n = 1712 || n = 1714 || n = 1715 || n = 1717 || n = 1718)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1593 && n < 1631) || n = 1648 || (n > 1665 && n < 1694) || n = 1711))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif((n .mod 8 = 1) && ((n > 1576 && n < 1618) || (n > 1632 && n < 1674) || (n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
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
.elseif(n = 1626 || n = 1640)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1627)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1671)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pla
	dex
	Jne *-3
.endif
.else
; 1719 to 1875
.if(n = 1719 || n = 1722 || (n > 1725 && n < 1729) || (n > 1730 && n < 1737) || (n > 1738 && n < 1744) || n = 1746 || (n > 1747 && n < 1751) || n = 1754 || n = 1755 || n = 1757 || n = 1759 || (n > 1761 && n < 1765) || n = 1766 || n = 1767 || n = 1770 || n = 1771 || n = 1773 || n = 1775 || n = 1776 || n = 1778 || (n > 1779 && n < 1783) || n = 1784 || (n > 1786 && n < 1792) || (n > 1793 && n < 1799) || n = 1800 || (n > 1801 && n < 1809) || (n > 1810 && n < 1816) || n = 1818 || (n > 1819 && n < 1824) || n = 1827 || (n > 1828 && n < 1832) || n = 1834 || n = 1836 || n = 1838 || (n > 1841 && n < 1846) || n = 1847 || n = 1848 || (n > 1849 && n < 1855) || n = 1856 || (n > 1857 && n < 1864) || (n > 1865 && n < 1873) || n = 1874 || n = 1875)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 9 = 1) && (n = 1720 || (n > 1737 && n < 1757) || (n > 1773 && n < 1793) || (n > 1809 && n < 1865)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1724)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1725)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 8 = 1) && (n = 1729 || (n > 1744 && n < 1786) || n = 1801 || n = 1809 || (n > 1824 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
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
	nop
	nop
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
.elseif(n = 1839)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pha
	dex
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
.if((n >= 1876) && (n < 2188))
; 1876 to 2187
.if((n > 1875 && n < 1881) || (n > 1882 && n < 1889) || n = 1890 || (n > 1891 && n < 1897) || n = 1898 || n = 1899 || (n > 1900 && n < 1905) || n = 1906 || n = 1908 || (n > 1909 && n < 1913) || n = 1914 || n = 1915 || n = 1917 || n = 1919 || n = 1920 || (n > 1921 && n < 1925) || n = 1926 || n = 1928 || (n > 1929 && n < 1936) || (n > 1937 && n < 1945) || (n > 1945 && n < 1953) || (n > 1954 && n < 1961) || n = 1962 || (n > 1963 && n < 1969) || n = 1970 || n = 1971 || (n > 1972 && n < 1977) || (n > 1977 && n < 1981) || (n > 1981 && n < 1985) || (n > 1985 && n < 1990) || n = 1991 || n = 1992 || (n > 1993 && n < 1999) || (n > 2001 && n < 2008) || (n > 2009 && n < 2016) || (n > 2017 && n < 2023) || n = 2024 || n = 2027 || n = 2028 || (n > 2029 && n < 2033) || n = 2034 || (n > 2035 && n < 2041) || n = 2042 || n = 2043 || (n > 2044 && n < 2049) || (n > 2049 && n < 2053) || (n > 2053 && n < 2062) || (n > 2062 && n < 2071) || (n > 2071 && n < 2078) || n = 2079 || (n > 2080 && n < 2087) || n = 2088 || (n > 2089 && n < 2098) || (n > 2098 && n < 2105) || n = 2106 || (n > 2107 && n < 2114) || n = 2115 || (n > 2116 && n < 2123) || n = 2124 || (n > 2125 && n < 2134) || (n > 2134 && n < 2139) || (n > 2139 && n < 2143) || (n > 2143 && n < 2152) || (n > 2152 && n < 2161) || (n > 2161 && n < 2170) || (n > 2170 && n < 2179) || (n > 2179 && n < 2188))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 2042)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1937) || (n > 1953 && n < 1991) || (n > 2025 && n < 2180)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1999)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pha
	inx
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
.elseif(n = 2023)
	; 6 bytes
	ldx #184 ;hides 'clv'
	pla
	dex
	Jne *-3
.elseif(n = 2029)
	; 6 bytes
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
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
.elseif(n = 2139)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pha
	inx
	Jne *-3
.endif
.else
; 2188 to 2500
.if((n .mod 9 = 1) && ((n > 2187 && n < 2297)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif((n > 2188 && n < 2195) || n = 2196 || n = 2198 || (n > 2199 && n < 2204) || n = 2205 || (n > 2206 && n < 2213) || n = 2214 || (n > 2215 && n < 2224) || (n > 2224 && n < 2232) || (n > 2233 && n < 2242) || (n > 2242 && n < 2251) || (n > 2251 && n < 2260) || (n > 2260 && n < 2269) || (n > 2269 && n < 2278) || (n > 2278 && n < 2287) || (n > 2287 && n < 2296) || (n > 2296 && n < 2305) || (n > 2305 && n < 2319) || (n > 2319 && n < 2339) || (n > 2339 && n < 2353) || (n > 2353 && n < 2435) || (n > 2435 && n < 2459) || (n > 2459 && n < 2501))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 2195)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 2199)
	; 6 bytes
	ldx #56 ;hides 'sec'
	pla
	inx
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
.elseif(n = 2232)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
.elseif(n = 2305)
	; 6 bytes
	ldx #0
	pla
	dex
	Jne *-2
.elseif(n = 2319)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pha
	inx
	Jne *-3
.elseif(n = 2339)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pha
	dex
	Jne *-3
.elseif(n = 2353)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pla
	inx
	Jne *-3
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2459)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pha
	inx
	Jne *-3
.endif
.endif
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2551) || (n > 2551 && n < 2573) || (n > 2573 && n < 2705) || (n > 2705 && n < 3653) || (n > 3653 && n < 4580) || (n > 4580 && n < 4598) || (n > 4598 && n < 5001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 2551)
	; 6 bytes
	ldx #24 ;hides 'clc'
	pla
	inx
	Jne *-3
.elseif(n = 2573)
	; 6 bytes
	ldx #234 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2705)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pla
	inx
	Jne *-3
.elseif(n = 3653)
	; 6 bytes
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
.elseif(n = 4580)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
.elseif(n = 4598)
	; 6 bytes
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.endif
.endif

.endif
	rest
.endmacro
