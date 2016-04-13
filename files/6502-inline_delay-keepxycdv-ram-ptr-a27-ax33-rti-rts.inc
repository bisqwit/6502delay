; delay_n, delays n cycles with small code
; Will clobber A, and Z+N
; Will preserve X, Y, S, C, D, V, and I (interrupt-disable flag)
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a disposable zeropage variable @zptemp.

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
; - 2 bytes or less when n <= 5
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 20
; - 6 bytes or less when n <= 31
; - 7 bytes or less when n <= 33
; - 8 bytes or less when n <= 295
; - 9 bytes or less when n <= 1293
; -10 bytes or less when n <= 1307
; -11 bytes or less when n <= 2172
; -12 bytes or less when n <= 3038
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.6, max=4
; - When 11 <= n <= 20: min=2, avg=3.7, max=5
; - When 21 <= n <= 30: min=4, avg=5.5, max=6
; - When 31 <= n <= 50: min=6, avg=6.8, max=8
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.3, max=9
; - When 501 <= n <= 750: min=6, avg=8.3, max=9
; - When 751 <= n <= 1000: min=7, avg=8.3, max=9
; - When 1001 <= n <= 2000: min=6, avg=9.1, max=11
; - When 2001 <= n <= 3000: min=8, avg=10.3, max=12
; - When 3001 <= n <= 5000: min=8, avg=11.2, max=13
; - When 5001 <= n <= 7500: min=9, avg=12.6, max=13
; - When 7501 <= n <= 10000: min=10, avg=12.9, max=13
; - When 10001 <= n <= 20000: min=12, avg=13.0, max=13
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
	lda $A5
.elseif(n = 4 || n = 6 || n = 15 || n = 17 || n = 28 || n = 30 || n = 32)
	; 2..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 19 || n = 20 || n = 22 || n = 33)
	; 3..6 bytes
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
	; 2 bytes
	brk
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 16 || n = 29)
	; 4..6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 18)
	; 4 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 23 || n = 26 || n = 39)
	; 4..6 bytes
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
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 34)
	; 8 bytes
	php
	lda $A5
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 35)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n > 35 && n < 39) || (n > 39 && n < 46) || (n > 46 && n < 53) || (n > 53 && n < 128) || (n > 128 && n < 157))
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
.if((n > 156 && n < 288) || (n > 288 && n < 292))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
	plp
.elseif(n = 288)
	; 6 bytes
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
.elseif(n = 292 || n = 293 || n = 297 || n = 302 || n = 307 || n = 309 || n = 312)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 294)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 296 || n = 298)
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 299 || n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 301 || n = 304)
	; 8..9 bytes
	brk
	delay_n (n-13)
.elseif(n = 303)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 305 || n = 310)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 308)
	; 9 bytes
	delay_n (n-3)
	lda $A5
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
; 313 to 625
.if(n = 313 || n = 348 || n = 362 || n = 369 || n = 404 || n = 432 || n = 439 || n = 446 || n = 481 || n = 488 || n = 509 || n = 537 || n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 314 || n = 317 || n = 319 || n = 322 || n = 324 || n = 332 || n = 336 || n = 337 || n = 339 || n = 342 || n = 344 || n = 347 || n = 349 || n = 352 || n = 354 || n = 357 || n = 359 || n = 367 || n = 371 || n = 372 || n = 374 || n = 377 || n = 379 || n = 382 || n = 384 || n = 387 || n = 389 || n = 392 || n = 394 || n = 406 || n = 407 || n = 412 || n = 414 || n = 417 || n = 419 || n = 422 || n = 424 || n = 427 || n = 429 || n = 437 || n = 441 || n = 442 || n = 444 || n = 447 || n = 449 || n = 452 || n = 454 || n = 457 || n = 462 || n = 464 || n = 472 || n = 476 || n = 477 || n = 479 || n = 482 || n = 484 || n = 487 || n = 492 || n = 494 || n = 497 || n = 499 || n = 511 || n = 512 || n = 517 || n = 519 || n = 522 || n = 524 || n = 527 || n = 529 || n = 532 || n = 534 || n = 539 || n = 542 || n = 546 || n = 547 || n = 549 || n = 552 || n = 557 || n = 559 || n = 562 || n = 564 || n = 567 || n = 569 || n = 574 || n = 577 || n = 582 || n = 584 || n = 587 || n = 589 || n = 592 || n = 594 || n = 597 || n = 602 || n = 604 || n = 609 || n = 612 || n = 616 || n = 617 || n = 619 || n = 622 || n = 624)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385 || n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460 || n = 470 || n = 480 || n = 485 || n = 495 || (n > 504 && n < 516) || (n > 524 && n < 566) || n = 590 || (n > 599 && n < 611)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 315 && n < 332) || n = 361 || n = 366 || n = 416 || (n > 425 && n < 437) || n = 451 || n = 456 || n = 466 || n = 486 || n = 526 || n = 531 || (n > 555 && n < 567) || n = 576 || n = 596 || n = 606))
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 318 || n = 323 || n = 328 || n = 333 || n = 338 || n = 343 || n = 353 || n = 358 || n = 363 || n = 368 || n = 373 || n = 378 || n = 386 || n = 393 || n = 398 || n = 403 || n = 408 || n = 413 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 458 || n = 463 || n = 468 || n = 473 || n = 478 || n = 483 || n = 493 || n = 498 || n = 503 || n = 508 || n = 513 || n = 518 || n = 528 || n = 533 || n = 538 || n = 543 || n = 548 || n = 553 || n = 563 || n = 568 || n = 573 || n = 578 || n = 583 || n = 588 || n = 598 || n = 603 || n = 608 || n = 613 || n = 618 || n = 623)
	; 8..9 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 329 || n = 364 || n = 399 || n = 434 || n = 469 || n = 504)
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
.elseif(n = 351 || n = 381 || n = 391 || n = 496 || n = 501 || n = 506 || n = 541 || n = 571 || n = 591 || n = 601)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 356 || n = 401 || n = 491 || n = 554)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 383)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
.elseif(n = 388)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 396)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 409 || n = 514)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 421)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	tax
.elseif(n = 459)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	sta @zptemp
	Jmi *-3
	tay
.elseif(n = 461)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
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
.elseif(n = 536 || n = 581 || n = 599)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 611)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	sta @zptemp
	Jmi *-3
	tax
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
; 626 to 781
.if(n = 626 || n = 689 || n = 716 || n = 734)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 627 || n = 629 || n = 632 || n = 634 || n = 637 || n = 639 || n = 647 || n = 652 || n = 654 || n = 657 || n = 659 || n = 662 || n = 664 || n = 667 || n = 669 || n = 672 || n = 674 || n = 679 || n = 682 || n = 686 || n = 687 || n = 692 || n = 697 || n = 699 || n = 702 || n = 704 || n = 707 || n = 709 || n = 717 || n = 721 || n = 722 || n = 724 || n = 727 || n = 729 || n = 732 || n = 737 || n = 739 || n = 742 || n = 744 || n = 756 || n = 757 || n = 762 || n = 764 || n = 767 || n = 769 || n = 772 || n = 774 || n = 777)
	; 8..9 bytes
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
.elseif(n = 633)
	; 9 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	plp
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636 || n = 646 || n = 666 || n = 681 || n = 746 || n = 771)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif((n .mod 5 = 3) && ((n > 637 && n < 659) || (n > 667 && n < 684) || (n > 702 && n < 729) || (n > 742 && n < 764) || n = 773 || n = 778))
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 641)
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
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 661 || n = 676 || n = 696 || n = 706 || n = 711 || n = 741 || n = 751 || n = 766 || n = 776 || n = 781)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 693)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 694)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 701)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 714 || n = 749)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 738 || n = 759)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif((n .mod 5 = 3) && ((n > 782 && n < 799) || (n > 807 && n < 834) || (n > 842 && n < 869) || (n > 877 && n < 899) || (n > 912 && n < 934)))
	; 9 bytes
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
.elseif(n = 786 || n = 841 || n = 856 || n = 876 || n = 906 || n = 911)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 787 || n = 791 || n = 792 || n = 794 || n = 797 || n = 799 || n = 802 || n = 804 || n = 807 || n = 809 || n = 812 || n = 814 || n = 822 || n = 826 || n = 827 || n = 829 || n = 832 || n = 834 || n = 837 || n = 839 || n = 842 || n = 844 || n = 847 || n = 849 || n = 857 || n = 861 || n = 862 || n = 864 || n = 867 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 892 || n = 897 || n = 899 || n = 902 || n = 904 || n = 907 || n = 909 || n = 912 || n = 914 || n = 917 || n = 919 || n = 927 || n = 931 || n = 932 || n = 934 || n = 937)
	; 8..9 bytes
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
.elseif(n = 806 || n = 851 || n = 896)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 811 || n = 816 || n = 846 || n = 871 || n = 886 || n = 891 || n = 916 || n = 921 || n = 926)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 948 || n = 953 || n = 958 || n = 963 || n = 968 || n = 973 || n = 983 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1021 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1053 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1088 || n = 1093)
	; 8..9 bytes
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
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1091)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 941 || n = 986)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 942 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 967 || n = 972 || n = 974 || n = 977 || n = 979 || n = 982 || n = 984 || n = 987 || n = 989 || n = 997 || n = 1001 || n = 1002 || n = 1004 || n = 1007 || n = 1009 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1032 || n = 1036 || n = 1037 || n = 1039 || n = 1042 || n = 1044 || n = 1047 || n = 1049 || n = 1052 || n = 1054 || n = 1057 || n = 1059 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1079 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1092)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && (n = 943 || (n > 956 && n < 979) || (n > 991 && n < 1014) || (n > 1026 && n < 1049) || (n > 1061 && n < 1084)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 946 || n = 956 || n = 996 || n = 1026 || n = 1051 || n = 1066 || n = 1081 || n = 1091)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif(n = 951 || n = 961 || n = 991 || n = 1016 || n = 1046 || n = 1086)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 959 || n = 994 || n = 1064)
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
.elseif(n = 969)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
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
	; 9 bytes
	tya
	ldy #121
	sta @zptemp
	dey
	Jpl *-3
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
.elseif(n = 1023)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1024)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	tax
.elseif(n = 1029)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	brk
	Jmi *-3
	tay
.elseif(n = 1031)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 1056)
	; 8 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1061)
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
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1102 || n = 1106 || n = 1107 || n = 1109 || n = 1112 || n = 1114 || n = 1117 || n = 1119 || n = 1122 || n = 1124 || n = 1127 || n = 1129 || n = 1137 || n = 1141 || n = 1142 || n = 1144 || n = 1147 || n = 1149 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1162 || n = 1164 || n = 1172 || n = 1176 || n = 1177 || n = 1179 || n = 1182 || n = 1184 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1197 || n = 1199 || n = 1207 || n = 1212 || n = 1214 || n = 1217 || n = 1219 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1232 || n = 1234 || n = 1242 || n = 1246 || n = 1247 || n = 1249)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1251)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && (n = 1096 || n = 1116 || n = 1126 || n = 1131 || (n > 1150 && n < 1162) || (n > 1185 && n < 1207) || n = 1226))
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
.elseif((n .mod 7 = 5) && ((n > 1096 && n < 1119) || (n > 1131 && n < 1154) || (n > 1166 && n < 1189) || (n > 1201 && n < 1224) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1114) || (n > 1122 && n < 1149) || (n > 1157 && n < 1184) || (n > 1192 && n < 1219) || (n > 1227 && n < 1249)))
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1099)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	sta @zptemp
	Jne *-3
	tay
.elseif(n = 1101)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1134 || n = 1169 || n = 1204 || n = 1239)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1136 || n = 1171 || n = 1221 || n = 1231)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1236)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
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
.elseif(n = 1252 || n = 1254 || n = 1257 || n = 1259 || n = 1262 || n = 1264 || n = 1267 || n = 1269 || n = 1277 || n = 1281 || n = 1282 || n = 1284 || n = 1287 || n = 1289 || n = 1294 || n = 1295 || n = 1297 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1316 || n = 1318 || n = 1323 || n = 1327)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1253 || n = 1263 || n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1303 || n = 1324)
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
.elseif(n = 1261)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 1266 || n = 1271 || n = 1276)
	; 9 bytes
	txa
	ldx #(n-11)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.elseif(n = 1291)
	; 9 bytes
	tya
	ldy #0
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 1292 || n = 1308)
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
.elseif(n = 1299 || n = 1306 || n = 1320)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1305 || n = 1315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
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
.elseif(n = 1317)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1322)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1326)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
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
.elseif(n = 1330 || n = 1332 || n = 1343 || n = 1344 || n = 1348 || n = 1350 || n = 1351 || n = 1353 || n = 1358 || n = 1360 || n = 1365 || n = 1367 || n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1388 || n = 1393 || n = 1399)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1352 || n = 1359 || n = 1366 || n = 1383 || n = 1394 || n = 1401 || n = 1406)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1333 || n = 1341 || n = 1357 || n = 1381 || n = 1389)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1334)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1340 || n = 1347 || n = 1354 || n = 1368 || n = 1396)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1345 || n = 1375 || n = 1395)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1361 || n = 1385 || n = 1397)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif((n .mod 14 = 4) && ((n > 1361 && n < 1405)))
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1369)
	; 10 bytes
	tya
	ldy #123
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1371)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
	tax
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
.elseif(n = 1387 || n = 1392)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
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
.if(n = 1407 || n = 1411 || n = 1414 || n = 1420 || n = 1421 || n = 1423 || n = 1428 || n = 1435 || n = 1442 || n = 1449 || n = 1451 || n = 1456 || n = 1458 || n = 1470 || n = 1474 || n = 1477 || n = 1479 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408)
	; 9 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 9 = 5) && ((n > 1408 && n < 1446) || (n > 1462 && n < 1482)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1410 || n = 1431 || n = 1434 || n = 1476)
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
.elseif(n = 1413 || n = 1429 || n = 1437 || n = 1453)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1415 || n = 1425 || n = 1455 || n = 1465)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1416)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1417 || n = 1438 || n = 1452 || n = 1466 || n = 1473 || n = 1480)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1422 || n = 1448 || n = 1450 || n = 1464 || n = 1471 || n = 1478)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1430)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	sta (@ptrtemp,x)
	tax
.elseif(n = 1432 || n = 1460)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1439 || n = 1467)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1441)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1443)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	brk
	iny
	Jpl *-4
	tay
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
.elseif(n = 1462)
	; 11 bytes
	php
	sec
	lda #120
	delay_n 7
	sbc #1
	Jpl *-4
	plp
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
.if(n = 1485 || n = 1509 || n = 1525 || n = 1549 || n = 1557)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1486 || n = 1491 || n = 1497 || n = 1498 || n = 1500 || n = 1505 || n = 1507 || n = 1512 || n = 1514 || n = 1519 || n = 1521 || n = 1533 || n = 1537 || n = 1539 || n = 1540 || n = 1542 || n = 1546 || n = 1547 || n = 1551 || n = 1554 || n = 1556 || n = 1561)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1487 || n = 1494 || n = 1536 || n = 1543 || n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1488 || n = 1502)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
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
.elseif(n = 1492 || n = 1506 || n = 1513 || n = 1527 || n = 1534 || n = 1548)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1493 || n = 1529 || n = 1541)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1495 || n = 1515 || n = 1555)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1511)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
	tay
.elseif(n = 1516 || n = 1530)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1518)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
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
.elseif(n = 1523)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	Jmi *-4
	tax
.elseif(n = 1528)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1532)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
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
.elseif(n = 1560)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1568 || n = 1570 || n = 1575 || n = 1582 || n = 1584 || n = 1596 || n = 1603 || n = 1609 || n = 1610 || n = 1612 || n = 1617 || n = 1619 || n = 1623 || n = 1624 || n = 1626 || n = 1631 || n = 1638 || n = 1645 || n = 1651 || n = 1659 || n = 1663 || n = 1666 || n = 1668 || n = 1672 || n = 1673 || n = 1680 || n = 1682 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1707 || n = 1708 || n = 1710)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && (n = 1564 || n = 1578 || (n > 1591 && n < 1607) || n = 1620 || n = 1662 || (n > 1675 && n < 1691) || (n > 1703 && n < 1719)))
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
.elseif(n = 1567 || n = 1600)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1569 || n = 1574 || n = 1576 || n = 1583 || n = 1590 || n = 1604 || n = 1611 || n = 1618 || n = 1630 || n = 1639 || n = 1644 || n = 1660 || n = 1667 || n = 1674 || n = 1681 || n = 1702 || n = 1716)
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
.elseif(n = 1572 || n = 1628 || n = 1642 || n = 1656 || n = 1684 || n = 1691 || n = 1712)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1581 || n = 1597 || n = 1605 || n = 1621 || n = 1653 || n = 1669 || n = 1677 || n = 1693 || n = 1717)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1585 || n = 1595 || n = 1635 || n = 1655 || n = 1665 || n = 1675 || n = 1695 || n = 1705)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1586 || n = 1593 || n = 1698)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
.elseif(n = 1588 || n = 1700)
	; 11 bytes
	php
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1591)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1602 || n = 1658 || n = 1714)
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
.elseif(n = 1633)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1634 || n = 1648)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1640 || n = 1654)
	; 9 bytes
	brk
	delay_n (n-13)
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
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1731 || n = 1736 || n = 1738 || n = 1743 || n = 1747 || n = 1750 || n = 1752 || n = 1757 || n = 1759 || n = 1764 || n = 1766 || n = 1771 || n = 1777 || n = 1785 || n = 1791 || n = 1792)
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
.elseif(n = 1721 || n = 1745 || n = 1781 || n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1723 || n = 1730 || n = 1737 || n = 1758 || n = 1772 || n = 1786)
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
.elseif(n = 1725 || n = 1749 || n = 1765 || n = 1773 || n = 1789)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1728)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1732 || n = 1739 || n = 1753 || n = 1767 || n = 1774 || n = 1788)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1735 || n = 1775 || n = 1795)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1740 || n = 1782)
	; 10 bytes
	txa
	ldx #(n-11)/7
	nop
	dex
	Jne *-2
	inc @zptemp,x
	tax
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
.elseif(n = 1761 || n = 1768)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 26
	Jmi *-5
	tay
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
.elseif(n = 1799 || n = 1807 || n = 1816 || n = 1819 || n = 1834)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1833)
	; 10 bytes
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
.elseif(n = 1808)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1811)
	; 10 bytes
	tya
	ldy #128
	delay_n 9
	dey
	Jpl *-4
	tay
.elseif(n = 1813 || n = 1821)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1815 || n = 1825 || n = 1835)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
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
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 1837 || n = 1845 || n = 1861 || n = 1869)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
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
	; 11 bytes
	php
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 9 = 5) && ((n > 1840 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1842 || n = 1864)
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
.elseif(n = 1847 || n = 1849 || n = 1852 || n = 1854 || n = 1857 || n = 1863 || n = 1870 || n = 1871)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1848 || n = 1860 || n = 1866 || n = 1872)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1853 || n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1855 || n = 1875)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1856 || n = 1862)
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
.elseif(n = 1878 || n = 1934)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1879 || n = 1883 || n = 1887 || n = 1888 || n = 1891 || n = 1903 || n = 1906 || n = 1911 || n = 1924 || n = 1939 || n = 1942 || n = 1943 || n = 1947 || n = 1951 || n = 1953)
	; 10..11 bytes
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
.elseif(n = 1881 || n = 1923)
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
.elseif(n = 1884 || n = 1896 || n = 1902 || n = 1912 || n = 1920 || n = 1928 || n = 1929 || n = 1932 || n = 1938 || n = 1944)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1885 || n = 1893 || n = 1909 || n = 1917 || n = 1933 || n = 1941)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
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
.elseif((n .mod 11 = 5) && ((n > 1896 && n < 1931)))
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
.elseif(n = 1905 || n = 1915 || n = 1935 || n = 1945)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
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
.elseif(n = 1926)
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
.elseif(n = 1950)
	; 11 bytes
	php
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	plp
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
.if(n = 1954)
	; 10 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	plp
.elseif(n = 1955 || n = 1975 || n = 1995 || n = 2015 || n = 2025)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 1956 || n = 1968 || n = 1980 || n = 1982 || n = 2001 || n = 2004 || n = 2010 || n = 2016)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1957 || n = 1965 || n = 1981 || n = 1989 || n = 2005 || n = 2013 || n = 2029)
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 2031)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 1960 || n = 1969 || n = 1977 || n = 1978 || n = 1983 || n = 1987 || n = 1991 || n = 1999 || n = 2011 || n = 2014 || n = 2017 || n = 2023 || n = 2031)
	; 10 bytes
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
.elseif(n = 1963 || n = 1974 || n = 1996 || n = 2007 || n = 2018)
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
.elseif(n = 1979 || n = 1993)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1984 || n = 2024)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1990 || n = 2008)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2000)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2006)
	; 10 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
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
	; 10 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
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
.if(n = 2032 || n = 2041 || n = 2043 || n = 2047 || n = 2050 || n = 2059 || n = 2061 || n = 2067 || n = 2068 || n = 2070)
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
.elseif(n = 2035 || n = 2055 || n = 2065)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
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
	; 9 bytes
	tya
	ldy #(n-5)/8
	sta @zptemp
	dey
	Jne *-3
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
.elseif(n = 2046 || n = 2052 || n = 2064)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2049 || n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2053)
	; 9 bytes
	tya
	ldy #0
	sta @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2054)
	; 11 bytes
	tya
	ldy #227
	delay_n 4
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2056)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2077 || n = 2086 || n = 2087 || n = 2097 || n = 2104 || n = 2107 || n = 2108)
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
.elseif(n = 2073 || n = 2106)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
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
.elseif(n = 2076 || n = 2089 || n = 2092 || n = 2098 || n = 2100)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2078 || n = 2094)
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
.elseif(n = 2083)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jpl *-4
	tay
.elseif(n = 2085 || n = 2095)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2088)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	sta (@ptrtemp,x)
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
.elseif(n = 2096)
	; 10 bytes
	php
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	plp
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
	sty @zptemp
	lda <$AD ;hides 'lda $E9A0'
	ldy #233
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2113 || n = 2122 || n = 2127 || n = 2131 || n = 2137 || n = 2140 || n = 2142 || n = 2149 || n = 2157 || n = 2158 || n = 2167 || n = 2169 || n = 2173 || n = 2176 || n = 2178 || n = 2179 || n = 2187)
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
.elseif((n .mod 10 = 5) && ((n > 2114 && n < 2156) || n = 2175 || n = 2185))
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2116 || n = 2118 || n = 2124 || n = 2136 || n = 2148 || n = 2154 || n = 2160 || n = 2162 || n = 2163 || n = 2184)
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
.elseif(n = 2119 || n = 2133)
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
.elseif(n = 2126)
	; 11 bytes
	txa
	ldx #235
	delay_n 4
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
.elseif(n = 2143)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jmi *-4
	tay
.elseif(n = 2151)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	sta @zptemp
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2152 || n = 2170)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2166)
	; 11 bytes
	sty @zptemp
	lda <$AD ;hides 'lda $F0A0'
	ldy #240
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2180)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if(n = 2188 || n = 2206 || n = 2224 || n = 2233)
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
.elseif(n = 2190 || n = 2196 || n = 2199 || n = 2208 || n = 2220 || n = 2226 || n = 2243 || n = 2244 || n = 2253 || n = 2256 || n = 2262)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2191 || n = 2193 || n = 2197 || n = 2207 || n = 2212 || n = 2214 || n = 2221 || n = 2223 || n = 2229 || n = 2230 || n = 2239 || n = 2241 || n = 2247 || n = 2248 || n = 2250 || n = 2251 || n = 2257 || n = 2263)
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
.elseif(n = 2194 || n = 2216 || n = 2227 || n = 2238 || n = 2260)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2195 || n = 2205 || n = 2215 || n = 2235 || n = 2245 || n = 2265)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 2217 || n = 2231 || n = 2259)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
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
.if(n = 2266 || n = 2267 || n = 2277 || n = 2284 || n = 2286 || n = 2299 || n = 2302)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2268 || n = 2272 || n = 2280 || n = 2289 || n = 2292 || n = 2298)
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
.elseif((n .mod 9 = 5) && ((n > 2272 && n < 2301)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2274)
	; 11 bytes
	sty @zptemp
	lda <$AD ;hides 'lda $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2275 || n = 2295)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2276 || n = 2294 || n = 2303)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
.if((n .mod 10 = 5) && ((n > 2304 && n < 2336)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
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
.elseif(n = 2307 || n = 2311 || n = 2313 || n = 2317 || n = 2331 || n = 2339)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2308 || n = 2312 || n = 2318 || n = 2324 || n = 2336)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2309)
	; 9 bytes
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2320)
	; 11 bytes
	php
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	plp
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
.elseif(n = 2316 || n = 2322 || n = 2328 || n = 2340)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 2323)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	sta @zptemp
	iny
	Jne *-4
	tay
.elseif(n = 2326 || n = 2337)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2327)
	; 9 bytes
	tya
	ldy #128
	brk
	dey
	Jpl *-3
	tay
.elseif(n = 2329)
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
.elseif(n = 2334)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
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
.elseif(n = 2343)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	sta @zptemp
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344 || n = 2352 || n = 2364 || n = 2376 || n = 2382 || n = 2388 || n = 2400 || n = 2412)
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
.elseif(n = 2346 || n = 2366 || n = 2378 || n = 2396 || n = 2406 || n = 2408)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2347 || n = 2349 || n = 2354 || n = 2361 || n = 2367 || n = 2372 || n = 2377 || n = 2379 || n = 2383 || n = 2387 || n = 2394 || n = 2397 || n = 2401 || n = 2404 || n = 2407 || n = 2409 || n = 2416 || n = 2419 || n = 2421)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2348 || n = 2359 || n = 2370 || n = 2392 || n = 2414)
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
.elseif((n .mod 10 = 5) && ((n > 2354 && n < 2396) || n = 2415))
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2356)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2358)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2360 || n = 2390 || n = 2420)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif((n .mod 12 = 10) && ((n > 2361 && n < 2411)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2363 || n = 2399)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2371 || n = 2413)
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
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	sta @zptemp
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2403)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	inc @zptemp
	iny
	Jne *-4
	tay
.elseif(n = 2411)
	; 11 bytes
	txa
	ldx #(n-11)/10
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp,x
	tax
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2494)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2423 || n = 2449 || n = 2462)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2424 || n = 2446 || n = 2448 || n = 2460 || n = 2476 || n = 2484 || n = 2496)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 10 = 5) && ((n > 2424 && n < 2456) || (n > 2474 && n < 2496)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2426 || n = 2431 || n = 2433 || n = 2437 || n = 2438 || n = 2443 || n = 2457 || n = 2459 || n = 2467 || n = 2473 || n = 2479 || n = 2481 || n = 2486 || n = 2487 || n = 2493 || n = 2499)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427 || n = 2483 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428 || n = 2432 || n = 2442 || n = 2456 || n = 2468 || n = 2472 || n = 2478 || n = 2492 || n = 2498)
	; 10..11 bytes
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
	; 11 bytes
	php
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	plp
.elseif(n = 2436 || n = 2447 || n = 2458 || n = 2480 || n = 2491)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2444 || n = 2474)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2450)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2451)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 2452 || n = 2466 || n = 2482)
	; 10..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2454)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 2461)
	; 11 bytes
	txa
	ldx #(n-11)/10
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2463)
	; 12 bytes
	php
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	plp
.elseif(n = 2464)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	sta @zptemp
	brk
	dey
	Jmi *-6
	tay
.elseif(n = 2469)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
.elseif(n = 2470)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2471)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 2502 || n = 2524 || n = 2546 || n = 2557 || n = 2568)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2503 || n = 2517 || n = 2519 || n = 2523 || n = 2526 || n = 2527 || n = 2529 || n = 2534 || n = 2547 || n = 2548 || n = 2551 || n = 2556 || n = 2559 || n = 2563 || n = 2569 || n = 2574 || n = 2575)
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
.elseif(n = 2505 || n = 2515 || n = 2535 || n = 2545)
	; 9 bytes
	tya
	ldy #(n-5)/10
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2506 || n = 2518 || n = 2530 || n = 2542 || n = 2566 || n = 2578)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2507 || n = 2543)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2508 || n = 2512 || n = 2520 || n = 2522 || n = 2532 || n = 2562)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2509)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
	Jne *-5
	tax
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
.elseif(n = 2514)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2516 || n = 2528 || n = 2552 || n = 2558 || n = 2564 || n = 2576)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2536)
	; 12 bytes
	sty @zptemp
	lda <$AD ;hides 'lda $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2550)
	; 11 bytes
	php
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	plp
.elseif(n = 2554)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	iny
	iny
	Jne *-4
	tay
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
.elseif(n = 2565)
	; 9 bytes
	tya
	ldy #0
	inc @zptemp
	dey
	Jne *-3
	tay
.elseif(n = 2571)
	; 11 bytes
	txa
	ldx #0
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
.if(n = 2579 || n = 2615 || n = 2651)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2580 || n = 2584 || n = 2592 || n = 2604 || n = 2616 || n = 2622 || n = 2628 || n = 2641 || n = 2652)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2581 || n = 2595 || n = 2637)
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
.elseif(n = 2583 || n = 2587 || n = 2596 || n = 2599 || n = 2607 || n = 2611 || n = 2613 || n = 2617 || n = 2619 || n = 2632 || n = 2635 || n = 2636 || n = 2639 || n = 2647 || n = 2649 || n = 2653 || n = 2655)
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
.elseif(n = 2588 || n = 2593 || n = 2624 || n = 2648 || n = 2654)
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
.elseif((n .mod 11 = 5) && ((n > 2589 && n < 2635) || n = 2656))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2591)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 2598 || n = 2610)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2600 || n = 2630)
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
.elseif(n = 2605 || n = 2625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
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
.elseif(n = 2608 || n = 2627 || n = 2646)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2618 || n = 2631 || n = 2644)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2620)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 2643)
	; 12 bytes
	sty @zptemp
	ldy #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2658 || n = 2659 || n = 2661 || n = 2666 || n = 2671 || n = 2673 || n = 2680 || n = 2683 || n = 2695 || n = 2697 || n = 2702 || n = 2704 || n = 2713 || n = 2715 || n = 2719 || n = 2727 || n = 2731)
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
.elseif(n = 2663 || n = 2699)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2664 || n = 2676 || n = 2688 || n = 2694 || n = 2712 || n = 2724 || n = 2730)
	; 11 bytes
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
.elseif(n = 2668 || n = 2672 || n = 2684 || n = 2726 || n = 2732)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2670 || n = 2696 || n = 2709)
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
.elseif(n = 2682 || n = 2706 || n = 2718)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 2687 || n = 2723)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2692 || n = 2728)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 2701)
	; 12 bytes
	tya
	ldy #224
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	nop
.elseif(n = 2703)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2714)
	; 11 bytes
	tya
	ldy #128
	sta @zptemp
	brk
	dey
	Jpl *-5
	tay
.elseif(n = 2716)
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
; 2735 to 2812
.if(n = 2735 || n = 2749 || n = 2763 || n = 2791 || n = 2805)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2736 || n = 2740 || n = 2748 || n = 2760 || n = 2772 || n = 2784 || n = 2796 || n = 2802 || n = 2808)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2737 || n = 2739 || n = 2751 || n = 2752 || n = 2757 || n = 2761 || (n > 2766 && n < 2770) || n = 2778 || n = 2779 || n = 2781 || n = 2786 || n = 2790 || n = 2797 || n = 2807 || n = 2812)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2738)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	plp
.elseif((n .mod 12 = 5) && ((n > 2740 && n < 2802)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2742 || n = 2776 || n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2743)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	brk
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
.elseif(n = 2745 || n = 2785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
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
.elseif(n = 2747 || n = 2771)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2750 || n = 2780)
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
.elseif(n = 2756 || n = 2762 || n = 2792 || n = 2804)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2759 || n = 2795)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2764)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2774 || n = 2800)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2775)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	sta @zptemp
	dex
	dex
	Jne *-5
	tax
.elseif(n = 2783)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2787)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	inc @zptemp
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
.elseif(n = 2809)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 2811)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if((n .mod 12 = 5) && ((n > 2812 && n < 2850)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2814 || n = 2816 || n = 2828 || n = 2834 || n = 2848)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2815 || n = 2817 || n = 2822 || n = 2823 || n = 2827 || n = 2829 || n = 2835 || n = 2839 || n = 2841 || n = 2846 || n = 2851)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2818 || n = 2830 || n = 2842)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2819 || n = 2833 || n = 2847)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2820 || n = 2832 || n = 2838)
	; 11 bytes
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
.elseif(n = 2824)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2826)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2831)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2836)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2840)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2843)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 2844)
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
.elseif(n = 2850)
	; 11 bytes
	delay_n (n-5)
	inc @zptemp
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2856 || n = 2868)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2853 || n = 2863 || n = 2869 || n = 2871 || n = 2887)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854 || n = 2866 || n = 2890)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2855 || n = 2870)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2857)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 2858 || n = 2860 || n = 2862 || n = 2886)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 2859)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	brk
	iny
	iny
	Jne *-6
	tay
.elseif(n = 2861 || n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2864 || n = 2876 || n = 2888)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 2867)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2872)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 2874)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2875 || n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2877)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2878)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2879)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2880)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	sta @zptemp
	brk
	Jne *-5
	tay
.elseif(n = 2881)
	; 12 bytes
	txa
	ldx #(n-11)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 2882)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 2883)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 2884)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	plp
.endif
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2943 || n = 2956)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2892 || n = 2896 || n = 2904 || n = 2910 || n = 2916 || n = 2928 || n = 2940 || n = 2952 || n = 2964)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2893)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2894 || n = 2899 || n = 2901 || n = 2911 || n = 2913 || n = 2918 || n = 2919 || n = 2923 || n = 2932 || n = 2935 || n = 2937 || n = 2941 || n = 2947 || n = 2949 || n = 2954 || n = 2967)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895 || n = 2912 || n = 2929 || n = 2946 || n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 5) && ((n > 2896 && n < 2958)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2898 || n = 2922 || n = 2934 || n = 2958)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2900 || n = 2915 || n = 2930 || n = 2960)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif((n .mod 12 = 10) && ((n > 2901 && n < 2963)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2903 || n = 2939)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2905 || n = 2925 || n = 2965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 2906 || n = 2920 || n = 2924 || n = 2936 || n = 2942 || n = 2948)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2907 || n = 2961)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2908 || n = 2944)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 2917 || n = 2931 || n = 2959)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2927 || n = 2951)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2953)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 2955)
	; 11 bytes
	tya
	ldy #(n-30)/25
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
.elseif(n = 2966)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 2968)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
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
.elseif(n = 2970 || n = 2994 || n = 3042)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2971 || n = 2977 || n = 2979 || n = 2983 || n = 2989 || n = 2991 || n = 2995 || n = 3002 || n = 3007 || n = 3009 || n = 3013 || n = 3016 || n = 3027 || n = 3028 || n = 3031 || n = 3033 || n = 3037 || n = 3038)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2978 || n = 2984 || n = 2996 || n = 3004 || n = 3044)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 14 = 5) && ((n > 2972 && n < 3016) || n = 3043))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 10) && ((n > 2973 && n < 3047)))
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2975 || n = 3011)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2976 || n = 2988 || n = 3000 || n = 3012 || n = 3018 || n = 3024 || n = 3036)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2980 || n = 2997 || n = 3014)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2982 || n = 3008 || n = 3021)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2985 || n = 3025)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 2990 || n = 3020 || n = 3035)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2992)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 2999)
	; 11 bytes
	txa
	ldx #(n-11)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
.elseif(n = 3019)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3023)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3026)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3030)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
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
.elseif(n = 3039 || n = 3040)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3045)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
	tay
.endif
.else
; 3047 to 3125
.if(n = 3047 || n = 3083 || n = 3101 || n = 3119)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3048)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
	tax
.elseif(n = 3049 || n = 3051 || n = 3055 || n = 3059 || n = 3062 || n = 3067 || n = 3069 || n = 3075 || n = 3079 || n = 3081 || n = 3087 || n = 3094 || n = 3097 || n = 3098 || n = 3103 || n = 3111 || n = 3115 || n = 3117 || n = 3118 || n = 3121 || n = 3123)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3050 || n = 3080 || n = 3095 || n = 3110)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3052 || n = 3088 || n = 3106 || n = 3124)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3054 || n = 3060 || n = 3072 || n = 3084 || n = 3090 || n = 3108 || n = 3120)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3056 || n = 3068 || n = 3104 || n = 3122)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 14 = 5) && ((n > 3056 && n < 3114)))
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
.elseif(n = 3061 || n = 3093 || n = 3109)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3063 || n = 3107)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3064 || n = 3102)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3066 || n = 3078 || n = 3096 || n = 3114)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3073 || n = 3086 || n = 3112)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3074)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3076)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3082 || n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3089)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3091)
	; 12 bytes
	tya
	ldy #(n-11)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3092)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3100)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3105 || n = 3125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3144 || n = 3148 || n = 3162 || n = 3180 || n = 3198)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3127 || n = 3141 || n = 3169 || n = 3183 || n = 3197)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3128 || n = 3129 || n = 3131 || n = 3135 || n = 3139 || n = 3143 || n = 3147 || n = 3152 || n = 3153 || n = 3157 || n = 3166 || n = 3171 || n = 3172 || n = 3175 || n = 3181 || n = 3182 || n = 3186 || n = 3192 || n = 3193 || n = 3195 || n = 3199)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3130)
	; 11 bytes
	tya
	ldy #(n-30)/25
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
.elseif(n = 3132 || n = 3168)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3133 || n = 3150 || n = 3167 || n = 3184 || n = 3201)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3134)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3136)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 3137 || n = 3155 || n = 3173 || n = 3191)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif((n .mod 13 = 5) && ((n > 3137 && n < 3191)))
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3140 || n = 3170 || n = 3200)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3142 || n = 3160 || n = 3196)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3145 || n = 3165 || n = 3185)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3146 || n = 3174 || n = 3188)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
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
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	sta @zptemp
	brk
	Jne *-6
	tay
.elseif(n = 3156 || n = 3202)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3158 || n = 3176 || n = 3194)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3159 || n = 3178)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3161)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3163)
	; 12 bytes
	tya
	ldy #(n-13)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	nop
.elseif(n = 3179)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 3189)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
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
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3222 || n = 3240)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3206 || n = 3212 || n = 3228 || n = 3238)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3207 || n = 3213 || n = 3219 || n = 3220 || n = 3223 || n = 3229 || n = 3231 || n = 3236 || n = 3241)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3208)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3209 || n = 3227)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3210 || n = 3234)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3211 || n = 3225 || n = 3239)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3214 || n = 3232)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3215 || n = 3230)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3216 || n = 3242)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3217)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3218 || n = 3235)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3221)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3224)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3226)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3233)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3237)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
	tax
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3247 || n = 3249 || n = 3262 || n = 3271 || n = 3277 || n = 3278)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244 || n = 3272)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3245 || n = 3263 || n = 3281)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3246 || n = 3258 || n = 3270 || n = 3274)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3248 || n = 3266)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3250)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3252 || n = 3269)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3253 || n = 3267)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3254 || n = 3273)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3255 || n = 3268)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
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
.elseif(n = 3257)
	; 11 bytes
	txa
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3259)
	; 12 bytes
	txa
	ldx #(n-11)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3260 || n = 3275)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3261)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3264 || n = 3276)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3265)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3280)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3289 || n = 3298 || n = 3302)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3283 || n = 3287 || n = 3288 || n = 3297 || n = 3301 || n = 3319)
	; 10..11 bytes
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
.elseif(n = 3285 || n = 3305)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3286 || n = 3303 || n = 3320)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3290)
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
.elseif(n = 3292 || n = 3311)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3293 || n = 3306 || n = 3310 || n = 3316)
	; 11..12 bytes
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
.elseif(n = 3295 || n = 3309)
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
.elseif(n = 3299 || n = 3317)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3300 || n = 3314)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3304)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3308)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3312 || n = 3318)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3313)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3315)
	; 12 bytes
	txa
	ldx #(n-11)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3322 || n = 3332 || n = 3339 || n = 3346 || n = 3347 || n = 3352 || (n > 3354 && n < 3358))
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3323 || n = 3337 || n = 3351)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3324 || n = 3342)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3325 || n = 3345)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
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
.elseif(n = 3327)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3328)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3329 || n = 3343)
	; 12 bytes
	txa
	ldx #(n-11)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3330)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3331)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3333)
	; 11 bytes
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3334)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3335 || n = 3353)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3336 || n = 3348)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3338)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3340 || n = 3358)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3341 || n = 3359)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3344)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3349)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3350)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3354)
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
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3361 || n = 3372 || n = 3378 || n = 3396)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3362 || n = 3367 || n = 3373 || n = 3375 || n = 3382 || n = 3390 || n = 3391)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3364 || n = 3366)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3365 || n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3368 || n = 3387)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3369)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3370 || n = 3398)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3371 || n = 3389)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3374 || n = 3392)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3376 || n = 3394)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3377)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3380 || n = 3395)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3381 || n = 3397)
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
.elseif(n = 3384)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 3385)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3386)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3388)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3408 || n = 3409 || n = 3411 || n = 3417 || n = 3423 || n = 3424 || n = 3427 || n = 3437)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3400 || n = 3414 || n = 3432)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3401 || n = 3419)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3402 || n = 3420)
	; 11 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3405 || n = 3422)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3406)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3407 || n = 3425)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3410)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3412 || n = 3430)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3413 || n = 3429)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3415)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
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
.elseif(n = 3421 || n = 3435)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3426)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3428)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
	tay
.elseif(n = 3431)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3433)
	; 12 bytes
	tya
	ldy #(n-13)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	nop
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
.elseif(n = 3436)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3474)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3439 || n = 3456 || n = 3473)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3440 || n = 3455 || n = 3470)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3441 || n = 3451 || n = 3453 || n = 3458 || n = 3467 || n = 3472 || n = 3475)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3442 || n = 3450)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3443 || n = 3461)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3444)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3445 || n = 3465)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3446)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 3447)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3448 || n = 3466)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3449 || n = 3463)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3452 || n = 3457 || n = 3464 || n = 3476)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3454)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
.elseif(n = 3459)
	; 11 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3460)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3462)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3468)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3469)
	; 12 bytes
	tya
	ldy #(n-11)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3471)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3491 || n = 3505)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3478)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3479 || n = 3497 || n = 3515)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3481 || (n > 3486 && n < 3490) || n = 3492 || n = 3499 || n = 3511)
	; 10..12 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3484 || n = 3502)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3486 || n = 3498 || n = 3504 || n = 3514)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3490 || n = 3507)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3493 || n = 3509)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3494)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3495)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3496)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3500)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3501)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3503)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 3508)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3510)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3512)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3513)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3586)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3517 || n = 3521 || n = 3523 || n = 3526 || n = 3527 || n = 3532 || n = 3535 || n = 3537 || n = 3542 || n = 3543 || n = 3549 || n = 3553 || n = 3555 || n = 3559 || n = 3562 || n = 3563 || n = 3567 || n = 3571 || n = 3584 || n = 3591)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3518 || n = 3536 || n = 3544 || n = 3550 || n = 3554 || n = 3572 || n = 3578 || n = 3588)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3519 || n = 3547 || n = 3561 || n = 3575)
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
.elseif(n = 3522 || n = 3531 || n = 3540 || n = 3548 || n = 3568 || n = 3576)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3524 || n = 3541 || n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3525 || n = 3545 || n = 3565 || n = 3585)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3528 || n = 3546 || n = 3564 || n = 3582)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3529)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3530 || n = 3560 || n = 3590)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3533 || n = 3551 || n = 3569 || n = 3587)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3534)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jmi *-6
	tay
.elseif(n = 3538 || n = 3556 || n = 3574)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3552 || n = 3566)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3570)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3579)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3580)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3581)
	; 12 bytes
	tya
	ldy #(n-11)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.elseif(n = 3589)
	; 10 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3593)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3598 || n = 3607 || n = 3611 || n = 3619 || n = 3622 || n = 3628)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595)
	; 12 bytes
	txa
	ldx #0
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3596 || n = 3615)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3597 || n = 3614 || n = 3631)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3599 || n = 3612 || n = 3630)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3600 || n = 3618)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3601)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3602)
	; 12 bytes
	tya
	ldy #109
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 11
	iny
	Jne *-6
	tay
.elseif(n = 3604 || n = 3606 || n = 3624)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3605 || n = 3623)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3608)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3609 || n = 3626)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3610)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3613)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3616)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3617)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3620)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3621)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3627)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3629)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3632)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.endif
.else
; 3633 to 3671
.if(n = 3633 || n = 3648 || n = 3664 || n = 3666)
	; 10..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3634)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3635 || n = 3650)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3636 || n = 3654 || n = 3658 || n = 3670)
	; 10..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3637 || n = 3653 || n = 3669)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3638)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3639)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3640 || n = 3647 || n = 3652 || n = 3656 || n = 3661 || n = 3667 || n = 3668 || n = 3671)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3641 || n = 3659)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3642)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3643 || n = 3660)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3644)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3645 || n = 3665)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3646)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3649)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3651)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3655)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3657)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
.elseif(n = 3662)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3663)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	sta (@ptrtemp,x)
	tax
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
.elseif(n = 3673 || n = 3698 || n = 3714 || n = 3716 || n = 3734)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3674 || n = 3679 || n = 3681 || n = 3686 || n = 3687 || n = 3689 || n = 3693 || n = 3696 || n = 3697 || n = 3703 || n = 3707 || n = 3709 || n = 3712 || n = 3724 || n = 3727 || n = 3730 || n = 3732 || n = 3733 || n = 3735 || n = 3739 || n = 3742 || n = 3746 || n = 3750)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	dex
	dex
	Jne *-6
	tax
.elseif(n = 3676 || n = 3678 || n = 3708 || n = 3726 || n = 3741 || n = 3744)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3677 || n = 3695 || n = 3713 || n = 3731 || n = 3749)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3680 || n = 3710 || n = 3740)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3682 || n = 3700 || n = 3718 || n = 3736)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3683 || n = 3719)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3684 || n = 3692 || n = 3702 || n = 3706 || n = 3720 || n = 3738)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3685 || n = 3705 || n = 3725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3688)
	; 11 bytes
	php
	sec
	lda #184 ;hides 'clv'
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 3690)
	; 11 bytes
	php
	tya
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
	tay
	plp
.elseif(n = 3694 || n = 3711 || n = 3728 || n = 3745)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3701 || n = 3717)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3704)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3721)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 3722 || n = 3743)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3723)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3737)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
.if((n >= 3751) && (n < 3790))
; 3751 to 3789
.if(n = 3751 || n = 3753 || n = 3757 || n = 3758 || n = 3764 || n = 3769 || n = 3771 || n = 3775 || n = 3783 || n = 3787 || n = 3789)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3752 || n = 3782 || n = 3788)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3754 || n = 3772)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3755 || n = 3770)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3756 || n = 3774)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3759 || n = 3763)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3760)
	; 12 bytes
	sty @zptemp
	ldy #21 ;hides 'ora zp,x'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3762 || n = 3779)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3766)
	; 12 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3767 || n = 3785)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3768)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3773)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
.elseif(n = 3777)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3778 || n = 3780)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3781)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
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
.elseif(n = 3786)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3808 || n = 3826)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3791 || n = 3809)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3792 || n = 3810 || n = 3812 || n = 3820 || n = 3828)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3793 || n = 3794 || n = 3798 || n = 3799 || n = 3802 || n = 3817 || n = 3823 || n = 3827)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3795 || n = 3804 || n = 3814 || n = 3822)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3796 || n = 3813)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3797)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3800 || n = 3815)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3801)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3803 || n = 3821)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3805 || n = 3825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3806)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3807)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3811)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3816)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3818)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3819)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3824)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 3829 to 3906
.if(n = 3829 || n = 3861)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3830 || n = 3847 || n = 3864 || n = 3881 || n = 3898)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3831 || n = 3832 || n = 3841 || n = 3849 || n = 3851 || n = 3859 || (n > 3865 && n < 3869) || n = 3877 || n = 3879 || n = 3883 || n = 3887 || n = 3889 || n = 3894 || n = 3895 || n = 3897 || n = 3902)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3833 || n = 3855 || n = 3899)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3834 || n = 3838 || n = 3852 || n = 3858 || n = 3870 || n = 3873 || n = 3888 || n = 3906)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3835 || n = 3886 || n = 3903)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3836)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3837 || n = 3846 || n = 3854 || n = 3871 || n = 3872 || n = 3882)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3839 || n = 3857 || n = 3875 || n = 3893)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3840 || n = 3856 || n = 3874 || n = 3876)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3842 || n = 3850 || n = 3878 || n = 3884 || n = 3896 || n = 3901)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3843 || n = 3862 || n = 3900)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3844 || n = 3880)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3845 || n = 3865 || n = 3885 || n = 3905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3848 || n = 3869 || n = 3890)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3853 || n = 3904)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 3860)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
.elseif(n = 3863)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3891)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3892)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3908 || n = 3913 || n = 3917 || n = 3923 || n = 3926 || n = 3927 || n = 3931 || n = 3933 || n = 3934 || n = 3940 || n = 3944 || n = 3951 || n = 3959 || n = 3962 || n = 3963 || (n > 3966 && n < 3970) || n = 3975 || n = 3982)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3909 || n = 3957 || n = 3973)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3910)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 3911 || n = 3929 || n = 3947 || n = 3965 || n = 3983)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3912 || n = 3918 || n = 3922 || n = 3936 || n = 3939 || n = 3948 || n = 3954 || n = 3956 || n = 3972)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3914 || n = 3928 || n = 3950)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3915 || n = 3932 || n = 3949 || n = 3966)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3916 || n = 3952 || n = 3970)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3919 || n = 3938 || n = 3976)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3920 || n = 3937)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3921 || n = 3943)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3924 || n = 3958 || n = 3960 || n = 3978 || n = 3979)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3925 || n = 3945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3930 || n = 3955 || n = 3980)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3935 || n = 3971)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 3941)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
	tax
.elseif(n = 3942)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3946)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3953)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3961 || n = 3984)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3964)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3974)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3977)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3981)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4005 || n = 4025 || n = 4045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3986 || n = 4020 || n = 4022 || n = 4040 || n = 4048 || n = 4058)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3987)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3988 || n = 4006 || n = 4024 || n = 4042 || n = 4060)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3989 || n = 4053)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3990 || n = 3992 || n = 4008 || n = 4010 || n = 4012 || n = 4026 || n = 4044 || n = 4062)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3991 || n = 3997 || n = 4002 || n = 4007 || n = 4011 || n = 4021 || n = 4023 || (n > 4026 && n < 4030) || n = 4036 || n = 4038 || n = 4039 || n = 4041 || n = 4046 || n = 4047 || n = 4049 || n = 4054 || n = 4057 || n = 4059)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3993 || n = 3994 || n = 3999)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3995 || n = 4014 || n = 4033 || n = 4052)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3996 || n = 4016 || n = 4018 || n = 4032 || n = 4050)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3998)
	; 12 bytes
	tya
	ldy #121
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 4000 || n = 4017 || n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4001 || n = 4019 || n = 4037 || n = 4055)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4003)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 4004)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 4009 || n = 4031)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4013)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4015)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4030)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4035)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4043)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4056)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4061)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
; 4063 to 4140
.if(n = 4063)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	Jne *-4
	tax
.elseif(n = 4064 || n = 4067 || n = 4070 || n = 4072 || n = 4075 || n = 4077 || n = 4081 || n = 4082 || n = 4087 || n = 4089 || n = 4092 || n = 4093 || n = 4095 || n = 4103 || n = 4104 || n = 4106 || n = 4107 || n = 4111 || n = 4113 || n = 4118 || n = 4121 || n = 4123 || n = 4129 || n = 4131 || n = 4135 || n = 4138)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065 || n = 4105 || n = 4125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4066 || n = 4097)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4068 || n = 4085 || n = 4102 || n = 4119 || n = 4136)
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
.elseif(n = 4071 || n = 4090 || n = 4128)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4073 || n = 4091 || n = 4109 || n = 4127)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4074)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4076 || n = 4088 || n = 4094 || n = 4112 || n = 4130 || n = 4139)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4078 || n = 4096 || n = 4114 || n = 4132)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4079 || n = 4100)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4080 || n = 4098 || n = 4116 || n = 4126 || n = 4134)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4083 || n = 4084)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4086 || n = 4122 || n = 4140)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4099)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4101)
	; 11 bytes
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 4108)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4115)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4117)
	; 12 bytes
	txa
	ldx #228
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	nop
.elseif(n = 4120)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	tax
.elseif(n = 4124)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4133)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4137)
	; 12 bytes
	php
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	plp
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4142)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4143 || n = 4152 || n = 4160 || n = 4177)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4144 || n = 4147 || n = 4149 || n = 4154 || n = 4155 || n = 4157 || n = 4172 || n = 4174 || n = 4178)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4145 || n = 4163)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4146 || n = 4162 || n = 4164)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4148 || n = 4156 || n = 4173)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4150 || n = 4168)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4151)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4153 || n = 4170)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4158 || n = 4176)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4159)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 4165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4166)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4167)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4169)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4171)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4175)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4179)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 11
	iny
	Jne *-6
	tay
.endif
.else
; 4180 to 4218
.if(n = 4180)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4181 || n = 4199 || n = 4217)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 4183 || n = 4189 || n = 4201 || n = 4206 || n = 4208 || n = 4216)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4184 || n = 4202)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4185 || n = 4205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4186)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4187 || n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4188 || n = 4211)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4190)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4191 || n = 4214)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4192 || n = 4209)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4193 || n = 4210)
	; 12 bytes
	txa
	ldx #(n-11)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4194 || n = 4198 || n = 4200 || n = 4212)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4195 || n = 4197 || n = 4213)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4196)
	; 12 bytes
	tya
	ldy #126
	nop
	delay_n 26
	dey
	Jpl *-6
	tay
.elseif(n = 4203)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 14
	iny
	Jne *-6
	tay
.elseif(n = 4207)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4215)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 4218)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	iny
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4227 || n = 4231 || n = 4237 || n = 4239 || n = 4240 || n = 4244 || n = 4247 || n = 4249 || n = 4254 || n = 4257 || n = 4263 || n = 4267 || n = 4269 || (n > 4272 && n < 4276) || n = 4282 || n = 4286 || n = 4287 || n = 4291 || n = 4293)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4220 || n = 4256 || n = 4288 || n = 4292)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4221 || n = 4238 || n = 4255 || n = 4272)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4222 || n = 4258 || n = 4276 || n = 4294)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4223 || n = 4242 || n = 4261 || n = 4280)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4224 || n = 4228 || n = 4232 || n = 4260 || n = 4262 || n = 4278 || n = 4279 || n = 4296)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4225 || n = 4245 || n = 4265 || n = 4285)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4226 || n = 4268)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4229 || n = 4251 || n = 4295)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4230 || n = 4234 || n = 4248 || n = 4284)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4233 || n = 4236 || n = 4246 || n = 4264 || n = 4281)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4235 || n = 4253 || n = 4271 || n = 4289)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4241 || n = 4277)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4243)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4250)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
	tay
.elseif(n = 4252 || n = 4283)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4259)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4266)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	sta @zptemp
	brk
	Jne *-5
	tax
.elseif(n = 4270 || n = 4290)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.endif
.else
; 4297 to 4375
.if(n = 4297 || n = 4298 || n = 4301 || n = 4304 || n = 4308 || n = 4309 || n = 4311 || n = 4316 || n = 4319 || n = 4322 || n = 4327 || n = 4329 || n = 4333 || n = 4334 || n = 4341 || n = 4342 || n = 4344 || n = 4354 || n = 4358 || n = 4359 || n = 4363 || n = 4367 || n = 4369 || n = 4370)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4318 || n = 4337 || n = 4356 || n = 4375)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4300 || n = 4303 || n = 4321 || n = 4324 || n = 4335)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4302 || n = 4320 || n = 4336 || n = 4338 || n = 4353 || n = 4374)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4305 || n = 4345 || n = 4365)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4306 || n = 4323 || n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4307 || n = 4325 || n = 4343 || n = 4361)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4310 || n = 4326 || n = 4346 || n = 4360 || n = 4364)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4312 || n = 4330 || n = 4348 || n = 4366)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4313)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4314 || n = 4332 || n = 4350 || n = 4368 || n = 4372)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4315)
	; 12 bytes
	tya
	ldy #(n-13)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	nop
.elseif(n = 4317 || n = 4339)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4328)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4331 || n = 4349)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4347)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4351)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4352 || n = 4373)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4355)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
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
.elseif(n = 4371)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
; 4376 to 4453
.if(n = 4376 || n = 4407)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4377 || n = 4378 || n = 4381 || n = 4383 || (n > 4386 && n < 4390) || n = 4396 || n = 4399 || n = 4401 || n = 4406 || n = 4409 || n = 4417 || n = 4419 || n = 4423 || n = 4424 || n = 4434 || n = 4435 || n = 4437 || n = 4441 || n = 4442 || n = 4447 || n = 4448 || n = 4453)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4379 || n = 4397 || n = 4415 || n = 4433 || n = 4451)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4380 || n = 4430)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4382 || n = 4400 || n = 4408 || n = 4414 || n = 4418 || n = 4436 || n = 4452)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4384 || n = 4402 || n = 4420 || n = 4438)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4385 || n = 4405 || n = 4425 || n = 4445)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4386 || n = 4404 || n = 4412 || n = 4422 || n = 4440)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4390 || n = 4450)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4391)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4392 || n = 4410 || n = 4428 || n = 4446)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4393 || n = 4395 || n = 4426 || n = 4443)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4394 || n = 4413 || n = 4432)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4398 || n = 4444)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	brk
	iny
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
.elseif(n = 4416)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	plp
.elseif(n = 4421)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4427)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4429)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4431)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4439)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4468 || n = 4472 || n = 4488 || n = 4490 || n = 4526)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4455 || n = 4459 || n = 4460 || n = 4466 || n = 4467 || n = 4471 || n = 4473 || n = 4477 || n = 4484 || n = 4489 || n = 4491 || n = 4495 || n = 4496 || n = 4501 || n = 4502 || n = 4507 || n = 4509 || n = 4514 || n = 4522)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4456 || n = 4474 || n = 4492 || n = 4510 || n = 4528)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4457)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4458 || n = 4476 || n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4461 || n = 4479 || n = 4483 || n = 4486 || n = 4497 || n = 4503 || n = 4504 || n = 4506 || n = 4516 || n = 4519 || n = 4521)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4462 || n = 4464 || n = 4482 || n = 4498 || n = 4518)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4463)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4465 || n = 4485 || n = 4525)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4469 || n = 4487 || n = 4505 || n = 4523)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4470 || n = 4508 || n = 4527)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4475 || n = 4511 || n = 4529)
	; 11 bytes
	txa
	ldx #(n-11)/18
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4478 || n = 4499 || n = 4520)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4480)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4481)
	; 12 bytes
	php
	tya
	ldy #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4493 || n = 4515)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4500 || n = 4531)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4513)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4517)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4524)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	delay_n 26
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4548 || n = 4552 || n = 4566 || n = 4591 || n = 4609)
	; 10..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4533 || n = 4538 || n = 4539 || n = 4543 || n = 4547 || n = 4549 || n = 4550 || n = 4556 || n = 4557 || n = 4561 || n = 4563 || (n > 4566 && n < 4570) || n = 4574 || n = 4579 || n = 4581 || n = 4586 || n = 4588 || n = 4592 || n = 4593 || n = 4597 || n = 4599 || n = 4604 || n = 4606)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4534 || n = 4540 || n = 4542 || n = 4560 || n = 4573 || n = 4575 || n = 4576)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4535)
	; 12 bytes
	tya
	ldy #151
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4536 || n = 4554 || n = 4558 || n = 4572 || n = 4578)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4537)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4541 || n = 4559 || n = 4577 || n = 4595)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4544 || n = 4580 || n = 4587 || n = 4598)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4545 || n = 4565 || n = 4585 || n = 4605)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4546 || n = 4603)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4551 || n = 4571)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4553 || n = 4596)
	; 11..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4555)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4562)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4564 || n = 4582 || n = 4600)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4570)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4583 || n = 4601)
	; 11 bytes
	tya
	ldy #(n-11)/18
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 4584)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 4589)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4590)
	; 10 bytes
	txa
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4594)
	; 12 bytes
	pha
	lda $A5
	delay_n (n-10)
	pla
.elseif(n = 4602)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 4607)
	; 10 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 4608)
	; 10 bytes
	txa
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	inc @zptemp,x
	tax
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
	nop
.elseif(n = 4611 || n = 4615 || n = 4617 || n = 4627)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4620 || n = 4632)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4613)
	; 9 bytes
	tya
	ldy #0
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4614)
	; 11 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4616 || n = 4646)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4618)
	; 11 bytes
	php
	sec
	lda #0
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4619)
	; 11 bytes
	txa
	ldx #0
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
.elseif(n = 4621)
	; 12 bytes
	tya
	ldy #0
	brk
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	nop
.elseif(n = 4622 || n = 4641)
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
	delay_n 11
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4624)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4625 || n = 4645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4626 || n = 4638)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4628)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4629)
	; 12 bytes
	php
	clc
	lda #36 ;hides 'bit zp'
	nop
	brk
	adc #1
	Jne *-6
	plp
.elseif(n = 4630)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4631)
	; 12 bytes
	php
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
	plp
.elseif(n = 4633)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif((n > 4633 && n < 4637) || n = 4639 || n = 4640 || n = 4642 || n = 4644)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4637)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4643)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 4647)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4648)
	; 11 bytes
	php
	clc
	lda #24 ;hides 'clc'
	brk
	adc #1
	Jne *-5
	plp
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 11 bytes
	txa
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4650 || n = 4652)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4651 || n = 4674)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4653 || n = 4657 || n = 4662 || n = 4667 || n = 4671 || n = 4681 || n = 4682 || n = 4687)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4654 || n = 4659 || n = 4663 || n = 4664 || n = 4666 || n = 4673 || n = 4675 || n = 4677 || n = 4684)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4655 || n = 4686)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4656 || n = 4678)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4658)
	; 12 bytes
	tya
	ldy #141
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 4660 || n = 4679)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4661)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4665 || n = 4685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4668)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4669)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4670)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4672)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	sta @zptemp
	brk
	Jne *-6
	tax
.elseif(n = 4676)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4680)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4683)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if(n = 4688)
	; 11 bytes
	php
	sec
	lda #234 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4689)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4690 || n = 4692 || n = 4712 || n = 4732 || n = 4752)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4691 || n = 4713 || n = 4735 || n = 4757)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4693 || n = 4699 || n = 4700 || n = 4707 || n = 4711 || n = 4715 || n = 4719 || n = 4727 || n = 4729 || n = 4737 || n = 4738 || n = 4747 || n = 4749 || n = 4753 || n = 4759)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4694 || n = 4701 || n = 4702 || n = 4704 || n = 4706 || n = 4714 || n = 4716 || n = 4721 || n = 4723 || n = 4734 || (n > 4738 && n < 4743) || n = 4744 || n = 4746 || n = 4754 || n = 4756 || n = 4761 || n = 4763 || n = 4764)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4695)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4696 || n = 4758)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4697 || n = 4720 || n = 4743)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4698 || n = 4717 || n = 4736 || n = 4755)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4703)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4705 || n = 4725 || n = 4745 || n = 4765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4708 || n = 4728)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4709 || n = 4751)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4710 || n = 4750)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4718)
	; 12 bytes
	php
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4722)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4724)
	; 12 bytes
	tya
	ldy #143
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 4726)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	tax
.elseif(n = 4730)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4731)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4733)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4748)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4760)
	; 12 bytes
	php
	sec
	lda #216 ;hides 'cld'
	nop
	brk
	sbc #1
	Jne *-6
	plp
.elseif(n = 4762)
	; 12 bytes
	php
	tya
	ldy #216 ;hides 'cld'
	nop
	brk
	dey
	Jne *-5
	tay
	plp
.endif
.else
; 4766 to 4843
.if(n = 4766 || n = 4789)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4767 || n = 4769 || n = 4776 || n = 4782 || n = 4787 || n = 4795 || n = 4807 || n = 4809 || n = 4816 || n = 4827 || n = 4833 || n = 4837 || n = 4843)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4768 || n = 4808 || n = 4828)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4770 || n = 4790)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4771 || n = 4791)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4772 || n = 4814 || n = 4835)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4773 || n = 4777 || n = 4783 || n = 4786 || n = 4794 || n = 4796 || n = 4797 || n = 4800 || n = 4802 || n = 4804 || n = 4806 || n = 4813 || n = 4815 || n = 4817 || n = 4819 || n = 4821 || n = 4822 || n = 4824 || n = 4826 || n = 4834 || n = 4839 || n = 4840 || n = 4842)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4774 || n = 4793 || n = 4812 || n = 4831)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4775)
	; 12 bytes
	tya
	ldy #159
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4778)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4779 || n = 4810 || n = 4841)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4780 || n = 4830)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4781 || n = 4829)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4784 || n = 4811)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4785 || n = 4805 || n = 4825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4788)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4792 || n = 4832)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4798 || n = 4818 || n = 4838)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4799)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	brk
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4801 || n = 4823)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4803)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4820)
	; 12 bytes
	php
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	plp
.elseif(n = 4836)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	brk
	inx
	inx
	Jne *-7
	tax
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
; 4844 to 4921
.if(n = 4844 || n = 4846 || n = 4859 || (n > 4860 && n < 4865) || n = 4866 || n = 4873 || n = 4876 || n = 4884 || n = 4886 || n = 4893 || n = 4894 || n = 4896 || n = 4897 || n = 4899 || n = 4902 || (n > 4913 && n < 4917) || n = 4920)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4845 || n = 4865 || n = 4885 || n = 4905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4847 || n = 4849 || n = 4852 || n = 4857 || n = 4860 || n = 4871 || n = 4874 || n = 4879 || n = 4882 || n = 4883 || n = 4887 || n = 4891 || n = 4900 || n = 4906 || n = 4907 || n = 4909 || n = 4913 || n = 4921)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4848 || n = 4868 || n = 4888 || n = 4908)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4850)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4851)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4853 || n = 4901)
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
.elseif(n = 4855 || n = 4880)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4856 || n = 4877 || n = 4898 || n = 4919)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4858 || n = 4881 || n = 4904)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4867 || n = 4889 || n = 4911)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4869)
	; 11 bytes
	tya
	ldy #0
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4870 || n = 4890 || n = 4910)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4872 || n = 4903)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
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
.elseif(n = 4878 || n = 4918)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4892 || n = 4912)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4895)
	; 12 bytes
	tya
	ldy #163
	jsr @nearby_rts
	brk
	dey
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
.endif
.else
; 4922 to 5000
.if(n = 4922)
	; 12 bytes
	tya
	ldy #149
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 4923 || n = 4924 || n = 4926 || n = 4937 || n = 4939 || n = 4943 || n = 4944 || n = 4953 || n = 4954 || n = 4956 || n = 4959 || n = 4960 || n = 4962 || n = 4964 || n = 4966 || n = 4974 || n = 4976 || n = 4981 || n = 4983 || n = 4986 || (n > 4992 && n < 4996))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4925 || n = 4945 || n = 4965 || n = 4985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4927 || n = 4929 || n = 4935 || n = 4936 || n = 4942 || n = 4947 || n = 4957 || n = 4963 || n = 4967 || n = 4969 || n = 4975 || n = 4979 || n = 4984 || n = 4987 || n = 4989 || n = 4998)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4928 || n = 4948)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4930 || n = 4980)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4931 || n = 4951 || n = 4991)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4933 || n = 4955 || n = 4977 || n = 4999)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4934 || n = 4996)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4938 || n = 4958 || n = 4978)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4940 || n = 4961 || n = 4982)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4941)
	; 12 bytes
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
.elseif(n = 4946 || n = 5000)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4949 || n = 4997)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4950 || n = 4973)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4968)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 4970)
	; 11 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	tay
	plp
.elseif(n = 4971)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 4988)
	; 12 bytes
	tya
	ldy #151
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 4990)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
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
.if(n = 5001 || n = 5007 || n = 5009 || n = 5023 || n = 5026 || n = 5029 || n = 5044 || n = 5047 || n = 5049 || n = 5057 || n = 5060 || n = 5067)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5002 || n = 5004 || n = 5006 || n = 5013 || n = 5014 || n = 5016 || n = 5017 || n = 5020 || n = 5022 || (n > 5032 && n < 5038) || (n > 5038 && n < 5042) || n = 5046 || n = 5053 || n = 5056 || n = 5059 || (n > 5060 && n < 5065) || n = 5073 || n = 5074 || n = 5076 || n = 5077)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5003 || n = 5024 || n = 5066)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5005 || n = 5025 || n = 5045 || n = 5065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 5008 || n = 5028 || n = 5048 || n = 5068)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5010 || n = 5050 || n = 5070)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 5011 || n = 5051)
	; 12 bytes
	txa
	ldx #(n-11)/20
	nop
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5015)
	; 12 bytes
	tya
	ldy #167
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5018 || n = 5038)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5019 || n = 5042)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5021 || n = 5043)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5027 || n = 5058)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5030 || n = 5055)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5031 || n = 5071)
	; 12 bytes
	tya
	ldy #(n-11)/20
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 5054)
	; 12 bytes
	tya
	ldy #153
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5069)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5075)
	; 12 bytes
	tya
	ldy #169
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5078)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	tax
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5086 || n = 5095 || n = 5104)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5080)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5081)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5082 || n = 5091 || n = 5096 || n = 5098 || n = 5102 || n = 5103 || n = 5116)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5083 || n = 5099)
	; 11..12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5084)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 5085 || n = 5105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 5087 || n = 5109)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5088 || n = 5111)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5089)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5090 || n = 5110)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	nop
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 5092 || n = 5101 || n = 5112)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5093)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5094)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 5097 || n = 5117)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5100)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	nop
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 5106)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5107)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 5108)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5113)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	brk
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 5114)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 5115)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	lda $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 5118 to 5156
.if(n = 5118)
	; 11 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
	plp
.elseif(n = 5119)
	; 11 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 5120)
	; 11 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	plp
.elseif(n = 5121 || n = 5132)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5122 || n = 5123 || n = 5127 || n = 5133 || n = 5136 || n = 5152)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5124 || n = 5137 || n = 5139 || n = 5140 || (n > 5141 && n < 5150))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5125)
	; 10 bytes
	tya
	ldy #0
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 5126)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5128 || n = 5154)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5129 || n = 5150)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5130 || n = 5155)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5131 || n = 5153)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5134)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5135)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5138)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5141)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5151)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5156)
	; 12 bytes
	php
	sec
	lda #234 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	plp
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157 || n = 5203 || n = 5226 || n = 5249 || n = 5272 || n = 5295)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5158)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5159 || n = 5173 || n = 5177 || n = 5184 || n = 5199 || n = 5207 || n = 5215 || n = 5221 || n = 5228 || n = 5232 || n = 5236 || n = 5246 || n = 5251 || n = 5257 || n = 5265 || n = 5274 || n = 5277 || n = 5278 || n = 5282 || n = 5287 || n = 5299 || n = 5308)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5160 || n = 5161 || n = 5163 || (n > 5165 && n < 5169) || n = 5170 || n = 5172 || n = 5174 || n = 5176 || n = 5178 || n = 5179 || n = 5181 || n = 5183 || n = 5185 || n = 5187 || n = 5188 || n = 5190 || n = 5191 || n = 5193 || n = 5196 || n = 5198 || n = 5200 || n = 5201 || n = 5204 || n = 5206 || n = 5208 || (n > 5209 && n < 5213) || n = 5214 || n = 5217 || n = 5218 || n = 5220 || (n > 5221 && n < 5225) || n = 5227 || n = 5229 || n = 5231 || n = 5233 || n = 5235 || (n > 5237 && n < 5241) || n = 5242 || n = 5245 || n = 5247 || n = 5248 || n = 5250 || n = 5253 || n = 5254 || (n > 5257 && n < 5261) || n = 5262 || n = 5264 || (n > 5266 && n < 5270) || n = 5273 || n = 5279 || n = 5283 || n = 5284 || n = 5286 || (n > 5287 && n < 5295) || n = 5296 || (n > 5299 && n < 5303) || n = 5304 || (n > 5309 && n < 5313))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5162 || n = 5216 || n = 5243 || n = 5270)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5164)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5165 || n = 5189 || n = 5237 || n = 5261 || n = 5309)
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
	pha
	ldy #21 ;hides 'ora zp,x'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
.elseif(n = 5171 || n = 5192 || n = 5234 || n = 5276 || n = 5297)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5175 || n = 5197 || n = 5219 || n = 5241 || n = 5263 || n = 5285 || n = 5307)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5180 || n = 5205 || n = 5230 || n = 5255 || n = 5280 || n = 5305)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5182 || n = 5213 || n = 5244 || n = 5275 || n = 5306)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5186)
	; 12 bytes
	tya
	ldy #157
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5194)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	brk
	inx
	inx
	Jne *-7
	tax
.elseif(n = 5195)
	; 12 bytes
	tya
	ldy #173
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5202)
	; 12 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	pla
	tay
.elseif(n = 5209)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	delay_n 26
	Jne *-7
	tay
.elseif(n = 5225)
	; 12 bytes
	tya
	ldy #174
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5252)
	; 12 bytes
	tya
	ldy #159
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5256)
	; 12 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	brk
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5266)
	; 12 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5271)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
.elseif(n = 5281)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 21
	Jne *-7
	tax
.elseif(n = 5298)
	; 12 bytes
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5303)
	; 12 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	pla
	tay
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if(n = 5313 || n = 5314 || n = 5317 || n = 5319 || (n > 5320 && n < 5324) || (n > 5324 && n < 5329) || (n > 5333 && n < 5337) || n = 5338 || n = 5340 || n = 5342 || n = 5344 || (n > 5345 && n < 5349) || n = 5350 || n = 5359 || n = 5361 || n = 5363 || n = 5365 || n = 5367 || n = 5369 || n = 5379 || n = 5386 || n = 5388 || n = 5390)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5315)
	; 12 bytes
	tya
	ldy #177
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5316)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
	tax
.elseif(n = 5318 || n = 5339 || n = 5360)
	; 11 bytes
	tya
	ldy #(n-5)/21
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5320 || n = 5331 || n = 5332 || n = 5343 || n = 5353 || n = 5362 || n = 5366 || n = 5372 || n = 5374 || n = 5382 || n = 5383 || n = 5389)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5324 || n = 5378)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5329 || n = 5351 || n = 5373)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5330 || n = 5355 || n = 5380)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5333 || n = 5357)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5337 || n = 5368)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5341 || n = 5364 || n = 5387)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5345)
	; 12 bytes
	tya
	ldy #178
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5349)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	sta @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 5352)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5354)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5356)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	iny
	Jne *-5
	tay
	plp
.elseif(n = 5358)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	dex
	dex
	Jne *-7
	tax
.elseif(n = 5370)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	sta @zptemp
	dey
	Jne *-5
	tay
.elseif(n = 5371)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	lda $A5A5 ;hides 'lda zp'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5375)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	sbc #1
	Jne *-6
	plp
.elseif(n = 5376)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	inc @zptemp
	brk
	dey
	dey
	Jne *-7
	tay
.elseif(n = 5377)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5381)
	; 11 bytes
	tya
	ldy #0
	sta @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5384)
	; 12 bytes
	tya
	ldy #163
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5385)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	sta @zptemp
	brk
	dey
	Jne *-6
	tay
.endif
.else
; 5391 to 5468
.if((n > 5390 && n < 5395) || n = 5396 || n = 5398 || n = 5400 || n = 5402 || n = 5404 || n = 5406 || n = 5408 || n = 5409 || n = 5411 || (n > 5412 && n < 5417) || n = 5418 || (n > 5419 && n < 5429) || n = 5431 || n = 5434 || (n > 5435 && n < 5439) || n = 5440 || (n > 5441 && n < 5450) || n = 5451 || n = 5452 || n = 5454 || n = 5460 || n = 5462 || n = 5467 || n = 5468)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5395 || n = 5417 || n = 5439)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5397 || n = 5401 || n = 5407 || n = 5412 || n = 5419 || n = 5432 || n = 5435 || n = 5441 || n = 5457 || n = 5458 || n = 5463)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5399 || n = 5430 || n = 5461)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5403)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5405 || n = 5455)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5410 || n = 5433 || n = 5456)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5429 || n = 5453)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5450)
	; 12 bytes
	tya
	ldy #165
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5459)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5464)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	nop
	brk
	sbc #1
	Jne *-6
	plp
.elseif(n = 5465)
	; 12 bytes
	tya
	ldy #182
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5466)
	; 12 bytes
	php
	tya
	ldy #248 ;hides 'sed'
	nop
	brk
	dey
	Jne *-5
	tay
	plp
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5477) || n = 5478 || (n > 5486 && n < 5492) || n = 5493 || (n > 5495 && n < 5501) || n = 5503 || n = 5506 || (n > 5507 && n < 5513) || n = 5514 || n = 5515 || (n > 5516 && n < 5523) || n = 5524 || n = 5526 || n = 5528 || n = 5531 || (n > 5532 && n < 5540) || (n > 5540 && n < 5547) || n = 5552 || n = 5553 || (n > 5557 && n < 5567) || (n > 5567 && n < 5571) || n = 5572 || (n > 5573 && n < 5580) || n = 5581 || n = 5583 || n = 5584 || n = 5586 || (n > 5587 && n < 5593) || (n > 5597 && n < 5604) || (n > 5611 && n < 5615) || n = 5620 || (n > 5621 && n < 5626))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5477 || n = 5501 || n = 5573 || n = 5597 || n = 5621)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5479 || n = 5502 || n = 5525 || n = 5548 || n = 5594 || n = 5617)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5480 || n = 5530 || n = 5555 || n = 5580 || n = 5605)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5481 || n = 5482 || n = 5485 || n = 5494 || n = 5504 || n = 5507 || n = 5529 || n = 5532 || n = 5550 || n = 5551 || n = 5556 || n = 5557 || n = 5587 || n = 5595 || n = 5596 || (n > 5605 && n < 5609) || n = 5618)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5483 || n = 5505 || n = 5527 || n = 5549 || n = 5571 || n = 5593 || n = 5615)
	; 11 bytes
	tya
	ldy #(n-5)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5484)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	brk
	Jne *-6
	tax
.elseif(n = 5486 || n = 5513 || n = 5540 || n = 5567)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5492 || n = 5523 || n = 5554 || n = 5585 || n = 5616)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5495)
	; 12 bytes
	tya
	ldy #183
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5516)
	; 12 bytes
	tya
	ldy #167
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5547)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	brk
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5582)
	; 12 bytes
	tya
	ldy #169
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5604)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 5609)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5610)
	; 12 bytes
	txa
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 4
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 5611)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5619)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	brk
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
; 5626 to 5781
.if(n = 5626)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 5627)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	lda $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5628 || n = 5639 || n = 5642 || n = 5649 || n = 5657 || n = 5665 || n = 5682 || n = 5688 || n = 5707 || n = 5711 || n = 5734 || n = 5742 || n = 5757 || n = 5773)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5629)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5630 || n = 5655 || n = 5680 || n = 5705 || n = 5730 || n = 5755 || n = 5780)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5631)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 6
	sbc #1
	Jne *-6
	plp
.elseif(n = 5632)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	plp
.elseif(n = 5633)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n > 5633 && n < 5637) || n = 5638 || n = 5641 || n = 5643 || n = 5644 || n = 5646 || (n > 5649 && n < 5655) || n = 5656 || (n > 5657 && n < 5663) || n = 5664 || (n > 5665 && n < 5669) || (n > 5669 && n < 5675) || n = 5676 || n = 5677 || n = 5679 || (n > 5682 && n < 5686) || (n > 5688 && n < 5693) || (n > 5693 && n < 5702) || n = 5703 || n = 5704 || n = 5706 || n = 5708 || n = 5710 || n = 5712 || n = 5713 || n = 5715 || n = 5716 || (n > 5717 && n < 5729) || n = 5731 || n = 5733 || (n > 5735 && n < 5740) || (n > 5742 && n < 5747) || (n > 5747 && n < 5755) || (n > 5757 && n < 5765) || (n > 5765 && n < 5771) || n = 5772 || (n > 5774 && n < 5778) || n = 5779 || n = 5781)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5637)
	; 11 bytes
	tya
	ldy #0
	delay_n 4
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5640 || n = 5663 || n = 5686 || n = 5732 || n = 5778)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5645 || n = 5669 || n = 5693 || n = 5717 || n = 5741 || n = 5765)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5647 || n = 5678 || n = 5709 || n = 5740 || n = 5771)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5648)
	; 12 bytes
	tya
	ldy #171
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5675 || n = 5702 || n = 5729 || n = 5756)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5681)
	; 12 bytes
	tya
	ldy #172
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5687)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 23
	Jne *-7
	tax
.elseif(n = 5714)
	; 12 bytes
	tya
	ldy #173
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5735)
	; 12 bytes
	tya
	ldy #191
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5747)
	; 12 bytes
	tya
	ldy #174
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5774)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	brk
	dey
	dey
	Jne *-7
	tay
.endif
.else
; 5782 to 5937
.if(n = 5782 || n = 5804 || n = 5807 || n = 5826 || n = 5832 || n = 5835 || n = 5849 || n = 5857 || n = 5861 || n = 5863 || n = 5866 || n = 5872 || n = 5884 || n = 5886 || n = 5897 || n = 5907 || n = 5928 || n = 5932)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5783 || n = 5810 || n = 5891 || n = 5918)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif((n > 5783 && n < 5789) || (n > 5789 && n < 5795) || (n > 5795 && n < 5801) || n = 5806 || n = 5808 || n = 5809 || n = 5811 || n = 5812 || (n > 5813 && n < 5824) || (n > 5826 && n < 5830) || n = 5831 || n = 5834 || n = 5836 || (n > 5837 && n < 5846) || n = 5848 || (n > 5849 && n < 5853) || n = 5854 || n = 5856 || n = 5858 || n = 5860 || (n > 5866 && n < 5870) || n = 5871 || (n > 5872 && n < 5879) || n = 5881 || n = 5883 || n = 5892 || n = 5894 || n = 5896 || (n > 5897 && n < 5905) || n = 5906 || n = 5908 || n = 5910 || n = 5911 || n = 5913 || n = 5914 || n = 5916 || n = 5917 || (n > 5918 && n < 5926) || n = 5927 || n = 5929 || n = 5931 || (n > 5933 && n < 5938))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5789 || n = 5813 || n = 5837 || n = 5885 || n = 5909 || n = 5933)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5795)
	; 12 bytes
	tya
	ldy #193
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5801 || n = 5824 || n = 5847 || n = 5870)
	; 11 bytes
	tya
	ldy #(n-5)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5802 || n = 5833 || n = 5864 || n = 5895 || n = 5926)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5803)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	inc @zptemp
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5805 || n = 5830 || n = 5855 || n = 5880 || n = 5905 || n = 5930)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5825)
	; 12 bytes
	tya
	ldy #194
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5846)
	; 12 bytes
	tya
	ldy #177
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5853)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	inc @zptemp
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5859)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
.elseif(n = 5862)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5865)
	; 12 bytes
	txa
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	inc @zptemp
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 5879)
	; 12 bytes
	tya
	ldy #178
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5882)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc @zptemp
	dey
	Jne *-5
	tay
.elseif(n = 5887)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	inc @zptemp
	sbc #1
	Jne *-6
	plp
.elseif(n = 5888)
	; 12 bytes
	txa
	ldx #0 ;hides 'brk imm'
	nop
	inc @zptemp
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 5889)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5890)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
.elseif(n = 5893)
	; 11 bytes
	tya
	ldy #0
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5912)
	; 12 bytes
	tya
	ldy #179
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5915)
	; 12 bytes
	tya
	ldy #197
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if((n > 5937 && n < 5945) || (n > 5945 && n < 5955) || n = 5956 || n = 5958 || (n > 5959 && n < 5972) || n = 5973 || n = 5974 || n = 5976 || n = 5977 || n = 5979 || (n > 5982 && n < 5988) || n = 5989 || (n > 5990 && n < 5999) || (n > 5999 && n < 6005) || n = 6006 || n = 6008 || n = 6010 || (n > 6011 && n < 6019) || n = 6020 || (n > 6021 && n < 6026) || n = 6027 || n = 6028 || n = 6033 || n = 6034 || (n > 6035 && n < 6044) || (n > 6044 && n < 6050) || n = 6051 || n = 6054 || n = 6056 || (n > 6057 && n < 6065) || (n > 6065 && n < 6077) || n = 6078 || n = 6079 || (n > 6083 && n < 6087) || (n > 6087 && n < 6093))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5945)
	; 12 bytes
	tya
	ldy #180
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5955 || n = 5980 || n = 6005 || n = 6030 || n = 6055 || n = 6080)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5957 || n = 5988 || n = 6019 || n = 6050 || n = 6081)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5959 || n = 5982 || n = 5990 || n = 6007 || n = 6021 || n = 6032 || n = 6052 || n = 6057 || n = 6082 || n = 6083)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5972 || n = 5999 || n = 6026)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5975)
	; 12 bytes
	tya
	ldy #199
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5978)
	; 12 bytes
	tya
	ldy #181
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5981 || n = 6029 || n = 6053 || n = 6077)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6009)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	brk
	dex
	dex
	Jne *-7
	tax
.elseif(n = 6011)
	; 12 bytes
	tya
	ldy #182
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6031)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 39
	Jne *-7
	tay
.elseif(n = 6035)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 6044)
	; 12 bytes
	tya
	ldy #183
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6065)
	; 12 bytes
	tya
	ldy #202
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.elseif(n = 6093)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk
	Jne *-6
	tax
.endif
.else
; 6094 to 6250
.if(n = 6094 || (n > 6095 && n < 6101) || (n > 6101 && n < 6105) || n = 6106 || n = 6108 || n = 6109 || n = 6111 || n = 6113 || n = 6115 || (n > 6116 && n < 6125) || (n > 6125 && n < 6130) || n = 6131 || n = 6133 || (n > 6134 && n < 6138) || n = 6139 || n = 6141 || n = 6142 || n = 6144 || (n > 6145 && n < 6149) || (n > 6149 && n < 6155) || n = 6156 || (n > 6157 && n < 6161) || (n > 6161 && n < 6174) || n = 6175 || (n > 6176 && n < 6180) || n = 6181 || n = 6183 || n = 6184 || n = 6186 || n = 6187 || (n > 6188 && n < 6205) || n = 6206 || n = 6208 || (n > 6209 && n < 6215) || (n > 6215 && n < 6230) || n = 6231 || (n > 6232 && n < 6236) || n = 6237 || n = 6239 || n = 6241 || n = 6243 || n = 6244 || (n > 6245 && n < 6251))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6095)
	; 12 bytes
	tya
	ldy #203
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6101 || n = 6125)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6105 || n = 6130 || n = 6155 || n = 6180 || n = 6230)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6107 || n = 6134 || n = 6161 || n = 6188 || n = 6215)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6110)
	; 12 bytes
	tya
	ldy #185
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6112 || n = 6143 || n = 6174 || n = 6205 || n = 6236)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6114)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 6116 || n = 6132 || n = 6140 || n = 6145 || n = 6157 || n = 6182 || n = 6207 || n = 6232 || n = 6238)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6138)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 8
	dey
	Jne *-6
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
.elseif(n = 6176)
	; 12 bytes
	tya
	ldy #187
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6185)
	; 12 bytes
	tya
	ldy #206
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6209)
	; 12 bytes
	tya
	ldy #188
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6240)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	brk
	dex
	dex
	Jne *-7
	tax
.elseif(n = 6242)
	; 12 bytes
	tya
	ldy #189
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6245)
	; 12 bytes
	tya
	ldy #208
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
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
.if((n > 6250 && n < 6255) || n = 6256 || (n > 6257 && n < 6267) || (n > 6269 && n < 6275) || (n > 6275 && n < 6280) || n = 6281 || (n > 6282 && n < 6296) || n = 6297 || n = 6304 || n = 6306 || (n > 6309 && n < 6321) || n = 6322 || (n > 6323 && n < 6329))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6255 || n = 6280 || n = 6305)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6257 || n = 6269 || n = 6282 || n = 6300 || n = 6307)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6267 || n = 6298)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6268)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 26
	inx
	inx
	Jne *-7
	tax
.elseif(n = 6275)
	; 12 bytes
	tya
	ldy #190
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6296)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
.elseif(n = 6299)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 6301)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6302)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	sta (@ptrtemp,x)
	tax
.elseif(n = 6303)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6308)
	; 12 bytes
	tya
	ldy #191
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6309)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 6321)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6323)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.endif
.else
; 6329 to 6406
.if(n = 6329 || n = 6360 || n = 6391)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6330 || n = 6355 || n = 6380)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6331 || n = 6332 || n = 6357 || n = 6362 || n = 6371 || n = 6373 || n = 6382 || n = 6393 || n = 6396 || n = 6398)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6333 || n = 6334 || (n > 6335 && n < 6341) || (n > 6341 && n < 6350) || (n > 6350 && n < 6355) || n = 6356 || n = 6358 || n = 6359 || n = 6361 || n = 6363 || n = 6364 || (n > 6365 && n < 6369) || n = 6370 || n = 6378 || n = 6379 || n = 6381 || (n > 6382 && n < 6391) || n = 6392 || n = 6402 || n = 6406)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6335)
	; 12 bytes
	tya
	ldy #211
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6341)
	; 12 bytes
	tya
	ldy #192
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6350 || n = 6377 || n = 6404)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6365)
	; 12 bytes
	tya
	ldy #212
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6369)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 6372 || n = 6397)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 6374)
	; 12 bytes
	tya
	ldy #193
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6375)
	; 12 bytes
	txa
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 6376 || n = 6401)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6394)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 6395)
	; 12 bytes
	tya
	ldy #213
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6399)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 6400)
	; 12 bytes
	txa
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 6403)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6405)
	; 11 bytes
	tya
	ldy #0
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 6407 to 6562
.if(n = 6407)
	; 12 bytes
	tya
	ldy #194
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif((n > 6407 && n < 6422) || n = 6423 || (n > 6425 && n < 6431) || (n > 6431 && n < 6440) || (n > 6440 && n < 6453) || n = 6454 || n = 6456 || n = 6457 || (n > 6458 && n < 6473) || (n > 6473 && n < 6484) || (n > 6486 && n < 6499) || (n > 6499 && n < 6506) || (n > 6506 && n < 6512) || n = 6513 || n = 6514 || n = 6516 || (n > 6517 && n < 6539) || (n > 6539 && n < 6545) || n = 6547 || (n > 6548 && n < 6563))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6422 || n = 6453 || n = 6484 || n = 6515 || n = 6546)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6424 || n = 6455 || n = 6486 || n = 6517 || n = 6548)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6425)
	; 12 bytes
	tya
	ldy #214
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6431 || n = 6458 || n = 6485 || n = 6512)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6440)
	; 12 bytes
	tya
	ldy #195
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6473)
	; 12 bytes
	tya
	ldy #196
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6499)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
	tax
.elseif(n = 6506)
	; 12 bytes
	tya
	ldy #197
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6539)
	; 12 bytes
	tya
	ldy #198
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6545)
	; 12 bytes
	tya
	ldy #218
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
; 6563 to 6718
.if((n > 6562 && n < 6566) || (n > 6566 && n < 6572) || n = 6573 || n = 6574 || n = 6576 || n = 6578 || (n > 6579 && n < 6593) || (n > 6593 && n < 6603) || n = 6604 || n = 6606 || n = 6607 || n = 6609 || (n > 6610 && n < 6620) || (n > 6620 && n < 6624) || (n > 6624 && n < 6635) || n = 6636 || n = 6637 || n = 6640 || (n > 6641 && n < 6647) || n = 6648 || n = 6649 || (n > 6650 && n < 6665) || (n > 6665 && n < 6670) || n = 6673 || (n > 6674 && n < 6695) || (n > 6695 && n < 6701) || (n > 6704 && n < 6719))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6566 || n = 6593 || n = 6620 || n = 6647 || n = 6674)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6572)
	; 12 bytes
	tya
	ldy #199
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6575)
	; 12 bytes
	tya
	ldy #219
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6577 || n = 6608 || n = 6639 || n = 6670 || n = 6701)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6579 || n = 6610 || n = 6641 || n = 6672 || n = 6703)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 26
	iny
	Jne *-6
	tay
.elseif(n = 6605)
	; 12 bytes
	tya
	ldy #200
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6624)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 8
	iny
	Jne *-7
	tay
.elseif(n = 6635)
	; 12 bytes
	tya
	ldy #221
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6638)
	; 12 bytes
	tya
	ldy #201
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6650)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 6665)
	; 12 bytes
	tya
	ldy #222
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6671)
	; 12 bytes
	tya
	ldy #202
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6695)
	; 12 bytes
	tya
	ldy #223
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6702)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 26
	Jne *-6
	tax
.elseif(n = 6704)
	; 12 bytes
	tya
	ldy #203
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.endif
.else
; 6719 to 6875
.if((n > 6718 && n < 6725) || n = 6726 || n = 6727 || n = 6729 || n = 6730 || n = 6733 || n = 6735 || n = 6736 || (n > 6737 && n < 6755) || (n > 6755 && n < 6763) || n = 6764 || (n > 6765 && n < 6770) || (n > 6770 && n < 6782) || n = 6783 || n = 6784 || (n > 6785 && n < 6789) || (n > 6789 && n < 6794) || n = 6795 || (n > 6796 && n < 6803) || (n > 6803 && n < 6809) || (n > 6809 && n < 6815) || (n > 6815 && n < 6825) || n = 6826 || (n > 6827 && n < 6836) || (n > 6836 && n < 6845) || (n > 6845 && n < 6856) || n = 6857 || (n > 6858 && n < 6863) || (n > 6863 && n < 6869) || (n > 6869 && n < 6875))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6725)
	; 12 bytes
	tya
	ldy #224
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6728 || n = 6755 || n = 6782 || n = 6809 || n = 6863)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6731)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6732 || n = 6763 || n = 6794 || n = 6825 || n = 6856)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6734 || n = 6765 || n = 6796 || n = 6827 || n = 6858)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6737)
	; 12 bytes
	tya
	ldy #204
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6770)
	; 12 bytes
	tya
	ldy #205
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6785)
	; 12 bytes
	tya
	ldy #226
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6789)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6803)
	; 12 bytes
	tya
	ldy #206
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6815)
	; 12 bytes
	tya
	ldy #227
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6836)
	; 12 bytes
	tya
	ldy #207
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6845)
	; 12 bytes
	tya
	ldy #228
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6869)
	; 12 bytes
	tya
	ldy #208
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6875)
	; 12 bytes
	tya
	ldy #229
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
; 6876 to 7031
.if((n > 6875 && n < 6879) || n = 6880 || (n > 6881 && n < 6887) || n = 6888 || (n > 6890 && n < 6902) || n = 6903 || n = 6904 || n = 6907 || (n > 6908 && n < 6917) || n = 6919 || (n > 6920 && n < 6935) || (n > 6935 && n < 6949) || n = 6950 || (n > 6951 && n < 6965) || n = 6966 || n = 6967 || (n > 6968 && n < 6980) || n = 6981 || (n > 6982 && n < 6995) || (n > 6995 && n < 7001) || n = 7002 || (n > 7003 && n < 7011) || n = 7012 || (n > 7013 && n < 7025) || (n > 7025 && n < 7032))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6879)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 6881 || n = 6889 || n = 6908 || n = 6920 || n = 6951 || n = 6982 || n = 7013)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6887 || n = 6918 || n = 6949 || n = 6980 || n = 7011)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6890)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6902)
	; 12 bytes
	tya
	ldy #209
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6905)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	sta @zptemp
	delay_n 26
	Jne *-7
	tax
.elseif(n = 6906)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 6917)
	; 12 bytes
	tya
	ldy #0
	delay_n 9
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6935)
	; 12 bytes
	tya
	ldy #210
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6965)
	; 12 bytes
	tya
	ldy #232
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6968)
	; 12 bytes
	tya
	ldy #211
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6995)
	; 12 bytes
	tya
	ldy #233
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7001)
	; 12 bytes
	tya
	ldy #212
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7003)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 7025)
	; 12 bytes
	tya
	ldy #234
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.endif
.else
; 7032 to 7187
.if(n = 7032 || n = 7033 || (n > 7034 && n < 7042) || n = 7043 || (n > 7044 && n < 7055) || (n > 7055 && n < 7067) || (n > 7067 && n < 7073) || n = 7074 || (n > 7075 && n < 7085) || (n > 7085 && n < 7100) || (n > 7100 && n < 7104) || n = 7105 || n = 7107 || (n > 7108 && n < 7115) || (n > 7115 && n < 7133) || n = 7136 || (n > 7137 && n < 7145) || (n > 7145 && n < 7162) || (n > 7162 && n < 7166) || n = 7167 || (n > 7168 && n < 7175) || (n > 7175 && n < 7188))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7034)
	; 12 bytes
	tya
	ldy #213
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7042 || n = 7073 || n = 7104 || n = 7135 || n = 7166)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7044 || n = 7075 || n = 7106 || n = 7137 || n = 7168)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7055)
	; 12 bytes
	tya
	ldy #235
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7067)
	; 12 bytes
	tya
	ldy #214
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7085)
	; 12 bytes
	tya
	ldy #236
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7100)
	; 12 bytes
	tya
	ldy #215
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7108)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 26
	Jne *-7
	tax
.elseif(n = 7115)
	; 12 bytes
	tya
	ldy #237
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7133)
	; 12 bytes
	tya
	ldy #216
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7134)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	tay
.elseif(n = 7145)
	; 12 bytes
	tya
	ldy #238
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7162)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
.elseif(n = 7175)
	; 12 bytes
	tya
	ldy #239
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.endif
.endif
.else
; 7188 to 7500
.if((n > 7187 && n < 7197) || n = 7198 || (n > 7199 && n < 7205) || (n > 7205 && n < 7228) || n = 7229 || n = 7231 || n = 7233 || n = 7234 || (n > 7235 && n < 7259) || n = 7260 || (n > 7261 && n < 7265) || (n > 7265 && n < 7290) || n = 7291 || n = 7293 || n = 7294 || n = 7296 || n = 7297 || (n > 7298 && n < 7321) || n = 7322 || n = 7324 || (n > 7325 && n < 7331) || (n > 7331 && n < 7352) || n = 7353 || (n > 7355 && n < 7364) || (n > 7364 && n < 7383) || n = 7384 || (n > 7385 && n < 7389) || (n > 7389 && n < 7397) || (n > 7397 && n < 7414) || n = 7417 || (n > 7418 && n < 7427) || n = 7428 || n = 7429 || (n > 7430 && n < 7445) || n = 7446 || (n > 7447 && n < 7463) || (n > 7463 && n < 7475) || n = 7477 || (n > 7478 && n < 7491) || (n > 7491 && n < 7496) || (n > 7496 && n < 7501))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7197 || n = 7228 || n = 7259 || n = 7290 || n = 7321 || n = 7352 || n = 7383 || n = 7414 || n = 7445 || n = 7476)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7199)
	; 12 bytes
	tya
	ldy #218
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7205)
	; 12 bytes
	tya
	ldy #240
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7230 || n = 7261 || n = 7292 || n = 7323 || n = 7354 || n = 7385 || n = 7416 || n = 7447 || n = 7478)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7232)
	; 12 bytes
	tya
	ldy #219
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7235)
	; 12 bytes
	tya
	ldy #241
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7265)
	; 12 bytes
	tya
	ldy #220
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7295)
	; 12 bytes
	tya
	ldy #243
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7298)
	; 12 bytes
	tya
	ldy #221
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7325)
	; 12 bytes
	tya
	ldy #244
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7331)
	; 12 bytes
	tya
	ldy #222
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7355)
	; 12 bytes
	tya
	ldy #245
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7364)
	; 12 bytes
	tya
	ldy #223
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7389)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 11
	iny
	Jne *-7
	tay
.elseif(n = 7397)
	; 12 bytes
	tya
	ldy #224
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7415)
	; 12 bytes
	tya
	ldy #247
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7418)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
.elseif(n = 7427)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
	tay
.elseif(n = 7430)
	; 12 bytes
	tya
	ldy #225
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7463)
	; 12 bytes
	tya
	ldy #226
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7475)
	; 12 bytes
	tya
	ldy #249
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7491)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
.elseif(n = 7496)
	; 12 bytes
	tya
	ldy #227
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
; 7501 to 7812
.if((n > 7500 && n < 7505) || n = 7506 || n = 7508 || (n > 7509 && n < 7529) || (n > 7529 && n < 7535) || n = 7536 || n = 7537 || n = 7539 || (n > 7540 && n < 7562) || n = 7563 || n = 7564 || (n > 7565 && n < 7569) || n = 7570 || (n > 7571 && n < 7595) || (n > 7595 && n < 7600) || n = 7601 || (n > 7602 && n < 7625) || n = 7626 || n = 7627 || n = 7629 || n = 7630 || n = 7632 || (n > 7633 && n < 7644) || n = 7645 || (n > 7646 && n < 7655) || (n > 7655 && n < 7659) || n = 7660 || n = 7663 || (n > 7664 && n < 7674) || n = 7675 || (n > 7676 && n < 7685) || (n > 7685 && n < 7693) || (n > 7695 && n < 7717) || (n > 7717 && n < 7724) || (n > 7727 && n < 7755) || n = 7756 || n = 7758 || n = 7759 || (n > 7760 && n < 7786) || n = 7787 || (n > 7788 && n < 7793) || (n > 7793 && n < 7813))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7505)
	; 12 bytes
	tya
	ldy #250
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7507 || n = 7538 || n = 7569 || n = 7600 || n = 7631 || n = 7662 || n = 7693 || n = 7724 || n = 7755 || n = 7786)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7509 || n = 7540 || n = 7571 || n = 7602 || n = 7633 || n = 7646 || n = 7664 || n = 7676 || n = 7695 || n = 7726 || n = 7757 || n = 7788)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7529)
	; 12 bytes
	tya
	ldy #228
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7535)
	; 12 bytes
	tya
	ldy #251
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7562)
	; 12 bytes
	tya
	ldy #229
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7565)
	; 12 bytes
	tya
	ldy #252
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7595)
	; 12 bytes
	tya
	ldy #230
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7625)
	; 12 bytes
	tya
	ldy #254
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7628)
	; 12 bytes
	tya
	ldy #231
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7644)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 7655)
	; 12 bytes
	tya
	ldy #255
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7659)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-6
	tay
.elseif(n = 7661)
	; 12 bytes
	tya
	ldy #232
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7674)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 7685)
	; 12 bytes
	tya
	ldy #0
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7694)
	; 12 bytes
	tya
	ldy #233
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7717)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 26
	Jne *-7
	tax
.elseif(n = 7725)
	; 11 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7727)
	; 12 bytes
	tya
	ldy #234
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7760)
	; 12 bytes
	tya
	ldy #235
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7793)
	; 12 bytes
	tya
	ldy #236
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 7813) && (n < 7969))
; 7813 to 7968
.if((n > 7812 && n < 7817) || n = 7818 || (n > 7819 && n < 7826) || (n > 7826 && n < 7848) || n = 7849 || (n > 7850 && n < 7859) || (n > 7859 && n < 7879) || n = 7880 || (n > 7881 && n < 7892) || (n > 7892 && n < 7899) || n = 7900 || (n > 7906 && n < 7910) || n = 7911 || (n > 7912 && n < 7921) || n = 7922 || n = 7923 || (n > 7926 && n < 7930) || n = 7931 || (n > 7937 && n < 7941) || n = 7942 || (n > 7943 && n < 7958) || (n > 7958 && n < 7969))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7817 || n = 7848 || n = 7879 || n = 7910)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7819 || n = 7850 || n = 7881 || n = 7901 || n = 7903 || n = 7912 || n = 7932 || n = 7934 || n = 7943)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7826)
	; 12 bytes
	tya
	ldy #237
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7859)
	; 12 bytes
	tya
	ldy #238
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7892)
	; 12 bytes
	tya
	ldy #239
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7899)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 7902 || n = 7933)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 7904)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7905)
	; 12 bytes
	txa
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk
	dex
	Jne *-5
	inc @zptemp,x
	tax
.elseif(n = 7906 || n = 7937)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 7921)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 7924)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	plp
.elseif(n = 7925)
	; 12 bytes
	tya
	ldy #240
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7926)
	; 12 bytes
	php
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	plp
.elseif(n = 7930)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif(n = 7935)
	; 12 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	brk
	sbc #1
	Jne *-6
	plp
.elseif(n = 7936)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 7941)
	; 11 bytes
	tya
	ldy #0
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7958)
	; 12 bytes
	tya
	ldy #241
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7991) || (n > 7991 && n < 8024) || (n > 8024 && n < 8057) || (n > 8057 && n < 8088) || n = 8089 || (n > 8090 && n < 8123) || n = 8124 || n = 8125)
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7991)
	; 12 bytes
	tya
	ldy #242
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8024)
	; 12 bytes
	tya
	ldy #243
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8057)
	; 12 bytes
	tya
	ldy #244
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8088)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 26
	dex
	dex
	Jne *-7
	tax
.elseif(n = 8090)
	; 12 bytes
	tya
	ldy #245
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8123)
	; 12 bytes
	tya
	ldy #246
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
; 8126 to 8750
.if((n > 8125 && n < 8154) || n = 8155 || (n > 8156 && n < 8186) || n = 8187 || n = 8188 || (n > 8189 && n < 8193) || (n > 8193 && n < 8222) || (n > 8222 && n < 8255) || (n > 8255 && n < 8288) || (n > 8288 && n < 8321) || (n > 8321 && n < 8354) || (n > 8354 && n < 8387) || (n > 8387 && n < 8409) || n = 8410 || (n > 8411 && n < 8420) || (n > 8420 && n < 8442) || n = 8443 || (n > 8444 && n < 8453) || (n > 8453 && n < 8664) || (n > 8664 && n < 8698) || (n > 8698 && n < 8751))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8154)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 8156)
	; 12 bytes
	tya
	ldy #247
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8186)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 8189)
	; 12 bytes
	tya
	ldy #248
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8193)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 8222)
	; 12 bytes
	tya
	ldy #249
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8255)
	; 12 bytes
	tya
	ldy #250
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8288)
	; 12 bytes
	tya
	ldy #251
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8321)
	; 12 bytes
	tya
	ldy #252
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8354)
	; 12 bytes
	tya
	ldy #253
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8387)
	; 12 bytes
	tya
	ldy #254
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8409)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	brk
	iny
	Jne *-6
	tay
.elseif(n = 8411 || n = 8444)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8420)
	; 12 bytes
	tya
	ldy #255
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8442)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk
	dey
	Jne *-6
	tay
.elseif(n = 8453)
	; 12 bytes
	tya
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8664)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	sta @zptemp
	brk
	iny
	Jne *-7
	tay
.elseif(n = 8698)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	sta @zptemp
	brk
	dey
	Jne *-7
	tay
.endif
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8919) || (n > 8919 && n < 8935) || (n > 8935 && n < 8954) || (n > 8954 && n < 9174) || (n > 9174 && n < 9210) || (n > 9210 && n < 9684) || (n > 9684 && n < 9722) || (n > 9722 && n < 10001))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8919)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	brk
	iny
	Jne *-7
	tay
.elseif(n = 8935)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
.elseif(n = 8954)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 6
	brk
	dey
	Jne *-7
	tay
.elseif(n = 9174)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc @zptemp
	brk
	iny
	Jne *-7
	tay
.elseif(n = 9210)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc @zptemp
	brk
	dey
	Jne *-7
	tay
.elseif(n = 9684)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
.elseif(n = 9722)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 11214) || (n > 11214 && n < 11258) || (n > 11258 && n < 20001))
	; 13 bytes
	php
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11214)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 11258)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
.endif
.endif

.endif
	rest
.endmacro
