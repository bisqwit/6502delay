; delay_n, delays n cycles with small code
; Will clobber S, Z+N, C, and V
; Will preserve A, X, Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 4 bytes or less when n <= 14
; - 5 bytes or less when n <= 17
; - 6 bytes or less when n <= 21
; - 7 bytes or less when n <= 24
; - 8 bytes or less when n <= 31
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 320
; -11 bytes or less when n <= 1310
; -12 bytes or less when n <= 1855
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=4, avg=4.9, max=6
; - When 21 <= n <= 30: min=6, avg=7.3, max=8
; - When 31 <= n <= 50: min=6, avg=7.0, max=9
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=10
; - When 301 <= n <= 500: min=8, avg=9.7, max=11
; - When 501 <= n <= 750: min=8, avg=9.7, max=11
; - When 751 <= n <= 1000: min=9, avg=9.7, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.6, max=13
; - When 2001 <= n <= 3000: min=10, avg=12.0, max=13
; - When 3001 <= n <= 5000: min=11, avg=12.9, max=13
; - When 5001 <= n <= 7500: min=13, avg=13.0, max=13
; - When 7501 <= n <= 10000: min=13, avg=13.0, max=13
; - When 10001 <= n <= 20000: min=13, avg=13.0, max=13
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
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8)
	; 2..3 bytes
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
.elseif((n > 8 && n < 12) || n = 13)
	; 3..4 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 14)
	; 4 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 5 bytes
	.repeat 5
	 pha
	.endrepeat
.elseif((n > 15 && n < 19))
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 19)
	; 6 bytes
	php
	delay_n 12
	plp
.endif
.else
; 20 to 39
.if(n = 20)
	; 6 bytes
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
	delay_n (n-19)
	delay_n 19
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
.elseif(n = 24 || n = 34)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 25)
	; 8 bytes
	delay_n 4
	delay_n (n-4)
.elseif(n = 26)
	; 8 bytes
	php
	pha
	delay_n 12
	pla
	plp
.elseif(n = 27)
	; 7 bytes
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
.elseif(n = 28)
	; 7 bytes
	pha
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
.elseif(n = 29)
	; 8 bytes
	pha
	sec
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
.elseif(n = 30)
	; 8 bytes
	pha
	clc
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
.elseif(n = 31)
	; 6 bytes
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif(n = 32)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 33)
	; 7 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif(n = 35)
	; 8 bytes
	pha
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif((n > 35 && n < 40))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.endif
.endif
.else
; 40 to 78
.if((n > 39 && n < 46) || (n > 46 && n < 79))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 46)
	; 6 bytes
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 157))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 292))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 292 || n = 293 || n = 299 || n = 304 || n = 309)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 294 || n = 301 || n = 303 || n = 308)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 295)
	; 8 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
.elseif((n > 295 && n < 299))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 300 || n = 310)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 302 || n = 307)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 305)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 306)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 311)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 312)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 369 || n = 376 || n = 411 || n = 439 || n = 446 || n = 453)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 314 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 339 || n = 344 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 364 || n = 366 || n = 374 || n = 379 || n = 384 || n = 386 || n = 389 || n = 391 || n = 394 || n = 399 || n = 401 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 444 || n = 449 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464 || n = 466)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 315 || n = 335 || n = 340 || n = 350 || n = 355 || (n > 364 && n < 381) || (n > 394 && n < 426) || (n > 439 && n < 456) || n = 465))
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 316)
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	pla
.elseif(n = 317 || n = 322 || n = 327 || n = 352 || n = 357 || n = 372 || n = 392 || n = 407 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 318 || n = 416)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 320 || n = 325 || n = 330 || n = 345 || n = 360 || n = 385 || n = 430 || n = 435 || n = 460)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif((n .mod 5 = 3) && ((n > 322 && n < 344) || (n > 357 && n < 379) || (n > 392 && n < 414) || (n > 427 && n < 449) || n = 463 || n = 468))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 331 && n < 348) || n = 362 || n = 367 || (n > 376 && n < 388) || n = 397 || n = 402 || n = 412 || n = 427 || (n > 436 && n < 458)))
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 334 || n = 341 || n = 348 || n = 383 || n = 404 || n = 418)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 336 || n = 371 || n = 441)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 346)
	; 9 bytes
	pha
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	pla
.elseif(n = 353 || n = 381 || n = 423 || n = 451 || n = 458)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 361)
	; 10 bytes
	pha
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	pla
.elseif(n = 388)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 390)
	; 8 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
.elseif(n = 396)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 406)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 409)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
	pla
.endif
.else
; 469 to 625
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541 || n = 546 || n = 549 || n = 554 || n = 559 || n = 564 || n = 566 || n = 569 || n = 574 || n = 581 || n = 584 || n = 589 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 485 || n = 495 || n = 500 || n = 505 || n = 520 || n = 525 || n = 535 || n = 540 || n = 565 || n = 570 || n = 580 || n = 585 || n = 605)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 472 || n = 482 || n = 497 || n = 507 || n = 527 || n = 577 || n = 582 || n = 587 || n = 592 || n = 602 || n = 622)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif((n .mod 5 = 3) && (n = 473 || (n > 497 && n < 519) || (n > 532 && n < 554) || (n > 567 && n < 589) || (n > 602 && n < 624)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 474 || n = 481 || n = 509 || n = 523 || n = 551 || n = 558 || n = 593 || n = 614 || n = 621)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif((n .mod 5 = 0) && (n = 475 || n = 480 || n = 490 || n = 510 || n = 515 || n = 530 || (n > 544 && n < 561) || n = 575 || (n > 589 && n < 601) || (n > 609 && n < 626)))
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 476 || n = 511)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 2) && (n = 477 || n = 487 || n = 492 || n = 502 || (n > 511 && n < 523) || (n > 531 && n < 573) || n = 597 || (n > 606 && n < 618)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 478)
	; 10 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
	pla
.elseif(n = 483)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 486 || n = 493 || n = 521 || n = 528 || n = 556 || n = 591)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 488 || n = 516 || n = 544 || n = 579 || n = 586)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 491)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 496)
	; 10 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 514)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	pla
.elseif(n = 561)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 563)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 571)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 576)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 598)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 606)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
; 626 to 937
.if(n = 626 || n = 633 || n = 661 || n = 668 || n = 766 || n = 773 || n = 801 || n = 836 || n = 871 || n = 878 || n = 913)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 628 || n = 656 || n = 691 || n = 733 || n = 803 || n = 824 || n = 838 || n = 866 || n = 873 || n = 901 || n = 908)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 686 || n = 689 || n = 694 || n = 699 || n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 724 || n = 729 || n = 734 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781 || n = 784 || n = 794 || n = 799 || n = 804 || n = 806 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 834 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 864 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 630 || n = 635 || n = 640 || n = 645 || n = 655)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 636 || n = 756)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 638 || n = 643 || n = 648 || n = 653 || n = 673 || n = 678 || n = 683 || n = 688 || n = 693 || n = 698 || n = 708 || n = 713 || n = 718 || n = 723 || n = 728 || n = 741 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 778 || n = 783 || n = 788 || n = 798 || n = 813 || n = 818 || n = 823 || n = 828 || n = 833 || n = 848 || n = 853 || n = 858 || n = 863 || n = 868 || n = 883 || n = 888 || n = 893 || n = 898 || n = 903 || n = 918 || n = 923 || n = 928 || n = 933)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 938)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 649 || n = 663 || n = 684 || n = 719 || n = 726 || n = 754 || n = 761 || n = 768 || n = 789 || n = 796 || n = 831 || n = 859 || n = 894 || n = 929 || n = 936)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 5 = 0) && (n = 650 || (n > 659 && n < 691) || (n > 699 && n < 936)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 651 || n = 786)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 658)
	; 10 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
	pla
.elseif(n = 695)
	; 8 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	pla
.elseif(n = 696 || n = 703 || n = 731 || n = 808 || n = 843 || n = 906)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 701)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 721 || n = 791 || n = 826 || n = 861 || n = 896 || n = 931)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 736)
	; 9 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
	pla
.elseif(n = 738)
	; 9 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
	pla
.elseif(n = 759)
	; 9 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
	pla
.elseif(n = 793)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 811)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 816)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 876)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 938 to 1250
.if((n .mod 5 = 3) && (n = 938 || (n > 952 && n < 969) || (n > 987 && n < 1009) || (n > 1022 && n < 1044) || n = 1058 || (n > 1067 && n < 1079) || (n > 1092 && n < 1114) || (n > 1127 && n < 1149) || (n > 1162 && n < 1184) || (n > 1197 && n < 1219) || n = 1233 || n = 1238 || n = 1248))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 939 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 991 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1039 || n = 1044 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091 || n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1109 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1149 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1179 || n = 1184 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1021) || (n > 1029 && n < 1251)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 941 || n = 948 || n = 976 || n = 983 || n = 1011 || n = 1018 || n = 1046 || n = 1053 || n = 1088 || n = 1116 || n = 1123 || n = 1151 || n = 1158 || n = 1186 || n = 1193 || n = 1221 || n = 1228)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1248)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && (n = 943 || (n > 963 && n < 979) || (n > 998 && n < 1014) || (n > 1033 && n < 1049) || (n > 1068 && n < 1084) || (n > 1103 && n < 1119) || (n > 1138 && n < 1154) || (n > 1173 && n < 1189) || (n > 1208 && n < 1224) || n = 1244))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 946)
	; 10 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
	tay
	pla
.elseif(n = 966 || n = 1001 || n = 1036 || n = 1071 || n = 1106 || n = 1141 || n = 1211 || n = 1246)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 969)
	; 9 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	tay
	pla
.elseif(n = 973)
	; 10 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 996)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1025)
	; 8 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
.elseif(n = 1031)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1063)
	; 10 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1081)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1176)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1243)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
; 1251 to 1406
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || (n > 1278 && n < 1406)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 5 = 2) && ((n > 1251 && n < 1288)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1253 || n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1295 || n = 1299 || n = 1301 || n = 1310 || n = 1329 || n = 1331 || n = 1336 || n = 1343 || n = 1345 || n = 1350 || n = 1352 || n = 1357 || n = 1359 || n = 1373 || n = 1385 || n = 1399 || n = 1401 || n = 1406)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1254 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1289 || n = 1294 || n = 1302 || n = 1309 || n = 1311 || n = 1316 || n = 1323 || n = 1325 || n = 1330 || n = 1334 || n = 1337 || n = 1339 || n = 1351 || n = 1355 || n = 1358 || n = 1365 || n = 1367 || n = 1372 || n = 1374 || n = 1379 || n = 1381 || n = 1386 || n = 1393 || n = 1395 || n = 1400)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1286)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif((n .mod 7 = 3) && (n = 1256 || n = 1263 || n = 1291 || (n > 1304 && n < 1404)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1281)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1290)
	; 9 bytes
	pha
	sec
	lda #0
	sbc #1
	Jne *-2
	pla
.elseif(n = 1292)
	; 9 bytes
	pha
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1296)
	; 9 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1297 || n = 1303)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1298)
	; 9 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1304 || n = 1313 || n = 1327 || n = 1383)
	; 11..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1306 || n = 1322 || n = 1338 || n = 1346 || n = 1378 || n = 1394 || n = 1402)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 3) && (n = 1308 || n = 1317 || (n > 1343 && n < 1381)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1315 || n = 1360 || n = 1369)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1318 || n = 1366)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
.elseif(n = 1320 || n = 1332)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1324 || n = 1348 || n = 1364 || n = 1388)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1341)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jpl *-5
	tay
	pla
.elseif(n = 1376)
	; 12 bytes
	pha
	tya
	ldy #123
	delay_n 4
	nop
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1387)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
	pla
.elseif(n = 1390)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1392 || n = 1404)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1397)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jpl *-5
	tay
	pla
.endif
.else
; 1407 to 1562
.if((n .mod 9 = 3) && ((n > 1406 && n < 1426) || n = 1443 || (n > 1469 && n < 1507) || (n > 1532 && n < 1561)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1408 || n = 1413 || n = 1415 || n = 1422 || n = 1427 || n = 1429 || n = 1434 || n = 1448 || n = 1455 || n = 1457 || n = 1469 || n = 1471 || n = 1478 || n = 1483 || n = 1485 || n = 1499 || n = 1511 || n = 1518 || n = 1520 || n = 1525 || n = 1539 || n = 1541 || n = 1553 || n = 1555)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1430 || n = 1472 || n = 1535)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1410)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	pla
.elseif(n = 1411 || n = 1437 || n = 1439 || n = 1467 || n = 1509 || n = 1523 || n = 1537)
	; 11..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1560)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1414 || n = 1421 || n = 1435 || n = 1442 || n = 1446 || n = 1449 || n = 1456 || n = 1463 || n = 1465 || n = 1477 || n = 1481 || n = 1484 || n = 1491 || n = 1493 || n = 1498 || n = 1502 || n = 1505 || n = 1507 || n = 1519 || n = 1521 || n = 1526 || n = 1528 || n = 1540 || n = 1544 || n = 1547 || n = 1554 || n = 1561)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && (n = 1417 || n = 1424 || (n > 1437 && n < 1558)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1418 || n = 1450 || n = 1458 || n = 1474 || n = 1490 || n = 1514 || n = 1530 || n = 1546 || n = 1562)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1420 || n = 1436 || n = 1444 || n = 1460 || n = 1492 || n = 1516 || n = 1532 || n = 1556)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 1) && ((n > 1422 && n < 1442) || (n > 1485 && n < 1514) || n = 1549 || n = 1558))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1428 || n = 1476 || n = 1500 || n = 1512 || n = 1548)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1431)
	; 9 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
	pla
.elseif(n = 1451)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1453)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1462 || n = 1534)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
.elseif(n = 1464)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
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
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1568 || n = 1575 || n = 1577 || n = 1581 || n = 1582 || n = 1589 || n = 1591 || n = 1597 || n = 1603 || n = 1610 || n = 1617 || n = 1619 || n = 1624 || n = 1631 || n = 1633 || n = 1638 || n = 1645 || n = 1647 || n = 1652 || n = 1670 || n = 1673 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1708 || n = 1710 || n = 1715 || n = 1717)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1628) || n = 1641 || (n > 1654 && n < 1719)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1565)
	; 12 bytes
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
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1714)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1567 || n = 1576 || n = 1630 || n = 1675 || n = 1702)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 3) && (n = 1569 || (n > 1595 && n < 1624) || (n > 1658 && n < 1696)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1570 || n = 1586 || n = 1602 || n = 1618 || n = 1626 || n = 1642 || n = 1658 || n = 1674 || n = 1682 || n = 1698 || n = 1714)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1572 || n = 1584 || n = 1644 || n = 1656 || n = 1680 || n = 1716)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1574 || n = 1583 || n = 1590 || n = 1598 || n = 1609 || n = 1611 || n = 1616 || n = 1625 || n = 1635 || n = 1637 || n = 1649 || n = 1651 || n = 1665 || n = 1667 || n = 1672 || n = 1679 || n = 1681 || n = 1688 || n = 1707 || n = 1709)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1579)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1588 || n = 1604 || n = 1612 || n = 1628 || n = 1660 || n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1593 || n = 1640 || n = 1663 || n = 1691 || n = 1705)
	; 11..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1595)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1600)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 1607)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1621)
	; 10 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
	pla
.elseif(n = 1632)
	; 9 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	pla
.elseif(n = 1634)
	; 9 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
	pla
.elseif(n = 1639 || n = 1653 || n = 1661)
	; 11..12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1646)
	; 9 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1648)
	; 9 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1654)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
	pla
.elseif(n = 1666)
	; 10 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1693)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
	pla
.elseif(n = 1712)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1768 || n = 1789)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1791)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1721 || n = 1723 || n = 1735 || n = 1737 || n = 1742 || n = 1744 || n = 1766 || n = 1777 || n = 1782 || n = 1784 || n = 1791 || n = 1793)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 3) && ((n > 1721 && n < 1759) || n = 1785 || n = 1794))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1724 || n = 1756 || n = 1772 || n = 1780 || n = 1796)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1796)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1726)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1728 || n = 1752 || n = 1764)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1729 || n = 1733 || n = 1736 || n = 1743 || n = 1745 || n = 1750 || n = 1757 || n = 1759 || n = 1765 || n = 1773 || n = 1775 || n = 1778 || n = 1792)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1730 || n = 1738 || n = 1754 || n = 1770 || n = 1786)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1747)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 1761)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1763)
	; 10 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1771)
	; 10 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1779)
	; 10 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 1797 to 1875
.if(n = 1797)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1798 || n = 1822 || n = 1870)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1799 || n = 1806 || n = 1814 || n = 1823 || n = 1829 || n = 1831 || n = 1841 || n = 1845 || n = 1854 || n = 1859 || n = 1862)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1824 || n = 1836 || n = 1860 || n = 1872)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1801)
	; 10 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1802)
	; 10 bytes
	pha
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	pla
.elseif((n .mod 9 = 3) && ((n > 1802 && n < 1876)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1804)
	; 10 bytes
	pha
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1805 || n = 1807 || n = 1813 || n = 1815 || n = 1833 || n = 1838 || n = 1840 || n = 1851 || n = 1861 || n = 1863 || n = 1867)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1808 || n = 1809 || n = 1811 || n = 1817)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1810)
	; 10 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1816 || n = 1849 || n = 1871)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 8 = 2) && ((n > 1817 && n < 1843) || n = 1858 || n = 1874))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 1) && (n = 1819 || (n > 1845 && n < 1874)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1820 || n = 1828 || n = 1844 || n = 1852 || n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1825 || n = 1847 || n = 1869)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 1827)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1835)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1837)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
	pla
.elseif(n = 1843)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1850)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1853)
	; 12 bytes
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
.elseif(n = 1856 || n = 1865)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876 || n = 1892 || n = 1900 || n = 1916 || n = 1924 || n = 1940 || n = 1948)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1877 || n = 1878 || n = 1886 || n = 1895 || n = 1910 || n = 1913 || n = 1931 || n = 1949 || n = 1950)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 2 = 1) && (n = 1879 || n = 1885 || n = 1887 || (n > 1896 && n < 1906) || (n > 1916 && n < 1924) || n = 1939 || n = 1941 || n = 1951))
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1880)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 1881 || n = 1883 || n = 1889 || n = 1907 || n = 1953)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 8 = 2) && ((n > 1881 && n < 1931) || n = 1946))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 3) && ((n > 1883 && n < 1948)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1888)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1891 || n = 1909 || n = 1945)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1894 || n = 1942)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1896 || n = 1908 || n = 1932 || n = 1944)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 11 = 1) && ((n > 1903 && n < 1938)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1912 || n = 1952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1918)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1925 || n = 1934 || n = 1943)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1927)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1928)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
	pla
.elseif(n = 1933)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1935)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 1936)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 1954 to 2031
.if((n .mod 8 = 2) && ((n > 1953 && n < 2003) || n = 2018 || n = 2026))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1955 || n = 1987 || n = 2009 || n = 2024)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 9 = 3) && ((n > 1955 && n < 2029)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1957 || n = 2023)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 1958 || n = 2000)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 1961)
	; 11 bytes
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 1) && (n = 1963 || (n > 1989 && n < 2018)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1964 || n = 1972 || n = 1988 || n = 1996 || n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1966 || n = 2014)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1967 || n = 1973 || n = 1976 || n = 1982 || n = 1985 || n = 1993 || n = 1998 || n = 2006 || n = 2021 || n = 2029 || n = 2030)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1968 || n = 1980 || n = 2004 || n = 2016)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1969 || n = 1975 || n = 1977 || n = 1989 || n = 1995 || n = 1997 || n = 2005 || n = 2007 || n = 2011 || n = 2013 || n = 2015 || n = 2031)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1971)
	; 11 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1979)
	; 11 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
	pla
.elseif(n = 1981)
	; 11 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1984)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 1991)
	; 11 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2003 || n = 2025)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2022)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2027)
	; 11 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
; 2032 to 2109
.if(n = 2032)
	; 11 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2033 || n = 2051 || n = 2059 || n = 2081 || n = 2087 || n = 2097 || n = 2108)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2034)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 1) && (n = 2035 || (n > 2052 && n < 2081) || n = 2107))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2036 || n = 2044)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2036 && n < 2110)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2038 || n = 2074 || n = 2086)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2039 || n = 2048 || n = 2054 || n = 2057 || n = 2066 || n = 2075 || n = 2078 || n = 2090 || n = 2093)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2040 || n = 2052 || n = 2076 || n = 2088)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2041 || n = 2043 || n = 2049 || n = 2061 || n = 2063 || n = 2077 || n = 2079 || n = 2083 || n = 2085 || n = 2092 || n = 2094 || n = 2099 || n = 2101)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2042 || n = 2050)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 2045 || n = 2067)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2047 || n = 2069 || n = 2102)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2056 || n = 2070 || n = 2084)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2058)
	; 11 bytes
	pha
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 2060)
	; 11 bytes
	pha
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2065)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	php
	cmp $C5
	plp
	Jne *-5
	tay
	pla
.elseif(n = 2068)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2089)
	; 10 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
	pla
.elseif(n = 2095 || n = 2104)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2096)
	; 10 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	pla
.elseif(n = 2098)
	; 10 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 2103 || n = 2105)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2106)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2122 || n = 2146 || n = 2158 || n = 2182)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2111 || n = 2120 || n = 2129 || n = 2138 || n = 2147 || n = 2150 || n = 2156 || n = 2162 || n = 2165 || n = 2174 || n = 2183 || n = 2186)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2112 || n = 2124 || n = 2148 || n = 2160 || n = 2184)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2113 || n = 2135 || n = 2157 || n = 2168)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2114)
	; 10 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 2115 || n = 2117 || n = 2119 || n = 2128 || n = 2137 || n = 2139 || n = 2149 || n = 2151 || n = 2164 || n = 2173 || n = 2175 || n = 2185 || n = 2187)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2116)
	; 10 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2117 && n < 2182)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2121 || n = 2123)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 1) && ((n > 2124 && n < 2180)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2126 || n = 2140)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2130 || n = 2180)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2131 || n = 2153 || n = 2159 || n = 2169 || n = 2171 || n = 2176)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2132)
	; 12 bytes
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
.elseif((n .mod 11 = 10) && ((n > 2132 && n < 2178)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2141)
	; 12 bytes
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
.elseif(n = 2142)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2167)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2178)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if((n .mod 9 = 1) && ((n > 2187 && n < 2225)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2189)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2189 && n < 2227)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2191 || n = 2193 || n = 2204 || n = 2214)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2192 || n = 2198 || n = 2203 || n = 2213 || n = 2219 || n = 2222)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194 || n = 2218)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2195 || n = 2205 || n = 2207 || n = 2225)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2196 || n = 2220)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2200)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2201)
	; 11 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2202)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 11
	Jne *-5
	tay
	pla
.elseif(n = 2209)
	; 11 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
	pla
.elseif(n = 2210)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2211)
	; 11 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2212 || n = 2223)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2216)
	; 12 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
.elseif(n = 2221)
	; 11 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2229 || n = 2257 || n = 2259)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2228 || n = 2237 || n = 2246 || n = 2255 || n = 2258 || n = 2263 || n = 2264)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2230 || n = 2254)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2231)
	; 12 bytes
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
.elseif(n = 2232 || n = 2256)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2233 || n = 2242)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2234 || n = 2245)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2234 && n < 2263)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2236)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2238)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2239 || n = 2241 || n = 2248)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2240 || n = 2250)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2243)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
	tax
	pla
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
.elseif(n = 2251)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2252)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2260)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2261)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2265)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2302)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2267 || n = 2300)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2268 || n = 2292 || n = 2304)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2269)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2270)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2270 && n < 2299)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2272)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2273 || n = 2282 || n = 2283 || n = 2293 || n = 2299 || n = 2303)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2274 || n = 2284 || n = 2295)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2275 || n = 2277 || n = 2285 || n = 2286)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2276)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2278)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2279)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2281)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2287)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2288)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2290)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2291)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2294)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2296)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2297)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2301)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2305 to 2343
.if(n = 2305)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2306)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2307)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2308 || n = 2336)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2309 || n = 2317 || n = 2318)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2310 || n = 2320)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2311 || n = 2322 || n = 2333)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2312 || n = 2332)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2313 || n = 2321 || (n > 2322 && n < 2326) || n = 2327 || n = 2335 || n = 2339)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2314)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2315)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2316)
	; 11 bytes
	pha
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2319 || n = 2329 || n = 2341 || n = 2343)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2326 || n = 2338)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2328 || n = 2340)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2330)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	cmp $C5
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2331)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2334)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2337)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2342)
	; 11 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.endif
.else
; 2344 to 2500
.if((n .mod 11 = 1) && ((n > 2343 && n < 2378) || n = 2399 || (n > 2420 && n < 2499)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2345 || n = 2356 || n = 2365 || n = 2367 || n = 2379 || n = 2389 || n = 2391 || n = 2401 || n = 2403 || n = 2413 || n = 2415 || n = 2425 || n = 2427 || n = 2437 || n = 2439 || n = 2449 || n = 2451 || n = 2461 || n = 2473 || n = 2497 || n = 2499)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2346 || n = 2347 || n = 2349 || (n > 2356 && n < 2360) || n = 2361 || n = 2368 || n = 2369 || n = 2371 || n = 2373 || n = 2381 || (n > 2382 && n < 2386) || (n > 2392 && n < 2397) || n = 2405 || n = 2407 || n = 2409 || n = 2411 || n = 2416 || n = 2417 || n = 2423 || n = 2428 || n = 2433 || n = 2435 || n = 2444 || n = 2445 || n = 2447 || (n > 2454 && n < 2458) || n = 2459 || n = 2464 || (n > 2465 && n < 2470) || n = 2471 || (n > 2476 && n < 2480) || n = 2481 || n = 2483 || n = 2489 || n = 2491 || n = 2493 || n = 2495)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2348)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2350)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2351)
	; 12 bytes
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2351 && n < 2497)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2353)
	; 11 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 2) && (n = 2354 || (n > 2389 && n < 2451) || n = 2474 || n = 2486))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2360 || n = 2370 || n = 2380 || n = 2430 || n = 2440 || n = 2480 || n = 2500)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 10) && ((n > 2361 && n < 2435) || (n > 2457 && n < 2495)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2363)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2372 || n = 2382 || n = 2442 || n = 2452 || n = 2492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2375)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2378 || n = 2392 || n = 2406 || n = 2420 || n = 2462 || n = 2490)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2387)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2397 || n = 2408 || n = 2419 || n = 2441 || n = 2463 || n = 2485)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2404 || n = 2418 || n = 2488)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2429)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
	pla
.elseif(n = 2431)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	bit $24
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2446)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
.elseif(n = 2453)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2475)
	; 12 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
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
; 2501 to 2656
.if(n = 2501 || n = 2503 || n = 2505 || (n > 2512 && n < 2516) || n = 2517 || (n > 2523 && n < 2529) || (n > 2535 && n < 2540) || n = 2541 || n = 2543 || n = 2548 || n = 2549 || n = 2555 || n = 2561 || n = 2567 || n = 2576 || n = 2577 || n = 2579 || n = 2591 || n = 2596 || n = 2598 || n = 2599 || n = 2601 || n = 2603 || n = 2609 || n = 2611 || n = 2612 || n = 2615 || (n > 2619 && n < 2626) || n = 2627 || (n > 2631 && n < 2638) || (n > 2644 && n < 2648))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2502 || n = 2512 || n = 2552)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2504 || n = 2546 || n = 2574 || n = 2588 || n = 2644)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2505 && n < 2651)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2507 || n = 2529 || n = 2573 || n = 2595 || n = 2617)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 0) && ((n > 2507 && n < 2653)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 11 = 1) && (n = 2509 || n = 2531 || (n > 2552 && n < 2576) || (n > 2596 && n < 2642)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2510 || n = 2522 || n = 2534 || n = 2558 || n = 2570 || n = 2594 || n = 2606 || n = 2618 || n = 2642 || n = 2654)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 2 = 1) && (n = 2511 || n = 2521 || n = 2523 || n = 2533 || n = 2535 || n = 2545 || n = 2547 || n = 2557 || n = 2559 || n = 2563 || n = 2565 || n = 2569 || n = 2571 || (n > 2580 && n < 2590) || n = 2593 || n = 2605 || n = 2607 || n = 2629 || n = 2631 || n = 2643 || n = 2653 || n = 2655))
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2516 || n = 2600 || n = 2656)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2519)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2540 || n = 2550)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2551)
	; 11 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
	pla
.elseif(n = 2560)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	pla
.elseif(n = 2562)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2572)
	; 12 bytes
	pha
	tya
	ldy #0
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2582)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2584)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2586)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2610)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2613)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
	tay
	pla
.elseif(n = 2639)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2648)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
.elseif(n = 2649)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	tax
	pla
.elseif(n = 2651)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 2657 to 2812
.if(n = 2657 || n = 2659 || n = 2660 || n = 2668 || n = 2669 || n = 2671 || n = 2673 || n = 2675 || (n > 2679 && n < 2683) || n = 2687 || n = 2692 || n = 2693 || n = 2695 || n = 2697 || n = 2699 || n = 2704 || n = 2706 || n = 2708 || n = 2709 || n = 2711 || n = 2717 || (n > 2718 && n < 2722) || n = 2723 || (n > 2729 && n < 2734) || n = 2741 || (n > 2742 && n < 2746) || n = 2752 || n = 2753 || n = 2755 || n = 2757 || n = 2764 || n = 2765 || n = 2767 || n = 2769 || n = 2776 || n = 2777 || n = 2780 || n = 2781 || (n > 2787 && n < 2792) || (n > 2799 && n < 2804) || n = 2807)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2658 || n = 2672 || n = 2714 || n = 2728 || n = 2742 || n = 2756 || n = 2798 || n = 2812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 11 = 10) && (n = 2661 || (n > 2682 && n < 2728)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 10) && ((n > 2661 && n < 2807)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 11 = 1) && (n = 2663 || (n > 2684 && n < 2774) || n = 2795))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2663 && n < 2809)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2665 || n = 2667 || n = 2677 || n = 2679 || n = 2689 || n = 2691 || n = 2701 || n = 2703 || n = 2713 || n = 2715 || n = 2725 || n = 2737 || n = 2739 || n = 2763 || n = 2775 || n = 2785 || n = 2787 || n = 2797 || n = 2799 || n = 2809 || n = 2811)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 2) && ((n > 2665 && n < 2703) || (n > 2725 && n < 2751) || n = 2774 || n = 2786))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2670 || n = 2684 || n = 2754 || n = 2768)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2735)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2747)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2749)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2759)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2761)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2766)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2771)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2778)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2779)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2783)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2792)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2793)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2804)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2805)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2810)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
.endif
.endif
.else
; 2813 to 3125
.if(n = 2813 || n = 2821 || n = 2823 || n = 2833 || n = 2835 || n = 2845 || n = 2847 || n = 2855 || n = 2857 || n = 2859 || n = 2869 || n = 2871 || n = 2881 || n = 2883 || n = 2893 || n = 2895 || n = 2905 || n = 2907 || n = 2917 || n = 2919 || n = 2929 || n = 2931 || n = 2941 || n = 2943 || n = 2953 || n = 2955 || n = 2965 || n = 2967 || n = 2977 || n = 2979 || n = 2989 || n = 2991 || n = 3001 || n = 3003 || n = 3015 || n = 3025 || n = 3027 || n = 3037 || n = 3039 || n = 3049 || n = 3051 || n = 3061 || n = 3063 || n = 3073 || n = 3075 || n = 3085 || n = 3087)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2825 || n = 2829 || n = 2831 || n = 2836 || n = 2837 || n = 2839 || n = 2841 || n = 2843 || (n > 2847 && n < 2852) || n = 2853 || (n > 2859 && n < 2866) || n = 2867 || n = 2872 || n = 2873 || (n > 2874 && n < 2878) || n = 2879 || (n > 2883 && n < 2890) || n = 2891 || (n > 2896 && n < 2902) || n = 2903 || n = 2909 || (n > 2910 && n < 2914) || n = 2915 || n = 2920 || n = 2921 || n = 2923 || n = 2925 || n = 2927 || (n > 2931 && n < 2936) || n = 2937 || n = 2939 || (n > 2943 && n < 2950) || (n > 2955 && n < 2962) || n = 2963 || (n > 2967 && n < 2974) || n = 2975 || (n > 2981 && n < 2986) || n = 2987 || n = 2993 || (n > 2994 && n < 2998) || n = 2999 || n = 3004 || n = 3005 || n = 3007 || n = 3009 || n = 3011 || (n > 3015 && n < 3020) || n = 3021 || n = 3023 || (n > 3027 && n < 3034) || n = 3035 || (n > 3039 && n < 3046) || n = 3047 || n = 3053 || n = 3054 || n = 3056 || n = 3057 || n = 3059 || (n > 3064 && n < 3070) || n = 3071 || n = 3077 || (n > 3078 && n < 3082) || n = 3083 || n = 3088 || n = 3089 || n = 3091 || (n > 3092 && n < 3104) || n = 3105 || (n > 3106 && n < 3118) || n = 3119 || (n > 3120 && n < 3126))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2815)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2816)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2817)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2817 && n < 3071)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 12 = 0) && ((n > 2819 && n < 3073)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 2) && ((n > 2821 && n < 2871) || (n > 2893 && n < 2955) || (n > 2977 && n < 3039) || (n > 3061 && n < 3087)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 10) && (n = 2824 || n = 2838 || (n > 2907 && n < 2937) || (n > 2991 && n < 3021) || (n > 3075 && n < 3119)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && (n = 2826 || n = 2840 || (n > 2881 && n < 2925) || (n > 2965 && n < 3009) || (n > 3049 && n < 3121)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2827)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2828)
	; 12 bytes
	pha
	tya
	ldy #0
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2852)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
	pla
.elseif(n = 2874)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2951)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2981)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3013)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 3052)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3055)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	php
	cmp $C5
	plp
	Jne *-5
	tax
	pla
.elseif(n = 3082)
	; 11 bytes
	pha
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 3084)
	; 11 bytes
	pha
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.endif
.endif
.else
; 3126 to 3750
.if((n > 3125 && n < 3132) || n = 3133 || (n > 3134 && n < 3146) || n = 3147 || (n > 3148 && n < 3160) || n = 3161 || (n > 3162 && n < 3174) || n = 3175 || (n > 3176 && n < 3188) || n = 3189 || (n > 3190 && n < 3194) || (n > 3194 && n < 3202) || n = 3203 || (n > 3204 && n < 3210) || (n > 3210 && n < 3216) || n = 3217 || (n > 3218 && n < 3230) || n = 3231 || (n > 3232 && n < 3244) || n = 3245 || (n > 3246 && n < 3256) || n = 3257 || (n > 3261 && n < 3272) || n = 3273 || (n > 3274 && n < 3284) || n = 3285 || (n > 3289 && n < 3296) || (n > 3296 && n < 3300) || n = 3301 || (n > 3302 && n < 3309) || n = 3310 || n = 3312 || n = 3313 || n = 3315 || (n > 3316 && n < 3326) || n = 3327 || n = 3329 || (n > 3330 && n < 3342) || n = 3343 || (n > 3344 && n < 3356) || n = 3357 || (n > 3358 && n < 3370) || n = 3371 || (n > 3372 && n < 3384) || n = 3385 || (n > 3386 && n < 3398) || n = 3399 || n = 3401 || (n > 3402 && n < 3412) || n = 3413 || n = 3415 || n = 3417 || (n > 3418 && n < 3426) || n = 3427 || n = 3429 || n = 3430 || n = 3432 || n = 3433 || (n > 3434 && n < 3440) || n = 3441 || (n > 3442 && n < 3446) || (n > 3446 && n < 3454) || n = 3455 || (n > 3456 && n < 3468) || n = 3469 || (n > 3470 && n < 3482) || n = 3483 || (n > 3484 && n < 3496) || n = 3497 || (n > 3498 && n < 3510) || n = 3511 || (n > 3512 && n < 3524) || n = 3525 || (n > 3526 && n < 3538) || n = 3539 || (n > 3540 && n < 3552) || n = 3553 || (n > 3554 && n < 3566) || n = 3567 || (n > 3568 && n < 3580) || n = 3581 || (n > 3582 && n < 3594) || n = 3595 || (n > 3596 && n < 3706) || (n > 3706 && n < 3720) || n = 3721 || (n > 3722 && n < 3751))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 14 = 10) && ((n > 3131 && n < 3245) || n = 3272 || (n > 3299 && n < 3581)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3133 && n < 3583)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3194)
	; 12 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3210)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3256)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	pla
.elseif(n = 3258)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 3259 || n = 3261 || n = 3287 || n = 3289)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3284)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
.elseif(n = 3286)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3296)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3309)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
	pla
.elseif(n = 3311)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3326)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3402)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3416)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3418)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3431)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3434)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3446)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3594)
	; 12 bytes
	pha
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3596)
	; 12 bytes
	pha
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3706)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 3720)
	; 12 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	pla
.elseif(n = 3722)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 3751 to 5000
.if(n = 3751 || n = 3753 || (n > 3754 && n < 3867) || (n > 3867 && n < 5001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3752)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
.elseif(n = 3754)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3867)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
	pla
.endif
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 10001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif

.endif
	rest
.endmacro
