; delay_n, delays n cycles with small code
; Will clobber S, Z+N, D, and V
; Will preserve A, X, Y, C, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Uses JMP (unrelocatable code).
;
; WARNING: May write garbage into $500-$5FF.
;
; Requires a dummy interrupt handler that does nothing but RTI.
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
; -11 bytes or less when n <= 1311
; -12 bytes or less when n <= 1356
; -13 bytes or less when n <= 2647
; -14 bytes or less when n <= 3648
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=8.9, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.1, max=11
; - When 501 <= n <= 750: min=8, avg=10.1, max=11
; - When 751 <= n <= 1000: min=9, avg=10.1, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.9, max=13
; - When 2001 <= n <= 3000: min=10, avg=12.1, max=14
; - When 3001 <= n <= 5000: min=10, avg=12.9, max=15
; - When 5001 <= n <= 7500: min=11, avg=14.4, max=15
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
.elseif(n = 4 || n = 5 || n = 8 || n = 17 || n = 28 || n = 31 || n = 41 || n = 43 || n = 44)
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
.elseif((n > 8 && n < 12) || (n > 19 && n < 23) || (n > 32 && n < 36) || n = 37 || n = 46)
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
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 36 || n = 39 || n = 42 || n = 45)
	; 3..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 40)
	; 5..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	pha
	tya
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
	tay
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
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 320 || n = 355 || n = 369 || n = 376 || n = 411 || n = 439 || n = 446 || n = 453)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 314 || n = 316 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 339 || n = 343 || n = 344 || n = 346 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 361 || n = 364 || n = 366 || n = 374 || n = 378 || n = 379 || n = 381 || n = 384 || n = 386 || n = 389 || n = 391 || n = 394 || n = 399 || n = 401 || n = 413 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 444 || n = 448 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464 || n = 466)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 315)
	; 10 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
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
.elseif(n = 318)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif((n .mod 5 = 3) && ((n > 322 && n < 339) || n = 358 || n = 368 || n = 373 || n = 398 || n = 423 || (n > 432 && n < 444) || n = 458 || n = 463))
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 325 || n = 330 || n = 335 || n = 340 || n = 345 || n = 350 || n = 360 || n = 365 || n = 375 || n = 380 || n = 385 || n = 393 || n = 395 || n = 400 || n = 405 || n = 410 || n = 420 || n = 430 || n = 435 || n = 440 || n = 445 || n = 450 || n = 455 || n = 465)
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
.elseif(n = 334 || n = 341 || n = 348 || n = 383 || n = 404 || n = 418 || n = 425 || n = 460)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 336 || n = 371 || n = 406 || n = 441)
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
.elseif(n = 363 || n = 408)
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
.elseif(n = 388)
	; 11 bytes
	pha
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
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
.elseif(n = 416)
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
.elseif(n = 468)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.endif
.else
; 469 to 625
.if(n = 469 || n = 471 || n = 479 || n = 483 || n = 484 || n = 486 || n = 489 || n = 491 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 518 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541 || n = 546 || n = 549 || n = 553 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 571 || n = 574 || n = 581 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 623 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 469 && n < 491) || (n > 499 && n < 526) || (n > 534 && n < 561) || (n > 569 && n < 596) || (n > 604 && n < 626)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 472 || n = 482 || n = 497 || n = 507 || n = 527 || n = 577 || n = 582 || n = 587 || n = 592 || n = 602 || n = 622)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif((n .mod 5 = 3) && (n = 473 || n = 493 || n = 503 || n = 513 || n = 533 || n = 538 || (n > 562 && n < 584) || (n > 597 && n < 614)))
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 474 || n = 481 || n = 509 || n = 523 || n = 530 || n = 551 || n = 558 || n = 593 || n = 614 || n = 621)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
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
.elseif(n = 488 || n = 495 || n = 516 || n = 544 || n = 565 || n = 579 || n = 586 || n = 600)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 498 || n = 561)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 508)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
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
.elseif(n = 521)
	; 11 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 543 || n = 588 || n = 606)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 548)
	; 11 bytes
	pha
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
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
.elseif(n = 618)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jmp *+3
	Jmi *-4
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 686 || n = 689 || n = 693 || n = 694 || n = 699 || n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 724 || n = 728 || n = 729 || n = 731 || n = 734 || n = 736 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 763 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
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
.elseif(n = 628 || n = 635 || n = 656 || n = 670 || n = 691 || n = 705 || n = 733)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 630 || n = 636 || n = 640 || n = 645 || n = 650 || n = 655 || n = 660 || n = 665 || n = 675 || n = 680 || n = 685 || n = 690 || n = 698 || n = 700 || n = 710 || n = 715 || n = 720 || n = 725 || n = 730 || n = 735 || n = 741 || n = 745 || n = 750 || n = 760 || n = 765 || n = 770 || n = 780)
	; 9..10 bytes
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
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 778)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 643 || n = 653 || n = 673 || n = 683 || n = 688 || n = 703 || n = 713 || n = 718 || n = 753 || n = 758 || n = 773 || n = 778)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 648)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 649 || n = 663 || n = 684 || n = 719 || n = 726 || n = 740 || n = 754 || n = 761 || n = 768 || n = 775)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 651 || n = 678)
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
.elseif(n = 668)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
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
.elseif(n = 696 || n = 723)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
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
.elseif(n = 721)
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
.elseif(n = 748)
	; 11 bytes
	pha
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 755)
	; 10 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jpl *-3
	tay
	pla
.elseif(n = 756)
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
.endif
.else
; 782 to 937
.if((n .mod 5 = 2) && ((n > 781 && n < 938)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 5 = 3) && (n = 783 || n = 793 || n = 818 || n = 823 || n = 848 || n = 853 || n = 863 || n = 878 || n = 883 || n = 893 || n = 898 || (n > 912 && n < 934)))
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 784 || n = 794 || n = 798 || n = 799 || n = 801 || n = 804 || n = 806 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 833 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 864 || n = 868 || n = 869 || n = 871 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 806) || (n > 814 && n < 841) || (n > 849 && n < 876) || (n > 884 && n < 906) || (n > 919 && n < 936)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 786 || n = 813 || n = 858 || n = 903)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 788)
	; 11 bytes
	pha
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 789 || n = 796 || n = 831 || n = 845 || n = 859 || n = 894 || n = 929 || n = 936)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 791 || n = 826 || n = 861 || n = 896 || n = 931)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 803 || n = 810 || n = 824 || n = 838 || n = 866 || n = 873 || n = 880 || n = 901 || n = 908 || n = 915)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
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
.elseif(n = 816 || n = 888)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 828)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 939 || n = 941 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 991 || n = 994 || n = 1004 || n = 1009 || n = 1011 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1039 || n = 1043 || n = 1044 || n = 1046 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1078 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 940 || n = 945 || n = 955 || n = 960 || n = 965 || n = 970 || n = 975 || n = 980 || n = 990 || n = 995 || n = 1000 || n = 1005 || n = 1010 || n = 1015 || n = 1028 || n = 1030 || n = 1035 || n = 1040 || n = 1045 || n = 1050 || n = 1060 || n = 1065 || n = 1070 || n = 1075 || n = 1080 || n = 1085)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1093)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && (n = 943 || n = 950 || (n > 963 && n < 986) || (n > 998 && n < 1021) || (n > 1033 && n < 1056) || (n > 1068 && n < 1091)))
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
.elseif(n = 948 || n = 993)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 953 || n = 958 || n = 963 || n = 968 || n = 998 || n = 1003 || n = 1023 || n = 1033 || n = 1053 || n = 1058 || n = 1073 || n = 1088 || n = 1093)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 966 || n = 1001 || n = 1071)
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
.elseif(n = 988)
	; 11 bytes
	pha
	tya
	ldy #(n-20)/8
	bit $24
	dey
	Jpl *-3
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
.elseif(n = 1008 || n = 1068)
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
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1109 || n = 1113 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1148 || n = 1149 || n = 1151 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1179 || n = 1183 || n = 1184 || n = 1186 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1121) || (n > 1129 && n < 1156) || (n > 1164 && n < 1191) || (n > 1199 && n < 1226) || (n > 1234 && n < 1251)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && ((n > 1096 && n < 1248)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 5 = 3) && (n = 1098 || n = 1103 || n = 1123 || (n > 1132 && n < 1144) || (n > 1157 && n < 1169) || n = 1178 || n = 1193 || (n > 1202 && n < 1214) || n = 1233 || n = 1238))
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1103 && n < 1126) || (n > 1138 && n < 1161) || (n > 1173 && n < 1196) || (n > 1208 && n < 1231) || n = 1244))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1106 || n = 1141 || n = 1211 || n = 1246)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1108 || n = 1228)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1116 || n = 1176 || n = 1248)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
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
.elseif(n = 1218)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1253 || n = 1254 || n = 1256 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1288 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1302 || n = 1309 || n = 1316 || n = 1318 || n = 1322 || n = 1323)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1255 || n = 1260 || n = 1270 || n = 1275 || n = 1280 || n = 1285 || n = 1290 || n = 1295 || n = 1301 || n = 1310)
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
.elseif(n = 1268 || n = 1324)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1273 || n = 1278 || n = 1283)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 1298)
	; 9 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1299 || n = 1305)
	; 11 bytes
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
.elseif(n = 1306)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $70A0'
	ldy #112
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1311)
	; 11 bytes
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
	tya
	ldy #<$2C ;hides 'bit $91A0'
	ldy #145
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1325)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1331 || n = 1338 || n = 1345 || n = 1359 || n = 1366 || n = 1373 || n = 1381 || n = 1390 || n = 1394 || n = 1401)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1330 || n = 1336 || n = 1337 || n = 1339 || n = 1350 || n = 1351 || n = 1355 || n = 1357 || n = 1358 || n = 1365 || n = 1367 || n = 1372 || n = 1379 || n = 1386 || n = 1393 || n = 1395 || n = 1399 || n = 1400 || n = 1402)
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
.elseif(n = 1333)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1334)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	dey
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
.elseif(n = 1340 || n = 1348 || n = 1364 || n = 1388 || n = 1396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
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
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $6CA0'
	ldy #108
	iny
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
.elseif(n = 1347 || n = 1354 || n = 1375 || n = 1382 || n = 1403)
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
.elseif(n = 1352)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1360)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $6AA0'
	ldy #106
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1361)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $96A0'
	ldy #150
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1369)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $97A0'
	ldy #151
	dey
	Jne *-4
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
.elseif(n = 1397)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $9AA0'
	ldy #154
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1406)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1416 || n = 1425 || n = 1443)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1408 || n = 1413 || n = 1422 || n = 1429 || n = 1434)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1430)
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
.elseif(n = 1411 || n = 1439)
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
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1441)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1414 || n = 1418 || n = 1421 || n = 1427 || n = 1435 || n = 1442)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1415)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $64A0'
	ldy #100
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1417 || n = 1438)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1420 || n = 1436 || n = 1444)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1423)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1424)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1428)
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
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $9EA0'
	ldy #158
	dey
	Jne *-4
	tay
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
.elseif(n = 1441)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $61A0'
	ldy #97
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1445)
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
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1449 || n = 1456 || n = 1458 || n = 1463 || n = 1465 || n = 1477 || n = 1481 || n = 1484)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1446 && n < 1483)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
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
	txa
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
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
.elseif(n = 1452 || n = 1470 || n = 1479)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1455 || n = 1457 || n = 1471)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1459)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A1A0'
	ldy #161
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1460)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1462)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
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
.elseif(n = 1467 || n = 1474)
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
.elseif(n = 1469)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1472)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1473 || n = 1480)
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
.elseif(n = 1476)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1478)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A3A0'
	ldy #163
	dey
	Jne *-4
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
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485 || n = 1499 || n = 1511 || n = 1520)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1486)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1487)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1487 && n < 1516)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1490 || n = 1491 || n = 1493 || n = 1498 || n = 1502 || n = 1505 || n = 1507 || n = 1519 || n = 1521)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1492 || n = 1516)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1494 || n = 1501)
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
.elseif(n = 1495)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1500 || n = 1512)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1504)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A6A0'
	ldy #166
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1509)
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
.elseif(n = 1513)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1514)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1523)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A8A0'
	ldy #168
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 1524 to 1562
.if((n .mod 7 = 5) && ((n > 1523 && n < 1560)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1525 || n = 1534 || n = 1555 || n = 1562)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1526 || n = 1540 || n = 1544 || n = 1546 || n = 1547 || n = 1553 || n = 1554 || n = 1561)
	; 11..13 bytes
	delay_n (n-2)
	nop
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
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1529)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1530)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	tax
	pla
.elseif(n = 1532 || n = 1556)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1532 && n < 1561)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1535)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1536 || n = 1548)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1537)
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
.elseif(n = 1539)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $75A0'
	ldy #117
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 1541)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1543 || n = 1557)
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
.elseif(n = 1549)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1550)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1558)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ACA0'
	ldy #172
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1568 || n = 1570 || n = 1575 || n = 1582 || n = 1589 || n = 1591 || n = 1603 || n = 1610 || n = 1616 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1633 || n = 1638)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564 || n = 1588 || n = 1604 || n = 1612 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
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
.elseif(n = 1567)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ADA0'
	ldy #173
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1571 || n = 1592 || n = 1599 || n = 1606 || n = 1627)
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
.elseif(n = 1576)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1577)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1579 || n = 1593 || n = 1600 || n = 1635)
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
.elseif(n = 1581 || n = 1583 || n = 1590 || n = 1597 || n = 1611 || n = 1618 || n = 1625 || n = 1637)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1585)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1586)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1595)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B0A0'
	ldy #176
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1613)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
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
.elseif(n = 1630)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B4A0'
	ldy #180
	dey
	Jne *-4
	tay
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
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641 || n = 1655)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1642 || n = 1672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1679)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1644 || n = 1656)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1645 || n = 1652 || n = 1670 || n = 1673 || n = 1679)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1651 || n = 1669 || n = 1674)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1649)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B6A0'
	ldy #182
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1658)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1659 || n = 1668 || n = 1677)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1660 || n = 1676)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1662)
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
.elseif(n = 1663)
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
.elseif(n = 1665)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
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
	bit <$2C ;hides 'bit $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1675)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 1680 to 1718
.if(n = 1680 || n = 1716)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1681 || n = 1688 || n = 1707 || n = 1709)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1682)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1683 || n = 1690 || n = 1697 || n = 1718)
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
.elseif(n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1684 && n < 1714)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1686 || n = 1695 || n = 1704)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1708 || n = 1710 || n = 1714 || n = 1715 || n = 1717)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1691 || n = 1698 || n = 1705)
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
.elseif(n = 1702)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1703)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1711)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1712)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719)
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
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1756)))
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
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1722 || n = 1731 || n = 1740)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1723 || n = 1735 || n = 1737 || n = 1744 || n = 1749)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1724 || n = 1732 || n = 1756)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1725)
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
.elseif(n = 1726 || n = 1729 || n = 1733 || n = 1736 || n = 1742 || n = 1743 || n = 1745 || n = 1750 || n = 1754 || n = 1757)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1728 || n = 1752)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1730)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BFA0'
	ldy #191
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1738)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1739)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1746)
	; 9 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1747)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	iny
	Jne *-4
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
.elseif(n = 1753)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 1758 to 1796
.if(n = 1758 || n = 1767 || n = 1785 || n = 1794)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1759)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1760 || n = 1795)
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
.elseif((n .mod 7 = 5) && ((n > 1761 && n < 1791)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 1764 || n = 1788)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1765 || n = 1773 || n = 1778 || n = 1781 || n = 1792)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1766)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1768 || n = 1789)
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
.elseif(n = 1770)
	; 12 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1772 || n = 1780 || n = 1796)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1774)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1775)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1777)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 1782 || n = 1786)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1784)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1793)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
	pla
.endif
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
.elseif(n = 1798 || n = 1799 || n = 1806 || n = 1814 || n = 1823 || n = 1826 || n = 1831)
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
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	iny
	Jne *-4
	tay
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
.elseif(n = 1807 || n = 1813 || n = 1833)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1808 || n = 1817)
	; 12 bytes
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
.elseif(n = 1811)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1815)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A4A0'
	ldy #164
	nop
	dey
	Jne *-5
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
.elseif(n = 1819)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1820 || n = 1828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1822)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1825)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1829)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
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
.elseif(n = 1838)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $35A0'
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
.elseif(n = 1840 || n = 1851 || n = 1863)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841 || n = 1854 || n = 1858 || n = 1859 || n = 1861 || n = 1862 || n = 1867 || n = 1870)
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
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 1844 || n = 1852 || n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1845)
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
.elseif(n = 1846)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $34A0'
	ldy #52
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1847)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	dey
	Jne *-4
	tay
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
.elseif(n = 1855)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CDA0'
	ldy #205
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1856)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1864)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1865)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1869)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A9A0'
	ldy #169
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1873)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1874)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	iny
	Jne *-4
	tay
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
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1877 || n = 1878 || n = 1885 || n = 1886 || n = 1890 || n = 1894 || n = 1895 || n = 1898 || n = 1903 || n = 1913)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jmi *-5
	tay
	pla
.elseif(n = 1880)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AAA0'
	ldy #170
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1881)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $AAA0'
	ldy #170
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1882)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1883)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	dey
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
.elseif(n = 1887 || n = 1905)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1889 || n = 1897)
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
.elseif(n = 1891)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	iny
	Jne *-4
	tay
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
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	iny
	Jne *-4
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
.elseif(n = 1906)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1907)
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
.elseif(n = 1909)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1910)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
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
	bit $24
	dey
	Jne *-3
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
.elseif(n = 1918)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2CA0'
	ldy #44
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1919)
	; 11 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	tax
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
.elseif(n = 1921 || n = 1922 || n = 1931 || n = 1939 || n = 1949 || n = 1950)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1923 || n = 1941 || n = 1951)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1927)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	dey
	Jne *-4
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
.elseif(n = 1930)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1935)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1936)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1937)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1942 || n = 1952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1945)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1946)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1953)
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
.endif
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
.elseif(n = 1955)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
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
.elseif(n = 1957 || n = 1971 || n = 1977)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1958 || n = 1986)
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
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1961)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1962)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1963)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1964 || n = 1972 || n = 1988)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1966 || n = 1967 || n = 1975 || n = 1976 || n = 1984 || n = 1985)
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
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1970)
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
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1981)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1982)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1989)
	; 13 bytes
	pha
	tya
	ldy #n/9-2
	delay_n 4
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1990)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1991)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 1994 || n = 1998 || n = 2006 || n = 2011 || n = 2021 || n = 2029 || n = 2030)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995 || n = 2015 || n = 2031)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1996 || n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1997)
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
.elseif(n = 1999)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2000)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
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
.elseif(n = 2002 || n = 2022)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
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
.elseif(n = 2007)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2008)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2009)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2013)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B6A0'
	ldy #182
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2017)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2018)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2023)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2024)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2026)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2027)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
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
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2035)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2036 || n = 2044)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
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
.elseif(n = 2038 || n = 2039 || n = 2048 || n = 2050 || n = 2057 || n = 2066)
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
	delay_n (n-7)
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
.elseif(n = 2045)
	; 11 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2047 || n = 2058 || n = 2069)
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
.elseif(n = 2049)
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
.elseif(n = 2053)
	; 11 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2054)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2060)
	; 11 bytes
	pha
	tya
	ldy #0
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2061)
	; 13 bytes
	pha
	tya
	ldy #n/9-2
	delay_n 4
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2062)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2063)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	dey
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
	tya
	ldy #<$2C ;hides 'bit $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2068)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 2071 to 2109
.if(n = 2071)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2072)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
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
.elseif(n = 2074 || n = 2075 || n = 2077 || n = 2078 || n = 2083 || n = 2093)
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
.elseif(n = 2079)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2080)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2081)
	; 11 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	tax
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
.elseif(n = 2085 || n = 2092 || n = 2094 || n = 2101)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	tax
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
.elseif(n = 2096 || n = 2105)
	; 12 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 2099)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	dey
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
.elseif(n = 2107)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2108)
	; 11 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	tax
	pla
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2111 || n = 2114 || n = 2120 || n = 2128 || n = 2129 || n = 2137 || n = 2138 || n = 2147)
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
.elseif(n = 2113 || n = 2146)
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
.elseif(n = 2115 || n = 2119 || n = 2130 || n = 2139)
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
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	iny
	Jne *-4
	tay
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
.elseif(n = 2122 || n = 2142)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2123 || n = 2131)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2125)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2126)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	tay
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
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2134)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2135)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	iny
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
.elseif(n = 2143)
	; 11 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2144)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2155 || n = 2156 || n = 2164 || n = 2165 || n = 2173 || n = 2174 || n = 2176 || n = 2183 || n = 2185 || n = 2186)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2151 || n = 2175 || n = 2187)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2152)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2153)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
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
.elseif(n = 2158)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	bit $24
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2159)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
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
.elseif(n = 2161)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2162)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2166)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2167)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2169)
	; 13 bytes
	pha
	tya
	ldy #n/9-2
	delay_n 4
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2170)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2171)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2177)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3BA0'
	ldy #59
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2178)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2179)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2180)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2182)
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
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188)
	; 11 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2189)
	; 11 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
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
.elseif(n = 2191 || n = 2192 || n = 2203 || n = 2209 || n = 2213 || n = 2219 || n = 2222)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193 || n = 2204 || n = 2214)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2194)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jpl *-5
	tay
	pla
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
.elseif(n = 2197)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2198)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2200)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	nop
	iny
	Jne *-5
	tay
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
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2206)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2207)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	iny
	Jne *-4
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
.elseif(n = 2215)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2216)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2218)
	; 12 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	bit $24
	dey
	Jne *-5
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
.elseif(n = 2224)
	; 11 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2225)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2228 || n = 2230 || n = 2236 || n = 2237 || n = 2246 || n = 2254 || n = 2255 || n = 2258 || n = 2263 || n = 2264)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229 || n = 2245 || n = 2259)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2233)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2234)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
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
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2249 || n = 2250)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2251)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2252)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2257)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jmi *-5
	tay
	pla
.elseif(n = 2260)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2261)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2265)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CDA0'
	ldy #205
	nop
	dey
	Jne *-5
	tay
	pla
.endif
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
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2270)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
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
.elseif(n = 2272 || n = 2302)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2273 || n = 2281 || n = 2282 || n = 2284 || n = 2290 || n = 2291 || n = 2293 || n = 2299)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2274 || n = 2283 || n = 2295 || n = 2301)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2275)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2276)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2277)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2278)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2279)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 2287)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2288)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2296)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2297)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
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
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2306)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
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
.elseif(n = 2310)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	nop
	dey
	Jne *-5
	tay
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
	bit $24
	dey
	Jne *-4
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
.elseif(n = 2314)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2315)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 2320)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2321)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2323)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 2324 to 2343
.if(n = 2324 || n = 2341)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2325)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	delay_n 4
	dey
	Jne *-6
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
.elseif(n = 2327)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	bit $24
	iny
	Jne *-6
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
.elseif(n = 2329)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2330)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	bit $24
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2331)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2DA0'
	ldy #45
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2332)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
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
.elseif(n = 2338 || n = 2342)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2339)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
	pla
.elseif(n = 2343)
	; 12 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	tax
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
.elseif(n = 2345 || n = 2369 || n = 2381)
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
.elseif(n = 2346 || n = 2354 || n = 2356 || n = 2374)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2359 || n = 2371)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2348)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2349 || n = 2368)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2350)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2351)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	bit $24
	iny
	Jne *-6
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
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	nop
	dey
	Jne *-5
	tay
	pla
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
.elseif((n .mod 6 = 3) && ((n > 2360 && n < 2380)))
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2362 || n = 2372 || n = 2382)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2363)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2365)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	nop
	dey
	Jne *-5
	tay
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
.elseif(n = 2375)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	nop
	iny
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
.elseif(n = 2380)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2395 || n = 2407 || n = 2413)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2384 || n = 2390 || n = 2394 || n = 2401 || n = 2402 || n = 2404 || n = 2414 || n = 2416)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2385)
	; 13 bytes
	pha
	tya
	ldy #(n-33)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
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
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	nop
	iny
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
.elseif(n = 2389 || n = 2391 || n = 2403 || n = 2415)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2396)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2397)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2398)
	; 12 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2399 || n = 2410 || n = 2421)
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
.elseif(n = 2408)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2409)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2411)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2418)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2419)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	nop
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2452)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2423 || n = 2426 || n = 2428 || n = 2438 || n = 2440 || n = 2450)
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
.elseif(n = 2425 || n = 2444)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2427 || n = 2433 || n = 2439 || n = 2445 || n = 2449 || n = 2451)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2429)
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
	pha
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2437 || n = 2453 || n = 2455)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2441)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	nop
	dey
	Jne *-5
	tay
	pla
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
	pha
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	bit $24
	iny
	Jne *-6
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
.elseif(n = 2462)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2463)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2464)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2466)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2467 || n = 2483 || n = 2491)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2468)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	delay_n 4
	dey
	Jne *-6
	tay
	pla
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
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CDA0'
	ldy #205
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2471)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	bit $24
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 2474 || n = 2480 || n = 2486 || n = 2488)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2475)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
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
.elseif(n = 2479 || n = 2481 || n = 2493 || n = 2499)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2482 || n = 2492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2485)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2490)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	bit $24
	dey
	Jne *-4
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
	; 12 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2500)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.if(n = 2501 || n = 2539)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2502 || n = 2512)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2503 || n = 2515 || n = 2527)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2504 || n = 2518)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2505 || n = 2517 || n = 2523 || n = 2535)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2506 || n = 2510 || n = 2516 || n = 2522 || n = 2524 || n = 2533 || n = 2534 || n = 2536)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2507)
	; 12 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2508 || n = 2520 || n = 2532)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2509 || n = 2531)
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
.elseif(n = 2511 || n = 2528)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2513 || n = 2525 || n = 2537)
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
.elseif(n = 2514)
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
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	nop
	dey
	Jne *-5
	tay
	pla
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
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2529)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2530)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
	pla
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
.endif
.else
; 2540 to 2578
.if(n = 2540)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2541)
	; 12 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	tax
	pla
.elseif((n .mod 11 = 1) && ((n > 2541 && n < 2576)))
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
.elseif(n = 2543)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2544 || n = 2556 || n = 2568)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2545)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2546 || n = 2560 || n = 2574)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2547 || n = 2554 || n = 2559 || n = 2565 || n = 2571)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2548)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 2552)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2555 || n = 2558 || n = 2566 || n = 2570 || n = 2576)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2557)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	sta (@ptrtemp,x)
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
.elseif(n = 2563)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	nop
	dey
	Jne *-5
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
.elseif(n = 2569)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2572)
	; 12 bytes
	pha
	tya
	ldy #0
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2573)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2577)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2578)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	bit $24
	iny
	Jne *-6
	tay
	pla
.endif
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
.elseif(n = 2581 || n = 2582 || n = 2590 || n = 2594 || n = 2606)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2587 || n = 2589 || n = 2600)
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
.elseif(n = 2585)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	nop
	iny
	Jne *-5
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
.elseif(n = 2588 || n = 2602)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2591 || n = 2593 || n = 2599 || n = 2611)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2595)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2596)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
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
.elseif(n = 2598)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 2603)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2605)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2607)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	nop
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2612)
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
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	tay
	pla
.elseif(n = 2614)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2615)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2617)
	; 12 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	nop
	inx
	Jne *-5
	tax
	pla
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2620 || n = 2624 || n = 2642 || n = 2646 || n = 2654 || n = 2656)
	; 12..13 bytes
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
.elseif(n = 2623 || n = 2635 || n = 2648)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2625 || n = 2631 || n = 2643 || n = 2655)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2626)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2627)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	bit $24
	dey
	Jne *-6
	tay
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
	bit <$2C ;hides 'bit $12A0'
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
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2634 || n = 2653)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2638)
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
.elseif(n = 2647)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2650)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
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
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657 || n = 2669 || n = 2693)
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
.elseif(n = 2658 || n = 2694)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2659 || n = 2671)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2660 || n = 2665 || n = 2666 || n = 2668 || n = 2678 || n = 2680 || n = 2687 || n = 2690)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2661)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2662)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2663 || n = 2674 || n = 2685)
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
.elseif(n = 2664 || n = 2676 || n = 2688)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2667 || n = 2679)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2670)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2672 || n = 2686)
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
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2675)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2677)
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
.elseif(n = 2681)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2682)
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
.elseif(n = 2683)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2684)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2689)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2691)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2692)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2695)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 2696 to 2734
.if((n .mod 11 = 1) && ((n > 2695 && n < 2730)))
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
.elseif(n = 2697 || n = 2703 || n = 2733)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2698 || n = 2715 || n = 2732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2699)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2700 || n = 2712 || n = 2724)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2701)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2702 || n = 2704 || n = 2708 || n = 2709 || n = 2720 || n = 2726)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2705)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2706)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2710)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2711)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2713 || n = 2725)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2714 || n = 2728)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2716)
	; 12 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2717)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2719 || n = 2731)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2721)
	; 13 bytes
	pha
	tya
	ldy #(n-33)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2722)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2723)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	bit $24
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2727)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2730)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2734)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	bit $24
	inx
	Jne *-6
	tax
	pla
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	bit $24
	iny
	Jne *-6
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
.elseif(n = 2737 || n = 2743 || n = 2745 || n = 2755)
	; 13 bytes
	php
	delay_n (n-7)
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
.elseif(n = 2739)
	; 12 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	tax
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
.elseif(n = 2741 || n = 2753 || n = 2763 || n = 2769)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2742 || n = 2756 || n = 2770)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2744 || n = 2758 || n = 2768)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2746)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2747)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2749)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
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
	jsr @nearby_rts_15cyc
	Jne *-4
	tay
	pla
.elseif(n = 2752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2754)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2757)
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
.elseif(n = 2759)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2761)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2764)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2767)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2771)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2776 || n = 2785 || n = 2786 || n = 2788 || n = 2797)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 3) && ((n > 2774 && n < 2812)))
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2778 || n = 2790)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2779 || n = 2791 || n = 2803 || n = 2809)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2780)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2781 || n = 2807)
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
.elseif(n = 2782)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	bit $24
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 2783)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
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
.elseif(n = 2792)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2793)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2794)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	bit $24
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2795 || n = 2806)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
	ldy #<$2C ;hides 'bit $02A0'
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
	bit <$2C ;hides 'bit $02A0'
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
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2823 || n = 2835 || n = 2841 || n = 2847)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2829 || n = 2830 || n = 2834 || n = 2836 || n = 2842 || n = 2846 || n = 2848)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
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
	bit <$2C ;hides 'bit $01A0'
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
.elseif(n = 2818)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	bit $24
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2820 || n = 2832 || n = 2844)
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
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2824 || n = 2843)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2825 || n = 2837 || n = 2849)
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
.elseif(n = 2826 || n = 2840)
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
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 2831)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	bit $24
	iny
	Jne *-6
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
.elseif(n = 2838)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2839)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2845)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2850)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2851)
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
.else
; 2852 to 2890
.if(n = 2852)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
	pla
.elseif(n = 2853 || n = 2858 || n = 2860 || n = 2864 || n = 2870 || n = 2876)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854 || n = 2882)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2855 || n = 2859 || n = 2871 || n = 2877 || n = 2883 || n = 2888)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2856 || n = 2868 || n = 2880)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2857)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2861 || n = 2873)
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
.elseif(n = 2862 || n = 2881)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2863 || n = 2875)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2865 || n = 2869)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2866)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2867)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	bit $24
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 2874)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2878)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2879)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2884)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2885)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2886)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2887)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tay
	pla
.elseif(n = 2889)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2890)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	bit $24
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2891 && n < 2929)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2893 || n = 2905 || n = 2929)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2894 || n = 2906 || n = 2908 || n = 2918 || n = 2920 || n = 2925 || n = 2926)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895 || n = 2907 || n = 2913)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2896 || n = 2924)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2897 || n = 2909 || n = 2921)
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
.elseif(n = 2898)
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
.elseif(n = 2899 || n = 2911 || n = 2917 || n = 2923)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2900)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2902 || n = 2919)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2903)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2910)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2914)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	bit $24
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2915)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	bit $24
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2922)
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
.elseif(n = 2927)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	bit $24
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2942 || n = 2944 || n = 2948 || n = 2954 || n = 2956)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2931 || n = 2943 || n = 2949 || n = 2955 || n = 2967)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2933 || n = 2945)
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
.elseif(n = 2934 || n = 2958)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2935 || n = 2947 || n = 2959)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2936 || n = 2953)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2937)
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
.elseif(n = 2938 || n = 2966)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2939)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2940 || n = 2952 || n = 2964)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2941 || n = 2965)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2946)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2950 || n = 2963)
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
.elseif(n = 2951)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2957)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2961)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 4
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2962)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	bit $24
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2968)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if((n .mod 12 = 5) && ((n > 2968 && n < 3006)))
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
.elseif(n = 2970 || n = 2987 || n = 3004)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2971 || n = 2983 || n = 3007)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2972 || n = 2992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2973)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2974)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2975)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2976 || n = 2988 || n = 3000)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2977)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif((n .mod 6 = 2) && ((n > 2977 && n < 3003)))
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2979 || n = 2985 || n = 2991 || n = 3003)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2980 || n = 2994)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2982)
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
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	bit $24
	dey
	Jne *-5
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
.elseif(n = 2995)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2997)
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
.elseif(n = 2998)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2999)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3001)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 4
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3006)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3022)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3009 || n = 3010 || n = 3014 || n = 3016 || n = 3020 || n = 3026 || n = 3028 || n = 3038 || n = 3040 || n = 3044 || n = 3045)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3011)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3012 || n = 3024 || n = 3036)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 3015 || n = 3027 || n = 3037 || n = 3039)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3017 || n = 3029 || n = 3041)
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
.elseif(n = 3018)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3019 || n = 3025 || n = 3031 || n = 3043)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3021)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3023)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3030)
	; 13 bytes
	pha
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3033)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3035)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3042)
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
.elseif(n = 3046)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	bit $24
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	bit $24
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3049 || n = 3085)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3050 || n = 3064 || n = 3078)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3051 || n = 3057 || n = 3063 || n = 3075)
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
.elseif(n = 3054)
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
.elseif(n = 3056 || n = 3062 || n = 3066 || n = 3073 || n = 3074 || n = 3076)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3058)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3059)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3061 || n = 3067 || n = 3079)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3068)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3069)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $75A0'
	ldy #117
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3070)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3071)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3080)
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
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3083)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	bit $24
	dey
	Jne *-6
	tay
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
.else
; 3086 to 3125
.if(n = 3086 || n = 3088 || n = 3094 || n = 3101 || n = 3107 || n = 3110 || n = 3122 || n = 3124 || n = 3125)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087 || n = 3093 || n = 3111)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3090 || n = 3108)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3091 || n = 3097)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3092 || n = 3106 || n = 3120)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3095 || n = 3113)
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
.elseif(n = 3096)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3098)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3099)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3100 || n = 3116)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3103 || n = 3121)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3104)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3105)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 4
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3109)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3114)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3EA0'
	ldy #62
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3115)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3118)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3119)
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
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
.if((n >= 3126) && (n < 3165))
; 3126 to 3164
.if(n = 3126 || n = 3144 || n = 3162)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3127 || n = 3133 || n = 3151)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3128 || n = 3147)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3129 || n = 3155 || n = 3160)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3130)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3131)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3132 || n = 3152)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3134 || n = 3148)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3135)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $72A0'
	ldy #114
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3136 || n = 3142 || n = 3146 || n = 3149 || n = 3150 || n = 3154 || n = 3159 || n = 3164)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3137)
	; 13 bytes
	pha
	tya
	ldy #124
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3138)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3139)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3140 || n = 3157)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3141)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3143)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3145 || n = 3158)
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
.elseif(n = 3153)
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
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3161)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	tay
	pla
.elseif(n = 3163)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3165 to 3203
.if(n = 3165 || n = 3183 || n = 3188 || n = 3201)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3166 || n = 3185)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3167 || n = 3203)
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
.elseif(n = 3168 || n = 3173 || n = 3178 || n = 3182 || n = 3187 || n = 3193 || n = 3199 || n = 3200)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3169)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3170)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3171 || n = 3184 || n = 3197)
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
.elseif(n = 3172 || n = 3192)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3174 || n = 3191)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3175)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3176 || n = 3190)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3179)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3180 || n = 3198)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3181)
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
.elseif(n = 3186)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3189)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	nop
	bit $24
	iny
	Jne *-7
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
.elseif(n = 3195)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 4
	dey
	Jne *-6
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
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3218 || n = 3232)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3205 || n = 3217 || n = 3241)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3206 || n = 3214 || n = 3220 || n = 3230 || n = 3236 || n = 3238)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3207 || n = 3222)
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
.elseif(n = 3208 || n = 3225 || n = 3242)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3209)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 4
	iny
	Jne *-6
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
.elseif(n = 3211)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3213 || n = 3215 || n = 3219 || n = 3237)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3216 || n = 3234)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3221)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3223)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3224)
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
.elseif(n = 3226)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3227)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3228)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3229)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3231)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	bit $24
	inx
	Jne *-7
	tax
	pla
.elseif(n = 3233)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B3A0'
	ldy #179
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3235)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3239)
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
.elseif(n = 3240)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 3243 to 3281
.if(n = 3243)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3244)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3245)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3246 || n = 3260 || n = 3274)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3247)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3248 || n = 3254 || n = 3256 || n = 3269 || n = 3278)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3249 || n = 3262 || n = 3275)
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
.elseif(n = 3251)
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
.elseif(n = 3252 || n = 3270)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3253 || n = 3265 || n = 3277 || n = 3281)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3255 || n = 3273 || n = 3279)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3257)
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
.elseif(n = 3259 || n = 3276)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3261 || n = 3280)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
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
.elseif(n = 3264)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3266)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $94A0'
	ldy #148
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3267)
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
.elseif(n = 3268)
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
.elseif(n = 3271)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3272)
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
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3297)
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
.elseif(n = 3283 || n = 3319)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3284 || n = 3290 || n = 3294 || n = 3295 || n = 3298 || n = 3304 || n = 3308 || n = 3320)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3285)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
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
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3288 || n = 3306)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3289 || n = 3291 || n = 3305 || n = 3309 || n = 3315)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3292 || n = 3312)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3293 || n = 3310)
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
.elseif(n = 3299 || n = 3318)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3300)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 4
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3302 || n = 3316)
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
	delay_n (n-7)
	plp
.elseif(n = 3307)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	delay_n 7
	dey
	Jne *-6
	tay
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
.elseif(n = 3313)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3317)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3335 || n = 3345 || n = 3359)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3322)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3323)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3324 || n = 3342)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3325)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
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
.elseif(n = 3327)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3328 || n = 3329 || n = 3341 || n = 3346 || n = 3353)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3330 || n = 3344 || n = 3358)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3331 || n = 3349)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3332 || n = 3352)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3333)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $69A0'
	ldy #105
	brk
	iny
	Jne *-6
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
.elseif(n = 3336 || n = 3350)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3337 || n = 3356)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3338)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3339)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
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
.elseif(n = 3343)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3347)
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
.elseif(n = 3348)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3351)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3354)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3355)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3357)
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
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3378 || n = 3396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3361 || n = 3395)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3362 || n = 3364 || n = 3374 || n = 3377 || n = 3380 || n = 3382 || n = 3397 || n = 3398)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3379 || n = 3381 || n = 3389)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3365 || n = 3383)
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
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3367 || n = 3368 || n = 3385)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3369)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $6EA0'
	ldy #110
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3370)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3371)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3372 || n = 3386)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3373)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3375 || n = 3394)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3387)
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
.elseif(n = 3388)
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
.elseif(n = 3393)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3417)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3400 || n = 3428)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3401 || n = 3437)
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
.elseif(n = 3402)
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
.elseif(n = 3403)
	; 13 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3404 || n = 3420 || n = 3436)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3405 || n = 3433)
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
.elseif(n = 3406 || n = 3415 || n = 3416 || n = 3424 || n = 3430 || n = 3431 || n = 3434)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3407 || n = 3421)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3408)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3409 || n = 3427)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3412 || n = 3429)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3413)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3414 || n = 3432)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3418)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3419)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 7
	iny
	Jne *-6
	tay
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
.elseif(n = 3426)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	bit $24
	iny
	Jne *-7
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
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3439 || n = 3457)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3440)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	bit $24
	dey
	Jne *-7
	tay
	pla
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
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $64A0'
	ldy #100
	brk
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3449 || n = 3453 || n = 3459 || n = 3464 || n = 3469 || n = 3471)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3467)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	delay_n 7
	iny
	Jne *-6
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
	pha
	tya
	ldy #(n-18)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 3478)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3479 || n = 3488 || n = 3491 || n = 3494 || n = 3510)
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
.elseif(n = 3483)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3485 || n = 3501 || n = 3507)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3489 || n = 3508)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3493 || n = 3505 || n = 3511 || n = 3513)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3496)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3499)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 7
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 3509)
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
.elseif(n = 3515)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3517 || n = 3535 || n = 3553)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3518 || n = 3524 || n = 3528 || n = 3529 || n = 3533 || n = 3534 || n = 3539 || n = 3542 || n = 3544 || n = 3550)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3521)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3522 || n = 3540)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3523 || n = 3525 || n = 3543 || n = 3549 || n = 3551)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3526 || n = 3554)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3527 || n = 3546)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3530)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3531 || n = 3548)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3532 || n = 3552)
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
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3537)
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
.elseif(n = 3538)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	nop
	bit $24
	iny
	Jne *-7
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
.elseif(n = 3545)
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
.elseif(n = 3547)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3557 || n = 3561 || n = 3579 || n = 3585)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3556 || n = 3560 || n = 3566 || n = 3569 || n = 3570 || n = 3574 || n = 3578 || n = 3586)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3558 || n = 3576)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3562)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3563)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3564 || n = 3580)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3565)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3567)
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
.elseif(n = 3568 || n = 3582)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3571 || n = 3589)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3572 || n = 3592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3575)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A2A0'
	ldy #162
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 3581)
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
.elseif(n = 3583 || n = 3591)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3584)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
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
	ldy #(n-18)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
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
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3612 || n = 3630)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3595)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3597 || n = 3602 || n = 3606 || n = 3615 || n = 3631)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3598 || n = 3601 || n = 3605 || n = 3613 || n = 3614 || n = 3618 || n = 3629)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599 || n = 3616)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3600)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3603 || n = 3622)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3604 || n = 3621)
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
.elseif(n = 3607 || n = 3625)
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
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.elseif(n = 3611)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3617)
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
.elseif(n = 3619 || n = 3623)
	; 13..14 bytes
	php
	delay_n (n-7)
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
.elseif(n = 3624)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3626)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3627)
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
.elseif(n = 3628)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3632)
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
.else
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633 || n = 3650)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3634 || n = 3635 || n = 3646)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3637)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3638)
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
.elseif(n = 3640)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3641)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3642)
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
.elseif(n = 3643)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3644)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3645)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3647)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3648)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3649)
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
.elseif(n = 3651)
	; 12 bytes
	pha
	delay_n (n-3)
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3653)
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
.elseif(n = 3654 || n = 3662 || n = 3668)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3655 || n = 3671)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3656)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3657)
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
.elseif(n = 3658)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3659)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3660)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3661)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3663)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A6A0'
	ldy #166
	brk
	dey
	Jne *-6
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
	pha
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3666)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3667 || n = 3669)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3670)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	sta (@ptrtemp,x)
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3692)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3673 || n = 3697 || n = 3699)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3674)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3675)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3676 || n = 3708)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3677)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3678)
	; 13 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3679 || n = 3698)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3680)
	; 13 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3681 || n = 3686 || n = 3688 || n = 3694 || n = 3700 || n = 3703 || n = 3704 || n = 3710)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	pha
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3684 || n = 3702)
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
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3687 || n = 3693 || n = 3705)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3689)
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
.elseif(n = 3691)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
	tax
	pla
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
.elseif(n = 3696)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3701)
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
.elseif(n = 3707)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
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
.endif
.else
; 3711 to 3750
.if(n = 3711)
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
.elseif(n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3713 || n = 3727 || n = 3745)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3714 || n = 3719 || n = 3737 || n = 3740 || n = 3742 || n = 3748 || n = 3749)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3715 || n = 3733)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3716)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3717 || n = 3736)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3718 || n = 3735)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3720 || n = 3738)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3721 || n = 3723 || n = 3741)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3724)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3725 || n = 3743)
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
.elseif(n = 3726 || n = 3744)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3728)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A9A0'
	ldy #169
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3729)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
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
.elseif(n = 3731)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	nop
	inx
	Jne *-7
	tax
	pla
.elseif(n = 3734)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3739)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3746)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3747)
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
.elseif(n = 3750)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.endif
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
.if((n >= 3751) && (n < 3790))
; 3751 to 3789
.if(n = 3751)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3752 || n = 3769 || n = 3786)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3753 || n = 3757 || n = 3758 || n = 3760 || n = 3764 || n = 3776 || n = 3778)
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
.elseif(n = 3755)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3756 || n = 3774)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3759 || n = 3766 || n = 3775 || n = 3777 || n = 3789)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3761 || n = 3779)
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
.elseif(n = 3762)
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
.elseif(n = 3763 || n = 3781)
	; 13 bytes
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
.elseif(n = 3767)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3768)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3770)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3771)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3772)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3773)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $ABA0'
	ldy #171
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3780)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3782)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 3784)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3785)
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
.elseif(n = 3787)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3788)
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
; 3790 to 3828
.if(n = 3790 || n = 3794 || n = 3801 || n = 3805 || n = 3809 || n = 3814 || n = 3824)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3792 || n = 3810 || n = 3828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3793)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3795 || n = 3806 || n = 3813)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3796)
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
.elseif(n = 3797 || n = 3815)
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
.elseif(n = 3798 || n = 3816)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3799 || n = 3800 || n = 3817)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3802)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3803 || n = 3820)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3804)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3807 || n = 3822)
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
.elseif(n = 3808)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2DA0'
	ldy #45
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3811)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B5A0'
	ldy #181
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3818)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3819)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3821)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3823)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3826)
	; 14 bytes
	pha
	tya
	ldy #44 ;hides 'bit abs'
	bit $EAEA ;hides 'nop'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3827)
	; 14 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A6A0'
	ldy #166
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3830 || n = 3833 || n = 3839)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3834)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3835)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3836)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3837)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3838)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3840)
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
.elseif(n = 3841)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3842)
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
.elseif(n = 3843)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3844)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3845)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3846)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3847)
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
; 3848 to 3867
.if(n = 3848 || n = 3856 || n = 3858 || n = 3866)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3849 || n = 3857)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3850)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3851)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3852)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3853)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3854)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3855)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3859)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3860)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3861)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3862)
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
.elseif(n = 3863)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3864)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3865)
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
.elseif(n = 3867)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
	pla
.endif
.endif
.else
; 3868 to 3906
.if(n = 3868)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3869)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3870 || n = 3875 || n = 3885 || n = 3891 || n = 3903)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3871 || n = 3888 || n = 3905)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3872 || n = 3892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3873 || n = 3884 || n = 3886 || n = 3890 || n = 3894 || n = 3901 || n = 3902 || n = 3904)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3874)
	; 12 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3876 || n = 3897)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3877 || n = 3880 || n = 3895)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3878 || n = 3879 || n = 3889)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3881)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3882 || n = 3900)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3883)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3887)
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
.elseif(n = 3893)
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
.elseif(n = 3896)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B9A0'
	ldy #185
	jsr @nearby_rts
	dey
	Jne *-7
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
.elseif(n = 3899)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3906)
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
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907 || n = 3926 || n = 3945)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3908 || n = 3910 || n = 3921 || n = 3929 || n = 3935)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3909 || n = 3920 || n = 3924 || n = 3933 || n = 3934 || n = 3938 || n = 3940 || n = 3941)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3911)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3912 || n = 3932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3913)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3914)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3915)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3916)
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
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3918 || n = 3936)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3919 || n = 3925 || n = 3943)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3922 || n = 3939)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3923)
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
.elseif(n = 3927)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3928)
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
.elseif(n = 3930)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3931)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3937)
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
.elseif(n = 3942)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3944)
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
; 3946 to 3984
.if(n = 3946)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3947)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3950)
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
.elseif(n = 3951 || n = 3955 || n = 3957 || n = 3975)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3953)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3954 || n = 3972)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3956 || n = 3973)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3958 || n = 3965 || n = 3966 || n = 3968 || n = 3974 || n = 3976 || n = 3982)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3959 || n = 3977)
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
.elseif(n = 3960 || n = 3981)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3961)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3962)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3963)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3964 || n = 3983)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3967)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3969)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3970)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B4A0'
	ldy #180
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3971)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B4A0'
	ldy #180
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 3979)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3984)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985 || n = 3998 || n = 4004 || n = 4009 || n = 4014 || n = 4018)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3986 || n = 3993 || n = 4005 || n = 4011 || n = 4013)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3987)
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
.elseif(n = 3989)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3990 || n = 4008)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3991)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3992 || n = 4012)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3994)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3995)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3996)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3997 || n = 3999 || n = 4015 || n = 4017 || n = 4019)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4000)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4001)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4002 || n = 4021)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4003)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4006)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4007)
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
.elseif(n = 4016)
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
	pha
	tya
	bit <$2C ;hides 'bit $BFA0'
	ldy #191
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4023)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4041 || n = 4058)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4025)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4026 || n = 4044 || n = 4062)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4027)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4028 || n = 4030 || n = 4034 || n = 4045 || n = 4046 || n = 4048 || n = 4053 || n = 4054 || n = 4056 || n = 4061)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4029 || n = 4035 || n = 4047 || n = 4055)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4031 || n = 4049)
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
.elseif(n = 4032 || n = 4052)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4033 || n = 4051)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4036)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B7A0'
	ldy #183
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4037)
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
.elseif(n = 4038)
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
.elseif(n = 4039 || n = 4057)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4040 || n = 4059)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4042)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4043)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4050)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4060)
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
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063)
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
.elseif(n = 4064 || n = 4066 || n = 4071 || n = 4077 || n = 4082 || n = 4084 || n = 4088 || n = 4094 || n = 4099 || n = 4100)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065 || n = 4073 || n = 4083 || n = 4095 || n = 4101)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4067 || n = 4085)
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
.elseif(n = 4068)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4069 || n = 4087)
	; 13 bytes
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
.elseif(n = 4072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4074)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4075 || n = 4092)
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
.elseif(n = 4078 || n = 4097)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4079)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4080 || n = 4098)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4081)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B9A0'
	ldy #185
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4086)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4090)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4091)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4093)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4096)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4110 || n = 4111 || n = 4113 || n = 4118 || n = 4120 || n = 4128 || n = 4130 || n = 4136 || n = 4138)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4103 || n = 4121 || n = 4139)
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
.elseif(n = 4105 || n = 4123)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4106)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4107)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
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
.elseif(n = 4109 || n = 4126)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4112 || n = 4132)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 4115 || n = 4117 || n = 4119 || n = 4137)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4116 || n = 4134)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4122 || n = 4140)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4124)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BBA0'
	ldy #187
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4125)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BBA0'
	ldy #187
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4127)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	tax
	pla
.elseif(n = 4129)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4131)
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
.elseif(n = 4133)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4135)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4142 || n = 4159 || n = 4161 || n = 4167)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4143 || n = 4160 || n = 4177)
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
.elseif(n = 4145 || n = 4156 || n = 4162 || n = 4164 || n = 4171 || n = 4174 || n = 4175 || n = 4179)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BCA0'
	ldy #188
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4147 || n = 4165)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4148)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4150)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4151)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4152 || n = 4170)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4153)
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
.elseif(n = 4154 || n = 4173)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4155 || n = 4163)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4157)
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
.elseif(n = 4158 || n = 4176)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4166)
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
.elseif(n = 4169)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4172)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4178)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 4180 to 4218
.if(n = 4180 || n = 4191 || n = 4209 || n = 4215)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4181 || n = 4185 || n = 4190 || n = 4196 || n = 4198 || n = 4208 || n = 4213)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4183 || n = 4201 || n = 4207)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4184 || n = 4195 || n = 4218)
	; 13..14 bytes
	php
	delay_n (n-7)
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
.elseif(n = 4188 || n = 4206)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 4192 || n = 4212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4193)
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
.elseif(n = 4194 || n = 4211)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4197)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4200 || n = 4217)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4202)
	; 14 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4205)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4210)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4214)
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
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4237 || n = 4255)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4220)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4221)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4223)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4224 || n = 4242)
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
.elseif(n = 4226 || n = 4234 || n = 4238 || n = 4244 || n = 4246 || n = 4247 || n = 4251)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4227)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4228 || n = 4245)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4229)
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
.elseif(n = 4230 || n = 4249)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4231 || n = 4239)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4232 || n = 4252)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4233 || n = 4254)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4235)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C0A0'
	ldy #192
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4236)
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
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4241)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4243)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B8A0'
	ldy #184
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4248)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 4253)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4256)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	brk
	iny
	Jne *-6
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
.endif
.else
; 4258 to 4296
.if(n = 4258)
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
.elseif(n = 4260 || n = 4278 || n = 4296)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4261 || n = 4264 || n = 4270 || n = 4274 || n = 4280 || n = 4282 || n = 4289 || n = 4294)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4262 || n = 4279)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4263 || n = 4271 || n = 4276 || n = 4281 || n = 4295)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4265 || n = 4283)
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
.elseif(n = 4266 || n = 4284)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4267 || n = 4269 || n = 4285 || n = 4286)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4268 || n = 4287)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4272 || n = 4292)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4273)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tax
	pla
.elseif(n = 4275)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4277)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4288)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BAA0'
	ldy #186
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4291)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4293)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
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
.elseif(n = 4298 || n = 4300 || n = 4304 || n = 4308 || n = 4310 || n = 4315 || n = 4316 || n = 4318 || n = 4326 || n = 4327 || n = 4329 || n = 4334)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4317 || n = 4328 || n = 4333 || n = 4335)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4301 || n = 4319)
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
.elseif(n = 4302 || n = 4324)
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
.elseif(n = 4303 || n = 4321)
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
.elseif(n = 4306 || n = 4325)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4307)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B3A0'
	ldy #179
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4309)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4311)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4312)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4313)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4314 || n = 4332)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4320)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4322)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4323)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4331)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 4336 to 4375
.if(n = 4336 || n = 4340 || n = 4341 || n = 4349 || n = 4351 || n = 4354 || n = 4361 || n = 4365 || n = 4366 || n = 4370 || n = 4374)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4337 || n = 4355 || n = 4373)
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
.elseif(n = 4338 || n = 4359)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4339 || n = 4357 || n = 4375)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4342)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4343 || n = 4345 || n = 4360)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4344 || n = 4363)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4346)
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
.elseif(n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4348)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4350 || n = 4368)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4353 || n = 4371)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4356)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4358)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 4367)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	brk
	iny
	Jne *-6
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
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
.if((n >= 4376) && (n < 4415))
; 4376 to 4414
.if(n = 4376 || n = 4384 || n = 4388 || n = 4394 || n = 4396 || n = 4403 || n = 4405 || n = 4406 || n = 4408 || n = 4414)
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
.elseif(n = 4378)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4379)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4380)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4381 || n = 4399)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4382 || n = 4401)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 4385 || n = 4389 || n = 4395 || n = 4407 || n = 4413)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4386 || n = 4404)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4387)
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
.elseif(n = 4390)
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
.elseif(n = 4391)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4392 || n = 4412)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4393)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4398)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4400)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4402)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4409)
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
.elseif(n = 4411)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4421 || n = 4423 || n = 4425 || n = 4443)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4416 || n = 4424 || n = 4430 || n = 4441 || n = 4442 || n = 4444 || n = 4448)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4417 || n = 4435 || n = 4453)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 4419 || n = 4429 || n = 4447)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4420 || n = 4439)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4422 || n = 4440)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4426)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4427 || n = 4445)
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
.elseif(n = 4428 || n = 4446)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4431)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4432 || n = 4452)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4433)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4434)
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
.elseif(n = 4436)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4437)
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
.elseif(n = 4438)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4449)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4450)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4451)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4460 || n = 4462 || n = 4466 || n = 4478 || n = 4480 || n = 4491)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4455)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	brk
	dey
	Jne *-6
	tay
	pla
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
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4458 || n = 4476)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4459 || n = 4461 || n = 4468 || n = 4475 || n = 4479 || n = 4486)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4463 || n = 4481)
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
.elseif(n = 4464 || n = 4485)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4465 || n = 4483)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4467)
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
.elseif(n = 4469 || n = 4471 || n = 4489 || n = 4490)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4470)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	iny
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4473)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4482)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4484)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	jsr @nearby_rts
	iny
	Jne *-7
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
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4495 || n = 4497)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4496 || n = 4515)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4498 || n = 4502 || n = 4508 || n = 4509 || n = 4514 || n = 4516 || n = 4517)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4499)
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
.elseif(n = 4501 || n = 4503 || n = 4519)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4504)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4505)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4506 || n = 4527)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 4510)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4511)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4513)
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
.elseif(n = 4518)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4520)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CDA0'
	ldy #205
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4521)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4523)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 4525)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4526)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4528)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4529)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4552)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4533 || n = 4549 || n = 4551 || n = 4556 || n = 4569)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4534 || n = 4553)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4535)
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
.elseif(n = 4536 || n = 4545 || n = 4550 || n = 4554 || n = 4555 || n = 4563 || n = 4568 || n = 4570)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4537 || n = 4539 || n = 4541 || n = 4559 || n = 4560)
	; 13..14 bytes
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
.elseif(n = 4540)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4542)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4543 || n = 4561)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4544)
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
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4547)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4548 || n = 4566)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4557)
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
.elseif(n = 4558)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
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
.elseif(n = 4564)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4565)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4567)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 4571 to 4609
.if(n = 4571 || n = 4589 || n = 4607)
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
.elseif(n = 4572)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4573 || n = 4598)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4574)
	; 13 bytes
	pha
	tya
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
	nop
.elseif(n = 4575 || n = 4587 || n = 4594 || n = 4601 || n = 4605)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4576 || n = 4581 || n = 4586 || n = 4593 || n = 4595 || n = 4596 || n = 4599 || n = 4600 || n = 4604 || n = 4606)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4577)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4578)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4579)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4580)
	; 14 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4582)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4583)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4584 || n = 4602)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4585)
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
.elseif(n = 4588)
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
.elseif(n = 4590)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
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
.elseif(n = 4592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 4603)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4608)
	; 13 bytes
	pha
	tya
	ldy #n/18-1
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4629 || n = 4648)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4611 || n = 4613 || n = 4617 || n = 4618 || n = 4622 || n = 4624 || n = 4628 || n = 4634)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4619 || n = 4623 || n = 4635)
	; 11..13 bytes
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
.elseif(n = 4616 || n = 4627 || n = 4636 || n = 4639)
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
.elseif(n = 4626)
	; 13 bytes
	pha
	tya
	ldy #0
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4630)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	brk
	dey
	Jne *-6
	tay
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
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4640)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4641 || n = 4646 || n = 4647)
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
.elseif(n = 4642)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4643)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
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
.if((n >= 4649) && (n < 4668))
; 4649 to 4667
.if(n = 4649)
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
.elseif(n = 4651 || n = 4653 || n = 4660)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4652)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4654)
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
	pha
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4657 || n = 4659)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4658)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4661 || n = 4663)
	; 13..14 bytes
	brk
	delay_n (n-13)
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
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4667)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 4668 to 4687
.if(n = 4668)
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
.elseif(n = 4669 || n = 4671 || n = 4674 || n = 4678)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4670 || n = 4682)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4673)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4675)
	; 13 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 4676)
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
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
	pla
.elseif(n = 4680)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4681)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4684)
	; 14 bytes
	pha
	tya
	ldy #146
	delay_n 27
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
.elseif(n = 4686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
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
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4694 || n = 4700 || n = 4707 || n = 4709 || n = 4714 || n = 4721 || n = 4726)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4689 || n = 4693 || n = 4701 || n = 4708 || n = 4715 || n = 4723 || n = 4725)
	; 12..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4691 || n = 4710)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4692 || n = 4712)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4695 || n = 4716)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4696)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4697 || n = 4699)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4698 || n = 4720)
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
	pha
	tya
	ldy #<$2C ;hides 'bit $34A0'
	ldy #52
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4703)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4704)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4705 || n = 4724)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4706)
	; 14 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4711)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4713)
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
.elseif(n = 4717)
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
.elseif(n = 4718)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4719)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	brk
	dey
	Jne *-6
	tay
	pla
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
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4735 || n = 4746 || n = 4761 || n = 4765)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4728)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4729 || n = 4731 || n = 4739 || n = 4750 || n = 4759)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4730 || n = 4749)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4732 || n = 4752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 4734 || n = 4744 || n = 4745 || n = 4747 || n = 4754 || n = 4760)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4736)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4737 || n = 4758)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4738)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4740)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4741)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4742 || n = 4764)
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
.elseif(n = 4743 || n = 4762)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4748)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4751 || n = 4753)
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
.elseif(n = 4756)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4757)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4763)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4774 || n = 4776 || n = 4783 || n = 4789 || n = 4794 || n = 4802 || n = 4804)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4768)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4769)
	; 14 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4770)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4771)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4772 || n = 4792)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4773 || n = 4796)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4775 || n = 4790 || n = 4795 || n = 4803)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4777)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4778)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4779)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4780)
	; 14 bytes
	pha
	tya
	ldy #149
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4781 || n = 4800)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 4784)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4785)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4786)
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
.elseif(n = 4787)
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
.elseif(n = 4788)
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
.elseif(n = 4791)
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
.elseif(n = 4793 || n = 4801)
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
.elseif(n = 4797)
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
.elseif(n = 4798)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4799)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4824 || n = 4843)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4806)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4807)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4808 || n = 4830)
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
.elseif(n = 4809 || n = 4814 || n = 4816 || n = 4831 || n = 4834 || n = 4840)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4811)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4812 || n = 4832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4813)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4815 || n = 4822 || n = 4833 || n = 4835 || n = 4841)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4818)
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
.elseif(n = 4819 || n = 4838)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4820)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4821 || n = 4842)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4823)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4825)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4826 || n = 4839)
	; 14 bytes
	php
	delay_n (n-7)
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
.elseif(n = 4828)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4829)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4836)
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
.elseif(n = 4837)
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
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4853 || n = 4854 || n = 4856 || n = 4859 || n = 4878 || n = 4880)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845 || n = 4864)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4846)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4847 || n = 4849 || (n > 4866 && n < 4870) || n = 4871)
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
.elseif(n = 4848 || n = 4879)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4850)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4851)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4852 || n = 4872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4855 || n = 4860 || n = 4866 || n = 4875)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4857)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4858)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.elseif(n = 4862)
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
.elseif(n = 4863)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4865)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4870)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4873)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4874)
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
	; 12 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4877)
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
.elseif(n = 4881)
	; 14 bytes
	php
	pha
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4882)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4884 || n = 4905)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4885 || n = 4890 || n = 4913 || n = 4915 || n = 4921)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4886 || n = 4894 || n = 4898 || n = 4907 || n = 4914 || n = 4920)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4887)
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
.elseif(n = 4888 || n = 4911)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4889 || n = 4899 || n = 4919)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4891 || n = 4893 || n = 4901)
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
.elseif(n = 4892 || n = 4912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4895)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4896 || n = 4918)
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
.elseif(n = 4897)
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
.elseif(n = 4900)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4903)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4904)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4906)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $34A0'
	ldy #52
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4908)
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
.elseif(n = 4909)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4910)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4916)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4917)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922 || n = 4923 || n = 4951)
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
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4925)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4926 || n = 4947)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4927 || n = 4935 || n = 4944 || n = 4950 || n = 4955)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4928 || n = 4934 || n = 4936 || n = 4942 || n = 4943 || n = 4949 || n = 4954)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4929)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4930)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4931 || n = 4959)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4932 || n = 4952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4933)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4937)
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
.elseif(n = 4938)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4939)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4940)
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
.elseif(n = 4945)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4946)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 4956)
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
.elseif(n = 4957)
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
.elseif(n = 4958)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4960)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 4961 to 5000
.if(n = 4961)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4962 || n = 4984)
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
.elseif(n = 4963 || n = 4964 || n = 4974 || n = 4986 || n = 4991 || n = 4994 || n = 4996)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4965)
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
.elseif(n = 4966)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4967)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4968 || n = 4989)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4969 || n = 4993 || n = 5000)
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
.elseif(n = 4971 || n = 4973 || n = 4981 || n = 4990 || n = 4995)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 4976)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4977 || n = 4979 || n = 4999)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4978)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4980)
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
.elseif(n = 4982)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4983)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4985)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	nop
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4987)
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
.elseif(n = 4988)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4997)
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
.elseif(n = 4998)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.endif
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
.if((n >= 5001) && (n < 5040))
; 5001 to 5039
.if(n = 5001)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5002)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5003)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5004)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5005)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5006 || n = 5028)
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
.elseif(n = 5007)
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
.elseif(n = 5008)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5009)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5010 || n = 5031)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5011 || n = 5020 || n = 5021 || n = 5023)
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
.elseif(n = 5012 || n = 5032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5013 || n = 5015 || n = 5035)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5014 || n = 5016 || n = 5033 || n = 5034 || n = 5036)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5017)
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
.elseif(n = 5018 || n = 5038)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5019 || n = 5039)
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
.elseif(n = 5024)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5025)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5026)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5027)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5029)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5030)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5037)
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
; 5040 to 5078
.if(n = 5040 || n = 5053 || n = 5055 || n = 5063 || n = 5075)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif((n > 5040 && n < 5045) || n = 5046 || n = 5066 || n = 5069)
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
.elseif(n = 5045)
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
.elseif(n = 5047)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5048)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5049)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5050)
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
.elseif(n = 5051)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5052 || n = 5072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5054 || n = 5056 || n = 5064 || n = 5067 || n = 5074)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5057 || n = 5077)
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
.elseif(n = 5058 || n = 5078)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5059)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5060)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5061)
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
.elseif(n = 5062)
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
.elseif(n = 5065)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5068)
	; 14 bytes
	pha
	tya
	ldy #158
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5070)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5071)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5073)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5076)
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
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5099 || n = 5108)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5080 || n = 5081 || n = 5083 || n = 5084 || n = 5086 || n = 5102)
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
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
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
.elseif(n = 5087)
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
.elseif(n = 5088 || n = 5090 || n = 5095 || n = 5104 || n = 5111 || n = 5117)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5089 || n = 5103 || n = 5105 || n = 5106 || n = 5109 || n = 5110)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5091)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	sta (@ptrtemp,x)
	tax
	pla
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
.elseif(n = 5093)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5094 || n = 5115)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
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
.elseif(n = 5098)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5100)
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
.elseif(n = 5107)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	nop
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5113)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 5116)
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
; 5118 to 5156
.if(n = 5118 || n = 5122 || n = 5123 || n = 5129 || n = 5130 || n = 5134 || n = 5140)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119 || n = 5128 || n = 5139)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5120)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
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
.elseif(n = 5124 || n = 5131 || n = 5135)
	; 12..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 5133)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 5136)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5137)
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
.elseif(n = 5138)
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
.elseif(n = 5141)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
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
.elseif(n = 5143 || n = 5144 || (n > 5148 && n < 5155))
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
.elseif(n = 5146)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5147)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5148)
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
.elseif(n = 5155)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5156)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || n = 5178)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 5159)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5160 || n = 5182)
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
.elseif(n = 5161 || n = 5190 || n = 5192)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5162 || n = 5187)
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
.elseif(n = 5164)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5165)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 5165 && n < 5169) || (n > 5172 && n < 5176) || n = 5179 || n = 5188)
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
.elseif(n = 5169)
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
.elseif(n = 5170)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5171)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5172)
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
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5177)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5180)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5181)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5183 || n = 5184 || n = 5191)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5185)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5186)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5189)
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
.elseif(n = 5195)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 5196 to 5234
.if(n = 5196)
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
.elseif(n = 5197)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5198)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5199 || n = 5220)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5200 || n = 5211 || n = 5213 || n = 5217 || n = 5230 || n = 5234)
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
.elseif(n = 5201 || n = 5205 || n = 5206 || n = 5214 || n = 5222 || n = 5227 || n = 5228)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5202)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5203)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5204 || n = 5226)
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
.elseif(n = 5207 || n = 5215 || n = 5221 || n = 5229)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5208)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5209)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
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
.elseif(n = 5212)
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
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5219)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5223)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5224)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5225)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5231)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5232)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5233)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235 || n = 5236 || n = 5245 || n = 5252 || n = 5257 || n = 5258 || n = 5263)
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
.elseif(n = 5237)
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
.elseif(n = 5238 || n = 5264)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5239)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5240)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5241 || n = 5262)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5242)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5243 || n = 5249 || n = 5253 || n = 5271 || n = 5272)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5244)
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
.elseif(n = 5246)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5247)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5248 || n = 5270)
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
.elseif(n = 5250)
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
.elseif(n = 5251)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5254)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5255)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5256)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 5260)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5261)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5265 || n = 5273)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5266)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5267)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5268)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5269)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5274 to 5312
.if((n > 5273 && n < 5277) || (n > 5295 && n < 5300) || n = 5305 || n = 5308 || n = 5309 || n = 5311)
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
.elseif(n = 5277)
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
.elseif(n = 5278)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5279)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5280)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5281)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5282)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5283 || n = 5304)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5284 || n = 5285 || n = 5289 || n = 5293 || n = 5294 || n = 5306)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5286 || n = 5295 || n = 5307)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5287 || n = 5312)
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
.elseif(n = 5288)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
	pla
.elseif(n = 5290)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5291)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5292)
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
.elseif(n = 5300)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5301)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5302)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5303)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
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
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if(n = 5313 || n = 5344)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5314 || n = 5336)
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
.elseif(n = 5315)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5316 || n = 5340)
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
.elseif(n = 5317 || n = 5326 || n = 5328 || n = 5349)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n > 5317 && n < 5322) || n = 5329 || n = 5330 || n = 5332 || n = 5333 || n = 5341 || n = 5343 || n = 5350 || n = 5351)
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
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 5324)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5325 || n = 5346)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5327 || n = 5338)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5331)
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
.elseif(n = 5334)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5335)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5337)
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
.elseif(n = 5339)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5342)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5345)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5347)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5348)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5352 to 5390
.if(n = 5352)
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
.elseif((n > 5352 && n < 5356) || (n > 5371 && n < 5375))
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
.elseif(n = 5356)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5357)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5358 || n = 5380)
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
.elseif(n = 5359 || n = 5361)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5360 || n = 5369 || n = 5379 || n = 5381 || n = 5382 || n = 5389 || n = 5390)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5362 || n = 5387)
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
.elseif(n = 5363)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5364)
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
.elseif(n = 5365)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5366)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5367)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5368)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5370)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5371)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5375)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5376)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
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
	bit $24
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5378)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5383)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
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
.elseif(n = 5385)
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
.elseif(n = 5386)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5388)
	; 13 bytes
	pha
	tya
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391)
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
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5393)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5394 || n = 5417)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5395 || n = 5405 || n = 5409 || n = 5413 || n = 5427)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n > 5395 && n < 5400) || n = 5407 || n = 5418 || n = 5419 || n = 5421 || n = 5428 || n = 5429)
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
.elseif(n = 5400)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5401)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5402 || n = 5424)
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
.elseif(n = 5403 || n = 5404 || n = 5408 || n = 5414 || n = 5425 || n = 5426)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5406)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 5411)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5412)
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
.elseif(n = 5415)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5416)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5420)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5422)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5423)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 5430 to 5468
.if((n > 5429 && n < 5434) || n = 5441 || n = 5443 || n = 5450 || n = 5451 || (n > 5452 && n < 5457))
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
.elseif(n = 5434)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5435)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5436 || n = 5460)
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
.elseif(n = 5437)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5438)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5439)
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
.elseif(n = 5440 || n = 5463)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
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
.elseif(n = 5444)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5445)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5446 || n = 5468)
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
.elseif(n = 5447 || n = 5448 || n = 5464)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5449 || n = 5465)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5452)
	; 14 bytes
	pha
	tya
	ldy #170
	delay_n 27
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
.elseif(n = 5458)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5459)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5461)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5462)
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
.elseif(n = 5467)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	brk
	inx
	Jne *-6
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
; 5469 to 5507
.if(n = 5469 || n = 5494)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5470 || n = 5492 || n = 5501)
	; 14 bytes
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
.elseif(n = 5472 || n = 5498)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5473)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 5473 && n < 5479) || n = 5480 || n = 5481 || (n > 5494 && n < 5498) || (n > 5502 && n < 5506))
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
.elseif(n = 5482)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5483)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5484)
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
.elseif(n = 5485)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5486)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5487)
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
.elseif(n = 5488)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5489)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5490)
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
.elseif(n = 5493)
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
.elseif(n = 5499)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5500)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	iny
	Jne *-7
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
.elseif(n = 5506)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5507)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 5508 to 5546
.if(n = 5508)
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
.elseif(n = 5509)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5510)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5511)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5512 || n = 5534)
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
.elseif(n = 5513 || n = 5514 || n = 5535 || n = 5536 || n = 5539)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5515 || n = 5540)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5516)
	; 14 bytes
	pha
	tya
	ldy #172
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif((n > 5516 && n < 5520) || n = 5521 || n = 5522 || (n > 5524 && n < 5530) || n = 5538 || (n > 5540 && n < 5547))
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
.elseif(n = 5520)
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
.elseif(n = 5524)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5530)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5531)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5532)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5533)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5537)
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
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if(n = 5547 || n = 5574)
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
.elseif(n = 5548)
	; 14 bytes
	pha
	tya
	ldy #173
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5549 || n = 5551 || n = 5552 || n = 5560 || (n > 5565 && n < 5574) || n = 5575 || (n > 5581 && n < 5586))
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
.elseif(n = 5550)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5553)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
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
.elseif(n = 5555)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5556 || n = 5578)
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
.elseif(n = 5557 || n = 5558 || n = 5563 || n = 5564)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5559 || n = 5565 || n = 5581)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5561)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5562)
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
.elseif(n = 5576)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5577)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5579)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5580)
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
.endif
.else
; 5586 to 5625
.if(n = 5586 || n = 5588 || n = 5591 || n = 5593 || n = 5596 || n = 5597 || (n > 5604 && n < 5610) || n = 5619)
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
.elseif(n = 5587 || n = 5612)
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
.elseif(n = 5590 || n = 5595 || n = 5614)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5592 || n = 5623)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5594 || n = 5602 || n = 5613 || n = 5615 || n = 5616 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5598)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5599)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5600 || n = 5622)
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
.elseif(n = 5601)
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
.elseif(n = 5603)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5604)
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
.elseif(n = 5610)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
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
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5618)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5620)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5621)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5624)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
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
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
.if((n >= 5626) && (n < 5665))
; 5626 to 5664
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
.elseif(n = 5627)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5628 || n = 5652)
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
.elseif(n = 5629 || n = 5636 || n = 5657)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5630 || n = 5648 || n = 5649 || n = 5653 || (n > 5657 && n < 5662) || n = 5663)
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
.elseif(n = 5631)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5632)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
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
.elseif(n = 5634 || n = 5635 || n = 5646 || n = 5656 || n = 5664)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5637 || n = 5662)
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
.elseif(n = 5641)
	; 14 bytes
	pha
	txa
	pha
	ldx #0 ;hides 'brk imm'
	inc $05B5,X ;hides 'lda $05,X'
	dex
	Jne *-5
	pla
	tax
	pla
.elseif(n = 5642)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5643)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5645)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5647)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5650)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5651)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5654)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5655)
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
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5690)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5666 || n = 5667 || (n > 5670 && n < 5674) || (n > 5676 && n < 5680) || n = 5681 || n = 5683 || n = 5684 || n = 5686 || (n > 5694 && n < 5698) || (n > 5700 && n < 5704))
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
.elseif(n = 5668)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5669)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5670 || n = 5693)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5674)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5675)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5676 || n = 5700)
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
.elseif(n = 5680)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5682)
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
.elseif(n = 5685)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5687)
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
.elseif(n = 5689)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5691)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5692)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5694)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
	tax
	pla
.elseif(n = 5698)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5699)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 5704 to 5781
.if(n = 5704 || n = 5705 || n = 5707 || n = 5710 || n = 5711 || n = 5713 || n = 5720 || (n > 5724 && n < 5732) || (n > 5732 && n < 5736) || n = 5741 || (n > 5742 && n < 5746) || (n > 5750 && n < 5754) || (n > 5754 && n < 5758) || n = 5759 || (n > 5765 && n < 5770) || (n > 5772 && n < 5778) || n = 5779)
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
.elseif(n = 5706 || n = 5732 || n = 5758)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5708)
	; 14 bytes
	pha
	tya
	ldy #178
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5709 || n = 5736 || n = 5763)
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
.elseif(n = 5712 || n = 5737 || n = 5762)
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
.elseif(n = 5714)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5715)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5716 || n = 5747 || n = 5778)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5717 || n = 5719 || n = 5750 || n = 5765)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5718 || n = 5749 || n = 5764 || n = 5780)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5722)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5723)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5724 || n = 5748 || n = 5772)
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
.elseif(n = 5738)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5739)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5740)
	; 14 bytes
	pha
	tya
	ldy #179
	delay_n 27
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
.elseif(n = 5746)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
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
.elseif(n = 5760)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5761)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5770)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5771)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
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
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
; 5782 to 5820
.if(n = 5782 || n = 5786 || n = 5788 || (n > 5790 && n < 5794) || (n > 5796 && n < 5802) || n = 5803 || n = 5805 || n = 5813 || n = 5816)
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
.elseif(n = 5783)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5784)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5785 || n = 5808)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5787 || n = 5812)
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
.elseif(n = 5789 || n = 5811 || n = 5814)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5790 || n = 5817)
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
.elseif(n = 5794)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5795)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5796 || n = 5820)
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
.elseif(n = 5804)
	; 14 bytes
	pha
	tya
	ldy #181
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5806)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5807)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5809)
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
.elseif(n = 5815)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5818)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5819)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.endif
.else
; 5821 to 5859
.if((n > 5820 && n < 5829) || (n > 5832 && n < 5836) || n = 5838 || n = 5841 || (n > 5845 && n < 5852) || (n > 5854 && n < 5860))
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
.elseif(n = 5829)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5830)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5831 || n = 5854)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5836)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5837)
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
.elseif(n = 5839)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5840)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 5843)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5844)
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
.elseif(n = 5845)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5852)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5853)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5861 || n = 5863 || n = 5870 || n = 5872 || (n > 5877 && n < 5886) || (n > 5892 && n < 5897))
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
.elseif(n = 5862 || n = 5887)
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
.elseif(n = 5864 || n = 5873)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5865 || n = 5869 || n = 5874)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5866)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	bit $24
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5867)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5868 || n = 5892)
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
.elseif(n = 5871)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5875)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5876)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5877)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
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
.elseif(n = 5888)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5889)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5890)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5891)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 5898)
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
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5900)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5901 || n = 5903 || (n > 5905 && n < 5912) || n = 5913 || n = 5917 || n = 5918 || n = 5920 || n = 5921 || n = 5923 || n = 5924 || (n > 5925 && n < 5932) || n = 5934)
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
.elseif(n = 5902 || n = 5933)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5904 || n = 5935)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5905 || n = 5936)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5912 || n = 5937)
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
.elseif(n = 5914)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5915)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5916)
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
.elseif(n = 5925)
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
.elseif(n = 5932)
	; 14 bytes
	pha
	tya
	ldy #185
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
; 5938 to 6015
.if(n = 5938)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5939)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5940 || n = 5988)
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
.elseif((n > 5940 && n < 5952) || (n > 5952 && n < 5962) || (n > 5967 && n < 5979) || n = 5980 || n = 5981 || n = 5983 || n = 5984 || n = 5991 || n = 5993 || n = 5994 || (n > 5998 && n < 6006) || (n > 6006 && n < 6010) || n = 6013)
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
.elseif(n = 5952 || n = 5979 || n = 6006)
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
.elseif(n = 5963)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5964 || n = 5995)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5965 || n = 5967 || n = 5990 || n = 5998 || n = 6015)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5966 || n = 5992)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5986)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5987 || n = 6012)
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
.elseif(n = 5989 || n = 5997 || n = 6014)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5996)
	; 14 bytes
	pha
	tya
	ldy #187
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6010)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6011)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 6016 to 6093
.if(n = 6016)
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
.elseif(n = 6017 || (n > 6018 && n < 6026) || n = 6027 || (n > 6029 && n < 6033) || n = 6041 || n = 6043 || (n > 6044 && n < 6051) || (n > 6051 && n < 6057) || n = 6061 || n = 6063 || (n > 6065 && n < 6070) || n = 6071 || (n > 6072 && n < 6082) || n = 6086 || n = 6093)
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
.elseif(n = 6018 || n = 6044 || n = 6070)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6026 || n = 6057 || n = 6088)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6028 || n = 6039 || n = 6064 || n = 6089 || n = 6090)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6029 || n = 6040 || n = 6065 || n = 6085 || n = 6091)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6033)
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
.elseif(n = 6034)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6035)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6036 || n = 6060 || n = 6084)
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
.elseif(n = 6037 || n = 6062 || n = 6087)
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
.elseif(n = 6058)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6059)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
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
.elseif(n = 6082)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6083)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6092)
	; 14 bytes
	pha
	tya
	ldy #190
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
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
.elseif(n = 6095 || (n > 6096 && n < 6100) || n = 6101 || n = 6104 || n = 6105 || (n > 6108 && n < 6112) || n = 6113 || n = 6116 || n = 6118 || n = 6120 || (n > 6124 && n < 6130))
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
.elseif(n = 6096 || n = 6122)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6103 || n = 6115)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6106)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6107)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6112)
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
.elseif(n = 6114)
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
.elseif(n = 6119)
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
.elseif(n = 6123)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6124)
	; 14 bytes
	pha
	tya
	ldy #191
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6130)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6131)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 6133 to 6171
.if((n > 6132 && n < 6137) || n = 6138 || n = 6142 || n = 6149 || n = 6151 || (n > 6156 && n < 6160) || n = 6161 || n = 6163 || n = 6167 || n = 6169 || n = 6170)
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
.elseif(n = 6137 || n = 6162)
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
.elseif(n = 6139 || n = 6146 || n = 6147 || n = 6152 || n = 6164)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6140 || n = 6153 || n = 6165)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6141 || n = 6168)
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
.elseif(n = 6143)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6144)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6148)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6150)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6154)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6155)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 6160)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6166)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6171)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172 || n = 6173 || (n > 6174 && n < 6181) || n = 6182 || n = 6185 || n = 6186 || n = 6191 || n = 6193 || n = 6194 || (n > 6195 && n < 6199) || (n > 6200 && n < 6210) || n = 6211 || n = 6213 || (n > 6216 && n < 6220) || n = 6221 || (n > 6222 && n < 6226) || (n > 6226 && n < 6237) || n = 6238 || n = 6241 || n = 6242 || n = 6244 || n = 6247 || n = 6248 || n = 6250)
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
.elseif(n = 6174 || n = 6200 || n = 6226)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6181 || n = 6212 || n = 6243)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 6184 || n = 6190 || n = 6215 || n = 6240 || n = 6246)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6187 || n = 6237)
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
.elseif(n = 6188)
	; 14 bytes
	pha
	tya
	ldy #193
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6189 || n = 6214 || n = 6239 || n = 6245)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6195 || n = 6222 || n = 6249)
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
.elseif(n = 6199)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6210)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
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
.elseif(n = 6220)
	; 14 bytes
	pha
	tya
	ldy #194
	delay_n 27
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
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
.if((n >= 6251) && (n < 6407))
.if((n >= 6251) && (n < 6329))
; 6251 to 6328
.if(n = 6251 || (n > 6252 && n < 6262) || n = 6263 || (n > 6266 && n < 6274) || (n > 6278 && n < 6282) || n = 6283 || n = 6285 || n = 6286 || n = 6288 || (n > 6290 && n < 6303) || n = 6311 || n = 6313 || (n > 6316 && n < 6328))
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
.elseif(n = 6252 || n = 6278 || n = 6304)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6262 || n = 6287 || n = 6312)
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
.elseif(n = 6264 || n = 6289 || n = 6307 || n = 6314)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6265 || n = 6277 || n = 6290 || n = 6306 || n = 6308)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6266)
	; 14 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6274 || n = 6305)
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
.elseif(n = 6276)
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
.elseif(n = 6284)
	; 14 bytes
	pha
	tya
	ldy #196
	delay_n 27
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
.elseif(n = 6309)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 6310)
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
.endif
.else
; 6329 to 6406
.if(n = 6329 || (n > 6331 && n < 6336) || n = 6341 || (n > 6342 && n < 6348) || (n > 6348 && n < 6356) || (n > 6357 && n < 6362) || n = 6363 || n = 6366 || n = 6368 || n = 6371 || (n > 6372 && n < 6376) || n = 6377 || n = 6385 || n = 6386 || n = 6388 || (n > 6390 && n < 6396) || n = 6397)
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
.elseif(n = 6330 || n = 6356 || n = 6382)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6331 || n = 6340 || n = 6365 || n = 6370 || n = 6379 || n = 6390 || n = 6399 || n = 6404)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6336 || n = 6367 || n = 6398)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6337 || n = 6362 || n = 6387)
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
.elseif(n = 6338 || n = 6339 || n = 6364 || n = 6369 || n = 6378 || n = 6380 || n = 6389 || n = 6400 || n = 6403 || n = 6405)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6357 || n = 6384)
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
.elseif(n = 6383)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6396)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	sta (@ptrtemp),y
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
.endif
.else
; 6407 to 6562
.if(n = 6407)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6408 || n = 6434 || n = 6486 || n = 6512 || n = 6538)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6409)
	; 14 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 6413 || (n > 6415 && n < 6429) || n = 6430 || n = 6433 || (n > 6434 && n < 6438) || (n > 6438 && n < 6444) || n = 6445 || n = 6446 || n = 6448 || n = 6449 || (n > 6450 && n < 6458) || n = 6459 || n = 6461 || n = 6464 || (n > 6465 && n < 6476) || (n > 6476 && n < 6480) || (n > 6480 && n < 6486) || (n > 6486 && n < 6491) || (n > 6494 && n < 6506) || n = 6507 || n = 6510 || n = 6511 || (n > 6513 && n < 6519) || n = 6520 || n = 6521 || n = 6523 || (n > 6525 && n < 6538) || n = 6539 || (n > 6540 && n < 6546) || (n > 6546 && n < 6552) || n = 6554 || (n > 6556 && n < 6562))
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
.elseif(n = 6415 || n = 6463 || n = 6494 || n = 6509 || n = 6525 || n = 6556)
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
.elseif(n = 6444)
	; 14 bytes
	pha
	tya
	ldy #201
	delay_n 27
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
.elseif(n = 6450)
	; 14 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	delay_n 26
	dey
	Jne *-7
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
.elseif(n = 6476)
	; 14 bytes
	pha
	tya
	ldy #202
	delay_n 27
	dey
	Jne *-6
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
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-6
	tax
	pla
.elseif(n = 6508)
	; 14 bytes
	pha
	tya
	ldy #203
	delay_n 27
	dey
	Jne *-6
	tay
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
.elseif(n = 6540)
	; 14 bytes
	pha
	tya
	ldy #204
	delay_n 27
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
.elseif(n = 6562)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if(n = 6563 || (n > 6564 && n < 6572) || (n > 6573 && n < 6579) || n = 6580 || n = 6581 || n = 6583 || n = 6585 || n = 6588 || n = 6589 || (n > 6590 && n < 6600) || (n > 6600 && n < 6604) || (n > 6604 && n < 6610) || n = 6611 || n = 6614 || (n > 6618 && n < 6627) || (n > 6627 && n < 6631) || (n > 6631 && n < 6636) || (n > 6636 && n < 6641))
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
.elseif(n = 6564 || n = 6590 || n = 6616)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6572)
	; 14 bytes
	pha
	tya
	ldy #205
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6573 || n = 6600 || n = 6627)
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
.elseif(n = 6584 || n = 6615)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6586 || n = 6617)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6587 || n = 6613 || n = 6618)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6604)
	; 14 bytes
	pha
	tya
	ldy #206
	delay_n 27
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6636)
	; 14 bytes
	pha
	tya
	ldy #207
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6641 to 6718
.if(n = 6641 || n = 6643 || n = 6644 || n = 6647 || (n > 6649 && n < 6654) || (n > 6657 && n < 6668) || (n > 6668 && n < 6672) || (n > 6672 && n < 6677) || (n > 6681 && n < 6700) || n = 6701 || (n > 6702 && n < 6706) || n = 6707 || (n > 6712 && n < 6719))
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
.elseif(n = 6642)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
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
.elseif(n = 6646 || n = 6677 || n = 6708)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6648 || n = 6679 || n = 6710)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6649 || n = 6680 || n = 6712)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6654 || n = 6681)
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
.elseif(n = 6655)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6656)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-7
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
.elseif(n = 6668)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 6700)
	; 14 bytes
	pha
	tya
	ldy #209
	delay_n 27
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
.endif
.else
; 6719 to 6875
.if((n > 6718 && n < 6732) || n = 6733 || n = 6734 || n = 6736 || n = 6737 || n = 6740 || n = 6743 || (n > 6744 && n < 6762) || n = 6763 || (n > 6764 && n < 6770) || n = 6771 || (n > 6773 && n < 6777) || (n > 6777 && n < 6789) || n = 6790 || n = 6791 || (n > 6792 && n < 6796) || (n > 6796 && n < 6801) || n = 6802 || (n > 6804 && n < 6810) || (n > 6810 && n < 6816) || (n > 6816 && n < 6822) || (n > 6822 && n < 6828) || (n > 6828 && n < 6832) || n = 6833 || (n > 6835 && n < 6843) || (n > 6843 && n < 6852) || (n > 6852 && n < 6860) || n = 6861 || n = 6862 || n = 6864 || (n > 6866 && n < 6870) || (n > 6870 && n < 6876))
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
.elseif(n = 6764)
	; 14 bytes
	pha
	tya
	ldy #211
	delay_n 27
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
.elseif(n = 6828)
	; 14 bytes
	pha
	tya
	ldy #213
	delay_n 27
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
.elseif(n = 6860)
	; 14 bytes
	pha
	tya
	ldy #214
	delay_n 27
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
.elseif((n > 6876 && n < 6882) || (n > 6882 && n < 6886) || n = 6887 || n = 6890 || n = 6891 || n = 6893 || n = 6895 || (n > 6897 && n < 6908) || (n > 6916 && n < 6922) || n = 6923 || n = 6926 || (n > 6928 && n < 6942) || (n > 6942 && n < 6954) || n = 6955 || n = 6957 || (n > 6959 && n < 6972) || n = 6973 || n = 6974 || (n > 6975 && n < 6987) || (n > 6990 && n < 7002) || (n > 7002 && n < 7008) || n = 7009 || (n > 7010 && n < 7018) || n = 7019 || (n > 7021 && n < 7032))
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
.elseif(n = 6888 || n = 6896 || n = 6910 || n = 6914 || n = 6915 || n = 6927 || n = 6958 || n = 6989 || n = 7020)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6889 || n = 6911 || n = 6916 || n = 6928 || n = 6959 || n = 6990 || n = 7021)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6892)
	; 14 bytes
	pha
	tya
	ldy #215
	delay_n 27
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6908)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
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
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
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
.elseif(n = 6954)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
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
.elseif(n = 6988)
	; 14 bytes
	pha
	tya
	ldy #218
	delay_n 27
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
.elseif((n > 7032 && n < 7041) || (n > 7041 && n < 7049) || n = 7050 || (n > 7052 && n < 7062) || (n > 7062 && n < 7074) || (n > 7074 && n < 7080) || n = 7081 || (n > 7084 && n < 7092) || (n > 7092 && n < 7107) || (n > 7107 && n < 7111) || n = 7112 || (n > 7116 && n < 7122) || (n > 7122 && n < 7138) || n = 7139 || n = 7143 || n = 7146 || n = 7147 || (n > 7148 && n < 7152) || (n > 7152 && n < 7164) || (n > 7164 && n < 7168) || (n > 7169 && n < 7173) || n = 7174 || (n > 7176 && n < 7180) || n = 7181 || (n > 7182 && n < 7188))
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
.elseif(n = 7052)
	; 14 bytes
	pha
	tya
	ldy #220
	delay_n 27
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7083 || n = 7114 || n = 7145 || n = 7176)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7084)
	; 14 bytes
	pha
	tya
	ldy #221
	delay_n 27
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
.elseif(n = 7116)
	; 14 bytes
	pha
	tya
	ldy #222
	delay_n 27
	dey
	Jne *-6
	tay
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
.elseif(n = 7148)
	; 14 bytes
	pha
	tya
	ldy #223
	delay_n 27
	dey
	Jne *-6
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
.elseif(n = 7164)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	bit $24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7168)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	delay_n 7
	dey
	Jne *-7
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
.elseif(n = 7180)
	; 14 bytes
	pha
	tya
	ldy #224
	delay_n 27
	dey
	Jne *-6
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
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if((n > 7187 && n < 7202) || n = 7203 || n = 7205 || (n > 7207 && n < 7212) || (n > 7212 && n < 7235) || n = 7236 || n = 7240 || n = 7241 || n = 7243 || (n > 7244 && n < 7266) || n = 7267 || n = 7270 || n = 7271 || (n > 7272 && n < 7276) || (n > 7276 && n < 7297) || n = 7298 || n = 7301 || n = 7303 || n = 7304 || n = 7306 || n = 7307 || (n > 7308 && n < 7328) || n = 7329 || (n > 7332 && n < 7338) || n = 7339 || (n > 7340 && n < 7344))
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
.elseif(n = 7204 || n = 7235 || n = 7266 || n = 7297 || n = 7328)
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
.elseif(n = 7207 || n = 7238 || n = 7269 || n = 7300 || n = 7331)
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
.elseif(n = 7237 || n = 7268 || n = 7299 || n = 7330)
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
.elseif(n = 7244)
	; 14 bytes
	pha
	tya
	ldy #226
	delay_n 27
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
.elseif(n = 7276)
	; 14 bytes
	pha
	tya
	ldy #227
	delay_n 27
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
.elseif(n = 7308)
	; 14 bytes
	pha
	tya
	ldy #228
	delay_n 27
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
.elseif(n = 7340)
	; 14 bytes
	pha
	tya
	ldy #229
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 7344 to 7500
.if((n > 7343 && n < 7354) || (n > 7354 && n < 7359) || n = 7360 || (n > 7362 && n < 7371) || (n > 7372 && n < 7390) || n = 7391 || n = 7394 || n = 7395 || (n > 7396 && n < 7404) || (n > 7404 && n < 7420) || (n > 7425 && n < 7434) || n = 7435 || (n > 7437 && n < 7452) || n = 7453 || n = 7456 || (n > 7457 && n < 7468) || n = 7469 || (n > 7470 && n < 7480) || n = 7481 || n = 7484 || (n > 7486 && n < 7491) || (n > 7491 && n < 7498) || n = 7499)
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
.elseif(n = 7354)
	; 14 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7359 || n = 7390 || n = 7421 || n = 7452 || n = 7483)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7361 || n = 7392 || n = 7423 || n = 7454 || n = 7485)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7372)
	; 14 bytes
	pha
	tya
	ldy #230
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7393 || n = 7455 || n = 7486)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 7420)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dey
	Jne *-7
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
.elseif(n = 7424)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
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
.elseif(n = 7436)
	; 14 bytes
	pha
	tya
	ldy #232
	delay_n 27
	dey
	Jne *-6
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
.elseif(n = 7457)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7468)
	; 14 bytes
	pha
	tya
	ldy #233
	delay_n 27
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
.elseif(n = 7480)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	iny
	Jne *-7
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
.elseif(n = 7491)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 26
	iny
	Jne *-7
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
.elseif(n = 7500)
	; 14 bytes
	pha
	tya
	ldy #234
	delay_n 27
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
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
; 7501 to 7656
.if(n = 7501 || n = 7502 || (n > 7503 && n < 7512) || n = 7513 || n = 7515 || (n > 7517 && n < 7525) || (n > 7525 && n < 7532) || (n > 7532 && n < 7536) || (n > 7536 && n < 7542) || n = 7543 || n = 7544 || n = 7546 || (n > 7548 && n < 7564) || (n > 7564 && n < 7569) || n = 7571 || (n > 7572 && n < 7576) || n = 7577 || (n > 7579 && n < 7596) || (n > 7596 && n < 7602) || (n > 7602 && n < 7607) || n = 7608 || (n > 7610 && n < 7628) || (n > 7628 && n < 7632) || n = 7633 || n = 7634 || n = 7636 || n = 7637 || n = 7639 || (n > 7641 && n < 7651) || n = 7652 || n = 7655 || n = 7656)
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
.elseif(n = 7525)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7532)
	; 14 bytes
	pha
	tya
	ldy #235
	delay_n 27
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7564)
	; 14 bytes
	pha
	tya
	ldy #236
	delay_n 27
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
.elseif(n = 7596)
	; 14 bytes
	pha
	tya
	ldy #237
	delay_n 27
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
.elseif(n = 7628)
	; 14 bytes
	pha
	tya
	ldy #238
	delay_n 27
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
.if((n > 7656 && n < 7660) || n = 7661 || (n > 7662 && n < 7666) || n = 7667 || n = 7670 || (n > 7672 && n < 7681) || n = 7682 || (n > 7684 && n < 7692) || (n > 7692 && n < 7700) || (n > 7703 && n < 7724) || (n > 7724 && n < 7731) || (n > 7735 && n < 7756) || (n > 7756 && n < 7762) || n = 7763 || n = 7766 || (n > 7767 && n < 7788) || (n > 7788 && n < 7793) || n = 7794 || (n > 7796 && n < 7800) || (n > 7800 && n < 7813))
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
.elseif(n = 7660)
	; 14 bytes
	pha
	tya
	ldy #239
	delay_n 27
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7756)
	; 14 bytes
	pha
	tya
	ldy #242
	delay_n 27
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
.elseif(n = 7788)
	; 14 bytes
	pha
	tya
	ldy #243
	delay_n 27
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
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7820) || (n > 7820 && n < 7824) || n = 7825 || (n > 7827 && n < 7833) || (n > 7833 && n < 7852) || n = 7853 || n = 7854 || n = 7856 || (n > 7858 && n < 7866) || (n > 7866 && n < 7884) || n = 7885 || n = 7887 || n = 7890)
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
.elseif(n = 7820)
	; 14 bytes
	pha
	tya
	ldy #244
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7824 || n = 7855 || n = 7886)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7826 || n = 7857 || n = 7888)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7827 || n = 7858 || n = 7889)
	; 14 bytes
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
.elseif(n = 7852)
	; 14 bytes
	pha
	tya
	ldy #245
	delay_n 27
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
.elseif(n = 7884)
	; 14 bytes
	pha
	tya
	ldy #246
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 7891 to 7968
.if((n > 7890 && n < 7898) || (n > 7899 && n < 7906) || n = 7907 || n = 7914 || n = 7915 || n = 7918 || (n > 7920 && n < 7926) || n = 7927 || (n > 7933 && n < 7937) || n = 7938 || n = 7945 || n = 7947 || n = 7949 || (n > 7951 && n < 7965) || n = 7967 || n = 7968)
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
.elseif(n = 7898)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 27
	iny
	Jne *-7
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
.elseif(n = 7908 || n = 7910 || n = 7911 || n = 7919 || n = 7928 || n = 7930 || n = 7939 || n = 7941 || n = 7950)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7909 || n = 7920 || n = 7929 || n = 7940 || n = 7951)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7912)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	brk
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 7913 || n = 7944)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7916)
	; 14 bytes
	pha
	tya
	ldy #247
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7917)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7926)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 7943)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 7966)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7980) || (n > 7980 && n < 7998) || (n > 7998 && n < 8012) || (n > 8012 && n < 8031) || (n > 8031 && n < 8044) || (n > 8044 && n < 8064) || (n > 8064 && n < 8076) || (n > 8076 && n < 8095) || n = 8096 || (n > 8097 && n < 8108) || (n > 8108 && n < 8126))
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
.elseif(n = 7980)
	; 14 bytes
	pha
	tya
	ldy #249
	delay_n 27
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
.elseif(n = 8012)
	; 14 bytes
	pha
	tya
	ldy #250
	delay_n 27
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
.elseif(n = 8044)
	; 14 bytes
	pha
	tya
	ldy #251
	delay_n 27
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
.elseif(n = 8076)
	; 14 bytes
	pha
	tya
	ldy #252
	delay_n 27
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
.elseif(n = 8108)
	; 14 bytes
	pha
	tya
	ldy #253
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
; 8126 to 8437
.if((n > 8125 && n < 8130) || (n > 8130 && n < 8140) || (n > 8140 && n < 8161) || n = 8162 || (n > 8164 && n < 8172) || (n > 8172 && n < 8188) || (n > 8188 && n < 8192) || n = 8198 || n = 8199 || (n > 8200 && n < 8204) || (n > 8204 && n < 8229) || (n > 8229 && n < 8262) || (n > 8262 && n < 8295) || (n > 8295 && n < 8328) || (n > 8328 && n < 8361) || (n > 8361 && n < 8394) || (n > 8394 && n < 8416) || n = 8417 || (n > 8419 && n < 8427) || (n > 8427 && n < 8438))
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
.elseif(n = 8140)
	; 14 bytes
	pha
	tya
	ldy #254
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8161)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
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
.elseif(n = 8164 || n = 8197 || n = 8419)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 8172)
	; 14 bytes
	pha
	tya
	ldy #255
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8188)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8192)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8193)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 8195 || n = 8418)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8204)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 27
	dey
	Jne *-6
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
.endif
.else
; 8438 to 8750
.if(n = 8438)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8439 || (n > 8442 && n < 8448) || (n > 8452 && n < 8460) || (n > 8460 && n < 8671) || (n > 8671 && n < 8690) || (n > 8690 && n < 8694) || (n > 8694 && n < 8704) || (n > 8705 && n < 8751))
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
.elseif(n = 8440 || n = 8450 || n = 8451)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8441 || n = 8452)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 8442)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8448)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk
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
.elseif(n = 8671)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	brk
	iny
	Jne *-7
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
.elseif(n = 8694)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	bit $24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8704)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8705)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8926) || (n > 8926 && n < 8942) || (n > 8942 && n < 8950) || (n > 8950 && n < 8959) || (n > 8961 && n < 9691) || (n > 9691 && n < 9718) || (n > 9718 && n < 9724) || (n > 9724 && n < 9729) || (n > 9729 && n < 10001))
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
.elseif(n = 8950)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8959)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8960)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk
	dey
	Jne *-7
	tay
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
.elseif(n = 9718)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9724)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
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
.if((n > 10000 && n < 11221) || (n > 11221 && n < 11254) || (n > 11254 && n < 11265) || (n > 11265 && n < 20001))
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
.elseif(n = 11254)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	dey
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
