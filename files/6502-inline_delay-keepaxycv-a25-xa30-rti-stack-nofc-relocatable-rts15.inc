; delay_n, delays n cycles with small code
; Will clobber S, Z+N, and D
; Will preserve A, X, Y, C, V, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 36
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 302
; -11 bytes or less when n <= 1300
; -12 bytes or less when n <= 1314
; -13 bytes or less when n <= 1990
; -14 bytes or less when n <= 3100
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=8.9, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.1, max=11
; - When 501 <= n <= 750: min=8, avg=10.1, max=11
; - When 751 <= n <= 1000: min=9, avg=10.1, max=11
; - When 1001 <= n <= 2000: min=8, avg=11.1, max=14
; - When 2001 <= n <= 3000: min=10, avg=12.3, max=14
; - When 3001 <= n <= 5000: min=10, avg=13.1, max=15
; - When 5001 <= n <= 7500: min=11, avg=14.6, max=15
; - When 7501 <= n <= 10000: min=12, avg=14.9, max=15
; - When 10001 <= n <= 20000: min=14, avg=15.0, max=15
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
.elseif(n = 4 || n = 5 || n = 8 || n = 17 || n = 28 || n = 31 || n = 34 || n = 41 || n = 43 || n = 44)
	; 2..8 bytes
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
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 36 || n = 39 || n = 42 || n = 45)
	; 3..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 21 || n = 27 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 40)
	; 8 bytes
	delay_n (n-14)
	delay_n 14
.elseif((n > 46 && n < 52) || (n > 53 && n < 60) || (n > 60 && n < 135) || (n > 135 && n < 157))
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
.elseif(n = 60)
	; 8 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	pla
.elseif(n = 135)
	; 8 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	pla
.endif
.else
; 157 to 312
.if((n > 156 && n < 295) || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 295)
	; 8 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
.elseif(n = 297 || n = 312)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n > 297 && n < 301) || n = 305)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 301)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 302 || n = 307)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 303)
	; 11 bytes
	php
	pha
	php
	lda #255
	jsr delay_a_25_clocks
	plp
	pla
	plp
.elseif(n = 304 || n = 309 || n = 311)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 308)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 310)
	; 10 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
	pla
.endif
.endif
.else
; 313 to 625
.if(n = 313 || n = 320 || n = 355 || n = 369 || n = 376 || n = 411 || n = 439 || n = 446 || n = 453 || n = 488 || n = 495 || n = 516 || n = 544 || n = 565 || n = 579 || n = 586 || n = 600)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 314 || n = 316 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 339 || n = 343 || n = 344 || n = 346 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 361 || n = 364 || n = 366 || n = 374 || n = 378 || n = 379 || n = 381 || n = 384 || n = 386 || n = 389 || n = 391 || n = 394 || n = 399 || n = 401 || n = 413 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 444 || n = 448 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464 || n = 466 || n = 469 || n = 471 || n = 479 || n = 483 || n = 484 || n = 486 || n = 489 || n = 491 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 518 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541 || n = 546 || n = 549 || n = 553 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 571 || n = 574 || n = 581 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 623 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 325 || n = 330 || n = 335 || n = 340 || n = 345 || n = 350 || n = 360 || n = 365 || n = 375 || n = 380 || n = 385 || n = 393 || n = 395 || n = 400 || n = 405 || n = 410 || n = 420 || n = 430 || n = 435 || n = 440 || n = 445 || n = 450 || n = 455 || n = 465 || n = 470 || n = 475 || n = 480 || n = 485 || n = 490 || n = 500 || n = 505 || n = 510 || n = 515 || n = 520 || n = 525 || n = 533 || n = 535 || n = 540 || n = 545 || n = 550 || n = 555 || n = 560 || n = 568 || n = 570 || n = 575 || n = 580 || n = 585 || n = 590 || n = 595 || n = 603 || n = 605 || n = 610 || n = 615 || n = 620 || n = 625)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 316 && n < 328) || n = 352 || n = 357 || n = 372 || n = 392 || n = 407 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467 || n = 477 || n = 487 || n = 492 || n = 502 || (n > 511 && n < 523) || (n > 531 && n < 573) || n = 597 || (n > 606 && n < 618)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 318 || n = 543 || n = 588 || n = 606)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 323 || n = 328 || n = 333 || n = 338 || n = 358 || n = 368 || n = 373 || n = 388 || n = 396 || n = 398 || n = 423 || n = 433 || n = 438 || n = 443 || n = 458 || n = 463 || n = 473 || n = 493 || n = 503 || n = 508 || n = 513 || n = 538 || n = 548 || n = 563 || n = 578 || n = 583 || n = 598 || n = 608 || n = 613 || n = 618)
	; 10..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 5 = 2) && ((n > 331 && n < 348) || n = 362 || n = 367 || (n > 376 && n < 388) || n = 397 || n = 402 || n = 412 || n = 427 || (n > 436 && n < 458) || n = 472 || n = 482 || n = 497 || n = 507 || n = 527 || (n > 576 && n < 593) || n = 602 || n = 622))
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 334 || n = 341 || n = 348 || n = 383 || n = 404 || n = 418 || n = 425 || n = 460 || n = 474 || n = 481 || n = 509 || n = 523 || n = 530 || n = 551 || n = 558 || n = 593 || n = 614 || n = 621)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 336 || n = 371 || n = 406 || n = 441 || n = 476 || n = 511)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 363 || n = 408 || n = 498 || n = 561)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 370)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
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
.elseif(n = 403)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 409)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 415)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 416 || n = 521)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 428)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	tax
	pla
.elseif(n = 468 || n = 576)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
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
.elseif(n = 528)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
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
.endif
.endif
.else
.if((n >= 626) && (n < 938))
; 626 to 937
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 686 || n = 689 || n = 693 || n = 694 || n = 699 || n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 724 || n = 728 || n = 729 || n = 731 || n = 734 || n = 736 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 763 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781 || n = 784 || n = 794 || n = 798 || n = 799 || n = 801 || n = 804 || n = 806 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 833 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 864 || n = 868 || n = 869 || n = 871 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 628 || n = 635 || n = 656 || n = 670 || n = 691 || n = 705 || n = 733 || n = 803 || n = 810 || n = 824 || n = 838 || n = 866 || n = 873 || n = 880 || n = 901 || n = 908 || n = 915)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 630 || n = 636 || n = 640 || n = 645 || n = 650 || n = 655 || n = 660 || n = 665 || n = 673 || n = 675 || n = 680 || n = 685 || n = 690 || n = 698 || n = 700 || n = 710 || n = 715 || n = 720 || n = 725 || n = 730 || n = 735 || n = 741 || n = 745 || n = 750 || n = 755 || n = 760 || n = 765 || n = 770 || n = 780 || n = 785 || n = 790 || n = 795 || n = 800 || n = 805 || n = 815 || n = 820 || n = 825 || n = 830 || n = 835 || n = 840 || n = 850 || n = 855 || n = 860 || n = 865 || n = 870 || n = 875 || n = 885 || n = 890 || n = 895 || n = 900 || n = 905 || n = 920 || n = 925 || n = 930 || n = 935)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 633)
	; 9 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	pla
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
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 938)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 643 || n = 653 || n = 668 || n = 683 || n = 688 || n = 701 || n = 703 || n = 713 || n = 718 || n = 748 || n = 753 || n = 758 || n = 773 || n = 778 || n = 783 || n = 788 || n = 793 || n = 818 || n = 823 || n = 848 || n = 853 || n = 863 || n = 878 || n = 883 || n = 893 || n = 898 || n = 913 || n = 918 || n = 923 || n = 928 || n = 933)
	; 10..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 648 || n = 816 || n = 888)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 649 || n = 663 || n = 684 || n = 719 || n = 726 || n = 740 || n = 754 || n = 761 || n = 768 || n = 775 || n = 789 || n = 796 || n = 831 || n = 845 || n = 859 || n = 894 || n = 929 || n = 936)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 651 || n = 678 || n = 786 || n = 813 || n = 858 || n = 903)
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
.elseif(n = 696 || n = 723 || n = 876)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 738)
	; 9 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
	pla
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
.elseif(n = 756 || n = 828)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 766)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 808)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 811)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 843)
	; 11 bytes
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	iny
	Jne *-2
	pla
	tay
	pla
.elseif(n = 910)
	; 10 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
	tay
	pla
.endif
.else
; 938 to 1250
.if(n = 938 || n = 939 || n = 941 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 991 || n = 994 || n = 1004 || n = 1009 || n = 1011 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1039 || n = 1043 || n = 1044 || n = 1046 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1078 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091 || n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1109 || n = 1113 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1148 || n = 1149 || n = 1151 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1179 || n = 1183 || n = 1184 || n = 1186 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 940 || n = 945 || n = 955 || n = 960 || n = 965 || n = 970 || n = 975 || n = 980 || n = 990 || n = 995 || n = 1000 || n = 1005 || n = 1010 || n = 1015 || n = 1028 || n = 1030 || n = 1035 || n = 1040 || n = 1045 || n = 1050 || n = 1060 || n = 1065 || n = 1070 || n = 1075 || n = 1080 || n = 1085 || n = 1095 || n = 1100 || n = 1105 || n = 1110 || n = 1115 || n = 1120 || n = 1130 || n = 1135 || n = 1140 || n = 1145 || n = 1150 || n = 1155 || n = 1165 || n = 1170 || n = 1175 || n = 1180 || n = 1185 || n = 1190 || n = 1200 || n = 1205 || n = 1210 || n = 1215 || n = 1220 || n = 1225 || n = 1235 || n = 1240 || n = 1245 || n = 1250)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1248)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && (n = 943 || n = 950 || (n > 963 && n < 986) || (n > 998 && n < 1021) || (n > 1033 && n < 1056) || (n > 1068 && n < 1091) || (n > 1103 && n < 1126) || (n > 1138 && n < 1161) || (n > 1173 && n < 1196) || (n > 1208 && n < 1231) || n = 1244))
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
.elseif(n = 948 || n = 993 || n = 1218)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 953 || n = 958 || n = 963 || n = 968 || n = 988 || n = 998 || n = 1003 || n = 1023 || n = 1031 || n = 1033 || n = 1053 || n = 1058 || n = 1073 || n = 1088 || n = 1093 || n = 1098 || n = 1103 || n = 1108 || n = 1123 || n = 1133 || n = 1138 || n = 1143 || n = 1158 || n = 1163 || n = 1168 || n = 1178 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1228 || n = 1233 || n = 1238)
	; 10..11 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 976)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 983)
	; 11 bytes
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	dey
	Jne *-2
	pla
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
.elseif(n = 1008 || n = 1068 || n = 1116 || n = 1176 || n = 1248)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1018)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jpl *-4
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
.elseif(n = 1126)
	; 10 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1128 || n = 1173)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1198)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	dey
	Jne *-4
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
.if((n .mod 7 = 5) && ((n > 1250 && n < 1266) || (n > 1278 && n < 1329)))
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
.elseif(n = 1253 || n = 1254 || n = 1256 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1288 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1301 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1316 || n = 1318 || n = 1322 || n = 1323 || n = 1325)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1255 || n = 1260 || n = 1270 || n = 1275 || n = 1280 || n = 1285 || n = 1290 || n = 1295 || n = 1310 || n = 1324)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1263 || n = 1308 || n = 1317 || n = 1326)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 5 = 3) && ((n > 1267 && n < 1284) || n = 1298))
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1281)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1292)
	; 9 bytes
	pha
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
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
.elseif(n = 1299 || n = 1315)
	; 11..13 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1306 || n = 1313 || n = 1327)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 1312)
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
.elseif(n = 1319)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
	tay
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
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1331 || n = 1338 || n = 1345 || n = 1352 || n = 1359 || n = 1366 || n = 1373 || n = 1381 || n = 1390 || n = 1394 || n = 1401)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1330 || n = 1334 || n = 1337 || n = 1339 || n = 1351 || n = 1355 || n = 1357 || n = 1358 || n = 1360 || n = 1364 || n = 1365 || n = 1367 || n = 1372 || n = 1379 || n = 1386 || n = 1388 || n = 1393 || n = 1395 || n = 1400 || n = 1402 || n = 1406)
	; 11..13 bytes
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
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362) || n = 1375 || n = 1382 || n = 1396 || n = 1403))
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
.elseif(n = 1336)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	ldx #240
	dex
	Jne *-1
	tax
	pla
.elseif(n = 1341)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 1343)
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
.elseif((n .mod 9 = 3) && ((n > 1343 && n < 1390)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1346)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1348 || n = 1369 || n = 1383 || n = 1397)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 1350)
	; 13 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1368 || n = 1392 || n = 1404)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1374)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1376)
	; 12 bytes
	pha
	tya
	ldy #(n-23)/11
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
.elseif(n = 1385)
	; 12 bytes
	pha
	tya
	pha
	ldy #152 ;hides 'tya'
	nop
	dey
	Jne *-3
	pla
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
.elseif(n = 1399)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1416 || n = 1425 || n = 1443 || n = 1452 || n = 1470 || n = 1479)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1408 || n = 1413 || n = 1415 || n = 1422 || n = 1429 || n = 1434 || n = 1436 || n = 1441 || n = 1455 || n = 1457 || n = 1469 || n = 1471 || n = 1478)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1430 || n = 1472)
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
.elseif(n = 1411 || n = 1423 || n = 1439 || n = 1444 || n = 1460 || n = 1467)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1414 || n = 1418 || n = 1421 || n = 1427 || n = 1435 || n = 1442 || n = 1449 || n = 1456 || n = 1458 || n = 1463 || n = 1465 || n = 1477 || n = 1481 || n = 1484)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1417 || n = 1438)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1420)
	; 12 bytes
	pha
	tya
	ldy #(n-23)/11
	delay_n 4
	nop
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1424 || n = 1445 || n = 1459 || n = 1473 || n = 1480)
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
.elseif(n = 1428 || n = 1476)
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
.elseif(n = 1432)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1437)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1446 || n = 1462)
	; 12..13 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1448)
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
.elseif(n = 1450)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	brk
	iny
	Jpl *-4
	tay
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
.elseif(n = 1466)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jpl *-4
	tay
	pla
.elseif(n = 1474)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1483)
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
.endif
.else
; 1485 to 1562
.if(n = 1485 || n = 1492 || n = 1499 || n = 1511 || n = 1513 || n = 1520 || n = 1525 || n = 1534 || n = 1539 || n = 1541 || n = 1555 || n = 1562)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1486 || n = 1490 || n = 1491 || n = 1493 || n = 1498 || n = 1502 || n = 1505 || n = 1507 || n = 1519 || n = 1521 || n = 1526 || n = 1540 || n = 1544 || n = 1546 || n = 1547 || n = 1549 || n = 1553 || n = 1554 || n = 1556 || n = 1561)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1487 || n = 1494 || n = 1501 || n = 1543 || n = 1550 || n = 1557)
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
.elseif((n .mod 9 = 3) && ((n > 1487 && n < 1516) || (n > 1532 && n < 1561)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1495 || n = 1509 || n = 1516 || n = 1523 || n = 1535 || n = 1558)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 1500 || n = 1512 || n = 1536 || n = 1548)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1504 || n = 1529)
	; 11..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1508)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 1514)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1518)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
	tay
	pla
.elseif(n = 1522)
	; 9 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
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
.elseif(n = 1528)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1530)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	tax
	pla
.elseif(n = 1532)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1537)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1568 || n = 1570 || n = 1575 || n = 1577 || n = 1582 || n = 1588 || n = 1589 || n = 1591 || n = 1603 || n = 1610 || n = 1612 || n = 1616 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1633 || n = 1638)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && (n = 1564 || n = 1571 || (n > 1584 && n < 1614) || n = 1627))
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
.elseif(n = 1567 || n = 1628 || n = 1630)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif((n .mod 9 = 3) && (n = 1569 || n = 1578 || (n > 1595 && n < 1633)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1572 || n = 1584 || n = 1620)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1574 || n = 1607)
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
.elseif(n = 1576 || n = 1581 || n = 1583 || n = 1590 || n = 1597 || n = 1604 || n = 1611 || n = 1618 || n = 1625 || n = 1637)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1579 || n = 1600 || n = 1635)
	; 12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1593)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1595)
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
.elseif(n = 1598)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1602)
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
.elseif(n = 1609)
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
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1640)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 1641 to 1718
.if(n = 1641 || n = 1655 || n = 1700)
	; 11..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1642 || n = 1647 || n = 1661 || n = 1691 || n = 1698 || n = 1705)
	; 11..12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1645 || n = 1652 || n = 1658 || n = 1666 || n = 1670 || n = 1673 || n = 1675 || n = 1679 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1708 || n = 1710 || n = 1714 || n = 1715 || n = 1717)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1651 || n = 1660 || n = 1667 || n = 1674 || n = 1681 || n = 1688 || n = 1702 || n = 1707 || n = 1709)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1648)
	; 9 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1649 || n = 1663)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1653)
	; 11 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
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
.elseif((n .mod 9 = 3) && ((n > 1658 && n < 1705)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 3) && ((n > 1661 && n < 1698) || n = 1711 || n = 1718))
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
.elseif(n = 1665)
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
.elseif(n = 1672)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1682)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1684)
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
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1726 || n = 1747 || n = 1759 || n = 1768 || n = 1775 || n = 1782 || n = 1789)
	; 11..12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1722 || n = 1731 || n = 1740 || n = 1758 || n = 1767 || n = 1785 || n = 1794)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1723 || n = 1730 || n = 1737 || n = 1744 || n = 1749 || n = 1756 || n = 1765 || n = 1772 || n = 1779 || n = 1786 || n = 1793)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1724 || n = 1766 || n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1725 || n = 1732 || n = 1739 || n = 1760 || n = 1774 || n = 1781 || n = 1795)
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
.elseif(n = 1728 || n = 1752 || n = 1764 || n = 1788)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1729 || n = 1733 || n = 1736 || n = 1738 || n = 1742 || n = 1743 || n = 1745 || n = 1750 || n = 1754 || n = 1757 || n = 1771 || n = 1773 || n = 1778 || n = 1780 || n = 1792 || n = 1796)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1735 || n = 1753)
	; 11..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1746)
	; 9 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
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
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
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
.elseif(n = 1777)
	; 12 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 26
	Jmi *-5
	tay
	pla
.elseif(n = 1784)
	; 13 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1791)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
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
.elseif(n = 1798)
	; 13 bytes
	pha
	tya
	ldy #(n-31)/19
	delay_n 14
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 1799 || n = 1806 || n = 1814 || n = 1823 || n = 1826)
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
.elseif(n = 1805 || n = 1816 || n = 1827)
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
.elseif(n = 1807 || n = 1813 || n = 1815 || n = 1833)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1808 || n = 1811 || n = 1817 || n = 1819 || n = 1828)
	; 12..13 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1818)
	; 12 bytes
	pha
	tya
	ldy #128
	delay_n 9
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1820)
	; 13 bytes
	pha
	tya
	ldy #(n-28)/16
	delay_n 11
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 1822)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1825)
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
.elseif(n = 1829)
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
.elseif(n = 1831)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 1835)
	; 12 bytes
	pha
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	iny ;first loop only
	iny
	Jne *-3
	pla
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
.elseif(n = 1838 || n = 1849 || n = 1871)
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
.elseif(n = 1840 || n = 1847 || n = 1851 || n = 1856 || n = 1863 || n = 1869)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841 || n = 1854 || n = 1859 || n = 1861 || n = 1862 || n = 1868 || n = 1870)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1843)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1844 || n = 1855 || n = 1864 || n = 1873)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1845 || n = 1858)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1846 || n = 1874)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1852)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
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
.elseif(n = 1865)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1867)
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
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876 || n = 1906)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1877 || n = 1886 || n = 1890 || n = 1894 || n = 1895 || n = 1898 || n = 1910 || n = 1913 || n = 1922 || n = 1924 || n = 1931 || n = 1940 || n = 1942 || n = 1946 || n = 1949)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1878 || n = 1887 || n = 1905 || n = 1919 || n = 1923 || n = 1935 || n = 1941)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1879 || n = 1891 || n = 1900 || n = 1903 || n = 1909 || n = 1918 || n = 1927 || n = 1936 || n = 1945 || n = 1951)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1880)
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
.elseif(n = 1881 || n = 1889 || n = 1897 || n = 1921 || n = 1953)
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
.elseif((n .mod 11 = 1) && (n = 1882 || (n > 1903 && n < 1949)))
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
.elseif(n = 1883 || n = 1907 || n = 1939)
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
.elseif(n = 1885)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 1888 || n = 1916 || n = 1930)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1892 || n = 1952)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
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
.elseif(n = 1912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1914)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	iny
	iny
	Jne *-5
	tay
	pla
.elseif(n = 1917)
	; 13 bytes
	pha
	tya
	ldy #126
	rol $26
	ror $26
	dey
	Jpl *-5
	tay
	pla
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
.elseif(n = 1928)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
	pla
.elseif(n = 1933 || n = 1950)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1934)
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
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1955 || n = 1987)
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
.elseif(n = 1957 || n = 1971 || n = 1977)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1958 || n = 1972 || n = 1986)
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
.elseif(n = 1960 || n = 1989)
	; 12..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1961 || n = 1963 || n = 1975 || n = 1990)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1962)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1964)
	; 13 bytes
	pha
	tya
	ldy #(n-28)/16
	delay_n 11
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 1966)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1967 || n = 1976 || n = 1982 || n = 1985)
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
.elseif(n = 1970 || n = 1981)
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
.elseif(n = 1973)
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
.elseif(n = 1984)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
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
.else
; 1993 to 2031
.if(n = 1993 || n = 2009)
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
.elseif(n = 1994 || n = 1996 || n = 2002 || n = 2006 || n = 2012 || n = 2018 || n = 2020 || n = 2021 || n = 2030)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995 || n = 2007 || n = 2013 || n = 2022 || n = 2031)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1997 || n = 2015 || n = 2024)
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
.elseif(n = 1999)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 2000)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
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
.elseif(n = 2003 || n = 2014 || n = 2025)
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
.elseif(n = 2005)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2008 || n = 2017 || n = 2023)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2011 || n = 2027)
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
.elseif(n = 2026)
	; 12 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
.elseif(n = 2029)
	; 13 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2033)
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
	; 12 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
	pla
.elseif(n = 2035)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 11 = 1) && ((n > 2035 && n < 2070)))
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
.elseif(n = 2038 || n = 2039 || n = 2048 || n = 2050 || n = 2054 || n = 2057 || n = 2066 || n = 2068)
	; 12..13 bytes
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
.elseif(n = 2042 || n = 2056 || n = 2070)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif(n = 2044 || n = 2053 || n = 2062)
	; 13 bytes
	php
	delay_n (n-7)
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
.elseif(n = 2049 || n = 2067)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2051 || n = 2059)
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
.elseif(n = 2060)
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
.elseif(n = 2061)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
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
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2083 || n = 2096 || n = 2105 || n = 2107)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2072)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
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
.elseif(n = 2074 || n = 2075 || n = 2077 || n = 2078 || n = 2093)
	; 12..13 bytes
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
.elseif(n = 2079 || n = 2085 || n = 2092 || n = 2094 || n = 2099 || n = 2101)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2080 || n = 2102)
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
.elseif(n = 2086)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2087)
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
	ldy #24 ;hides 'clc'
	brk
	iny
	Jpl *-4
	tay
	pla
.elseif(n = 2095 || n = 2104)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2097)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2111 || n = 2114 || n = 2120 || n = 2129 || n = 2138 || n = 2147 || n = 2149 || n = 2156 || n = 2158 || n = 2162 || n = 2164 || n = 2165 || n = 2173 || n = 2174 || n = 2176 || n = 2180 || n = 2183 || n = 2185 || n = 2186)
	; 12..14 bytes
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
.elseif((n .mod 11 = 1) && (n = 2113 || (n > 2134 && n < 2180)))
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
.elseif(n = 2115 || n = 2119 || n = 2130 || n = 2139 || n = 2151 || n = 2153 || n = 2166 || n = 2175 || n = 2187)
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
.elseif(n = 2121)
	; 12 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2122 || n = 2133 || n = 2169)
	; 12..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2123 || n = 2125 || n = 2131 || n = 2134 || n = 2143 || n = 2155 || n = 2161 || n = 2167 || n = 2170)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2126 || n = 2140 || n = 2182)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2128)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2137 || n = 2171)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2144)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 2152)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2159 || n = 2177)
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
.elseif(n = 2178)
	; 13 bytes
	pha
	tya
	ldy #(n-31)/19
	delay_n 14
	dey
	Jpl *-5
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if(n = 2188 || n = 2205)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2189 || n = 2225 || n = 2261)
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
.elseif((n .mod 9 = 3) && ((n > 2189 && n < 2263)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2191 || n = 2197 || n = 2203 || n = 2206 || n = 2215 || n = 2227 || n = 2233 || (n > 2248 && n < 2252) || n = 2260 || n = 2263)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2192 || n = 2194 || n = 2198 || n = 2200 || n = 2214 || n = 2219 || n = 2221 || n = 2222 || n = 2228 || n = 2230 || n = 2237 || n = 2246 || n = 2255 || n = 2257 || n = 2258 || n = 2264)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193 || n = 2211 || n = 2213 || n = 2229 || n = 2245 || n = 2259 || n = 2265)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2195 || n = 2204 || n = 2231 || n = 2240)
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
.elseif(n = 2196 || n = 2220 || n = 2232 || n = 2256)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 11 = 1) && ((n > 2200 && n < 2235)))
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
.elseif(n = 2202)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 11
	Jne *-5
	tay
	pla
.elseif(n = 2207)
	; 13 bytes
	pha
	tya
	pha
	ldy #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
	pla
.elseif(n = 2209)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
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
.elseif(n = 2224 || n = 2238 || n = 2252)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2236)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2241)
	; 13 bytes
	pha
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
	pla
.elseif(n = 2242)
	; 10 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
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
.elseif(n = 2248)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2254)
	; 13 bytes
	pha
	tya
	ldy #(n-31)/19
	delay_n 14
	dey
	Jpl *-5
	tay
	pla
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2294)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2267 || n = 2278 || n = 2300)
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
.elseif(n = 2269 || n = 2275 || n = 2287 || n = 2296 || n = 2299)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2270 || n = 2273 || n = 2282 || n = 2284 || n = 2291 || n = 2293)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2274 || n = 2283 || n = 2295 || n = 2301)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2281)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2285)
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
.elseif(n = 2290)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2302)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.endif
.else
; 2305 to 2343
.if(n = 2305 || n = 2314 || n = 2323)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2306 || n = 2309 || n = 2318 || n = 2320 || n = 2330 || n = 2338 || n = 2342)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2310 || n = 2315 || n = 2319 || n = 2325 || n = 2327 || n = 2331 || n = 2343)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
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
.elseif(n = 2317 || n = 2329)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 2324 || n = 2341)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2326)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
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
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if((n .mod 11 = 1) && ((n > 2343 && n < 2378) || (n > 2398 && n < 2422)))
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
.elseif((n .mod 12 = 5) && (n = 2345 || (n > 2368 && n < 2418)))
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
.elseif(n = 2346 || n = 2354 || n = 2356 || n = 2374 || n = 2390 || n = 2394 || n = 2398 || n = 2401 || n = 2402 || n = 2404 || n = 2408 || n = 2411 || n = 2414 || n = 2416)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2351 || n = 2359 || n = 2371 || (n > 2382 && n < 2386) || n = 2395 || n = 2407 || n = 2413 || n = 2419)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2348 || n = 2396)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 2350 || n = 2378 || n = 2392 || n = 2420)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2351 && n < 2413)))
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
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 2358 || n = 2375 || n = 2409)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2361 || n = 2367 || n = 2373 || n = 2379 || n = 2389 || n = 2391 || n = 2403 || n = 2415)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2362)
	; 13 bytes
	pha
	tya
	ldy #(n-37)/25
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2363)
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
.elseif(n = 2365)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2368 || n = 2380)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2370 || n = 2406)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2372)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2382 || n = 2397)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2386)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2387)
	; 13 bytes
	pha
	tya
	ldy #(n-31)/19
	delay_n 14
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2418)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2423 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2444 || n = 2450)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 0) && ((n > 2423 && n < 2461)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2425)
	; 13 bytes
	pha
	tya
	ldy #(n-31)/19
	delay_n 14
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2427 || n = 2433 || n = 2439 || n = 2445 || n = 2449 || n = 2451)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2430)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 2431 || n = 2437 || n = 2453 || n = 2455)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2432 || n = 2443 || n = 2454)
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
.elseif(n = 2434)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2442)
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
.elseif(n = 2452)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2456)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2457)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2458)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 2459)
	; 12 bytes
	brk
	delay_n (n-13)
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 13 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 2463 || n = 2468 || n = 2475 || n = 2479 || n = 2481 || n = 2493 || n = 2499)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2464 || n = 2500)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2465 || n = 2487 || n = 2498)
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
.elseif(n = 2466 || n = 2482)
	; 12..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2467 || n = 2483 || n = 2485 || n = 2491)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2469)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2470)
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
.elseif(n = 2471 || n = 2474 || n = 2480 || n = 2486 || n = 2488)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2472 || n = 2484 || n = 2496)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2473 || n = 2489)
	; 12..13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2476)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
	pla
.elseif(n = 2477 || n = 2494)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2478)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2492)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
	pla
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
; 2501 to 2578
.if((n .mod 12 = 5) && ((n > 2500 && n < 2550) || n = 2573))
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
.elseif(n = 2502 || n = 2577)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2503 || n = 2505 || n = 2515 || n = 2527 || n = 2539 || n = 2557 || n = 2569)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2504 || n = 2518 || n = 2546 || n = 2560 || n = 2574)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2506 || n = 2510 || n = 2516 || n = 2522 || n = 2524 || n = 2530 || n = 2533 || n = 2534 || n = 2536 || n = 2541 || n = 2555 || n = 2558 || n = 2563 || n = 2566 || n = 2570 || n = 2576)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2507 || n = 2517 || n = 2523 || n = 2535 || n = 2543 || n = 2547 || n = 2554 || n = 2559 || n = 2565 || n = 2571)
	; 12..14 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 11 = 1) && (n = 2509 || (n > 2530 && n < 2576)))
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
.elseif(n = 2512 || n = 2552 || n = 2572)
	; 12 bytes
	pha
	tya
	ldy #(n-32)/20
	jsr @nearby_rts_15cyc
	dey
	Jpl *-4
	tay
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
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2526)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2529)
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
.elseif(n = 2538)
	; 13 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	pla
	tay
	pla
.elseif(n = 2540)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2548)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2578)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jmi *-6
	tay
	pla
.endif
.else
; 2579 to 2656
.if(n = 2579 || n = 2596 || n = 2647)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2579 && n < 2653)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2581 || n = 2582 || n = 2590 || n = 2594 || n = 2598 || n = 2603 || n = 2606 || n = 2618 || n = 2620 || n = 2624 || n = 2626 || n = 2639 || n = 2642 || n = 2646 || n = 2654 || n = 2656)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2587 || n = 2589 || n = 2595 || n = 2600 || n = 2607 || n = 2625 || n = 2631 || n = 2643 || n = 2650 || n = 2655)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 12 = 5) && (n = 2585 || (n > 2608 && n < 2646)))
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
.elseif(n = 2586 || n = 2622)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2588 || n = 2602 || n = 2644)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2591 || n = 2599 || n = 2611 || n = 2623 || n = 2629 || n = 2635 || n = 2648)
	; 13..14 bytes
	php
	delay_n (n-7)
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
.elseif((n .mod 11 = 1) && ((n > 2596 && n < 2642)))
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
.elseif(n = 2601)
	; 13 bytes
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
	pla
.elseif(n = 2605 || n = 2617)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 2612 || n = 2632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 2614)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2615 || n = 2634 || n = 2653)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2627)
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
.elseif(n = 2636)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2637)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2638 || n = 2651)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
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
.endif
.endif
.else
; 2657 to 2812
.if((n .mod 12 = 5) && (n = 2657 || n = 2669 || (n > 2692 && n < 2718) || (n > 2764 && n < 2802)))
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
.elseif(n = 2658 || n = 2694 || n = 2730 || n = 2766 || n = 2802)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2659 || n = 2671 || n = 2683 || n = 2695 || n = 2701 || n = 2719 || n = 2731 || n = 2737 || n = 2743 || n = 2745 || n = 2747 || n = 2755 || n = 2767 || n = 2779 || n = 2791 || n = 2803 || n = 2809)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2660 || n = 2662 || n = 2665 || n = 2666 || n = 2668 || n = 2673 || n = 2678 || n = 2680 || n = 2687 || n = 2690 || n = 2702 || n = 2704 || n = 2709 || n = 2711 || n = 2720 || n = 2722 || n = 2726 || n = 2734 || n = 2744 || n = 2746 || n = 2754 || n = 2758 || n = 2768 || n = 2774 || n = 2776 || n = 2782 || n = 2785 || n = 2786 || n = 2788 || n = 2793 || n = 2797 || n = 2804)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2661 || n = 2667 || n = 2675 || n = 2679 || n = 2691 || n = 2697 || n = 2703 || n = 2708 || n = 2721 || n = 2727 || n = 2733 || n = 2739 || n = 2741 || n = 2753 || n = 2759 || n = 2763 || n = 2769 || n = 2775 || n = 2787 || n = 2799 || n = 2805 || n = 2811)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2774) || n = 2795 || n = 2806))
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
.elseif(n = 2670 || n = 2706 || n = 2778 || n = 2790)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 14 = 12) && (n = 2672 || n = 2686 || (n > 2713 && n < 2771) || n = 2798 || n = 2812))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2677 || n = 2781 || n = 2794 || n = 2807)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2681 || n = 2698 || n = 2715 || n = 2732 || n = 2749 || n = 2783 || n = 2800)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2682 || n = 2757)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2684 || n = 2780)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2689 || n = 2713 || n = 2725)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2692 || n = 2752 || n = 2792)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2699 || n = 2735 || n = 2771)
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
.elseif(n = 2710)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2716 || n = 2764)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2723)
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
.elseif(n = 2738)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2750)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	tay
	pla
.elseif(n = 2761)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
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
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if(n = 2813 || n = 2823 || n = 2829 || n = 2831 || n = 2835 || n = 2841 || n = 2847 || n = 2855 || n = 2859 || n = 2871 || n = 2877 || n = 2883 || n = 2888)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2824 || n = 2830 || n = 2834 || n = 2836 || n = 2842 || n = 2846 || n = 2848 || n = 2853 || n = 2858 || n = 2860 || n = 2870 || n = 2876 || n = 2878)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815 || n = 2827 || n = 2839 || n = 2845 || n = 2863 || n = 2875 || n = 2887 || n = 2889)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2816 || n = 2886)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 2818)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2819 && n < 2881)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2821 || n = 2857 || n = 2865 || n = 2869)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif((n .mod 12 = 5) && ((n > 2824 && n < 2874)))
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
.elseif(n = 2826 || n = 2840 || n = 2854 || n = 2882)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2833)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2838 || n = 2874)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2843 || n = 2881)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2850)
	; 13 bytes
	pha
	tya
	ldy #128
	nop
	jsr @nearby_rts_15cyc
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2851 || n = 2885)
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
.elseif(n = 2862)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2864)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2866)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	iny
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2867)
	; 14 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2879)
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
.elseif(n = 2884)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2890)
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
; 2891 to 2968
.if(n = 2891 || n = 2958)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 12 = 0) && ((n > 2891 && n < 2965)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2893 || n = 2905 || n = 2929 || n = 2941 || n = 2965)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2894 || n = 2906 || n = 2908 || n = 2914 || n = 2918 || n = 2920 || n = 2925 || n = 2926 || n = 2930 || n = 2934 || n = 2942 || n = 2944 || n = 2948 || n = 2954 || n = 2956 || n = 2961)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895 || n = 2907 || n = 2913 || n = 2927 || n = 2931 || n = 2939 || n = 2943 || n = 2949 || n = 2955 || n = 2967)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2896 || n = 2924 || n = 2938 || n = 2966)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2898 || n = 2950 || n = 2963)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2899 || n = 2903 || n = 2911 || n = 2917 || n = 2923 || n = 2935 || n = 2947 || n = 2959)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2900)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2901)
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
.elseif(n = 2902 || n = 2919 || n = 2936 || n = 2953)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2910 || n = 2946)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2912 || n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2922 || n = 2937)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2960)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2962)
	; 13 bytes
	pha
	tya
	ldy #(n-37)/25
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2968)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2970 || n = 2987 || n = 3004 || n = 3021)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2971 || n = 2983 || n = 2995 || n = 3007 || n = 3019 || n = 3025 || n = 3031 || n = 3043)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2972 || n = 2992 || n = 3032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2973 || n = 2975 || n = 2979 || n = 2985 || n = 2991 || n = 3003 || n = 3011 || n = 3015 || n = 3027 || n = 3035 || n = 3037 || n = 3039)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2974 || n = 2978 || n = 2984 || n = 2990 || n = 2996 || n = 2998 || n = 3002 || n = 3006 || n = 3009 || n = 3010 || n = 3014 || n = 3016 || n = 3020 || n = 3023 || n = 3026 || n = 3028 || n = 3038 || n = 3040 || n = 3044 || n = 3045)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 0) && ((n > 2975 && n < 3037)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2977 || n = 3001)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2980 || n = 2994 || n = 3008 || n = 3022)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2982 || n = 3018)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2986)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2989)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2997 || n = 3042)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2999)
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
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3033)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3034)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3046)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3049 || n = 3073 || n = 3085 || n = 3103 || n = 3121)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3048 || n = 3060 || n = 3072)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 14 = 12) && ((n > 3049 && n < 3121)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3051 || n = 3057 || n = 3063 || n = 3075 || n = 3087 || n = 3093 || n = 3111)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3054 || n = 3090 || n = 3108)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3056 || n = 3058 || n = 3062 || n = 3066 || n = 3074 || n = 3076 || n = 3086 || n = 3088 || n = 3094 || n = 3104 || n = 3105 || n = 3110 || n = 3118 || n = 3122 || n = 3124 || n = 3125)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3061 || n = 3067 || n = 3079 || n = 3091 || n = 3097 || n = 3099 || n = 3115)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3068 || n = 3100 || n = 3116)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3069 || n = 3083)
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
.elseif(n = 3070 || n = 3114)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3071 || n = 3109)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3080 || n = 3119)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3081)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3082)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 3089 || n = 3123)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3096 || n = 3098)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3101 || n = 3107)
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
.elseif(n = 3102 || n = 3117)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3144 || n = 3162 || n = 3180 || n = 3198)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3127 || n = 3133 || n = 3141 || n = 3151 || n = 3169 || n = 3187)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3128 || n = 3130 || n = 3136 || n = 3138 || n = 3142 || n = 3146 || n = 3150 || n = 3154 || n = 3159 || n = 3164 || n = 3173 || n = 3178 || n = 3182 || n = 3189 || n = 3193 || n = 3199 || n = 3200)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129 || n = 3135 || n = 3147 || n = 3155 || n = 3160 || n = 3165 || n = 3179 || n = 3183 || n = 3188 || n = 3195 || n = 3201)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3131 || n = 3149 || n = 3167 || n = 3203)
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
.elseif(n = 3132 || n = 3152 || n = 3172 || n = 3192)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3134 || n = 3148 || n = 3176 || n = 3190)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3137)
	; 13 bytes
	pha
	tya
	ldy #(n-37)/25
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3139 || n = 3170 || n = 3175)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3140 || n = 3157 || n = 3174 || n = 3191)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3143)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 13 = 12) && ((n > 3144 && n < 3198)))
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3153 || n = 3181)
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
.elseif(n = 3156)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3161)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	delay_n 14
	Jne *-7
	tay
	pla
.elseif(n = 3163 || n = 3168 || n = 3186)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3166 || n = 3185)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3177)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
.elseif(n = 3196)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3202)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 3204 to 3281
.if((n .mod 14 = 12) && ((n > 3203 && n < 3275)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3205 || n = 3217 || n = 3241 || n = 3253 || n = 3265 || n = 3266 || n = 3277 || n = 3281)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3206 || n = 3214 || n = 3220 || n = 3226 || n = 3227 || n = 3230 || n = 3231 || n = 3236 || n = 3238 || n = 3243 || n = 3248 || n = 3254 || n = 3256 || n = 3269 || n = 3278)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3207 || n = 3222 || n = 3267)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3208 || n = 3225 || n = 3242 || n = 3259 || n = 3276)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3209 || n = 3251)
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
.elseif(n = 3211 || n = 3229 || n = 3247 || n = 3271)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3212 || n = 3272)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3213 || n = 3215 || n = 3219 || n = 3235 || n = 3237 || n = 3245 || n = 3255 || n = 3261 || n = 3273 || n = 3279)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3216 || n = 3234 || n = 3252 || n = 3270)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3221 || n = 3239 || n = 3257)
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
.elseif(n = 3223 || n = 3249 || n = 3262 || n = 3275)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3224 || n = 3268)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3228 || n = 3244)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3233)
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
.elseif(n = 3240 || n = 3264)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3250)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
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
.elseif(n = 3280)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
; 3282 to 3359
.if(n = 3282 || n = 3297 || n = 3357)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3283 || n = 3319 || n = 3343 || n = 3355)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3284 || n = 3285 || n = 3290 || n = 3294 || n = 3295 || n = 3304 || n = 3308 || n = 3326 || n = 3328 || n = 3329 || n = 3339 || n = 3346 || n = 3354 || n = 3359)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3287)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3288 || n = 3306 || n = 3324 || n = 3342)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3289 || n = 3291 || n = 3296 || n = 3305 || n = 3309 || n = 3315 || n = 3333 || n = 3335 || n = 3345)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3292 || n = 3312 || n = 3332 || n = 3352)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3293 || n = 3310 || n = 3327)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3298)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts
	Jne *-6
	tay
	pla
.elseif(n = 3299 || n = 3318 || n = 3337)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3300 || n = 3313 || n = 3317 || n = 3323 || n = 3331 || n = 3349 || n = 3351)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3301 || n = 3314)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 14 = 12) && ((n > 3301 && n < 3359)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3311 || n = 3347)
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
.elseif(n = 3320 || n = 3336)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3322 || n = 3348 || n = 3350)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3325)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3334)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3338)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk
	iny
	Jpl *-7
	tay
	pla
.elseif(n = 3340)
	; 13 bytes
	pha
	tya
	ldy #0
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3341 || n = 3353)
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
.elseif(n = 3356)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3378 || n = 3396 || n = 3414 || n = 3432)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3361 || n = 3395 || n = 3412 || n = 3429)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3362 || n = 3364 || n = 3369 || n = 3374 || n = 3380 || n = 3382 || n = 3397 || n = 3398 || n = 3406 || n = 3416 || n = 3418 || n = 3424 || n = 3430 || n = 3431 || n = 3434)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3370 || n = 3379 || n = 3381 || n = 3389 || n = 3399 || n = 3415 || n = 3417)
	; 12..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3366 || n = 3384)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3367 || n = 3368 || n = 3385 || n = 3393 || n = 3403 || n = 3407 || n = 3421)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3371 || n = 3373 || n = 3409 || n = 3427)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3372 || n = 3386 || n = 3400 || n = 3428)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3375 || n = 3394 || n = 3413)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3376)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 3377 || n = 3405 || n = 3433)
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
.elseif(n = 3387 || n = 3402)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3388 || n = 3404 || n = 3420 || n = 3436)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3390)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3391)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3392)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3408 || n = 3426)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3410)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3411)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 3422)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
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
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3467)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3439 || n = 3457)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3440 || n = 3449 || n = 3453 || n = 3459 || n = 3464 || n = 3469 || n = 3471)
	; 12..14 bytes
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
.elseif(n = 3442 || n = 3456 || n = 3470)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3443)
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
.elseif(n = 3444 || n = 3448 || n = 3454 || n = 3458 || n = 3460 || n = 3465 || n = 3474)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3445)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3446 || n = 3463)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3447 || n = 3462)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3450 || n = 3468)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3451)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3452 || n = 3472)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3455 || n = 3473)
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
.elseif(n = 3461)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
.elseif(n = 3466)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3475)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	nop
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 3476)
	; 14 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.else
; 3477 to 3515
.if(n = 3477)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3478 || n = 3483 || n = 3485 || n = 3489 || n = 3501 || n = 3507)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479 || n = 3488 || n = 3494 || n = 3496 || n = 3499 || n = 3506)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3480 || n = 3497 || n = 3514)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3481 || n = 3495)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3482)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3484 || n = 3498 || n = 3512)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3486 || n = 3504)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 3490)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3491 || n = 3509)
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
.elseif(n = 3492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3493 || n = 3505 || n = 3511)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3500)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3502)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3503)
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
.elseif(n = 3508)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3510)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
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
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3564 || n = 3580)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3517 || n = 3535 || n = 3553 || n = 3571 || n = 3589)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3518 || n = 3524 || n = 3528 || n = 3530 || n = 3533 || n = 3534 || n = 3539 || n = 3542 || n = 3544 || n = 3550 || n = 3556 || n = 3560 || n = 3562 || n = 3566 || n = 3569 || n = 3570 || n = 3574 || n = 3578 || n = 3591)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519 || n = 3521 || n = 3529 || n = 3538 || n = 3547 || n = 3583)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3520)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3522 || n = 3540 || n = 3558 || n = 3576)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3523 || n = 3525 || n = 3543 || n = 3551 || n = 3555 || n = 3557 || n = 3561 || n = 3575 || n = 3579 || n = 3585)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3526 || n = 3554 || n = 3568 || n = 3582)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3527 || n = 3546 || n = 3584)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3531 || n = 3548 || n = 3565)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3532 || n = 3552 || n = 3572 || n = 3592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3536)
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
.elseif(n = 3537 || n = 3567)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3541)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jmi *-6
	tay
	pla
.elseif(n = 3545 || n = 3563 || n = 3581)
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
.elseif(n = 3549)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3559 || n = 3573)
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
.elseif(n = 3577)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3586)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3587)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3588)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3590)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 3593)
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
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if(n = 3594 || n = 3612 || n = 3630 || n = 3648 || n = 3666)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3595 || n = 3597 || n = 3602 || n = 3611 || n = 3613 || n = 3615 || n = 3631 || n = 3636 || n = 3647 || n = 3651 || n = 3667 || n = 3669)
	; 11..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3598 || n = 3601 || n = 3605 || n = 3614 || n = 3618 || n = 3629 || n = 3634 || n = 3635 || n = 3646 || n = 3654 || n = 3656 || n = 3659 || n = 3663 || n = 3668)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599 || n = 3616 || n = 3633 || n = 3650)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3600 || n = 3658 || n = 3670)
	; 12..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3603 || n = 3622 || n = 3641)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3604 || n = 3621 || n = 3638)
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
.elseif(n = 3606 || n = 3619 || n = 3623 || n = 3637 || n = 3640 || n = 3655 || n = 3671)
	; 12..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3607 || n = 3625 || n = 3643 || n = 3661)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3608)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3609)
	; 14 bytes
	pha
	tya
	ldy #109
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3610)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3617 || n = 3653)
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
.elseif(n = 3620)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3624 || n = 3645)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3626 || n = 3649)
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
.elseif(n = 3627 || n = 3642 || n = 3657)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3628 || n = 3644 || n = 3660)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3632 || n = 3652)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3639)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3662)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
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
.elseif(n = 3665)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3692 || n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3673 || n = 3691 || n = 3699 || n = 3713 || n = 3727 || n = 3729 || n = 3745)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3674 || n = 3678 || n = 3681 || n = 3686 || n = 3688 || n = 3693 || n = 3694 || n = 3696 || n = 3700 || n = 3703 || n = 3704 || n = 3710 || n = 3714 || n = 3716 || n = 3719 || n = 3734 || n = 3737 || n = 3739 || n = 3740 || n = 3742 || n = 3749)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675 || n = 3687 || n = 3705 || n = 3721 || n = 3723 || n = 3741 || n = 3746)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3676 || n = 3708 || n = 3724)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3677 || n = 3715 || n = 3731 || n = 3733 || n = 3748)
	; 12..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3679 || n = 3698 || n = 3736)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3680)
	; 13 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3682)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3683)
	; 14 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 3684 || n = 3702 || n = 3720 || n = 3738)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3685)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3689 || n = 3707 || n = 3725 || n = 3743)
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
.elseif(n = 3690 || n = 3726 || n = 3728 || n = 3744)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 3701 || n = 3718 || n = 3735)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
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
.elseif(n = 3709)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tay
	pla
.elseif(n = 3711)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3717 || n = 3747)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
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
.elseif(n = 3730)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3750)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
; 3751 to 3906
.if(n = 3751 || n = 3767 || n = 3782 || n = 3787 || n = 3823 || n = 3841 || n = 3859 || n = 3877 || n = 3880 || n = 3895 || n = 3901)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3752 || n = 3769 || n = 3786 || n = 3803 || n = 3820 || n = 3837 || n = 3854 || n = 3871 || n = 3888 || n = 3905)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3753 || n = 3758 || n = 3760 || n = 3764 || n = 3771 || n = 3773 || n = 3776 || n = 3778 || n = 3790 || n = 3794 || n = 3801 || n = 3805 || n = 3809 || n = 3814 || n = 3824 || n = 3830 || n = 3834 || n = 3838 || n = 3839 || n = 3843 || n = 3848 || n = 3856 || n = 3858 || n = 3866 || n = 3873 || n = 3874 || n = 3884 || n = 3886 || n = 3890 || n = 3894 || n = 3896 || n = 3902 || n = 3904)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3755 || n = 3793 || n = 3850 || n = 3869)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3756 || n = 3774 || n = 3792 || n = 3810 || n = 3828 || n = 3846 || n = 3864 || n = 3882 || n = 3900)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3757 || n = 3759 || n = 3766 || n = 3775 || n = 3777 || n = 3784 || n = 3789 || n = 3795 || n = 3800 || n = 3802 || n = 3806 || n = 3813 || n = 3821 || n = 3831 || n = 3849 || n = 3855 || n = 3857 || n = 3870 || n = 3875 || n = 3885 || n = 3891 || n = 3903)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3761 || n = 3779 || n = 3797 || n = 3815 || n = 3833 || n = 3851 || n = 3887)
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
.elseif(n = 3762 || n = 3807 || n = 3822)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3763 || n = 3781 || n = 3799 || n = 3817 || n = 3819 || n = 3827 || n = 3835 || n = 3844 || n = 3853 || n = 3861 || n = 3878 || n = 3879 || n = 3889 || n = 3899)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3765)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3768)
	; 14 bytes
	pha
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
	pla
.elseif(n = 3770 || n = 3811 || n = 3829 || n = 3847 || n = 3863 || n = 3881 || n = 3883)
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
.elseif(n = 3772 || n = 3812 || n = 3832 || n = 3852 || n = 3872 || n = 3892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3780 || n = 3798 || n = 3816 || n = 3826 || n = 3860)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3783)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	nop
	pla
	pha
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 3785 || n = 3845 || n = 3865)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3788 || n = 3804 || n = 3836 || n = 3868)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3791)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	rol $26
	ror $26
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3796 || n = 3818 || n = 3840 || n = 3862 || n = 3906)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3808 || n = 3842 || n = 3893)
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
.elseif(n = 3825)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
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
.elseif(n = 3876 || n = 3897)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3898)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3926 || n = 3945 || n = 3983)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3908 || n = 3915 || n = 3921 || n = 3935 || n = 3942 || n = 3951 || n = 3953 || n = 3955 || n = 3957 || n = 3975 || n = 3981)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3909 || n = 3914 || n = 3920 || n = 3924 || n = 3930 || n = 3933 || n = 3934 || n = 3938 || n = 3940 || n = 3941 || n = 3947 || n = 3958 || n = 3966 || n = 3969 || n = 3974 || n = 3976 || n = 3982)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3927 || n = 3944)
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
.elseif(n = 3911 || n = 3960)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3912 || n = 3932 || n = 3952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3913 || n = 3931 || n = 3967)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3916 || n = 3964 || n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3917)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3918 || n = 3936 || n = 3954 || n = 3972)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3919 || n = 3925 || n = 3929 || n = 3943 || n = 3946 || n = 3961 || n = 3963 || n = 3979)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3922 || n = 3939 || n = 3956 || n = 3973)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3923 || n = 3959 || n = 3977)
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
.elseif(n = 3928 || n = 3950)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3937 || n = 3962)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 3949)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3965)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3968 || n = 3971)
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
.elseif(n = 3970)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3978)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3984)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 3986 || n = 3991 || n = 4003 || n = 4023 || n = 4039 || n = 4057)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3987 || n = 4037)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3988)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3989 || n = 3998 || n = 4004 || n = 4009 || n = 4014 || n = 4018 || n = 4028 || n = 4030 || n = 4034 || n = 4036 || n = 4043 || n = 4045 || n = 4046 || n = 4048 || n = 4053 || n = 4054 || n = 4056)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3990 || n = 4008 || n = 4026 || n = 4044 || n = 4062)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3992 || n = 4012 || n = 4032 || n = 4052)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3993 || n = 4000 || n = 4011 || n = 4013 || n = 4027 || n = 4029 || n = 4035 || n = 4047 || n = 4055 || n = 4061)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3994 || n = 4016 || n = 4038)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3995 || n = 4031 || n = 4049)
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
.elseif(n = 3996 || n = 4060)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3997 || n = 3999 || n = 4015 || n = 4017 || n = 4019 || n = 4033 || n = 4051)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4001)
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
.elseif(n = 4002 || n = 4021 || n = 4040 || n = 4059)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4005)
	; 14 bytes
	pha
	tya
	ldy #121
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4006)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4007 || n = 4024 || n = 4041 || n = 4058)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
.elseif(n = 4020)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4022)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4025)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4042)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4050)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
; 4063 to 4140
.if(n = 4063 || n = 4131)
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
.elseif(n = 4064 || n = 4066 || n = 4071 || n = 4074 || n = 4077 || n = 4079 || n = 4082 || n = 4084 || n = 4094 || n = 4096 || n = 4099 || n = 4100 || n = 4102 || n = 4106 || n = 4110 || n = 4111 || n = 4113 || n = 4118 || n = 4120 || n = 4125 || n = 4128 || n = 4130 || n = 4136 || n = 4138)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065 || n = 4073 || n = 4081 || n = 4083 || n = 4090 || n = 4095 || n = 4101 || n = 4115 || n = 4117 || n = 4119 || n = 4124 || n = 4137)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4067 || n = 4085 || n = 4103 || n = 4121 || n = 4139)
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
.elseif(n = 4068 || n = 4086 || n = 4122 || n = 4140)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4069 || n = 4087 || n = 4105 || n = 4123 || n = 4133)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4070)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
	pla
.elseif(n = 4072 || n = 4112 || n = 4132)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4075 || n = 4092 || n = 4109 || n = 4126)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4076)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4078 || n = 4097 || n = 4135)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4080 || n = 4098 || n = 4116 || n = 4134)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4088 || n = 4093 || n = 4129)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4089)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4091)
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
.elseif(n = 4104)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4107)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4108)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4114)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4127)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	tax
	pla
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4150 || n = 4159 || n = 4167 || n = 4184 || n = 4195 || n = 4218)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4142 || n = 4146 || n = 4151 || n = 4155 || n = 4163 || n = 4169 || n = 4171 || n = 4173 || n = 4180 || n = 4191 || n = 4209 || n = 4215)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4143 || n = 4160 || n = 4177 || n = 4194 || n = 4211)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 4145 || n = 4154 || n = 4156 || n = 4161 || n = 4162 || n = 4164 || n = 4174 || n = 4179 || n = 4181 || n = 4185 || n = 4190 || n = 4196 || n = 4198 || n = 4208 || n = 4213)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4147 || n = 4165 || n = 4183 || n = 4201 || n = 4207)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4148 || n = 4214)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4149)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4152 || n = 4170 || n = 4188 || n = 4206)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4153 || n = 4202)
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
.elseif(n = 4157 || n = 4175 || n = 4193)
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
.elseif(n = 4158 || n = 4176 || n = 4178)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4166 || n = 4197)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4168)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 4172 || n = 4192 || n = 4212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4182 || n = 4199 || n = 4216)
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
.elseif(n = 4186)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4187)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4189)
	; 13 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4200)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4203)
	; 14 bytes
	pha
	tya
	ldy #126
	nop
	delay_n 26
	dey
	Jpl *-6
	tay
	pla
.elseif(n = 4204)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4205)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4210)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 14
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4217)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4237 || n = 4241 || n = 4255 || n = 4291)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4220 || n = 4223 || n = 4243 || n = 4253)
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
.elseif(n = 4221 || n = 4226 || n = 4234 || n = 4238 || n = 4244 || n = 4246 || n = 4247 || n = 4251 || n = 4254 || n = 4256 || n = 4261 || n = 4264 || n = 4270 || n = 4274 || n = 4276 || n = 4280 || n = 4282 || n = 4289 || n = 4293 || n = 4294)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4222)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4224 || n = 4242 || n = 4260 || n = 4278 || n = 4296)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4225)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	iny
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4227 || n = 4235 || n = 4248 || n = 4263 || n = 4271 || n = 4275 || n = 4281 || n = 4288 || n = 4295)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4228 || n = 4245 || n = 4262 || n = 4279)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4229 || n = 4265 || n = 4283)
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
.elseif(n = 4230 || n = 4249 || n = 4268 || n = 4287)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4231 || n = 4239 || n = 4267 || n = 4269 || n = 4285 || n = 4286)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4232 || n = 4252 || n = 4272 || n = 4292)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4233)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4236 || n = 4258)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4240)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4250)
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
.elseif(n = 4257)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
	tay
	pla
.elseif(n = 4259 || n = 4290)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4266 || n = 4284)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4273)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 14
	Jne *-6
	tax
	pla
.elseif(n = 4277)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 4297 to 4375
.if(n = 4297)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4298 || n = 4300 || n = 4304 || n = 4308 || n = 4311 || n = 4315 || n = 4316 || n = 4318 || n = 4323 || n = 4326 || n = 4329 || n = 4334 || n = 4336 || n = 4341 || n = 4348 || n = 4349 || n = 4351 || n = 4354 || n = 4365 || n = 4366 || n = 4370 || n = 4374)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4317 || n = 4322 || n = 4328 || n = 4333 || n = 4335 || n = 4340 || n = 4353 || n = 4367 || n = 4371)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4301 || n = 4319 || n = 4337 || n = 4355 || n = 4373)
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
.elseif(n = 4302 || n = 4324 || n = 4346)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4305)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4306 || n = 4325 || n = 4344 || n = 4363)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4307 || n = 4310 || n = 4331 || n = 4361)
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
.elseif(n = 4309 || n = 4327 || n = 4343 || n = 4345 || n = 4360)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4312 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4313 || n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4314 || n = 4332 || n = 4350 || n = 4368)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4320 || n = 4338 || n = 4356 || n = 4358)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4330)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4342)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4359)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4362)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
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
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
; 4376 to 4453
.if(n = 4376 || n = 4384 || n = 4385 || n = 4388 || n = 4394 || n = 4396 || n = 4403 || n = 4406 || n = 4408 || n = 4414 || n = 4416 || n = 4424 || n = 4426 || n = 4431 || n = 4441 || n = 4442 || n = 4444 || n = 4449)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377 || n = 4397)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4378 || n = 4428 || n = 4438 || n = 4446)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4379 || n = 4393 || n = 4411 || n = 4419 || n = 4429 || n = 4447)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4380)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4381 || n = 4399 || n = 4417 || n = 4435 || n = 4453)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4382 || n = 4401 || n = 4420 || n = 4439)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4383)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4386 || n = 4404 || n = 4422 || n = 4440)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4387 || n = 4437)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4389 || n = 4395 || n = 4407 || n = 4413 || n = 4415 || n = 4421 || n = 4425 || n = 4430 || n = 4443 || n = 4448 || n = 4450)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4390 || n = 4434)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4391 || n = 4409 || n = 4427 || n = 4445)
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
.elseif(n = 4392 || n = 4412 || n = 4432 || n = 4452)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4398)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4400 || n = 4433 || n = 4451)
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
.elseif(n = 4402)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4405)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
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
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
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
.elseif(n = 4436)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4460 || n = 4462 || n = 4473 || n = 4478 || n = 4480 || n = 4491 || n = 4496 || n = 4498 || n = 4502 || n = 4503 || n = 4509 || n = 4514 || n = 4516)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4455 || n = 4459 || n = 4461 || n = 4466 || n = 4468 || n = 4475 || n = 4479 || n = 4484 || n = 4486 || n = 4490 || n = 4495 || n = 4497 || n = 4504 || n = 4510 || n = 4515 || n = 4520)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4456)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4457)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4458 || n = 4476 || n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4463 || n = 4481 || n = 4499 || n = 4517)
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
.elseif(n = 4464 || n = 4482 || n = 4518)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4465 || n = 4483 || n = 4501 || n = 4519)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4467 || n = 4521)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4469 || n = 4471 || n = 4489 || n = 4525)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4470)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4472 || n = 4492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4474)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4477)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4485 || n = 4506 || n = 4527)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4487)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4493 || n = 4508 || n = 4511 || n = 4513 || n = 4523 || n = 4526 || n = 4529)
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
.elseif(n = 4500 || n = 4522)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4505)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4507)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4524)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4528)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4531)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	delay_n 26
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4552 || n = 4572 || n = 4592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4533 || n = 4541 || n = 4547 || n = 4549 || n = 4551 || n = 4569 || n = 4575 || n = 4587 || n = 4594 || n = 4601 || n = 4605)
	; 11..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4534 || n = 4553)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4535 || n = 4571 || n = 4589 || n = 4607)
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
.elseif(n = 4536 || n = 4558 || n = 4590 || n = 4597 || n = 4608)
	; 12..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4537 || n = 4539 || n = 4555 || n = 4559 || n = 4573 || n = 4598)
	; 12..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4538)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4540 || n = 4545 || n = 4550 || n = 4554 || n = 4556 || n = 4563 || n = 4564 || n = 4568 || n = 4570 || n = 4574 || n = 4576 || n = 4581 || n = 4586 || n = 4593 || n = 4595 || n = 4596 || n = 4599 || n = 4600 || n = 4604 || n = 4606)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4542)
	; 14 bytes
	pha
	tya
	ldy #151
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4543 || n = 4561 || n = 4579)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4544 || n = 4588)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4546)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 14
	dey
	Jne *-6
	tay
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
.elseif(n = 4557 || n = 4577)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4560 || n = 4578 || n = 4603)
	; 13..14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4562)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4565 || n = 4585)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4567 || n = 4580 || n = 4583)
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
.elseif(n = 4582)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
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
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4629 || n = 4648)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4611 || n = 4613 || n = 4618 || n = 4622 || n = 4624 || n = 4634 || n = 4636)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4617 || n = 4619 || n = 4623 || n = 4628 || n = 4635)
	; 11..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4615 || n = 4626)
	; 12..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4616 || n = 4627 || n = 4639)
	; 12..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4621)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 4630)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 4631)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4633)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4637)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4640)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif((n > 4640 && n < 4644) || n = 4646 || n = 4647)
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
.elseif(n = 4644)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4645)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4661 || n = 4671 || n = 4673 || n = 4680 || n = 4682 || n = 4684)
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
.elseif(n = 4651 || n = 4653 || n = 4660 || n = 4670 || n = 4675)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4652 || n = 4672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4654 || n = 4676)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4656)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4657 || n = 4659)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4658)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4662)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4663)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4664)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4665)
	; 14 bytes
	pha
	tya
	ldy #141
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4666)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4667 || n = 4686)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4668)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4669 || n = 4674 || n = 4678)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4677)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4679)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 14
	Jne *-7
	tax
	pla
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4685)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4687)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if(n = 4688 || n = 4689 || n = 4694 || n = 4700 || n = 4707 || n = 4714 || n = 4726 || n = 4734 || n = 4736 || n = 4744 || n = 4745 || n = 4754 || n = 4756)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4691 || n = 4704 || n = 4709 || n = 4711 || n = 4713 || n = 4721 || n = 4730 || n = 4741 || (n > 4746 && n < 4752) || n = 4753 || n = 4760 || n = 4761 || n = 4763)
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
.elseif(n = 4692 || n = 4712 || n = 4732 || n = 4752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4693 || n = 4701 || n = 4708 || n = 4715 || n = 4723 || n = 4725 || n = 4735 || n = 4746 || n = 4765)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4695)
	; 13 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4696 || n = 4718 || n = 4728 || n = 4738)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4697 || n = 4699 || n = 4719 || n = 4729 || n = 4739 || n = 4759)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4698 || n = 4720 || n = 4742 || n = 4764)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4702)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4703)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4705 || n = 4724 || n = 4743)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4706)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4710)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4716 || n = 4758)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4717 || n = 4757)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4722)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4731)
	; 14 bytes
	pha
	tya
	ldy #143
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4733)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	tax
	pla
.elseif(n = 4737)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4740)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4755)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4762)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4766 to 4843
.if(n = 4766 || n = 4774 || n = 4776 || n = 4783 || n = 4789 || n = 4794 || n = 4796 || n = 4802 || n = 4814 || n = 4816 || n = 4834 || n = 4840)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767)
	; 14 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4768 || n = 4778 || n = 4798)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4769 || n = 4799 || n = 4839)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4770 || n = 4771 || n = 4773 || n = 4780 || n = 4793 || n = 4801 || n = 4804 || n = 4807 || n = 4809 || n = 4823 || n = 4824 || n = 4826 || n = 4828 || n = 4829 || n = 4831 || n = 4843)
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
.elseif(n = 4772 || n = 4792 || n = 4812 || n = 4832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4775 || n = 4784 || n = 4790 || n = 4795 || n = 4803 || n = 4811 || n = 4813 || n = 4815 || n = 4820 || n = 4822 || n = 4833 || n = 4835 || n = 4841)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4777 || n = 4797)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4779 || n = 4821 || n = 4842)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4781 || n = 4800 || n = 4819 || n = 4838)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4782)
	; 14 bytes
	pha
	tya
	ldy #159
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4786 || n = 4808 || n = 4830)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4787 || n = 4837)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4788 || n = 4836)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4791 || n = 4818)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4805)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 30
	Jne *-7
	tay
	pla
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
.elseif(n = 4810)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4817)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4825)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
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
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
; 4844 to 4921
.if(n = 4844 || n = 4846 || n = 4847 || n = 4849 || n = 4853 || (n > 4865 && n < 4872) || n = 4873 || n = 4880 || n = 4883 || n = 4886 || n = 4888 || n = 4891 || n = 4893 || n = 4900 || n = 4901 || n = 4903 || n = 4904 || n = 4906 || n = 4907 || n = 4909 || n = 4911)
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
.elseif(n = 4845)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4848 || n = 4879 || n = 4910)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4850 || n = 4854 || n = 4856 || n = 4859 || n = 4864 || n = 4878 || n = 4881 || n = 4889 || n = 4894 || n = 4898 || n = 4914 || n = 4916 || n = 4920)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4851 || n = 4855 || n = 4875 || n = 4890 || n = 4895 || n = 4913 || n = 4915 || n = 4921)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4852 || n = 4872 || n = 4892 || n = 4912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4857)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4858)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4860 || n = 4908)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4861)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 4862 || n = 4887)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4863 || n = 4884 || n = 4905)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4865 || n = 4885)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4874 || n = 4896 || n = 4918)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4876)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4877 || n = 4897 || n = 4917)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4882)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 14
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4899 || n = 4919)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4902)
	; 14 bytes
	pha
	tya
	ldy #163
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 4922 to 5000
.if(n = 4922 || n = 4923 || n = 4928 || n = 4930 || n = 4931 || n = 4933 || n = 4946 || (n > 4948 && n < 4952) || n = 4960 || n = 4961 || n = 4963 || n = 4966 || n = 4967 || n = 4969 || n = 4971 || n = 4981 || n = 4988 || n = 4991 || n = 4993 || n = 5000)
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
.elseif(n = 4924)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4925 || n = 4945 || n = 4965 || n = 4985)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4926 || n = 4947 || n = 4968 || n = 4989)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4927 || n = 4935 || n = 4944 || n = 4955 || n = 4973 || n = 4990 || n = 4995)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4929)
	; 14 bytes
	pha
	tya
	ldy #149
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4934 || n = 4936 || n = 4942 || n = 4943 || n = 4954 || n = 4964 || n = 4974 || n = 4986 || n = 4994 || n = 4996)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4937 || n = 4987)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4938 || n = 4958 || n = 4976 || n = 4998)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4939 || n = 4959 || n = 4977 || n = 4979 || n = 4999)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4940 || n = 4962 || n = 4984)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4941)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4948)
	; 14 bytes
	pha
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 4953)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4956 || n = 4980)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4957 || n = 4997)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4970)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 4978)
	; 14 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4982)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4983)
	; 13 bytes
	brk
	delay_n (n-13)
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
.if(n = 5001 || n = 5002 || n = 5011 || n = 5013 || n = 5020 || n = 5021 || n = 5023 || n = 5024 || n = 5026 || n = 5027 || n = 5029 || n = 5033 || (n > 5040 && n < 5045) || (n > 5045 && n < 5050) || n = 5051 || n = 5060 || n = 5063 || n = 5066 || (n > 5068 && n < 5072))
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
.elseif(n = 5003 || n = 5065)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5004 || n = 5076)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5005 || n = 5008 || n = 5014 || n = 5016 || n = 5030 || n = 5034 || n = 5036 || n = 5054 || n = 5056 || n = 5064 || n = 5067 || n = 5074)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5006 || n = 5028 || n = 5050)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5007 || n = 5061)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5009 || n = 5015 || n = 5035 || n = 5040 || n = 5053 || n = 5055 || n = 5068 || n = 5075)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5010 || n = 5031 || n = 5073)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5017 || n = 5057 || n = 5077)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5018 || n = 5038 || n = 5058 || n = 5078)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5019 || n = 5039 || n = 5059)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5022)
	; 14 bytes
	pha
	tya
	ldy #167
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5025 || n = 5045)
	; 14 bytes
	brk
	pha
	tya
	ldy #(n-25)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5037 || n = 5062)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 5079 to 5156
.if(n = 5079 || n = 5099 || n = 5108 || n = 5119 || n = 5128 || n = 5139)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5080 || n = 5081 || n = 5083 || n = 5084 || n = 5086 || n = 5093 || n = 5102 || n = 5122 || n = 5143 || n = 5144 || n = 5146 || n = 5147 || (n > 5148 && n < 5157))
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
.elseif(n = 5082)
	; 14 bytes
	pha
	tya
	ldy #169
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.elseif(n = 5087 || n = 5137)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5088 || n = 5090 || n = 5095 || n = 5104 || n = 5111 || n = 5115 || n = 5117 || n = 5124 || n = 5131 || n = 5135 || n = 5141)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5089 || n = 5098 || n = 5103 || n = 5105 || n = 5106 || n = 5109 || n = 5110 || n = 5118 || n = 5123 || n = 5129 || n = 5130 || n = 5134 || n = 5140)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5091 || n = 5107 || n = 5120)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5092 || n = 5112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5094 || n = 5116 || n = 5138)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5096)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5097)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5100 || n = 5148)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
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
.elseif(n = 5113 || n = 5133)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5114)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
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
.elseif(n = 5132)
	; 12 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5136)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5142)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5145)
	; 14 bytes
	brk
	pha
	tya
	ldy #0
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5178 || n = 5199)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5158)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5159 || (n > 5165 && n < 5169) || n = 5170 || (n > 5172 && n < 5176) || n = 5177 || (n > 5178 && n < 5182) || n = 5183 || n = 5186 || n = 5188 || n = 5195 || n = 5198 || n = 5200 || n = 5203 || n = 5205 || n = 5208 || n = 5211 || n = 5213 || n = 5217 || n = 5219 || n = 5224 || n = 5225 || n = 5227 || n = 5230 || n = 5231 || n = 5233 || n = 5234)
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
.elseif(n = 5160 || n = 5182 || n = 5204 || n = 5226)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5161 || n = 5185 || n = 5190 || n = 5192 || n = 5197 || n = 5207 || n = 5215 || n = 5221 || n = 5229)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5162 || n = 5187 || n = 5212)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5163)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5164 || n = 5184 || n = 5191 || n = 5206 || n = 5214 || n = 5222 || n = 5228)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5165)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5169 || n = 5223)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5171)
	; 14 bytes
	brk
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5172 || n = 5196)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5176)
	; 14 bytes
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 5189 || n = 5220)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5193)
	; 14 bytes
	pha
	tya
	ldy #157
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5194)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5201)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	brk
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5202)
	; 14 bytes
	pha
	tya
	ldy #173
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5209)
	; 14 bytes
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 5210)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5216)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	delay_n 26
	Jne *-7
	tay
	pla
.elseif(n = 5218)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5232)
	; 14 bytes
	pha
	tya
	ldy #174
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5235 to 5312
.if(n = 5235 || n = 5236 || n = 5238 || n = 5242 || n = 5243 || (n > 5244 && n < 5248) || n = 5249 || n = 5252 || (n > 5254 && n < 5259) || n = 5260 || n = 5261 || n = 5263 || n = 5266 || n = 5267 || n = 5269 || n = 5271 || (n > 5273 && n < 5277) || (n > 5278 && n < 5282) || n = 5286 || n = 5291 || n = 5293 || (n > 5295 && n < 5304) || (n > 5304 && n < 5310) || n = 5311)
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
.elseif(n = 5237 || n = 5262 || n = 5287 || n = 5312)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5239 || n = 5253 || n = 5264 || n = 5272 || n = 5284 || n = 5289 || n = 5294)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5240 || n = 5254 || n = 5265 || n = 5273 || n = 5285 || n = 5290 || n = 5295)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5241 || n = 5283 || n = 5304)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5244 || n = 5268)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5248 || n = 5270 || n = 5292)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5250 || n = 5277)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5251 || n = 5282)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5259)
	; 14 bytes
	pha
	tya
	ldy #159
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5278)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 5288)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	.repeat 3
	 php
	 plp
	.endrepeat
	Jne *-7
	tax
	pla
.elseif(n = 5310)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	pla
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
; 5313 to 5468
.if(n = 5313 || n = 5344 || n = 5375 || n = 5406 || n = 5437)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5314 || n = 5336 || n = 5358 || n = 5380 || n = 5402 || n = 5424 || n = 5446 || n = 5468)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5315 || n = 5338 || n = 5339 || n = 5360 || n = 5377 || n = 5382 || n = 5389 || n = 5404 || n = 5408 || n = 5414 || n = 5426 || n = 5448 || n = 5464)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5316 || n = 5340 || n = 5364 || n = 5436 || n = 5460)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5317 || n = 5347 || n = 5361 || n = 5378 || n = 5383 || n = 5390 || n = 5405 || n = 5409 || n = 5413 || n = 5415 || n = 5427 || n = 5440 || n = 5449 || n = 5465)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n > 5317 && n < 5322) || n = 5324 || (n > 5325 && n < 5331) || (n > 5331 && n < 5336) || (n > 5340 && n < 5344) || n = 5345 || (n > 5347 && n < 5352) || (n > 5352 && n < 5358) || n = 5359 || n = 5366 || (n > 5367 && n < 5375) || n = 5376 || n = 5379 || n = 5381 || n = 5386 || (n > 5392 && n < 5402) || n = 5403 || n = 5407 || n = 5411 || (n > 5415 && n < 5424) || n = 5425 || (n > 5427 && n < 5436) || n = 5438 || n = 5441 || (n > 5442 && n < 5446) || n = 5447 || (n > 5449 && n < 5457) || n = 5458 || n = 5459 || n = 5461 || n = 5463 || n = 5467)
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
.elseif(n = 5322)
	; 14 bytes
	pha
	tya
	ldy #177
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5323)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 5325 || n = 5346 || n = 5367)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5331 || n = 5385 || n = 5439)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5337 || n = 5362 || n = 5387 || n = 5412 || n = 5462)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5352)
	; 14 bytes
	pha
	tya
	ldy #178
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5365)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	dex
	dex
	Jne *-7
	tax
	pla
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
.elseif(n = 5388)
	; 14 bytes
	pha
	tya
	ldy #0
	nop
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5391)
	; 14 bytes
	pha
	tya
	ldy #163
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5392)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5410)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5442)
	; 14 bytes
	pha
	tya
	ldy #181
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5457)
	; 14 bytes
	pha
	tya
	ldy #165
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5466)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 5469 to 5625
.if(n = 5469 || n = 5494 || n = 5515 || n = 5533 || n = 5540 || n = 5557 || n = 5559 || n = 5565 || n = 5581 || n = 5590 || n = 5595 || n = 5603 || n = 5614)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5470 || n = 5489 || n = 5492 || n = 5501 || n = 5514 || n = 5536 || n = 5539 || n = 5558 || n = 5563 || n = 5564 || n = 5594 || n = 5602 || n = 5613 || n = 5615 || n = 5616 || n = 5624 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5471)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5472)
	; 14 bytes
	pha
	tya
	ldy #182
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5473 || n = 5618)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 5473 && n < 5479) || (n > 5479 && n < 5484) || n = 5485 || n = 5486 || n = 5488 || (n > 5494 && n < 5499) || n = 5500 || (n > 5502 && n < 5508) || (n > 5508 && n < 5512) || n = 5513 || (n > 5515 && n < 5520) || n = 5521 || n = 5522 || (n > 5523 && n < 5530) || n = 5531 || n = 5535 || n = 5538 || (n > 5540 && n < 5547) || (n > 5547 && n < 5554) || n = 5555 || n = 5560 || (n > 5565 && n < 5574) || (n > 5574 && n < 5578) || n = 5579 || (n > 5581 && n < 5587) || n = 5588 || n = 5591 || n = 5593 || (n > 5595 && n < 5600) || (n > 5604 && n < 5611) || (n > 5618 && n < 5622))
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
.elseif(n = 5479)
	; 14 bytes
	brk
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5484 || n = 5508 || n = 5532 || n = 5580 || n = 5604)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5487 || n = 5537 || n = 5562 || n = 5587 || n = 5612)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5490 || n = 5512 || n = 5534 || n = 5556 || n = 5578 || n = 5600 || n = 5622)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5491)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tax
	pla
.elseif(n = 5493 || n = 5520 || n = 5547 || n = 5574 || n = 5601)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5499 || n = 5530 || n = 5561 || n = 5592 || n = 5623)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5502)
	; 14 bytes
	pha
	tya
	ldy #183
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5523)
	; 14 bytes
	pha
	tya
	ldy #167
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5554)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 5589)
	; 14 bytes
	pha
	tya
	ldy #169
	nop
	delay_n 26
	dey
	Jne *-6
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
.elseif(n = 5617)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
; 5626 to 5781
.if(n = 5626)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5627 || (n > 5629 && n < 5633) || n = 5634 || (n > 5640 && n < 5644) || n = 5645 || (n > 5647 && n < 5652) || n = 5653 || (n > 5657 && n < 5662) || n = 5663 || (n > 5665 && n < 5676) || (n > 5676 && n < 5682) || n = 5683 || n = 5684 || n = 5686 || (n > 5690 && n < 5694) || (n > 5694 && n < 5700) || (n > 5700 && n < 5709) || n = 5710 || n = 5711 || n = 5713 || n = 5717 || n = 5720 || n = 5722 || n = 5723 || (n > 5724 && n < 5736) || n = 5738 || n = 5741 || (n > 5742 && n < 5747) || (n > 5750 && n < 5754) || (n > 5754 && n < 5762) || (n > 5765 && n < 5772) || (n > 5772 && n < 5778) || n = 5779)
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
.elseif(n = 5628 || n = 5652 || n = 5676 || n = 5700 || n = 5724 || n = 5748 || n = 5772)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5629 || n = 5636 || n = 5647 || n = 5657 || n = 5665 || n = 5690 || n = 5715 || n = 5719 || n = 5740 || n = 5750 || n = 5765)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 5635 || n = 5646 || n = 5656 || n = 5664 || n = 5689 || n = 5718 || n = 5739 || n = 5749 || n = 5764 || n = 5780)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5637 || n = 5662 || n = 5687 || n = 5712 || n = 5737 || n = 5762)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
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
	delay_n (n-7)
	plp
.elseif(n = 5644)
	; 13 bytes
	pha
	tya
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5654 || n = 5685 || n = 5716 || n = 5747 || n = 5778)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5655 || n = 5682 || n = 5709 || n = 5736 || n = 5763)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5688)
	; 14 bytes
	pha
	tya
	ldy #172
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5694)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	php
	cmp $C5
	plp
	brk
	Jne *-7
	tax
	pla
.elseif(n = 5714)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5721)
	; 14 bytes
	pha
	tya
	ldy #173
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5742)
	; 14 bytes
	pha
	tya
	ldy #191
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5754)
	; 14 bytes
	pha
	tya
	ldy #174
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5781)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	brk
	dey
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 5782 to 5937
.if((n > 5781 && n < 5787) || n = 5788 || (n > 5790 && n < 5796) || (n > 5796 && n < 5802) || (n > 5802 && n < 5809) || n = 5813 || n = 5816 || n = 5818 || n = 5819 || (n > 5820 && n < 5832) || (n > 5832 && n < 5837) || n = 5838 || n = 5841 || (n > 5845 && n < 5853) || (n > 5853 && n < 5862) || n = 5863 || n = 5866 || n = 5867 || n = 5869 || n = 5870 || n = 5872 || (n > 5874 && n < 5886) || n = 5888 || n = 5891 || (n > 5892 && n < 5897) || (n > 5898 && n < 5902) || n = 5903 || (n > 5905 && n < 5912) || n = 5913 || n = 5917 || n = 5918 || n = 5920 || n = 5921 || n = 5923 || n = 5924 || (n > 5925 && n < 5933) || n = 5934)
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
.elseif(n = 5787 || n = 5812 || n = 5837 || n = 5862 || n = 5887 || n = 5912 || n = 5937)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5789 || n = 5811 || n = 5814 || n = 5839 || n = 5864 || n = 5873 || n = 5889 || n = 5904 || n = 5914 || n = 5935)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5790 || n = 5817 || n = 5898 || n = 5925)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5796 || n = 5820 || n = 5844 || n = 5868 || n = 5892 || n = 5916)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5802)
	; 14 bytes
	pha
	tya
	ldy #193
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5809 || n = 5840 || n = 5871 || n = 5902 || n = 5933)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5810)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5815 || n = 5843 || n = 5845 || n = 5865 || n = 5874 || n = 5890 || n = 5905 || n = 5915 || n = 5936)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5832)
	; 14 bytes
	pha
	tya
	ldy #194
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5842)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5853)
	; 14 bytes
	pha
	tya
	ldy #177
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5886)
	; 14 bytes
	pha
	tya
	ldy #178
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5897)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
	pla
.elseif(n = 5919)
	; 14 bytes
	pha
	tya
	ldy #179
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5922)
	; 14 bytes
	pha
	tya
	ldy #197
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938 || (n > 5940 && n < 5952) || (n > 5952 && n < 5962) || n = 5963 || (n > 5967 && n < 5979) || n = 5980 || n = 5981 || n = 5983 || n = 5984 || n = 5986 || (n > 5990 && n < 5995) || n = 5996 || (n > 5998 && n < 6006) || (n > 6006 && n < 6012) || n = 6013 || n = 6017 || (n > 6018 && n < 6026) || n = 6027 || (n > 6029 && n < 6033) || n = 6034 || n = 6035 || n = 6041 || (n > 6042 && n < 6051) || (n > 6051 && n < 6057) || n = 6058 || n = 6061 || n = 6063 || (n > 6065 && n < 6072) || (n > 6072 && n < 6084) || n = 6085 || n = 6086 || n = 6092 || n = 6093)
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
.elseif(n = 5939 || n = 5966 || n = 5989 || n = 5997 || n = 6014 || n = 6028 || n = 6039 || n = 6059 || n = 6064 || n = 6089 || n = 6090)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5940 || n = 5988 || n = 6036 || n = 6060 || n = 6084)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5952 || n = 5979 || n = 6006 || n = 6033)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5962)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5964 || n = 5995 || n = 6026 || n = 6057 || n = 6088)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5965 || n = 5967 || n = 5990 || n = 5998 || n = 6015 || n = 6029 || n = 6040 || n = 6065 || n = 6091)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5982)
	; 14 bytes
	pha
	tya
	ldy #199
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5985)
	; 14 bytes
	pha
	tya
	ldy #181
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5987 || n = 6012 || n = 6037 || n = 6062 || n = 6087)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6016)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 6018)
	; 14 bytes
	pha
	tya
	ldy #182
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6038)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 39
	Jne *-7
	tay
	pla
.elseif(n = 6042)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6051)
	; 14 bytes
	pha
	tya
	ldy #183
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6072)
	; 14 bytes
	pha
	tya
	ldy #202
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6094 to 6250
.if(n = 6094)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif((n > 6094 && n < 6100) || n = 6101 || (n > 6103 && n < 6108) || (n > 6108 && n < 6112) || n = 6113 || n = 6116 || n = 6118 || n = 6120 || (n > 6124 && n < 6132) || (n > 6132 && n < 6137) || n = 6138 || (n > 6141 && n < 6145) || n = 6146 || n = 6149 || n = 6151 || n = 6154 || n = 6155 || (n > 6156 && n < 6162) || n = 6163 || n = 6166 || n = 6167 || (n > 6168 && n < 6181) || n = 6182 || n = 6185 || n = 6186 || n = 6188 || n = 6191 || n = 6193 || n = 6194 || (n > 6195 && n < 6212) || n = 6213 || (n > 6216 && n < 6222) || (n > 6222 && n < 6237) || n = 6238 || n = 6241 || n = 6242 || n = 6244 || n = 6247 || n = 6248 || n = 6250)
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
.elseif(n = 6100)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk
	Jne *-6
	tax
	pla
.elseif(n = 6102)
	; 14 bytes
	pha
	tya
	ldy #203
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6103 || n = 6115 || n = 6122 || n = 6124 || n = 6140 || n = 6148 || n = 6153 || n = 6165 || n = 6184 || n = 6190 || n = 6215 || n = 6240 || n = 6246)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6108 || n = 6132)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6112 || n = 6137 || n = 6162 || n = 6187 || n = 6237)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6114 || n = 6141 || n = 6168 || n = 6195 || n = 6222 || n = 6249)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6117)
	; 14 bytes
	pha
	tya
	ldy #185
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6119 || n = 6150 || n = 6181 || n = 6212 || n = 6243)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6121)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6123 || n = 6139 || n = 6147 || n = 6152 || n = 6164 || n = 6189 || n = 6214 || n = 6239 || n = 6245)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6145)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6156)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6183)
	; 14 bytes
	pha
	tya
	ldy #187
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6192)
	; 14 bytes
	pha
	tya
	ldy #206
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6216)
	; 14 bytes
	pha
	tya
	ldy #188
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
.if((n >= 6251) && (n < 6407))
; 6251 to 6406
.if(n = 6251 || (n > 6252 && n < 6262) || n = 6263 || (n > 6265 && n < 6274) || (n > 6277 && n < 6282) || (n > 6282 && n < 6287) || n = 6288 || (n > 6290 && n < 6303) || n = 6304 || n = 6311 || n = 6313 || (n > 6316 && n < 6328) || n = 6329 || (n > 6331 && n < 6336) || n = 6341 || (n > 6342 && n < 6348) || (n > 6348 && n < 6357) || (n > 6357 && n < 6362) || n = 6363 || n = 6366 || n = 6368 || n = 6371 || (n > 6372 && n < 6376) || n = 6377 || n = 6385 || n = 6386 || n = 6388 || (n > 6390 && n < 6398) || n = 6399)
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
.elseif(n = 6252)
	; 14 bytes
	pha
	tya
	ldy #208
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6262 || n = 6287 || n = 6312 || n = 6337 || n = 6362 || n = 6387)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6264 || n = 6289 || n = 6307 || n = 6314 || n = 6338 || n = 6339 || n = 6364 || n = 6369 || n = 6378 || n = 6380 || n = 6389 || n = 6400 || n = 6403 || n = 6405)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6265 || n = 6277 || n = 6290 || n = 6306 || n = 6308 || n = 6331 || n = 6340 || n = 6365 || n = 6370 || n = 6379 || n = 6390 || n = 6404)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6274 || n = 6305 || n = 6336 || n = 6367 || n = 6398)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6275)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 26
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 6276 || n = 6330 || n = 6357 || n = 6384)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6282)
	; 14 bytes
	pha
	tya
	ldy #190
	nop
	delay_n 26
	dey
	Jne *-6
	tay
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
.elseif(n = 6309 || n = 6382)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6310 || n = 6383)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6315)
	; 14 bytes
	pha
	tya
	ldy #191
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6316)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 6328)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6342)
	; 14 bytes
	pha
	tya
	ldy #211
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6348)
	; 14 bytes
	pha
	tya
	ldy #192
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6372)
	; 14 bytes
	pha
	tya
	ldy #212
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
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
.elseif(n = 6381)
	; 14 bytes
	pha
	tya
	ldy #193
	nop
	delay_n 26
	dey
	Jne *-6
	tay
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
.elseif(n = 6402)
	; 14 bytes
	pha
	tya
	ldy #213
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.endif
.else
; 6407 to 6562
.if(n = 6407)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6408)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6409 || n = 6413 || (n > 6415 && n < 6429) || n = 6430 || (n > 6432 && n < 6438) || (n > 6438 && n < 6447) || (n > 6447 && n < 6458) || n = 6459 || n = 6461 || n = 6464 || (n > 6465 && n < 6480) || (n > 6480 && n < 6491) || (n > 6494 && n < 6506) || (n > 6506 && n < 6513) || (n > 6513 && n < 6519) || n = 6520 || n = 6521 || n = 6523 || (n > 6525 && n < 6546) || (n > 6546 && n < 6552) || n = 6554 || (n > 6556 && n < 6563))
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
.elseif(n = 6410)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6411 || n = 6438 || n = 6465 || n = 6492 || n = 6519 || n = 6546)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6412)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 7
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6414)
	; 14 bytes
	pha
	tya
	ldy #194
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6415 || n = 6463 || n = 6494 || n = 6525 || n = 6556)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6429 || n = 6460 || n = 6491 || n = 6522 || n = 6553)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6431 || n = 6462 || n = 6493 || n = 6524 || n = 6555)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6432)
	; 14 bytes
	pha
	tya
	ldy #214
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6447)
	; 14 bytes
	pha
	tya
	ldy #195
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6458)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6480)
	; 14 bytes
	pha
	tya
	ldy #196
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6506)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
	tax
	pla
.elseif(n = 6513)
	; 14 bytes
	pha
	tya
	ldy #197
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6552)
	; 14 bytes
	pha
	tya
	ldy #218
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
; 6563 to 6718
.if((n > 6562 && n < 6573) || (n > 6573 && n < 6579) || n = 6580 || n = 6581 || n = 6583 || n = 6585 || (n > 6587 && n < 6600) || (n > 6600 && n < 6610) || n = 6611 || n = 6614 || n = 6616 || (n > 6618 && n < 6627) || (n > 6627 && n < 6631) || (n > 6631 && n < 6642) || n = 6643 || n = 6644 || n = 6647 || (n > 6649 && n < 6654) || n = 6655 || n = 6656 || (n > 6657 && n < 6672) || (n > 6672 && n < 6677) || (n > 6681 && n < 6702) || (n > 6702 && n < 6706) || n = 6707 || (n > 6712 && n < 6719))
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
.elseif(n = 6573 || n = 6600 || n = 6627 || n = 6654 || n = 6681)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6579)
	; 14 bytes
	pha
	tya
	ldy #199
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6582)
	; 14 bytes
	pha
	tya
	ldy #219
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6584 || n = 6615 || n = 6646 || n = 6677 || n = 6708)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6586 || n = 6617 || n = 6648 || n = 6679 || n = 6710)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6587 || n = 6613 || n = 6618 || n = 6649 || n = 6680 || n = 6712)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6610)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6612)
	; 14 bytes
	pha
	tya
	ldy #200
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6631)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6642)
	; 14 bytes
	pha
	tya
	ldy #221
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6645)
	; 14 bytes
	pha
	tya
	ldy #201
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6657)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6672)
	; 14 bytes
	pha
	tya
	ldy #222
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6678)
	; 14 bytes
	pha
	tya
	ldy #202
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6702)
	; 14 bytes
	pha
	tya
	ldy #223
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6706)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6709)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 26
	Jne *-6
	tax
	pla
.elseif(n = 6711)
	; 14 bytes
	pha
	tya
	ldy #203
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6719 to 6875
.if((n > 6718 && n < 6732) || n = 6733 || n = 6734 || n = 6736 || n = 6737 || n = 6740 || n = 6743 || (n > 6744 && n < 6762) || (n > 6762 && n < 6770) || n = 6771 || (n > 6773 && n < 6777) || (n > 6777 && n < 6789) || n = 6790 || n = 6791 || (n > 6792 && n < 6796) || (n > 6796 && n < 6801) || n = 6802 || (n > 6804 && n < 6810) || (n > 6810 && n < 6816) || (n > 6816 && n < 6822) || (n > 6822 && n < 6832) || n = 6833 || (n > 6835 && n < 6843) || (n > 6843 && n < 6852) || (n > 6852 && n < 6863) || n = 6864 || (n > 6866 && n < 6870) || (n > 6870 && n < 6876))
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
.elseif(n = 6732)
	; 14 bytes
	pha
	tya
	ldy #224
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6735 || n = 6762 || n = 6789 || n = 6816 || n = 6843 || n = 6870)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6738)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6739 || n = 6770 || n = 6801 || n = 6832 || n = 6863)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6741 || n = 6772 || n = 6803 || n = 6834 || n = 6865)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6742 || n = 6773 || n = 6804 || n = 6835 || n = 6866)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6744)
	; 14 bytes
	pha
	tya
	ldy #204
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6777)
	; 14 bytes
	pha
	tya
	ldy #205
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6792)
	; 14 bytes
	pha
	tya
	ldy #226
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6796)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6810)
	; 14 bytes
	pha
	tya
	ldy #206
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6822)
	; 14 bytes
	pha
	tya
	ldy #227
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6852)
	; 14 bytes
	pha
	tya
	ldy #228
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
; 6876 to 7031
.if(n = 6876)
	; 14 bytes
	pha
	tya
	ldy #208
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif((n > 6876 && n < 6882) || (n > 6882 && n < 6886) || n = 6887 || (n > 6889 && n < 6894) || n = 6895 || (n > 6897 && n < 6909) || n = 6910 || n = 6911 || n = 6914 || (n > 6916 && n < 6922) || n = 6923 || n = 6926 || (n > 6928 && n < 6942) || (n > 6942 && n < 6956) || n = 6957 || (n > 6959 && n < 6972) || n = 6973 || n = 6974 || (n > 6975 && n < 6987) || n = 6988 || (n > 6990 && n < 7002) || (n > 7002 && n < 7008) || n = 7009 || (n > 7010 && n < 7018) || n = 7019 || (n > 7021 && n < 7032))
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
.elseif(n = 6882)
	; 14 bytes
	pha
	tya
	ldy #229
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6886)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6888 || n = 6896 || n = 6915 || n = 6927 || n = 6958 || n = 6989 || n = 7020)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6889 || n = 6916 || n = 6928 || n = 6959 || n = 6990 || n = 7021)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6894 || n = 6925 || n = 6956 || n = 6987 || n = 7018)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6897)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6909)
	; 14 bytes
	pha
	tya
	ldy #209
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6912)
	; 14 bytes
	pha
	tya
	ldy #230
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6913)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6922)
	; 14 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6924)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6942)
	; 14 bytes
	pha
	tya
	ldy #210
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6972)
	; 14 bytes
	pha
	tya
	ldy #232
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6975)
	; 14 bytes
	pha
	tya
	ldy #211
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7002)
	; 14 bytes
	pha
	tya
	ldy #233
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7008)
	; 14 bytes
	pha
	tya
	ldy #212
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7010)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 7032 to 7187
.if(n = 7032)
	; 14 bytes
	pha
	tya
	ldy #234
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif((n > 7032 && n < 7041) || (n > 7041 && n < 7049) || n = 7050 || (n > 7052 && n < 7062) || (n > 7062 && n < 7074) || (n > 7074 && n < 7080) || n = 7081 || (n > 7083 && n < 7092) || (n > 7092 && n < 7107) || (n > 7107 && n < 7111) || n = 7112 || (n > 7115 && n < 7122) || (n > 7122 && n < 7138) || n = 7139 || n = 7143 || (n > 7145 && n < 7152) || (n > 7152 && n < 7169) || (n > 7169 && n < 7173) || n = 7174 || (n > 7176 && n < 7182) || (n > 7182 && n < 7188))
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
.elseif(n = 7041)
	; 14 bytes
	pha
	tya
	ldy #213
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7049 || n = 7080 || n = 7111 || n = 7142 || n = 7173)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7051 || n = 7082 || n = 7113 || n = 7144 || n = 7175)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7052 || n = 7083 || n = 7114 || n = 7145 || n = 7176)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7062)
	; 14 bytes
	pha
	tya
	ldy #235
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7074)
	; 14 bytes
	pha
	tya
	ldy #214
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7092)
	; 14 bytes
	pha
	tya
	ldy #236
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7107)
	; 14 bytes
	pha
	tya
	ldy #215
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7115)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	tax
	pla
.elseif(n = 7122)
	; 14 bytes
	pha
	tya
	ldy #237
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7138)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7140)
	; 14 bytes
	pha
	tya
	ldy #216
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7141)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7152)
	; 14 bytes
	pha
	tya
	ldy #238
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7169)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7182)
	; 14 bytes
	pha
	tya
	ldy #239
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 7188 to 7500
.if((n > 7187 && n < 7202) || n = 7203 || n = 7205 || (n > 7207 && n < 7212) || (n > 7212 && n < 7235) || n = 7236 || n = 7240 || n = 7241 || (n > 7242 && n < 7266) || n = 7267 || n = 7270 || n = 7271 || (n > 7272 && n < 7297) || n = 7298 || n = 7301 || n = 7303 || n = 7304 || (n > 7305 && n < 7328) || n = 7329 || (n > 7332 && n < 7338) || (n > 7338 && n < 7359) || n = 7360 || (n > 7362 && n < 7371) || (n > 7371 && n < 7390) || n = 7391 || n = 7394 || n = 7395 || (n > 7396 && n < 7404) || (n > 7404 && n < 7421) || (n > 7425 && n < 7434) || n = 7435 || n = 7436 || (n > 7437 && n < 7452) || n = 7453 || (n > 7455 && n < 7470) || (n > 7470 && n < 7482) || n = 7484 || (n > 7486 && n < 7498) || n = 7499 || n = 7500)
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
.elseif(n = 7202)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7204 || n = 7235 || n = 7266 || n = 7297 || n = 7328 || n = 7359 || n = 7390 || n = 7421 || n = 7452 || n = 7483)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7206)
	; 14 bytes
	pha
	tya
	ldy #218
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7207 || n = 7238 || n = 7269 || n = 7300 || n = 7331 || n = 7393 || n = 7424 || n = 7455 || n = 7486)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7212)
	; 14 bytes
	pha
	tya
	ldy #240
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7237 || n = 7268 || n = 7299 || n = 7330 || n = 7361 || n = 7392 || n = 7423 || n = 7454 || n = 7485)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7239)
	; 14 bytes
	pha
	tya
	ldy #219
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7242)
	; 14 bytes
	pha
	tya
	ldy #241
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7272)
	; 14 bytes
	pha
	tya
	ldy #220
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7302)
	; 14 bytes
	pha
	tya
	ldy #243
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7305)
	; 14 bytes
	pha
	tya
	ldy #221
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7332)
	; 14 bytes
	pha
	tya
	ldy #244
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7338)
	; 14 bytes
	pha
	tya
	ldy #222
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7362)
	; 14 bytes
	pha
	tya
	ldy #245
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7371)
	; 14 bytes
	pha
	tya
	ldy #223
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7396)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 11
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7404)
	; 14 bytes
	pha
	tya
	ldy #224
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7422)
	; 14 bytes
	pha
	tya
	ldy #247
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7425)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7434)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7437)
	; 14 bytes
	pha
	tya
	ldy #225
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7470)
	; 14 bytes
	pha
	tya
	ldy #226
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7482)
	; 14 bytes
	pha
	tya
	ldy #249
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7498)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
; 7501 to 7656
.if(n = 7501 || n = 7502 || (n > 7503 && n < 7512) || n = 7513 || n = 7515 || (n > 7517 && n < 7536) || (n > 7536 && n < 7542) || n = 7543 || n = 7544 || n = 7546 || (n > 7548 && n < 7569) || n = 7571 || (n > 7572 && n < 7576) || n = 7577 || (n > 7579 && n < 7602) || (n > 7602 && n < 7607) || n = 7608 || (n > 7610 && n < 7632) || n = 7633 || n = 7634 || n = 7636 || n = 7637 || n = 7639 || (n > 7641 && n < 7651) || n = 7652 || n = 7655 || n = 7656)
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
.elseif(n = 7503)
	; 14 bytes
	pha
	tya
	ldy #227
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7512)
	; 14 bytes
	pha
	tya
	ldy #250
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7514 || n = 7545 || n = 7576 || n = 7607 || n = 7638)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7516 || n = 7547 || n = 7578 || n = 7609 || n = 7640 || n = 7653)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7517 || n = 7548 || n = 7579 || n = 7610 || n = 7641 || n = 7654)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7536)
	; 14 bytes
	pha
	tya
	ldy #228
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7542)
	; 14 bytes
	pha
	tya
	ldy #251
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7569)
	; 14 bytes
	pha
	tya
	ldy #229
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7570)
	; 14 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7572)
	; 14 bytes
	pha
	tya
	ldy #252
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7602)
	; 14 bytes
	pha
	tya
	ldy #230
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7632)
	; 14 bytes
	pha
	tya
	ldy #254
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7635)
	; 14 bytes
	pha
	tya
	ldy #231
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7651)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 7657 to 7812
.if((n > 7656 && n < 7662) || (n > 7662 && n < 7666) || n = 7667 || n = 7670 || (n > 7672 && n < 7681) || n = 7682 || (n > 7684 && n < 7692) || (n > 7692 && n < 7700) || (n > 7703 && n < 7724) || (n > 7724 && n < 7731) || (n > 7735 && n < 7762) || n = 7763 || n = 7766 || (n > 7767 && n < 7793) || n = 7794 || (n > 7796 && n < 7800) || (n > 7800 && n < 7813))
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
.elseif(n = 7662)
	; 14 bytes
	pha
	tya
	ldy #255
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7666)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 7668)
	; 14 bytes
	pha
	tya
	ldy #232
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7669 || n = 7700 || n = 7731 || n = 7762 || n = 7793)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7671 || n = 7683 || n = 7702 || n = 7733 || n = 7764 || n = 7795)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7672 || n = 7684 || n = 7703 || n = 7735 || n = 7765 || n = 7796)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7681)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7692)
	; 14 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7701)
	; 14 bytes
	pha
	tya
	ldy #233
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7724)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 26
	Jne *-7
	tax
	pla
.elseif(n = 7732)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7734)
	; 14 bytes
	pha
	tya
	ldy #234
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7767)
	; 14 bytes
	pha
	tya
	ldy #235
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7800)
	; 14 bytes
	pha
	tya
	ldy #236
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 7813 to 8125
.if((n > 7812 && n < 7824) || n = 7825 || (n > 7827 && n < 7833) || (n > 7833 && n < 7855) || n = 7856 || (n > 7858 && n < 7866) || (n > 7866 && n < 7886) || n = 7887 || (n > 7889 && n < 7899) || (n > 7899 && n < 7906) || n = 7907 || (n > 7913 && n < 7917) || n = 7918 || (n > 7920 && n < 7931) || (n > 7933 && n < 7937) || n = 7938 || n = 7945 || n = 7947 || n = 7949 || (n > 7951 && n < 7965) || (n > 7965 && n < 7998) || (n > 7998 && n < 8031) || (n > 8031 && n < 8064) || (n > 8064 && n < 8095) || n = 8096 || (n > 8097 && n < 8126))
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
.elseif(n = 7824 || n = 7855 || n = 7886 || n = 7917)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7826 || n = 7857 || n = 7888 || n = 7908 || n = 7910 || n = 7911 || n = 7919 || n = 7939 || n = 7941 || n = 7950)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7827 || n = 7858 || n = 7889 || n = 7909 || n = 7920 || n = 7940 || n = 7951)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7833)
	; 14 bytes
	pha
	tya
	ldy #237
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7866)
	; 14 bytes
	pha
	tya
	ldy #238
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7899)
	; 14 bytes
	pha
	tya
	ldy #239
	nop
	delay_n 26
	dey
	Jne *-6
	tay
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
.elseif(n = 7912 || n = 7943)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7913 || n = 7944)
	; 14 bytes
	php
	delay_n (n-7)
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
.elseif(n = 7932)
	; 14 bytes
	pha
	tya
	ldy #240
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7946)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7948)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7965)
	; 14 bytes
	pha
	tya
	ldy #241
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7998)
	; 14 bytes
	pha
	tya
	ldy #242
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8031)
	; 14 bytes
	pha
	tya
	ldy #243
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8064)
	; 14 bytes
	pha
	tya
	ldy #244
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8095)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 26
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 8097)
	; 14 bytes
	pha
	tya
	ldy #245
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 8126 to 8750
.if((n > 8125 && n < 8130) || (n > 8130 && n < 8161) || n = 8162 || (n > 8163 && n < 8193) || n = 8195 || n = 8198 || n = 8199 || (n > 8200 && n < 8229) || (n > 8229 && n < 8262) || (n > 8262 && n < 8295) || (n > 8295 && n < 8328) || (n > 8328 && n < 8361) || (n > 8361 && n < 8394) || (n > 8394 && n < 8416) || n = 8417 || (n > 8419 && n < 8427) || (n > 8427 && n < 8449) || n = 8450 || (n > 8452 && n < 8460) || (n > 8460 && n < 8690) || (n > 8690 && n < 8751))
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
.elseif(n = 8130)
	; 14 bytes
	pha
	tya
	ldy #246
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8161)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8163)
	; 14 bytes
	pha
	tya
	ldy #247
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8193)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8194)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8196)
	; 14 bytes
	pha
	tya
	ldy #248
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8197 || n = 8419 || n = 8452)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 8200)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8229)
	; 14 bytes
	pha
	tya
	ldy #249
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8262)
	; 14 bytes
	pha
	tya
	ldy #250
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8295)
	; 14 bytes
	pha
	tya
	ldy #251
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8328)
	; 14 bytes
	pha
	tya
	ldy #252
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8361)
	; 14 bytes
	pha
	tya
	ldy #253
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8394)
	; 14 bytes
	pha
	tya
	ldy #254
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8416)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 8418 || n = 8451)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8427)
	; 14 bytes
	pha
	tya
	ldy #255
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8449)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8460)
	; 14 bytes
	pha
	tya
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8690)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8926) || (n > 8926 && n < 8942) || (n > 8942 && n < 8961) || (n > 8961 && n < 9691) || (n > 9691 && n < 9729) || (n > 9729 && n < 10001))
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
.elseif(n = 8926)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8942)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
	pla
.elseif(n = 8961)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9691)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9729)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 11221) || (n > 11221 && n < 11265) || (n > 11265 && n < 20001))
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
.elseif(n = 11221)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 11265)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.endif
.endif

.endif
	rest
.endmacro
