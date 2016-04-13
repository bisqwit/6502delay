; delay_n, delays n cycles with small code
; Will clobber A, Z+N, and D
; Will preserve X, Y, S, C, V, and I (interrupt-disable flag)
;
; Uses JMP (unrelocatable code).
;
; WARNING: May write garbage into $200-$2FF.
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 28
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 315
; -10 bytes or less when n <= 1298
; -11 bytes or less when n <= 1776
; -12 bytes or less when n <= 2465
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=5.0, max=7
; - When 21 <= n <= 30: min=6, avg=6.9, max=8
; - When 31 <= n <= 50: min=6, avg=7.0, max=8
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.4, max=10
; - When 501 <= n <= 750: min=6, avg=8.4, max=10
; - When 751 <= n <= 1000: min=7, avg=8.4, max=10
; - When 1001 <= n <= 2000: min=6, avg=9.3, max=12
; - When 2001 <= n <= 3000: min=8, avg=10.6, max=13
; - When 3001 <= n <= 5000: min=9, avg=12.1, max=13
; - When 5001 <= n <= 7500: min=11, avg=13.0, max=13
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
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	lda $A5
.elseif((n > 3 && n < 7) || n = 26 || n = 32)
	; 2..8 bytes
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
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 5 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 16 || n = 18 || n = 20)
	; 5..7 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 17)
	; 6 bytes
	php
	rol $26
	ror $26
	plp
.elseif(n = 22 || n = 24)
	; 6..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
.elseif(n = 25)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
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
.elseif((n > 33 && n < 46) || (n > 46 && n < 53) || (n > 53 && n < 79))
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
.endif
.else
; 79 to 156
.if((n > 78 && n < 128) || (n > 128 && n < 157))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 128)
	; 6 bytes
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
.endif
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
.elseif(n = 291 || n = 297 || n = 301 || n = 302 || n = 304 || n = 307 || n = 309 || n = 312)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 292)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 293 || n = 294 || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 298 || n = 308)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 299 || n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
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
	delay_n 4
	dey
	Jpl *-3
	tay
.endif
.endif
.else
.if((n >= 313) && (n < 469))
; 313 to 468
.if(n = 313 || n = 348 || n = 362 || n = 369 || n = 404 || n = 432 || n = 439 || n = 446)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 314 || n = 317 || n = 319 || n = 321 || n = 322 || n = 324 || n = 326 || n = 331 || n = 332 || n = 336 || n = 337 || n = 339 || n = 342 || n = 344 || n = 347 || n = 349 || n = 352 || n = 354 || n = 357 || n = 359 || n = 366 || n = 367 || n = 371 || n = 372 || n = 374 || n = 377 || n = 379 || n = 382 || n = 384 || n = 387 || n = 389 || n = 391 || n = 392 || n = 394 || n = 396 || n = 406 || n = 407 || n = 412 || n = 414 || n = 417 || n = 419 || n = 422 || n = 424 || n = 426 || n = 427 || n = 429 || n = 431 || n = 436 || n = 437 || n = 441 || n = 442 || n = 444 || n = 447 || n = 449 || n = 452 || n = 454 || n = 457 || n = 459 || n = 462 || n = 464 || n = 466)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385 || n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316)
	; 10 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	plp
.elseif(n = 318 || n = 323 || n = 328 || n = 333 || n = 338 || n = 343 || n = 351 || n = 353 || n = 358 || n = 368 || n = 373 || n = 378 || n = 386 || n = 388 || n = 393 || n = 398 || n = 403 || n = 413 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 456 || n = 458 || n = 463 || n = 468)
	; 8..10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 0) && ((n > 324 && n < 341) || n = 355 || n = 360 || (n > 369 && n < 381) || n = 390 || n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 334 || n = 341 || n = 376 || n = 397 || n = 411 || n = 418 || n = 453 || n = 467)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 329 || n = 364 || n = 399 || n = 434)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 346)
	; 9 bytes
	php
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	plp
.elseif(n = 356 || n = 401)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 361)
	; 10 bytes
	php
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	plp
.elseif(n = 363)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 381 || n = 451)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 383)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 408)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
	tay
.elseif(n = 409)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 416)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 421)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	tax
.elseif(n = 461)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.endif
.else
; 469 to 625
.if(n = 469 || n = 504)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 0) && (n = 470 || n = 480 || n = 485 || n = 495 || (n > 504 && n < 516) || (n > 524 && n < 566) || n = 590 || (n > 599 && n < 611)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 471)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
.elseif(n = 472 || n = 476 || n = 477 || n = 479 || n = 482 || n = 484 || n = 487 || n = 492 || n = 494 || n = 497 || n = 499 || n = 501 || n = 506 || n = 511 || n = 512 || n = 517 || n = 519 || n = 522 || n = 524 || n = 527 || n = 529 || n = 531 || n = 532 || n = 534 || n = 539 || n = 541 || n = 542 || n = 546 || n = 547 || n = 549 || n = 552 || n = 557 || n = 559 || n = 562 || n = 564 || n = 567 || n = 569 || n = 574 || n = 576 || n = 577 || n = 582 || n = 584 || n = 587 || n = 589 || n = 592 || n = 594 || n = 597 || n = 601 || n = 602 || n = 604 || n = 606 || n = 609 || n = 612 || n = 616 || n = 617 || n = 619 || n = 622 || n = 624)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 473 || n = 478 || n = 483 || n = 493 || n = 498 || n = 503 || n = 508 || n = 513 || n = 518 || n = 526 || n = 528 || n = 533 || n = 538 || n = 543 || n = 548 || n = 553 || n = 561 || n = 563 || n = 568 || n = 573 || n = 578 || n = 583 || n = 588 || n = 596 || n = 598 || n = 603 || n = 608 || n = 613 || n = 618 || n = 623)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 474 || n = 502 || n = 516 || n = 523 || n = 544 || n = 551 || n = 586 || n = 607 || n = 614 || n = 621)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif((n .mod 5 = 0) && (n = 475 || n = 490 || n = 500 || n = 520 || (n > 569 && n < 586) || n = 595 || (n > 614 && n < 626)))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 481 || n = 488 || n = 509 || n = 537 || n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 486 || n = 556 || n = 591)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 489)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
.elseif(n = 491 || n = 554)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 496 || n = 514)
	; 9..10 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 536 || n = 581 || n = 599)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 566)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 571)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
	plp
.elseif(n = 611)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jmp *+3
	Jmi *-4
	tax
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626)
	; 7 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
.elseif(n = 627 || n = 629 || n = 632 || n = 634 || n = 636 || n = 637 || n = 639 || n = 647 || n = 652 || n = 654 || n = 657 || n = 659 || n = 662 || n = 664 || n = 667 || n = 669 || n = 672 || n = 674 || n = 679 || n = 681 || n = 682 || n = 686 || n = 687 || n = 692 || n = 694 || n = 697 || n = 699 || n = 702 || n = 704 || n = 706 || n = 707 || n = 709 || n = 711 || n = 717 || n = 721 || n = 722 || n = 724 || n = 727 || n = 729 || n = 732 || n = 737 || n = 739 || n = 741 || n = 742 || n = 744 || n = 746 || n = 751 || n = 756 || n = 757 || n = 762 || n = 764 || n = 767 || n = 769 || n = 772 || n = 774 || n = 776 || n = 777 || n = 781)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 649 || n = 663 || n = 684 || n = 698 || n = 726)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
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
.elseif(n = 633 || n = 738 || n = 759)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 638 || n = 643 || n = 648 || n = 653 || n = 658 || n = 666 || n = 668 || n = 673 || n = 678 || n = 683 || n = 693 || n = 703 || n = 708 || n = 713 || n = 718 || n = 723 || n = 728 || n = 743 || n = 748 || n = 753 || n = 758 || n = 763 || n = 771 || n = 773 || n = 778)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 641 || n = 701)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 642 || n = 656 || n = 677 || n = 691 || n = 712 || n = 719 || n = 733 || n = 747 || n = 754 || n = 761 || n = 768)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 644 || n = 671 || n = 779)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 646 || n = 714 || n = 749)
	; 9..10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 661 || n = 766)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 676)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 689 || n = 716 || n = 734)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 696)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
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
.endif
.else
; 782 to 937
.if(n = 782 || n = 789 || n = 824 || n = 838 || n = 852 || n = 887 || n = 922 || n = 929 || n = 936)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 783 || n = 788 || n = 793 || n = 798 || n = 808 || n = 813 || n = 818 || n = 823 || n = 828 || n = 833 || n = 841 || n = 843 || n = 848 || n = 853 || n = 858 || n = 863 || n = 868 || n = 876 || n = 878 || n = 883 || n = 888 || n = 893 || n = 898 || n = 911 || n = 913 || n = 918 || n = 923 || n = 928 || n = 933)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 784 || n = 819 || n = 854 || n = 889 || n = 924)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 0) && ((n > 784 && n < 936)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 786)
	; 10 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 787 || n = 791 || n = 792 || n = 794 || n = 797 || n = 799 || n = 802 || n = 807 || n = 809 || n = 812 || n = 814 || n = 816 || n = 822 || n = 826 || n = 827 || n = 829 || n = 832 || n = 834 || n = 837 || n = 839 || n = 842 || n = 844 || n = 846 || n = 847 || n = 849 || n = 856 || n = 857 || n = 861 || n = 862 || n = 864 || n = 867 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 886 || n = 891 || n = 892 || n = 897 || n = 899 || n = 902 || n = 904 || n = 907 || n = 909 || n = 912 || n = 914 || n = 916 || n = 917 || n = 919 || n = 926 || n = 927 || n = 931 || n = 932 || n = 934 || n = 937)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 796 || n = 803 || n = 817 || n = 831 || n = 859 || n = 866 || n = 873 || n = 894 || n = 901 || n = 908)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 801)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	dey
	Jpl *-4
	tay
.elseif(n = 804)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
.elseif(n = 806 || n = 851 || n = 896)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 811)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 821)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 836)
	; 9 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	iny
	Jne *-2
	pla
	tay
.elseif(n = 869)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 871)
	; 10 bytes
	php
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	plp
.elseif(n = 881)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 903)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 906)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 921)
	; 9 bytes
	tya
	ldy #<$8D ;hides 'sta $1AA0'
	ldy #26
	iny
	Jpl *-4
	tay
.endif
.endif
.else
; 938 to 1250
.if(n = 938 || n = 948 || n = 953 || n = 958 || n = 963 || n = 968 || n = 973 || n = 981 || n = 983 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1016 || n = 1021 || n = 1023 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1051 || n = 1053 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1086 || n = 1088 || n = 1093 || n = 1098 || n = 1103 || n = 1108 || n = 1113 || n = 1123 || n = 1128 || n = 1133 || n = 1138 || n = 1143 || n = 1148 || n = 1156 || n = 1158 || n = 1163 || n = 1168 || n = 1173 || n = 1178 || n = 1183 || n = 1193 || n = 1198 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1226 || n = 1228 || n = 1233 || n = 1238 || n = 1243 || n = 1248)
	; 8..10 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 941 || n = 986 || n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 942 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 956 || n = 961 || n = 967 || n = 972 || n = 974 || n = 977 || n = 979 || n = 982 || n = 984 || n = 987 || n = 989 || n = 991 || n = 996 || n = 997 || n = 1001 || n = 1002 || n = 1004 || n = 1007 || n = 1009 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1024 || n = 1026 || n = 1032 || n = 1036 || n = 1037 || n = 1039 || n = 1042 || n = 1044 || n = 1047 || n = 1049 || n = 1052 || n = 1054 || n = 1057 || n = 1059 || n = 1066 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1079 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1091 || n = 1092 || n = 1094 || n = 1096 || n = 1101 || n = 1102 || n = 1106 || n = 1107 || n = 1109 || n = 1112 || n = 1114 || n = 1117 || n = 1122 || n = 1124 || n = 1127 || n = 1129 || n = 1131 || n = 1136 || n = 1137 || n = 1141 || n = 1142 || n = 1144 || n = 1147 || n = 1149 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1161 || n = 1162 || n = 1164 || n = 1171 || n = 1172 || n = 1176 || n = 1177 || n = 1179 || n = 1182 || n = 1184 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1196 || n = 1197 || n = 1199 || n = 1201 || n = 1206 || n = 1207 || n = 1212 || n = 1214 || n = 1217 || n = 1219 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1231 || n = 1232 || n = 1234 || n = 1242 || n = 1246 || n = 1247 || n = 1249)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && (n = 943 || (n > 956 && n < 979) || (n > 991 && n < 1014) || (n > 1026 && n < 1049) || (n > 1061 && n < 1084) || (n > 1096 && n < 1119) || (n > 1131 && n < 1154) || (n > 1166 && n < 1189) || (n > 1201 && n < 1224) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 946 || n = 969 || n = 1081 || n = 1126)
	; 9..10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 951)
	; 10 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 959 || n = 994 || n = 1029 || n = 1064 || n = 1099 || n = 1134 || n = 1169 || n = 1204 || n = 1239)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1031 || n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1046 || n = 1116 || n = 1151 || n = 1186 || n = 1221)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1056)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1061 || n = 1241)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1074)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 1119)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1191)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1236)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	tay
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || (n > 1271 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1254 || n = 1257 || n = 1259 || n = 1262 || n = 1264 || n = 1266 || n = 1267 || n = 1269 || n = 1271 || n = 1276 || n = 1277 || n = 1281 || n = 1282 || n = 1284 || n = 1287 || n = 1289 || n = 1294 || n = 1295 || n = 1297 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1315 || n = 1316 || n = 1318 || n = 1323 || n = 1327)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1253 || n = 1261 || n = 1263 || n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1303 || n = 1317 || n = 1324)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1281)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 1256 || n = 1301 || n = 1310 || n = 1319)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1274)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1291 || n = 1305 || n = 1326)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1292 || n = 1308 || n = 1320)
	; 9..11 bytes
	pha
	delay_n (n-7)
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
.elseif(n = 1299)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1306 || n = 1322)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1312)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	iny
	iny
	Jne *-4
	tay
.elseif(n = 1313 || n = 1325)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.endif
.else
; 1329 to 1406
.if(n = 1329)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	ldx #240
	dex
	Jne *-1
	tax
.elseif(n = 1330 || n = 1332 || n = 1343 || n = 1344 || n = 1348 || n = 1350 || n = 1351 || n = 1353 || n = 1357 || n = 1358 || n = 1360 || n = 1365 || n = 1372 || n = 1374 || n = 1379 || n = 1381 || n = 1386 || n = 1388 || n = 1393 || n = 1395 || n = 1399)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1345 || n = 1352 || n = 1359 || n = 1366 || n = 1376 || n = 1383 || n = 1394 || n = 1401 || n = 1406)
	; 9..11 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1355) || n = 1368 || n = 1375 || n = 1389 || n = 1396))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1334)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1406)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1336)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1336 && n < 1383) || n = 1400))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1339 || n = 1402)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1341)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1361 || n = 1385 || n = 1397)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1362 || n = 1387 || n = 1392 || n = 1404)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1367)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1369)
	; 10 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1371)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jpl *-5
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
.elseif(n = 1390)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1411 || n = 1414 || n = 1416 || n = 1420 || n = 1421 || n = 1423 || n = 1428 || n = 1430 || n = 1435 || n = 1437 || n = 1442 || n = 1449 || n = 1451 || n = 1456 || n = 1458 || n = 1470 || n = 1474 || n = 1477 || n = 1479 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408 || n = 1415 || n = 1422 || n = 1429 || n = 1439 || n = 1443 || n = 1448 || n = 1450 || n = 1464 || n = 1471 || n = 1478)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 9 = 5) && ((n > 1408 && n < 1446) || (n > 1462 && n < 1482)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1410 || n = 1425 || n = 1431 || n = 1434 || n = 1476)
	; 9..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1413)
	; 10 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1417 || n = 1438 || n = 1452 || n = 1466 || n = 1473 || n = 1480)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1432 || n = 1441)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1446)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1453)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1455)
	; 11 bytes
	tya
	ldy #(n-5)/10
	nop
	jmp *+3
	dey
	Jne *-5
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
.elseif(n = 1460)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1462)
	; 11 bytes
	php
	sec
	lda #120
	delay_n 7
	sbc #1
	Jpl *-4
	plp
.elseif(n = 1465)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1467)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 1469)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.endif
.else
; 1485 to 1562
.if(n = 1485 || n = 1492 || n = 1502 || n = 1506 || n = 1513 || n = 1518 || n = 1527 || n = 1534 || n = 1548 || n = 1555)
	; 9..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1486 || n = 1491 || n = 1495 || n = 1498 || n = 1500 || n = 1505 || n = 1507 || n = 1512 || n = 1514 || n = 1519 || n = 1521 || n = 1533 || n = 1537 || n = 1539 || n = 1540 || n = 1542 || n = 1546 || n = 1547 || n = 1549 || n = 1554 || n = 1556 || n = 1561)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && (n = 1487 || n = 1494 || (n > 1535 && n < 1558)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1488 || n = 1497 || n = 1522 || n = 1551 || n = 1560)
	; 9..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && ((n > 1489 && n < 1509) || (n > 1525 && n < 1563)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1493 || n = 1529 || n = 1541)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1501)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jmi *-4
	tay
.elseif(n = 1504)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1509 || n = 1525)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1511)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1515)
	; 7 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
.elseif(n = 1516 || n = 1532)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1520)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1523)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1528)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1530)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1558)
	; 10 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	dey ;first loop only
	iny
	Jne *-3
	pla
	tay
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1568 || n = 1570 || n = 1575 || n = 1582 || n = 1584 || n = 1593 || n = 1596 || n = 1603 || n = 1605 || n = 1609 || n = 1610 || n = 1612 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1633 || n = 1638 || n = 1640 || n = 1645 || n = 1651 || n = 1659 || n = 1663 || n = 1665 || n = 1666 || n = 1668 || n = 1672 || n = 1673 || n = 1675 || n = 1680 || n = 1682 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1707 || n = 1708 || n = 1710 || n = 1712)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && (n = 1564 || (n > 1577 && n < 1607) || n = 1620 || (n > 1654 && n < 1691) || (n > 1703 && n < 1719)))
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1565 || n = 1577 || n = 1613 || n = 1637 || n = 1649 || n = 1709)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1567 || n = 1600 || n = 1677)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1569 || n = 1574 || n = 1576 || n = 1583 || n = 1590 || n = 1597 || n = 1604 || n = 1611 || n = 1618 || n = 1630 || n = 1639 || n = 1644 || n = 1653 || n = 1660 || n = 1667 || n = 1674 || n = 1681 || n = 1691 || n = 1695 || n = 1702 || n = 1716)
	; 9..11 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 9 = 5) && (n = 1571 || (n > 1588 && n < 1626) || (n > 1651 && n < 1698) || n = 1715))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1572 || n = 1621 || n = 1623 || n = 1634 || n = 1648 || n = 1654 || n = 1656)
	; 9..11 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 1581 || n = 1693)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1586 || n = 1705)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1588 || n = 1628 || n = 1684 || n = 1700)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1591 || n = 1717)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1595)
	; 10 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1602 || n = 1642 || n = 1658 || n = 1698 || n = 1714)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1635)
	; 11 bytes
	php
	lda $A5
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	plp
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
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1731 || n = 1735 || n = 1736 || n = 1738 || n = 1743 || n = 1747 || n = 1750 || n = 1752 || n = 1757 || n = 1759 || n = 1761 || n = 1764 || n = 1766 || n = 1768 || n = 1771 || n = 1773 || n = 1782 || n = 1785 || n = 1789 || n = 1791 || n = 1792)
	; 9..11 bytes
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
.elseif(n = 1721 || n = 1745 || n = 1781 || n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1723 || n = 1730 || n = 1737 || n = 1758 || n = 1765 || n = 1772 || n = 1786)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 9 = 5) && ((n > 1723 && n < 1761) || (n > 1777 && n < 1797)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1732 || n = 1753 || n = 1767 || n = 1774 || n = 1788 || n = 1795)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1728 || n = 1740 || n = 1746)
	; 9..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1739)
	; 7 bytes
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
.elseif(n = 1744)
	; 9 bytes
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1749)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1754)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1756)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
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
.elseif(n = 1770)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1775)
	; 11 bytes
	tya
	ldy #(n-20)/15
	rol $26
	ror $26
	dey
	Jpl *-5
	tay
.elseif(n = 1777)
	; 12 bytes
	php
	lda $A5
	sec
	lda #252
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1779)
	; 10 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1780)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1784)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
	tay
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
.if((n >= 1797) && (n < 1836))
; 1797 to 1835
.if(n = 1797)
	; 8 bytes
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 11 = 5) && ((n > 1797 && n < 1832)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1799 || n = 1807 || n = 1816 || n = 1819 || n = 1825 || n = 1834)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1833)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1801 || n = 1804 || n = 1810 || n = 1812)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif((n .mod 9 = 5) && ((n > 1804 && n < 1833)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
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
.elseif(n = 1813 || n = 1821)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1815)
	; 11 bytes
	tya
	ldy #(n-5)/10
	nop
	jmp *+3
	dey
	Jne *-5
	tay
.elseif(n = 1817 || n = 1829)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1818 || n = 1826)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1822)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1824)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1827)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1828)
	; 10 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	iny ;first loop only
	iny
	Jne *-3
	pla
	tay
.elseif(n = 1830)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
.elseif(n = 1835)
	; 10 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	tay
	plp
.endif
.else
; 1836 to 1875
.if(n = 1836 || n = 1837 || n = 1848 || n = 1849 || n = 1857 || n = 1860 || n = 1866 || n = 1872)
	; 10..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1838 || n = 1851)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 1839 || n = 1867)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1840)
	; 12 bytes
	pha
	lda $A5
	delay_n (n-10)
	pla
.elseif((n .mod 9 = 5) && ((n > 1840 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1842 || n = 1864 || n = 1875)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
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
.elseif(n = 1844)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1845 || n = 1869)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
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
.elseif(n = 1847 || n = 1852 || n = 1854 || n = 1855 || n = 1861 || n = 1863 || n = 1870)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1853 || n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1856 || n = 1862 || n = 1871)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1858)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1873)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1874)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.if(n = 1876 || n = 1900 || n = 1916 || n = 1948)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif((n .mod 9 = 5) && ((n > 1876 && n < 1950)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1878 || n = 1879 || n = 1883 || n = 1887 || n = 1888 || n = 1891 || n = 1903 || n = 1905 || n = 1906 || n = 1915 || n = 1924 || n = 1933 || n = 1935 || n = 1939 || n = 1942 || n = 1951 || n = 1953)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1880 || n = 1910)
	; 11 bytes
	tya
	ldy #(n-20)/15
	rol $26
	ror $26
	dey
	Jpl *-5
	tay
.elseif(n = 1881 || n = 1909 || n = 1923)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1882 || n = 1890 || n = 1898 || n = 1914 || n = 1946)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1884 || n = 1896 || n = 1902 || n = 1911 || n = 1920 || n = 1928 || n = 1929 || n = 1932 || n = 1938 || n = 1944)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1885 || n = 1893 || n = 1917)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 1889 || n = 1901 || n = 1925 || n = 1937)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1892)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif((n .mod 11 = 5) && ((n > 1896 && n < 1942)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1899)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1907)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1912)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
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
.elseif(n = 1926 || n = 1943)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 1927)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1934 || n = 1950)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1936)
	; 10 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1945)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1947)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1952)
	; 10 bytes
	php
	sec
	lda #216 ;hides 'cld'
	nop
	sbc #1
	Jne *-4
	plp
.endif
.else
; 1954 to 2031
.if(n = 1954 || n = 1956 || n = 1968 || n = 1980 || n = 1983 || n = 2001 || n = 2004 || n = 2010 || n = 2016)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1955 || n = 2000 || n = 2015)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 1957 || n = 1981 || n = 1989 || n = 2013)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 2031)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 1960 || n = 1969 || n = 1975 || n = 1978 || n = 1987 || n = 1995 || n = 1999 || n = 2005 || n = 2011 || n = 2014 || n = 2023 || n = 2025 || n = 2031)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1961 || n = 1973 || n = 1997 || n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1962 || n = 1970 || n = 1986 || n = 2002 || n = 2026)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 11 = 5) && (n = 1963 || n = 1974 || (n > 1995 && n < 2030)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1964 || n = 1988 || n = 2020)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1965 || n = 1979 || n = 1993)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1966)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 1971)
	; 11 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1972)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
.elseif(n = 1977)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
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
.elseif(n = 1984)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 1990 || n = 2008 || n = 2017)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1991)
	; 11 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.elseif(n = 1992)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
.elseif(n = 1998)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2006 || n = 2024)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 2022)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
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
.elseif(n = 2028)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2041 || n = 2043 || n = 2047 || n = 2050 || n = 2059 || n = 2061 || n = 2065 || n = 2068 || n = 2070)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2033 || n = 2045 || n = 2069)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2034)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2035 || n = 2049 || n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2036)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jmi *-5
	tay
.elseif(n = 2037)
	; 10 bytes
	tya
	ldy #(n-5)/8
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 2038)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 5) && ((n > 2038 && n < 2067)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2040 || n = 2051 || n = 2062)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2042)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 2044)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2046 || n = 2052 || n = 2055 || n = 2056 || n = 2064)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2053)
	; 10 bytes
	tya
	ldy #0
	jmp *+3
	dey
	Jne *-4
	tay
.elseif(n = 2054)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2058)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
	tay
.elseif(n = 2060)
	; 11 bytes
	php
	tya
	ldy #0
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 2067)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	plp
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2077 || n = 2083 || n = 2086 || n = 2104 || n = 2107 || n = 2108)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2072)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2073)
	; 9 bytes
	tya
	ldy #<$8D ;hides 'sta $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
.elseif(n = 2074)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 5) && ((n > 2074 && n < 2103)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2076 || n = 2088 || n = 2089 || n = 2098 || n = 2100)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2078 || n = 2085 || n = 2087 || n = 2094)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2079)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 2080)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2081 || n = 2105)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2082)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 2090)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2091)
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2092)
	; 12 bytes
	pha
	lda $A5
	delay_n (n-10)
	pla
.elseif(n = 2095 || n = 2106)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2096)
	; 10 bytes
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2097)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
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
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2103)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	iny
	Jne *-6
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
.if(n = 2110 || n = 2146 || n = 2182)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2184)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2123 || n = 2132 || n = 2144 || n = 2159 || n = 2186)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2113 || n = 2115 || n = 2122 || n = 2131 || n = 2140 || n = 2142 || n = 2143 || n = 2149 || n = 2151 || n = 2155 || n = 2157 || n = 2158 || n = 2166 || n = 2167 || n = 2169 || n = 2173 || n = 2176 || n = 2178 || n = 2179 || n = 2185 || n = 2187)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2116 || n = 2118 || n = 2124 || n = 2127 || n = 2136 || n = 2148 || n = 2154 || n = 2160 || n = 2184)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2117 || n = 2141 || n = 2153 || n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2119 || n = 2133 || n = 2175)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2121)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2125)
	; 10 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	nop
	iny
	Jne *-3
	pla
	tay
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
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2130 || n = 2164 || n = 2181)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
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
.elseif(n = 2135 || n = 2180)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2137)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2145)
	; 9 bytes
	tya
	ldy #<$8D ;hides 'sta $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.elseif(n = 2152 || n = 2170)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
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
.elseif(n = 2163)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	jmp *+3
	dey
	Jne *-5
	tay
.elseif(n = 2168)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2171)
	; 11 bytes
	tya
	ldy #(n-24)/19
	delay_n 14
	dey
	Jpl *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if(n = 2188 || n = 2197 || n = 2206 || n = 2224 || n = 2233)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2189 || n = 2213 || n = 2225 || n = 2249 || n = 2261)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2190 || n = 2196 || n = 2199 || n = 2208 || n = 2220 || n = 2226 || (n > 2241 && n < 2245) || n = 2253 || n = 2256 || n = 2262)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2191 || n = 2193 || n = 2207 || n = 2212 || n = 2214 || n = 2215 || n = 2221 || n = 2223 || n = 2229 || n = 2230 || n = 2239 || n = 2241 || n = 2248 || n = 2250 || n = 2251 || n = 2257 || n = 2263 || n = 2265)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 5) && ((n > 2191 && n < 2265)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif((n .mod 11 = 5) && ((n > 2193 && n < 2239) || n = 2260))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2195)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 11
	Jne *-5
	tay
.elseif(n = 2198)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
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
.elseif(n = 2202)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2203)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2204 || n = 2222 || n = 2258)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2209)
	; 11 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 2211)
	; 11 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2217)
	; 9 bytes
	tya
	ldy #<$8D ;hides 'sta $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
.elseif(n = 2218 || n = 2254)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2231 || n = 2245 || n = 2259)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2232)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
.elseif(n = 2235)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2236)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2240)
	; 10 bytes
	php
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2247)
	; 11 bytes
	tya
	ldy #(n-24)/19
	delay_n 14
	dey
	Jpl *-5
	tay
.elseif(n = 2252)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	plp
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2275 || n = 2277 || n = 2284 || n = 2286 || n = 2299 || n = 2302)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267 || n = 2274 || n = 2276 || n = 2294 || n = 2303)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2268 || n = 2280 || n = 2292 || n = 2298)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2269)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2270)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2271 || n = 2293 || n = 2304)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2272)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif((n .mod 9 = 5) && ((n > 2272 && n < 2301)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2278)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2279)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	plp
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
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2285 || n = 2297)
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
.elseif(n = 2288)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2289)
	; 9 bytes
	tya
	ldy #<$8D ;hides 'sta $02A0'
	ldy #2
	iny
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
.elseif(n = 2295)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2296)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	plp
.endif
.else
; 2305 to 2343
.if(n = 2305)
	; 11 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2306)
	; 11 bytes
	php
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	plp
.elseif(n = 2307 || n = 2308 || n = 2316 || n = 2322 || n = 2328 || n = 2340)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2309)
	; 9 bytes
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2320)
	; 12 bytes
	php
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2311 || n = 2313 || n = 2318 || n = 2323 || n = 2325 || n = 2327 || n = 2331 || n = 2335 || n = 2339)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2312 || n = 2324 || n = 2336)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 2315 || n = 2326 || n = 2337)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2317 || n = 2334)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2319)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2321 || n = 2333)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2329 || n = 2343)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2330)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2332)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
.elseif(n = 2338)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2341)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2342)
	; 11 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344 || n = 2352 || n = 2364 || n = 2366 || (n > 2375 && n < 2379) || n = 2386 || n = 2388 || n = 2400 || n = 2406 || n = 2412)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2418)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2346 || n = 2382 || n = 2396 || n = 2408)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2347 || n = 2349 || n = 2354 || n = 2361 || n = 2367 || n = 2372 || n = 2383 || n = 2387 || n = 2394 || n = 2395 || n = 2401 || n = 2404 || n = 2407 || n = 2409 || n = 2411 || n = 2415 || n = 2416 || n = 2419 || n = 2421)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2348 || n = 2359 || n = 2370 || n = 2392 || n = 2403 || n = 2414)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2350)
	; 11 bytes
	php
	tya
	ldy #234 ;hides 'nop'
	bit $24
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2351 || n = 2368 || n = 2402)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2353)
	; 11 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2355 || n = 2365)
	; 11 bytes
	tya
	ldy #(n-5)/10
	nop
	jmp *+3
	dey
	Jne *-5
	tay
.elseif(n = 2356)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2358 || n = 2397)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2360 || n = 2375 || n = 2390 || n = 2420)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2362 || n = 2374 || n = 2398 || n = 2410)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2363)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	plp
.elseif((n .mod 14 = 5) && ((n > 2370 && n < 2414)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2373)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2379)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2380 || n = 2418)
	; 11 bytes
	tya
	ldy #(n-24)/19
	delay_n 14
	dey
	Jpl *-5
	tay
.elseif(n = 2384)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2389)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2391)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	jmp *+3
	dey
	dey
	Jne *-6
	tay
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2482 || n = 2494)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2423 || n = 2449 || n = 2462 || n = 2475)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2424 || n = 2446 || n = 2448 || n = 2454 || n = 2460 || n = 2472 || n = 2476 || n = 2484 || n = 2496 || n = 2498)
	; 10..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 11 = 5) && ((n > 2424 && n < 2492)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2426 || n = 2431 || n = 2433 || n = 2438 || n = 2443 || n = 2445 || n = 2459 || n = 2464 || n = 2467 || n = 2471 || n = 2473 || n = 2474 || n = 2479 || n = 2481 || n = 2486 || n = 2493 || n = 2499)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427 || n = 2455 || n = 2483 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428 || n = 2432 || n = 2442 || n = 2444 || n = 2456 || n = 2461 || n = 2468 || n = 2492)
	; 10..12 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 12 = 5) && ((n > 2428 && n < 2490)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2430 || n = 2440 || n = 2500)
	; 12 bytes
	php
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2435 || n = 2450 || n = 2495)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2437 || n = 2485)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
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
.elseif(n = 2452)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2457)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2463)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	plp
.elseif(n = 2466 || n = 2478)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 2470 || n = 2487)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2488)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	bit $24
	sbc #1
	Jne *-5
	plp
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
.elseif((n .mod 11 = 5) && (n = 2502 || (n > 2523 && n < 2569)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2503 || n = 2505 || n = 2515 || n = 2517 || n = 2522 || n = 2523 || n = 2526 || n = 2527 || n = 2529 || n = 2534 || n = 2543 || n = 2548 || n = 2551 || n = 2559 || n = 2563 || n = 2565 || n = 2569 || n = 2574 || n = 2575)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2521 || n = 2538 || n = 2572)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 10) && ((n > 2505 && n < 2579)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2507)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	plp
.elseif(n = 2508 || n = 2509 || n = 2520 || n = 2532 || n = 2556 || n = 2562)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2510 || n = 2540 || n = 2570)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2511 || n = 2539 || n = 2553 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2512)
	; 12 bytes
	php
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2514)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2516 || n = 2528 || n = 2536 || n = 2547 || n = 2552 || n = 2558 || n = 2564 || n = 2571 || n = 2576)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2519)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
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
	; 10 bytes
	tya
	ldy #<$8D ;hides 'sta $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2541)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2544)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
.elseif(n = 2545)
	; 11 bytes
	tya
	ldy #(n-5)/10
	nop
	jmp *+3
	dey
	Jne *-5
	tay
.elseif(n = 2550)
	; 12 bytes
	php
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	plp
.elseif(n = 2555)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2560)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	plp
.elseif(n = 2577)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.else
; 2579 to 2656
.if((n .mod 11 = 5) && ((n > 2578 && n < 2635) || n = 2656))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2584 || n = 2592 || n = 2604 || n = 2610 || n = 2616 || n = 2619 || n = 2628 || n = 2641 || n = 2652)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2581 || n = 2595 || n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2582)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2583 || n = 2587 || n = 2591 || n = 2596 || n = 2599 || n = 2611 || n = 2613 || n = 2620 || n = 2625 || n = 2632 || n = 2635 || n = 2636 || n = 2639 || n = 2647 || n = 2649 || (n > 2652 && n < 2656))
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 5) && ((n > 2584 && n < 2646)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2586)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
	plp
.elseif(n = 2588 || n = 2593 || n = 2598 || n = 2624 || n = 2643 || n = 2648)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2589 || n = 2640)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2594)
	; 11 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2600 || n = 2615 || n = 2630)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif((n .mod 12 = 10) && ((n > 2601 && n < 2651)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2603)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif((n .mod 13 = 5) && ((n > 2604 && n < 2645)))
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2606)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
	tay
.elseif(n = 2607)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	tay
.elseif(n = 2608 || n = 2627 || n = 2646)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2617)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	plp
.elseif(n = 2622)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 2629)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2642)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	tax
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if((n .mod 12 = 5) && ((n > 2656 && n < 2730)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2658 || n = 2659 || n = 2661 || n = 2666 || n = 2671 || n = 2673 || n = 2680 || n = 2682 || n = 2683 || n = 2685 || n = 2687 || n = 2692 || n = 2695 || n = 2697 || n = 2699 || n = 2702 || n = 2704 || n = 2713 || n = 2715 || n = 2719 || n = 2723 || n = 2726 || n = 2727)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675 || n = 2690 || n = 2720)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2662 || n = 2686 || n = 2698 || n = 2710 || n = 2734)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2663 || n = 2664 || n = 2676 || n = 2688 || n = 2712 || n = 2714 || n = 2718 || n = 2724 || n = 2728)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2665 || n = 2679 || n = 2707 || n = 2721)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif((n .mod 11 = 5) && ((n > 2666 && n < 2734)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2668 || n = 2672 || n = 2684 || n = 2732)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2670 || n = 2696)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2674 || n = 2691 || n = 2708 || n = 2725)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2677)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2694 || n = 2716)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	plp
.elseif(n = 2701)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 2703)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2706)
	; 12 bytes
	php
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2709)
	; 10 bytes
	tya
	ldy #<$8D ;hides 'sta $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2730)
	; 12 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	iny ;first loop only
	pla
	pha
	iny
	Jne *-5
	pla
	tay
.elseif(n = 2731)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.else
; 2735 to 2812
.if(n = 2735 || n = 2749 || n = 2763 || n = 2791 || n = 2805)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2736 || n = 2738 || n = 2740 || n = 2748 || n = 2760 || n = 2762 || n = 2772 || n = 2784 || n = 2796 || n = 2808)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2737 || n = 2739 || n = 2743 || n = 2745 || n = 2751 || n = 2752 || n = 2757 || (n > 2766 && n < 2770) || n = 2778 || n = 2779 || n = 2781 || n = 2786 || n = 2790 || n = 2807 || n = 2809 || n = 2812)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 5) && ((n > 2740 && n < 2802)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2742 || n = 2759 || n = 2776 || n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2744 || n = 2755 || n = 2766 || n = 2788 || n = 2799 || n = 2810)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 10) && ((n > 2745 && n < 2807)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2747)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 6
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2750 || n = 2780 || n = 2795)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2754)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	tay
.elseif(n = 2756 || n = 2792 || n = 2802 || n = 2804)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 13 = 5) && ((n > 2760 && n < 2801)))
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2764)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2771)
	; 12 bytes
	php
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2775)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jmp *+3
	dex
	dex
	Jne *-6
	tax
.elseif(n = 2783)
	; 12 bytes
	php
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
	plp
.elseif(n = 2785)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jmi *-7
	tay
.elseif(n = 2797)
	; 10 bytes
	tya
	ldy #<$8D ;hides 'sta $02A0'
	ldy #2
	nop
	iny
	Jne *-5
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
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 6
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
; 2813 to 2968
.if((n .mod 12 = 5) && ((n > 2812 && n < 2958)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2814 || n = 2815 || n = 2817 || n = 2822 || n = 2823 || n = 2827 || n = 2829 || n = 2831 || n = 2834 || n = 2835 || n = 2839 || n = 2841 || n = 2843 || n = 2846 || n = 2851 || n = 2853 || n = 2857 || n = 2858 || n = 2863 || n = 2871 || n = 2872 || n = 2879 || n = 2887 || n = 2894 || n = 2899 || n = 2901 || (n > 2904 && n < 2908) || n = 2911 || n = 2913 || n = 2918 || n = 2919 || n = 2923 || n = 2925 || n = 2927 || n = 2932 || n = 2935 || n = 2937 || n = 2939 || n = 2942 || n = 2947 || n = 2949 || n = 2954 || n = 2958 || n = 2967)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2848 || n = 2850 || n = 2864 || n = 2876 || n = 2881 || n = 2888 || n = 2898 || n = 2920 || n = 2924 || n = 2934 || n = 2936 || n = 2948)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 12 = 10) && ((n > 2817 && n < 2867) || (n > 2889 && n < 2963)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 14 = 5) && ((n > 2818 && n < 2848) || (n > 2874 && n < 2932) || n = 2959))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2820 || n = 2832 || n = 2852 || n = 2856 || n = 2868 || n = 2880 || n = 2882 || n = 2892 || n = 2896 || n = 2904 || n = 2910 || n = 2916 || n = 2928 || n = 2940 || n = 2952 || n = 2953 || n = 2964 || n = 2966)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2821)
	; 10 bytes
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2824 || n = 2838 || n = 2908 || n = 2922)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2826 || n = 2865 || n = 2891 || n = 2943 || n = 2956)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2836 || n = 2874 || n = 2893)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2840 || n = 2855 || n = 2870 || n = 2900 || n = 2915 || n = 2930 || n = 2960)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2844 || n = 2878 || n = 2895 || n = 2912 || n = 2929 || n = 2946 || n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2845)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
.elseif(n = 2859)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $12A0'
	ldy #18
	jmp *+3
	iny
	Jne *-7
	tay
.elseif(n = 2860 || n = 2862 || n = 2884 || n = 2886 || n = 2941 || n = 2944)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 2869 || n = 2965)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2877 || n = 2961)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2883 || n = 2968)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2951)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 2955)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $0AA0'
	ldy #10
	jmp *+3
	iny
	Jne *-7
	tay
.endif
.else
; 2969 to 3125
.if((n .mod 12 = 5) && ((n > 2968 && n < 3066)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2970 || n = 2976 || n = 2988 || n = 2994 || n = 3000 || n = 3012 || n = 3024 || n = 3034 || n = 3036 || n = 3052 || n = 3060 || n = 3072 || n = 3078 || n = 3084 || n = 3092 || n = 3106 || n = 3120)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2971 || n = 2977 || n = 2978 || n = 2983 || n = 2985 || n = 2989 || n = 2991 || n = 2995 || n = 2999 || n = 3002 || n = 3007 || n = 3009 || n = 3011 || n = 3016 || n = 3023 || n = 3028 || n = 3031 || n = 3033 || n = 3037 || n = 3038 || n = 3049 || n = 3055 || n = 3059 || n = 3062 || n = 3063 || n = 3066 || n = 3067 || n = 3069 || n = 3074 || n = 3075 || n = 3079 || n = 3081 || (n > 3086 && n < 3090) || n = 3101 || n = 3103 || n = 3111 || n = 3114 || n = 3115 || n = 3117 || n = 3118 || n = 3123)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2984 || n = 2996 || n = 3004 || n = 3018 || n = 3030 || n = 3044 || n = 3051 || n = 3056 || n = 3068)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 14 = 5) && ((n > 2972 && n < 3016) || (n > 3042 && n < 3114)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 10) && ((n > 2973 && n < 3023) || (n > 3045 && n < 3071)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 15 = 5) && (n = 2975 || n = 2990 || (n > 3019 && n < 3051) || (n > 3079 && n < 3126)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2979)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	jmp *+3
	dey
	Jne *-6
	tay
.elseif(n = 2980 || n = 2997 || n = 3014 || n = 3082 || n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2982 || n = 3008 || n = 3021 || n = 3047 || n = 3073 || n = 3086 || n = 3112)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2992 || n = 3076 || n = 3090 || n = 3104)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3003)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3006)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3013 || n = 3061 || n = 3093 || n = 3109)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3019)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3025 || n = 3040 || n = 3042 || n = 3054 || n = 3091 || n = 3094 || n = 3096 || n = 3098 || n = 3100 || n = 3105 || n = 3107 || n = 3108 || n = 3122 || n = 3124)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3026 || n = 3064 || n = 3083 || n = 3102 || n = 3121)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3027)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3032)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3039)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3045)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3048)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
	tax
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3097)
	; 11 bytes
	tya
	ldy #<$8D ;hides 'sta $12A0'
	ldy #18
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3119)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126 || n = 3128 || n = 3139 || n = 3147 || n = 3156 || n = 3163 || n = 3165 || n = 3168 || n = 3181 || n = 3182 || n = 3193 || n = 3195 || n = 3196 || n = 3198 || n = 3212 || n = 3219 || n = 3224 || n = 3226 || n = 3233 || n = 3236 || (n > 3247 && n < 3251) || n = 3261 || (n > 3263 && n < 3267) || n = 3278 || n = 3280)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif((n .mod 14 = 5) && ((n > 3126 && n < 3282)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3129 || n = 3131 || n = 3135 || n = 3142 || n = 3143 || n = 3145 || n = 3152 || n = 3153 || n = 3161 || n = 3166 || n = 3171 || n = 3172 || n = 3175 || n = 3179 || n = 3180 || n = 3186 || n = 3192 || n = 3199 || n = 3207 || n = 3213 || n = 3217 || n = 3220 || n = 3222 || n = 3223 || n = 3231 || n = 3241 || n = 3247 || n = 3257 || n = 3262 || n = 3270 || n = 3271 || n = 3277)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3130 || n = 3136 || n = 3144 || n = 3158 || n = 3206 || n = 3214 || n = 3228 || n = 3238)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 14 = 10) && ((n > 3131 && n < 3203) || n = 3244 || n = 3272))
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3133 || n = 3150 || n = 3167 || n = 3184 || n = 3201 || n = 3218 || n = 3235 || n = 3252 || n = 3269)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3134 || n = 3148 || n = 3162 || n = 3176 || n = 3194 || n = 3204 || n = 3208 || n = 3210 || n = 3232 || n = 3234 || n = 3246 || n = 3258 || n = 3259 || n = 3274 || n = 3276)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3137 || n = 3191 || n = 3209 || n = 3263)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif((n .mod 13 = 5) && ((n > 3137 && n < 3191) || (n > 3215 && n < 3269)))
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif((n .mod 15 = 5) && (n = 3140 || (n > 3169 && n < 3276)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3149)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3154)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 18
	Jne *-7
	tay
.elseif((n .mod 16 = 5) && ((n > 3156 && n < 3238)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3159 || n = 3178 || n = 3254 || n = 3273)
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
.elseif(n = 3227)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3240)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3243)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 8
	dey
	Jne *-6
	tay
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
.if(n = 3282 || n = 3289 || n = 3298 || n = 3312 || n = 3340 || n = 3357)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3283 || n = 3287 || n = 3288 || n = 3297 || n = 3299 || n = 3319 || n = 3322 || n = 3324 || n = 3325 || n = 3327 || n = 3332 || n = 3339 || n = 3341 || n = 3352 || n = 3353 || n = 3355 || n = 3356)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3286 || n = 3303 || n = 3320 || n = 3354)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3290 || n = 3305 || n = 3335 || n = 3350)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3291)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts
	Jne *-6
	tay
.elseif(n = 3292 || n = 3311 || n = 3330)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3293 || n = 3302 || n = 3310 || n = 3316 || n = 3344 || n = 3358)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3294 || n = 3307)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif((n .mod 14 = 5) && ((n > 3294 && n < 3352)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3296)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3300 || n = 3314 || n = 3328 || n = 3342)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
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
.elseif(n = 3308 || n = 3359)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3313)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3315 || n = 3318 || n = 3321 || n = 3329 || n = 3331 || n = 3334 || n = 3336 || n = 3338 || n = 3343 || (n > 3344 && n < 3349))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3326)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	plp
.elseif(n = 3333)
	; 11 bytes
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
.endif
.else
; 3360 to 3437
.if(n = 3360 || (n > 3361 && n < 3365) || n = 3366 || n = 3377 || n = 3385 || n = 3394 || n = 3401 || n = 3417 || n = 3419 || n = 3420 || n = 3430 || n = 3432 || n = 3433 || n = 3436)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3361 || n = 3372 || n = 3378 || n = 3386 || n = 3400 || n = 3414 || n = 3428)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 14 = 5) && ((n > 3364 && n < 3436)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3367 || n = 3369 || n = 3373 || n = 3375 || n = 3382 || n = 3390 || n = 3392 || n = 3399 || n = 3408 || n = 3409 || n = 3411 || n = 3415 || n = 3423 || n = 3424 || n = 3427 || n = 3437)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3368 || n = 3387 || n = 3406)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif((n .mod 14 = 10) && ((n > 3369 && n < 3427)))
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3371 || n = 3388 || n = 3405 || n = 3422)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3374 || n = 3391 || n = 3396)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3376)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif((n .mod 15 = 5) && ((n > 3379 && n < 3426)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif((n .mod 16 = 5) && ((n > 3380 && n < 3430)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3383)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3389)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3402)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3403)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
.elseif(n = 3404)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	inx
	inx
	Jne *-7
	tax
.elseif(n = 3416)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3418)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3431)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3434)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3452 || n = 3457 || n = 3462 || n = 3476 || n = 3478 || n = 3494 || n = 3508)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3439 || n = 3456 || n = 3473 || n = 3490 || n = 3507)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif((n .mod 15 = 5) && ((n > 3439 && n < 3516)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3441 || n = 3447 || n = 3451 || n = 3453 || n = 3458 || n = 3460 || n = 3465 || n = 3467 || n = 3472 || n = 3479 || n = 3481 || n = 3487 || n = 3492 || n = 3502 || n = 3503 || n = 3511)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3442 || n = 3466 || n = 3482 || n = 3484 || n = 3498 || n = 3512 || n = 3514)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3443 || n = 3497)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3444 || n = 3501)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3445 || n = 3461 || n = 3493 || n = 3509)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3446)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3448 || n = 3450 || n = 3469 || n = 3471 || n = 3474 || n = 3486 || n = 3488 || n = 3489 || n = 3499)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif((n .mod 14 = 5) && ((n > 3448 && n < 3506)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3454)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
.elseif(n = 3459)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3464)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3468)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3475)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3480)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3483)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3495)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3496 || n = 3510)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3504)
	; 12 bytes
	php
	sec
	lda #184 ;hides 'clv'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 3506)
	; 12 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	plp
.elseif(n = 3513)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3518 || n = 3534 || n = 3542 || n = 3553 || n = 3555 || n = 3556 || n = 3567 || (n > 3569 && n < 3573) || n = 3574 || n = 3576 || n = 3584 || n = 3586 || n = 3588)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3517 || n = 3521 || n = 3523 || (n > 3525 && n < 3529) || n = 3532 || n = 3535 || n = 3537 || n = 3543 || n = 3549 || n = 3551 || n = 3559 || n = 3562 || n = 3563 || n = 3565 || n = 3591 || n = 3593)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 5) && ((n > 3518 && n < 3576)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3520 || n = 3539 || n = 3577)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3522 || n = 3536 || n = 3544 || n = 3550 || n = 3564 || n = 3578)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3524 || n = 3541 || n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3525 || n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3529 || n = 3546)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3530 || n = 3545 || n = 3560 || n = 3590)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3531 || n = 3540 || n = 3548 || n = 3554 || n = 3568 || n = 3582)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3538 || n = 3552 || n = 3566 || n = 3580)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3569 || n = 3587)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3579)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3581)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3583)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
	tax
.elseif(n = 3585)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
.elseif(n = 3589)
	; 10 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.endif
.else
; 3594 to 3750
.if(n = 3594 || n = 3598 || n = 3607 || n = 3611 || n = 3613 || n = 3622 || n = 3628 || n = 3630 || n = 3636 || n = 3639 || n = 3645 || n = 3647 || n = 3652 || n = 3655 || n = 3662 || n = 3664 || n = 3667 || n = 3671 || n = 3674 || n = 3679 || n = 3681 || n = 3682 || n = 3687 || n = 3693 || (n > 3695 && n < 3699) || n = 3703 || n = 3712 || n = 3713 || n = 3719 || n = 3727 || n = 3730 || n = 3732 || n = 3735 || n = 3742 || n = 3750)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3612 || n = 3629 || n = 3646 || n = 3663 || n = 3702 || n = 3714 || n = 3718)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3596 || n = 3615 || n = 3634 || n = 3672 || n = 3691 || n = 3729 || n = 3748)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3597 || n = 3614 || n = 3631 || n = 3648 || n = 3716)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3599 || n = 3616 || n = 3633 || n = 3684 || n = 3706 || n = 3722)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n > 3599 && n < 3603) || n = 3604 || n = 3606 || n = 3608 || n = 3610 || n = 3618 || n = 3619 || n = 3624 || n = 3625 || n = 3627 || n = 3632 || n = 3640 || n = 3642 || n = 3644 || n = 3649 || n = 3651 || n = 3654 || n = 3656 || n = 3658 || n = 3661 || n = 3666 || n = 3668 || n = 3670 || n = 3673 || n = 3676 || n = 3678 || n = 3686 || (n > 3687 && n < 3691) || n = 3692 || n = 3700 || n = 3704 || n = 3705 || (n > 3706 && n < 3710) || n = 3721 || n = 3724 || n = 3726 || n = 3734 || (n > 3735 && n < 3740) || n = 3741 || n = 3744 || n = 3746)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 11
	iny
	Jne *-6
	tay
.elseif((n .mod 15 = 5) && ((n > 3604 && n < 3741)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3609 || n = 3626 || n = 3643 || n = 3660 || n = 3677 || n = 3694 || n = 3711 || n = 3728 || n = 3745)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3617 || n = 3638 || n = 3743)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif((n .mod 16 = 5) && ((n > 3620 && n < 3750)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3623 || n = 3641 || n = 3659 || n = 3731)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3657)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
	tax
.elseif(n = 3675)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.elseif(n = 3683)
	; 11 bytes
	tya
	ldy #<$8D ;hides 'sta $1AA0'
	ldy #26
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3720)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 3723)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 8
	dey
	Jne *-6
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
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
; 3751 to 3906
.if(n = 3751 || n = 3757 || n = 3764 || n = 3766 || n = 3769 || n = 3772 || n = 3783 || n = 3787 || n = 3788 || n = 3798 || n = 3799 || n = 3802 || n = 3807 || n = 3817 || n = 3826 || n = 3831 || n = 3832 || n = 3834 || n = 3849 || n = 3851 || n = 3863 || n = 3866 || n = 3868 || n = 3879 || n = 3883 || n = 3885 || n = 3895 || n = 3902)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3752)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3753 || n = 3756 || (n > 3757 && n < 3761) || n = 3763 || n = 3768 || n = 3774 || n = 3775 || n = 3777 || n = 3778 || n = 3780 || n = 3789 || n = 3790 || (n > 3791 && n < 3796) || n = 3804 || n = 3808 || n = 3809 || n = 3812 || n = 3814 || n = 3819 || n = 3822 || n = 3823 || n = 3825 || n = 3828 || n = 3836 || n = 3838 || (n > 3839 && n < 3843) || n = 3844 || n = 3846 || n = 3853 || n = 3855 || n = 3856 || n = 3858 || n = 3859 || n = 3865 || n = 3870 || (n > 3871 && n < 3875) || n = 3876 || n = 3878 || n = 3880 || n = 3882 || n = 3887 || n = 3889 || n = 3892 || n = 3894 || n = 3896 || n = 3897 || n = 3899 || n = 3904 || n = 3906)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 3754 || n = 3810 || n = 3820 || n = 3837 || n = 3854 || n = 3867 || n = 3871 || n = 3888 || n = 3905)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 15 = 5) && ((n > 3754 && n < 3816)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3761)
	; 12 bytes
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
.elseif(n = 3762 || n = 3779 || n = 3796 || n = 3813 || n = 3830 || n = 3847 || n = 3864 || n = 3881 || n = 3898)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3765 || n = 3781 || n = 3797 || n = 3829 || n = 3861 || n = 3877 || n = 3893)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3767 || n = 3786 || n = 3805 || n = 3824 || n = 3843 || n = 3862 || n = 3900)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3771)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3773)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 3776)
	; 12 bytes
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
.elseif(n = 3782 || n = 3816 || n = 3833 || n = 3850 || n = 3884 || n = 3901)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3784)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	rol $26
	ror $26
	dey
	dey
	Jne *-7
	tay
.elseif(n = 3791)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	pla
	pha
	iny
	Jne *-5
	pla
	tay
.elseif(n = 3801 || n = 3818 || n = 3835 || n = 3852 || n = 3886 || n = 3903)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3803)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 3806 || n = 3827 || n = 3848 || n = 3869 || n = 3890)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 3811)
	; 11 bytes
	tya
	ldy #<$8D ;hides 'sta $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3821 || n = 3839 || n = 3857 || n = 3875)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3845)
	; 11 bytes
	tya
	ldy #0
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3860)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
.elseif(n = 3891)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 11
	dey
	Jne *-6
	tay
.endif
.else
; 3907 to 4062
.if(n = 3907 || n = 3908 || n = 3910 || (n > 3911 && n < 3915) || n = 3916 || n = 3923 || n = 3924 || n = 3926 || n = 3928 || n = 3931 || n = 3933 || n = 3942 || (n > 3943 && n < 3947) || n = 3948 || n = 3950 || n = 3955 || n = 3958 || (n > 3959 && n < 3965) || n = 3967 || n = 3972 || n = 3977 || n = 3979 || n = 3980 || n = 3982 || n = 3984 || (n > 3991 && n < 3995) || n = 3996 || n = 3998 || n = 3999 || n = 4006 || (n > 4007 && n < 4013) || n = 4015 || n = 4018 || (n > 4024 && n < 4033) || n = 4040 || (n > 4041 && n < 4051) || n = 4057 || n = 4059 || n = 4060 || n = 4062)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif((n .mod 16 = 5) && ((n > 3908 && n < 4054)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3911 || n = 3929 || n = 3965 || n = 4001 || n = 4055)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3915 || n = 3932 || n = 3949 || n = 3966 || n = 3983 || n = 4000 || n = 4017 || n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3917 || n = 3921 || n = 3927 || n = 3934 || n = 3936 || n = 3940 || n = 3943 || n = 3951 || n = 3959 || n = 3968 || n = 3970 || n = 3975 || n = 3985 || n = 3987 || n = 3991 || n = 3997 || n = 4002 || n = 4004 || n = 4007 || n = 4019 || n = 4023 || n = 4035 || n = 4036 || n = 4038 || n = 4039 || n = 4054)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3918 || n = 3935 || n = 3952 || n = 3969 || n = 3986 || n = 4020)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3919 || n = 3938 || n = 3976 || n = 3995 || n = 4014 || n = 4033 || n = 4052)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3920 || n = 3937 || n = 3954 || n = 3988 || n = 4022 || n = 4056)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3922 || n = 3956 || n = 3978 || n = 3990 || n = 4024 || n = 4041)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3930)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
	tax
.elseif(n = 3939)
	; 11 bytes
	tya
	ldy #<$8D ;hides 'sta $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3947)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
.elseif(n = 3953 || n = 3974 || n = 4016 || n = 4058)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 3971)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3981)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.elseif(n = 4003)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 13
	iny
	Jne *-7
	tay
.elseif(n = 4013 || n = 4061)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
; 4063 to 4218
.if(n = 4063)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 8
	Jne *-7
	tax
.elseif((n > 4063 && n < 4067) || n = 4074 || (n > 4075 && n < 4079) || (n > 4079 && n < 4085) || n = 4086 || (n > 4092 && n < 4100) || n = 4108 || n = 4113 || (n > 4114 && n < 4119) || n = 4125 || n = 4129 || n = 4131 || n = 4132 || n = 4134 || n = 4135 || n = 4137 || n = 4144 || n = 4146 || n = 4148 || (n > 4149 && n < 4153) || n = 4154 || n = 4159 || n = 4162 || n = 4164 || n = 4165 || n = 4167 || n = 4169 || n = 4171 || n = 4176 || n = 4178 || n = 4180 || n = 4183 || n = 4186 || n = 4188 || n = 4193 || (n > 4194 && n < 4199) || (n > 4199 && n < 4203) || n = 4210 || (n > 4211 && n < 4215) || n = 4216 || n = 4218)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4067)
	; 11 bytes
	tya
	ldy #<$8D ;hides 'sta $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4068 || n = 4085 || n = 4102 || n = 4119 || n = 4136 || n = 4153 || n = 4170 || n = 4187 || n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4069)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 4070 || n = 4072 || n = 4087 || n = 4089 || n = 4092 || n = 4103 || n = 4104 || n = 4106 || n = 4111 || n = 4121 || n = 4123 || n = 4130 || n = 4138 || n = 4140 || n = 4149 || n = 4155 || n = 4157 || n = 4168 || n = 4172 || n = 4174 || n = 4189 || n = 4191 || n = 4206 || n = 4208)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4071 || n = 4090 || n = 4109 || n = 4128 || n = 4147 || n = 4166 || n = 4185)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4073 || n = 4091 || n = 4127 || n = 4145 || n = 4163 || n = 4181 || n = 4199 || n = 4217)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4075 || n = 4114 || n = 4126 || n = 4160 || n = 4177 || n = 4194 || n = 4211)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4079 || n = 4100 || n = 4142 || n = 4184 || n = 4205)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4088 || n = 4105 || n = 4110 || n = 4122 || n = 4139 || n = 4156 || n = 4173 || n = 4190 || n = 4207)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4101)
	; 11 bytes
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 4107)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4112)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4120)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	tax
.elseif(n = 4124 || n = 4141 || n = 4158 || n = 4175 || n = 4192 || n = 4209)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4133)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4143)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $1AA0'
	ldy #26
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4161)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4179)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 11
	iny
	Jne *-6
	tay
.elseif(n = 4182)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts
	dey
	dey
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
	delay_n 11
	dey
	Jne *-6
	tay
.endif
.else
; 4219 to 4375
.if(n = 4219)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.elseif(n = 4220 || n = 4222 || n = 4227 || (n > 4229 && n < 4235) || n = 4236 || n = 4237 || n = 4239 || n = 4246 || (n > 4247 && n < 4253) || n = 4254 || n = 4256 || n = 4264 || n = 4265 || n = 4267 || n = 4269 || n = 4270 || n = 4273 || n = 4278 || n = 4281 || (n > 4282 && n < 4287) || n = 4288 || n = 4290 || n = 4295 || n = 4297 || n = 4298 || n = 4300 || (n > 4301 && n < 4306) || n = 4312 || (n > 4313 && n < 4318) || n = 4319 || n = 4321 || n = 4322 || n = 4324 || n = 4329 || (n > 4331 && n < 4337) || n = 4338 || n = 4341 || n = 4346 || n = 4348 || n = 4350 || n = 4351 || (n > 4352 && n < 4356) || n = 4363 || (n > 4364 && n < 4373) || n = 4374)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4221 || n = 4238 || n = 4255 || n = 4272 || n = 4289 || n = 4306 || n = 4323 || n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4223 || n = 4242 || n = 4261 || n = 4280 || n = 4299 || n = 4318 || n = 4337 || n = 4356 || n = 4375)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4224 || n = 4241 || n = 4258 || n = 4275 || n = 4292 || n = 4309 || n = 4326 || n = 4360)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4225 || n = 4240 || n = 4244 || n = 4257 || n = 4259 || n = 4263 || n = 4274 || n = 4276 || n = 4282 || n = 4291 || n = 4293 || n = 4301 || n = 4308 || n = 4320 || n = 4325 || n = 4327 || n = 4339 || n = 4342 || n = 4344 || n = 4358 || n = 4359)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4226 || n = 4247 || n = 4268 || n = 4310 || n = 4331 || n = 4352 || n = 4373)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4228 || n = 4245 || n = 4262 || n = 4279 || n = 4296 || n = 4313 || n = 4330 || n = 4347 || n = 4364)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4229 || n = 4349)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4235 || n = 4253 || n = 4271 || n = 4307 || n = 4343 || n = 4361)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4243 || n = 4260 || n = 4277 || n = 4294 || n = 4311 || n = 4328 || n = 4345)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4266)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
	tax
.elseif(n = 4287)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $12A0'
	ldy #18
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4357)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4362)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
; 4376 to 4687
.if(n = 4376 || n = 4378 || (n > 4379 && n < 4392) || n = 4393 || n = 4395 || (n > 4397 && n < 4411) || n = 4412 || n = 4417 || n = 4419 || n = 4420 || (n > 4421 && n < 4431) || n = 4435 || (n > 4436 && n < 4445) || (n > 4445 && n < 4449) || n = 4450 || n = 4455 || (n > 4457 && n < 4467) || n = 4468 || n = 4471 || (n > 4472 && n < 4478) || (n > 4478 && n < 4487) || n = 4488 || n = 4490 || n = 4492 || (n > 4493 && n < 4499) || (n > 4499 && n < 4505) || n = 4506 || n = 4507 || n = 4509 || (n > 4510 && n < 4517) || n = 4518 || n = 4519 || n = 4521 || n = 4522 || (n > 4523 && n < 4527) || n = 4528 || (n > 4529 && n < 4539) || n = 4540 || (n > 4541 && n < 4546) || n = 4547 || (n > 4548 && n < 4559) || n = 4560 || n = 4561 || n = 4563 || n = 4564 || n = 4566 || (n > 4567 && n < 4575) || n = 4576 || n = 4578 || n = 4579 || n = 4581 || n = 4582 || n = 4585 || n = 4587 || n = 4588 || (n > 4589 && n < 4595) || (n > 4595 && n < 4603) || (n > 4605 && n < 4613) || (n > 4613 && n < 4622) || (n > 4625 && n < 4637) || (n > 4637 && n < 4641) || n = 4642 || n = 4644 || n = 4645 || (n > 4646 && n < 4660) || (n > 4662 && n < 4667) || (n > 4667 && n < 4672) || (n > 4672 && n < 4679) || n = 4680 || n = 4682 || n = 4684 || n = 4686 || n = 4687)
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4377 || n = 4396 || n = 4434 || n = 4453 || n = 4472 || n = 4491 || n = 4510 || n = 4529 || n = 4548 || n = 4567 || n = 4586 || n = 4605 || n = 4624 || n = 4643 || n = 4662 || n = 4681)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4379 || n = 4397 || n = 4415 || n = 4433 || n = 4487 || n = 4505 || n = 4523 || n = 4541 || n = 4559 || n = 4577 || n = 4595)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4392)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4394 || n = 4413 || n = 4432 || n = 4451 || n = 4470 || n = 4489 || n = 4508 || n = 4527 || n = 4546 || n = 4565 || n = 4584 || n = 4603 || n = 4622 || n = 4641 || n = 4660 || n = 4679)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4411)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 4414 || n = 4452)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4416)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	plp
.elseif(n = 4418 || n = 4456)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4421 || n = 4445 || n = 4493 || n = 4517 || n = 4589 || n = 4637 || n = 4661 || n = 4685)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4431)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $0AA0'
	ldy #10
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4436 || n = 4457 || n = 4478 || n = 4499 || n = 4520 || n = 4562 || n = 4583 || n = 4604 || n = 4625 || n = 4646 || n = 4667)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4454)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4467)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 4469)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 17
	Jne *-7
	tax
.elseif(n = 4539)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4575)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4580)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 14
	dey
	dey
	Jne *-7
	tay
.elseif(n = 4613)
	; 12 bytes
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4623)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4672)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
	tax
.elseif(n = 4683)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 13
	dey
	Jne *-7
	tay
.endif
.else
; 4688 to 5000
.if(n = 4688 || n = 4709 || n = 4730 || n = 4751 || n = 4772 || n = 4814 || n = 4835 || n = 4856 || n = 4877 || n = 4898 || n = 4919 || n = 4940 || n = 4961 || n = 4982)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif((n > 4688 && n < 4698) || n = 4699 || (n > 4700 && n < 4709) || (n > 4709 && n < 4715) || n = 4716 || n = 4721 || (n > 4722 && n < 4730) || n = 4731 || n = 4732 || n = 4734 || n = 4735 || n = 4737 || (n > 4738 && n < 4751) || (n > 4751 && n < 4755) || n = 4756 || (n > 4757 && n < 4772) || n = 4773 || n = 4775 || (n > 4776 && n < 4781) || (n > 4781 && n < 4793) || n = 4794 || (n > 4795 && n < 4805) || (n > 4805 && n < 4812) || n = 4813 || (n > 4814 && n < 4829) || n = 4830 || n = 4832 || n = 4834 || (n > 4835 && n < 4850) || n = 4851 || n = 4855 || (n > 4856 && n < 4869) || n = 4870 || (n > 4871 && n < 4875) || n = 4876 || (n > 4877 && n < 4898) || n = 4899 || n = 4900 || (n > 4901 && n < 4917) || n = 4918 || (n > 4919 && n < 4925) || (n > 4925 && n < 4940) || (n > 4940 && n < 4949) || (n > 4949 && n < 4961) || n = 4962 || (n > 4963 && n < 4971) || n = 4972 || (n > 4973 && n < 4982) || (n > 4982 && n < 4997) || (n > 4997 && n < 5001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4698 || n = 4717 || n = 4736 || n = 4755 || n = 4774 || n = 4793 || n = 4812 || n = 4831 || n = 4850)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4700 || n = 4719 || n = 4738 || n = 4757 || n = 4776 || n = 4795 || n = 4833 || n = 4852 || n = 4871)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4715)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4718)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4720)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	plp
.elseif(n = 4722)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4733 || n = 4781 || n = 4805 || n = 4829 || n = 4853 || n = 4901 || n = 4925 || n = 4949 || n = 4973 || n = 4997)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4854)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4869)
	; 11 bytes
	tya
	ldy #0
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4875)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 14
	iny
	Jne *-6
	tay
.elseif(n = 4917)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4963)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 4971)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 16
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.endif
.else
; 5001 to 10000
.if(n = 5001)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5002 || (n > 5003 && n < 5021) || n = 5022 || n = 5023 || (n > 5024 && n < 5045) || (n > 5045 && n < 5066) || n = 5067 || n = 5068 || (n > 5069 && n < 5087) || (n > 5087 && n < 5093) || (n > 5093 && n < 5108) || (n > 5108 && n < 5117) || (n > 5117 && n < 5129) || (n > 5129 && n < 5141) || (n > 5141 && n < 5150) || (n > 5150 && n < 5165) || (n > 5165 && n < 5169) || n = 5170 || (n > 5171 && n < 5189) || n = 5190 || n = 5191 || (n > 5192 && n < 5203) || (n > 5203 && n < 5211) || n = 5212 || (n > 5213 && n < 5234) || n = 5235 || n = 5236 || (n > 5237 && n < 5255) || (n > 5255 && n < 5261) || (n > 5261 && n < 5276) || (n > 5276 && n < 5281) || (n > 5281 && n < 5285) || (n > 5285 && n < 5297) || (n > 5297 && n < 5309) || (n > 5309 && n < 5316) || n = 5317 || (n > 5318 && n < 5333) || (n > 5333 && n < 5337) || n = 5338 || (n > 5339 && n < 5357) || n = 5358 || n = 5359 || (n > 5360 && n < 5381) || (n > 5381 && n < 5385) || (n > 5385 && n < 5405) || (n > 5405 && n < 5429) || (n > 5429 && n < 5453) || (n > 5453 && n < 5477) || (n > 5477 && n < 5501) || (n > 5501 && n < 5525) || (n > 5525 && n < 5549) || (n > 5549 && n < 5573) || (n > 5573 && n < 5597) || (n > 5597 && n < 5619) || n = 5620 || (n > 5621 && n < 5645) || (n > 5645 && n < 5669) || (n > 5669 && n < 5693) || (n > 5693 && n < 5707) || (n > 5707 && n < 5717) || (n > 5717 && n < 5741) || (n > 5741 && n < 5765) || (n > 5765 && n < 5789) || (n > 5789 && n < 5813) || (n > 5813 && n < 5837) || (n > 5837 && n < 5861) || (n > 5861 && n < 5885) || (n > 5885 && n < 5890) || (n > 5890 && n < 5909) || (n > 5909 && n < 5933) || (n > 5933 && n < 5957) || (n > 5957 && n < 5981) || (n > 5981 && n < 6005) || (n > 6005 && n < 6029) || (n > 6029 && n < 6035) || (n > 6035 && n < 6053) || (n > 6053 && n < 6077) || (n > 6077 && n < 6087) || (n > 6087 && n < 6101) || (n > 6101 && n < 6125) || (n > 6125 && n < 6149) || (n > 6149 && n < 6451) || (n > 6451 && n < 10001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5003 || n = 5024 || n = 5045 || n = 5066 || n = 5087 || n = 5108 || n = 5129 || n = 5150 || n = 5171 || n = 5192 || n = 5213 || n = 5234 || n = 5255 || n = 5276 || n = 5297 || n = 5318 || n = 5339 || n = 5360)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 5021 || n = 5069 || n = 5093 || n = 5117 || n = 5141 || n = 5165 || n = 5189 || n = 5237 || n = 5261 || n = 5285 || n = 5309 || n = 5333 || n = 5357 || n = 5405 || n = 5429 || n = 5453 || n = 5477 || n = 5501 || n = 5525 || n = 5549 || n = 5573 || n = 5597 || n = 5621 || n = 5645 || n = 5669 || n = 5693 || n = 5717 || n = 5741 || n = 5765 || n = 5789 || n = 5813 || n = 5837 || n = 5861 || n = 5885 || n = 5909 || n = 5933 || n = 5957 || n = 5981 || n = 6005 || n = 6029 || n = 6053 || n = 6077 || n = 6101 || n = 6125)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5169)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5203)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5211)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5281)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 21
	Jne *-7
	tax
.elseif(n = 5316)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
	tax
.elseif(n = 5337)
	; 12 bytes
	tya
	ldy #<$8D ;hides 'sta $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5381)
	; 12 bytes
	tya
	ldy #0
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 5385)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5619)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5707)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5890)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
.elseif(n = 6035)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 6087)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6149)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6451)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 13 bytes
	php
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif

.endif
	rest
.endmacro
