; delay_n, delays n cycles with small code
; Will clobber A, S, and Z+N
; Will preserve X, Y, C, D, V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires a function called "delay_256a_16_clocks"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A*256 clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A*256+16 clocks (including JSR)
;; Depends on delay_a_25_clocks
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_256a_16_clocks:
;	cmp #0
;	bne :+
;	rts
;delay_256a_11_clocks_:
;:       pha
;	 lda #(256-25-7-2-2-3)
;	 jsr delay_a_25_clocks
;	pla
;	sec
;	sbc #1
;	bne :-
;	rts
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
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 16
; - 5 bytes or less when n <= 20
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 28
; - 8 bytes or less when n <= 310
; - 9 bytes or less when n <= 1293
; -10 bytes or less when n <= 1893
; -11 bytes or less when n <= 2403
; -12 bytes or less when n <= 3317
; -13 bytes or less when n <= 4083
; -14 bytes or less when n <= 4865
; -15 bytes or less when n <= 5125
; -16 bytes or less when n <= 9485
; -17 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=4.2, max=5
; - When 21 <= n <= 30: min=6, avg=6.7, max=8
; - When 31 <= n <= 50: min=6, avg=7.0, max=8
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.0, max=8
; - When 301 <= n <= 500: min=6, avg=7.9, max=9
; - When 501 <= n <= 750: min=6, avg=7.9, max=9
; - When 751 <= n <= 1000: min=7, avg=7.9, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.6, max=11
; - When 2001 <= n <= 3000: min=7, avg=9.9, max=12
; - When 3001 <= n <= 5000: min=7, avg=11.4, max=15
; - When 5001 <= n <= 7500: min=7, avg=12.2, max=16
; - When 7501 <= n <= 10000: min=7, avg=12.4, max=17
; - When 10001 <= n <= 20000: min=7, avg=12.4, max=17
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
.elseif(n = 5 || n = 18 || n = 22 || n = 26 || n = 32)
	; 2..8 bytes
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
.elseif(n = 8 || n = 12 || n = 19)
	; 2..5 bytes
	delay_n (n-4)
	pla
.elseif(n = 9 || n = 10 || n = 13)
	; 3..4 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 4 bytes
	pla
	pha
	pla
	pla
.elseif(n = 16 || n = 20 || n = 24)
	; 4..6 bytes
	.repeat n/4
	 pla
	.endrepeat
.elseif(n = 17)
	; 5 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 23)
	; 6 bytes
	pla
	pha
	delay_n (n-7)
.elseif(n = 25)
	; 7 bytes
	php
	lda #0
	jsr delay_256a_16_clocks
	plp
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
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 31)
	; 6 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 33)
	; 7 bytes
	php
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif((n > 33 && n < 46) || (n > 46 && n < 53) || (n > 53 && n < 128) || (n > 128 && n < 157))
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
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.elseif(n = 128)
	; 6 bytes
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
.endif
.else
; 157 to 312
.if((n > 156 && n < 288) || n = 289)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
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
.elseif(n = 291 || n = 298 || n = 308)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n > 291 && n < 295) || n = 304 || n = 309)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 296)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 297 || n = 302 || n = 307 || n = 312)
	; 8 bytes
	delay_n (n-2)
	nop
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
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 303)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 311)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.endif
.endif
.else
; 313 to 625
.if(n = 313 || n = 348 || n = 362 || n = 369 || n = 404 || n = 432 || n = 439 || n = 446 || n = 481 || n = 488 || n = 509 || n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 339 || n = 344 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 379 || n = 384 || n = 387 || n = 389 || n = 394 || n = 399 || n = 406 || n = 409 || n = 414 || n = 419 || n = 424 || n = 429 || n = 434 || n = 444 || n = 449 || n = 454 || n = 464 || n = 469 || n = 479 || n = 484 || n = 494 || n = 499 || n = 504 || n = 511 || n = 514 || n = 519 || n = 524 || n = 529 || n = 536 || n = 541 || n = 549 || n = 554 || n = 559 || n = 564 || n = 569 || n = 574 || n = 576 || n = 584 || n = 589 || n = 594 || n = 599 || n = 604 || n = 609 || n = 619 || n = 624)
	; 7..9 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && (n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385 || n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460 || n = 470 || n = 480 || n = 485 || n = 495 || (n > 504 && n < 516) || (n > 524 && n < 566) || n = 590 || (n > 599 && n < 611)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316 || n = 317 || n = 321 || n = 322 || n = 326 || n = 331 || n = 332 || n = 336 || n = 337 || n = 342 || n = 347 || n = 351 || n = 352 || n = 357 || n = 361 || n = 366 || n = 367 || n = 371 || n = 372 || n = 377 || n = 382 || n = 392 || n = 396 || n = 407 || n = 412 || n = 416 || n = 417 || n = 422 || n = 426 || n = 427 || n = 431 || n = 436 || n = 437 || n = 441 || n = 442 || n = 447 || n = 451 || n = 452 || n = 456 || n = 457 || n = 462 || n = 466 || n = 472 || n = 476 || n = 477 || n = 482 || n = 486 || n = 487 || n = 492 || n = 496 || n = 497 || n = 506 || n = 512 || n = 517 || n = 522 || n = 526 || n = 527 || n = 531 || n = 534 || n = 539 || n = 542 || n = 546 || n = 547 || n = 552 || n = 556 || n = 557 || n = 561 || n = 562 || n = 566 || n = 567 || n = 571 || n = 577 || n = 582 || n = 587 || n = 591 || n = 592 || n = 596 || n = 597 || n = 601 || n = 602 || n = 606 || n = 612 || n = 616 || n = 617 || n = 622)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 318 || n = 323 || n = 328 || n = 338 || n = 343 || n = 353 || n = 358 || n = 363 || n = 368 || n = 378 || n = 386 || n = 388 || n = 393 || n = 398 || n = 403 || n = 408 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 458 || n = 463 || n = 468 || n = 473 || n = 478 || n = 483 || n = 498 || n = 503 || n = 508 || n = 513 || n = 518 || n = 528 || n = 538 || n = 543 || n = 548 || n = 553 || n = 563 || n = 568 || n = 578 || n = 583 || n = 588 || n = 598 || n = 603 || n = 608 || n = 618 || n = 623)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 0) && ((n > 324 && n < 341) || n = 355 || n = 360 || (n > 369 && n < 381) || n = 390 || n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465 || n = 475 || n = 490 || n = 500 || n = 520 || (n > 569 && n < 586) || n = 595 || (n > 614 && n < 626)))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 334 || n = 341 || n = 376 || n = 397 || n = 411 || n = 418 || n = 453 || n = 467 || n = 474 || n = 502 || n = 516 || n = 523 || n = 544 || n = 551 || n = 586 || n = 607 || n = 614 || n = 621)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 333 || n = 373 || n = 381 || n = 533 || n = 573)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 346)
	; 9 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 356 || n = 401 || n = 491)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 383)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
.elseif(n = 391)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 413 || n = 421 || n = 461 || n = 493 || n = 501 || n = 581 || n = 613)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 459)
	; 7 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
	tay
.elseif(n = 471)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
.elseif(n = 489)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
.elseif(n = 521)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 532)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 537)
	; 7 bytes
	php
	lda #2
	jsr delay_256a_16_clocks
	plp
.elseif(n = 611)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
	tax
.endif
.endif
.else
.if((n >= 626) && (n < 938))
; 626 to 937
.if(n = 626 || n = 716)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 627 || n = 632 || n = 636 || n = 637 || n = 647 || n = 652 || n = 657 || n = 662 || n = 666 || n = 667 || n = 672 || n = 676 || n = 682 || n = 686 || n = 687 || n = 697 || n = 702 || n = 706 || n = 707 || n = 711 || n = 717 || n = 721 || n = 722 || n = 727 || n = 732 || n = 737 || n = 742 || n = 746 || n = 751 || n = 757 || n = 762 || n = 766 || n = 767 || n = 771 || n = 772 || n = 776 || n = 777 || n = 786 || n = 787 || n = 802 || n = 807 || n = 811 || n = 812 || n = 816 || n = 822 || n = 826 || n = 827 || n = 832 || n = 837 || n = 841 || n = 842 || n = 846 || n = 847 || n = 856 || n = 857 || n = 862 || n = 867 || n = 871 || n = 872 || n = 876 || n = 877 || n = 882 || n = 886 || n = 891 || n = 892 || n = 897 || n = 902 || n = 906 || n = 907 || n = 911 || n = 912 || n = 916 || n = 917 || n = 921 || n = 926 || n = 927 || n = 931 || n = 932 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 649 || n = 663 || n = 684 || n = 698 || n = 726 || n = 803 || n = 817 || n = 831 || n = 859 || n = 866 || n = 873 || n = 894 || n = 901 || n = 908)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 646 || n = 654 || n = 659 || n = 664 || n = 669 || n = 674 || n = 679 || n = 681 || n = 689 || n = 692 || n = 694 || n = 699 || n = 704 || n = 709 || n = 714 || n = 724 || n = 729 || n = 734 || n = 739 || n = 744 || n = 749 || n = 756 || n = 759 || n = 764 || n = 769 || n = 774 || n = 779 || n = 784 || n = 792 || n = 794 || n = 797 || n = 799 || n = 804 || n = 809 || n = 814 || n = 819 || n = 829 || n = 834 || n = 839 || n = 844 || n = 849 || n = 854 || n = 864 || n = 869 || n = 874 || n = 879 || n = 884 || n = 889 || n = 899 || n = 904 || n = 909 || n = 914 || n = 919 || n = 924 || n = 934)
	; 7..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 630 || n = 640 || n = 645 || n = 650)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 631)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
	plp
.elseif(n = 633 || n = 638 || n = 643 || n = 648 || n = 658 || n = 668 || n = 673 || n = 678 || n = 683 || n = 691 || n = 703 || n = 708 || n = 713 || n = 718 || n = 723 || n = 728 || n = 738 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 778 || n = 783 || n = 791 || n = 796 || n = 798 || n = 808 || n = 818 || n = 823 || n = 828 || n = 833 || n = 843 || n = 848 || n = 858 || n = 863 || n = 868 || n = 878 || n = 883 || n = 888 || n = 898 || n = 913 || n = 918 || n = 923 || n = 928)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 936)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 641 || n = 881)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif((n .mod 7 = 5) && (n = 642 || n = 656 || n = 677 || n = 712 || n = 719 || n = 733 || (n > 746 && n < 769) || n = 782 || n = 789 || n = 824 || n = 838 || n = 852 || n = 887 || (n > 921 && n < 937)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 653 || n = 661 || n = 773 || n = 813 || n = 821 || n = 853 || n = 861 || n = 893)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 671 || n = 806 || n = 851 || n = 896)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 693 || n = 701 || n = 741 || n = 781 || n = 933)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 696 || n = 801)
	; 8..9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 731)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
.elseif(n = 736)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
	plp
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 788)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 793)
	; 7 bytes
	php
	lda #3
	jsr delay_256a_16_clocks
	plp
.elseif(n = 836)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	iny
	Jne *-2
	pla
	tay
.elseif(n = 903)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
	tay
.endif
.else
; 938 to 1250
.if(n = 938 || n = 948 || n = 953 || n = 958 || n = 963 || n = 968 || n = 983 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1021 || n = 1023 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1047 || n = 1052 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1088 || n = 1098 || n = 1103 || n = 1108 || n = 1113 || n = 1123 || n = 1128 || n = 1138 || n = 1143 || n = 1148 || n = 1158 || n = 1163 || n = 1168 || n = 1178 || n = 1183 || n = 1193 || n = 1198 || n = 1203 || n = 1208 || n = 1218 || n = 1228 || n = 1233 || n = 1238 || n = 1243 || n = 1248)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 939)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
	tay
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1251)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 941 || n = 973 || n = 1061 || n = 1093 || n = 1101 || n = 1133 || n = 1141 || n = 1173 || n = 1213 || n = 1221)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 942 || n = 946 || n = 947 || n = 951 || n = 952 || n = 956 || n = 961 || n = 967 || n = 972 || n = 977 || n = 982 || n = 987 || n = 991 || n = 996 || n = 997 || n = 1001 || n = 1002 || n = 1007 || n = 1012 || n = 1016 || n = 1017 || n = 1032 || n = 1036 || n = 1037 || n = 1042 || n = 1046 || n = 1051 || n = 1057 || n = 1066 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1081 || n = 1082 || n = 1086 || n = 1087 || n = 1091 || n = 1092 || n = 1096 || n = 1102 || n = 1106 || n = 1107 || n = 1112 || n = 1116 || n = 1117 || n = 1122 || n = 1126 || n = 1127 || n = 1131 || n = 1136 || n = 1137 || n = 1142 || n = 1147 || n = 1151 || n = 1152 || n = 1156 || n = 1157 || n = 1161 || n = 1162 || n = 1171 || n = 1172 || n = 1176 || n = 1177 || n = 1182 || n = 1186 || n = 1187 || n = 1191 || n = 1192 || n = 1196 || n = 1197 || n = 1201 || n = 1206 || n = 1207 || n = 1212 || n = 1217 || n = 1222 || n = 1226 || n = 1227 || n = 1231 || n = 1232 || n = 1242 || n = 1246 || n = 1247)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && (n = 943 || (n > 956 && n < 979) || (n > 991 && n < 1014) || (n > 1026 && n < 1042) || (n > 1061 && n < 1084) || (n > 1096 && n < 1119) || (n > 1131 && n < 1154) || (n > 1166 && n < 1189) || (n > 1201 && n < 1224) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 969 || n = 974 || n = 979 || n = 984 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1019 || n = 1022 || n = 1024 || n = 1029 || n = 1039 || n = 1048 || n = 1053 || n = 1054 || n = 1059 || n = 1064 || n = 1079 || n = 1084 || n = 1089 || n = 1094 || n = 1109 || n = 1114 || n = 1119 || n = 1124 || n = 1129 || n = 1134 || n = 1144 || n = 1149 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1179 || n = 1184 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1219 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1249)
	; 7..8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 976)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	dey
	Jne *-2
	pla
	tay
.elseif(n = 981)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 986 || n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1011)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jpl *-4
	tay
.elseif(n = 1018)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
.elseif(n = 1026)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1031 || n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 1044)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1049)
	; 7 bytes
	php
	lda #4
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1056)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1074)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 1099)
	; 7 bytes
	tya
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
	tay
.elseif(n = 1236)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	nop
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
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
; 1251 to 1406
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || (n > 1271 && n < 1294) || (n > 1313 && n < 1406)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1257 || n = 1262 || n = 1266 || n = 1267 || n = 1271 || n = 1276 || n = 1277 || n = 1281 || n = 1282 || n = 1287 || n = 1291 || n = 1294 || n = 1295 || n = 1302 || n = 1307 || n = 1310 || n = 1316 || n = 1323 || n = 1327 || n = 1330 || n = 1334 || n = 1343 || n = 1344 || n = 1348 || n = 1351 || n = 1358 || n = 1359 || n = 1362 || n = 1372 || n = 1379 || n = 1383 || n = 1386 || n = 1390 || n = 1393 || n = 1399 || n = 1404)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 5) && (n = 1253 || n = 1261 || n = 1301 || (n > 1316 && n < 1342) || (n > 1356 && n < 1398)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1254 || n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1284 || n = 1289 || n = 1297 || n = 1304 || n = 1309 || n = 1318 || n = 1329 || n = 1332 || n = 1339 || n = 1353 || n = 1360 || n = 1367 || n = 1374 || n = 1388 || n = 1395 || n = 1402)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1281)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 9 = 5) && (n = 1256 || n = 1319 || (n > 1336 && n < 1365) || n = 1382 || n = 1400))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1263 || n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1299 || n = 1303 || n = 1308 || n = 1320 || n = 1324 || n = 1331 || n = 1338 || n = 1352 || n = 1366 || n = 1368 || n = 1376 || n = 1387 || n = 1392 || n = 1394 || n = 1401 || n = 1406)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.elseif(n = 1290)
	; 9 bytes
	php
	sec
	lda #0
	sbc #1
	Jne *-2
	plp
.elseif(n = 1292)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1296)
	; 9 bytes
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1298)
	; 9 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1300)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1305)
	; 7 bytes
	php
	lda #5
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1306 || n = 1311)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1313 || n = 1322 || n = 1350 || n = 1371)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1315 || n = 1345 || n = 1375)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1326 || n = 1340 || n = 1354 || n = 1396)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1336)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1347 || n = 1369)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1361 || n = 1385)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1378)
	; 10 bytes
	tya
	pha
	ldy #152 ;hides 'tya'
	nop
	dey
	Jne *-3
	pla
	tay
.elseif(n = 1380)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.endif
.else
; 1407 to 1562
.if(n = 1407 || n = 1416 || n = 1417 || n = 1423 || n = 1428 || n = 1430 || n = 1441 || n = 1451 || n = 1458 || n = 1465 || n = 1473 || n = 1479 || n = 1486 || n = 1497 || n = 1500 || n = 1507 || n = 1514 || n = 1521 || n = 1528 || n = 1537 || n = 1542 || n = 1560)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1408 || n = 1422 || n = 1427 || n = 1434 || n = 1443 || n = 1448 || n = 1450 || n = 1464 || n = 1478 || n = 1480 || n = 1488 || n = 1492 || n = 1504 || n = 1506 || n = 1513 || n = 1536 || n = 1548 || n = 1559)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 5) && (n = 1409 || n = 1418 || n = 1436 || (n > 1462 && n < 1509) || (n > 1525 && n < 1554)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1410 || n = 1431)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1411 || n = 1432 || n = 1462 || n = 1476 || n = 1518 || n = 1532 || n = 1539)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1553)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && ((n > 1412 && n < 1454) || (n > 1468 && n < 1510) || (n > 1524 && n < 1558)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1414 || n = 1420 || n = 1435 || n = 1439 || n = 1442 || n = 1449 || n = 1456 || n = 1460 || n = 1467 || n = 1470 || n = 1471 || n = 1474 || n = 1483 || n = 1484 || n = 1487 || n = 1491 || n = 1498 || n = 1502 || n = 1505 || n = 1511 || n = 1512 || n = 1516 || n = 1519 || n = 1527 || n = 1530 || n = 1540 || n = 1543 || n = 1546 || n = 1547 || n = 1551 || n = 1554 || n = 1558)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1415 || n = 1425 || n = 1455 || n = 1495 || n = 1515 || n = 1555)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1438 || n = 1452 || n = 1466 || n = 1494 || n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1444)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1446 || n = 1523 || n = 1534)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1457)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1459)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jpl *-4
	tay
.elseif(n = 1520)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1522)
	; 9 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1529)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1556)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1561)
	; 7 bytes
	php
	lda #6
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1562)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1575 || n = 1582 || n = 1583 || n = 1586 || n = 1596 || n = 1599 || n = 1603 || n = 1610 || n = 1617 || n = 1623 || n = 1624 || n = 1628 || n = 1638 || n = 1639 || n = 1642 || n = 1659 || n = 1663 || n = 1666 || n = 1673 || n = 1680 || n = 1684 || n = 1687 || n = 1691 || n = 1694 || n = 1698 || n = 1708 || n = 1711)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564 || n = 1572 || n = 1576 || n = 1590 || n = 1592 || n = 1604 || n = 1630 || n = 1644 || n = 1656 || n = 1660 || n = 1667 || n = 1672 || n = 1674 || n = 1681 || n = 1702 || n = 1704 || n = 1712 || n = 1716)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1565 || n = 1570 || n = 1584 || n = 1591 || n = 1593 || n = 1609 || n = 1612 || n = 1618 || n = 1619 || n = 1626 || n = 1631 || n = 1633 || n = 1640 || n = 1645 || n = 1651 || n = 1654 || n = 1668 || n = 1675 || n = 1682 || n = 1689 || n = 1696 || n = 1703 || n = 1710)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1567)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1569 || n = 1574 || n = 1588 || n = 1602 || n = 1635 || n = 1658 || n = 1700 || n = 1707 || n = 1714)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 9 = 5) && (n = 1571 || (n > 1597 && n < 1626) || n = 1652 || (n > 1669 && n < 1698) || n = 1715))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1577 || n = 1649)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1578 || n = 1600 || n = 1611)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1579)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 8 = 5) && ((n > 1580 && n < 1622) || n = 1637 || (n > 1652 && n < 1678) || (n > 1692 && n < 1718)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1585 || n = 1595 || n = 1655 || n = 1665 || n = 1695 || n = 1705)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1606 || n = 1620 || n = 1662 || n = 1676 || n = 1683 || n = 1690 || n = 1718)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1614)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1632)
	; 9 bytes
	php
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	plp
.elseif(n = 1634 || n = 1648)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1641)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1646)
	; 9 bytes
	php
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1647)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
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
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1736 || n = 1740 || n = 1743 || n = 1747 || n = 1750 || n = 1764 || n = 1767 || n = 1771 || n = 1782 || n = 1785 || n = 1791 || n = 1792)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1791)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1721 || n = 1745 || n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1723 || n = 1728 || n = 1730 || n = 1737 || n = 1758 || n = 1768 || n = 1772 || n = 1779 || n = 1784 || n = 1786)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1724 || n = 1742 || n = 1751 || n = 1760 || n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && (n = 1725 || n = 1733 || (n > 1748 && n < 1790)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1731 || n = 1738 || n = 1752 || n = 1753 || n = 1759 || n = 1761 || n = 1766 || n = 1777 || n = 1780)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1732 || n = 1754)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1735 || n = 1775 || n = 1795)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1739 || n = 1774 || n = 1788)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1744)
	; 9 bytes
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1746)
	; 9 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	plp
.elseif(n = 1756 || n = 1770)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1763)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1794)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
.endif
.else
; 1797 to 1875
.if(n = 1797)
	; 8 bytes
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1798 || n = 1801 || n = 1807 || n = 1816 || n = 1821 || n = 1833 || n = 1834 || n = 1836 || n = 1849 || n = 1854 || n = 1857 || n = 1873)
	; 8..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1799 || n = 1814 || n = 1819 || n = 1822 || n = 1826 || n = 1839 || n = 1844 || n = 1847 || n = 1852 || n = 1858 || n = 1863 || n = 1866 || n = 1870 || n = 1871 || n = 1874)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1815 || n = 1820 || n = 1827 || n = 1828 || n = 1840 || n = 1848 || n = 1856 || n = 1860 || n = 1862 || n = 1872)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1802)
	; 10 bytes
	php
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1803)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1804 || n = 1810)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif((n .mod 8 = 5) && (n = 1805 || n = 1813 || (n > 1828 && n < 1870)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1809 || n = 1831 || n = 1842 || n = 1864)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1811)
	; 10 bytes
	tya
	ldy #128
	delay_n 9
	dey
	Jpl *-4
	tay
.elseif(n = 1812)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1817)
	; 7 bytes
	php
	lda #7
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1818 || n = 1823 || n = 1851)
	; 9..10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1825 || n = 1838)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1830)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	nop
	Jne *-3
	tax
.elseif((n .mod 9 = 5) && ((n > 1831 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1835 || n = 1855 || n = 1875)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1843)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
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
.elseif(n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1867)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if((n .mod 2 = 0) && ((n > 1875 && n < 1881) || n = 1884 || n = 1888 || n = 1892 || n = 1896 || n = 1900 || n = 1912 || n = 1920 || n = 1928 || n = 1934 || n = 1936 || n = 1944 || n = 1948 || n = 1956 || n = 1960 || (n > 1963 && n < 1969) || n = 1984 || n = 1988 || n = 1992 || n = 2000 || (n > 2003 && n < 2009) || n = 2016 || n = 2024 || n = 2028))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 5) && ((n > 1876 && n < 2030)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1879 || n = 1882 || n = 1887 || n = 1891 || n = 1898 || n = 1903 || n = 1906 || n = 1911 || n = 1914 || n = 1924 || n = 1927 || n = 1932 || n = 1938 || n = 1939 || n = 1942 || n = 1943 || n = 1946 || n = 1951 || n = 1959 || n = 1978 || n = 1983 || n = 1986 || n = 1987 || n = 1991 || n = 1999 || n = 2002 || n = 2010 || n = 2011 || n = 2014 || n = 2023 || n = 2026 || n = 2031)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1881 || n = 1890 || n = 1899 || n = 1926 || n = 1929 || n = 1953 || n = 1962 || n = 1969 || n = 1971 || n = 1977 || n = 1980 || n = 1993 || n = 1998 || n = 2001 || n = 2017 || n = 2022)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1883 || n = 1947 || n = 1950)
	; 10..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 9 = 5) && ((n > 1885 && n < 1941) || (n > 1957 && n < 1995) || n = 2012 || n = 2030))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1889 || n = 1937 || n = 1961 || n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1894)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 11 = 5) && ((n > 1896 && n < 1931) || (n > 1951 && n < 1975) || (n > 1995 && n < 2019)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1902)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1905 || n = 1915 || n = 1935 || n = 1945 || n = 1955 || n = 1975 || n = 1995 || n = 2015 || n = 2025)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1907)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1910)
	; 10 bytes
	tya
	ldy #126
	delay_n 10
	dey
	Jpl *-4
	tay
.elseif(n = 1916 || n = 2020)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1918)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1921)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
.elseif(n = 1923 || n = 1979)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1954)
	; 10 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1970)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 1972)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
.elseif(n = 1982)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1990)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	nop
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2003)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 2019)
	; 10 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	nop
	iny
	Jne *-3
	pla
	tay
.elseif(n = 2027)
	; 10 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2036 || n = 2038 || n = 2044 || n = 2054 || n = 2056)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2033)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
	tax
.elseif(n = 2034 || n = 2041 || n = 2043 || n = 2049 || n = 2052 || n = 2060 || n = 2061)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2035 || n = 2065)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2037 || n = 2045)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && ((n > 2038 && n < 2067)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2040 || n = 2051 || n = 2062)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2042 || n = 2047 || n = 2050 || n = 2055 || n = 2059 || n = 2064 || n = 2067 || n = 2070)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2046)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2053)
	; 8 bytes
	tya
	ldy #0
	pha
	dey
	Jne *-2
	tay
.elseif(n = 2058)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
	tay
.elseif(n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2068)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2069)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2076 || n = 2083 || n = 2094)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2072 || n = 2077 || n = 2086 || n = 2088 || n = 2095 || n = 2097)
	; 8..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2073)
	; 7 bytes
	php
	lda #8
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2074 || n = 2079)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2075 || n = 2078 || n = 2087 || n = 2104 || n = 2107 || n = 2108)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2081 || n = 2090)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2082)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 2084 || n = 2093 || n = 2102)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2085)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2089 || n = 2100)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2091)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2092)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2096)
	; 10 bytes
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2098)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2099)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2101)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2103)
	; 11 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2105)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2106)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2109)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2113 || n = 2121 || n = 2124 || n = 2142 || n = 2154 || n = 2160 || n = 2169 || n = 2178 || n = 2181 || n = 2187)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2184)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2118 || n = 2123 || n = 2132 || n = 2144 || n = 2148 || n = 2159 || n = 2164 || n = 2168 || n = 2186)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 10 = 5) && ((n > 2114 && n < 2156) || n = 2175 || n = 2185))
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2116 || n = 2136)
	; 10..11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2117 || n = 2141 || n = 2153 || n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
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
.elseif(n = 2122 || n = 2127 || n = 2130 || n = 2131 || n = 2140 || n = 2143 || n = 2149 || n = 2152 || n = 2157 || n = 2158 || n = 2166 || n = 2167 || n = 2176 || n = 2179)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2126)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	plp
.elseif((n .mod 11 = 5) && ((n > 2127 && n < 2173)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
.elseif(n = 2137 || n = 2163)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2146 || n = 2182)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2151)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
	tax
.elseif(n = 2162)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2170)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	nop
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2171)
	; 11 bytes
	tya
	ldy #(n-24)/19
	delay_n 14
	dey
	Jpl *-5
	tay
.elseif(n = 2173)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2180)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2184)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if(n = 2188 || n = 2198 || n = 2204 || n = 2206 || n = 2208 || n = 2222 || n = 2239 || n = 2243 || n = 2252 || n = 2258)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2189 || n = 2213 || n = 2225 || n = 2249 || n = 2261)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2190 || n = 2226 || n = 2256)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2191 || n = 2197 || n = 2212 || n = 2218 || n = 2221 || n = 2229 || n = 2230 || n = 2247 || n = 2248 || n = 2251 || n = 2257 || n = 2262 || n = 2263)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 5) && ((n > 2191 && n < 2265)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2193 || n = 2196 || n = 2199 || n = 2207 || n = 2209 || n = 2214 || n = 2220 || n = 2223 || n = 2232 || n = 2240 || n = 2241 || n = 2250 || n = 2253)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2194 || n = 2216 || n = 2227 || n = 2238 || n = 2260)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2195 || n = 2205 || n = 2215 || n = 2235 || n = 2245 || n = 2265)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2200)
	; 11 bytes
	tya
	pha
	ldy #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2202 || n = 2254)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2203)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2211)
	; 11 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	nop
	nop
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2217 || n = 2231 || n = 2259)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2224 || n = 2233)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	nop
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2234)
	; 11 bytes
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2236)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
	tax
.elseif(n = 2242)
	; 10 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2244)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.endif
.else
; 2266 to 2343
.if(n = 2266 || n = 2284 || n = 2299 || n = 2302 || n = 2307 || n = 2311 || n = 2317 || n = 2320 || n = 2326 || n = 2331 || n = 2334 || n = 2338)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267 || n = 2280 || n = 2306 || n = 2319)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2268 || n = 2269 || n = 2277 || n = 2279 || n = 2286 || n = 2289 || n = 2313 || n = 2328 || n = 2333)
	; 8..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2270)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2271 || n = 2293 || n = 2304)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2272 || n = 2337)
	; 9..11 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 9 = 5) && ((n > 2272 && n < 2301)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2274 || n = 2276 || n = 2278 || n = 2288 || n = 2294 || n = 2296 || n = 2298 || n = 2303 || n = 2308 || n = 2312 || n = 2318 || n = 2322 || n = 2327 || n = 2332 || n = 2339 || n = 2340)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 10 = 5) && (n = 2275 || (n > 2294 && n < 2326)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2281)
	; 11 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2283)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2285 || n = 2297 || n = 2321)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2287 || n = 2301)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2290)
	; 11 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2292 || n = 2316)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2309)
	; 9 bytes
	tya
	ldy #0
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2330 || n = 2335 || n = 2342)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2314)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2323)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 2324)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2329)
	; 7 bytes
	php
	lda #9
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2336)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2341)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2343)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	pha
	dey
	Jne *-3
	tay
.endif
.endif
.else
; 2344 to 2500
.if(n = 2344 || n = 2346 || n = 2351 || n = 2362 || n = 2366 || n = 2368 || n = 2373 || n = 2378 || n = 2382 || n = 2388 || n = 2396 || n = 2398 || n = 2406 || n = 2408 || n = 2418 || n = 2426 || n = 2428 || n = 2432 || n = 2442 || n = 2444 || n = 2448 || n = 2456 || n = 2461 || n = 2468 || n = 2472 || n = 2476 || n = 2478 || n = 2492 || n = 2494 || n = 2498)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2490)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2347 || n = 2349 || n = 2352 || n = 2363 || n = 2374 || n = 2379 || n = 2389 || n = 2409 || n = 2421 || n = 2433 || n = 2440 || n = 2443 || n = 2454 || n = 2459 || n = 2473 || n = 2481 || n = 2484 || n = 2499)
	; 9..11 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 11 = 5) && ((n > 2347 && n < 2371) || (n > 2391 && n < 2415) || (n > 2435 && n < 2492)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2350 || n = 2361 || n = 2367 || n = 2372 || n = 2377 || n = 2383 || n = 2387 || n = 2394 || n = 2401 || n = 2407 || n = 2416 || n = 2419 || n = 2430 || n = 2431 || n = 2437 || n = 2438 || n = 2452 || n = 2457 || n = 2460 || n = 2467 || n = 2471 || n = 2474 || n = 2479 || n = 2482 || n = 2487 || n = 2493 || n = 2500)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2353)
	; 11 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2354 || n = 2411 || n = 2464 || n = 2486)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 10 = 5) && ((n > 2354 && n < 2396) || (n > 2414 && n < 2456) || (n > 2474 && n < 2496)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2356 || n = 2463 || n = 2466)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 13 = 5) && (n = 2358 || (n > 2383 && n < 2424) || n = 2449 || n = 2462 || n = 2488))
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2360 || n = 2390 || n = 2420 || n = 2450)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2364 || n = 2376 || n = 2424)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2371 || n = 2399 || n = 2413 || n = 2427 || n = 2483 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2380)
	; 11 bytes
	tya
	ldy #(n-24)/19
	delay_n 14
	dey
	Jpl *-5
	tay
.elseif(n = 2386 || n = 2422 || n = 2434 || n = 2470)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2391)
	; 9 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
	tay
.elseif(n = 2400 || n = 2412 || n = 2446 || n = 2496)
	; 10..11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2402)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2404)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2451)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 2490)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	bit $24
	dey
	Jne *-4
	tay
	plp
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2503 || n = 2504 || n = 2507 || n = 2512 || n = 2517 || n = 2526 || n = 2527 || n = 2534 || n = 2547 || n = 2551 || n = 2563 || n = 2575)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2505 || n = 2515 || n = 2535 || n = 2545)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2506 || n = 2509 || n = 2519 || n = 2529 || n = 2541 || n = 2548 || n = 2550 || n = 2559 || n = 2569 || n = 2572)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2508 || n = 2516 || n = 2518 || n = 2522 || n = 2528 || n = 2538 || n = 2552 || n = 2558 || n = 2560 || n = 2564 || n = 2571 || n = 2576)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2510 || n = 2570)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2511 || n = 2539 || n = 2553 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2514 || n = 2540 || n = 2566)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2520 || n = 2532 || n = 2556 || n = 2562)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2521)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2523)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2530 || n = 2542 || n = 2554 || n = 2578)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2531)
	; 11 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2533)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2536)
	; 12 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2543 || n = 2574)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2544)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
.elseif(n = 2555)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2565)
	; 9 bytes
	tya
	ldy #0
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2577)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	nop
	dey
	Jne *-4
	tay
.endif
.else
; 2579 to 2656
.if((n .mod 11 = 5) && (n = 2579 || (n > 2600 && n < 2635) || n = 2656))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2580)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2581 || n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2582 || n = 2587 || n = 2590 || n = 2594 || n = 2599 || n = 2611 || n = 2614 || n = 2625 || n = 2632 || n = 2635 || n = 2636 || n = 2639 || n = 2647 || n = 2653)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2588 || n = 2595 || n = 2596 || n = 2604 || n = 2610 || n = 2624 || n = 2626 || n = 2648 || n = 2654)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2584 || n = 2589 || n = 2607 || n = 2613 || n = 2616 || n = 2622 || n = 2627 || n = 2638 || n = 2646 || n = 2649 || n = 2655)
	; 8..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 2585)
	; 7 bytes
	php
	lda #10
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2586 || n = 2591 || n = 2598)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2593 || n = 2617 || n = 2620 || n = 2641)
	; 9..11 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 12 = 5) && ((n > 2596 && n < 2646)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2600 || n = 2615 || n = 2630)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2602 || n = 2650)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2603)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-4
	tay
.elseif((n .mod 13 = 5) && ((n > 2604 && n < 2645)))
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2606)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
	tay
.elseif(n = 2608)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2619)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2628 || n = 2652)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2629)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2640)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2642)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2643)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 2657 to 2812
.if((n .mod 12 = 5) && ((n > 2656 && n < 2802)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2658 || n = 2659 || n = 2671 || n = 2680 || n = 2683 || n = 2691 || n = 2694 || n = 2695 || n = 2702 || n = 2713 || n = 2716 || n = 2719 || n = 2723 || n = 2724 || n = 2727 || n = 2731 || n = 2743 || n = 2746 || n = 2751 || n = 2757 || n = 2767 || n = 2768 || n = 2779 || n = 2786 || n = 2790 || n = 2812)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675 || n = 2690 || n = 2720 || n = 2750 || n = 2780 || n = 2795)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2661 || n = 2673 || n = 2682 || n = 2685 || n = 2697 || n = 2704 || n = 2715 || n = 2726 || n = 2737 || n = 2745 || n = 2754 || n = 2759 || n = 2770 || n = 2781 || n = 2793 || n = 2807)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 12 = 10) && ((n > 2661 && n < 2711) || n = 2734 || n = 2782 || n = 2794))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2663 || n = 2687 || n = 2699 || n = 2706 || n = 2728 || n = 2739 || n = 2771 || n = 2783)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2664 || n = 2772 || n = 2784)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 14 = 5) && (n = 2665 || n = 2679 || (n > 2706 && n < 2764) || n = 2791 || n = 2805))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2666)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 11 = 5) && ((n > 2666 && n < 2767) || (n > 2787 && n < 2811)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2668 || n = 2672 || n = 2684 || n = 2692 || n = 2703 || n = 2708 || n = 2714 || n = 2718 || n = 2725 || n = 2732 || n = 2736 || n = 2740 || n = 2747 || n = 2756 || n = 2758 || n = 2762 || n = 2764 || n = 2769 || n = 2778 || n = 2792 || n = 2802 || n = 2804 || n = 2806)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2670 || n = 2696 || n = 2709 || n = 2748 || n = 2761 || n = 2774 || n = 2800)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2676 || n = 2712 || n = 2760 || n = 2796 || n = 2808)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2677 || n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2688)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2701 || n = 2730 || n = 2752 || n = 2785 || n = 2797 || n = 2809)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2738)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2742 || n = 2776)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2775)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
	tax
.elseif(n = 2787)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2798)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
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
.elseif(n = 2811)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if(n = 2813 || n = 2825 || n = 2837 || n = 2861 || n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2814 || n = 2817 || n = 2829 || n = 2840 || n = 2845 || n = 2859 || n = 2871 || n = 2877 || n = 2882)
	; 8..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 2815 || n = 2823 || n = 2827 || n = 2834 || n = 2835 || n = 2838 || n = 2843 || n = 2846 || n = 2850 || n = 2863 || n = 2872 || n = 2887)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2822 || n = 2824 || n = 2828 || n = 2839 || n = 2844 || n = 2851 || n = 2852 || n = 2856 || n = 2858 || n = 2864 || n = 2876 || n = 2888)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2818 || n = 2830 || n = 2866 || n = 2890)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2819 || n = 2833 || n = 2875 || n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2820 || n = 2832 || n = 2868 || n = 2880)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2821)
	; 9 bytes
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2826 || n = 2865 || n = 2878)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2831 || n = 2842 || n = 2847 || n = 2854 || n = 2879)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2836)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2841)
	; 7 bytes
	php
	lda #11
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2849 || n = 2881)
	; 9..11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2853)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 2855 || n = 2870)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2857)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2860)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2862)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2867)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2869)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2874)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2883 || n = 2884 || n = 2886)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2904 || n = 2930 || n = 2943 || n = 2956)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 0) && (n = 2892 || (n > 2915 && n < 2953)))
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2893)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2894 || n = 2907 || n = 2912 || n = 2918 || n = 2920 || n = 2924 || n = 2934 || n = 2936 || n = 2948)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2895 || n = 2929 || n = 2946 || n = 2963)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2896 || n = 2898 || n = 2922 || n = 2942 || n = 2944 || n = 2954 || n = 2955 || (n > 2965 && n < 2969))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 12 = 5) && ((n > 2896 && n < 2958)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2899 || n = 2905 || n = 2906 || n = 2911 || n = 2919 || n = 2923 || n = 2932 || n = 2935 || n = 2947 || n = 2958)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2900 || n = 2915 || n = 2960)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2901 || n = 2908 || n = 2913 || n = 2925 || n = 2937 || n = 2949 || n = 2961)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 12 = 10) && ((n > 2901 && n < 2963)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2903 || n = 2917 || n = 2931 || n = 2959)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2910)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	dex
	dex
	Jne *-6
	tax
.elseif(n = 2927 || n = 2939 || n = 2951)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2941 || n = 2953)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2964)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2965)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n .mod 12 = 5) && ((n > 2968 && n < 3042)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2970 || n = 3002 || n = 3016 || n = 3028 || n = 3030 || n = 3040)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2971 || n = 2983 || n = 2989 || n = 2992 || n = 2995 || n = 3007 || n = 3031)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2978 || n = 2984 || n = 2996 || n = 3004 || n = 3018 || n = 3022 || n = 3032 || n = 3038 || n = 3044)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 14 = 5) && ((n > 2972 && n < 3016) || n = 3043))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2974 || n = 2986 || n = 2998 || n = 3010 || n = 3046)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2975 || n = 2990 || n = 3020 || n = 3035)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2976 || n = 3000 || n = 3036)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2977 || n = 3025 || n = 3027 || n = 3037 || n = 3042)
	; 11..12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2979 || n = 2985 || n = 2991 || n = 2994 || n = 2997 || n = 3009 || n = 3023 || n = 3033 || n = 3039)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 2980 || n = 3014)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2982 || n = 3008 || n = 3021 || n = 3034)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2988)
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 6
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2999 || n = 3011)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3003)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3006)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	nop
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3012 || n = 3024)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3013)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3019)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-4
	tay
.elseif(n = 3026)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3045)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047 || n = 3060 || n = 3073)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3048)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
	tax
.elseif(n = 3049 || n = 3052 || n = 3054 || n = 3069 || n = 3081)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3050)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3051 || n = 3056 || n = 3063 || n = 3068 || n = 3074 || n = 3076 || n = 3080)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3055 || n = 3059 || n = 3062 || n = 3066 || n = 3067 || n = 3075 || n = 3079)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3057 || n = 3071 || n = 3085)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3058 || n = 3070)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3061)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3064 || n = 3083)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3072 || n = 3078 || n = 3084)
	; 11..12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3082)
	; 11 bytes
	php
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
	plp
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3112 || n = 3125)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3087 || n = 3088 || n = 3094 || n = 3099 || n = 3102 || n = 3106 || n = 3114 || n = 3115)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3089 || n = 3095 || n = 3100 || n = 3107 || n = 3108)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3090 || n = 3096 || n = 3101 || n = 3117)
	; 8..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3091 || n = 3098 || n = 3103)
	; 9..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3092)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3093)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3097)
	; 7 bytes
	php
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3104)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3105)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3109)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3110)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3111)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 3113)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3116)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3118)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3119)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3120 || n = 3122 || n = 3123)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3121)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3124)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3132 || n = 3134 || n = 3135 || n = 3139 || (n > 3145 && n < 3150) || n = 3152 || n = 3156 || (n > 3159 && n < 3164) || n = 3165 || n = 3175 || n = 3176 || n = 3182 || n = 3195 || n = 3196 || n = 3198 || n = 3208 || n = 3210 || n = 3212 || (n > 3221 && n < 3225) || n = 3226 || n = 3236 || n = 3238 || n = 3250 || n = 3261 || n = 3265 || n = 3266 || n = 3274 || n = 3276 || n = 3280)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 14 = 5) && ((n > 3126 && n < 3282)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3128 || n = 3130 || n = 3144 || n = 3154 || n = 3158 || n = 3180 || n = 3186 || n = 3188 || n = 3193 || n = 3206 || n = 3214 || n = 3219 || n = 3228 || n = 3233 || n = 3240 || n = 3248 || n = 3254 || n = 3271 || n = 3278)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3129 || n = 3142 || n = 3143 || n = 3153 || n = 3166 || n = 3171 || n = 3172 || n = 3179 || n = 3192 || n = 3199 || n = 3202 || n = 3213 || n = 3217 || n = 3227 || n = 3231 || n = 3232 || n = 3241 || n = 3244 || n = 3247 || n = 3257 || n = 3262 || n = 3270 || n = 3277)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3131)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3133 || n = 3150 || n = 3167 || n = 3184 || n = 3201 || n = 3218 || n = 3235 || n = 3252 || n = 3269)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3136)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3137 || n = 3191 || n = 3209 || n = 3263)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif((n .mod 13 = 5) && ((n > 3137 && n < 3191) || (n > 3215 && n < 3269)))
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 15 = 5) && (n = 3140 || (n > 3169 && n < 3276)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3145 || n = 3168 || n = 3174 || n = 3181 || n = 3194 || n = 3204 || n = 3207 || n = 3220 || n = 3234 || n = 3243 || n = 3246 || n = 3249 || n = 3259 || n = 3264 || n = 3272)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 16 = 5) && ((n > 3156 && n < 3222)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3159 || n = 3178 || n = 3273)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
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
.elseif(n = 3203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3237)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	pha
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3256)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	plp
.elseif(n = 3258)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 3282) && (n < 3360))
; 3282 to 3359
.if(n = 3282 || n = 3293 || n = 3298 || n = 3308 || n = 3310 || n = 3312 || n = 3326 || n = 3336 || n = 3338 || n = 3340 || n = 3351 || n = 3356)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3283 || n = 3313 || n = 3321 || n = 3324 || n = 3327 || n = 3334 || n = 3341 || n = 3352 || n = 3357)
	; 8..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3284)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3285 || n = 3301 || n = 3317 || n = 3349)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3286 || n = 3302 || n = 3316 || n = 3344 || n = 3347)
	; 11..13 bytes
	delay_n (n-4)
	pla
.elseif(n = 3287 || n = 3331 || n = 3345)
	; 11..12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3288 || n = 3289 || n = 3291)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3290 || n = 3305 || n = 3335)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3292 || n = 3311 || n = 3330)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3294 || n = 3307 || n = 3320)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 14 = 5) && ((n > 3294 && n < 3338)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3296 || n = 3297 || n = 3319 || n = 3322 || n = 3325 || n = 3332 || n = 3339 || n = 3346 || n = 3350 || n = 3355 || n = 3358)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3299)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3300 || n = 3314 || n = 3328 || n = 3342)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3303)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3304)
	; 12 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	plp
.elseif(n = 3306)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3315 || n = 3329 || n = 3343 || n = 3354 || n = 3359)
	; 9..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3318)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3333)
	; 10 bytes
	tya
	ldy #0
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3348)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3353)
	; 7 bytes
	php
	lda #13
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3361)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3362 || n = 3382)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3364 || n = 3368 || n = 3383 || n = 3396 || n = 3398)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3365)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3366)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3367)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 3369)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3370)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 3371 || n = 3388)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3372)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3373)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3374)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3375 || n = 3376 || n = 3378)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3377)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3380 || n = 3395)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3381 || n = 3397)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3384)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3385 || n = 3390 || n = 3391 || n = 3394)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3386)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	nop
	delay_n 6
	dey
	dey
	Jne *-6
	tay
.elseif(n = 3387)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3389)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3392)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3411 || n = 3414)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif((n > 3399 && n < 3403) || n = 3408 || (n > 3414 && n < 3421) || n = 3426 || (n > 3429 && n < 3435) || n = 3436)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3404)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 10
	inx
	inx
	Jne *-6
	tax
.elseif(n = 3405 || n = 3422)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3406)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3407 || n = 3421 || n = 3435)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3409 || n = 3412 || n = 3423 || n = 3427 || n = 3437)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3410 || n = 3425)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3413 || n = 3429)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3424 || n = 3428)
	; 11 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if(n = 3438 || n = 3452 || n = 3458 || n = 3466 || n = 3471 || n = 3486 || n = 3488 || n = 3494 || n = 3503 || n = 3508 || n = 3516 || n = 3518 || n = 3522 || n = 3536 || n = 3548 || n = 3550 || n = 3556 || n = 3564 || n = 3572 || n = 3576 || n = 3578 || n = 3588)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3439 || n = 3456 || n = 3473 || n = 3490 || n = 3507 || n = 3524 || n = 3541 || n = 3558 || n = 3592)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif((n .mod 15 = 5) && ((n > 3439 && n < 3561) || n = 3590))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3441 || (n > 3445 && n < 3449) || n = 3450 || n = 3460 || n = 3462 || n = 3464 || n = 3469 || n = 3475 || n = 3476 || n = 3478 || n = 3484 || n = 3492 || n = 3496 || n = 3498 || n = 3499 || n = 3506 || (n > 3509 && n < 3513) || n = 3514 || (n > 3525 && n < 3530) || n = 3531 || n = 3538 || n = 3540 || (n > 3541 && n < 3545) || n = 3546 || n = 3552 || n = 3553)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3442 || n = 3451 || n = 3457 || n = 3465 || n = 3472 || n = 3479 || n = 3487 || n = 3502 || n = 3517 || n = 3521 || n = 3532 || n = 3535 || n = 3549 || n = 3559 || n = 3562 || n = 3563 || n = 3570 || n = 3571 || n = 3574 || n = 3591)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3443 || n = 3497 || n = 3551 || n = 3569 || n = 3587)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3444 || n = 3501 || n = 3520 || n = 3539 || n = 3577)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3445 || n = 3461 || n = 3493 || n = 3509 || n = 3525 || n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif((n .mod 14 = 5) && ((n > 3448 && n < 3576)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3453 || n = 3459 || n = 3467 || n = 3474 || n = 3481 || n = 3489 || n = 3495 || n = 3504 || n = 3523 || n = 3534 || n = 3537 || n = 3565 || n = 3579 || n = 3593)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3454)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 pha
	.endrepeat
	Jne *-5
	tax
.elseif(n = 3468)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3480)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3482)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3483)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3513)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3554 || n = 3568 || n = 3582)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3555)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3566 || n = 3580)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3567 || n = 3581)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3583)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	delay_n 9
	inx
	inx
	Jne *-7
	tax
.elseif(n = 3584)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3585)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
.elseif(n = 3586)
	; 13 bytes
	delay_n 11
	delay_n (n-11)
.elseif(n = 3589)
	; 10 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3600 || n = 3608 || n = 3613)
	; 8..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3595 || n = 3599 || n = 3607 || n = 3612 || n = 3619 || n = 3620 || n = 3624)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3596)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3597 || n = 3617)
	; 9..12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3598 || n = 3606 || n = 3611 || n = 3614 || n = 3618)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3601)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 3602 || n = 3621)
	; 10..13 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3604)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3605)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3609)
	; 7 bytes
	php
	lda #14
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3610 || n = 3615 || n = 3622)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3616)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3623)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 3625)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3626)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3627)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3628)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3629)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3630)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3631 || n = 3632)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 3633 to 3671
.if(n = 3633)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3634)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3635 || n = 3650 || n = 3665)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3636 || n = 3640 || n = 3642 || (n > 3643 && n < 3648) || n = 3649 || n = 3651 || n = 3655 || n = 3656 || n = 3658 || (n > 3661 && n < 3665) || n = 3666 || n = 3670 || n = 3671)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3637 || n = 3653 || n = 3669)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3638 || n = 3668)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3639 || n = 3654 || n = 3661)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3641 || n = 3659)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3643)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3648)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3652 || n = 3667)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3657)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	tax
.elseif(n = 3660)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3691 || n = 3729 || n = 3748)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif((n > 3672 && n < 3677) || n = 3678 || n = 3679 || n = 3681 || (n > 3685 && n < 3691) || n = 3692 || n = 3693 || n = 3696 || n = 3700 || (n > 3703 && n < 3710) || n = 3716 || (n > 3719 && n < 3725) || n = 3726 || n = 3730 || n = 3732 || (n > 3733 && n < 3740) || n = 3741 || n = 3746)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3677 || n = 3694 || n = 3711 || n = 3728 || n = 3745)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif((n .mod 15 = 5) && ((n > 3679 && n < 3741)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif((n .mod 15 = 7) && ((n > 3681 && n < 3743)))
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3683 || n = 3698 || n = 3702 || n = 3718 || n = 3743 || n = 3750)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3684 || n = 3703 || n = 3714 || n = 3719 || n = 3744)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 16 = 5) && ((n > 3684 && n < 3750)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
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
.elseif(n = 3713 || n = 3731)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3747)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
; 3751 to 3828
.if(n = 3751 || n = 3759 || n = 3774 || n = 3789 || n = 3804 || n = 3819 || n = 3828)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif((n > 3751 && n < 3755) || n = 3756 || n = 3760 || n = 3761 || n = 3763 || n = 3764 || n = 3766 || n = 3768 || n = 3769 || n = 3771 || (n > 3774 && n < 3779) || n = 3780 || n = 3782 || n = 3783 || (n > 3789 && n < 3796) || n = 3798 || n = 3799 || n = 3801 || (n > 3805 && n < 3810))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 15 = 5) && ((n > 3754 && n < 3816)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3757 || n = 3772 || n = 3787 || n = 3802 || n = 3817 || n = 3820 || n = 3826)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3758 || n = 3773 || n = 3788 || n = 3812 || n = 3814 || n = 3816 || n = 3818 || n = 3822)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3762 || n = 3779 || n = 3796 || n = 3813)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3765 || n = 3781 || n = 3797)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3767 || n = 3786 || n = 3805 || n = 3824)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3784)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 10
	dey
	dey
	Jne *-6
	tay
.elseif(n = 3803)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	rol $36,X
	ror $36,X
	iny
	Jne *-6
	tay
.elseif(n = 3810)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3811)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 3821)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3823)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3825)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 3827)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3861)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3830)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3831 || n = 3832 || n = 3835 || n = 3840 || n = 3841 || n = 3859 || n = 3862 || n = 3867)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3833 || n = 3837 || n = 3842 || n = 3846 || n = 3848 || n = 3850 || n = 3852 || n = 3856 || n = 3858 || n = 3863)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3834 || n = 3849 || n = 3851 || n = 3864)
	; 8..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3836 || n = 3866)
	; 9..12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3838 || n = 3853)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3839 || n = 3857)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3843)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3844)
	; 13 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
	plp
.elseif(n = 3845)
	; 10 bytes
	tya
	ldy #0
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3847)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3854)
	; 13 bytes
	delay_n (n-4)
	pla
.elseif(n = 3855)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 3860)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3865)
	; 7 bytes
	php
	lda #15
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3875 || n = 3876 || n = 3880)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3869)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 3870 || n = 3874)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3871 || n = 3878)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3873)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3877)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 3879)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 3881)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3882)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 3883)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3884)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3885)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3886)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3887 || n = 3888 || n = 3890 || n = 3891)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3889)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3892 || (n > 3893 && n < 3898) || (n > 3900 && n < 3904) || n = 3905 || n = 3906)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3893)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3898)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3899)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3900)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3904)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 3907 to 4062
.if(n = 3907 || n = 3908 || n = 3910 || (n > 3911 && n < 3915) || (n > 3915 && n < 3919) || (n > 3919 && n < 3925) || (n > 3925 && n < 3929) || n = 3931 || (n > 3932 && n < 3938) || n = 3939 || n = 3940 || (n > 3941 && n < 3947) || n = 3948 || (n > 3951 && n < 3957) || (n > 3957 && n < 3965) || (n > 3966 && n < 3973) || n = 3974 || n = 3975 || (n > 3976 && n < 3981) || n = 3982 || (n > 3985 && n < 3989) || (n > 3989 && n < 3995) || (n > 3995 && n < 4000) || n = 4002 || n = 4004 || (n > 4005 && n < 4014) || n = 4015 || n = 4016 || n = 4018 || n = 4020 || (n > 4021 && n < 4033) || n = 4035 || n = 4036 || (n > 4037 && n < 4051) || n = 4054 || (n > 4055 && n < 4063))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 16 = 5) && ((n > 3908 && n < 4054)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3911 || n = 3929 || n = 3965 || n = 4001 || n = 4019 || n = 4055)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3915 || n = 3932 || n = 3949 || n = 3966 || n = 3983 || n = 4000 || n = 4017 || n = 4034 || n = 4051)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3919 || n = 3938 || n = 3976 || n = 3995 || n = 4014 || n = 4033 || n = 4052)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3930)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	delay_n 6
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3947)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3950 || n = 3984)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3951 || n = 3985)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3981)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 4003)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 13
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	.repeat 5
	 pha
	.endrepeat
	Jne *-6
	tax
.elseif(n = 4064 || n = 4065)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4066 || n = 4074 || n = 4076 || n = 4082 || n = 4088 || n = 4094)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4067 || n = 4072 || n = 4077 || n = 4083 || n = 4089 || n = 4095)
	; 12..13 bytes
	pla
	delay_n (n-4)
.elseif(n = 4068 || n = 4085)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4069)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4070 || n = 4081 || n = 4087 || n = 4092 || n = 4093)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4071 || n = 4090)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4073 || n = 4091)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4075 || n = 4097)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4078 || n = 4086)
	; 13..14 bytes
	delay_n (n-4)
	pla
.elseif(n = 4079 || n = 4100)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4080)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 4084)
	; 14 bytes
	php
	sec
	lda #(n-10)/21
	nop
	delay_n 14
	sbc #1
	Jne *-7
	plp
.elseif(n = 4096)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4098)
	; 13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4099)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 4101)
	; 11 bytes
	tya
	ldy #0
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.else
; 4102 to 4140
.if(n = 4102)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4103 || n = 4104 || n = 4111 || n = 4118 || n = 4123 || n = 4126 || n = 4130)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4112 || n = 4119 || n = 4124 || n = 4131 || n = 4132 || n = 4136)
	; 8..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4106 || n = 4113 || n = 4120 || n = 4125)
	; 8..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4107)
	; 13 bytes
	php
	sec
	lda #241
	rol $36,X
	ror $36,X
	sbc #1
	Jne *-6
	plp
.elseif(n = 4108)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4109)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4110 || n = 4117 || n = 4129)
	; 9..13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4114)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4115 || n = 4122 || n = 4127 || n = 4134)
	; 9..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4116)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4121)
	; 7 bytes
	php
	lda #16
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4128)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4133)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4135)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 4137)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4138)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 4139)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4140)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4142)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4143 || n = 4144 || n = 4146)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4145)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4147 || n = 4166 || n = 4185)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif((n > 4147 && n < 4153) || n = 4154 || (n > 4155 && n < 4160) || n = 4162 || n = 4164 || n = 4165 || (n > 4166 && n < 4170) || (n > 4170 && n < 4179) || n = 4180 || (n > 4181 && n < 4185) || n = 4186 || (n > 4187 && n < 4199) || (n > 4199 && n < 4203) || (n > 4204 && n < 4215) || n = 4216 || n = 4218)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4153 || n = 4170 || n = 4187 || n = 4204)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4155)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4160)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4161)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4163 || n = 4181 || n = 4199 || n = 4217)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4179)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4203)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 14
	iny
	Jne *-6
	tay
.elseif(n = 4215)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.endif
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4220 || n = 4222 || (n > 4223 && n < 4235) || n = 4236 || n = 4237 || (n > 4238 && n < 4242) || (n > 4242 && n < 4253) || n = 4254 || (n > 4255 && n < 4261) || (n > 4261 && n < 4266) || (n > 4266 && n < 4271) || (n > 4272 && n < 4280) || (n > 4280 && n < 4289) || (n > 4289 && n < 4299) || (n > 4299 && n < 4306) || (n > 4307 && n < 4318) || (n > 4318 && n < 4322))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4221 || n = 4238 || n = 4255 || n = 4272 || n = 4289 || n = 4306 || n = 4323 || n = 4340)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4223 || n = 4242 || n = 4261 || n = 4280 || n = 4299 || n = 4318 || n = 4337 || n = 4356)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4235 || n = 4253 || n = 4271 || n = 4307 || n = 4325 || n = 4343 || n = 4361)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4266)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 14
	Jne *-6
	tax
.elseif(n = 4322 || n = 4327 || n = 4329 || n = 4335 || n = 4341 || n = 4344 || n = 4347 || n = 4353)
	; 12..14 bytes
	pla
	delay_n (n-4)
.elseif(n = 4324 || n = 4326 || n = 4328 || n = 4332 || n = 4334 || n = 4346 || n = 4350 || n = 4360 || n = 4364 || n = 4368 || n = 4375)
	; 8..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4330 || n = 4338)
	; 13..14 bytes
	delay_n (n-4)
	pla
.elseif(n = 4331 || n = 4352 || n = 4373)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4333 || n = 4339 || n = 4342 || n = 4354 || n = 4358 || n = 4359 || n = 4363 || n = 4366 || n = 4374)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4336)
	; 14 bytes
	php
	sec
	lda #(n-10)/21
	nop
	delay_n 14
	sbc #1
	Jne *-7
	plp
.elseif(n = 4345 || n = 4365)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4348 || n = 4351 || n = 4369)
	; 13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4349)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 4355)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 4357)
	; 11 bytes
	tya
	ldy #0
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4362)
	; 13 bytes
	php
	sec
	lda #0
	rol $36,X
	ror $36,X
	sbc #1
	Jne *-6
	plp
.elseif(n = 4367 || n = 4370)
	; 13..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4371)
	; 14 bytes
	tya
	pha
	ldy #37 ;hides 'and zp'
	iny ;first loop only
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	pla
	tay
.elseif(n = 4372)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4381)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4377)
	; 7 bytes
	php
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4378 || n = 4383 || n = 4390)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4379 || n = 4382 || n = 4386)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4380 || n = 4387 || n = 4388 || n = 4392)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4384)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4385)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4389)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4391)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 4393)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4394 || n = 4413 || n = 4432 || n = 4451 || n = 4470 || n = 4489 || n = 4508 || n = 4527)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4395)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4396)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4397 || n = 4415 || n = 4433 || n = 4469 || n = 4487 || n = 4505 || n = 4523)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4398)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4399 || n = 4400 || n = 4402 || n = 4403)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4401)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 4403 && n < 4411) || n = 4412 || n = 4414 || (n > 4416 && n < 4432) || (n > 4433 && n < 4449) || n = 4450 || (n > 4451 && n < 4469) || (n > 4470 && n < 4487) || n = 4488 || (n > 4489 && n < 4505) || n = 4506 || n = 4507 || (n > 4508 && n < 4523) || (n > 4523 && n < 4527) || (n > 4527 && n < 4532))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4411)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4416)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4449)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4541) || (n > 4541 && n < 4546) || (n > 4546 && n < 4559) || (n > 4559 && n < 4565) || (n > 4565 && n < 4577))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4541 || n = 4559 || n = 4577 || n = 4595)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4546 || n = 4565 || n = 4584 || n = 4603)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4578)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4579 || n = 4586 || n = 4593 || n = 4597 || n = 4600 || n = 4605)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4580)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 14
	dey
	dey
	Jne *-7
	tay
.elseif(n = 4581)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4582)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 4583 || n = 4604)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4585)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4587 || n = 4598 || n = 4602 || n = 4606)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4588 || n = 4599 || n = 4607)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4589 || n = 4608)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4590 || n = 4601 || n = 4609)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4591 || n = 4594)
	; 13..14 bytes
	delay_n (n-4)
	pla
.elseif(n = 4592)
	; 13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4596)
	; 14 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610)
	; 13 bytes
	delay_n (n-4)
	pla
.elseif(n = 4611 || n = 4621 || n = 4641)
	; 9..13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4612 || n = 4615 || n = 4618 || n = 4624 || n = 4630 || n = 4635 || n = 4638 || n = 4642)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4613)
	; 11 bytes
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4614 || n = 4616 || n = 4620 || n = 4631 || n = 4636 || n = 4643 || n = 4644 || n = 4648)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4617 || n = 4626 || n = 4632 || n = 4637)
	; 8..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4619 || n = 4634 || n = 4639 || n = 4646)
	; 9..13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4622)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4623)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	delay_n 9
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4625)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4627)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4628)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4629)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4633)
	; 7 bytes
	php
	lda #18
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4640)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4645)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4647)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4650)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 4651)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4652)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4653)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4654)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4655 || n = 4656 || n = 4658 || n = 4659)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4657)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4660 || n = 4679)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif((n > 4660 && n < 4667) || (n > 4667 && n < 4672) || (n > 4672 && n < 4679) || (n > 4679 && n < 4683) || (n > 4683 && n < 4688))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4667)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4672)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4683)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if((n > 4687 && n < 4698) || (n > 4698 && n < 4717) || (n > 4717 && n < 4736) || (n > 4736 && n < 4755) || (n > 4755 && n < 4774) || (n > 4774 && n < 4793) || (n > 4793 && n < 4812) || (n > 4812 && n < 4831) || n = 4832 || n = 4833)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4698 || n = 4717 || n = 4736 || n = 4755 || n = 4774 || n = 4793 || n = 4812 || n = 4831)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4834 || n = 4836)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4835)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4837)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif(n = 4838)
	; 13 bytes
	delay_n (n-4)
	pla
.elseif(n = 4839)
	; 13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4840)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4841)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 4842)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4843)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844)
	; 14 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	adc #1
	Jne *-8
	plp
.elseif(n = 4845 || n = 4865)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4846)
	; 14 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4847 || n = 4852 || n = 4871)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4848 || n = 4853 || n = 4859 || n = 4866 || n = 4868 || n = 4870 || n = 4872 || n = 4878)
	; 12..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 4849 || n = 4860 || n = 4873 || n = 4879)
	; 12..13 bytes
	pla
	delay_n (n-4)
.elseif(n = 4850)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4851)
	; 14 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4854)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	rol $36,X
	ror $36,X
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4855)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4856 || n = 4877)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4857 || n = 4863 || n = 4876 || n = 4882)
	; 13..14 bytes
	delay_n (n-4)
	pla
.elseif(n = 4858 || n = 4881)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 4861)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4862 || n = 4864)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4867)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4869)
	; 11 bytes
	tya
	ldy #0
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4874)
	; 13 bytes
	php
	sec
	lda #0
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4875)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 14
	iny
	Jne *-6
	tay
.elseif(n = 4880)
	; 13 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 14
	adc #1
	Jne *-7
	plp
.endif
.else
; 4883 to 4921
.if(n = 4883 || n = 4897)
	; 9..13 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4884)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4885)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4886 || n = 4891 || n = 4894 || n = 4898)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4887 || n = 4892 || n = 4899 || n = 4900 || n = 4904)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4888 || n = 4893)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 4889)
	; 7 bytes
	php
	lda #19
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4890 || n = 4895 || n = 4902)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4896)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4901)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 4903)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 4905)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4906)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 4907)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4908)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4909)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4910)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4911 || n = 4912 || n = 4914 || n = 4915)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4913)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4916 || (n > 4917 && n < 4922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4917)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922 || (n > 4923 && n < 4928) || (n > 4928 && n < 5001))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4923)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4928)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
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
.if((n > 5000 && n < 5079))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n > 5078 && n < 5090))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5090 || n = 5091)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5092 || n = 5099 || n = 5104 || n = 5112)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5093 || n = 5113)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 5093 && n < 5097))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5097 || n = 5116)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5098)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5100)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5101)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 5102)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	plp
	Jmi *-8
	pla
	tax
.elseif(n = 5103 || n = 5110)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5105)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 5106)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 21
	sbc #1
	Jne *-8
	plp
.elseif(n = 5107)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	.repeat 5
	 pha
	.endrepeat
	iny
	Jne *-7
	tay
.elseif(n = 5108)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5109)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5111)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5114)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5115)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5117)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.endif
.else
; 5118 to 5156
.if(n = 5118)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5119 || n = 5126 || n = 5133 || n = 5142 || n = 5147 || n = 5150 || n = 5154)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5120 || n = 5128 || n = 5132 || n = 5143 || n = 5148 || n = 5155 || n = 5156)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5121 || n = 5135 || n = 5144 || n = 5149)
	; 8..14 bytes
	pla
	delay_n (n-4)
.elseif(n = 5122)
	; 15 bytes
	php
	sec
	lda #213
	php
	rol $36,X
	ror $36,X
	plp
	sbc #1
	Jne *-8
	plp
.elseif(n = 5123)
	; 15 bytes
	php
	tya
	pha
	ldy #176
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5124 || n = 5136)
	; 14..15 bytes
	delay_n (n-4)
	pla
.elseif(n = 5125)
	; 12 bytes
	tya
	ldy #0
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 5127)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 5129)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5130)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5131)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5134)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5137 || n = 5146 || n = 5151)
	; 9..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5138)
	; 14 bytes
	tya
	ldy #177
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 5139 || n = 5153)
	; 9..14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5140)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5141)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 5145)
	; 7 bytes
	php
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5152)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5158)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5159)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 5160)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5161)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5162)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5163)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5164)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5165)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5166)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5167 || n = 5168 || n = 5170 || n = 5171)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5169)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5171 && n < 5179) || (n > 5179 && n < 5184) || (n > 5184 && n < 5313))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5179)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5184)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if((n > 5312 && n < 5346))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5346 || n = 5347)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5348 || n = 5354 || n = 5358 || n = 5363 || n = 5376 || n = 5384 || n = 5388)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5349 || n = 5374 || n = 5377 || n = 5389)
	; 13..15 bytes
	pla
	delay_n (n-4)
.elseif(n = 5350 || n = 5352)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5351 || n = 5373)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5353 || n = 5356 || n = 5359 || n = 5362 || n = 5366 || n = 5369 || n = 5375 || n = 5382 || n = 5383)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5355 || n = 5380)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5357)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 5360)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5361)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 5364 || n = 5387)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5365)
	; 14 bytes
	php
	sec
	lda #(n-10)/21
	nop
	delay_n 14
	sbc #1
	Jne *-7
	plp
.elseif(n = 5367)
	; 14 bytes
	delay_n (n-4)
	pla
.elseif(n = 5368)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5370)
	; 14 bytes
	tya
	ldy #185
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 5371)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5372)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5378)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 5379)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5381)
	; 12 bytes
	tya
	ldy #0
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5385)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5386)
	; 14 bytes
	php
	sec
	lda #0
	nop
	delay_n 14
	sbc #1
	Jne *-7
	plp
.elseif(n = 5390)
	; 14 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	delay_n 14
	sbc #1
	Jne *-8
	plp
.endif
.else
; 5391 to 5468
.if(n = 5391 || n = 5400 || n = 5405)
	; 8..14 bytes
	pla
	delay_n (n-4)
.elseif(n = 5392)
	; 14 bytes
	delay_n (n-4)
	pla
.elseif(n = 5393 || n = 5409)
	; 9..14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5394)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5395)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5396)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5397 || n = 5398 || n = 5403 || n = 5406 || n = 5410)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5399 || n = 5404 || n = 5411 || n = 5412 || n = 5416)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5401)
	; 7 bytes
	php
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5402 || n = 5407 || n = 5414)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5408)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5413)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5415)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 5417)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5418)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5419)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5420)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5421)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5422)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5423 || n = 5424 || n = 5426 || n = 5427)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5425)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5427 && n < 5435) || (n > 5435 && n < 5440) || (n > 5440 && n < 5469))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5435)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5440)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5602))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5602 || n = 5603)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5604 || n = 5611 || n = 5618 || n = 5622 || n = 5624)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5605 || n = 5623 || n = 5625)
	; 13..14 bytes
	pla
	delay_n (n-4)
.elseif((n > 5605 && n < 5609))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5609)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5610)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5612 || n = 5613)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5614)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 5615)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5616)
	; 15 bytes
	php
	tya
	pha
	ldy #193
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5617)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5619)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5620)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5621)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
.if((n >= 5626) && (n < 5665))
; 5626 to 5664
.if(n = 5626 || n = 5644 || n = 5650 || n = 5655 || n = 5660)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5627 || n = 5629)
	; 14..15 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5628)
	; 15 bytes
	delay_n (n-4)
	pla
.elseif(n = 5630)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5631)
	; 14 bytes
	tya
	ldy #194
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 5632 || n = 5635 || n = 5639 || n = 5642 || n = 5649 || n = 5654 || n = 5659 || n = 5662)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5633)
	; 14 bytes
	tya
	ldy #201
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 5634 || n = 5641 || n = 5651 || n = 5656 || n = 5661)
	; 8..14 bytes
	pla
	delay_n (n-4)
.elseif(n = 5636 || n = 5643 || n = 5658 || n = 5663)
	; 9..15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5637)
	; 12 bytes
	tya
	ldy #0
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5638)
	; 15 bytes
	tya
	pha
	ldy #96
	delay_n 53
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 5640)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5645)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 5646)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 5647)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 5648)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 5652)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5653)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 94
	Jmi *-8
	pla
	tay
.elseif(n = 5657)
	; 7 bytes
	php
	lda #22
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5664)
	; 9 bytes
	php
	pha
	lda #22
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 5665 to 5703
.if(n = 5665)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5666)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667 || n = 5668 || n = 5672)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5669)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5670)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5671)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 5673)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5674)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5675)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5676)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5677)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5678)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5679 || n = 5680 || n = 5682 || n = 5683)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5681)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5683 && n < 5691) || (n > 5691 && n < 5696) || (n > 5696 && n < 5704))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5691)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5696)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 5704 to 5781
.if((n > 5703 && n < 5782))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5858))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5858 || n = 5859)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860 || n = 5867 || n = 5873 || n = 5884 || n = 5888 || n = 5896)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5861 || n = 5874 || n = 5889 || n = 5897)
	; 13..14 bytes
	pla
	delay_n (n-4)
.elseif((n > 5861 && n < 5865))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5865)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5866)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5868 || n = 5869)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5870)
	; 13 bytes
	tya
	ldy #(n-5)/23
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5871)
	; 15 bytes
	php
	tya
	pha
	ldy #133
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5872 || n = 5882 || n = 5883 || n = 5887 || n = 5895)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5875)
	; 15 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 14
	iny
	Jne *-8
	tay
	plp
.elseif(n = 5876)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 14
	iny
	Jne *-8
	tay
	plp
.elseif(n = 5877)
	; 15 bytes
	delay_n (n-4)
	pla
.elseif(n = 5878)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	php
	pha
	lda #189
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 5879)
	; 15 bytes
	tya
	ldy #178
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 5880)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5881)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 5885)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 5886)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 21
	sbc #1
	Jne *-8
	plp
.elseif(n = 5890)
	; 13 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	delay_n 21
	Jne *-8
	tax
.elseif(n = 5891)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 5892)
	; 14 bytes
	tya
	ldy #203
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 5893)
	; 13 bytes
	tya
	ldy #0
	nop
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5894)
	; 14 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	Jne *-9
	tay
.elseif(n = 5898)
	; 15 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 14
	iny
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 5899) && (n < 5918))
; 5899 to 5917
.if(n = 5899)
	; 15 bytes
	php
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 14
	dey
	Jne *-8
	tay
	plp
.elseif(n = 5900)
	; 15 bytes
	delay_n (n-4)
	pla
.elseif(n = 5901)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	plp
	Jne *-8
	pla
	tay
.elseif(n = 5902)
	; 15 bytes
	txa
	pha
	tya
	ldx #108
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 5903)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	php
	pha
	lda #190
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 5904)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 728
	pla
	Jcc *-11
	plp
.elseif(n = 5905)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5906)
	; 15 bytes
	php
	tya
	pha
	ldy #203
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 5907)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 5908)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5909)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 5910 || n = 5915)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 5911 || n = 5916)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 5912 || n = 5917)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 5913)
	; 7 bytes
	php
	lda #23
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5914)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 5918 to 5937
.if(n = 5918 || n = 5922)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5919 || n = 5926)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5921)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5923 || n = 5924 || n = 5928)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5925)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 5927)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 5929)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5930)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5931)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5932)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5933)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5934)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5935 || n = 5936)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5937)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938 || n = 5939)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 5939 && n < 5947) || (n > 5947 && n < 5952) || (n > 5952 && n < 6094))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5947)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5952)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6114))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6114 || n = 6115)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6116 || n = 6123 || n = 6128)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6117 || n = 6129)
	; 13..14 bytes
	pla
	delay_n (n-4)
.elseif((n > 6117 && n < 6121))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6121)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6122)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6124)
	; 14 bytes
	tya
	ldy #211
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6125)
	; 13 bytes
	tya
	ldy #(n-5)/24
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 6126 || n = 6127 || n = 6132)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6130)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 6131)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 6133 to 6171
.if(n = 6133 || n = 6140 || n = 6144 || n = 6152 || n = 6158 || n = 6162 || n = 6167)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6134 || n = 6161)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 6135)
	; 15 bytes
	php
	tya
	pha
	ldy #139
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6136 || n = 6139 || n = 6143 || n = 6151 || n = 6157 || n = 6163 || n = 6166 || n = 6171)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6137)
	; 14 bytes
	tya
	ldy #219
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6138)
	; 15 bytes
	php
	tya
	pha
	ldy #211
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6141)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 6142)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6145 || n = 6153 || n = 6159 || n = 6168)
	; 8..14 bytes
	pla
	delay_n (n-4)
.elseif(n = 6146)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 21
	sbc #1
	Jne *-8
	plp
.elseif(n = 6147 || n = 6170)
	; 9..15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6148 || n = 6156)
	; 15 bytes
	delay_n (n-4)
	pla
.elseif(n = 6149)
	; 13 bytes
	tya
	ldy #0
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-7
	tay
.elseif(n = 6150)
	; 15 bytes
	tya
	pha
	ldy #6
	delay_n 1018
	dey
	Jne *-7
	pla
	tay
.elseif(n = 6154)
	; 15 bytes
	php
	sec
	lda #0
	php
	rol $36,X
	ror $36,X
	plp
	sbc #1
	Jne *-8
	plp
.elseif(n = 6155)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 6160)
	; 15 bytes
	txa
	pha
	tya
	ldy #15
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 6164)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6165)
	; 14 bytes
	tya
	ldy #220
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6169)
	; 7 bytes
	php
	lda #24
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172 || n = 6179 || n = 6180 || n = 6184)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 6173)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 6174 || n = 6178)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6175 || n = 6182)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6176)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6177)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6181)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6183)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 6185)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6186)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6187)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6188)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6189)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6190)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6191 || n = 6192 || n = 6194 || n = 6195)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6193)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6195 && n < 6203) || (n > 6203 && n < 6208) || (n > 6208 && n < 6251))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6203)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6208)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
.if((n >= 6251) && (n < 6407))
.if((n >= 6251) && (n < 6329))
; 6251 to 6328
.if((n > 6250 && n < 6329))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if((n > 6328 && n < 6368))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 6368) && (n < 6387))
; 6368 to 6386
.if(n = 6368 || n = 6369)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6370 || n = 6371)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6372 || n = 6378 || n = 6383)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6373 || n = 6379 || n = 6384)
	; 13..14 bytes
	pla
	delay_n (n-4)
.elseif(n = 6374 || n = 6376)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6375)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 6377)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6380)
	; 13 bytes
	tya
	ldy #(n-5)/25
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 6381)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6382)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6385)
	; 14 bytes
	tya
	ldy #220
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6386)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 15 bytes
	php
	tya
	ldy #124
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jpl *-7
	tay
	plp
.elseif(n = 6388)
	; 15 bytes
	txa
	pha
	tya
	ldx #118
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 6389)
	; 14 bytes
	tya
	ldy #228
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6390)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	iny
	iny
	Jne *-9
	tay
	plp
.elseif(n = 6391)
	; 15 bytes
	tya
	ldy #(n-5)/31
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 6392)
	; 15 bytes
	tya
	pha
	ldy #110
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 6393)
	; 15 bytes
	pla
	delay_n (n-4)
.elseif(n = 6394)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 107
	Jmi *-8
	pla
	tay
.elseif(n = 6395)
	; 15 bytes
	tya
	ldy #213
	nop
	nop
	delay_n 21
	dey
	Jne *-9
	tay
.elseif(n = 6396)
	; 15 bytes
	tya
	pha
	ldy #47
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 6397)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #24
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6398)
	; 16 bytes
	php
	tya
	ldy #62
	pha
	lda #64
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6399)
	; 15 bytes
	php
	tya
	pha
	ldy #220
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6400)
	; 16 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 21
	dey
	Jne *-9
	tay
	plp
.elseif(n = 6401)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 6402)
	; 16 bytes
	php
	tya
	ldy #142
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6403)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6404)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 6405)
	; 13 bytes
	tya
	ldy #0
	pha
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 6406)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 21
	sbc #1
	Jne *-8
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
.if(n = 6407 || n = 6419 || n = 6422 || n = 6427 || n = 6430 || n = 6434)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6408 || n = 6423 || n = 6428 || n = 6435 || n = 6436 || n = 6440)
	; 8..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6409 || n = 6418 || n = 6421 || n = 6424 || n = 6429)
	; 8..15 bytes
	pla
	delay_n (n-4)
.elseif(n = 6410)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 251
	Jne *-8
	pla
	tax
.elseif(n = 6411 || n = 6426 || n = 6431 || n = 6438)
	; 9..15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6412)
	; 15 bytes
	tya
	ldy #149
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 6413 || n = 6433)
	; 9..15 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6414)
	; 14 bytes
	tya
	ldy #221
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6415)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6416)
	; 15 bytes
	txa
	pha
	tya
	ldx #234 ;hides 'nop'
	ldy #200 ;hides 'iny'
	Jmi *-1
	inx
	Jne *-6
	tay
	pla
	tax
.elseif(n = 6417)
	; 14 bytes
	tya
	ldy #229
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6420)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6425)
	; 7 bytes
	php
	lda #25
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6432)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6437)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6439)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 6441)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6442)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6443)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6444)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6445)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 6446 to 6484
.if(n = 6446)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6447 || n = 6448 || n = 6450 || n = 6451)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6449)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6451 && n < 6459) || (n > 6459 && n < 6464) || (n > 6464 && n < 6485))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6459)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6464)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6563))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6626))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6626 || n = 6627)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6628 || n = 6638)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6629 || n = 6639)
	; 13..14 bytes
	pla
	delay_n (n-4)
.elseif((n > 6629 && n < 6633))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6633)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6634)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6635)
	; 13 bytes
	tya
	ldy #(n-5)/26
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 6636)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6637)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6640)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	delay_n 21
	sbc #1
	Jne *-8
	plp
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 14 bytes
	tya
	ldy #237
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6642)
	; 15 bytes
	delay_n (n-4)
	pla
.elseif(n = 6643 || n = 6648 || n = 6649)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6644 || n = 6650)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6645)
	; 15 bytes
	pla
	delay_n (n-4)
.elseif(n = 6646)
	; 14 bytes
	tya
	ldy #229
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6647)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 6651)
	; 15 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6652)
	; 16 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 21
	dey
	Jne *-9
	tay
	plp
.elseif(n = 6653)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	php
	pha
	lda #220
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 6654)
	; 16 bytes
	php
	tya
	ldy #54
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6655)
	; 16 bytes
	php
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #220
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
	plp
.elseif(n = 6656)
	; 16 bytes
	php
	tya
	ldy #151
	pha
	lda #5
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6657)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #132
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 6658)
	; 15 bytes
	txa
	pha
	tya
	ldx #122
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 6659)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	iny
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 15 bytes
	php
	tya
	pha
	ldy #229
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6661)
	; 13 bytes
	tya
	ldy #0
	delay_n 21
	dey
	Jne *-7
	tay
.elseif(n = 6662)
	; 15 bytes
	tya
	pha
	ldy #49
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 6663 || n = 6678)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6664 || n = 6677 || n = 6679)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6665 || n = 6673)
	; 14..15 bytes
	pla
	delay_n (n-4)
.elseif(n = 6666)
	; 15 bytes
	php
	sec
	lda #0
	delay_n 21
	sbc #1
	Jne *-8
	plp
.elseif(n = 6667)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6668)
	; 15 bytes
	delay_n (n-4)
	pla
.elseif(n = 6669)
	; 14 bytes
	tya
	ldy #238
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6670)
	; 15 bytes
	tya
	ldy #(n-5)/31
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 6671)
	; 15 bytes
	tya
	ldy #202
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 6672)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6674)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 6675)
	; 14 bytes
	tya
	ldy #230
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6676)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6685)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 6681)
	; 7 bytes
	php
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6682 || n = 6687 || n = 6694)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6683 || n = 6686 || n = 6690)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6684 || n = 6691 || n = 6692 || n = 6696)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6689)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6693)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6695)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 6697)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6698)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6699)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6700)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6701)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6702)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6703 || n = 6704 || n = 6706 || n = 6707)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6705)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6707 && n < 6715) || (n > 6715 && n < 6719))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6715)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 6719 to 6875
.if(n = 6719 || (n > 6720 && n < 6876))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6720)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
.if((n >= 6876) && (n < 6895))
; 6876 to 6894
.if((n > 6875 && n < 6882))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6882 || n = 6883)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6884 || n = 6891)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6885 || n = 6894)
	; 13..15 bytes
	pla
	delay_n (n-4)
.elseif((n > 6885 && n < 6889))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6889)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6890)
	; 14 bytes
	tya
	ldy #(n-5)/27
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 6892)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6893)
	; 14 bytes
	tya
	ldy #246
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.endif
.else
; 6895 to 6914
.if(n = 6895)
	; 15 bytes
	tya
	ldy #106
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 6896)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6897)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6898)
	; 16 bytes
	php
	sec
	lda #246
	nop
	delay_n 21
	sbc #1
	Jne *-9
	plp
.elseif(n = 6899 || n = 6909)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6900)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #75
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 6901)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6902)
	; 15 bytes
	tya
	ldy #209
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 6903)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	php
	pha
	lda #230
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 6904)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	delay_n 850
	pla
	Jcc *-11
	plp
.elseif(n = 6905)
	; 15 bytes
	tya
	ldy #22
	delay_n 291
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 6906)
	; 15 bytes
	txa
	pha
	tya
	ldx #9
	ldy #136 ;hides 'dey'
	Jne *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 6907)
	; 14 bytes
	tya
	ldy #238
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6908)
	; 15 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 291
	pla
	Jcs *-10
	plp
.elseif(n = 6910)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 271
	Jne *-8
	pla
	tax
.elseif(n = 6911)
	; 15 bytes
	pla
	delay_n (n-4)
.elseif(n = 6912)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6913)
	; 16 bytes
	php
	tya
	ldy #102
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 6914)
	; 15 bytes
	tya
	pha
	ldy #119
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.endif
.endif
.else
; 6915 to 6953
.if(n = 6915)
	; 16 bytes
	php
	tya
	ldy #177
	pha
	lda #0
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 6916)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6917)
	; 14 bytes
	tya
	ldy #0
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dey
	Jne *-8
	tay
.elseif(n = 6918)
	; 15 bytes
	tya
	ldy #(n-5)/31
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 6919)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #45
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 6920 || n = 6935 || n = 6940 || n = 6947 || n = 6948 || n = 6952)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6921)
	; 14 bytes
	tya
	ldy #247
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 6922)
	; 16 bytes
	php
	tya
	pha
	ldy #177
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6923 || n = 6929 || n = 6934 || n = 6939 || n = 6942 || n = 6946)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6924)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 6925 || n = 6936 || n = 6941)
	; 8..15 bytes
	pla
	delay_n (n-4)
.elseif(n = 6926)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 86
	Jmi *-8
	pla
	tax
.elseif(n = 6927)
	; 15 bytes
	php
	tya
	pha
	ldy #157
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 6928)
	; 15 bytes
	txa
	pha
	tya
	ldy #18
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 6930)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #132
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 6931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 6932)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6933)
	; 14 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 21
	dex
	dex
	Jne *-9
	tax
.elseif(n = 6937)
	; 7 bytes
	php
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6938 || n = 6943 || n = 6950)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6944)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6945)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6949)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 6951)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 6953)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6955)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6956)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6957)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6958)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6959 || n = 6960 || n = 6962 || n = 6963)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6961)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 6963 && n < 6971) || (n > 6971 && n < 6976) || (n > 6976 && n < 7032))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6971)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6976)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7110))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if((n > 7109 && n < 7138))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7138 || n = 7139)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7140 || n = 7147)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7141)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 7141 && n < 7145))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7145)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7146)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7148)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7150)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 7151)
	; 16 bytes
	php
	tya
	ldy #(n-133)/121
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7152)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7153)
	; 15 bytes
	php
	tya
	pha
	ldy #246
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7154)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 89
	Jmi *-8
	pla
	tax
.elseif(n = 7155)
	; 15 bytes
	tya
	ldy #110
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7156)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7157)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #51
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7158)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-140)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7159)
	; 16 bytes
	php
	tya
	pha
	ldy #119
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7160)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 281
	Jne *-8
	pla
	tax
.elseif(n = 7161)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #13
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7162)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	delay_n 1010
	pla
	Jcs *-11
	plp
.elseif(n = 7163)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #6
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7164)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	php
	rol $36,X
	ror $36,X
	plp
	dex
	dex
	Jne *-10
	tax
.elseif(n = 7165)
	; 16 bytes
	php
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #237
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
	plp
.elseif(n = 7166)
	; 15 bytes
	tya
	ldy #(n-5)/31
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7167)
	; 16 bytes
	php
	tya
	ldy #135
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 14 bytes
	tya
	ldy #247
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 7169)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #78
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7170 || n = 7175 || n = 7184)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7171)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7172)
	; 15 bytes
	tya
	ldy #234 ;hides 'nop'
	pha
	delay_n 53
	pla
	dey
	Jmi *-10
	tay
.elseif(n = 7173)
	; 14 bytes
	tya
	ldy #0
	nop
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 7174)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #(n-531)/511
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7176)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7177)
	; 15 bytes
	pla
	delay_n (n-4)
.elseif(n = 7178)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	php
	pha
	lda #241
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 7179)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #8
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7180)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 7181)
	; 16 bytes
	php
	tya
	ldy #107
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7182)
	; 15 bytes
	php
	tya
	pha
	ldy #247
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7183)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7185)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 282
	Jne *-8
	pla
	tax
.elseif(n = 7186)
	; 15 bytes
	tya
	ldy #167
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7187)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #178
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if(n = 7188)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7189)
	; 16 bytes
	php
	tya
	pha
	ldy #29
	lda #207
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7190 || n = 7195 || n = 7198 || n = 7202)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7191 || n = 7196 || n = 7203 || n = 7204 || n = 7208)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 7192 || n = 7197)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 7193)
	; 7 bytes
	php
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7194 || n = 7199 || n = 7206)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7200)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7201)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7205)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7207)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 7209)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7210)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7211)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7212)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7213)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7214)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7215 || n = 7216 || n = 7218 || n = 7219)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7217)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7219 && n < 7227) || (n > 7227 && n < 7232) || (n > 7232 && n < 7344))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7227)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7232)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7394))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7394 || n = 7395)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7396)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 7397)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 7397 && n < 7401))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7401)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7402)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7403)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	php
	pha
	lda #250
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 7404 || n = 7405)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7406)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #51
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7407)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7408)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 49
	Jne *-8
	pla
	tay
.elseif(n = 7409)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7410)
	; 16 bytes
	php
	tya
	ldy #54
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7411)
	; 15 bytes
	php
	tya
	pha
	ldy #168
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7412)
	; 16 bytes
	php
	tya
	ldy #185
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7413)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 7414)
	; 15 bytes
	php
	tya
	pha
	ldy #255
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7415)
	; 15 bytes
	tya
	ldy #113
	pha
	delay_n 53
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 7416)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7417)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7418)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7419)
	; 16 bytes
	php
	tya
	pha
	ldy #<((n-19)/5)
	lda #>((n-19)/5)
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7420)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 125
	Jmi *-8
	pla
	tay
.elseif(n = 7421)
	; 16 bytes
	php
	tya
	ldy #31
	pha
	lda #200
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 16 bytes
	php
	tya
	ldy #195
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7423)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7424)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 918
	pla
	Jcc *-11
	plp
.elseif(n = 7425)
	; 15 bytes
	tya
	ldy #52
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 7426)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 7427)
	; 15 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	nop
	delay_n 21
	iny
	Jne *-10
	tay
.elseif(n = 7428)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7429)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 21
	dey
	Jne *-8
	tay
.elseif(n = 7430)
	; 15 bytes
	tya
	ldy #225
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 7431)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7432)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7433)
	; 15 bytes
	pla
	delay_n (n-4)
.elseif(n = 7434)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #0
	jsr delay_256a_16_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 7435)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7436)
	; 15 bytes
	tya
	pha
	ldy #127
	delay_n 53
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 7437)
	; 16 bytes
	php
	tya
	ldy #225
	pha
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7438)
	; 16 bytes
	php
	tya
	ldy #158
	pha
	lda #8
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7439)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #3
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7440)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 16 bytes
	tya
	ldy #143
	pha
	tya
	ldy #6
	dey
	Jpl *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 7442)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	delay_n 1050
	pla
	Jcs *-11
	plp
.elseif(n = 7443)
	; 15 bytes
	php
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7444)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7445)
	; 15 bytes
	tya
	ldy #(n-5)/31
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7446 || n = 7451 || n = 7454 || n = 7458)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7447 || n = 7452 || n = 7459 || n = 7460)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7448 || n = 7453)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 7449)
	; 7 bytes
	php
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7450 || n = 7455)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7456)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7457)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7462)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7463)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 7464)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 7465)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7466)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7467)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7468)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7469)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7470)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7471 || n = 7472 || n = 7474 || n = 7475)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7473)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7475 && n < 7483) || (n > 7483 && n < 7488) || (n > 7488 && n < 7501))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7483)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7488)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
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
.if((n > 7500 && n < 7650))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7650 || n = 7651)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7652)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 7653)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 7653 && n < 7657))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7658)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7659)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7660 || n = 7661)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7662)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7663)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 7664)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7665)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	php
	pha
	lda #201
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-11
	plp
.elseif(n = 7666)
	; 16 bytes
	php
	tya
	ldy #178
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7667)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #190
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	plp
.elseif(n = 7668)
	; 15 bytes
	tya
	pha
	ldy #132
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7669)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7670)
	; 16 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1268
	pla
	Jcc *-10
	plp
.elseif(n = 7671)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7672)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7673)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 7674)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7675)
	; 15 bytes
	php
	tya
	pha
	ldy #174
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7677)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7678)
	; 16 bytes
	php
	tya
	pha
	ldy #207
	lda #5
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7679)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #55
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 7680)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7681)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7682)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 51
	Jne *-8
	pla
	tay
.elseif(n = 7683)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7684)
	; 15 bytes
	txa
	pha
	tya
	ldx #142
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 7685)
	; 15 bytes
	tya
	ldy #0
	nop
	nop
	delay_n 21
	dey
	Jne *-9
	tay
.elseif(n = 7686)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 96
	Jmi *-8
	pla
	tax
.elseif(n = 7687)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7688)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7689)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #86
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7690)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #151
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 7691)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7692)
	; 16 bytes
	php
	tya
	ldy #240
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7693)
	; 15 bytes
	tya
	ldy #(n-5)/31
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7694)
	; 15 bytes
	tya
	ldy #233
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 7695)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #6
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 7696 to 7734
.if(n = 7696)
	; 15 bytes
	txa
	pha
	tya
	ldy #19
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 7697)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7698)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 7699)
	; 16 bytes
	php
	tya
	pha
	ldy #127
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 7700)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7701)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7702 || n = 7707 || n = 7710 || n = 7714)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7703 || n = 7708 || n = 7715 || n = 7716 || n = 7720)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 7704 || n = 7709)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 7705)
	; 7 bytes
	php
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7706 || n = 7711 || n = 7718)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7712)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7713)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7717)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7719)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 7721)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7722)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7723)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7724)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7725)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7726)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7727 || n = 7728 || n = 7730 || n = 7731)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7729)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7731 && n < 7735))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 7735 to 7812
.if((n > 7734 && n < 7739) || (n > 7739 && n < 7744) || (n > 7744 && n < 7813))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7739)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7744)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7891))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7906))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7906 || n = 7907)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7908 || n = 7915)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7909)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 7909 && n < 7913))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7913)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7914)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 99
	Jmi *-8
	pla
	tax
.elseif(n = 7916 || n = 7917)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7918)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 7919)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 7920)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	plp
	Jne *-10
	tax
.elseif(n = 7921)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #42
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 7922)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7923)
	; 15 bytes
	tya
	pha
	ldy #27
	delay_n 288
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7924)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7925)
	; 15 bytes
	tya
	ldy #240
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 7926)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #93
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 7927)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 34
	Jne *-8
	pla
	tax
.elseif(n = 7928)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 7929)
	; 16 bytes
	php
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 16 bytes
	php
	tya
	ldy #73
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7932)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 7933)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 134
	Jmi *-8
	pla
	tay
.elseif(n = 7934)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7935)
	; 15 bytes
	tya
	ldy #121
	pha
	delay_n 53
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 7936)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7937)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #5
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7938)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #55
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 7939)
	; 15 bytes
	php
	tya
	pha
	ldy #180
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 7940)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 788
	Jmi *-8
	pla
	tay
.elseif(n = 7941)
	; 15 bytes
	tya
	ldy #0
	php
	pha
	rol $36,X
	ror $36,X
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7942)
	; 16 bytes
	php
	tya
	ldy #64
	pha
	lda #83
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7943)
	; 16 bytes
	php
	tya
	ldy #76
	pha
	lda #64
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 7944)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 983
	pla
	Jcc *-11
	plp
.elseif(n = 7945)
	; 16 bytes
	txa
	pha
	tya
	ldx #8
	ldy #175
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 7946)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #0
	jsr delay_256a_16_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 7947)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #197
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	plp
.elseif(n = 7948)
	; 16 bytes
	php
	tya
	ldy #248
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 7949 to 7968
.if(n = 7949)
	; 14 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	Jne *-9
	tay
.elseif(n = 7950)
	; 16 bytes
	php
	tya
	ldy #126
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 7951)
	; 15 bytes
	tya
	ldy #137
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7952 || n = 7959 || n = 7964)
	; 8..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7953 || n = 7960 || n = 7965)
	; 8..15 bytes
	pla
	delay_n (n-4)
.elseif(n = 7954)
	; 15 bytes
	txa
	pha
	tya
	ldx #147
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 7955)
	; 15 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	delay_n 128
	pla
	dey
	Jne *-10
	tay
.elseif(n = 7956)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7957)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 7958 || n = 7963 || n = 7966)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 7961)
	; 7 bytes
	php
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7962 || n = 7967)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7968)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 7969 to 8125
.if(n = 7969)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7970)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971 || n = 7972 || n = 7976)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 7973)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 7974)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7975)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 7977)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7978)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7979)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7980)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7981)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7982)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7983 || n = 7984 || n = 7986 || n = 7987)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7985)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 7987 && n < 7995) || (n > 7995 && n < 8000) || (n > 8000 && n < 8126))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7995)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8000)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
.if((n >= 8126) && (n < 8165))
; 8126 to 8164
.if((n > 8125 && n < 8162))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8162 || n = 8163)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8164)
	; 13 bytes
	pha
	delay_n (n-3)
.endif
.else
.if((n >= 8165) && (n < 8184))
; 8165 to 8183
.if(n = 8165)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 8165 && n < 8169))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8169)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8170)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8171)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8172 || n = 8173)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8174)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 8175)
	; 15 bytes
	tya
	ldy #190
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 8176)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #13
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8177)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8178)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8179)
	; 16 bytes
	php
	tya
	pha
	ldy #136
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8180)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #174
	jsr delay_a_25_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
.elseif(n = 8181)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #232
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 8182)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8183)
	; 15 bytes
	tya
	ldy #141
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8185)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8186)
	; 16 bytes
	php
	tya
	ldy #121
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8187)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #203
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	plp
.elseif(n = 8188)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #73
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8189)
	; 15 bytes
	tya
	ldy #248
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 8190)
	; 15 bytes
	tya
	pha
	ldy #141
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8191)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8192)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #75
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8193)
	; 15 bytes
	tya
	ldy #234 ;hides 'nop'
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-10
	tay
.elseif(n = 8194)
	; 16 bytes
	php
	tya
	pha
	ldy #74
	lda #77
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8195)
	; 15 bytes
	tya
	ldy #126
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8196)
	; 15 bytes
	tya
	pha
	ldy #7
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 8197)
	; 15 bytes
	tya
	ldy #0
	delay_n 6
	delay_n 21
	dey
	Jne *-9
	tay
.elseif(n = 8198)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 8199)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8200)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8201)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 8202)
	; 16 bytes
	php
	tya
	ldy #130
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8203)
	; 15 bytes
	php
	tya
	pha
	ldy #186
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
.if((n >= 8204) && (n < 8223))
; 8204 to 8222
.if(n = 8204)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8205 || n = 8214 || n = 8219 || n = 8222)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8206)
	; 16 bytes
	php
	tya
	ldy #33
	pha
	lda #202
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8207)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8208)
	; 16 bytes
	txa
	pha
	tya
	ldx #32
	ldy #49
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 8209)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8210)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #0
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 8211)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 8212)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8213)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8215 || n = 8220)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8216 || n = 8221)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 8217)
	; 7 bytes
	php
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8218)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 8223 to 8242
.if(n = 8223 || n = 8230)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8225)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8226)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8227 || n = 8228 || n = 8232)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 8229)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8231)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 8233)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8234)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8235)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8236)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8237)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8238)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8239 || n = 8240 || n = 8242)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8241)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 8243 to 8281
.if(n = 8243)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 8243 && n < 8251) || (n > 8251 && n < 8256) || (n > 8256 && n < 8282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8251)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8256)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8418))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8418 || n = 8419)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8420 || n = 8427)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8421)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 8421 && n < 8425))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8425)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8426)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8428 || n = 8429)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8430)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 8431)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 8432)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #15
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8433)
	; 15 bytes
	tya
	ldy #196
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 8434)
	; 16 bytes
	php
	tya
	pha
	ldy #255
	lda #1
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8435)
	; 16 bytes
	php
	tya
	pha
	ldy #31
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8436)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8437)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
.if((n >= 8438) && (n < 8457))
; 8438 to 8456
.if(n = 8438)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8439)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #65
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 8440)
	; 15 bytes
	txa
	pha
	tya
	ldx #156
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 8441)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 8442)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 8443)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8444)
	; 16 bytes
	php
	tya
	ldy #68
	pha
	lda #85
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8445)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #64
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8446)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 106
	Jmi *-8
	pla
	tax
.elseif(n = 8447)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8448)
	; 16 bytes
	php
	tya
	ldy #222
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8449)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 358
	pla
	Jcs *-11
	plp
.elseif(n = 8450)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #170
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 8451)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8452)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8453)
	; 15 bytes
	tya
	ldy #0
	.repeat 4
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 8454)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 8455)
	; 15 bytes
	tya
	ldy #130
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8456)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 8457 to 8476
.if(n = 8457)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 1198
	pla
	Jcs *-11
	plp
.elseif(n = 8458)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #55
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8459)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #8
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8460)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8461)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #240
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 8462)
	; 16 bytes
	php
	tya
	ldy #64
	pha
	lda #91
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8463)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #38
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 8464)
	; 15 bytes
	txa
	pha
	tya
	ldy #21
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 8465)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8466)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #110
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 8467)
	; 15 bytes
	php
	tya
	pha
	ldy #192
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 8468)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8469 || n = 8470 || n = 8475)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8471 || n = 8476)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8472)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 8473)
	; 7 bytes
	php
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8474)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 8478 || n = 8482)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8479 || n = 8486)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8480)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8481)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8483 || n = 8484 || n = 8488)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 8485)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8487)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 8489)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8490)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8491)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8492)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8493)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8494)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8495 || n = 8496 || n = 8498 || n = 8499)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8497)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 8499 && n < 8507) || (n > 8507 && n < 8512) || (n > 8512 && n < 8516))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8507)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8512)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 8516 to 8593
.if((n > 8515 && n < 8594))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if((n > 8593 && n < 8672))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 8672) && (n < 8711))
.if((n >= 8672) && (n < 8691))
; 8672 to 8690
.if(n = 8672 || n = 8673)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8674 || n = 8675)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8676 || n = 8683)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8677)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 8677 && n < 8681))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8681)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8682)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8684 || n = 8685)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8686)
	; 15 bytes
	txa
	pha
	tya
	ldx #30
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 8687)
	; 15 bytes
	php
	tya
	pha
	ldy #197
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 8688)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8689)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8690)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 15 bytes
	tya
	ldy #202
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 8692)
	; 16 bytes
	php
	tya
	ldy #217
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8693)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8694)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8695)
	; 15 bytes
	tya
	ldy #21
	delay_n 386
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 8696)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #13
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8697)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8698)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8699)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8700)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8701)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8702)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 1233
	pla
	Jcs *-11
	plp
.elseif(n = 8703)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8704)
	; 16 bytes
	php
	tya
	ldy #164
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8705)
	; 15 bytes
	tya
	ldy #28
	delay_n 291
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 8706)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8707)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #216
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	plp
.elseif(n = 8708)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8709)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8710)
	; 15 bytes
	txa
	pha
	tya
	ldx #161
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.endif
.endif
.else
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
.if(n = 8711)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8712)
	; 15 bytes
	tya
	pha
	ldy #150
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8713)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8714)
	; 16 bytes
	php
	tya
	ldy #229
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8715)
	; 15 bytes
	tya
	ldy #134
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8716)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8717)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 8718 || n = 8727)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8719)
	; 16 bytes
	php
	tya
	pha
	ldy #145
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8720)
	; 16 bytes
	php
	tya
	pha
	ldy #76
	lda #81
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8721)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #6
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8722)
	; 16 bytes
	php
	tya
	ldy #130
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8723)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #2
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8724)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8725)
	; 16 bytes
	tya
	ldy #9
	delay_n 863
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 8726)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8728)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 8729)
	; 7 bytes
	php
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8730)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 8731 to 8750
.if(n = 8731 || n = 8734 || n = 8738)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8732 || n = 8739 || n = 8740 || n = 8744)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 8733)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 8735 || n = 8742)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8736)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8737)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8741)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8743)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 8745)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8746)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8747)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8748)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8749)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8750)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
.if((n >= 8751) && (n < 9063))
.if((n >= 8751) && (n < 8907))
; 8751 to 8906
.if(n = 8751 || n = 8752 || n = 8754 || n = 8755)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8753)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 8755 && n < 8763) || (n > 8763 && n < 8768) || (n > 8768 && n < 8907))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8763)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8768)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if((n > 8906 && n < 8930))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8930 || n = 8931)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8932 || n = 8939)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8933)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 8933 && n < 8937))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8937)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8938)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8940 || n = 8941)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8942)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-8
	pla
	tax
.elseif(n = 8943)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 8944)
	; 15 bytes
	tya
	pha
	ldy #154
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8945)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #90
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8947)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8948)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8949)
	; 15 bytes
	tya
	ldy #208
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 8950)
	; 16 bytes
	php
	tya
	pha
	ldy #229
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8951)
	; 15 bytes
	php
	tya
	pha
	ldy #203
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 8952)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #111
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 8953)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8954)
	; 16 bytes
	php
	tya
	ldy #33
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8955)
	; 16 bytes
	php
	tya
	ldy #(n-283)/271
	pha
	lda #232
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8956)
	; 16 bytes
	php
	tya
	ldy #208
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8957)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8958)
	; 16 bytes
	php
	tya
	ldy #142
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8959)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 152
	Jmi *-8
	pla
	tay
.elseif(n = 8960)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8961)
	; 16 bytes
	php
	tya
	pha
	ldy #33
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8962)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8963)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #72
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 8964)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 15 bytes
	tya
	ldy #63
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 8966)
	; 16 bytes
	php
	tya
	ldy #(n-133)/121
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8967)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8968)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8969)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8970)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #119
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 8971)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 8972)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 8973)
	; 15 bytes
	txa
	pha
	tya
	ldx #12
	ldy #136 ;hides 'dey'
	Jne *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 8974)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #13
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8975)
	; 15 bytes
	txa
	pha
	tya
	ldx #31
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 8976)
	; 15 bytes
	php
	tya
	ldy #(n-12)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 8977)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 8978)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 113
	Jmi *-8
	pla
	tax
.elseif(n = 8979)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 8980)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8981)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #132
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 8982)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 8983)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 8984)
	; 8 bytes
	pla
	delay_n (n-4)
.endif
.endif
.endif
.else
; 8985 to 9062
.if(n = 8985)
	; 7 bytes
	php
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8986 || n = 8991 || n = 8998)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8987 || n = 8990 || n = 8994)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988 || n = 8995 || n = 8996 || n = 9000)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 8989)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 8992)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8993)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 8997)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 8999)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 9001)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9002)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9003)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9004)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9005)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9006)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9007 || n = 9008 || n = 9010 || n = 9011)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9009)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9011 && n < 9019) || (n > 9019 && n < 9024) || (n > 9024 && n < 9063))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9019)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9024)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
; 9063 to 9140
.if((n > 9062 && n < 9141))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if((n > 9140 && n < 9180))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9180) && (n < 9199))
; 9180 to 9198
.if((n > 9179 && n < 9186))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9186 || n = 9187)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9188 || n = 9195)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9189)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 9189 && n < 9193))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9193)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9194)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9196 || n = 9197)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9198)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 9199 to 9218
.if(n = 9199)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #75
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 9200)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9201)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #130
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9202)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9203)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9204)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9205)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9206)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 116
	Jmi *-8
	pla
	tax
.elseif(n = 9207)
	; 15 bytes
	tya
	ldy #214
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9208)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9209)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9210)
	; 16 bytes
	php
	tya
	ldy #146
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9211)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9212)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9213)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #42
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 9214)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9215)
	; 15 bytes
	php
	tya
	pha
	ldy #209
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 9216)
	; 16 bytes
	php
	tya
	ldy #236
	pha
	lda #0
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9217)
	; 15 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9218)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
.if((n >= 9219) && (n < 9238))
; 9219 to 9237
.if(n = 9219)
	; 15 bytes
	tya
	pha
	ldy #8
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 9220)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9221)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #75
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9222)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9223)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9224)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 1143
	pla
	Jcc *-11
	plp
.elseif(n = 9225)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9226)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #61
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9227)
	; 15 bytes
	tya
	ldy #159
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9228)
	; 15 bytes
	php
	tya
	ldy #0
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 9229)
	; 16 bytes
	txa
	pha
	tya
	ldx #83
	ldy #21
	dey
	Jne *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 9230)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9231)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9232)
	; 15 bytes
	txa
	pha
	tya
	ldy #23
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 9233)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9234)
	; 15 bytes
	tya
	pha
	ldy #159
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9235)
	; 15 bytes
	tya
	ldy #142
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9236)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9237)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #116
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.endif
.else
; 9238 to 9257
.if(n = 9238 || n = 9243 || n = 9246 || n = 9250)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9239 || n = 9244 || n = 9251 || n = 9252 || n = 9256)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 9240 || n = 9245)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 9241)
	; 7 bytes
	php
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9242 || n = 9247 || n = 9254)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9248)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9249)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9253)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9255)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 9257)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9259)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9260)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9261)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9262)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9263 || n = 9264 || n = 9266 || n = 9267)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9265)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9267 && n < 9275) || (n > 9275 && n < 9280) || (n > 9280 && n < 9297))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9275)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9280)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 9297 to 9375
.if((n > 9296 && n < 9376))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 9376) && (n < 9688))
.if((n >= 9376) && (n < 9532))
.if((n >= 9376) && (n < 9454))
; 9376 to 9453
.if((n > 9375 && n < 9442))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9442 || n = 9443)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9444 || n = 9451)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9445)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 9445 && n < 9449))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9449)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9450)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9452 || n = 9453)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 16 bytes
	php
	tya
	pha
	ldy #255
	lda #5
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9455)
	; 15 bytes
	tya
	pha
	ldy #70
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 9456)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 9457)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9458)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9459)
	; 15 bytes
	tya
	ldy #163
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9460)
	; 16 bytes
	tya
	ldy #(n-5)/61
	php
	pha
	lda #15
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 9461)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9462)
	; 16 bytes
	php
	tya
	ldy #150
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9463)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 64
	Jne *-8
	pla
	tay
.elseif(n = 9464)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	delay_n 1170
	pla
	Jcc *-11
	plp
.elseif(n = 9465)
	; 15 bytes
	tya
	ldy #220
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9466)
	; 15 bytes
	txa
	pha
	tya
	ldx #175
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 9467)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #235
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	plp
.elseif(n = 9468)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9469)
	; 16 bytes
	php
	tya
	pha
	ldy #90
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9470)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9471)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9472)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 161
	Jmi *-8
	pla
	tay
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #26
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9474)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9475)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #16
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9476)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #65
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9477)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #76
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9478)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9479)
	; 15 bytes
	php
	tya
	pha
	ldy #215
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 9480)
	; 16 bytes
	php
	tya
	ldy #35
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9481)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9482)
	; 16 bytes
	tya
	ldy #81
	pha
	tya
	ldy #19
	dey
	Jpl *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 9483)
	; 16 bytes
	php
	tya
	ldy #76
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9484)
	; 16 bytes
	php
	tya
	ldy #73
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9485)
	; 15 bytes
	tya
	ldy #23
	delay_n 386
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 9486)
	; 17 bytes
	php
	lda #92
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9487)
	; 16 bytes
	php
	tya
	pha
	ldy #35
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9488)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9489)
	; 16 bytes
	php
	tya
	ldy #243
	pha
	lda #0
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9490)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9491)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #5
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9492)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 9493 to 9531
.if(n = 9493)
	; 16 bytes
	txa
	pha
	tya
	ldx #26
	ldy #69
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 9494 || n = 9499 || n = 9502 || n = 9506)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495 || n = 9500 || n = 9507 || n = 9508 || n = 9512)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 9496 || n = 9501)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 9497)
	; 7 bytes
	php
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9498 || n = 9503 || n = 9510)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9504)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9505)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9509)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9511)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 9513)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9514)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9515)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9516)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9517)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9518)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9519 || n = 9520 || n = 9522 || n = 9523)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9521)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9523 && n < 9531))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9531)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 9532 to 9687
.if((n > 9531 && n < 9536) || (n > 9536 && n < 9688))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9536)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if((n > 9687 && n < 9698))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9698 || n = 9699)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9700 || n = 9707)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9701)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 9701 && n < 9705))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9705)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9706)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9708 || n = 9709)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9710)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 9711)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 9712)
	; 15 bytes
	tya
	pha
	ldy #25
	delay_n 383
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9713)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9714)
	; 15 bytes
	tya
	pha
	ldy #13
	delay_n 688
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 9715)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9716)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9717)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9718)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9719)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #68
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9720)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #77
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9721)
	; 15 bytes
	tya
	pha
	ldy #72
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 9722)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9723)
	; 15 bytes
	tya
	ldy #226
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9724)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9725)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9726)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9728)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9729)
	; 16 bytes
	php
	tya
	ldy #79
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9730)
	; 15 bytes
	tya
	pha
	ldy #56 ;hides 'sec'
	delay_n 128
	iny
	Jpl *-8
	pla
	tay
.elseif(n = 9731)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 9732)
	; 16 bytes
	php
	tya
	ldy #243
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9733)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9734)
	; 16 bytes
	php
	tya
	pha
	ldy #145
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9735)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #79
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9736)
	; 15 bytes
	txa
	pha
	tya
	ldx #180
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 9737)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 66
	Jne *-8
	pla
	tay
.elseif(n = 9738)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	plp
	Jmi *-8
	pla
	tax
.elseif(n = 9739)
	; 16 bytes
	php
	tya
	pha
	ldy #162
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9740)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9741)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #8
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9742)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9743)
	; 15 bytes
	php
	tya
	pha
	ldy #221
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 9744)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 1208
	pla
	Jcc *-11
	plp
.elseif(n = 9745)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9747)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #2
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 9748)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9749)
	; 15 bytes
	tya
	ldy #168
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9750 || n = 9755 || n = 9758 || n = 9762)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 9751 || n = 9756 || n = 9763 || n = 9764)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 9752 || n = 9757)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 9753)
	; 7 bytes
	php
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9754 || n = 9759)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9760)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9761)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9765)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9767)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 9768)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 9769)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9770)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9771)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9772)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9773)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9774)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9775 || n = 9776 || n = 9778 || n = 9779)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9777)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 9779 && n < 9787) || (n > 9787 && n < 9792) || (n > 9792 && n < 9844))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9787)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9792)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
; 9844 to 9921
.if((n > 9843 && n < 9922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if((n > 9921 && n < 9954))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9954 || n = 9955)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9956)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 9957)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 9957 && n < 9961))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 9961) && (n < 9981))
; 9961 to 9980
.if(n = 9961)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 9962)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9963)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9964 || n = 9965)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 9966)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 126
	Jmi *-8
	pla
	tax
.elseif(n = 9967)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9968)
	; 16 bytes
	php
	tya
	ldy #(n-12)/131
	pha
	lda #92
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9969)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #18
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9970)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9971)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #203
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9972)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	lda #1
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9973)
	; 16 bytes
	php
	tya
	pha
	ldy #79
	lda #94
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9974)
	; 15 bytes
	tya
	pha
	ldy #33
	delay_n 288
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 9975)
	; 16 bytes
	php
	tya
	ldy #81
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9976)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9977)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #215
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9978)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 9979)
	; 16 bytes
	php
	tya
	pha
	ldy #166
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9980)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #73
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 15 bytes
	tya
	ldy #232
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9982)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9983)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-125)/106
	lda #74
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9984)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 1238
	pla
	Jcc *-11
	plp
.elseif(n = 9985)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 170
	Jmi *-8
	pla
	tay
.elseif(n = 9986)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #2
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 9987)
	; 15 bytes
	tya
	pha
	ldy #74
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 9988)
	; 15 bytes
	tya
	pha
	ldy #172
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9989)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9990)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 9991)
	; 16 bytes
	php
	tya
	pha
	ldy #35
	lda #245
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 9992)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #141
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 9993)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 9994)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9995)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 9996)
	; 16 bytes
	php
	tya
	ldy #77
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 9997)
	; 16 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #215
	jsr delay_a_25_clocks
	pla
	plp
	dex
	Jmi *-11
	tax
.elseif(n = 9998)
	; 16 bytes
	txa
	pha
	tya
	ldx #217
	ldy #7
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 9999)
	; 16 bytes
	tya
	ldy #37
	php
	pha
	lda #217
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jpl *-10
	tay
.elseif(n = 10000)
	; 15 bytes
	txa
	pha
	tya
	ldy #26
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	tay
	pla
	tax
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
.if((n >= 10001) && (n < 11251))
.if((n >= 10001) && (n < 10626))
.if((n >= 10001) && (n < 10313))
.if((n >= 10001) && (n < 10157))
; 10001 to 10156
.if(n = 10001)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10002)
	; 16 bytes
	php
	tya
	ldy #222
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10003)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10004)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10005)
	; 16 bytes
	tya
	ldy #250
	php
	pha
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 10006 || n = 10011 || n = 10014 || n = 10018)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10007 || n = 10012 || n = 10019 || n = 10020 || n = 10024)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 10008 || n = 10013)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 10009)
	; 7 bytes
	php
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10010 || n = 10015 || n = 10022)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10016)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10017)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10021)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10023)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 10025)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10026)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10027)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10028)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10029)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10030)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10031 || n = 10032 || n = 10034 || n = 10035)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10033)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 10035 && n < 10043) || (n > 10043 && n < 10048) || (n > 10048 && n < 10157))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10043)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10048)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10210))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10210 || n = 10211)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10212 || n = 10219)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10213)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 10213 && n < 10217))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10217)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10218)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10220 || n = 10221)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10222)
	; 15 bytes
	txa
	pha
	tya
	ldx #189
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 10223)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #155
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 10224)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10225)
	; 15 bytes
	tya
	ldy #72
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 10226)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10227)
	; 15 bytes
	php
	tya
	pha
	ldy #232
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 10228)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10229)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10230)
	; 16 bytes
	php
	tya
	ldy #(n-143)/131
	pha
	lda #92
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10231)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10232)
	; 16 bytes
	php
	tya
	ldy #146
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10233)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10234)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #13
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if(n = 10235)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10236)
	; 16 bytes
	php
	tya
	ldy #213
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10237)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-150)/131
	lda #99
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10238)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10239)
	; 15 bytes
	tya
	ldy #238
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 10240)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10241)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10242)
	; 15 bytes
	tya
	pha
	ldy #9
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 10243)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #16
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10244)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10245)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 10246)
	; 16 bytes
	php
	tya
	ldy #238
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10247)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10248)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10249)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10250)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #215
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 10251)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #85
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 10252)
	; 16 bytes
	php
	tya
	ldy #79
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10253)
	; 15 bytes
	tya
	pha
	ldy #76
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.endif
.else
; 10254 to 10273
.if(n = 10254)
	; 16 bytes
	php
	tya
	pha
	ldy #88
	lda #83
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10255)
	; 16 bytes
	tya
	ldy #124
	pha
	tya
	ldy #13
	dey
	Jne *-1
	tay
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 10256)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #13
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10257)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #222
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 10258)
	; 16 bytes
	php
	tya
	ldy #218
	pha
	lda #8
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10259)
	; 16 bytes
	php
	tya
	pha
	ldy #<((n-19)/5)
	lda #>((n-19)/5)
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10260)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10261)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10262 || n = 10267 || n = 10270)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 10263 || n = 10268)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10264 || n = 10269)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 10265)
	; 7 bytes
	php
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10266 || n = 10271)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10272)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10273)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275 || n = 10276 || n = 10280)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 10277)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10278)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10279)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 10281)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10282)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10283)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10284)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10285)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10286)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10287 || n = 10288 || n = 10290 || n = 10291)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10289)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 10291 && n < 10299) || (n > 10299 && n < 10304) || (n > 10304 && n < 10313))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10299)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10304)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
; 10313 to 10468
.if((n > 10312 && n < 10466))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10466 || n = 10467)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10468)
	; 13 bytes
	pha
	delay_n (n-3)
.endif
.else
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
.if((n >= 10469) && (n < 10488))
; 10469 to 10487
.if(n = 10469)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 10469 && n < 10473))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10473)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10474)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10475)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10476 || n = 10477)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10478)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10479)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 10480)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10481)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10482)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #146
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 10483)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10484)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10485)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10486)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10487)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 10488 to 10507
.if(n = 10488)
	; 15 bytes
	tya
	pha
	ldy #27
	delay_n 383
	dey
	Jne *-7
	pla
	tay
.elseif(n = 10489)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #153
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 10490)
	; 16 bytes
	php
	tya
	pha
	ldy #37
	lda #251
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10491)
	; 15 bytes
	php
	tya
	pha
	ldy #238
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 10492)
	; 15 bytes
	txa
	pha
	tya
	ldx #194
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 10493)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10494)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10495)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #76
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10496)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #57
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 10497)
	; 15 bytes
	tya
	ldy #244
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 10498)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 133
	Jmi *-8
	pla
	tax
.elseif(n = 10499)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-150)/131
	lda #99
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10500)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 1044
	Jmi *-8
	pla
	tay
.elseif(n = 10501)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10502)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 10503)
	; 15 bytes
	tya
	ldy #181
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 10504)
	; 16 bytes
	php
	tya
	ldy #244
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10505)
	; 15 bytes
	tya
	ldy #74
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 10506)
	; 16 bytes
	php
	tya
	ldy #198
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10507)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 16 bytes
	php
	tya
	ldy #82
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10509)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #88
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 10510)
	; 15 bytes
	tya
	pha
	ldy #181
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 10511)
	; 16 bytes
	php
	tya
	pha
	ldy #85
	lda #90
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10512)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #3
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10513)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-125)/106
	lda #74
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 10514)
	; 16 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	pla
	Jcc *-10
	plp
.elseif(n = 10515)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10516)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10517)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #26
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10518 || n = 10523 || n = 10526)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 10519 || n = 10524)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10520 || n = 10525)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 10521)
	; 7 bytes
	php
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10522)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 10527 to 10546
.if(n = 10527 || n = 10534)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10528)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10529)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10530)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10531 || n = 10532 || n = 10536)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 10533)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10535)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 10537)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10538)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10539)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10540)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10541)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10542)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10543 || n = 10544 || n = 10546)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10545)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 10547 to 10625
.if(n = 10547)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 10547 && n < 10555) || (n > 10555 && n < 10560) || (n > 10560 && n < 10626))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10555)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10560)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 10626) && (n < 10938))
.if((n >= 10626) && (n < 10782))
.if((n >= 10626) && (n < 10704))
; 10626 to 10703
.if((n > 10625 && n < 10704))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if((n > 10703 && n < 10722))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10722 || n = 10723)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10724 || n = 10731)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10725)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 10725 && n < 10729))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10729)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10730)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10732 || n = 10733)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10734)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10735)
	; 15 bytes
	tya
	ldy #185
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 10736)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10737)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10738)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10739)
	; 16 bytes
	php
	tya
	pha
	ldy #160
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10740)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10741)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10742)
	; 15 bytes
	tya
	pha
	ldy #185
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 16 bytes
	php
	tya
	ldy #219
	pha
	lda #10
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10744)
	; 16 bytes
	php
	tya
	pha
	ldy #195
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10745)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10746)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10747)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10748)
	; 16 bytes
	php
	tya
	ldy #244
	pha
	lda #5
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10749)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 458
	pla
	Jcs *-11
	plp
.elseif(n = 10750)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #79
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10751)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10752)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #21
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10753)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10754)
	; 16 bytes
	php
	tya
	pha
	ldy #95
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10755)
	; 15 bytes
	php
	tya
	pha
	ldy #244
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 10756)
	; 16 bytes
	php
	tya
	ldy #79
	pha
	lda #97
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10757)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10758)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #15
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10759)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10760)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10761)
	; 16 bytes
	php
	tya
	pha
	ldy #82
	lda #99
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 15 bytes
	txa
	pha
	tya
	ldx #199
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 10763)
	; 16 bytes
	php
	tya
	pha
	ldy #158
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10764)
	; 16 bytes
	php
	tya
	ldy #63
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10765)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10766)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #244
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10767)
	; 16 bytes
	php
	tya
	ldy #239
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10768)
	; 15 bytes
	txa
	pha
	tya
	ldy #27
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 10769)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	plp
	Jne *-8
	pla
	tax
.elseif(n = 10770)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #27
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10771)
	; 16 bytes
	php
	tya
	ldy #203
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10772)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10773)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 10774 || n = 10779)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 10775 || n = 10780)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 10776 || n = 10781)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 10777)
	; 7 bytes
	php
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10778)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782 || n = 10786)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10783 || n = 10790)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10784)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10785)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10787 || n = 10788 || n = 10792)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 10789)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10791)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 10793)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10794)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10795)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10796)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10797)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10798)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10799 || n = 10800 || n = 10802 || n = 10803)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10801)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 10803 && n < 10811) || (n > 10811 && n < 10816) || (n > 10816 && n < 10938))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10811)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10816)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if((n > 10937 && n < 10977))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 10977) && (n < 10996))
; 10977 to 10995
.if(n = 10977)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10978 || n = 10979)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10980 || n = 10987)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10981)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 10981 && n < 10985))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10985)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 10986)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10988 || n = 10989)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 10990)
	; 15 bytes
	tya
	ldy #169
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10991)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10992)
	; 16 bytes
	php
	tya
	ldy #244
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10993)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 10994)
	; 16 bytes
	php
	tya
	ldy #37
	pha
	lda #250
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10995)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10997)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 10998)
	; 15 bytes
	txa
	pha
	tya
	ldx #37
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 10999)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11000)
	; 16 bytes
	php
	tya
	ldy #164
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11001)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11002)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11003)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11004)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #147
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 11005)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11006)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11007)
	; 16 bytes
	php
	tya
	pha
	ldy #164
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11008)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-130)/111
	lda #79
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11009)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11010)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 11011)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 188
	Jmi *-8
	pla
	tay
.elseif(n = 11012)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #5
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11013)
	; 15 bytes
	tya
	ldy #0
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 11014)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11015)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #13
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
.if(n = 11016)
	; 16 bytes
	php
	tya
	ldy #(n-12)/131
	pha
	lda #92
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11017)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #85
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11018)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #159
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11019)
	; 15 bytes
	php
	tya
	pha
	ldy #250
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 11020)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11021 || n = 11030)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 11022)
	; 16 bytes
	txa
	pha
	tya
	ldy #170 ;hides 'tax'
	ldx #24
	dex
	Jne *-1
	iny
	Jne *-7
	tay
	pla
	tax
.elseif(n = 11023)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #99
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 11024)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11025)
	; 15 bytes
	tya
	ldy #190
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 11026)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #112
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 11027)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #11
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11028)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11029)
	; 16 bytes
	tya
	ldy #212
	pha
	tya
	ldy #6
	dey
	Jpl *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 11031)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11032)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 11033)
	; 7 bytes
	php
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11034)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 11035 to 11054
.if(n = 11035 || n = 11038 || n = 11042)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11036 || n = 11043 || n = 11044 || n = 11048)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 11037)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 11039 || n = 11046)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11041)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11045)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11047)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 11049)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11050)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11051)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11052)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11053)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11054)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 11055 to 11093
.if(n = 11055 || n = 11056 || n = 11058 || n = 11059)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11057)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11059 && n < 11067) || (n > 11067 && n < 11072) || (n > 11072 && n < 11094))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11067)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11072)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 11094 to 11250
.if((n > 11093 && n < 11234))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11234 || n = 11235)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11236 || n = 11243)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11237)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 11237 && n < 11241))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11241)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11242)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11244)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 77
	Jne *-8
	pla
	tay
.elseif(n = 11245)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11246)
	; 16 bytes
	php
	tya
	ldy #82
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11247)
	; 16 bytes
	php
	tya
	ldy #107
	pha
	lda #66
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11248)
	; 15 bytes
	txa
	pha
	tya
	ldx #208
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 11249)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11250)
	; 15 bytes
	tya
	ldy #173
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 11251) && (n < 11876))
.if((n >= 11251) && (n < 11563))
.if((n >= 11251) && (n < 11407))
.if((n >= 11251) && (n < 11329))
.if((n >= 11251) && (n < 11290))
.if((n >= 11251) && (n < 11270))
; 11251 to 11269
.if(n = 11251)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #102
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 11252)
	; 16 bytes
	php
	tya
	pha
	ldy #239
	lda #15
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11253)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11254)
	; 16 bytes
	php
	tya
	pha
	ldy #107
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11255)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11256)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11257)
	; 15 bytes
	tya
	ldy #194
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 11258)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 143
	Jmi *-8
	pla
	tax
.elseif(n = 11259)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #249
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11260)
	; 16 bytes
	php
	tya
	ldy #152
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11261)
	; 16 bytes
	php
	tya
	pha
	ldy #154
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11262)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #6
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11263)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #52
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 11264)
	; 15 bytes
	tya
	pha
	ldy #28
	delay_n 383
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 11265)
	; 15 bytes
	tya
	pha
	ldy #10
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 11266)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11267)
	; 16 bytes
	php
	tya
	pha
	ldy #152
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11268)
	; 16 bytes
	php
	tya
	ldy #168
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11269)
	; 16 bytes
	php
	tya
	pha
	ldy #90
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 11270 to 11289
.if(n = 11270)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #59
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 11271)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-135)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11272)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11273)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #167
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 11274)
	; 16 bytes
	txa
	pha
	tya
	ldx #12
	ldy #172
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 11275)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11276)
	; 16 bytes
	php
	tya
	ldy #87
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11277)
	; 17 bytes
	php
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11278)
	; 16 bytes
	php
	tya
	ldy #(n-143)/131
	pha
	lda #92
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11279)
	; 17 bytes
	php
	lda #93
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11280)
	; 15 bytes
	txa
	pha
	tya
	ldy #170 ;hides 'tax'
	ldx #232 ;hides 'inx'
	Jne *-1
	iny
	Jne *-6
	tay
	pla
	tax
.elseif(n = 11281)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #(n-561)/536
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11282)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11283)
	; 15 bytes
	php
	tya
	pha
	ldy #0
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	plp
.elseif(n = 11284)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11285)
	; 15 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	delay_n 1018
	dey
	Jmi *-8
	pla
	tay
.elseif(n = 11286)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 11287)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 11288)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 11289)
	; 7 bytes
	php
	lda #44
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290 || n = 11295 || n = 11302)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11291 || n = 11294 || n = 11298)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292 || n = 11299 || n = 11300 || n = 11304)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 11293)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 11296)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11297)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11301)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11303)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 11305)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11306)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11307)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11308)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11309)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11310)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11311 || n = 11312 || n = 11314 || n = 11315)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11313)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11315 && n < 11323) || (n > 11323 && n < 11328))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11323)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11328)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 11329 to 11406
.if((n > 11328 && n < 11407))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if((n > 11406 && n < 11485))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 11485) && (n < 11524))
.if((n >= 11485) && (n < 11504))
; 11485 to 11503
.if((n > 11484 && n < 11490))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11490 || n = 11491)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11492 || n = 11499)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11493)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 11493 && n < 11497))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11497)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11498)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11500 || n = 11501)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11502)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 11503)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.endif
.else
; 11504 to 11523
.if(n = 11504)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #13
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11505)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 11506)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11507)
	; 16 bytes
	php
	tya
	ldy #(n-12)/121
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11508)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11509)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11510)
	; 15 bytes
	tya
	ldy #177
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11511)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #38
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 11512)
	; 16 bytes
	php
	tya
	ldy #(n-12)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11513)
	; 16 bytes
	php
	tya
	ldy #217
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11514)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11515)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11516)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11517)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #156
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 11518)
	; 15 bytes
	txa
	pha
	tya
	ldx #213
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 11519)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11520)
	; 16 bytes
	php
	tya
	ldy #84
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11521)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11522)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11523)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 197
	Jmi *-8
	pla
	tay
.elseif(n = 11525)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #240
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 11526)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #18
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11527)
	; 16 bytes
	php
	tya
	ldy #245
	pha
	lda #8
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11528)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11529)
	; 16 bytes
	tya
	ldy #172
	pha
	tya
	ldy #9
	dey
	Jpl *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 11530)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #119
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 11531)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #168
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11532)
	; 16 bytes
	php
	tya
	ldy #90
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11533)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11534)
	; 16 bytes
	php
	tya
	pha
	ldy #245
	lda #15
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11535)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #30
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11536)
	; 15 bytes
	txa
	pha
	tya
	ldy #29
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 11537)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #126
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11538)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11539)
	; 16 bytes
	php
	tya
	pha
	ldy #192
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11540)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11541)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11542)
	; 8 bytes
	delay_n (n-2)
	nop
.endif
.else
; 11543 to 11562
.if(n = 11543 || n = 11548 || n = 11555 || n = 11556 || n = 11560)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 11544 || n = 11549)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 11545)
	; 7 bytes
	php
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11546 || n = 11551 || n = 11558)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11547 || n = 11550 || n = 11554)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11552)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11553)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11557)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11559)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 11561)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11562)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11564)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11565)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11566)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11567 || n = 11568 || n = 11570 || n = 11571)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11569)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11571 && n < 11579) || (n > 11579 && n < 11584) || (n > 11584 && n < 11719))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11579)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11584)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if((n > 11718 && n < 11746))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11746 || n = 11747)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11748 || n = 11755)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11749)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 11749 && n < 11753))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11753)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11754)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11756 || n = 11757)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758)
	; 16 bytes
	php
	tya
	pha
	ldy #91
	lda #97
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11759)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #172
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 11760)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #93
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11761)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11762)
	; 16 bytes
	php
	tya
	ldy #250
	pha
	lda #8
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11763)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11764)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11765)
	; 15 bytes
	tya
	ldy #84
	pha
	delay_n 128
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11766)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11767)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11768)
	; 16 bytes
	php
	tya
	pha
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 11769)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #93
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11770)
	; 15 bytes
	tya
	ldy #181
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11771)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11772)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 11773)
	; 16 bytes
	tya
	ldy #234 ;hides 'nop'
	php
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jmi *-11
	tay
.elseif(n = 11774)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11775)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11776)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 14 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	Jne *-9
	tax
.elseif(n = 11778)
	; 16 bytes
	php
	tya
	ldy #222
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11779)
	; 15 bytes
	tya
	ldy #203
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 11780)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11781)
	; 15 bytes
	pla
	delay_n (n-4)
.elseif(n = 11782)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11783)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #109
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 11784)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #19
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
.elseif(n = 11785)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 11786)
	; 15 bytes
	tya
	pha
	ldy #203
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 11787)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11788)
	; 15 bytes
	txa
	pha
	tya
	ldx #218
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 11789)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11790)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 150
	Jmi *-8
	pla
	tax
.elseif(n = 11791)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11792)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 81
	Jne *-8
	pla
	tay
.elseif(n = 11793)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 11794)
	; 16 bytes
	php
	tya
	ldy #85
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 11795)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11796)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 11797 to 11875
.if(n = 11797)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 11798 || n = 11803 || n = 11806 || n = 11810)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 11799 || n = 11804 || n = 11811 || n = 11812 || n = 11816)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 11800 || n = 11805)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 11801)
	; 7 bytes
	php
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11802 || n = 11807 || n = 11814)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11808)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11809)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 11813)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 11815)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 11817)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11818)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11819)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11820)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11821)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11822)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11823 || n = 11824 || n = 11826 || n = 11827)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11825)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 11827 && n < 11835) || (n > 11835 && n < 11840) || (n > 11840 && n < 11876))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11835)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11840)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
.if((n >= 11876) && (n < 11954))
; 11876 to 11953
.if((n > 11875 && n < 11954))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 11954) && (n < 11993))
; 11954 to 11992
.if((n > 11953 && n < 11993))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 11993) && (n < 12012))
; 11993 to 12011
.if((n > 11992 && n < 12002))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12002 || n = 12003)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12004 || n = 12011)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12005)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 12005 && n < 12009))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12009)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 12010)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 12012 to 12031
.if(n = 12012 || n = 12013)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12014)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12015)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 12016)
	; 16 bytes
	php
	tya
	pha
	ldy #93
	lda #97
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12017)
	; 16 bytes
	php
	tya
	ldy #245
	pha
	lda #10
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12018)
	; 15 bytes
	tya
	pha
	ldy #207
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12019)
	; 16 bytes
	php
	tya
	pha
	ldy #200
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12020)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #37
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12021)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12022)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12023)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #53
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12024)
	; 16 bytes
	php
	tya
	ldy #91
	pha
	lda #93
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12025)
	; 15 bytes
	tya
	pha
	ldy #40
	delay_n 288
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 12026)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12027)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #96
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12028)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12029)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12030)
	; 15 bytes
	tya
	ldy #185
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12031)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if(n = 12032)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #101
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 12033)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12034)
	; 16 bytes
	php
	tya
	pha
	ldy #135
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12035)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12036)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12037)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 206
	Jmi *-8
	pla
	tay
.elseif(n = 12038)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12039)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #18
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12040)
	; 15 bytes
	tya
	pha
	ldy #31
	delay_n 383
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12041)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #133
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12042 || n = 12050)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12043)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12044)
	; 16 bytes
	php
	tya
	ldy #93
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12045)
	; 15 bytes
	tya
	ldy #85
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 12046)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12047)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #92
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12048)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 12049)
	; 16 bytes
	txa
	pha
	tya
	ldx #62
	ldy #37
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.endif
.else
; 12051 to 12070
.if(n = 12051)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #15
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12052)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12053)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #68
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12054 || n = 12059 || n = 12062 || n = 12066)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12055 || n = 12060 || n = 12067 || n = 12068)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12056 || n = 12061)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 12057)
	; 7 bytes
	php
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12058 || n = 12063 || n = 12070)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12065)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 12069)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.endif
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 12072)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 12073)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12074)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12075)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12076)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12077)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12078)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12079 || n = 12080 || n = 12082 || n = 12083)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12081)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 12083 && n < 12091) || (n > 12091 && n < 12096) || (n > 12096 && n < 12110))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12091)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12096)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 12110 to 12187
.if((n > 12109 && n < 12188))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12258))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12258 || n = 12259)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12260)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 12261)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 12261 && n < 12265))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12265)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.endif
.else
.if((n >= 12266) && (n < 12305))
.if((n >= 12266) && (n < 12285))
; 12266 to 12284
.if(n = 12266)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12267)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12268 || n = 12269)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12270)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #15
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12271)
	; 15 bytes
	txa
	pha
	tya
	ldy #95
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 12272)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #181
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12273)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12274)
	; 15 bytes
	txa
	pha
	tya
	ldx #227
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 12275)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #193
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12276)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12277)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12278)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12279)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12280)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12281)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #185
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12282)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #77
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 12283)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12284)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #(n-531)/511
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12286)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12287)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12288)
	; 15 bytes
	tya
	pha
	ldy #12
	delay_n 1018
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12289)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 12290)
	; 15 bytes
	tya
	ldy #189
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12291)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12292)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #97
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 12293)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12294)
	; 16 bytes
	php
	tya
	ldy #88
	pha
	lda #99
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12295)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #92
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12296)
	; 16 bytes
	php
	tya
	ldy #166
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12297)
	; 16 bytes
	php
	tya
	ldy #195
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12298)
	; 16 bytes
	tya
	ldy #18
	delay_n 638
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 12299)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12300)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12301)
	; 15 bytes
	tya
	ldy #212
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 12302)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12303)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 12304)
	; 15 bytes
	txa
	pha
	tya
	ldy #31
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.endif
.endif
.else
; 12305 to 12343
.if(n = 12305)
	; 15 bytes
	tya
	ldy #40
	delay_n 291
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 12306)
	; 15 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	iny
	Jne *-8
	tay
	plp
.elseif(n = 12307)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #16
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12308)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12309 || n = 12311 || n = 12316 || n = 12323 || n = 12324 || n = 12328)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12310 || n = 12315 || n = 12318 || n = 12322)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 12312 || n = 12317)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 12313)
	; 7 bytes
	php
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12314 || n = 12319 || n = 12326)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12321)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 12325)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12327)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 12329)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12330)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12331)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12332)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12333)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12334)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12335 || n = 12336 || n = 12338 || n = 12339)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12337)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 12339 && n < 12344))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 12344 to 12500
.if((n > 12343 && n < 12347) || (n > 12347 && n < 12352) || (n > 12352 && n < 12501))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12347)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12352)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12501) && (n < 13751))
.if((n >= 12501) && (n < 13126))
.if((n >= 12501) && (n < 12813))
.if((n >= 12501) && (n < 12657))
.if((n >= 12501) && (n < 12579))
.if((n >= 12501) && (n < 12540))
; 12501 to 12539
.if((n > 12500 && n < 12514))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12514 || n = 12515)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12516 || n = 12523)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12517)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 12517 && n < 12521))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12521)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 12522)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12524 || n = 12525)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12526)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12527)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12528)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #45
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12529)
	; 15 bytes
	txa
	pha
	tya
	ldy #96
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 12530)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12531)
	; 16 bytes
	php
	tya
	pha
	ldy #184
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12532)
	; 16 bytes
	php
	tya
	pha
	ldy #96
	lda #97
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12533)
	; 15 bytes
	tya
	ldy #216
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 12534)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12535)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #52
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12536)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12537)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12538)
	; 16 bytes
	php
	tya
	pha
	ldy #117
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12539)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 12540) && (n < 12559))
; 12540 to 12558
.if(n = 12540)
	; 15 bytes
	tya
	pha
	ldy #216
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12541)
	; 16 bytes
	php
	tya
	ldy #187
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12542)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12543)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #174
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 12544)
	; 15 bytes
	txa
	pha
	tya
	ldx #232
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 12545)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 12546)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12547)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12548)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12549)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12550)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 12551)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12552)
	; 16 bytes
	php
	tya
	ldy #95
	pha
	lda #93
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12553)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12554)
	; 16 bytes
	php
	tya
	pha
	ldy #114
	lda #77
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12555)
	; 16 bytes
	php
	tya
	ldy #(n-123)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12556)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12557)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12558)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 16 bytes
	php
	tya
	pha
	ldy #209
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12560 || n = 12566 || n = 12571 || n = 12574 || n = 12578)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12561)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12562)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-130)/111
	lda #79
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12563)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12564)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12565)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #19
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12567 || n = 12572)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 12568 || n = 12573)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 12569)
	; 7 bytes
	php
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12570 || n = 12575)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12576)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12577)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579 || n = 12580 || n = 12584)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 12581)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12582)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12583)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 12585)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12586)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12587)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12588)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12589)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12590)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12591 || n = 12592 || n = 12594 || n = 12595)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12593)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 12595 && n < 12603) || (n > 12603 && n < 12608) || (n > 12608 && n < 12657))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12603)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12608)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
; 12657 to 12734
.if((n > 12656 && n < 12735))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if((n > 12734 && n < 12770))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12770 || n = 12771)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12772)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 12773)
	; 13 bytes
	pla
	delay_n (n-4)
.endif
.else
.if((n >= 12774) && (n < 12793))
; 12774 to 12792
.if((n > 12773 && n < 12777))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12777)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 12778)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12779)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 12780 || n = 12781)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12782)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12783)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #90
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12784)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12785)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12786)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12787)
	; 15 bytes
	txa
	pha
	tya
	ldy #98
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 12788)
	; 16 bytes
	php
	tya
	pha
	ldy #112
	lda #81
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12789)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12790)
	; 16 bytes
	php
	tya
	pha
	ldy #98
	lda #97
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12791)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12792)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #17
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12794)
	; 16 bytes
	php
	tya
	pha
	ldy #175
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12795)
	; 16 bytes
	tya
	ldy #24 ;hides 'clc'
	php
	pha
	lda #75
	jsr delay_a_25_clocks
	pla
	plp
	iny
	Jpl *-11
	tay
.elseif(n = 12796)
	; 16 bytes
	php
	tya
	ldy #93
	pha
	lda #97
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 12797)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12798)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12799)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	plp
	Jne *-8
	pla
	tax
.elseif(n = 12800)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12801)
	; 16 bytes
	php
	tya
	ldy #203
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12802)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 12803)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12804)
	; 16 bytes
	php
	tya
	ldy #104
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12805)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12806)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12807)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #110
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 12808)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12809)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 12810)
	; 15 bytes
	tya
	ldy #197
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12811)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12812)
	; 16 bytes
	php
	tya
	ldy #100
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12813) && (n < 12969))
.if((n >= 12813) && (n < 12891))
.if((n >= 12813) && (n < 12852))
; 12813 to 12851
.if(n = 12813)
	; 15 bytes
	php
	tya
	ldy #(n-12)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 12814)
	; 15 bytes
	txa
	pha
	tya
	ldx #237
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 12815 || n = 12822 || n = 12827 || n = 12830 || n = 12834)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12816)
	; 15 bytes
	tya
	pha
	ldy #32
	delay_n 383
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 12817)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12818)
	; 16 bytes
	txa
	pha
	tya
	ldx #36
	ldy #68
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 12819)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 548
	pla
	Jcs *-11
	plp
.elseif(n = 12820)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12821)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 12823 || n = 12828 || n = 12835 || n = 12836 || n = 12840)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 12824 || n = 12829)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 12825)
	; 7 bytes
	php
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12826 || n = 12831 || n = 12838)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12832)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12833)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 12837)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 12839)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 12841)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12842)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12843)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12844)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12845)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12846)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12847 || n = 12848 || n = 12850 || n = 12851)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12849)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 12852 to 12890
.if((n > 12851 && n < 12859) || (n > 12859 && n < 12864) || (n > 12864 && n < 12891))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12859)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12864)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 12891 to 12968
.if((n > 12890 && n < 12969))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 13026))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13026 || n = 13027)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13028 || n = 13035)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13029)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 13029 && n < 13033))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13033)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13034)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13036 || n = 13037)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13038)
	; 16 bytes
	php
	tya
	pha
	ldy #46
	lda #245
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13039)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 13040)
	; 15 bytes
	tya
	ldy #32
	delay_n 386
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 13041)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13042)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #140
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 13043)
	; 16 bytes
	php
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	Jcc *-7
	pla
	tax
	plp
.elseif(n = 13044)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13045)
	; 15 bytes
	txa
	pha
	tya
	ldy #101
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 13046)
	; 15 bytes
	tya
	pha
	ldy #98
	delay_n 128
	dey
	Jne *-7
	pla
	tay
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047)
	; 16 bytes
	php
	tya
	ldy #237
	pha
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13048)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #69
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13049)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	delay_n 555
	pla
	Jcs *-11
	plp
.elseif(n = 13050)
	; 16 bytes
	php
	tya
	ldy #246
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13051)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #(n-11)/326
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13052)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13053)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13054)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13055)
	; 15 bytes
	tya
	ldy #225
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 13056)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #183
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 13057)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13058)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13059)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13060)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 1300
	Jmi *-8
	pla
	tay
.elseif(n = 13061)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13062)
	; 15 bytes
	tya
	pha
	ldy #225
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13063)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 224
	Jmi *-8
	pla
	tay
.elseif(n = 13064)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	lda #13
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13065)
	; 16 bytes
	php
	tya
	ldy #229
	pha
	lda #18
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13067)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13068)
	; 15 bytes
	php
	tya
	ldy #0
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	plp
.elseif(n = 13069)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #26
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13070)
	; 15 bytes
	tya
	ldy #201
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13071)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13072)
	; 15 bytes
	txa
	pha
	tya
	ldy #33
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 13073)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13074)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #30
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 13075)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #126
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 13076)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13077)
	; 16 bytes
	php
	tya
	ldy #195
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13078 || n = 13083)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13079 || n = 13084)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13080 || n = 13085)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 13081)
	; 7 bytes
	php
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13082)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086 || n = 13090)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13087 || n = 13094)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13088)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13089)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13091 || n = 13092 || n = 13096)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 13093)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13095)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 13097)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13098)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13099)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13100)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13101)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13102)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13103 || n = 13104 || n = 13106 || n = 13107)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13105)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13107 && n < 13115) || (n > 13115 && n < 13120) || (n > 13120 && n < 13126))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13115)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13120)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13126) && (n < 13438))
.if((n >= 13126) && (n < 13282))
; 13126 to 13281
.if((n > 13125 && n < 13282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
.if((n >= 13282) && (n < 13301))
; 13282 to 13300
.if(n = 13282 || n = 13283)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13284)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 13285)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 13285 && n < 13289))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13289)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13290)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13291)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 228
	Jmi *-8
	pla
	tay
.elseif(n = 13292)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 13293)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13294)
	; 15 bytes
	tya
	pha
	ldy #229
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13295)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 13296)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13297)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13298)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13299)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 92
	Jne *-8
	pla
	tay
.elseif(n = 13300)
	; 15 bytes
	txa
	pha
	tya
	ldx #246
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13302)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13303)
	; 15 bytes
	txa
	pha
	tya
	ldy #102
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 13304)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13305)
	; 15 bytes
	tya
	ldy #95
	pha
	delay_n 128
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13306)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #71
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13307)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13308)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #65
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13309)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13310)
	; 15 bytes
	txa
	pha
	tya
	ldx #45
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 13311)
	; 15 bytes
	tya
	pha
	ldy #13
	delay_n 1018
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13312)
	; 15 bytes
	tya
	pha
	ldy #100
	delay_n 128
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13313)
	; 16 bytes
	php
	tya
	ldy #46
	pha
	lda #244
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13314)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13315)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13316)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13317)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13318)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13319)
	; 16 bytes
	php
	tya
	pha
	ldy #190
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13320)
	; 16 bytes
	php
	tya
	pha
	ldy #46
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
.if(n = 13321 || n = 13334 || n = 13339)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13322)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13323)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13324)
	; 16 bytes
	php
	tya
	ldy #104
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13325)
	; 16 bytes
	tya
	ldy #60
	pha
	tya
	ldy #40
	dey
	Jpl *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 13326)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-16)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 13327)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #76
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 13328)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-272)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 13329)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13330)
	; 15 bytes
	tya
	ldy #205
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13331)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13332)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13333)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #38
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13335)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 13336)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 13337)
	; 7 bytes
	php
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13338)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 13340 to 13359
.if(n = 13340 || n = 13347 || n = 13348 || n = 13352)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 13341)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 13342 || n = 13346)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13343 || n = 13350)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13344)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13345)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13349)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13351)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 13353)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13354)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13355)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13356)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13357)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13358)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13359)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360 || n = 13362 || n = 13363)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13361)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13363 && n < 13371) || (n > 13371 && n < 13376) || (n > 13376 && n < 13438))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13371)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13376)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
; 13438 to 13515
.if((n > 13437 && n < 13516))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if((n > 13515 && n < 13538))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13538 || n = 13539)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13540 || n = 13547)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13541)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 13541 && n < 13545))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13545)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13546)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13548 || n = 13549)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13550)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 13551)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13552)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13553)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13554)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 13555) && (n < 13574))
; 13555 to 13573
.if(n = 13555)
	; 16 bytes
	php
	tya
	pha
	ldy #188
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13556)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13557)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13558)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13559)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #149
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 13560)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 537
	Jne *-8
	pla
	tax
.elseif(n = 13561)
	; 15 bytes
	txa
	pha
	tya
	ldy #105
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 13562)
	; 16 bytes
	php
	tya
	ldy #(n-283)/271
	pha
	lda #232
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13563)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13564)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13565)
	; 16 bytes
	php
	tya
	pha
	ldy #13
	lda #4
	jsr delay_256a_11_clocks_
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13566)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 13567)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13568)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13569)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #192
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 13570)
	; 15 bytes
	txa
	pha
	tya
	ldx #251
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 13571)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13572)
	; 16 bytes
	php
	tya
	ldy #226
	pha
	lda #21
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13573)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 94
	Jne *-8
	pla
	tay
.endif
.else
; 13574 to 13593
.if(n = 13574)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13575)
	; 16 bytes
	php
	tya
	ldy #98
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13576)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 233
	Jmi *-8
	pla
	tay
.elseif(n = 13577)
	; 15 bytes
	tya
	ldy #234
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 13578)
	; 15 bytes
	tya
	pha
	ldy #102
	delay_n 128
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13579)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13580)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13581 || n = 13591)
	; 8..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 13582)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13583)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #204
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 13584)
	; 15 bytes
	tya
	pha
	ldy #234
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13585)
	; 15 bytes
	tya
	ldy #96
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 13586)
	; 16 bytes
	txa
	pha
	tya
	ldx #234 ;hides 'nop'
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-7
	tay
	pla
	tax
.elseif(n = 13587)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13588)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13589)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 13590)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13592)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 13593)
	; 7 bytes
	php
	lda #53
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
; 13594 to 13750
.if(n = 13594 || n = 13599 || n = 13606)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13595 || n = 13598 || n = 13602)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596 || n = 13603 || n = 13604 || n = 13608)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 13597)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 13600)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13601)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13605)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13607)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 13609)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13610)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13611)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13612)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13613)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13614)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13615 || n = 13616 || n = 13618 || n = 13619)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13617)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13619 && n < 13627) || (n > 13627 && n < 13632) || (n > 13632 && n < 13751))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13627)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13632)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 13751) && (n < 14376))
.if((n >= 13751) && (n < 14063))
.if((n >= 13751) && (n < 13907))
.if((n >= 13751) && (n < 13829))
.if((n >= 13751) && (n < 13790))
; 13751 to 13789
.if((n > 13750 && n < 13790))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 13790) && (n < 13809))
; 13790 to 13808
.if((n > 13789 && n < 13794))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13794 || n = 13795)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13796 || n = 13803)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13797)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 13797 && n < 13801))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13801)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13802)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13804)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 237
	Jmi *-8
	pla
	tay
.elseif(n = 13805)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13806)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	lda #18
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13807)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 13808)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 15 bytes
	tya
	ldy #238
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 13810)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 13811)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #208
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 13812)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #21
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13813)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-140)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13814)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 63
	Jne *-8
	pla
	tax
.elseif(n = 13815)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 13816)
	; 15 bytes
	tya
	pha
	ldy #238
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13817)
	; 16 bytes
	php
	tya
	ldy #251
	pha
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13818)
	; 16 bytes
	php
	tya
	ldy #234
	pha
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13819)
	; 15 bytes
	txa
	pha
	tya
	ldy #107
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 13820)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #95
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 13821)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13822)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #75
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 13823)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13824)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13825)
	; 16 bytes
	php
	tya
	pha
	ldy #234
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13826)
	; 16 bytes
	tya
	ldy #51
	php
	pha
	lda #225
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 13827)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13828)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #49
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
.if((n >= 13829) && (n < 13848))
; 13829 to 13847
.if(n = 13829)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13830)
	; 15 bytes
	tya
	ldy #35
	delay_n 386
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13831)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13832)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13833)
	; 16 bytes
	php
	tya
	ldy #51
	pha
	lda #232
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13834)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #151
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 13835)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #136
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 13836)
	; 16 bytes
	php
	tya
	ldy #108
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13837)
	; 16 bytes
	php
	tya
	pha
	ldy #147
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13838)
	; 16 bytes
	php
	tya
	ldy #223
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13839)
	; 16 bytes
	txa
	pha
	tya
	ldx #22
	ldy #119
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 13840)
	; 15 bytes
	txa
	pha
	tya
	ldx #0
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 13841)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #158
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 13842)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 177
	Jmi *-8
	pla
	tax
.elseif(n = 13843)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13844)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13845)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 13846)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 13847)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 13848 to 13867
.if(n = 13848 || n = 13853)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 13849)
	; 7 bytes
	php
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13850 || n = 13855 || n = 13862)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13851 || n = 13854 || n = 13858)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13852 || n = 13859 || n = 13860 || n = 13864)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 13856)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13857)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 13861)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 13863)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 13865)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13866)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13867)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13869)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13870)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13871 || n = 13872 || n = 13874 || n = 13875)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13873)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 13875 && n < 13883) || (n > 13883 && n < 13888) || (n > 13888 && n < 13907))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13883)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13888)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 13907 to 14062
.if((n > 13906 && n < 14050))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14050 || n = 14051)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14052 || n = 14059)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14053)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 14053 && n < 14057))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14057)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14058)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14060 || n = 14061)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14062)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 14063) && (n < 14219))
.if((n >= 14063) && (n < 14141))
.if((n >= 14063) && (n < 14102))
.if((n >= 14063) && (n < 14082))
; 14063 to 14081
.if(n = 14063)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #139
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 14064)
	; 16 bytes
	php
	tya
	pha
	ldy #52
	lda #233
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14065)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14066)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #210
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 14067)
	; 16 bytes
	tya
	ldy #178
	php
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 14068)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14069)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #249
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14070)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 180
	Jmi *-8
	pla
	tax
.elseif(n = 14071)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14072)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #120
	dex
	Jpl *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 14073)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14074)
	; 16 bytes
	php
	tya
	ldy #178
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14075)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/56
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14076)
	; 15 bytes
	tya
	pha
	ldy #47
	delay_n 288
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 14077)
	; 15 bytes
	txa
	pha
	tya
	ldy #109
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 14078)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14079)
	; 16 bytes
	php
	tya
	pha
	ldy #190
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14080)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #77
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14081)
	; 16 bytes
	php
	tya
	pha
	ldy #178
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 14082 to 14101
.if(n = 14082)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #201
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 14083)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14084)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 603
	pla
	Jcs *-11
	plp
.elseif(n = 14085)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14086)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14087)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14088)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14089)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 242
	Jmi *-8
	pla
	tay
.elseif(n = 14090)
	; 17 bytes
	php
	lda #88
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14091)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #18
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14092)
	; 16 bytes
	php
	tya
	ldy #110
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14093)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14094)
	; 16 bytes
	txa
	pha
	tya
	ldx #168 ;hides 'tay'
	ldy #234 ;hides 'nop'
	iny
	Jne *-2
	inx
	Jne *-7
	tay
	pla
	tax
.elseif(n = 14095)
	; 16 bytes
	php
	tya
	pha
	ldy #207
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14096)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #213
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14097)
	; 16 bytes
	tya
	ldy #26
	pha
	delay_n 530
	pla
	dey
	Jne *-10
	tay
.elseif(n = 14098)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14099)
	; 15 bytes
	tya
	ldy #243
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 14100)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14101)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.else
; 14102 to 14140
.if(n = 14102 || n = 14107 || n = 14110 || n = 14114)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14103 || n = 14108 || n = 14115 || n = 14116 || n = 14120)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 14104 || n = 14109)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 14105)
	; 7 bytes
	php
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14106 || n = 14111 || n = 14118)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14112)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14113)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14117)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14119)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 14121)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14122)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14123)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14124)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14125)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14126)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14127 || n = 14128 || n = 14130 || n = 14131)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14129)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14131 && n < 14139) || n = 14140)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14139)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 14141 to 14218
.if((n > 14140 && n < 14144) || (n > 14144 && n < 14219))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14144)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if((n > 14218 && n < 14297))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 14297) && (n < 14336))
.if((n >= 14297) && (n < 14316))
; 14297 to 14315
.if((n > 14296 && n < 14306))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14306 || n = 14307)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14308 || n = 14315)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14309)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 14309 && n < 14313))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14313)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14314)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 14316 to 14335
.if(n = 14316)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14317)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 246
	Jmi *-8
	pla
	tay
.elseif(n = 14318)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14319)
	; 16 bytes
	php
	tya
	pha
	ldy #220
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14320)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14321)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14322)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14323)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14324)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #217
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14325)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14326)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14327)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14328)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14329)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14330)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14331)
	; 15 bytes
	tya
	ldy #247
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 14332)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14333)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14334)
	; 15 bytes
	tya
	pha
	ldy #13
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 14335)
	; 15 bytes
	txa
	pha
	tya
	ldy #111
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.endif
.endif
.else
.if((n >= 14336) && (n < 14356))
; 14336 to 14355
.if(n = 14336)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14337)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14338)
	; 15 bytes
	tya
	pha
	ldy #247
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 14339)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14340)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14341)
	; 16 bytes
	php
	tya
	pha
	ldy #231
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14342)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #157
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14343)
	; 16 bytes
	php
	tya
	ldy #(n-12)/281
	pha
	lda #242
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14344)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14345)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #26
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14346)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #215
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 14347)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14348)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14349)
	; 16 bytes
	php
	tya
	ldy #243
	pha
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14350)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14351)
	; 16 bytes
	txa
	pha
	tya
	ldx #235
	ldy #10
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 14352)
	; 16 bytes
	php
	tya
	ldy #239
	pha
	lda #21
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14353)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #222
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14354)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14355)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 14356 to 14375
.if(n = 14356)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14357)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14358 || n = 14363 || n = 14366 || n = 14370)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14359 || n = 14364 || n = 14371 || n = 14372)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14360 || n = 14365)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 14361)
	; 7 bytes
	php
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14362 || n = 14367 || n = 14374)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14369)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14373)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14375)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 14376) && (n < 14688))
.if((n >= 14376) && (n < 14532))
; 14376 to 14531
.if(n = 14376)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 14377)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14378)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14379)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14380)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14381)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14382)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14383 || n = 14384 || n = 14386 || n = 14387)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14385)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14387 && n < 14395) || (n > 14395 && n < 14400) || (n > 14400 && n < 14532))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14395)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14400)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if((n > 14531 && n < 14562))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14562 || n = 14563)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14564)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 14565)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 14565 && n < 14569))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14569)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14570)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 14571) && (n < 14590))
; 14571 to 14589
.if(n = 14571)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14572 || n = 14573)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14574)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14575)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 14576)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14577)
	; 16 bytes
	php
	tya
	pha
	ldy #251
	lda #26
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14578)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14579)
	; 16 bytes
	php
	tya
	pha
	ldy #224
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14580)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14582)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14583)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14584)
	; 16 bytes
	tya
	ldy #(n-5)/61
	php
	pha
	lda #15
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 14585)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14586)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14587)
	; 16 bytes
	php
	tya
	ldy #55
	pha
	lda #226
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14588)
	; 16 bytes
	php
	tya
	ldy #7
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14589)
	; 16 bytes
	php
	tya
	pha
	ldy #235
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14591)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14592)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14593)
	; 15 bytes
	txa
	pha
	tya
	ldy #112
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 14594)
	; 16 bytes
	php
	tya
	pha
	ldy #55
	lda #233
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14595)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #210
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 14596)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #81
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14597)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14598)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 14599)
	; 16 bytes
	php
	tya
	pha
	ldy #243
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14600)
	; 16 bytes
	php
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 14601)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 14602)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 251
	Jmi *-8
	pla
	tay
.elseif(n = 14603)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2080
	Jcs *-11
	plp
.elseif(n = 14604)
	; 16 bytes
	php
	tya
	ldy #113
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 14605)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14606)
	; 16 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #123
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 14607)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #66
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14608)
	; 15 bytes
	txa
	pha
	tya
	ldy #37
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 14609)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.endif
.endif
.else
; 14610 to 14687
.if(n = 14610)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 14611)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14612)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14613)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14614 || n = 14619 || n = 14622 || n = 14626)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14615 || n = 14620 || n = 14627 || n = 14628 || n = 14632)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 14616 || n = 14621)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 14617)
	; 7 bytes
	php
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14618 || n = 14623 || n = 14630)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14625)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14629)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14631)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 14633)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14634)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14635)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14636)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14637)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14638)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14639 || n = 14640 || n = 14642 || n = 14643)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14641)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14643 && n < 14651) || (n > 14651 && n < 14656) || (n > 14656 && n < 14688))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14651)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14656)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
; 14688 to 14843
.if((n > 14687 && n < 14818))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14818 || n = 14819)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14820 || n = 14827)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14821)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 14821 && n < 14825))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14825)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 14826)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14828)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14829)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 68
	Jne *-8
	pla
	tax
.elseif(n = 14830)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 383
	dex
	Jmi *-8
	pla
	tax
.elseif(n = 14831)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 14832)
	; 16 bytes
	php
	tya
	ldy #247
	pha
	lda #21
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14833)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14834)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14835)
	; 16 bytes
	php
	tya
	ldy #(n-12)/61
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14836)
	; 16 bytes
	php
	tya
	ldy #218
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14837)
	; 16 bytes
	php
	tya
	pha
	ldy #239
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14838)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14839)
	; 16 bytes
	php
	tya
	pha
	ldy #247
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14840)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14841)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14842)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14843)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 16 bytes
	php
	tya
	ldy #206
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14845)
	; 15 bytes
	tya
	ldy #106
	pha
	delay_n 128
	pla
	dey
	Jne *-9
	tay
.elseif(n = 14846)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14847)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	lda #30
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14848)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14849)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #172
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 14850)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 14851)
	; 15 bytes
	txa
	pha
	tya
	ldy #114
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 14852)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14853)
	; 15 bytes
	tya
	ldy #0
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
	dey
	Jne *-9
	tay
.elseif(n = 14854)
	; 16 bytes
	php
	tya
	pha
	ldy #215
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14855)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14856)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14857)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14858)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14859)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14860)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 53
	dey
	Jne *-7
	pla
	tay
.elseif(n = 14861)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14862)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #27
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-16)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 14864)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 14865)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #30
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 14866)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14867)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-135)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 14868)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14869)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 14870 || n = 14875 || n = 14878 || n = 14882)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14871 || n = 14876)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 14872 || n = 14877)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 14873)
	; 7 bytes
	php
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14874 || n = 14879)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14880)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14881)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883 || n = 14884 || n = 14888)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 14885)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 14886)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14887)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 14889)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14890)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14891)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14892)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14893)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14894)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14895 || n = 14896 || n = 14898 || n = 14899)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14897)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 14899 && n < 14907) || (n > 14907 && n < 14912) || (n > 14912 && n < 14922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14907)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14912)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 14922 to 15000
.if((n > 14921 && n < 15001))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
.if((n >= 15001) && (n < 15626))
.if((n >= 15001) && (n < 15313))
.if((n >= 15001) && (n < 15157))
.if((n >= 15001) && (n < 15079))
; 15001 to 15078
.if((n > 15000 && n < 15074))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15074 || n = 15075)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15076)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 15077)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif(n = 15078)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 15079) && (n < 15118))
.if((n >= 15079) && (n < 15098))
; 15079 to 15097
.if(n = 15079 || n = 15080)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15081)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15082)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15083)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15084 || n = 15085)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15086)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/61
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15087)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 15088)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15089)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 386
	pla
	dex
	Jmi *-10
	tax
.elseif(n = 15090)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 15091)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15092)
	; 16 bytes
	php
	tya
	ldy #232
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15093)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15094)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15095)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15096)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15097)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #31
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 15098 to 15117
.if(n = 15098)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 15099)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15100)
	; 16 bytes
	php
	tya
	ldy #184
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15101)
	; 16 bytes
	php
	tya
	ldy #191
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15102)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15103)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15104)
	; 16 bytes
	php
	tya
	ldy #196
	pha
	lda #38
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15105)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #78
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15106)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15107)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #130
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 15108)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #219
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 15109)
	; 15 bytes
	txa
	pha
	tya
	ldy #117
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 15110)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15111)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15112)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #85
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15113)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #170
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15114)
	; 16 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	delay_n 121
	iny
	Jne *-9
	pla
	tay
.elseif(n = 15115)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 260
	Jmi *-8
	pla
	tay
.elseif(n = 15116)
	; 16 bytes
	php
	tya
	ldy #117
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15117)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.else
.if((n >= 15118) && (n < 15137))
; 15118 to 15136
.if(n = 15118)
	; 16 bytes
	php
	tya
	ldy #182
	pha
	lda #44
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15119)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 648
	pla
	Jcs *-11
	plp
.elseif(n = 15120)
	; 16 bytes
	txa
	pha
	tya
	ldx #64
	ldy #45
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 15121)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #94
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15122)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 15123)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15124)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15125)
	; 15 bytes
	tya
	ldy #108
	pha
	delay_n 128
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15126 || n = 15131 || n = 15134)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 15127 || n = 15132)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15128 || n = 15133)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15129)
	; 7 bytes
	php
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15130 || n = 15135)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15136)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 15137 to 15156
.if(n = 15137)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15138)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15139 || n = 15140 || n = 15144)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 15141)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15142)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15143)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 15145)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15146)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15147)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15148)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15149)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15150)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15151 || n = 15152 || n = 15154 || n = 15155)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15153)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15156)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
; 15157 to 15312
.if((n > 15156 && n < 15163) || (n > 15163 && n < 15168) || (n > 15168 && n < 15313))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15163)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15168)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
; 15313 to 15351
.if((n > 15312 && n < 15330))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15330 || n = 15331)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15332 || n = 15339)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15333)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 15333 && n < 15337))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15337)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15338)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15340 || n = 15341)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15342)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15343)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 264
	Jmi *-8
	pla
	tay
.elseif(n = 15344)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15345)
	; 15 bytes
	tya
	ldy #236
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15346)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #85
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15347)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #66
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 15348)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15349)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 658
	pla
	Jcs *-11
	plp
.elseif(n = 15350)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #235
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 15351)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 16 bytes
	php
	tya
	pha
	ldy #57
	lda #237
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15353)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15354)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 107
	Jne *-8
	pla
	tay
.elseif(n = 15355)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #156
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 15356)
	; 16 bytes
	php
	tya
	ldy #137
	pha
	lda #73
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15357)
	; 15 bytes
	tya
	pha
	ldy #15
	delay_n 1018
	dey
	Jne *-7
	pla
	tay
.elseif(n = 15358)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15359)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15360)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15361)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #240
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15362)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 197
	Jmi *-8
	pla
	tax
.elseif(n = 15363)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #80
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15364)
	; 16 bytes
	php
	tya
	pha
	ldy #165
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15365)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15366)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15367)
	; 15 bytes
	txa
	pha
	tya
	ldy #119
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 15368)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15369)
	; 16 bytes
	php
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15370)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #87
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.else
; 15371 to 15390
.if(n = 15371)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15372)
	; 16 bytes
	php
	tya
	ldy #120
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15373)
	; 16 bytes
	tya
	ldy #33
	delay_n 443
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 15374)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15375)
	; 16 bytes
	tya
	ldy #145
	php
	pha
	lda #60
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 15376)
	; 15 bytes
	txa
	pha
	tya
	ldy #39
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 15377)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #86
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 15378)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15379)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15380)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15381)
	; 16 bytes
	php
	tya
	ldy #141
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15382 || n = 15387 || n = 15390)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 15383 || n = 15388)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15384 || n = 15389)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15385)
	; 7 bytes
	php
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15386)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15398)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15392)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15393)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15394)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15395 || n = 15396 || n = 15400)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 15397)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15399)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 15401)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15402)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15403)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15404)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15405)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15406)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15407 || n = 15408 || n = 15410 || n = 15411)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15409)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 15411 && n < 15419) || (n > 15419 && n < 15424) || (n > 15424 && n < 15469))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15419)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15424)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
; 15469 to 15546
.if((n > 15468 && n < 15547))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if((n > 15546 && n < 15586))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 15586) && (n < 15606))
; 15586 to 15605
.if(n = 15586 || n = 15587)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15588 || n = 15595)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15589)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 15589 && n < 15593))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15593)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15594)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15596 || n = 15597)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15598)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15599)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15600)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15601)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #66
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15602)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15605)
	; 15 bytes
	tya
	ldy #52
	delay_n 291
	pla
	dey
	Jne *-9
	tay
.endif
.else
; 15606 to 15625
.if(n = 15606)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15607)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15608)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #87
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15609)
	; 16 bytes
	php
	tya
	pha
	ldy #9
	lda #6
	jsr delay_256a_16_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15610)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #37
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 15611)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #79
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15612)
	; 16 bytes
	php
	tya
	ldy #240
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15613)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15614)
	; 16 bytes
	php
	tya
	ldy #57
	pha
	lda #230
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15615)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15616)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #44
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15617)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #44
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15618)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15619)
	; 16 bytes
	php
	tya
	pha
	ldy #240
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15620)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 1556
	Jmi *-8
	pla
	tay
.elseif(n = 15621)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 15622)
	; 15 bytes
	txa
	pha
	tya
	ldx #54
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 15623)
	; 16 bytes
	php
	tya
	ldy #233
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15624)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15625)
	; 15 bytes
	txa
	pha
	tya
	ldy #121
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 15626) && (n < 15938))
.if((n >= 15626) && (n < 15782))
.if((n >= 15626) && (n < 15704))
.if((n >= 15626) && (n < 15665))
.if((n >= 15626) && (n < 15645))
; 15626 to 15644
.if(n = 15626)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15627)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #6
	jsr delay_256a_16_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15628)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 109
	Jne *-8
	pla
	tay
.elseif(n = 15629)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15630)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15631)
	; 16 bytes
	php
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #177
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 15632)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15633)
	; 16 bytes
	php
	tya
	ldy #126
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15634)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 15635)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15636)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15637)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #82
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15638 || n = 15643)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15639 || n = 15644)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15640)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15641)
	; 7 bytes
	php
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15642)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 15645 to 15664
.if(n = 15645)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15646 || n = 15650)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15647 || n = 15654)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15648)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15649)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15651 || n = 15652 || n = 15656)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 15653)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15655)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 15657)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15658)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15659)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15660)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15661)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15662)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15663 || n = 15664)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15666 || n = 15667)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 15667 && n < 15675) || (n > 15675 && n < 15680) || (n > 15680 && n < 15704))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15675)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15680)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 15704 to 15781
.if((n > 15703 && n < 15782))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15842))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15842 || n = 15843)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15844 || n = 15851)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15845)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 15845 && n < 15849))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15849)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15850)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15852 || n = 15853)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15854)
	; 16 bytes
	php
	tya
	ldy #178
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15855)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15856)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 273
	Jmi *-8
	pla
	tay
.elseif(n = 15857)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #186
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 15858)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #75
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15859)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 15860) && (n < 15899))
.if((n >= 15860) && (n < 15879))
; 15860 to 15878
.if(n = 15860)
	; 16 bytes
	php
	tya
	pha
	ldy #217
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15861)
	; 16 bytes
	php
	tya
	pha
	ldy #178
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15862)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15863)
	; 15 bytes
	txa
	pha
	tya
	ldx #22
	ldy #136 ;hides 'dey'
	Jne *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 15864)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15865)
	; 15 bytes
	tya
	ldy #244
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15866)
	; 15 bytes
	php
	tya
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcs *-6
	pla
	tay
	plp
.elseif(n = 15867)
	; 16 bytes
	php
	tya
	pha
	ldy #55
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15868)
	; 16 bytes
	php
	tya
	pha
	clc
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcs *-6
	pla
	tay
	plp
.elseif(n = 15869)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15870)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15871)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15872)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15873)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15874)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15875)
	; 16 bytes
	tya
	ldy #230
	php
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 15876)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15877)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15878)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 15879 to 15898
.if(n = 15879)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15880)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15881)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15882)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #30
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15883)
	; 15 bytes
	txa
	pha
	tya
	ldy #122
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 15884)
	; 16 bytes
	php
	tya
	ldy #123
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 15885)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15886)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #91
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15887)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #163
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 15888)
	; 16 bytes
	php
	tya
	ldy #252
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15889)
	; 16 bytes
	php
	tya
	pha
	ldy #230
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 15890)
	; 16 bytes
	php
	tya
	pha
	ldy #58
	lda #237
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 15891)
	; 16 bytes
	php
	tya
	ldy #237
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 15892)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15893)
	; 16 bytes
	tya
	ldy #23
	pha
	delay_n 650
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 15894)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 15895)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 15896)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15897)
	; 7 bytes
	php
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15898)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15902 || n = 15906)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900 || n = 15907 || n = 15908 || n = 15912)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 15901)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 15903 || n = 15910)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15904)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15905)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 15909)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 15911)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 15913)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15914)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15915)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15916)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15917)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15918)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15919 || n = 15920 || n = 15922 || n = 15923)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15921)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 15923 && n < 15931) || (n > 15931 && n < 15936) || n = 15937)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15931)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15936)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
; 15938 to 16093
.if((n > 15937 && n < 16094))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
.if((n >= 16094) && (n < 16113))
; 16094 to 16112
.if((n > 16093 && n < 16098))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16098 || n = 16099)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16100 || n = 16107)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16101)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 16101 && n < 16105))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16105)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16106)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16108 || n = 16109)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16110)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 16111)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #76
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16112)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16114)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16115)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #166
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 16116)
	; 16 bytes
	php
	tya
	ldy #(n-12)/66
	pha
	lda #27
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16117)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16118)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16119)
	; 16 bytes
	php
	tya
	pha
	ldy #100
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16120)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16121)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16122)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 207
	Jmi *-8
	pla
	tax
.elseif(n = 16123)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16124)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #37
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16125)
	; 15 bytes
	tya
	ldy #248
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 16126)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16127)
	; 15 bytes
	tya
	pha
	ldy #55
	delay_n 288
	dey
	Jne *-7
	pla
	tay
.elseif(n = 16128)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16129)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16130)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 16131)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16132)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #91
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 16133) && (n < 16152))
; 16133 to 16151
.if(n = 16133)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16134)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #237
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 16135)
	; 16 bytes
	php
	tya
	pha
	ldy #237
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16136)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16137)
	; 16 bytes
	php
	tya
	ldy #215
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16138)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16139)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16140)
	; 16 bytes
	php
	tya
	ldy #96
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16141)
	; 15 bytes
	txa
	pha
	tya
	ldy #124
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 16142)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #139
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 16143)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16144)
	; 15 bytes
	txa
	pha
	tya
	ldy #42
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 16145)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16146 || n = 16150)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 16147)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16148)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16149)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #60
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16151)
	; 8 bytes
	pha
	delay_n (n-3)
.endif
.else
; 16152 to 16171
.if(n = 16152 || n = 16157)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 16153)
	; 7 bytes
	php
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16154 || n = 16159 || n = 16166)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16155 || n = 16158 || n = 16162)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16156 || n = 16163 || n = 16164 || n = 16168)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 16160)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16161)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16165)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16167)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 16169)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16170)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16171)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16173)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16174)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16175 || n = 16176 || n = 16178 || n = 16179)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16177)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16179 && n < 16187) || (n > 16187 && n < 16192) || (n > 16192 && n < 16251))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16187)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16192)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 16251) && (n < 16876))
.if((n >= 16251) && (n < 16563))
.if((n >= 16251) && (n < 16407))
.if((n >= 16251) && (n < 16329))
; 16251 to 16328
.if((n > 16250 && n < 16329))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if((n > 16328 && n < 16354))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16354 || n = 16355)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16356 || n = 16363)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16357)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 16357 && n < 16361))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16361)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16362)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #241
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 16364 || n = 16365)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16366)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16367)
	; 16 bytes
	php
	tya
	pha
	ldy #244
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368)
	; 16 bytes
	php
	tya
	ldy #188
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16369)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 282
	Jmi *-8
	pla
	tay
.elseif(n = 16370)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16371)
	; 15 bytes
	tya
	pha
	ldy #122
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 16372)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #141
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 16373)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16374)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16375)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16376)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #253
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 16377)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16378)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #91
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16379)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16380)
	; 15 bytes
	tya
	pha
	ldy #15
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 16381)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16382)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16383)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16384)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 703
	pla
	Jcs *-11
	plp
.elseif(n = 16385)
	; 15 bytes
	tya
	ldy #252
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 16386)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.endif
.else
; 16387 to 16406
.if(n = 16387)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16388)
	; 16 bytes
	php
	tya
	ldy #184
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16389)
	; 16 bytes
	php
	tya
	ldy #159
	pha
	lda #64
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16390)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16391)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16392)
	; 16 bytes
	php
	tya
	ldy #252
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16393)
	; 16 bytes
	php
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 16394)
	; 16 bytes
	php
	tya
	pha
	ldy #131
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16395)
	; 16 bytes
	php
	tya
	pha
	ldy #184
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16396)
	; 16 bytes
	php
	tya
	ldy #127
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 16397)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16398)
	; 16 bytes
	tya
	ldy #169
	pha
	tya
	ldy #15
	dey
	Jpl *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 16399)
	; 15 bytes
	txa
	pha
	tya
	ldy #126
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 16400)
	; 16 bytes
	php
	tya
	ldy #241
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16401 || n = 16406)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 16402)
	; 16 bytes
	php
	tya
	pha
	ldy #128
	lda #95
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 16403)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #32
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16404)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16405)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #58
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.endif
.endif
.endif
.else
; 16407 to 16562
.if(n = 16407 || n = 16412 || n = 16419 || n = 16420 || n = 16424)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 16408 || n = 16413)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 16409)
	; 7 bytes
	php
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16410 || n = 16415 || n = 16422)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16411 || n = 16414 || n = 16418)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16416)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16417)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16421)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16423)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 16425)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16426)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16427)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16428)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16429)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16430)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16431 || n = 16432 || n = 16434 || n = 16435)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16433)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16435 && n < 16443) || (n > 16443 && n < 16448) || (n > 16448 && n < 16563))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16443)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16448)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
.if((n >= 16563) && (n < 16602))
; 16563 to 16601
.if((n > 16562 && n < 16602))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 16602) && (n < 16621))
; 16602 to 16620
.if((n > 16601 && n < 16610))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16610 || n = 16611)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16612 || n = 16619)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16613)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 16613 && n < 16617))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16617)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16618)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16620)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 16621 to 16640
.if(n = 16621)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16622)
	; 16 bytes
	php
	tya
	ldy #151
	pha
	lda #71
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16623)
	; 16 bytes
	php
	tya
	ldy #147
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16624)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16625)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16626)
	; 16 bytes
	php
	tya
	ldy #(n-12)/71
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16627)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16628)
	; 16 bytes
	php
	tya
	ldy #248
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16629)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16630)
	; 16 bytes
	php
	tya
	pha
	ldy #147
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16631)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 16632)
	; 16 bytes
	php
	tya
	ldy #60
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16633)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16634)
	; 16 bytes
	tya
	ldy #241
	php
	pha
	lda #23
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 16635)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16636)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16637)
	; 15 bytes
	tya
	pha
	ldy #124
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 16638)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16639)
	; 16 bytes
	php
	tya
	pha
	ldy #60
	lda #245
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16640)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 16641) && (n < 16680))
.if((n >= 16641) && (n < 16660))
; 16641 to 16659
.if(n = 16641)
	; 16 bytes
	php
	tya
	ldy #241
	pha
	lda #(n-561)/536
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16642)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #190
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 16643)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16644)
	; 15 bytes
	tya
	pha
	ldy #23
	delay_n 688
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 16645)
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 53
	pla
	dey
	Jne *-9
	tay
.elseif(n = 16646)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 16647)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #246
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 16648)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16649)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 16650)
	; 16 bytes
	php
	tya
	ldy #177
	pha
	lda #55
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16651)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/66
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16652)
	; 16 bytes
	php
	tya
	ldy #130
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16653)
	; 16 bytes
	php
	tya
	ldy #128
	pha
	lda #90
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 16654)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 287
	Jmi *-8
	pla
	tay
.elseif(n = 16655)
	; 16 bytes
	php
	tya
	ldy #187
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16656)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 77
	Jne *-8
	pla
	tax
.elseif(n = 16657)
	; 15 bytes
	txa
	pha
	tya
	ldy #128
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 16658)
	; 16 bytes
	php
	tya
	ldy #203
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16659)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 16660 to 16679
.if(n = 16660)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16661)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #74
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16662 || n = 16667 || n = 16670 || n = 16674)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16663 || n = 16668 || n = 16675 || n = 16676)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16664 || n = 16669)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 16665)
	; 7 bytes
	php
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16666 || n = 16671 || n = 16678)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16673)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16677)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16679)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 16681)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16682)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16683)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16684)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16685)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16686)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16687 || n = 16688 || n = 16690 || n = 16691)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16689)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16691 && n < 16699) || (n > 16699 && n < 16704) || (n > 16704 && n < 16719))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16699)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16704)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 16719 to 16875
.if((n > 16718 && n < 16866))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16866 || n = 16867)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16868)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 16869)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 16869 && n < 16873))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16873)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16874)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16875)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #250
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.endif
.endif
.endif
.else
.if((n >= 16876) && (n < 17188))
.if((n >= 16876) && (n < 17032))
.if((n >= 16876) && (n < 16954))
.if((n >= 16876) && (n < 16915))
.if((n >= 16876) && (n < 16895))
; 16876 to 16894
.if(n = 16876 || n = 16877)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16878)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 16879)
	; 16 bytes
	php
	tya
	pha
	ldy #60
	lda #249
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16880)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16881)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 16882)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 217
	Jmi *-8
	pla
	tax
.elseif(n = 16883)
	; 16 bytes
	php
	tya
	pha
	ldy #248
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16884)
	; 16 bytes
	php
	tya
	ldy #228
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16885)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16886)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16887)
	; 16 bytes
	php
	tya
	ldy #225
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16888)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16889)
	; 16 bytes
	php
	tya
	pha
	ldy #241
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16890)
	; 16 bytes
	php
	tya
	ldy #194
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16891)
	; 16 bytes
	php
	tya
	pha
	ldy #228
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16892)
	; 16 bytes
	php
	tya
	ldy #211
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16893)
	; 16 bytes
	php
	tya
	pha
	ldy #143
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16894)
	; 16 bytes
	php
	tya
	pha
	ldy #225
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 16895 to 16914
.if(n = 16895)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16896)
	; 16 bytes
	php
	tya
	ldy #252
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16897)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16898)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16899)
	; 16 bytes
	php
	tya
	pha
	ldy #211
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16900)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16901)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16902)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16903)
	; 15 bytes
	tya
	pha
	ldy #126
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 16904)
	; 16 bytes
	php
	tya
	ldy #206
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16905)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 16906)
	; 16 bytes
	txa
	pha
	tya
	ldx #15
	ldy #224
	dey
	Jne *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 16907)
	; 16 bytes
	php
	tya
	ldy #155
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16908)
	; 16 bytes
	php
	tya
	ldy #132
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16909)
	; 16 bytes
	php
	tya
	ldy #61
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 16910)
	; 16 bytes
	php
	tya
	pha
	ldy #133
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16911)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16912)
	; 15 bytes
	txa
	pha
	tya
	ldy #44
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 16913)
	; 16 bytes
	txa
	pha
	tya
	ldy #170 ;hides 'tax'
	ldx #76
	dex
	Jpl *-1
	dey
	Jmi *-7
	tay
	pla
	tax
.elseif(n = 16914)
	; 16 bytes
	php
	tya
	pha
	ldy #155
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
; 16915 to 16953
.if(n = 16915)
	; 15 bytes
	txa
	pha
	tya
	ldy #131
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 16916)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16917)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16918 || n = 16923 || n = 16926 || n = 16930)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 16919 || n = 16924 || n = 16931 || n = 16932 || n = 16936)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 16920 || n = 16925)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 16921)
	; 7 bytes
	php
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16922 || n = 16927 || n = 16934)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16928)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16929)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 16933)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 16935)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 16937)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16938)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16939)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16940)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16941)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16942)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16943 || n = 16944 || n = 16946 || n = 16947)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16945)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 16947 && n < 16954))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 16954 to 17031
.if(n = 16954 || (n > 16955 && n < 16960) || (n > 16960 && n < 17032))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16955)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16960)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
; 17032 to 17109
.if((n > 17031 && n < 17110))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if((n > 17109 && n < 17122))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17122 || n = 17123)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17124 || n = 17131)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17125)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 17125 && n < 17129))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17129)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17130)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17132 || n = 17133)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17134)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17135)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 120
	Jne *-8
	pla
	tay
.elseif(n = 17136)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17137)
	; 16 bytes
	php
	tya
	ldy #137
	pha
	lda #86
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17138)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17139)
	; 16 bytes
	php
	tya
	pha
	ldy #214
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17140)
	; 16 bytes
	tya
	ldy #149
	php
	pha
	lda #69
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 17141)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17142)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17143)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17144)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17145)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17146)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17147)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17148)
	; 16 bytes
	php
	tya
	ldy #102
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17150)
	; 16 bytes
	php
	tya
	ldy #209
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17151)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17152)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17153)
	; 16 bytes
	php
	tya
	ldy #(n-12)/281
	pha
	lda #242
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17154)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #83
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17155)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17156)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17157)
	; 16 bytes
	php
	tya
	pha
	ldy #209
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17158)
	; 16 bytes
	php
	tya
	pha
	ldy #197
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17159)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17160)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #255
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 17161)
	; 16 bytes
	txa
	pha
	tya
	ldx #45
	ldy #74
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 17162)
	; 16 bytes
	php
	tya
	pha
	ldy #217
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17163)
	; 16 bytes
	tya
	ldy #24 ;hides 'clc'
	php
	pha
	lda #117
	jsr delay_a_25_clocks
	pla
	plp
	iny
	Jpl *-11
	tay
.elseif(n = 17164)
	; 16 bytes
	php
	tya
	ldy #0
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17165)
	; 16 bytes
	tya
	ldy #55
	pha
	delay_n 300
	pla
	dey
	Jne *-10
	tay
.elseif(n = 17166)
	; 16 bytes
	txa
	pha
	tya
	ldx #24
	ldy #136
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 17167)
	; 16 bytes
	php
	tya
	ldy #235
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 17168 to 17187
.if(n = 17168)
	; 16 bytes
	php
	tya
	pha
	ldy #10
	lda #6
	jsr delay_256a_16_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 17169)
	; 15 bytes
	tya
	pha
	ldy #128
	delay_n 128
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 17170)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #124
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 17171)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17172)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17173)
	; 15 bytes
	txa
	pha
	tya
	ldy #133
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 17174 || n = 17179 || n = 17182 || n = 17186)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17175 || n = 17180 || n = 17187)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17176 || n = 17181)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 17177)
	; 7 bytes
	php
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17178 || n = 17183)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17184)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17185)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17192)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 17189)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17190)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17191)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 17193)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17194)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17195)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17196)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17197)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17198)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17199 || n = 17200 || n = 17202 || n = 17203)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17201)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 17203 && n < 17211) || (n > 17211 && n < 17216) || (n > 17216 && n < 17344))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17211)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17216)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if((n > 17343 && n < 17378))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17378 || n = 17379)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17380)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 17381)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif(n = 17382)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 17383) && (n < 17402))
; 17383 to 17401
.if(n = 17383 || n = 17384)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17385)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17386)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17387)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17388 || n = 17389)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17390)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17391)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17392)
	; 16 bytes
	php
	tya
	ldy #220
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17393)
	; 16 bytes
	php
	tya
	pha
	ldy #238
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17394)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17395)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17396)
	; 16 bytes
	php
	tya
	ldy #212
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17397)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17398)
	; 16 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	delay_n 119
	dey
	Jne *-9
	pla
	tay
.elseif(n = 17399)
	; 16 bytes
	php
	tya
	pha
	ldy #220
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17400)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17401)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #130
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 17402 to 17421
.if(n = 17402)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #81
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 17403)
	; 15 bytes
	tya
	pha
	ldy #16
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 17404)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17405)
	; 15 bytes
	tya
	ldy #57
	delay_n 291
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 17406)
	; 16 bytes
	php
	tya
	ldy #223
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17407)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #183
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 17408)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17409)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 122
	Jne *-8
	pla
	tay
.elseif(n = 17410)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17411)
	; 16 bytes
	php
	tya
	ldy #137
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17412)
	; 16 bytes
	php
	tya
	ldy #232
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17413)
	; 16 bytes
	php
	tya
	pha
	ldy #223
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17414)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 224
	Jmi *-8
	pla
	tax
.elseif(n = 17415)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17416)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #37
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17417)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #41
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17418)
	; 16 bytes
	php
	tya
	pha
	ldy #137
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17419)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17420)
	; 16 bytes
	php
	tya
	ldy #136
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17421)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
.if((n >= 17422) && (n < 17441))
; 17422 to 17440
.if(n = 17422)
	; 16 bytes
	txa
	pha
	tya
	ldx #136 ;hides 'dey'
	ldy #23
	dey
	Jpl *-1
	dex
	Jne *-7
	tay
	pla
	tax
.elseif(n = 17423)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17424)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-272)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 17425)
	; 16 bytes
	tya
	ldy #134
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 17426)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17427)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17428)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17429)
	; 16 bytes
	tya
	ldy #72
	pha
	tya
	ldy #45
	dey
	Jne *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 17430 || n = 17435 || n = 17438)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 17431 || n = 17436)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17432 || n = 17437)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 17433)
	; 7 bytes
	php
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17434 || n = 17439)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17440)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 17441 to 17460
.if(n = 17441)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17442)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17443 || n = 17444 || n = 17448)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 17445)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17446)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17447)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 17449)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17450)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17451)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17452)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17453)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17454)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17455 || n = 17456 || n = 17458 || n = 17459)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17457)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17460)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 17461 to 17500
.if((n > 17460 && n < 17467) || (n > 17467 && n < 17472) || (n > 17472 && n < 17501))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17467)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17472)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17501) && (n < 18751))
.if((n >= 17501) && (n < 18126))
.if((n >= 17501) && (n < 17813))
.if((n >= 17501) && (n < 17657))
; 17501 to 17656
.if((n > 17500 && n < 17634))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17634 || n = 17635)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17636 || n = 17643)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17637)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 17637 && n < 17641))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17641)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17642)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 227
	Jmi *-8
	pla
	tax
.elseif(n = 17644 || n = 17645)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17646)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17647)
	; 16 bytes
	php
	tya
	pha
	ldy #226
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17648)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17649)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17650)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #204
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 17651)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17652)
	; 16 bytes
	php
	tya
	ldy #105
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17653)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17654)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17655)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17656)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
.if((n >= 17657) && (n < 17676))
; 17657 to 17675
.if(n = 17657)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #211
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 17658)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17659)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17660)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17661)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17662)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17663)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17664)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 17665)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #70
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17666)
	; 16 bytes
	php
	tya
	ldy #(n-12)/91
	pha
	lda #52
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17667)
	; 16 bytes
	php
	tya
	ldy #165
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17668)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #79
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17669)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17670)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17671)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 82
	Jne *-8
	pla
	tax
.elseif(n = 17672)
	; 16 bytes
	php
	tya
	pha
	ldy #139
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17673)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17674)
	; 16 bytes
	php
	tya
	pha
	ldy #165
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17675)
	; 16 bytes
	tya
	ldy #93
	pha
	tya
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	tay
	pla
	dey
	Jne *-10
	tay
.endif
.else
; 17676 to 17695
.if(n = 17676)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #83
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 17677)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17678)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17679)
	; 16 bytes
	php
	tya
	ldy #151
	pha
	lda #78
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17680)
	; 15 bytes
	txa
	pha
	tya
	ldy #45
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 17681)
	; 16 bytes
	php
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 17682)
	; 16 bytes
	php
	tya
	ldy #190
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17683)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 124
	Jne *-8
	pla
	tay
.elseif(n = 17684)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17685)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17686 || n = 17691 || n = 17694)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 17687 || n = 17692)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17688 || n = 17693)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 17689)
	; 7 bytes
	php
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17690 || n = 17695)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 17696 to 17734
.if(n = 17696)
	; 9 bytes
	php
	pha
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17697)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17698)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699 || n = 17700 || n = 17704)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 17701)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17702)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17703)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 17705)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17706)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17707)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17708)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17709)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17710)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17711 || n = 17712 || n = 17714 || n = 17715)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17713)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 17715 && n < 17723) || (n > 17723 && n < 17728) || (n > 17728 && n < 17735))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17723)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17728)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 17735 to 17812
.if((n > 17734 && n < 17813))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17890))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17890)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 17891) && (n < 17930))
.if((n >= 17891) && (n < 17910))
; 17891 to 17909
.if(n = 17891)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17892 || n = 17899)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17893)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 17893 && n < 17897))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17897)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17898)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17900 || n = 17901)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17902)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17903)
	; 16 bytes
	php
	tya
	pha
	ldy #68
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17904)
	; 16 bytes
	php
	tya
	pha
	ldy #245
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17905)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 17906)
	; 16 bytes
	php
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #167
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 17907)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17908)
	; 16 bytes
	php
	tya
	pha
	ldy #201
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17909)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 17910 to 17929
.if(n = 17910)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #75
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17911)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/71
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17912)
	; 16 bytes
	php
	tya
	ldy #179
	pha
	lda #61
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17913)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17914)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17915)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17916)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17917)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #82
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17918)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #107
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 17919)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #68
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17920)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17921)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17922)
	; 16 bytes
	php
	tya
	ldy #199
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17923)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17924)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17925)
	; 15 bytes
	tya
	ldy #127
	pha
	delay_n 128
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 17926)
	; 16 bytes
	php
	tya
	pha
	ldy #141
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17927)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17928)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17929)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 17930) && (n < 17949))
; 17930 to 17948
.if(n = 17930)
	; 15 bytes
	txa
	pha
	tya
	ldx #26
	ldy #136 ;hides 'dey'
	Jne *-1
	dex
	Jne *-5
	tay
	pla
	tax
.elseif(n = 17931)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #60
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17932)
	; 16 bytes
	php
	tya
	ldy #140
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17933)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/106
	lda #74
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17934)
	; 15 bytes
	txa
	pha
	tya
	ldx #61
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 17935)
	; 16 bytes
	txa
	pha
	tya
	ldx #98
	ldy #35
	dey
	Jne *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 17936)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-272)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 17937)
	; 16 bytes
	php
	tya
	ldy #239
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 17938)
	; 16 bytes
	php
	tya
	pha
	ldy #181
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17939)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #190
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 17940)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17941)
	; 16 bytes
	php
	tya
	pha
	ldy #206
	lda #(n-11)/326
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 17942 || n = 17947)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 17943 || n = 17948)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 17944)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 17945)
	; 7 bytes
	php
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17946)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 17949 to 17968
.if(n = 17949)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 17950 || n = 17954)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17951 || n = 17958)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17952)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17953)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 17955 || n = 17956 || n = 17960)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 17957)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 17959)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 17961)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17962)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17963)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17964)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17965)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17966)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17967 || n = 17968)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.endif
.else
; 17969 to 18125
.if(n = 17969)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17970 || n = 17971)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif((n > 17971 && n < 17979) || (n > 17979 && n < 17984) || (n > 17984 && n < 18126))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17979)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17984)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 18126) && (n < 18438))
.if((n >= 18126) && (n < 18282))
.if((n >= 18126) && (n < 18204))
.if((n >= 18126) && (n < 18165))
; 18126 to 18164
.if((n > 18125 && n < 18146))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18146 || n = 18147)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18148 || n = 18155)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18149)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 18149 && n < 18153))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18153)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18154)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18156 || n = 18157)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18158)
	; 16 bytes
	php
	tya
	pha
	ldy #187
	lda #65
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18159)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18160)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #109
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 18161)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18162)
	; 16 bytes
	php
	tya
	ldy #242
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18163)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18164)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18166)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18167)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #193
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 18168)
	; 16 bytes
	php
	tya
	ldy #204
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18169)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18170)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18171)
	; 16 bytes
	tya
	ldy #62
	php
	pha
	lda #247
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 18172)
	; 16 bytes
	php
	tya
	ldy #227
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18173)
	; 16 bytes
	php
	tya
	ldy #143
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18174)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 234
	Jmi *-8
	pla
	tax
.elseif(n = 18175)
	; 15 bytes
	tya
	ldy #45
	delay_n 386
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 18176)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-290)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18177)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #66
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18178)
	; 15 bytes
	tya
	pha
	ldy #62
	delay_n 288
	dey
	Jne *-7
	pla
	tay
.elseif(n = 18179)
	; 16 bytes
	php
	tya
	pha
	ldy #227
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18180)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 1812
	Jmi *-8
	pla
	tay
.elseif(n = 18181)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18182)
	; 16 bytes
	php
	tya
	ldy #230
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18183)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
; 18184 to 18203
.if(n = 18184)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18185)
	; 16 bytes
	tya
	ldy #90
	pha
	tya
	ldy #37
	dey
	Jne *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 18186)
	; 16 bytes
	php
	tya
	ldy #233
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18187)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #7
	jsr delay_256a_16_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18188)
	; 16 bytes
	php
	tya
	ldy #142
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18189)
	; 16 bytes
	php
	tya
	pha
	ldy #230
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18190)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #83
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18191)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18192)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18193)
	; 16 bytes
	php
	tya
	pha
	ldy #233
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18194)
	; 16 bytes
	tya
	ldy #46
	delay_n 378
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 18195)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18196)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18197)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #90
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18198 || n = 18203)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18199)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18200)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 18201)
	; 7 bytes
	php
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18202)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204 || n = 18211 || n = 18212 || n = 18216)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 18205)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 18206 || n = 18210)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18207 || n = 18214)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18208)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18209)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18213)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18215)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 18217)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18218)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18219)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18220)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18221)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18222)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18223 || n = 18224 || n = 18226 || n = 18227)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18225)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18227 && n < 18235) || (n > 18235 && n < 18240) || (n > 18240 && n < 18282))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18235)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18240)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
; 18282 to 18359
.if((n > 18281 && n < 18360))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18399))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 18399) && (n < 18418))
; 18399 to 18417
.if((n > 18398 && n < 18402))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18402 || n = 18403)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18404 || n = 18411)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18405)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 18405 && n < 18409))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18409)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18410)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18412 || n = 18413)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18414)
	; 16 bytes
	php
	tya
	pha
	ldy #64
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18415)
	; 16 bytes
	php
	tya
	pha
	ldy #252
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18416)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18417)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #58
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #136
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 18419)
	; 16 bytes
	php
	tya
	pha
	ldy #230
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18420)
	; 16 bytes
	php
	tya
	ldy #236
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18421)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #52
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18422)
	; 16 bytes
	php
	tya
	pha
	ldy #239
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18423)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18424)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18425)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #143
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18426)
	; 15 bytes
	tya
	pha
	ldy #18
	delay_n 1018
	dey
	Jne *-7
	pla
	tay
.elseif(n = 18427)
	; 16 bytes
	php
	tya
	pha
	ldy #236
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18428)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18429)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18430)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18431)
	; 16 bytes
	php
	tya
	ldy #163
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18432)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18433)
	; 16 bytes
	php
	tya
	pha
	ldy #198
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18434)
	; 16 bytes
	php
	tya
	pha
	ldy #145
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18435)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18436)
	; 16 bytes
	php
	tya
	ldy #196
	pha
	lda #55
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18437)
	; 16 bytes
	php
	tya
	ldy #66
	pha
	lda #236
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18438) && (n < 18594))
.if((n >= 18438) && (n < 18516))
.if((n >= 18438) && (n < 18477))
.if((n >= 18438) && (n < 18457))
; 18438 to 18456
.if(n = 18438)
	; 16 bytes
	php
	tya
	pha
	ldy #163
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18439)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18440)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18441)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #90
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18442)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #159
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 18443)
	; 16 bytes
	php
	tya
	pha
	ldy #196
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18444)
	; 16 bytes
	php
	tya
	ldy #144
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18445)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18446)
	; 16 bytes
	txa
	pha
	tya
	ldx #24 ;hides 'clc'
	ldy #152
	dey
	Jne *-1
	dex
	Jne *-7
	tay
	pla
	tax
.elseif(n = 18447)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/271
	lda #239
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18448)
	; 15 bytes
	txa
	pha
	tya
	ldy #47
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 18449)
	; 16 bytes
	php
	tya
	pha
	ldy #194
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18450)
	; 16 bytes
	tya
	ldy #84
	pha
	tya
	ldy #39
	dey
	Jpl *-1
	tay
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 18451)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18452)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18453)
	; 16 bytes
	txa
	pha
	tya
	ldx #102
	ldy #24 ;hides 'clc'
	dey
	Jpl *-2
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 18454)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 18455)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 18456)
	; 8 bytes
	pla
	delay_n (n-4)
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 7 bytes
	php
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18458 || n = 18463 || n = 18470)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18459 || n = 18462 || n = 18466)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18460 || n = 18467 || n = 18468 || n = 18472)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 18461)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 18464)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18465)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18469)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18471)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 18473)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18474)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18475)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18476)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18478)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18479 || n = 18480 || n = 18482 || n = 18483)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18481)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18483 && n < 18491) || (n > 18491 && n < 18496) || (n > 18496 && n < 18516))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18491)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18496)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 18516 to 18593
.if((n > 18515 && n < 18594))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18658))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18658 || n = 18659)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18660 || n = 18667)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18661)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 18661 && n < 18665))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18665)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18666)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18668 || n = 18669)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18670)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 18671)
	; 16 bytes
	php
	delay_n (n-11)
	pla
	plp
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18673)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #142
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18674)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18675)
	; 16 bytes
	php
	tya
	pha
	ldy #212
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18676)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18678)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18679)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #46
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tax
.elseif(n = 18680)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18681)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18682)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18683)
	; 16 bytes
	tya
	ldy #65
	php
	pha
	lda #237
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jpl *-10
	tay
.elseif(n = 18684)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	delay_n 800
	pla
	Jcs *-11
	plp
.elseif(n = 18685)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #70
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18686)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 87
	Jne *-8
	pla
	tax
.elseif(n = 18687)
	; 16 bytes
	php
	tya
	ldy #249
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18688)
	; 16 bytes
	php
	tya
	pha
	ldy #147
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18689)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18690)
	; 16 bytes
	php
	tya
	ldy #65
	pha
	lda #244
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18692)
	; 16 bytes
	php
	tya
	pha
	ldy #70
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18693)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18694)
	; 15 bytes
	php
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
	plp
.elseif(n = 18695)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18696)
	; 16 bytes
	php
	tya
	ldy #173
	pha
	lda #69
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18697)
	; 16 bytes
	php
	tya
	pha
	ldy #65
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18698)
	; 16 bytes
	tya
	ldy #201
	php
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	plp
	dey
	Jne *-10
	tay
.elseif(n = 18699)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #200
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 18700)
	; 16 bytes
	php
	tya
	ldy #146
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18701)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-16)/101
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 18702)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #87
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18703)
	; 16 bytes
	php
	tya
	pha
	ldy #173
	lda #76
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18704)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18705)
	; 16 bytes
	php
	tya
	ldy #201
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18706)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 241
	Jmi *-8
	pla
	tax
.elseif(n = 18707)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18708)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18709)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18710)
	; 8 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711 || n = 18716 || n = 18723 || n = 18724 || n = 18728)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 18712 || n = 18717)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 18713)
	; 7 bytes
	php
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18714 || n = 18719 || n = 18726)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18715 || n = 18718 || n = 18722)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18720)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18721)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18725)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18727)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 18729)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18730)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18731)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18732)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18733)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18734)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18735 || n = 18736 || n = 18738 || n = 18739)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18737)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18739 && n < 18747) || (n > 18747 && n < 18751))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18747)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
; 18751 to 18906
.if(n = 18751 || (n > 18752 && n < 18907))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18752)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
.if((n >= 18907) && (n < 18926))
; 18907 to 18925
.if((n > 18906 && n < 18914))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18914 || n = 18915)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18916 || n = 18923)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18917)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 18917 && n < 18921))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18921)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18922)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18924 || n = 18925)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.endif
.else
; 18926 to 18945
.if(n = 18926)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #227
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 18927)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #203
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 18928)
	; 16 bytes
	php
	tya
	pha
	ldy #191
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18929)
	; 16 bytes
	php
	tya
	pha
	ldy #155
	lda #90
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18930)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18931)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/96
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18932)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18933)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #66
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18934)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 244
	Jmi *-8
	pla
	tax
.elseif(n = 18935)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18936)
	; 16 bytes
	php
	tya
	ldy #(n-12)/76
	pha
	lda #37
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18937)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #186
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18938)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #141
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 18939)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18940)
	; 16 bytes
	php
	tya
	pha
	ldy #159
	lda #87
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18941)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18942)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18943)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/76
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18944)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18945)
	; 16 bytes
	php
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #148
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.endif
.endif
.else
.if((n >= 18946) && (n < 18965))
; 18946 to 18964
.if(n = 18946)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #222
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 18947)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/91
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18948)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18949)
	; 16 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #136
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 18950)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18951)
	; 16 bytes
	php
	tya
	ldy #177
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18952)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18953)
	; 16 bytes
	php
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 18954)
	; 16 bytes
	php
	tya
	ldy #154
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18955)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18956)
	; 16 bytes
	php
	tya
	ldy #148
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 18957)
	; 16 bytes
	txa
	pha
	tya
	ldx #31
	ldy #121
	dey
	Jne *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 18958)
	; 16 bytes
	php
	tya
	pha
	ldy #177
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18959)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #43
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 18960)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-272)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 18961)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18962)
	; 16 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	plp
.elseif(n = 18963)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 18964)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 18965 to 18984
.if(n = 18965)
	; 15 bytes
	tya
	ldy #47
	delay_n 386
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 18966 || n = 18971 || n = 18974 || n = 18978)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18967 || n = 18972 || n = 18979 || n = 18980 || n = 18984)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 18968 || n = 18973)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 18969)
	; 7 bytes
	php
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18970 || n = 18975 || n = 18982)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18976)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18977)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 18981)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 18983)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18986)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18987)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18988)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18989)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18990)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18991 || n = 18992 || n = 18994 || n = 18995)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18993)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 18995 && n < 19003) || (n > 19003 && n < 19008) || (n > 19008 && n < 19063))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19003)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19008)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
; 19063 to 19140
.if((n > 19062 && n < 19141))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if((n > 19140 && n < 19170))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19170 || n = 19171)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19172 || n = 19179)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19173)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 19173 && n < 19177))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19177)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 19178)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180 || n = 19181)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19182)
	; 16 bytes
	php
	tya
	ldy #213
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19183)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #94
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 19184)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19185)
	; 15 bytes
	tya
	ldy #137
	pha
	delay_n 128
	pla
	dey
	Jne *-9
	tay
.elseif(n = 19186)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19187)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19188)
	; 16 bytes
	php
	tya
	ldy #204
	pha
	lda #55
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19189)
	; 16 bytes
	php
	tya
	pha
	ldy #213
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19190)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 135
	Jne *-8
	pla
	tay
.elseif(n = 19191)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19192)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19193)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19194)
	; 16 bytes
	php
	tya
	ldy #139
	pha
	lda #99
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19195)
	; 16 bytes
	php
	tya
	pha
	ldy #204
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19196)
	; 16 bytes
	php
	tya
	pha
	ldy #151
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19197)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19198)
	; 16 bytes
	php
	tya
	ldy #181
	pha
	lda #67
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 19199 to 19218
.if(n = 19199)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #242
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19200)
	; 16 bytes
	php
	tya
	ldy #156
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19201)
	; 16 bytes
	php
	tya
	pha
	ldy #69
	lda #246
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19202)
	; 16 bytes
	php
	tya
	ldy #202
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19203)
	; 16 bytes
	php
	tya
	pha
	ldy #218
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19204)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19205)
	; 15 bytes
	tya
	ldy #63
	delay_n 291
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 19206)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19207)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19208)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19209)
	; 16 bytes
	php
	tya
	pha
	ldy #202
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19210)
	; 16 bytes
	php
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #55
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 19211)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 19212)
	; 16 bytes
	php
	tya
	ldy #150
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19213)
	; 16 bytes
	php
	tya
	ldy #(n-12)/91
	pha
	lda #52
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19214)
	; 16 bytes
	txa
	pha
	tya
	ldx #57
	ldy #64
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
.elseif(n = 19215)
	; 16 bytes
	php
	tya
	ldy #(n-12)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19216)
	; 15 bytes
	txa
	pha
	tya
	ldy #49
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 19217)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19218)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.endif
.endif
.endif
.endif
.else
; 19219 to 19375
.if(n = 19219)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19220)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19221)
	; 17 bytes
	php
	.repeat 2
	 lda #99
	 jsr delay_a_25_clocks
	.endrepeat
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19222 || n = 19227 || n = 19230 || n = 19234)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19223 || n = 19228 || n = 19235 || n = 19236 || n = 19240)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 19224 || n = 19229)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 19225)
	; 7 bytes
	php
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19226 || n = 19231 || n = 19238)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19232)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19233)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 19237)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19239)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 19241)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19242)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19243)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19244)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19245)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19246)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19247 || n = 19248 || n = 19250 || n = 19251)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19249)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 19251 && n < 19259) || (n > 19259 && n < 19264) || (n > 19264 && n < 19376))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19259)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19264)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
; 19376 to 19453
.if((n > 19375 && n < 19426))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19426 || n = 19427)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19428 || n = 19435)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19429)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 19429 && n < 19433))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19433)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 19434)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19436 || n = 19437)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19438)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 19439)
	; 16 bytes
	php
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19440)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19441)
	; 16 bytes
	php
	tya
	pha
	ldy #249
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19442)
	; 16 bytes
	php
	tya
	ldy #145
	pha
	lda #95
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19443)
	; 16 bytes
	php
	tya
	ldy #153
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19444)
	; 16 bytes
	php
	tya
	pha
	ldy #185
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19445)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19446)
	; 16 bytes
	php
	tya
	ldy #158
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19447)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19448)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19449)
	; 15 bytes
	tya
	pha
	ldy #18
	delay_n 1018
	dey
	Jpl *-7
	pla
	tay
.elseif(n = 19450)
	; 16 bytes
	php
	tya
	pha
	ldy #153
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19451)
	; 16 bytes
	php
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #45
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19452)
	; 16 bytes
	php
	tya
	ldy #243
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19453)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.endif
.else
.if((n >= 19454) && (n < 19493))
.if((n >= 19454) && (n < 19473))
; 19454 to 19472
.if(n = 19454)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #83
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19455)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19456)
	; 16 bytes
	php
	tya
	pha
	ldy #209
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19457)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #96
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 19458)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #146
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	plp
.elseif(n = 19459)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	lda #210
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tax
.elseif(n = 19460)
	; 16 bytes
	php
	tya
	ldy #221
	pha
	lda #49
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19461)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19462)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19463)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19464)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 137
	Jne *-8
	pla
	tay
.elseif(n = 19465)
	; 15 bytes
	tya
	ldy #139
	pha
	delay_n 128
	pla
	dey
	Jne *-9
	tay
.elseif(n = 19466)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 251
	Jmi *-8
	pla
	tax
.elseif(n = 19467)
	; 16 bytes
	php
	tya
	pha
	ldy #221
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19468)
	; 16 bytes
	php
	tya
	ldy #152
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19469)
	; 16 bytes
	pla
	delay_n (n-4)
.elseif(n = 19470)
	; 16 bytes
	php
	tya
	ldy #207
	pha
	lda #55
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19471)
	; 17 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 19472)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #239
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.endif
.else
; 19473 to 19492
.if(n = 19473)
	; 16 bytes
	php
	tya
	pha
	ldy #70
	lda #242
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19474)
	; 16 bytes
	php
	tya
	ldy #73
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 19475)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19476)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19477)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #168
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 19478 || n = 19483 || n = 19486 || n = 19490)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19479 || n = 19484 || n = 19491 || n = 19492)
	; 8..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19480 || n = 19485)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 19481)
	; 7 bytes
	php
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19482 || n = 19487)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19488)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19489)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19494)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19495)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 19496)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 19497)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19498)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19499)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19500)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19501)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19502)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19503 || n = 19504 || n = 19506 || n = 19507)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19505)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif((n > 19507 && n < 19515) || (n > 19515 && n < 19520) || (n > 19520 && n < 19532))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19515)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19520)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 19532 to 19687
.if((n > 19531 && n < 19682))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19682 || n = 19683)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19684)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 19685)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif(n = 19686 || n = 19687)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 19688) && (n < 19844))
.if((n >= 19688) && (n < 19766))
.if((n >= 19688) && (n < 19727))
.if((n >= 19688) && (n < 19707))
; 19688 to 19706
.if(n = 19688)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19689)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 19690)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19691)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19692 || n = 19693)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19694)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 254
	Jmi *-8
	pla
	tax
.elseif(n = 19695)
	; 16 bytes
	php
	tya
	ldy #(n-12)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19696)
	; 15 bytes
	tya
	pha
	ldy #148
	delay_n 128
	dey
	Jne *-7
	pla
	tay
.elseif(n = 19697)
	; 16 bytes
	php
	tya
	ldy #155
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19698)
	; 16 bytes
	php
	tya
	ldy #193
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19699)
	; 16 bytes
	php
	tya
	pha
	ldy #246
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19700)
	; 16 bytes
	php
	tya
	ldy #214
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19701)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 92
	Jne *-8
	pla
	tax
.elseif(n = 19702)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/81
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19703)
	; 16 bytes
	php
	tya
	pha
	ldy #76
	lda #227
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19704)
	; 16 bytes
	php
	tya
	pha
	ldy #155
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19705)
	; 16 bytes
	php
	tya
	pha
	ldy #193
	lda #70
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19706)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #170
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
.elseif(n = 19708)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19709)
	; 16 bytes
	php
	tya
	pha
	ldy #179
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19710)
	; 16 bytes
	php
	tya
	pha
	ldy #203
	lda #65
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19711)
	; 16 bytes
	php
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #195
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	plp
.elseif(n = 19712)
	; 16 bytes
	php
	tya
	ldy #197
	pha
	lda #61
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19713)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/86
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19714)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/101
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19715)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19716)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19717)
	; 16 bytes
	php
	tya
	pha
	ldy #201
	lda #66
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19718)
	; 16 bytes
	php
	tya
	ldy #167
	pha
	lda #79
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19719)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 848
	pla
	Jcs *-11
	plp
.elseif(n = 19720)
	; 16 bytes
	php
	tya
	pha
	ldy #199
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19721)
	; 16 bytes
	tya
	ldy #93
	pha
	tya
	ldy #39
	dey
	Jne *-1
	tay
	pla
	dey
	Jne *-10
	tay
.elseif(n = 19722)
	; 16 bytes
	php
	tya
	ldy #219
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19723)
	; 16 bytes
	php
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #133
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19724)
	; 16 bytes
	php
	tya
	ldy #154
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19725)
	; 16 bytes
	php
	tya
	pha
	ldy #167
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19726)
	; 16 bytes
	txa
	pha
	tya
	ldx #135
	ldy #27
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.endif
.endif
.else
; 19727 to 19765
.if(n = 19727)
	; 16 bytes
	tya
	ldy #37
	pha
	delay_n 507
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 19728)
	; 16 bytes
	php
	tya
	ldy #212
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19729)
	; 16 bytes
	php
	tya
	pha
	ldy #219
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19730)
	; 16 bytes
	php
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #44
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	plp
.elseif(n = 19731)
	; 15 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	plp
	Jne *-10
	tay
.elseif(n = 19732)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19733)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #152
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19734 || n = 19739 || n = 19742 || n = 19746)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19735 || n = 19740 || n = 19747 || n = 19748 || n = 19752)
	; 8..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 19736 || n = 19741)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 19737)
	; 7 bytes
	php
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19738 || n = 19743 || n = 19750)
	; 9..11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19744)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19745)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 19749)
	; 10 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19751)
	; 11 bytes
	php
	delay_n (n-11)
	pla
	plp
.elseif(n = 19753)
	; 11 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19754)
	; 12 bytes
	.repeat <(n-40)
	 pha
	 php
	.endrepeat
	lda #>(n-40)
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19755)
	; 12 bytes
	php
	lda #<(n-43)
	jsr delay_256a_16_clocks
	lda #>(n-43)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19756)
	; 11 bytes
	php
	lda #<(n-34) ;hides 'asl a'
	Jpl *-1
	lda #>(n-34)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19757)
	; 12 bytes
	php
	lda #<(n-3) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-3)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19758)
	; 12 bytes
	php
	clc
	lda #<(n-4) ;hides 'rol a'
	Jcc *-1
	lda #>(n-4)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19759 || n = 19760 || n = 19762 || n = 19763)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19761)
	; 11 bytes
	php
	lda #<(n-39) ;hides 'asl a'
	Jpl *-1
	lda #>(n-39)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19764 || n = 19765)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 19766 to 19843
.if((n > 19765 && n < 19771) || (n > 19771 && n < 19776) || (n > 19776 && n < 19844))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19771)
	; 11 bytes
	php
	lda #<(n+15) ;hides 'lsr a'
	Jne *-1
	lda #>(n+15)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19776)
	; 11 bytes
	php
	lda #<(n+10) ;hides 'lsr a'
	Jne *-1
	lda #>(n+10)
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
; 19844 to 19921
.if((n > 19843 && n < 19922))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if((n > 19921 && n < 19938))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19938 || n = 19939)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19940 || n = 19947)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 19941)
	; 13 bytes
	pla
	delay_n (n-4)
.elseif((n > 19941 && n < 19945))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19945)
	; 14 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 19946)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19948 || n = 19949)
	; 15 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 19950)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 19951)
	; 16 bytes
	php
	tya
	ldy #157
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19952)
	; 16 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19953)
	; 16 bytes
	php
	tya
	ldy #69
	pha
	lda #250
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19954)
	; 16 bytes
	php
	tya
	ldy #169
	pha
	lda #79
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19955)
	; 16 bytes
	php
	tya
	pha
	ldy #224
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19956)
	; 16 bytes
	php
	tya
	ldy #72
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19957)
	; 15 bytes
	txa
	pha
	tya
	ldx #68
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	tay
	pla
	tax
.elseif(n = 19958)
	; 16 bytes
	php
	tya
	pha
	ldy #157
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19959)
	; 16 bytes
	php
	tya
	ldy #183
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19960)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 793
	Jne *-8
	pla
	tax
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if(n = 19961)
	; 16 bytes
	php
	tya
	pha
	ldy #169
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19962)
	; 15 bytes
	tya
	pha
	ldy #150
	delay_n 128
	dey
	Jne *-7
	pla
	tay
.elseif(n = 19963)
	; 16 bytes
	php
	tya
	pha
	ldy #72
	lda #245
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19964)
	; 16 bytes
	php
	tya
	ldy #(n-12)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19965)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19966)
	; 16 bytes
	php
	tya
	pha
	ldy #183
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19967)
	; 16 bytes
	php
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #99
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	plp
.elseif(n = 19968)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19969)
	; 16 bytes
	php
	tya
	pha
	ldy #210
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19970)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-300)/281
	lda #249
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19971)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-19)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19972)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19973)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19974)
	; 16 bytes
	php
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19975)
	; 16 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #124
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	plp
.elseif(n = 19976)
	; 16 bytes
	php
	tya
	ldy #217
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19977)
	; 16 bytes
	php
	tya
	ldy #(n-12)/121
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19978)
	; 16 bytes
	php
	tya
	ldy #149
	pha
	lda #95
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 19979)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2848
	Jcs *-11
	plp
.elseif(n = 19980)
	; 16 bytes
	php
	tya
	ldy #156
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	plp
.endif
.else
; 19981 to 20000
.if(n = 19981)
	; 16 bytes
	txa
	pha
	tya
	ldx #(n-16)/121
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
.elseif(n = 19982)
	; 16 bytes
	php
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #131
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	plp
.elseif(n = 19983)
	; 16 bytes
	php
	tya
	pha
	ldy #(n-180)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 19984)
	; 15 bytes
	txa
	pha
	tya
	ldy #52
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	tay
	pla
	tax
.elseif(n = 19985)
	; 16 bytes
	php
	tya
	pha
	ldy #149
	lda #102
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19986)
	; 16 bytes
	tya
	ldy #52
	delay_n 368
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 19987)
	; 16 bytes
	php
	tya
	pha
	ldy #0
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 19988)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19989)
	; 17 bytes
	php
	.repeat 2
	 lda #99
	 jsr delay_a_25_clocks
	.endrepeat
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19990 || n = 19995 || n = 19998)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 19991 || n = 19996)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 19992 || n = 19997)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 19993)
	; 7 bytes
	php
	lda #78
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19994 || n = 19999)
	; 9 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 20000)
	; 9 bytes
	php
	pha
	lda #78
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif
.endif

.endif
	rest
.endmacro
