; delay_n, delays n cycles with small code
; Will clobber A, X, Z+N, C, and V
; Will preserve Y, S, D, and I (interrupt-disable flag)
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
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=4, avg=5.2, max=7
; - When 31 <= n <= 50: min=4, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=4, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.2, max=7
; - When 301 <= n <= 500: min=4, avg=6.2, max=7
; - When 501 <= n <= 750: min=5, avg=6.2, max=7
; - When 751 <= n <= 1000: min=5, avg=6.3, max=7
; - When 1001 <= n <= 2000: min=4, avg=6.7, max=7
; - When 2001 <= n <= 3000: min=6, avg=7.0, max=7
; - When 3001 <= n <= 5000: min=7, avg=7.0, max=7
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
.if((n >= 1) && (n < 79))
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif(n = 4 || n = 6 || (n > 15 && n < 19))
	; 2..5 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 3 bytes
	clc
	Jcc *+2
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || n = 22)
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
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
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
.elseif(n = 23 || n = 26)
	; 5..6 bytes
	clc
	delay_n (n-2)
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n > 26 && n < 31) || n = 32 || n = 33 || n = 35 || n = 37 || n = 38 || n = 40 || (n > 41 && n < 51) || (n > 51 && n < 55) || (n > 55 && n < 79))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 31)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
.elseif(n = 34)
	; 5 bytes
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
.elseif(n = 36)
	; 5 bytes
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.elseif(n = 41)
	; 5 bytes
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 51)
	; 5 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
.elseif(n = 55)
	; 5 bytes
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
.endif
.else
; 79 to 156
.if((n > 78 && n < 111) || (n > 111 && n < 116) || (n > 116 && n < 124) || (n > 124 && n < 156))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 111)
	; 5 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
.elseif(n = 116)
	; 5 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
.elseif(n = 124)
	; 4 bytes
	ldx #232 ;hides 'inx'
	Jne *-1
.elseif(n = 156)
	; 5 bytes
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
.endif
.endif
.else
; 157 to 312
.if((n > 156 && n < 283))
	; 5 bytes
	lda #n-27
	jsr delay_a_25_clocks
.elseif(n = 283 || n = 284 || n = 297 || n = 298 || n = 303 || n = 308)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 285 || n = 287 || n = 289)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 286 || n = 291 || n = 296 || n = 306 || n = 311)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
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
.elseif(n = 294)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
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
.if(n = 313 || n = 318 || n = 328 || n = 333 || n = 338 || n = 343 || n = 348 || n = 353 || n = 363 || n = 367 || n = 368 || n = 373 || n = 378 || n = 388 || n = 403 || n = 408 || n = 413 || n = 418 || n = 423 || n = 433 || n = 437 || n = 438 || n = 443 || n = 448 || n = 453 || n = 458 || n = 468)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 314 || n = 319 || n = 324 || n = 329 || n = 334 || n = 349 || n = 354 || n = 359 || n = 364 || n = 369 || n = 374 || n = 382 || n = 384 || n = 389 || n = 394 || n = 399 || n = 404 || n = 409 || n = 419 || n = 424 || n = 429 || n = 434 || n = 439 || n = 444 || n = 454 || n = 459)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 385 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && (n = 316 || n = 331 || (n > 340 && n < 352) || (n > 360 && n < 402) || n = 411 || n = 426 || n = 441 || (n > 450 && n < 462)))
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 317 || n = 322 || n = 327 || n = 342 || n = 347 || n = 357 || n = 362 || n = 387 || n = 392 || n = 412 || n = 422 || n = 427 || n = 432 || n = 447 || n = 452 || n = 462)
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
.elseif(n = 325 || n = 335 || n = 340 || n = 355 || n = 360 || n = 370 || n = 375 || n = 380 || n = 390 || n = 405 || n = 420 || n = 430 || n = 440 || n = 445 || n = 450 || n = 465)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
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
.elseif(n = 339)
	; 7 bytes
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
.elseif(n = 352 || n = 397)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 377)
	; 5 bytes
	ldx #74 ;hides 'lsr a'
	inx
	Jpl *-2
.elseif(n = 379)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jmi *-1
.elseif(n = 383)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
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
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 417)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
.elseif(n = 457)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 479 || n = 489 || n = 499 || n = 504 || n = 509 || n = 514 || n = 527 || n = 529 || n = 534 || n = 539)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 473 || n = 478 || n = 482 || n = 483 || n = 488 || n = 493 || n = 508 || n = 513 || n = 518 || n = 523 || n = 528 || n = 538 || n = 542 || n = 543)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 475 || n = 490 || n = 500 || n = 520)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
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
.elseif(n = 485)
	; 6 bytes
	ldx #234 ;hides 'nop'
	dex
	dex
	Jmi *-3
.elseif(n = 487)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 492 || n = 502 || n = 522)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 494)
	; 6 bytes
	ldx #53 ;hides 'and zp,x'
	inx ;first loop only
	dex
	Jpl *-3
.elseif(n = 495 || n = 510 || n = 515 || n = 525 || n = 535 || n = 545)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 497)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 503)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jpl *-2
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
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
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 537)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
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
.elseif(n = 548 || n = 553 || n = 558 || n = 562 || n = 563 || n = 573 || n = 578 || n = 583 || n = 588 || n = 593 || n = 598 || n = 608 || n = 612 || n = 613 || n = 618 || n = 623)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && (n = 549 || (n > 558 && n < 585) || (n > 593 && n < 620)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 550 || n = 555 || n = 565 || n = 590 || n = 600 || n = 605)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 551 || n = 566 || n = 586 || n = 591 || n = 601 || n = 621)
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 552)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 556 || n = 561 || n = 571 || n = 576 || n = 581 || n = 596 || n = 606 || n = 611 || n = 616)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif(n = 557 || n = 567 || n = 572 || n = 597 || n = 602 || n = 607)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 560)
	; 6 bytes
	ldx #197 ;hides 'cmp zp'
	nop
	dex
	Jmi *-3
.elseif(n = 568 || n = 582 || n = 589 || n = 603 || n = 610 || n = 624)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 570 || n = 580 || n = 585 || n = 595 || n = 615 || n = 625)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 577)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 587)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 592)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	dex
	Jpl *-4
.elseif(n = 620)
	; 6 bytes
	ldx #138 ;hides 'txa'
	dex
	dex
	Jne *-3
.elseif(n = 622)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.endif
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
.elseif(n = 627 || n = 632 || n = 642 || n = 662 || n = 672 || n = 677 || n = 692 || n = 702 || n = 707 || n = 732 || n = 742 || n = 747 || n = 762 || n = 767 || n = 772)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 628 || n = 633 || n = 643 || n = 648 || n = 653 || n = 658 || n = 663 || n = 668 || n = 678 || n = 682 || n = 683 || n = 693 || n = 698 || n = 703 || n = 713 || n = 717 || n = 718 || n = 723 || n = 728 || n = 733 || n = 738 || n = 753 || n = 758 || n = 763 || n = 768 || n = 773)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 629)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jmi *-2
	plp
.elseif(n = 630 || n = 640 || n = 650)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 631 || n = 646)
	; 5 bytes
	ldx #(n-6)/5
	dex
	Jpl *-1
.elseif((n .mod 5 = 4) && ((n > 633 && n < 655) || n = 664 || n = 669 || (n > 678 && n < 690) || (n > 698 && n < 725) || n = 739 || (n > 748 && n < 760) || (n > 768 && n < 780)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 0) && (n = 635 || n = 655 || n = 660 || n = 670 || n = 675 || (n > 684 && n < 711) || (n > 724 && n < 746) || (n > 759 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 637)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif((n .mod 7 = 1) && (n = 638 || n = 645 || (n > 672 && n < 695) || (n > 707 && n < 723) || (n > 742 && n < 765) || n = 778))
	; 6 bytes
	ldx #(n-8)/7
	nop
	dex
	Jpl *-2
.elseif(n = 647)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jpl *-3
.elseif(n = 652 || n = 659 || n = 729)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 657 || n = 737 || n = 777)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 665)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 667 || n = 712)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
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
.elseif(n = 697)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 734)
	; 7 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
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
.elseif(n = 783 || n = 787 || n = 788 || n = 793 || n = 798 || n = 803 || n = 808 || n = 818 || n = 822 || n = 823 || n = 828 || n = 833 || n = 838 || n = 843 || n = 853 || n = 857 || n = 858 || n = 863 || n = 868 || n = 873 || n = 878 || n = 888 || n = 893 || n = 898 || n = 903 || n = 908 || n = 913 || n = 923 || n = 927 || n = 928 || n = 933)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || (n > 803 && n < 820) || n = 829 || (n > 838 && n < 865) || (n > 873 && n < 895) || (n > 908 && n < 935)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif((n .mod 5 = 0) && (n = 790 || n = 795 || (n > 804 && n < 816) || (n > 829 && n < 851) || (n > 859 && n < 886) || (n > 894 && n < 921) || n = 930))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 792 || n = 820 || n = 827 || n = 869 || n = 890 || n = 897 || n = 918 || n = 925 || n = 932)
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 797)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	dex
	Jpl *-4
.elseif(n = 800)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	dex
	Jmi *-3
.elseif(n = 802 || n = 847 || n = 937)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif((n .mod 5 = 2) && (n = 807 || n = 812 || n = 837 || n = 842 || n = 852 || n = 867 || n = 872 || n = 882 || n = 887 || (n > 901 && n < 923)))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 817 || n = 877)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 832)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 892)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 899)
	; 6 bytes
	ldx #56 ;hides 'sec'
	inx
	inx
	Jne *-3
.elseif(n = 935)
	; 6 bytes
	ldx #24 ;hides 'clc'
	nop
	inx
	Jpl *-3
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 943 || n = 948 || n = 958 || n = 963 || n = 968 || n = 973 || n = 978 || n = 983 || n = 993 || n = 997 || n = 998 || n = 1003 || n = 1008 || n = 1013 || n = 1028 || n = 1032 || n = 1033 || n = 1038 || n = 1043 || n = 1048 || n = 1053 || n = 1067 || n = 1068 || n = 1073 || n = 1078 || n = 1083 || n = 1088)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 939 || (n > 952 && n < 975) || (n > 987 && n < 1010) || (n > 1022 && n < 1045) || (n > 1057 && n < 1080) || n = 1093))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && ((n > 939 && n < 956) || (n > 969 && n < 991) || (n > 999 && n < 1026) || (n > 1034 && n < 1061) || (n > 1074 && n < 1091)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 940 && n < 1092)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif((n .mod 5 = 2) && ((n > 941 && n < 958) || n = 987 || n = 1012 || n = 1022 || n = 1042 || n = 1047 || n = 1062 || (n > 1076 && n < 1093)))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 964 || n = 969 || n = 979 || n = 989 || n = 994 || n = 999 || n = 1004 || n = 1017 || n = 1019 || n = 1024 || n = 1029 || n = 1034 || n = 1039 || n = 1054 || n = 1059 || n = 1064 || n = 1069 || n = 1074 || n = 1084 || n = 1089)
	; 6..7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 972)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 977)
	; 7 bytes
	ldx #(n-9)/8
	cmp $C5
	dex
	Jpl *-3
.elseif(n = 982 || n = 1027)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
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
.elseif(n = 1007)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jpl *-4
.elseif(n = 1014)
	; 4 bytes
	ldx #202 ;hides 'dex'
	Jne *-1
.elseif(n = 1018)
	; 6 bytes
	txa
	delay_n (n-4)
	tax
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
.elseif(n = 1057)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && ((n > 1093 && n < 1110) || (n > 1118 && n < 1145) || (n > 1153 && n < 1180) || (n > 1188 && n < 1215) || (n > 1223 && n < 1250)))
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif((n .mod 5 = 0) && (n = 1095 || (n > 1104 && n < 1131) || (n > 1139 && n < 1166) || (n > 1174 && n < 1201) || (n > 1209 && n < 1236) || n = 1245))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 1095 && n < 1247)))
	; 5 bytes
	ldx #(n-1)/5
	dex
	Jne *-1
.elseif(n = 1097 || n = 1217)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1098 || n = 1102 || n = 1103 || n = 1108 || n = 1113 || n = 1118 || n = 1123 || n = 1133 || n = 1137 || n = 1138 || n = 1143 || n = 1148 || n = 1153 || n = 1158 || n = 1168 || n = 1172 || n = 1173 || n = 1178 || n = 1183 || n = 1188 || n = 1193 || n = 1203 || n = 1208 || n = 1213 || n = 1218 || n = 1223 || n = 1228 || n = 1238 || n = 1242 || n = 1243 || n = 1248)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1099 && n < 1115) || (n > 1127 && n < 1150) || (n > 1162 && n < 1185) || (n > 1197 && n < 1220) || (n > 1232 && n < 1248)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 2) && (n = 1112 || (n > 1121 && n < 1133) || (n > 1146 && n < 1158) || n = 1167 || n = 1182 || (n > 1191 && n < 1203) || n = 1222 || n = 1227))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1117 || n = 1207)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1162)
	; 7 bytes
	ldx #(n-10)/9
	delay_n 4
	dex
	Jpl *-3
.elseif(n = 1187)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jpl *-4
.elseif(n = 1232)
	; 6 bytes
	ldx #101 ;hides 'adc zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1237)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 1252 || n = 1297 || n = 1306 || n = 1315)
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1253 || n = 1258 || n = 1263 || n = 1277 || n = 1278 || n = 1283 || n = 1298 || n = 1305 || n = 1314 || n = 1319 || n = 1322 || n = 1326)
	; 6..7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && (n = 1254 || (n > 1267 && n < 1325)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1271) || n = 1280))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 1257 || n = 1313)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1259)
	; 6 bytes
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1262 || n = 1267 || n = 1272 || n = 1292 || n = 1293 || n = 1295 || (n > 1298 && n < 1303) || n = 1304 || n = 1307 || n = 1311 || n = 1316 || n = 1318 || n = 1323 || n = 1325 || n = 1327)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1264 || n = 1269 || n = 1274 || n = 1279 || n = 1284 || n = 1290)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
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
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.elseif(n = 1286)
	; 7 bytes
	ldx #234 ;hides 'nop'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 1287)
	; 5 bytes
	ldx #184 ;hides 'clv'
	dex
	Jne *-2
.elseif(n = 1288 || n = 1294)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1291)
	; 7 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
.elseif(n = 1308)
	; 7 bytes
	ldx #245 ;hides 'sbc zp,x'
	inx ;first loop only
	nop
	dex
	Jmi *-4
.elseif(n = 1309)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
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
.elseif(n = 1321)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1328)
	; 6 bytes
	ldx #165 ;hides 'lda zp'
	inx ;first loop only
	dex
	Jne *-3
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1337 || n = 1353 || n = 1377 || n = 1385)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1330 || n = 1340 || n = 1347 || n = 1354 || n = 1361 || n = 1368 || n = 1375 || n = 1382 || n = 1389)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 1) && ((n > 1330 && n < 1402)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1332 || (n > 1333 && n < 1337) || n = 1339 || n = 1343 || n = 1344 || n = 1346 || n = 1348 || n = 1350 || n = 1355 || n = 1356 || n = 1358 || (n > 1361 && n < 1366) || (n > 1369 && n < 1373) || n = 1374 || n = 1383 || n = 1384 || n = 1386 || n = 1388 || (n > 1389 && n < 1393) || n = 1395 || n = 1397 || n = 1398 || n = 1400 || n = 1404)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 9 = 1) && ((n > 1332 && n < 1379) || n = 1396 || n = 1405))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1341)
	; 7 bytes
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	dex
	dex
	Jne *-4
.elseif(n = 1349)
	; 6 bytes
	ldx #106 ;hides 'ror a'
	nop
	inx
	Jne *-3
.elseif(n = 1357)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
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
	; 7 bytes
	ldx #138 ;hides 'txa'
	lda $A5
	dex
	Jne *-4
.elseif(n = 1381 || n = 1393)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1399)
	; 5 bytes
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
.elseif(n = 1402)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.elseif(n = 1406)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || (n > 1410 && n < 1414) || n = 1416 || n = 1418 || n = 1419 || n = 1421 || n = 1426 || n = 1428 || n = 1430 || n = 1434 || n = 1435 || n = 1437 || n = 1439 || n = 1440 || n = 1442 || n = 1444 || (n > 1445 && n < 1449) || n = 1451 || n = 1454 || n = 1456 || n = 1458 || (n > 1459 && n < 1464) || n = 1467 || n = 1469 || n = 1470 || n = 1472 || (n > 1473 && n < 1477) || n = 1479 || n = 1482 || n = 1484)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1479)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif(n = 1409 || n = 1425 || n = 1433 || n = 1449 || n = 1481)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif((n .mod 7 = 3) && (n = 1410 || n = 1417 || (n > 1430 && n < 1453) || (n > 1465 && n < 1481)))
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1442) || (n > 1458 && n < 1478)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1420)
	; 5 bytes
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
.elseif(n = 1424)
	; 7 bytes
	txa
	delay_n (n-4)
	tax
.elseif(n = 1427)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1453 || n = 1465)
	; 7 bytes
	ldx #(n-13)/12
	delay_n 7
	dex
	Jpl *-3
.elseif(n = 1455)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jpl *-4
.elseif(n = 1483)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	pla
	pha
	dex
	Jne *-4
.endif
.else
; 1485 to 1562
.if((n .mod 7 = 1) && ((n > 1484 && n < 1563)))
	; 6 bytes
	ldx #(n-1)/7
	nop
	dex
	Jne *-2
.elseif((n .mod 9 = 1) && (n = 1486 || n = 1495 || (n > 1521 && n < 1559)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1487 || n = 1490 || n = 1494 || n = 1498 || n = 1501 || n = 1508 || n = 1515 || n = 1536 || n = 1543 || n = 1550 || n = 1557)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 1489 || n = 1525 || n = 1537 || n = 1561)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1491 || n = 1493 || n = 1502 || n = 1503 || n = 1507 || (n > 1508 && n < 1513) || n = 1514 || n = 1516 || n = 1523 || n = 1524 || n = 1526 || n = 1530 || n = 1532 || n = 1533 || n = 1535 || n = 1542 || n = 1544 || n = 1546 || n = 1551 || n = 1552 || n = 1554 || n = 1556 || n = 1559)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
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
.elseif(n = 1500)
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1504)
	; 6 bytes
	ldx #69 ;hides 'eor zp'
	dex ;first loop only
	inx
	Jne *-3
.elseif(n = 1505 || n = 1521 || n = 1545 || n = 1553)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1517)
	; 7 bytes
	ldx #138 ;hides 'txa'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1518)
	; 7 bytes
	php
	ldx #216 ;hides 'cld'
	dex
	Jne *-2
	plp
.elseif(n = 1519)
	; 7 bytes
	ldx #152 ;hides 'tya'
	lda $A5
	dex
	Jne *-4
.elseif(n = 1528)
	; 7 bytes
	ldx #117 ;hides 'adc zp,x'
	delay_n 4
	inx
	Jne *-4
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
.elseif(n = 1539)
	; 7 bytes
	ldx #117 ;hides 'adc zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
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
.endif
.else
.if((n >= 1563) && (n < 1719))
; 1563 to 1718
.if(n = 1563 || n = 1565 || n = 1566 || n = 1572 || n = 1574 || n = 1575 || (n > 1578 && n < 1583) || (n > 1586 && n < 1590) || n = 1591 || n = 1595 || n = 1596 || n = 1598 || n = 1600 || n = 1602 || n = 1605 || n = 1607 || n = 1608 || (n > 1613 && n < 1617) || n = 1622 || n = 1624 || n = 1629 || n = 1631 || n = 1635 || n = 1636 || n = 1638 || n = 1642 || n = 1643 || n = 1652 || n = 1654 || n = 1656 || n = 1658 || n = 1659 || n = 1661 || n = 1663 || n = 1664 || n = 1668 || n = 1670 || n = 1672 || (n > 1676 && n < 1681) || (n > 1684 && n < 1688) || n = 1691 || n = 1692 || n = 1694 || n = 1696 || (n > 1697 && n < 1702) || n = 1703 || (n > 1705 && n < 1709) || n = 1710 || n = 1712 || n = 1714 || n = 1715)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1564 || n = 1570 || n = 1571 || n = 1578 || n = 1586 || n = 1592 || n = 1599 || n = 1606 || n = 1613 || n = 1620 || n = 1634 || n = 1662 || n = 1669 || n = 1676 || n = 1683 || n = 1690 || n = 1697 || n = 1704 || n = 1718)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && (n = 1567 || (n > 1584 && n < 1622) || (n > 1647 && n < 1694) || n = 1711))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
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
.elseif(n = 1573 || n = 1609 || n = 1633 || n = 1645 || n = 1705 || n = 1717)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1577 || n = 1593 || n = 1601 || n = 1617 || n = 1649 || n = 1665 || n = 1673 || n = 1689 || n = 1713)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
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
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1628)
	; 6 bytes
	ldx #181 ;hides 'lda zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 1630 || n = 1644)
	; 7 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1637)
	; 5 bytes
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
.elseif(n = 1641)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1647)
	; 7 bytes
	ldx #73 ;hides 'eor #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1650)
	; 7 bytes
	ldx #165 ;hides 'lda zp'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1651)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 1655)
	; 6 bytes
	ldx #184 ;hides 'clv'
	nop
	dex
	Jne *-3
.elseif(n = 1671)
	; 7 bytes
	ldx #152 ;hides 'tya'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1682)
	; 7 bytes
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1722 || n = 1726 || n = 1727 || n = 1731 || (n > 1732 && n < 1737) || n = 1740 || n = 1743 || (n > 1747 && n < 1751) || n = 1755 || n = 1757 || n = 1759 || n = 1763 || n = 1764 || n = 1766 || n = 1771 || n = 1773 || n = 1775 || n = 1776 || n = 1778 || n = 1780 || n = 1782 || n = 1784 || n = 1787 || n = 1790 || n = 1791 || n = 1794 || n = 1796)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 9 = 1) && ((n > 1719 && n < 1757) || (n > 1773 && n < 1793)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
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
	; 7 bytes
	delay_n (n-3)
	ldx $A6
.elseif(n = 1725 || n = 1732 || n = 1739 || n = 1746 || n = 1753 || n = 1754 || n = 1762 || n = 1767 || n = 1770 || n = 1781 || n = 1788 || n = 1795)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1728)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1741 || n = 1777 || n = 1789)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 1742)
	; 7 bytes
	php
	ldx #248 ;hides 'sed'
	dex
	Jne *-2
	plp
.elseif(n = 1745 || n = 1761 || n = 1769 || n = 1785)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
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
.endif
.else
; 1797 to 1875
.if(n = 1797 || n = 1798 || n = 1800 || (n > 1801 && n < 1809) || n = 1811 || n = 1812 || n = 1814 || n = 1815 || (n > 1819 && n < 1824) || n = 1827 || (n > 1828 && n < 1832) || n = 1836 || n = 1838 || (n > 1842 && n < 1846) || n = 1847 || n = 1848 || (n > 1849 && n < 1855) || n = 1856 || n = 1858 || n = 1859 || n = 1862 || n = 1863 || (n > 1865 && n < 1869) || n = 1871 || n = 1872 || n = 1874 || n = 1875)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1799)
	; 6 bytes
	ldx #56 ;hides 'sec'
	nop
	inx
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1800 && n < 1874)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1809 || n = 1833 || n = 1841 || n = 1857 || n = 1865)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif(n = 1813 || n = 1825 || n = 1849 || n = 1861)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
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
.elseif(n = 1818 || n = 1834 || n = 1842)
	; 7 bytes
	delay_n (n-2)
	nop
.elseif(n = 1824)
	; 6 bytes
	ldx #228 ;hides 'cpx zp'
	nop
	dex
	Jne *-3
.elseif(n = 1826)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 4
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
	; 7 bytes
	ldx #184 ;hides 'clv'
	bit $24
	dex
	Jne *-4
.elseif(n = 1840)
	; 6 bytes
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	dex
	Jne *-3
.elseif(n = 1860)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 1869)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1870)
	; 7 bytes
	ldx #69 ;hides 'eor zp'
	delay_n 4
	inx
	Jne *-4
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if((n > 1875 && n < 1879) || n = 1883 || n = 1884 || (n > 1885 && n < 1889) || n = 1890 || (n > 1891 && n < 1897) || n = 1898 || n = 1899 || (n > 1900 && n < 1905) || n = 1906 || n = 1908 || (n > 1909 && n < 1913) || n = 1914 || n = 1915 || n = 1917 || n = 1919 || n = 1920 || (n > 1921 && n < 1925) || n = 1926 || n = 1928 || n = 1930 || n = 1932 || n = 1934 || (n > 1937 && n < 1945) || (n > 1945 && n < 1950) || n = 1951 || n = 1952 || n = 1955 || n = 1956 || n = 1958 || n = 1962 || (n > 1963 && n < 1968) || n = 1971 || (n > 1972 && n < 1977) || n = 1978 || (n > 1981 && n < 1985) || (n > 1985 && n < 1989) || n = 1991 || n = 1992 || (n > 1993 && n < 1999) || n = 2003 || n = 2004 || n = 2006 || n = 2007 || (n > 2010 && n < 2016) || (n > 2018 && n < 2023) || n = 2024 || n = 2027 || n = 2028 || n = 2030 || n = 2031)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 1879)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1880)
	; 7 bytes
	ldx #85 ;hides 'eor zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 1881 || n = 1889 || n = 1905 || n = 1913 || n = 1929 || n = 1937 || n = 1953 || n = 1961 || n = 1977 || n = 1985 || n = 2001 || n = 2009 || n = 2025)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 2000) || n = 2017 || n = 2026))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 1885 || n = 1897 || n = 1921 || n = 1933 || n = 1957 || n = 1969 || n = 1993 || n = 2005 || n = 2029)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
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
.elseif(n = 1931)
	; 7 bytes
	ldx #138 ;hides 'txa'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 1935)
	; 7 bytes
	ldx #41 ;hides 'and #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 1950)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1959)
	; 7 bytes
	ldx #196 ;hides 'cpy zp'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 1960)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1968)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
.elseif(n = 1970)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 1979)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1980)
	; 7 bytes
	ldx #197 ;hides 'cmp zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 1989)
	; 7 bytes
	ldx #181 ;hides 'lda zp,x'
	lda $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2000)
	; 6 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	inx
	Jne *-3
.elseif(n = 2002 || n = 2010 || n = 2018)
	; 7 bytes
	delay_n (n-2)
	nop
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
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 4
	dex
	Jne *-4
.endif
.else
; 2032 to 2187
.if(n = 2032 || n = 2034 || (n > 2035 && n < 2041) || n = 2042 || n = 2043 || (n > 2044 && n < 2049) || (n > 2049 && n < 2053) || (n > 2053 && n < 2062) || n = 2063 || n = 2064 || (n > 2065 && n < 2071) || (n > 2071 && n < 2077) || n = 2079 || (n > 2080 && n < 2087) || n = 2088 || (n > 2089 && n < 2097) || n = 2100 || (n > 2101 && n < 2105) || n = 2106 || (n > 2107 && n < 2113) || n = 2115 || (n > 2116 && n < 2123) || n = 2124 || n = 2126 || (n > 2127 && n < 2134) || n = 2135 || n = 2136 || n = 2138 || (n > 2139 && n < 2143) || (n > 2143 && n < 2149) || n = 2150 || n = 2151 || (n > 2152 && n < 2161) || (n > 2161 && n < 2170) || n = 2171 || n = 2172 || (n > 2173 && n < 2179) || (n > 2180 && n < 2185) || n = 2186 || n = 2187)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 2033)
	; 7 bytes
	ldx #(n-1)/8
	cmp $C5
	dex
	Jne *-3
.elseif((n .mod 9 = 1) && ((n > 2034 && n < 2180)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2041 || n = 2065 || n = 2077 || n = 2101 || n = 2113 || n = 2137 || n = 2149 || n = 2173 || n = 2185)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2049)
	; 7 bytes
	ldx #0
	cmp $C5
	dex
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
.elseif(n = 2097)
	; 7 bytes
	ldx #233 ;hides 'sbc #imm'
	lda $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2099)
	; 7 bytes
	ldx #106 ;hides 'ror a'
	pla
	pha
	inx
	Jne *-4
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
.elseif(n = 2127)
	; 7 bytes
	ldx #152 ;hides 'tya'
	pla
	pha
	dex
	Jne *-4
.elseif(n = 2139)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	lda $A5
	inx
	Jne *-4
.elseif(n = 2180)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n .mod 9 = 1) && ((n > 2187 && n < 2261)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2189)
	; 7 bytes
	ldx #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2190)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.elseif((n > 2190 && n < 2195) || n = 2196 || n = 2198 || (n > 2200 && n < 2204) || n = 2205 || n = 2207 || n = 2208 || n = 2211 || n = 2212 || n = 2214 || (n > 2215 && n < 2221) || n = 2222 || (n > 2224 && n < 2231) || (n > 2233 && n < 2240) || n = 2244 || (n > 2245 && n < 2249) || (n > 2251 && n < 2257) || (n > 2260 && n < 2266))
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
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2200)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2204)
	; 6 bytes
	ldx #245 ;hides 'sbc zp,x'
	nop
	dex
	Jne *-3
.elseif(n = 2209 || n = 2221 || n = 2245 || n = 2257)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2210)
	; 7 bytes
	ldx #36 ;hides 'bit zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2213)
	; 6 bytes
	ldx #10 ;hides 'asl a'
	nop
	inx
	Jne *-3
.elseif(n = 2223)
	; 7 bytes
	ldx #9 ;hides 'ora #imm'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2231)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2232)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
.elseif(n = 2240)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2241)
	; 7 bytes
	cmp <$CD ;hides 'cmp $07A2'
	ldx #7
	inx
	Jne *-4
.elseif(n = 2243)
	; 7 bytes
	ldx #53 ;hides 'and zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2249)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2250)
	; 7 bytes
	cmp <$CD ;hides 'cmp $06A2'
	ldx #6
	inx
	Jne *-4
.elseif(n = 2258)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $05A2'
	ldx #5
	inx
	Jne *-4
.elseif(n = 2259)
	; 7 bytes
	cmp <$CD ;hides 'cmp $05A2'
	ldx #5
	inx
	Jne *-4
.endif
.else
; 2266 to 2343
.if(n = 2266 || (n > 2270 && n < 2276) || (n > 2281 && n < 2285) || n = 2288 || n = 2291 || n = 2292 || (n > 2296 && n < 2300) || n = 2301 || n = 2302 || (n > 2305 && n < 2317) || n = 2318 || (n > 2319 && n < 2329) || n = 2330 || (n > 2331 && n < 2339) || n = 2340 || n = 2343)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif(n = 2267)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $04A2'
	ldx #4
	inx
	Jne *-4
.elseif(n = 2268)
	; 7 bytes
	cmp <$CD ;hides 'cmp $04A2'
	ldx #4
	inx
	Jne *-4
.elseif((n .mod 9 = 1) && ((n > 2268 && n < 2297)))
	; 7 bytes
	ldx #(n-1)/9
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2270)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2276)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2277)
	; 7 bytes
	cmp <$CD ;hides 'cmp $03A2'
	ldx #3
	inx
	Jne *-4
.elseif(n = 2279)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2280)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2281 || n = 2293 || n = 2317 || n = 2329 || n = 2341)
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2285)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2286)
	; 7 bytes
	cmp <$CD ;hides 'cmp $02A2'
	ldx #2
	inx
	Jne *-4
.elseif(n = 2289)
	; 7 bytes
	ldx #229 ;hides 'sbc zp'
	bit $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2290)
	; 7 bytes
	ldx #228 ;hides 'cpx zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2294)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2295)
	; 7 bytes
	cmp <$CD ;hides 'cmp $01A2'
	ldx #1
	inx
	Jne *-4
.elseif(n = 2300)
	; 7 bytes
	ldx #229 ;hides 'sbc zp'
	inx ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2303)
	; 7 bytes
	cmp #<$CD ;hides 'cmp $00A2'
	ldx #0
	inx
	Jne *-4
.elseif(n = 2304)
	; 7 bytes
	cmp <$CD ;hides 'cmp $00A2'
	ldx #0
	inx
	Jne *-4
.elseif(n = 2305)
	; 7 bytes
	ldx #0
	delay_n 4
	dex
	Jne *-3
.elseif(n = 2319)
	; 7 bytes
	ldx #24 ;hides 'clc'
	cmp $C5
	inx
	Jne *-4
.elseif(n = 2331)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2339)
	; 7 bytes
	ldx #234 ;hides 'nop'
	cmp $C5
	dex
	Jne *-4
.elseif(n = 2342)
	; 7 bytes
	ldx #213 ;hides 'cmp zp,x'
	delay_n 4
	dex
	Jne *-4
.endif
.endif
.else
; 2344 to 2500
.if((n > 2343 && n < 2353) || (n > 2353 && n < 2365) || (n > 2365 && n < 2377) || (n > 2377 && n < 2389) || (n > 2389 && n < 2401) || (n > 2401 && n < 2413) || (n > 2413 && n < 2425) || (n > 2425 && n < 2435) || n = 2436 || (n > 2437 && n < 2449) || (n > 2449 && n < 2459) || n = 2460 || (n > 2461 && n < 2473) || (n > 2473 && n < 2485) || (n > 2485 && n < 2497) || n = 2498 || n = 2499)
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 12 = 1) && ((n > 2352 && n < 2498)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2435)
	; 6 bytes
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
.elseif(n = 2459)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	lda $A5
	inx
	Jne *-4
.elseif(n = 2500)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	inx ;first loop only
	nop
	inx
	Jne *-4
.endif
.endif
.endif
.endif
.endif
.else
; 2501 to 5000
.if((n > 2500 && n < 2509) || (n > 2510 && n < 2520) || (n > 2521 && n < 2533) || (n > 2533 && n < 2540) || (n > 2540 && n < 2545) || n = 2546 || (n > 2547 && n < 2551) || (n > 2551 && n < 2557) || (n > 2557 && n < 2569) || (n > 2569 && n < 2573) || n = 2574 || (n > 2575 && n < 2581) || n = 2582 || (n > 2584 && n < 2593) || n = 2594 || (n > 2595 && n < 2605) || (n > 2605 && n < 2617) || (n > 2617 && n < 2629) || (n > 2629 && n < 2641) || (n > 2641 && n < 2653) || (n > 2653 && n < 2665) || (n > 2665 && n < 2677) || (n > 2677 && n < 2683) || (n > 2683 && n < 2689) || (n > 2689 && n < 2693) || (n > 2694 && n < 2701) || (n > 2701 && n < 2705) || (n > 2705 && n < 2713) || (n > 2713 && n < 2725) || (n > 2725 && n < 2737) || (n > 2737 && n < 2749) || (n > 2749 && n < 2761) || (n > 2761 && n < 2773) || (n > 2773 && n < 2785) || (n > 2785 && n < 2797) || n = 2798 || (n > 2799 && n < 2809) || (n > 2809 && n < 2821) || (n > 2821 && n < 2833) || (n > 2833 && n < 2841) || (n > 2841 && n < 2845) || (n > 2845 && n < 2857) || (n > 2857 && n < 2869) || (n > 2869 && n < 2881) || (n > 2881 && n < 2893) || (n > 2893 && n < 2905) || (n > 2905 && n < 2917) || (n > 2917 && n < 2929) || (n > 2929 && n < 2941) || (n > 2941 && n < 2953) || (n > 2953 && n < 2965) || (n > 2965 && n < 2977) || (n > 2977 && n < 2989) || (n > 2989 && n < 2995) || (n > 2995 && n < 3001) || (n > 3001 && n < 3013) || (n > 3013 && n < 3025) || (n > 3025 && n < 3037) || (n > 3037 && n < 3049) || (n > 3049 && n < 3061) || (n > 3061 && n < 3073) || (n > 3073 && n < 3247) || (n > 3247 && n < 3275) || (n > 3275 && n < 3443) || (n > 3443 && n < 5001))
	; 7 bytes
	ldx #>(n-34)
	lda #<(n-34)
	jsr delay_256x_a_30_clocks
.elseif((n .mod 12 = 1) && ((n > 2508 && n < 3062)))
	; 7 bytes
	ldx #(n-1)/12
	delay_n 7
	dex
	Jne *-3
.elseif(n = 2510)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2520)
	; 7 bytes
	ldx #5 ;hides 'ora zp'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2540)
	; 7 bytes
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
.elseif(n = 2547)
	; 7 bytes
	ldx #74 ;hides 'lsr a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 2551)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2573)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2575)
	; 7 bytes
	ldx #184 ;hides 'clv'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 2583)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	inx
	Jne *-4
.elseif(n = 2584)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2595)
	; 7 bytes
	ldx #21 ;hides 'ora zp,x'
	dex ;first loop only
	nop
	inx
	Jne *-4
.elseif(n = 2683)
	; 7 bytes
	ldx #245 ;hides 'sbc zp,x'
	dex ;first loop only
	nop
	dex
	Jne *-4
.elseif(n = 2693)
	; 7 bytes
	ldx #245 ;hides 'sbc zp,x'
	cmp $EA ;hides 'nop'
	dex
	Jne *-4
.elseif(n = 2694)
	; 7 bytes
	ldx #245 ;hides 'sbc zp,x'
	delay_n 4
	dex
	Jne *-4
.elseif(n = 2705)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	delay_n 4
	inx
	Jne *-4
.elseif(n = 2799)
	; 7 bytes
	ldx #56 ;hides 'sec'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 2841)
	; 7 bytes
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
.elseif(n = 2995)
	; 7 bytes
	ldx #42 ;hides 'rol a'
	pla
	pha
	inx
	Jne *-4
.elseif(n = 3073)
	; 7 bytes
	ldx #0
	delay_n 7
	dex
	Jne *-3
.elseif(n = 3247)
	; 7 bytes
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
.elseif(n = 3275)
	; 7 bytes
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
.elseif(n = 3443)
	; 7 bytes
	ldx #10 ;hides 'asl a'
	pla
	pha
	inx
	Jne *-4
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
