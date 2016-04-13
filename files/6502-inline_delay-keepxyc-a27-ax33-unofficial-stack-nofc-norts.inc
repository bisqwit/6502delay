; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, S, Z+N, D, and V
; Will preserve X, Y, C, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 16
; - 5 bytes or less when n <= 20
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 28
; - 8 bytes or less when n <= 310
; - 9 bytes or less when n <= 1305
; -10 bytes or less when n <= 1981
; -11 bytes or less when n <= 2533
; -12 bytes or less when n <= 3125
; -13 bytes or less when n <= 20000
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
; - When 2001 <= n <= 3000: min=8, avg=9.8, max=12
; - When 3001 <= n <= 5000: min=9, avg=11.7, max=13
; - When 5001 <= n <= 7500: min=11, avg=12.9, max=13
; - When 7501 <= n <= 10000: min=11, avg=13.0, max=13
; - When 10001 <= n <= 20000: min=11, avg=13.0, max=13
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
.elseif(n = 25 || n = 35)
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
.elseif(n = 34)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif((n > 35 && n < 46) || (n > 46 && n < 53) || (n > 53 && n < 79))
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
.endif
.else
; 79 to 156
.if((n > 78 && n < 128) || (n > 128 && n < 157))
	; 7 bytes
	php
	lda #n-36
	jsr delay_a_27_clocks
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
.elseif(n = 292 || n = 304 || n = 309)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 293 || n = 297 || n = 302 || n = 307 || n = 312)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 294 || n = 298)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 305 || n = 310)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 308)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
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
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 339 || n = 344 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 379 || n = 384 || n = 387 || n = 389 || n = 394 || n = 406 || n = 414 || n = 419 || n = 424 || n = 429 || n = 434 || n = 444 || n = 449 || n = 454 || n = 464)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385 || n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316 || n = 317 || n = 321 || n = 322 || n = 326 || n = 331 || n = 332 || n = 336 || n = 337 || n = 342 || n = 347 || n = 351 || n = 352 || n = 357 || n = 361 || n = 366 || n = 367 || n = 371 || n = 372 || n = 377 || n = 382 || n = 392 || n = 396 || n = 407 || n = 412 || n = 417 || n = 422 || n = 426 || n = 427 || n = 431 || n = 436 || n = 437 || n = 441 || n = 442 || n = 447 || n = 451 || n = 452 || n = 456 || n = 457 || n = 462 || n = 466)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 318)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	iny
	Jpl *-3
	tay
.elseif(n = 323 || n = 328 || n = 338 || n = 343 || n = 353 || n = 358 || n = 368 || n = 378 || n = 386 || n = 388 || n = 393 || n = 398 || n = 403 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 458 || n = 463 || n = 468)
	; 7..8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 333 || n = 373 || n = 381)
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
.elseif(n = 356 || n = 401)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 363)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	iny
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
.elseif(n = 399)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jpl *-3
	tay
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
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 413 || n = 421 || n = 461)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 416)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 459)
	; 7 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
	tay
.endif
.else
; 469 to 625
.if(n = 469 || n = 479 || n = 494 || n = 499 || n = 504 || n = 511 || n = 514 || n = 519 || n = 524 || n = 529 || n = 539 || n = 549 || n = 554 || n = 559 || n = 569 || n = 574 || n = 576 || n = 584 || n = 589 || n = 594 || n = 599 || n = 604 || n = 609 || n = 619 || n = 624)
	; 8..9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 472 || n = 477 || n = 482 || n = 486 || n = 487 || n = 492 || n = 497 || n = 506 || n = 512 || n = 517 || n = 522 || n = 526 || n = 527 || n = 531 || n = 532 || n = 542 || n = 546 || n = 547 || n = 552 || n = 557 || n = 562 || n = 566 || n = 567 || n = 571 || n = 577 || n = 582 || n = 587 || n = 591 || n = 592 || n = 596 || n = 597 || n = 601 || n = 602 || n = 612 || n = 616 || n = 617 || n = 622)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 473 || n = 478 || n = 483 || n = 496 || n = 498 || n = 503 || n = 508 || n = 513 || n = 518 || n = 528 || n = 538 || n = 543 || n = 553 || n = 563 || n = 568 || n = 578 || n = 583 || n = 588 || n = 598 || n = 603 || n = 608 || n = 618 || n = 623)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 476)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 481 || n = 488 || n = 509 || n = 537 || n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 484)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
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
.elseif(n = 491)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 493)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jpl *-2
	tay
.elseif(n = 501 || n = 581 || n = 613)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 533 || n = 541 || n = 573)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 534)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 536)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 548)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 561)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	dey
	dey
	Jpl *-3
	tay
.elseif(n = 564)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
.elseif(n = 606)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 611)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
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
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657 || n = 662 || n = 666 || n = 667 || n = 672 || n = 676 || n = 682 || n = 686 || n = 687 || n = 697 || n = 702 || n = 706 || n = 707 || n = 711 || n = 722 || n = 727 || n = 732 || n = 737 || n = 742 || n = 746 || n = 751 || n = 757 || n = 762 || n = 766 || n = 767 || n = 771 || n = 772 || n = 776 || n = 777)
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
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 646 || n = 654 || n = 659 || n = 664 || n = 669 || n = 674 || n = 679 || n = 681 || n = 689 || n = 692 || n = 694 || n = 699 || n = 704 || n = 709 || n = 714 || n = 721 || n = 724 || n = 729 || n = 734 || n = 739 || n = 744 || n = 749 || n = 756 || n = 764 || n = 769 || n = 774 || n = 779)
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
.elseif(n = 633)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636 || n = 638 || n = 643 || n = 648 || n = 658 || n = 668 || n = 673 || n = 683 || n = 691 || n = 703 || n = 708 || n = 713 || n = 718 || n = 723 || n = 728 || n = 738 || n = 743 || n = 753 || n = 758 || n = 763 || n = 778)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 641)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 642 || n = 656 || n = 677 || n = 712 || n = 719 || n = 733 || n = 747 || n = 754 || n = 761 || n = 768)
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
.elseif(n = 653 || n = 661 || n = 773)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 671)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 678)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 693 || n = 701 || n = 741 || n = 781)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 696)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 716)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 717)
	; 7 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
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
.elseif(n = 748)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 759)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
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
.elseif((n .mod 5 = 3) && ((n > 782 && n < 799) || n = 808 || (n > 817 && n < 834) || n = 843 || n = 848 || (n > 857 && n < 869) || (n > 877 && n < 889) || n = 898 || (n > 912 && n < 929)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 4) && (n = 784 || n = 794 || n = 799 || (n > 808 && n < 820) || (n > 828 && n < 855) || (n > 868 && n < 890) || (n > 898 && n < 925) || n = 934))
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && ((n > 784 && n < 936)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 786 || n = 787 || n = 791 || n = 792 || n = 797 || n = 802 || n = 807 || n = 811 || n = 812 || n = 816 || n = 822 || n = 826 || n = 827 || n = 832 || n = 837 || n = 841 || n = 842 || n = 846 || n = 847 || n = 856 || n = 862 || n = 867 || n = 872 || n = 876 || n = 877 || n = 882 || n = 886 || n = 891 || n = 892 || n = 897 || n = 902 || n = 906 || n = 907 || n = 911 || n = 912 || n = 917 || n = 927 || n = 931 || n = 932 || n = 937)
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
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 813)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 821 || n = 853 || n = 861 || n = 893)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 857)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 864)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 871)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 916)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	iny
	Jne *-4
	tay
.elseif(n = 921)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 926)
	; 9 bytes
	tya
	ldy #(n-2)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 933)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 948 || n = 953 || n = 958 || n = 963 || n = 968 || n = 983 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1021 || n = 1023 || n = 1028 || n = 1033 || n = 1043 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1088)
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
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1091)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 941)
	; 7 bytes
	tya
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 942 || n = 947 || n = 951 || n = 952 || n = 956 || n = 961 || n = 967 || n = 972 || n = 977 || n = 982 || n = 987 || n = 991 || n = 996 || n = 997 || n = 1001 || n = 1002 || n = 1007 || n = 1012 || n = 1016 || n = 1017 || n = 1032 || n = 1036 || n = 1037 || n = 1042 || n = 1046 || n = 1047 || n = 1051 || n = 1052 || n = 1057 || n = 1066 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1081 || n = 1082 || n = 1086 || n = 1087 || n = 1091 || n = 1092)
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
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 969 || n = 974 || n = 979 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1019 || n = 1022 || n = 1024 || n = 1029 || n = 1039 || n = 1044 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1079 || n = 1084 || n = 1089)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 946)
	; 9 bytes
	php
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
	plp
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
.elseif(n = 973 || n = 1053 || n = 1061 || n = 1093)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 984)
	; 8 bytes
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 986)
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
.elseif(n = 1031)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 1038)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
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
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && (n = 1094 || n = 1109 || n = 1114 || (n > 1123 && n < 1135) || (n > 1143 && n < 1170) || (n > 1178 && n < 1205) || (n > 1213 && n < 1240) || n = 1249))
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1251)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 1096 || n = 1102 || n = 1106 || n = 1107 || n = 1112 || n = 1116 || n = 1117 || n = 1122 || n = 1126 || n = 1127 || n = 1131 || n = 1136 || n = 1137 || n = 1142 || n = 1147 || n = 1151 || n = 1152 || n = 1157 || n = 1161 || n = 1162 || n = 1171 || n = 1172 || n = 1176 || n = 1177 || n = 1182 || n = 1186 || n = 1187 || n = 1192 || n = 1196 || n = 1197 || n = 1201 || n = 1206 || n = 1207 || n = 1212 || n = 1217 || n = 1222 || n = 1226 || n = 1227 || n = 1231 || n = 1232 || n = 1242 || n = 1246 || n = 1247)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1096 && n < 1119) || (n > 1131 && n < 1154) || (n > 1166 && n < 1189) || (n > 1201 && n < 1224) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1114) || n = 1123 || (n > 1137 && n < 1149) || (n > 1157 && n < 1169) || n = 1178 || n = 1183 || (n > 1192 && n < 1209) || n = 1218 || (n > 1227 && n < 1249)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1099)
	; 7 bytes
	tya
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
	tay
.elseif(n = 1101)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
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
.elseif(n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	nop
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 1128)
	; 8 bytes
	tya
	ldy #250 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1133 || n = 1141 || n = 1173 || n = 1213 || n = 1221)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1156)
	; 9 bytes
	tya
	ldy #128 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
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
.elseif(n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
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
.elseif(n = 1252 || n = 1257 || n = 1262 || n = 1266 || n = 1267 || n = 1271 || n = 1276 || n = 1277 || n = 1281 || n = 1282 || n = 1287 || n = 1302 || n = 1306 || n = 1316 || n = 1323 || n = 1327)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 5) && (n = 1253 || n = 1261 || (n > 1300 && n < 1326)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1254)
	; 8 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1281)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 1256 || n = 1310 || n = 1319)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1284 || n = 1289 || n = 1295 || n = 1297 || n = 1304 || n = 1311 || n = 1318)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1263)
	; 8 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1288 || n = 1294 || n = 1320 || n = 1324)
	; 8..9 bytes
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
.elseif(n = 1291)
	; 7 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
.elseif(n = 1292 || n = 1298)
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
.elseif(n = 1299 || n = 1308 || n = 1322)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1303)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1305 || n = 1315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
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
.elseif(n = 1313)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
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
.if(n = 1329 || n = 1332 || n = 1339 || n = 1353 || n = 1360 || n = 1374 || n = 1388 || n = 1393 || n = 1395 || n = 1402)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1330 || n = 1344 || n = 1348 || n = 1351 || n = 1358 || n = 1359 || n = 1362 || n = 1372 || n = 1379 || n = 1383 || n = 1386 || n = 1399 || n = 1404)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1352 || n = 1366 || n = 1368 || n = 1376 || n = 1387 || n = 1392 || n = 1401 || n = 1406)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 5) && (n = 1333 || n = 1341 || (n > 1356 && n < 1382) || n = 1397))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1334)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	nop
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1336 && n < 1365) || n = 1382 || n = 1400))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1340 || n = 1354)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1343)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 1345)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1347 || n = 1369)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1350 || n = 1371)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1361 || n = 1385)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1367)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1375)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1378)
	; 9 bytes
	tya
	ldy #250 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
	tay
.elseif(n = 1380)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
.elseif(n = 1389)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1390)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jpl *-5
	tay
.elseif(n = 1394)
	; 9 bytes
	php
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1396)
	; 9 bytes
	delay_n (n-4)
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
.if(n = 1407 || n = 1416 || n = 1417 || n = 1423 || n = 1428 || n = 1430 || n = 1441 || n = 1451 || n = 1458 || n = 1465 || n = 1473 || n = 1479)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1408 || n = 1422 || n = 1427 || n = 1443 || n = 1448 || n = 1450 || n = 1464 || n = 1478 || n = 1480)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1418 || n = 1436 || n = 1463 || n = 1472 || n = 1481)
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
.elseif(n = 1411 || n = 1432 || n = 1462 || n = 1476)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && ((n > 1412 && n < 1454) || n = 1469 || n = 1477))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1414 || n = 1420 || n = 1435 || n = 1439 || n = 1442 || n = 1449 || n = 1456 || n = 1460 || n = 1470 || n = 1471 || n = 1474 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1415 || n = 1425 || n = 1455)
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
.elseif(n = 1434)
	; 10 bytes
	tya
	ldy #(n-4)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1438 || n = 1452 || n = 1466)
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
.elseif(n = 1446)
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
.elseif(n = 1467)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	tay
.endif
.else
; 1485 to 1562
.if((n .mod 8 = 5) && ((n > 1484 && n < 1510) || (n > 1524 && n < 1558)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1486 || n = 1507 || n = 1514 || n = 1519 || n = 1521 || n = 1528 || n = 1537 || n = 1546 || n = 1556)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1487 || n = 1491 || n = 1498 || n = 1502 || n = 1505 || n = 1511 || n = 1512 || n = 1527 || n = 1530 || n = 1540 || n = 1547 || n = 1554 || n = 1561)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488 || n = 1492 || n = 1504 || n = 1506 || n = 1513 || n = 1518 || n = 1532 || n = 1548)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && (n = 1490 || n = 1499 || (n > 1525 && n < 1563)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1494 || n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1495 || n = 1555)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1497)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1500)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1508)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1515)
	; 7 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
.elseif(n = 1516)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1520)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1522 || n = 1536)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1523)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1529)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1534)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1539)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1542)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1543)
	; 9 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1551)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
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
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1564)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && (n = 1565 || (n > 1580 && n < 1606) || n = 1621 || n = 1637))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1637)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1567 || n = 1578 || n = 1600 || n = 1611)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1568 || n = 1572 || n = 1575 || n = 1582 || n = 1583 || n = 1586 || n = 1596 || n = 1599 || n = 1603 || n = 1610 || n = 1623 || n = 1624 || n = 1638 || n = 1639)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569 || n = 1576 || n = 1590 || n = 1592 || n = 1604 || n = 1616 || n = 1630)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1570 || n = 1584 || n = 1591 || n = 1593 || n = 1609 || n = 1612 || n = 1617 || n = 1619 || n = 1626 || n = 1631 || n = 1633 || n = 1640)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1571 || n = 1598 || n = 1607 || n = 1625)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1574)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1577)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
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
.elseif(n = 1585 || n = 1595)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1588)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	dey
	Jne *-5
	tay
.elseif(n = 1602)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1606)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1613)
	; 7 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1614)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
.elseif(n = 1618)
	; 9 bytes
	php
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	plp
.elseif(n = 1620 || n = 1634)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1628)
	; 10 bytes
	tya
	ldy #250 ;hides 'nop'
	nop
	nop
	dey
	dey
	Jne *-5
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
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.else
; 1641 to 1718
.if(n = 1641)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642 || n = 1666 || n = 1673 || n = 1680 || n = 1684 || n = 1687 || n = 1691 || n = 1694 || n = 1698 || n = 1708)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1644 || n = 1656 || n = 1660 || n = 1662 || n = 1667 || n = 1672 || n = 1674 || n = 1681 || n = 1702 || n = 1704 || n = 1712 || n = 1716)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1645 || n = 1651 || n = 1654 || n = 1668 || n = 1675 || n = 1682 || n = 1689 || n = 1696 || n = 1703 || n = 1710)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1646)
	; 9 bytes
	php
	sec
	lda #(n-8)/7 ;hides 'nop'
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
.elseif(n = 1648)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1649)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && (n = 1652 || (n > 1669 && n < 1698) || n = 1715))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && ((n > 1652 && n < 1678) || (n > 1692 && n < 1718)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1655 || n = 1665 || n = 1695 || n = 1705)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1658 || n = 1700 || n = 1707 || n = 1714)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1659)
	; 8 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1663)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 1676 || n = 1690)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1683)
	; 9 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
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
.elseif(n = 1711)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 1718)
	; 9 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1736 || n = 1740 || n = 1750 || n = 1758 || n = 1766 || n = 1767 || n = 1771 || n = 1774 || n = 1782 || n = 1791 || n = 1792)
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
.elseif(n = 1723 || n = 1728 || n = 1730 || n = 1737 || n = 1742 || n = 1768 || n = 1779 || n = 1784 || n = 1786 || n = 1788)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1724 || n = 1751 || n = 1760 || n = 1778 || n = 1787 || n = 1796)
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
.elseif(n = 1731 || n = 1738 || n = 1743 || n = 1752 || n = 1759 || n = 1761 || n = 1777 || n = 1780)
	; 8..9 bytes
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
.elseif(n = 1746)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1747 || n = 1770)
	; 9..10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1753)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1756)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1763)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1764)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1772)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1785)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
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
.elseif(n = 1798 || n = 1801 || n = 1807 || n = 1818 || n = 1833 || n = 1834)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1799 || n = 1819 || n = 1826)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1816 || n = 1824)
	; 9 bytes
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
.elseif((n .mod 8 = 5) && ((n > 1804 && n < 1830)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1809 || n = 1820 || n = 1831)
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
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1814 || n = 1823 || n = 1832)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1815 || n = 1825 || n = 1835)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1817)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1822)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
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
	nop
	nop
	Jne *-3
	tax
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif((n .mod 8 = 5) && ((n > 1836 && n < 1870)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1838)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1839)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1840 || n = 1846 || n = 1851 || n = 1856 || n = 1860 || n = 1862 || n = 1872)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 5) && ((n > 1840 && n < 1869)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1842 || n = 1864)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1843)
	; 8 bytes
	tya
	ldy #184 ;hides 'clv'
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1844)
	; 10 bytes
	php
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 1847 || n = 1858 || n = 1863 || n = 1866 || n = 1870 || n = 1871)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1848)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1849 || n = 1852 || n = 1854 || n = 1857 || n = 1873)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1855 || n = 1875)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
.elseif(n = 1874)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
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
.if(n = 1876)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	nop
	nop
	iny
	Jne *-5
	tay
.elseif((n .mod 8 = 5) && ((n > 1876 && n < 1950)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1878 || n = 1880 || n = 1888 || n = 1892 || n = 1896 || n = 1900 || n = 1912 || n = 1914 || n = 1923 || n = 1928 || n = 1934 || n = 1936 || n = 1944 || n = 1948 || n = 1950)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1879)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 1881 || n = 1890 || n = 1899 || n = 1924 || n = 1926 || n = 1929 || n = 1953)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1882 || n = 1887 || n = 1891 || n = 1898 || n = 1903 || n = 1927 || n = 1932 || n = 1938 || n = 1939 || n = 1942 || n = 1943 || n = 1946 || n = 1951)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1883)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 1884)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1885 && n < 1941)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1889 || n = 1937)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1894)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif((n .mod 11 = 5) && ((n > 1896 && n < 1931) || n = 1952))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1902)
	; 8 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1905 || n = 1915 || n = 1935 || n = 1945)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1906)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
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
.elseif(n = 1911)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1916)
	; 10 bytes
	php
	sec
	lda #212 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 1920)
	; 8 bytes
	tya
	ldy #(n-12)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1921)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
.elseif(n = 1947)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
.endif
.else
; 1954 to 2031
.if(n = 1954)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 1955 || n = 1975 || n = 1995 || n = 2015 || n = 2025)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif((n .mod 2 = 0) && (n = 1956 || n = 1960 || n = 1964 || n = 1966 || (n > 1983 && n < 1993) || n = 2000 || (n > 2003 && n < 2009) || n = 2016 || n = 2024 || n = 2028))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 5) && (n = 1957 || (n > 1972 && n < 2030)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 1995) || n = 2012 || n = 2030))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 1978 || n = 1982 || n = 1991 || n = 1999 || n = 2002 || n = 2010 || n = 2011 || n = 2014 || n = 2023 || n = 2026 || n = 2031)
	; 9..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1961 || n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1962 || n = 1977 || n = 1980 || n = 1987 || n = 1993 || n = 1998 || n = 2001 || n = 2017 || n = 2022)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1963 || n = 1974 || n = 1996 || n = 2007 || n = 2018)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1965)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1968)
	; 9 bytes
	pha
	tya
	ldy #245
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1969)
	; 9 bytes
	pla
	tya
	ldy #245
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1970)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 1971)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 9
	dey
	Jpl *-5
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
.elseif(n = 1979)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1983)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
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
.elseif(n = 2020)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2027)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	nop
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2036 || n = 2044 || n = 2054 || n = 2056)
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
.elseif(n = 2034 || n = 2041 || n = 2043 || n = 2049 || n = 2052 || n = 2060)
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
.elseif(n = 2038)
	; 9 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
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
.elseif(n = 2042 || n = 2047 || n = 2050 || n = 2055 || n = 2059 || n = 2064 || n = 2068)
	; 9..10 bytes
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
.elseif(n = 2061)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	pha
	iny
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
.elseif(n = 2067)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2069)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2070)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.endif
.else
; 2071 to 2109
.if(n = 2071)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop $48,X ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2072)
	; 9 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2073)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2074 || n = 2087 || n = 2104 || n = 2107 || n = 2108)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 5) && ((n > 2074 && n < 2103)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2076 || n = 2092 || n = 2094)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2077 || n = 2086 || n = 2088 || n = 2095 || n = 2097)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2078)
	; 10 bytes
	php
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	plp
.elseif(n = 2079)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2080 || n = 2098)
	; 10 bytes
	delay_n (n-4)
	pla
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
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	pha
	iny
	Jne *-4
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
.elseif(n = 2090)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
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
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2103)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	delay_n 6
	iny
	Jne *-5
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
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2113 || n = 2122 || n = 2124 || n = 2131 || n = 2140 || n = 2142 || n = 2154 || n = 2160 || n = 2167 || n = 2169 || n = 2178 || n = 2187)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2175)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2121 || n = 2123 || n = 2130 || n = 2144 || n = 2148 || n = 2159 || n = 2164 || n = 2166 || n = 2168 || n = 2170 || n = 2184 || n = 2186)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
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
.elseif(n = 2116)
	; 10 bytes
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
.elseif(n = 2118)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
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
.elseif(n = 2126 || n = 2171 || n = 2173)
	; 10..11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2127)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif((n .mod 11 = 5) && ((n > 2127 && n < 2173)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2132)
	; 10 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	plp
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
.elseif(n = 2136)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 2137)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2143 || n = 2149 || n = 2152 || n = 2157 || n = 2158 || n = 2162 || n = 2176 || n = 2179)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2163)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2180)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2181)
	; 9 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2183)
	; 8 bytes
	tya
	ldy #(n-3)/10 ;hides 'nop'
	pha
	dey
	Jne *-3
	tay
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188 || n = 2193 || n = 2198 || n = 2202 || n = 2222)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2189 || n = 2213 || n = 2225)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2190)
	; 8 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 2191 || n = 2196 || n = 2206 || n = 2218 || n = 2221)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 5) && ((n > 2191 && n < 2220)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2194)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2195 || n = 2205 || n = 2215)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2197)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2199)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2200)
	; 9 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	iny
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
.elseif(n = 2204)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2207 || n = 2209 || n = 2212 || n = 2220 || n = 2223)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2208)
	; 8 bytes
	tya
	ldy #(n-12)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 2211)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2214)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2216)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2217)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2224)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	nop
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2226)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2238 || n = 2260)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 2227 && n < 2265)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2229 || n = 2230 || n = 2248 || n = 2251)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2231 || n = 2259)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2232 || n = 2240 || n = 2241 || n = 2250 || n = 2257)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2233)
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
.elseif(n = 2235)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2236)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
	tax
.elseif(n = 2239 || n = 2256)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2242)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2243)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2244)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2245)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2247)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2249 || n = 2261)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2252)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2253)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2254)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
	tay
.elseif(n = 2258)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2262)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2263)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2265)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2274 || n = 2283 || n = 2284 || n = 2292 || n = 2302)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2268 || n = 2269 || n = 2277 || n = 2279 || n = 2286)
	; 10 bytes
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
.elseif(n = 2271)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif(n = 2272)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2272 && n < 2301)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2275 || n = 2295)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2276 || n = 2278 || n = 2288 || n = 2294 || n = 2296)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2280)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	tay
.elseif(n = 2281)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
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
.elseif(n = 2289)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2290)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2293 || n = 2304)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2298)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2299)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2303)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.endif
.else
; 2305 to 2343
.if(n = 2305 || n = 2315 || n = 2325)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2306 || n = 2312 || n = 2318 || n = 2322 || n = 2340)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2307)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	tay
.elseif(n = 2308)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	dey
	Jne *-4
	tay
.elseif(n = 2309)
	; 9 bytes
	tya
	ldy #0
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2311 || n = 2317 || n = 2320 || n = 2328 || n = 2336 || n = 2339)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2313 || n = 2341)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2314)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2316)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2319 || n = 2332)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
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
	; 8 bytes
	tya
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 2324)
	; 9 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2326 || n = 2337)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2327)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
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
	nop
	nop
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2331)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2334)
	; 9 bytes
	tya
	ldy #(n-2)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2335)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop zp,x'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2338)
	; 10 bytes
	pha
	tya
	ldy #233
	nop
	pha
	dey
	Jne *-3
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
	; 8 bytes
	tya
	ldy #(n-3)/10 ;hides 'nop'
	pha
	dey
	Jne *-3
	tay
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2418)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2346)
	; 9 bytes
	tya
	ldy #(n-14)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2347 || n = 2349 || n = 2352 || n = 2363 || n = 2374 || n = 2389 || n = 2409 || n = 2421)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2348 || n = 2359 || n = 2370 || n = 2392 || n = 2403 || n = 2414)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2350 || n = 2361 || n = 2367 || n = 2372 || n = 2377 || n = 2383 || n = 2387 || n = 2394 || n = 2407 || n = 2416 || n = 2419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351 || n = 2362 || n = 2368 || n = 2373 || n = 2378 || n = 2382 || n = 2388 || n = 2396 || n = 2398 || n = 2404 || n = 2406 || n = 2408 || n = 2418)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2354)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 10 = 5) && ((n > 2354 && n < 2396) || n = 2415))
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2356)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2358 || n = 2384 || n = 2397 || n = 2410)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2360 || n = 2390 || n = 2420)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2364 || n = 2376)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2366)
	; 11 bytes
	php
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 2371 || n = 2399)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2379)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2380)
	; 11 bytes
	tya
	ldy #124
	delay_n 14
	dey
	Jpl *-5
	tay
.elseif(n = 2386 || n = 2400 || n = 2412)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2391)
	; 9 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
	tay
.elseif(n = 2401)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2402)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2411)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	nop
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2413)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	pha
	iny
	Jne *-4
	tay
.endif
.else
; 2422 to 2500
.if(n = 2422)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2423)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2424)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif((n .mod 10 = 5) && ((n > 2424 && n < 2456) || (n > 2474 && n < 2496)))
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2426 || n = 2430 || n = 2431 || n = 2437 || n = 2438 || n = 2457 || n = 2460 || n = 2467 || n = 2471 || n = 2474 || n = 2479 || n = 2482 || n = 2493 || n = 2500)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428 || n = 2432 || n = 2442 || n = 2444 || n = 2448 || n = 2452 || n = 2454 || n = 2456 || n = 2461 || n = 2466 || n = 2468 || n = 2472 || n = 2476 || n = 2478 || n = 2486 || n = 2492 || n = 2494 || n = 2498)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 5) && ((n > 2428 && n < 2490)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2433 || n = 2440 || n = 2443 || n = 2459 || n = 2473 || n = 2481 || n = 2484 || n = 2487 || n = 2499)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2434 || n = 2470)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif((n .mod 11 = 5) && ((n > 2435 && n < 2492)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2446 || n = 2496)
	; 10..11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2449)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2450)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2451)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2462 || n = 2488)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2463)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2464)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2483)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2490)
	; 10 bytes
	tya
	ldy #(n-4)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2503)
	; 8 bytes
	tya
	ldy #(n-3)/10 ;hides 'nop'
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2504 || n = 2512 || n = 2517 || n = 2526 || n = 2527 || n = 2534 || n = 2551 || n = 2563 || n = 2575)
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
.elseif(n = 2506 || n = 2508 || n = 2516 || n = 2518 || n = 2528 || n = 2536 || n = 2538 || n = 2552 || n = 2558 || n = 2560 || n = 2562 || n = 2564 || n = 2571 || n = 2576)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2507 || n = 2509 || n = 2519 || n = 2529 || n = 2541 || n = 2548 || n = 2550 || n = 2569 || n = 2572)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2520 || n = 2532 || n = 2556)
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
.elseif(n = 2522)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 2523)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
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
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	nop
	iny
	Jne *-4
	tay
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
.elseif(n = 2547)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop zp,x'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2555)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2559)
	; 9 bytes
	tya
	ldy #(n-15)/12 ;hides 'nop zp,x'
	iny ;first loop only
	pha
	dey
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
	ldy #(n-3)/11 ;hides 'nop'
	nop
	nop
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2591 || n = 2593 || n = 2604)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2581)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2582)
	; 10 bytes
	pha
	tya
	ldy #234
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2583)
	; 10 bytes
	pla
	tya
	ldy #234
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2584)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2585 || n = 2597 || n = 2609)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2586 || n = 2617)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2587 || n = 2600 || n = 2611 || n = 2614)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2588)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2589 || n = 2594 || n = 2602 || n = 2607 || n = 2613 || n = 2616)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2590 || n = 2601 || n = 2612)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2592 || n = 2605)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2595)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2596)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2598)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
.elseif(n = 2599)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2603)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
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
.elseif(n = 2610)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2615)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2631)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2619)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2620 || n = 2641)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2621 || n = 2633 || n = 2645)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2622 || n = 2624 || n = 2626 || n = 2648)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2623 || n = 2634 || n = 2656)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2625 || n = 2635 || n = 2636 || n = 2639 || n = 2647 || n = 2653)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2627 || n = 2638 || n = 2646 || n = 2649)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2630)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2632)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
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
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2644)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2650)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2654)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2655)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	nop
	nop
	iny
	Jne *-5
	tay
.endif
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
.elseif(n = 2658 || n = 2659 || n = 2666 || n = 2671 || n = 2680 || n = 2683 || n = 2691 || n = 2695 || n = 2702 || n = 2713 || n = 2719 || n = 2723 || n = 2724 || n = 2727)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675 || n = 2690 || n = 2720)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2661 || n = 2673 || n = 2682 || n = 2685 || n = 2697 || n = 2704 || n = 2715 || n = 2726)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2662 || n = 2674 || n = 2710)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2663 || n = 2699 || n = 2706 || n = 2728)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2664 || n = 2694)
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2668)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2670 || n = 2696 || n = 2709)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2672 || n = 2684 || n = 2692 || n = 2701 || n = 2703 || n = 2708 || n = 2714 || n = 2718 || n = 2725 || n = 2732 || n = 2734)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2676)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2677)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2686)
	; 9 bytes
	tya
	ldy #(n-2)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2687)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop zp,x'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2688)
	; 10 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #(n-14)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2712)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2716)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2730)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2731)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	nop
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735 || n = 2749)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2736 || n = 2740 || n = 2747 || n = 2756 || n = 2769)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2737 || n = 2745 || n = 2770)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2738 || n = 2760)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2739)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 2741 || n = 2753 || n = 2765)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2742)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2743 || n = 2751 || n = 2757 || n = 2767 || n = 2768)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2744 || n = 2755 || n = 2766)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2746)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2748 || n = 2761)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2750)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2752)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2754)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2758)
	; 10 bytes
	tya
	ldy #(n-2)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2759)
	; 10 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop zp,x'
	nop
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2762)
	; 11 bytes
	pha
	pla
	delay_n (n-7)
.elseif(n = 2763)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2764)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2771)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 2772)
	; 10 bytes
	tya
	ldy #(n-16)/13 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2800)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2775)
	; 9 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2776)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2777 || n = 2789 || n = 2801)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2778)
	; 10 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
	tax
.elseif(n = 2779 || n = 2790 || n = 2812)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2780 || n = 2795)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2781 || n = 2793 || n = 2807)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2782 || n = 2794)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2783)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2784)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2785)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2786)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2787)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2788 || n = 2799 || n = 2810)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2791 || n = 2805)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2792 || n = 2802 || n = 2804 || n = 2806)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2796)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2797)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2798)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
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
.elseif(n = 2808)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2809)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2811)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	tay
.endif
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
.elseif(n = 2814 || n = 2817 || n = 2829 || n = 2841)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2815 || n = 2822 || n = 2827 || n = 2839 || n = 2846 || n = 2851)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2836 || n = 2838 || n = 2848 || n = 2850)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2818 || n = 2830 || n = 2842)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2819)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2820)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2821)
	; 9 bytes
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2823)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2824)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2826)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2831 || n = 2843)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2832)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2833)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2834)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 2835)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2840)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2844)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2845)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
.elseif(n = 2847)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2865 || n = 2878)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2853 || n = 2871 || n = 2877 || n = 2882 || n = 2884)
	; 10..12 bytes
	pla
	delay_n (n-4)
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
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2856)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2857)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2858)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2859)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	nop
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2860 || n = 2863 || n = 2872 || n = 2887)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2861 || n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2862)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2864 || n = 2876 || n = 2883 || n = 2888)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2867)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2868)
	; 11 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 2875 || n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2879)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2880)
	; 11 bytes
	php
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 2881 || n = 2886)
	; 11..12 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2904 || n = 2956)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2892 || n = 2916 || n = 2928 || n = 2940 || n = 2952 || n = 2955)
	; 11..12 bytes
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
.elseif(n = 2894 || n = 2898 || n = 2907 || n = 2912 || n = 2918 || n = 2920 || n = 2922 || n = 2924 || n = 2934 || n = 2936 || n = 2944 || n = 2946 || n = 2948)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2895 || n = 2963)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2896 || n = 2964 || n = 2966)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 12 = 5) && ((n > 2896 && n < 2958)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2899 || n = 2905 || n = 2906 || n = 2911 || n = 2923 || n = 2935 || n = 2947 || n = 2954 || n = 2958 || n = 2967)
	; 10..12 bytes
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
.elseif(n = 2903 || n = 2917 || n = 2959)
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
.elseif(n = 2919)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2927 || n = 2939 || n = 2951)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2929)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2930)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2931)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop zp,x'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2932)
	; 10 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2941)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2942)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2943)
	; 9 bytes
	tya
	ldy #(n-15)/12 ;hides 'nop zp,x'
	iny ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2953)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2965)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 2968)
	; 12 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	nop
	delay_n 9
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if((n .mod 12 = 5) && ((n > 2968 && n < 3006)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2970)
	; 10 bytes
	tya
	ldy #(n-2)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2971 || n = 2983 || n = 2989 || n = 2995 || n = 3002 || n = 3007)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2978 || n = 2984 || n = 2996)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2973 || n = 2987 || n = 3001)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2974 || n = 2986 || n = 2998)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2975 || n = 2990)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2976 || n = 3000)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2977)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2979)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2980)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 2982)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2985 || n = 2994 || n = 2997)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 2988)
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 6
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2991)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2992)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2999)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3003)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 3004)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	pha
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
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3021 || n = 3034)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3009 || n = 3023 || n = 3033)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3010 || n = 3046)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3011)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 3014)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3015 || n = 3043)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3016)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3017 || n = 3029 || n = 3041)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3018 || n = 3022 || n = 3030 || n = 3032 || n = 3038 || n = 3044)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3019)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-4
	tay
.elseif(n = 3020 || n = 3035)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3025 || n = 3037)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3026)
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
.elseif(n = 3028)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3031 || n = 3042)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3036)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3039)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3040)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3045)
	; 9 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.endif
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
.elseif(n = 3049 || n = 3052 || n = 3054 || n = 3059 || n = 3069 || n = 3081)
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
.elseif(n = 3051 || n = 3056 || n = 3068 || n = 3074 || n = 3080)
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
.elseif(n = 3055)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3057 || n = 3071 || n = 3085)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3058)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3061)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3062)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3063)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3064)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3066 || n = 3067 || n = 3079)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3070)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3072)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3075)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3076)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3078)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3082)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3083)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3084)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 3087 || n = 3088 || n = 3097 || n = 3100 || n = 3101 || n = 3111 || n = 3114 || n = 3115 || n = 3123)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3089 || n = 3096 || n = 3122)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3090 || n = 3103)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3091 || n = 3092 || n = 3106 || n = 3120)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3093 || n = 3109)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3094)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3095 || n = 3110)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3098)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3099 || n = 3113)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3102 || n = 3121)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3104 || n = 3118)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3105)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3107)
	; 11 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3108)
	; 11 bytes
	tya
	ldy #194 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3116)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3117)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3119)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3124)
	; 12 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	tay
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
.if(n = 3126)
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
.elseif(n = 3127 || n = 3141 || n = 3155)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3128 || n = 3130 || n = 3134 || n = 3136 || n = 3144 || n = 3154 || n = 3158)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3129 || n = 3139 || n = 3142 || n = 3143 || n = 3152 || n = 3153 || n = 3156 || n = 3161)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3131)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3132 || n = 3146 || n = 3160)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3133 || n = 3150)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3135 || n = 3145)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3137)
	; 11 bytes
	php
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3138 || n = 3151 || n = 3164)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3140)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3147)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3148)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3149 || n = 3163)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3157)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3159)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3162)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.endif
.else
; 3165 to 3203
.if(n = 3165 || n = 3180 || n = 3186 || n = 3193)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3166 || n = 3172 || n = 3179 || n = 3192 || n = 3199 || n = 3202)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3167 || n = 3184 || n = 3201)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3168)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3169 || n = 3183 || n = 3197)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3170 || n = 3185 || n = 3200)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3171)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3173 || n = 3189)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3174)
	; 10 bytes
	tya
	ldy #(n-2)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3175)
	; 10 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop zp,x'
	nop
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3176)
	; 11 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3177 || n = 3190)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3178)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3181)
	; 10 bytes
	tya
	ldy #(n-1)/15 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3182)
	; 10 bytes
	tya
	ldy #(n-2)/15 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
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
.elseif(n = 3188)
	; 10 bytes
	tya
	ldy #(n-16)/13 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3191)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3194)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3195)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3196)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3198)
	; 10 bytes
	tya
	ldy #(n-18)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
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
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3207 || n = 3220 || n = 3222 || n = 3231 || n = 3234)
	; 10..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3205 || n = 3221)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3206 || n = 3212 || n = 3214 || n = 3219 || n = 3226 || n = 3233 || n = 3238)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3208 || n = 3224)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3209)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3210)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3211 || n = 3225 || n = 3239)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3213 || n = 3232 || n = 3241)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3215 || n = 3230)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3216 || n = 3229 || n = 3242)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3217)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3218 || n = 3235)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3223)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3227)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3228)
	; 11 bytes
	php
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	plp
.elseif(n = 3236)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 3240)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	nop
	iny
	Jne *-6
	tay
.endif
.else
; 3243 to 3281
.if(n = 3243)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3244 || n = 3247 || n = 3262 || n = 3270 || n = 3277)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3245 || n = 3260 || n = 3275)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3246 || n = 3249 || n = 3257 || n = 3259 || n = 3264 || n = 3272)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3248 || n = 3254 || n = 3256 || n = 3271 || n = 3276 || n = 3278)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3250 || n = 3261)
	; 11..12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3252 || n = 3269)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3253)
	; 9 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3255 || n = 3268)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3258)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3263)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3265)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3266)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3267 || n = 3281)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3273)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3274)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3280)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	nop
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3298 || n = 3308 || n = 3310 || n = 3312)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3283)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3284)
	; 11 bytes
	php
	sec
	lda #(n-8)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3285 || n = 3301 || n = 3317)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3286 || n = 3316)
	; 11..12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3287)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3288)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3289)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3290 || n = 3305)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3291 || n = 3296 || n = 3297)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3292 || n = 3311)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3293)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	pha
	iny
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
.elseif(n = 3295 || n = 3309)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3299)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
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
.elseif(n = 3302)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	nop
	iny
	Jne *-6
	tay
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
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3306)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3313)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3315)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3318)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3319)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.endif
.else
; 3321 to 3359
.if(n = 3321)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3322 || n = 3325 || n = 3334 || n = 3339 || n = 3352 || n = 3353)
	; 11..12 bytes
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
.elseif(n = 3324 || n = 3327 || n = 3341 || n = 3355)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3326 || n = 3336 || n = 3338 || n = 3340 || n = 3357)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3328 || n = 3342 || n = 3356)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 3330)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3331)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3332)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3333)
	; 10 bytes
	tya
	ldy #0
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3335 || n = 3350)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3344)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3345 || n = 3359)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n > 3345 && n < 3349))
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
.elseif(n = 3349)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3354)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3358)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if((n > 3359 && n < 3365) || n = 3366 || n = 3378)
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
.elseif(n = 3365 || n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3367 || n = 3382 || n = 3390)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
	tya
	ldy #58 ;hides 'nop'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3370)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3371 || n = 3388)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3372 || n = 3374 || n = 3376 || n = 3383 || n = 3391 || n = 3396)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3373 || n = 3375 || n = 3384 || n = 3392)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3377)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3380)
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
.elseif(n = 3385)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
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
.elseif(n = 3389)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3394)
	; 11 bytes
	php
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3398)
	; 11 bytes
	pha
	tya
	ldy #226
	delay_n 10
	dey
	Jne *-4
	tay
.endif
.else
; 3399 to 3437
.if(n = 3399)
	; 11 bytes
	pla
	tya
	ldy #226
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3400)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3401 || n = 3408 || n = 3424 || n = 3428 || n = 3432 || n = 3436)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3405)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	pha
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
.elseif(n = 3409 || n = 3412 || n = 3423 || n = 3427 || n = 3430 || n = 3437)
	; 11..12 bytes
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
.elseif(n = 3411)
	; 10 bytes
	tya
	ldy #(n-19)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
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
.elseif(n = 3414 || n = 3415)
	; 11 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 3417)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3418)
	; 10 bytes
	tya
	ldy #(n-2)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3419)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	nop
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3420)
	; 11 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3422)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
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
.elseif(n = 3433)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	pha
	dey
	Jne *-5
	tay
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
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3452 || n = 3458 || n = 3462 || n = 3466 || n = 3471 || n = 3476)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3439 || n = 3456 || n = 3473)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3440 || n = 3455 || n = 3470)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3441 || n = 3442 || n = 3447 || n = 3451 || n = 3457 || n = 3465 || n = 3472)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3443)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3444)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3445 || n = 3461)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
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
.elseif(n = 3448)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3449 || n = 3463)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3450)
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
.elseif(n = 3453)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3454)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 pha
	.endrepeat
	Jne *-5
	tax
.elseif(n = 3459)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3460 || n = 3467 || n = 3474)
	; 11..12 bytes
	pla
	delay_n (n-4)
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
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3469)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3475)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
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
.elseif(n = 3478 || n = 3486 || n = 3488 || n = 3494 || n = 3506 || n = 3514)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479 || n = 3481 || n = 3495 || n = 3504 || n = 3507)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3480)
	; 11 bytes
	php
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3482 || n = 3498 || n = 3510)
	; 11..12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3483)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3484 || n = 3512)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3485 || n = 3500 || n = 3515)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3487 || n = 3492 || n = 3502)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3489)
	; 11 bytes
	tya
	ldy #90 ;hides 'nop'
	delay_n 14
	iny
	Jne *-6
	tay
.elseif(n = 3490)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3493 || n = 3509)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3496)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3497)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3499)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
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
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3508)
	; 11 bytes
	php
	sec
	lda #(n-8)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	plp
.elseif(n = 3511)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3513)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3518 || n = 3522 || n = 3536 || n = 3544 || n = 3548 || n = 3550)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3517 || n = 3521 || n = 3527 || n = 3532 || n = 3535 || n = 3549)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519 || n = 3533 || n = 3547)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3520 || n = 3539)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3523 || n = 3529 || n = 3534 || n = 3537)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3524)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3525)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3526 || n = 3554)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3528)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3530 || n = 3545)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3531)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3538 || n = 3552)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3540)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3541)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3542)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3543)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3546)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3551)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3553)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 3555 to 3593
.if(n = 3555)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3556 || n = 3584 || n = 3586)
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
.elseif(n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3558)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3559)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3560 || n = 3590)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3561 || n = 3575)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3562 || n = 3563 || n = 3571 || n = 3591)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3564 || n = 3570 || n = 3572 || n = 3578)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3565 || n = 3579 || n = 3593)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3566 || n = 3580)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3567)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3568 || n = 3582)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3569 || n = 3587)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3574)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3576)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	nop
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3577)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3581)
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
.elseif(n = 3585)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
.elseif(n = 3588)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	nop
	pha
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
.elseif(n = 3592)
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
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if(n = 3594 || n = 3600 || n = 3613 || n = 3624 || n = 3630 || n = 3639 || n = 3647 || n = 3654 || n = 3655)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3595)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	nop
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3596 || n = 3615 || n = 3634)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3597 || n = 3658)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3598 || n = 3607 || n = 3610 || n = 3611 || n = 3622 || n = 3628 || n = 3636 || n = 3640 || n = 3645 || n = 3652 || n = 3667 || n = 3670 || n = 3671)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599 || n = 3608 || n = 3612 || n = 3618 || n = 3627 || n = 3629 || n = 3638 || n = 3642 || n = 3644 || n = 3646 || n = 3666 || n = 3668)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3601 || n = 3602 || n = 3604 || n = 3616 || (n > 3630 && n < 3634) || n = 3649)
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
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif((n .mod 15 = 5) && ((n > 3604 && n < 3666)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3606)
	; 11 bytes
	tya
	ldy #(n-2)/17 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3609 || n = 3626 || n = 3643 || n = 3660)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3614)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 3617)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 3619)
	; 11 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif((n .mod 16 = 5) && ((n > 3620 && n < 3670)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3623 || n = 3641 || n = 3659)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3625)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 3648)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3651)
	; 11 bytes
	php
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3656)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3657)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	tax
.elseif(n = 3661)
	; 10 bytes
	tya
	ldy #(n-1)/15 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3662)
	; 10 bytes
	tya
	ldy #(n-2)/15 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3663)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3664)
	; 11 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	delay_n 6
	iny
	Jne *-6
	tay
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3691)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3673)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3674)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3675)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3676 || n = 3682 || n = 3696 || n = 3697 || n = 3700)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3677 || n = 3694)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3678)
	; 10 bytes
	tya
	ldy #(n-18)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3679 || n = 3684 || n = 3687 || n = 3689 || n = 3703)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3680 || n = 3695 || n = 3710)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3681 || n = 3686 || n = 3698 || n = 3702 || n = 3704)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3683)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3685 || n = 3701)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3688)
	; 12 bytes
	php
	clc
	lda #26 ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 3690 || n = 3706)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3692 || n = 3708)
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
.elseif(n = 3693)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
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
.elseif(n = 3705)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 3707)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3709)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.endif
.else
; 3711 to 3750
.if(n = 3711 || n = 3728 || n = 3745)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3712 || n = 3730 || n = 3735 || n = 3741 || n = 3742 || n = 3746)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3713 || n = 3731)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3714 || n = 3719 || n = 3721 || n = 3732 || n = 3737)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3716)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3717 || n = 3733 || n = 3749)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3718 || n = 3726 || n = 3734 || n = 3736 || n = 3750)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3720)
	; 12 bytes
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	plp
.elseif(n = 3722)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3723)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3724)
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
.elseif(n = 3725 || n = 3740)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3727)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3729 || n = 3748)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3738)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3739)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3743)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3744)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3747)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
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
.if(n = 3751 || n = 3759 || n = 3771 || n = 3774 || n = 3789)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3752)
	; 12 bytes
	php
	sec
	lda #(n-8)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 3753)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3754)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3755 || n = 3770 || n = 3785)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3756 || n = 3758 || n = 3766 || n = 3773 || n = 3782 || n = 3788)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3757 || n = 3760 || n = 3764 || n = 3772 || n = 3775 || n = 3780 || n = 3787)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3761)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3762)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3763)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3765 || n = 3781)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3767 || n = 3786)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3768)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3769)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 6
	iny
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
.elseif(n = 3777)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3778)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3779)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3783)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 6
	iny
	Jne *-6
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
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3802 || n = 3807 || n = 3817 || n = 3820 || n = 3826)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3792 || n = 3816)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3793 || n = 3808 || n = 3823)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3794 || n = 3810 || n = 3812)
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
.elseif(n = 3795)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3796 || n = 3813)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3797)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3798)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3799)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3800 || n = 3815)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3801)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
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
.elseif(n = 3804 || n = 3809)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3805 || n = 3824)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3806 || n = 3827)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 3811)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 3814)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3818)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3819)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop zp,x'
	nop
	delay_n 9
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
.elseif(n = 3822)
	; 12 bytes
	pha
	pla
	delay_n (n-7)
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
.elseif(n = 3828)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 6
	iny
	Jne *-6
	tay
.endif
.endif
.else
; 3829 to 3906
.if(n = 3829 || n = 3861 || n = 3877 || n = 3893)
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
.elseif(n = 3831 || n = 3832 || n = 3835 || n = 3841 || n = 3859 || n = 3866 || n = 3883 || n = 3902)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3833 || n = 3840 || n = 3842 || n = 3848 || n = 3850 || n = 3852 || n = 3863 || n = 3870 || n = 3880 || n = 3882 || n = 3884 || n = 3894 || n = 3896 || n = 3901 || n = 3903)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3834 || n = 3849 || n = 3851 || n = 3868 || n = 3871 || n = 3885 || n = 3895 || n = 3897 || n = 3904)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3836)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3837)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3838 || n = 3853)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3839 || n = 3857 || n = 3875)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3843 || n = 3862 || n = 3900)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3844)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3845)
	; 10 bytes
	tya
	ldy #0
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3846)
	; 11 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3847 || n = 3864 || n = 3881 || n = 3898)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3854 || n = 3856 || n = 3858 || (n > 3871 && n < 3875) || n = 3876 || (n > 3885 && n < 3890) || n = 3892)
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
.elseif(n = 3855 || n = 3899)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 3860)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
.elseif(n = 3865)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 3867)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 3869 || n = 3890)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 3878)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3879)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3891)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3905)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3906)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	pha
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3908)
	; 11 bytes
	php
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 3909 || n = 3925 || n = 3941)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3910 || n = 3916 || n = 3918 || n = 3920 || n = 3922 || n = 3926 || n = 3928 || n = 3935 || n = 3944)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911 || n = 3929)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3912 || n = 3917 || n = 3933 || n = 3942)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3913)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3914)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3915 || n = 3932)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3919 || n = 3938)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3921 || n = 3931 || n = 3934 || n = 3940 || n = 3943)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3923)
	; 10 bytes
	tya
	ldy #(n-19)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3924 || n = 3937 || n = 3939)
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
.elseif(n = 3927)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop'
	nop
	delay_n 9
	dey
	Jne *-6
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
.elseif(n = 3936)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 3945)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.endif
.else
; 3946 to 3984
.if(n = 3946)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3947)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3948 || n = 3959 || n = 3967 || n = 3968)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3949 || n = 3966 || n = 3983)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 3950 || n = 3952 || n = 3954 || n = 3960 || n = 3969 || n = 3974 || n = 3979 || n = 3984)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3951 || n = 3961 || n = 3970 || n = 3975 || n = 3977 || n = 3980)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3953)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 3955)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3956 || n = 3958 || n = 3972)
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
.elseif(n = 3957 || n = 3973)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3962)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3963)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 13
	iny
	Jne *-6
	tay
.elseif(n = 3964)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 3965)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 3971)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3976)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3978)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 3981)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3982)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985 || n = 3993 || n = 3999 || n = 4007 || n = 4018)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 3986 || n = 3992 || n = 4006)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3987)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3988)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3989 || n = 4005 || n = 4021)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3990 || n = 3991 || n = 3997 || n = 4002 || n = 4022 || n = 4023)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3994 || n = 3996)
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
.elseif(n = 3995 || n = 4014)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3998)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 4000 || n = 4017)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4001)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4003)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4004)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4008)
	; 12 bytes
	php
	sec
	lda #(n-8)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	plp
.elseif(n = 4009)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4010)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4011)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4012)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4013)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 4015)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 4016)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4019)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4020)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4040 || n = 4056)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4025 || n = 4045)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif((n > 4025 && n < 4029) || (n > 4041 && n < 4045) || (n > 4045 && n < 4050) || (n > 4059 && n < 4063))
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
.elseif(n = 4029)
	; 11 bytes
	tya
	ldy #(n-1)/19 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4030)
	; 11 bytes
	tya
	ldy #(n-2)/19 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4031)
	; 11 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop zp,x'
	nop
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4032)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 4033 || n = 4052)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4034 || n = 4051)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4035)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4036)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4037 || n = 4053)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4038 || n = 4039 || n = 4054 || n = 4057)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4041 || n = 4059)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4050)
	; 11 bytes
	tya
	ldy #(n-22)/19 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4055)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4058)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
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
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	.repeat 5
	 pha
	.endrepeat
	Jne *-6
	tax
.elseif(n = 4064 || n = 4078 || n = 4080 || n = 4096 || n = 4098)
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
.elseif(n = 4065)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4066 || n = 4074 || n = 4076 || n = 4088 || n = 4094)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4067)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
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
.elseif(n = 4070 || n = 4086 || n = 4087 || n = 4092 || n = 4093)
	; 12 bytes
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
.elseif(n = 4072)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
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
.elseif(n = 4077 || n = 4089 || n = 4095)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4079)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4081)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4082)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4083)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4084)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4099)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4100)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
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
.if(n = 4102 || n = 4119 || n = 4136)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4103 || n = 4104 || n = 4111 || n = 4121 || n = 4129 || n = 4130 || n = 4138)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4110 || n = 4112 || n = 4122 || n = 4131 || n = 4137 || n = 4139)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4106 || n = 4113 || n = 4123 || n = 4132)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4107)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
	tay
.elseif(n = 4108 || (n > 4113 && n < 4118) || n = 4124 || n = 4126 || n = 4133 || n = 4135)
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
.elseif(n = 4109 || n = 4128)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4118)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4120)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	tax
.elseif(n = 4125)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4127)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4134)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4140)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4142 || n = 4184 || n = 4205)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4143)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4144 || (n > 4157 && n < 4161) || (n > 4174 && n < 4179) || n = 4180 || n = 4186 || (n > 4191 && n < 4199) || n = 4200 || (n > 4208 && n < 4215) || n = 4216)
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
.elseif(n = 4145)
	; 11 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
	tay
.elseif(n = 4146 || n = 4154 || n = 4156 || n = 4164 || n = 4171 || n = 4173 || n = 4188 || n = 4190 || n = 4202 || n = 4207 || n = 4218)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4147 || n = 4166 || n = 4185)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4148)
	; 12 bytes
	pha
	tya
	ldy #230
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4149)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	pha
	dey
	Jne *-6
	tay
.elseif(n = 4150)
	; 11 bytes
	tya
	ldy #(n-2)/17 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4151)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	nop
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4152)
	; 12 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4153 || n = 4170 || n = 4187 || n = 4204)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4155 || n = 4167 || n = 4172 || n = 4189 || n = 4201 || n = 4206)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4157 || n = 4169 || n = 4174 || n = 4191 || n = 4208)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4161)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4162)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4163 || n = 4181 || n = 4199 || n = 4217)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4165)
	; 11 bytes
	php
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4168)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	delay_n 6
	dey
	Jne *-6
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
.elseif(n = 4182)
	; 12 bytes
	tya
	ldy #200 ;hides 'iny'
	rol $36,X
	ror $36,X
	dey
	dey
	Jne *-7
	tay
.elseif(n = 4183)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
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
	ldy #(n-3)/18 ;hides 'nop'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 4220 || n = 4222 || n = 4224 || n = 4241 || n = 4256 || n = 4258 || n = 4260 || n = 4264 || n = 4275 || n = 4283 || n = 4292)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4221 || n = 4238 || n = 4255 || n = 4272 || n = 4289)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4223 || n = 4242 || n = 4261 || n = 4280)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4225 || n = 4237 || n = 4240 || n = 4244 || n = 4252 || n = 4263 || n = 4274 || n = 4282 || n = 4291)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4226 || n = 4247 || n = 4268)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4227 || n = 4239 || n = 4246 || n = 4257 || n = 4259 || n = 4270 || n = 4276 || n = 4284 || n = 4293)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4228 || (n > 4229 && n < 4233) || n = 4234 || n = 4248 || n = 4262 || n = 4267 || (n > 4276 && n < 4280) || n = 4281 || n = 4290 || n = 4294 || n = 4296)
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
.elseif(n = 4229 || n = 4273 || n = 4295)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4233)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4235 || n = 4271)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4236)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4243)
	; 12 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop zp,x'
	nop
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4245 || n = 4265 || n = 4285)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4249)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 4250)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4251)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4253)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 4254)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4266)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 14
	Jne *-6
	tax
.elseif(n = 4269)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4286)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4287)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	tay
.elseif(n = 4288)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.endif
.else
; 4297 to 4375
.if(n = 4297)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4298)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 4299 || n = 4318 || n = 4337 || n = 4356 || n = 4375)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4300 || (n > 4311 && n < 4317) || n = 4319 || n = 4324 || n = 4330 || (n > 4331 && n < 4337) || (n > 4347 && n < 4352) || n = 4353 || n = 4354 || n = 4362 || (n > 4367 && n < 4373) || n = 4374)
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
.elseif(n = 4301 || n = 4308 || n = 4320 || n = 4339 || n = 4342 || n = 4358 || n = 4359)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4302 || n = 4309 || n = 4321 || n = 4326 || n = 4328 || n = 4346 || n = 4360 || n = 4364 || n = 4366)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4303 || n = 4311 || n = 4322 || n = 4327 || n = 4329 || n = 4341 || n = 4344 || n = 4347 || n = 4367)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4304)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4305 || n = 4345 || n = 4365)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4306 || n = 4323 || n = 4340)
	; 11 bytes
	tya
	ldy #(n-5)/17
	rol $36,X
	ror $36,X
	dey
	Jne *-5
	tay
.elseif(n = 4307 || n = 4325 || n = 4343 || n = 4361)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4310 || n = 4331 || n = 4352)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4317)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4338)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	nop
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 4355)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	nop
	delay_n 6
	iny
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
.elseif(n = 4363)
	; 11 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4373)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	rol $36,X
	ror $36,X
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
; 4376 to 4453
.if(n = 4376 || n = 4378 || n = 4380 || n = 4382 || n = 4400 || n = 4414 || n = 4416 || n = 4418 || n = 4425 || n = 4452)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4377)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4379 || n = 4397 || n = 4415 || n = 4433)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4381 || n = 4396 || n = 4399 || n = 4417 || n = 4424 || n = 4434 || n = 4435 || n = 4453)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4383 || n = 4427)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4384)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	tay
.elseif(n = 4385 || n = 4405 || n = 4445)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif((n > 4385 && n < 4392) || (n > 4405 && n < 4411) || n = 4412 || n = 4420 || n = 4421 || n = 4423 || (n > 4427 && n < 4432) || (n > 4437 && n < 4445) || (n > 4445 && n < 4449) || n = 4450)
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
.elseif(n = 4392)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 10
	dex
	dex
	Jne *-6
	tax
.elseif(n = 4393)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4394 || n = 4413 || n = 4432 || n = 4451)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4395)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop zp,x'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4398 || n = 4401 || n = 4419 || n = 4426 || n = 4437)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4402)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4403)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	.repeat 5
	 pha
	.endrepeat
	iny
	Jne *-7
	tay
.elseif(n = 4404)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	tay
.elseif(n = 4411)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	rol $36,X
	ror $36,X
	iny
	Jne *-6
	tay
.elseif(n = 4422)
	; 11 bytes
	php
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4436)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4449)
	; 11 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
	tay
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4473 || n = 4488 || n = 4490 || n = 4492 || n = 4511 || n = 4526 || n = 4530)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4455)
	; 12 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop zp,x'
	nop
	rol $36,X
	ror $36,X
	dey
	Jne *-7
	tay
.elseif(n = 4456 || (n > 4457 && n < 4465) || n = 4466 || n = 4475 || n = 4477 || (n > 4478 && n < 4485) || (n > 4493 && n < 4499) || (n > 4499 && n < 4503) || n = 4514 || (n > 4515 && n < 4520) || n = 4528)
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
.elseif(n = 4457 || n = 4478 || n = 4499 || n = 4520)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4465 || n = 4525)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4467)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4468)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 4469 || n = 4487 || n = 4505 || n = 4523)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4470 || n = 4489 || n = 4508 || n = 4527)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4471 || n = 4493 || n = 4515)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4472 || n = 4491 || n = 4507 || n = 4510 || n = 4529)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4474 || n = 4509 || n = 4512 || n = 4531)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4476)
	; 12 bytes
	tya
	ldy #(n-24)/21 ;hides 'nop zp,x'
	iny ;first loop only
	rol $36,X
	ror $36,X
	dey
	Jne *-7
	tay
.elseif(n = 4485)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4486)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4503)
	; 11 bytes
	tya
	ldy #(n-3)/18 ;hides 'nop'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 4504)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4506)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 4513)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 13
	iny
	Jne *-7
	tay
.elseif(n = 4521)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4522)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4524)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4537) || n = 4538 || n = 4547 || (n > 4550 && n < 4557) || n = 4560 || n = 4564 || n = 4566 || (n > 4569 && n < 4575) || n = 4578 || n = 4582 || (n > 4588 && n < 4593) || n = 4596 || (n > 4599 && n < 4603) || n = 4608 || n = 4609)
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
.elseif(n = 4537)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4539)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4540)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4541 || n = 4559 || n = 4577 || n = 4595)
	; 11 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4542 || n = 4544 || n = 4549 || n = 4568 || n = 4587 || n = 4598 || n = 4606)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4543 || n = 4548 || n = 4561 || n = 4567 || n = 4579 || n = 4586 || n = 4597 || n = 4605)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4545 || n = 4585)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4546 || n = 4565 || n = 4584 || n = 4603)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4550 || n = 4563 || n = 4569 || n = 4588 || n = 4599 || n = 4607)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4557)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4558)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4562 || n = 4583 || n = 4604)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4575)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4576)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
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
.elseif(n = 4581)
	; 11 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4593)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4594)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.endif
.else
; 4610 to 4687
.if(n = 4610 || n = 4614 || n = 4618 || n = 4619 || n = 4621 || (n > 4626 && n < 4637) || (n > 4647 && n < 4658) || n = 4659 || n = 4668 || n = 4670 || n = 4671 || (n > 4672 && n < 4679) || n = 4680 || n = 4684)
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
.elseif(n = 4611)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4612)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4613)
	; 11 bytes
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-5
	tay
.elseif(n = 4615 || n = 4624 || n = 4662 || n = 4681)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4616 || n = 4642 || n = 4644 || n = 4661 || n = 4663 || n = 4682 || n = 4686)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4617 || n = 4626 || n = 4664 || n = 4687)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4620)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4622 || n = 4641 || n = 4660)
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
.elseif(n = 4625 || n = 4646 || n = 4667)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4637)
	; 11 bytes
	tya
	ldy #(n-1)/19 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4638)
	; 11 bytes
	tya
	ldy #(n-2)/19 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4639)
	; 11 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop zp,x'
	nop
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4640)
	; 12 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4643)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 13
	iny
	Jne *-6
	tay
.elseif(n = 4645 || n = 4665 || n = 4685)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4647 || n = 4669)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4658)
	; 11 bytes
	tya
	ldy #(n-22)/19 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4666)
	; 12 bytes
	tya
	ldy #(n-2)/22 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 4672)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	nop
	delay_n 14
	Jne *-7
	tax
.elseif(n = 4679)
	; 11 bytes
	php
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4683)
	; 11 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if(n = 4688 || n = 4709 || n = 4730 || n = 4751)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4689)
	; 12 bytes
	tya
	ldy #(n-25)/22 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 4690 || (n > 4691 && n < 4695) || n = 4696 || n = 4697 || n = 4699 || n = 4704 || (n > 4705 && n < 4709) || (n > 4709 && n < 4713) || n = 4714 || n = 4716 || n = 4724 || (n > 4725 && n < 4730) || (n > 4730 && n < 4734) || n = 4737 || n = 4741 || n = 4742 || n = 4744 || (n > 4745 && n < 4751) || n = 4752 || (n > 4759 && n < 4765))
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
.elseif(n = 4691 || n = 4713 || n = 4735)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4695)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4698 || n = 4717 || n = 4736 || n = 4755)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4700 || n = 4719 || n = 4738 || n = 4757)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4701 || n = 4720 || n = 4739 || n = 4756 || n = 4758)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4702 || n = 4721 || n = 4740 || n = 4759)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4703)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4705 || n = 4725 || n = 4745 || n = 4765)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4715)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
	tay
.elseif(n = 4718)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4722)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4723)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4734)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4743)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4753)
	; 11 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	rol $36,X
	ror $36,X
	dey
	Jne *-6
	tay
.elseif(n = 4754)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 10
	iny
	Jne *-7
	tay
.endif
.else
; 4766 to 4843
.if((n > 4765 && n < 4772) || n = 4775 || (n > 4779 && n < 4785) || (n > 4785 && n < 4791) || n = 4794 || n = 4798 || n = 4800 || n = 4802 || n = 4804 || (n > 4805 && n < 4810) || n = 4813 || (n > 4816 && n < 4822) || n = 4824 || (n > 4825 && n < 4829) || n = 4832 || (n > 4837 && n < 4844))
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
.elseif(n = 4772 || n = 4814 || n = 4835)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4773)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4774 || n = 4793 || n = 4812 || n = 4831)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4776 || n = 4795)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4777 || n = 4796 || n = 4815 || n = 4834 || n = 4836)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4778 || n = 4797 || n = 4816 || n = 4837)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4779 || n = 4801 || n = 4823)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4785 || n = 4805 || n = 4825)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4791)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4792)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4799)
	; 12 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 4803)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 17
	iny
	Jne *-7
	tay
.elseif(n = 4810)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4811)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4822)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4829)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4830)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4833)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 14
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
; 4844 to 4921
.if(n = 4844)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4845 || n = 4865 || n = 4885 || n = 4905)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4846 || n = 4847 || n = 4851 || n = 4855 || (n > 4856 && n < 4863) || n = 4864 || n = 4870 || n = 4874 || n = 4876 || n = 4880 || n = 4884 || (n > 4885 && n < 4889) || (n > 4889 && n < 4898) || (n > 4898 && n < 4902) || n = 4904 || (n > 4905 && n < 4911) || (n > 4911 && n < 4917) || n = 4918)
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
.elseif(n = 4848)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4849)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4850)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4852 || n = 4871)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4853 || n = 4872 || n = 4878 || n = 4920)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4856 || n = 4877 || n = 4898 || n = 4919)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4863)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4866)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 4867 || n = 4889 || n = 4911)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4868)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 4869)
	; 11 bytes
	tya
	ldy #0
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4873 || n = 4921)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4875)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 14
	iny
	Jne *-6
	tay
.elseif(n = 4879)
	; 12 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop zp,x'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 4881)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 4882)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4883)
	; 12 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop zp,x'
	nop
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4902)
	; 12 bytes
	tya
	ldy #(n-26)/23 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 4903)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4917)
	; 11 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
.endif
.else
; 4922 to 5000
.if((n > 4921 && n < 4925) || (n > 4925 && n < 4933) || n = 4934 || n = 4935 || n = 4937 || (n > 4940 && n < 4945) || (n > 4945 && n < 4955) || n = 4956 || n = 4960 || n = 4962 || n = 4964 || (n > 4967 && n < 4971) || (n > 4971 && n < 4977) || n = 4978 || n = 4979 || n = 4981 || n = 4983 || n = 4984 || (n > 4985 && n < 4999) || n = 5000)
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
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 4933 || n = 4955 || n = 4977 || n = 4999)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 4936)
	; 11 bytes
	php
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 4938)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4939 || n = 4966)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4940 || n = 4967)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4957)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4958)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4959)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	tay
.elseif(n = 4961 || n = 4982)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 4963)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4971)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 4980)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	rol $36,X
	ror $36,X
	iny
	Jne *-7
	tay
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
; 5001 to 5156
.if(n = 5001 || n = 5002 || n = 5004 || (n > 5007 && n < 5017) || (n > 5017 && n < 5021) || n = 5022 || n = 5023 || (n > 5025 && n < 5043) || n = 5044 || (n > 5045 && n < 5063) || n = 5064 || (n > 5066 && n < 5078) || (n > 5078 && n < 5085) || n = 5086 || (n > 5087 && n < 5105) || (n > 5109 && n < 5125) || n = 5128 || n = 5130 || (n > 5131 && n < 5148) || n = 5149 || n = 5152 || (n > 5153 && n < 5157))
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
.elseif(n = 5003)
	; 11 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 5005 || n = 5025 || n = 5065 || n = 5085 || n = 5105)
	; 12 bytes
	tya
	ldy #(n-5)/20
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-6
	tay
.elseif(n = 5006)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5007)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5017)
	; 12 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5021 || n = 5043 || n = 5109 || n = 5131 || n = 5153)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5024 || n = 5045 || n = 5066 || n = 5087 || n = 5108 || n = 5129 || n = 5150)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5063)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	.repeat 5
	 pha
	.endrepeat
	iny
	Jne *-7
	tay
.elseif(n = 5078)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	delay_n 17
	Jne *-7
	tax
.elseif(n = 5106)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	rol $36,X
	ror $36,X
	dey
	Jne *-7
	tay
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
.elseif(n = 5126)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 5127)
	; 12 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop zp,x'
	nop
	rol $36,X
	ror $36,X
	dey
	Jne *-7
	tay
.elseif(n = 5148)
	; 12 bytes
	tya
	ldy #(n-24)/21 ;hides 'nop zp,x'
	iny ;first loop only
	rol $36,X
	ror $36,X
	dey
	Jne *-7
	tay
.elseif(n = 5151)
	; 12 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-7
	tay
.endif
.else
; 5157 to 5312
.if((n > 5156 && n < 5171) || n = 5172 || n = 5174 || (n > 5175 && n < 5187) || (n > 5187 && n < 5192) || (n > 5197 && n < 5211) || n = 5212 || n = 5216 || n = 5218 || (n > 5219 && n < 5234) || (n > 5235 && n < 5241) || (n > 5241 && n < 5253) || n = 5254 || (n > 5257 && n < 5263) || (n > 5263 && n < 5276) || (n > 5276 && n < 5281) || (n > 5281 && n < 5285) || (n > 5285 && n < 5293) || (n > 5293 && n < 5297) || (n > 5297 && n < 5307) || (n > 5307 && n < 5313))
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
.elseif(n = 5171 || n = 5192 || n = 5213 || n = 5234 || n = 5255 || n = 5276 || n = 5297)
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5173)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 13
	iny
	Jne *-7
	tay
.elseif(n = 5175 || n = 5219 || n = 5241 || n = 5263 || n = 5285 || n = 5307)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5187)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5193)
	; 11 bytes
	php
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5194)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 5195)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5196 || n = 5214 || n = 5256)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5197 || n = 5215 || n = 5257)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5211)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5217)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 13
	iny
	Jne *-7
	tay
.elseif(n = 5235)
	; 12 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5253)
	; 11 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
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
.elseif(n = 5293)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
.endif
.endif
.else
; 5313 to 5625
.if((n > 5312 && n < 5316) || n = 5317 || (n > 5318 && n < 5329) || (n > 5329 && n < 5339) || (n > 5339 && n < 5349) || n = 5350 || (n > 5351 && n < 5360) || (n > 5360 && n < 5369) || n = 5372 || (n > 5373 && n < 5381) || (n > 5381 && n < 5385) || (n > 5385 && n < 5393) || n = 5394 || (n > 5395 && n < 5408) || (n > 5408 && n < 5417) || (n > 5417 && n < 5431) || (n > 5431 && n < 5439) || (n > 5439 && n < 5450) || n = 5451 || (n > 5454 && n < 5459) || n = 5460 || (n > 5461 && n < 5483) || (n > 5483 && n < 5503) || n = 5504 || (n > 5505 && n < 5523) || (n > 5523 && n < 5527) || (n > 5527 && n < 5549) || (n > 5549 && n < 5571) || (n > 5571 && n < 5592) || (n > 5593 && n < 5615) || (n > 5615 && n < 5619) || (n > 5619 && n < 5626))
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
	; 12 bytes
	tya
	ldy #(n-5)/21
	nop
	delay_n 14
	dey
	Jne *-6
	tay
.elseif(n = 5329 || n = 5351 || n = 5373 || n = 5395 || n = 5417 || n = 5439 || n = 5461 || n = 5483 || n = 5505 || n = 5527 || n = 5549 || n = 5571 || n = 5593)
	; 12 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5349)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 5369)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 5370)
	; 12 bytes
	tya
	ldy #(n-2)/22 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.elseif(n = 5371)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 13
	dey
	Jne *-7
	tay
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
	ldy #(n-3)/23 ;hides 'nop'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5393)
	; 12 bytes
	tya
	ldy #(n-25)/22 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 13
	dey
	Jne *-7
	tay
.elseif(n = 5408)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 5431)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
.elseif(n = 5450)
	; 11 bytes
	php
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5452)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5453)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5454)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5459)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 5503)
	; 12 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	.repeat 5
	 pha
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 5523)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 17
	iny
	Jne *-7
	tay
.elseif(n = 5592)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5615)
	; 12 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop zp,x'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5619)
	; 12 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	delay_n 17
	dey
	Jne *-7
	tay
.endif
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5637) || (n > 5638 && n < 5707) || n = 5708 || (n > 5711 && n < 5753) || (n > 5753 && n < 5955) || (n > 5955 && n < 5964) || n = 5965 || (n > 5968 && n < 6003) || (n > 6003 && n < 6221) || n = 6222 || (n > 6225 && n < 6251))
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
	; 12 bytes
	tya
	ldy #0
	delay_n 17
	dey
	Jne *-6
	tay
.elseif(n = 5638)
	; 12 bytes
	tya
	ldy #(n-26)/23 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5707)
	; 11 bytes
	php
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 5709 || n = 5966 || n = 6223)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5710 || n = 5967 || n = 6224)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5711 || n = 5968 || n = 6225)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5753)
	; 12 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	nop
	delay_n 14
	dey
	Jne *-7
	tay
.elseif(n = 5955)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5964)
	; 11 bytes
	php
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6003)
	; 12 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 6221)
	; 11 bytes
	php
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6478) || n = 6479 || (n > 6482 && n < 6735) || n = 6736 || (n > 6739 && n < 6992) || n = 6993 || (n > 6996 && n < 7249) || n = 7250 || (n > 7253 && n < 7501))
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
.elseif(n = 6478)
	; 11 bytes
	php
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6480 || n = 6737 || n = 6994 || n = 7251)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6481 || n = 6738 || n = 6995 || n = 7252)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6482 || n = 6739 || n = 6996 || n = 7253)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6735)
	; 11 bytes
	php
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 6992)
	; 11 bytes
	php
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7249)
	; 11 bytes
	php
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7506) || n = 7507 || (n > 7510 && n < 7763) || n = 7764 || (n > 7767 && n < 8020) || n = 8021 || (n > 8024 && n < 8277) || n = 8278 || (n > 8281 && n < 8534) || n = 8535 || (n > 8538 && n < 8791) || n = 8792 || (n > 8795 && n < 9048) || n = 9049 || (n > 9052 && n < 9305) || n = 9306 || (n > 9309 && n < 9562) || n = 9563 || (n > 9566 && n < 9819) || n = 9820 || (n > 9823 && n < 10001))
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
.elseif(n = 7506)
	; 11 bytes
	php
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 7508 || n = 7765 || n = 8022 || n = 8279 || n = 8536 || n = 8793 || n = 9050 || n = 9307 || n = 9564 || n = 9821)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7509 || n = 7766 || n = 8023 || n = 8280 || n = 8537 || n = 8794 || n = 9051 || n = 9308 || n = 9565 || n = 9822)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7510 || n = 7767 || n = 8024 || n = 8281 || n = 8538 || n = 8795 || n = 9052 || n = 9309 || n = 9566 || n = 9823)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7763)
	; 11 bytes
	php
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8020)
	; 11 bytes
	php
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8277)
	; 11 bytes
	php
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8534)
	; 11 bytes
	php
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 8791)
	; 11 bytes
	php
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9048)
	; 11 bytes
	php
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9305)
	; 11 bytes
	php
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9562)
	; 11 bytes
	php
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 9819)
	; 11 bytes
	php
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10076) || n = 10077 || (n > 10080 && n < 10333) || n = 10334 || (n > 10337 && n < 10590) || n = 10591 || (n > 10594 && n < 10847) || n = 10848 || (n > 10851 && n < 11104) || n = 11105 || (n > 11108 && n < 11361) || n = 11362 || (n > 11365 && n < 11618) || n = 11619 || (n > 11622 && n < 11875) || n = 11876 || (n > 11879 && n < 12132) || n = 12133 || (n > 12136 && n < 12389) || n = 12390 || (n > 12393 && n < 12646) || n = 12647 || (n > 12650 && n < 12903) || n = 12904 || (n > 12907 && n < 13160) || n = 13161 || (n > 13164 && n < 13417) || n = 13418 || (n > 13421 && n < 13674) || n = 13675 || (n > 13678 && n < 13931) || n = 13932 || (n > 13935 && n < 14188) || n = 14189 || (n > 14192 && n < 14445) || n = 14446 || (n > 14449 && n < 14702) || n = 14703 || (n > 14706 && n < 14959) || n = 14960 || (n > 14963 && n < 15001))
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
.elseif(n = 10076)
	; 11 bytes
	php
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10078 || n = 10335 || n = 10592 || n = 10849 || n = 11106 || n = 11363 || n = 11620 || n = 11877 || n = 12134 || n = 12391 || n = 12648 || n = 12905 || n = 13162 || n = 13419 || n = 13676 || n = 13933 || n = 14190 || n = 14447 || n = 14704 || n = 14961)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10079 || n = 10336 || n = 10593 || n = 10850 || n = 11107 || n = 11364 || n = 11621 || n = 11878 || n = 12135 || n = 12392 || n = 12649 || n = 12906 || n = 13163 || n = 13420 || n = 13677 || n = 13934 || n = 14191 || n = 14448 || n = 14705 || n = 14962)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10080 || n = 10337 || n = 10594 || n = 10851 || n = 11108 || n = 11365 || n = 11622 || n = 11879 || n = 12136 || n = 12393 || n = 12650 || n = 12907 || n = 13164 || n = 13421 || n = 13678 || n = 13935 || n = 14192 || n = 14449 || n = 14706 || n = 14963)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 10333)
	; 11 bytes
	php
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10590)
	; 11 bytes
	php
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 10847)
	; 11 bytes
	php
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11104)
	; 11 bytes
	php
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11361)
	; 11 bytes
	php
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11618)
	; 11 bytes
	php
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 11875)
	; 11 bytes
	php
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12132)
	; 11 bytes
	php
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12389)
	; 11 bytes
	php
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12646)
	; 11 bytes
	php
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 12903)
	; 11 bytes
	php
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13160)
	; 11 bytes
	php
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13417)
	; 11 bytes
	php
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13674)
	; 11 bytes
	php
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 13931)
	; 11 bytes
	php
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14188)
	; 11 bytes
	php
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14445)
	; 11 bytes
	php
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14702)
	; 11 bytes
	php
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 14959)
	; 11 bytes
	php
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15216) || n = 15217 || (n > 15220 && n < 15473) || n = 15474 || (n > 15477 && n < 15730) || n = 15731 || (n > 15734 && n < 15987) || n = 15988 || (n > 15991 && n < 16244) || n = 16245 || (n > 16248 && n < 16501) || n = 16502 || (n > 16505 && n < 16758) || n = 16759 || (n > 16762 && n < 17015) || n = 17016 || (n > 17019 && n < 17272) || n = 17273 || (n > 17276 && n < 17529) || n = 17530 || (n > 17533 && n < 17786) || n = 17787 || (n > 17790 && n < 18043) || n = 18044 || (n > 18047 && n < 18300) || n = 18301 || (n > 18304 && n < 18557) || n = 18558 || (n > 18561 && n < 18814) || n = 18815 || (n > 18818 && n < 19071) || n = 19072 || (n > 19075 && n < 19328) || n = 19329 || (n > 19332 && n < 19585) || n = 19586 || (n > 19589 && n < 19842) || n = 19843 || (n > 19846 && n < 20001))
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
.elseif(n = 15216)
	; 11 bytes
	php
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15218 || n = 15475 || n = 15732 || n = 15989 || n = 16246 || n = 16503 || n = 16760 || n = 17017 || n = 17274 || n = 17531 || n = 17788 || n = 18045 || n = 18302 || n = 18559 || n = 18816 || n = 19073 || n = 19330 || n = 19587 || n = 19844)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15219 || n = 15476 || n = 15733 || n = 15990 || n = 16247 || n = 16504 || n = 16761 || n = 17018 || n = 17275 || n = 17532 || n = 17789 || n = 18046 || n = 18303 || n = 18560 || n = 18817 || n = 19074 || n = 19331 || n = 19588 || n = 19845)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15220 || n = 15477 || n = 15734 || n = 15991 || n = 16248 || n = 16505 || n = 16762 || n = 17019 || n = 17276 || n = 17533 || n = 17790 || n = 18047 || n = 18304 || n = 18561 || n = 18818 || n = 19075 || n = 19332 || n = 19589 || n = 19846)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 15473)
	; 11 bytes
	php
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15730)
	; 11 bytes
	php
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 15987)
	; 11 bytes
	php
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 16244)
	; 11 bytes
	php
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 16501)
	; 11 bytes
	php
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 16758)
	; 11 bytes
	php
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17015)
	; 11 bytes
	php
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17272)
	; 11 bytes
	php
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17529)
	; 11 bytes
	php
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 17786)
	; 11 bytes
	php
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18043)
	; 11 bytes
	php
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18300)
	; 11 bytes
	php
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18557)
	; 11 bytes
	php
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 18814)
	; 11 bytes
	php
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19071)
	; 11 bytes
	php
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19328)
	; 11 bytes
	php
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19585)
	; 11 bytes
	php
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.elseif(n = 19842)
	; 11 bytes
	php
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
