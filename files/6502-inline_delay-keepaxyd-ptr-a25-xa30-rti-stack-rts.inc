; delay_n, delays n cycles with small code
; Will clobber S, Z+N, C, and V
; Will preserve A, X, Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a zeropage two-byte variable @ptrtemp, that points
; into a memory address that can be safely read and overwritten
; with random data.
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
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 302
; -10 bytes or less when n <= 320
; -11 bytes or less when n <= 1312
; -12 bytes or less when n <= 1888
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=9
; - When 301 <= n <= 500: min=8, avg=9.7, max=11
; - When 501 <= n <= 750: min=8, avg=9.7, max=11
; - When 751 <= n <= 1000: min=9, avg=9.7, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.6, max=13
; - When 2001 <= n <= 3000: min=10, avg=11.8, max=13
; - When 3001 <= n <= 5000: min=10, avg=12.5, max=13
; - When 5001 <= n <= 7500: min=11, avg=13.0, max=13
; - When 7501 <= n <= 10000: min=12, avg=13.0, max=13
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
; 1 to 156
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 15 || n = 17 || n = 18 || n = 21 || n = 28 || n = 30)
	; 2..6 bytes
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
.elseif((n > 8 && n < 12) || n = 20 || n = 22 || n = 33)
	; 3..6 bytes
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
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 24 || n = 25)
	; 5..6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 31)
	; 6 bytes
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif((n > 33 && n < 39) || (n > 39 && n < 46) || (n > 46 && n < 157))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 46)
	; 6 bytes
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
.endif
.else
; 157 to 312
.if((n > 156 && n < 290))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 290 || n = 291 || n = 303 || n = 304 || n = 309)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 292)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 293 || n = 294 || n = 296)
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 295)
	; 8 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
.elseif((n > 296 && n < 303) || n = 308)
	; 9..10 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 307)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 310)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
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
.elseif((n .mod 5 = 3) && ((n > 322 && n < 344) || (n > 357 && n < 379) || n = 393 || n = 398 || n = 408 || n = 413 || (n > 427 && n < 444) || n = 463))
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
.elseif(n = 334 || n = 341 || n = 348 || n = 383 || n = 418)
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
.elseif(n = 403)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 404)
	; 9 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	brk
	Jne *-4
	pla
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
.elseif(n = 448)
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	nop
	brk
	Jne *-5
	pla
.elseif(n = 468)
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	nop
	brk
	Jcs *-5
	pla
.endif
.else
; 469 to 625
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541 || n = 546 || n = 549 || n = 554 || n = 559 || n = 564 || n = 569 || n = 574 || n = 581 || n = 584 || n = 589 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 624)
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
.elseif((n .mod 5 = 3) && (n = 473 || (n > 497 && n < 519) || (n > 532 && n < 554) || (n > 572 && n < 589) || (n > 602 && n < 624)))
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
.elseif(n = 566)
	; 9 bytes
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	brk
	Jne *-4
	pla
.elseif(n = 568)
	; 10 bytes
	pha
	sec
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	brk
	Jne *-4
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
.elseif(n = 638 || n = 643 || n = 648 || n = 653 || n = 673 || n = 678 || n = 683 || n = 688 || n = 693 || n = 698 || n = 713 || n = 718 || n = 723 || n = 728 || n = 741 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 778 || n = 783 || n = 788 || n = 798 || n = 813 || n = 818 || n = 823 || n = 828 || n = 833 || n = 848 || n = 853 || n = 858 || n = 863 || n = 868 || n = 883 || n = 888 || n = 893 || n = 898 || n = 903 || n = 918 || n = 923 || n = 928 || n = 933)
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
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 708)
	; 10 bytes
	brk
	delay_n (n-13)
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
.if((n .mod 5 = 3) && (n = 938 || (n > 952 && n < 969) || (n > 987 && n < 1009) || (n > 1022 && n < 1034) || n = 1043 || n = 1058 || (n > 1067 && n < 1079) || (n > 1092 && n < 1114) || (n > 1127 && n < 1149) || (n > 1162 && n < 1184) || (n > 1197 && n < 1219) || n = 1233 || n = 1238 || n = 1248))
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
.elseif(n = 966 || n = 1001 || n = 1071 || n = 1106 || n = 1141 || n = 1211 || n = 1246)
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
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 1036)
	; 10 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	brk
	Jmi *-3
	tay
	pla
.elseif(n = 1038)
	; 10 bytes
	brk
	delay_n (n-13)
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
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || (n > 1278 && n < 1329)))
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
.elseif(n = 1253 || n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1295 || n = 1299 || n = 1301 || n = 1310)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1254 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1289 || n = 1294 || n = 1302 || n = 1309 || n = 1316 || n = 1323)
	; 10..11 bytes
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
.elseif((n .mod 7 = 3) && (n = 1256 || n = 1263 || n = 1291 || (n > 1304 && n < 1327)))
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
.elseif(n = 1304)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1306 || n = 1322)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1308 || n = 1317)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1311)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1313 || n = 1327)
	; 12 bytes
	pha
	tya
	ldy #(n-18)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1315)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1318)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
.elseif(n = 1320)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1324)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1325)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1331 || n = 1336 || n = 1345 || n = 1350 || n = 1357 || n = 1359 || n = 1373 || n = 1381 || n = 1385 || n = 1399 || n = 1401)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1330 || n = 1337 || n = 1339 || n = 1351 || n = 1355 || n = 1358 || n = 1365 || n = 1367 || n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1393 || n = 1395 || n = 1400)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1332)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1404)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1334)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1406)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1338 || n = 1346 || n = 1394 || n = 1402)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1343)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1343 && n < 1381)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1348 || n = 1364 || n = 1388)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1352)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1360 || n = 1369)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1366)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
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
.elseif(n = 1378)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
	tax
	pla
.elseif(n = 1383)
	; 12 bytes
	pha
	tya
	ldy #(n-18)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1406)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
	pla
.endif
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
.elseif(n = 1408 || n = 1413 || n = 1422 || n = 1427 || n = 1429 || n = 1434 || n = 1455 || n = 1457 || n = 1471 || n = 1483 || n = 1485 || n = 1499 || n = 1511 || n = 1518 || n = 1520 || n = 1525 || n = 1539 || n = 1553 || n = 1555)
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
.elseif(n = 1411 || n = 1439 || n = 1467 || n = 1509 || n = 1537)
	; 12 bytes
	pha
	tya
	ldy #(n-18)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 1415)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 1437)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 1448)
	; 11 bytes
	pha
	clc
	lda #184 ;hides 'clv'
	brk
	adc #1
	Jne *-5
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
.elseif(n = 1469)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1478)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1523)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A8A0'
	ldy #168
	dey
	Jne *-4
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
.elseif(n = 1541)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1568 || n = 1575 || n = 1581 || n = 1582 || n = 1589 || n = 1591 || n = 1597 || n = 1603 || n = 1610 || n = 1617 || n = 1619 || n = 1624 || n = 1631 || n = 1633 || n = 1638)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1628)))
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
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1637)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1567 || n = 1576 || n = 1630)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 3) && (n = 1569 || (n > 1595 && n < 1624)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1570 || n = 1586 || n = 1602 || n = 1618 || n = 1626)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1572 || n = 1584)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1574 || n = 1583 || n = 1590 || n = 1598 || n = 1609 || n = 1611 || n = 1616 || n = 1625 || n = 1635 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1577)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1588 || n = 1604 || n = 1612 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1593)
	; 12 bytes
	pha
	tya
	ldy #(n-18)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 1639)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1640)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 1641 to 1718
.if((n .mod 7 = 3) && (n = 1641 || (n > 1654 && n < 1719)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1642 || n = 1658 || n = 1674 || n = 1682 || n = 1698 || n = 1714)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1644 || n = 1656 || n = 1680 || n = 1716)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1645 || n = 1652 || n = 1670 || n = 1673 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1708 || n = 1710 || n = 1715 || n = 1717)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646)
	; 9 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1647 || n = 1661)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1648)
	; 9 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1649 || n = 1651 || n = 1665 || n = 1672 || n = 1679 || n = 1681 || n = 1688 || n = 1707 || n = 1709)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1653)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1654)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1658 && n < 1696)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1660 || n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1663 || n = 1691 || n = 1705)
	; 12 bytes
	pha
	tya
	ldy #(n-18)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1667)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1675 || n = 1702)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1703)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1712)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1768 || n = 1789)
	; 12 bytes
	pha
	tya
	ldy #(n-18)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 1721)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1723 || n = 1735 || n = 1737 || n = 1742 || n = 1744 || n = 1777 || n = 1782 || n = 1791)
	; 11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1729 || n = 1733 || n = 1736 || n = 1743 || n = 1745 || n = 1750 || n = 1757 || n = 1759 || n = 1765 || n = 1773 || n = 1778 || n = 1792)
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
.elseif(n = 1766)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
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
.elseif(n = 1775)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
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
.elseif(n = 1784)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1793)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	dey
	Jne *-4
	tay
	pla
.endif
.else
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
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
.elseif(n = 1798 || n = 1822)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1799 || n = 1806 || n = 1814 || n = 1823 || n = 1829 || n = 1831)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1824)
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
.elseif((n .mod 9 = 3) && ((n > 1802 && n < 1831)))
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
.elseif(n = 1805 || n = 1807 || n = 1813)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1808 || n = 1809 || n = 1817)
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
.elseif(n = 1811)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1815)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1816)
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
.elseif(n = 1818 || n = 1826 || n = 1834)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1819)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1820 || n = 1828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1825)
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
.elseif(n = 1833)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1860 || n = 1872)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
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
.elseif(n = 1838)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1838 && n < 1876)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1840 || n = 1851 || n = 1861)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841 || n = 1845 || n = 1854 || n = 1859 || n = 1862 || n = 1867)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1842 || n = 1858 || n = 1874)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1844 || n = 1852 || n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 1) && ((n > 1845 && n < 1874)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1847)
	; 11 bytes
	pha
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	tax
	pla
.elseif(n = 1849 || n = 1871)
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
.elseif(n = 1856)
	; 11 bytes
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1863)
	; 12 bytes
	pha
	sec
	lda #(n-27)/17
	jsr @nearby_rts
	sbc #1
	Jpl *-5
	pla
.elseif(n = 1865)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1869)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 1870)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
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
.if(n = 1876 || n = 1892 || n = 1900)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1877 || n = 1878 || n = 1885 || n = 1886 || n = 1895 || n = 1913)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1887 || n = 1905)
	; 12 bytes
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
.elseif(n = 1881)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	nop
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 8 = 2) && ((n > 1881 && n < 1915)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1883)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1883 && n < 1912)))
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
.elseif(n = 1889 || n = 1907)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 1891 || n = 1909)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1894)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1896 || n = 1908)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1897)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 1899)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1901)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1903)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $ACA0'
	ldy #172
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1904)
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
.elseif(n = 1910)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1912)
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
; 1915 to 1953
.if(n = 1915 || n = 1926)
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
.elseif(n = 1916 || n = 1924 || n = 1940 || n = 1948)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1917)
	; 11 bytes
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	adc #1
	Jne *-5
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
.elseif(n = 1919)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1919 && n < 1948)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1921 || n = 1923 || n = 1939 || n = 1951)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1922 || n = 1930 || n = 1946)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1931 || n = 1949 || n = 1950)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1932 || n = 1944)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1933)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 1937)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1941)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	brk
	adc #1
	Jpl *-6
	pla
.elseif(n = 1942)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1945)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1952)
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
.elseif(n = 1953)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if((n .mod 8 = 2) && ((n > 1953 && n < 1987)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1955)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1955 && n < 1993)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1957)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 1958)
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
.elseif(n = 1963 || n = 1990)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1964 || n = 1972 || n = 1988)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1966)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1967 || n = 1975 || n = 1976 || n = 1985)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1968 || n = 1980)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1969)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 1973)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1977 || n = 1989)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1982)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
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
.elseif(n = 1987)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 1998 || n = 2006 || n = 2011 || n = 2021 || n = 2029 || n = 2030)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1994 || n = 2002 || n = 2018 || n = 2026)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1995 || n = 1997 || n = 2005 || n = 2007 || n = 2015 || n = 2031)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1996 || n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1999 || n = 2008 || n = 2017)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2000)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2000 && n < 2029)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 2004 || n = 2016)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2009)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2013)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2014)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2023)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2024)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
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
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032)
	; 11 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2033)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 2035 || n = 2053 || n = 2062)
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
.elseif((n .mod 9 = 3) && ((n > 2036 && n < 2065)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2038)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 5) && ((n > 2038 && n < 2067)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2040 || n = 2052)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2041)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	pla
.elseif(n = 2042 || n = 2050)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 2043)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jmi *-5
	tay
	pla
.elseif(n = 2045)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2047 || n = 2069)
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
.elseif(n = 2049 || n = 2061)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2051 || n = 2059)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2054)
	; 11 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2056 || n = 2070)
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
.elseif(n = 2063)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
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
.elseif(n = 2067)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
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
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2080 || n = 2107)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2072)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2072 && n < 2110)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2074 || n = 2086)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2075 || n = 2078 || n = 2083 || n = 2093)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2076 || n = 2088)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2077 || n = 2085 || n = 2092 || n = 2094 || n = 2101)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2079)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2081)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2084)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2087 || n = 2097)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 2090)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2095)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	sta (@ptrtemp,x)
	tax
	pla
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
.elseif(n = 2099)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2102)
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
.elseif(n = 2103 || n = 2105)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2104)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 2108)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2122 || n = 2146)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2111 || n = 2120 || n = 2128 || n = 2129 || n = 2137 || n = 2138 || n = 2147)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2112 || n = 2124 || n = 2148)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2113)
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
.elseif(n = 2115 || n = 2119 || n = 2139)
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
.elseif(n = 2117)
	; 11 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	tax
	pla
.elseif((n .mod 9 = 3) && ((n > 2117 && n < 2146)))
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
.elseif(n = 2125 || n = 2134 || n = 2143)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2126)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2130)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2131)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2133)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2135)
	; 11 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2140)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif(n = 2144)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2151 || n = 2175 || n = 2185 || n = 2187)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2150)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jmi *-4
	tay
	pla
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2153)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 2153 && n < 2182)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2155 || n = 2166 || n = 2177)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2156 || n = 2164 || n = 2165 || n = 2173 || n = 2174 || n = 2183 || n = 2186)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2157 || n = 2168)
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
.elseif(n = 2158 || n = 2182)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2159 || n = 2176)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2160 || n = 2184)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2162)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
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
.elseif(n = 2169)
	; 12 bytes
	pha
	sec
	lda #(n-27)/17
	jsr @nearby_rts
	sbc #1
	Jpl *-5
	pla
.elseif(n = 2171)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
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
.elseif(n = 2180)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
	pla
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
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2189)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
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
.elseif(n = 2191 || n = 2192 || n = 2203 || n = 2213 || n = 2219 || n = 2222)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193 || n = 2204 || n = 2214)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2194 || n = 2218)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2195)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2198)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
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
.elseif(n = 2205)
	; 12 bytes
	pha
	tya
	ldy #128
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2207)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
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
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
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
.elseif(n = 2225)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2228 || n = 2237 || n = 2246 || n = 2255 || n = 2258 || n = 2263 || n = 2264)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229 || n = 2257 || n = 2259)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2234)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
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
.elseif(n = 2239)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
.elseif(n = 2241 || n = 2248)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 2245)
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
.elseif(n = 2275 || n = 2285 || n = 2286)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2277)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
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
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
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
.elseif(n = 2308)
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
.elseif(n = 2311 || n = 2322)
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
.elseif(n = 2312)
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
.elseif(n = 2313 || n = 2323)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2319)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2321)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2325 || n = 2327)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
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
.elseif(n = 2329 || n = 2335)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2332)
	; 11 bytes
	pha
	sec
	lda #128
	brk
	sbc #1
	Jpl *-4
	pla
.elseif(n = 2333)
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
.elseif(n = 2334)
	; 11 bytes
	pha
	tya
	ldy #128
	brk
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 2336)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif(n = 2339)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
	pla
.elseif(n = 2341)
	; 12 bytes
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	nop
	adc #1
	Jne *-6
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
.elseif(n = 2343)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	nop
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if((n .mod 11 = 1) && ((n > 2343 && n < 2378)))
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
.elseif(n = 2345 || n = 2367 || n = 2371 || n = 2379)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2346 || n = 2347 || n = 2349 || n = 2357 || n = 2359 || n = 2369 || n = 2381)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 2352 || n = 2364 || n = 2376)
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
.elseif(n = 2354)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2356)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2358)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2360 || n = 2380)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2361)
	; 12 bytes
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2362 || n = 2374)
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
.elseif(n = 2365)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2368)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2370)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2372 || n = 2382)
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
.elseif(n = 2373)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
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
.elseif(n = 2378)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2383 to 2421
.if((n > 2382 && n < 2386) || (n > 2392 && n < 2397) || n = 2405 || n = 2411 || n = 2416 || n = 2417)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2386 || n = 2398 || n = 2410)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2388 || n = 2400 || n = 2412)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2389 || n = 2391 || n = 2401 || n = 2403 || n = 2413 || n = 2415)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2390 || n = 2402 || n = 2414)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2392 || n = 2420)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2397 || n = 2408 || n = 2419)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2399 || n = 2421)
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
.elseif(n = 2404)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2406)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2407)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2409)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2418)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.endif
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2458 || n = 2470 || n = 2482 || n = 2494)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2423 || n = 2433 || n = 2435 || n = 2447 || (n > 2454 && n < 2458) || (n > 2465 && n < 2470) || n = 2471 || n = 2491 || n = 2493 || n = 2495)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif((n .mod 12 = 0) && ((n > 2423 && n < 2497)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2425 || n = 2427 || n = 2437 || n = 2439 || n = 2445 || n = 2449 || n = 2451 || n = 2461 || n = 2473 || n = 2479 || n = 2481 || n = 2499)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2426 || n = 2438 || n = 2444 || n = 2450 || n = 2474 || n = 2486)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2428)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
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
.elseif(n = 2430 || n = 2480 || n = 2500)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2466) || n = 2487 || n = 2498))
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
.elseif(n = 2440)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2441 || n = 2463 || n = 2485)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2442 || n = 2478)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 2452 || n = 2492)
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
.elseif(n = 2453 || n = 2483)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2459 || n = 2489)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 2462 || n = 2490)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2464)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2476)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
	pla
.elseif(n = 2477)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2488)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2497)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	iny
	Jne *-5
	tay
	pla
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
.if(n = 2501 || n = 2503 || n = 2505 || n = 2513 || n = 2524 || n = 2525 || n = 2527 || (n > 2535 && n < 2540) || n = 2549 || n = 2555 || n = 2567 || n = 2576)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2502 || n = 2552)
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
.elseif(n = 2504 || n = 2546 || n = 2574)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2505 && n < 2579)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2507 || n = 2529 || n = 2573)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 0) && ((n > 2507 && n < 2569)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2509 || n = 2531 || n = 2553 || n = 2564 || n = 2575)
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
.elseif(n = 2510 || n = 2522 || n = 2534 || n = 2558 || n = 2570)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2511 || n = 2528 || n = 2545)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2512 || n = 2548)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2514 || n = 2550)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2515 || n = 2517 || n = 2521 || n = 2523 || n = 2533 || n = 2535 || n = 2547 || n = 2557 || n = 2559 || n = 2569 || n = 2571)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2516)
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
.elseif(n = 2526 || n = 2543 || n = 2577)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2540)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2541)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2561)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	iny
	iny
	Jne *-4
	tay
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
.elseif(n = 2563)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2565)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
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
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2579 && n < 2617)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2581 || n = 2583 || n = 2587 || n = 2589 || n = 2593 || n = 2605)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2585)
	; 12 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	tax
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
.elseif(n = 2588)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2590 || n = 2602 || n = 2614)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2591 || n = 2598 || n = 2599 || n = 2601 || n = 2603 || n = 2609 || n = 2615)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2594 || n = 2606)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2595 || n = 2617)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif(n = 2596)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2597 || n = 2608)
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
.elseif(n = 2600)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2607)
	; 12 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	nop
	dey
	Jne *-5
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
.elseif(n = 2611)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2612)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
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
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2624 || n = 2642 || n = 2654)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619 || n = 2630 || n = 2641)
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
.elseif(n = 2620 || n = 2656)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2621 || n = 2627 || (n > 2632 && n < 2638) || n = 2646)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2622)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2623 || n = 2625 || n = 2631 || n = 2643 || n = 2653 || n = 2655)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2628 || n = 2640 || n = 2652)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2629)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
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
.elseif(n = 2644)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2645)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2647)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
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
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2668 || n = 2669 || n = 2671 || n = 2675 || n = 2680 || n = 2682 || n = 2687 || n = 2693 || n = 2699 || n = 2704 || n = 2708 || n = 2709 || n = 2711 || (n > 2718 && n < 2722) || n = 2723)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2658 || n = 2694 || n = 2730)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2659 || n = 2665 || n = 2667 || n = 2677 || n = 2689 || n = 2691 || n = 2697 || n = 2701 || n = 2703 || n = 2725 || n = 2731 || n = 2733)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2660 || n = 2666 || n = 2678 || n = 2690 || n = 2702 || n = 2726)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2661 || n = 2683 || n = 2705 || n = 2716 || n = 2727)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 10) && ((n > 2661 && n < 2735)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 11 = 1) && (n = 2663 || (n > 2684 && n < 2730)))
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
.elseif((n .mod 12 = 0) && ((n > 2663 && n < 2725)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2670 || n = 2684)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2672 || n = 2714)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2673)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2679 || n = 2713)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2681 || n = 2715 || n = 2732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2692 || n = 2728)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2695)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2706)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2717)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
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
.elseif((n .mod 12 = 0) && ((n > 2735 && n < 2773)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2737 || n = 2753 || n = 2763 || n = 2767 || n = 2769)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2738)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2739)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	nop
	iny
	Jne *-5
	tay
	pla
.elseif((n .mod 11 = 1) && ((n > 2739 && n < 2774)))
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
.elseif(n = 2741 || n = 2743 || n = 2744 || n = 2755 || n = 2757 || n = 2765)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2742 || n = 2756)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2745)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2746 || n = 2758 || n = 2770)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2750)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	brk
	Jne *-4
	tay
	pla
.elseif(n = 2752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2754 || n = 2768)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 2764)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2766)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2786)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2775 || n = 2785 || n = 2787 || n = 2797 || n = 2799 || n = 2803 || n = 2809 || n = 2811)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2776 || n = 2780 || n = 2788 || n = 2789 || n = 2791 || n = 2801 || n = 2807)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2777)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dey
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
.elseif(n = 2781)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2782 || n = 2794 || n = 2806)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2784 || n = 2796 || n = 2808)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2790)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
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
.elseif(n = 2795)
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
.elseif(n = 2798 || n = 2812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2800)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2802)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if(n = 2813 || n = 2823 || n = 2833 || n = 2835 || n = 2839 || n = 2841 || n = 2845 || n = 2847 || n = 2855 || n = 2857 || n = 2859 || n = 2869 || n = 2871 || n = 2875 || n = 2877 || n = 2881 || n = 2893 || n = 2895 || n = 2905 || n = 2907 || n = 2911 || n = 2913 || n = 2929 || n = 2931 || n = 2941 || n = 2943 || n = 2947 || n = 2949 || n = 2955 || n = 2965 || n = 2967)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2825 || n = 2829 || n = 2831 || n = 2837 || n = 2843 || n = 2848 || n = 2853 || (n > 2859 && n < 2866) || n = 2867 || n = 2873 || n = 2879 || n = 2884 || (n > 2885 && n < 2890) || n = 2891 || (n > 2896 && n < 2900) || n = 2901 || n = 2903 || n = 2909 || n = 2915 || n = 2920 || n = 2921 || n = 2923 || n = 2925 || n = 2927 || n = 2933 || n = 2935 || n = 2937 || n = 2939 || n = 2945 || (n > 2955 && n < 2961) || n = 2963)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif((n .mod 12 = 10) && ((n > 2817 && n < 2963)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 12 = 0) && ((n > 2819 && n < 2965)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2821)
	; 12 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2822 || n = 2834 || n = 2846 || n = 2858 || n = 2870 || n = 2876 || n = 2894 || n = 2906 || n = 2918 || n = 2930 || n = 2942 || n = 2948 || n = 2954)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2824 || n = 2922 || n = 2936)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2826 || n = 2840 || n = 2882 || n = 2896 || n = 2924 || n = 2966)
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
.elseif(n = 2836 || n = 2872 || n = 2908 || n = 2944)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 2838 || n = 2874 || n = 2910 || n = 2946)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2849 || n = 2883 || n = 2900 || n = 2917 || n = 2934 || n = 2968)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2850)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 2851 || n = 2885 || n = 2919 || n = 2953)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
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
.elseif(n = 2912 || n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
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
.elseif(n = 2961)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 2969 to 3125
.if(n = 2969 || n = 2971 || n = 2973 || n = 2975 || n = 2993 || (n > 2994 && n < 2998) || n = 2999 || n = 3005 || n = 3007 || n = 3009 || n = 3011 || n = 3017 || n = 3023 || n = 3028 || n = 3029 || n = 3031 || n = 3033 || n = 3035 || (n > 3039 && n < 3046) || n = 3047 || n = 3059 || (n > 3064 && n < 3070) || n = 3071 || n = 3077 || (n > 3078 && n < 3082) || n = 3083 || (n > 3093 && n < 3104) || n = 3105 || n = 3107 || (n > 3112 && n < 3118) || n = 3119 || n = 3122 || n = 3125)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2970 || n = 2987 || n = 3004 || n = 3021 || n = 3089 || n = 3123)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2972 || n = 3032 || n = 3112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2973 && n < 3071)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 12 = 0) && ((n > 2975 && n < 3073)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2977 || n = 2979 || n = 2983 || n = 2989 || n = 2991 || n = 3001 || n = 3003 || n = 3015 || n = 3025 || n = 3027 || n = 3037 || n = 3039 || n = 3049 || n = 3051 || n = 3057 || n = 3061 || n = 3063 || n = 3073 || n = 3075 || n = 3085 || n = 3091 || n = 3093 || n = 3109 || n = 3111)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2978 || n = 2984 || n = 2990 || n = 3002 || n = 3014 || n = 3026 || n = 3038 || n = 3056 || n = 3062 || n = 3074 || n = 3086 || n = 3110)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2980 || n = 3016 || n = 3052 || n = 3088 || n = 3106 || n = 3124)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 2982 || n = 3018 || n = 3054 || n = 3090 || n = 3108)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2985 || n = 3019 || n = 3053 || n = 3087 || n = 3121)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2992 || n = 3006 || n = 3020 || n = 3076 || n = 3104 || n = 3118)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2994 || n = 3008 || n = 3050 || n = 3064 || n = 3078 || n = 3092 || n = 3120)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3030)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
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
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3144 || n = 3162 || n = 3180 || n = 3198 || n = 3216 || n = 3234 || n = 3252 || n = 3270)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3127 || n = 3129 || n = 3145 || n = 3147 || n = 3163 || n = 3165 || n = 3181 || n = 3183 || n = 3199 || n = 3201 || n = 3217 || n = 3219 || n = 3235 || n = 3237 || n = 3253 || n = 3255 || n = 3261 || n = 3271 || n = 3273)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3128 || n = 3164 || n = 3182 || n = 3200 || n = 3236 || n = 3254)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3130 || n = 3150 || n = 3170)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 3131 || n = 3133 || (n > 3134 && n < 3138) || n = 3139 || n = 3141 || n = 3149 || n = 3151 || n = 3153 || n = 3154 || n = 3156 || n = 3158 || n = 3159 || n = 3161 || (n > 3165 && n < 3170) || n = 3171 || n = 3173 || n = 3175 || n = 3177 || n = 3179 || (n > 3183 && n < 3188) || n = 3193 || n = 3195 || n = 3197 || n = 3203 || n = 3205 || n = 3207 || n = 3209 || n = 3211 || n = 3213 || n = 3215 || (n > 3219 && n < 3223) || n = 3224 || (n > 3225 && n < 3230) || n = 3231 || n = 3233 || n = 3238 || n = 3239 || n = 3241 || n = 3243 || n = 3245 || (n > 3246 && n < 3250) || n = 3251 || (n > 3261 && n < 3268) || n = 3269 || n = 3275 || (n > 3276 && n < 3282))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3132 || n = 3146 || n = 3174 || n = 3188 || n = 3202 || n = 3230 || n = 3244 || n = 3272)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && (n = 3134 || n = 3148 || (n > 3175 && n < 3219) || (n > 3245 && n < 3275)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3138 || n = 3155 || n = 3172 || n = 3189 || n = 3206 || n = 3223 || n = 3240 || n = 3257)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3140 || n = 3157 || n = 3191 || n = 3208 || n = 3225 || n = 3242 || n = 3259 || n = 3276)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3142 || n = 3160 || n = 3178 || n = 3196 || n = 3214 || n = 3232 || n = 3250 || n = 3268)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 3143)
	; 12 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3152 || n = 3192 || n = 3212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
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
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3283 || n = 3285 || n = 3294 || n = 3295 || (n > 3296 && n < 3300) || n = 3301 || n = 3303 || n = 3305 || n = 3313 || n = 3315 || (n > 3316 && n < 3322) || n = 3323 || n = 3329 || n = 3331 || (n > 3332 && n < 3340) || n = 3341 || (n > 3345 && n < 3350) || n = 3351 || (n > 3352 && n < 3356) || n = 3357 || (n > 3363 && n < 3370) || n = 3371 || (n > 3372 && n < 3376) || n = 3377 || n = 3382 || n = 3383 || n = 3385 || (n > 3386 && n < 3390) || n = 3391 || n = 3401 || (n > 3402 && n < 3410) || n = 3411 || (n > 3418 && n < 3426) || n = 3436 || n = 3437)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
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
.elseif(n = 3287 || n = 3289 || n = 3307 || n = 3343 || n = 3345 || n = 3363 || n = 3379 || n = 3381 || n = 3397 || n = 3399 || n = 3415 || n = 3417 || n = 3433)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3288 || n = 3306 || n = 3324 || n = 3342 || n = 3360 || n = 3378 || n = 3396 || n = 3414 || n = 3432)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3290 || n = 3362 || n = 3380)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3291 || n = 3308 || n = 3325 || n = 3359 || n = 3393 || n = 3410 || n = 3427)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3292 || n = 3312 || n = 3332 || n = 3352 || n = 3392)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3293 || n = 3310 || n = 3327 || n = 3361 || n = 3395 || n = 3429)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
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
.elseif((n .mod 14 = 10) && ((n > 3299 && n < 3329) || (n > 3355 && n < 3399) || n = 3426))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3301 && n < 3345) || (n > 3371 && n < 3401) || n = 3428))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3304 || n = 3322 || n = 3340 || n = 3358 || n = 3376 || n = 3394 || n = 3412 || n = 3430)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 3350 || n = 3390)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
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
.elseif(n = 3413)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
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
.elseif(n = 3435)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 3438 to 3750
.if(n = 3438 || n = 3439 || n = 3441 || n = 3443 || n = 3445 || n = 3447 || n = 3449 || n = 3455 || (n > 3456 && n < 3461) || n = 3462 || n = 3465 || n = 3467 || (n > 3472 && n < 3478) || n = 3479 || n = 3481 || n = 3483 || n = 3485 || n = 3491 || n = 3493 || n = 3494 || (n > 3498 && n < 3502) || n = 3503 || n = 3508 || n = 3509 || n = 3511 || n = 3513 || (n > 3514 && n < 3520) || n = 3521 || n = 3527 || n = 3528 || (n > 3532 && n < 3538) || n = 3539 || n = 3544 || n = 3545 || n = 3547 || n = 3549 || n = 3551 || n = 3553 || n = 3555 || n = 3557 || n = 3562 || n = 3564 || n = 3567 || n = 3569 || n = 3571 || n = 3573 || n = 3575 || n = 3581 || (n > 3582 && n < 3590) || n = 3591 || n = 3593 || n = 3598 || (n > 3599 && n < 3610) || n = 3611 || (n > 3616 && n < 3628) || n = 3629 || (n > 3633 && n < 3646) || n = 3647 || (n > 3652 && n < 3664) || (n > 3672 && n < 3677) || (n > 3677 && n < 3682) || n = 3683 || n = 3689 || (n > 3693 && n < 3699) || (n > 3706 && n < 3710) || n = 3711 || (n > 3712 && n < 3716) || n = 3717 || n = 3719 || (n > 3723 && n < 3730) || n = 3731 || n = 3734 || n = 3737 || (n > 3741 && n < 3750))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif((n .mod 14 = 10) && (n = 3440 || n = 3454 || (n > 3481 && n < 3525) || (n > 3551 && n < 3581)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3441 && n < 3471) || (n > 3497 && n < 3527) || (n > 3553 && n < 3583)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3444 || n = 3478 || n = 3495 || n = 3529 || n = 3546 || n = 3563 || n = 3597 || n = 3614 || n = 3631 || n = 3665 || n = 3699 || n = 3716 || n = 3733 || n = 3750)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 3448 || n = 3466 || n = 3484 || n = 3502 || n = 3520 || n = 3538 || n = 3556 || n = 3574 || n = 3592 || n = 3610 || n = 3628 || n = 3646 || n = 3682 || n = 3700 || n = 3718 || n = 3736)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 3450 || n = 3468 || n = 3486 || n = 3504 || n = 3522 || n = 3540 || n = 3558 || n = 3576 || n = 3594 || n = 3612 || n = 3630 || n = 3648 || n = 3666 || n = 3684 || n = 3702 || n = 3720 || n = 3738)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3451 || n = 3453 || n = 3464 || n = 3469 || n = 3471 || n = 3487 || n = 3489 || n = 3505 || n = 3507 || n = 3523 || n = 3525 || n = 3541 || n = 3543 || n = 3559 || n = 3561 || n = 3577 || n = 3579 || n = 3595 || n = 3613 || n = 3615 || n = 3649 || n = 3651 || n = 3667 || n = 3669 || n = 3685 || n = 3687 || n = 3691 || n = 3693 || n = 3703 || n = 3705 || n = 3721 || n = 3723 || n = 3739 || n = 3741)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3452 || n = 3472 || n = 3492 || n = 3532 || n = 3572 || n = 3632 || n = 3652 || n = 3672 || n = 3692 || n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3461)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
.elseif(n = 3463 || n = 3480 || n = 3497 || n = 3514 || n = 3531 || n = 3548 || n = 3565 || n = 3599 || n = 3616 || n = 3633 || n = 3650 || n = 3701 || n = 3735)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3488 || n = 3542 || n = 3560 || n = 3578 || n = 3668 || n = 3686 || n = 3704 || n = 3740)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3490 || n = 3530 || n = 3550 || n = 3570 || n = 3590 || n = 3670 || n = 3710 || n = 3730)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 3506)
	; 12 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
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
.elseif(n = 3664)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
	pla
.elseif(n = 3671)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3677)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3688)
	; 11 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 3690)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	brk
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
.endif
.else
.if((n >= 3751) && (n < 4376))
; 3751 to 4375
.if(n = 3751 || n = 3753 || n = 3755 || (n > 3759 && n < 3767) || n = 3768 || n = 3771 || n = 3773 || (n > 3777 && n < 3784) || n = 3785 || (n > 3786 && n < 3790) || n = 3791 || (n > 3795 && n < 3801) || n = 3802 || (n > 3803 && n < 3808) || n = 3809 || (n > 3813 && n < 3818) || n = 3819 || (n > 3820 && n < 3826) || n = 3827 || n = 3833 || n = 3834 || n = 3836 || (n > 3837 && n < 3841) || n = 3842 || n = 3843 || n = 3845 || n = 3851 || n = 3853 || (n > 3854 && n < 3862) || n = 3863 || n = 3868 || (n > 3872 && n < 3880) || n = 3881 || n = 3887 || n = 3889 || n = 3891 || (n > 3892 && n < 3898) || n = 3899 || n = 3904 || (n > 3905 && n < 3910) || n = 3911 || (n > 3912 && n < 3916) || n = 3917 || (n > 3922 && n < 3930) || n = 3931 || n = 3933 || n = 3935 || (n > 3939 && n < 3948) || n = 3949 || n = 3951 || n = 3953 || (n > 3957 && n < 3970) || (n > 3975 && n < 3988) || n = 3989 || (n > 3993 && n < 4005) || (n > 4013 && n < 4021) || n = 4023 || n = 4025 || n = 4031 || (n > 4032 && n < 4039) || n = 4040 || n = 4043 || n = 4048 || n = 4049 || n = 4051 || (n > 4052 && n < 4056) || n = 4057 || n = 4059 || n = 4061 || (n > 4065 && n < 4070) || n = 4071 || n = 4074 || n = 4076 || n = 4077 || n = 4079 || (n > 4083 && n < 4090) || n = 4091 || (n > 4092 && n < 4096) || n = 4097 || (n > 4101 && n < 4106) || n = 4108 || n = 4111 || n = 4113 || n = 4115 || (n > 4119 && n < 4124) || n = 4125 || n = 4128 || n = 4129 || n = 4131 || n = 4133 || (n > 4137 && n < 4141) || n = 4142 || (n > 4143 && n < 4148) || n = 4149 || n = 4151 || n = 4156 || n = 4157 || n = 4159 || (n > 4160 && n < 4168) || n = 4174 || n = 4176 || (n > 4177 && n < 4186) || n = 4187 || n = 4193 || (n > 4194 && n < 4204) || n = 4205 || (n > 4212 && n < 4222) || n = 4223 || n = 4229 || n = 4231 || (n > 4232 && n < 4240) || n = 4241 || (n > 4245 && n < 4250) || n = 4251 || (n > 4252 && n < 4257) || n = 4259 || (n > 4263 && n < 4270) || n = 4271 || (n > 4272 && n < 4276) || (n > 4281 && n < 4290) || n = 4291 || n = 4293 || n = 4295 || (n > 4299 && n < 4310) || (n > 4317 && n < 4328) || n = 4329 || n = 4331 || (n > 4335 && n < 4345) || n = 4346 || n = 4349 || (n > 4353 && n < 4362) || n = 4363 || n = 4365 || n = 4367 || (n > 4372 && n < 4376))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 3754 || n = 3772 || n = 3790 || n = 3808 || n = 3826 || n = 3844 || n = 3862 || n = 3880 || n = 3898 || n = 3916 || n = 3934 || n = 3952 || n = 3970 || n = 3988 || n = 4006 || n = 4024 || n = 4042 || n = 4060 || n = 4078 || n = 4096 || n = 4114 || n = 4132 || n = 4150 || n = 4168 || n = 4186 || n = 4204 || n = 4222 || n = 4240 || n = 4258 || n = 4276 || n = 4294 || n = 4312 || n = 4330 || n = 4348 || n = 4366)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 3756 || n = 3774 || n = 3792 || n = 3810 || n = 3828 || n = 3846 || n = 3864 || n = 3882 || n = 3900 || n = 3918 || n = 3936 || n = 3954 || n = 3972 || n = 3990 || n = 4008 || n = 4026 || n = 4044 || n = 4062 || n = 4080 || n = 4098 || n = 4116 || n = 4134 || n = 4152 || n = 4170 || n = 4188 || n = 4206 || n = 4224 || n = 4242 || n = 4260 || n = 4278 || n = 4296 || n = 4314 || n = 4332 || n = 4350 || n = 4368)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3757 || n = 3759 || n = 3775 || n = 3777 || n = 3793 || n = 3795 || n = 3811 || n = 3813 || n = 3829 || n = 3831 || n = 3847 || n = 3849 || n = 3865 || n = 3883 || n = 3885 || n = 3901 || n = 3919 || n = 3921 || n = 3955 || n = 3957 || n = 3975 || n = 3991 || n = 3993 || n = 4009 || n = 4011 || n = 4013 || n = 4027 || n = 4029 || n = 4045 || n = 4047 || n = 4063 || n = 4065 || n = 4081 || n = 4083 || n = 4099 || n = 4101 || n = 4117 || n = 4119 || n = 4135 || n = 4153 || n = 4155 || n = 4171 || n = 4173 || n = 4189 || n = 4191 || n = 4207 || n = 4225 || n = 4227 || n = 4261 || n = 4263 || n = 4281 || n = 4297 || n = 4299 || n = 4315 || n = 4317 || n = 4333 || n = 4335 || n = 4351 || n = 4353 || n = 4369 || n = 4371)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3758 || n = 3776 || n = 3794 || n = 3830 || n = 3848 || n = 3866 || n = 3884 || n = 3902 || n = 3938 || n = 3974 || n = 4028 || n = 4046 || n = 4064 || n = 4082 || n = 4100 || n = 4118 || n = 4136 || n = 4154 || n = 4190 || n = 4208 || n = 4244 || n = 4280 || n = 4298 || n = 4316 || n = 4334 || n = 4370)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3767 || n = 3784 || n = 3801 || n = 3818 || n = 3835 || n = 3852 || n = 3869 || n = 3886 || n = 3903 || n = 3920 || n = 3937 || n = 3971 || n = 4005 || n = 4022 || n = 4039 || n = 4056 || n = 4073 || n = 4090 || n = 4107 || n = 4124 || n = 4141 || n = 4158 || n = 4175 || n = 4192 || n = 4209 || n = 4226 || n = 4243 || n = 4277 || n = 4311 || n = 4328 || n = 4345)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3769 || n = 3786 || n = 3803 || n = 3820 || n = 3837 || n = 3854 || n = 3871 || n = 3888 || n = 3905 || n = 3922 || n = 3939 || n = 3956 || n = 3973 || n = 4007 || n = 4041 || n = 4058 || n = 4075 || n = 4092 || n = 4109 || n = 4126 || n = 4143 || n = 4160 || n = 4177 || n = 4194 || n = 4211 || n = 4228 || n = 4245 || n = 4262 || n = 4279 || n = 4313 || n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3770 || n = 3850 || n = 3870 || n = 3890 || n = 3910 || n = 3930 || n = 3950 || n = 4030 || n = 4050 || n = 4110 || n = 4130 || n = 4210 || n = 4230 || n = 4250 || n = 4270 || n = 4290 || n = 4310)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 3812 || n = 3832 || n = 3872 || n = 3892 || n = 3912 || n = 3932 || n = 3992 || n = 4012 || n = 4032 || n = 4052 || n = 4072 || n = 4112 || n = 4172 || n = 4212 || n = 4232 || n = 4252 || n = 4272 || n = 4292 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3841)
	; 12 bytes
	pha
	tya
	ldy #192 ;hides 'cpy #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
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
.elseif(n = 3948)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 4010)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	brk
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4021)
	; 12 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4070)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
	tax
	pla
.elseif(n = 4106)
	; 12 bytes
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4127)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4137)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4148)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4169)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4257)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
	tay
	pla
.elseif(n = 4362)
	; 12 bytes
	pha
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 4364)
	; 12 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if((n > 4375 && n < 4384) || n = 4385 || n = 4391 || (n > 4392 && n < 4402) || n = 4403 || n = 4408 || n = 4409 || n = 4411 || (n > 4412 && n < 4416) || n = 4417 || n = 4419 || n = 4421 || (n > 4425 && n < 4430) || n = 4431 || (n > 4432 && n < 4438) || n = 4439 || (n > 4443 && n < 4450) || n = 4451 || n = 4453 || n = 4455 || n = 4457 || (n > 4461 && n < 4470) || n = 4471 || n = 4473 || n = 4475 || n = 4480 || (n > 4481 && n < 4490) || n = 4491 || n = 4493 || (n > 4497 && n < 4510) || n = 4511 || (n > 4515 && n < 4528) || n = 4529)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4384 || n = 4402 || n = 4420 || n = 4438 || n = 4456 || n = 4474 || n = 4492 || n = 4510 || n = 4528)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 4386 || n = 4404 || n = 4422 || n = 4440 || n = 4458 || n = 4476 || n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4387 || n = 4389 || n = 4405 || n = 4407 || n = 4423 || n = 4425 || n = 4441 || n = 4443 || n = 4459 || n = 4461 || n = 4477 || n = 4479 || n = 4495 || n = 4497 || n = 4513 || n = 4515 || n = 4531)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4388 || n = 4406 || n = 4424 || n = 4442 || n = 4460 || n = 4478 || n = 4496 || n = 4514)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4390 || n = 4430 || n = 4450 || n = 4470 || n = 4490)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 4392 || n = 4412 || n = 4432 || n = 4452 || n = 4472)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4410)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4416)
	; 12 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
.elseif(n = 4418)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4454)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
.elseif(n = 4481)
	; 12 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4532 to 4687
.if(n = 4532 || n = 4552 || n = 4572 || n = 4592 || n = 4632 || n = 4652 || n = 4672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4533 || n = 4549 || n = 4551 || n = 4567 || n = 4569 || n = 4585 || n = 4587 || n = 4594 || n = 4603 || n = 4605 || n = 4610 || n = 4612 || n = 4621 || n = 4623 || n = 4651 || n = 4653)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif((n > 4533 && n < 4546) || n = 4547 || (n > 4552 && n < 4564) || n = 4565 || n = 4571 || (n > 4572 && n < 4582) || n = 4583 || n = 4588 || n = 4589 || n = 4599 || n = 4601 || n = 4606 || n = 4608 || n = 4617 || n = 4619 || (n > 4623 && n < 4629) || (n > 4632 && n < 4648) || n = 4649 || (n > 4653 && n < 4670) || n = 4671 || n = 4673 || (n > 4674 && n < 4685) || n = 4686 || n = 4687)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4546 || n = 4564 || n = 4582 || n = 4600)
	; 11 bytes
	pha
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 4548 || n = 4566 || n = 4584 || n = 4602)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4550 || n = 4568 || n = 4586 || n = 4593 || n = 4595 || n = 4596 || n = 4604 || n = 4611 || n = 4613 || n = 4622)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4570 || n = 4590 || n = 4630 || n = 4670)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 4591)
	; 10 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 4597)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4598 || n = 4614 || n = 4616)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4607)
	; 10 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 4609)
	; 10 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4615)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4618)
	; 11 bytes
	pha
	sec
	lda #0
	brk
	sbc #1
	Jne *-4
	pla
.elseif(n = 4620)
	; 11 bytes
	pha
	tya
	ldy #0
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4629)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	brk
	adc #1
	Jne *-6
	pla
.elseif(n = 4631)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4648)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	brk
	adc #1
	Jne *-5
	pla
.elseif(n = 4650)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4674)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4685)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 4688 to 5000
.if(n = 4688)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 4689 || n = 4694 || n = 4695 || (n > 4696 && n < 4710) || n = 4711 || (n > 4712 && n < 4718) || (n > 4718 && n < 4730) || n = 4731 || (n > 4732 && n < 4750) || n = 4751 || (n > 4752 && n < 4770) || n = 4771 || (n > 4772 && n < 4778) || (n > 4778 && n < 4788) || n = 4789 || n = 4791 || (n > 4792 && n < 4799) || (n > 4799 && n < 4810) || n = 4811 || (n > 4812 && n < 4820) || (n > 4820 && n < 4830) || n = 4831 || (n > 4832 && n < 4850) || n = 4851 || (n > 4852 && n < 4870) || n = 4871 || (n > 4872 && n < 4890) || n = 4891 || (n > 4892 && n < 4910) || n = 4911 || (n > 4912 && n < 4930) || n = 4931 || (n > 4932 && n < 4950) || n = 4951 || (n > 4952 && n < 4970) || n = 4971 || (n > 4972 && n < 4990) || n = 4991 || (n > 4992 && n < 5001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4690)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4691 || n = 4693)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4692 || n = 4712 || n = 4732 || n = 4752 || n = 4772 || n = 4792 || n = 4812 || n = 4832 || n = 4852 || n = 4872 || n = 4892 || n = 4912 || n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4696)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4710 || n = 4730 || n = 4750 || n = 4770 || n = 4790 || n = 4810 || n = 4830 || n = 4850 || n = 4870 || n = 4890 || n = 4910 || n = 4930 || n = 4950 || n = 4970 || n = 4990)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 4718)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4778)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4788)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4799)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4820)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	brk
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
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
; 5001 to 5625
.if((n > 5000 && n < 5010) || n = 5011 || (n > 5012 && n < 5030) || n = 5031 || (n > 5032 && n < 5050) || n = 5051 || (n > 5052 && n < 5070) || n = 5071 || (n > 5072 && n < 5085) || (n > 5085 && n < 5090) || n = 5091 || (n > 5092 && n < 5101) || n = 5102 || (n > 5104 && n < 5110) || n = 5111 || n = 5113 || (n > 5114 && n < 5118) || (n > 5124 && n < 5130) || n = 5131 || (n > 5132 && n < 5156) || n = 5157 || (n > 5158 && n < 5356) || (n > 5356 && n < 5374) || (n > 5377 && n < 5611) || (n > 5611 && n < 5626))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5010 || n = 5030 || n = 5050 || n = 5070 || n = 5090 || n = 5110)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072 || n = 5092 || n = 5112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5085)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	tax
	pla
.elseif(n = 5101)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.elseif(n = 5103 || n = 5122 || n = 5123)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5104 || n = 5124)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5114)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5118)
	; 11 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
.elseif(n = 5119)
	; 11 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 5120)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
.elseif(n = 5121)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5130)
	; 12 bytes
	pha
	sec
	lda #0
	nop
	brk
	sbc #1
	Jne *-5
	pla
.elseif(n = 5132)
	; 12 bytes
	pha
	tya
	ldy #0
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5156)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	pla
.elseif(n = 5158)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5356)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	cmp $C5
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5374)
	; 12 bytes
	pha
	clc
	lda #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	pla
.elseif(n = 5375)
	; 12 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
.elseif(n = 5376)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5377)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5611)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5629) || (n > 5633 && n < 6251))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5629)
	; 12 bytes
	pha
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
	pla
.elseif(n = 5630)
	; 12 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
.elseif(n = 5631)
	; 12 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	nop
	sbc #1
	Jne *-6
	pla
.elseif(n = 5632)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5633)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6303) || (n > 6303 && n < 6376) || (n > 6376 && n < 6394) || n = 6395 || n = 6397 || n = 6398 || n = 6400 || (n > 6401 && n < 7501))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 6303)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
	pla
.elseif(n = 6376)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
.elseif(n = 6394)
	; 12 bytes
	pha
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
.elseif(n = 6396)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
.elseif(n = 6399)
	; 12 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	sbc #1
	Jne *-6
	pla
.elseif(n = 6401)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7906) || (n > 7906 && n < 7924) || n = 7925 || (n > 7926 && n < 7935) || n = 7936 || (n > 7937 && n < 10001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 7906)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 7924)
	; 12 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	pla
.elseif(n = 7926)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7935)
	; 12 bytes
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	brk
	sbc #1
	Jne *-6
	pla
.elseif(n = 7937)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif

.endif
	rest
.endmacro
