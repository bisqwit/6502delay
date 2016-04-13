; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, X, Y, S, Z+N, C, and V
; Will preserve D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 12
; - 4 bytes or less when n <= 16
; - 5 bytes or less when n <= 21
; - 6 bytes or less when n <= 316
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=4.2, max=5
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=4, avg=4.9, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=4, avg=5.0, max=5
; - When 201 <= n <= 300: min=4, avg=5.1, max=6
; - When 301 <= n <= 500: min=4, avg=5.8, max=7
; - When 501 <= n <= 750: min=4, avg=5.8, max=7
; - When 751 <= n <= 1000: min=5, avg=5.9, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.4, max=7
; - When 2001 <= n <= 3000: min=5, avg=6.9, max=7
; - When 3001 <= n <= 5000: min=5, avg=7.0, max=7
; - When 5001 <= n <= 7500: min=5, avg=7.0, max=7
; - When 7501 <= n <= 10000: min=5, avg=7.0, max=7
; - When 10001 <= n <= 20000: min=5, avg=7.0, max=7
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
.elseif(n = 5 || n = 18)
	; 2..5 bytes
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
.elseif(n = 9 || n = 10 || n = 13 || n = 19)
	; 3..5 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 4 bytes
	pla
	pha
	pla
	pla
.elseif(n = 16)
	; 4 bytes
	.repeat 4
	 pla
	.endrepeat
.elseif(n = 17)
	; 5 bytes
	delay_n (n-14)
	delay_n 14
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
	; 5 bytes
	lda #$2A ;hides 'rol a'
	pha
	Jpl *-2
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25 || n = 28)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 27)
	; 5 bytes
	pha
	delay_n (n-3)
.elseif((n > 28 && n < 39) || (n > 39 && n < 49) || (n > 49 && n < 124) || (n > 124 && n < 157))
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
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
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
.elseif(n = 285 || n = 294 || n = 298 || n = 303 || n = 308)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 287 || n = 295)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 288 || n = 310)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 289 || n = 297 || n = 305)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 290)
	; 6 bytes
	ldy #200 ;hides 'iny'
	Jne *-1
	sta (@ptrtemp),y
.elseif(n = 292)
	; 5 bytes
	lax #1
	jsr delay_256a_x_33_clocks
.elseif(n = 293)
	; 5 bytes
	ldx #42 ;hides 'rol a'
	dex
	Jne *-2
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
.elseif(n = 302 || n = 309)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
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
.if((n >= 313) && (n < 391))
; 313 to 390
.if((n .mod 5 = 3) && (n = 313 || n = 318 || (n > 327 && n < 354) || (n > 362 && n < 379) || n = 388))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 314)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	iny
	iny
	Jne *-3
.elseif(n = 315 || n = 335 || n = 340 || n = 350 || n = 355 || n = 360 || n = 375 || n = 380 || n = 383 || n = 390)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 387)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 327 || n = 342 || n = 347 || n = 357 || n = 362 || n = 367)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 319 || n = 324 || n = 334 || n = 339 || n = 349 || n = 354 || n = 359 || n = 364 || n = 374 || n = 382 || n = 384 || n = 389)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 320)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jmi *-3
.elseif(n = 321 || n = 326 || n = 336 || n = 356)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 323 || n = 330 || n = 344 || n = 365)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 325 || n = 352 || n = 370)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 329 || n = 345 || n = 369)
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
.elseif(n = 337 || n = 358 || n = 372)
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
.elseif(n = 385)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 387)
	; 6 bytes
	delay_n 8
	delay_n (n-8)
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 401 || n = 411 || n = 426 || n = 441 || n = 451 || n = 456 || n = 461)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 392 || n = 422 || n = 427 || n = 432 || n = 437 || n = 447 || n = 452 || n = 462)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 393 || n = 449)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 4) && (n = 394 || n = 399 || (n > 418 && n < 445) || n = 454 || n = 459))
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 395)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 397 || n = 415)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 398)
	; 5 bytes
	ldx #56 ;hides 'sec'
	dex
	Jpl *-2
.elseif(n = 400 || n = 407 || n = 414 || n = 428 || n = 435 || n = 442 || n = 463)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 402 || n = 410 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 3) && ((n > 402 && n < 424) || (n > 432 && n < 459) || n = 468))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 404)
	; 6 bytes
	ldx #90 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 405)
	; 5 bytes
	ldx #58 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 406 || n = 416 || n = 421 || n = 431 || n = 436 || n = 446 || n = 466)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 409 || n = 425)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 412)
	; 5 bytes
	ldy #186 ;hides 'tsx'
	dey
	Jmi *-2
.elseif(n = 417 || n = 457 || n = 465)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 455)
	; 5 bytes
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
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
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 479 || n = 492 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 534)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 470 || n = 498)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 471 || n = 501 || n = 521)
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
.elseif((n .mod 5 = 3) && ((n > 472 && n < 494) || (n > 507 && n < 529) || n = 538 || n = 543))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 475 || n = 490 || n = 495 || n = 500 || n = 507 || n = 515 || n = 520 || n = 525 || n = 535)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 475 && n < 497) || (n > 505 && n < 517) || (n > 525 && n < 547)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 477 || n = 484 || n = 505 || n = 512 || n = 519 || n = 533 || n = 540)
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
.elseif(n = 482 || n = 502 || n = 522 || n = 542)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 497 || n = 529 || n = 545)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
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
.elseif(n = 532)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 537)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.endif
.else
; 547 to 625
.if(n = 547 || n = 554 || n = 575 || n = 617)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 3) && (n = 548 || n = 558 || n = 563 || (n > 572 && n < 599) || (n > 607 && n < 624)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 549)
	; 5 bytes
	lax #2
	jsr delay_256a_x_33_clocks
.elseif(n = 550 || n = 595 || n = 622)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
.elseif(n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 552 || n = 559 || n = 564 || n = 574 || n = 579 || n = 584 || n = 594 || n = 599 || n = 604 || n = 614 || n = 619)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 553 || n = 555 || n = 565 || n = 570 || n = 580 || n = 590 || n = 600 || n = 605)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 560)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 562 || n = 567 || n = 572 || n = 587 || n = 592 || n = 597 || n = 612)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 569)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 577 || n = 585 || n = 609 || n = 625)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 602)
	; 6 bytes
	ldx #122 ;hides 'nop'
	inx
	inx
	Jne *-3
.elseif(n = 607)
	; 5 bytes
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
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
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if((n .mod 5 = 1) && (n = 626 || n = 636 || n = 641 || (n > 650 && n < 702)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 627 || n = 642 || n = 662 || n = 672 || n = 677 || n = 682 || n = 702)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 3) && (n = 628 || n = 633 || (n > 642 && n < 669) || n = 678 || n = 683 || (n > 692 && n < 704)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 5 bytes
	ldx #90 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 630 || n = 635 || n = 650 || n = 655 || n = 660 || n = 670 || n = 675 || n = 688 || n = 695 || n = 700)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 632 || n = 634 || n = 639 || n = 644 || n = 654 || n = 664 || n = 669 || n = 679 || n = 687 || n = 699)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 637)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 638 || n = 645 || n = 673 || n = 680 || n = 694)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 640)
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
.elseif(n = 649 || n = 689 || n = 697)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 652 || n = 659)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657)
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
.elseif(n = 667 || n = 685)
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
.elseif(n = 684)
	; 4 bytes
	ldy #136 ;hides 'dey'
	Jne *-1
.elseif(n = 690)
	; 6 bytes
	delay_n (n-6)
	sta (@ptrtemp),y
.elseif(n = 692)
	; 6 bytes
	ldx #180 ;hides 'ldy zp,x'
	dex ;first loop only
	inx
	Jne *-3
.endif
.else
; 704 to 781
.if(n = 704)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 705 || n = 737 || n = 769 || n = 777)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 705 && n < 782)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 707 || n = 732 || n = 742 || n = 747 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 7 = 1) && ((n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 709 || n = 714 || n = 719 || n = 724 || n = 730 || n = 739 || n = 749 || n = 754 || n = 759 || n = 774 || n = 779)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 710 || n = 717 || n = 725 || n = 735 || n = 740 || n = 752 || n = 760 || n = 765 || n = 770 || n = 780)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 712)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	iny
	Jne *-3
.elseif(n = 713)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jpl *-2
.elseif((n .mod 5 = 3) && ((n > 717 && n < 739) || (n > 752 && n < 774)))
	; 6 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
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
.elseif(n = 745)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
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
.elseif(n = 775)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.endif
.endif
.else
; 782 to 937
.if(n = 782)
	; 6 bytes
	ldx #213 ;hides 'cmp zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.elseif((n .mod 5 = 3) && ((n > 782 && n < 809) || (n > 817 && n < 844) || (n > 857 && n < 879) || (n > 887 && n < 914) || (n > 922 && n < 934)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 784 || n = 789 || n = 794 || n = 804 || n = 809 || n = 814 || n = 819 || n = 829 || n = 842 || n = 844 || n = 854 || n = 859 || n = 864 || n = 874 || n = 879 || n = 884 || n = 894 || n = 909 || n = 914 || n = 919 || n = 924 || n = 934)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 785 || n = 799 || n = 813 || n = 834 || n = 848 || n = 855 || n = 862 || n = 883 || n = 904)
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif((n .mod 5 = 1) && ((n > 785 && n < 802) || (n > 810 && n < 937)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && (n = 787 || n = 797 || n = 807 || n = 812 || n = 822 || n = 832 || n = 837 || n = 852 || (n > 866 && n < 888) || (n > 901 && n < 913) || n = 922 || n = 927))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 5 = 0) && (n = 790 || n = 795 || (n > 804 && n < 816) || n = 830 || (n > 839 && n < 851) || (n > 869 && n < 886) || n = 895 || n = 900 || n = 915 || n = 920 || n = 930))
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
.elseif(n = 806)
	; 5 bytes
	lax #3
	jsr delay_256a_x_33_clocks
.elseif(n = 817 || n = 849 || n = 857 || n = 865 || n = 889 || n = 929)
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
.elseif(n = 839)
	; 5 bytes
	ldx #136 ;hides 'dey'
	inx
	Jne *-2
.elseif(n = 853)
	; 5 bytes
	ldx #122 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 860)
	; 5 bytes
	ldx #250 ;hides 'nop'
	dex
	Jmi *-2
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
.elseif(n = 905)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 917)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
	inx
	Jpl *-3
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.elseif(n = 937)
	; 5 bytes
	ldx #122 ;hides 'nop'
	inx
	Jne *-2
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 943 || n = 948 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1013 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1065 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || n = 1058 || n = 1072 || n = 1079 || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 940 || n = 950 || n = 970 || n = 975 || n = 990 || n = 1005 || n = 1010 || n = 1015 || n = 1018 || n = 1035 || n = 1040 || n = 1050 || n = 1055 || n = 1060 || n = 1067 || n = 1075 || n = 1080 || n = 1085)
	; 5..6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && ((n > 941 && n < 958) || n = 972 || n = 987 || n = 997 || n = 1007 || n = 1012 || n = 1032 || n = 1042 || n = 1047 || n = 1062 || (n > 1076 && n < 1093)))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 944)
	; 6 bytes
	ldy #232 ;hides 'inx'
	nop
	dey
	Jmi *-3
.elseif(n = 945 || n = 1025 || n = 1057 || n = 1089)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 949 || n = 954 || n = 959 || n = 964 || n = 979 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084)
	; 5..6 bytes
	pha
	delay_n (n-3)
.elseif(n = 955 || n = 982 || n = 1000 || n = 1027 || n = 1045 || n = 1090)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.elseif(n = 969 || n = 977 || n = 985)
	; 6 bytes
	ldx #(n-9)/8
	pha
	dex
	Jpl *-2
.elseif(n = 980)
	; 6 bytes
	ldx #218 ;hides 'nop'
	dex
	dex
	Jne *-3
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
	delay_n (n-6)
	sta (@ptrtemp,x)
.elseif(n = 1022)
	; 6 bytes
	delay_n (n-8)
	slo (@ptrtemp,x)
.elseif(n = 1034)
	; 6 bytes
	ldx #26 ;hides 'nop'
	inx
	inx
	Jne *-3
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
	lax #4
	jsr delay_256a_x_33_clocks
.elseif(n = 1070)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	dex
	Jmi *-3
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1099 || n = 1104 || n = 1109 || n = 1119 || n = 1134 || n = 1139 || n = 1144 || n = 1154 || n = 1159 || n = 1164 || n = 1174 || n = 1179 || n = 1192 || n = 1194 || n = 1199 || n = 1204 || n = 1214 || n = 1224 || n = 1229 || n = 1234 || n = 1239 || n = 1244)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1095)
	; 5 bytes
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1097)
	; 6 bytes
	ldx #122 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1124) || (n > 1132 && n < 1159) || (n > 1167 && n < 1194) || (n > 1202 && n < 1229) || (n > 1237 && n < 1249)))
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1102 || n = 1112 || n = 1122 || n = 1127 || n = 1132 || n = 1147 || n = 1152 || n = 1157 || n = 1167 || n = 1172 || n = 1182 || n = 1187 || n = 1197 || n = 1202 || n = 1222 || n = 1227 || n = 1237 || n = 1242)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1105 || n = 1129 || n = 1137 || n = 1145 || n = 1169 || n = 1185 || n = 1209 || n = 1217 || n = 1225 || n = 1249)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && ((n > 1109 && n < 1131) || n = 1140 || (n > 1149 && n < 1166) || (n > 1189 && n < 1201) || (n > 1209 && n < 1221) || n = 1230 || n = 1235 || n = 1245))
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1117 || n = 1180 || n = 1207)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1124)
	; 6 bytes
	ldx #250 ;hides 'nop'
	dex
	dex
	Jne *-3
.elseif(n = 1162)
	; 6 bytes
	ldx #(n-10)/9
	pla
	dex
	Jpl *-2
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
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 5 = 1) && ((n > 1250 && n < 1277)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1252 || n = 1270 || n = 1288 || n = 1306 || n = 1315)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1278 || n = 1283 || n = 1322)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1318)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1255 || n = 1260 || n = 1277 || n = 1280 || n = 1285 || n = 1291 || n = 1324)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 8 = 1) && (n = 1257 || n = 1265 || (n > 1296 && n < 1322)))
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
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1286 || (n > 1291 && n < 1296) || (n > 1297 && n < 1301) || n = 1302 || (n > 1306 && n < 1310) || n = 1311 || n = 1314 || n = 1316 || n = 1318 || (n > 1324 && n < 1328))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290 || n = 1323)
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
.elseif(n = 1319)
	; 6 bytes
	ldx #136 ;hides 'dey'
	pla
	inx
	Jne *-3
.elseif(n = 1320)
	; 5 bytes
	lax #5
	jsr delay_256a_x_33_clocks
.elseif(n = 1328)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
.endif
.else
; 1329 to 1406
.if((n .mod 8 = 1) && (n = 1329 || n = 1337 || (n > 1352 && n < 1378) || n = 1393))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1330 || n = 1332 || n = 1334 || n = 1335 || n = 1340 || n = 1343 || (n > 1345 && n < 1349) || n = 1350 || (n > 1353 && n < 1359) || (n > 1361 && n < 1366) || n = 1368 || (n > 1369 && n < 1373) || n = 1374 || n = 1375 || (n > 1380 && n < 1385) || n = 1386 || (n > 1389 && n < 1393) || n = 1395 || n = 1397 || n = 1398 || n = 1400 || n = 1404 || n = 1406)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1361) || n = 1378 || n = 1396 || n = 1405))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1336)
	; 6 bytes
	ldy #166 ;hides 'ldx zp'
	iny ;first loop only
	dey
	Jne *-3
.elseif(n = 1339)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1341)
	; 6 bytes
	ldx #122 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 1344)
	; 6 bytes
	ldy #167 ;hides 'lax zp'
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
.elseif(n = 1379)
	; 6 bytes
	ldx #138 ;hides 'txa'
	pha
	dex
	Jne *-3
.elseif(n = 1385)
	; 5 bytes
	ldx #58 ;hides 'nop'
	inx
	Jne *-2
.elseif(n = 1388 || n = 1402)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1389 || n = 1403)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.endif
.endif
.else
; 1407 to 1562
.if(n = 1407 || (n > 1409 && n < 1413) || n = 1418 || n = 1419 || n = 1421 || (n > 1425 && n < 1429) || n = 1430 || n = 1431 || n = 1434 || n = 1435 || (n > 1436 && n < 1441) || n = 1442 || (n > 1443 && n < 1449) || (n > 1450 && n < 1457) || n = 1458 || (n > 1459 && n < 1464) || n = 1466 || n = 1467 || n = 1469 || n = 1470 || n = 1472 || (n > 1473 && n < 1477) || n = 1479 || n = 1480 || (n > 1481 && n < 1485) || n = 1487 || n = 1490 || n = 1491 || n = 1494 || n = 1498 || n = 1502 || n = 1503 || (n > 1506 && n < 1511) || (n > 1513 && n < 1517) || n = 1518 || n = 1523 || n = 1524 || n = 1526 || n = 1530 || n = 1532 || n = 1533 || n = 1535 || n = 1536 || n = 1539 || (n > 1541 && n < 1545) || n = 1546 || (n > 1549 && n < 1553) || n = 1554 || n = 1557 || n = 1559)
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
.elseif((n .mod 8 = 1) && ((n > 1408 && n < 1450) || (n > 1464 && n < 1490) || n = 1505 || n = 1521 || (n > 1536 && n < 1562)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1413)
	; 5 bytes
	ldy #202 ;hides 'dex'
	dey
	Jne *-2
.elseif((n .mod 9 = 1) && (n = 1414 || n = 1432 || (n > 1458 && n < 1496) || (n > 1521 && n < 1559)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1416 || n = 1423 || n = 1500 || n = 1528)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
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
.elseif(n = 1493)
	; 6 bytes
	ldx #90 ;hides 'nop'
	nop
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
	; 6 bytes
	ldx #168 ;hides 'tay'
	nop
	dex
	Jne *-3
.elseif(n = 1512)
	; 6 bytes
	ldx #68 ;hides 'nop zp'
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
.elseif(n = 1525)
	; 5 bytes
	ldx #218 ;hides 'nop'
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
.elseif(n = 1556)
	; 6 bytes
	ldx #84 ;hides 'nop zp,x'
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
.if((n > 1562 && n < 1567) || (n > 1569 && n < 1576) || n = 1578 || n = 1582 || (n > 1585 && n < 1590) || n = 1591 || n = 1592 || n = 1595 || n = 1596 || (n > 1597 && n < 1601) || n = 1602 || (n > 1604 && n < 1609) || (n > 1613 && n < 1617) || n = 1620 || n = 1622 || n = 1624 || n = 1629 || n = 1631 || (n > 1633 && n < 1637) || n = 1638 || n = 1642 || n = 1644 || n = 1645 || n = 1647 || (n > 1649 && n < 1653) || n = 1654 || n = 1656 || n = 1658 || (n > 1660 && n < 1665) || (n > 1667 && n < 1671) || n = 1672 || (n > 1675 && n < 1679) || n = 1680 || n = 1682 || n = 1683 || (n > 1684 && n < 1688) || (n > 1689 && n < 1693) || n = 1694 || n = 1696 || n = 1698 || n = 1700 || n = 1701 || n = 1703 || n = 1704 || (n > 1705 && n < 1709) || n = 1710 || n = 1712 || n = 1714 || n = 1715 || n = 1717 || n = 1718)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 1577)
	; 5 bytes
	lax #6
	jsr delay_256a_x_33_clocks
.elseif(n = 1579)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 1580 || n = 1626 || n = 1640)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1581 || n = 1613 || n = 1627)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 1584)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif((n .mod 8 = 1) && ((n > 1584 && n < 1602) || n = 1617 || (n > 1632 && n < 1666) || (n > 1688 && n < 1714)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1609)
	; 5 bytes
	ldx #26 ;hides 'nop'
	inx
	Jne *-2
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
.elseif(n = 1659)
	; 6 bytes
	ldx #90 ;hides 'nop'
	pha
	inx
	Jne *-3
.elseif(n = 1671)
	; 6 bytes
	ldx #152 ;hides 'tya'
	pla
	dex
	Jne *-3
.elseif(n = 1673)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	nop
	dey
	Jne *-3
.elseif(n = 1679)
	; 6 bytes
	ldx #168 ;hides 'tay'
	pha
	dex
	Jne *-3
.elseif(n = 1699)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pha
	dey
	Jne *-3
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1722 || (n > 1725 && n < 1729) || (n > 1730 && n < 1737) || (n > 1738 && n < 1744) || n = 1746 || n = 1748 || n = 1750 || n = 1754 || n = 1755 || n = 1757 || n = 1759 || (n > 1761 && n < 1765) || n = 1766 || n = 1767 || n = 1770 || n = 1771 || n = 1773 || n = 1775 || n = 1776 || n = 1778 || n = 1780 || n = 1782 || n = 1784 || (n > 1786 && n < 1790) || n = 1791 || (n > 1793 && n < 1797))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1720 || n = 1738 || n = 1747 || n = 1756 || n = 1774 || n = 1783 || n = 1792)
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
.elseif((n .mod 8 = 1) && (n = 1729 || (n > 1744 && n < 1786)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1749)
	; 5 bytes
	ldx #250 ;hides 'nop'
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
.elseif(n = 1781)
	; 6 bytes
	ldx #58 ;hides 'nop'
	nop
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
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1800 || (n > 1801 && n < 1809) || (n > 1810 && n < 1816) || n = 1818 || (n > 1819 && n < 1824) || n = 1827 || (n > 1828 && n < 1832) || n = 1842 || n = 1843 || n = 1845 || n = 1848 || (n > 1849 && n < 1855) || n = 1856 || n = 1858 || (n > 1859 && n < 1864) || (n > 1865 && n < 1869) || (n > 1869 && n < 1873) || n = 1874 || n = 1875)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif((n .mod 8 = 1) && (n = 1801 || n = 1809 || (n > 1824 && n < 1874)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif(n = 1810 || n = 1819 || n = 1828 || n = 1846 || n = 1855 || n = 1864)
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
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
.elseif(n = 1834)
	; 5 bytes
	lax #7
	jsr delay_256a_x_33_clocks
.elseif(n = 1835)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1836)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 1837)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 1838)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1844)
	; 6 bytes
	ldx #52 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1847)
	; 6 bytes
	ldx #168 ;hides 'tay'
	pla
	dex
	Jne *-3
.elseif(n = 1859)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pha
	dey
	Jne *-3
.elseif(n = 1869)
	; 6 bytes
	ldy #170 ;hides 'tax'
	pla
	dey
	Jne *-3
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if((n > 1875 && n < 1881) || (n > 1882 && n < 1889) || n = 1890 || (n > 1891 && n < 1897) || n = 1899 || (n > 1900 && n < 1905) || n = 1906 || n = 1908 || (n > 1909 && n < 1913) || n = 1914 || n = 1915 || n = 1917 || n = 1919 || n = 1920 || (n > 1921 && n < 1925) || n = 1926 || n = 1928 || (n > 1929 && n < 1936) || (n > 1937 && n < 1945) || (n > 1945 && n < 1953) || (n > 1954 && n < 1961) || n = 1962 || (n > 1963 && n < 1969) || n = 1970 || n = 1971 || (n > 1972 && n < 1977) || n = 1978 || n = 1980 || (n > 1981 && n < 1985) || (n > 1985 && n < 1990) || n = 1991 || n = 1992 || (n > 1993 && n < 1999) || (n > 2001 && n < 2008) || (n > 2009 && n < 2016) || n = 2018 || (n > 2019 && n < 2023) || n = 2024 || n = 2027 || n = 2028 || n = 2030 || n = 2031)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1954) || (n > 1968 && n < 2026)))
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1937) || (n > 1953 && n < 1991) || n = 2026))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 1898)
	; 6 bytes
	ldx #212 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
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
.elseif(n = 1961)
	; 6 bytes
	ldx #218 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 1979)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pha
	inx
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
.elseif(n = 2019)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pha
	dey
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
.endif
.else
; 2032 to 2187
.if(n = 2032 || n = 2034 || (n > 2035 && n < 2041) || n = 2042 || n = 2043 || (n > 2045 && n < 2049) || (n > 2049 && n < 2053) || (n > 2053 && n < 2062) || (n > 2062 && n < 2069) || n = 2070 || (n > 2071 && n < 2078) || n = 2079 || (n > 2080 && n < 2087) || n = 2088 || n = 2090 || n = 2092 || n = 2096 || n = 2097 || (n > 2098 && n < 2105) || n = 2106 || (n > 2107 && n < 2114) || n = 2115 || (n > 2116 && n < 2123) || n = 2124 || (n > 2125 && n < 2132) || n = 2133 || (n > 2134 && n < 2139) || (n > 2139 && n < 2143) || (n > 2143 && n < 2152) || (n > 2152 && n < 2161) || (n > 2161 && n < 2170) || (n > 2170 && n < 2177) || n = 2178 || (n > 2179 && n < 2186) || n = 2187)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 2033 || n = 2041)
	; 6 bytes
	ldx #(n-1)/8
	pha
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2180)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif(n = 2045)
	; 6 bytes
	ldy #186 ;hides 'tsx'
	pla
	dey
	Jne *-3
.elseif(n = 2049)
	; 6 bytes
	ldx #0
	pha
	dex
	Jne *-2
.elseif(n = 2069)
	; 6 bytes
	ldx #26 ;hides 'nop'
	nop
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
.elseif(n = 2091)
	; 5 bytes
	lax #8
	jsr delay_256a_x_33_clocks
.elseif(n = 2093)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2094)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 2095)
	; 6 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2132)
	; 6 bytes
	ldx #20 ;hides 'nop zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 2139)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pha
	inx
	Jne *-3
.elseif(n = 2177)
	; 6 bytes
	ldx #58 ;hides 'nop'
	pla
	inx
	Jne *-3
.elseif(n = 2186)
	; 6 bytes
	ldx #244 ;hides 'nop zp,x'
	dex ;first loop only
	dex
	Jne *-3
.endif
.endif
.else
; 2188 to 2500
.if((n .mod 9 = 1) && ((n > 2187 && n < 2297)))
	; 6 bytes
	ldx #(n-1)/9
	pla
	dex
	Jne *-2
.elseif((n > 2188 && n < 2195) || n = 2196 || n = 2198 || (n > 2199 && n < 2204) || n = 2205 || (n > 2206 && n < 2213) || n = 2214 || (n > 2215 && n < 2221) || n = 2222 || n = 2223 || (n > 2224 && n < 2232) || (n > 2233 && n < 2242) || (n > 2242 && n < 2249) || n = 2250 || (n > 2251 && n < 2260) || (n > 2260 && n < 2269) || (n > 2269 && n < 2278) || (n > 2278 && n < 2287) || (n > 2287 && n < 2296) || n = 2297 || n = 2298 || (n > 2299 && n < 2305) || (n > 2305 && n < 2319) || (n > 2319 && n < 2339) || (n > 2339 && n < 2348) || n = 2349 || (n > 2353 && n < 2397) || (n > 2397 && n < 2435) || (n > 2435 && n < 2459) || (n > 2459 && n < 2499) || n = 2500)
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
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
.elseif(n = 2221)
	; 6 bytes
	ldy #202 ;hides 'dex'
	pla
	dey
	Jne *-3
.elseif(n = 2232)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
.elseif(n = 2249)
	; 6 bytes
	ldx #250 ;hides 'nop'
	nop
	dex
	Jne *-3
.elseif(n = 2299)
	; 6 bytes
	ldx #26 ;hides 'nop'
	pha
	inx
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
.elseif(n = 2348)
	; 5 bytes
	lax #9
	jsr delay_256a_x_33_clocks
.elseif(n = 2350)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 2352)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 2353)
	; 6 bytes
	ldx #42 ;hides 'rol a'
	pla
	inx
	Jne *-3
.elseif(n = 2397)
	; 6 bytes
	ldx #218 ;hides 'nop'
	pla
	dex
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
.elseif(n = 2499)
	; 6 bytes
	ldx #250 ;hides 'nop'
	pha
	dex
	Jne *-3
.endif
.endif
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2529) || (n > 2529 && n < 2551) || (n > 2551 && n < 2573) || (n > 2573 && n < 2605) || n = 2606 || (n > 2609 && n < 2705) || (n > 2705 && n < 2749) || (n > 2749 && n < 2862) || n = 2863 || (n > 2866 && n < 3119) || n = 3120 || (n > 3123 && n < 3376) || n = 3377 || (n > 3380 && n < 3633) || n = 3634 || (n > 3637 && n < 3890) || n = 3891 || (n > 3894 && n < 4147) || n = 4148 || (n > 4151 && n < 4404) || n = 4405 || (n > 4408 && n < 4661) || n = 4662 || (n > 4665 && n < 4918) || n = 4919 || (n > 4922 && n < 5001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 2529)
	; 6 bytes
	ldx #26 ;hides 'nop'
	pla
	inx
	Jne *-3
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
.elseif(n = 2605)
	; 5 bytes
	lax #10
	jsr delay_256a_x_33_clocks
.elseif(n = 2607 || n = 2864 || n = 3121 || n = 3378 || n = 3635 || n = 3892 || n = 4149 || n = 4406 || n = 4663 || n = 4920)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 2608 || n = 2865 || n = 3122 || n = 3379 || n = 3636 || n = 3893 || n = 4150 || n = 4407 || n = 4664 || n = 4921)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 2609 || n = 2866 || n = 3123 || n = 3380 || n = 3637 || n = 3894 || n = 4151 || n = 4408 || n = 4665 || n = 4922)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 2705)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	pla
	inx
	Jne *-3
.elseif(n = 2749)
	; 6 bytes
	ldx #250 ;hides 'nop'
	pla
	dex
	Jne *-3
.elseif(n = 2862)
	; 5 bytes
	lax #11
	jsr delay_256a_x_33_clocks
.elseif(n = 3119)
	; 5 bytes
	lax #12
	jsr delay_256a_x_33_clocks
.elseif(n = 3376)
	; 5 bytes
	lax #13
	jsr delay_256a_x_33_clocks
.elseif(n = 3633)
	; 5 bytes
	lax #14
	jsr delay_256a_x_33_clocks
.elseif(n = 3890)
	; 5 bytes
	lax #15
	jsr delay_256a_x_33_clocks
.elseif(n = 4147)
	; 5 bytes
	lax #16
	jsr delay_256a_x_33_clocks
.elseif(n = 4404)
	; 5 bytes
	lax #17
	jsr delay_256a_x_33_clocks
.elseif(n = 4661)
	; 5 bytes
	lax #18
	jsr delay_256a_x_33_clocks
.elseif(n = 4918)
	; 5 bytes
	lax #19
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
; 5001 to 10000
.if((n > 5000 && n < 5175) || n = 5176 || (n > 5179 && n < 5432) || n = 5433 || (n > 5436 && n < 5689) || n = 5690 || (n > 5693 && n < 5946) || n = 5947 || (n > 5950 && n < 6203) || n = 6204 || (n > 6207 && n < 6460) || n = 6461 || (n > 6464 && n < 6717) || n = 6718 || (n > 6721 && n < 6974) || n = 6975 || (n > 6978 && n < 7231) || n = 7232 || (n > 7235 && n < 7488) || n = 7489 || (n > 7492 && n < 7745) || n = 7746 || (n > 7749 && n < 8002) || n = 8003 || (n > 8006 && n < 8259) || n = 8260 || (n > 8263 && n < 8516) || n = 8517 || (n > 8520 && n < 8773) || n = 8774 || (n > 8777 && n < 9030) || n = 9031 || (n > 9034 && n < 9287) || n = 9288 || (n > 9291 && n < 9544) || n = 9545 || (n > 9548 && n < 9801) || n = 9802 || (n > 9805 && n < 10001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 5175)
	; 5 bytes
	lax #20
	jsr delay_256a_x_33_clocks
.elseif(n = 5177 || n = 5434 || n = 5691 || n = 5948 || n = 6205 || n = 6462 || n = 6719 || n = 6976 || n = 7233 || n = 7490 || n = 7747 || n = 8004 || n = 8261 || n = 8518 || n = 8775 || n = 9032 || n = 9289 || n = 9546 || n = 9803)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 5178 || n = 5435 || n = 5692 || n = 5949 || n = 6206 || n = 6463 || n = 6720 || n = 6977 || n = 7234 || n = 7491 || n = 7748 || n = 8005 || n = 8262 || n = 8519 || n = 8776 || n = 9033 || n = 9290 || n = 9547 || n = 9804)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 5179 || n = 5436 || n = 5693 || n = 5950 || n = 6207 || n = 6464 || n = 6721 || n = 6978 || n = 7235 || n = 7492 || n = 7749 || n = 8006 || n = 8263 || n = 8520 || n = 8777 || n = 9034 || n = 9291 || n = 9548 || n = 9805)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 5432)
	; 5 bytes
	lax #21
	jsr delay_256a_x_33_clocks
.elseif(n = 5689)
	; 5 bytes
	lax #22
	jsr delay_256a_x_33_clocks
.elseif(n = 5946)
	; 5 bytes
	lax #23
	jsr delay_256a_x_33_clocks
.elseif(n = 6203)
	; 5 bytes
	lax #24
	jsr delay_256a_x_33_clocks
.elseif(n = 6460)
	; 5 bytes
	lax #25
	jsr delay_256a_x_33_clocks
.elseif(n = 6717)
	; 5 bytes
	lax #26
	jsr delay_256a_x_33_clocks
.elseif(n = 6974)
	; 5 bytes
	lax #27
	jsr delay_256a_x_33_clocks
.elseif(n = 7231)
	; 5 bytes
	lax #28
	jsr delay_256a_x_33_clocks
.elseif(n = 7488)
	; 5 bytes
	lax #29
	jsr delay_256a_x_33_clocks
.elseif(n = 7745)
	; 5 bytes
	lax #30
	jsr delay_256a_x_33_clocks
.elseif(n = 8002)
	; 5 bytes
	lax #31
	jsr delay_256a_x_33_clocks
.elseif(n = 8259)
	; 5 bytes
	lax #32
	jsr delay_256a_x_33_clocks
.elseif(n = 8516)
	; 5 bytes
	lax #33
	jsr delay_256a_x_33_clocks
.elseif(n = 8773)
	; 5 bytes
	lax #34
	jsr delay_256a_x_33_clocks
.elseif(n = 9030)
	; 5 bytes
	lax #35
	jsr delay_256a_x_33_clocks
.elseif(n = 9287)
	; 5 bytes
	lax #36
	jsr delay_256a_x_33_clocks
.elseif(n = 9544)
	; 5 bytes
	lax #37
	jsr delay_256a_x_33_clocks
.elseif(n = 9801)
	; 5 bytes
	lax #38
	jsr delay_256a_x_33_clocks
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10058) || n = 10059 || (n > 10062 && n < 10315) || n = 10316 || (n > 10319 && n < 10572) || n = 10573 || (n > 10576 && n < 10829) || n = 10830 || (n > 10833 && n < 11086) || n = 11087 || (n > 11090 && n < 11343) || n = 11344 || (n > 11347 && n < 11600) || n = 11601 || (n > 11604 && n < 11857) || n = 11858 || (n > 11861 && n < 12114) || n = 12115 || (n > 12118 && n < 12371) || n = 12372 || (n > 12375 && n < 12628) || n = 12629 || (n > 12632 && n < 12885) || n = 12886 || (n > 12889 && n < 13142) || n = 13143 || (n > 13146 && n < 13399) || n = 13400 || (n > 13403 && n < 13656) || n = 13657 || (n > 13660 && n < 13913) || n = 13914 || (n > 13917 && n < 14170) || n = 14171 || (n > 14174 && n < 14427) || n = 14428 || (n > 14431 && n < 14684) || n = 14685 || (n > 14688 && n < 14941) || n = 14942 || (n > 14945 && n < 15001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 10058)
	; 5 bytes
	lax #39
	jsr delay_256a_x_33_clocks
.elseif(n = 10060 || n = 10317 || n = 10574 || n = 10831 || n = 11088 || n = 11345 || n = 11602 || n = 11859 || n = 12116 || n = 12373 || n = 12630 || n = 12887 || n = 13144 || n = 13401 || n = 13658 || n = 13915 || n = 14172 || n = 14429 || n = 14686 || n = 14943)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 10061 || n = 10318 || n = 10575 || n = 10832 || n = 11089 || n = 11346 || n = 11603 || n = 11860 || n = 12117 || n = 12374 || n = 12631 || n = 12888 || n = 13145 || n = 13402 || n = 13659 || n = 13916 || n = 14173 || n = 14430 || n = 14687 || n = 14944)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 10062 || n = 10319 || n = 10576 || n = 10833 || n = 11090 || n = 11347 || n = 11604 || n = 11861 || n = 12118 || n = 12375 || n = 12632 || n = 12889 || n = 13146 || n = 13403 || n = 13660 || n = 13917 || n = 14174 || n = 14431 || n = 14688 || n = 14945)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 10315)
	; 5 bytes
	lax #40
	jsr delay_256a_x_33_clocks
.elseif(n = 10572)
	; 5 bytes
	lax #41
	jsr delay_256a_x_33_clocks
.elseif(n = 10829)
	; 5 bytes
	lax #42
	jsr delay_256a_x_33_clocks
.elseif(n = 11086)
	; 5 bytes
	lax #43
	jsr delay_256a_x_33_clocks
.elseif(n = 11343)
	; 5 bytes
	lax #44
	jsr delay_256a_x_33_clocks
.elseif(n = 11600)
	; 5 bytes
	lax #45
	jsr delay_256a_x_33_clocks
.elseif(n = 11857)
	; 5 bytes
	lax #46
	jsr delay_256a_x_33_clocks
.elseif(n = 12114)
	; 5 bytes
	lax #47
	jsr delay_256a_x_33_clocks
.elseif(n = 12371)
	; 5 bytes
	lax #48
	jsr delay_256a_x_33_clocks
.elseif(n = 12628)
	; 5 bytes
	lax #49
	jsr delay_256a_x_33_clocks
.elseif(n = 12885)
	; 5 bytes
	lax #50
	jsr delay_256a_x_33_clocks
.elseif(n = 13142)
	; 5 bytes
	lax #51
	jsr delay_256a_x_33_clocks
.elseif(n = 13399)
	; 5 bytes
	lax #52
	jsr delay_256a_x_33_clocks
.elseif(n = 13656)
	; 5 bytes
	lax #53
	jsr delay_256a_x_33_clocks
.elseif(n = 13913)
	; 5 bytes
	lax #54
	jsr delay_256a_x_33_clocks
.elseif(n = 14170)
	; 5 bytes
	lax #55
	jsr delay_256a_x_33_clocks
.elseif(n = 14427)
	; 5 bytes
	lax #56
	jsr delay_256a_x_33_clocks
.elseif(n = 14684)
	; 5 bytes
	lax #57
	jsr delay_256a_x_33_clocks
.elseif(n = 14941)
	; 5 bytes
	lax #58
	jsr delay_256a_x_33_clocks
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15198) || n = 15199 || (n > 15202 && n < 15455) || n = 15456 || (n > 15459 && n < 15712) || n = 15713 || (n > 15716 && n < 15969) || n = 15970 || (n > 15973 && n < 16226) || n = 16227 || (n > 16230 && n < 16483) || n = 16484 || (n > 16487 && n < 16740) || n = 16741 || (n > 16744 && n < 16997) || n = 16998 || (n > 17001 && n < 17254) || n = 17255 || (n > 17258 && n < 17511) || n = 17512 || (n > 17515 && n < 17768) || n = 17769 || (n > 17772 && n < 18025) || n = 18026 || (n > 18029 && n < 18282) || n = 18283 || (n > 18286 && n < 18539) || n = 18540 || (n > 18543 && n < 18796) || n = 18797 || (n > 18800 && n < 19053) || n = 19054 || (n > 19057 && n < 19310) || n = 19311 || (n > 19314 && n < 19567) || n = 19568 || (n > 19571 && n < 19824) || n = 19825 || (n > 19828 && n < 20001))
	; 7 bytes
	lda #>(n-37)
	ldx #<(n-37)
	jsr delay_256a_x_33_clocks
.elseif(n = 15198)
	; 5 bytes
	lax #59
	jsr delay_256a_x_33_clocks
.elseif(n = 15200 || n = 15457 || n = 15714 || n = 15971 || n = 16228 || n = 16485 || n = 16742 || n = 16999 || n = 17256 || n = 17513 || n = 17770 || n = 18027 || n = 18284 || n = 18541 || n = 18798 || n = 19055 || n = 19312 || n = 19569 || n = 19826)
	; 6 bytes
	delay_n (n-2)
	nop
.elseif(n = 15201 || n = 15458 || n = 15715 || n = 15972 || n = 16229 || n = 16486 || n = 16743 || n = 17000 || n = 17257 || n = 17514 || n = 17771 || n = 18028 || n = 18285 || n = 18542 || n = 18799 || n = 19056 || n = 19313 || n = 19570 || n = 19827)
	; 6 bytes
	pha
	delay_n (n-3)
.elseif(n = 15202 || n = 15459 || n = 15716 || n = 15973 || n = 16230 || n = 16487 || n = 16744 || n = 17001 || n = 17258 || n = 17515 || n = 17772 || n = 18029 || n = 18286 || n = 18543 || n = 18800 || n = 19057 || n = 19314 || n = 19571 || n = 19828)
	; 6 bytes
	pla
	delay_n (n-4)
.elseif(n = 15455)
	; 5 bytes
	lax #60
	jsr delay_256a_x_33_clocks
.elseif(n = 15712)
	; 5 bytes
	lax #61
	jsr delay_256a_x_33_clocks
.elseif(n = 15969)
	; 5 bytes
	lax #62
	jsr delay_256a_x_33_clocks
.elseif(n = 16226)
	; 5 bytes
	lax #63
	jsr delay_256a_x_33_clocks
.elseif(n = 16483)
	; 5 bytes
	lax #64
	jsr delay_256a_x_33_clocks
.elseif(n = 16740)
	; 5 bytes
	lax #65
	jsr delay_256a_x_33_clocks
.elseif(n = 16997)
	; 5 bytes
	lax #66
	jsr delay_256a_x_33_clocks
.elseif(n = 17254)
	; 5 bytes
	lax #67
	jsr delay_256a_x_33_clocks
.elseif(n = 17511)
	; 5 bytes
	lax #68
	jsr delay_256a_x_33_clocks
.elseif(n = 17768)
	; 5 bytes
	lax #69
	jsr delay_256a_x_33_clocks
.elseif(n = 18025)
	; 5 bytes
	lax #70
	jsr delay_256a_x_33_clocks
.elseif(n = 18282)
	; 5 bytes
	lax #71
	jsr delay_256a_x_33_clocks
.elseif(n = 18539)
	; 5 bytes
	lax #72
	jsr delay_256a_x_33_clocks
.elseif(n = 18796)
	; 5 bytes
	lax #73
	jsr delay_256a_x_33_clocks
.elseif(n = 19053)
	; 5 bytes
	lax #74
	jsr delay_256a_x_33_clocks
.elseif(n = 19310)
	; 5 bytes
	lax #75
	jsr delay_256a_x_33_clocks
.elseif(n = 19567)
	; 5 bytes
	lax #76
	jsr delay_256a_x_33_clocks
.elseif(n = 19824)
	; 5 bytes
	lax #77
	jsr delay_256a_x_33_clocks
.endif
.endif
.endif

.endif
	rest
.endmacro
