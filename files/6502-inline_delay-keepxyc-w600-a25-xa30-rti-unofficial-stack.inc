; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, S, Z+N, D, and V
; Will preserve X, Y, C, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; WARNING: May write garbage into $600-$6FF.
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
; - 1 bytes or less when n <= 4
; - 2 bytes or less when n <= 8
; - 3 bytes or less when n <= 17
; - 4 bytes or less when n <= 21
; - 5 bytes or less when n <= 30
; - 6 bytes or less when n <= 34
; - 7 bytes or less when n <= 292
; - 8 bytes or less when n <= 310
; - 9 bytes or less when n <= 1305
; -10 bytes or less when n <= 1981
; -11 bytes or less when n <= 3001
; -12 bytes or less when n <= 4785
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=2, avg=3.2, max=4
; - When 21 <= n <= 30: min=4, avg=4.8, max=5
; - When 31 <= n <= 50: min=6, avg=6.7, max=7
; - When 51 <= n <= 75: min=6, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.0, max=8
; - When 301 <= n <= 500: min=6, avg=7.9, max=9
; - When 501 <= n <= 750: min=6, avg=7.9, max=9
; - When 751 <= n <= 1000: min=7, avg=7.9, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.5, max=11
; - When 2001 <= n <= 3000: min=8, avg=9.6, max=11
; - When 3001 <= n <= 5000: min=8, avg=10.6, max=13
; - When 5001 <= n <= 7500: min=9, avg=12.1, max=13
; - When 7501 <= n <= 10000: min=10, avg=12.8, max=13
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
.elseif(n = 5 || n = 28)
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
.elseif(n = 9 || n = 10 || n = 20 || n = 22 || n = 33)
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
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 17 || n = 21 || n = 26 || n = 29 || n = 30 || n = 34 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 19 || n = 23)
	; 4..5 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24)
	; 5 bytes
	pha
	delay_n (n-3)
.elseif(n = 25)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 31)
	; 6 bytes
	php
	lda #$0A ;hides 'asl a'
	Jpl *-1
	plp
.elseif(n = 32)
	; 6 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n > 34 && n < 39) || (n > 39 && n < 46) || (n > 46 && n < 53) || (n > 53 && n < 128) || (n > 128 && n < 157))
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
.elseif(n = 291 || n = 298)
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
	brk
	delay_n (n-13)
.elseif(n = 303)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
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
.elseif(n = 316 || n = 317 || n = 321 || n = 322 || n = 326 || n = 331 || n = 332 || n = 336 || n = 337 || n = 342 || n = 347 || n = 351 || n = 352 || n = 357 || n = 361 || n = 366 || n = 367 || n = 371 || n = 372 || n = 377 || n = 382 || n = 392 || n = 407 || n = 412 || n = 417 || n = 422 || n = 427 || n = 431 || n = 437 || n = 441 || n = 442 || n = 447 || n = 451 || n = 452 || n = 456 || n = 457 || n = 462 || n = 466)
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
.elseif(n = 396)
	; 8 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 426)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $2EA0'
	ldy #46
	dey
	Jpl *-4
	tay
.elseif(n = 436)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	iny
	Jne *-4
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
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 479 || n = 494 || n = 499 || n = 504 || n = 511 || n = 514 || n = 519 || n = 524 || n = 529 || n = 539)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && (n = 470 || n = 480 || n = 485 || n = 495 || (n > 504 && n < 516) || (n > 524 && n < 546)))
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
.elseif(n = 472 || n = 477 || n = 482 || n = 486 || n = 487 || n = 492 || n = 497 || n = 512 || n = 517 || n = 522 || n = 527 || n = 531 || n = 532 || n = 542 || n = 546)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 473 || n = 478 || n = 483 || n = 496 || n = 498 || n = 503 || n = 508 || n = 513 || n = 518 || n = 528 || n = 538 || n = 543)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 474 || n = 502 || n = 516 || n = 523 || n = 544)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 475 || n = 490 || n = 500 || n = 520)
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
.elseif(n = 481 || n = 488 || n = 509 || n = 537)
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
.elseif(n = 501)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 506)
	; 9 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 526)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $C6A0'
	ldy #198
	iny
	Jne *-4
	tay
.elseif(n = 533 || n = 541)
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
.endif
.else
; 547 to 625
.if(n = 547 || n = 552 || n = 557 || n = 562 || n = 566 || n = 567 || n = 577 || n = 582 || n = 587 || n = 591 || n = 592 || n = 596 || n = 597 || n = 601 || n = 602 || n = 612 || n = 616 || n = 617 || n = 622)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 548)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 549 || n = 554 || n = 559 || n = 569 || n = 574 || n = 576 || n = 584 || n = 589 || n = 594 || n = 599 || n = 604 || n = 609 || n = 619 || n = 624)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && ((n > 549 && n < 566) || n = 590 || (n > 599 && n < 611)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 551 || n = 586 || n = 607 || n = 614 || n = 621)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 553 || n = 563 || n = 568 || n = 578 || n = 583 || n = 588 || n = 598 || n = 603 || n = 608 || n = 618 || n = 623)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 558 || n = 572 || n = 579 || n = 593)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
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
.elseif((n .mod 5 = 0) && ((n > 569 && n < 586) || n = 595 || (n > 614 && n < 626)))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 571)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	dey
	Jne *-4
	tay
.elseif(n = 573)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 581 || n = 613)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626)
	; 7 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657 || n = 662 || n = 666 || n = 667 || n = 672 || n = 676 || n = 682 || n = 686 || n = 687 || n = 697 || n = 702)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 649 || n = 663 || n = 684 || n = 698)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 654 || n = 659 || n = 664 || n = 669 || n = 674 || n = 679 || n = 681 || n = 689 || n = 692 || n = 694 || n = 699)
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
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 701)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636 || n = 638 || n = 643 || n = 648 || n = 658 || n = 668 || n = 673 || n = 683 || n = 691 || n = 703)
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
.elseif(n = 642 || n = 656 || n = 677)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 646 || n = 701)
	; 8..9 bytes
	brk
	delay_n (n-13)
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 653 || n = 661)
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
.elseif(n = 693)
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
.endif
.else
; 704 to 781
.if(n = 704 || n = 709 || n = 714 || n = 721 || n = 724 || n = 729 || n = 734 || n = 739 || n = 744 || n = 749 || n = 756 || n = 764 || n = 769 || n = 774 || n = 779)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 706)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	dey
	Jmi *-4
	tay
.elseif(n = 707 || n = 711 || n = 722 || n = 727 || n = 732 || n = 737 || n = 742 || n = 746 || n = 757 || n = 762 || n = 766 || n = 767 || n = 771 || n = 772 || n = 776 || n = 777)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 707 && n < 729) || n = 738 || n = 743 || (n > 752 && n < 764) || n = 778))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 712 || n = 719 || n = 733 || n = 747 || n = 754 || n = 761 || n = 768)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 726)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
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
.elseif(n = 741 || n = 781)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 748)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jpl *-3
	tay
.elseif(n = 751)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $D2A0'
	ldy #210
	dey
	Jmi *-4
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
.elseif(n = 773)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.endif
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
.elseif(n = 786)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $D6A0'
	ldy #214
	dey
	Jmi *-4
	tay
.elseif(n = 787 || n = 791 || n = 792 || n = 797 || n = 802 || n = 807 || n = 811 || n = 812 || n = 816 || n = 822 || n = 826 || n = 827 || n = 832 || n = 837 || n = 842 || n = 846 || n = 847 || n = 856 || n = 862 || n = 867 || n = 872 || n = 877 || n = 882 || n = 891 || n = 892 || n = 897 || n = 902 || n = 906 || n = 907 || n = 911 || n = 912 || n = 917 || n = 927 || n = 931 || n = 932 || n = 937)
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
.elseif(n = 841)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jpl *-4
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
.elseif(n = 876)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $60A0'
	ldy #96
	dey
	Jpl *-4
	tay
.elseif(n = 881)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 886)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $1EA0'
	ldy #30
	iny
	Jpl *-4
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
.elseif(n = 942 || n = 947 || n = 951 || n = 952 || n = 956 || n = 961 || n = 967 || n = 972 || n = 977 || n = 982 || n = 987 || n = 991 || n = 996 || n = 997 || n = 1001 || n = 1002 || n = 1007 || n = 1012 || n = 1016 || n = 1017 || n = 1032 || n = 1036 || n = 1037 || n = 1042 || n = 1046 || n = 1047 || n = 1051 || n = 1052 || n = 1057 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1081 || n = 1082 || n = 1087 || n = 1091 || n = 1092)
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
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 969 || n = 974 || n = 979 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1019 || n = 1022 || n = 1024 || n = 1039 || n = 1044 || n = 1049 || n = 1054 || n = 1059 || n = 1064 || n = 1079 || n = 1084 || n = 1089)
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
.elseif(n = 1066)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	iny
	Jpl *-4
	tay
.elseif(n = 1074)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 1086)
	; 9 bytes
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	Jmi *-4
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
.elseif(n = 1096 || n = 1102 || n = 1106 || n = 1107 || n = 1112 || n = 1116 || n = 1117 || n = 1122 || n = 1126 || n = 1127 || n = 1131 || n = 1136 || n = 1137 || n = 1142 || n = 1147 || n = 1151 || n = 1152 || n = 1157 || n = 1161 || n = 1162 || n = 1171 || n = 1172 || n = 1176 || n = 1177 || n = 1182 || n = 1186 || n = 1187 || n = 1192 || n = 1196 || n = 1197 || n = 1206 || n = 1207 || n = 1212 || n = 1217 || n = 1222 || n = 1226 || n = 1227 || n = 1231 || n = 1232 || n = 1242 || n = 1246 || n = 1247)
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
	bit <$2C ;hides 'bit $80A0'
	ldy #128
	dey
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
.elseif(n = 1201)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $85A0'
	ldy #133
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
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if(n = 1251 || n = 1258 || n = 1272 || n = 1279 || n = 1286)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1257 || n = 1262 || n = 1266 || n = 1267 || n = 1271 || n = 1276 || n = 1277 || n = 1281 || n = 1282 || n = 1287)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1253 || n = 1261)
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
.elseif(n = 1256)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 5 = 4) && ((n > 1258 && n < 1275) || n = 1284 || n = 1289))
	; 8 bytes
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
.elseif((n .mod 5 = 3) && ((n > 1267 && n < 1289)))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.endif
.else
; 1290 to 1328
.if(n = 1290)
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
.elseif((n .mod 7 = 5) && ((n > 1292 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1294 || n = 1320 || n = 1324)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1295 || n = 1297 || n = 1311)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1296)
	; 9 bytes
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1299)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $70A0'
	ldy #112
	iny
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && ((n > 1300 && n < 1326)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1302 || n = 1316 || n = 1323)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1303)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1304)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1305 || n = 1315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1306)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1308)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $91A0'
	ldy #145
	dey
	Jne *-4
	tay
.elseif(n = 1310 || n = 1319)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
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
.elseif(n = 1318)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
.elseif(n = 1322)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1326)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.elseif(n = 1327)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1332 || n = 1339 || n = 1360)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1330 || n = 1344 || n = 1348 || n = 1351 || n = 1358 || n = 1359)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1352 || n = 1366)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1333 || n = 1341 || n = 1357 || n = 1365)
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
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1364)))
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
	bit <$2C ;hides 'bit $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1336 && n < 1365)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1340)
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
	bit <$2C ;hides 'bit $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
.elseif(n = 1347)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1350)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1353)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $6AA0'
	ldy #106
	iny
	Jne *-4
	tay
.elseif(n = 1354)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $96A0'
	ldy #150
	dey
	Jne *-4
	tay
.elseif(n = 1361)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1362)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $97A0'
	ldy #151
	dey
	Jne *-4
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
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1376 || n = 1387 || n = 1392 || n = 1401 || n = 1406)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1369)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1406)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1371)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
	tax
.elseif(n = 1372 || n = 1379 || n = 1383 || n = 1386)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373 || n = 1381 || n = 1397)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1374 || n = 1388 || n = 1393 || n = 1395)
	; 8..9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1382 || n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1385)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1389)
	; 7 bytes
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
.elseif(n = 1390)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $9AA0'
	ldy #154
	dey
	Jne *-4
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
.elseif(n = 1399)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
.elseif(n = 1402)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.elseif(n = 1404)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1423 || n = 1428 || n = 1430 || n = 1441)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1408)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $64A0'
	ldy #100
	iny
	Jne *-4
	tay
.elseif(n = 1409 || n = 1418 || n = 1436)
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
.elseif(n = 1411 || n = 1432)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1441)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && ((n > 1412 && n < 1446)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1414 || n = 1420 || n = 1435 || n = 1439 || n = 1442)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1415)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1416)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	tay
.elseif(n = 1417)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	tay
.elseif(n = 1422 || n = 1427)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1425)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $9EA0'
	ldy #158
	dey
	Jne *-4
	tay
.elseif(n = 1434)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $61A0'
	ldy #97
	iny
	Jne *-4
	tay
.elseif(n = 1438)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1443)
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
	tax
.elseif(n = 1444)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1446 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1448 || n = 1450 || n = 1464 || n = 1478)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1449 || n = 1456 || n = 1470 || n = 1474 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1451 || n = 1458 || n = 1465 || n = 1473)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1452)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A1A0'
	ldy #161
	dey
	Jne *-4
	tay
.elseif(n = 1453 || n = 1469 || n = 1477)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1455)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
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
.elseif(n = 1460)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 1462)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
.elseif(n = 1463 || n = 1472 || n = 1481)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1466)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1467)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 1471)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
.elseif(n = 1476)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1479)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.elseif(n = 1480)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if((n .mod 8 = 5) && ((n > 1484 && n < 1510)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1486 || n = 1514 || n = 1519 || n = 1521)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1487 || n = 1491 || n = 1498 || n = 1502 || n = 1505 || n = 1511 || n = 1512)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1488)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1490 || n = 1499)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1492 || n = 1504 || n = 1513 || n = 1518)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1494)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1495)
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
.elseif(n = 1506)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
.elseif(n = 1507)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
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
.elseif(n = 1522)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1523)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	tax
.endif
.else
; 1524 to 1562
.if((n .mod 7 = 5) && ((n > 1523 && n < 1560)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && ((n > 1524 && n < 1558)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && ((n > 1525 && n < 1563)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1527 || n = 1540 || n = 1547 || n = 1554 || n = 1561)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1528)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1529)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1530)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1532 || n = 1548)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1534)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
.elseif(n = 1536)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1537 || n = 1546 || n = 1556)
	; 9 bytes
	pla
	delay_n (n-4)
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
	bit <$2C ;hides 'bit $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
.elseif(n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1551)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1555)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
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
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
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
.elseif(n = 1565 || n = 1581 || n = 1589 || n = 1597)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1567 || n = 1600)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1568 || n = 1572 || n = 1575 || n = 1582 || n = 1583 || n = 1596 || n = 1599)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1570)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1571 || n = 1598)
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
.elseif(n = 1576 || n = 1590 || n = 1592)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1577)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1578)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1579)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1584 || n = 1591 || n = 1593)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1585 || n = 1595)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1586)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 1588)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B0A0'
	ldy #176
	dey
	Jne *-4
	tay
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1603 || n = 1610 || n = 1624 || n = 1638 || n = 1639)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1604 || n = 1616 || n = 1630)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1605 || n = 1621 || n = 1637)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1606)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	dey
	Jne *-4
	tay
.elseif(n = 1607 || n = 1625)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1607 && n < 1637)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1609 || n = 1612 || n = 1617 || n = 1619 || n = 1631)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1611)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
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
.elseif(n = 1623)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $B4A0'
	ldy #180
	dey
	Jne *-4
	tay
.elseif(n = 1626 || n = 1640)
	; 9 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1633)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B5A0'
	ldy #181
	dey
	Jne *-4
	tay
.elseif(n = 1635)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B6A0'
	ldy #182
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1679)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1644 || n = 1656 || n = 1662 || n = 1667 || n = 1672 || n = 1674)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1645 || n = 1675)
	; 8..9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1651)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	dey
	Jne *-4
	tay
.elseif(n = 1652 || n = 1670 || n = 1679)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && ((n > 1652 && n < 1678)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1654)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1655 || n = 1665)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1658)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 1659)
	; 8 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1660)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
.elseif(n = 1663)
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 1666 || n = 1673)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1668)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	dey
	Jne *-4
	tay
.elseif(n = 1676)
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
; 1680 to 1718
.if(n = 1680 || n = 1687 || n = 1694 || n = 1707 || n = 1708)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1681 || n = 1702 || n = 1712 || n = 1716)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1682 || n = 1689 || n = 1703 || n = 1710)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1683)
	; 9 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif(n = 1684 || n = 1691)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif((n .mod 7 = 5) && ((n > 1684 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 1688 || n = 1697 || n = 1715)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1690)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif((n .mod 8 = 5) && ((n > 1692 && n < 1718)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1695)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1696)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1698)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $9AA0'
	ldy #154
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1700)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1704)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1705)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1711)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 1714)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
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
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1736 || n = 1750)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1756)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1721 || n = 1745)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1723)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $BFA0'
	ldy #191
	dey
	Jne *-4
	tay
.elseif(n = 1724 || n = 1751)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1733 || n = 1749 || n = 1757)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1728 || n = 1730 || n = 1737 || n = 1742)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1731)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
.elseif(n = 1732)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
.elseif(n = 1735)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1738 || n = 1743)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1739)
	; 7 bytes
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
.elseif(n = 1740)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	iny
	Jne *-4
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
.elseif(n = 1747)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1752)
	; 9 bytes
	brk
	delay_n (n-13)
.elseif(n = 1753)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1754)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1756)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.endif
.else
; 1758 to 1796
.if(n = 1758 || n = 1766 || n = 1770 || n = 1771 || n = 1774 || n = 1791 || n = 1792)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1759)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	tay
.elseif(n = 1760 || n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1761 || n = 1780)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 7 = 5) && ((n > 1761 && n < 1791)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
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
.elseif((n .mod 8 = 5) && ((n > 1764 && n < 1790)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1767)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1768)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1772)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1775)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1777)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
.elseif(n = 1779 || n = 1784 || n = 1788)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1782)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 1785)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1786)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
.elseif(n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
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
.elseif(n = 1795)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	iny
	Jne *-4
	tay
.endif
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
.elseif(n = 1798 || n = 1801 || n = 1807 || n = 1833 || n = 1834)
	; 9 bytes
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
.elseif(n = 1804)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && ((n > 1804 && n < 1830)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1809 || n = 1820)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1810)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 1811)
	; 10 bytes
	tya
	ldy #128
	delay_n 9
	dey
	Jpl *-4
	tay
.elseif(n = 1812)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
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
.elseif(n = 1818)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1822)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
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
	nop
	nop
	Jne *-3
	tax
.elseif(n = 1831)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	iny
	Jne *-4
	tay
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
.elseif(n = 1840)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	dey
	Jne *-4
	tay
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
.elseif(n = 1846 || n = 1851 || n = 1856 || n = 1872)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1847 || n = 1860 || n = 1863 || n = 1870 || n = 1871)
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
.elseif(n = 1849)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	dey
	Jne *-4
	tay
.elseif(n = 1852 || n = 1873)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1854)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 1855)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1857)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
.elseif(n = 1858)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	tay
.elseif(n = 1862)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $A9A0'
	ldy #169
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1866)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	dey
	Jne *-4
	tay
.elseif(n = 1867)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	iny
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
.elseif(n = 1875)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	dey
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
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
.if(n = 1876)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	dey
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && ((n > 1876 && n < 1910)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1878 || n = 1882 || n = 1887 || n = 1891 || n = 1898)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 1880 || n = 1888 || n = 1896 || n = 1900 || n = 1914)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1881 || n = 1890 || n = 1899)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1885 && n < 1914)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1889)
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
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	iny
	Jne *-4
	tay
.elseif(n = 1897 || n = 1908)
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
.elseif(n = 1903)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
.elseif(n = 1905)
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
.elseif(n = 1912)
	; 9 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	tax
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1935 || n = 1945)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
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
.elseif((n .mod 8 = 5) && ((n > 1916 && n < 1950)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 1919 || n = 1952)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
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
.elseif(n = 1922 || n = 1931 || n = 1940)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1923 || n = 1928 || n = 1934 || n = 1936 || n = 1944 || n = 1950)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1924 || n = 1953)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1926)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
.elseif(n = 1927 || n = 1932 || n = 1942 || n = 1943 || n = 1946 || n = 1951)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1929)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
.elseif(n = 1930)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	dey
	Jne *-4
	tay
.elseif(n = 1937)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1938)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	tay
.elseif(n = 1939)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	iny
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
.elseif(n = 1948)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 1955)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1956)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	dey
	Jne *-4
	tay
.elseif(n = 1957 || n = 1973 || n = 1981 || n = 1989)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 1986)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 1978 || n = 1991)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1960 || n = 1964 || n = 1986 || n = 1988 || n = 1990)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1961)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1962)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1963)
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
.elseif(n = 1966)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	dey
	Jne *-4
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
.elseif(n = 1974)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
.elseif(n = 1975)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
.elseif(n = 1977 || n = 1980 || n = 1987)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1979)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1982)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1983)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 1984)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	tay
.elseif(n = 1992)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
.endif
.else
; 1993 to 2031
.if(n = 1993)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jne *-4
	tay
.elseif(n = 1994 || n = 2012 || n = 2030)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1995 || n = 2015 || n = 2025)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1996 || n = 2007 || n = 2018)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && ((n > 1996 && n < 2030)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1998 || n = 2017)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 1999 || n = 2004 || n = 2014 || n = 2022 || n = 2023 || n = 2026 || n = 2031)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2000 || n = 2006 || n = 2008 || n = 2016 || n = 2024)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2001)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
.elseif(n = 2002)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
.elseif(n = 2003)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2010)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2011)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2019)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	iny
	Jne *-4
	tay
.elseif(n = 2020)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
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
.elseif(n = 2028)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	iny
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2036 || n = 2044 || n = 2054)
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
.elseif(n = 2034 || n = 2041 || n = 2049 || n = 2052)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2035)
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
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	tax
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
.elseif(n = 2042 || n = 2050 || n = 2059 || n = 2068)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2043)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	brk
	iny
	Jpl *-4
	tay
.elseif(n = 2046)
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
.elseif(n = 2047)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
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
.elseif(n = 2055)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
.elseif(n = 2056)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	dey
	Jne *-4
	tay
.elseif(n = 2058)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
	tay
.elseif(n = 2060)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
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
.elseif(n = 2064)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
.elseif(n = 2065)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	iny
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
.elseif(n = 2074)
	; 9 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	tax
.elseif((n .mod 9 = 5) && ((n > 2074 && n < 2103)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2076 || n = 2094)
	; 9 bytes
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
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	tax
.elseif(n = 2085)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2087 || n = 2103 || n = 2104 || n = 2107 || n = 2108)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2089)
	; 10 bytes
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
.elseif(n = 2092)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
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
.elseif(n = 2099)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2100)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	dey
	Jne *-4
	tay
.elseif(n = 2101)
	; 9 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	tax
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
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2148)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2121 || n = 2123 || n = 2130 || n = 2144)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2113 || n = 2122 || n = 2124 || n = 2131 || n = 2140 || n = 2142)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2114)
	; 10 bytes
	php
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2115 || n = 2125 || n = 2135)
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
.elseif(n = 2117 || n = 2141)
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
.elseif(n = 2119)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	tay
.elseif(n = 2126)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2127)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 2128)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	iny
	Jne *-4
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
.elseif(n = 2133)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2136)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 2137)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
.elseif(n = 2139)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2143)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	brk
	dey
	Jmi *-4
	tay
.elseif(n = 2145)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	dey
	Jne *-4
	tay
.elseif(n = 2146)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.elseif(n = 2148)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2157 || n = 2158 || n = 2176 || n = 2179 || n = 2184)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2150 || n = 2161)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2151)
	; 9 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
	tax
.elseif(n = 2152)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2153 || n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2154)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	iny
	Jne *-4
	tay
.elseif(n = 2155)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
.elseif(n = 2156 || n = 2165 || n = 2174)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2159)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2160)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2162)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2163)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2164)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	iny
	Jne *-4
	tay
.elseif(n = 2166 || n = 2168 || n = 2186)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2167 || n = 2169 || n = 2178 || n = 2187)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2170)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $3BA0'
	ldy #59
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2171)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2172)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2173)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2175 || n = 2185)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
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
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
.elseif(n = 2182)
	; 9 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
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
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188 || n = 2193 || n = 2202 || n = 2222)
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
.elseif(n = 2191)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	tay
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
.elseif(n = 2196 || n = 2206 || n = 2220 || n = 2221)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2197)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2198)
	; 10 bytes
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
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
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
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
.elseif(n = 2207 || n = 2212 || n = 2223)
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
.elseif(n = 2209)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	dey
	Jne *-4
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
	; 9 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	tax
.elseif(n = 2218)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
.elseif(n = 2224)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $B9A0'
	ldy #185
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2226)
	; 9 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
	; 9 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
.elseif(n = 2228 || n = 2237)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2229 || n = 2230)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2231)
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
.elseif(n = 2233)
	; 11 bytes
	php
	sec
	lda #247
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
.elseif(n = 2238)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2239)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2240 || n = 2241)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
.endif
.else
; 2246 to 2265
.if(n = 2246 || n = 2255 || n = 2264)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
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
.elseif(n = 2248 || n = 2251)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2249 || n = 2261)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2250)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jmi *-5
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
.elseif(n = 2256)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2257)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 2258)
	; 9 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2259)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2260)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
.elseif(n = 2268 || n = 2277 || n = 2279 || n = 2286)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2269)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2270)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	nop
	dey
	Jne *-5
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
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305 || n = 2315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2306 || n = 2312 || n = 2318)
	; 9..10 bytes
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
.elseif(n = 2310 || n = 2311)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2313)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2314)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2316 || n = 2322)
	; 10..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2317)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2319)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2320)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2321)
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
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 9 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2325)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
.elseif(n = 2328)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2332)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
.elseif(n = 2333)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
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
.elseif(n = 2336)
	; 10 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	tax
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
.elseif(n = 2339)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 2340)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jpl *-5
	tay
.elseif(n = 2341)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2342)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
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
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif((n .mod 12 = 5) && ((n > 2344 && n < 2382)))
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
.elseif(n = 2347 || n = 2349 || n = 2352)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2348 || n = 2359 || n = 2370)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2350 || n = 2367 || n = 2372 || n = 2377)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2351)
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
.elseif(n = 2355 || n = 2365 || n = 2375)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2356)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2358)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2360)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2361)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2362 || n = 2366 || n = 2373 || n = 2378 || n = 2382)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2363)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2364 || n = 2376)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2368)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2371)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2374)
	; 10 bytes
	tya
	ldy #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
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
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	nop
	iny
	Jne *-5
	tay
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2387 || n = 2394 || n = 2407 || n = 2416 || n = 2419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || n = 2397 || n = 2410)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2385 || n = 2395 || n = 2415)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2386 || n = 2400)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2388 || n = 2396 || n = 2398 || n = 2406 || n = 2408)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2389 || n = 2409 || n = 2421)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2390)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2391)
	; 9 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
	tay
.elseif(n = 2392 || n = 2403 || n = 2414)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2393 || n = 2405 || n = 2417)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2399)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2404)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	pha
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
.elseif(n = 2412)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	nop
	dey
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
.elseif(n = 2418)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2420)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
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
	ldy #248 ;hides 'sed'
	brk
	dey
	Jmi *-4
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
.elseif(n = 2425 || n = 2445 || n = 2455)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2426 || n = 2431 || n = 2438 || n = 2460)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428 || n = 2432 || n = 2442 || n = 2444 || n = 2448 || n = 2454)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2429 || n = 2441 || n = 2453)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2430)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2433 || n = 2440 || n = 2459)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2434)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2435)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2436 || n = 2447 || n = 2458)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2437)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2439)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
.elseif(n = 2443)
	; 9 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 2446)
	; 10 bytes
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
.elseif(n = 2452)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 2456)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2457)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.endif
.else
; 2461 to 2500
.if(n = 2461 || n = 2472 || n = 2474 || n = 2486 || n = 2492 || n = 2498)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
	ldy #122 ;hides 'nop'
	brk
	dey
	Jpl *-4
	tay
.elseif(n = 2464)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2465 || n = 2477 || n = 2489)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2466)
	; 10 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 2467 || n = 2479 || n = 2493)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2468)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	nop
	dey
	Jne *-5
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
.elseif(n = 2473 || n = 2481 || n = 2484 || n = 2487 || n = 2499)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2475 || n = 2485 || n = 2495)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2476)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2478)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2480 || n = 2491)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2482 || n = 2496)
	; 10 bytes
	brk
	delay_n (n-13)
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
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2494)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2500)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	inx
	Jne *-5
	tax
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
.if((n .mod 12 = 5) && ((n > 2500 && n < 2538)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2502 || n = 2524)
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
.elseif(n = 2504 || n = 2521 || n = 2538)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2505 || n = 2515 || n = 2535)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2506 || n = 2508 || n = 2516 || n = 2518 || n = 2528)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2507)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2509 || n = 2517 || n = 2526 || n = 2527)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2510)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2511 || n = 2539)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2512)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2514)
	; 10 bytes
	tya
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 2519 || n = 2529)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2520)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2522)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2523)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2530)
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
.elseif(n = 2532)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2534)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2536)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	pha
	iny
	Jne *-5
	tay
.endif
.else
; 2540 to 2578
.if(n = 2540)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2541 || n = 2548 || n = 2550 || n = 2569)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2542)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2543)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
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
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2546 || n = 2557 || n = 2568)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2547)
	; 9 bytes
	tya
	ldy #(n-3)/12 ;hides 'nop zp,x'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2549 || n = 2561 || n = 2573)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2551 || n = 2563 || n = 2575)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2552 || n = 2558 || n = 2560 || n = 2562 || n = 2564 || n = 2571 || n = 2576)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2553 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2556)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	nop
	dey
	Jne *-5
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
.elseif(n = 2566)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2570)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2572)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2574)
	; 10 bytes
	tya
	ldy #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jpl *-5
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
.elseif(n = 2578)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	nop
	iny
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
.if((n >= 2579) && (n < 2598))
; 2579 to 2597
.if(n = 2579)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2591 || n = 2593)
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
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2585 || n = 2597)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2586)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jpl *-5
	tay
.elseif(n = 2587)
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
.elseif(n = 2589)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2590)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2592)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2594)
	; 10 bytes
	tya
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
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
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	pha
	iny
	Jne *-5
	tay
.endif
.else
; 2598 to 2617
.if(n = 2598)
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
.elseif(n = 2600)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2601 || n = 2612)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2602 || n = 2607 || n = 2613 || n = 2616)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2603)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-4
	tay
.elseif(n = 2604)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2605)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2606)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	tay
.elseif(n = 2608)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2609)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2611 || n = 2614 || n = 2617)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2615)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.endif
.endif
.else
.if((n >= 2618) && (n < 2637))
; 2618 to 2636
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
.elseif(n = 2620)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2621 || n = 2633)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2622)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2623 || n = 2634)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2624 || n = 2626)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2627)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2628)
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
.elseif(n = 2635 || n = 2636)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.else
; 2637 to 2656
.if(n = 2637)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2638 || n = 2649)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2639 || n = 2647)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2640)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2641)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 2645)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2646)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2648)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2650)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2651)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2652)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2653)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 2654)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2655)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2656)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if((n .mod 12 = 5) && ((n > 2656 && n < 2694)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2658 || n = 2659 || n = 2671 || n = 2680 || n = 2695)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675 || n = 2690)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2661 || n = 2673 || n = 2682)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2662)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2663)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2664)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2665 || n = 2679)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2666)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2667 || n = 2678 || n = 2689)
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
.elseif(n = 2670)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2672 || n = 2692)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2674 || n = 2691)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2683)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 2684)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2694)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	nop
	nop
	dey
	Jne *-6
	tay
.endif
.else
; 2696 to 2734
.if(n = 2696)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2697 || n = 2704 || n = 2715 || n = 2726)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #(n-14)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2699)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	dey
	Jne *-5
	tay
.elseif((n .mod 11 = 5) && ((n > 2699 && n < 2734)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2701 || n = 2714)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2702 || n = 2713 || n = 2718 || n = 2719 || n = 2724)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2703)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2705 || n = 2717 || n = 2729)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2706)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2707 || n = 2721)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2708 || n = 2725)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2709)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2710)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2712 || n = 2730)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2716)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2720)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2723)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2727)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2728)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2731)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2732)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2734)
	; 10 bytes
	tya
	ldy #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
.if((n >= 2735) && (n < 2754))
; 2735 to 2753
.if(n = 2735 || n = 2749)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2736 || n = 2747)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2737)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2738)
	; 11 bytes
	pha
	delay_n (n-7)
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
.elseif(n = 2740)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2741 || n = 2753)
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
.elseif(n = 2743)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	tay
.elseif(n = 2744)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2745)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 2748)
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
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 2751)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2752)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	pha
	iny
	Jne *-5
	tay
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2755 || n = 2766)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2756 || n = 2762 || n = 2769)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2757)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
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
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2760)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2761 || n = 2767 || n = 2768)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2765)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2770)
	; 10 bytes
	pla
	delay_n (n-4)
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
.endif
.else
.if((n >= 2774) && (n < 2793))
; 2774 to 2792
.if(n = 2774)
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
.elseif(n = 2777 || n = 2789)
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
.elseif(n = 2779 || n = 2790)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2780)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2781)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2782)
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
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 2788)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2791)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2792)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.else
; 2793 to 2812
.if(n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2794)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2795)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
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
.elseif(n = 2799 || n = 2810)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2800)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2801)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2802 || n = 2804 || n = 2806)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2803)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2805)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2807)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2812)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
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
.elseif(n = 2814 || n = 2829 || n = 2841)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2815 || n = 2822 || n = 2827 || n = 2839 || n = 2846 || n = 2851)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2838 || n = 2848 || n = 2850)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2817 || n = 2836)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 2831)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2832)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	delay_n 16
	dey
	Jpl *-5
	tay
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
.elseif(n = 2843)
	; 11 bytes
	tya
	ldy #128
	nop
	jsr @nearby_rts_15cyc
	dey
	Jpl *-5
	tay
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
.if((n >= 2852) && (n < 2871))
; 2852 to 2870
.if(n = 2852)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2853)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2854 || n = 2866)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2855)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
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
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2860)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2861)
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
.elseif(n = 2863 || n = 2869)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2864)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 2868)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2870)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.endif
.else
; 2871 to 2890
.if(n = 2871)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2872)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2874)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2875 || n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2876 || n = 2888)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2877)
	; 10 bytes
	pla
	delay_n (n-4)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 2880)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 16
	Jne *-4
	tay
.elseif(n = 2881)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 2882)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 2883)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2884)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2886)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2887)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2890)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2904)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2892 || n = 2910 || n = 2928)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2893)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2894 || n = 2906 || n = 2918 || n = 2922 || n = 2924)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2895 || n = 2912)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2896)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2897 || n = 2909 || n = 2921)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2898)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2899 || n = 2911 || n = 2923 || n = 2927)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2900 || n = 2915)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2901 || n = 2913)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2902 || n = 2914 || n = 2926)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2903)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2905 || n = 2925)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2907)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2908)
	; 10 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2916)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2917)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2919)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2920)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2929)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 2930) && (n < 2949))
; 2930 to 2948
.if(n = 2930)
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
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2933 || n = 2945)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2934 || n = 2936 || n = 2948)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2935 || n = 2941 || n = 2947)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2937)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2938)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	pha
	iny
	iny
	Jne *-5
	tay
.elseif(n = 2939)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2940)
	; 11 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 2944)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2946)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.endif
.else
; 2949 to 2968
.if(n = 2949 || n = 2961)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2950)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2951)
	; 10 bytes
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 2952)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2953)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 2954)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 2955)
	; 10 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	pha
	inx
	Jne *-5
	tax
.elseif(n = 2956)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2957)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2958)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2959)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2960)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2962)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2964)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2966)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 2967)
	; 10 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2968)
	; 10 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	pha
	iny
	Jne *-5
	tay
.endif
.endif
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
.elseif(n = 2971 || n = 2977 || n = 2983 || n = 2989 || n = 2995 || n = 2999 || n = 3002 || n = 3007)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2978 || n = 2984 || n = 2996)
	; 10 bytes
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
.elseif(n = 2975)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2976 || n = 3000)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 2979)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2980 || n = 2997)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2988)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2990)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
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
.elseif(n = 2994)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	nop
	inx
	Jne *-6
	tax
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
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3012 || n = 3018 || n = 3024)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3013 || n = 3031)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3014)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3022 || n = 3030 || n = 3032 || n = 3038 || n = 3044)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3025)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3026)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3036)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3037)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 3042)
	; 11 bytes
	brk
	delay_n (n-13)
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
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
.if(n = 3047)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3048)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
	tax
.elseif(n = 3049)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3050)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3051 || n = 3056)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3052 || n = 3059)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3054)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3055)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3057)
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
.elseif(n = 3060)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3061)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	brk
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
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.endif
.else
; 3066 to 3085
.if(n = 3066 || n = 3067 || n = 3079)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3068 || n = 3080)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3069)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 3070)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3071 || n = 3085)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3072)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 3073)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3074)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
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
.elseif(n = 3081)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3082)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3083)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3112)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3087 || n = 3120 || n = 3123)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3088 || n = 3106)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3089 || n = 3104 || n = 3122)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3090)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3091)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3092)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
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
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3095 || n = 3110)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3096 || n = 3114)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3097)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	nop
	nop
	dey
	Jne *-6
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
.elseif(n = 3100 || n = 3103 || n = 3115 || n = 3118)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3101 || n = 3119)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3102 || n = 3121)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3105 || n = 3125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3107)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $3EA0'
	ldy #62
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3108)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3111)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	nop
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3124)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	nop
	nop
	iny
	Jne *-6
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
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3127 || n = 3141)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3128)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $72A0'
	ldy #114
	brk
	iny
	Jne *-6
	tay
.elseif(n = 3129 || n = 3135 || n = 3139 || n = 3142 || n = 3143 || n = 3147 || n = 3152 || n = 3153 || n = 3161)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3130)
	; 11 bytes
	tya
	ldy #124
	delay_n 7
	brk
	dey
	Jpl *-5
	tay
.elseif(n = 3131)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3132)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3133 || n = 3150)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3134)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3136)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3137 || n = 3155)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3138 || n = 3151)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3140 || n = 3159)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3144)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3145)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3146)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3148 || n = 3158)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3149)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 3154)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 18
	Jne *-5
	tay
.elseif(n = 3156)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3157)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 3160)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
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
.elseif(n = 3163)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3164)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3166 || n = 3180)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3167)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3169 || n = 3183)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3170)
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
.elseif(n = 3172)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3173)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
	ldy #244 ;hides 'nop zp,x'
	nop
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3176)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3177)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3178)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3179)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	dey
	Jne *-5
	tay
.elseif(n = 3181)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
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
.endif
.else
; 3184 to 3203
.if(n = 3184 || n = 3201)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3185)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3186 || n = 3192 || n = 3193 || n = 3199)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3189)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3190)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3191)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3194)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3195)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3196)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3197)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3198)
	; 10 bytes
	tya
	ldy #(n-18)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3200)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3202)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	nop
	iny
	Jne *-6
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
.endif
.else
.if((n >= 3204) && (n < 3243))
.if((n >= 3204) && (n < 3223))
; 3204 to 3222
.if(n = 3204)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3206 || n = 3208 || n = 3212)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3207)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3209)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3210)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 3211)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3213)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3214)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3215)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3216)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3217)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3218)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3219)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3220)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3221)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3222)
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.else
; 3223 to 3242
.if(n = 3223)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3224)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	pha
	inx
	Jne *-6
	tax
.elseif(n = 3225 || n = 3239)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3226 || n = 3233)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3227)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3228)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	nop
	inx
	Jne *-6
	tax
.elseif(n = 3229 || n = 3241)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3230)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3231)
	; 10 bytes
	pla
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3232)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3234)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3235)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3236)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3238)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	brk
	dey
	Jne *-6
	tay
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
.elseif(n = 3242)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.endif
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
.elseif(n = 3244 || n = 3247 || n = 3264 || n = 3271 || n = 3277)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3245 || n = 3263 || n = 3281)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3246)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3248 || n = 3256 || n = 3266 || n = 3278)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3249 || n = 3257 || n = 3272)
	; 10..11 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 3253)
	; 9 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3254 || n = 3273)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 3259)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3260 || n = 3275)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3261)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3262)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3265)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3267)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3270)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3274)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3276)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3280)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3284 || n = 3298 || n = 3310 || n = 3314)
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
.elseif(n = 3285 || n = 3305)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 3287 || n = 3288 || n = 3296 || n = 3297 || n = 3301 || n = 3313)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3289)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3290)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3291)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 6
	brk
	Jne *-5
	tay
.elseif(n = 3292 || n = 3311)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3294 || n = 3307)
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
.elseif(n = 3299 || n = 3317)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3300)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3302)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.elseif(n = 3308)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3312)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3315)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3316)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	delay_n 7
	iny
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
.if((n >= 3321) && (n < 3340))
; 3321 to 3339
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
.elseif(n = 3322)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3323)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 3324)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3325)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3326 || n = 3338)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3327 || n = 3329 || n = 3339)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3328)
	; 11 bytes
	php
	clc
	lda #90 ;hides 'nop'
	brk
	adc #1
	Jne *-5
	plp
.elseif(n = 3330)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3334)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3335)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3336)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3337)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.else
; 3340 to 3359
.if(n = 3340 || n = 3358)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3341 || n = 3357)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3342)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3343)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3344)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3345)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3346 || n = 3355)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3347)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3348)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3349)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3350)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3351)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3352)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3353)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3354)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3356)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3359)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3378)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3361)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3362)
	; 11 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
.elseif(n = 3363)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3364)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3365 || n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3366)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3367 || n = 3370 || n = 3373 || n = 3382 || n = 3391)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3368 || n = 3387)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3371 || n = 3389)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3372 || n = 3375)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3374 || n = 3383 || n = 3392)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	pha
	iny
	Jne *-6
	tay
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
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3386)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3388)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3390)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3396)
	; 11 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3398)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 3399) && (n < 3418))
; 3399 to 3417
.if(n = 3399)
	; 11 bytes
	pla
	tya
	ldy #(n-9)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3400)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3401)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3402)
	; 11 bytes
	brk
	delay_n (n-13)
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
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3406)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3407)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3408 || n = 3409)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3410)
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
.elseif(n = 3412)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3413)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3414)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3415)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
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
.elseif(n = 3417)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	dey
	Jne *-5
	tay
.endif
.else
; 3418 to 3437
.if(n = 3418)
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
	brk
	delay_n (n-13)
.elseif(n = 3421 || n = 3435)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3422)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3423 || n = 3424 || n = 3427 || n = 3437)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3425)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3426 || n = 3429)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3428)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
	tay
.elseif(n = 3430)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3431)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3432)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
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
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	pha
	dey
	Jne *-6
	tay
.elseif(n = 3436)
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
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
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3441 || n = 3442 || n = 3451 || n = 3467 || n = 3472)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3443 || n = 3461)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3444)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3445 || n = 3465)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3446 || n = 3452 || n = 3457 || n = 3462 || n = 3464 || n = 3471)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3447 || n = 3458)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 3450)
	; 11 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 3460)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	delay_n 7
	iny
	Jne *-6
	tay
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
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 6
	inx
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
.elseif(n = 3476)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
.if((n >= 3477) && (n < 3496))
; 3477 to 3495
.if(n = 3477 || n = 3491)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3478 || n = 3494)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479)
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
.elseif(n = 3481 || n = 3484 || n = 3487)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3482)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3483)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3486)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3488)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3489)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 3490)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3492)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3493)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3495)
	; 11 bytes
	pla
	delay_n (n-4)
.endif
.else
; 3496 to 3515
.if(n = 3496)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3497 || n = 3515)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3498)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 3499)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 3500)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3501)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3502)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3503)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3504)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3505)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3506)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3507)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3508)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3509)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3510)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3511)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3512)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3513)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3514)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 6
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3546)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3517 || n = 3521 || n = 3522 || n = 3526 || n = 3527 || n = 3532 || n = 3535 || n = 3549 || n = 3553)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3518 || n = 3536 || n = 3550)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3519 || n = 3547)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3520 || n = 3539)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3523)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3524 || n = 3541)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3525 || n = 3545)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3528)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3529)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3530)
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
.elseif(n = 3533 || n = 3551)
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
.elseif(n = 3537)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3538)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3540)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3544)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3548)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3552)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3554)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3573 || n = 3591 || n = 3593)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3556)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3557)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3562 || n = 3563 || n = 3567 || n = 3571 || n = 3579)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3564 || n = 3582)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3565 || n = 3585)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3566)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3568)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $A2A0'
	ldy #162
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3569 || n = 3587)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3570)
	; 11 bytes
	tya
	ldy #(n-36)/31
	delay_n 26
	dey
	Jpl *-5
	tay
.elseif(n = 3572 || n = 3578)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3574)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3576)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3577)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
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
	; 11 bytes
	tya
	ldy #100 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
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
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3586)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3588)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	delay_n 7
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
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3598 || n = 3607 || n = 3611 || n = 3622 || n = 3628)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595 || n = 3599 || n = 3608 || n = 3629)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3596 || n = 3615)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3597 || n = 3614)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
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
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3604)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3605 || n = 3623)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3606)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 6
	dey
	Jne *-6
	tay
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
.elseif(n = 3612)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3613)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3616)
	; 11 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3617)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3619)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3620)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3621)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3624 || n = 3627)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3630)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3631)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
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
.if((n >= 3633) && (n < 3652))
; 3633 to 3651
.if(n = 3633)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3634)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3635 || n = 3650)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3636)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3637)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3638)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3639 || n = 3642)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3640 || n = 3647)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3641)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3643)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3644)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 3648)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3649)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 3651)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.else
; 3652 to 3671
.if(n = 3652)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3653)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3654 || n = 3670)
	; 10..11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3655)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 3656)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $A6A0'
	ldy #166
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3657)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
.elseif(n = 3658 || n = 3660)
	; 9..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3659)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3661)
	; 9 bytes
	pla
	delay_n (n-4)
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
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 3665)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3666)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3667)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3668)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3669)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3671)
	; 11 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3691 || n = 3710)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3674 || n = 3679 || n = 3693 || n = 3696 || n = 3697 || n = 3707)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3675)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3676 || n = 3681 || n = 3687 || n = 3689 || n = 3703)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3677 || n = 3695)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3680)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3682)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3683)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3684)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3685 || n = 3705)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3686 || n = 3698 || n = 3704)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
	delay_n (n-4)
	pla
.elseif(n = 3692 || n = 3706)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3694)
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
.elseif(n = 3700)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3701)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3702)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tay
.elseif(n = 3708)
	; 11 bytes
	brk
	delay_n (n-13)
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
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3712 || n = 3730 || n = 3733 || n = 3741 || n = 3742 || n = 3750)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3713 || n = 3731 || n = 3749)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3714)
	; 11 bytes
	pla
	tya
	ldy #(n-9)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3716 || n = 3734 || n = 3746)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3717 || n = 3719 || n = 3735)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3718 || n = 3736)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3720 || n = 3738)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3721)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $A9A0'
	ldy #169
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3722 || n = 3743)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
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
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 6
	inx
	Jne *-6
	tax
.elseif(n = 3725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3726)
	; 11 bytes
	brk
	delay_n (n-13)
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
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3732)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3737)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3739)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 3740)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
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
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
; 3751 to 3789
.if(n = 3751 || n = 3757 || n = 3769 || n = 3778 || n = 3787)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3752 || n = 3758 || n = 3773 || n = 3782 || n = 3788)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3753)
	; 10 bytes
	tya
	ldy #(n-3)/15 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
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
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3756)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3759 || n = 3771 || n = 3789)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3760)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 3761)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 3762 || n = 3779)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3764)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3766)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $ABA0'
	ldy #171
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3767 || n = 3785)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3774)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3775)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3776)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 3777)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 3780)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3781)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
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
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 3786)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790 || n = 3808)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3791)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3792)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3793)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3794 || n = 3798 || n = 3802)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3795)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3796)
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
	nop
	delay_n 9
	dey
	Jne *-5
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
.elseif(n = 3800)
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
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3804 || n = 3807)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3805)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3806)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.endif
.else
; 3809 to 3828
.if(n = 3809)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3810)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3811)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3812)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3813)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3814)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3815)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3816)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3817 || n = 3823 || n = 3826)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3820)
	; 12 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
	tax
.elseif(n = 3821)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3822)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $A6A0'
	ldy #166
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 3824)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3827)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3828)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	delay_n 7
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3830 || n = 3847 || n = 3864)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3831)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3832 || n = 3836 || n = 3841 || n = 3849 || n = 3859 || n = 3866)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3833 || n = 3855)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3834 || n = 3852)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3835)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3837)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3838)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3839 || n = 3857)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3840 || n = 3842 || n = 3850 || n = 3863)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3843 || n = 3862)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3844)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3845 || n = 3865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3846)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3848)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3851 || n = 3861)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3853)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3854)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3856)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3858)
	; 12 bytes
	brk
	tya
	ldy #(n-18)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3860)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
.elseif(n = 3867)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3871 || n = 3879 || n = 3889 || n = 3897)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3869 || n = 3890)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3870 || n = 3873 || n = 3888 || n = 3906)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3872 || n = 3878 || n = 3884 || n = 3894 || n = 3896 || n = 3901 || n = 3903)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3874)
	; 10 bytes
	tya
	ldy #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 3875 || n = 3893)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3876)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3877 || n = 3883 || n = 3887 || n = 3895 || n = 3904)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3880)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3881 || n = 3898)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3882)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3885 || n = 3905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3886)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3891)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3892)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3899)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 3900)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3902)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.endif
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
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3909)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3910 || n = 3914 || n = 3922 || n = 3926 || n = 3928 || n = 3935 || n = 3944)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911 || n = 3929)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3912)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3913 || n = 3917 || n = 3921 || n = 3927 || n = 3931 || n = 3934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3915 || n = 3932)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3916)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3918)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3919 || n = 3938)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3920)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3923)
	; 10 bytes
	tya
	ldy #(n-19)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3924)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3925 || n = 3945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3930)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 3933)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3936)
	; 11 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 3937)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3939)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3940)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	tay
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
.elseif(n = 3943)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
.elseif(n = 3947 || n = 3965 || n = 3983)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3948 || n = 3950 || n = 3968 || n = 3977 || n = 3979 || n = 3984)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3949 || n = 3966)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3951 || n = 3969)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3952)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3953 || n = 3974)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3954)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3955)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3956)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3957 || n = 3976)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3958 || n = 3959 || n = 3967 || n = 3975 || n = 3978)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3960)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3961)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
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
	; 9 bytes
	tya
	ldy #58 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 3964)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $B4A0'
	ldy #180
	brk
	dey
	Jne *-6
	tay
.elseif(n = 3970)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 3971)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3972)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3973)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3980)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
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
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985 || n = 4005)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3986 || n = 3998 || n = 4004 || n = 4006 || n = 4022)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 3987 || n = 3999 || n = 4007 || n = 4018 || n = 4023)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 3988)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3989)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3990 || n = 4008 || n = 4010)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 3991)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 3992)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3993)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3994)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 3995 || n = 4014)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3996)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3997 || n = 4002 || n = 4021)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4000 || n = 4017)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4001 || n = 4019)
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
.elseif(n = 4009)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4011)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	nop
	iny
	Jne *-6
	tay
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
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.if(n = 4024 || n = 4042 || n = 4060)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4025 || n = 4045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4026 || n = 4044)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4027 || n = 4039 || n = 4046 || n = 4047 || n = 4054 || n = 4057)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4028 || n = 4040 || n = 4048)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4029)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $B7A0'
	ldy #183
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4030)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4031)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4032 || n = 4050)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4033 || n = 4052)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 4037 || n = 4055)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4038 || n = 4041 || n = 4049 || n = 4056 || n = 4059)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4043 || n = 4061)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4053)
	; 11 bytes
	tya
	ldy #(n-5)/16
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4058)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4062)
	; 11 bytes
	pha
	delay_n (n-7)
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
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
.elseif(n = 4064 || n = 4070 || n = 4075 || n = 4087 || n = 4092 || n = 4093)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4066 || n = 4076 || n = 4082 || n = 4088 || n = 4094)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4067 || n = 4072 || n = 4077 || n = 4089 || n = 4095)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4068 || n = 4085)
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
	nop
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4071 || n = 4090)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4073 || n = 4091)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4074)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $B9A0'
	ldy #185
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4078 || n = 4096)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4079 || n = 4100)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4080 || n = 4098)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4081)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 4086)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4097)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4103 || n = 4108 || n = 4110 || n = 4112 || n = 4124 || n = 4130)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4104 || n = 4111 || n = 4129 || n = 4138)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4125)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4106 || n = 4113 || n = 4123 || n = 4131)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4107)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4109 || n = 4127)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4114 || n = 4132)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4115 || n = 4133)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4116 || n = 4134)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4117)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $BBA0'
	ldy #187
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4118)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4120)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	tax
.elseif(n = 4121)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4122)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4126)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4128)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4135)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 4139)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $BCA0'
	ldy #188
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4140)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4142)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.elseif(n = 4144 || n = 4155 || n = 4168 || n = 4172 || n = 4178)
	; 11..12 bytes
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
.elseif(n = 4146 || n = 4149 || n = 4151 || n = 4157 || n = 4167 || n = 4169 || n = 4174)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4147 || n = 4166)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4148 || n = 4156 || n = 4164 || n = 4173)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4150)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4152 || n = 4160 || n = 4177)
	; 11..12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4153 || n = 4170)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4154)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
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
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4162)
	; 11 bytes
	php
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4165)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4171)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
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
	nop
	delay_n 9
	iny
	Jne *-6
	tay
.endif
.else
.if((n >= 4180) && (n < 4199))
; 4180 to 4198
.if(n = 4180)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4181)
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
.elseif(n = 4183 || n = 4189)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4184)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4185)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 4187)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4188)
	; 11 bytes
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
.elseif(n = 4191)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4192)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4193)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4194)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4195)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4196)
	; 12 bytes
	tya
	ldy #126
	nop
	delay_n 26
	dey
	Jpl *-6
	tay
.elseif(n = 4197)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4198)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 9
	inx
	Jne *-7
	tax
.endif
.else
; 4199 to 4218
.if(n = 4199 || n = 4217)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4200 || n = 4212)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4201 || n = 4206)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4202)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4207)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4208 || n = 4209)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4210 || n = 4211)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4213)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $B7A0'
	ldy #183
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4214)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
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
.elseif(n = 4216)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 9
	iny
	Jne *-7
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
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4227 || n = 4231 || n = 4237 || n = 4240 || n = 4244)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4220 || n = 4241 || n = 4256)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4221 || n = 4238 || n = 4255)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4222)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4223 || n = 4242)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4224)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4225 || n = 4245)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4226 || n = 4247)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4228)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C0A0'
	ldy #192
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4229 || n = 4251)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4230 || n = 4248)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4232)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4233)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 4234)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4235 || n = 4253)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4236)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $B8A0'
	ldy #184
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4239 || n = 4254 || n = 4257)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4243)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4246)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
.elseif(n = 4249)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4250)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
	tay
.elseif(n = 4252)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.endif
.else
; 4258 to 4296
.if(n = 4258 || n = 4276 || n = 4294)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4259 || n = 4270 || n = 4275 || n = 4293)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4260 || n = 4278 || n = 4296)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4261 || n = 4280)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4262)
	; 11 bytes
	tya
	ldy #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
.elseif(n = 4263 || n = 4267 || n = 4273 || n = 4282 || n = 4286 || n = 4287 || n = 4291)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4264 || n = 4269 || n = 4274 || n = 4288 || n = 4292)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4265 || n = 4285)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4266)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
	tax
.elseif(n = 4268)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4271 || n = 4289)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4272)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4277)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4279)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4281)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $BAA0'
	ldy #186
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4283)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4284)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4290)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4295)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297 || n = 4333)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4298)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4299 || n = 4318)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4300)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $B3A0'
	ldy #179
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4301 || n = 4308 || n = 4309 || n = 4320 || n = 4327)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4302)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4303 || n = 4311 || n = 4322 || n = 4329 || n = 4335)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4304)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4305)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4306)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4307 || n = 4325)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4310 || n = 4331)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4312)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 4314)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4315)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4316)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4317)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4319)
	; 11 bytes
	tya
	ldy #90 ;hides 'nop'
	delay_n 6
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4321 || n = 4326 || n = 4328 || n = 4334)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4323)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4324)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4330 || n = 4332)
	; 11 bytes
	delay_n (n-4)
	pla
.endif
.else
; 4336 to 4375
.if(n = 4336 || n = 4338 || n = 4374)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4337 || n = 4356 || n = 4375)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4339)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4341 || n = 4344 || n = 4347 || n = 4349 || n = 4367 || n = 4369)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4342 || n = 4353 || n = 4354 || n = 4358)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4343 || n = 4361)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4345 || n = 4365)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4346 || n = 4362 || n = 4364 || n = 4366)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4348)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4350 || n = 4368 || n = 4370)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4351)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4352)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.elseif(n = 4359)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4360)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4363)
	; 9 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4371)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4372)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4373)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
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
.if(n = 4376)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4377 || n = 4381 || n = 4387 || n = 4396 || n = 4399 || n = 4407)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4378 || n = 4382 || n = 4388 || n = 4400 || n = 4406 || n = 4408 || n = 4414)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4379 || n = 4397)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4380)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4383 || n = 4389 || n = 4398 || n = 4401 || n = 4409)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4384)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4385 || n = 4405)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4386 || n = 4412)
	; 11..12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4390)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 4391)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	pha
	dey
	Jne *-6
	tay
.elseif(n = 4392 || n = 4410)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4393)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 4394 || n = 4413)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 4402)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4404)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4411)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4433 || n = 4451)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4416 || n = 4418 || n = 4452)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4417 || n = 4434 || n = 4435 || n = 4447 || n = 4453)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4419 || n = 4429 || n = 4437)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4420 || n = 4438)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4421 || n = 4444)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4422)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4423 || n = 4441)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4424)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4425 || n = 4445)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4426)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4427)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4428 || n = 4446)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4430)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4431)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
	tax
.elseif(n = 4432)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4436)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4439)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4440)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4442)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4443)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 4448)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4450)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 9
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4460 || n = 4468 || n = 4472 || n = 4479 || n = 4481 || n = 4483 || n = 4488 || n = 4490)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4455 || n = 4461 || n = 4473 || n = 4484)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4456 || n = 4474 || n = 4492)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4457 || n = 4478)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4458)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4459 || n = 4477)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4462 || n = 4464 || n = 4482)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4463)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	iny
	iny
	Jne *-7
	tay
.elseif(n = 4465 || n = 4485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4466)
	; 11 bytes
	tya
	ldy #(n-4)/23 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	brk
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
.elseif(n = 4469 || n = 4487)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4470 || n = 4489)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4471)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4475)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4476)
	; 11 bytes
	tya
	ldy #(n-24)/21 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4480)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
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
.elseif(n = 4491)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
	tay
.endif
.else
; 4493 to 4531
.if(n = 4493 || n = 4515)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4494 || n = 4530)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4495)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4496 || n = 4512)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4497)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4498)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4499 || n = 4520)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4500 || n = 4531)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4501 || n = 4507 || n = 4510 || n = 4516 || n = 4522 || n = 4529)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4502 || n = 4511 || n = 4526)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4505 || n = 4523)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 4508 || n = 4527)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4509)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4513)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 4514)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4517)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4518)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4519)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts
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
.elseif(n = 4524)
	; 11 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	delay_n 26
	Jne *-6
	tay
.elseif(n = 4525)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4528)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.endif
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4534 || n = 4552 || n = 4553 || n = 4566)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4533 || n = 4538 || n = 4543 || n = 4547 || n = 4548 || n = 4556 || n = 4561)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4535)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4536 || n = 4554)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4537)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4539)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4540)
	; 11 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 4541 || n = 4559)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4542 || n = 4544 || n = 4560 || n = 4568)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4545)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4546 || n = 4565)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4549 || n = 4550 || n = 4563 || n = 4569)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4551 || n = 4570)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4555)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4557)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 16
	iny
	Jne *-5
	tay
.elseif(n = 4558)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4562)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4564)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4567)
	; 11 bytes
	tya
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	nop
.endif
.else
; 4571 to 4609
.if(n = 4571 || n = 4596)
	; 11..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4572)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4573)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4574 || n = 4579 || n = 4586 || n = 4589 || n = 4593 || n = 4597 || n = 4604)
	; 9..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4577 || n = 4595)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4578)
	; 12 bytes
	brk
	tya
	ldy #(n-18)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4580 || n = 4587 || n = 4598 || n = 4605)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4581)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4582 || n = 4600)
	; 11 bytes
	php
	sec
	lda #(n-10)/18
	brk
	sbc #1
	Jne *-4
	plp
.elseif(n = 4583)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4584)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 4585)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4588 || n = 4599 || n = 4606)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4590 || n = 4608)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4591 || n = 4594 || n = 4609)
	; 10..11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4592)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4601)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4602)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 4603)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	brk
	iny
	Jne *-4
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
.endif
.endif
.else
.if((n >= 4610) && (n < 4649))
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if(n = 4610 || n = 4621)
	; 10..11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4611 || n = 4615 || n = 4627)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4620)
	; 11 bytes
	delay_n (n-4)
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
.elseif(n = 4616)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4617)
	; 10 bytes
	pla
	delay_n (n-4)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 4622)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4623)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4624)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 4625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4626)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4628)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.endif
.else
; 4629 to 4648
.if(n = 4629 || n = 4647)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4630)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4631 || n = 4632)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4633)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4634)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dey
	Jne *-6
	tay
.elseif(n = 4635)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4636)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4637)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4638)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4639)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 4640)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 4641)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4642 || n = 4644)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4643)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 4645)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4646)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.endif
.else
.if((n >= 4649) && (n < 4668))
; 4649 to 4667
.if(n = 4649 || n = 4664)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4650 || n = 4652)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4651)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4653)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4654)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 4655)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4656)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4657)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	inx
	inx
	Jne *-7
	tax
.elseif(n = 4658 || n = 4662)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4659 || n = 4663)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4660)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4661)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4665)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4666)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4667)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
.endif
.else
; 4668 to 4687
.if(n = 4668)
	; 11 bytes
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	brk
	inx
	Jne *-6
	tax
.elseif(n = 4669)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4670)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4671)
	; 11 bytes
	pla
	tya
	ldy #222
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4672)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-5
	tax
.elseif(n = 4673)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4674)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4675 || n = 4682 || n = 4686)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4676)
	; 11 bytes
	php
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4677)
	; 12 bytes
	tya
	ldy #146
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 4678 || n = 4681)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4679)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4684)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4687)
	; 10 bytes
	pla
	delay_n (n-4)
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
.if(n = 4688 || n = 4709)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4689)
	; 10 bytes
	tya
	ldy #(n-25)/22 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4690)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 4691 || n = 4713)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4692 || n = 4693 || n = 4702 || n = 4721)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4694)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4695)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4696)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4697 || n = 4720)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4698 || n = 4717)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4699 || n = 4700 || n = 4707 || n = 4714 || n = 4719)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4701 || n = 4708)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4703)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4704)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4705 || n = 4725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4706)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4710)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4711)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4712)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4715)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4716)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 10
	inx
	Jne *-7
	tax
.elseif(n = 4718)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4722 || n = 4724)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
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
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4732 || n = 4737 || n = 4738 || n = 4747)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4728 || n = 4739 || n = 4754 || n = 4758)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4729 || n = 4740 || n = 4749 || n = 4759 || n = 4761)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4730 || n = 4751)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4731)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $3BA0'
	ldy #59
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 4733)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4734)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4735 || n = 4757)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4736 || n = 4755)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4741 || n = 4760)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4742)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4743)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4744)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4745 || n = 4765)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4746)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4748)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4750)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4752)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4753)
	; 10 bytes
	tya
	ldy #(n-3)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4756)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4762)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4763)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4764)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
.if((n >= 4766) && (n < 4785))
; 4766 to 4784
.if(n = 4766)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4767 || n = 4776 || n = 4782)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4768 || n = 4775)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4769)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4770)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4771)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4772)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4773)
	; 12 bytes
	tya
	ldy #149
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 4774)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4777)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4778)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4779)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4780)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4781)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4783)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4784)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.endif
.else
; 4785 to 4804
.if(n = 4785)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4786 || n = 4794)
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
.elseif(n = 4787 || n = 4795)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4788 || n = 4796 || n = 4802)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4789)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4790)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4791)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4792)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4793)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4797)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4798)
	; 12 bytes
	php
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4799)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4800)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4801)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4803)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4804)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4806 || n = 4819 || n = 4832 || n = 4840)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4807 || n = 4824 || n = 4827)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4808 || n = 4815 || n = 4817 || n = 4826 || n = 4828 || n = 4834 || n = 4836 || n = 4838 || n = 4842)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4809 || n = 4816 || n = 4818 || n = 4829 || n = 4837 || n = 4839)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4810 || n = 4841)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4811)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4812 || n = 4831)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4813)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4814 || n = 4835)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.elseif(n = 4821)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4822)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4823)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4830)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4833)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4843)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	brk
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4845 || n = 4865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4846 || n = 4847 || n = 4852 || n = 4871)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4848 || n = 4853 || n = 4859 || n = 4861 || n = 4868 || n = 4878)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4849 || n = 4860 || n = 4862 || n = 4873)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4850)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4851)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $36A0'
	ldy #54
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
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
.elseif(n = 4856 || n = 4877)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4857 || n = 4882)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4858 || n = 4881)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
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
.elseif(n = 4864)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4866)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 4867)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4869)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4870)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4872)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4874)
	; 12 bytes
	php
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4875)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4876)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4879)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4884 || n = 4908 || n = 4914 || n = 4920)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4885 || n = 4905)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4886 || n = 4894 || n = 4916)
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
.elseif(n = 4887 || n = 4891 || n = 4900 || n = 4906 || n = 4907 || n = 4913)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4888)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4889 || n = 4911)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4890)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4892)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4893)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4895)
	; 12 bytes
	tya
	ldy #163
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4896)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4897)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 4898 || n = 4919)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4899)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4901)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4902)
	; 11 bytes
	tya
	ldy #(n-26)/23 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 4903)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4904)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4909)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4910)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4912)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4915 || n = 4921)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4917)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4918)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922 || n = 4928 || n = 4936 || n = 4943 || n = 4948)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4923)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 4924 || n = 4952)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4925 || n = 4945)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4926)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4927 || n = 4935 || n = 4942 || n = 4947 || n = 4956 || n = 4957)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4929 || n = 4937 || n = 4944 || n = 4949)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 4930)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4931)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4932)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4933)
	; 11 bytes
	php
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 4934)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4938)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4939)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4940)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.elseif(n = 4946)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 4950)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4951)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4953)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4954)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	brk
	dey
	Jne *-6
	tay
.elseif(n = 4955)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4958)
	; 12 bytes
	brk
	tya
	ldy #(n-18)/20
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4959)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.elseif(n = 4960)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.else
.if((n >= 4961) && (n < 4981))
; 4961 to 4980
.if(n = 4961)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4962)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
.elseif(n = 4963)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4964 || n = 4966 || n = 4974)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 4965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4967 || n = 4969)
	; 10..11 bytes
	pla
	delay_n (n-4)
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
	delay_n (n-4)
	pla
.elseif(n = 4971)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 4972)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4973)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 4975)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4976)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4977)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 4978)
	; 11 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 16
	dey
	dey
	Jne *-6
	tay
.elseif(n = 4979)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4980)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
.endif
.else
; 4981 to 5000
.if(n = 4981)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4982)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4983 || n = 4988)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4984 || n = 4987)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4986 || n = 4989)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 4990)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 4991)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
.elseif(n = 4992)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 4993)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4994)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 4995)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 4996)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 4997)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
	tax
.elseif(n = 4998)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	brk
	iny
	Jne *-6
	tay
.elseif(n = 4999)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5000)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
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
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
.if((n >= 5001) && (n < 5313))
.if((n >= 5001) && (n < 5157))
.if((n >= 5001) && (n < 5079))
.if((n >= 5001) && (n < 5040))
.if((n >= 5001) && (n < 5020))
; 5001 to 5019
.if(n = 5001)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5002)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5003)
	; 9 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 5004)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5005)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5006 || n = 5014)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5007 || n = 5009)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5008)
	; 11 bytes
	php
	sec
	lda #250 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	plp
.elseif(n = 5010)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 5011 || n = 5013)
	; 11..12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5012)
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
.elseif(n = 5016)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5017)
	; 10 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5018)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5019)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	brk
	iny
	Jne *-6
	tay
.endif
.else
; 5020 to 5039
.if(n = 5020)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5021)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5022)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5023)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5024)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5025)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5026 || n = 5027)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5028)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5029 || n = 5034)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5030)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5031)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5032)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5033)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5035 || n = 5036 || n = 5039)
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
.elseif(n = 5037)
	; 12 bytes
	tya
	ldy #148
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 5038)
	; 12 bytes
	brk
	tya
	ldy #(n-18)/20
	nop
	brk
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 5040) && (n < 5059))
; 5040 to 5058
.if(n = 5040)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5041)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5042)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5043)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5044)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5046)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5047 || n = 5057)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5048 || n = 5056)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5049)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5050)
	; 12 bytes
	php
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	plp
.elseif(n = 5051)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5052)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5053)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
.elseif(n = 5054)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5055)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5058)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.endif
.else
; 5059 to 5078
.if(n = 5059 || n = 5069 || n = 5070)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5060 || n = 5067)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5061)
	; 12 bytes
	tya
	ldy #158
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5062)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5063)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5064)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5065)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5066)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5068)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5071)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5072)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5073 || n = 5074)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5075)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5076)
	; 12 bytes
	brk
	tya
	ldy #26 ;hides 'nop'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 5077)
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
.elseif(n = 5078)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	tax
.endif
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5080)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5081 || n = 5083 || n = 5097 || n = 5110 || n = 5117)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5082 || n = 5084 || n = 5095 || n = 5098 || n = 5112)
	; 10..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5085 || n = 5105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5086)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	brk
	inx
	Jne *-6
	tax
.elseif(n = 5087 || n = 5108)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5088 || n = 5111)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
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
.elseif(n = 5090)
	; 11 bytes
	tya
	ldy #(n-2)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5091)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5092)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	iny
	Jne *-6
	tay
.elseif(n = 5093)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
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
.elseif(n = 5096 || n = 5099 || n = 5115 || n = 5116)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5100)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5101 || n = 5104)
	; 11..12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5102)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5103)
	; 11 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 5106)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5107)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5109)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5113)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 5114)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	dey
	Jne *-4
	tay
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5133 || n = 5145)
	; 10..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5119)
	; 11 bytes
	php
	sec
	lda #0 ;hides 'brk imm'
	nop
	nop
	sbc #1
	Jne *-5
	plp
.elseif(n = 5120)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5121)
	; 11 bytes
	delay_n (n-4)
	pla
.elseif(n = 5122)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5123 || n = 5127 || n = 5136 || n = 5143)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5124 || n = 5128 || n = 5132 || n = 5137 || n = 5144 || n = 5154)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5125)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5126 || n = 5142)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5129 || n = 5150)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5134)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5135)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5138)
	; 12 bytes
	brk
	tya
	ldy #0
	nop
	brk
	dey
	Jne *-4
	tay
.elseif(n = 5139)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5140)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5141)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5146 || n = 5147)
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
.elseif(n = 5148)
	; 11 bytes
	tya
	ldy #(n-24)/21 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5149)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5151)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5152)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5156)
	; 12 bytes
	php
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
.if((n >= 5157) && (n < 5176))
; 5157 to 5175
.if(n = 5157)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5158)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5159 || n = 5167)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5160 || n = 5168)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5161)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5162)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5163)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5164)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5165)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5166 || n = 5172)
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
.elseif(n = 5169)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 5170)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5171)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5173)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 5174)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5175)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.else
; 5176 to 5195
.if(n = 5176 || n = 5185 || n = 5193)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5177 || n = 5188)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5178)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5179)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5180)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5181)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5182)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5183)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5184)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 5186 || n = 5194)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5187)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5189)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5190)
	; 11 bytes
	php
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5191)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5192)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5195)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
	tay
.endif
.endif
.else
; 5196 to 5234
.if(n = 5196)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5197 || n = 5219)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5198 || n = 5206 || n = 5214 || n = 5216 || n = 5220 || n = 5222 || n = 5229)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5199 || n = 5215 || n = 5221)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5200)
	; 12 bytes
	php
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	brk
	adc #1
	Jne *-6
	plp
.elseif(n = 5201)
	; 11 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5202)
	; 11 bytes
	tya
	ldy #(n-4)/23 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5203 || n = 5226)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5204)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5205 || n = 5230)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5207 || n = 5228)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5208)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5209)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	delay_n 26
	Jne *-7
	tay
.elseif(n = 5210 || n = 5227)
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
.elseif(n = 5211)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5212)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	tax
.elseif(n = 5213 || n = 5234)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5217)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 5218)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5223)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5224)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5225)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5231)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5232)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5233)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
.if((n >= 5235) && (n < 5254))
; 5235 to 5253
.if(n = 5235)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5236 || n = 5242 || n = 5246 || n = 5251)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5237)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5238 || n = 5245)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5239)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5240)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5241)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5243)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5244)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5247)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5248)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5249)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5250)
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
.elseif(n = 5252)
	; 12 bytes
	tya
	ldy #159
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5253)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.endif
.else
; 5254 to 5273
.if(n = 5254)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5255)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5256 || n = 5258)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 5257 || n = 5259 || n = 5267)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5260)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 5261)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5262)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5263)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5264 || n = 5265)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5266)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5268)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5269)
	; 12 bytes
	tya
	ldy #188
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5270)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5271)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5272)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5273)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	pha
	iny
	Jne *-6
	tay
.endif
.endif
.else
; 5274 to 5312
.if(n = 5274)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5275 || n = 5306)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5276 || n = 5297)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5277 || n = 5278 || n = 5286 || n = 5287 || n = 5299)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5279 || n = 5296 || n = 5298 || n = 5300 || n = 5304 || n = 5312)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5280 || n = 5305)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5281)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
.elseif(n = 5282)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5283)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5284)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5285 || n = 5307)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5288 || n = 5310)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5289)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5290 || n = 5291)
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
.elseif(n = 5292)
	; 11 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	dey
	Jne *-6
	tay
.elseif(n = 5293)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 16
	iny
	Jne *-5
	tay
.elseif(n = 5294)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5295)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5301 || n = 5311)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5302)
	; 11 bytes
	tya
	ldy #(n-2)/25 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5303)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop zp,x'
	nop
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 5308)
	; 11 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 5309)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if(n = 5313 || n = 5322 || n = 5334 || n = 5343)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5314 || n = 5348)
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
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5317)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5318 || n = 5339)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5319 || n = 5321 || n = 5336 || n = 5342 || n = 5344 || n = 5346)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5320 || n = 5323 || n = 5331 || n = 5351)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5324)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5325)
	; 12 bytes
	tya
	ldy #190
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5326 || n = 5347)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5327)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	tax
.elseif(n = 5328)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	tax
.elseif(n = 5329)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5330)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5332)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5333)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5335)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5337)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5338)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5340)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 5341)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
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
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	pha
	iny
	Jne *-4
	tay
.elseif(n = 5350)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	brk
	iny
	Jne *-6
	tay
.endif
.else
; 5352 to 5390
.if(n = 5352 || n = 5356 || n = 5373 || n = 5377 || n = 5384 || n = 5388 || n = 5390)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5353 || n = 5374 || n = 5389)
	; 10..11 bytes
	pla
	delay_n (n-4)
.elseif(n = 5354 || n = 5362 || n = 5366 || n = 5372 || n = 5375 || n = 5383)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5355 || n = 5380)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5357)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5358)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5359)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5360)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 5361)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5363)
	; 11 bytes
	pha
	tya
	ldy #(n-8)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5364 || n = 5387)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5365)
	; 12 bytes
	php
	sec
	lda #244 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
	plp
.elseif(n = 5367)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5368)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	iny
	Jne *-4
	tay
.elseif(n = 5369)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 5370)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 5371)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5376)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5378)
	; 11 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5379)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5381)
	; 10 bytes
	tya
	ldy #0
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5382)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5385)
	; 10 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5386)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391 || n = 5397 || n = 5414 || n = 5421)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5392 || n = 5396 || n = 5398 || n = 5402 || n = 5411 || n = 5420)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5393)
	; 10 bytes
	tya
	ldy #(n-25)/22 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5394)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5395 || n = 5417)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5399)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5400)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5401 || n = 5407 || n = 5412 || n = 5418 || n = 5419)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5403)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5404)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5405)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5406)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5408)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5409)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5410)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5413)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5415)
	; 11 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	brk
	inx
	Jne *-6
	tax
.elseif(n = 5416)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	brk
	iny
	Jne *-6
	tay
.elseif((n > 5421 && n < 5426))
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
.elseif(n = 5426)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 5427)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5428)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5429)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.endif
.else
; 5430 to 5468
.if(n = 5430)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5431)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5432)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5433 || n = 5456)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5434 || n = 5436 || n = 5442 || n = 5450 || n = 5462)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5435)
	; 12 bytes
	tya
	ldy #181
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5437)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5438)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5439 || n = 5461)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5440 || n = 5441 || n = 5449 || n = 5457 || n = 5458)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5443 || n = 5451 || n = 5463)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5444 || n = 5446 || n = 5448 || n = 5468)
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
.elseif(n = 5445)
	; 12 bytes
	tya
	ldy #170
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5447)
	; 11 bytes
	php
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5452)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5453)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 5454)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
.elseif(n = 5455)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5459)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5460)
	; 11 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	brk
	inx
	Jne *-6
	tax
.elseif(n = 5464)
	; 12 bytes
	php
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5465)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5466)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5467)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
.if((n >= 5469) && (n < 5488))
; 5469 to 5487
.if((n > 5468 && n < 5472) || n = 5473 || n = 5474)
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
.elseif(n = 5472)
	; 12 bytes
	brk
	tya
	ldy #248 ;hides 'sed'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5475)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5476)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5477)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5478)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5479)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5480)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5481)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5482)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5483)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5484)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tax
.elseif(n = 5485)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5486)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5487)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.endif
.else
; 5488 to 5507
.if(n = 5488 || n = 5496 || n = 5507)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5489)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5490 || n = 5497 || n = 5498)
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
.elseif(n = 5491)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5492)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5493)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	nop
	iny
	Jne *-7
	tay
.elseif(n = 5494)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5495)
	; 12 bytes
	tya
	ldy #183
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5499)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5500)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5501)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5502)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5503)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5504)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5505)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5506)
	; 11 bytes
	pha
	delay_n (n-3)
.endif
.endif
.else
; 5508 to 5546
.if(n = 5508)
	; 12 bytes
	php
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	plp
.elseif(n = 5509)
	; 12 bytes
	tya
	ldy #172
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5510)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5511)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5512 || (n > 5517 && n < 5521) || n = 5522 || (n > 5534 && n < 5538) || n = 5539 || n = 5542 || n = 5544 || n = 5545)
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
.elseif(n = 5513 || n = 5540)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5514)
	; 12 bytes
	tya
	ldy #(n-2)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5515)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.elseif(n = 5517 || n = 5543)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5521)
	; 12 bytes
	tya
	ldy #197
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5523)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5524)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5525)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5526)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5527)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5528 || n = 5529 || n = 5532)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5530)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5531)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5533)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5534)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5538)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5541)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5546)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if(n = 5547)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
.elseif(n = 5548)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5549 || n = 5571)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5550 || n = 5551 || n = 5556 || n = 5557)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5552 || n = 5574 || n = 5576 || n = 5583)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5553)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5554 || n = 5585)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5555 || n = 5580)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5558 || n = 5559 || n = 5575 || n = 5584)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif((n > 5559 && n < 5567) || n = 5568)
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
.elseif(n = 5567)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5569)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5570)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5572)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5573)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5577)
	; 12 bytes
	tya
	ldy #199
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5578)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5579)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5581)
	; 12 bytes
	tya
	ldy #164
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 5582)
	; 12 bytes
	tya
	ldy #169
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.endif
.else
; 5586 to 5625
.if(n = 5586 || n = 5590 || n = 5602)
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
.elseif(n = 5587 || n = 5599 || n = 5606 || n = 5618)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5588 || n = 5600 || n = 5607 || n = 5622)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5589 || n = 5598 || n = 5601 || n = 5608 || n = 5609 || n = 5623)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5591)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5592)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5593 || n = 5615)
	; 11 bytes
	tya
	ldy #(n-5)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5594 || n = 5617)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5595)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5596)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5597 || n = 5621)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5603)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5604)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	nop
	iny
	Jne *-5
	tay
.elseif(n = 5605)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5610)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 5611)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5612)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5613)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5614)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5616)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5619)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5620)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5624)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
.elseif(n = 5625)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
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
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 5627 || n = 5628 || n = 5639 || n = 5642 || n = 5649 || n = 5657 || n = 5658)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5629 || n = 5646 || n = 5650)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5630 || n = 5655)
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
	nop
	nop
	nop
	sbc #1
	Jne *-6
	plp
.elseif(n = 5632)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5633)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 5634)
	; 12 bytes
	txa
	pha
	ldx #0 ;hides 'brk imm'
	inc $06B5,X ;hides 'lda $06,X'
	dex
	Jne *-5
	pla
	tax
.elseif(n = 5635)
	; 11 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5636)
	; 11 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5637)
	; 11 bytes
	tya
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5638)
	; 11 bytes
	tya
	ldy #(n-26)/23 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5640 || n = 5663)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5641 || n = 5651 || n = 5659 || n = 5660)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5643)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5644)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5645)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5647)
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
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5652 || n = 5664)
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
.elseif(n = 5653)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48 ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5654)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5656)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
.elseif(n = 5661)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
.elseif(n = 5662)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.endif
.else
.if((n >= 5665) && (n < 5684))
; 5665 to 5683
.if(n = 5665 || n = 5682)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5666)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5667)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5668)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5669)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5670 || n = 5672 || n = 5674)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5671)
	; 11 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 6
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5673)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5675)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5676 || n = 5677)
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
.elseif(n = 5678)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5679)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5680)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
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
.elseif(n = 5683)
	; 12 bytes
	tya
	ldy #167
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.else
; 5684 to 5703
.if(n = 5684)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5685)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5686)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5687)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	brk
	Jne *-6
	tax
.elseif(n = 5688)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5689 || n = 5694 || n = 5696)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5690 || n = 5695 || n = 5697)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5691)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
.elseif(n = 5692)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5693)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5698)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5699)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5700)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5701)
	; 12 bytes
	tya
	ldy #178
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5702)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5703)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 16
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
.if((n >= 5704) && (n < 5723))
; 5704 to 5722
.if(n = 5704)
	; 11 bytes
	php
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5705)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5706 || n = 5719)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5707)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5708 || n = 5711 || n = 5721)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5709)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5710 || n = 5720)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5712)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5713)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5714)
	; 12 bytes
	tya
	ldy #173
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5715)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5716)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5717)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5718)
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
.elseif(n = 5722)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.else
; 5723 to 5742
.if(n = 5723)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5724)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5725)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5726)
	; 12 bytes
	tya
	ldy #(n-2)/27 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5727)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop zp,x'
	nop
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5728)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5729)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5730)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5731)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5732)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5733)
	; 12 bytes
	tya
	ldy #179
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5734 || n = 5742)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5735)
	; 12 bytes
	tya
	ldy #191
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5736)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5737 || n = 5738)
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
.elseif(n = 5739)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5740)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5741)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 5743 to 5781
.if(n = 5743 || n = 5756 || n = 5760 || n = 5768 || n = 5781)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5744 || n = 5757 || n = 5769 || n = 5775)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5745)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5746 || n = 5748 || n = 5749 || n = 5752 || n = 5762 || n = 5770 || n = 5772)
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
.elseif(n = 5747)
	; 12 bytes
	tya
	ldy #174
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5750)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 5751 || n = 5777)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5753)
	; 10 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5754)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5755 || n = 5780)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5758)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5759)
	; 12 bytes
	pla
	tya
	ldy #(n-9)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5761)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 5763)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 5764)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5765)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5766)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 5767 || n = 5773)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5771)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 5776)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5778)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5779)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 16
	inx
	Jne *-7
	tax
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
; 5782 to 5820
.if(n = 5782 || n = 5791 || n = 5804 || n = 5807 || n = 5815)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5783 || n = 5810)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5784 || n = 5793 || n = 5809 || n = 5817)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5785)
	; 12 bytes
	tya
	ldy #170
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 5786 || n = 5790 || n = 5794 || n = 5796 || n = 5798 || n = 5814 || n = 5818 || n = 5820)
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
.elseif(n = 5787)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5788)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5789 || n = 5813)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5792 || n = 5806 || n = 5808 || n = 5816)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5795)
	; 12 bytes
	tya
	ldy #193
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5797)
	; 12 bytes
	tya
	ldy #181
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5799)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5800)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5801)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5802)
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
	delay_n 18
	iny
	Jne *-6
	tay
.elseif(n = 5805)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5811)
	; 12 bytes
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5812)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5819)
	; 12 bytes
	tya
	ldy #171
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.else
; 5821 to 5859
.if(n = 5821 || n = 5831 || (n > 5841 && n < 5845))
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
.elseif(n = 5822)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5823)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5824 || n = 5847)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
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
.elseif(n = 5826 || n = 5832 || n = 5849 || n = 5850 || n = 5857)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5827 || n = 5838 || n = 5840 || n = 5856)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5828 || n = 5834 || n = 5839 || n = 5841 || n = 5851 || n = 5852)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5829)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5830 || n = 5855)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5833)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5835)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5836)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5837)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5845)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5846)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5848)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
.elseif(n = 5853)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 5854)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5858)
	; 11 bytes
	tya
	ldy #(n-2)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5859)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	pha
	iny
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5861 || n = 5885)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5862 || n = 5867 || n = 5873 || n = 5888 || n = 5896 || n = 5898)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5863 || n = 5865 || n = 5874 || n = 5886 || n = 5887)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5864 || n = 5895)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5866 || n = 5872 || n = 5884 || n = 5897)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5868)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5869)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 5870)
	; 11 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5871 || n = 5875 || n = 5876)
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
.elseif(n = 5877)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $4848 ;hides two 'pha's
	brk
	dey
	Jne *-7
	tay
.elseif(n = 5878)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 5879)
	; 12 bytes
	tya
	ldy #178
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5880)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 5881)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 5882)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	pha
	dey
	Jne *-5
	tay
.elseif(n = 5883)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5889)
	; 11 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5890)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	jsr @nearby_rts
	Jne *-7
	tax
.elseif(n = 5891)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5892)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	iny
	Jne *-6
	tay
.elseif(n = 5893)
	; 11 bytes
	tya
	ldy #0
	delay_n 18
	dey
	Jne *-5
	tay
.elseif(n = 5894)
	; 12 bytes
	brk
	delay_n (n-13)
.endif
.else
; 5899 to 5937
.if(n = 5899 || n = 5934 || n = 5937)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5900 || n = 5910 || n = 5916 || n = 5920 || (n > 5921 && n < 5925) || n = 5927)
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
.elseif(n = 5901)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5902)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5903)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 5904)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5905 || n = 5930)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5906 || n = 5929 || n = 5936)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5907)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5908)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5909 || n = 5933)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5911 || n = 5935)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5912)
	; 12 bytes
	tya
	ldy #179
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5913)
	; 12 bytes
	tya
	ldy #211
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5914)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
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
.elseif(n = 5917)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 6
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5918)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5919)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5921)
	; 12 bytes
	tya
	ldy #174
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 5925)
	; 12 bytes
	tya
	ldy #185
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5926)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5928)
	; 11 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 5931)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5932)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
; 5938 to 5976
.if(n = 5938)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5939)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	delay_n 6
	brk
	dey
	Jne *-7
	tay
.elseif(n = 5940)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $4848 ;hides two 'pha's
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5941)
	; 12 bytes
	tya
	ldy #212
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5942)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5943)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 10
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5944)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 6
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5945 || n = 5972)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5946 || n = 5947 || (n > 5948 && n < 5953) || n = 5962 || n = 5966 || n = 5968 || n = 5970 || n = 5973 || n = 5974 || n = 5976)
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
.elseif(n = 5948)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
.elseif(n = 5953)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5954)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 5955)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 5956)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
.elseif(n = 5957)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5958)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5959)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5960 || n = 5964)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5961)
	; 11 bytes
	php
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 5963)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5965)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5967)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	brk
	dey
	Jne *-7
	tay
.elseif(n = 5969)
	; 12 bytes
	tya
	ldy #213
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5971)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 6
	brk
	iny
	Jne *-7
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
.endif
.else
; 5977 to 6015
.if(n = 5977 || (n > 5992 && n < 5997) || n = 5998 || (n > 5999 && n < 6003) || n = 6010 || (n > 6012 && n < 6016))
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
.elseif(n = 5978)
	; 12 bytes
	tya
	ldy #181
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 5979)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 5980 || n = 6005)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5981)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5982 || n = 5990 || n = 6007)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5983)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 6
	brk
	iny
	Jne *-6
	tay
.elseif(n = 5984 || n = 5986 || n = 5991 || n = 6008)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 5985 || n = 6011)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 5987 || n = 5992)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 5988)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 5989)
	; 12 bytes
	tya
	ldy #187
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 5997)
	; 12 bytes
	tya
	ldy #214
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 5999)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6003)
	; 11 bytes
	tya
	ldy #(n-3)/24 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6004)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6006)
	; 12 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6009)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
	tax
.elseif(n = 6012)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
; 6016 to 6054
.if((n > 6015 && n < 6019) || n = 6020 || n = 6024 || n = 6036 || (n > 6039 && n < 6044) || (n > 6044 && n < 6050))
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
.elseif(n = 6019 || n = 6050)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6021 || n = 6032)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6022 || n = 6033 || n = 6038)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6023)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 6025)
	; 12 bytes
	tya
	ldy #215
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6026)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6027)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6028)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6029 || n = 6053)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6030)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6031)
	; 12 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 39
	Jne *-7
	tay
.elseif(n = 6034 || n = 6039 || n = 6054)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6035)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	brk
	iny
	Jne *-6
	tay
.elseif(n = 6037)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 6051)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6052)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.endif
.else
; 6055 to 6093
.if(n = 6055 || n = 6080)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6056 || n = 6084 || n = 6090)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6057 || n = 6079 || n = 6082 || n = 6083)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6058)
	; 12 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 6059)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif((n > 6059 && n < 6063) || n = 6064 || (n > 6065 && n < 6075) || n = 6086 || n = 6088)
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
.elseif(n = 6063 || n = 6089)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
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
.elseif(n = 6075)
	; 11 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6076)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6077)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6078)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 6081)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6085)
	; 12 bytes
	tya
	ldy #190
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6087)
	; 11 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 6
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6091)
	; 12 bytes
	tya
	ldy #179
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6092)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	pha
	dey
	Jne *-6
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
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if(n = 6094 || n = 6116 || n = 6119 || n = 6127 || n = 6132)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6095)
	; 12 bytes
	tya
	ldy #203
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6096)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 6097 || n = 6118 || n = 6129)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6098 || n = 6111 || n = 6126)
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
.elseif(n = 6099)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6100)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6101 || n = 6125)
	; 11 bytes
	tya
	ldy #(n-5)/24
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6102)
	; 11 bytes
	tya
	ldy #(n-2)/25 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6103)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop zp,x'
	nop
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 6104 || n = 6106 || n = 6108 || n = 6117 || n = 6121 || n = 6131)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6105 || n = 6130)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6107)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6109)
	; 12 bytes
	tya
	ldy #218
	delay_n 10
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
.elseif(n = 6112)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6113)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6114)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 6115)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6120)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6122)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6123)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6124)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6128)
	; 11 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 16
	dey
	Jne *-6
	tay
.endif
.else
; 6133 to 6171
.if(n = 6133 || n = 6139 || n = 6141 || n = 6152 || n = 6158)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6134)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	dey
	Jne *-5
	tay
.elseif(n = 6135 || n = 6156 || n = 6160 || n = 6162 || n = 6163 || n = 6166 || (n > 6167 && n < 6172))
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
.elseif(n = 6136)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 6137)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 6138)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 6140 || n = 6145 || n = 6157)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6142)
	; 11 bytes
	pla
	delay_n (n-4)
.elseif(n = 6143)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6144)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6146)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6147)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6148)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6149)
	; 11 bytes
	tya
	ldy #0
	delay_n 6
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6150)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 6151)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6153)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	iny
	Jne *-7
	tay
.elseif(n = 6154)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6155)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6159)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6161)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6164)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6165)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6167)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172 || n = 6173 || n = 6175 || n = 6186 || n = 6187 || (n > 6188 && n < 6192) || (n > 6193 && n < 6203) || (n > 6209 && n < 6213) || n = 6214 || (n > 6222 && n < 6227) || n = 6231 || n = 6235 || n = 6237 || n = 6241 || n = 6243 || n = 6244 || (n > 6245 && n < 6249) || n = 6250)
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
.elseif(n = 6174 || n = 6205 || n = 6236)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 6177 || n = 6183 || n = 6206 || n = 6208 || n = 6216 || n = 6221 || n = 6233 || n = 6239)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6178 || n = 6184 || n = 6217 || n = 6222 || n = 6234)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6179)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6180 || n = 6230)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6181)
	; 12 bytes
	tya
	ldy #193
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6182 || n = 6207 || n = 6220 || n = 6232 || n = 6238)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6185)
	; 12 bytes
	tya
	ldy #206
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6188 || n = 6215 || n = 6242)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6192)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6193 || n = 6219 || n = 6245)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6203)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 6204)
	; 12 bytes
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 16
	inx
	Jne *-7
	tax
.elseif(n = 6209)
	; 12 bytes
	tya
	ldy #188
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6213)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	brk
	iny
	Jne *-6
	tay
.elseif(n = 6218)
	; 11 bytes
	php
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6227)
	; 12 bytes
	tya
	ldy #183
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6228)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6229)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6240)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 18
	dex
	dex
	Jne *-7
	tax
.elseif(n = 6249)
	; 12 bytes
	tya
	ldy #223
	delay_n 10
	brk
	dey
	Jne *-6
	tay
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
.if((n >= 6251) && (n < 6290))
; 6251 to 6289
.if(n = 6251 || n = 6252 || n = 6260 || (n > 6261 && n < 6267) || (n > 6271 && n < 6275) || n = 6276 || n = 6281 || (n > 6284 && n < 6290))
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
.elseif(n = 6253)
	; 11 bytes
	tya
	ldy #(n-3)/25 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	tay
.elseif(n = 6254)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6255 || n = 6280)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6256 || n = 6258 || n = 6270 || n = 6283)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6257 || n = 6282)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6259)
	; 12 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 27
	dey
	Jne *-7
	tay
.elseif(n = 6261)
	; 12 bytes
	tya
	ldy #184
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6267)
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
.elseif(n = 6269)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6271)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6275)
	; 12 bytes
	tya
	ldy #190
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6277)
	; 12 bytes
	tya
	ldy #196
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6278)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6279)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6284)
	; 12 bytes
	pla
	delay_n (n-4)
.endif
.else
; 6290 to 6328
.if((n > 6289 && n < 6295) || n = 6306 || (n > 6309 && n < 6321) || n = 6322 || n = 6326 || n = 6327)
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
.elseif(n = 6295)
	; 12 bytes
	tya
	ldy #185
	delay_n 29
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
.elseif(n = 6297 || n = 6323)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6298)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6299 || n = 6301 || n = 6324)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6300 || n = 6302)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6303)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 6304)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 6305)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6307)
	; 12 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-3)/27 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6325)
	; 12 bytes
	tya
	ldy #218 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6328)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 16
	iny
	Jne *-7
	tay
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if(n = 6329 || n = 6360)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6330 || n = 6355)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6331 || n = 6332 || n = 6357 || n = 6362)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6333 || n = 6334 || n = 6359 || n = 6364)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6335)
	; 12 bytes
	tya
	ldy #211
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6336)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	nop
	dey
	Jne *-7
	tay
.elseif(n = 6337 || n = 6338 || n = 6340 || (n > 6341 && n < 6345) || n = 6351 || n = 6352 || n = 6356 || n = 6366 || n = 6367)
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
.elseif(n = 6339)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
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
.elseif(n = 6345)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6346)
	; 12 bytes
	tya
	ldy #(n-2)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6347)
	; 12 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop zp,x'
	nop
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6348)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 18
	iny
	Jne *-7
	tay
.elseif(n = 6349)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6350)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6353)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6354)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6358)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6361)
	; 12 bytes
	tya
	ldy #227
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6363)
	; 12 bytes
	tya
	ldy #187
	delay_n 29
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
.endif
.else
; 6368 to 6406
.if(n = 6368 || n = 6370 || n = 6381 || (n > 6385 && n < 6389) || n = 6390 || n = 6406)
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
.elseif(n = 6369)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 6371 || n = 6382 || n = 6396)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6372 || n = 6383 || n = 6397)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6373 || n = 6384 || n = 6398)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6374)
	; 12 bytes
	tya
	ldy #193
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6375 || n = 6401)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6376)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 6377 || n = 6404)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6378)
	; 12 bytes
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6379)
	; 12 bytes
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 6380)
	; 11 bytes
	tya
	ldy #(n-5)/25
	delay_n 7
	brk
	dey
	Jne *-5
	tay
.elseif(n = 6385)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6389)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 6391)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6392)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	nop
	iny
	Jne *-6
	tay
.elseif(n = 6393)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	pha
	dey
	Jne *-6
	tay
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 6402)
	; 12 bytes
	brk
	delay_n (n-13)
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
.endif
.else
.if((n >= 6407) && (n < 6485))
; 6407 to 6484
.if(n = 6407)
	; 12 bytes
	tya
	ldy #194
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6408 || n = 6454 || n = 6456 || n = 6478)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6409 || n = 6426 || n = 6457 || n = 6479)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif((n > 6409 && n < 6417) || (n > 6417 && n < 6422) || n = 6423 || n = 6428 || n = 6429 || (n > 6431 && n < 6437) || n = 6438 || n = 6439 || n = 6441 || n = 6442 || n = 6444 || (n > 6445 && n < 6451) || n = 6452 || (n > 6458 && n < 6465) || (n > 6465 && n < 6469) || n = 6470 || n = 6472 || n = 6474 || n = 6476 || (n > 6479 && n < 6483))
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
.elseif(n = 6417)
	; 12 bytes
	tya
	ldy #229
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6422 || n = 6453 || n = 6484)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6424 || n = 6455 || n = 6477)
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
.elseif(n = 6427)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6430)
	; 12 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 6431 || n = 6458)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6437)
	; 12 bytes
	tya
	ldy #201
	delay_n 27
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
.elseif(n = 6443)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
.elseif(n = 6445)
	; 12 bytes
	tya
	ldy #230
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6451)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6465)
	; 12 bytes
	tya
	ldy #190
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6469)
	; 12 bytes
	tya
	ldy #202
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6471)
	; 12 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	brk
	dex
	dex
	Jne *-7
	tax
.elseif(n = 6473)
	; 12 bytes
	tya
	ldy #196
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6475)
	; 11 bytes
	php
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6483)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 10
	brk
	dey
	Jne *-7
	tay
.endif
.else
; 6485 to 6562
.if(n = 6485 || n = 6512 || n = 6539)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6486 || n = 6517 || n = 6548)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6487 || n = 6502 || n = 6518 || n = 6540 || n = 6549)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6488 || n = 6507 || n = 6519 || n = 6541 || n = 6550)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif((n > 6488 && n < 6499) || n = 6500 || n = 6504 || (n > 6507 && n < 6512) || n = 6513 || n = 6514 || n = 6516 || (n > 6519 && n < 6529) || n = 6530 || n = 6532 || (n > 6533 && n < 6537) || n = 6538 || n = 6542 || n = 6544 || n = 6547 || (n > 6550 && n < 6555) || n = 6556 || (n > 6557 && n < 6563))
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
.elseif(n = 6499)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-6
	tax
.elseif(n = 6501)
	; 12 bytes
	tya
	ldy #203
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6503)
	; 11 bytes
	tya
	ldy #(n-3)/26 ;hides 'nop'
	delay_n 6
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6505 || n = 6531 || n = 6557)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6506)
	; 12 bytes
	tya
	ldy #197
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6515 || n = 6546)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6529)
	; 12 bytes
	tya
	ldy #233
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6533)
	; 12 bytes
	tya
	ldy #204
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6537)
	; 11 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 26
	iny
	Jne *-6
	tay
.elseif(n = 6543)
	; 12 bytes
	tya
	ldy #218 ;hides 'nop'
	delay_n 10
	brk
	dey
	Jne *-7
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
.elseif(n = 6555)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
.if((n >= 6563) && (n < 6602))
; 6563 to 6601
.if(n = 6563 || (n > 6567 && n < 6572) || n = 6573 || n = 6574 || n = 6576 || n = 6578 || n = 6582 || n = 6584 || (n > 6585 && n < 6589) || n = 6592 || (n > 6593 && n < 6597) || n = 6598 || n = 6599)
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
.elseif(n = 6564)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 6565)
	; 12 bytes
	tya
	ldy #205
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6566 || n = 6593)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6567)
	; 12 bytes
	tya
	ldy #193
	delay_n 29
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
.elseif(n = 6577)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6579)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6580)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6581)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6583)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6585)
	; 12 bytes
	tya
	ldy #235
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6589)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6590)
	; 12 bytes
	tya
	ldy #(n-2)/27 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 6591)
	; 12 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop zp,x'
	nop
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 6597)
	; 12 bytes
	tya
	ldy #206
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6600)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 6601)
	; 12 bytes
	tya
	ldy #194
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.else
; 6602 to 6640
.if(n = 6602 || n = 6604 || (n > 6613 && n < 6618) || n = 6619 || (n > 6620 && n < 6624) || n = 6625 || (n > 6629 && n < 6635) || n = 6636 || n = 6637 || n = 6640)
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
.elseif(n = 6606 || n = 6611 || n = 6627)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6607 || n = 6612 || n = 6628)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6608 || n = 6639)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6609 || n = 6635)
	; 12 bytes
	tya
	ldy #(n-5)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 6610 || n = 6626)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6613)
	; 12 bytes
	tya
	ldy #236
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6618)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 6620)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6624)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 6629)
	; 12 bytes
	tya
	ldy #207
	delay_n 27
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
.endif
.endif
.else
; 6641 to 6718
.if(n = 6641 || n = 6651 || n = 6652 || n = 6672 || n = 6703)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6642 || n = 6653)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6643 || n = 6654 || n = 6705 || n = 6706)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6644 || n = 6645 || (n > 6654 && n < 6661) || (n > 6661 && n < 6665) || (n > 6665 && n < 6669) || (n > 6674 && n < 6693) || n = 6694 || n = 6696 || n = 6698 || n = 6700 || (n > 6706 && n < 6719))
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
.elseif(n = 6646)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	nop
	dey
	Jne *-6
	tay
.elseif(n = 6647 || n = 6674)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6648)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	pha
	iny
	Jne *-6
	tay
.elseif(n = 6649)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 6650)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	delay_n 6
	dey
	Jne *-6
	tay
.elseif(n = 6661)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 6669)
	; 12 bytes
	tya
	ldy #196
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6670 || n = 6701)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 6673)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6693)
	; 12 bytes
	tya
	ldy #209
	delay_n 27
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
.elseif(n = 6697)
	; 12 bytes
	tya
	ldy #239
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6699)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
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
.endif
.else
.if((n >= 6719) && (n < 6797))
; 6719 to 6796
.if((n > 6718 && n < 6725) || n = 6726 || n = 6727 || n = 6729 || n = 6730 || n = 6733 || (n > 6737 && n < 6753) || n = 6754 || (n > 6757 && n < 6763) || n = 6764 || n = 6768 || n = 6769 || (n > 6771 && n < 6781) || n = 6783 || n = 6784 || (n > 6785 && n < 6789) || (n > 6789 && n < 6794) || n = 6795)
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
.elseif(n = 6725)
	; 12 bytes
	tya
	ldy #224
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6728 || n = 6755 || n = 6782)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6731)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
.elseif(n = 6732)
	; 11 bytes
	php
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 6734 || n = 6765 || n = 6796)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6735 || n = 6756 || n = 6766)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6736 || n = 6767)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6737)
	; 12 bytes
	tya
	ldy #204
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6753)
	; 11 bytes
	tya
	ldy #(n-3)/27 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6757)
	; 12 bytes
	tya
	ldy #211
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6763 || n = 6794)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 6771)
	; 12 bytes
	tya
	ldy #199
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6781)
	; 12 bytes
	tya
	ldy #242
	delay_n 10
	brk
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
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.endif
.else
.if((n >= 6797) && (n < 6836))
; 6797 to 6835
.if(n = 6797 || n = 6828)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6798 || n = 6829)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif((n > 6798 && n < 6803) || n = 6804 || n = 6806 || n = 6808 || (n > 6809 && n < 6815) || n = 6816 || n = 6817 || n = 6819 || n = 6820 || n = 6822 || n = 6823 || n = 6826 || n = 6830 || n = 6832)
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
.elseif(n = 6803)
	; 12 bytes
	tya
	ldy #206
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 6805)
	; 12 bytes
	tya
	ldy #200
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6807)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6809)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
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
.elseif(n = 6818)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6821)
	; 12 bytes
	tya
	ldy #213
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6824)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 6825)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6827)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6831)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $4848 ;hides two 'pha's
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6833)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 6834)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 6835)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 6
	brk
	dey
	Jne *-7
	tay
.endif
.else
; 6836 to 6875
.if(n = 6836 || n = 6863)
	; 12 bytes
	tya
	ldy #(n-5)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6837)
	; 12 bytes
	tya
	ldy #244
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6838 || n = 6840 || n = 6841 || n = 6843 || n = 6844 || n = 6846 || (n > 6847 && n < 6853) || n = 6854 || n = 6855 || n = 6857 || n = 6861 || n = 6862 || n = 6864 || (n > 6865 && n < 6869) || (n > 6869 && n < 6873) || n = 6874)
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
.elseif(n = 6839)
	; 12 bytes
	tya
	ldy #201
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6842)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
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
.elseif(n = 6847)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6853)
	; 12 bytes
	tya
	ldy #214
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6856)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6858)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6859)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6860)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6865)
	; 12 bytes
	tya
	ldy #245
	delay_n 10
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
.elseif(n = 6873)
	; 12 bytes
	tya
	ldy #202
	delay_n 29
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
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
; 6876 to 6953
.if(n = 6876)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
.elseif(n = 6877 || n = 6878 || n = 6880 || n = 6884 || n = 6886 || n = 6888 || n = 6892 || (n > 6893 && n < 6901) || (n > 6910 && n < 6915) || n = 6916 || n = 6919 || (n > 6922 && n < 6933) || n = 6934 || (n > 6935 && n < 6941) || (n > 6941 && n < 6947) || n = 6948 || n = 6950)
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
.elseif(n = 6879)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	tay
.elseif(n = 6881 || n = 6889 || n = 6903 || n = 6907 || n = 6908 || n = 6920 || n = 6951)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6882 || n = 6921 || n = 6952)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6883 || n = 6891 || n = 6910 || n = 6922 || n = 6953)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6885)
	; 12 bytes
	tya
	ldy #215
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6887 || n = 6918 || n = 6949)
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
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6893)
	; 12 bytes
	tya
	ldy #246
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6901)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 6902)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	pha
	dey
	Jne *-6
	tay
.elseif(n = 6904)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 6
	iny
	Jne *-6
	tay
.elseif(n = 6905)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 6906)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 6909)
	; 12 bytes
	pla
	txa
	ldx #202 ;hides 'dex'
	delay_n 29
	Jne *-6
	tax
.elseif(n = 6915)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6917)
	; 12 bytes
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
.elseif(n = 6933)
	; 12 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-7
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
.elseif(n = 6941)
	; 12 bytes
	tya
	ldy #204
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6947)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.endif
.else
; 6954 to 7031
.if((n > 6953 && n < 6963) || n = 6964 || n = 6966 || n = 6967 || (n > 6968 && n < 6975) || n = 6976 || n = 6978 || (n > 6984 && n < 6989) || n = 6990 || n = 6994 || (n > 6995 && n < 7001) || n = 7002 || n = 7004 || (n > 7005 && n < 7009) || n = 7010 || n = 7012 || (n > 7015 && n < 7023) || n = 7024 || (n > 7025 && n < 7032))
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
.elseif(n = 6963)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 10
	brk
	iny
	Jne *-7
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
.elseif(n = 6975)
	; 12 bytes
	tya
	ldy #205
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 6977)
	; 12 bytes
	tya
	ldy #249
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 6979)
	; 12 bytes
	tya
	ldy #218 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
.elseif(n = 6980 || n = 7011)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 6981)
	; 12 bytes
	tya
	ldy #218
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 6982 || n = 6991 || n = 7013)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6983 || n = 6992 || n = 7014)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 6984 || n = 6993 || n = 7015)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 6989)
	; 11 bytes
	php
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
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
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
.elseif(n = 7005)
	; 12 bytes
	tya
	ldy #250
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7009)
	; 12 bytes
	tya
	ldy #206
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7023)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 10
	brk
	dey
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
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if(n = 7032 || (n > 7034 && n < 7042) || (n > 7046 && n < 7050) || (n > 7050 && n < 7055) || (n > 7055 && n < 7061) || (n > 7061 && n < 7067) || (n > 7068 && n < 7073) || n = 7078 || (n > 7079 && n < 7085) || (n > 7085 && n < 7089) || (n > 7089 && n < 7100) || (n > 7100 && n < 7104) || n = 7105)
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
.elseif(n = 7033)
	; 12 bytes
	tya
	ldy #251
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7034)
	; 12 bytes
	tya
	ldy #213
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7042 || n = 7073 || n = 7104)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7043)
	; 12 bytes
	tya
	ldy #207
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7044 || n = 7075 || n = 7106)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7045)
	; 12 bytes
	tya
	ldy #220
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7046)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7050)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	dey
	Jne *-7
	tay
.elseif(n = 7055)
	; 12 bytes
	tya
	ldy #235
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7061)
	; 12 bytes
	tya
	ldy #252
	delay_n 10
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
.elseif(n = 7068)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 7074)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
	tay
.elseif(n = 7076 || n = 7107)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7077)
	; 12 bytes
	tya
	ldy #221
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7079)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	brk
	dey
	Jne *-7
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
.elseif(n = 7089)
	; 12 bytes
	tya
	ldy #253
	delay_n 10
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
	jsr @nearby_rts_15cyc
	jsr @nearby_rts_15cyc
	Jne *-7
	tax
.elseif(n = 7109)
	; 12 bytes
	tya
	ldy #222
	delay_n 27
	dey
	Jne *-6
	tay
.endif
.else
; 7110 to 7187
.if(n = 7110 || (n > 7111 && n < 7115) || n = 7116 || (n > 7117 && n < 7131) || n = 7140 || n = 7142 || n = 7144 || (n > 7145 && n < 7157) || n = 7167 || n = 7171 || n = 7172 || n = 7174 || n = 7176 || n = 7177 || (n > 7179 && n < 7188))
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
.elseif(n = 7111)
	; 12 bytes
	tya
	ldy #209
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7115)
	; 12 bytes
	tya
	ldy #237
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7117)
	; 12 bytes
	tya
	ldy #254
	delay_n 10
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7131)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7132)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
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
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 10
	iny
	Jne *-6
	tay
.elseif(n = 7135 || n = 7166)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7136 || n = 7137 || n = 7159 || n = 7164 || n = 7168)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7138 || n = 7163 || n = 7165 || n = 7169)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7139 || n = 7170)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7141)
	; 12 bytes
	tya
	ldy #223
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7143)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 26
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
.elseif(n = 7157)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	pha
	dey
	Jne *-6
	tay
.elseif(n = 7158)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	nop
	nop
	dey
	Jne *-7
	tay
.elseif(n = 7160)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 7161)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 7162)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 10
	dey
	Jne *-6
	tay
.elseif(n = 7173)
	; 12 bytes
	tya
	ldy #224
	delay_n 27
	dey
	Jne *-6
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
.elseif(n = 7178)
	; 12 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 7179)
	; 12 bytes
	tya
	ldy #211
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
; 7188 to 7265
.if((n > 7187 && n < 7195) || n = 7196 || n = 7198 || n = 7202 || n = 7204 || (n > 7205 && n < 7213) || (n > 7213 && n < 7228) || n = 7229 || n = 7233 || n = 7234 || n = 7236 || (n > 7237 && n < 7246) || n = 7251 || n = 7252 || (n > 7253 && n < 7259) || n = 7260 || n = 7264)
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
.elseif(n = 7195)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
.elseif(n = 7197 || n = 7228 || n = 7259)
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
.elseif(n = 7200 || n = 7231 || n = 7249 || n = 7262)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7201 || n = 7250 || n = 7263)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7203)
	; 12 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 29
	iny
	Jne *-7
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
.elseif(n = 7213)
	; 12 bytes
	tya
	ldy #212
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7230 || n = 7248 || n = 7261)
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
.elseif(n = 7237)
	; 12 bytes
	tya
	ldy #226
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7246)
	; 11 bytes
	php
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 7247)
	; 12 bytes
	tya
	ldy #213
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7253)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
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
.endif
.else
; 7266 to 7343
.if((n > 7265 && n < 7269) || (n > 7269 && n < 7281) || (n > 7281 && n < 7290) || n = 7291 || n = 7296 || n = 7297 || n = 7299 || n = 7300 || (n > 7301 && n < 7311) || (n > 7311 && n < 7315) || (n > 7315 && n < 7321) || n = 7322 || (n > 7325 && n < 7331) || n = 7332 || (n > 7333 && n < 7344))
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
.elseif(n = 7269)
	; 12 bytes
	tya
	ldy #227
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7281)
	; 12 bytes
	tya
	ldy #214
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7290 || n = 7321)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7292 || n = 7323)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7293 || n = 7324)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7294)
	; 12 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 7301)
	; 12 bytes
	tya
	ldy #228
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7311)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 29
	Jne *-7
	tax
.elseif(n = 7315)
	; 12 bytes
	tya
	ldy #215
	delay_n 29
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
.elseif(n = 7333)
	; 12 bytes
	tya
	ldy #229
	delay_n 27
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7347) || n = 7348 || n = 7350 || n = 7351 || n = 7353 || (n > 7356 && n < 7363) || (n > 7365 && n < 7383) || n = 7384 || (n > 7389 && n < 7397) || (n > 7397 && n < 7412) || (n > 7418 && n < 7422))
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
.elseif(n = 7347)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 27
	dey
	Jne *-7
	tay
.elseif(n = 7349)
	; 12 bytes
	tya
	ldy #216
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7352 || n = 7383 || n = 7414)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7354 || n = 7385 || n = 7416)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7355)
	; 12 bytes
	tya
	ldy #245
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
.elseif(n = 7356 || n = 7387)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7363)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
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
.elseif(n = 7365)
	; 12 bytes
	tya
	ldy #230
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7386)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7388)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 7389)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	delay_n 9
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
.elseif(n = 7412)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
.elseif(n = 7413)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	nop
	dey
	Jne *-7
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
.elseif(n = 7417)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 7418)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	delay_n 9
	dey
	Jne *-7
	tay
.endif
.else
; 7422 to 7500
.if(n = 7422 || (n > 7423 && n < 7427) || n = 7428 || (n > 7430 && n < 7443) || n = 7444 || n = 7446 || (n > 7452 && n < 7458) || n = 7459 || n = 7460 || n = 7462 || (n > 7463 && n < 7473) || n = 7474 || n = 7477 || (n > 7480 && n < 7484) || (n > 7485 && n < 7491) || n = 7492 || n = 7494 || n = 7495 || (n > 7496 && n < 7501))
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
.elseif(n = 7423)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	delay_n 26
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
.elseif(n = 7429)
	; 12 bytes
	tya
	ldy #232
	delay_n 27
	dey
	Jne *-6
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
.elseif(n = 7443)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 10
	brk
	dey
	Jne *-7
	tay
.elseif(n = 7445 || n = 7476)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7447 || n = 7478)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7448 || n = 7479)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7449 || n = 7480)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7450)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 7451)
	; 12 bytes
	tya
	ldy #219
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7452)
	; 12 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
.elseif(n = 7458)
	; 12 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 7461)
	; 12 bytes
	tya
	ldy #233
	delay_n 27
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
.elseif(n = 7473)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	iny
	Jne *-7
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
.elseif(n = 7484)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 7485)
	; 12 bytes
	tya
	ldy #220
	delay_n 29
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
.elseif(n = 7493)
	; 12 bytes
	tya
	ldy #234
	delay_n 27
	dey
	Jne *-6
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
.endif
.endif
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
.if((n >= 7501) && (n < 7579))
; 7501 to 7578
.if(n = 7501 || n = 7502 || n = 7504 || n = 7508 || n = 7512 || n = 7513 || (n > 7514 && n < 7518) || (n > 7519 && n < 7525) || (n > 7525 && n < 7529) || (n > 7529 && n < 7535) || n = 7536 || n = 7537 || n = 7539 || (n > 7542 && n < 7553) || (n > 7553 && n < 7557) || (n > 7557 && n < 7562) || n = 7564 || (n > 7565 && n < 7569) || n = 7570 || (n > 7573 && n < 7579))
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
.elseif(n = 7503)
	; 11 bytes
	php
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 7505 || n = 7509 || n = 7540 || n = 7571)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7506 || n = 7510 || n = 7541 || n = 7572)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7507 || n = 7538 || n = 7569)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7511 || n = 7542 || n = 7573)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7514)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 32
	Jne *-7
	tax
.elseif(n = 7518)
	; 12 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 7519)
	; 12 bytes
	tya
	ldy #221
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7525)
	; 12 bytes
	tya
	ldy #235
	delay_n 27
	dey
	Jne *-6
	tay
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
.elseif(n = 7553)
	; 12 bytes
	tya
	ldy #222
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7557)
	; 12 bytes
	tya
	ldy #236
	delay_n 27
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
.elseif(n = 7563)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	delay_n 26
	dey
	Jne *-7
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
.endif
.else
; 7579 to 7656
.if((n > 7578 && n < 7587) || n = 7588 || (n > 7589 && n < 7593) || n = 7594 || n = 7598 || n = 7599 || n = 7601 || (n > 7604 && n < 7621) || (n > 7621 && n < 7625) || n = 7626 || n = 7627 || n = 7629 || n = 7630 || n = 7632 || (n > 7635 && n < 7644) || n = 7645 || (n > 7648 && n < 7653) || n = 7654 || n = 7656)
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
.elseif(n = 7587)
	; 12 bytes
	tya
	ldy #223
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7589)
	; 12 bytes
	tya
	ldy #237
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7593)
	; 11 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 26
	iny
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
.elseif(n = 7596 || n = 7603 || n = 7634 || n = 7647)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7597 || n = 7604 || n = 7635 || n = 7648)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7600 || n = 7631)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7602 || n = 7633 || n = 7646)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7621)
	; 12 bytes
	tya
	ldy #238
	delay_n 27
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
.elseif(n = 7653)
	; 12 bytes
	tya
	ldy #239
	delay_n 27
	dey
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
.endif
.endif
.else
; 7657 to 7812
.if(n = 7657 || n = 7658 || n = 7660 || n = 7667 || n = 7668 || n = 7671 || n = 7675 || (n > 7678 && n < 7685) || (n > 7685 && n < 7689) || (n > 7689 && n < 7693) || (n > 7697 && n < 7717) || (n > 7717 && n < 7723) || (n > 7729 && n < 7749) || (n > 7749 && n < 7755) || n = 7756 || n = 7761 || (n > 7764 && n < 7779) || n = 7780 || (n > 7781 && n < 7786) || n = 7787 || n = 7792 || (n > 7793 && n < 7813))
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
.elseif(n = 7662 || n = 7693 || n = 7724 || n = 7755 || n = 7786)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7663 || n = 7666 || n = 7678 || n = 7697 || n = 7728 || n = 7729 || n = 7759 || n = 7764 || n = 7790)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7664 || n = 7676 || n = 7695 || n = 7726 || n = 7757 || n = 7762 || n = 7788)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7665 || n = 7677 || n = 7696 || n = 7758 || n = 7763 || n = 7789)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7669)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	pha
	dey
	Jne *-7
	tay
.elseif(n = 7670)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 7672)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 7673)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 10
	dey
	Jne *-7
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
.elseif(n = 7689)
	; 12 bytes
	tya
	ldy #226
	delay_n 29
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
.elseif(n = 7723)
	; 12 bytes
	tya
	ldy #227
	delay_n 29
	dey
	Jne *-6
	tay
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
.elseif(n = 7749)
	; 12 bytes
	tya
	ldy #242
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7760)
	; 11 bytes
	php
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 7779)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 29
	dey
	Jne *-7
	tay
.elseif(n = 7781)
	; 12 bytes
	tya
	ldy #243
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7791)
	; 12 bytes
	tya
	ldy #229
	delay_n 29
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
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if(n = 7813)
	; 12 bytes
	tya
	ldy #244
	delay_n 27
	dey
	Jne *-6
	tay
.elseif((n > 7813 && n < 7817) || n = 7818 || n = 7822 || n = 7824 || (n > 7826 && n < 7845) || n = 7846 || n = 7847 || n = 7849 || (n > 7852 && n < 7859) || (n > 7859 && n < 7877) || n = 7878 || n = 7880 || (n > 7883 && n < 7891))
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
.elseif(n = 7817 || n = 7848 || n = 7879)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7819 || n = 7850 || n = 7881)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7820 || n = 7851 || n = 7882)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7821 || n = 7852 || n = 7883)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7823)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 27
	iny
	Jne *-7
	tay
.elseif(n = 7825)
	; 12 bytes
	tya
	ldy #230
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7826)
	; 12 bytes
	tya
	ldy #237
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7845)
	; 12 bytes
	tya
	ldy #245
	delay_n 27
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
.elseif(n = 7877)
	; 12 bytes
	tya
	ldy #246
	delay_n 27
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if(n = 7891)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 27
	iny
	Jne *-7
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
.elseif(n = 7893)
	; 12 bytes
	tya
	ldy #232
	delay_n 29
	dey
	Jne *-6
	tay
.elseif((n > 7893 && n < 7899) || n = 7900 || n = 7908 || n = 7911 || (n > 7914 && n < 7919) || n = 7920)
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
.elseif(n = 7899)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 7901 || n = 7904 || n = 7912 || n = 7921)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7902 || n = 7913 || n = 7922)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7903 || n = 7914 || n = 7923)
	; 11..12 bytes
	pla
	delay_n (n-4)
.elseif(n = 7905)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7906 || n = 7926)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 7907)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7909)
	; 12 bytes
	tya
	ldy #247
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7910)
	; 11 bytes
	tya
	ldy #(n-5)/31
	delay_n 26
	dey
	Jne *-5
	tay
.elseif(n = 7919)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
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
.elseif(n = 7927)
	; 12 bytes
	tya
	ldy #233
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 7928)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 10
	iny
	Jne *-7
	tay
.elseif(n = 7929)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	delay_n 10
	dey
	Jne *-7
	tay
.endif
.else
; 7930 to 7968
.if(n = 7930)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif(n = 7931 || n = 7940 || n = 7942 || (n > 7945 && n < 7958) || n = 7960 || (n > 7961 && n < 7969))
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
.elseif(n = 7932 || n = 7943)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7933 || n = 7944)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 7934 || n = 7945)
	; 11..12 bytes
	pla
	delay_n (n-4)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 7937)
	; 12 bytes
	delay_n (n-4)
	pla
.elseif(n = 7938)
	; 12 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 7939)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
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
.elseif(n = 7959)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 27
	dey
	Jne *-7
	tay
.elseif(n = 7961)
	; 12 bytes
	tya
	ldy #234
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7973) || (n > 7973 && n < 7991) || (n > 7991 && n < 7995) || (n > 7995 && n < 8003) || n = 8004 || (n > 8005 && n < 8017) || n = 8018 || n = 8022 || n = 8023 || (n > 8024 && n < 8029) || (n > 8029 && n < 8037) || (n > 8037 && n < 8053) || (n > 8053 && n < 8057) || (n > 8057 && n < 8063) || (n > 8063 && n < 8069) || (n > 8069 && n < 8088) || n = 8089 || (n > 8090 && n < 8097) || (n > 8097 && n < 8101) || (n > 8101 && n < 8123) || n = 8124 || n = 8125)
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
.elseif(n = 7973)
	; 12 bytes
	tya
	ldy #249
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 7991)
	; 12 bytes
	tya
	ldy #242
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 7995)
	; 12 bytes
	tya
	ldy #235
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8003)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
.elseif(n = 8005)
	; 12 bytes
	tya
	ldy #250
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 8017)
	; 11 bytes
	php
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8019)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8020)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8021)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8024)
	; 12 bytes
	tya
	ldy #243
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8029)
	; 12 bytes
	tya
	ldy #236
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8037)
	; 12 bytes
	tya
	ldy #251
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 8053)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-7
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
.elseif(n = 8063)
	; 12 bytes
	tya
	ldy #237
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8069)
	; 12 bytes
	tya
	ldy #252
	delay_n 27
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
.elseif(n = 8097)
	; 12 bytes
	tya
	ldy #238
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8101)
	; 12 bytes
	tya
	ldy #253
	delay_n 27
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
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
; 8126 to 8203
.if((n > 8125 && n < 8131) || n = 8132 || (n > 8133 && n < 8154) || n = 8155 || (n > 8158 && n < 8165) || (n > 8165 && n < 8181) || (n > 8181 && n < 8185) || n = 8192 || (n > 8193 && n < 8197) || n = 8198 || (n > 8199 && n < 8204))
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
.elseif(n = 8131)
	; 12 bytes
	tya
	ldy #239
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8133)
	; 12 bytes
	tya
	ldy #254
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 8154)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
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
.elseif(n = 8157 || n = 8190)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8158 || n = 8191)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8165)
	; 12 bytes
	tya
	ldy #255
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 8181)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 8185)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 8186)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
.elseif(n = 8187)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8188)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8197)
	; 12 bytes
	tya
	ldy #0
	delay_n 27
	dey
	Jne *-6
	tay
.elseif(n = 8199)
	; 12 bytes
	tya
	ldy #241
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.else
; 8204 to 8281
.if((n > 8203 && n < 8222) || (n > 8222 && n < 8228) || (n > 8228 && n < 8233) || (n > 8233 && n < 8252) || n = 8254 || (n > 8257 && n < 8263) || (n > 8263 && n < 8267) || (n > 8267 && n < 8274) || n = 8275 || (n > 8278 && n < 8282))
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
.elseif(n = 8222)
	; 12 bytes
	tya
	ldy #249
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8228)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 8233)
	; 12 bytes
	tya
	ldy #242
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8252)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
.elseif(n = 8253)
	; 11 bytes
	tya
	ldy #250 ;hides 'nop'
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
.elseif(n = 8256 || n = 8277)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8257 || n = 8278)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8263)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 8267)
	; 12 bytes
	tya
	ldy #243
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8274)
	; 11 bytes
	php
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8276)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 8282 to 8437
.if(n = 8282 || (n > 8283 && n < 8288) || (n > 8288 && n < 8298) || n = 8299 || n = 8300 || (n > 8301 && n < 8321) || (n > 8321 && n < 8335) || (n > 8335 && n < 8354) || (n > 8355 && n < 8369) || (n > 8369 && n < 8387) || (n > 8387 && n < 8403) || (n > 8403 && n < 8409) || n = 8410 || (n > 8413 && n < 8420) || (n > 8420 && n < 8427) || (n > 8427 && n < 8431) || n = 8432 || n = 8436)
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
.elseif(n = 8283)
	; 12 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 29
	iny
	Jne *-7
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
.elseif(n = 8298)
	; 12 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
.elseif(n = 8301)
	; 12 bytes
	tya
	ldy #244
	delay_n 29
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
.elseif(n = 8335)
	; 12 bytes
	tya
	ldy #245
	delay_n 29
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
.elseif(n = 8355)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 29
	iny
	Jne *-7
	tay
.elseif(n = 8369)
	; 12 bytes
	tya
	ldy #246
	delay_n 29
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
.elseif(n = 8403)
	; 12 bytes
	tya
	ldy #247
	delay_n 29
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
.elseif(n = 8411 || n = 8433)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8412 || n = 8434)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8413)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8420)
	; 12 bytes
	tya
	ldy #255
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8427)
	; 12 bytes
	tya
	ldy #(n-3)/36 ;hides 'nop'
	delay_n 29
	dey
	Jne *-7
	tay
.elseif(n = 8431)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
.elseif(n = 8435)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	tay
.elseif(n = 8437)
	; 12 bytes
	tya
	ldy #248
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
; 8438 to 8593
.if(n = 8438 || n = 8439 || (n > 8446 && n < 8453) || (n > 8453 && n < 8471) || (n > 8471 && n < 8503) || n = 8504 || n = 8506 || n = 8507 || (n > 8508 && n < 8531) || (n > 8535 && n < 8539) || (n > 8539 && n < 8543) || (n > 8543 && n < 8573) || (n > 8573 && n < 8578) || (n > 8578 && n < 8594))
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
.elseif(n = 8440)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 8441)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 8442)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	brk
	dey
	Jne *-6
	tay
.elseif(n = 8443 || n = 8444 || n = 8533)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8445 || n = 8534)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8446 || n = 8535)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8453)
	; 12 bytes
	tya
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	tay
.elseif(n = 8471)
	; 12 bytes
	tya
	ldy #249
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8503)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	delay_n 27
	dey
	Jne *-7
	tay
.elseif(n = 8505)
	; 12 bytes
	tya
	ldy #250
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8508)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 8531)
	; 11 bytes
	php
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8532)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
.elseif(n = 8539)
	; 12 bytes
	tya
	ldy #251
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8543)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 8573)
	; 12 bytes
	tya
	ldy #252
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8578)
	; 12 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
.endif
.else
; 8594 to 8750
.if((n > 8593 && n < 8607) || (n > 8607 && n < 8641) || (n > 8641 && n < 8664) || n = 8665 || (n > 8668 && n < 8675) || (n > 8675 && n < 8683) || (n > 8683 && n < 8687) || n = 8688 || (n > 8691 && n < 8696) || (n > 8702 && n < 8709) || (n > 8709 && n < 8751))
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
.elseif(n = 8607)
	; 12 bytes
	tya
	ldy #253
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8641)
	; 12 bytes
	tya
	ldy #254
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8664)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 16
	iny
	Jne *-6
	tay
.elseif(n = 8666 || n = 8689 || n = 8700)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8667 || n = 8690 || n = 8699 || n = 8701)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8668 || n = 8691 || n = 8702)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8675)
	; 12 bytes
	tya
	ldy #255
	delay_n 29
	dey
	Jne *-6
	tay
.elseif(n = 8683)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 8687)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	pha
	dey
	Jne *-6
	tay
.elseif(n = 8696)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	brk
	iny
	Jne *-6
	tay
.elseif(n = 8697)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	brk
	dey
	Jne *-6
	tay
.elseif(n = 8698)
	; 11 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 8709)
	; 12 bytes
	tya
	ldy #0
	delay_n 29
	dey
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
; 8751 to 9375
.if(n = 8751 || n = 8752 || (n > 8753 && n < 8788) || n = 8789 || (n > 8792 && n < 8919) || (n > 8919 && n < 8931) || (n > 8931 && n < 8935) || (n > 8935 && n < 8943) || (n > 8943 && n < 8952) || (n > 8954 && n < 9003) || (n > 9003 && n < 9045) || n = 9046 || (n > 9049 && n < 9174) || (n > 9174 && n < 9199) || (n > 9199 && n < 9208) || (n > 9210 && n < 9302) || n = 9303 || (n > 9306 && n < 9376))
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
.elseif(n = 8753)
	; 12 bytes
	tya
	ldy #250 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
.elseif(n = 8788)
	; 11 bytes
	php
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 8790 || n = 9047 || n = 9304)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8791 || n = 9048 || n = 9305)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 8792 || n = 9049 || n = 9306)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 8919)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	nop
	brk
	iny
	Jne *-7
	tay
.elseif(n = 8931)
	; 12 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 29
	dey
	Jne *-7
	tay
.elseif(n = 8935)
	; 12 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
.elseif(n = 8943)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	nop
	dey
	Jne *-7
	tay
.elseif(n = 8952)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	brk
	iny
	Jne *-7
	tay
.elseif(n = 8953)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk
	dey
	Jne *-7
	tay
.elseif(n = 8954)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	nop
	brk
	dey
	Jne *-7
	tay
.elseif(n = 9003)
	; 12 bytes
	tya
	ldy #(n-3)/36 ;hides 'nop'
	delay_n 29
	dey
	Jne *-7
	tay
.elseif(n = 9045)
	; 11 bytes
	php
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 9174)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 9199)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	pha
	dey
	Jne *-7
	tay
.elseif(n = 9208)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	brk
	iny
	Jne *-7
	tay
.elseif(n = 9209)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 9210)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	nop
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 9302)
	; 11 bytes
	php
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.else
; 9376 to 10000
.if((n > 9375 && n < 9429) || (n > 9429 && n < 9455) || (n > 9455 && n < 9462) || n = 9463 || (n > 9466 && n < 9559) || n = 9560 || (n > 9563 && n < 9684) || (n > 9684 && n < 9711) || (n > 9711 && n < 9717) || (n > 9717 && n < 9722) || (n > 9722 && n < 9816) || n = 9817 || (n > 9820 && n < 10001))
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
.elseif(n = 9429)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
	brk
	iny
	Jne *-7
	tay
.elseif(n = 9455)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 6
	dey
	Jne *-7
	tay
.elseif(n = 9462)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	brk
	dey
	Jne *-7
	tay
.elseif(n = 9464)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	delay_n 16
	iny
	Jne *-7
	tay
.elseif(n = 9465)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 9466)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 6
	brk
	dey
	Jne *-7
	tay
.elseif(n = 9559)
	; 11 bytes
	php
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 9561 || n = 9818)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9562 || n = 9819)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 9563 || n = 9820)
	; 12 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 9711)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 9717)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
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
.elseif(n = 9816)
	; 11 bytes
	php
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
.if((n >= 10001) && (n < 12501))
; 10001 to 12500
.if((n > 10000 && n < 10073) || n = 10074 || (n > 10077 && n < 10330) || n = 10331 || (n > 10334 && n < 10587) || n = 10588 || (n > 10591 && n < 10844) || n = 10845 || (n > 10848 && n < 11101) || n = 11102 || (n > 11105 && n < 11214) || (n > 11214 && n < 11247) || (n > 11247 && n < 11258) || (n > 11258 && n < 11358) || n = 11359 || (n > 11362 && n < 11615) || n = 11616 || (n > 11619 && n < 11872) || n = 11873 || (n > 11876 && n < 12129) || n = 12130 || (n > 12133 && n < 12386) || n = 12387 || (n > 12390 && n < 12501))
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
.elseif(n = 10073)
	; 11 bytes
	php
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10075 || n = 10332 || n = 10589 || n = 10846 || n = 11103 || n = 11360 || n = 11617 || n = 11874 || n = 12131 || n = 12388)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10076 || n = 10333 || n = 10590 || n = 10847 || n = 11104 || n = 11361 || n = 11618 || n = 11875 || n = 12132 || n = 12389)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 10077 || n = 10334 || n = 10591 || n = 10848 || n = 11105 || n = 11362 || n = 11619 || n = 11876 || n = 12133 || n = 12390)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 10330)
	; 11 bytes
	php
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10587)
	; 11 bytes
	php
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 10844)
	; 11 bytes
	php
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11101)
	; 11 bytes
	php
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
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
.elseif(n = 11247)
	; 12 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	dey
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
.elseif(n = 11358)
	; 11 bytes
	php
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11615)
	; 11 bytes
	php
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 11872)
	; 11 bytes
	php
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12129)
	; 11 bytes
	php
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12386)
	; 11 bytes
	php
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12643) || n = 12644 || (n > 12647 && n < 12900) || n = 12901 || (n > 12904 && n < 13157) || n = 13158 || (n > 13161 && n < 13414) || n = 13415 || (n > 13418 && n < 13671) || n = 13672 || (n > 13675 && n < 13928) || n = 13929 || (n > 13932 && n < 14185) || n = 14186 || (n > 14189 && n < 14442) || n = 14443 || (n > 14446 && n < 14699) || n = 14700 || (n > 14703 && n < 14956) || n = 14957 || (n > 14960 && n < 15001))
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
.elseif(n = 12643)
	; 11 bytes
	php
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 12645 || n = 12902 || n = 13159 || n = 13416 || n = 13673 || n = 13930 || n = 14187 || n = 14444 || n = 14701 || n = 14958)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12646 || n = 12903 || n = 13160 || n = 13417 || n = 13674 || n = 13931 || n = 14188 || n = 14445 || n = 14702 || n = 14959)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 12647 || n = 12904 || n = 13161 || n = 13418 || n = 13675 || n = 13932 || n = 14189 || n = 14446 || n = 14703 || n = 14960)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 12900)
	; 11 bytes
	php
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13157)
	; 11 bytes
	php
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13414)
	; 11 bytes
	php
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13671)
	; 11 bytes
	php
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 13928)
	; 11 bytes
	php
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14185)
	; 11 bytes
	php
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14442)
	; 11 bytes
	php
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14699)
	; 11 bytes
	php
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 14956)
	; 11 bytes
	php
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15213) || n = 15214 || (n > 15217 && n < 15470) || n = 15471 || (n > 15474 && n < 15727) || n = 15728 || (n > 15731 && n < 15984) || n = 15985 || (n > 15988 && n < 16241) || n = 16242 || (n > 16245 && n < 16498) || n = 16499 || (n > 16502 && n < 16755) || n = 16756 || (n > 16759 && n < 17012) || n = 17013 || (n > 17016 && n < 17269) || n = 17270 || (n > 17273 && n < 17526) || n = 17527 || (n > 17530 && n < 17783) || n = 17784 || (n > 17787 && n < 18040) || n = 18041 || (n > 18044 && n < 18297) || n = 18298 || (n > 18301 && n < 18554) || n = 18555 || (n > 18558 && n < 18811) || n = 18812 || (n > 18815 && n < 19068) || n = 19069 || (n > 19072 && n < 19325) || n = 19326 || (n > 19329 && n < 19582) || n = 19583 || (n > 19586 && n < 19839) || n = 19840 || (n > 19843 && n < 20001))
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
.elseif(n = 15213)
	; 11 bytes
	php
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15215 || n = 15472 || n = 15729 || n = 15986 || n = 16243 || n = 16500 || n = 16757 || n = 17014 || n = 17271 || n = 17528 || n = 17785 || n = 18042 || n = 18299 || n = 18556 || n = 18813 || n = 19070 || n = 19327 || n = 19584 || n = 19841)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15216 || n = 15473 || n = 15730 || n = 15987 || n = 16244 || n = 16501 || n = 16758 || n = 17015 || n = 17272 || n = 17529 || n = 17786 || n = 18043 || n = 18300 || n = 18557 || n = 18814 || n = 19071 || n = 19328 || n = 19585 || n = 19842)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 15217 || n = 15474 || n = 15731 || n = 15988 || n = 16245 || n = 16502 || n = 16759 || n = 17016 || n = 17273 || n = 17530 || n = 17787 || n = 18044 || n = 18301 || n = 18558 || n = 18815 || n = 19072 || n = 19329 || n = 19586 || n = 19843)
	; 12 bytes
	pla
	delay_n (n-4)
.elseif(n = 15470)
	; 11 bytes
	php
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15727)
	; 11 bytes
	php
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 15984)
	; 11 bytes
	php
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 16241)
	; 11 bytes
	php
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 16498)
	; 11 bytes
	php
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 16755)
	; 11 bytes
	php
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17012)
	; 11 bytes
	php
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17269)
	; 11 bytes
	php
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17526)
	; 11 bytes
	php
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 17783)
	; 11 bytes
	php
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18040)
	; 11 bytes
	php
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18297)
	; 11 bytes
	php
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18554)
	; 11 bytes
	php
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 18811)
	; 11 bytes
	php
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19068)
	; 11 bytes
	php
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19325)
	; 11 bytes
	php
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19582)
	; 11 bytes
	php
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.elseif(n = 19839)
	; 11 bytes
	php
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
