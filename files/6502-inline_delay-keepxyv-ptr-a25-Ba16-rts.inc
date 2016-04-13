; delay_n, delays n cycles with small code
; Will clobber A, Z+N, C, and D
; Will preserve X, Y, S, V, and I (interrupt-disable flag)
;
; Uses JMP (unrelocatable code).
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a zeropage two-byte variable @ptrtemp, that points
; into a memory address that can be safely read and overwritten
; with random data.
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 16
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 290
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 1293
; -10 bytes or less when n <= 1321
; -11 bytes or less when n <= 2321
; -12 bytes or less when n <= 3345
; -13 bytes or less when n <= 4081
; -14 bytes or less when n <= 5101
; -15 bytes or less when n <= 6930
; -16 bytes or less when n <= 13320
; -17 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.8, max=6
; - When 21 <= n <= 30: min=4, avg=5.8, max=7
; - When 31 <= n <= 50: min=4, avg=6.2, max=7
; - When 51 <= n <= 75: min=5, avg=6.8, max=7
; - When 76 <= n <= 100: min=6, avg=7.0, max=7
; - When 101 <= n <= 200: min=6, avg=7.0, max=7
; - When 201 <= n <= 300: min=6, avg=7.1, max=9
; - When 301 <= n <= 500: min=6, avg=8.2, max=9
; - When 501 <= n <= 750: min=6, avg=8.3, max=9
; - When 751 <= n <= 1000: min=7, avg=8.3, max=9
; - When 1001 <= n <= 2000: min=6, avg=9.0, max=11
; - When 2001 <= n <= 3000: min=7, avg=10.2, max=12
; - When 3001 <= n <= 5000: min=7, avg=11.5, max=14
; - When 5001 <= n <= 7500: min=7, avg=12.2, max=16
; - When 7501 <= n <= 10000: min=7, avg=12.3, max=16
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
.if((n >= 1) && (n < 79))
.if((n >= 1) && (n < 40))
.if((n >= 1) && (n < 20))
; 1 to 19
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif((n > 3 && n < 7))
	; 2..3 bytes
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
.elseif(n = 14)
	; 4 bytes
	.repeat 2
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16 || n = 18)
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 17)
	; 6 bytes
	php
	rol $26
	ror $26
	plp
.endif
.else
; 20 to 39
.if(n = 20)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
.elseif(n = 21)
	; 5 bytes
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
.elseif(n = 22 || n = 26 || n = 36)
	; 5..6 bytes
	sec
	delay_n (n-2)
.elseif(n = 23 || n = 33)
	; 6 bytes
	clc
	delay_n (n-2)
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 7 bytes
	php
	lda #0
	jsr delay_256a_16_clocks
	plp
.elseif(n = 27)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 28)
	; 6 bytes
	clc
	clc
	delay_n (n-4)
.elseif(n = 29)
	; 6 bytes
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
.elseif(n = 30)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 31)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
.elseif(n = 32)
	; 7 bytes
	lda #$2A ;hides 'rol a'
	delay_n 6
	Jpl *-4
.elseif(n = 34)
	; 5 bytes
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
.elseif(n = 35)
	; 6 bytes
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
.elseif(n = 37 || n = 38)
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.endif
.endif
.else
; 40 to 78
.if(n = 40 || n = 45 || (n > 46 && n < 53) || n = 54 || n = 56 || n = 58 || (n > 59 && n < 71) || (n > 71 && n < 79))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 41)
	; 5 bytes
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 42)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 43)
	; 6 bytes
	sec
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
.elseif(n = 44)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 4
	Jpl *-3
.elseif(n = 46)
	; 6 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 53)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
.elseif(n = 55)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
.elseif(n = 57)
	; 6 bytes
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
.elseif(n = 59)
	; 6 bytes
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
.elseif(n = 71)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 4
	Jne *-3
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 95) || (n > 95 && n < 128) || (n > 128 && n < 157))
	; 7 bytes
	php
	lda #n-34
	jsr delay_a_25_clocks
	plp
.elseif(n = 95)
	; 6 bytes
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
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
.elseif(n = 293 || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
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
.elseif(n = 314 || n = 317 || n = 319 || n = 322 || n = 324 || n = 332 || n = 336 || n = 337 || n = 339 || n = 342 || n = 344 || n = 347 || n = 349 || n = 352 || n = 354 || n = 357 || n = 359 || n = 367 || n = 371 || n = 372 || n = 374 || n = 377 || n = 379 || n = 382 || n = 384 || n = 387 || n = 389 || n = 392 || n = 394 || n = 406 || n = 407 || n = 412 || n = 414 || n = 417 || n = 419 || n = 422 || n = 424 || n = 427 || n = 429 || n = 437 || n = 441 || n = 442 || n = 444 || n = 447 || n = 449 || n = 452 || n = 454 || n = 457 || n = 459 || n = 462 || n = 464)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385 || n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 315 && n < 332) || n = 351 || n = 361 || n = 366 || n = 391 || n = 396 || n = 416 || (n > 425 && n < 437) || n = 451 || n = 456 || n = 466))
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 318 || n = 323 || n = 328 || n = 333 || n = 338 || n = 343 || n = 353 || n = 358 || n = 368 || n = 373 || n = 378 || n = 386 || n = 388 || n = 393 || n = 398 || n = 403 || n = 413 || n = 423 || n = 428 || n = 433 || n = 438 || n = 443 || n = 448 || n = 458 || n = 463)
	; 8..9 bytes
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
.elseif(n = 329 || n = 364 || n = 434)
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
.elseif(n = 363)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 381)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 383)
	; 6 bytes
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
.elseif(n = 399)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
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
.elseif(n = 461)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 468)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif(n = 472 || n = 477 || n = 479 || n = 482 || n = 486 || n = 487 || n = 492 || n = 494 || n = 497 || n = 499 || n = 511 || n = 512 || n = 517 || n = 519 || n = 522 || n = 524 || n = 527 || n = 529 || n = 534 || n = 536 || n = 539 || n = 541 || n = 542 || n = 546 || n = 547 || n = 549 || n = 552 || n = 557 || n = 559 || n = 562 || n = 566 || n = 567 || n = 569 || n = 574 || n = 577 || n = 582 || n = 584 || n = 587 || n = 589 || n = 592 || n = 594 || n = 597 || n = 602 || n = 604 || n = 609 || n = 612 || n = 616 || n = 617 || n = 619 || n = 622 || n = 624)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 472 && n < 484) || (n > 492 && n < 519) || (n > 527 && n < 554) || (n > 562 && n < 589) || (n > 597 && n < 624)))
	; 9 bytes
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
.elseif(n = 476)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 481 || n = 488 || n = 509 || n = 558 || n = 572 || n = 579 || n = 593)
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
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
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
.elseif(n = 496 || n = 506 || n = 526 || n = 531 || n = 561 || n = 571 || n = 576 || n = 591 || n = 596 || n = 601 || n = 606)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 501)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
.elseif(n = 514)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 564)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
.elseif(n = 581 || n = 599)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
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
.elseif(n = 636 || n = 646 || n = 666 || n = 676 || n = 681 || n = 696 || n = 706 || n = 711 || n = 746 || n = 751 || n = 766 || n = 771 || n = 776)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif((n .mod 5 = 3) && ((n > 637 && n < 659) || (n > 667 && n < 684) || n = 693 || (n > 702 && n < 729) || (n > 742 && n < 764) || n = 773 || n = 778))
	; 9 bytes
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
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 661)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
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
.elseif(n = 694)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	tay
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
.elseif(n = 741 || n = 781)
	; 9 bytes
	tya
	ldy #(n-13)/8
	cmp $C5
	dey
	Jpl *-3
	tay
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
.elseif((n .mod 5 = 3) && (n = 783 || n = 798 || (n > 807 && n < 834) || (n > 842 && n < 869) || (n > 877 && n < 899) || (n > 912 && n < 934)))
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
.elseif(n = 786)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 787 || n = 792 || n = 794 || n = 797 || n = 799 || n = 802 || n = 807 || n = 809 || n = 812 || n = 814 || n = 822 || n = 826 || n = 827 || n = 829 || n = 832 || n = 834 || n = 837 || n = 839 || n = 842 || n = 844 || n = 847 || n = 849 || n = 857 || n = 861 || n = 862 || n = 864 || n = 867 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 892 || n = 897 || n = 899 || n = 902 || n = 904 || n = 907 || n = 909 || n = 912 || n = 914 || n = 917 || n = 919 || n = 927 || n = 931 || n = 932 || n = 934 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 788)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 791)
	; 9 bytes
	php
	lda $A5
	lda #3
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 793)
	; 7 bytes
	php
	lda #3
	jsr delay_256a_16_clocks
	plp
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
.elseif((n .mod 5 = 1) && (n = 811 || n = 816 || n = 841 || n = 846 || n = 856 || n = 871 || n = 876 || n = 886 || n = 891 || (n > 905 && n < 927)))
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
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
.if(n = 938 || n = 948 || n = 953 || n = 958 || n = 963 || n = 968 || n = 973 || n = 983 || n = 988 || n = 993 || n = 998 || n = 1003 || n = 1008 || n = 1021 || n = 1023 || n = 1028 || n = 1033 || n = 1038 || n = 1043 || n = 1058 || n = 1063 || n = 1068 || n = 1073 || n = 1078 || n = 1088 || n = 1093)
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
.elseif(n = 942 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 967 || n = 972 || n = 974 || n = 977 || n = 979 || n = 982 || n = 984 || n = 987 || n = 989 || n = 997 || n = 1001 || n = 1002 || n = 1004 || n = 1007 || n = 1009 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1032 || n = 1036 || n = 1037 || n = 1039 || n = 1042 || n = 1046 || n = 1047 || (n > 1050 && n < 1055) || n = 1057 || n = 1059 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1079 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1092)
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
.elseif((n .mod 5 = 1) && ((n > 945 && n < 962) || n = 991 || n = 996 || n = 1016 || n = 1026 || n = 1066 || (n > 1080 && n < 1092)))
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 959 || n = 994 || n = 1029 || n = 1064)
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
	ldy #(n-13)/8
	cmp $C5
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
.elseif(n = 1024)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	tax
.elseif(n = 1031)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
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
.if(n = 1094 || n = 1102 || n = 1106 || n = 1107 || n = 1109 || n = 1112 || n = 1114 || n = 1117 || n = 1122 || n = 1124 || n = 1127 || n = 1129 || n = 1137 || n = 1141 || n = 1142 || n = 1144 || n = 1147 || n = 1149 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1162 || n = 1164 || n = 1172 || n = 1176 || n = 1177 || n = 1179 || n = 1182 || n = 1184 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1197 || n = 1199 || n = 1207 || n = 1212 || n = 1214 || n = 1217 || n = 1219 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1232 || n = 1234 || n = 1242 || n = 1246 || n = 1247 || n = 1249)
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
.elseif((n .mod 5 = 1) && (n = 1096 || n = 1116 || (n > 1125 && n < 1137) || (n > 1150 && n < 1162) || n = 1171 || n = 1186 || (n > 1195 && n < 1207) || n = 1226 || n = 1231))
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
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
.elseif(n = 1099 || n = 1134 || n = 1169 || n = 1204 || n = 1239)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1101 || n = 1221)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
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
	delay_n 4
	dey
	Jpl *-3
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
.elseif(n = 1252 || n = 1254 || n = 1257 || n = 1259 || n = 1262 || n = 1264 || n = 1267 || n = 1269 || n = 1277 || n = 1281 || n = 1282 || n = 1284 || n = 1287 || n = 1289)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && (n = 1253 || (n > 1262 && n < 1289)))
	; 9 bytes
	delay_n (n-3)
	lda $A5
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
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1261)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1266 || n = 1271 || n = 1276)
	; 9 bytes
	tya
	ldy #(n-11)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
	; 9 bytes
	tya
	ldy #0
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 1292)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1293 || n = 1314 || n = 1321 || n = 1328)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1294 || n = 1295 || n = 1297 || n = 1302 || n = 1304 || n = 1306 || n = 1307 || n = 1309 || n = 1311 || n = 1315 || n = 1316 || n = 1323)
	; 8..10 bytes
	delay_n (n-2)
	nop
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
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $70A0'
	ldy #112
	iny
	Jne *-4
	tay
.elseif(n = 1300)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1301 || n = 1310 || n = 1319)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1303)
	; 9 bytes
	php
	lda $A5
	lda #5
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1305)
	; 7 bytes
	php
	lda #5
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1308)
	; 9 bytes
	php
	lda $A5
	lda #5
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks
	pla
	plp
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
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1318)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
.elseif(n = 1320)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1324)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1326)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.elseif(n = 1327)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.endif
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1330 || n = 1332 || n = 1343 || n = 1344 || n = 1348 || n = 1350 || n = 1351 || n = 1358 || n = 1360 || n = 1365 || n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1388 || n = 1392 || n = 1393 || n = 1395)
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
	cmp $C5
	dey
	Jne *-3
	tay
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
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
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
.elseif(n = 1340 || n = 1347 || n = 1368 || n = 1375 || n = 1396)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1345)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
.elseif(n = 1353)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $96A0'
	ldy #150
	dey
	Jne *-4
	tay
.elseif(n = 1354)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $96A0'
	ldy #150
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
.elseif(n = 1362)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $69A0'
	ldy #105
	iny
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
.elseif(n = 1369)
	; 10 bytes
	tya
	ldy #123
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif(n = 1371)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $98A0'
	ldy #152
	dey
	Jne *-4
	tay
.elseif(n = 1376 || n = 1404)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1387)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1390)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
.elseif(n = 1399)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
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
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if(n = 1407 || n = 1411 || n = 1414 || n = 1420 || n = 1421 || n = 1423 || n = 1428 || n = 1435 || n = 1439 || n = 1442)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1408 && n < 1446)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1410 || n = 1431)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1441)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1413 || n = 1429 || n = 1437)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1415 || n = 1422)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1416)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $9DA0'
	ldy #157
	dey
	Jne *-4
	tay
.elseif(n = 1417)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $9DA0'
	ldy #157
	dey
	Jne *-4
	tay
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
	cmp #<$CD ;hides 'cmp $62A0'
	ldy #98
	iny
	Jne *-4
	tay
.elseif(n = 1430)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	sta (@ptrtemp,x)
	tax
.elseif(n = 1432)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1434)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
	tax
.elseif(n = 1438)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
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
	cmp #<$CD ;hides 'cmp $60A0'
	ldy #96
	iny
	Jne *-4
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
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
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
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1449 || n = 1451 || n = 1456 || n = 1458 || n = 1470 || n = 1474 || n = 1477 || n = 1483 || n = 1484)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1452)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A1A0'
	ldy #161
	dey
	Jne *-4
	tay
.elseif(n = 1453)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1455)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
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
.elseif(n = 1460 || n = 1467)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1462)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
.elseif(n = 1463 || n = 1472 || n = 1481)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1465)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1466 || n = 1473)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1469)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1471)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
.elseif(n = 1476)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1479)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.elseif(n = 1480)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485 || n = 1509)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1486 || n = 1491 || n = 1495 || n = 1498 || n = 1500 || n = 1505 || n = 1512 || n = 1514 || n = 1519)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1487 || n = 1494)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1488)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A5A0'
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
.elseif(n = 1490 || n = 1499 || n = 1508)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1492 || n = 1513 || n = 1518)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1493)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1497)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A6A0'
	ldy #166
	dey
	Jne *-4
	tay
.elseif(n = 1501)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jmi *-4
	tay
.elseif(n = 1502)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1506)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
.elseif(n = 1507)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A7A0'
	ldy #167
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
.elseif(n = 1516)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A8A0'
	ldy #168
	dey
	Jne *-4
	tay
.elseif(n = 1520)
	; 9 bytes
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1521)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1522)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1523)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
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
.elseif(n = 1525 || n = 1549 || n = 1557)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1525 && n < 1563)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1527 || n = 1548 || n = 1555)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1528)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1529 || n = 1541)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1530)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1532)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $75A0'
	ldy #117
	nop
	iny
	Jne *-5
	tay
.elseif(n = 1533 || n = 1537 || n = 1539 || n = 1540 || n = 1546 || n = 1547 || n = 1554 || n = 1558 || n = 1560)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1534)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
.elseif(n = 1536 || n = 1550)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1542)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
.elseif(n = 1543)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
.elseif(n = 1551)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $ACA0'
	ldy #172
	dey
	Jne *-4
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
.endif
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563 || n = 1565 || n = 1567 || n = 1574 || n = 1575 || n = 1582 || n = 1584 || n = 1590 || n = 1596)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564)
	; 9 bytes
	php
	lda $A5
	lda #6
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1569)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1570)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1571 || n = 1589 || n = 1598)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1572)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1576 || n = 1583)
	; 10 bytes
	delay_n (n-3)
	lda $A5
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
	cmp #<$CD ;hides 'cmp $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1579)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
.elseif(n = 1581 || n = 1597)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1585 || n = 1592 || n = 1599)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1586 || n = 1593)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1588)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1591)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1595)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1600)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1603 || n = 1609 || n = 1610 || n = 1612 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1638 || n = 1640)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1604 || n = 1611 || n = 1618 || n = 1630 || n = 1639)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1605 || n = 1621)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1606)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	dey
	Jne *-4
	tay
.elseif(n = 1607 || n = 1616 || n = 1625)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
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
.elseif(n = 1613 || n = 1637)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1614)
	; 8 bytes
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
.elseif(n = 1620)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1623)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $B4A0'
	ldy #180
	dey
	Jne *-4
	tay
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
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1634)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1635)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.endif
.endif
.else
; 1641 to 1718
.if(n = 1641)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1642)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1644 || n = 1667 || n = 1674 || n = 1681 || n = 1702 || n = 1716)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1645 || n = 1659 || n = 1663 || n = 1666 || n = 1672 || n = 1673 || n = 1675 || n = 1680 || n = 1682 || n = 1687 || n = 1689 || n = 1694 || n = 1701 || n = 1703 || n = 1707 || n = 1708 || n = 1710)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1648 || n = 1654 || n = 1700)
	; 9..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1649 || n = 1709)
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
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1651 && n < 1698) || n = 1715))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1653 || n = 1669 || n = 1677 || n = 1693 || n = 1717)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1655 || n = 1662 || n = 1676 || n = 1683 || n = 1690 || n = 1711 || n = 1718)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1656 || n = 1684 || n = 1691 || n = 1698 || n = 1712)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1658)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1660)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
.elseif(n = 1665)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1668)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $B9A0'
	ldy #185
	dey
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
.elseif(n = 1695)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1696)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1704)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1705)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
.elseif(n = 1714)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1722 || n = 1726 || n = 1729 || n = 1735 || n = 1736 || n = 1738 || n = 1743 || n = 1747 || n = 1750 || n = 1752 || n = 1757)
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
	cmp <$CD ;hides 'cmp $BFA0'
	ldy #191
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1723 && n < 1752)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1749)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1728 || n = 1746)
	; 9..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1730 || n = 1737)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1731)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
.elseif(n = 1732)
	; 9 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	iny
	Jne *-2
	pla
	tay
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
	cmp #<$CD ;hides 'cmp $3FA0'
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
.elseif(n = 1753)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.endif
.else
; 1758 to 1796
.if(n = 1758)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
	tay
.elseif(n = 1759)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
	tay
.elseif(n = 1760 || n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1761 || n = 1782)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
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
	php
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	plp
.elseif(n = 1764 || n = 1766 || n = 1770 || n = 1771 || n = 1785 || n = 1791 || n = 1792)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1765 || n = 1773 || n = 1789)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1767)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1768)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1772)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1774 || n = 1788)
	; 10 bytes
	php
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	plp
.elseif(n = 1775)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1777)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	dey
	Jne *-4
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
.elseif(n = 1781 || n = 1793)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1784)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
	tay
.elseif(n = 1786)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	dey
	Jne *-4
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
	cmp <$CD ;hides 'cmp $39A0'
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
.elseif(n = 1798 || n = 1809)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1799 || n = 1807 || n = 1814 || n = 1816 || n = 1818 || n = 1819 || n = 1821 || n = 1825 || n = 1826 || n = 1834)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1833)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1801 || n = 1810)
	; 10 bytes
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
.elseif(n = 1804)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
.elseif(n = 1805 || n = 1823 || n = 1832)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1808)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	nop
	dey
	Jne *-5
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
.elseif(n = 1813)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1815)
	; 9 bytes
	php
	lda $A5
	lda #7
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 1817)
	; 7 bytes
	php
	lda #7
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1820)
	; 9 bytes
	php
	lda $A5
	lda #7
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1822)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	iny
	Jne *-4
	tay
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 1829)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1830)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
.elseif(n = 1831)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	iny
	Jne *-4
	tay
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
.if(n = 1836)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	sta (@ptrtemp,x)
	tax
.elseif(n = 1837 || n = 1845 || n = 1861 || n = 1869)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1838)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 1839)
	; 9 bytes
	tya
	ldy #204 ;hides 'cpy abs'
	cmp #$A9
	dey
	Jne *-4
	tay
.elseif(n = 1840)
	; 9 bytes
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	tax
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
.elseif(n = 1844 || n = 1872)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 1847 || n = 1851 || n = 1852 || n = 1854 || n = 1855 || n = 1860 || n = 1863 || n = 1870 || n = 1871)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1848)
	; 9 bytes
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	dex
	Jne *-4
	tax
.elseif(n = 1849)
	; 9 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1853 || n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1856)
	; 11 bytes
	php
	delay_n 39
	lda #7
	jsr delay_256a_16_clocks
	plp
.elseif(n = 1857)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
.elseif(n = 1858)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
.elseif(n = 1862)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $A9A0'
	ldy #169
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1866)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	dey
	Jne *-4
	tay
.elseif(n = 1867)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	iny
	Jne *-4
	tay
.elseif(n = 1873)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $AAA0'
	ldy #170
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1874)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1875)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
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
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1876 && n < 1914)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1878 || n = 1879 || n = 1883 || n = 1887 || n = 1888 || n = 1891 || n = 1906)
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
.elseif(n = 1881)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1882 || n = 1890 || n = 1898)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	plp
.elseif(n = 1884)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	dey
	Jne *-4
	tay
.elseif(n = 1885 || n = 1893 || n = 1909)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1889 || n = 1901)
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
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jne *-4
	tay
.elseif(n = 1896)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $ACA0'
	ldy #172
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1897 || n = 1908)
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
.elseif(n = 1900)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1902)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
.elseif(n = 1903)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	dey
	Jne *-4
	tay
.elseif(n = 1905)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
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
.elseif(n = 1911)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1912)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	dey
	Jne *-4
	tay
.elseif(n = 1914)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1924 || n = 1942 || n = 1943 || n = 1951)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1916 || n = 1918 || n = 1927 || n = 1936 || n = 1944)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1917 || n = 1933 || n = 1941)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1919)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1920)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
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
.elseif((n .mod 9 = 5) && ((n > 1921 && n < 1950)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1923)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 1925 || n = 1937)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1926)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 1928)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $AFA0'
	ldy #175
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1929)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1930)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
.elseif(n = 1932 || n = 1934 || n = 1950)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1935 || n = 1945)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1938)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	dey
	Jne *-4
	tay
.elseif(n = 1939)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	iny
	Jne *-4
	tay
.elseif(n = 1946)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
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
.elseif(n = 1948)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
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
.elseif(n = 1953)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954 || n = 1971 || n = 1980 || n = 1988)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1955)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1956)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	iny
	Jne *-4
	tay
.elseif(n = 1957 || n = 1965 || n = 1981 || n = 1989)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif((n .mod 9 = 5) && ((n > 1957 && n < 1986)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1959 || n = 1960 || n = 1968 || n = 1969 || n = 1977 || n = 1978 || n = 1986 || n = 1987 || n = 1991)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1961 || n = 1973)
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
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	nop
	dey
	Jne *-5
	tay
.elseif(n = 1963)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1964)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1966)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
.elseif(n = 1970)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1974)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 1975)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
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
	tya
	ldy #(n-11)/9
	delay_n 4
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 1983)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	iny
	Jne *-4
	tay
.elseif(n = 1984)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1990)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 1992)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
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
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1993 && n < 2031)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1995 || n = 2015 || n = 2025)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1996 || n = 2007 || n = 2018)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 1997 || n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1998)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1999 || n = 2004 || n = 2014 || n = 2022 || n = 2023 || n = 2031)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2000)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2001)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
.elseif(n = 2002)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
.elseif(n = 2005 || n = 2013 || n = 2029)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 2006)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2008)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2010)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2011)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	tay
.elseif(n = 2016)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2017)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2019)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	dey
	Jne *-4
	tay
.elseif(n = 2020)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	dey
	Jne *-4
	tay
.elseif(n = 2024)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2026)
	; 11 bytes
	php
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
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
.if(n = 2032 || n = 2041 || n = 2043 || n = 2050 || n = 2059 || n = 2067 || n = 2070)
	; 8..11 bytes
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
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 2038)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	iny
	Jne *-4
	tay
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
.elseif(n = 2044 || n = 2052)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2046)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
.elseif(n = 2047)
	; 9 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
.elseif(n = 2053)
	; 9 bytes
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 2054)
	; 11 bytes
	tya
	ldy #(n-11)/9
	delay_n 4
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2055)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
	tax
.elseif(n = 2056)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
.elseif(n = 2058)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
	tay
.elseif(n = 2060)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2061)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $BBA0'
	ldy #187
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2064)
	; 9 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	tax
.elseif(n = 2065)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
.elseif(n = 2068)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071)
	; 9 bytes
	php
	lda $A5
	lda #8
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2072 || n = 2075 || n = 2077 || n = 2079 || n = 2086)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2073)
	; 7 bytes
	php
	lda #8
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2074)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
.elseif(n = 2076)
	; 9 bytes
	php
	lda $A5
	lda #8
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2078)
	; 10 bytes
	php
	nop
	lda $A5
	lda #8
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2081)
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
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	tay
.elseif(n = 2084)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2085)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2087)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 2088)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	sta (@ptrtemp,x)
	tax
.elseif(n = 2089)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.endif
.else
; 2090 to 2109
.if(n = 2090)
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
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	tay
.elseif(n = 2093 || n = 2102)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2094)
	; 10 bytes
	delay_n (n-3)
	lda $A5
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
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2098)
	; 10 bytes
	pha
	delay_n (n-7)
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
.elseif(n = 2100)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
.elseif(n = 2101)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
.elseif(n = 2103 || n = 2104 || n = 2107 || n = 2108)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2105)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
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
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110)
	; 9 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	tax
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2148)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2123 || n = 2132 || n = 2144)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2113 || n = 2121 || n = 2122 || n = 2130 || n = 2131 || n = 2140 || n = 2142 || n = 2143 || n = 2148)
	; 10..11 bytes
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
.elseif(n = 2115 || n = 2135)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2116 || n = 2124)
	; 10..11 bytes
	pha
	delay_n (n-7)
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
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
.elseif(n = 2119)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
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
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2127)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $ECA0'
	ldy #236
	dey
	Jne *-4
	tay
.elseif(n = 2128)
	; 9 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
	Jne *-4
	tay
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
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	iny
	Jne *-4
	tay
.elseif(n = 2137)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
.elseif(n = 2139)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2145)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.elseif(n = 2146)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2157 || n = 2158 || n = 2162 || n = 2166 || n = 2167 || n = 2169 || n = 2176 || n = 2178 || n = 2179 || n = 2184 || n = 2185 || n = 2187)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2150 || n = 2161)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2151)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	cmp $C5
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2152)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
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
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	iny
	Jne *-4
	tay
.elseif(n = 2155)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2155 && n < 2184)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2159)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2160)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2163)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2164)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
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
.elseif(n = 2170)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
.elseif(n = 2171)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2172)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2173)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2175)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2181)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
.elseif(n = 2182)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
.elseif(n = 2186)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188 || n = 2197 || n = 2206)
	; 11 bytes
	php
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	plp
.elseif(n = 2189 || n = 2213 || n = 2225)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2190)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
.elseif(n = 2191)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2191 && n < 2220)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2193)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2194 || n = 2205 || n = 2216)
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
.elseif(n = 2196)
	; 11 bytes
	pha
	delay_n (n-7)
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
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
.elseif(n = 2200)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
.elseif(n = 2202 || n = 2207 || n = 2211 || n = 2212 || n = 2215 || n = 2220 || n = 2221 || n = 2223)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2203)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2204)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2208)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
.elseif(n = 2209)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
.elseif(n = 2214)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2217)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	dey
	Jne *-4
	tay
.elseif(n = 2218)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
.elseif(n = 2222)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2224)
	; 10 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	iny
	Jne *-3
	pla
	tay
.elseif(n = 2226)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	tay
.endif
.else
; 2227 to 2265
.if(n = 2227)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 2227 && n < 2265)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2229 || n = 2230 || n = 2239 || n = 2247 || n = 2248 || n = 2250 || n = 2251 || n = 2256 || n = 2257)
	; 10..11 bytes
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
	lda #(n-10)/9
	delay_n 4
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
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2238 || n = 2260)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2240)
	; 10 bytes
	php
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
	plp
.elseif(n = 2241)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2242 || n = 2243 || n = 2252)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2244)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
.elseif(n = 2245)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
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
.elseif(n = 2253)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
.elseif(n = 2254)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
.elseif(n = 2258)
	; 10 bytes
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	nop
	dex
	Jne *-5
	tax
.elseif(n = 2262)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2263)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
.elseif(n = 2265)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2274 || n = 2275 || n = 2277 || n = 2283 || n = 2284 || n = 2286 || n = 2292 || n = 2302)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267 || n = 2276 || n = 2294)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2268 || n = 2278 || n = 2279 || n = 2288 || n = 2296)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2269)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2270)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2271)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
.elseif(n = 2272)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	iny
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
.elseif(n = 2280)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
.elseif(n = 2281)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
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
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2290)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
.elseif(n = 2293 || n = 2304)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2295)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2298)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2299)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
.elseif(n = 2303)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	nop
	iny
	Jne *-5
	tay
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2306 || n = 2316 || n = 2322)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2307)
	; 9 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
.elseif(n = 2308)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
.elseif(n = 2309)
	; 9 bytes
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2311)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2312)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2313)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2314)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2315)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2317)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2318)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2319)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2320)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	cmp $C5
	dey
	Jne *-6
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
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	cmp $C5
	iny
	Jne *-4
	tay
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2325)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2326 || n = 2328 || n = 2330 || n = 2331 || n = 2333 || n = 2338 || n = 2340)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2327)
	; 9 bytes
	php
	lda $A5
	lda #9
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2329)
	; 7 bytes
	php
	lda #9
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2332)
	; 9 bytes
	php
	lda $A5
	lda #9
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2334)
	; 10 bytes
	php
	nop
	lda $A5
	lda #9
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2335)
	; 9 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2336)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2337)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2339)
	; 11 bytes
	php
	pha
	lda $A5
	lda #9
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2341)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2342)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2343)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	dey
	Jne *-4
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
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C3A0'
	ldy #195
	cmp $C5
	dey
	Jne *-6
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
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2347 || n = 2349 || n = 2361 || n = 2372 || n = 2377 || n = 2382)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2348 || n = 2359 || n = 2370)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2350 || n = 2352 || n = 2353 || n = 2364 || n = 2376)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2351)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2354)
	; 12 bytes
	php
	lda #<(n-47)
	jsr delay_a_25_clocks
	lda #>(n-47)
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2355 || n = 2365 || n = 2375)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2356)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2358)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2360)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2362 || n = 2374)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2363)
	; 11 bytes
	php
	delay_n 39
	lda #9
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2366)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2367)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2368)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 6
	dey
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
.elseif(n = 2373)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2378)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2379)
	; 9 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2380)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2387 || n = 2394 || n = 2395 || n = 2407 || n = 2409 || n = 2416 || n = 2419 || n = 2421)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384)
	; 11 bytes
	php
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
	plp
.elseif(n = 2385 || n = 2399 || n = 2413)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2386 || n = 2388 || n = 2400)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2389)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2390)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2391)
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	cmp $C5
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2392 || n = 2403 || n = 2414)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2393 || n = 2405 || n = 2417)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2396 || n = 2408)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2397)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2398 || n = 2410)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2401)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2402)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2404)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2406)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2411)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2412)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2415)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2418)
	; 11 bytes
	tya
	ldy #126
	delay_n 14
	dey
	Jpl *-5
	tay
.elseif(n = 2420)
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
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2423)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2424)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	tay
.elseif((n .mod 11 = 5) && ((n > 2424 && n < 2459)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2426 || n = 2431 || n = 2433 || n = 2438 || n = 2443)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2427 || n = 2455)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2428)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2429 || n = 2441 || n = 2453)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2430 || n = 2454)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2432 || n = 2442 || n = 2444)
	; 10..11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2434)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2435 || n = 2445)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2437)
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
.elseif(n = 2440)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2446 || n = 2448 || n = 2460)
	; 10..11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2449)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
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
.elseif(n = 2452)
	; 11 bytes
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	cmp $C5
	dex
	Jne *-6
	tax
.elseif(n = 2456)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2457)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2459)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2462)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2463)
	; 11 bytes
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	cmp $C5
	dex
	Jne *-6
	tax
.elseif(n = 2464)
	; 11 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2465 || n = 2477 || n = 2489)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2466 || n = 2498)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2467 || n = 2471 || n = 2479 || n = 2481 || n = 2493 || n = 2499)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2468)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2469 || n = 2480 || n = 2491)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2470 || n = 2487)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2472 || n = 2484 || n = 2496)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2473)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $BEA0'
	ldy #190
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2474)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 2475 || n = 2485 || n = 2495)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2476)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2478)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2482 || n = 2494)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2483)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2486 || n = 2492)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	iny
	Jne *-5
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
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2502 || n = 2524 || n = 2535)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2503 || n = 2507 || n = 2515 || n = 2517 || n = 2526 || n = 2527 || n = 2529)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2504 || n = 2521 || n = 2538)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2505)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2506 || n = 2518 || n = 2530)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2508 || n = 2520 || n = 2532)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2509)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	cmp $C5
	inx
	inx
	Jne *-6
	tax
.elseif(n = 2510)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2514)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2516 || n = 2528)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2519)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2522)
	; 10 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2523)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	tay
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
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2534)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2536)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	cmp $C5
	dey
	Jne *-6
	tay
.endif
.else
; 2540 to 2578
.if(n = 2540 || n = 2570)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2541)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2542 || n = 2554)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2543)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
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
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2546 || n = 2557 || n = 2568)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2547)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2548 || n = 2551 || n = 2563 || n = 2569 || n = 2574 || n = 2575)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2549 || n = 2561 || n = 2573)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2550)
	; 11 bytes
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	sta (@ptrtemp,x)
	tax
.elseif(n = 2552 || n = 2564 || n = 2576)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2553 || n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
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
.elseif(n = 2556)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2558)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
.elseif(n = 2559)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dey
	Jne *-5
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
.elseif(n = 2562)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2565)
	; 10 bytes
	tya
	ldy #0
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2566)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2571)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2572)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2577)
	; 9 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2578)
	; 10 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	tax
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2601 || n = 2612)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2580)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2581 || n = 2595)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2582 || n = 2584 || n = 2586 || n = 2587 || n = 2589 || n = 2591 || n = 2594 || n = 2596 || n = 2598 || n = 2599 || n = 2611 || n = 2613)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583)
	; 9 bytes
	php
	lda $A5
	lda #10
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2585)
	; 7 bytes
	php
	lda #10
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2588)
	; 9 bytes
	php
	lda $A5
	lda #10
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2590)
	; 10 bytes
	php
	nop
	lda $A5
	lda #10
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2593)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2597 || n = 2609)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2600)
	; 10 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	delay_n 5
	dey
	Jne *-5
	tay
.elseif(n = 2602 || n = 2614)
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
.elseif(n = 2604 || n = 2616)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2605)
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
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	cmp $C5
	iny
	Jne *-6
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
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2615)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2617)
	; 11 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2631)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2619)
	; 11 bytes
	php
	delay_n 39
	lda #10
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2620)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2621 || n = 2633 || n = 2645)
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
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2623 || n = 2634 || n = 2656)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2624)
	; 11 bytes
	php
	delay_n 39
	lda #10
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2625 || n = 2635 || n = 2636 || n = 2639 || n = 2647 || n = 2649 || n = 2653)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2627 || n = 2646)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2628 || n = 2652)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2629)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2630)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2632)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
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
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2641)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2642)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	tax
.elseif(n = 2643)
	; 10 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	inx
	Jne *-5
	tax
.elseif(n = 2644)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2648)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2654)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2655)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2658 || n = 2659 || n = 2661 || n = 2671 || n = 2673 || n = 2680 || n = 2683 || n = 2685 || n = 2695)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675 || n = 2690)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2662 || n = 2686)
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
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
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
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2667 || n = 2678 || n = 2689)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2668)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2670)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2672 || n = 2684)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2674 || n = 2691)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2676)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2677)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2682)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2687)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2688)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2692)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2694)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	delay_n 4
	dey
	Jne *-6
	tay
.endif
.else
; 2696 to 2734
.if(n = 2696)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2697 || n = 2701 || n = 2702 || n = 2713 || n = 2719 || n = 2723)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2698)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2699)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
.elseif((n .mod 11 = 5) && ((n > 2699 && n < 2734)))
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2703)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 2704)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2705 || n = 2717 || n = 2729)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2706 || n = 2714 || n = 2718 || n = 2726 || n = 2730)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
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
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2710)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2712 || n = 2724)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2715)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2716)
	; 11 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 4
	iny
	Jne *-4
	pla
	tay
.elseif(n = 2720)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2727)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2728)
	; 11 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	cmp $C5
	inx
	Jne *-6
	tax
.elseif(n = 2731)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2732)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2734)
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
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735 || n = 2749 || n = 2763)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2736 || n = 2738 || n = 2748 || n = 2760)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2737 || n = 2743 || n = 2745 || n = 2751 || n = 2757 || (n > 2766 && n < 2770))
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2739)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	cmp $C5
	inx
	Jne *-6
	tax
.elseif(n = 2740)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	cmp $C5
	iny
	Jne *-6
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
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2744 || n = 2755 || n = 2766)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2746 || n = 2758)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2747)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2750)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2752)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2754)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2756)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2759)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2761)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2762)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 2764)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2770)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2771)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2772)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 2773)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2800)
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
	cmp $C5
	dex
	dex
	Jne *-5
	tax
.elseif(n = 2776)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
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
.elseif(n = 2778 || n = 2779 || n = 2781 || n = 2790 || n = 2807 || n = 2812)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2780 || n = 2795)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2782 || n = 2794 || n = 2806)
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
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2784 || n = 2796)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2785)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2786)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2787)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	cmp $C5
	iny
	Jne *-5
	tay
.elseif(n = 2788 || n = 2799 || n = 2810)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2791 || n = 2805)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2792 || n = 2802 || n = 2804)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2797)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2798)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
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
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2809)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2811)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	cmp $C5
	dey
	Jne *-5
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
.elseif(n = 2814)
	; 10 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2815 || n = 2817 || n = 2822 || n = 2823 || n = 2827 || n = 2829 || n = 2834 || n = 2835 || n = 2838 || n = 2840 || n = 2842 || n = 2843 || n = 2845 || n = 2847 || n = 2850 || n = 2851)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2818 || n = 2830)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2819 || n = 2833)
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
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
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
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2826)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2831)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2832)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2836)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2839)
	; 9 bytes
	php
	lda $A5
	lda #11
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2841)
	; 7 bytes
	php
	lda #11
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2844)
	; 9 bytes
	php
	lda $A5
	lda #11
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2846)
	; 10 bytes
	php
	nop
	lda $A5
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
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2857 || n = 2858 || n = 2863 || n = 2887)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2854 || n = 2866 || n = 2890)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2855)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 2856 || n = 2868 || n = 2880)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2859)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2860)
	; 11 bytes
	php
	delay_n 24
	lda #11
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 2861 || n = 2873 || n = 2885)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2862)
	; 12 bytes
	php
	delay_n 21
	lda #11
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2864 || n = 2876 || n = 2888)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2865)
	; 11 bytes
	php
	delay_n 24
	lda #11
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
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2870)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2871)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2872)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	cmp $C5
	dey
	Jne *-6
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
.elseif(n = 2881 || n = 2882 || n = 2886)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2883)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	cmp $C5
	inx
	Jne *-6
	tax
.elseif(n = 2884)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	cmp $C5
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif((n .mod 12 = 0) && ((n > 2891 && n < 2929)))
	; 11 bytes
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
.elseif(n = 2894 || n = 2898 || n = 2906 || n = 2910 || n = 2918 || n = 2922)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2895 || n = 2912 || n = 2929)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2896)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2897 || n = 2909 || n = 2921)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2899 || n = 2901 || n = 2905 || n = 2911 || n = 2913 || n = 2919 || n = 2923 || n = 2925)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2900 || n = 2915)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2902 || n = 2914 || n = 2926)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2903 || n = 2917)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2907)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2908)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2920)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2924)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2927)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2960)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2931 || n = 2959)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2932)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2933 || n = 2945 || n = 2957)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2934 || n = 2953 || n = 2958 || n = 2966)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2935 || n = 2937 || n = 2947 || n = 2949)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2936 || n = 2948)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2938 || n = 2950 || n = 2962)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2939 || n = 2951)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2940 || n = 2952 || n = 2964)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2941)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2942)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2943 || n = 2956)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2944)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 2946 || n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2954)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2955)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2961)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2965)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 2967)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 2968)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	cmp $C5
	dey
	Jne *-6
	tay
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
.elseif(n = 2970 || n = 2977 || n = 2978 || n = 3002)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 2971 || n = 2983 || n = 2985 || n = 2989 || n = 2995 || n = 3007)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2972 || n = 2984 || n = 2996)
	; 11 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2973 || n = 2987 || n = 3001)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2974)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 2975 || n = 2990)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 2976 || n = 2988 || n = 3000)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2979)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	cmp $C5
	dey
	Jne *-5
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
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 2986 || n = 2998)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 2991)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2992)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 2994)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 2999)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	nop
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 3003)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3004)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	cmp $C5
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
.endif
.else
; 3008 to 3046
.if(n = 3008 || n = 3021)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3009 || n = 3031 || n = 3033 || n = 3042)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010 || n = 3022 || n = 3046)
	; 11 bytes
	php
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	plp
.elseif(n = 3011 || n = 3023)
	; 11 bytes
	tya
	ldy #(n-11)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 3012 || n = 3024 || n = 3034 || n = 3036)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3013)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
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
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3017 || n = 3029 || n = 3041)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3018 || n = 3025 || n = 3037 || n = 3038)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3019)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 3020 || n = 3035)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
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
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3030 || n = 3044)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3040)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3045)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047 || n = 3073)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
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
.elseif(n = 3049 || n = 3054 || n = 3055 || n = 3059 || n = 3062 || n = 3066 || n = 3067 || n = 3069 || n = 3074 || n = 3079 || n = 3081)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3050 || n = 3080)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3051)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3052)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3056 || n = 3068)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 3060 || n = 3072 || n = 3078 || n = 3084)
	; 11..12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3061)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3063)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	cmp $C5
	iny
	Jne *-6
	tay
.elseif(n = 3064 || n = 3083)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3075)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 3076)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
	dey
	Jne *-6
	tay
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3082)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3112)
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif((n > 3086 && n < 3090) || n = 3094 || n = 3096 || n = 3098 || n = 3099 || n = 3101 || n = 3103 || n = 3106 || n = 3108 || n = 3114 || n = 3115 || n = 3118)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3090)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3091)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
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
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3095)
	; 9 bytes
	php
	lda $A5
	lda #12
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3097)
	; 7 bytes
	php
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3100)
	; 9 bytes
	php
	lda $A5
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3102)
	; 10 bytes
	php
	nop
	lda $A5
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
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3107)
	; 11 bytes
	php
	pha
	lda $A5
	lda #12
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3109)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3110 || n = 3125)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3111)
	; 11 bytes
	php
	pha
	delay_n (n-14)
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
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3117)
	; 12 bytes
	php
	delay_n 20
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3119 || n = 3120 || n = 3122)
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
	delay_n 24
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3123)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 4
	inx
	Jne *-6
	tax
.elseif(n = 3124)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 4
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
; 3126 to 3203
.if(n = 3126 || n = 3128 || n = 3130 || n = 3132 || n = 3134 || n = 3142 || (n > 3143 && n < 3149) || n = 3153 || n = 3154 || n = 3158 || n = 3160 || n = 3161 || n = 3165 || n = 3166 || n = 3168 || n = 3174 || (n > 3178 && n < 3183) || (n > 3190 && n < 3196))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 14 = 5) && ((n > 3126 && n < 3198)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3129 || n = 3135 || n = 3143 || n = 3152 || n = 3171 || n = 3186 || n = 3199)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3131)
	; 11 bytes
	php
	delay_n 39
	lda #12
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3133 || n = 3150 || n = 3167 || n = 3184 || n = 3201)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3136)
	; 11 bytes
	php
	delay_n 39
	lda #12
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3137)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
	tay
.elseif((n .mod 13 = 5) && ((n > 3137 && n < 3191)))
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3139)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3140 || n = 3170 || n = 3185 || n = 3200)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3149)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3156)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3157 || n = 3173 || n = 3189)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3159 || n = 3178)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3162)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3163)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3172)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3175)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3176)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 4
	iny
	Jne *-6
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
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3196)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3198)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 3202)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	delay_n 4
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
.else
; 3204 to 3281
.if(n = 3204)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3205 || n = 3221 || n = 3237)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif((n > 3205 && n < 3211) || n = 3212 || n = 3217 || (n > 3221 && n < 3225) || n = 3226 || (n > 3230 && n < 3235) || n = 3238 || n = 3244 || (n > 3245 && n < 3251) || n = 3259 || (n > 3260 && n < 3266) || n = 3270 || n = 3272 || n = 3274 || (n > 3275 && n < 3279))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 14 = 5) && ((n > 3210 && n < 3282)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3213 || n = 3241 || n = 3271)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3214)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 4
	dey
	Jne *-6
	tay
.elseif((n .mod 15 = 5) && ((n > 3214 && n < 3276)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif((n .mod 13 = 5) && ((n > 3215 && n < 3269)))
	; 11 bytes
	tya
	ldy #(n-5)/13
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3218 || n = 3235 || n = 3252 || n = 3269)
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
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3220)
	; 11 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3227)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3228)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3236)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3240)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 4
	iny
	Jne *-6
	tay
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
.elseif(n = 3254 || n = 3273)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
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
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3258)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3266)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 4
	iny
	Jne *-6
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
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3298 || n = 3312)
	; 11..12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3283 || n = 3288 || n = 3297 || n = 3308)
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
.elseif(n = 3285 || n = 3301 || n = 3317)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3286 || n = 3303 || n = 3320)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3287 || n = 3289 || n = 3296)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3290 || n = 3305)
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
.elseif(n = 3293)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	delay_n 4
	iny
	Jne *-6
	tay
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
.elseif(n = 3299)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3300)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3302 || n = 3310)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
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
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3313)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3314)
	; 12 bytes
	php
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
.elseif(n = 3315)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3316)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3318)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
	tay
.elseif(n = 3319)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
	tay
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3322 || n = 3324 || n = 3325 || n = 3327 || n = 3334 || n = 3338 || n = 3339 || n = 3341 || n = 3346 || n = 3350 || n = 3352 || n = 3354 || n = 3355 || n = 3357 || n = 3359)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3323 || n = 3337)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
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
.elseif(n = 3328 || n = 3342)
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
	tya
	ldy #(n-11)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
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
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3332)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
.elseif(n = 3333)
	; 11 bytes
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
.elseif(n = 3335)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3336)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3340)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3344)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3345)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 3347)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3348)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3349)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3351)
	; 9 bytes
	php
	lda $A5
	lda #13
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3353)
	; 7 bytes
	php
	lda #13
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3356)
	; 9 bytes
	php
	lda $A5
	lda #13
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3358)
	; 10 bytes
	php
	nop
	lda $A5
	lda #13
	jsr delay_256a_16_clocks
	plp
.endif
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
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3362 || n = 3364 || n = 3366 || n = 3369 || n = 3373 || n = 3390)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363)
	; 11 bytes
	php
	pha
	lda $A5
	lda #13
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3365)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3367)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 3368)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3370)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3371 || n = 3388)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3372)
	; 11 bytes
	php
	delay_n 24
	lda #13
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3374)
	; 12 bytes
	php
	delay_n 21
	lda #13
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
	delay_n 24
	lda #13
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
.elseif(n = 3380)
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
.elseif((n > 3381 && n < 3387) || n = 3389 || n = 3391 || n = 3394 || n = 3398)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3387)
	; 11 bytes
	php
	delay_n 39
	lda #13
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3392)
	; 11 bytes
	php
	delay_n 39
	lda #13
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3396)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 7
	dey
	Jne *-6
	tay
.endif
.else
; 3399 to 3437
.if((n > 3398 && n < 3403) || n = 3404 || (n > 3413 && n < 3421) || n = 3426 || (n > 3429 && n < 3435) || n = 3436)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
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
.elseif(n = 3405 || n = 3422)
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
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
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
.elseif(n = 3408 || n = 3409 || n = 3423 || n = 3424 || n = 3437)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3410 || n = 3425)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3411)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3412)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3413 || n = 3429)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3427)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3428)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3442 || (n > 3445 && n < 3449) || n = 3450 || n = 3452 || n = 3453 || n = 3462 || n = 3464 || n = 3466 || n = 3467 || n = 3469 || n = 3471 || n = 3472 || n = 3474 || n = 3484 || (n > 3485 && n < 3490) || (n > 3493 && n < 3500) || (n > 3501 && n < 3505) || n = 3506 || (n > 3509 && n < 3513) || n = 3514)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
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
.elseif(n = 3441 || n = 3451 || n = 3458 || n = 3465 || n = 3479)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3443)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	delay_n 7
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
.elseif(n = 3457 || n = 3478)
	; 11 bytes
	delay_n (n-3)
	lda $A5
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
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	delay_n 7
	dey
	Jne *-6
	tay
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
.elseif(n = 3476)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3481)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3482)
	; 11 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3483)
	; 11 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	Jne *-6
	tay
.elseif(n = 3492)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3508)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 7
	dey
	Jne *-6
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
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if((n > 3515 && n < 3519) || n = 3522 || (n > 3526 && n < 3530) || n = 3531 || n = 3532 || n = 3534 || (n > 3535 && n < 3539) || n = 3542 || n = 3544 || n = 3546 || n = 3548 || (n > 3549 && n < 3554))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
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
.elseif(n = 3521 || n = 3526 || n = 3535 || n = 3543 || n = 3549)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3523)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $24A0'
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
.elseif(n = 3525)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3530 || n = 3545)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3540)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3554)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.endif
.else
; 3555 to 3593
.if(n = 3555)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3556)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3557 || n = 3573)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
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
.elseif(n = 3559 || n = 3562 || n = 3563 || n = 3565 || n = 3570 || n = 3591 || n = 3593)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3560 || n = 3590)
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3561 || n = 3575)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3564 || n = 3576 || n = 3578)
	; 12..13 bytes
	delay_n (n-3)
	lda $A5
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
	tya
	ldy #(n-11)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3568 || n = 3582 || n = 3584)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3569)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3571)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3572)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3574)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	cmp $C5
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
.elseif(n = 3579)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 3586)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3587)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3588)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $E0A0'
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
.if((n >= 3594) && (n < 3613))
; 3594 to 3612
.if(n = 3594 || n = 3598 || n = 3600 || n = 3606 || n = 3608 || n = 3610 || n = 3611)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3595)
	; 12 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3596)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3597)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3599)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3601)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 3602)
	; 13 bytes
	tya
	ldy #(n-11)/19
	delay_n 14
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 3603)
	; 11 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 11
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
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3607)
	; 9 bytes
	php
	lda $A5
	lda #14
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3609)
	; 7 bytes
	php
	lda #14
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3612)
	; 9 bytes
	php
	lda $A5
	lda #14
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 3613 to 3632
.if(n = 3613 || n = 3615 || n = 3618 || n = 3620 || n = 3622 || n = 3625)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3614)
	; 10 bytes
	php
	nop
	lda $A5
	lda #14
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3616)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3617)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3619)
	; 11 bytes
	php
	pha
	lda $A5
	lda #14
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3621)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3623)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 3624)
	; 12 bytes
	php
	delay_n 20
	lda #14
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3626)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
	delay_n 24
	lda #14
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3629)
	; 12 bytes
	php
	delay_n 20
	lda #14
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3630)
	; 12 bytes
	php
	delay_n 21
	lda #14
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
.endif
.else
; 3633 to 3671
.if(n = 3633)
	; 11 bytes
	php
	delay_n 24
	lda #14
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
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3636)
	; 11 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3637 || n = 3653 || n = 3669)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif((n > 3637 && n < 3643) || n = 3644 || n = 3646 || n = 3647 || n = 3649 || (n > 3653 && n < 3657) || n = 3658 || n = 3659 || n = 3661 || n = 3663 || n = 3664 || n = 3666 || n = 3670 || n = 3671)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3643 || n = 3660)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3645 || n = 3662)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3648)
	; 11 bytes
	php
	delay_n 39
	lda #14
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3651)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3652)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3657)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
	tax
.elseif(n = 3667)
	; 11 bytes
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	delay_n 7
	inx
	Jne *-6
	tax
.elseif(n = 3668)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3673 || n = 3674 || n = 3676 || n = 3678 || n = 3681 || n = 3682 || (n > 3685 && n < 3691) || n = 3692 || n = 3693 || n = 3697 || n = 3698 || (n > 3701 && n < 3710) || n = 3712 || n = 3714 || (n > 3717 && n < 3723) || n = 3724 || n = 3726 || n = 3727 || (n > 3733 && n < 3740) || (n > 3740 && n < 3745) || n = 3746 || n = 3750)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3675)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.elseif(n = 3677 || n = 3694 || n = 3711 || n = 3728 || n = 3745)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3679 || n = 3696 || n = 3713 || n = 3730)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 15 = 5) && ((n > 3679 && n < 3741)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3683)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3684)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 7
	iny
	Jne *-6
	tay
.elseif((n .mod 16 = 5) && ((n > 3684 && n < 3750)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
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
.elseif(n = 3700)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3716)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3723)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 8
	dey
	Jne *-6
	tay
.elseif(n = 3731)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3732)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 7
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
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
; 3751 to 3828
.if((n > 3750 && n < 3755) || (n > 3755 && n < 3762) || n = 3766 || n = 3768 || n = 3769 || (n > 3770 && n < 3779) || (n > 3781 && n < 3785) || (n > 3786 && n < 3795) || n = 3799 || n = 3801 || n = 3802 || n = 3804 || (n > 3805 && n < 3810))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif((n .mod 15 = 5) && ((n > 3754 && n < 3816)))
	; 11 bytes
	tya
	ldy #(n-5)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
.elseif(n = 3762 || n = 3779 || n = 3796 || n = 3813)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3763)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3764)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3765 || n = 3781 || n = 3797)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
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
.elseif(n = 3780)
	; 11 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3795)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3798 || n = 3817 || n = 3826)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3803)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 3810 || n = 3822)
	; 12..13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3811)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3812)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3814)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 6
	iny
	Jne *-7
	tay
.elseif(n = 3816)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3818)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3819)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3820)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 3821)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3823)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 3825)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 3827)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3828)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	delay_n 7
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
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3830 || n = 3847)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3831 || n = 3834 || n = 3838 || n = 3841 || n = 3846 || n = 3849 || n = 3851 || n = 3862 || n = 3864 || n = 3866 || n = 3867)
	; 8..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3832)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
.elseif(n = 3833 || n = 3850 || n = 3858)
	; 12..13 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 3835 || n = 3852)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 3836)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3837)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 3839 || n = 3857)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 3840)
	; 13 bytes
	tya
	pha
	ldy #86
	delay_n 39
	dey
	Jpl *-5
	pla
	tay
.elseif(n = 3842)
	; 13 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	sta (@ptrtemp),y
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
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 7
	iny
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
.elseif(n = 3848)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 3853)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 3854)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 3855)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
.elseif(n = 3856)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 3859)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3860)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3863)
	; 9 bytes
	php
	lda $A5
	lda #15
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
.if(n = 3868)
	; 9 bytes
	php
	lda $A5
	lda #15
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3869 || n = 3871 || n = 3874 || n = 3876 || n = 3878 || n = 3883)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870)
	; 10 bytes
	php
	nop
	lda $A5
	lda #15
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3873)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3875)
	; 11 bytes
	php
	pha
	lda $A5
	lda #15
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3877)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3879)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 3880)
	; 12 bytes
	php
	delay_n 20
	lda #15
	jsr delay_256a_11_clocks_
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
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3884)
	; 11 bytes
	php
	delay_n 24
	lda #15
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 3885)
	; 12 bytes
	php
	delay_n 20
	lda #15
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3886)
	; 12 bytes
	php
	delay_n 21
	lda #15
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3887 || n = 3888 || n = 3890)
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
	delay_n 24
	lda #15
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3891)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 3892)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3893)
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif((n > 3893 && n < 3898) || (n > 3900 && n < 3904) || n = 3905 || n = 3906)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3899)
	; 11 bytes
	php
	delay_n 39
	lda #15
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
	delay_n 39
	lda #15
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3908)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
.elseif((n .mod 16 = 5) && ((n > 3908 && n < 3974)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif((n > 3909 && n < 3915) || n = 3916 || n = 3918 || (n > 3919 && n < 3923) || (n > 3925 && n < 3932) || n = 3933 || (n > 3934 && n < 3938) || n = 3942 || n = 3943 || n = 3945 || n = 3946 || n = 3948 || n = 3950 || (n > 3951 && n < 3955) || (n > 3957 && n < 3964) || n = 3965 || n = 3967 || n = 3969 || n = 3970 || n = 3974 || n = 3975 || (n > 3976 && n < 3981) || n = 3982 || n = 3984)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3915 || n = 3932 || n = 3949 || n = 3966 || n = 3983)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3917 || n = 3934 || n = 3951 || n = 3968)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3919 || n = 3938 || n = 3976)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 3923)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3924)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3939)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3940)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3944)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts
	dey
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
.elseif(n = 3955)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3956)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 3964)
	; 11 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts
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
.elseif(n = 3972)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 7
	iny
	Jne *-6
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
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4002)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 3986 || (n > 3989 && n < 3995) || (n > 3995 && n < 4000) || n = 4001 || (n > 4005 && n < 4014) || n = 4015 || n = 4016 || n = 4018 || (n > 4021 && n < 4033) || (n > 4037 && n < 4051) || (n > 4053 && n < 4063))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 3987)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 3988)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
.elseif((n .mod 16 = 5) && ((n > 3988 && n < 4054)))
	; 11 bytes
	tya
	ldy #(n-5)/16
	delay_n 11
	dey
	Jne *-5
	tay
.elseif(n = 3995 || n = 4014 || n = 4033 || n = 4052)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4000 || n = 4017 || n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4003)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4004)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4019)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4020)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4035)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4036)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	tay
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
.if((n >= 4063) && (n < 4082))
; 4063 to 4081
.if((n > 4062 && n < 4066))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4066)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4067)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4068)
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
.elseif(n = 4070 || n = 4081)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4071)
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
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4073)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 4075)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4076)
	; 12 bytes
	tya
	pha
	ldy #42 ;hides 'rol a'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
.elseif(n = 4077)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $C2A0'
	ldy #194
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4078)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $3EA0'
	ldy #62
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4079)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4080)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 4082 to 4101
.if(n = 4082 || n = 4088 || n = 4093)
	; 12..14 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4083)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4084)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
.elseif(n = 4085)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4086 || n = 4087 || n = 4092 || n = 4094)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4089)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4090)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4091)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4095)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4096)
	; 13 bytes
	tya
	ldy #(n-11)/19
	delay_n 14
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 4097)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4098)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $C3A0'
	ldy #195
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4099)
	; 11 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4100)
	; 11 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
.elseif(n = 4101)
	; 11 bytes
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4136)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4103 || n = 4104 || n = 4106 || n = 4111 || n = 4118 || n = 4120 || n = 4122 || n = 4123 || n = 4125 || n = 4127 || n = 4130 || n = 4132 || n = 4134 || n = 4137 || n = 4138)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4105 || n = 4110)
	; 12 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4107)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4108)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4109)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
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
.elseif(n = 4113)
	; 12 bytes
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 4114)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4115)
	; 13 bytes
	tya
	ldy #(n-11)/19
	delay_n 14
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 4116)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4117)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 8
	iny
	Jne *-8
	tay
.elseif(n = 4119)
	; 9 bytes
	php
	lda $A5
	lda #16
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4121)
	; 7 bytes
	php
	lda #16
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4124)
	; 9 bytes
	php
	lda $A5
	lda #16
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4126)
	; 10 bytes
	php
	nop
	lda $A5
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
.elseif(n = 4129)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4131)
	; 11 bytes
	php
	pha
	lda $A5
	lda #16
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4133)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4135)
	; 11 bytes
	php
	pha
	delay_n (n-14)
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
	delay_n 24
	lda #16
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 12 bytes
	php
	delay_n 20
	lda #16
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4142)
	; 12 bytes
	php
	delay_n 21
	lda #16
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
	delay_n 24
	lda #16
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
.elseif((n > 4147 && n < 4153) || n = 4154 || (n > 4155 && n < 4160) || (n > 4161 && n < 4166) || (n > 4166 && n < 4170) || n = 4171 || (n > 4172 && n < 4179) || n = 4180 || n = 4181 || n = 4183 || n = 4184 || n = 4186 || n = 4188 || (n > 4189 && n < 4203) || n = 4205 || (n > 4206 && n < 4215) || (n > 4215 && n < 4219))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4153 || n = 4170 || n = 4187 || n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4155)
	; 11 bytes
	php
	delay_n 39
	lda #16
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4160)
	; 11 bytes
	php
	delay_n 39
	lda #16
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
.elseif(n = 4172 || n = 4189 || n = 4206)
	; 11 bytes
	delay_n (n-2)
	nop
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
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
.elseif(n = 4220 || n = 4222 || (n > 4223 && n < 4238) || n = 4239 || n = 4241 || (n > 4242 && n < 4255) || n = 4256 || (n > 4257 && n < 4261) || (n > 4261 && n < 4266) || (n > 4266 && n < 4272) || n = 4273 || (n > 4274 && n < 4280) || (n > 4280 && n < 4289) || n = 4290 || (n > 4291 && n < 4297))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4221 || n = 4238 || n = 4255 || n = 4272 || n = 4289)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4223 || n = 4242 || n = 4261 || n = 4280)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4240 || n = 4257 || n = 4274 || n = 4291)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 4266)
	; 11 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
	tax
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297 || n = 4298 || (n > 4299 && n < 4306) || n = 4307 || (n > 4308 && n < 4318) || (n > 4318 && n < 4322))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4299 || n = 4318)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4306 || n = 4323)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4308 || n = 4325 || n = 4327 || n = 4332)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4322)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4324)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4326 || n = 4334)
	; 12..14 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4328)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4329)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4330)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4331)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4333)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
.elseif(n = 4335)
	; 13 bytes
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	rol $26
	ror $26
	inx
	Jne *-8
	tax
.endif
.else
; 4336 to 4375
.if(n = 4336)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	rol $26
	ror $26
	iny
	Jne *-8
	tay
.elseif(n = 4337 || n = 4356)
	; 11 bytes
	tya
	ldy #(n-5)/19
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4338)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 4339 || n = 4342 || n = 4344 || n = 4348 || n = 4353 || n = 4354 || n = 4358 || n = 4359 || n = 4366 || n = 4367 || n = 4370 || n = 4374)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4341)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4343 || n = 4361)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4345)
	; 12 bytes
	php
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
.elseif(n = 4346)
	; 12 bytes
	tya
	ldy #(n-11)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4347 || n = 4364)
	; 12 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4349)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4350)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4351)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4352 || n = 4373)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4355)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 8
	dey
	Jne *-8
	tay
.elseif(n = 4357)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4360)
	; 12 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
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
.elseif(n = 4363)
	; 12 bytes
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
.elseif(n = 4365)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 4368)
	; 13 bytes
	tya
	pha
	ldy #98
	delay_n 39
	dey
	Jpl *-5
	pla
	tay
.elseif(n = 4369)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4371)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4372)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4375)
	; 9 bytes
	php
	lda $A5
	lda #17
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4378 || n = 4379 || n = 4381 || n = 4383 || n = 4386 || n = 4388 || n = 4390 || n = 4393)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 7 bytes
	php
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4380)
	; 9 bytes
	php
	lda $A5
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4382)
	; 10 bytes
	php
	nop
	lda $A5
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4384)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4385)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4387)
	; 11 bytes
	php
	pha
	lda $A5
	lda #17
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4389)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4391)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 4392)
	; 12 bytes
	php
	delay_n 20
	lda #17
	jsr delay_256a_11_clocks_
	plp
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
	delay_n 24
	lda #17
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4397)
	; 12 bytes
	php
	delay_n 20
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4398)
	; 12 bytes
	php
	delay_n 21
	lda #17
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
	delay_n 24
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif((n > 4403 && n < 4411) || n = 4412 || n = 4414 || n = 4415 || (n > 4416 && n < 4432) || (n > 4432 && n < 4449) || n = 4450 || (n > 4451 && n < 4455) || (n > 4455 && n < 4467) || n = 4468 || n = 4469 || (n > 4470 && n < 4476) || (n > 4476 && n < 4489) || (n > 4489 && n < 4497) || (n > 4497 && n < 4508) || (n > 4508 && n < 4527) || (n > 4527 && n < 4532))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4411)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 4416)
	; 11 bytes
	php
	delay_n 39
	lda #17
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4455)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4467)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 11
	dey
	Jne *-6
	tay
.elseif(n = 4476)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4497)
	; 11 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4539) || (n > 4539 && n < 4546) || (n > 4546 && n < 4565) || (n > 4565 && n < 4578))
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4539)
	; 11 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 14
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
.elseif(n = 4578 || n = 4579)
	; 13 bytes
	php
	nop
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_16_clocks
	plp
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
	cmp #<$CD ;hides 'cmp $DAA0'
	ldy #218
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4582)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4583 || n = 4604)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4585)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 4586 || n = 4588 || n = 4596 || n = 4597 || n = 4599 || n = 4605 || n = 4607)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4587 || n = 4598 || n = 4606)
	; 13..14 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 4589)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 4590 || n = 4609)
	; 13 bytes
	tya
	ldy #(n-11)/19
	delay_n 14
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
.elseif(n = 4591 || n = 4600)
	; 13..14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4592)
	; 14 bytes
	tya
	ldy #139
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 4593)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4594)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
.elseif(n = 4595)
	; 12 bytes
	tya
	ldy #(n-5)/18
	delay_n 13
	dey
	Jne *-6
	tay
.elseif(n = 4601)
	; 13 bytes
	tya
	ldy #248 ;hides 'sed'
	pha
	delay_n 24
	pla
	dey
	Jmi *-8
	tay
.elseif(n = 4602)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4608)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4618 || n = 4619 || n = 4629)
	; 13..14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4611)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
.elseif(n = 4612)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
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
.elseif(n = 4614 || n = 4615 || n = 4617 || n = 4624 || n = 4626 || n = 4630 || n = 4632 || n = 4634 || n = 4635 || n = 4637 || n = 4639 || n = 4642 || n = 4644 || n = 4646)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4616)
	; 13 bytes
	tya
	ldy #159
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 4620)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	rol $26
	ror $26
	dey
	Jne *-8
	tay
.elseif(n = 4621)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	rol $26
	ror $26
	dey
	Jne *-8
	tay
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
	delay_n 11
	dex
	dex
	Jne *-7
	tax
.elseif(n = 4625)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
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
.elseif(n = 4631)
	; 9 bytes
	php
	lda $A5
	lda #18
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4633)
	; 7 bytes
	php
	lda #18
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4636)
	; 9 bytes
	php
	lda $A5
	lda #18
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4638)
	; 10 bytes
	php
	nop
	lda $A5
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
.elseif(n = 4641)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4643)
	; 11 bytes
	php
	pha
	lda $A5
	lda #18
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4645)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4647)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 4648)
	; 12 bytes
	php
	delay_n 20
	lda #18
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 4649 to 4687
.if(n = 4649)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #18
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4653)
	; 12 bytes
	php
	delay_n 20
	lda #18
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4654)
	; 12 bytes
	php
	delay_n 21
	lda #18
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
	delay_n 24
	lda #18
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
.elseif((n > 4660 && n < 4667) || (n > 4667 && n < 4672) || (n > 4672 && n < 4679) || (n > 4679 && n < 4688))
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
	delay_n 39
	lda #18
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4672)
	; 11 bytes
	php
	delay_n 39
	lda #18
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if((n > 4687 && n < 4698) || (n > 4698 && n < 4715) || n = 4716 || (n > 4717 && n < 4736) || (n > 4736 && n < 4755) || (n > 4755 && n < 4774) || (n > 4774 && n < 4793) || (n > 4793 && n < 4812) || (n > 4812 && n < 4831) || n = 4832 || n = 4833)
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
.elseif(n = 4715)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4834)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4835)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4836)
	; 13 bytes
	php
	sec
	lda #(n-10)/19
	delay_n 14
	sbc #1
	Jne *-6
	plp
.elseif(n = 4837)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4838 || n = 4842)
	; 13..14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4839 || n = 4840)
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
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 4843)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 11
	iny
	Jne *-8
	tay
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if(n = 4844)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 11
	iny
	Jne *-8
	tay
.elseif(n = 4845)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
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
.elseif(n = 4847 || n = 4852 || n = 4859)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4848)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	rol $26
	ror $26
	iny
	Jne *-8
	tay
.elseif(n = 4849)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	rol $26
	ror $26
	iny
	Jne *-8
	tay
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
	delay_n 16
	iny
	Jne *-7
	tay
	plp
.elseif(n = 4853)
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
.elseif(n = 4855)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4856)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4857 || n = 4861)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 4858)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 4860)
	; 13 bytes
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	sta (@ptrtemp,x)
	tax
.elseif(n = 4862)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	pla
	tay
.endif
.else
; 4863 to 4882
.if(n = 4863)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 11
	dey
	Jne *-8
	tay
.elseif(n = 4864)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 11
	dey
	Jne *-8
	tay
.elseif(n = 4865)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 4866)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	delay_n 13
	dey
	Jne *-9
	tay
.elseif(n = 4867)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 4868)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	rol $26
	ror $26
	dey
	Jne *-8
	tay
.elseif(n = 4869)
	; 11 bytes
	tya
	ldy #0
	delay_n 14
	dey
	Jne *-5
	tay
.elseif(n = 4870 || n = 4871 || n = 4873)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4872 || n = 4878)
	; 13 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 4876)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 4877)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 4879)
	; 12 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 14
	dey
	Jne *-7
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
.elseif(n = 4881)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 4882)
	; 13 bytes
	pha
	delay_n (n-7)
	pla
.endif
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 11
	iny
	Jne *-8
	tay
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
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 4886 || n = 4888 || n = 4890 || n = 4891 || n = 4893 || n = 4895 || n = 4898 || n = 4900 || n = 4902 || n = 4905)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4887)
	; 9 bytes
	php
	lda $A5
	lda #19
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4889)
	; 7 bytes
	php
	lda #19
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4892)
	; 9 bytes
	php
	lda $A5
	lda #19
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4894)
	; 10 bytes
	php
	nop
	lda $A5
	lda #19
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4896)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4897)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4899)
	; 11 bytes
	php
	pha
	lda $A5
	lda #19
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4901)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4903)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 4904)
	; 12 bytes
	php
	delay_n 20
	lda #19
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4906)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #19
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4909)
	; 12 bytes
	php
	delay_n 20
	lda #19
	jsr delay_256a_16_clocks
	plp
.elseif(n = 4910)
	; 12 bytes
	php
	delay_n 21
	lda #19
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
	delay_n 24
	lda #19
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
	delay_n 39
	lda #19
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 4928)
	; 11 bytes
	php
	delay_n 39
	lda #19
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
.elseif(n = 5092 || n = 5094 || n = 5096)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5093 || n = 5117)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5095 || n = 5110 || n = 5112)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5097)
	; 14 bytes
	tya
	ldy #76
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5098)
	; 14 bytes
	tya
	pha
	ldy #106 ;hides 'ror a'
	delay_n 41
	dey
	Jne *-7
	pla
	tay
.elseif(n = 5099)
	; 14 bytes
	tya
	ldy #(n-11)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
.elseif(n = 5100)
	; 14 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 18
	dey
	Jne *-9
	tay
.elseif(n = 5101)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 5102)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	delay_n 55
	plp
	Jmi *-8
	pla
	tax
.elseif(n = 5103)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 11
	iny
	Jne *-8
	tay
.elseif(n = 5104)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 11
	iny
	Jne *-8
	tay
.elseif(n = 5105)
	; 12 bytes
	tya
	ldy #(n-5)/20
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 5106)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5107)
	; 12 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 15
	iny
	Jne *-7
	tay
.elseif(n = 5108)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 5109)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5111)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5113)
	; 14 bytes
	php
	sec
	lda #(n-10)/21
	delay_n 16
	sbc #1
	Jne *-7
	plp
.elseif(n = 5114)
	; 13 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 5115)
	; 13 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 15
	dey
	Jne *-8
	tay
.elseif(n = 5116)
	; 13 bytes
	tya
	pha
	ldy #176
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5119 || n = 5126 || n = 5130)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5120)
	; 14 bytes
	tya
	ldy #155
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5121)
	; 14 bytes
	tya
	pha
	ldy #131
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5122)
	; 14 bytes
	tya
	ldy #119
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5123)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 11
	dey
	Jne *-8
	tay
.elseif(n = 5124)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 11
	dey
	Jne *-8
	tay
.elseif(n = 5125)
	; 12 bytes
	tya
	ldy #0
	delay_n 15
	dey
	Jne *-6
	tay
.elseif(n = 5127)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5128)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5129)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 5131)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5132)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5133)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5134)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5135)
	; 14 bytes
	tya
	ldy #135
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5136)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.endif
.else
; 5137 to 5156
.if(n = 5137)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5138)
	; 13 bytes
	tya
	ldy #177
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5139)
	; 13 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 15
	dey
	Jne *-8
	tay
.elseif(n = 5140)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5141)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5142 || n = 5144 || n = 5146 || n = 5147 || n = 5149 || n = 5151 || n = 5154 || n = 5156)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 5143)
	; 9 bytes
	php
	lda $A5
	lda #20
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5145)
	; 7 bytes
	php
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5148)
	; 9 bytes
	php
	lda $A5
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5150)
	; 10 bytes
	php
	nop
	lda $A5
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
.elseif(n = 5153)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5155)
	; 11 bytes
	php
	pha
	lda $A5
	lda #20
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5158 || n = 5161)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5159)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 5160)
	; 12 bytes
	php
	delay_n 20
	lda #20
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5162)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #20
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5165)
	; 12 bytes
	php
	delay_n 20
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5166)
	; 12 bytes
	php
	delay_n 21
	lda #20
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
	delay_n 24
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif((n > 5171 && n < 5179) || (n > 5179 && n < 5184) || (n > 5184 && n < 5211) || (n > 5211 && n < 5313))
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
	delay_n 39
	lda #20
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5184)
	; 11 bytes
	php
	delay_n 39
	lda #20
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5211)
	; 11 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 14
	dey
	Jne *-6
	tay
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
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
.elseif(n = 5348)
	; 13 bytes
	tya
	pha
	ldy #184
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 5349)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5350)
	; 13 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 5351)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if(n = 5352)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5353)
	; 13 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 16
	dey
	Jne *-8
	tay
.elseif(n = 5354)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	delay_n 16
	iny
	Jne *-9
	tay
.elseif(n = 5355)
	; 14 bytes
	tya
	ldy #(n-5)/25
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 5356)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 15
	iny
	Jne *-9
	tay
.elseif(n = 5357)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5358)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5359)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
.elseif(n = 5360)
	; 12 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-6
	tay
.elseif(n = 5361)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 5362 || n = 5366)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5363)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5364)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5365)
	; 14 bytes
	php
	sec
	lda #(n-10)/21
	delay_n 16
	sbc #1
	Jne *-7
	plp
.elseif(n = 5367)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5368)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5369)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 5370)
	; 13 bytes
	tya
	ldy #185
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.else
; 5371 to 5390
.if(n = 5371 || n = 5374 || n = 5375 || n = 5382 || n = 5383 || n = 5389)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5372)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 13
	iny
	Jne *-9
	tay
.elseif(n = 5373)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5376)
	; 14 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 18
	dey
	dey
	Jne *-9
	tay
.elseif(n = 5377)
	; 13 bytes
	tya
	pha
	ldy #185
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 5378)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	delay_n 16
	dey
	Jne *-9
	tay
.elseif(n = 5379)
	; 12 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5380)
	; 12 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	dey
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
.elseif(n = 5384)
	; 14 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5385)
	; 12 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	tay
.elseif(n = 5386)
	; 13 bytes
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 14
	inx
	Jne *-8
	tax
.elseif(n = 5387)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5388)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5390)
	; 14 bytes
	php
	sec
	lda #234 ;hides 'nop'
	delay_n 16
	sbc #1
	Jne *-8
	plp
.endif
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
; 5391 to 5429
.if(n = 5391)
	; 14 bytes
	tya
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
.elseif(n = 5392)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5393)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 13
	iny
	Jne *-9
	tay
.elseif(n = 5394)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5395)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5396)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5397 || n = 5398 || n = 5400 || n = 5402 || n = 5403 || n = 5405 || n = 5407 || n = 5410 || n = 5412 || n = 5414 || n = 5417)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5399)
	; 9 bytes
	php
	lda $A5
	lda #21
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5401)
	; 7 bytes
	php
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5404)
	; 9 bytes
	php
	lda $A5
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5406)
	; 10 bytes
	php
	nop
	lda $A5
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5408)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5409)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5411)
	; 11 bytes
	php
	pha
	lda $A5
	lda #21
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5413)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5415)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 5416)
	; 12 bytes
	php
	delay_n 20
	lda #21
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5418)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #21
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5421)
	; 12 bytes
	php
	delay_n 20
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5422)
	; 12 bytes
	php
	delay_n 21
	lda #21
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
	delay_n 24
	lda #21
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5428 || n = 5429)
	; 12 bytes
	php
	lda #<(n-52)
	jsr delay_a_25_clocks
	lda #>(n-52)
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 5430 to 5468
.if((n > 5429 && n < 5435) || (n > 5435 && n < 5440) || (n > 5440 && n < 5469))
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
	delay_n 39
	lda #21
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5440)
	; 11 bytes
	php
	delay_n 39
	lda #21
	jsr delay_256a_16_clocks
	plp
.endif
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
.elseif((n > 5603 && n < 5609))
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
	; 13 bytes
	tya
	pha
	ldy #193
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 5610)
	; 14 bytes
	tya
	pha
	ldy #56 ;hides 'sec'
	delay_n 21
	iny
	Jne *-7
	pla
	tay
.elseif(n = 5611 || n = 5618 || n = 5623 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5612)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5613)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 13
	iny
	Jne *-9
	tay
.elseif(n = 5614)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 13
	iny
	Jne *-9
	tay
.elseif(n = 5615)
	; 13 bytes
	tya
	ldy #(n-5)/22
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5616)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5617)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
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
.elseif(n = 5620)
	; 14 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	iny ;first loop only
	delay_n 24
	iny
	Jne *-7
	pla
	tay
.elseif(n = 5621)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5622)
	; 14 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 5624)
	; 14 bytes
	tya
	pha
	ldy #(n-58)/46
	delay_n 41
	dey
	Jpl *-6
	pla
	tay
.endif
.endif
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
.if((n >= 5626) && (n < 5665))
.if((n >= 5626) && (n < 5645))
; 5626 to 5644
.if(n = 5626)
	; 14 bytes
	tya
	pha
	ldy #234 ;hides 'nop'
	delay_n 39
	dey
	dey
	Jne *-7
	pla
	tay
.elseif(n = 5627)
	; 14 bytes
	tya
	ldy #(n-11)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
.elseif(n = 5628)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5629)
	; 14 bytes
	tya
	ldy #148
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5630)
	; 14 bytes
	tya
	ldy #(n-5)/25
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 5631)
	; 13 bytes
	tya
	ldy #194
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5632)
	; 14 bytes
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-9
	tay
.elseif(n = 5633)
	; 14 bytes
	tya
	ldy #201
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 5634)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 69
	Jmi *-8
	pla
	tax
.elseif(n = 5635)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 13
	dey
	Jne *-9
	tay
.elseif(n = 5636)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 13
	dey
	Jne *-9
	tay
.elseif(n = 5637)
	; 13 bytes
	tya
	ldy #0
	delay_n 17
	dey
	Jne *-7
	tay
.elseif(n = 5638)
	; 13 bytes
	tya
	pha
	ldy #194
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 5639)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5640)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5641 || n = 5642)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5643)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 15
	iny
	Jne *-9
	tay
.elseif(n = 5644)
	; 13 bytes
	tya
	pha
	ldy #127
	delay_n 39
	dey
	Jpl *-5
	pla
	tay
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5646 || n = 5649 || n = 5654 || n = 5656 || n = 5658 || n = 5659 || n = 5661 || n = 5663)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5647)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 5648)
	; 14 bytes
	tya
	ldy #171
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5650)
	; 14 bytes
	php
	sec
	lda #235
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	plp
.elseif(n = 5651)
	; 14 bytes
	tya
	ldy #(n-11)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
.elseif(n = 5652)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5653)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 930
	pla
	Jcc *-10
.elseif(n = 5655)
	; 9 bytes
	php
	lda $A5
	lda #22
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5657)
	; 7 bytes
	php
	lda #22
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5660)
	; 9 bytes
	php
	lda $A5
	lda #22
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5662)
	; 10 bytes
	php
	nop
	lda $A5
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
.endif
.else
; 5665 to 5703
.if(n = 5665)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5666 || n = 5668 || n = 5670 || n = 5673)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667)
	; 11 bytes
	php
	pha
	lda $A5
	lda #22
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5669)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5671)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 5672)
	; 12 bytes
	php
	delay_n 20
	lda #22
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5674)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #22
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5677)
	; 12 bytes
	php
	delay_n 20
	lda #22
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5678)
	; 12 bytes
	php
	delay_n 21
	lda #22
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
	delay_n 24
	lda #22
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
	delay_n 39
	lda #22
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5696)
	; 11 bytes
	php
	delay_n 39
	lda #22
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
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860 || n = 5862)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5861)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5863)
	; 13 bytes
	tya
	ldy #202
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 5864)
	; 13 bytes
	tya
	pha
	ldy #133
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 5865 || n = 5866 || n = 5872 || n = 5874 || n = 5875)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5867)
	; 14 bytes
	tya
	ldy #(n-11)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
.elseif(n = 5868)
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5869)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 14
	iny
	Jne *-8
	tay
.elseif(n = 5870)
	; 13 bytes
	tya
	ldy #(n-5)/23
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5871)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5873)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 5876 || n = 5877)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5878)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 16
	iny
	Jne *-9
	tay
.endif
.else
; 5879 to 5898
.if(n = 5879)
	; 14 bytes
	tya
	ldy #178
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5880)
	; 14 bytes
	tya
	ldy #(n-5)/25
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 5881)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 5882 || n = 5883 || n = 5887 || n = 5889 || n = 5895)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5884)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 15
	iny
	Jne *-9
	tay
.elseif(n = 5885)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5886)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	sbc #1
	Jne *-8
	plp
.elseif(n = 5888)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
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
	; 13 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 14
	dey
	Jne *-8
	tay
.elseif(n = 5892)
	; 13 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 14
	dey
	Jne *-8
	tay
.elseif(n = 5893)
	; 13 bytes
	tya
	ldy #0
	delay_n 18
	dey
	Jne *-7
	tay
.elseif(n = 5894)
	; 14 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	delay_n 24
	pla
	plp
	Jne *-9
	tay
.elseif(n = 5896)
	; 14 bytes
	tya
	ldy #137
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 5897)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5898)
	; 14 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #195
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
.endif
.endif
.else
.if((n >= 5899) && (n < 5918))
; 5899 to 5917
.if(n = 5899)
	; 13 bytes
	tya
	pha
	ldy #203
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 5900)
	; 14 bytes
	tya
	pha
	ldy #(n-58)/46
	delay_n 41
	dey
	Jpl *-6
	pla
	tay
.elseif(n = 5901)
	; 14 bytes
	tya
	ldy #87
	pha
	delay_n 55
	pla
	dey
	Jpl *-8
	tay
.elseif(n = 5902)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 5903)
	; 14 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $EACD ;hides 'nop'
	delay_n 14
	dey
	Jne *-9
	tay
.elseif(n = 5904)
	; 14 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	delay_n 16
	dey
	Jne *-9
	tay
.elseif(n = 5905)
	; 14 bytes
	tya
	ldy #(n-5)/25
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 5906)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 5907)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
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
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 5910 || n = 5912 || n = 5914 || n = 5915 || n = 5917)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5911)
	; 9 bytes
	php
	lda $A5
	lda #23
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5913)
	; 7 bytes
	php
	lda #23
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5916)
	; 9 bytes
	php
	lda $A5
	lda #23
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 5918 to 5937
.if(n = 5918)
	; 10 bytes
	php
	nop
	lda $A5
	lda #23
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5919 || n = 5922 || n = 5924 || n = 5926 || n = 5929)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5921)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5923)
	; 11 bytes
	php
	pha
	lda $A5
	lda #23
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5925)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5927)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 5928)
	; 12 bytes
	php
	delay_n 20
	lda #23
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5930)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #23
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5933)
	; 12 bytes
	php
	delay_n 20
	lda #23
	jsr delay_256a_16_clocks
	plp
.elseif(n = 5934)
	; 12 bytes
	php
	delay_n 21
	lda #23
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
	delay_n 24
	lda #23
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
	delay_n 39
	lda #23
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 5952)
	; 11 bytes
	php
	delay_n 39
	lda #23
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
.elseif((n > 6115 && n < 6121))
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6122)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6123)
	; 14 bytes
	tya
	ldy #161
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6124)
	; 13 bytes
	tya
	ldy #211
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6125)
	; 12 bytes
	tya
	ldy #(n-5)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
.elseif(n = 6126)
	; 13 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-5
	pla
	tax
.elseif(n = 6127)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6128)
	; 13 bytes
	tya
	pha
	ldy #139
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6129)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 16
	iny
	Jne *-9
	tay
.elseif(n = 6130)
	; 14 bytes
	tya
	ldy #(n-5)/25
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 6131)
	; 13 bytes
	tya
	pha
	ldy #211
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6132)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133 || n = 6139 || n = 6143 || n = 6145 || n = 6151)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6134)
	; 14 bytes
	tya
	ldy #(n-5)/27
	delay_n 10
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 6135)
	; 14 bytes
	tya
	pha
	ldy #157
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6136)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-9
	tay
.elseif(n = 6137)
	; 14 bytes
	tya
	ldy #219
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6138)
	; 14 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #203
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
.elseif(n = 6140)
	; 15 bytes
	txa
	pha
	clc
	clc
	lda #$0A ;hides 'asl a'
	ldx #203
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
.elseif(n = 6141)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 6142)
	; 15 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-10
	tay
.elseif(n = 6144)
	; 14 bytes
	tya
	ldy #216 ;hides 'cld'
	pha
	delay_n 55
	pla
	dey
	Jmi *-9
	tay
.elseif(n = 6146)
	; 15 bytes
	php
	sec
	lda #(n-10)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	sbc #1
	Jne *-8
	plp
.elseif(n = 6147)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 15
	dey
	Jne *-9
	tay
.elseif(n = 6148)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
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
.elseif(n = 6150)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.endif
.else
; 6152 to 6171
.if(n = 6152)
	; 14 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 6153)
	; 13 bytes
	tya
	ldy #212
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6154)
	; 14 bytes
	tya
	ldy #143
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6155)
	; 14 bytes
	tya
	ldy #(n-5)/25
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 6156)
	; 14 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6157 || n = 6163 || n = 6166 || n = 6168 || n = 6170 || n = 6171)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6158)
	; 15 bytes
	tya
	pha
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 20
	dey
	Jne *-8
	pla
	tay
.elseif(n = 6159)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 6160)
	; 13 bytes
	tya
	pha
	ldy #212
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6161)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 6162)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	pla
	tay
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
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6167)
	; 9 bytes
	php
	lda $A5
	lda #24
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6169)
	; 7 bytes
	php
	lda #24
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 9 bytes
	php
	lda $A5
	lda #24
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6173 || n = 6175 || n = 6178 || n = 6180 || n = 6182 || n = 6185)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174)
	; 10 bytes
	php
	nop
	lda $A5
	lda #24
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6176)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6177)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6179)
	; 11 bytes
	php
	pha
	lda $A5
	lda #24
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6181)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6183)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 6184)
	; 12 bytes
	php
	delay_n 20
	lda #24
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6186)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #24
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6189)
	; 12 bytes
	php
	delay_n 20
	lda #24
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6190)
	; 12 bytes
	php
	delay_n 21
	lda #24
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
	delay_n 24
	lda #24
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
	delay_n 39
	lda #24
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6208)
	; 11 bytes
	php
	delay_n 39
	lda #24
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
.elseif((n > 6371 && n < 6375) || n = 6376)
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
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 6377)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 6378)
	; 14 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #211
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
.elseif(n = 6379 || n = 6386)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6380)
	; 13 bytes
	tya
	ldy #124
	delay_n 46
	dey
	Jpl *-7
	tay
.elseif(n = 6381)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6382)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6383)
	; 14 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 46
	iny
	iny
	Jne *-9
	tay
.elseif(n = 6384)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6385)
	; 13 bytes
	tya
	ldy #220
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6388)
	; 14 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 14
	dey
	Jne *-9
	tay
.elseif(n = 6389)
	; 14 bytes
	tya
	ldy #228
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6390)
	; 15 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #151
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
.elseif(n = 6391)
	; 14 bytes
	tya
	ldy #(n-5)/31
	nop
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 6392)
	; 13 bytes
	tya
	pha
	ldy #220
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6393)
	; 14 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 6394 || n = 6400 || n = 6403)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6395)
	; 15 bytes
	tya
	ldy #213
	delay_n 4
	.repeat 3
	 php
	 plp
	.endrepeat
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
	; 15 bytes
	tya
	ldy #188
	pha
	delay_n 22
	pla
	dey
	Jne *-9
	tay
.elseif(n = 6398)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6399)
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #3
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 6401)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 6402)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 6404)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 16
	dey
	Jne *-9
	tay
.elseif(n = 6405)
	; 14 bytes
	tya
	ldy #0
	nop
	delay_n 18
	dey
	Jne *-8
	tay
.elseif(n = 6406)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
.if((n >= 6407) && (n < 6426))
; 6407 to 6425
.if(n = 6407)
	; 14 bytes
	tya
	ldy #194
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6408)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6409)
	; 15 bytes
	lda #$0A ;hides 'asl a'
	php
	pha
	sec
	lda #252
	sbc #1
	Jne *-2
	pla
	plp
	Jpl *-12
.elseif(n = 6410)
	; 15 bytes
	txa
	pha
	ldx #232 ;hides 'inx'
	delay_n 251
	Jne *-8
	pla
	tax
.elseif(n = 6411)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/27
	delay_n 22
	dey
	Jne *-7
	pla
	tay
.elseif(n = 6412)
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 20
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6413)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 6414)
	; 13 bytes
	tya
	ldy #221
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6415 || n = 6419 || n = 6422 || n = 6424)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6416)
	; 13 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 46
	dey
	Jmi *-8
	tay
.elseif(n = 6417)
	; 14 bytes
	tya
	ldy #229
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6418)
	; 14 bytes
	tya
	ldy #121
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6420)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6421)
	; 13 bytes
	tya
	pha
	ldy #221
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6423)
	; 9 bytes
	php
	lda $A5
	lda #25
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6425)
	; 7 bytes
	php
	lda #25
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 6426 to 6445
.if(n = 6426 || n = 6427 || n = 6429 || n = 6431 || n = 6434 || n = 6436 || n = 6438 || n = 6441)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6428)
	; 9 bytes
	php
	lda $A5
	lda #25
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6430)
	; 10 bytes
	php
	nop
	lda $A5
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
.elseif(n = 6433)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6435)
	; 11 bytes
	php
	pha
	lda $A5
	lda #25
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6437)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6439)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 6440)
	; 12 bytes
	php
	delay_n 20
	lda #25
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6442)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #25
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6445)
	; 12 bytes
	php
	delay_n 20
	lda #25
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 6446 to 6484
.if(n = 6446)
	; 12 bytes
	php
	delay_n 21
	lda #25
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
	delay_n 24
	lda #25
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
	delay_n 39
	lda #25
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6464)
	; 11 bytes
	php
	delay_n 39
	lda #25
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
.elseif(n = 6628 || (n > 6629 && n < 6633))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6629)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 6633)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6634)
	; 14 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	delay_n 41
	dey
	Jne *-7
	pla
	tay
.elseif(n = 6635)
	; 13 bytes
	tya
	ldy #(n-5)/26
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
.elseif(n = 6636)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6637)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6638)
	; 14 bytes
	tya
	ldy #98
	pha
	delay_n 55
	pla
	dey
	Jpl *-8
	tay
.elseif(n = 6639)
	; 14 bytes
	tya
	ldy #(n-5)/31
	nop
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 6640)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-9
	tay
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
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6642)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/26
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6643)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1095
	pla
	Jcc *-10
.elseif(n = 6644)
	; 13 bytes
	tya
	pha
	ldy #42 ;hides 'rol a'
	delay_n 24
	iny
	Jne *-6
	pla
	tay
.elseif(n = 6645)
	; 15 bytes
	tya
	ldy #79
	pha
	delay_n 71
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 6646)
	; 13 bytes
	tya
	ldy #229
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6647)
	; 14 bytes
	tya
	ldy #(n-5)/27
	delay_n 10
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 6648)
	; 14 bytes
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
.elseif(n = 6649)
	; 15 bytes
	clc
	lda #$2A ;hides 'rol a'
	pha
	delay_n 1650
	pla
	Jcc *-11
.elseif(n = 6650)
	; 15 bytes
	txa
	pha
	clc
	clc
	lda #$0A ;hides 'asl a'
	ldx #220
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
.elseif(n = 6651)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6652)
	; 14 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 20
	iny
	Jne *-7
	pla
	tay
.elseif(n = 6653)
	; 13 bytes
	tya
	pha
	ldy #229
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6654)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/27
	delay_n 22
	dey
	Jne *-7
	pla
	tay
.elseif(n = 6655)
	; 14 bytes
	tya
	ldy #175
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6656)
	; 13 bytes
	tya
	pha
	ldy #151
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6657)
	; 15 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6658)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6659)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6661)
	; 13 bytes
	tya
	ldy #0
	.repeat 3
	 php
	 plp
	.endrepeat
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
.elseif(n = 6663 || n = 6667 || n = 6678)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6664)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 6665)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 6666)
	; 15 bytes
	php
	sec
	lda #0
	.repeat 3
	 php
	 plp
	.endrepeat
	sbc #1
	Jne *-8
	plp
.elseif(n = 6668)
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 21
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6669)
	; 14 bytes
	tya
	ldy #238
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6670)
	; 14 bytes
	tya
	ldy #155
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6671)
	; 14 bytes
	tya
	ldy #202
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6672)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6673)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1100
	pla
	Jcc *-10
.elseif(n = 6674)
	; 14 bytes
	tya
	ldy #(n-5)/27
	delay_n 10
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 6675)
	; 13 bytes
	tya
	ldy #230
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6676)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6677)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6679)
	; 9 bytes
	php
	lda $A5
	lda #26
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6682 || n = 6683 || n = 6685 || n = 6687 || n = 6690 || n = 6692 || n = 6694 || n = 6697)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6681)
	; 7 bytes
	php
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6684)
	; 9 bytes
	php
	lda $A5
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6686)
	; 10 bytes
	php
	nop
	lda $A5
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6689)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6691)
	; 11 bytes
	php
	pha
	lda $A5
	lda #26
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6693)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6695)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 6696)
	; 12 bytes
	php
	delay_n 20
	lda #26
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6698)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #26
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6701)
	; 12 bytes
	php
	delay_n 20
	lda #26
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6702)
	; 12 bytes
	php
	delay_n 21
	lda #26
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
	delay_n 24
	lda #26
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
	delay_n 39
	lda #26
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
	delay_n 39
	lda #26
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
.elseif(n = 6884 || (n > 6885 && n < 6889))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6885)
	; 13 bytes
	tya
	pha
	ldy #237
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6889)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6890)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 6891)
	; 14 bytes
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-9
	tay
.elseif(n = 6892)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6893)
	; 14 bytes
	tya
	ldy #246
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6894)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.endif
.else
; 6895 to 6914
.if(n = 6895)
	; 14 bytes
	tya
	ldy #130
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6896)
	; 15 bytes
	txa
	pha
	clc
	lda #$2A ;hides 'rol a'
	ldx #245
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
.elseif(n = 6897)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6898)
	; 14 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 21
	iny
	Jne *-7
	pla
	tay
.elseif(n = 6899)
	; 15 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	pha
	delay_n 20
	pla
	dey
	Jne *-10
	tay
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
	; 15 bytes
	clc
	lda #$2A ;hides 'rol a'
	pha
	delay_n 1713
	pla
	Jcc *-11
.elseif(n = 6902)
	; 14 bytes
	tya
	ldy #209
	pha
	delay_n 21
	pla
	dey
	Jne *-8
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
.elseif(n = 6904 || n = 6909)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6905)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6906)
	; 14 bytes
	tya
	ldy #102
	pha
	delay_n 55
	pla
	dey
	Jpl *-8
	tay
.elseif(n = 6907)
	; 13 bytes
	tya
	ldy #238
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 6908)
	; 15 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	delay_n 288
	pla
	Jcs *-10
	plp
.elseif(n = 6910)
	; 13 bytes
	tya
	pha
	ldy #106 ;hides 'ror a'
	delay_n 39
	iny
	Jne *-6
	pla
	tay
.elseif(n = 6911)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	delay_n 845
	pla
	plp
	Jne *-12
.elseif(n = 6912)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6913)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1140
	pla
	Jcc *-10
.elseif(n = 6914)
	; 13 bytes
	tya
	pha
	ldy #238
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.endif
.endif
.else
; 6915 to 6953
.if(n = 6915)
	; 14 bytes
	tya
	pha
	ldy #177
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6916 || n = 6919 || n = 6922 || n = 6923 || n = 6930 || n = 6934 || n = 6936 || n = 6938 || n = 6939 || n = 6941 || n = 6943 || n = 6946 || n = 6948 || n = 6950 || n = 6953)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6917)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 6918)
	; 14 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #229
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
.elseif(n = 6920)
	; 13 bytes
	tya
	pha
	ldy #157
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 6921)
	; 14 bytes
	tya
	ldy #247
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 6924)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6925)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 6926)
	; 14 bytes
	tya
	pha
	ldy #74 ;hides 'lsr a'
	delay_n 31
	iny
	Jne *-7
	pla
	tay
.elseif(n = 6927 || n = 6931)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 6928)
	; 14 bytes
	tya
	ldy #161
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 6929)
	; 15 bytes
	clc
	lda #$2A ;hides 'rol a'
	pha
	delay_n 1720
	pla
	Jcc *-11
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
	.repeat 3
	 php
	 plp
	.endrepeat
	dex
	dex
	Jne *-9
	tax
.elseif(n = 6935)
	; 9 bytes
	php
	lda $A5
	lda #27
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6937)
	; 7 bytes
	php
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6940)
	; 9 bytes
	php
	lda $A5
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6942)
	; 10 bytes
	php
	nop
	lda $A5
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6944)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6945)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6947)
	; 11 bytes
	php
	pha
	lda $A5
	lda #27
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6949)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6951)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 6952)
	; 12 bytes
	php
	delay_n 20
	lda #27
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #27
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6957)
	; 12 bytes
	php
	delay_n 20
	lda #27
	jsr delay_256a_16_clocks
	plp
.elseif(n = 6958)
	; 12 bytes
	php
	delay_n 21
	lda #27
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
	delay_n 24
	lda #27
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
	delay_n 39
	lda #27
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 6976)
	; 11 bytes
	php
	delay_n 39
	lda #27
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
.elseif(n = 7140)
	; 13 bytes
	tya
	pha
	ldy #162
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif((n > 7140 && n < 7145))
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 7146)
	; 13 bytes
	tya
	pha
	ldy #246
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7147 || n = 7148)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 14 bytes
	tya
	pha
	ldy #183
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7150 || n = 7151 || n = 7162)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7152)
	; 14 bytes
	tya
	pha
	ldy #119
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7153)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1180
	pla
	Jcc *-10
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
	; 15 bytes
	tya
	pha
	ldy #152
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7157)
	; 15 bytes
	tya
	ldy #149
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7158)
	; 14 bytes
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
.elseif(n = 7159)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1181
	pla
	Jcc *-11
.elseif(n = 7160)
	; 14 bytes
	tya
	ldy #135
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
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
.elseif(n = 7163)
	; 14 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	delay_n 24
	pla
	inx
	inx
	Jne *-9
	tax
.elseif(n = 7164)
	; 15 bytes
	tya
	pha
	ldy #149
	delay_n 43
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7165)
	; 15 bytes
	tya
	ldy #179
	pha
	delay_n 28
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7166)
	; 14 bytes
	tya
	ldy #217
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7167)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	delay_n 20
	pla
	dey
	dey
	Jne *-10
	tay
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 13 bytes
	tya
	ldy #247
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 7169)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 7170 || n = 7171 || n = 7177)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7172)
	; 14 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-9
	tay
.elseif(n = 7173)
	; 14 bytes
	tya
	ldy #0
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-8
	tay
.elseif(n = 7174)
	; 14 bytes
	tya
	ldy #107
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7175)
	; 13 bytes
	tya
	pha
	ldy #247
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7176)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
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
	; 14 bytes
	tya
	ldy #24 ;hides 'clc'
	pha
	delay_n 55
	pla
	iny
	Jpl *-9
	tay
.elseif(n = 7180)
	; 15 bytes
	tya
	pha
	ldy #224
	lda $A5
	delay_n 24
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7181)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7182)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 7183)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1185
	pla
	Jcc *-10
.elseif(n = 7184)
	; 13 bytes
	tya
	pha
	ldy #163
	delay_n 39
	dey
	Jne *-5
	pla
	tay
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
	; 14 bytes
	tya
	ldy #167
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7187)
	; 14 bytes
	tya
	ldy #189
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
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
.elseif(n = 7189 || n = 7190 || n = 7192 || n = 7194 || n = 7195 || n = 7197 || n = 7199 || n = 7202 || n = 7204 || n = 7206 || n = 7209)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7191)
	; 9 bytes
	php
	lda $A5
	lda #28
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7193)
	; 7 bytes
	php
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7196)
	; 9 bytes
	php
	lda $A5
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7198)
	; 10 bytes
	php
	nop
	lda $A5
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7200)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7201)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7203)
	; 11 bytes
	php
	pha
	lda $A5
	lda #28
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7205)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7207)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 7208)
	; 12 bytes
	php
	delay_n 20
	lda #28
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7210)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #28
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7213)
	; 12 bytes
	php
	delay_n 20
	lda #28
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7214)
	; 12 bytes
	php
	delay_n 21
	lda #28
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
	delay_n 24
	lda #28
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
	delay_n 39
	lda #28
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7232)
	; 11 bytes
	php
	delay_n 39
	lda #28
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
.elseif((n > 7395 && n < 7400))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7400)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 7401)
	; 14 bytes
	tya
	ldy #172
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7402 || n = 7406 || n = 7409 || n = 7416)
	; 14..15 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7404)
	; 13 bytes
	tya
	pha
	ldy #168
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7405)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7407)
	; 13 bytes
	tya
	pha
	ldy #255
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7408)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 49
	Jne *-8
	pla
	tay
.elseif(n = 7410)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 7411)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7412)
	; 15 bytes
	tya
	pha
	ldy #73
	delay_n 95
	dey
	Jpl *-7
	pla
	tay
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
	; 14 bytes
	tya
	ldy #(n-5)/31
	nop
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 7415)
	; 14 bytes
	tya
	ldy #195
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7417)
	; 15 bytes
	tya
	ldy #218
	pha
	delay_n 22
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7418)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7419)
	; 16 bytes
	php
	tya
	pha
	ldy #200
	lda #5
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
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 14 bytes
	tya
	pha
	ldy #190
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7423 || n = 7431 || n = 7432 || n = 7438 || n = 7440)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424 || n = 7439)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 7425)
	; 14 bytes
	tya
	ldy #140
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7426)
	; 15 bytes
	tya
	ldy #(n-5)/41
	pha
	delay_n 29
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7427)
	; 15 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	.repeat 3
	 php
	 plp
	.endrepeat
	iny
	Jne *-10
	tay
.elseif(n = 7428)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7429)
	; 13 bytes
	tya
	ldy #0
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-7
	tay
.elseif(n = 7430)
	; 14 bytes
	tya
	ldy #225
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7433)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/41
	delay_n 36
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7434)
	; 15 bytes
	tya
	pha
	ldy #24 ;hides 'clc'
	delay_n 55
	iny
	iny
	Jne *-8
	pla
	tay
.elseif(n = 7435)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	pha
	delay_n 1225
	pla
	Jcc *-11
.elseif(n = 7436)
	; 13 bytes
	tya
	pha
	ldy #0
	delay_n 24
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7437)
	; 15 bytes
	tya
	pha
	ldy #225
	delay_n 28
	dey
	Jne *-7
	pla
	tay
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
	; 14 bytes
	tya
	ldy #111
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7443)
	; 14 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
.elseif(n = 7444)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7445)
	; 14 bytes
	tya
	ldy #(n-5)/31
	nop
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 7446 || n = 7448 || n = 7450 || n = 7451 || n = 7453 || n = 7455 || n = 7458 || n = 7460)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 7447)
	; 9 bytes
	php
	lda $A5
	lda #29
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7449)
	; 7 bytes
	php
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7452)
	; 9 bytes
	php
	lda $A5
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7454)
	; 10 bytes
	php
	nop
	lda $A5
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7456)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7457)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7459)
	; 11 bytes
	php
	pha
	lda $A5
	lda #29
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7462 || n = 7465)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7463)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 7464)
	; 12 bytes
	php
	delay_n 20
	lda #29
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7466)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #29
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7469)
	; 12 bytes
	php
	delay_n 20
	lda #29
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7470)
	; 12 bytes
	php
	delay_n 21
	lda #29
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
	delay_n 24
	lda #29
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
	delay_n 39
	lda #29
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7488)
	; 11 bytes
	php
	delay_n 39
	lda #29
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
.elseif((n > 7651 && n < 7655) || n = 7656)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7655)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657 || n = 7670 || n = 7672 || n = 7675)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7658)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7659)
	; 14 bytes
	tya
	ldy #178
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7660)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7661)
	; 14 bytes
	tya
	ldy #232
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7662)
	; 14 bytes
	tya
	ldy #(n-5)/31
	nop
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 7663)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1265
	pla
	Jcc *-10
.elseif(n = 7664)
	; 15 bytes
	tya
	ldy #111
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7665)
	; 15 bytes
	clc
	delay_n (n-2)
.elseif(n = 7666)
	; 14 bytes
	tya
	pha
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-7
	pla
	tay
.elseif(n = 7667)
	; 15 bytes
	tya
	pha
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	delay_n 24
	iny
	Jne *-8
	pla
	tay
.elseif(n = 7668)
	; 13 bytes
	tya
	pha
	ldy #174
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7669)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7671)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 7673)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 7674)
	; 15 bytes
	tya
	pha
	lda #$4A ;hides 'lsr a'
	php
	ldy #189
	dey
	Jne *-1
	plp
	Jne *-8
	pla
	tay
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 7677 || n = 7683)
	; 15 bytes
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
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7681)
	; 14 bytes
	tya
	ldy #202
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7682)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 51
	Jne *-8
	pla
	tay
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
	; 14 bytes
	tya
	ldy #240
	pha
	delay_n 20
	pla
	dey
	Jne *-8
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7688)
	; 15 bytes
	tya
	ldy #197
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7689)
	; 15 bytes
	tya
	ldy #226
	pha
	delay_n 22
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7690)
	; 14 bytes
	tya
	ldy #145
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7691)
	; 13 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-8
	tay
.elseif(n = 7692)
	; 14 bytes
	tya
	pha
	ldy #127
	delay_n 55
	dey
	Jpl *-6
	pla
	tay
.elseif(n = 7693)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1270
	pla
	Jcc *-10
.elseif(n = 7694)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7695)
	; 14 bytes
	tya
	pha
	ldy #197
	delay_n 34
	dey
	Jne *-6
	pla
	tay
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
.elseif(n = 7697 || n = 7702 || n = 7704 || n = 7706 || n = 7707 || n = 7709 || n = 7711 || n = 7714 || n = 7716 || n = 7718 || n = 7721)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7698)
	; 13 bytes
	tya
	pha
	ldy #248 ;hides 'sed'
	delay_n 24
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7699)
	; 15 bytes
	tya
	pha
	lda #<$AD ;hides 'lda $17A0'
	ldy #23
	delay_n 24
	iny
	Jne *-8
	pla
	tay
.elseif(n = 7700)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7701)
	; 15 bytes
	tya
	pha
	ldy #233
	delay_n 28
	dey
	Jne *-7
	pla
	tay
.elseif(n = 7703)
	; 9 bytes
	php
	lda $A5
	lda #30
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7705)
	; 7 bytes
	php
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7708)
	; 9 bytes
	php
	lda $A5
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7710)
	; 10 bytes
	php
	nop
	lda $A5
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7712)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7713)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7715)
	; 11 bytes
	php
	pha
	lda $A5
	lda #30
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7717)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7719)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 7720)
	; 12 bytes
	php
	delay_n 20
	lda #30
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7722)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #30
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7725)
	; 12 bytes
	php
	delay_n 20
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7726)
	; 12 bytes
	php
	delay_n 21
	lda #30
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
	delay_n 24
	lda #30
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
	delay_n 39
	lda #30
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7744)
	; 11 bytes
	php
	delay_n 39
	lda #30
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
.elseif(n = 7908 || n = 7909 || n = 7911 || n = 7912)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7910)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 7913)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7914)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 99
	Jmi *-8
	pla
	tax
.elseif(n = 7915)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #30
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7916)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7917)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7918)
	; 15 bytes
	tya
	ldy #(n-5)/41
	pha
	delay_n 29
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7919 || n = 7927)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7920)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	php
	pha
	delay_n 20
	pla
	plp
	Jne *-10
	tax
.elseif(n = 7921)
	; 15 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	pha
	delay_n 21
	pla
	dey
	Jne *-10
	tay
.elseif(n = 7922)
	; 15 bytes
	tya
	ldy #203
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
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
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 7925)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
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
.elseif(n = 7928)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 7929)
	; 14 bytes
	tya
	pha
	ldy #203
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 15 bytes
	txa
	pha
	lda #<$AD ;hides 'lda $10A2'
	ldx #16
	delay_n 24
	inx
	Jne *-8
	pla
	tax
.elseif(n = 7931 || n = 7934 || n = 7936)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7932)
	; 13 bytes
	tya
	pha
	ldy #180
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 7933)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 134
	Jmi *-8
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #180
	delay_n 39
	dey
	Jne *-5
	sta (@ptrtemp),y
	pla
	tay
.elseif(n = 7939)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	tya
	ldy #248
	pha
	delay_n 20
	pla
	dey
	Jne *-8
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
	; 15 bytes
	tya
	ldy #147
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 7944)
	; 15 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #188
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
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
	; 15 bytes
	tya
	pha
	ldy #248 ;hides 'sed'
	delay_n 55
	dey
	dey
	Jne *-8
	pla
	tay
.elseif(n = 7947)
	; 14 bytes
	tya
	ldy #209
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7948)
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 26
	dey
	Jne *-6
	pla
	tay
.endif
.else
; 7949 to 7968
.if(n = 7949)
	; 14 bytes
	tya
	ldy #136 ;hides 'dey'
	php
	delay_n 46
	plp
	Jne *-9
	tay
.elseif(n = 7950 || n = 7953 || n = 7958 || n = 7960 || n = 7962 || n = 7963 || n = 7965 || n = 7967)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7951)
	; 15 bytes
	tya
	ldy #137
	php
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 7952)
	; 16 bytes
	php
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #168
	jsr delay_a_25_clocks
	pla
	dex
	Jmi *-9
	tax
	plp
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
	; 14 bytes
	tya
	ldy #150
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 7956)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7957)
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.elseif(n = 7959)
	; 9 bytes
	php
	lda $A5
	lda #31
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7961)
	; 7 bytes
	php
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7964)
	; 9 bytes
	php
	lda $A5
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7966)
	; 10 bytes
	php
	nop
	lda $A5
	lda #31
	jsr delay_256a_16_clocks
	plp
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
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7970 || n = 7972 || n = 7974 || n = 7977)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971)
	; 11 bytes
	php
	pha
	lda $A5
	lda #31
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7973)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7975)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 7976)
	; 12 bytes
	php
	delay_n 20
	lda #31
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7978)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #31
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 7981)
	; 12 bytes
	php
	delay_n 20
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 7982)
	; 12 bytes
	php
	delay_n 21
	lda #31
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
	delay_n 24
	lda #31
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
	delay_n 39
	lda #31
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8000)
	; 11 bytes
	php
	delay_n 39
	lda #31
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
.if((n >= 8165) && (n < 8184))
; 8165 to 8183
.if(n = 8165)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8170)
	; 14 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	iny ;first loop only
	delay_n 39
	iny
	Jne *-7
	pla
	tay
.elseif(n = 8171)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #31
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8172)
	; 14 bytes
	tya
	pha
	ldy #136
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8173)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8174)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8175)
	; 14 bytes
	tya
	ldy #215
	pha
	delay_n 26
	pla
	dey
	Jne *-8
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
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
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
	; 14 bytes
	tya
	ldy #121
	pha
	delay_n 55
	pla
	dey
	Jpl *-8
	tay
.elseif(n = 8180)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 8181)
	; 15 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8182)
	; 15 bytes
	tya
	pha
	ldy #215
	delay_n 33
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8183)
	; 15 bytes
	tya
	ldy #141
	php
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8185)
	; 15 bytes
	tya
	pha
	lda #$0A ;hides 'asl a'
	php
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	plp
	Jpl *-8
	pla
	tay
.elseif(n = 8186 || n = 8191 || n = 8198)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8187)
	; 14 bytes
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts
	jsr @nearby_rts
	dey
	Jne *-9
	tay
.elseif(n = 8188)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	jsr @nearby_rts
	jsr @nearby_rts
	iny
	Jne *-10
	tay
.elseif(n = 8189)
	; 14 bytes
	tya
	ldy #248
	pha
	delay_n 21
	pla
	dey
	Jne *-8
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8194)
	; 14 bytes
	tya
	pha
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-7
	pla
	tay
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
	; 13 bytes
	tya
	pha
	ldy #186
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 8197)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 20
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8199)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 8200)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8201 || n = 8203)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 8202)
	; 14 bytes
	tya
	pha
	ldy #210
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
.if((n >= 8204) && (n < 8223))
; 8204 to 8222
.if(n = 8204)
	; 15 bytes
	tya
	pha
	ldy #0
	lda $A5
	delay_n 24
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8205)
	; 15 bytes
	tya
	ldy #205
	pha
	delay_n 28
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8206)
	; 15 bytes
	tya
	pha
	ldy #241
	delay_n 29
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8207)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1356
	pla
	Jcc *-11
.elseif(n = 8210)
	; 14 bytes
	tya
	pha
	ldy #56 ;hides 'sec'
	delay_n 34
	iny
	Jne *-7
	pla
	tay
.elseif(n = 8211)
	; 13 bytes
	tya
	ldy #216 ;hides 'cld'
	pha
	delay_n 24
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8212)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8213)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8214 || n = 8216 || n = 8218 || n = 8219 || n = 8221)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 8215)
	; 9 bytes
	php
	lda $A5
	lda #32
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8217)
	; 7 bytes
	php
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8220)
	; 9 bytes
	php
	lda $A5
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8222)
	; 10 bytes
	php
	nop
	lda $A5
	lda #32
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 8223 to 8242
.if(n = 8223 || n = 8226 || n = 8228 || n = 8230 || n = 8233)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8225)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8227)
	; 11 bytes
	php
	pha
	lda $A5
	lda #32
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8229)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8231)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 8232)
	; 12 bytes
	php
	delay_n 20
	lda #32
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8234)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #32
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8237)
	; 12 bytes
	php
	delay_n 20
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8238)
	; 12 bytes
	php
	delay_n 21
	lda #32
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
	delay_n 24
	lda #32
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
	delay_n 39
	lda #32
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8256)
	; 11 bytes
	php
	delay_n 39
	lda #32
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
.elseif(n = 8420)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif((n > 8420 && n < 8425))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8425 || n = 8426)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8427)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #32
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8428)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8429)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8430)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8431 || n = 8434)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8432)
	; 14 bytes
	tya
	ldy #159
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8433)
	; 14 bytes
	tya
	ldy #196
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8435)
	; 14 bytes
	tya
	ldy #248 ;hides 'sed'
	pha
	delay_n 20
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8436)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8437)
	; 15 bytes
	tya
	ldy #248
	pha
	delay_n 22
	pla
	dey
	Jne *-9
	tay
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
.if((n >= 8438) && (n < 8457))
; 8438 to 8456
.if(n = 8438 || n = 8443)
	; 15 bytes
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
	; 14 bytes
	tya
	ldy #222
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8442)
	; 15 bytes
	tya
	pha
	ldy #248 ;hides 'sed'
	lda $A5
	delay_n 24
	dey
	Jne *-8
	pla
	tay
.elseif(n = 8444)
	; 15 bytes
	tya
	pha
	ldy #248
	delay_n 29
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8445)
	; 15 bytes
	tya
	ldy #211
	pha
	delay_n 28
	pla
	dey
	Jne *-9
	tay
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
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 8448)
	; 15 bytes
	tya
	pha
	ldy #222
	delay_n 33
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8449)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 8450)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 8451)
	; 15 bytes
	tya
	ldy #(n-5)/41
	pha
	delay_n 29
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8452)
	; 15 bytes
	txa
	pha
	clc
	lda #$2A ;hides 'rol a'
	ldx #234 ;hides 'nop'
	nop
	dex
	Jne *-3
	Jcc *-7
	pla
	tax
.elseif(n = 8453)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 21
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8454)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
	pha
	delay_n 1195
	pla
	Jcs *-11
	plp
.elseif(n = 8458)
	; 15 bytes
	tya
	ldy #79
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8459)
	; 15 bytes
	tya
	pha
	lda <$AD ;hides 'lda $00A0'
	ldy #0
	delay_n 24
	iny
	Jne *-8
	pla
	tay
.elseif(n = 8460)
	; 13 bytes
	tya
	pha
	ldy #192
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 8461)
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8462 || n = 8467 || n = 8470 || n = 8472 || n = 8474 || n = 8475)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8463)
	; 14 bytes
	lda #$2A ;hides 'rol a'
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
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
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8466)
	; 15 bytes
	tya
	pha
	ldy #192
	delay_n 39
	dey
	Jne *-5
	sta (@ptrtemp),y
	pla
	tay
.elseif(n = 8468)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8469)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8471)
	; 9 bytes
	php
	lda $A5
	lda #33
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8473)
	; 7 bytes
	php
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8476)
	; 9 bytes
	php
	lda $A5
	lda #33
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8479 || n = 8482 || n = 8484 || n = 8486 || n = 8489)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478)
	; 10 bytes
	php
	nop
	lda $A5
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8480)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8481)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8483)
	; 11 bytes
	php
	pha
	lda $A5
	lda #33
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8485)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8487)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 8488)
	; 12 bytes
	php
	delay_n 20
	lda #33
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8490)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #33
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8493)
	; 12 bytes
	php
	delay_n 20
	lda #33
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8494)
	; 12 bytes
	php
	delay_n 21
	lda #33
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
	delay_n 24
	lda #33
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
	delay_n 39
	lda #33
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8512)
	; 11 bytes
	php
	delay_n 39
	lda #33
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
.elseif((n > 8675 && n < 8680))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8680)
	; 13 bytes
	tya
	pha
	ldy #197
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 8681)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8682 || n = 8683 || n = 8689)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8684)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8685)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 8688)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8690)
	; 15 bytes
	tya
	ldy #193
	pha
	delay_n 33
	pla
	dey
	Jne *-9
	tay
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 14 bytes
	tya
	ldy #202
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8692)
	; 15 bytes
	tya
	pha
	ldy #140
	delay_n 57
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8693)
	; 15 bytes
	tya
	ldy #181
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8694)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $62A0'
	ldy #98
	delay_n 46
	iny
	Jne *-10
	tay
.elseif(n = 8695)
	; 15 bytes
	tya
	ldy #21
	pha
	delay_n 383
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 8696)
	; 15 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	iny ;first loop only
	delay_n 34
	iny
	Jne *-8
	pla
	tay
.elseif(n = 8697)
	; 14 bytes
	tya
	ldy #164
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8698)
	; 14 bytes
	tya
	pha
	ldy #181 ;hides 'lda zp,x'
	nop
	delay_n 39
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8699)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8700)
	; 15 bytes
	tya
	pha
	ldy #181
	delay_n 43
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8701 || n = 8708)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8702)
	; 15 bytes
	tya
	ldy #223
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/41
	delay_n 36
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8705)
	; 15 bytes
	tya
	ldy #28
	pha
	delay_n 288
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 8706)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8707)
	; 14 bytes
	tya
	ldy #229
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8709)
	; 14 bytes
	tya
	pha
	ldy #223
	delay_n 34
	dey
	Jne *-6
	pla
	tay
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
.if(n = 8711 || n = 8719 || n = 8721 || n = 8726 || n = 8728 || n = 8730)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8712)
	; 14 bytes
	tya
	pha
	ldy #145
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8713)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	pha
	delay_n 1440
	pla
	Jcc *-11
.elseif(n = 8714)
	; 15 bytes
	tya
	pha
	ldy #229
	delay_n 33
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8715)
	; 14 bytes
	tya
	ldy #130
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8716)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 29
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8717)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8718 || n = 8720)
	; 15..16 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 8722)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 8723)
	; 15 bytes
	tya
	ldy #242
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
.elseif(n = 8724)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8725)
	; 15 bytes
	tya
	ldy #218
	pha
	delay_n 28
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8727)
	; 9 bytes
	php
	lda $A5
	lda #34
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8729)
	; 7 bytes
	php
	lda #34
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 8731 to 8750
.if(n = 8731 || n = 8733 || n = 8735 || n = 8738 || n = 8740 || n = 8742 || n = 8745)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8732)
	; 9 bytes
	php
	lda $A5
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8734)
	; 10 bytes
	php
	nop
	lda $A5
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8736)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8737)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8739)
	; 11 bytes
	php
	pha
	lda $A5
	lda #34
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8741)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8743)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 8744)
	; 12 bytes
	php
	delay_n 20
	lda #34
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8746)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #34
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8749)
	; 12 bytes
	php
	delay_n 20
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8750)
	; 12 bytes
	php
	delay_n 21
	lda #34
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
	delay_n 24
	lda #34
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
	delay_n 39
	lda #34
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8768)
	; 11 bytes
	php
	delay_n 39
	lda #34
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
.elseif(n = 8932 || (n > 8933 && n < 8937))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8933)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8937 || n = 8938)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8939)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #34
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8940)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8941)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8942)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #5
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
.elseif(n = 8943)
	; 14 bytes
	tya
	pha
	ldy #229
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8944)
	; 13 bytes
	tya
	pha
	ldy #203
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 8945)
	; 15 bytes
	delay_n (n-2)
	nop
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946 || n = 8948 || n = 8954 || n = 8964)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8947)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 8949)
	; 14 bytes
	tya
	ldy #208
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8950)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/41
	delay_n 36
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8951)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 8952)
	; 14 bytes
	tya
	pha
	ldy #149
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 8953)
	; 15 bytes
	txa
	ldx #232 ;hides 'inx'
	pha
	delay_n 34
	pla
	inx
	inx
	Jne *-10
	tax
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
	; 15 bytes
	txa
	pha
	clc
	lda #$2A ;hides 'rol a'
	ldx #248 ;hides 'sed'
	nop
	dex
	Jne *-3
	Jcc *-7
	pla
	tax
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
	; 15 bytes
	tya
	ldy #199
	pha
	delay_n 33
	pla
	dey
	Jne *-9
	tay
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
	; 14 bytes
	tya
	ldy #169
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8963)
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	pha
	delay_n 24
	pla
	dey
	Jne *-10
	tay
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
	ldy #73
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
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $A3A0'
	ldy #163
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 8969)
	; 13 bytes
	tya
	ldy #(n-5)/36
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 8970)
	; 15 bytes
	tya
	ldy #163
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
.elseif(n = 8971 || n = 8977 || n = 8982 || n = 8984)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8972)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 8973)
	; 14 bytes
	tya
	ldy #236
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 8976)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-6
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #183
	delay_n 44
	dey
	Jne *-7
	pla
	tay
.elseif(n = 8980)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 8981)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 8983)
	; 9 bytes
	php
	lda $A5
	lda #35
	jsr delay_256a_11_clocks_
	plp
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
.elseif(n = 8986 || n = 8987 || n = 8989 || n = 8991 || n = 8994 || n = 8996 || n = 8998 || n = 9001)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8988)
	; 9 bytes
	php
	lda $A5
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8990)
	; 10 bytes
	php
	nop
	lda $A5
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 8992)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8993)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8995)
	; 11 bytes
	php
	pha
	lda $A5
	lda #35
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8997)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8999)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 9000)
	; 12 bytes
	php
	delay_n 20
	lda #35
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9002)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #35
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9005)
	; 12 bytes
	php
	delay_n 20
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9006)
	; 12 bytes
	php
	delay_n 21
	lda #35
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
	delay_n 24
	lda #35
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
	delay_n 39
	lda #35
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9024)
	; 11 bytes
	php
	delay_n 39
	lda #35
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
; 9063 to 9218
.if((n > 9062 && n < 9186))
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
.elseif((n > 9187 && n < 9193))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9193 || n = 9194)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9195)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #35
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9196)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9197)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
.elseif(n = 9199)
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9201)
	; 14 bytes
	tya
	ldy #242
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9202)
	; 16 bytes
	php
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9203 || n = 9214)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 9204)
	; 15 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #218
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
.elseif(n = 9205)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
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
	; 14 bytes
	tya
	ldy #214
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9208)
	; 13 bytes
	tya
	pha
	ldy #209
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 9209)
	; 15 bytes
	tya
	ldy #236
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9210)
	; 13 bytes
	tya
	pha
	ldy #56 ;hides 'sec'
	delay_n 39
	iny
	Jne *-6
	pla
	tay
.elseif(n = 9211 || n = 9213)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 9212)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9215 || n = 9217)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 9216)
	; 14 bytes
	tya
	pha
	ldy #236
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9218)
	; 15 bytes
	tya
	ldy #111
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9221)
	; 13 bytes
	tya
	ldy #0
	pha
	delay_n 24
	pla
	dey
	Jne *-7
	tay
.elseif(n = 9222)
	; 16 bytes
	php
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9223 || n = 9225 || n = 9229 || n = 9230)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 9224)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 9226)
	; 15 bytes
	tya
	pha
	lda #<$AD ;hides 'lda $C0A0'
	ldy #192
	delay_n 39
	dey
	Jne *-8
	pla
	tay
.elseif(n = 9227)
	; 14 bytes
	tya
	ldy #174
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9228)
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 31
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9231)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	delay_n 1135
	pla
	plp
	Jne *-12
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
.if(n = 9238 || n = 9240 || n = 9242 || n = 9243 || n = 9245 || n = 9247 || n = 9250 || n = 9252 || n = 9254 || n = 9257)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9239)
	; 9 bytes
	php
	lda $A5
	lda #36
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9241)
	; 7 bytes
	php
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9244)
	; 9 bytes
	php
	lda $A5
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9246)
	; 10 bytes
	php
	nop
	lda $A5
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9248)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9249)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9251)
	; 11 bytes
	php
	pha
	lda $A5
	lda #36
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9253)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9255)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 9256)
	; 12 bytes
	php
	delay_n 20
	lda #36
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #36
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9261)
	; 12 bytes
	php
	delay_n 20
	lda #36
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9262)
	; 12 bytes
	php
	delay_n 21
	lda #36
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
	delay_n 24
	lda #36
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
	delay_n 39
	lda #36
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9280)
	; 11 bytes
	php
	delay_n 39
	lda #36
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
.elseif((n > 9443 && n < 9449))
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9450)
	; 14 bytes
	tya
	pha
	ldy #242
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9451)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 1568
	Jcc *-11
.elseif(n = 9452)
	; 14 bytes
	tya
	ldy #141
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9453)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 15 bytes
	delay_n (n-2)
	nop
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 9458)
	; 15 bytes
	tya
	ldy #137
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9459)
	; 15 bytes
	tya
	ldy #163
	php
	delay_n 46
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
	; 15 bytes
	tya
	ldy #197
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $ACA0'
	ldy #172
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 9465)
	; 14 bytes
	tya
	ldy #220
	pha
	delay_n 31
	pla
	dey
	Jne *-8
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
	; 14 bytes
	tya
	ldy #249
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9468)
	; 15 bytes
	tya
	pha
	ldy #197
	delay_n 43
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9469)
	; 15 bytes
	tya
	pha
	ldy #193
	delay_n 44
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9470)
	; 16 bytes
	php
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9471)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	delay_n 1165
	pla
	plp
	Jne *-12
.elseif(n = 9472)
	; 13 bytes
	tya
	pha
	ldy #215
	delay_n 39
	dey
	Jne *-5
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
.elseif(n = 9474 || n = 9476 || n = 9486 || n = 9490)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9475)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
	; 15 bytes
	tya
	pha
	ldy #215
	delay_n 39
	dey
	Jne *-5
	sta (@ptrtemp),y
	pla
	tay
.elseif(n = 9479)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9482)
	; 15 bytes
	tya
	ldy #243
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9483)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/41
	delay_n 36
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9484)
	; 15 bytes
	tya
	pha
	ldy #148
	delay_n 59
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9485)
	; 15 bytes
	tya
	ldy #23
	pha
	delay_n 383
	pla
	dey
	Jpl *-9
	tay
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
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9489)
	; 14 bytes
	tya
	pha
	ldy #243
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9491)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
.if(n = 9493 || n = 9494 || n = 9496 || n = 9498 || n = 9499 || n = 9501 || n = 9503 || n = 9506 || n = 9508 || n = 9510 || n = 9513)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495)
	; 9 bytes
	php
	lda $A5
	lda #37
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9497)
	; 7 bytes
	php
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9500)
	; 9 bytes
	php
	lda $A5
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9502)
	; 10 bytes
	php
	nop
	lda $A5
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9504)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9505)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9507)
	; 11 bytes
	php
	pha
	lda $A5
	lda #37
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9509)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9511)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 9512)
	; 12 bytes
	php
	delay_n 20
	lda #37
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9514)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #37
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9517)
	; 12 bytes
	php
	delay_n 20
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9518)
	; 12 bytes
	php
	delay_n 21
	lda #37
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
	delay_n 24
	lda #37
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
	delay_n 39
	lda #37
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
	delay_n 39
	lda #37
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
.elseif((n > 9699 && n < 9705))
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9706)
	; 14 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	iny ;first loop only
	delay_n 39
	iny
	Jne *-7
	pla
	tay
.elseif(n = 9707)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #37
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9708)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9709)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9710)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 9711)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
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
	; 15 bytes
	tya
	ldy #249
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9717)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9718)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
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
	; 14 bytes
	tya
	ldy #145
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	tya
	ldy #(n-5)/41
	pha
	delay_n 29
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9723)
	; 14 bytes
	tya
	pha
	ldy #249
	delay_n 34
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9724)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	pha
	delay_n 24
	pla
	dey
	Jne *-10
	tay
.elseif(n = 9725)
	; 15 bytes
	tya
	ldy #180
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 9726)
	; 16 bytes
	php
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727 || n = 9743)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/41
	delay_n 36
	dey
	Jne *-7
	pla
	tay
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
	; 14 bytes
	tya
	pha
	ldy #162
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9733)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 26
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9734)
	; 15 bytes
	tya
	ldy #207
	pha
	delay_n 35
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9735)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	sec
	lda #239
	sbc #1
	Jne *-2
	pla
	plp
	Jne *-12
.elseif(n = 9736)
	; 13 bytes
	tya
	pha
	ldy #221
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 9737)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 66
	Jne *-8
	pla
	tay
.elseif(n = 9738 || n = 9745)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9739)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 9740)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 33
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9741)
	; 15 bytes
	tya
	pha
	ldy #207
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9742)
	; 15 bytes
	tya
	ldy #91
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9744)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	delay_n 1205
	pla
	Jcc *-11
	plp
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 9750 || n = 9752 || n = 9754 || n = 9755 || n = 9757 || n = 9759 || n = 9762 || n = 9764)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 9751)
	; 9 bytes
	php
	lda $A5
	lda #38
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9753)
	; 7 bytes
	php
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9756)
	; 9 bytes
	php
	lda $A5
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9758)
	; 10 bytes
	php
	nop
	lda $A5
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9760)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9761)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9763)
	; 11 bytes
	php
	pha
	lda $A5
	lda #38
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9765)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766 || n = 9769)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9767)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 9768)
	; 12 bytes
	php
	delay_n 20
	lda #38
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9770)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #38
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9773)
	; 12 bytes
	php
	delay_n 20
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9774)
	; 12 bytes
	php
	delay_n 21
	lda #38
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
	delay_n 24
	lda #38
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
	delay_n 39
	lda #38
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 9792)
	; 11 bytes
	php
	delay_n 39
	lda #38
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
	tya
	pha
	ldy #226
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif((n > 9956 && n < 9961))
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
.if(n = 9961 || n = 9962)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9963)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #38
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9964)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 9965)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	pha
	delay_n 31
	pla
	dey
	Jne *-10
	tay
.elseif(n = 9968)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
.elseif(n = 9969)
	; 14 bytes
	tya
	ldy #188
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 9972)
	; 14 bytes
	tya
	pha
	ldy #166
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9973)
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	sec
	lda #245
	sbc #1
	Jne *-2
	pla
	plp
	Jne *-12
.elseif(n = 9976)
	; 15 bytes
	tya
	pha
	ldy #212
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	ldy #24 ;hides 'clc'
	pha
	delay_n 29
	pla
	iny
	Jne *-10
	tay
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
	; 14 bytes
	tya
	ldy #232
	pha
	delay_n 31
	pla
	dey
	Jne *-8
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
.elseif(n = 9983 || n = 9990 || n = 9998)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 9984)
	; 16 bytes
	php
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	delay_n 1235
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
	; 15 bytes
	tya
	pha
	ldy #24 ;hides 'clc'
	delay_n 36
	iny
	Jne *-8
	pla
	tay
.elseif(n = 9987)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9988)
	; 14 bytes
	tya
	ldy #149
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 9989)
	; 15 bytes
	tya
	ldy #0
	cmp $C5
	pha
	delay_n 24
	pla
	dey
	Jne *-9
	tay
.elseif(n = 9991)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	delay_n 1230
	pla
	plp
	Jne *-12
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
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 9995)
	; 15 bytes
	tya
	ldy #185
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 9996)
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 34
	dey
	Jne *-6
	pla
	tay
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
	; 13 bytes
	tya
	pha
	ldy #227
	delay_n 39
	dey
	Jne *-5
	pla
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 10002 || n = 10003 || n = 10005 || n = 10006 || n = 10008 || n = 10010 || n = 10011 || n = 10013 || n = 10015 || n = 10018 || n = 10020 || n = 10022 || n = 10025)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 10004)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10007)
	; 9 bytes
	php
	lda $A5
	lda #39
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10009)
	; 7 bytes
	php
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10012)
	; 9 bytes
	php
	lda $A5
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10014)
	; 10 bytes
	php
	nop
	lda $A5
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10016)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10017)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10019)
	; 11 bytes
	php
	pha
	lda $A5
	lda #39
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10021)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10023)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10024)
	; 12 bytes
	php
	delay_n 20
	lda #39
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10026)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #39
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10029)
	; 12 bytes
	php
	delay_n 20
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10030)
	; 12 bytes
	php
	delay_n 21
	lda #39
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
	delay_n 24
	lda #39
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
	delay_n 39
	lda #39
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10048)
	; 11 bytes
	php
	delay_n 39
	lda #39
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
.elseif((n > 10211 && n < 10217))
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
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 10218)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10219)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #39
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10220)
	; 13 bytes
	tya
	pha
	ldy #232
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 10221)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10222 || n = 10226)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 10223)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 10224)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
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
.elseif(n = 10227)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	tya
	ldy #213
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10230)
	; 16 bytes
	php
	tya
	ldy #77
	pha
	lda #92
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	plp
.elseif(n = 10231)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	delay_n 1260
	pla
	plp
	Jne *-12
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
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $BAA0'
	ldy #186
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 10234)
	; 14 bytes
	tya
	ldy #193
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if(n = 10235)
	; 15 bytes
	tya
	ldy #186
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10236)
	; 15 bytes
	tya
	pha
	ldy #213
	delay_n 43
	dey
	Jne *-7
	pla
	tay
.elseif(n = 10237)
	; 16 bytes
	php
	tya
	pha
	ldy #77
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10239)
	; 14 bytes
	tya
	ldy #238
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 10240)
	; 16 bytes
	php
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10241)
	; 15 bytes
	delay_n (n-2)
	nop
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
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	pha
	delay_n 24
	pla
	dey
	Jne *-10
	tay
.elseif(n = 10244)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	pha
	delay_n 24
	pla
	dey
	Jne *-10
	tay
.elseif(n = 10245)
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 28
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10246)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	tya
	ldy #218
	pha
	delay_n 35
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10252)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 35
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #243
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
.elseif(n = 10255)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	sec
	lda #252
	sbc #1
	Jne *-2
	pla
	plp
	Jne *-12
.elseif(n = 10256)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 10257)
	; 14 bytes
	tya
	pha
	ldy #37 ;hides 'and zp'
	iny ;first loop only
	delay_n 39
	iny
	Jne *-7
	pla
	tay
.elseif(n = 10258 || n = 10262 || n = 10264 || n = 10266 || n = 10267 || n = 10269 || n = 10271)
	; 8..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10259)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
.elseif(n = 10263)
	; 9 bytes
	php
	lda $A5
	lda #40
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10265)
	; 7 bytes
	php
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10268)
	; 9 bytes
	php
	lda $A5
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10270)
	; 10 bytes
	php
	nop
	lda $A5
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10272)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10273)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10276 || n = 10278 || n = 10281)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275)
	; 11 bytes
	php
	pha
	lda $A5
	lda #40
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10277)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10279)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10280)
	; 12 bytes
	php
	delay_n 20
	lda #40
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10282)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #40
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10285)
	; 12 bytes
	php
	delay_n 20
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10286)
	; 12 bytes
	php
	delay_n 21
	lda #40
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
	delay_n 24
	lda #40
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
	delay_n 39
	lda #40
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10304)
	; 11 bytes
	php
	delay_n 39
	lda #40
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
.if((n >= 10469) && (n < 10547))
.if((n >= 10469) && (n < 10508))
.if((n >= 10469) && (n < 10488))
; 10469 to 10487
.if((n > 10468 && n < 10473))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10473 || n = 10474)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10475)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10476)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10477)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10478)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10479)
	; 15 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	pla
	plp
	Jne *-12
.elseif(n = 10480)
	; 16 bytes
	php
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10481)
	; 15 bytes
	tya
	ldy #194
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
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
	; 13 bytes
	tya
	pha
	ldy #238
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 10485)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10486)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10487)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
	; 15 bytes
	tya
	pha
	ldy #169
	delay_n 57
	dey
	Jne *-7
	pla
	tay
.elseif(n = 10491 || n = 10502 || n = 10504)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 10494)
	; 16 bytes
	php
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10495)
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 10496)
	; 15 bytes
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda $A5
	delay_n 39
	iny
	Jne *-8
	pla
	tay
.elseif(n = 10497)
	; 14 bytes
	tya
	ldy #244
	pha
	delay_n 31
	pla
	dey
	Jne *-8
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
	; 14 bytes
	tya
	ldy #198
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 10500)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 10501)
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 29
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10503)
	; 15 bytes
	tya
	ldy #181
	php
	delay_n 46
	plp
	dey
	Jne *-9
	tay
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
	; 15 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #249
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
.elseif(n = 10507)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1746
	Jcc *-10
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 36
	dey
	Jne *-7
	pla
	tay
.elseif(n = 10509)
	; 15 bytes
	clc
	delay_n (n-2)
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 10512)
	; 14 bytes
	tya
	pha
	ldy #175
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 10513 || n = 10515 || n = 10518 || n = 10520 || n = 10522 || n = 10523 || n = 10525)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 10514)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 10516)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10517)
	; 15 bytes
	tya
	ldy #219
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10519)
	; 9 bytes
	php
	lda $A5
	lda #41
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10521)
	; 7 bytes
	php
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10524)
	; 9 bytes
	php
	lda $A5
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10526)
	; 10 bytes
	php
	nop
	lda $A5
	lda #41
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 10527 to 10546
.if(n = 10527 || n = 10530 || n = 10532 || n = 10534 || n = 10537)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10528)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10529)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10531)
	; 11 bytes
	php
	pha
	lda $A5
	lda #41
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10533)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10535)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10536)
	; 12 bytes
	php
	delay_n 20
	lda #41
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10538)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #41
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10541)
	; 12 bytes
	php
	delay_n 20
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10542)
	; 12 bytes
	php
	delay_n 21
	lda #41
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
	delay_n 24
	lda #41
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
	delay_n 39
	lda #41
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10560)
	; 11 bytes
	php
	delay_n 39
	lda #41
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
.elseif((n > 10723 && n < 10729))
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10730)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 10731)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #41
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10732)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10733)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 10736)
	; 16 bytes
	php
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10737)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 10738)
	; 15 bytes
	tya
	pha
	ldy #173
	delay_n 57
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #37 ;hides 'and zp'
	lda $EA ;hides 'nop'
	delay_n 39
	iny
	Jne *-8
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #219
	delay_n 44
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
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
	; 13 bytes
	tya
	pha
	ldy #244
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 10749)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	delay_n 455
	pla
	Jcs *-11
	plp
.elseif(n = 10750 || n = 10754)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 10751)
	; 15 bytes
	tya
	ldy #199
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 10752)
	; 14 bytes
	tya
	pha
	ldy #179
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 10753)
	; 16 bytes
	php
	delay_n 39
	lda #166
	jsr delay_a_25_clocks
	lda #(n-257)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10755)
	; 14 bytes
	tya
	ldy #250
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	tya
	ldy #224
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
.elseif(n = 10758)
	; 15 bytes
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #255
	nop
	dex
	Jne *-2
	Jcc *-7
	pla
	tax
.elseif(n = 10759)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 10760)
	; 15 bytes
	tya
	ldy #239
	pha
	delay_n 33
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	pha
	lda <$AD ;hides 'lda $E0A0'
	ldy #224
	delay_n 39
	dey
	Jne *-8
	pla
	tay
.elseif(n = 10764)
	; 14 bytes
	tya
	ldy #203
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
.elseif(n = 10768 || n = 10770 || n = 10771 || n = 10774 || n = 10776 || n = 10778 || n = 10779 || n = 10781)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 10769)
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 10772)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10773)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 10775)
	; 9 bytes
	php
	lda $A5
	lda #42
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10777)
	; 7 bytes
	php
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10780)
	; 9 bytes
	php
	lda $A5
	lda #42
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782)
	; 10 bytes
	php
	nop
	lda $A5
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10783 || n = 10786 || n = 10788 || n = 10790 || n = 10793)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10784)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10785)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10787)
	; 11 bytes
	php
	pha
	lda $A5
	lda #42
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10789)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10791)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 10792)
	; 12 bytes
	php
	delay_n 20
	lda #42
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10794)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #42
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10797)
	; 12 bytes
	php
	delay_n 20
	lda #42
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10798)
	; 12 bytes
	php
	delay_n 21
	lda #42
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
	delay_n 24
	lda #42
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
	delay_n 39
	lda #42
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 10816)
	; 11 bytes
	php
	delay_n 39
	lda #42
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
.elseif((n > 10979 && n < 10985))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10985 || n = 10986)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10987)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 1824
	Jcc *-11
.elseif(n = 10988)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10989)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 14 bytes
	tya
	pha
	ldy #183
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 10993)
	; 14 bytes
	tya
	ldy #164
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 10994 || n = 10995)
	; 15 bytes
	delay_n (n-2)
	nop
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 16 bytes
	php
	delay_n 24
	lda #168
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 10997)
	; 15 bytes
	tya
	ldy #229
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11000)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 11001 || n = 11008 || n = 11014)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 11002)
	; 15 bytes
	tya
	pha
	lda #<$AD ;hides 'lda $1BA0'
	ldy #27
	delay_n 39
	iny
	Jne *-8
	pla
	tay
.elseif(n = 11003)
	; 14 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	delay_n 41
	pla
	iny
	Jne *-9
	tay
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
	; 15 bytes
	tya
	ldy #200
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11006)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #234
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
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
	; 13 bytes
	tya
	pha
	ldy #250
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 11013)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 31
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11015)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
.if(n = 11016 || n = 11023 || n = 11030 || n = 11032 || n = 11034)
	; 8..15 bytes
	delay_n (n-2)
	nop
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
	; 15 bytes
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	dey ;first loop only
	delay_n 55
	iny
	Jne *-8
	pla
	tay
.elseif(n = 11019)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 11020)
	; 15 bytes
	tya
	pha
	ldy #172
	delay_n 59
	dey
	Jne *-7
	pla
	tay
.elseif(n = 11021)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
.elseif(n = 11024)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 11025)
	; 15 bytes
	tya
	ldy #190
	php
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 11026)
	; 15 bytes
	tya
	ldy #102
	pha
	delay_n 95
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 11027)
	; 15 bytes
	tya
	ldy #(n-11)/51
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
.elseif(n = 11028)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11029)
	; 14 bytes
	tya
	ldy #208
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11031)
	; 9 bytes
	php
	lda $A5
	lda #43
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11033)
	; 7 bytes
	php
	lda #43
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 11035 to 11054
.if(n = 11035 || n = 11037 || n = 11039 || n = 11042 || n = 11044 || n = 11046 || n = 11049)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11036)
	; 9 bytes
	php
	lda $A5
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11038)
	; 10 bytes
	php
	nop
	lda $A5
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11041)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11043)
	; 11 bytes
	php
	pha
	lda $A5
	lda #43
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11045)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11047)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 11048)
	; 12 bytes
	php
	delay_n 20
	lda #43
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11050)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #43
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11053)
	; 12 bytes
	php
	delay_n 20
	lda #43
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11054)
	; 12 bytes
	php
	delay_n 21
	lda #43
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
	delay_n 24
	lda #43
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
	delay_n 39
	lda #43
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11072)
	; 11 bytes
	php
	delay_n 39
	lda #43
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
.elseif((n > 11235 && n < 11241))
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
	tya
	ldy #212
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11242)
	; 14 bytes
	tya
	pha
	ldy #234 ;hides 'nop'
	delay_n 41
	dey
	Jne *-7
	pla
	tay
.elseif(n = 11243)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #43
	jsr delay_256a_16_clocks
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
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	tya
	ldy #163
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	ldy #250
	pha
	delay_n 33
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11256)
	; 16 bytes
	php
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11257)
	; 15 bytes
	tya
	ldy #194
	php
	delay_n 46
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
.elseif(n = 11261)
	; 14 bytes
	tya
	ldy #168
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
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
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11276)
	; 13 bytes
	tya
	pha
	ldy #0
	delay_n 39
	dey
	Jne *-5
	pla
	tay
.elseif(n = 11277 || n = 11278 || n = 11286 || n = 11288)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 11279)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 11282)
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 11283)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 11287)
	; 9 bytes
	php
	lda $A5
	lda #44
	jsr delay_256a_11_clocks_
	plp
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
.if(n = 11290 || n = 11291 || n = 11293 || n = 11295 || n = 11298 || n = 11300 || n = 11302 || n = 11305)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11292)
	; 9 bytes
	php
	lda $A5
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11294)
	; 10 bytes
	php
	nop
	lda $A5
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11296)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11297)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11299)
	; 11 bytes
	php
	pha
	lda $A5
	lda #44
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11301)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11303)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 11304)
	; 12 bytes
	php
	delay_n 20
	lda #44
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11306)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #44
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11309)
	; 12 bytes
	php
	delay_n 20
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11310)
	; 12 bytes
	php
	delay_n 21
	lda #44
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
	delay_n 24
	lda #44
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
	delay_n 39
	lda #44
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11328)
	; 11 bytes
	php
	delay_n 39
	lda #44
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
.elseif((n > 11491 && n < 11497))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11497 || n = 11498)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11499)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11500)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11501)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11502 || n = 11503)
	; 16 bytes
	php
	pha
	delay_n (n-14)
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
	; 14 bytes
	tya
	ldy #(n-5)/46
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11506)
	; 14 bytes
	tya
	ldy #217
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11507)
	; 15 bytes
	tya
	ldy #213
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 11508 || n = 11514)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 11509)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	; 14 bytes
	tya
	pha
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 11513 || n = 11519)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 11515)
	; 16 bytes
	php
	delay_n 39
	lda #160
	jsr delay_a_25_clocks
	lda #(n-251)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11516)
	; 16 bytes
	php
	delay_n 39
	lda #161
	jsr delay_a_25_clocks
	lda #(n-252)/256
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
.elseif(n = 11520)
	; 15 bytes
	tya
	ldy #245
	pha
	delay_n 35
	pla
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #167
	jsr delay_a_25_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11523)
	; 16 bytes
	php
	delay_n 39
	lda #168
	jsr delay_a_25_clocks
	lda #(n-259)/256
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
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 33
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	pha
	ldy #245
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
.elseif(n = 11528)
	; 15 bytes
	tya
	ldy #167
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11529)
	; 14 bytes
	tya
	ldy #172
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11530)
	; 15 bytes
	tya
	pha
	ldy #181 ;hides 'lda zp,x'
	dey ;first loop only
	delay_n 55
	dey
	Jne *-8
	pla
	tay
.elseif(n = 11531)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 11532)
	; 14 bytes
	tya
	pha
	ldy #192
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 11533 || n = 11535 || n = 11542)
	; 8..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 11534)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
	; 15 bytes
	tya
	ldy #(n-11)/51
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
.elseif(n = 11538 || n = 11539)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 11540)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11541)
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.endif
.else
; 11543 to 11562
.if(n = 11543)
	; 9 bytes
	php
	lda $A5
	lda #45
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11544 || n = 11546 || n = 11547 || n = 11549 || n = 11551 || n = 11554 || n = 11556 || n = 11558 || n = 11561)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11545)
	; 7 bytes
	php
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11548)
	; 9 bytes
	php
	lda $A5
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11550)
	; 10 bytes
	php
	nop
	lda $A5
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11552)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11553)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11555)
	; 11 bytes
	php
	pha
	lda $A5
	lda #45
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11557)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11559)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 11560)
	; 12 bytes
	php
	delay_n 20
	lda #45
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11562)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #45
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11565)
	; 12 bytes
	php
	delay_n 20
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11566)
	; 12 bytes
	php
	delay_n 21
	lda #45
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
	delay_n 24
	lda #45
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
	delay_n 39
	lda #45
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11584)
	; 11 bytes
	php
	delay_n 39
	lda #45
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
.elseif((n > 11747 && n < 11753))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11753 || n = 11754)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11755)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #45
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11756)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11757)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	lda $EA ;hides 'nop'
	delay_n 39
	iny
	Jne *-8
	pla
	tay
.elseif(n = 11760)
	; 15 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	nop
	delay_n 41
	iny
	Jne *-8
	pla
	tay
.elseif(n = 11761)
	; 14 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	delay_n 39
	iny
	Jne *-7
	pla
	tay
.elseif(n = 11762)
	; 15 bytes
	tya
	pha
	ldy #250
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
.elseif(n = 11763 || n = 11774)
	; 15 bytes
	delay_n (n-2)
	nop
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
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
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
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	tya
	ldy #222
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11772)
	; 14 bytes
	tya
	pha
	ldy #196
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 11773)
	; 14 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 11775)
	; 15 bytes
	tya
	ldy #214
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
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
	delay_n 46
	plp
	Jne *-9
	tax
.elseif(n = 11778 || n = 11780 || n = 11793 || n = 11795)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 11779)
	; 15 bytes
	tya
	ldy #203
	php
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 11781)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 34
	pla
	dey
	Jne *-8
	tay
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
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
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 41
	dey
	Jne *-6
	pla
	tay
.elseif(n = 11789)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
	; 15 bytes
	tya
	ldy #(n-5)/71
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
.elseif(n = 11792)
	; 15 bytes
	tya
	pha
	ldy #136 ;hides 'dey'
	delay_n 81
	Jne *-8
	pla
	tay
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
	; 14 bytes
	tya
	ldy #176
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 11798 || n = 11800 || n = 11802 || n = 11803 || n = 11805 || n = 11807 || n = 11810 || n = 11812 || n = 11814 || n = 11817)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11799)
	; 9 bytes
	php
	lda $A5
	lda #46
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11801)
	; 7 bytes
	php
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11804)
	; 9 bytes
	php
	lda $A5
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11806)
	; 10 bytes
	php
	nop
	lda $A5
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11808)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11809)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11811)
	; 11 bytes
	php
	pha
	lda $A5
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11813)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11815)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 11816)
	; 12 bytes
	php
	delay_n 20
	lda #46
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11818)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #46
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11821)
	; 12 bytes
	php
	delay_n 20
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 11822)
	; 12 bytes
	php
	delay_n 21
	lda #46
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
	delay_n 24
	lda #46
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
	delay_n 39
	lda #46
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 11840)
	; 11 bytes
	php
	delay_n 39
	lda #46
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
; 11876 to 12031
.if((n > 11875 && n < 12002))
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
.elseif((n > 12003 && n < 12009))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12009 || n = 12010)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12011)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12012)
	; 14 bytes
	tya
	pha
	ldy #200
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 12013)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12014)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 12015)
	; 16 bytes
	php
	pha
	delay_n (n-14)
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
	; 15 bytes
	tya
	pha
	ldy #245
	delay_n 44
	dey
	Jne *-7
	pla
	tay
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 12020)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12021)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12022)
	; 15 bytes
	tya
	pha
	ldy #74 ;hides 'lsr a'
	delay_n 59
	iny
	Jne *-8
	pla
	tay
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
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
	delay_n 39
	lda #161
	jsr delay_a_25_clocks
	lda #(n-252)/256
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
	delay_n 39
	lda #166
	jsr delay_a_25_clocks
	lda #(n-257)/256
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
	delay_n 39
	lda #168
	jsr delay_a_25_clocks
	lda #(n-259)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12036)
	; 14 bytes
	tya
	ldy #227
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 12037)
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 206
	Jmi *-8
	pla
	tay
.elseif(n = 12038 || n = 12042 || n = 12043)
	; 14..16 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 12044)
	; 15 bytes
	tya
	pha
	ldy #0
	lda $A5
	delay_n 39
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #138 ;hides 'txa'
	delay_n 95
	iny
	Jne *-8
	pla
	tay
.elseif(n = 12047)
	; 15 bytes
	tya
	ldy #223
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 12048)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 12049)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 12050)
	; 15 bytes
	tya
	ldy #219
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	ldy #251
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12054 || n = 12056 || n = 12058 || n = 12059 || n = 12061 || n = 12063 || n = 12066 || n = 12068 || n = 12070)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12055)
	; 9 bytes
	php
	lda $A5
	lda #47
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12057)
	; 7 bytes
	php
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12060)
	; 9 bytes
	php
	lda $A5
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12062)
	; 10 bytes
	php
	nop
	lda $A5
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12065)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12067)
	; 11 bytes
	php
	pha
	lda $A5
	lda #47
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12069)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12072)
	; 12 bytes
	php
	delay_n 20
	lda #47
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12073)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12074)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #47
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12077)
	; 12 bytes
	php
	delay_n 20
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12078)
	; 12 bytes
	php
	delay_n 21
	lda #47
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
	delay_n 24
	lda #47
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
	delay_n 39
	lda #47
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12096)
	; 11 bytes
	php
	delay_n 39
	lda #47
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
.elseif((n > 12259 && n < 12265))
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
	; 15 bytes
	tya
	pha
	ldy #200 ;hides 'iny'
	delay_n 210
	Jmi *-8
	pla
	tay
.endif
.else
.if((n >= 12266) && (n < 12305))
.if((n >= 12266) && (n < 12285))
; 12266 to 12284
.if(n = 12266)
	; 14 bytes
	tya
	ldy #183
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 12267)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12268)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12269)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12270)
	; 15 bytes
	tya
	ldy #223
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
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
	pha
	delay_n (n-7)
	pla
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
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
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
	lda #23
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
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
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
	; 15 bytes
	tya
	ldy #178
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	ldy #148
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 36
	pla
	dey
	Jne *-9
	tay
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
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12299)
	; 13 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 46
	iny
	Jne *-8
	tay
.elseif(n = 12300)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 43
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12301)
	; 14 bytes
	tya
	ldy #232
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 12302)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 12303)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
	pha
	delay_n 288
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 12306)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 12309)
	; 15 bytes
	tya
	pha
	ldy #5 ;hides 'ora zp'
	lda $EA ;hides 'nop'
	delay_n 39
	iny
	Jne *-8
	pla
	tay
.elseif(n = 12310 || n = 12312 || n = 12314 || n = 12315 || n = 12317 || n = 12319 || n = 12322 || n = 12324 || n = 12326 || n = 12329)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12311)
	; 9 bytes
	php
	lda $A5
	lda #48
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12313)
	; 7 bytes
	php
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12316)
	; 9 bytes
	php
	lda $A5
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12318)
	; 10 bytes
	php
	nop
	lda $A5
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12321)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12323)
	; 11 bytes
	php
	pha
	lda $A5
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12325)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12327)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12328)
	; 12 bytes
	php
	delay_n 20
	lda #48
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12330)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #48
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12333)
	; 12 bytes
	php
	delay_n 20
	lda #48
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12334)
	; 12 bytes
	php
	delay_n 21
	lda #48
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
	delay_n 24
	lda #48
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
	delay_n 39
	lda #48
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12352)
	; 11 bytes
	php
	delay_n 39
	lda #48
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
.elseif((n > 12515 && n < 12521))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12521 || n = 12522)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12523)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 2080
	Jcc *-11
.elseif(n = 12524)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12525)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12531)
	; 15 bytes
	tya
	ldy #216 ;hides 'cld'
	pha
	delay_n 44
	pla
	dey
	Jne *-10
	tay
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
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 12534)
	; 14 bytes
	tya
	ldy #187
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	tya
	pha
	ldy #202
	delay_n 57
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	ldy #151
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12539)
	; 16 bytes
	php
	delay_n 39
	lda #160
	jsr delay_a_25_clocks
	lda #(n-251)/256
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
.elseif(n = 12541 || n = 12550 || n = 12558)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
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
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
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
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 12546 || n = 12554)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 12547)
	; 15 bytes
	sec
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
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
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12551)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
.elseif(n = 12552)
	; 14 bytes
	tya
	pha
	ldy #209
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 12553 || n = 12555)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 12556)
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 44
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12557)
	; 15 bytes
	tya
	ldy #(n-11)/51
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 14 bytes
	lda #$2A ;hides 'rol a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
.elseif(n = 12560 || n = 12566 || n = 12568 || n = 12570 || n = 12571 || n = 12573 || n = 12575 || n = 12578)
	; 8..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12561)
	; 15 bytes
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
.elseif(n = 12562)
	; 16 bytes
	php
	tya
	pha
	ldy #112
	lda #79
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	plp
.elseif(n = 12563)
	; 15 bytes
	tya
	ldy #182
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12564)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12565)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	delay_n 2089
	Jcc *-11
.elseif(n = 12567)
	; 9 bytes
	php
	lda $A5
	lda #49
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12569)
	; 7 bytes
	php
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12572)
	; 9 bytes
	php
	lda $A5
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12574)
	; 10 bytes
	php
	nop
	lda $A5
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12576)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12577)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579)
	; 11 bytes
	php
	pha
	lda $A5
	lda #49
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12580 || n = 12582 || n = 12585)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12581)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12583)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12584)
	; 12 bytes
	php
	delay_n 20
	lda #49
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12586)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #49
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12589)
	; 12 bytes
	php
	delay_n 20
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12590)
	; 12 bytes
	php
	delay_n 21
	lda #49
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
	delay_n 24
	lda #49
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
	delay_n 39
	lda #49
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12608)
	; 11 bytes
	php
	delay_n 39
	lda #49
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
.elseif(n = 12772 || n = 12773)
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12778)
	; 14 bytes
	tya
	ldy #241
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 12779)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12780)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12781)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	pha
	ldy #206
	delay_n 57
	dey
	Jne *-7
	pla
	tay
.elseif(n = 12785)
	; 15 bytes
	tya
	ldy #(n-5)/71
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
.elseif(n = 12786)
	; 16 bytes
	php
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	; 14 bytes
	tya
	pha
	ldy #213
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 16 bytes
	php
	delay_n 24
	lda #173
	jsr delay_a_25_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12794 || n = 12804 || n = 12808)
	; 14..15 bytes
	delay_n (n-2)
	nop
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
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12798)
	; 16 bytes
	php
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
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
	; 14 bytes
	tya
	ldy #191
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 12803)
	; 15 bytes
	tya
	ldy #237
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 12805)
	; 16 bytes
	php
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12806)
	; 13 bytes
	tya
	ldy #(n-5)/51
	delay_n 46
	dey
	Jne *-7
	tay
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
.elseif(n = 12809)
	; 15 bytes
	delay_n (n-3)
	lda $A5
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
	; 15 bytes
	tya
	pha
	ldy #200
	delay_n 59
	dey
	Jne *-7
	pla
	tay
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 12813) && (n < 12969))
.if((n >= 12813) && (n < 12891))
.if((n >= 12813) && (n < 12852))
.if((n >= 12813) && (n < 12832))
; 12813 to 12831
.if(n = 12813)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 12815 || n = 12822 || n = 12824 || n = 12826 || n = 12827 || n = 12829 || n = 12831)
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
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 46
	iny
	Jne *-10
	tay
.elseif(n = 12819)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 46
	iny
	Jne *-10
	tay
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
.elseif(n = 12823)
	; 9 bytes
	php
	lda $A5
	lda #50
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12825)
	; 7 bytes
	php
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12828)
	; 9 bytes
	php
	lda $A5
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12830)
	; 10 bytes
	php
	nop
	lda $A5
	lda #50
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 12832 to 12851
.if(n = 12832)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12833)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12834 || n = 12836 || n = 12838 || n = 12841)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12835)
	; 11 bytes
	php
	pha
	lda $A5
	lda #50
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12837)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12839)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 12840)
	; 12 bytes
	php
	delay_n 20
	lda #50
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12842)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #50
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12845)
	; 12 bytes
	php
	delay_n 20
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 12846)
	; 12 bytes
	php
	delay_n 21
	lda #50
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
	delay_n 24
	lda #50
	jsr delay_256a_16_clocks
	plp
.endif
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
	delay_n 39
	lda #50
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 12864)
	; 11 bytes
	php
	delay_n 39
	lda #50
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
.elseif((n > 13027 && n < 13033))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13033 || n = 13034)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13035)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #50
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13036)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13037)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13038)
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	delay_n 46
	iny
	Jne *-10
	tay
.elseif(n = 13039)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 13040)
	; 15 bytes
	tya
	ldy #32
	pha
	delay_n 383
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
	; 14 bytes
	tya
	ldy #246
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	pla
	pha
	delay_n 39
	iny
	Jne *-8
	pla
	tay
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
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
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
	delay_n 46
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
	; 15 bytes
	tya
	ldy #121
	pha
	delay_n 95
	pla
	dey
	Jpl *-9
	tay
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
	; 13 bytes
	tya
	ldy #0
	delay_n 46
	dey
	Jne *-7
	tay
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
.elseif(n = 13063 || n = 13065)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 13064)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 15 bytes
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	dey ;first loop only
	delay_n 55
	iny
	Jne *-8
	pla
	tay
.elseif(n = 13067)
	; 15 bytes
	tya
	ldy #0
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
.elseif(n = 13068 || n = 13077)
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 13069)
	; 15 bytes
	tya
	ldy #(n-5)/71
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13070)
	; 14 bytes
	tya
	ldy #195
	pha
	delay_n 55
	pla
	dey
	Jne *-8
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
	; 15 bytes
	tya
	ldy #242
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
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
.elseif(n = 13078 || n = 13080 || n = 13082 || n = 13083 || n = 13085)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 13079)
	; 9 bytes
	php
	lda $A5
	lda #51
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13081)
	; 7 bytes
	php
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13084)
	; 9 bytes
	php
	lda $A5
	lda #51
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086)
	; 10 bytes
	php
	nop
	lda $A5
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13087 || n = 13090 || n = 13092 || n = 13094 || n = 13097)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13088)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13089)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13091)
	; 11 bytes
	php
	pha
	lda $A5
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13093)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13095)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 13096)
	; 12 bytes
	php
	delay_n 20
	lda #51
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13098)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #51
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13101)
	; 12 bytes
	php
	delay_n 20
	lda #51
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13102)
	; 12 bytes
	php
	delay_n 21
	lda #51
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
	delay_n 24
	lda #51
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
	delay_n 39
	lda #51
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13120)
	; 11 bytes
	php
	delay_n 39
	lda #51
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
.elseif((n > 13283 && n < 13289))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13289 || n = 13290)
	; 15 bytes
	php
	pha
	nop
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
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 13296)
	; 16 bytes
	php
	tya
	ldy #108
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	plp
.elseif(n = 13297)
	; 16 bytes
	php
	delay_n 24
	lda #165
	jsr delay_a_25_clocks
	lda #(n-241)/256
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
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
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
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	; 14 bytes
	tya
	ldy #251
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.elseif(n = 13309)
	; 16 bytes
	php
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
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
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 46
	iny
	Jne *-10
	tay
.elseif(n = 13314)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 46
	iny
	Jne *-10
	tay
.elseif(n = 13315)
	; 15 bytes
	tya
	ldy #242
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13316)
	; 16 bytes
	php
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
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
.if(n = 13321 || n = 13334 || n = 13336 || n = 13338 || n = 13339)
	; 8..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13322)
	; 15 bytes
	tya
	ldy #193
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	pha
	ldy #208
	delay_n 59
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13335)
	; 9 bytes
	php
	lda $A5
	lda #52
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13337)
	; 7 bytes
	php
	lda #52
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 13340 to 13359
.if(n = 13340)
	; 9 bytes
	php
	lda $A5
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13341 || n = 13343 || n = 13346 || n = 13348 || n = 13350 || n = 13353)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13342)
	; 10 bytes
	php
	nop
	lda $A5
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13344)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13345)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13347)
	; 11 bytes
	php
	pha
	lda $A5
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13349)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13351)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 13352)
	; 12 bytes
	php
	delay_n 20
	lda #52
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13354)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #52
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13357)
	; 12 bytes
	php
	delay_n 20
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13358)
	; 12 bytes
	php
	delay_n 21
	lda #52
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
	delay_n 24
	lda #52
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
	delay_n 39
	lda #52
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13376)
	; 11 bytes
	php
	delay_n 39
	lda #52
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
.elseif((n > 13539 && n < 13545))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13545 || n = 13546)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13547)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13548)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13549)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 24
	lda #168
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13557)
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13558)
	; 16 bytes
	php
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13559)
	; 15 bytes
	tya
	ldy #251
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #160
	jsr delay_a_25_clocks
	lda #(n-251)/256
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
	delay_n 39
	lda #164
	jsr delay_a_25_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13568)
	; 16 bytes
	php
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
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
	; 14 bytes
	tya
	pha
	ldy #226
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 13573)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 41
	pla
	dey
	Jne *-8
	tay
.endif
.else
; 13574 to 13593
.if(n = 13574 || n = 13575 || n = 13590 || n = 13592)
	; 8..15 bytes
	delay_n (n-2)
	nop
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
	delay_n 46
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 13580)
	; 15 bytes
	tya
	pha
	ldy #212
	delay_n 59
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13581)
	; 17 bytes
	php
	lda #91
	jsr delay_a_25_clocks
	lda #99
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 13591)
	; 9 bytes
	php
	lda $A5
	lda #53
	jsr delay_256a_11_clocks_
	plp
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
.if(n = 13594 || n = 13595 || n = 13597 || n = 13599 || n = 13602 || n = 13604 || n = 13606 || n = 13609)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13596)
	; 9 bytes
	php
	lda $A5
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13598)
	; 10 bytes
	php
	nop
	lda $A5
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13600)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13601)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13603)
	; 11 bytes
	php
	pha
	lda $A5
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13605)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13607)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 13608)
	; 12 bytes
	php
	delay_n 20
	lda #53
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13610)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #53
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13613)
	; 12 bytes
	php
	delay_n 20
	lda #53
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13614)
	; 12 bytes
	php
	delay_n 21
	lda #53
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
	delay_n 24
	lda #53
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
	delay_n 39
	lda #53
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13632)
	; 11 bytes
	php
	delay_n 39
	lda #53
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
.elseif((n > 13795 && n < 13801))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13801 || n = 13802)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13803)
	; 14 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	delay_n 55
	pla
	iny
	Jne *-9
	tay
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
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 14 bytes
	tya
	ldy #206
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 13808)
	; 15 bytes
	tya
	ldy #128
	pha
	delay_n 95
	pla
	dey
	Jpl *-9
	tay
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 15 bytes
	tya
	ldy #238
	php
	delay_n 46
	plp
	dey
	Jne *-9
	tay
.elseif(n = 13810)
	; 15 bytes
	tya
	ldy #251
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
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
	; 14 bytes
	tya
	pha
	ldy #230
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 13813)
	; 16 bytes
	php
	tya
	pha
	ldy #113
	lda #$E9 ;hides 'sbc #$0A'
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
	delay_n 39
	lda #168
	jsr delay_a_25_clocks
	lda #(n-259)/256
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
	; 15 bytes
	tya
	ldy #0
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
.elseif(n = 13830)
	; 15 bytes
	tya
	ldy #35
	pha
	delay_n 383
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13831)
	; 16 bytes
	php
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13832)
	; 16 bytes
	php
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 15 bytes
	tya
	pha
	ldy #216 ;hides 'cld'
	delay_n 57
	dey
	Jne *-8
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #216
	delay_n 59
	dey
	Jne *-7
	pla
	tay
.elseif(n = 13837)
	; 15 bytes
	tya
	ldy #(n-5)/56
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
.elseif(n = 13838)
	; 15 bytes
	tya
	pha
	ldy #223
	delay_n 57
	dey
	Jne *-7
	pla
	tay
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
	; 9 bytes
	php
	lda $A5
	lda #54
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 13848 to 13867
.if(n = 13848 || n = 13850 || n = 13851 || n = 13853 || n = 13855 || n = 13858 || n = 13860 || n = 13862 || n = 13865)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13849)
	; 7 bytes
	php
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13852)
	; 9 bytes
	php
	lda $A5
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13854)
	; 10 bytes
	php
	nop
	lda $A5
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13856)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13857)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13859)
	; 11 bytes
	php
	pha
	lda $A5
	lda #54
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13861)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13863)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 13864)
	; 12 bytes
	php
	delay_n 20
	lda #54
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13866)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #54
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13869)
	; 12 bytes
	php
	delay_n 20
	lda #54
	jsr delay_256a_16_clocks
	plp
.elseif(n = 13870)
	; 12 bytes
	php
	delay_n 21
	lda #54
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
	delay_n 24
	lda #54
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
	delay_n 39
	lda #54
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 13888)
	; 11 bytes
	php
	delay_n 39
	lda #54
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
.elseif((n > 14051 && n < 14057))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14057 || n = 14058)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14059)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 2336
	Jcc *-11
.elseif(n = 14060)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14061)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #165
	jsr delay_a_25_clocks
	lda #(n-241)/256
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
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14072)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
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
	; 14 bytes
	tya
	ldy #210
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
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
	; 15 bytes
	tya
	ldy #204
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 14084)
	; 15 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 46
	dey
	Jne *-10
	tay
.elseif(n = 14085)
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 43
	pla
	dey
	Jne *-9
	tay
.elseif(n = 14086)
	; 15 bytes
	tya
	pha
	ldy #227
	delay_n 57
	dey
	Jne *-7
	pla
	tay
.elseif(n = 14087)
	; 16 bytes
	php
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
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
	; 16 bytes
	tya
	pha
	ldy #216 ;hides 'cld'
	delay_n 345
	iny
	Jne *-9
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #220
	delay_n 59
	dey
	Jne *-7
	pla
	tay
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
	delay_n 46
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
.if(n = 14102 || n = 14104 || n = 14106 || n = 14107 || n = 14109 || n = 14111 || n = 14114 || n = 14116 || n = 14118 || n = 14121)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14103)
	; 9 bytes
	php
	lda $A5
	lda #55
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14105)
	; 7 bytes
	php
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14108)
	; 9 bytes
	php
	lda $A5
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14110)
	; 10 bytes
	php
	nop
	lda $A5
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14112)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14113)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14115)
	; 11 bytes
	php
	pha
	lda $A5
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14117)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14119)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 14120)
	; 12 bytes
	php
	delay_n 20
	lda #55
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14122)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #55
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14125)
	; 12 bytes
	php
	delay_n 20
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14126)
	; 12 bytes
	php
	delay_n 21
	lda #55
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
	delay_n 24
	lda #55
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
	delay_n 39
	lda #55
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
	delay_n 39
	lda #55
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
; 14297 to 14335
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
.elseif((n > 14307 && n < 14313))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14313 || n = 14314)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14315)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14316)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	pha
	ldy #106 ;hides 'ror a'
	delay_n 128
	dey
	Jne *-8
	pla
	tay
.elseif(n = 14321)
	; 16 bytes
	php
	delay_n 24
	lda #165
	jsr delay_a_25_clocks
	lda #(n-241)/256
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14331)
	; 15 bytes
	tya
	ldy #247
	php
	delay_n 46
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
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
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
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 44
	pla
	dey
	Jne *-9
	tay
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
	; 14 bytes
	tya
	ldy #214
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
.elseif(n = 14345 || n = 14354)
	; 15 bytes
	delay_n (n-2)
	nop
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
	; 15 bytes
	tya
	ldy #(n-5)/71
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
.elseif(n = 14348)
	; 15 bytes
	tya
	pha
	ldy #224
	delay_n 59
	dey
	Jne *-7
	pla
	tay
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
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	tya
	pha
	ldy #239
	delay_n 55
	dey
	Jne *-6
	pla
	tay
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
	; 15 bytes
	tya
	ldy #208
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 14358 || n = 14360 || n = 14362 || n = 14363 || n = 14365 || n = 14367 || n = 14370 || n = 14372 || n = 14374)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14359)
	; 9 bytes
	php
	lda $A5
	lda #56
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14361)
	; 7 bytes
	php
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14364)
	; 9 bytes
	php
	lda $A5
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14366)
	; 10 bytes
	php
	nop
	lda $A5
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14369)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14371)
	; 11 bytes
	php
	pha
	lda $A5
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14373)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14375)
	; 11 bytes
	php
	pha
	delay_n (n-14)
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
	; 12 bytes
	php
	delay_n 20
	lda #56
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14377)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14378)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #56
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14381)
	; 12 bytes
	php
	delay_n 20
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14382)
	; 12 bytes
	php
	delay_n 21
	lda #56
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
	delay_n 24
	lda #56
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
	delay_n 39
	lda #56
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14400)
	; 11 bytes
	php
	delay_n 39
	lda #56
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
.elseif((n > 14563 && n < 14569))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14569 || n = 14570)
	; 15 bytes
	php
	pha
	nop
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
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14572)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14573)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 14576)
	; 16 bytes
	php
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 24
	lda #168
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14582)
	; 15 bytes
	tya
	pha
	ldy #235
	delay_n 57
	dey
	Jne *-7
	pla
	tay
.elseif(n = 14583)
	; 16 bytes
	php
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
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
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 14592)
	; 14 bytes
	tya
	pha
	ldy #243
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 14593)
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 14594)
	; 15 bytes
	delay_n (n-2)
	nop
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
.elseif((n > 14597 && n < 14601))
	; 15..16 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 14601)
	; 16 bytes
	tya
	ldy #178
	pha
	tya
	ldy #13
	dey
	Jne *-1
	tay
	pla
	dey
	Jne *-10
	tay
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
	; 15 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	delay_n 59
	pla
	iny
	Jne *-10
	tay
.elseif(n = 14604)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
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
	; 14 bytes
	lda #$2A ;hides 'rol a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
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
	; 15 bytes
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
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
	; 14 bytes
	tya
	ldy #218
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 14612)
	; 7 bytes
	php
	lda #(n-20)/256
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14613)
	; 15 bytes
	tya
	ldy #176
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
.elseif(n = 14614 || n = 14616 || n = 14618 || n = 14619 || n = 14621 || n = 14623 || n = 14626 || n = 14628 || n = 14630 || n = 14633)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14615)
	; 9 bytes
	php
	lda $A5
	lda #57
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14617)
	; 7 bytes
	php
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14620)
	; 9 bytes
	php
	lda $A5
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14622)
	; 10 bytes
	php
	nop
	lda $A5
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14625)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14627)
	; 11 bytes
	php
	pha
	lda $A5
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14629)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14631)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 14632)
	; 12 bytes
	php
	delay_n 20
	lda #57
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14634)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #57
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14637)
	; 12 bytes
	php
	delay_n 20
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14638)
	; 12 bytes
	php
	delay_n 21
	lda #57
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
	delay_n 24
	lda #57
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
	delay_n 39
	lda #57
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14656)
	; 11 bytes
	php
	delay_n 39
	lda #57
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
.elseif((n > 14819 && n < 14825))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14825 || n = 14826)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14827)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14828)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
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
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 14832)
	; 14 bytes
	tya
	pha
	ldy #247
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 14833)
	; 16 bytes
	php
	delay_n 24
	lda #165
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14834)
	; 15 bytes
	delay_n (n-2)
	nop
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 14840)
	; 15 bytes
	tya
	ldy #215
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 14841)
	; 16 bytes
	php
	delay_n 24
	lda #173
	jsr delay_a_25_clocks
	lda #(n-249)/256
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
	; 15 bytes
	tya
	ldy #(n-5)/71
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
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
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
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
	delay_n 46
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
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14856)
	; 16 bytes
	php
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 15 bytes
	tya
	pha
	ldy #24 ;hides 'clc'
	delay_n 57
	iny
	Jne *-8
	pla
	tay
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
	; 15 bytes
	tya
	ldy #179
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
.elseif(n = 14870 || n = 14872 || n = 14874 || n = 14875 || n = 14877 || n = 14879 || n = 14882)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 14871)
	; 9 bytes
	php
	lda $A5
	lda #58
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14873)
	; 7 bytes
	php
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14876)
	; 9 bytes
	php
	lda $A5
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14878)
	; 10 bytes
	php
	nop
	lda $A5
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14880)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14881)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883)
	; 11 bytes
	php
	pha
	lda $A5
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14884 || n = 14886 || n = 14889)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14885)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14887)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 14888)
	; 12 bytes
	php
	delay_n 20
	lda #58
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14890)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #58
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14893)
	; 12 bytes
	php
	delay_n 20
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 14894)
	; 12 bytes
	php
	delay_n 21
	lda #58
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
	delay_n 24
	lda #58
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
	delay_n 39
	lda #58
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 14912)
	; 11 bytes
	php
	delay_n 39
	lda #58
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
.elseif((n > 15075 && n < 15079))
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
.elseif(n = 15081 || n = 15082)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15083)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15084)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15085)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15088)
	; 16 bytes
	php
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15089)
	; 15 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	delay_n 383
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
	; 15 bytes
	tya
	ldy #141
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15093)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15096)
	; 16 bytes
	php
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15103)
	; 16 bytes
	php
	delay_n 39
	lda #164
	jsr delay_a_25_clocks
	lda #(n-255)/256
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
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15111)
	; 15 bytes
	tya
	ldy #182
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15112)
	; 15 bytes
	tya
	pha
	ldy #151
	delay_n 95
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	delay_n 55
	iny
	Jne *-8
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
	; 15 bytes
	tya
	pha
	ldy #236
	delay_n 59
	dey
	Jne *-7
	pla
	tay
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
	pha
	delay_n 645
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
.elseif(n = 15126 || n = 15128 || n = 15130 || n = 15131 || n = 15133 || n = 15135)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15127)
	; 9 bytes
	php
	lda $A5
	lda #59
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15129)
	; 7 bytes
	php
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15132)
	; 9 bytes
	php
	lda $A5
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15134)
	; 10 bytes
	php
	nop
	lda $A5
	lda #59
	jsr delay_256a_16_clocks
	plp
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
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15138 || n = 15140 || n = 15142 || n = 15145)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15139)
	; 11 bytes
	php
	pha
	lda $A5
	lda #59
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15141)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15143)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15144)
	; 12 bytes
	php
	delay_n 20
	lda #59
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15146)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #59
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15149)
	; 12 bytes
	php
	delay_n 20
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15150)
	; 12 bytes
	php
	delay_n 21
	lda #59
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
	delay_n 24
	lda #59
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
	delay_n 39
	lda #59
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15168)
	; 11 bytes
	php
	delay_n 39
	lda #59
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
.elseif((n > 15331 && n < 15337))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15337 || n = 15338)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15339)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15340)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15341)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	; 14 bytes
	tya
	ldy #229
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
.elseif(n = 15349)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	delay_n 655
	pla
	Jcs *-11
	plp
.elseif(n = 15350)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 15351)
	; 16 bytes
	php
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
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
	; 15 bytes
	tya
	ldy #185
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
.elseif(n = 15365)
	; 16 bytes
	php
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
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
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 14 bytes
	tya
	pha
	ldy #0
	delay_n 55
	dey
	Jne *-6
	pla
	tay
.elseif(n = 15373)
	; 16 bytes
	tya
	ldy #33
	pha
	delay_n 440
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 15374 || n = 15382 || n = 15384 || n = 15386 || n = 15387 || n = 15389)
	; 8..15 bytes
	delay_n (n-2)
	nop
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
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 15383)
	; 9 bytes
	php
	lda $A5
	lda #60
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15385)
	; 7 bytes
	php
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15388)
	; 9 bytes
	php
	lda $A5
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15390)
	; 10 bytes
	php
	nop
	lda $A5
	lda #60
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15394 || n = 15396 || n = 15398 || n = 15401)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15392)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15393)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15395)
	; 11 bytes
	php
	pha
	lda $A5
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15397)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15399)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15400)
	; 12 bytes
	php
	delay_n 20
	lda #60
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15402)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #60
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15405)
	; 12 bytes
	php
	delay_n 20
	lda #60
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15406)
	; 12 bytes
	php
	delay_n 21
	lda #60
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
	delay_n 24
	lda #60
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
	delay_n 39
	lda #60
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15424)
	; 11 bytes
	php
	delay_n 39
	lda #60
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
.elseif((n > 15587 && n < 15593))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15593 || n = 15594)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15595)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 2592
	Jcc *-11
.elseif(n = 15596)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15597)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15598)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15599)
	; 15 bytes
	tya
	ldy #226
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15600)
	; 16 bytes
	php
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	delay_n 24
	lda #167
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	delay_n 24
	lda #168
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15605)
	; 15 bytes
	tya
	ldy #52
	pha
	delay_n 288
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
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
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
	; 15 bytes
	tya
	ldy #188
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
.elseif(n = 15610)
	; 15 bytes
	tya
	pha
	ldy #56 ;hides 'sec'
	delay_n 71
	iny
	Jne *-8
	pla
	tay
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
	; 15 bytes
	tya
	pha
	ldy #156
	delay_n 95
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #164
	jsr delay_a_25_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15616)
	; 14 bytes
	tya
	ldy #233
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 15624)
	; 16 bytes
	php
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 15 bytes
	tya
	ldy #146
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
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
.elseif(n = 15638 || n = 15640 || n = 15642 || n = 15643)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15639)
	; 9 bytes
	php
	lda $A5
	lda #61
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15641)
	; 7 bytes
	php
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15644)
	; 9 bytes
	php
	lda $A5
	lda #61
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 15645 to 15664
.if(n = 15645 || n = 15647 || n = 15650 || n = 15652 || n = 15654 || n = 15657)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15646)
	; 10 bytes
	php
	nop
	lda $A5
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15648)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15649)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15651)
	; 11 bytes
	php
	pha
	lda $A5
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15653)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15655)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15656)
	; 12 bytes
	php
	delay_n 20
	lda #61
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15658)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #61
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15661)
	; 12 bytes
	php
	delay_n 20
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15662)
	; 12 bytes
	php
	delay_n 21
	lda #61
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
	delay_n 24
	lda #61
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
	delay_n 39
	lda #61
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15680)
	; 11 bytes
	php
	delay_n 39
	lda #61
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
.elseif((n > 15843 && n < 15849))
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
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	php
	pha
	lda #232
	jsr delay_a_25_clocks
	pla
	plp
	Jmi *-10
	tay
.elseif(n = 15850)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15851)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15852)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15853)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	ldy #191
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
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
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	delay_n 39
	lda #164
	jsr delay_a_25_clocks
	lda #(n-255)/256
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
	delay_n 39
	lda #166
	jsr delay_a_25_clocks
	lda #(n-257)/256
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
	; 15 bytes
	tya
	ldy #230
	pha
	delay_n 57
	pla
	dey
	Jne *-9
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
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
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
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 15 bytes
	tya
	pha
	ldy #248 ;hides 'sed'
	delay_n 57
	dey
	Jne *-8
	pla
	tay
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
	; 14 bytes
	tya
	ldy #237
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
.elseif(n = 15886 || n = 15894 || n = 15896 || n = 15898)
	; 8..15 bytes
	delay_n (n-2)
	nop
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
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 15895)
	; 9 bytes
	php
	lda $A5
	lda #62
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15897)
	; 7 bytes
	php
	lda #62
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899 || n = 15901 || n = 15903 || n = 15906 || n = 15908 || n = 15910 || n = 15913)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15900)
	; 9 bytes
	php
	lda $A5
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15902)
	; 10 bytes
	php
	nop
	lda $A5
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15904)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15905)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15907)
	; 11 bytes
	php
	pha
	lda $A5
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15909)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15911)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 15912)
	; 12 bytes
	php
	delay_n 20
	lda #62
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15914)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #62
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15917)
	; 12 bytes
	php
	delay_n 20
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 15918)
	; 12 bytes
	php
	delay_n 21
	lda #62
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
	delay_n 24
	lda #62
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
	delay_n 39
	lda #62
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 15936)
	; 11 bytes
	php
	delay_n 39
	lda #62
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
.elseif((n > 16099 && n < 16105))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16105 || n = 16106)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16107)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16108)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16109)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	pha
	ldy #161
	delay_n 95
	dey
	Jne *-7
	pla
	tay
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	php
	delay_n 24
	lda #165
	jsr delay_a_25_clocks
	lda #(n-241)/256
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
	ldy #244
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16118)
	; 16 bytes
	php
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
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
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
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
	; 15 bytes
	tya
	pha
	ldy #252
	delay_n 59
	dey
	Jne *-7
	pla
	tay
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
	; 14 bytes
	tya
	ldy #234 ;hides 'nop'
	pha
	delay_n 55
	pla
	dey
	Jne *-9
	tay
.elseif(n = 16151)
	; 9 bytes
	php
	lda $A5
	lda #63
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 16152 to 16171
.if(n = 16152 || n = 16154 || n = 16155 || n = 16157 || n = 16159 || n = 16162 || n = 16164 || n = 16166 || n = 16169)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16153)
	; 7 bytes
	php
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16156)
	; 9 bytes
	php
	lda $A5
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16158)
	; 10 bytes
	php
	nop
	lda $A5
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16160)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16161)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16163)
	; 11 bytes
	php
	pha
	lda $A5
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16165)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16167)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 16168)
	; 12 bytes
	php
	delay_n 20
	lda #63
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16170)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #63
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16173)
	; 12 bytes
	php
	delay_n 20
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16174)
	; 12 bytes
	php
	delay_n 21
	lda #63
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
	delay_n 24
	lda #63
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
	delay_n 39
	lda #63
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16192)
	; 11 bytes
	php
	delay_n 39
	lda #63
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
.elseif((n > 16355 && n < 16361))
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 16363)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16364)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16365)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	ldy #153
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
.elseif(n = 16377)
	; 16 bytes
	php
	delay_n 24
	lda #173
	jsr delay_a_25_clocks
	lda #(n-249)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16378)
	; 15 bytes
	tya
	pha
	ldy #248 ;hides 'sed'
	delay_n 59
	dey
	Jne *-8
	pla
	tay
.elseif(n = 16379)
	; 16 bytes
	php
	delay_n 39
	lda #160
	jsr delay_a_25_clocks
	lda #(n-251)/256
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
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
	pha
	delay_n 700
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
	; 15 bytes
	tya
	pha
	ldy #42 ;hides 'rol a'
	delay_n 383
	dey
	Jne *-8
	pla
	tay
.elseif(n = 16391)
	; 16 bytes
	php
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
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
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 59
	dey
	Jne *-7
	pla
	tay
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
.if(n = 16407)
	; 9 bytes
	php
	lda $A5
	lda #64
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16408 || n = 16410 || n = 16411 || n = 16413 || n = 16415 || n = 16418 || n = 16420 || n = 16422 || n = 16425)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16409)
	; 7 bytes
	php
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16412)
	; 9 bytes
	php
	lda $A5
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16414)
	; 10 bytes
	php
	nop
	lda $A5
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16416)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16417)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16419)
	; 11 bytes
	php
	pha
	lda $A5
	lda #64
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16421)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16423)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 16424)
	; 12 bytes
	php
	delay_n 20
	lda #64
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16426)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #64
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16429)
	; 12 bytes
	php
	delay_n 20
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16430)
	; 12 bytes
	php
	delay_n 21
	lda #64
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
	delay_n 24
	lda #64
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
	delay_n 39
	lda #64
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16448)
	; 11 bytes
	php
	delay_n 39
	lda #64
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
.elseif((n > 16611 && n < 16617))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16617 || n = 16618)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16619)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16620)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 16621 to 16640
.if(n = 16621)
	; 14 bytes
	tya
	ldy #248
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
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
	delay_n 24
	lda #165
	jsr delay_a_25_clocks
	lda #(n-241)/256
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
	pha
	delay_n (n-7)
	pla
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
	; 15 bytes
	tya
	ldy #241
	pha
	delay_n 57
	pla
	dey
	Jne *-9
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
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 16640)
	; 16 bytes
	php
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
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
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 16642)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 16643)
	; 15 bytes
	sec
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
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
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	lda #$2A ;hides 'rol a'
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
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
	ldy #157
	lda #74
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	plp
.elseif(n = 16662 || n = 16664 || n = 16666 || n = 16667 || n = 16669 || n = 16671 || n = 16674 || n = 16676 || n = 16678)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16663)
	; 9 bytes
	php
	lda $A5
	lda #65
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16665)
	; 7 bytes
	php
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16668)
	; 9 bytes
	php
	lda $A5
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16670)
	; 10 bytes
	php
	nop
	lda $A5
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16673)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16675)
	; 11 bytes
	php
	pha
	lda $A5
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16677)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16679)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680)
	; 12 bytes
	php
	delay_n 20
	lda #65
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16681)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16682)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #65
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16685)
	; 12 bytes
	php
	delay_n 20
	lda #65
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16686)
	; 12 bytes
	php
	delay_n 21
	lda #65
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
	delay_n 24
	lda #65
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
	delay_n 39
	lda #65
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16704)
	; 11 bytes
	php
	delay_n 39
	lda #65
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
.elseif((n > 16867 && n < 16873))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16873 || n = 16874)
	; 15 bytes
	php
	pha
	nop
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
.if(n = 16876)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16877)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
.elseif(n = 16885)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16889)
	; 14 bytes
	tya
	ldy #252
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
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
	delay_n 39
	lda #164
	jsr delay_a_25_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16896)
	; 16 bytes
	pha
	delay_n (n-7)
	pla
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
	delay_n 39
	lda #167
	jsr delay_a_25_clocks
	lda #(n-258)/256
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
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16901)
	; 16 bytes
	php
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16902)
	; 16 bytes
	php
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
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
	; 15 bytes
	tya
	ldy #245
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
.elseif(n = 16911)
	; 15 bytes
	tya
	ldy #158
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
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
.elseif(n = 16918 || n = 16920 || n = 16922 || n = 16923 || n = 16925 || n = 16927 || n = 16930 || n = 16932 || n = 16934 || n = 16937)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16919)
	; 9 bytes
	php
	lda $A5
	lda #66
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16921)
	; 7 bytes
	php
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16924)
	; 9 bytes
	php
	lda $A5
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16926)
	; 10 bytes
	php
	nop
	lda $A5
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16928)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16929)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16931)
	; 11 bytes
	php
	pha
	lda $A5
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16933)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16935)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 16936)
	; 12 bytes
	php
	delay_n 20
	lda #66
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16938)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #66
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16941)
	; 12 bytes
	php
	delay_n 20
	lda #66
	jsr delay_256a_16_clocks
	plp
.elseif(n = 16942)
	; 12 bytes
	php
	delay_n 21
	lda #66
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
	delay_n 24
	lda #66
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
	delay_n 39
	lda #66
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 16960)
	; 11 bytes
	php
	delay_n 39
	lda #66
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
.elseif((n > 17123 && n < 17129))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17129 || n = 17130)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17131)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 2848
	Jcc *-11
.elseif(n = 17132)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17133)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17142)
	; 16 bytes
	php
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17143)
	; 16 bytes
	php
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
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
	delay_n 24
	lda #173
	jsr delay_a_25_clocks
	lda #(n-249)/256
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
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
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
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17153)
	; 16 bytes
	php
	tya
	ldy #61
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
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17157)
	; 14 bytes
	tya
	ldy #0
	pha
	delay_n 55
	pla
	dey
	Jne *-8
	tay
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
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 17160)
	; 15 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 296
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
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 17174 || n = 17176 || n = 17178 || n = 17179 || n = 17181 || n = 17183 || n = 17186)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17175)
	; 9 bytes
	php
	lda $A5
	lda #67
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17177)
	; 7 bytes
	php
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17180)
	; 9 bytes
	php
	lda $A5
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17182)
	; 10 bytes
	php
	nop
	lda $A5
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17184)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17185)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17187)
	; 11 bytes
	php
	pha
	lda $A5
	lda #67
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17190 || n = 17193)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17189)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17191)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17192)
	; 12 bytes
	php
	delay_n 20
	lda #67
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17194)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #67
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17197)
	; 12 bytes
	php
	delay_n 20
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17198)
	; 12 bytes
	php
	delay_n 21
	lda #67
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
	delay_n 24
	lda #67
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
	delay_n 39
	lda #67
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17216)
	; 11 bytes
	php
	delay_n 39
	lda #67
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
.elseif((n > 17379 && n < 17383))
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
.elseif(n = 17385 || n = 17386)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17387)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17388)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17389)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	ldy #252
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
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
	; 15 bytes
	tya
	ldy #(n-5)/71
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
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
	pha
	delay_n 288
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
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
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
	; 15 bytes
	tya
	pha
	ldy #174
	delay_n 95
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17416)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
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
.elseif(n = 17430 || n = 17432 || n = 17434 || n = 17435 || n = 17437 || n = 17439)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17431)
	; 9 bytes
	php
	lda $A5
	lda #68
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17433)
	; 7 bytes
	php
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17436)
	; 9 bytes
	php
	lda $A5
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17438)
	; 10 bytes
	php
	nop
	lda $A5
	lda #68
	jsr delay_256a_16_clocks
	plp
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
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17442 || n = 17444 || n = 17446 || n = 17449)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17443)
	; 11 bytes
	php
	pha
	lda $A5
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17445)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17447)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17448)
	; 12 bytes
	php
	delay_n 20
	lda #68
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17450)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #68
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17453)
	; 12 bytes
	php
	delay_n 20
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17454)
	; 12 bytes
	php
	delay_n 21
	lda #68
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
	delay_n 24
	lda #68
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
	delay_n 39
	lda #68
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17472)
	; 11 bytes
	php
	delay_n 39
	lda #68
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
.elseif((n > 17635 && n < 17641))
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
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17642)
	; 15 bytes
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 227
	Jmi *-8
	pla
	tax
.elseif(n = 17643)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17644)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17645)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17649)
	; 16 bytes
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	delay_n 755
	pla
	Jcs *-11
	plp
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17654)
	; 16 bytes
	php
	delay_n 24
	lda #170
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17655)
	; 16 bytes
	php
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17656)
	; 16 bytes
	php
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	; 15 bytes
	tya
	ldy #165
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17663)
	; 16 bytes
	php
	delay_n 39
	lda #164
	jsr delay_a_25_clocks
	lda #(n-255)/256
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
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 57
	pla
	dey
	Jne *-9
	tay
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
.elseif(n = 17686 || n = 17688 || n = 17690 || n = 17691 || n = 17693 || n = 17695)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17687)
	; 9 bytes
	php
	lda $A5
	lda #69
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17689)
	; 7 bytes
	php
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17692)
	; 9 bytes
	php
	lda $A5
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17694)
	; 10 bytes
	php
	nop
	lda $A5
	lda #69
	jsr delay_256a_16_clocks
	plp
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
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17698 || n = 17700 || n = 17702 || n = 17705)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699)
	; 11 bytes
	php
	pha
	lda $A5
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17701)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17703)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17704)
	; 12 bytes
	php
	delay_n 20
	lda #69
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17706)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #69
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17709)
	; 12 bytes
	php
	delay_n 20
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17710)
	; 12 bytes
	php
	delay_n 21
	lda #69
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
	delay_n 24
	lda #69
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
	delay_n 39
	lda #69
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17728)
	; 11 bytes
	php
	delay_n 39
	lda #69
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
.elseif((n > 17891 && n < 17897))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17897 || n = 17898)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17899)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17900)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17901)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	; 15 bytes
	tya
	pha
	ldy #179
	delay_n 95
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17915)
	; 16 bytes
	php
	delay_n 39
	lda #160
	jsr delay_a_25_clocks
	lda #(n-251)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17916)
	; 16 bytes
	php
	delay_n 39
	lda #161
	jsr delay_a_25_clocks
	lda #(n-252)/256
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
	delay_n 39
	lda #166
	jsr delay_a_25_clocks
	lda #(n-257)/256
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
	delay_n 39
	lda #168
	jsr delay_a_25_clocks
	lda #(n-259)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17924)
	; 16 bytes
	php
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
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
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 15 bytes
	tya
	ldy #216
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
.elseif(n = 17942 || n = 17944 || n = 17946 || n = 17947)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 17943)
	; 9 bytes
	php
	lda $A5
	lda #70
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17945)
	; 7 bytes
	php
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17948)
	; 9 bytes
	php
	lda $A5
	lda #70
	jsr delay_256a_16_clocks
	plp
.endif
.else
; 17949 to 17968
.if(n = 17949 || n = 17951 || n = 17954 || n = 17956 || n = 17958 || n = 17961)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17950)
	; 10 bytes
	php
	nop
	lda $A5
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17952)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17953)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17955)
	; 11 bytes
	php
	pha
	lda $A5
	lda #70
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17957)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17959)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 17960)
	; 12 bytes
	php
	delay_n 20
	lda #70
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17962)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #70
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17965)
	; 12 bytes
	php
	delay_n 20
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 17966)
	; 12 bytes
	php
	delay_n 21
	lda #70
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
	delay_n 24
	lda #70
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
	delay_n 39
	lda #70
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 17984)
	; 11 bytes
	php
	delay_n 39
	lda #70
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
.elseif((n > 18147 && n < 18153))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18153 || n = 18154)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18155)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18156)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18157)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
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
	pha
	delay_n 383
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 18176)
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
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
	; 15 bytes
	tya
	ldy #0
	pha
	delay_n 59
	pla
	dey
	Jne *-9
	tay
.elseif(n = 18182)
	; 15 bytes
	tya
	ldy #219
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
	pha
	delay_n 375
	pla
	dey
	Jpl *-10
	tay
.elseif(n = 18195)
	; 15 bytes
	tya
	ldy #170
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
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
.elseif(n = 18198 || n = 18200 || n = 18202 || n = 18203)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 18199)
	; 9 bytes
	php
	lda $A5
	lda #71
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18201)
	; 7 bytes
	php
	lda #71
	jsr delay_256a_16_clocks
	plp
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204)
	; 9 bytes
	php
	lda $A5
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18205 || n = 18207 || n = 18210 || n = 18212 || n = 18214 || n = 18217)
	; 9..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18206)
	; 10 bytes
	php
	nop
	lda $A5
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18208)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18209)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18211)
	; 11 bytes
	php
	pha
	lda $A5
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18213)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18215)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 18216)
	; 12 bytes
	php
	delay_n 20
	lda #71
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18218)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #71
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18221)
	; 12 bytes
	php
	delay_n 20
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18222)
	; 12 bytes
	php
	delay_n 21
	lda #71
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
	delay_n 24
	lda #71
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
	delay_n 39
	lda #71
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18240)
	; 11 bytes
	php
	delay_n 39
	lda #71
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
.elseif((n > 18403 && n < 18409))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18409 || n = 18410)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18411)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18412)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18413)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #172
	jsr delay_a_25_clocks
	lda #(n-248)/256
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
	delay_n 39
	lda #161
	jsr delay_a_25_clocks
	lda #(n-252)/256
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
	delay_n 39
	lda #163
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18431)
	; 15 bytes
	tya
	ldy #222
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
.elseif(n = 18432)
	; 16 bytes
	php
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
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
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
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
.elseif(n = 18454 || n = 18456)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 18455)
	; 9 bytes
	php
	lda $A5
	lda #72
	jsr delay_256a_11_clocks_
	plp
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 7 bytes
	php
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18458 || n = 18459 || n = 18461 || n = 18463 || n = 18466 || n = 18468 || n = 18470 || n = 18473)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18460)
	; 9 bytes
	php
	lda $A5
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18462)
	; 10 bytes
	php
	nop
	lda $A5
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18464)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18465)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18467)
	; 11 bytes
	php
	pha
	lda $A5
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18469)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18471)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 18472)
	; 12 bytes
	php
	delay_n 20
	lda #72
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18474)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #72
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 12 bytes
	php
	delay_n 20
	lda #72
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18478)
	; 12 bytes
	php
	delay_n 21
	lda #72
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
	delay_n 24
	lda #72
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
	delay_n 39
	lda #72
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18496)
	; 11 bytes
	php
	delay_n 39
	lda #72
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
.elseif((n > 18659 && n < 18665))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18665 || n = 18666)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18667)
	; 15 bytes
	clc
	lda #$0A ;hides 'asl a'
	nop
	delay_n 3104
	Jcc *-11
.elseif(n = 18668)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18669)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18670 || n = 18671)
	; 16 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.endif
.else
.if((n >= 18672) && (n < 18711))
.if((n >= 18672) && (n < 18691))
; 18672 to 18690
.if(n = 18672)
	; 16 bytes
	php
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #161
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	plp
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
	delay_n 24
	lda #168
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	; 15 bytes
	tya
	ldy #225
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #159
	jsr delay_a_25_clocks
	lda #(n-250)/256
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
	; 13 bytes
	lda #$4A ;hides 'lsr a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
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
	; 14 bytes
	clc
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
.elseif(n = 18690)
	; 15 bytes
	delay_n (n-3)
	lda $A5
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 15 bytes
	sec
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
	Jne *-10
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
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18694)
	; 15 bytes
	pha
	delay_n (n-7)
	pla
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
	; 14 bytes
	lda #$2A ;hides 'rol a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
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
	; 15 bytes
	sec
	lda #$0A ;hides 'asl a'
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
	clc
	Jne *-11
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
.if(n = 18711)
	; 9 bytes
	php
	lda $A5
	lda #73
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18712 || n = 18714 || n = 18715 || n = 18717 || n = 18719 || n = 18722 || n = 18724 || n = 18726 || n = 18729)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18713)
	; 7 bytes
	php
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18716)
	; 9 bytes
	php
	lda $A5
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18718)
	; 10 bytes
	php
	nop
	lda $A5
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18720)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18721)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18723)
	; 11 bytes
	php
	pha
	lda $A5
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18725)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18727)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 18728)
	; 12 bytes
	php
	delay_n 20
	lda #73
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18730)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #73
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18733)
	; 12 bytes
	php
	delay_n 20
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18734)
	; 12 bytes
	php
	delay_n 21
	lda #73
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
	delay_n 24
	lda #73
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
	delay_n 39
	lda #73
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
	delay_n 39
	lda #73
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
.elseif((n > 18915 && n < 18921))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18921 || n = 18922)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18923)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18924)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18925)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	; 15 bytes
	tya
	ldy #228
	pha
	delay_n 71
	pla
	dey
	Jne *-9
	tay
.elseif(n = 18930)
	; 16 bytes
	php
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
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
	; 15 bytes
	tya
	pha
	ldy #(n-12)/76
	delay_n 71
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
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
	; 15 bytes
	tya
	ldy #177
	pha
	delay_n 95
	pla
	dey
	Jne *-9
	tay
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
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
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
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	pha
	delay_n 383
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 18966 || n = 18968 || n = 18970 || n = 18971 || n = 18973 || n = 18975 || n = 18978 || n = 18980 || n = 18982)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18967)
	; 9 bytes
	php
	lda $A5
	lda #74
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18969)
	; 7 bytes
	php
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18972)
	; 9 bytes
	php
	lda $A5
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18974)
	; 10 bytes
	php
	nop
	lda $A5
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18976)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18977)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18979)
	; 11 bytes
	php
	pha
	lda $A5
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18981)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18983)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 18984)
	; 12 bytes
	php
	delay_n 20
	lda #74
	jsr delay_256a_11_clocks_
	plp
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18986)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #74
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 18989)
	; 12 bytes
	php
	delay_n 20
	lda #74
	jsr delay_256a_16_clocks
	plp
.elseif(n = 18990)
	; 12 bytes
	php
	delay_n 21
	lda #74
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
	delay_n 24
	lda #74
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
	delay_n 39
	lda #74
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19008)
	; 11 bytes
	php
	delay_n 39
	lda #74
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
.elseif((n > 19171 && n < 19177))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19177 || n = 19178)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19179)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	plp
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19181)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 24
	lda #166
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19187)
	; 16 bytes
	php
	delay_n 24
	lda #167
	jsr delay_a_25_clocks
	lda #(n-243)/256
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
	delay_n 24
	lda #171
	jsr delay_a_25_clocks
	lda #(n-247)/256
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
	delay_n 24
	lda #173
	jsr delay_a_25_clocks
	lda #(n-249)/256
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
	; 15 bytes
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 71
	iny
	Jne *-8
	pla
	tay
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
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19205)
	; 15 bytes
	tya
	ldy #63
	pha
	delay_n 288
	pla
	dey
	Jpl *-9
	tay
.elseif(n = 19206)
	; 16 bytes
	php
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
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
	delay_n 39
	lda #173
	jsr delay_a_25_clocks
	lda #(n-264)/256
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
	; 15 bytes
	tya
	pha
	ldy #192
	delay_n 95
	dey
	Jne *-7
	pla
	tay
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
.elseif(n = 19222 || n = 19224 || n = 19226 || n = 19227 || n = 19229 || n = 19231 || n = 19234 || n = 19236 || n = 19238 || n = 19241)
	; 8..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19223)
	; 9 bytes
	php
	lda $A5
	lda #75
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19225)
	; 7 bytes
	php
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19228)
	; 9 bytes
	php
	lda $A5
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19230)
	; 10 bytes
	php
	nop
	lda $A5
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19232)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19233)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19235)
	; 11 bytes
	php
	pha
	lda $A5
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19237)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19239)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 19240)
	; 12 bytes
	php
	delay_n 20
	lda #75
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19242)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #75
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19245)
	; 12 bytes
	php
	delay_n 20
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19246)
	; 12 bytes
	php
	delay_n 21
	lda #75
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
	delay_n 24
	lda #75
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
	delay_n 39
	lda #75
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19264)
	; 11 bytes
	php
	delay_n 39
	lda #75
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
.elseif((n > 19427 && n < 19433))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19433 || n = 19434)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19435)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19436)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19437)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 24
	lda #169
	jsr delay_a_25_clocks
	lda #(n-245)/256
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
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19462)
	; 16 bytes
	php
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19463)
	; 16 bytes
	php
	delay_n 39
	lda #172
	jsr delay_a_25_clocks
	lda #(n-263)/256
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
	; 15 bytes
	tya
	pha
	ldy #0
	delay_n 71
	dey
	Jne *-7
	pla
	tay
.elseif(n = 19469)
	; 17 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 19478 || n = 19480 || n = 19482 || n = 19483 || n = 19485 || n = 19487 || n = 19490 || n = 19492)
	; 8..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19479)
	; 9 bytes
	php
	lda $A5
	lda #76
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19481)
	; 7 bytes
	php
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19484)
	; 9 bytes
	php
	lda $A5
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19486)
	; 10 bytes
	php
	nop
	lda $A5
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19488)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19489)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19491)
	; 11 bytes
	php
	pha
	lda $A5
	lda #76
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19494 || n = 19497)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19495)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 19496)
	; 12 bytes
	php
	delay_n 20
	lda #76
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19498)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #76
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19501)
	; 12 bytes
	php
	delay_n 20
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19502)
	; 12 bytes
	php
	delay_n 21
	lda #76
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
	delay_n 24
	lda #76
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
	delay_n 39
	lda #76
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19520)
	; 11 bytes
	php
	delay_n 39
	lda #76
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
.elseif((n > 19683 && n < 19688))
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
.elseif(n = 19689 || n = 19690)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19691)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19692)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19693)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 39
	lda #161
	jsr delay_a_25_clocks
	lda #(n-252)/256
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
	; 15 bytes
	tya
	pha
	ldy #197
	delay_n 95
	dey
	Jne *-7
	pla
	tay
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
	delay_n 39
	lda #168
	jsr delay_a_25_clocks
	lda #(n-259)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19716)
	; 16 bytes
	php
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
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
	pha
	delay_n 845
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
	; 15 bytes
	tya
	ldy #24 ;hides 'clc'
	pha
	delay_n 71
	pla
	iny
	Jne *-10
	tay
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
.if((n >= 19727) && (n < 19746))
; 19727 to 19745
.if(n = 19727)
	; 16 bytes
	tya
	ldy #37
	delay_n 514
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
.elseif(n = 19734 || n = 19736 || n = 19738 || n = 19739 || n = 19741 || n = 19743)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19735)
	; 9 bytes
	php
	lda $A5
	lda #77
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19737)
	; 7 bytes
	php
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19740)
	; 9 bytes
	php
	lda $A5
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19742)
	; 10 bytes
	php
	nop
	lda $A5
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19744)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19745)
	; 11 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 19746 to 19765
.if(n = 19746 || n = 19748 || n = 19750 || n = 19753)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19747)
	; 11 bytes
	php
	pha
	lda $A5
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19749)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19751)
	; 11 bytes
	php
	pha
	delay_n (n-14)
	pla
	plp
.elseif(n = 19752)
	; 12 bytes
	php
	delay_n 20
	lda #77
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19754)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	delay_n 24
	lda #77
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19757)
	; 12 bytes
	php
	delay_n 20
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19758)
	; 12 bytes
	php
	delay_n 21
	lda #77
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
	delay_n 24
	lda #77
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
	delay_n 39
	lda #77
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19776)
	; 11 bytes
	php
	delay_n 39
	lda #77
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
.elseif((n > 19939 && n < 19945))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19945 || n = 19946)
	; 15 bytes
	php
	pha
	nop
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19947)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19948)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #172
	jsr delay_a_25_clocks
	lda #(n-236)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19949)
	; 15 bytes
	php
	jsr @nearby_rts
	lda #173
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 24
	lda #164
	jsr delay_a_25_clocks
	lda #(n-240)/256
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
	delay_n 39
	lda #162
	jsr delay_a_25_clocks
	lda #(n-253)/256
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
	delay_n 39
	lda #165
	jsr delay_a_25_clocks
	lda #n/256-1
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
	delay_n 39
	lda #169
	jsr delay_a_25_clocks
	lda #(n-260)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19973)
	; 16 bytes
	php
	delay_n 39
	lda #170
	jsr delay_a_25_clocks
	lda #(n-261)/256
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19974)
	; 16 bytes
	php
	delay_n 39
	lda #171
	jsr delay_a_25_clocks
	lda #(n-262)/256
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
	ldy #165
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
	pha
	delay_n 365
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
.elseif(n = 19990 || n = 19992 || n = 19994 || n = 19995 || n = 19997 || n = 19999)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 19991)
	; 9 bytes
	php
	lda $A5
	lda #78
	jsr delay_256a_11_clocks_
	plp
.elseif(n = 19993)
	; 7 bytes
	php
	lda #78
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19996)
	; 9 bytes
	php
	lda $A5
	lda #78
	jsr delay_256a_16_clocks
	plp
.elseif(n = 19998)
	; 10 bytes
	php
	nop
	lda $A5
	lda #78
	jsr delay_256a_16_clocks
	plp
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
