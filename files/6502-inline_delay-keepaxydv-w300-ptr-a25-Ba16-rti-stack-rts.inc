; delay_n, delays n cycles with small code
; Will clobber S, Z+N, and C
; Will preserve A, X, Y, D, V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; Uses JMP (unrelocatable code).
;
; WARNING: May write garbage into $300-$3FF.
;
; Requires a dummy interrupt handler that does nothing but RTI.
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
; -12 bytes or less when n <= 1356
; -13 bytes or less when n <= 2647
; -14 bytes or less when n <= 4842
; -15 bytes or less when n <= 5880
; -16 bytes or less when n <= 6672
; -17 bytes or less when n <= 9239
; -18 bytes or less when n <= 19227
; -19 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.2, max=6
; - When 31 <= n <= 50: min=6, avg=7.2, max=9
; - When 51 <= n <= 75: min=7, avg=8.6, max=9
; - When 76 <= n <= 100: min=8, avg=8.9, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.1, max=11
; - When 501 <= n <= 750: min=8, avg=10.1, max=11
; - When 751 <= n <= 1000: min=9, avg=10.1, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.9, max=13
; - When 2001 <= n <= 3000: min=9, avg=12.0, max=14
; - When 3001 <= n <= 5000: min=9, avg=12.8, max=15
; - When 5001 <= n <= 7500: min=9, avg=13.7, max=17
; - When 7501 <= n <= 10000: min=9, avg=14.0, max=18
; - When 10001 <= n <= 20000: min=9, avg=14.3, max=19
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
; 1 to 39
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 15 || n = 17 || n = 18 || n = 21 || n = 28 || n = 30)
	; 2..6 bytes
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
.elseif((n > 8 && n < 12) || n = 20 || n = 22 || n = 33 || n = 35 || n = 37)
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
.elseif(n = 16 || n = 19 || n = 23 || n = 26 || n = 29 || n = 32 || n = 36 || n = 39)
	; 3..7 bytes
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif(n = 34)
	; 7 bytes
	pha
	delay_n (n-3)
.elseif(n = 38)
	; 7 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
.endif
.else
; 40 to 78
.if(n = 40)
	; 8 bytes
	pha
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
.elseif(n = 41)
	; 7 bytes
	pha
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	pla
.elseif(n = 42 || n = 45)
	; 7..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 43)
	; 8 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	pla
.elseif(n = 44 || n = 59)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 46)
	; 6 bytes
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
.elseif(n = 47 || (n > 53 && n < 59) || n = 61 || n = 63 || (n > 66 && n < 78))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 48)
	; 7 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
.elseif(n = 49 || n = 65)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 50)
	; 8 bytes
	pha
	sec
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
.elseif(n = 51)
	; 8 bytes
	pha
	lda #$0A ;hides 'asl a'
	delay_n 4
	Jpl *-3
	pla
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk
	.endrepeat
.elseif(n = 53)
	; 8 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 60)
	; 8 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	pla
.elseif(n = 62)
	; 7 bytes
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
.elseif(n = 64)
	; 8 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	pla
.elseif(n = 66)
	; 8 bytes
	pha
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	pla
.elseif(n = 78)
	; 8 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 4
	Jne *-3
	pla
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 96) || (n > 96 && n < 102) || (n > 102 && n < 135) || (n > 135 && n < 150) || (n > 150 && n < 157))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 96)
	; 8 bytes
	pha
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	pla
.elseif(n = 102)
	; 8 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	pla
.elseif(n = 135)
	; 8 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	pla
.elseif(n = 150)
	; 8 bytes
	pha
	lda #$4A ;hides 'lsr a'
	brk
	Jne *-3
	pla
.endif
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
.elseif(n = 315 || n = 325 || n = 330 || n = 335 || n = 340 || n = 345 || n = 350 || n = 360 || n = 365 || n = 370 || n = 375 || n = 380 || n = 385 || n = 393 || n = 395 || n = 400 || n = 405 || n = 410 || n = 415 || n = 420 || n = 430 || n = 435 || n = 440 || n = 445 || n = 450 || n = 455 || n = 465)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 336 || n = 371 || n = 441)
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
.elseif(n = 388)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
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
.elseif(n = 396)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 403)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 406)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 409)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
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
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 486 || n = 489 || n = 493 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 518 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 541 || n = 543 || n = 546 || n = 548 || n = 549 || n = 553 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 573 || n = 574 || n = 581 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 623 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 470 || (n > 479 && n < 491) || (n > 499 && n < 526) || (n > 534 && n < 561) || (n > 569 && n < 596) || (n > 604 && n < 626)))
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
.elseif(n = 473 || n = 503 || n = 513 || n = 533 || n = 538 || n = 568 || n = 578 || n = 583 || n = 598 || n = 603 || n = 608 || n = 613)
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
.elseif(n = 475)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif(n = 483)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 488 || n = 495 || n = 516 || n = 565 || n = 579 || n = 586 || n = 600)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 491)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
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
	cmp $C5
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
.elseif(n = 539)
	; 9 bytes
	php
	pha
	lda #2
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 544)
	; 9 bytes
	php
	pha
	lda #2
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 563)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 571)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
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
.elseif(n = 588 || n = 606)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
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
.elseif(n = 630 || n = 640 || n = 645 || n = 650 || n = 655 || n = 660 || n = 665 || n = 675 || n = 680 || n = 685 || n = 690 || n = 698 || n = 700 || n = 710 || n = 715 || n = 720 || n = 725 || n = 730 || n = 735 || n = 741 || n = 745 || n = 750 || n = 755 || n = 760 || n = 765 || n = 770 || n = 780)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 633 || n = 696 || n = 723)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 636 || n = 756)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
	cmp $C5
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
	cmp $C5
	dey
	Jpl *-3
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
.elseif((n .mod 5 = 3) && (n = 783 || n = 818 || n = 823 || n = 848 || n = 853 || n = 863 || n = 878 || n = 883 || n = 893 || n = 898 || (n > 912 && n < 934)))
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 784 || n = 794 || n = 799 || n = 804 || n = 809 || n = 811 || n = 814 || n = 819 || n = 821 || n = 829 || n = 833 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 864 || n = 868 || n = 869 || n = 871 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 785 || n = 790 || n = 798 || n = 803 || n = 805 || n = 815 || n = 820 || n = 825 || n = 830 || n = 835 || n = 840 || n = 850 || n = 855 || n = 860 || n = 865 || n = 870 || n = 875 || n = 885 || n = 890 || n = 895 || n = 900 || n = 905 || n = 920 || n = 925 || n = 930 || n = 935)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 786 || n = 858 || n = 903)
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
	cmp $C5
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
.elseif(n = 793)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 795)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 800)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 801 || n = 806)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 808 || n = 813)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 810 || n = 824 || n = 838 || n = 866 || n = 873 || n = 880 || n = 901 || n = 908 || n = 915)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
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
.if(n = 938 || n = 939 || n = 941 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 991 || n = 994 || n = 1004 || n = 1009 || n = 1011 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1039 || n = 1043 || n = 1044 || n = 1046 || n = 1049 || n = 1053 || n = 1058 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1078 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 940 || n = 945 || n = 955 || n = 960 || n = 965 || n = 970 || n = 975 || n = 980 || n = 990 || n = 995 || n = 1000 || n = 1005 || n = 1010 || n = 1015 || n = 1028 || n = 1030 || n = 1035 || n = 1040 || n = 1045 || n = 1050 || n = 1054 || n = 1059 || n = 1060 || n = 1065 || n = 1070 || n = 1075 || n = 1080 || n = 1085)
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
.elseif(n = 953 || n = 958 || n = 963 || n = 968 || n = 998 || n = 1003 || n = 1023 || n = 1033 || n = 1073 || n = 1088 || n = 1093)
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
	cmp $C5
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
.elseif(n = 1051)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 1056)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_16_clocks
	pla
	plp
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
.if(n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1109 || n = 1113 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1148 || n = 1149 || n = 1151 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1179 || n = 1183 || n = 1184 || n = 1186 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
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
.elseif((n .mod 5 = 3) && (n = 1098 || n = 1103 || n = 1123 || (n > 1132 && n < 1144) || (n > 1157 && n < 1169) || n = 1178 || (n > 1192 && n < 1214) || n = 1233 || n = 1238))
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
	cmp $C5
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
.if((n .mod 7 = 5) && ((n > 1250 && n < 1266) || (n > 1278 && n < 1301) || n = 1321 || n = 1328))
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
.elseif(n = 1253 || n = 1254 || n = 1256 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1288 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1301 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1314 || n = 1316 || n = 1317 || n = 1322 || n = 1323 || n = 1327)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 1255 || n = 1260 || (n > 1269 && n < 1296) || n = 1310 || n = 1315))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1263 || n = 1308 || n = 1326)
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
	cmp $C5
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
	; 11 bytes
	pha
	tya
	ldy #0
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1299)
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
.elseif(n = 1306)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $70A0'
	ldy #112
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1307)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1313 || n = 1318)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1319)
	; 11 bytes
	php
	pha
	php
	lda #5
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 1320 || n = 1325)
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1331 || n = 1338 || n = 1345 || n = 1359 || n = 1366 || n = 1373 || n = 1381 || n = 1390 || n = 1394 || n = 1401)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1330 || n = 1336 || n = 1337 || n = 1339 || n = 1350 || n = 1351 || n = 1355 || n = 1357 || n = 1358 || n = 1365 || n = 1367 || n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1393 || n = 1395 || n = 1399 || n = 1400 || n = 1402)
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
	cmp #<$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1334)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1341)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jpl *-5
	tay
	pla
.elseif(n = 1343)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $6CA0'
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
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1360)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $96A0'
	ldy #150
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1361)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $96A0'
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
	cmp #<$CD ;hides 'cmp $69A0'
	ldy #105
	iny
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
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1406)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $9BA0'
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
	cmp <$CD ;hides 'cmp $64A0'
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1423)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $9DA0'
	ldy #157
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1424)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $9DA0'
	ldy #157
	dey
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
	cmp #<$CD ;hides 'cmp $62A0'
	ldy #98
	iny
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
	txa
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	inx
	Jne *-4
	tax
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
	tya
	cmp #<$CD ;hides 'cmp $60A0'
	ldy #96
	iny
	Jne *-4
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
	cmp #<$CD ;hides 'cmp $A1A0'
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
	cmp $C5
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
	cmp $C5
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
	cmp <$CD ;hides 'cmp $A2A0'
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
	cmp <$CD ;hides 'cmp $A3A0'
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
	cmp #<$CD ;hides 'cmp $A4A0'
	ldy #164
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1487)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A4A0'
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
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $A5A0'
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
	cmp #<$CD ;hides 'cmp $A6A0'
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
	cmp #<$CD ;hides 'cmp $A7A0'
	ldy #167
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1514)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A7A0'
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
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $A8A0'
	ldy #168
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1523)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A8A0'
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
.elseif(n = 1525 || n = 1526 || n = 1528 || n = 1540 || n = 1544 || n = 1546 || n = 1547 || n = 1553 || n = 1554 || n = 1561)
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
.elseif(n = 1529)
	; 11 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1530)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	Jmi *-4
	tax
	pla
.elseif(n = 1532 || n = 1556)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
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
.elseif(n = 1534 || n = 1555 || n = 1562)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1535)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
	cmp #<$CD ;hides 'cmp $75A0'
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
	cmp <$CD ;hides 'cmp $AAA0'
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
	cmp #<$CD ;hides 'cmp $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1550)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $ABA0'
	ldy #171
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1558)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $ACA0'
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
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 1564 || n = 1588)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1565 || n = 1567 || n = 1570 || n = 1572 || n = 1582 || n = 1589 || n = 1591 || n = 1597)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1566 || n = 1571 || n = 1583 || n = 1590 || n = 1598)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1569 || n = 1574)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 7 = 5) && ((n > 1572 && n < 1602)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1575)
	; 11 bytes
	php
	pha
	php
	lda #6
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 1576 || n = 1581)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1577)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1578 || n = 1596)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1579)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1584)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1585)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1586)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AFA0'
	ldy #175
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1592 || n = 1599)
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
.elseif(n = 1593 || n = 1600)
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
.elseif(n = 1595)
	; 10 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1603 || n = 1610 || n = 1616 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1633 || n = 1638)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1604 || n = 1612 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1604 && n < 1633)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1606 || n = 1627)
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
.elseif(n = 1607)
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
.elseif((n .mod 7 = 5) && ((n > 1607 && n < 1637)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
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
.elseif(n = 1611 || n = 1618 || n = 1625 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1613)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1620)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
	cmp #<$CD ;hides 'cmp $B4A0'
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
.elseif(n = 1635)
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
.endif
.else
; 1641 to 1718
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1645 || n = 1652 || n = 1666 || n = 1670 || n = 1673 || n = 1679 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1708 || n = 1710 || n = 1714 || n = 1715 || n = 1717)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1651 || n = 1674 || n = 1681 || n = 1688 || n = 1707 || n = 1709)
	; 10..12 bytes
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
	cmp <$CD ;hides 'cmp $B6A0'
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
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	dey
	Jne *-4
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
.elseif(n = 1660 || n = 1676 || n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1662 || n = 1669 || n = 1683 || n = 1690 || n = 1697 || n = 1718)
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
.elseif(n = 1663 || n = 1691 || n = 1698 || n = 1705)
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
.elseif(n = 1667)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1675)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B9A0'
	ldy #185
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1682)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
	cmp #<$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1703)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1711)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1712)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
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
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1721 && n < 1750)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1723 || n = 1735 || n = 1737 || n = 1744)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1724 || n = 1732 || n = 1756)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1725 || n = 1746)
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
	cmp <$CD ;hides 'cmp $BFA0'
	ldy #191
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1738)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C0A0'
	ldy #192
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1739)
	; 11 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	iny
	Jne *-2
	pla
	tay
	pla
.elseif(n = 1747)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
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
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	pla
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
.elseif(n = 1759 || n = 1771 || n = 1773 || n = 1777 || n = 1778 || n = 1792)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1760 || n = 1781 || n = 1795)
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
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
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
.elseif(n = 1765)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1766)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
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
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1772 || n = 1780 || n = 1796)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1774)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1775)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1779 || n = 1786)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1782)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1784)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	dey
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
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	dey
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
.elseif(n = 1798 || n = 1799 || n = 1806 || n = 1814 || n = 1821 || n = 1823 || n = 1826 || n = 1828 || n = 1829 || n = 1833 || n = 1834)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800)
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
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1803 || n = 1812)
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
.elseif(n = 1805 || n = 1816)
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
.elseif(n = 1807 || n = 1813 || n = 1822 || n = 1827)
	; 10..11 bytes
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
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1815)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A4A0'
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
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 1820)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 1825 || n = 1830)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1831)
	; 11 bytes
	php
	pha
	php
	lda #7
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 1832)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 1835)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
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
	cmp <$CD ;hides 'cmp $35A0'
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1845)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 1846)
	; 11 bytes
	pha
	tya
	ldy #204 ;hides 'cpy abs'
	cmp #$A9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1847)
	; 11 bytes
	pha
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	tax
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
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	dex
	Jne *-4
	tax
	pla
.elseif(n = 1856)
	; 11 bytes
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1864)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1865)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1869)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $A9A0'
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
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1874)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $31A0'
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1877 || n = 1878 || n = 1885 || n = 1886 || n = 1890 || n = 1894 || n = 1895 || n = 1898 || n = 1913)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1887 || n = 1905)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1880)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $AAA0'
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
	cmp <$CD ;hides 'cmp $AAA0'
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
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1883)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
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
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	dey
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
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1903)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $ACA0'
	ldy #172
	nop
	dey
	Jne *-5
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
	delay_n (n-7)
	plp
.elseif(n = 1909)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1910)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
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
	cmp $C5
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
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1919)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	dey
	Jne *-4
	tay
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
.elseif(n = 1921 || n = 1923 || n = 1930 || n = 1939 || n = 1941 || n = 1951)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1922 || n = 1931 || n = 1949 || n = 1950)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1925 || n = 1934 || n = 1943)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1927)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
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
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1935)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $AFA0'
	ldy #175
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1936)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1937)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1942 || n = 1952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1945)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1946)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $29A0'
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
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1955)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
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
.elseif(n = 1957 || n = 1966 || n = 1967 || n = 1975 || n = 1976 || n = 1984 || n = 1985)
	; 12 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1961)
	; 12 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1962)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1963)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1964 || n = 1972 || n = 1988)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
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
	cmp <$CD ;hides 'cmp $B2A0'
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
.elseif(n = 1971)
	; 11 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1973)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1977)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1978)
	; 13 bytes
	php
	delay_n (n-7)
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
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1982)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
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
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1991)
	; 11 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
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
.elseif(n = 1995 || n = 2007 || n = 2015 || n = 2031)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1996 || n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2000)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
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
.elseif(n = 2002 || n = 2022)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
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
.elseif(n = 2008)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2009)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2013)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B6A0'
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
	cmp #<$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2018)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2023)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B7A0'
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
	cmp <$CD ;hides 'cmp $B7A0'
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
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2027)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $E0A0'
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
	cmp $C5
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
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
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
	cmp $C5
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
.elseif(n = 2045)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	iny
	Jne *-4
	tay
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
.elseif(n = 2051)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2053)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2054)
	; 11 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2059)
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
	; 11 bytes
	pha
	tya
	ldy #0
	cmp $C5
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
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2063)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
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
	cmp #<$CD ;hides 'cmp $BBA0'
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
	cmp <$CD ;hides 'cmp $BBA0'
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
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2072)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2073 || n = 2091 || n = 2100 || n = 2109)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2074 || n = 2077 || n = 2079 || n = 2082 || n = 2084 || n = 2085 || n = 2095 || n = 2097)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2075)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 2076)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2078 || n = 2083 || n = 2092 || n = 2094 || n = 2101)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2081 || n = 2086)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2087)
	; 11 bytes
	php
	pha
	php
	lda #8
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 2088 || n = 2093)
	; 11 bytes
	brk
	delay_n (n-13)
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
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2096)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
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
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
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
.elseif(n = 2105)
	; 12 bytes
	php
	delay_n (n-7)
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
.elseif(n = 2107)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2108)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
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
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	tax
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
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2126)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
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
	cmp #<$CD ;hides 'cmp $3FA0'
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
	cmp #<$CD ;hides 'cmp $ECA0'
	ldy #236
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2135)
	; 11 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
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
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2144)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2155 || n = 2156 || n = 2164 || n = 2165 || n = 2169 || n = 2173 || n = 2174 || n = 2176 || n = 2183 || n = 2185 || n = 2186)
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
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2153)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
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
	cmp $C5
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2159)
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
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2162)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2166)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C4A0'
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
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2170)
	; 11 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2171)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2177)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2178)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2179)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2180)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0FA0'
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
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2189)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
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
.elseif(n = 2191 || n = 2192 || n = 2194 || n = 2209 || n = 2214 || n = 2218 || n = 2219 || n = 2222)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193 || n = 2213)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2195 || n = 2204)
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
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2198)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2200)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2201 || n = 2212 || n = 2223)
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
.elseif(n = 2203)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2207)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
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
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2215)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2216)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2221)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2224)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2225)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2228 || n = 2230 || n = 2236 || n = 2237 || n = 2246 || n = 2248 || n = 2254 || n = 2255 || n = 2257 || n = 2258 || n = 2263 || n = 2264)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229 || n = 2259)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2231)
	; 12 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
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
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2234)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
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
.elseif(n = 2240)
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
	; 11 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	inx
	Jne *-4
	tax
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
.elseif(n = 2245)
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
.elseif(n = 2249)
	; 12 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2250)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2251)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2252)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2260)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2261)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2265)
	; 12 bytes
	pha
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	nop
	dex
	Jne *-5
	tax
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
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2270)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $05A0'
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
	cmp $C5
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
.elseif(n = 2275 || n = 2285 || n = 2286 || n = 2303)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2276)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2277)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2278)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2279)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2287)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2288)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2296)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2297)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2306)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
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
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	nop
	iny
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2313 || n = 2323)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2314)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2315)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
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
	cmp #<$CD ;hides 'cmp $D2A0'
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
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 2324 to 2343
.if(n = 2324)
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
	cmp <$CD ;hides 'cmp $B2A0'
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
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2328)
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
	cmp $C5
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2331)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 2332)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2333 || n = 2335 || n = 2338 || n = 2340 || n = 2341)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 2334 || n = 2339)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2336)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2337 || n = 2342)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2343)
	; 11 bytes
	php
	pha
	php
	lda #9
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2349)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2345 || n = 2346 || n = 2351 || n = 2354)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2348)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2350)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	dey
	Jne *-4
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
	; 11 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2355 || n = 2366 || n = 2377)
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
.elseif(n = 2356 || n = 2361 || n = 2367 || n = 2379)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2357)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 2359 || n = 2360 || n = 2371)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2362 || n = 2372 || n = 2382)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2363)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2365)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2368 || n = 2380)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2369 || n = 2381)
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
.elseif(n = 2373)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2374)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2375)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	nop
	dey
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
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2395 || n = 2407 || n = 2413)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2384 || n = 2389 || n = 2390 || n = 2394 || n = 2401 || n = 2402 || n = 2404 || n = 2414 || n = 2416)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2385)
	; 13 bytes
	pha
	tya
	ldy #(n-33)/21
	cmp $C5
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2386)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2387)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
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
.elseif(n = 2391 || n = 2403 || n = 2415)
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
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2398)
	; 12 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $DAA0'
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
	cmp <$CD ;hides 'cmp $DAA0'
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
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	nop
	iny
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2423 || n = 2426 || n = 2428 || n = 2433 || n = 2438 || n = 2440 || n = 2444 || n = 2450)
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
	ldy #126
	delay_n 14
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2427 || n = 2439 || n = 2445 || n = 2449 || n = 2451)
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
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2431)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $24A0'
	ldy #36
	nop
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
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2437)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jmi *-4
	tay
	pla
	plp
.elseif(n = 2441)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
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
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2453 || n = 2455)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
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
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 2463)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $21A0'
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
	cmp <$CD ;hides 'cmp $21A0'
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
	cmp <$CD ;hides 'cmp $BDA0'
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
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	cmp $C5
	dex
	Jne *-6
	tax
	pla
.elseif(n = 2471)
	; 13 bytes
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	cmp $C5
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
	cmp <$CD ;hides 'cmp $E0A0'
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
.elseif(n = 2479 || n = 2481 || n = 2499)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2482 || n = 2492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2485)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2493)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $BFA0'
	ldy #191
	delay_n 4
	dey
	Jne *-6
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
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	iny
	Jne *-5
	tay
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
.if((n .mod 12 = 5) && ((n > 2500 && n < 2538)))
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
.elseif(n = 2502 || n = 2512)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 7) && ((n > 2502 && n < 2540)))
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
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	nop
	iny
	Jne *-5
	tay
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
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	nop
	iny
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
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2530)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
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
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2541)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2542 || n = 2553)
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
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2544 || n = 2556)
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
.elseif(n = 2546)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2547)
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2554)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2555 || n = 2558)
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
.endif
.else
; 2559 to 2578
.if(n = 2559 || n = 2571)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2560 || n = 2574)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2564 || n = 2575)
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
.elseif(n = 2565)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2566)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
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
.elseif(n = 2568)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2569)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2570 || n = 2576)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2572)
	; 12 bytes
	pha
	tya
	ldy #0
	nop
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2573)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2577)
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
.elseif(n = 2578)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.endif
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
.elseif(n = 2580 || n = 2604 || n = 2616)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2581 || n = 2582 || n = 2589 || n = 2591 || n = 2594 || n = 2596 || n = 2597 || n = 2601 || n = 2602 || n = 2606 || n = 2607 || n = 2609)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2590 || n = 2595)
	; 10..12 bytes
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
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2586)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2587)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 2588)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2593 || n = 2598)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2599)
	; 11 bytes
	php
	pha
	php
	lda #10
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 2600 || n = 2605)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2603 || n = 2608)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
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
.elseif(n = 2611)
	; 13 bytes
	delay_n 19
	delay_n (n-19)
.elseif(n = 2612)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
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
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2615)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2617)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2623 || n = 2635)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2625 || n = 2631 || n = 2643 || n = 2655)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2626)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 2627)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	cmp $C5
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
	cmp <$CD ;hides 'cmp $12A0'
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2634 || n = 2653)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	nop
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
.elseif(n = 2648)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2651)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
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
	cmp #<$CD ;hides 'cmp $0FA0'
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
	cmp <$CD ;hides 'cmp $0FA0'
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
.elseif(n = 2667 || n = 2679 || n = 2691)
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
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2675)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	cmp $C5
	iny
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
	cmp #<$CD ;hides 'cmp $F3A0'
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2689)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2692)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2695)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
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
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $CFA0'
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
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2706)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2711)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	cmp $C5
	iny
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
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2717)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
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
	cmp $C5
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2722)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2723)
	; 13 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 4
	iny
	Jne *-4
	pla
	tay
	pla
.elseif(n = 2727)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	nop
	dey
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
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 13 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	cmp $C5
	inx
	Jne *-6
	tax
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
.elseif(n = 2737 || n = 2743 || n = 2755 || n = 2767)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2738 || n = 2744 || n = 2758 || n = 2768)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2739)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	nop
	iny
	Jne *-5
	tay
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
.elseif(n = 2741 || n = 2765)
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
.elseif(n = 2745)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2746)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	cmp $C5
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2747)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2749)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
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
	nop
	brk
	Jne *-4
	tay
	pla
.elseif(n = 2752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2753 || n = 2763 || n = 2769)
	; 12 bytes
	pha
	delay_n (n-3)
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
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2761)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
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
.elseif(n = 2771)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
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
.elseif(n = 2777)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2778)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2779)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
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
	cmp $C5
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 2783)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
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
.elseif(n = 2789 || n = 2801)
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
.elseif(n = 2790)
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
.elseif(n = 2791 || n = 2803 || n = 2809)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2792)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2793)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
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
	cmp $C5
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
	cmp #<$CD ;hides 'cmp $02A0'
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
	cmp <$CD ;hides 'cmp $02A0'
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
.if(n = 2813 || n = 2823 || n = 2835 || n = 2841 || n = 2846 || n = 2851)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2824 || n = 2829 || n = 2830 || n = 2834 || n = 2836 || n = 2842 || n = 2845 || n = 2847 || n = 2850)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
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
	cmp <$CD ;hides 'cmp $01A0'
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
	cmp $C5
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
	; 12 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2825 || n = 2837)
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
	cmp <$CD ;hides 'cmp $00A0'
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	cmp $C5
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
.elseif(n = 2843)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 2849)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2853 || n = 2857 || n = 2858 || n = 2863 || n = 2865 || n = 2866 || n = 2870 || n = 2876)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2855)
	; 11 bytes
	php
	pha
	php
	lda #11
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 2856 || n = 2861)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 2859 || n = 2864)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2860)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2862)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 2867)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 2868 || n = 2880)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2869)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 2871 || n = 2877 || n = 2883 || n = 2888)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2873)
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
.elseif(n = 2875 || n = 2887)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2878)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2879)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2881)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2882)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2889)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2890)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	cmp $C5
	inx
	Jne *-6
	tax
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
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	cmp $C5
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
.elseif(n = 2894 || n = 2906 || n = 2908 || n = 2918 || n = 2920 || n = 2926)
	; 12..13 bytes
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2901 || n = 2925)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	cmp $C5
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2914)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2915)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	cmp $C5
	iny
	Jne *-6
	tay
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	cmp $C5
	dey
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2933 || n = 2945 || n = 2957)
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
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	cmp $C5
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
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2960)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2961)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2962)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	cmp $C5
	dey
	Jne *-6
	tay
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
.if(n = 2969 || n = 2993 || n = 3005)
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
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3008)))
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2972 || n = 2992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2973)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2974)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2975)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	cmp $C5
	dey
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
.elseif(n = 2981)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
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
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	cmp $C5
	inx
	Jne *-6
	tax
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
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2999)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3001)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3006)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	nop
	cmp $C5
	dey
	Jne *-6
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
.elseif(n = 3009 || n = 3044 || n = 3045)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3010 || n = 3014 || n = 3016 || n = 3020 || n = 3026 || n = 3028 || n = 3038 || n = 3040)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3011)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	cmp $C5
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
.elseif(n = 3015 || n = 3027 || n = 3039)
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
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	cmp $C5
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
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
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3035)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3037)
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
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
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
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
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
.elseif(n = 3049 || n = 3073 || n = 3085)
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
.elseif(n = 3056 || n = 3062 || n = 3066 || n = 3074 || n = 3076)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3058)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3059)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
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
	cmp <$CD ;hides 'cmp $8BA0'
	ldy #139
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3070)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3071)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3083)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
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
.if(n = 3086 || n = 3088 || n = 3094 || n = 3101 || n = 3103 || n = 3106 || n = 3108 || n = 3109 || n = 3113 || n = 3114 || n = 3119 || n = 3121 || n = 3122)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087 || n = 3093 || n = 3102 || n = 3107)
	; 10..12 bytes
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
.elseif(n = 3090)
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
.elseif(n = 3092)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3095)
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
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3099)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3100)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3104)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3105 || n = 3110)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3111)
	; 11 bytes
	php
	pha
	php
	lda #12
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 3112 || n = 3117)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3115 || n = 3120)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3116)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3118)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 3124)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3125)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
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
.elseif(n = 3127)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3128 || n = 3136 || n = 3142 || n = 3146 || n = 3150 || n = 3154 || n = 3159 || n = 3164)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129 || n = 3147 || n = 3155 || n = 3160)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3130)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3131)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3133 || n = 3151)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	cmp <$CD ;hides 'cmp $8EA0'
	ldy #142
	brk
	dey
	Jne *-6
	tay
	pla
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
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3143)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 3149)
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
.elseif(n = 3153)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3156)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3161)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	clc
	Jcc *+2
	brk
	Jne *-6
	tay
	pla
.elseif(n = 3163)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 3165 to 3203
.if(n = 3165 || n = 3183 || n = 3201)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3167)
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
.elseif(n = 3168 || n = 3186)
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
.elseif(n = 3169)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 4
	dey
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3173 || n = 3181 || n = 3188 || n = 3189 || n = 3199)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 3178 || n = 3182 || n = 3193 || n = 3200 || n = 3202)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3179)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	delay_n 7
	dey
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
.elseif(n = 3187)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 4
	iny
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
.elseif(n = 3203)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
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
.elseif(n = 3205)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3206 || n = 3214 || n = 3220 || n = 3236 || n = 3238)
	; 12..13 bytes
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
	cmp <$CD ;hides 'cmp $0AA0'
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
	cmp <$CD ;hides 'cmp $C8A0'
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
	nop
	brk
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
.elseif(n = 3217 || n = 3230 || n = 3231 || n = 3233)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3221)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3223)
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
.elseif(n = 3224)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3226)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
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
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
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
.elseif(n = 3235)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 4
	iny
	Jne *-6
	tay
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
.elseif(n = 3241)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 3243 to 3281
.if(n = 3243)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	delay_n 7
	iny
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
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	brk
	iny
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
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3248 || n = 3254 || n = 3256 || n = 3278)
	; 12..13 bytes
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
.elseif(n = 3250 || n = 3251 || n = 3253 || n = 3269 || n = 3281)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
.elseif(n = 3255 || n = 3261 || n = 3273 || n = 3279)
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
.elseif(n = 3265 || n = 3277)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3266)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $6CA0'
	ldy #108
	brk
	iny
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
	delay_n 4
	brk
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3284 || n = 3285 || n = 3303)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	cmp <$CD ;hides 'cmp $04A0'
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
.elseif(n = 3289 || n = 3291 || n = 3296 || n = 3305 || n = 3309 || n = 3315)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3290 || n = 3294 || n = 3295 || n = 3304 || n = 3308)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3292 || n = 3312)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
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
.elseif(n = 3299 || n = 3318)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3300)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
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
.elseif(n = 3307)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3311)
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
.elseif(n = 3313)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3317)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3320)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 3321 to 3359
.if(n = 3321)
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
.elseif(n = 3322)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3323)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $31A0'
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
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3326 || n = 3328 || n = 3329 || n = 3341 || n = 3346 || n = 3353 || n = 3357 || n = 3359)
	; 10..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3330 || n = 3344)
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3333)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $97A0'
	ldy #151
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3334)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3335 || n = 3345 || n = 3358)
	; 10..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3337)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3338)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3339)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
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
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3354)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3355)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_11_clocks_
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
.elseif(n = 3361 || n = 3366)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3362 || n = 3364 || n = 3365 || n = 3369 || n = 3370 || n = 3375 || n = 3377 || n = 3380 || n = 3382 || n = 3397 || n = 3398)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363 || n = 3381 || n = 3389)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3367)
	; 11 bytes
	php
	pha
	php
	lda #13
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 3368 || n = 3373)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 3371 || n = 3376)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3372)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3374)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 3378 || n = 3396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3379)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3383)
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
.elseif(n = 3384)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3385)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3386)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3393)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3394)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3395)
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
; 3399 to 3437
.if(n = 3399 || n = 3405 || n = 3417)
	; 12..13 bytes
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
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3403)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 7
	dey
	Jne *-6
	tay
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
.elseif((n > 3405 && n < 3409) || n = 3411 || (n > 3422 && n < 3427) || n = 3433)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
.elseif(n = 3415 || n = 3416 || n = 3430 || n = 3431)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3418)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3419)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3421)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3422)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3434)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
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
.elseif(n = 3440 || n = 3441 || n = 3460 || n = 3469 || n = 3476)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3444 || n = 3448 || n = 3458 || n = 3465 || n = 3474)
	; 13 bytes
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
.elseif(n = 3449 || n = 3453 || n = 3459 || n = 3464 || n = 3471)
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3454)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3467)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	delay_n 7
	dey
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
.elseif(n = 3478 || n = 3479 || n = 3485 || n = 3496 || n = 3502 || n = 3503 || n = 3505 || n = 3513)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 3481)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3482)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3483)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $27A0'
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
.elseif(n = 3488 || n = 3494 || n = 3506)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 6 = 3) && ((n > 3488 && n < 3508)))
	; 12..13 bytes
	pha
	delay_n (n-3)
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3493 || n = 3511)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3499)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3515)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
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
.elseif(n = 3518 || n = 3521 || n = 3523 || n = 3536 || n = 3538 || n = 3539 || n = 3549)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3519)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
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
.elseif(n = 3524 || n = 3528 || n = 3533 || n = 3534 || n = 3542 || n = 3544 || n = 3550)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3525 || n = 3543 || n = 3551)
	; 12..13 bytes
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3529)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3530)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $24A0'
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
	nop
	brk
	dey
	Jne *-4
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
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	delay_n 7
	dey
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
.elseif(n = 3556 || n = 3560 || n = 3566 || n = 3569 || n = 3570 || n = 3574 || n = 3578 || n = 3591)
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
.elseif(n = 3559)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3562)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
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
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	delay_n 7
	iny
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3573)
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
.elseif(n = 3575)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A2A0'
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
.elseif(n = 3583)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3584)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	cmp <$CD ;hides 'cmp $39A0'
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
	cmp <$CD ;hides 'cmp $E0A0'
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
.elseif(n = 3597 || n = 3602 || n = 3614 || n = 3619)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3598 || n = 3601 || n = 3605 || n = 3613 || n = 3615 || n = 3618 || n = 3620 || n = 3621 || n = 3625 || n = 3626 || n = 3631)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599)
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
.elseif(n = 3603)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3604)
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
.elseif(n = 3606)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3607 || n = 3624 || n = 3629)
	; 11..13 bytes
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
.elseif(n = 3611)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3616)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3617 || n = 3622)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3623)
	; 11 bytes
	php
	pha
	php
	lda #14
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 3627 || n = 3632)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3628)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 3634)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3635)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3636)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 3637 || n = 3642)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3638)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
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
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3641)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3643)
	; 13 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	delay_n 7
	inx
	Jne *-6
	tax
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3646 || n = 3647 || n = 3649)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	nop
	brk
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
.elseif(n = 3654 || n = 3668)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3655)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3656 || n = 3665)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3659)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3660)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3661)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3663)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $A6A0'
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
.elseif(n = 3671)
	; 12 bytes
	php
	delay_n (n-7)
	plp
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3673 || n = 3709)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3674)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3675)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
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
.elseif(n = 3678 || n = 3681 || n = 3683 || n = 3693 || n = 3696 || n = 3699 || n = 3700 || n = 3710)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3679 || n = 3698)
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
	txa
	ldx #202 ;hides 'dex'
	nop
	brk
	dex
	dex
	Jne *-6
	tax
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
	cmp <$CD ;hides 'cmp $A7A0'
	ldy #167
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3686 || n = 3688 || n = 3694 || n = 3703 || n = 3704)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3687 || n = 3705)
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
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3691)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 7
	iny
	Jne *-6
	tay
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
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3711 to 3750
.if(n = 3711 || n = 3713 || n = 3716 || n = 3719 || n = 3731 || n = 3746 || n = 3748 || n = 3749)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3714 || n = 3734 || n = 3737 || n = 3740 || n = 3742)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3715 || n = 3733)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3727 || n = 3745)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3728)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $A9A0'
	ldy #169
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3729 || n = 3750)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3730)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3736)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3739)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
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
	brk
	delay_n (n-13)
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
.elseif(n = 3753 || (n > 3763 && n < 3769) || (n > 3781 && n < 3786))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 3757 || n = 3759 || n = 3775 || n = 3777 || n = 3789)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3758 || n = 3760 || n = 3776 || n = 3778)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3770)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3771)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3773)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $ABA0'
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
.elseif(n = 3787)
	; 13 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	delay_n 7
	dey
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
.if(n = 3790 || n = 3791 || n = 3800 || n = 3801 || n = 3808 || n = 3809 || n = 3811)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3794 || n = 3796 || n = 3805 || n = 3824)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3795 || n = 3806 || n = 3813)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3799 || n = 3817)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3802)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	delay_n 7
	iny
	Jne *-6
	tay
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
.elseif(n = 3812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3814)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3818)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3819)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
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
	cmp <$CD ;hides 'cmp $02A0'
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
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3827)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3831 || n = 3847 || n = 3849 || n = 3857)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3830 || n = 3845 || n = 3848 || n = 3856 || n = 3858 || n = 3866)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3832 || n = 3852)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3833)
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
.elseif(n = 3834)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $11A0'
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
	cmp <$CD ;hides 'cmp $EFA0'
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
.elseif(n = 3837 || n = 3854)
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
	cmp #<$CD ;hides 'cmp $AEA0'
	ldy #174
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3839)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3840)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3841 || n = 3859 || n = 3865)
	; 13..14 bytes
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
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3844)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3846 || n = 3864)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3850)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3851)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3853)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3855)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3860)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $AFA0'
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
	cmp <$CD ;hides 'cmp $AFA0'
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
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3863)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3867)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_11_clocks_
	pla
	plp
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
.elseif(n = 3869 || n = 3871 || n = 3874 || n = 3876 || n = 3877 || n = 3881 || n = 3887 || n = 3889 || n = 3890 || n = 3894 || n = 3902 || n = 3904)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870 || n = 3875 || n = 3903)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3873 || n = 3878)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3879)
	; 11 bytes
	php
	pha
	php
	lda #15
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 3880 || n = 3885 || n = 3895)
	; 11..13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3883 || n = 3888)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3884)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3886)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 3891)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 3892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3893 || n = 3898)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 3896)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3897)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3899)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3901)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3905)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3906)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
.if((n >= 3907) && (n < 3946))
; 3907 to 3945
.if(n = 3907 || n = 3926 || n = 3945)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3908 || n = 3921 || n = 3935)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3909 || n = 3910 || n = 3917 || n = 3919 || n = 3929 || n = 3933 || n = 3944)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3911)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 3912 || n = 3932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
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
	cmp #<$CD ;hides 'cmp $0CA0'
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
	cmp <$CD ;hides 'cmp $0CA0'
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
.elseif(n = 3920 || n = 3924 || n = 3934 || n = 3938 || n = 3940 || n = 3941)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3925 || n = 3943)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3927)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B2A0'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3930)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3931)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 7
	iny
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
.endif
.else
; 3946 to 3984
.if(n = 3946)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3947)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3951)
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3953 || n = 3965 || n = 3966 || n = 3968 || n = 3969 || n = 3982 || n = 3984)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 3955 || n = 3957 || n = 3975)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3958 || n = 3974 || n = 3976)
	; 12..13 bytes
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
	cmp $C5
	brk
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
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3963)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3964 || n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3967)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3970)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B4A0'
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
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3978)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3979)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3983)
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
; 3985 to 4062
.if(n = 3985 || n = 4003 || n = 4023 || n = 4039 || n = 4057)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3986 || n = 3989 || n = 3991 || (n > 3997 && n < 4002) || (n > 4003 && n < 4007) || (n > 4017 && n < 4021) || n = 4022 || n = 4025 || n = 4045 || n = 4053 || n = 4056)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3993 || n = 4011 || n = 4013 || n = 4029 || n = 4035 || n = 4047 || n = 4055 || n = 4061)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3994)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
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
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3997 || n = 4015 || n = 4017 || n = 4033 || n = 4051)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4002 || n = 4040 || n = 4059)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4009 || n = 4014 || n = 4028 || n = 4030 || n = 4034 || n = 4046 || n = 4048 || n = 4054)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4016 || n = 4038)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4021)
	; 12 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4027)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 4036)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B7A0'
	ldy #183
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4042)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
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
	cmp <$CD ;hides 'cmp $04A0'
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
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063 || n = 4071)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4064 || n = 4066 || n = 4077 || n = 4082 || n = 4084 || n = 4088 || n = 4089 || n = 4094 || n = 4099 || n = 4100)
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
	nop
	brk
	Jne *-4
	tax
	pla
.elseif(n = 4072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4074)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4079)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 9
	iny
	Jne *-7
	tay
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
	cmp <$CD ;hides 'cmp $B9A0'
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4090)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
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
	cmp <$CD ;hides 'cmp $01A0'
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
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 4102 to 4140
.if(n = 4102 || n = 4110 || n = 4111 || n = 4113 || n = 4114 || n = 4118 || n = 4120 || n = 4125 || n = 4127 || n = 4130 || n = 4132 || n = 4133 || n = 4137 || n = 4138)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4103 || n = 4121)
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
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4105)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4106)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4107)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
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
.elseif(n = 4109)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4115 || n = 4119 || n = 4126 || n = 4131)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4116)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4117)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4122)
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
.elseif(n = 4123)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4124)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $BBA0'
	ldy #187
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4128)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4129 || n = 4134)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4135)
	; 11 bytes
	php
	pha
	php
	lda #16
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 4136)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4139)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4140)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4165)
	; 11..13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4142)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 4143 || n = 4145 || n = 4146 || n = 4150 || n = 4154 || n = 4156 || n = 4171 || n = 4179)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4144)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4147)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4148)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4149)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4151 || n = 4155 || n = 4163 || n = 4173)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4157 || n = 4175)
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
.elseif(n = 4159)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4160 || n = 4177)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4161 || n = 4164 || n = 4178)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4162)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
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
.elseif(n = 4167)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
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
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4172)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4174)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 4180 to 4218
.if(n = 4180 || n = 4191 || n = 4209 || n = 4215)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4181 || n = 4182 || n = 4184 || n = 4185 || (n > 4197 && n < 4201) || (n > 4201 && n < 4206) || n = 4207 || (n > 4215 && n < 4219))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4183 || n = 4201)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 4190 || n = 4196 || n = 4208 || n = 4213)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4192 || n = 4212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
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
.elseif(n = 4195)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4214)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4237 || n = 4255 || n = 4291)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4220 || n = 4221 || n = 4223 || (n > 4237 && n < 4242) || n = 4243 || n = 4250 || n = 4251 || n = 4253 || n = 4261 || (n > 4268 && n < 4272) || n = 4276 || n = 4277 || n = 4286 || n = 4288 || n = 4289 || n = 4293)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 4226 || n = 4234 || n = 4244 || n = 4246 || n = 4247 || n = 4254 || n = 4264 || n = 4274 || n = 4280 || n = 4282 || n = 4294)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4227 || n = 4263 || n = 4281 || n = 4295)
	; 12..13 bytes
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
.elseif(n = 4231 || n = 4267 || n = 4285)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4232 || n = 4252 || n = 4272 || n = 4292)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4233 || n = 4275)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4235)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C0A0'
	ldy #192
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4236 || n = 4258)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4248 || n = 4266 || n = 4284)
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
.elseif(n = 4256)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
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
.endif
.else
; 4297 to 4375
.if(n = 4297)
	; 13 bytes
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	brk
	iny
	Jne *-4
	pla
	tay
	pla
.elseif(n = 4298 || n = 4300 || n = 4315 || n = 4316 || n = 4318 || n = 4329 || n = 4334 || n = 4336 || n = 4341 || n = 4349 || n = 4351 || n = 4361 || n = 4365 || n = 4366 || n = 4370 || n = 4374)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4317 || n = 4333 || n = 4335 || n = 4342 || n = 4353 || n = 4371)
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4303 || n = 4321 || n = 4339 || n = 4357 || n = 4375)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4304 || n = 4305 || n = 4307 || n = 4308 || n = 4310 || n = 4311 || n = 4326 || n = 4328)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 4309 || n = 4327 || n = 4343 || n = 4345 || n = 4360)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4312 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4313 || n = 4330 || n = 4347)
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
.elseif(n = 4320 || n = 4356)
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
	cmp #<$CD ;hides 'cmp $C4A0'
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
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4331)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4338 || n = 4359)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4340)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4348)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4354)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4358)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $31A0'
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
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	brk
	dey
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
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
.if((n >= 4376) && (n < 4415))
; 4376 to 4414
.if(n = 4376 || n = 4381 || n = 4383 || n = 4386 || n = 4388 || n = 4389 || n = 4393 || n = 4394 || n = 4399 || n = 4401 || n = 4402 || n = 4406)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4378)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4379)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4380)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4382 || n = 4387 || n = 4407 || n = 4413)
	; 10..13 bytes
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
.elseif(n = 4385 || n = 4390)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4391)
	; 11 bytes
	php
	pha
	php
	lda #17
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 4392 || n = 4397)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 4395 || n = 4400)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4396)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4398)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 4403)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4404)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4405)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4408)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
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
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4411)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4412)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4414)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4421 || n = 4425 || n = 4443)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4416 || n = 4419 || n = 4430 || n = 4431 || n = 4436 || n = 4438 || n = 4441 || (n > 4447 && n < 4452))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 4420 || n = 4439)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4423)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4424 || n = 4426 || n = 4442 || n = 4444)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4429 || n = 4447)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4432 || n = 4452)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4433)
	; 13 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $EA ;hides 'nop'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
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
.endif
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4460 || n = 4474 || n = 4478 || n = 4480 || n = 4496 || n = 4498 || n = 4514 || n = 4516)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4455)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	brk
	iny
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
.elseif(n = 4457 || (n > 4465 && n < 4471) || n = 4473 || n = 4484 || n = 4486 || n = 4488 || n = 4490 || n = 4491 || n = 4493 || n = 4502 || n = 4503 || n = 4505 || (n > 4507 && n < 4512) || n = 4513 || n = 4523 || n = 4524 || n = 4526 || n = 4528 || n = 4529)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
.elseif(n = 4459 || n = 4461 || n = 4475 || n = 4479 || n = 4495 || n = 4497 || n = 4515)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4462)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
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
.elseif(n = 4464 || n = 4485 || n = 4506 || n = 4527)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4465 || n = 4501 || n = 4519)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4471 || n = 4489 || n = 4525)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4472 || n = 4492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
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
.elseif(n = 4482 || n = 4518)
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
.elseif(n = 4483)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	jsr @nearby_rts
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
.elseif(n = 4500 || n = 4522)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4504)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
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
.elseif(n = 4520)
	; 13 bytes
	pha
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	brk
	dex
	Jne *-6
	tax
	pla
.elseif(n = 4521)
	; 13 bytes
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	brk
	dey
	Jne *-6
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
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4552 || n = 4572 || n = 4592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4533 || n = 4551 || n = 4569 || n = 4575 || n = 4587 || n = 4594 || n = 4601 || n = 4605)
	; 11..13 bytes
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
.elseif(n = 4536 || n = 4608)
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
.elseif(n = 4537 || n = 4555 || n = 4573 || n = 4598)
	; 12..13 bytes
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
.elseif(n = 4539 || n = 4540 || (n > 4544 && n < 4548) || n = 4549 || (n > 4555 && n < 4561) || n = 4563 || n = 4567 || (n > 4575 && n < 4579) || (n > 4579 && n < 4584))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4541)
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4542)
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4543 || n = 4561 || n = 4579 || n = 4585)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4544)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
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
.elseif(n = 4550 || n = 4554 || n = 4568 || n = 4570 || n = 4574 || n = 4586 || n = 4588 || n = 4593 || n = 4595 || n = 4596 || n = 4599 || n = 4600 || n = 4604 || n = 4606)
	; 11..13 bytes
	delay_n (n-2)
	nop
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
	cmp #<$CD ;hides 'cmp $CFA0'
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
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4590)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
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
.if(n = 4610 || n = 4629)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4611 || n = 4613 || n = 4617 || n = 4618 || n = 4622 || n = 4624 || n = 4628 || n = 4634 || n = 4637 || n = 4639 || n = 4642 || n = 4644 || n = 4645)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4619 || n = 4623 || n = 4638 || n = 4643)
	; 10..13 bytes
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
.elseif(n = 4616 || n = 4627)
	; 12..13 bytes
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
	cmp #<$CD ;hides 'cmp $D2A0'
	ldy #210
	brk
	dey
	Jne *-6
	tay
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4633 || n = 4648)
	; 11..13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4635)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4636)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4640)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4641 || n = 4646)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4647)
	; 11 bytes
	php
	pha
	php
	lda #18
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4655 || n = 4657 || n = 4658)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4651 || n = 4656)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4652)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4653 || n = 4663)
	; 11..13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4654)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 4659)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4660)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4661 || n = 4666)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
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
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4665)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
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
.elseif((n > 4667 && n < 4672) || n = 4673 || n = 4678 || (n > 4679 && n < 4685))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4674)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4675)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4676)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4677)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4679)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4685)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
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
.if(n = 4688 || n = 4693 || n = 4699 || n = 4715 || n = 4721 || n = 4735)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4689 || n = 4691 || (n > 4699 && n < 4703) || n = 4704 || (n > 4705 && n < 4712) || n = 4713 || n = 4717 || n = 4722 || n = 4723 || (n > 4724 && n < 4732) || n = 4733 || n = 4736 || n = 4738 || n = 4739 || (n > 4743 && n < 4752) || n = 4753 || n = 4756 || n = 4757 || (n > 4758 && n < 4762) || n = 4765)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 4692 || n = 4712 || n = 4732 || n = 4752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4694 || n = 4714 || n = 4734 || n = 4754)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4696)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4697)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4698 || n = 4720 || n = 4742 || n = 4764)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4703)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4705 || n = 4724 || n = 4743 || n = 4762)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4716 || n = 4758)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4718)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4719)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	brk
	iny
	Jne *-6
	tay
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
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4741)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	brk
	iny
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
.elseif(n = 4763)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 4766 to 4843
.if((n > 4765 && n < 4772) || n = 4773 || (n > 4775 && n < 4779) || n = 4780 || n = 4782 || n = 4783 || (n > 4787 && n < 4792) || n = 4793 || (n > 4795 && n < 4800) || (n > 4800 && n < 4806) || n = 4809 || n = 4811 || n = 4813 || n = 4816 || n = 4818 || n = 4820 || (n > 4821 && n < 4828) || n = 4831 || n = 4833 || n = 4836 || n = 4839 || n = 4840)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4772 || n = 4792 || n = 4812 || n = 4832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4774 || n = 4794 || n = 4814 || n = 4834)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4775 || n = 4795 || n = 4815 || n = 4835 || n = 4843)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 4779 || n = 4821 || n = 4842)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
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
.elseif(n = 4784)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4785)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4786 || n = 4817)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
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
.elseif(n = 4806)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DAA0'
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
	cmp <$CD ;hides 'cmp $DAA0'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4810)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4828)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4829)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4841)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4846 || n = 4853 || n = 4854 || n = 4856 || n = 4859 || n = 4864 || n = 4878 || n = 4880 || n = 4881)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845 || n = 4865)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4847)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	bit $24
	brk
	sbc #1
	Jne *-6
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
.elseif(n = 4849)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	bit $24
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4850)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $24A0'
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
	cmp <$CD ;hides 'cmp $24A0'
	ldy #36
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4852 || n = 4872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4855 || n = 4866 || n = 4875)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
	pha
	tya
	ldy #242
	nop
	brk
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4860)
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4867)
	; 15 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 4868 || n = 4869)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4870)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 11
	dey
	Jne *-8
	tay
	pla
.elseif(n = 4871)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 11
	dey
	Jne *-8
	tay
	pla
.elseif(n = 4873)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4874)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
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
.elseif(n = 4877)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	nop
	brk
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
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4884)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4885 || n = 4904 || n = 4909)
	; 11..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4886)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4888)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4889 || n = 4893 || n = 4895 || n = 4898 || n = 4900 || n = 4901 || n = 4905 || n = 4906 || n = 4911 || n = 4913 || n = 4914)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4890 || n = 4894 || n = 4899)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4891)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4896)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 4897 || n = 4902)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4903)
	; 11 bytes
	php
	pha
	php
	lda #19
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 4907 || n = 4912)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4908)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4910)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 4915)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4916)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4917)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4918)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4919 || n = 4921)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4920)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 4923 || n = 4925 || (n > 4926 && n < 4930) || n = 4931 || n = 4933 || n = 4936 || (n > 4941 && n < 4947) || (n > 4947 && n < 4952) || n = 4953 || (n > 4955 && n < 4960) || (n > 4962 && n < 4968) || (n > 4968 && n < 4972) || n = 4973 || n = 4976 || (n > 4977 && n < 4982) || n = 4985 || n = 4986 || n = 4988 || n = 4990 || n = 4991 || n = 4993 || (n > 4995 && n < 5001))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
.elseif(n = 4926 || n = 4947 || n = 4968 || n = 4989)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4930)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4934 || n = 4954 || n = 4974 || n = 4994)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4935)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 4938)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4939)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4940 || n = 4962 || n = 4984)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
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
.elseif(n = 4955 || n = 4995)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4960)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4961)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	brk
	iny
	Jne *-6
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
.elseif(n = 4977)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4982)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4983)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	brk
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
.endif
.else
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
.if((n >= 5001) && (n < 5313))
.if((n >= 5001) && (n < 5157))
.if((n >= 5001) && (n < 5079))
; 5001 to 5078
.if(n = 5001 || n = 5002 || (n > 5006 && n < 5010) || n = 5011 || n = 5013 || (n > 5015 && n < 5026) || n = 5029 || n = 5030 || n = 5033 || n = 5036 || (n > 5037 && n < 5048) || n = 5051 || n = 5053 || (n > 5055 && n < 5062) || n = 5063 || n = 5064 || (n > 5065 && n < 5070) || (n > 5075 && n < 5079))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5003 || n = 5034 || n = 5065)
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
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5005)
	; 13 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5006 || n = 5028 || n = 5050)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5010 || n = 5031 || n = 5073)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5014 || n = 5054 || n = 5074)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5015 || n = 5035 || n = 5055 || n = 5075)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5026)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5027)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	brk
	iny
	Jne *-6
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
.elseif(n = 5048)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5049)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5070)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5071)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n > 5078 && n < 5085) || n = 5086 || (n > 5088 && n < 5092))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 5088 || n = 5095 || n = 5104 || n = 5117)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5092 || n = 5112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5093)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5094 || n = 5116)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
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
	nop
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5098)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5099 || n = 5108)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 5102 || n = 5103 || n = 5105 || n = 5106 || n = 5109 || n = 5110)
	; 12..15 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5111)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5113)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F3A0'
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
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5115)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5122 || n = 5123 || n = 5129 || n = 5130 || n = 5134 || n = 5140 || n = 5149 || n = 5151 || n = 5154 || n = 5156)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119 || n = 5127 || n = 5128 || n = 5139 || n = 5144)
	; 13..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5120)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	iny
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
.elseif(n = 5124 || n = 5131 || n = 5135 || n = 5150 || n = 5155)
	; 10..14 bytes
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
.elseif(n = 5132)
	; 12 bytes
	pha
	tya
	ldy #0
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5133 || n = 5145)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 5136)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5141)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5142)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5143)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	bit $24
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5146)
	; 13 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	nop
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5147)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_11_clocks_
	pla
	plp
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
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5161 || n = 5162 || n = 5167 || n = 5169 || n = 5170)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5158)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5159)
	; 11 bytes
	php
	pha
	php
	lda #20
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5160 || n = 5165)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5163 || n = 5168)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5164)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5166)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 5171)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5172)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5173 || n = 5178)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5174 || n = 5175 || n = 5177)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5176)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5179 || (n > 5182 && n < 5186) || n = 5188 || n = 5190 || (n > 5191 && n < 5199) || n = 5200 || n = 5201 || (n > 5204 && n < 5212) || (n > 5212 && n < 5220) || (n > 5220 && n < 5224) || (n > 5226 && n < 5235))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5180)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5181)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5182 || n = 5204 || n = 5226)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5186)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5187 || n = 5212)
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
.elseif(n = 5191)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5199)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5202)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $ECA0'
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
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5224)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5225)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5235 to 5312
.if(n = 5235 || n = 5236 || (n > 5237 && n < 5241) || (n > 5241 && n < 5246) || n = 5249 || n = 5250 || (n > 5251 && n < 5262) || (n > 5262 && n < 5268) || (n > 5270 && n < 5282) || (n > 5283 && n < 5287) || n = 5288 || n = 5289 || (n > 5292 && n < 5304) || (n > 5304 && n < 5312))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
.elseif(n = 5241 || n = 5283 || n = 5304)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5246)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5247)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5248 || n = 5270 || n = 5292)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
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
.elseif(n = 5268)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $11A0'
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
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5290)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5291)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	brk
	iny
	Jne *-6
	tay
	pla
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif((n > 5314 && n < 5325) || (n > 5325 && n < 5334) || (n > 5337 && n < 5344) || n = 5345 || (n > 5346 && n < 5352))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5325 || n = 5346)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5334)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5335)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	brk
	iny
	Jne *-6
	tay
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
.elseif(n = 5353 || n = 5354)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5355 || n = 5359 || n = 5361 || n = 5370)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5356)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	cmp $C5
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5357)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5358)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5360 || n = 5369)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5362)
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
	php
	delay_n (n-7)
	plp
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
	cmp #<$CD ;hides 'cmp $01A0'
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
	cmp <$CD ;hides 'cmp $01A0'
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5368)
	; 15 bytes
	pha
	tya
	ldy #206
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5372)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	bit $24
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5373 || n = 5379 || n = 5381 || n = 5389 || n = 5390)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5374)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	bit $24
	brk
	dey
	Jne *-5
	tay
	pla
	plp
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
	cmp $C5C5 ;hides 'cmp zp'
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
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5378)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5380)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5382)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5383)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5384)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5385)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5386)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5387)
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
.elseif(n = 5388)
	; 13 bytes
	pha
	tya
	ldy #0
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.endif
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
	cmp $C5
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5393)
	; 15 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 14
	inx
	Jne *-8
	tax
	pla
.elseif(n = 5394)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5395 || n = 5406 || n = 5411)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5396 || n = 5404 || n = 5405 || n = 5407 || n = 5410 || n = 5412 || n = 5413 || n = 5417 || n = 5418 || n = 5423 || n = 5425 || n = 5426)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5397)
	; 15 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	brk
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5398)
	; 15 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	brk
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5399)
	; 15 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	bit $24
	brk
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5400)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0BA0'
	ldy #11
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5401)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5402)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5403)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5408)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5409 || n = 5414)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5415)
	; 11 bytes
	php
	pha
	php
	lda #21
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5416 || n = 5421)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5419 || n = 5424)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5420)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5422)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 5427)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5428)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5429)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 5430 to 5468
.if(n = 5430 || n = 5431 || n = 5433)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5432)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5434)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5435 || n = 5436 || (n > 5437 && n < 5442) || n = 5443 || (n > 5447 && n < 5462) || (n > 5462 && n < 5466))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5437 || n = 5468)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5442)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5444)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5445)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5446)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5447)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
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
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5467)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5487) || (n > 5491 && n < 5499) || (n > 5499 && n < 5510) || (n > 5512 && n < 5530) || n = 5531 || n = 5535 || n = 5536 || (n > 5537 && n < 5554) || (n > 5556 && n < 5561) || (n > 5562 && n < 5576) || (n > 5578 && n < 5587) || (n > 5587 && n < 5592) || (n > 5592 && n < 5598) || (n > 5600 && n < 5609))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 5488)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
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
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5490 || n = 5512 || n = 5534 || n = 5556 || n = 5578 || n = 5600 || n = 5622)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5491)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	brk
	Jne *-6
	tax
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
.elseif(n = 5510)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $06A0'
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
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5532)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
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
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5554)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	brk
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 5555)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5576)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
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
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5598)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
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
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5609 || n = 5610)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 5613 || n = 5615 || n = 5616 || n = 5624 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5614)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 5618 || n = 5619)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5620)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
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
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	tay
	pla
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
	delay_n 4
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5627)
	; 14 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	brk
	inx
	Jne *-7
	tax
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
.elseif(n = 5629 || n = 5636 || n = 5657 || n = 5662)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5630 || n = 5634 || n = 5635 || n = 5645 || n = 5646 || n = 5648 || n = 5656 || n = 5658 || n = 5661 || n = 5663)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5631)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5632)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	iny
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
.elseif(n = 5637)
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
.elseif(n = 5639 || n = 5640 || n = 5649)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5641)
	; 14 bytes
	pha
	txa
	pha
	ldx #0 ;hides 'brk imm'
	inc $03B5,X ;hides 'lda $03,X'
	dex
	Jne *-5
	pla
	tax
	pla
.elseif(n = 5642)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5643)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5647)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5650)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5651)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5653)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 4
	brk
	iny
	Jne *-8
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
	ldy #171
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5659)
	; 9 bytes
	php
	pha
	lda #22
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5660)
	; 15 bytes
	pha
	tya
	ldy #(n-18)/31
	delay_n 26
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
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
.if(n = 5665 || n = 5670)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5666 || n = 5668 || n = 5669 || n = 5673 || n = 5674 || n = 5679 || n = 5681 || n = 5682)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5671)
	; 11 bytes
	php
	pha
	php
	lda #22
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5672 || n = 5677)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 5675 || n = 5680)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5676)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5678)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 5683)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5684)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5685 || n = 5690)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5686 || n = 5689)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
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
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 5690 && n < 5698) || (n > 5698 && n < 5703))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5698)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5703)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 5704 to 5781
.if((n > 5703 && n < 5712) || (n > 5712 && n < 5716) || (n > 5716 && n < 5737) || (n > 5737 && n < 5747) || (n > 5747 && n < 5762) || (n > 5762 && n < 5778) || (n > 5778 && n < 5782))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5787) || (n > 5787 && n < 5809) || n = 5810 || n = 5811 || (n > 5812 && n < 5837) || n = 5838 || n = 5839 || (n > 5840 && n < 5860))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5787 || n = 5812 || n = 5837)
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
.elseif(n = 5809 || n = 5840)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860 || n = 5861 || n = 5863 || n = 5864)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5862)
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
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5867)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5868)
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
.elseif(n = 5870)
	; 15 bytes
	pha
	tya
	ldy #202
	delay_n 24
	dey
	Jne *-7
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
.elseif(n = 5872)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	nop
	cmp $C5
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5873)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5875)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 14
	iny
	Jne *-8
	tay
	pla
.elseif(n = 5876)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 14
	iny
	Jne *-8
	tay
	pla
.elseif(n = 5877)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5878)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 5879 to 5898
.if(n = 5879 || n = 5882 || n = 5890 || n = 5891 || n = 5894)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 5880)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5881)
	; 16 bytes
	brk
	pha
	tya
	ldy #244
	delay_n 4
	nop
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5883)
	; 16 bytes
	pha
	tya
	ldy #255
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5884)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 5885)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $15A0'
	ldy #21
	cmp $C5
	brk
	iny
	Jne *-8
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
.elseif(n = 5887)
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
.elseif(n = 5888)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5889)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5892)
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
.elseif(n = 5893)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	bit $24
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5895)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5896)
	; 15 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	bit $24
	dey
	Jne *-6
	tay
	pla
	plp
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
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 14
	dey
	Jne *-8
	tay
	pla
.elseif(n = 5900)
	; 14 bytes
	pha
	tya
	ldy #0
	clc
	Jcc *+2
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5901 || n = 5928 || n = 5933)
	; 11..15 bytes
	brk
	delay_n (n-13)
.elseif(n = 5902)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5903 || n = 5905 || n = 5918 || n = 5923)
	; 10..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 5904 || n = 5914 || n = 5917 || n = 5919 || n = 5922 || n = 5924 || n = 5925 || n = 5929 || n = 5930 || n = 5935 || n = 5937)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5906)
	; 15 bytes
	pha
	tya
	pha
	ldy #203
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 5907)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5908)
	; 15 bytes
	pha
	tya
	ldy #134
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5909)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5910)
	; 15 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 9
	dey
	Jne *-8
	tay
	pla
.elseif(n = 5911)
	; 15 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	cmp $C5
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 5912)
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
.elseif(n = 5913)
	; 15 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	delay_n 4
	brk
	inx
	Jne *-8
	tax
	pla
.elseif(n = 5915)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_11_clocks_
	pla
	plp
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
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5921 || n = 5926)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5927)
	; 11 bytes
	php
	pha
	php
	lda #23
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 5931 || n = 5936)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5932)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5934)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5939)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5940)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5941 || n = 5946)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 5942 || n = 5943 || n = 5945)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5944)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 5946 && n < 5954) || (n > 5954 && n < 5959) || n = 5960 || n = 5961 || n = 5963 || (n > 5964 && n < 5987) || (n > 5987 && n < 5995) || (n > 5995 && n < 6012) || (n > 6012 && n < 6026) || (n > 6026 && n < 6037) || (n > 6037 && n < 6057) || (n > 6057 && n < 6062) || (n > 6062 && n < 6087) || (n > 6088 && n < 6094))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5954)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 5959)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 5962 || n = 5987 || n = 6012 || n = 6037 || n = 6062 || n = 6087)
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
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6100) || (n > 6100 && n < 6112) || (n > 6112 && n < 6119) || n = 6120)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
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
.elseif(n = 6122 || n = 6124)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6123)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6125)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 6126 || n = 6128)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6127)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	delay_n 4
	nop
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6129)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $2DA0'
	ldy #45
	delay_n 7
	brk
	iny
	Jne *-8
	tay
	pla
.elseif(n = 6130)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6131)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6132)
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
; 6133 to 6171
.if(n = 6133)
	; 15 bytes
	pha
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-5
	pla
	tax
	pla
.elseif(n = 6134 || n = 6139 || n = 6146 || n = 6147 || n = 6152 || n = 6164 || n = 6166 || n = 6170)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6135)
	; 15 bytes
	pha
	tya
	pha
	ldy #139
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6136)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AFA0'
	ldy #175
	delay_n 26
	dey
	Jne *-8
	tay
	pla
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
.elseif(n = 6138)
	; 15 bytes
	pha
	tya
	pha
	ldy #211
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6140 || n = 6148 || n = 6153 || n = 6165)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6141 || n = 6168)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6142)
	; 15 bytes
	php
	pha
	sec
	lda #245
	delay_n 7
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 6143)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6144)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	iny
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
.elseif(n = 6149)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-7
	pla
	plp
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
.elseif(n = 6151 || n = 6157 || n = 6169)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6154)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6155)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	brk
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
.elseif(n = 6158)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6159)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 7
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6160)
	; 15 bytes
	pha
	tya
	ldy #212
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6161)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	cmp $C5
	brk
	iny
	Jne *-8
	tay
	pla
.elseif(n = 6163)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 6167)
	; 15 bytes
	pha
	tya
	pha
	ldy #212
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6171)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 15 bytes
	pha
	tya
	ldy #140
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6173 || n = 6175 || n = 6178 || n = 6180 || n = 6181 || n = 6185 || n = 6186 || n = 6191 || n = 6193 || n = 6194)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174 || n = 6179)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6176)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6177 || n = 6182)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6183)
	; 11 bytes
	php
	pha
	php
	lda #24
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6184 || n = 6189)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6187 || n = 6192)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6188)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6190)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 6195)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6196)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6197 || n = 6202)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6198 || n = 6199 || n = 6201)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6200)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 6202 && n < 6210) || n = 6211 || n = 6213 || n = 6214 || (n > 6215 && n < 6237) || (n > 6237 && n < 6243) || (n > 6243 && n < 6251))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6210)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6212 || n = 6243)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6215)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6237)
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
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
.if((n >= 6251) && (n < 6407))
; 6251 to 6406
.if((n > 6250 && n < 6262) || (n > 6262 && n < 6274) || (n > 6274 && n < 6287) || (n > 6287 && n < 6303) || n = 6304 || (n > 6306 && n < 6312) || (n > 6312 && n < 6328) || (n > 6328 && n < 6336) || (n > 6337 && n < 6362) || (n > 6362 && n < 6367) || (n > 6367 && n < 6376))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 6303)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
	pla
.elseif(n = 6306 || n = 6379 || n = 6390 || n = 6399 || n = 6404)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 6377)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6378 || n = 6380 || n = 6385 || n = 6386 || n = 6389 || n = 6391 || n = 6400 || n = 6403 || n = 6405)
	; 13..15 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6382)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6383 || n = 6394)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6384)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6388)
	; 15 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6392)
	; 15 bytes
	pha
	tya
	ldy #220
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6393)
	; 15 bytes
	pha
	tya
	ldy #255
	delay_n 7
	brk
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6395)
	; 16 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 14
	dey
	Jne *-9
	tay
	pla
.elseif(n = 6396)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
.elseif(n = 6397)
	; 16 bytes
	brk
	delay_n (n-13)
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
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
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
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
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
.elseif(n = 6408 || n = 6419)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6409 || n = 6425 || n = 6440 || n = 6445)
	; 11..15 bytes
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
.elseif(n = 6411)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
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
.elseif(n = 6413 || n = 6416 || n = 6422 || n = 6423 || n = 6426 || n = 6429 || n = 6431 || n = 6434 || n = 6436 || n = 6437 || n = 6441 || n = 6442)
	; 10..16 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6415 || n = 6430 || n = 6435)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6417)
	; 15 bytes
	php
	pha
	sec
	lda #0
	delay_n 7
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 6418)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 7
	brk
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6420)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6421)
	; 15 bytes
	pha
	tya
	ldy #221
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6424)
	; 15 bytes
	pha
	tya
	ldy #229
	php
	cmp $C5
	plp
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6427)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6428)
	; 15 bytes
	pha
	tya
	pha
	ldy #221
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6432)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6433 || n = 6438)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6439)
	; 11 bytes
	php
	pha
	php
	lda #25
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6443)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6444)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 6446 to 6484
.if(n = 6446)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 6447 || n = 6449 || n = 6450)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6448)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6451)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6452)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6453 || n = 6458)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6454 || n = 6455 || n = 6457)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6456)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6459 || (n > 6460 && n < 6466) || (n > 6466 && n < 6471) || (n > 6471 && n < 6485))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6460)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6466)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6471)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 6485 to 6562
.if((n > 6484 && n < 6491) || (n > 6491 && n < 6522) || (n > 6522 && n < 6553) || (n > 6553 && n < 6563))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6491 || n = 6522 || n = 6553)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6584) || (n > 6584 && n < 6610) || (n > 6610 && n < 6615) || (n > 6615 && n < 6633))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 6633)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6634)
	; 14 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6635 || n = 6637)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6636)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6638)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6639)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6640)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	brk
	iny
	Jne *-8
	tay
	pla
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 4
	brk
	iny
	Jne *-8
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
.elseif(n = 6643)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6644)
	; 15 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 9
	brk
	dey
	Jne *-8
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
.elseif(n = 6646)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6647 || n = 6648 || n = 6650 || n = 6658)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6649)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6651)
	; 15 bytes
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-6
	pla
	tay
	pla
.elseif(n = 6652)
	; 15 bytes
	pha
	tya
	ldy #(n-18)/31
	delay_n 26
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6653)
	; 15 bytes
	pha
	tya
	ldy #229
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6654)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6655)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6656)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	nop
	iny
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
.elseif(n = 6659)
	; 15 bytes
	brk
	delay_n (n-13)
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 15 bytes
	pha
	tya
	pha
	ldy #229
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6661)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6662)
	; 15 bytes
	pha
	tya
	ldy #190
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6663)
	; 15 bytes
	pha
	tya
	pha
	ldy #151
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6664)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6665 || n = 6669 || n = 6670 || n = 6674 || n = 6679)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6666)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6667)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6668)
	; 15 bytes
	pha
	tya
	ldy #0
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6671)
	; 16 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
	pla
	plp
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
.elseif(n = 6673)
	; 17 bytes
	php
	pha
	sec
	lda #0
	.repeat 3
	 php
	 plp
	.endrepeat
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6675)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6676)
	; 15 bytes
	pha
	tya
	ldy #238
	php
	cmp $C5
	plp
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6677)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.endif
.endif
.else
; 6680 to 6718
.if(n = 6680 || n = 6686 || n = 6691)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6681)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6682)
	; 15 bytes
	pha
	tya
	ldy #230
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6683)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6684)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6685 || n = 6687 || n = 6690 || n = 6692 || n = 6693 || n = 6697 || n = 6698 || n = 6703 || n = 6705 || n = 6706)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6689 || n = 6694)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6695)
	; 11 bytes
	php
	pha
	php
	lda #26
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6696 || n = 6701)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6699 || n = 6704)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6700)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6702)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 6707)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6708)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6709 || n = 6714)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6710 || n = 6711 || n = 6713)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6712)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 6714 && n < 6719))
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
.endif
.else
; 6719 to 6875
.if((n > 6718 && n < 6722) || (n > 6722 && n < 6727) || (n > 6727 && n < 6739) || (n > 6739 && n < 6770) || (n > 6770 && n < 6801) || (n > 6801 && n < 6832) || (n > 6832 && n < 6863) || (n > 6863 && n < 6876))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6722)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6727)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
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
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
; 6876 to 6914
.if((n > 6875 && n < 6886) || n = 6887 || n = 6888)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 6889 || n = 6891 || n = 6911)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6890)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6892)
	; 15 bytes
	pha
	tya
	pha
	ldy #237
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6893 || n = 6901 || n = 6904)
	; 15..16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6894)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6895)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 6896 || n = 6898 || n = 6903 || n = 6910 || n = 6914)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6897)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6899)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 6900)
	; 15 bytes
	pha
	tya
	ldy #246
	php
	cmp $C5
	plp
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6902)
	; 16 bytes
	pha
	tya
	ldy #130
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6905)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C5A0'
	ldy #197
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6906)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6907)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 6908)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
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
	cmp $EA ;hides 'nop'
	delay_n 7
	iny
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
.endif
.else
; 6915 to 6953
.if(n = 6915 || n = 6923 || n = 6926 || n = 6927 || n = 6929 || n = 6934 || n = 6941 || n = 6943 || n = 6946 || n = 6948 || n = 6949 || n = 6953)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6916 || n = 6928 || n = 6942 || n = 6947)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6917)
	; 15 bytes
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	pla
.elseif(n = 6918)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 11
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6919 || n = 6920 || n = 6932)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6921)
	; 15 bytes
	pha
	tya
	pha
	ldy #238
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 6922)
	; 16 bytes
	pha
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
	pla
.elseif(n = 6924)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6925)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6930)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 6931)
	; 15 bytes
	pha
	tya
	ldy #(n-18)/31
	delay_n 26
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 6933)
	; 16 bytes
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	iny
	Jne *-7
	pla
	tay
	pla
.elseif(n = 6935)
	; 16 bytes
	pha
	tya
	ldy #161
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6936)
	; 17 bytes
	pha
	clc
	lda #$2A ;hides 'rol a'
	delay_n 1727
	Jcc *-11
	pla
.elseif(n = 6937 || n = 6938 || n = 6952)
	; 11..16 bytes
	brk
	delay_n (n-13)
.elseif(n = 6939)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6940)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C6A0'
	ldy #198
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 6944)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6945 || n = 6950)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6951)
	; 11 bytes
	php
	pha
	php
	lda #27
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954 || n = 6959 || n = 6961 || n = 6962)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6955 || n = 6960)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6956)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6957)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 6958)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 6963)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6964)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6965 || n = 6970)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 6966 || n = 6967 || n = 6969)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6968)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 6970 && n < 6978) || (n > 6978 && n < 6983) || (n > 6983 && n < 6987) || (n > 6987 && n < 7018) || (n > 7018 && n < 7032))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6978)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 6983)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 6987 || n = 7018)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7049) || (n > 7049 && n < 7080) || (n > 7080 && n < 7110))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7049 || n = 7080)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if(n = 7110 || (n > 7111 && n < 7142) || n = 7143 || n = 7144)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7111 || n = 7142)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7145 || n = 7147)
	; 14..15 bytes
	pha
	delay_n (n-3)
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
	pha
	tya
	ldy #223
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7150)
	; 15 bytes
	pha
	tya
	ldy #204 ;hides 'cpy abs'
	cmp #$A9
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7151)
	; 15 bytes
	pha
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	delay_n 26
	dex
	Jne *-8
	tax
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
.elseif(n = 7153)
	; 15 bytes
	pha
	tya
	pha
	ldy #246
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 7154 || n = 7158 || n = 7166)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7155)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 7156)
	; 15 bytes
	pha
	tya
	ldy #188
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7157)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7159)
	; 16 bytes
	pha
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
	pla
.elseif(n = 7160)
	; 16 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1187
	Jcc *-10
	pla
.elseif(n = 7161)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #55
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 7162)
	; 17 bytes
	pha
	tya
	ldy #110
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 7163)
	; 17 bytes
	pha
	tya
	pha
	ldy #152
	lda $A5
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 7164)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	cmp $C5
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7165)
	; 16 bytes
	pha
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
	pla
.elseif(n = 7167)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	delay_n 4
	nop
	dey
	Jne *-8
	tay
	pla
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
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
.elseif(n = 7170 || n = 7171 || n = 7175 || n = 7177)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7172 || n = 7176)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7173)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7174)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 7
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7178)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7179)
	; 15 bytes
	pha
	tya
	ldy #(n-18)/31
	delay_n 26
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 7180)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7181)
	; 16 bytes
	pha
	tya
	ldy #107
	delay_n 62
	dey
	Jne *-8
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
.elseif(n = 7183)
	; 16 bytes
	pha
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
	pla
.elseif(n = 7184)
	; 15 bytes
	pha
	tya
	ldy #163
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7185)
	; 15 bytes
	pha
	txa
	ldx #205 ;hides 'cmp abs'
	cmp #$A9
	delay_n 26
	dex
	Jne *-8
	tax
	pla
.elseif(n = 7186)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 7187)
	; 15 bytes
	pha
	tya
	ldy #205
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if(n = 7188)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7189 || n = 7196 || n = 7197 || n = 7199 || n = 7202 || n = 7204 || n = 7205 || n = 7209 || n = 7210 || n = 7215 || n = 7217 || n = 7218)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7190)
	; 16 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1192
	Jcc *-10
	pla
.elseif(n = 7191)
	; 15 bytes
	pha
	tya
	pha
	ldy #163
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 7192)
	; 17 bytes
	pha
	txa
	pha
	ldx #232 ;hides 'inx'
	php
	lda #248
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 7193)
	; 16 bytes
	pha
	tya
	ldy #167
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7194)
	; 15 bytes
	pha
	tya
	ldy #189
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7195)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7198 || n = 7203)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7200)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7201 || n = 7206)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7207)
	; 11 bytes
	php
	pha
	php
	lda #28
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7208 || n = 7213)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7211 || n = 7216)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7212)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7214)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 7219)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7220)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7221 || n = 7226)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7222 || n = 7223 || n = 7225)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7224)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 7226 && n < 7234) || (n > 7235 && n < 7239) || (n > 7239 && n < 7266) || (n > 7266 && n < 7297) || (n > 7297 && n < 7328) || (n > 7328 && n < 7344))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7234)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7235 || n = 7266 || n = 7297 || n = 7328)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7239)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7359) || (n > 7359 && n < 7390) || (n > 7390 && n < 7401))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7359 || n = 7390 || n = 7421)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7401 || n = 7402)
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
	pha
	delay_n (n-3)
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
.elseif(n = 7405)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 7
	brk
	iny
	Jne *-8
	tay
	pla
.elseif(n = 7406 || n = 7416)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7407)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7408)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7409)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7410)
	; 15 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	delay_n 26
	iny
	Jne *-8
	tay
	pla
.elseif(n = 7411)
	; 15 bytes
	pha
	tya
	pha
	ldy #168
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 7412)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7413)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7414)
	; 15 bytes
	pha
	tya
	pha
	ldy #255
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 7415)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #15
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 7417)
	; 16 bytes
	brk
	delay_n (n-13)
.elseif(n = 7418)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7419)
	; 16 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	delay_n 9
	brk
	iny
	Jne *-9
	tay
	pla
.elseif(n = 7420)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.endif
.else
.if((n >= 7422) && (n < 7461))
; 7422 to 7460
.if(n = 7422)
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
.elseif(n = 7423 || n = 7427 || n = 7438 || n = 7439 || n = 7441 || n = 7445 || n = 7450 || n = 7453 || n = 7455 || n = 7458 || n = 7460)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 9
	iny
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
.elseif(n = 7426)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7428)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7429)
	; 16 bytes
	pha
	tya
	pha
	ldy #190
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 7430)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7431)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7432)
	; 15 bytes
	pha
	tya
	ldy #212
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7433 || n = 7440 || n = 7446 || n = 7454 || n = 7459)
	; 10..16 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 7435)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7436)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 24
	dey
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
.elseif(n = 7442)
	; 16 bytes
	php
	pha
	sec
	lda #225
	nop
	delay_n 26
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 7443)
	; 15 bytes
	pha
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 7444)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7447)
	; 16 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	nop
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 7448)
	; 15 bytes
	pha
	tya
	ldy #169
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7449)
	; 16 bytes
	pha
	tya
	ldy #111
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7451)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7452)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
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
	delay_n 6
	delay_n (n-6)
.endif
.else
; 7461 to 7500
.if(n = 7461 || n = 7465 || n = 7466 || n = 7471 || n = 7473 || n = 7474)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 7462)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7463)
	; 11 bytes
	php
	pha
	php
	lda #29
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7464 || n = 7469)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7467 || n = 7472)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7468)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7470)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 7475)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7476)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7477 || n = 7482)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7478 || n = 7479 || n = 7481)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7480)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7483)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif((n > 7483 && n < 7490) || (n > 7490 && n < 7495) || (n > 7495 && n < 7501))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7490)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7495)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
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
.else
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
; 7501 to 7656
.if((n > 7500 && n < 7514) || (n > 7514 && n < 7545) || (n > 7545 && n < 7576) || (n > 7576 && n < 7607) || (n > 7607 && n < 7638) || (n > 7638 && n < 7651) || (n > 7651 && n < 7657))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
; 7657 to 7695
.if(n = 7657 || n = 7658)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7659 || n = 7665 || n = 7672 || n = 7684 || n = 7691 || n = 7695)
	; 14..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 7660)
	; 15 bytes
	pha
	tya
	ldy #239
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7661)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
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
.elseif(n = 7663)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7664 || n = 7670 || n = 7671 || n = 7678 || n = 7683 || n = 7685 || n = 7689 || n = 7690 || n = 7694)
	; 14..16 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7667)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 7669)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7673 || n = 7676)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7674)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7675)
	; 15 bytes
	pha
	tya
	pha
	ldy #174
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 7677)
	; 15 bytes
	pha
	tya
	ldy #219
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7679 || n = 7682)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 7680)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 7686)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 7687)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 7688)
	; 15 bytes
	pha
	tya
	ldy #202
	delay_n 7
	delay_n 26
	dey
	Jne *-7
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
.elseif(n = 7693)
	; 16 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.else
; 7696 to 7734
.if(n = 7696)
	; 15 bytes
	pha
	tya
	ldy #226
	cmp $C5
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7697)
	; 16 bytes
	pha
	tya
	ldy #145
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7698 || n = 7702 || n = 7704 || n = 7709 || n = 7711 || n = 7714 || n = 7716 || n = 7717 || n = 7721 || n = 7722 || n = 7727 || n = 7729 || n = 7730)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7699)
	; 16 bytes
	pha
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
	pla
.elseif(n = 7700)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 7703 || n = 7710 || n = 7715)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7705)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7706)
	; 15 bytes
	pha
	tya
	ldy #(n-18)/31
	delay_n 26
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 7707)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7708)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7712)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7713 || n = 7718)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7719)
	; 11 bytes
	php
	pha
	php
	lda #30
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7720 || n = 7725)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7723 || n = 7728)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7724)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7726)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 7731)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7732)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7733)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7734)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.else
; 7735 to 7812
.if(n = 7735 || n = 7737)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7736)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7738)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 7738 && n < 7746) || (n > 7746 && n < 7751) || (n > 7751 && n < 7762) || (n > 7762 && n < 7793) || (n > 7793 && n < 7813))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7746)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7751)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7762 || n = 7793)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7824) || (n > 7824 && n < 7855) || (n > 7855 && n < 7886) || (n > 7886 && n < 7891))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7906) || n = 7907 || (n > 7909 && n < 7913))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 7908 || n = 7919 || n = 7921 || n = 7928)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7909 || n = 7915 || n = 7920 || n = 7929)
	; 13..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7913 || n = 7924)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7914)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7916)
	; 15 bytes
	pha
	tya
	ldy #208
	delay_n 7
	delay_n 26
	dey
	Jne *-7
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
.elseif(n = 7918)
	; 15 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7922)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7923)
	; 15 bytes
	pha
	tya
	ldy #(n-18)/31
	delay_n 26
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 7925)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 7926)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7927)
	; 16 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
	delay_n 26
	Jne *-9
	tax
	pla
.endif
.else
; 7930 to 7968
.if(n = 7930 || n = 7934 || n = 7935 || n = 7939 || n = 7941 || n = 7945 || n = 7946 || n = 7950 || n = 7952 || n = 7958 || n = 7959 || n = 7965 || n = 7967)
	; 10..16 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7933 || n = 7944 || n = 7955)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7936)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 11
	iny
	Jne *-8
	tay
	pla
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
.elseif(n = 7938 || n = 7949 || n = 7960)
	; 15..16 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7940 || n = 7947 || n = 7951 || n = 7966)
	; 10..15 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 7953)
	; 15 bytes
	php
	pha
	sec
	lda #0
	delay_n 26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7954)
	; 15 bytes
	pha
	tya
	ldy #209
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7956)
	; 15 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	delay_n 26
	inx
	Jne *-8
	tax
	pla
.elseif(n = 7957)
	; 15 bytes
	pha
	tya
	ldy #227
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7961)
	; 15 bytes
	brk
	delay_n (n-13)
.elseif(n = 7962)
	; 16 bytes
	pha
	tya
	ldy #150
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7963)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7964)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
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
.else
; 7969 to 8125
.if(n = 7969 || n = 7974)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7970 || n = 7972 || n = 7973 || n = 7977 || n = 7978 || n = 7983 || n = 7985 || n = 7986)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7975)
	; 11 bytes
	php
	pha
	php
	lda #31
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 7976 || n = 7981)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 7979 || n = 7984)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7980)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7982)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 7987)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7988)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7989 || n = 7994)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 7990 || n = 7991 || n = 7993)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 7992)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 7994 && n < 8002) || (n > 8002 && n < 8007) || (n > 8007 && n < 8126))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8002)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8007)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
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
.if((n > 8125 && n < 8165))
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
.if((n > 8164 && n < 8169))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8169 || n = 8170)
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
.elseif(n = 8172)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8173)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8174)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8175)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8176)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8177)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8178)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8179)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8180)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8181)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8182)
	; 15 bytes
	pha
	tya
	ldy #215
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8183)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8185 || n = 8191)
	; 15..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8186)
	; 16 bytes
	pha
	tya
	ldy #121
	delay_n 62
	dey
	Jpl *-8
	tay
	pla
.elseif(n = 8187)
	; 16 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	nop
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 8188)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8189)
	; 16 bytes
	pha
	tya
	ldy #221
	delay_n 4
	nop
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8190 || n = 8194 || n = 8195 || n = 8197 || n = 8198)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8192)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 8199)
	; 15 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 26
	sbc #1
	Jne *-7
	pla
	plp
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
.elseif(n = 8201)
	; 15 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 8202)
	; 15 bytes
	pha
	tya
	ldy #234
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8203)
	; 15 bytes
	pha
	tya
	pha
	ldy #186
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if(n = 8204)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8205 || n = 8208 || n = 8221 || n = 8223 || n = 8226 || n = 8228 || n = 8229 || n = 8233 || n = 8234 || n = 8239 || n = 8241 || n = 8242)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8206)
	; 15 bytes
	pha
	tya
	ldy #241
	cmp $C5
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8207)
	; 16 bytes
	pha
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
	pla
.elseif(n = 8209)
	; 16 bytes
	pha
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
	pla
.elseif(n = 8210)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8211)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	lda $A5
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 8212)
	; 16 bytes
	pha
	tya
	ldy #205
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8213 || n = 8214 || n = 8232 || n = 8237)
	; 11..17 bytes
	brk
	delay_n (n-13)
.elseif(n = 8215)
	; 17 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	brk
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8216)
	; 17 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1363
	Jcc *-11
	pla
.elseif(n = 8217)
	; 16 bytes
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	iny
	Jne *-7
	pla
	tay
	pla
.elseif(n = 8218)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 139
	Jmi *-10
	tay
	pla
.elseif(n = 8219)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8220)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8222 || n = 8227)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8225 || n = 8230)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8231)
	; 11 bytes
	php
	pha
	php
	lda #32
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8235 || n = 8240)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8236)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8238)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.else
; 8243 to 8281
.if(n = 8243)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8244)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8245 || n = 8250)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8246 || n = 8247 || n = 8249)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8248)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 8250 && n < 8258) || (n > 8258 && n < 8263) || (n > 8263 && n < 8282))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8258)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8263)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8416) || (n > 8416 && n < 8425))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 8425 || n = 8426)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
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
.elseif(n = 8428)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8429)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8430)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8431)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8432)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8433)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8434)
	; 15 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	cmp $C5
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8435)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8436)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8437)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
; 8438 to 8476
.if(n = 8438)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8439)
	; 16 bytes
	pha
	tya
	ldy #159
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8440 || n = 8442 || n = 8450 || n = 8451 || n = 8458 || n = 8462 || n = 8464 || n = 8469 || n = 8474)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8441 || n = 8452 || n = 8457 || n = 8459 || n = 8463)
	; 14..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8443)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8444)
	; 15 bytes
	pha
	tya
	ldy #248
	cmp $C5
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8445 || n = 8455 || n = 8456)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8446)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	delay_n 26
	iny
	Jne *-8
	tay
	pla
.elseif(n = 8447)
	; 15 bytes
	pha
	tya
	ldy #241
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8448)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	brk
	iny
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
.elseif(n = 8453)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8454)
	; 15 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1051
	Jne *-10
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
.elseif(n = 8461 || n = 8473)
	; 15..16 bytes
	brk
	delay_n (n-13)
.elseif(n = 8465)
	; 16 bytes
	php
	pha
	sec
	lda #0
	nop
	delay_n 26
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8466)
	; 16 bytes
	pha
	tya
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 8467)
	; 15 bytes
	pha
	tya
	pha
	ldy #192
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 8468)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 8470)
	; 16 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 1051
	clc
	Jne *-11
	pla
.elseif(n = 8471)
	; 17 bytes
	pha
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
	pla
.elseif(n = 8472)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8475)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8476)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8479 || n = 8482 || n = 8484 || n = 8485 || n = 8489 || n = 8490 || n = 8495 || n = 8497 || n = 8498)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478 || n = 8483)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8480)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8481 || n = 8486)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8487)
	; 11 bytes
	php
	pha
	php
	lda #33
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8488 || n = 8493)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8491 || n = 8496)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8492)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8494)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 8499)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8500)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8501 || n = 8506)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8502 || n = 8503 || n = 8505)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8504)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 8506 && n < 8514) || n = 8515)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8514)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.else
; 8516 to 8593
.if((n > 8515 && n < 8519) || (n > 8519 && n < 8594))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8519)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if((n > 8593 && n < 8672))
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
.if((n >= 8672) && (n < 8711))
; 8672 to 8710
.if((n > 8671 && n < 8681))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8681 || n = 8682)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8683 || n = 8697 || n = 8708)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8684)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8685)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8686)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8687)
	; 15 bytes
	pha
	tya
	pha
	ldy #197
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 8688)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8689)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8690)
	; 15 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	delay_n 26
	inx
	Jne *-8
	tax
	pla
.elseif(n = 8691)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 26
	iny
	Jne *-8
	tay
	pla
.elseif(n = 8692)
	; 15 bytes
	pha
	tya
	ldy #248
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8693)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8694)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	cmp $C5
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8695)
	; 16 bytes
	pha
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
	pla
.elseif(n = 8696 || n = 8702 || n = 8706 || n = 8707 || n = 8709)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8698)
	; 16 bytes
	pha
	tya
	ldy #202
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8699)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	bit $24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8700)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 9
	iny
	Jne *-8
	tay
	pla
.elseif(n = 8701)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8703)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8704)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
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
	cmp $C5
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8710)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	brk
	sbc #1
	Jne *-8
	pla
	plp
.endif
.else
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
.if(n = 8711)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	brk
	iny
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 8712)
	; 16 bytes
	php
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
	plp
.elseif(n = 8713)
	; 16 bytes
	pha
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
	pla
.elseif(n = 8714)
	; 15 bytes
	pha
	tya
	ldy #229
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8715)
	; 16 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 8716)
	; 15 bytes
	pha
	tya
	ldy #0
	cmp $C5
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8717)
	; 16 bytes
	brk
	delay_n (n-13)
.elseif(n = 8718 || n = 8728 || n = 8729)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8719)
	; 16 bytes
	pha
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
	pla
.elseif(n = 8720)
	; 17 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1447
	Jcc *-11
	pla
.elseif(n = 8721)
	; 17 bytes
	pha
	tya
	pha
	ldy #229
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 8722)
	; 16 bytes
	pha
	tya
	ldy #130
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8723)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 8724)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8725)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 26
	iny
	Jne *-8
	tay
	pla
.elseif(n = 8726)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 26
	iny
	Jne *-8
	tay
	pla
.elseif(n = 8727)
	; 15 bytes
	pha
	tya
	ldy #249
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8730)
	; 16 bytes
	pha
	delay_n (n-3)
.endif
.else
; 8731 to 8750
.if(n = 8731)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8732)
	; 16 bytes
	pha
	tya
	ldy #218
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8733 || n = 8735 || n = 8738 || n = 8740 || n = 8741 || n = 8745 || n = 8746)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 8734 || n = 8739)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8736)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8737 || n = 8742)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8743)
	; 11 bytes
	php
	pha
	php
	lda #34
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8744 || n = 8749)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 8747)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8748)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8750)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
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
.if(n = 8751 || n = 8753 || n = 8754)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 8752)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8755)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8756)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8757 || n = 8762)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 8758 || n = 8759 || n = 8761)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8760)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 8762 && n < 8770) || (n > 8770 && n < 8775) || (n > 8775 && n < 8907))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8770)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8775)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 8907) && (n < 8985))
.if((n >= 8907) && (n < 8946))
; 8907 to 8945
.if((n > 8906 && n < 8937))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8937 || n = 8938)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8939 || n = 8945)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8940)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8941)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8942)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
	pla
.elseif(n = 8943)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 8944)
	; 15 bytes
	pha
	tya
	ldy #203
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8947)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8948)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8949)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8950)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8951)
	; 15 bytes
	pha
	tya
	pha
	ldy #203
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 8952 || n = 8954 || n = 8962 || n = 8963)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8953 || n = 8964)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8955)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8956)
	; 16 bytes
	pha
	tya
	ldy #208
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8957 || n = 8958)
	; 16..17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8959)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8960)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	brk
	iny
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
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8966)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8967 || n = 8968)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8969)
	; 16 bytes
	pha
	tya
	ldy #169
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8970)
	; 15 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8971)
	; 15 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8972)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 4
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8973 || n = 8974 || n = 8978 || n = 8984)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 8975 || n = 8979)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8976)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8977)
	; 17 bytes
	pha
	tya
	ldy #163
	delay_n 50
	dey
	Jne *-9
	tay
	pla
.elseif(n = 8980)
	; 15 bytes
	pha
	tya
	ldy #236
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8981)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 4
	delay_n 26
	iny
	Jne *-10
	tay
	pla
.elseif(n = 8982)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8983)
	; 16 bytes
	pha
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
	pla
.endif
.endif
.endif
.else
; 8985 to 9062
.if(n = 8985)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #79
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 8986)
	; 17 bytes
	pha
	tya
	pha
	ldy #183
	lda #$0A ;hides 'asl a'
	delay_n 4
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 8987)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 8988)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8989 || n = 8991 || n = 8994 || n = 8996 || n = 8997 || n = 9001 || n = 9002 || n = 9007 || n = 9009 || n = 9010)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 8990 || n = 8995)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8992)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 8993 || n = 8998)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8999)
	; 11 bytes
	php
	pha
	php
	lda #35
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9000 || n = 9005)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9003 || n = 9008)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9004)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9006)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 9011)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9012)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9013 || n = 9018)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9014 || n = 9015 || n = 9017)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9016)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 9018 && n < 9026) || (n > 9026 && n < 9031) || (n > 9031 && n < 9063))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9026)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9031)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
; 9063 to 9218
.if((n > 9062 && n < 9193))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9195 || n = 9209 || n = 9211 || n = 9213)
	; 15..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9196)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 9197)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9198)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9199)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9200)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9201)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9202)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9203)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9204)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9205)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9206)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	cmp $C5
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9207)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9208)
	; 15 bytes
	pha
	tya
	ldy #209
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9210)
	; 15 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 39
	iny
	Jne *-8
	tay
	pla
.elseif(n = 9212)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9214)
	; 16 bytes
	pha
	tya
	ldy #214
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9215)
	; 15 bytes
	pha
	tya
	pha
	ldy #209
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 9216)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	cmp $C5
	brk
	iny
	Jne *-8
	tay
	pla
.elseif(n = 9217)
	; 15 bytes
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	pla
.elseif(n = 9218)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
; 9219 to 9257
.if(n = 9219)
	; 16 bytes
	pha
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
	pla
.elseif(n = 9220 || n = 9231 || n = 9246 || n = 9251)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9221 || n = 9227 || n = 9230 || n = 9232 || n = 9236 || n = 9237 || n = 9245 || n = 9247 || n = 9250 || n = 9252 || n = 9253 || n = 9257)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9222)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9223)
	; 16 bytes
	pha
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
	pla
.elseif(n = 9224)
	; 16 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	delay_n 26
	iny
	Jne *-9
	tay
	pla
.elseif(n = 9225)
	; 16 bytes
	pha
	tya
	ldy #249
	delay_n 4
	nop
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9226)
	; 17 bytes
	pha
	tya
	pha
	ldy #8
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 9228)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 31
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9229 || n = 9256)
	; 11..17 bytes
	brk
	delay_n (n-13)
.elseif(n = 9233)
	; 17 bytes
	pha
	tya
	pha
	lda #<$AD ;hides 'lda $C0A0'
	ldy #192
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-8
	pla
	tay
	pla
.elseif(n = 9234)
	; 16 bytes
	pha
	tya
	ldy #174
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9235)
	; 16 bytes
	pha
	tya
	pha
	ldy #0
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 9238)
	; 17 bytes
	pha
	lda #$4A ;hides 'lsr a'
	php
	delay_n 1142
	plp
	Jne *-12
	pla
.elseif(n = 9239)
	; 17 bytes
	pha
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
	pla
.elseif(n = 9240)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #33
	lda #239
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9241)
	; 17 bytes
	pha
	tya
	pha
	ldy #159
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 9242)
	; 17 bytes
	pha
	tya
	ldy #142
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9243)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9244)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 157
	Jmi *-10
	tay
	pla
.elseif(n = 9248)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9249 || n = 9254)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9255)
	; 11 bytes
	php
	pha
	php
	lda #36
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.else
; 9258 to 9296
.if(n = 9258 || n = 9263 || n = 9265 || n = 9266)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 9259 || n = 9264)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9260)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9261)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9262)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 9267)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9268)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9269 || n = 9274)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9270 || n = 9271 || n = 9273)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9272)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 9274 && n < 9282) || (n > 9282 && n < 9287) || (n > 9287 && n < 9297))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9282)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9287)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 9297 to 9375
.if((n > 9296 && n < 9376))
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
.endif
.endif
.else
.if((n >= 9376) && (n < 9688))
.if((n >= 9376) && (n < 9532))
.if((n >= 9376) && (n < 9454))
; 9376 to 9453
.if((n > 9375 && n < 9449))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9449 || n = 9450)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9451)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9452)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 9453)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.else
.if((n >= 9454) && (n < 9493))
.if((n >= 9454) && (n < 9473))
; 9454 to 9472
.if(n = 9454)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9455)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9456)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9457)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9458)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9459)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9460)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9461)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9462)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 4
	nop
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9463)
	; 17 bytes
	pha
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
	pla
.elseif(n = 9464 || n = 9470)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9465)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9466)
	; 17 bytes
	pha
	tya
	ldy #163
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9467)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	nop
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 9468)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	iny
	Jne *-8
	tay
	pla
.elseif(n = 9469)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9471)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	nop
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9472)
	; 15 bytes
	pha
	tya
	ldy #215
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9474)
	; 15 bytes
	pha
	tya
	ldy #249
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9475 || n = 9476 || n = 9481)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9477)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9478)
	; 17 bytes
	pha
	lda #$4A ;hides 'lsr a'
	php
	delay_n 1172
	plp
	Jne *-12
	pla
.elseif(n = 9479)
	; 15 bytes
	pha
	tya
	pha
	ldy #215
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 9480 || n = 9486)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9482)
	; 16 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9483)
	; 16 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9484)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 4
	nop
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9485)
	; 17 bytes
	pha
	tya
	pha
	ldy #215
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	sta (@ptrtemp),y
	pla
	tay
	pla
.elseif(n = 9487)
	; 17 bytes
	brk
	delay_n (n-13)
.elseif(n = 9488)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9489)
	; 17 bytes
	pha
	tya
	ldy #243
	cmp $C5
	delay_n 31
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9490)
	; 17 bytes
	pha
	tya
	pha
	ldy #231
	sec
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 9491)
	; 17 bytes
	pha
	tya
	pha
	ldy #148
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 9492)
	; 16 bytes
	pha
	tya
	ldy #237
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.endif
.endif
.else
; 9493 to 9531
.if(n = 9493 || n = 9494 || n = 9497 || n = 9500 || n = 9501 || n = 9503 || n = 9506 || n = 9508 || n = 9509 || n = 9513 || n = 9514 || n = 9519 || n = 9521 || n = 9522)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495)
	; 16 bytes
	pha
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
	pla
.elseif(n = 9496)
	; 16 bytes
	pha
	tya
	pha
	ldy #243
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 9498)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9499)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9502 || n = 9507)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 9504)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9505 || n = 9510)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9511)
	; 11 bytes
	php
	pha
	php
	lda #37
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9512 || n = 9517)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9515 || n = 9520)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9516)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9518)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 9523)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9524)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9525 || n = 9530)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9526 || n = 9527 || n = 9529)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9528)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9531)
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
.endif
.else
; 9532 to 9687
.if((n > 9531 && n < 9538) || (n > 9538 && n < 9543) || (n > 9543 && n < 9688))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9538)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9543)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 9688) && (n < 9844))
.if((n >= 9688) && (n < 9766))
.if((n >= 9688) && (n < 9727))
; 9688 to 9726
.if((n > 9687 && n < 9705))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9705 || n = 9706)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9707 || n = 9721)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 9708)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 9709)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9710)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9711)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9712)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9713)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9714)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9715)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9716)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9717)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9718)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9719)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9720 || n = 9722 || n = 9726)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9723)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9724)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9725)
	; 16 bytes
	pha
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
	pla
.endif
.else
; 9727 to 9765
.if(n = 9727 || n = 9732 || n = 9746 || n = 9756 || n = 9758 || n = 9763)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9728)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	clc
	Jcc *+2
	brk
	iny
	Jne *-9
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
.elseif(n = 9730)
	; 16 bytes
	pha
	tya
	pha
	ldy #249
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 9731 || n = 9733 || n = 9738 || n = 9742 || n = 9745 || n = 9752 || n = 9757 || n = 9759 || n = 9762 || n = 9764 || n = 9765)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 9734)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	brk
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9735)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk
	iny
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 9736)
	; 15 bytes
	pha
	tya
	ldy #221
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9737)
	; 16 bytes
	brk
	delay_n (n-13)
.elseif(n = 9739)
	; 16 bytes
	pha
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
	pla
.elseif(n = 9740)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9741)
	; 17 bytes
	pha
	tya
	ldy #207
	pha
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
	pla
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9743)
	; 15 bytes
	pha
	tya
	pha
	ldy #221
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 9744)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #32
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 9747)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 9748)
	; 17 bytes
	pha
	tya
	pha
	ldy #207
	lda $A5
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 9749)
	; 17 bytes
	pha
	tya
	ldy #91
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9750)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9751)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	delay_n 1212
	Jcc *-11
	pla
	plp
.elseif(n = 9753)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9754)
	; 16 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 39
	Jne *-9
	tax
	pla
.elseif(n = 9755)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_11_clocks_
	pla
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
	delay_n 6
	delay_n (n-6)
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
	pha
	php
	lda #38
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9768 || n = 9773)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 9769 || n = 9770 || n = 9775 || n = 9777 || n = 9778)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 9771 || n = 9776)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9772)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9774)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 9779)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9780)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9781 || n = 9786)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 9782 || n = 9783 || n = 9785)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9784)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 9786 && n < 9794) || (n > 9794 && n < 9799) || (n > 9799 && n < 9844))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9794)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 9799)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
; 9844 to 9921
.if((n > 9843 && n < 9922))
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
.if((n >= 9922) && (n < 9961))
; 9922 to 9960
.if((n > 9921 && n < 9961))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9963 || n = 9977)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 9964)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 9965)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9966)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9967)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 9968)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9969)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9970)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9971)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9972)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9973)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9974)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9975)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 9976)
	; 16 bytes
	pha
	tya
	ldy #188
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9978)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9979)
	; 16 bytes
	pha
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
	pla
.elseif(n = 9980)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 14
	iny
	Jne *-9
	tay
	pla
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 17 bytes
	pha
	tya
	pha
	ldy #33
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 9982)
	; 17 bytes
	pha
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
	pla
.elseif(n = 9983)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	delay_n 4
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9984)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	nop
	brk
	iny
	Jne *-9
	tay
	pla
.elseif(n = 9985)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	.repeat 3
	 php
	 plp
	.endrepeat
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9986)
	; 17 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
	pla
	iny
	Jne *-10
	tay
	pla
.elseif(n = 9987)
	; 17 bytes
	pha
	tya
	ldy #175
	delay_n 52
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9988)
	; 16 bytes
	pha
	tya
	ldy #232
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9989)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9990 || n = 9997)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9991)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 9992)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #136
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 9993)
	; 17 bytes
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	sec
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 9994)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9995)
	; 16 bytes
	pha
	tya
	ldy #149
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9996)
	; 17 bytes
	pha
	tya
	ldy #0
	cmp $C5
	delay_n 31
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9998)
	; 17 bytes
	pha
	lda #$4A ;hides 'lsr a'
	php
	delay_n 1237
	plp
	Jne *-12
	pla
.elseif(n = 9999)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #141
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 10000)
	; 15 bytes
	pha
	tya
	ldy #227
	delay_n 39
	dey
	Jne *-7
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
	pha
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
	pla
.elseif(n = 10002 || n = 10004 || n = 10005 || n = 10009 || n = 10010 || n = 10013 || n = 10015 || n = 10018 || n = 10020 || n = 10021 || n = 10025 || n = 10026 || n = 10031 || n = 10033 || n = 10034)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10003)
	; 16 bytes
	pha
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
	pla
.elseif(n = 10006)
	; 17 bytes
	pha
	tya
	ldy #227
	delay_n 39
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 10007)
	; 15 bytes
	pha
	tya
	pha
	ldy #227
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 10008)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10011)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10012)
	; 16 bytes
	pha
	tya
	ldy #250
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10014 || n = 10019)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10016)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10017 || n = 10022)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10023)
	; 11 bytes
	php
	pha
	php
	lda #39
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10024 || n = 10029)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10027 || n = 10032)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10028)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10030)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 10035)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10036)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10037 || n = 10042)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10038 || n = 10039 || n = 10041)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10040)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 10042 && n < 10050) || (n > 10050 && n < 10055) || (n > 10055 && n < 10157))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10050)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10055)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10217))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10217 || n = 10218)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10219 || n = 10233)
	; 15..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 10220)
	; 15 bytes
	pha
	tya
	ldy #232
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10221)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10222)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10223)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10224)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10225)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10226)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10227)
	; 15 bytes
	pha
	tya
	pha
	ldy #232
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 10228)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10229)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10230)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 9
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10231)
	; 16 bytes
	pha
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
	pla
.elseif(n = 10232)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10234)
	; 16 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.endif
.else
.if((n >= 10235) && (n < 10274))
.if((n >= 10235) && (n < 10254))
; 10235 to 10253
.if(n = 10235)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 9
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 10236)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	brk
	iny
	Jne *-8
	tay
	pla
.elseif(n = 10237 || n = 10247)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10238 || n = 10242 || n = 10243 || n = 10248)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10239 || n = 10244)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 10240)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 7
	brk
	iny
	Jne *-8
	tay
	pla
.elseif(n = 10241)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10245)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	brk
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 10246)
	; 16 bytes
	pha
	tya
	ldy #238
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10249)
	; 17 bytes
	pha
	tya
	pha
	ldy #9
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 10250)
	; 17 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 31
	dey
	Jne *-10
	tay
	pla
.elseif(n = 10251)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 31
	dey
	Jne *-10
	tay
	pla
.elseif(n = 10252)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10253)
	; 17 bytes
	pha
	tya
	ldy #209
	delay_n 44
	dey
	Jne *-9
	tay
	pla
.endif
.else
; 10254 to 10273
.if(n = 10254 || n = 10265 || n = 10266 || n = 10268 || n = 10269 || n = 10271)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10255 || n = 10270)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10256)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10257)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #215
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 10258)
	; 17 bytes
	pha
	tya
	ldy #218
	pha
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
	pla
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10259)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 10260)
	; 17 bytes
	pha
	tya
	pha
	ldy #76
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 10261)
	; 17 bytes
	pha
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
	pla
.elseif(n = 10262)
	; 17 bytes
	pha
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
	pla
.elseif(n = 10263)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10264)
	; 15 bytes
	pha
	tya
	ldy #233
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10267)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_11_clocks_
	pla
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
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10276 || n = 10277 || n = 10281 || n = 10282 || n = 10287 || n = 10289 || n = 10290)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10278)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10279)
	; 11 bytes
	php
	pha
	php
	lda #40
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10280 || n = 10285)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10283 || n = 10288)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10284)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10286)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 10291)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10292)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10293 || n = 10298)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10294 || n = 10295 || n = 10297)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10296)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 10298 && n < 10306) || (n > 10306 && n < 10311) || n = 10312)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10306)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10311)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
; 10313 to 10468
.if((n > 10312 && n < 10469))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10475 || n = 10487)
	; 15..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 10476)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 10477)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10478)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10479)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10480)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10481)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10482)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10483)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10484)
	; 15 bytes
	pha
	tya
	ldy #238
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10485)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10486)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
; 10488 to 10507
.if(n = 10488 || n = 10494 || n = 10505)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10489)
	; 17 bytes
	php
	pha
	sec
	lda #238
	delay_n 39
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 10490)
	; 17 bytes
	pha
	tya
	ldy #238
	delay_n 39
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 10491)
	; 15 bytes
	pha
	tya
	pha
	ldy #238
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 10492)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	cmp $C5
	brk
	iny
	Jne *-9
	tay
	pla
.elseif(n = 10493)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 10495)
	; 17 bytes
	pha
	tya
	pha
	ldy #27
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 10496)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
	delay_n 7
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10497)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	php
	cmp $C5
	plp
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10498)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10499)
	; 17 bytes
	pha
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
	pla
.elseif(n = 10500)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10501)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #40
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10502)
	; 15 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1307
	Jne *-10
	pla
.elseif(n = 10503)
	; 17 bytes
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda $A5
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 10504)
	; 16 bytes
	pha
	tya
	ldy #244
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10506)
	; 16 bytes
	pha
	tya
	ldy #198
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10507)
	; 16 bytes
	pha
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
	pla
.endif
.endif
.else
; 10508 to 10546
.if(n = 10508)
	; 17 bytes
	pha
	tya
	ldy #0
	pha
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
	pla
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10509 || n = 10514)
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10510)
	; 17 bytes
	pha
	tya
	ldy #181
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10511)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	delay_n 7
	delay_n 26
	iny
	Jne *-10
	tay
	pla
.elseif(n = 10512)
	; 17 bytes
	pha
	tya
	ldy #74
	delay_n 135
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 10513)
	; 17 bytes
	pha
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
	pla
.elseif(n = 10515)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	sec
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 10516)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BFA0'
	ldy #191
	delay_n 46
	dey
	Jne *-10
	tay
	pla
.elseif(n = 10517)
	; 17 bytes
	pha
	tya
	pha
	ldy #181
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 10518)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10519)
	; 16 bytes
	pha
	tya
	pha
	ldy #175
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 10520 || n = 10522 || n = 10525 || n = 10527 || n = 10530 || n = 10532 || n = 10533 || n = 10537 || n = 10538 || n = 10543 || n = 10545 || n = 10546)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10521 || n = 10526 || n = 10531)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 10523)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10524)
	; 17 bytes
	pha
	tya
	ldy #219
	delay_n 43
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10528)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10529 || n = 10534)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10535)
	; 11 bytes
	php
	pha
	php
	lda #41
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10536 || n = 10541)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10539 || n = 10544)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10540)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10542)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.endif
.else
; 10547 to 10625
.if(n = 10547)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10548)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10549 || n = 10554)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10550 || n = 10551 || n = 10553)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10552)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 10554 && n < 10562) || (n > 10562 && n < 10567) || (n > 10567 && n < 10626))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10562)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10567)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
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
.if((n >= 10704) && (n < 10743))
; 10704 to 10742
.if((n > 10703 && n < 10729))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10729 || n = 10730)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10731)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10732)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 10733)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10734)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10735)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10736)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10737)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10738)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10739)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10740)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10741)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10742)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 11
	dey
	Jne *-9
	tay
	pla
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10744 || n = 10756 || n = 10758)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10745)
	; 17 bytes
	pha
	tya
	pha
	ldy #173
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 10746)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #160
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10747)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10748)
	; 15 bytes
	pha
	tya
	ldy #244
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10749)
	; 17 bytes
	pha
	tya
	pha
	ldy #185
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 10750 || n = 10754 || n = 10757 || n = 10761)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10751)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	delay_n 7
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10752)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 9
	brk
	iny
	Jne *-9
	tay
	pla
.elseif(n = 10753)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 11
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10755)
	; 15 bytes
	pha
	tya
	pha
	ldy #244
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 10759)
	; 16 bytes
	pha
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
	pla
.elseif(n = 10760)
	; 16 bytes
	pha
	tya
	pha
	ldy #0 ;hides 'brk imm'
	nop
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-7
	pla
	tay
	pla
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 16 bytes
	pha
	tya
	ldy #250
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10763)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	delay_n 26
	dey
	Jne *-10
	tay
	pla
.elseif(n = 10764)
	; 17 bytes
	pha
	tya
	ldy #224
	delay_n 43
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10765)
	; 17 bytes
	pha
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
	pla
.elseif(n = 10766 || n = 10780)
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10767)
	; 17 bytes
	pha
	tya
	ldy #239
	delay_n 40
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10768)
	; 17 bytes
	brk
	delay_n (n-13)
.elseif(n = 10769)
	; 17 bytes
	pha
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
	pla
.elseif(n = 10770)
	; 17 bytes
	pha
	tya
	pha
	lda <$AD ;hides 'lda $E0A0'
	ldy #224
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-8
	pla
	tay
	pla
.elseif(n = 10771)
	; 16 bytes
	pha
	tya
	ldy #203
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10772)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10773)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 10774)
	; 15 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10775 || n = 10778 || n = 10781)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10776)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10777)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 10779)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782 || n = 10787)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10783 || n = 10786 || n = 10788 || n = 10789 || n = 10793 || n = 10794 || n = 10799 || n = 10801 || n = 10802)
	; 10..13 bytes
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
.elseif(n = 10785 || n = 10790)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10791)
	; 11 bytes
	php
	pha
	php
	lda #42
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10792 || n = 10797)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 10795 || n = 10800)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10796)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10798)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 10803)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10804)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10805 || n = 10810)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 10806 || n = 10807 || n = 10809)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10808)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 10810 && n < 10818) || (n > 10818 && n < 10823) || (n > 10823 && n < 10938))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10818)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 10823)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if((n > 10937 && n < 10977))
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
.if((n >= 10977) && (n < 10996))
; 10977 to 10995
.if((n > 10976 && n < 10985))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10987)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 10988)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 10989)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10990)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10991)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 10992)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10993)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10994)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10995)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10997)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 10998)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10999)
	; 16 bytes
	pha
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
	pla
.elseif(n = 11000)
	; 16 bytes
	pha
	tya
	ldy #164
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11001 || n = 11015)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11002 || n = 11008 || n = 11011 || n = 11014)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11003)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	delay_n 39
	iny
	Jne *-10
	tay
	pla
.elseif(n = 11004)
	; 17 bytes
	pha
	tya
	ldy #229
	delay_n 43
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11005)
	; 17 bytes
	pha
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
	pla
.elseif(n = 11006)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11007)
	; 18 bytes
	php
	delay_n 29
	lda #170
	jsr delay_a_25_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11009)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11010)
	; 16 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 48
	iny
	Jne *-9
	tay
	pla
.elseif(n = 11012)
	; 15 bytes
	pha
	tya
	ldy #250
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11013)
	; 16 bytes
	pha
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
	pla
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
; 11016 to 11054
.if(n = 11016 || n = 11021 || n = 11023 || n = 11024 || n = 11030 || n = 11037 || n = 11039 || n = 11042 || n = 11044 || n = 11045 || n = 11049 || n = 11050)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11017)
	; 17 bytes
	pha
	tya
	pha
	ldy #234
	lda $A5
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 11018)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #154
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 11019)
	; 15 bytes
	pha
	tya
	pha
	ldy #250
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 11020)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 38
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11022 || n = 11031 || n = 11038 || n = 11043)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11025)
	; 17 bytes
	pha
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	dey ;first loop only
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 11026)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11027)
	; 17 bytes
	pha
	tya
	pha
	ldy #172
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 11028)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11029)
	; 18 bytes
	pha
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
	pla
.elseif(n = 11032)
	; 17 bytes
	pha
	tya
	ldy #190
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11033)
	; 17 bytes
	pha
	tya
	ldy #102
	delay_n 102
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 11034)
	; 17 bytes
	pha
	tya
	ldy #(n-18)/51
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 11035)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11036)
	; 16 bytes
	pha
	tya
	ldy #208
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11041 || n = 11046)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11047)
	; 11 bytes
	php
	pha
	php
	lda #43
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11048 || n = 11053)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11051)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11052)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11054)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.else
; 11055 to 11093
.if(n = 11055 || n = 11057 || n = 11058)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11056)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11059)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11060)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11061 || n = 11066)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11062 || n = 11063 || n = 11065)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11064)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 11066 && n < 11074) || (n > 11074 && n < 11079) || (n > 11079 && n < 11094))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11074)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11079)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 11094 to 11250
.if((n > 11093 && n < 11241))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11241 || n = 11242)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11243)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11244)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 11245)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11246)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11247)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11248)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11249)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11250)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
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
; 11251 to 11289
.if(n = 11251)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11252)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11253)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11254)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11255)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11256 || n = 11258 || n = 11263 || n = 11267 || n = 11269 || n = 11273 || n = 11274 || n = 11278 || n = 11280 || n = 11284 || n = 11285 || n = 11288)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11257 || n = 11268 || n = 11275 || n = 11279 || n = 11286)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 11259)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 11260)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	brk
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 11261 || n = 11272)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11262)
	; 17 bytes
	pha
	tya
	ldy #250
	delay_n 40
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11264)
	; 17 bytes
	pha
	tya
	ldy #194
	delay_n 53
	dey
	Jne *-9
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
.elseif(n = 11266 || n = 11277)
	; 17 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11270)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 11271)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 26
	iny
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 11276)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11281)
	; 17 bytes
	php
	pha
	sec
	lda #0
	delay_n 39
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 11282)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11283)
	; 15 bytes
	pha
	tya
	pha
	ldy #0
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
.elseif(n = 11287)
	; 17 bytes
	pha
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
	pla
.elseif(n = 11289)
	; 16 bytes
	pha
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
	pla
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11291)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11292)
	; 17 bytes
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 11293 || n = 11295 || n = 11298 || n = 11300 || n = 11301 || n = 11305 || n = 11306 || n = 11311 || n = 11313 || n = 11314)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11294 || n = 11299)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 11296)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11297 || n = 11302)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11303)
	; 11 bytes
	php
	pha
	php
	lda #44
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11304 || n = 11309)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11307 || n = 11312)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11308)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11310)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 11315)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11316)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11317 || n = 11322)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11318 || n = 11319 || n = 11321)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11320)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 11322 && n < 11329))
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
; 11329 to 11406
.if(n = 11329 || (n > 11330 && n < 11335) || (n > 11335 && n < 11407))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11330)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11335)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if((n > 11406 && n < 11485))
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
.if((n >= 11485) && (n < 11524))
; 11485 to 11523
.if((n > 11484 && n < 11497))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11499)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11500)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 11501)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11502)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11503)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11504)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11505)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11506)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11507)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11508)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11509)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11510)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 14
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11511)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11512)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11513)
	; 16 bytes
	pha
	tya
	ldy #217
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11514)
	; 17 bytes
	pha
	tya
	ldy #213
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11515 || n = 11522 || n = 11523)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11516)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	brk
	iny
	Jne *-9
	tay
	pla
.elseif(n = 11517)
	; 17 bytes
	pha
	tya
	ldy #177
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11518)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 79
	Jne *-10
	tay
	pla
.elseif(n = 11519)
	; 16 bytes
	pha
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
	pla
.elseif(n = 11520)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	jsr @nearby_rts
	brk
	iny
	Jne *-9
	tay
	pla
.elseif(n = 11521)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 27
	dey
	Jne *-9
	tay
	pla
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 197
	Jmi *-10
	tay
	pla
.elseif(n = 11525)
	; 17 bytes
	pha
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
	pla
.elseif(n = 11526)
	; 17 bytes
	pha
	tya
	ldy #202
	delay_n 52
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11527)
	; 17 bytes
	pha
	tya
	ldy #245
	pha
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
	pla
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11528)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11529)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #44
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11530)
	; 17 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 39
	inx
	Jne *-10
	tax
	pla
.elseif(n = 11531)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #163
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 11532)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 40
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11533)
	; 17 bytes
	pha
	tya
	pha
	ldy #202
	brk
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 11534)
	; 17 bytes
	pha
	tya
	pha
	ldy #245
	lda $A5
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 11535)
	; 17 bytes
	pha
	tya
	ldy #167
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11536)
	; 16 bytes
	pha
	tya
	ldy #172
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11537)
	; 17 bytes
	pha
	tya
	pha
	ldy #181 ;hides 'lda zp,x'
	dey ;first loop only
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-8
	pla
	tay
	pla
.elseif(n = 11538)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11539)
	; 16 bytes
	pha
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
	pla
.elseif(n = 11540 || n = 11542)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11541)
	; 16 bytes
	pha
	delay_n (n-3)
.endif
.else
; 11543 to 11562
.if(n = 11543)
	; 17 bytes
	pha
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
	pla
.elseif(n = 11544)
	; 17 bytes
	pha
	tya
	ldy #(n-18)/51
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 11545 || n = 11546)
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11547)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11548)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11549 || n = 11551 || n = 11554 || n = 11556 || n = 11557 || n = 11561 || n = 11562)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11550 || n = 11555)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 11552)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11553 || n = 11558)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11559)
	; 11 bytes
	php
	pha
	php
	lda #45
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11560)
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563 || n = 11568)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11564)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11565)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11566)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 11567 || n = 11569 || n = 11570)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11571)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11572)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11573 || n = 11578)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11574 || n = 11575 || n = 11577)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11576)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 11578 && n < 11586) || (n > 11586 && n < 11591) || (n > 11591 && n < 11719))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11586)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11591)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 11719) && (n < 11797))
.if((n >= 11719) && (n < 11758))
; 11719 to 11757
.if((n > 11718 && n < 11753))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11755)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 11756)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 11757)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.else
.if((n >= 11758) && (n < 11777))
; 11758 to 11776
.if(n = 11758)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11759)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11760)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11761)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11762)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11763)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11764)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11765)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11766)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	brk
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11767)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11768)
	; 16 bytes
	pha
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
	pla
.elseif(n = 11769)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11770)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11771)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	brk
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 11772)
	; 17 bytes
	pha
	tya
	ldy #84
	delay_n 135
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11773)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11774)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #178
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11775)
	; 18 bytes
	php
	delay_n 29
	lda #170
	jsr delay_a_25_clocks
	lda #(n-255)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11776)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 26
	iny
	Jne *-8
	tay
	pla
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11778)
	; 16 bytes
	pha
	tya
	ldy #222
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11779)
	; 16 bytes
	pha
	tya
	pha
	ldy #196
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 11780)
	; 16 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11781)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11782)
	; 17 bytes
	pha
	tya
	ldy #214
	delay_n 50
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11783 || n = 11785)
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11784)
	; 16 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 53
	Jne *-9
	tax
	pla
.elseif(n = 11786)
	; 17 bytes
	pha
	tya
	ldy #203
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11787 || n = 11796)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 11788)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 41
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11789)
	; 17 bytes
	brk
	delay_n (n-13)
.elseif(n = 11790)
	; 17 bytes
	pha
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
	pla
.elseif(n = 11791)
	; 17 bytes
	pha
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
	pla
.elseif(n = 11792)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 81
	Jne *-10
	tay
	pla
.elseif(n = 11793)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11794)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11795)
	; 16 bytes
	pha
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
	pla
.endif
.endif
.endif
.else
; 11797 to 11875
.if(n = 11797)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #116
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 11798)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/71
	delay_n 66
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11799)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #47
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 11800 || n = 11802)
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11801)
	; 18 bytes
	php
	pha
	tya
	ldy #85
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 11803)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11804)
	; 16 bytes
	pha
	tya
	ldy #176
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11805 || n = 11807 || n = 11810 || n = 11812 || n = 11813 || n = 11817 || n = 11818 || n = 11823 || n = 11825 || n = 11826)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11806 || n = 11811)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 11808)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11809 || n = 11814)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11815)
	; 11 bytes
	php
	pha
	php
	lda #46
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 11816 || n = 11821)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 11819 || n = 11824)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11820)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11822)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 11827)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11828)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11829 || n = 11834)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 11830 || n = 11831 || n = 11833)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11832)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 11834 && n < 11842) || (n > 11842 && n < 11847) || (n > 11847 && n < 11876))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 11842)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 11847)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
; 11876 to 12031
.if((n > 11875 && n < 12009))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12011 || n = 12024)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12012)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 12013)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12014)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12015)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12016)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12017)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12018)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12019)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12020)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12021)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12022)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	cmp $C5
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12023)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12025)
	; 17 bytes
	pha
	tya
	pha
	ldy #207
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12026)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12027)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12028)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 9
	brk
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12029)
	; 17 bytes
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 12030)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #53
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 12031)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/101
	delay_n 96
	dey
	Jne *-9
	tay
	pla
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if(n = 12032)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $C5C5 ;hides 'cmp zp'
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12033)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12034 || n = 12035 || n = 12045 || n = 12049 || n = 12050)
	; 16..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 12036)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12037)
	; 17 bytes
	pha
	tya
	ldy #185
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12038)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #231
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12039)
	; 17 bytes
	pha
	tya
	ldy #211
	delay_n 52
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12040)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12041)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #135
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12042)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #46
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12043)
	; 16 bytes
	pha
	tya
	ldy #227
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 12044)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #172
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 12046)
	; 17 bytes
	pha
	tya
	pha
	ldy #211
	brk
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12047)
	; 17 bytes
	pha
	tya
	pha
	ldy #31
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12048)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 12051 to 12070
.if(n = 12051 || n = 12062 || n = 12067)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12052)
	; 17 bytes
	pha
	tya
	ldy #85
	delay_n 135
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 12053)
	; 17 bytes
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 12054)
	; 17 bytes
	pha
	tya
	ldy #223
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12055)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12056)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 46
	dey
	Jne *-10
	tay
	pla
.elseif(n = 12057)
	; 17 bytes
	pha
	tya
	ldy #219
	delay_n 50
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12058)
	; 17 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 39
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12059)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12060)
	; 17 bytes
	pha
	tya
	ldy #251
	delay_n 43
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12061 || n = 12063 || n = 12066 || n = 12068 || n = 12069)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12065 || n = 12070)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 12071 to 12109
.if(n = 12071)
	; 11 bytes
	php
	pha
	php
	lda #47
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12072 || n = 12077)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12073 || n = 12074 || n = 12079 || n = 12081 || n = 12082)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 12075 || n = 12080)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12076)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12078)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 12083)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12084)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12085 || n = 12090)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12086 || n = 12087 || n = 12089)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12088)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 12090 && n < 12098) || (n > 12098 && n < 12103) || (n > 12103 && n < 12110))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12098)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12103)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 12110 to 12187
.if((n > 12109 && n < 12188))
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
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12265))
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
.elseif(n = 12267 || n = 12280)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12268)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 12269)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12270)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12271)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12272)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12273)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12274)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12275)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12276)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12277)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12278)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 4
	brk
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12279)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12281)
	; 17 bytes
	pha
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
	pla
.elseif(n = 12282)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #193
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 12283)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12284)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #47
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12286)
	; 18 bytes
	php
	pha
	tya
	ldy #141
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12287)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12288)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-9
	tay
	pla
.elseif(n = 12289)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12290 || n = 12291)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12292)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12293)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #141
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12294)
	; 17 bytes
	pha
	tya
	ldy #178
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12295)
	; 17 bytes
	pha
	tya
	pha
	ldy #12
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12296)
	; 17 bytes
	pha
	tya
	ldy #148
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12297)
	; 17 bytes
	pha
	tya
	ldy #189
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12298)
	; 17 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 39
	dey
	Jne *-10
	tay
	pla
.elseif(n = 12299)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 39
	dey
	Jne *-10
	tay
	pla
.elseif(n = 12300)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 43
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12301)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #99
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12302)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #92
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12303)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 12304)
	; 18 bytes
	php
	pha
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
	pla
	plp
.endif
.endif
.else
; 12305 to 12343
.if(n = 12305 || n = 12317 || n = 12319 || n = 12322 || n = 12324 || n = 12325 || n = 12329 || n = 12330 || n = 12335 || n = 12337 || n = 12338)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 12306)
	; 15 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 46
	iny
	Jne *-8
	tay
	pla
.elseif(n = 12307)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	sec
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12308)
	; 16 bytes
	pha
	tya
	ldy #232
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 12309 || n = 12318 || n = 12323)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12310 || n = 12313)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12311)
	; 17 bytes
	pha
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
	pla
.elseif(n = 12312)
	; 17 bytes
	pha
	tya
	ldy #40
	delay_n 295
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 12314)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #16
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12315)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12316)
	; 17 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	lda $EA ;hides 'nop'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12321 || n = 12326)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12327)
	; 11 bytes
	php
	pha
	php
	lda #48
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12328 || n = 12333)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12331 || n = 12336)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12332)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12334)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 12339)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12340)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12341)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12342 || n = 12343)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.endif
.else
; 12344 to 12500
.if(n = 12344)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12345)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12346)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 12346 && n < 12354) || (n > 12354 && n < 12359) || (n > 12359 && n < 12501))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12354)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12359)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
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
.if((n > 12500 && n < 12521))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12523 || n = 12536)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12524)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 12525)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12526)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12527)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12528)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12529)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12530)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12531)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12532)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12533)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12534)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12535)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12537)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12538)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12539)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #97
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.else
.if((n >= 12540) && (n < 12559))
; 12540 to 12558
.if(n = 12540)
	; 17 bytes
	pha
	tya
	ldy #216
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12541)
	; 16 bytes
	pha
	tya
	ldy #187
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 12542)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #52
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12543)
	; 17 bytes
	pha
	tya
	pha
	ldy #202
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12544)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	cmp $C5
	delay_n 26
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12545)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 31
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12546)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #48
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12547)
	; 17 bytes
	pha
	tya
	pha
	ldy #216
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12548 || n = 12553)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12549)
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #31
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12550)
	; 15 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1563
	Jne *-10
	pla
.elseif(n = 12551)
	; 17 bytes
	pha
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
	pla
.elseif(n = 12552)
	; 16 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1563
	Jne *-10
	pla
.elseif(n = 12554)
	; 17 bytes
	pha
	sec
	sec
	lda #$0A ;hides 'asl a'
	delay_n 1563
	Jne *-10
	pla
.elseif(n = 12555)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12556)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12557)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12558)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 16 bytes
	pha
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
	pla
.elseif(n = 12560 || n = 12562 || n = 12573 || n = 12575 || n = 12578)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12561 || n = 12569 || n = 12574)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12563)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	delay_n 4
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12564)
	; 17 bytes
	pha
	tya
	ldy #(n-18)/51
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 12565)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12566)
	; 16 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 1563
	clc
	Jne *-11
	pla
.elseif(n = 12567)
	; 17 bytes
	pha
	tya
	ldy #81
	delay_n 150
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12568)
	; 17 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	delay_n 1563
	clc
	Jne *-11
	pla
.elseif(n = 12570)
	; 17 bytes
	pha
	tya
	ldy #182
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12571)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12572)
	; 17 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 2089
	Jcc *-11
	pla
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
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12580 || n = 12581 || n = 12585 || n = 12586 || n = 12591 || n = 12593 || n = 12594)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 12582)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12583)
	; 11 bytes
	php
	pha
	php
	lda #49
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12584 || n = 12589)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12587 || n = 12592)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12588)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12590)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 12595)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12596)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12597 || n = 12602)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12598 || n = 12599 || n = 12601)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12600)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 12602 && n < 12610) || (n > 12610 && n < 12615) || (n > 12615 && n < 12657))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12610)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12615)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
; 12657 to 12734
.if((n > 12656 && n < 12735))
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
.if((n >= 12735) && (n < 12774))
; 12735 to 12773
.if((n > 12734 && n < 12774))
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
.elseif(n = 12777 || n = 12778)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12779 || n = 12792)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12780)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 12781)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12782)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12783)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12784)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12785)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12786)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12787)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12788)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12789)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12790)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12791)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12794)
	; 17 bytes
	pha
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
	pla
.elseif(n = 12795)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #112
	lda #81
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12796)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	brk
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12797)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #97
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12798)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12799)
	; 16 bytes
	pha
	tya
	pha
	ldy #213
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 12800)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 4
	delay_n 26
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12801 || n = 12811)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12802)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12803)
	; 17 bytes
	pha
	tya
	pha
	ldy #136
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12804)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12805)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #49
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12806)
	; 17 bytes
	pha
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
	pla
.elseif(n = 12807)
	; 18 bytes
	php
	pha
	tya
	ldy #139
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12808)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 12809)
	; 16 bytes
	pha
	tya
	ldy #191
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 12810)
	; 17 bytes
	pha
	tya
	ldy #237
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12812)
	; 17 bytes
	pha
	tya
	ldy #200
	delay_n 59
	dey
	Jne *-9
	tay
	pla
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
	pha
	tya
	ldy #(n-12)/51
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 12814)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 12815 || n = 12818 || n = 12822 || n = 12829 || n = 12831 || n = 12834 || n = 12836 || n = 12837 || n = 12841 || n = 12842 || n = 12847 || n = 12849 || n = 12850)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 12816 || n = 12830 || n = 12835)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 12817)
	; 17 bytes
	pha
	tya
	ldy #197
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12819)
	; 17 bytes
	pha
	tya
	pha
	ldy #200
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 12820)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 12821)
	; 17 bytes
	pha
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
	pla
.elseif(n = 12823)
	; 17 bytes
	pha
	tya
	pha
	ldy #32
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 12824)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #237
	lda #22
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12825)
	; 17 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 46
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12826)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 46
	iny
	Jne *-10
	tay
	pla
.elseif(n = 12827)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12828)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12832)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12833 || n = 12838)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12839)
	; 11 bytes
	php
	pha
	php
	lda #50
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 12840 || n = 12845)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 12843 || n = 12848)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12844)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12846)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 12851)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.else
; 12852 to 12890
.if(n = 12852)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12853 || n = 12858)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 12854 || n = 12855 || n = 12857)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12856)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 12858 && n < 12866) || (n > 12866 && n < 12871) || (n > 12871 && n < 12891))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 12866)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 12871)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 12891 to 12968
.if((n > 12890 && n < 12969))
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
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 13033))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13035)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13036)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 13037)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13038)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13039)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13040)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13041)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13042)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13043)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13044)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13045)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13046)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 7
	brk
	dey
	Jne *-9
	tay
	pla
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13048 || n = 13049)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13050)
	; 16 bytes
	pha
	tya
	ldy #246
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 13051)
	; 18 bytes
	php
	pha
	tya
	ldy #181
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13052)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 26
	iny
	Jne *-9
	tay
	pla
.elseif(n = 13053)
	; 17 bytes
	pha
	tya
	pha
	ldy #98
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13054 || n = 13059)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13055)
	; 17 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	pla
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 13056)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 31
	iny
	Jne *-10
	tay
	pla
.elseif(n = 13057)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13058)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #(n-18)/326
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13060)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13061)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #237
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13062)
	; 17 bytes
	pha
	tya
	ldy #225
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13063)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 224
	Jmi *-10
	tay
	pla
.elseif(n = 13064)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13065)
	; 17 bytes
	pha
	tya
	ldy #229
	delay_n 52
	dey
	Jne *-9
	tay
	pla
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 17 bytes
	pha
	tya
	ldy #121
	delay_n 102
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 13067)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #5
	jsr delay_256a_11_clocks_
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 13068)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 46
	dey
	Jne *-7
	tay
	pla
.elseif(n = 13069)
	; 17 bytes
	pha
	tya
	pha
	ldy #225
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13070 || n = 13072 || n = 13085)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13071)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 13073)
	; 17 bytes
	pha
	tya
	pha
	ldy #53 ;hides 'and zp,x'
	dey ;first loop only
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 13074)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 46
	dey
	Jne *-7
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 13075 || n = 13084)
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13076)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/71
	delay_n 66
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13077)
	; 16 bytes
	pha
	tya
	ldy #195
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 13078)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13079)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13080)
	; 17 bytes
	pha
	tya
	ldy #242
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13081)
	; 17 bytes
	brk
	delay_n (n-13)
.elseif(n = 13082)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 167
	Jmi *-10
	tax
	pla
.elseif(n = 13083)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086 || n = 13091)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 13087 || n = 13090 || n = 13092 || n = 13093 || n = 13097 || n = 13098 || n = 13103 || n = 13105 || n = 13106)
	; 10..13 bytes
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
.elseif(n = 13089 || n = 13094)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13095)
	; 11 bytes
	php
	pha
	php
	lda #51
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13096 || n = 13101)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13099 || n = 13104)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13100)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13102)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 13107)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13108)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13109 || n = 13114)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13110 || n = 13111 || n = 13113)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13112)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 13114 && n < 13122) || (n > 13122 && n < 13126))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13122)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
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
.if(n = 13126 || (n > 13127 && n < 13282))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13127)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 13282) && (n < 13360))
.if((n >= 13282) && (n < 13321))
.if((n >= 13282) && (n < 13301))
; 13282 to 13300
.if((n > 13281 && n < 13289))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13291)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13292)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 13293)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13294)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13295)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13296)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13297)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13298)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13299)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13300)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13302)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13303)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13304)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13305)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #49
	lda #239
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13306)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #58
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 13307)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13308)
	; 18 bytes
	php
	pha
	tya
	ldy #96
	pha
	lda #98
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 13309)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13310)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13311)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13312)
	; 17 bytes
	pha
	tya
	ldy #95
	delay_n 135
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13313)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #71
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13314)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13315)
	; 16 bytes
	pha
	tya
	ldy #251
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 13316)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13317)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13318)
	; 17 bytes
	pha
	tya
	pha
	ldy #13
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13319)
	; 17 bytes
	pha
	tya
	pha
	ldy #100
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13320)
	; 17 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 46
	iny
	Jne *-10
	tay
	pla
.endif
.endif
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
.if(n = 13321)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 46
	iny
	Jne *-10
	tay
	pla
.elseif(n = 13322)
	; 17 bytes
	pha
	tya
	ldy #242
	delay_n 50
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13323)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #51
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13324)
	; 17 bytes
	pha
	tya
	ldy #208
	delay_n 59
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13325)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13326)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #190
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13327)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #46
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13328)
	; 18 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #<$8D ;hides 'sta $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
	pla
	Jcc *-12
	pla
.elseif(n = 13329)
	; 17 bytes
	pha
	tya
	ldy #193
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13330)
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13331)
	; 17 bytes
	pha
	tya
	pha
	ldy #208
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13332)
	; 18 bytes
	pha
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
	pla
.elseif(n = 13333)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #110
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
	pla
.elseif(n = 13334)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #76
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 13335)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #(n-279)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
	pla
.elseif(n = 13336)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #110
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13337)
	; 17 bytes
	pha
	tya
	ldy #205
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13338)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #20
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13339)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.else
; 13340 to 13359
.if(n = 13340)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13341 || n = 13343 || n = 13346 || n = 13348 || n = 13349 || n = 13353 || n = 13354 || n = 13359)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13342 || n = 13347)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 13344)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13345 || n = 13350)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13351)
	; 11 bytes
	php
	pha
	php
	lda #52
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13352 || n = 13357)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13355)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13356)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13358)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13361 || n = 13362)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13363)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13364)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13365 || n = 13370)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13366 || n = 13367 || n = 13369)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13368)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 13370 && n < 13378) || (n > 13378 && n < 13383) || (n > 13383 && n < 13438))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13378)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13383)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
; 13438 to 13515
.if((n > 13437 && n < 13516))
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
.if((n >= 13516) && (n < 13555))
; 13516 to 13554
.if((n > 13515 && n < 13545))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13547)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13548)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 13549)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13550)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13551)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13552)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13553)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13554)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 13555) && (n < 13574))
; 13555 to 13573
.if(n = 13555)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13556)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13557)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13558)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13559)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13560)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13561)
	; 18 bytes
	php
	pha
	tya
	ldy #222
	pha
	lda #22
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13562)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #188
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13563)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13564)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13565)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13566)
	; 17 bytes
	pha
	tya
	ldy #251
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13567)
	; 17 bytes
	pha
	txa
	pha
	ldx #232 ;hides 'inx'
	php
	lda #2
	jsr delay_256a_16_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 13568)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13569)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	delay_n 26
	dey
	Jne *-10
	tay
	pla
.elseif(n = 13570)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13571)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #215
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13572)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #13
	lda #4
	jsr delay_256a_11_clocks_
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13573)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 94
	Jne *-10
	tay
	pla
.endif
.else
; 13574 to 13593
.if(n = 13574)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13575)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #52
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13576)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 233
	Jmi *-10
	tay
	pla
.elseif(n = 13577)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13578)
	; 17 bytes
	pha
	tya
	ldy #238
	delay_n 52
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13579)
	; 16 bytes
	pha
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
	pla
.elseif(n = 13580)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 48
	dey
	Jne *-8
	tay
	pla
.elseif(n = 13581 || n = 13582)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 13583)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #199
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 13584)
	; 17 bytes
	pha
	tya
	ldy #234
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13585)
	; 17 bytes
	pha
	tya
	pha
	ldy #102
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13586)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 13587)
	; 17 bytes
	pha
	tya
	pha
	ldy #212
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13588)
	; 18 bytes
	pha
	delay_n (n-3)
.elseif(n = 13589)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13590)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #204
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 13591)
	; 17 bytes
	pha
	tya
	pha
	ldy #234
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13592)
	; 17 bytes
	pha
	tya
	ldy #96
	delay_n 135
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 13593)
	; 18 bytes
	pha
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
	pla
.endif
.endif
.endif
.endif
.else
; 13594 to 13750
.if(n = 13594)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #21
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13595)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13596)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 46
	dey
	Jne *-10
	tay
	pla
.elseif(n = 13597 || n = 13599 || n = 13602 || n = 13604 || n = 13605 || n = 13609 || n = 13610 || n = 13615 || n = 13617 || n = 13618)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13598 || n = 13603)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 13600)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13601 || n = 13606)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13607)
	; 11 bytes
	php
	pha
	php
	lda #53
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13608 || n = 13613)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13611 || n = 13616)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13612)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13614)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 13619)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13620)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13621 || n = 13626)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13622 || n = 13623 || n = 13625)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13624)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 13626 && n < 13634) || (n > 13634 && n < 13639) || (n > 13639 && n < 13751))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13634)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13639)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
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
.if((n >= 13790) && (n < 13809))
; 13790 to 13808
.if((n > 13789 && n < 13801))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13803)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 13804)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 13805)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13806)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13807)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13808)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13810)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13811)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13812)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13813)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13814)
	; 16 bytes
	pha
	tya
	ldy #206
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 13815)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13816 || n = 13822)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13817)
	; 17 bytes
	pha
	tya
	ldy #251
	delay_n 50
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13818)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #208
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 13819)
	; 16 bytes
	pha
	tya
	pha
	ldy #230
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 13820)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13821)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #29
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 13823)
	; 17 bytes
	pha
	tya
	pha
	ldy #238
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13824)
	; 18 bytes
	php
	pha
	tya
	ldy #251
	pha
	lda #16
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13825)
	; 18 bytes
	php
	pha
	tya
	ldy #234
	pha
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13826)
	; 17 bytes
	pha
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
	pla
.elseif(n = 13827)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #95
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 13828)
	; 17 bytes
	pha
	txa
	pha
	clc
	lda #$4A ;hides 'lsr a'
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	Jcc *-7
	pla
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 13829) && (n < 13868))
.if((n >= 13829) && (n < 13848))
; 13829 to 13847
.if(n = 13829)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #75
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13830)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #119
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13831)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #23
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13832)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13833)
	; 18 bytes
	pha
	tya
	ldy #51
	delay_n 266
	dey
	Jne *-10
	tay
	pla
.elseif(n = 13834)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13835)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #49
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13836)
	; 17 bytes
	pha
	tya
	ldy #0
	cmp $C5
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13837)
	; 17 bytes
	pha
	tya
	ldy #35
	delay_n 390
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13838)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13839)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #53
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13840)
	; 18 bytes
	php
	pha
	tya
	ldy #51
	pha
	lda #232
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13841)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #151
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 13842)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 177
	Jmi *-10
	tax
	pla
.elseif(n = 13843)
	; 17 bytes
	pha
	tya
	pha
	ldy #216
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13844)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/56
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13845)
	; 17 bytes
	pha
	tya
	pha
	ldy #223
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 13846)
	; 18 bytes
	pha
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
	pla
.elseif(n = 13847)
	; 17 bytes
	pha
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
	pla
.endif
.else
; 13848 to 13867
.if(n = 13848)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #158
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 13849)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #143
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 13850)
	; 17 bytes
	pha
	clc
	lda #$4A ;hides 'lsr a'
	delay_n 4609
	Jcc *-11
	pla
.elseif(n = 13851)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13852)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13853 || n = 13855 || n = 13858 || n = 13860 || n = 13861 || n = 13865 || n = 13866)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 13854 || n = 13859)
	; 10 bytes
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
.elseif(n = 13857 || n = 13862)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13863)
	; 11 bytes
	php
	pha
	php
	lda #54
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 13864)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13867)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13869)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 13870)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 13871 || n = 13873 || n = 13874)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13872)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13875)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13876)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13877 || n = 13882)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 13878 || n = 13879 || n = 13881)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13880)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 13882 && n < 13890) || (n > 13890 && n < 13895) || (n > 13895 && n < 13907))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 13890)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 13895)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 13907 to 14062
.if((n > 13906 && n < 14057))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14059)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 14060)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 14061)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14062)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
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
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14064)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14065)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14066)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14067)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14068)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14069)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14070)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 180
	Jmi *-10
	tax
	pla
.elseif(n = 14071)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14072)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14073)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #210
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 14074)
	; 18 bytes
	pha
	tya
	ldy #178
	delay_n 74
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14075)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14076)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-307)/281
	lda #249
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14077)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #146
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 14078)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14079)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14080)
	; 18 bytes
	php
	pha
	tya
	ldy #128
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 14081)
	; 18 bytes
	php
	pha
	tya
	ldy #178
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 14082 to 14101
.if(n = 14082)
	; 16 bytes
	pha
	tya
	ldy #210
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 14083)
	; 17 bytes
	pha
	tya
	pha
	ldy #47
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 14084)
	; 17 bytes
	pha
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
	pla
.elseif(n = 14085)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14086)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #190
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14087)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #77
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14088)
	; 17 bytes
	pha
	tya
	ldy #204
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14089)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 242
	Jmi *-10
	tay
	pla
.elseif(n = 14090)
	; 17 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 46
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14091)
	; 17 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 46
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14092)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 50
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14093)
	; 17 bytes
	pha
	tya
	pha
	ldy #227
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14094)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #54
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14095)
	; 18 bytes
	php
	pha
	tya
	ldy #207
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14096)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #208
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 14097)
	; 18 bytes
	pha
	tya
	pha
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	iny
	Jne *-9
	pla
	tay
	pla
.elseif(n = 14098)
	; 17 bytes
	pha
	tya
	pha
	ldy #247
	brk
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14099)
	; 17 bytes
	pha
	tya
	pha
	ldy #220
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14100)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14101)
	; 18 bytes
	pha
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
	pla
.endif
.endif
.else
; 14102 to 14140
.if(n = 14102)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #207
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14103)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #213
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 14104)
	; 18 bytes
	pha
	tya
	ldy #26
	delay_n 537
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14105)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #247
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14106)
	; 17 bytes
	pha
	tya
	ldy #243
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14107)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14108)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14109 || n = 14111 || n = 14114 || n = 14116 || n = 14117 || n = 14121 || n = 14122 || n = 14127 || n = 14129 || n = 14130)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14110 || n = 14115)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14112)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14113 || n = 14118)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14119)
	; 11 bytes
	php
	pha
	php
	lda #55
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14120 || n = 14125)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14123 || n = 14128)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14124)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14126)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 14131)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14132)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14133 || n = 14138)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14134 || n = 14135 || n = 14137)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14136)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14139 || n = 14140)
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
; 14141 to 14218
.if((n > 14140 && n < 14146) || (n > 14146 && n < 14151) || (n > 14151 && n < 14219))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14146)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14151)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if((n > 14218 && n < 14297))
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
.if((n >= 14297) && (n < 14336))
; 14297 to 14335
.if((n > 14296 && n < 14313))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14315 || n = 14328)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14316)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 14317)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14318)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14319)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14320)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14321)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14322)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14323)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14324)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14325)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14326)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14327)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14329)
	; 18 bytes
	php
	pha
	tya
	ldy #159
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14330)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14331)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #217
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 14332)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14333)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #25
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14334)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14335)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 14336) && (n < 14356))
; 14336 to 14355
.if(n = 14336)
	; 17 bytes
	pha
	lda #$0A ;hides 'asl a'
	delay_n 2861
	Jpl *-12
	pla
.elseif(n = 14337)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 38
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14338)
	; 17 bytes
	pha
	tya
	ldy #247
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14339)
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14340)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #55
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14341)
	; 17 bytes
	pha
	tya
	pha
	ldy #13
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 14342)
	; 17 bytes
	pha
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
	pla
.elseif(n = 14343)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14344)
	; 18 bytes
	php
	pha
	tya
	ldy #191
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14345)
	; 17 bytes
	pha
	tya
	pha
	ldy #247
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14346)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14347)
	; 18 bytes
	php
	pha
	tya
	ldy #199
	pha
	lda #33
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14348)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 51
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14349)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #157
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 14350)
	; 16 bytes
	pha
	tya
	ldy #214
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 14351)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14352)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14353)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14354)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/71
	delay_n 66
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14355)
	; 17 bytes
	pha
	tya
	pha
	ldy #224
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.endif
.else
; 14356 to 14375
.if(n = 14356)
	; 18 bytes
	php
	pha
	tya
	ldy #243
	pha
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14357)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14358)
	; 18 bytes
	pha
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
	pla
.elseif(n = 14359)
	; 16 bytes
	pha
	tya
	pha
	ldy #239
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
.elseif(n = 14360)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #222
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 14361 || n = 14365 || n = 14367 || n = 14370 || n = 14372 || n = 14373)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14362 || n = 14366 || n = 14371)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14363)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14364)
	; 17 bytes
	pha
	tya
	ldy #208
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14369 || n = 14374)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14375)
	; 11 bytes
	php
	pha
	php
	lda #56
	jsr delay_256a_16_clocks
	plp
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
.if(n = 14376 || n = 14381)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14377 || n = 14378 || n = 14383 || n = 14385 || n = 14386)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14379 || n = 14384)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14380)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14382)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 14387)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14388)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14389 || n = 14394)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14390 || n = 14391 || n = 14393)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14392)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 14394 && n < 14402) || (n > 14402 && n < 14407) || (n > 14407 && n < 14532))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14402)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14407)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 14532) && (n < 14610))
.if((n >= 14532) && (n < 14571))
; 14532 to 14570
.if((n > 14531 && n < 14569))
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
.if(n = 14571 || n = 14584 || n = 14585)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14572)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 14573)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14574)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14575)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14576)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14577)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14578)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14579)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14580)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14582)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14583)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14586)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #224
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14587)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14588)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14589)
	; 17 bytes
	pha
	tya
	pha
	ldy #235
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	dey
	Jne *-7
	pla
	tay
	pla
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #56
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14591)
	; 18 bytes
	pha
	tya
	ldy #239
	delay_n 56
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14592)
	; 18 bytes
	php
	pha
	tya
	ldy #247
	pha
	lda #20
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14593)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14594)
	; 18 bytes
	php
	pha
	tya
	ldy #55
	pha
	lda #226
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14595 || n = 14603)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14596 || n = 14601 || n = 14608)
	; 16..18 bytes
	pha
	delay_n (n-3)
.elseif(n = 14597)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14598)
	; 15 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1819
	Jne *-10
	pla
.elseif(n = 14599)
	; 16 bytes
	pha
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
	pla
.elseif(n = 14600)
	; 16 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1819
	Jne *-10
	pla
.elseif(n = 14602)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 251
	Jmi *-10
	tay
	pla
.elseif(n = 14604)
	; 17 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n > 14604 && n < 14608))
	; 17..18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 14609)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #217
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 14610) && (n < 14649))
; 14610 to 14648
.if(n = 14610)
	; 17 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 66
	iny
	Jne *-10
	tay
	pla
.elseif(n = 14611)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14612)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14613)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #123
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 14614)
	; 16 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 1819
	clc
	Jne *-11
	pla
.elseif(n = 14615)
	; 17 bytes
	pha
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
	pla
.elseif(n = 14616)
	; 17 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	delay_n 1819
	clc
	Jne *-11
	pla
.elseif(n = 14617 || n = 14622 || n = 14627)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14618)
	; 16 bytes
	pha
	tya
	ldy #218
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 14619)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14620)
	; 17 bytes
	pha
	tya
	ldy #176
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14621 || n = 14623 || n = 14626 || n = 14628 || n = 14629 || n = 14633 || n = 14634 || n = 14639 || n = 14641 || n = 14642)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14625 || n = 14630)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14631)
	; 11 bytes
	php
	pha
	php
	lda #57
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14632 || n = 14637)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14635 || n = 14640)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14636)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14638)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 14643)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14644)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14645)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14646 || n = 14647)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14648)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 14649 to 14687
.if(n = 14649)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14650)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 14650 && n < 14658) || (n > 14658 && n < 14663) || (n > 14663 && n < 14688))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14658)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14663)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
; 14688 to 14843
.if((n > 14687 && n < 14825))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14827 || n = 14840 || n = 14842)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14828)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 14829)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14830)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14831)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14832)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14833)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14834)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14835)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14836)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14837)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14838)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14839)
	; 16 bytes
	pha
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
	pla
.elseif(n = 14841)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14843)
	; 18 bytes
	php
	pha
	tya
	ldy #218
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
.if((n >= 14844) && (n < 14922))
.if((n >= 14844) && (n < 14883))
.if((n >= 14844) && (n < 14863))
; 14844 to 14862
.if(n = 14844)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #239
	lda #30
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14845)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14846)
	; 18 bytes
	php
	delay_n 29
	lda #169
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14847)
	; 17 bytes
	pha
	tya
	ldy #215
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14848)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14849)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14850)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14851)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/71
	delay_n 66
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14852)
	; 17 bytes
	pha
	tya
	ldy #106
	delay_n 135
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14853)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14854)
	; 18 bytes
	php
	pha
	tya
	ldy #215
	pha
	lda #30
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14855)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14856)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #172
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 14857)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 14858)
	; 17 bytes
	pha
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
	pla
.elseif(n = 14859)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/101
	delay_n 96
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14860)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14861)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #215
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14862)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #57
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14864)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 14865)
	; 17 bytes
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	sec
	lda #$0A ;hides 'asl a'
	clc
	Jne *-2
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 14866)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14867)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14868)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14869)
	; 17 bytes
	pha
	tya
	ldy #179
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14870)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #147
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
	pla
.elseif(n = 14871)
	; 17 bytes
	pha
	tya
	pha
	ldy #158
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 14872)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #30
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 14873)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #147
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14874)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #127
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14875)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14876)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14877 || n = 14879 || n = 14882)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 14878)
	; 10 bytes
	pha
	delay_n (n-3)
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
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14884 || n = 14885 || n = 14889 || n = 14890 || n = 14895 || n = 14897 || n = 14898)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 14886)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14887)
	; 11 bytes
	php
	pha
	php
	lda #58
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 14888 || n = 14893)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 14891 || n = 14896)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14892)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14894)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 14899)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14900)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14901 || n = 14906)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 14902 || n = 14903 || n = 14905)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14904)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 14906 && n < 14914) || (n > 14914 && n < 14919) || n = 14920 || n = 14921)
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 14914)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 14919)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 14922 to 15000
.if((n > 14921 && n < 15001))
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
.if((n > 15000 && n < 15079))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15083 || n = 15096)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 15084)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 15085)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15086)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15087)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15088)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15089)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15090)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15091)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15092)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15093)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15094)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15095)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15097)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.endif
.else
; 15098 to 15117
.if(n = 15098)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15099)
	; 17 bytes
	pha
	tya
	ldy #141
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15100)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15101)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15102)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15103)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15104)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	delay_n 39
	iny
	Jne *-10
	tay
	pla
.elseif(n = 15105)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 39
	dey
	Jne *-10
	tay
	pla
.elseif(n = 15106)
	; 18 bytes
	php
	delay_n 29
	lda #173
	jsr delay_a_25_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15107)
	; 18 bytes
	php
	pha
	tya
	ldy #184
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15108)
	; 18 bytes
	php
	pha
	tya
	ldy #191
	pha
	lda #40
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15109)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15110)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15111)
	; 17 bytes
	pha
	tya
	pha
	ldy #0 ;hides 'brk imm'
	lda $EA ;hides 'nop'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15112)
	; 17 bytes
	pha
	tya
	pha
	ldy #0 ;hides 'brk imm'
	nop
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	dey
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15113)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #141
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15114)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 15115)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 260
	Jmi *-10
	tay
	pla
.elseif(n = 15116)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15117)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #58
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 15118) && (n < 15137))
; 15118 to 15136
.if(n = 15118)
	; 17 bytes
	pha
	tya
	ldy #182
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15119)
	; 17 bytes
	pha
	tya
	pha
	ldy #151
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 15120)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #170
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15121)
	; 17 bytes
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15122)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #226
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 15123)
	; 17 bytes
	pha
	tya
	pha
	ldy #236
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 15124)
	; 18 bytes
	php
	pha
	tya
	ldy #159
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15125)
	; 18 bytes
	php
	pha
	tya
	ldy #182
	pha
	lda #44
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15126)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 652
	Jcs *-11
	pla
	plp
.elseif(n = 15127)
	; 18 bytes
	pha
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
	pla
.elseif(n = 15128)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #94
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15129)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 15130)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #27
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15131)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15132)
	; 17 bytes
	pha
	tya
	ldy #108
	delay_n 135
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15133 || n = 15135)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 15134)
	; 10 bytes
	pha
	delay_n (n-3)
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
.if(n = 15137 || n = 15142)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15138 || n = 15140 || n = 15141 || n = 15145 || n = 15146 || n = 15151 || n = 15153 || n = 15154)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 15139)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15143)
	; 11 bytes
	php
	pha
	php
	lda #59
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15144 || n = 15149)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15147 || n = 15152)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15148)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15150)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 15155)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15156)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.endif
.endif
.else
; 15157 to 15312
.if(n = 15157 || n = 15162)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15158 || n = 15159 || n = 15161)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15160)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 15162 && n < 15170) || (n > 15170 && n < 15175) || (n > 15175 && n < 15313))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15170)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15175)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 15313) && (n < 15469))
.if((n >= 15313) && (n < 15391))
.if((n >= 15313) && (n < 15352))
; 15313 to 15351
.if((n > 15312 && n < 15337))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15339)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15340)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 15341)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15342)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15343)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15344)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15345)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15346)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15347)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15348)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15349)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15350)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #230
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 15351)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352 || n = 15358)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 15353)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #85
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15354)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 107
	Jne *-10
	tay
	pla
.elseif(n = 15355)
	; 16 bytes
	pha
	tya
	ldy #229
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 15356)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15357)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 15359)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #57
	lda #237
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15360)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #187
	lda #50
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15361)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #73
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15362)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 197
	Jmi *-10
	tax
	pla
.elseif(n = 15363)
	; 18 bytes
	php
	pha
	tya
	ldy #137
	pha
	lda #73
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15364)
	; 17 bytes
	pha
	tya
	pha
	ldy #15
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 15365)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15366)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #236
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15367)
	; 17 bytes
	pha
	tya
	ldy #185
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15368)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #240
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15369)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #163
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 15370)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #80
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 15371 to 15390
.if(n = 15371)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 15372)
	; 17 bytes
	pha
	tya
	ldy #240
	delay_n 59
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15373)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15374)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15375)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #59
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15376)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15377)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #87
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15378)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15379)
	; 16 bytes
	pha
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
	pla
.elseif(n = 15380)
	; 18 bytes
	pha
	tya
	ldy #33
	delay_n 447
	dey
	Jpl *-10
	tay
	pla
.elseif(n = 15381 || n = 15389)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 15382 || n = 15390)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 15383)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15384)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #86
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 15385)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15386)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15387)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15388)
	; 18 bytes
	php
	pha
	tya
	ldy #141
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15394 || n = 15396 || n = 15397 || n = 15401 || n = 15402 || n = 15407 || n = 15409 || n = 15410)
	; 10..13 bytes
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
.elseif(n = 15393 || n = 15398)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15395)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15399)
	; 11 bytes
	php
	pha
	php
	lda #60
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15400 || n = 15405)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15403 || n = 15408)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15404)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15406)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 15411)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15412)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15413 || n = 15418)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15414 || n = 15415 || n = 15417)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15416)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 15418 && n < 15426) || (n > 15426 && n < 15431) || (n > 15431 && n < 15469))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15426)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15431)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
; 15469 to 15546
.if((n > 15468 && n < 15547))
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
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if((n > 15546 && n < 15586))
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
.if((n >= 15586) && (n < 15606))
; 15586 to 15605
.if((n > 15585 && n < 15593))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15595)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15596)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 15597)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15598)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15599)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15600)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15601)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15602)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15605)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 15606 to 15625
.if(n = 15606)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15607)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15608)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 15609)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15610)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15611)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15612)
	; 17 bytes
	pha
	tya
	ldy #52
	delay_n 295
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15613)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15614)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15615)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #87
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15616)
	; 17 bytes
	pha
	tya
	ldy #188
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15617)
	; 17 bytes
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$4A ;hides 'lsr a'
	delay_n 4
	Jne *-3
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15618)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #79
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15619)
	; 17 bytes
	pha
	tya
	pha
	ldy #156
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 15620)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15621)
	; 18 bytes
	php
	pha
	tya
	ldy #57
	pha
	lda #230
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 15622)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15623)
	; 16 bytes
	pha
	tya
	ldy #233
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 15624)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #44
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15625)
	; 17 bytes
	delay_n (n-2)
	nop
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
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 15627)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #6
	jsr delay_256a_11_clocks_
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 15628)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 109
	Jne *-10
	tay
	pla
.elseif(n = 15629)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15630)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 15631)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #60
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15632)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15633)
	; 18 bytes
	php
	pha
	tya
	ldy #211
	pha
	lda #35
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15634)
	; 17 bytes
	pha
	tya
	ldy #146
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15635)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #75
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15636)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15637)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15638)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #177
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15639)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/71
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15640)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 15641)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 72
	Jne *-10
	tax
	pla
.elseif(n = 15642)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #29
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15643)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15644)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #82
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 15645 to 15664
.if(n = 15645 || n = 15647 || n = 15650 || n = 15652 || n = 15653 || n = 15657 || n = 15658 || n = 15663)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15646 || n = 15651)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15648)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15649 || n = 15654)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15655)
	; 11 bytes
	php
	pha
	php
	lda #61
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15656 || n = 15661)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15659 || n = 15664)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15660)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15662)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665 || n = 15666)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 15667)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15668)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15669 || n = 15674)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15670 || n = 15671 || n = 15673)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15672)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 15674 && n < 15682) || (n > 15682 && n < 15687) || (n > 15687 && n < 15704))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15682)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15687)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 15704 to 15781
.if((n > 15703 && n < 15782))
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
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15849))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15849 || n = 15850)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15851)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 15852)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 15853)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15854)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15855)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15856)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15857)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15858)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15859)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 15860) && (n < 15899))
.if((n >= 15860) && (n < 15879))
; 15860 to 15878
.if(n = 15860)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15861)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15862)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15863)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #239
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 15864)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 15865)
	; 17 bytes
	pha
	tya
	ldy #191
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15866)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #240
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15867)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #217
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15868)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #178
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15869)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/101
	delay_n 96
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15870)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15871)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15872)
	; 17 bytes
	pha
	tya
	ldy #244
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15873)
	; 17 bytes
	php
	pha
	tya
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcs *-6
	pla
	tay
	pla
	plp
.elseif(n = 15874)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #55
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15875)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 15876)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15877)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15878)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 15879 to 15898
.if(n = 15879)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15880)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15881)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15882)
	; 17 bytes
	pha
	tya
	ldy #230
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15883)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15884)
	; 17 bytes
	pha
	tya
	ldy #248
	delay_n 59
	dey
	Jne *-9
	tay
	pla
.elseif(n = 15885)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15886)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 15887)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #61
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15888)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15889)
	; 18 bytes
	php
	pha
	tya
	ldy #230
	pha
	lda #30
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15890)
	; 17 bytes
	pha
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
	pla
.elseif(n = 15891)
	; 16 bytes
	pha
	tya
	ldy #237
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 15892)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15893)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 15894)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 204
	Jmi *-10
	tax
	pla
.elseif(n = 15895)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 15896)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #230
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15897)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #58
	lda #237
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15898)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 15899 to 15937
.if(n = 15899)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15900)
	; 18 bytes
	pha
	tya
	ldy #23
	delay_n 657
	dey
	Jpl *-10
	tay
	pla
.elseif(n = 15901 || n = 15903 || n = 15906 || n = 15908 || n = 15909 || n = 15913 || n = 15914 || n = 15919 || n = 15921 || n = 15922)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 15902 || n = 15907)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15904)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15905 || n = 15910)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15911)
	; 11 bytes
	php
	pha
	php
	lda #62
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 15912 || n = 15917)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 15915 || n = 15920)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15916)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15918)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 15923)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15924)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15925 || n = 15930)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 15926 || n = 15927 || n = 15929)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 15928)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 15930 && n < 15938))
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
.endif
.endif
.else
.if((n >= 15938) && (n < 16094))
; 15938 to 16093
.if(n = 15938)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif((n > 15938 && n < 15943) || (n > 15943 && n < 16094))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 15943)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 16094) && (n < 16172))
.if((n >= 16094) && (n < 16133))
.if((n >= 16094) && (n < 16113))
; 16094 to 16112
.if((n > 16093 && n < 16105))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16107)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16108)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 16109)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16110)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16111)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16112)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16114)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16115)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16116)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16117)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16118)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16119)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16120)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16121)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16122)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 207
	Jmi *-10
	tax
	pla
.elseif(n = 16123)
	; 18 bytes
	php
	pha
	tya
	ldy #244
	pha
	lda #27
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16124)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16125)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16126)
	; 18 bytes
	php
	delay_n 29
	lda #169
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16127)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #62
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16128)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16129)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #173
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 16130)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #244
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16131)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 16132)
	; 17 bytes
	pha
	tya
	ldy #248
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.endif
.endif
.else
.if((n >= 16133) && (n < 16152))
; 16133 to 16151
.if(n = 16133)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #177
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16134)
	; 17 bytes
	pha
	tya
	pha
	ldy #55
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 16135)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16136)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16137)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 16138)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16139)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #91
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16140)
	; 17 bytes
	pha
	tya
	ldy #252
	delay_n 59
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16141)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 278
	Jmi *-10
	tay
	pla
.elseif(n = 16142)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #237
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16143)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16144)
	; 18 bytes
	php
	pha
	tya
	ldy #215
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16145)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16146)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16147)
	; 17 bytes
	pha
	tya
	pha
	ldy #252
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 16148)
	; 17 bytes
	pha
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
	pla
.elseif(n = 16149)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 16150)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16151)
	; 17 bytes
	pha
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
	pla
.endif
.else
; 16152 to 16171
.if(n = 16152)
	; 18 bytes
	php
	pha
	tya
	ldy #221
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16153 || n = 16157 || n = 16159 || n = 16162 || n = 16164 || n = 16165 || n = 16169 || n = 16170)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 16154)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #31
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16155)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16156)
	; 16 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 62
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16158 || n = 16163)
	; 10 bytes
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
.elseif(n = 16161 || n = 16166)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16167)
	; 11 bytes
	php
	pha
	php
	lda #63
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16168)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16171)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16173)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16174)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 16175 || n = 16177 || n = 16178)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16176)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16179)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16180)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16181 || n = 16186)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16182 || n = 16183 || n = 16185)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16184)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 16186 && n < 16194) || (n > 16194 && n < 16199) || (n > 16199 && n < 16251))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16194)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16199)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
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
.if((n >= 16329) && (n < 16368))
; 16329 to 16367
.if((n > 16328 && n < 16361))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16361 || n = 16362)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16363)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 16364)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 16365)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16366)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16367)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.else
.if((n >= 16368) && (n < 16387))
; 16368 to 16386
.if(n = 16368)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16369)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16370)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16371)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16372)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16373)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16374)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16375)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16376)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #248
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 16377)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #51
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16378)
	; 17 bytes
	pha
	tya
	pha
	ldy #122
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 16379)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16380)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16381)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16382)
	; 18 bytes
	php
	delay_n 29
	lda #169
	jsr delay_a_25_clocks
	lda #(n-254)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16383)
	; 17 bytes
	pha
	tya
	ldy #153
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16384)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16385)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 46
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16386)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 16387 to 16406
.if(n = 16387)
	; 17 bytes
	pha
	tya
	pha
	ldy #15
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 16388)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16389)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16390)
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16391)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 707
	Jcs *-11
	pla
	plp
.elseif(n = 16392)
	; 17 bytes
	pha
	tya
	ldy #252
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16393)
	; 18 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 16394)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16395)
	; 18 bytes
	php
	pha
	tya
	ldy #184
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16396)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 59
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16397)
	; 17 bytes
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jne *-8
	pla
	tay
	pla
.elseif(n = 16398)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #63
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16399)
	; 18 bytes
	php
	pha
	tya
	ldy #252
	pha
	lda #26
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16400)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 16401)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16402)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16403)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	delay_n 7
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 16404)
	; 18 bytes
	php
	pha
	tya
	ldy #145
	pha
	lda #74
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16405)
	; 18 bytes
	pha
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
	pla
.elseif(n = 16406)
	; 17 bytes
	pha
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
	pla
.endif
.endif
.endif
.endif
.else
; 16407 to 16562
.if(n = 16407)
	; 18 bytes
	php
	pha
	tya
	ldy #241
	pha
	lda #29
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16408 || n = 16413 || n = 16415 || n = 16418 || n = 16420 || n = 16421 || n = 16425 || n = 16426 || n = 16431 || n = 16433 || n = 16434)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 16409)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #95
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 16410)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #32
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16411)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16412)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #58
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16414 || n = 16419)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16416)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16417 || n = 16422)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16423)
	; 11 bytes
	php
	pha
	php
	lda #64
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16424 || n = 16429)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16427 || n = 16432)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16428)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16430)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 16435)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16436)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16437 || n = 16442)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16438 || n = 16439 || n = 16441)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16440)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 16442 && n < 16450) || (n > 16450 && n < 16455) || (n > 16455 && n < 16563))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16450)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16455)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
; 16563 to 16640
.if((n > 16562 && n < 16617))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16619 || n = 16632)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16620)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 16621)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16622)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16623)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16624)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16625)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16626)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16627)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16628)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16629)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16630)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16631)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16633)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/71
	pha
	lda #32
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16634)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16635)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16636)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16637)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #147
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16638)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 16639)
	; 18 bytes
	php
	pha
	tya
	ldy #60
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16640)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
.if((n >= 16641) && (n < 16680))
.if((n >= 16641) && (n < 16660))
; 16641 to 16659
.if(n = 16641)
	; 17 bytes
	pha
	tya
	ldy #241
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16642)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16643)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16644)
	; 17 bytes
	pha
	tya
	pha
	ldy #124
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 16645)
	; 18 bytes
	php
	pha
	tya
	ldy #163
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16646)
	; 15 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 2075
	Jne *-10
	pla
.elseif(n = 16647)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #64
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16648)
	; 16 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 2075
	Jne *-10
	pla
.elseif(n = 16649)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 16650)
	; 17 bytes
	pha
	sec
	sec
	lda #$0A ;hides 'asl a'
	delay_n 2075
	Jne *-10
	pla
.elseif(n = 16651)
	; 17 bytes
	pha
	tya
	pha
	ldy #23
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 16652)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16653)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16654)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 287
	Jmi *-10
	tay
	pla
.elseif(n = 16655)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #241
	lda #37
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16656)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 77
	Jne *-10
	tax
	pla
.elseif(n = 16657)
	; 17 bytes
	pha
	tya
	pha
	ldy #177
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 16658)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #34
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16659)
	; 17 bytes
	brk
	delay_n (n-13)
.endif
.else
; 16660 to 16679
.if(n = 16660)
	; 18 bytes
	php
	pha
	tya
	ldy #128
	pha
	lda #90
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 16661)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #253
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 16662)
	; 16 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 2075
	clc
	Jne *-11
	pla
.elseif(n = 16663)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #43
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 16664)
	; 17 bytes
	pha
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
	pla
.elseif(n = 16665 || n = 16670 || n = 16675)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16666)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #33
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16667)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16668)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #74
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16669 || n = 16671 || n = 16674 || n = 16676 || n = 16677)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16673 || n = 16678)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16679)
	; 11 bytes
	php
	pha
	php
	lda #65
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680 || n = 16685)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16681 || n = 16682 || n = 16687 || n = 16689 || n = 16690)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16683 || n = 16688)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16684)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16686)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 16691)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16692)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16693 || n = 16698)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16694 || n = 16695 || n = 16697)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16696)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 16698 && n < 16706) || (n > 16706 && n < 16711) || (n > 16711 && n < 16719))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16706)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16711)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 16719 to 16875
.if((n > 16718 && n < 16873))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16875)
	; 15 bytes
	pha
	delay_n (n-3)
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
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 16877)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16878)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16879)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16880)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16881)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16882)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16883)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16884)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16885)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16886)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16887)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16888)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16889)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #183
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 16890)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16891)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 16892)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16893)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16894)
	; 18 bytes
	php
	pha
	tya
	ldy #225
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 16895 to 16914
.if(n = 16895)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16896)
	; 16 bytes
	pha
	tya
	ldy #252
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 16897)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 48
	dey
	Jne *-10
	tay
	pla
.elseif(n = 16898 || n = 16912)
	; 17..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 16899)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16900)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16901)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16902)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16903)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 16904)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16905)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16906)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16907)
	; 17 bytes
	pha
	tya
	ldy #109
	delay_n 150
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16908)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16909)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #65
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16910)
	; 17 bytes
	pha
	tya
	pha
	ldy #126
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 16911)
	; 18 bytes
	php
	pha
	tya
	ldy #206
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16913)
	; 18 bytes
	pha
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
	pla
.elseif(n = 16914)
	; 18 bytes
	php
	pha
	tya
	ldy #155
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.else
; 16915 to 16953
.if(n = 16915)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 16916)
	; 18 bytes
	php
	pha
	tya
	ldy #61
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16917)
	; 17 bytes
	pha
	tya
	ldy #245
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16918)
	; 17 bytes
	pha
	tya
	ldy #158
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 16919)
	; 17 bytes
	pha
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
	pla
.elseif(n = 16920)
	; 18 bytes
	pha
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
	pla
.elseif(n = 16921)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #155
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16922)
	; 17 bytes
	pha
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
	pla
.elseif(n = 16923)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16924)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #238
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16925 || n = 16927 || n = 16930 || n = 16932 || n = 16933 || n = 16937 || n = 16938 || n = 16943 || n = 16945 || n = 16946)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16926 || n = 16931)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16928)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16929 || n = 16934)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16935)
	; 11 bytes
	php
	pha
	php
	lda #66
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 16936 || n = 16941)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 16939 || n = 16944)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16940)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16942)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 16947)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16948)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16949)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 16950 || n = 16951 || n = 16953)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16952)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 16954 to 17031
.if(n = 16954)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 16954 && n < 16962) || (n > 16962 && n < 16967) || (n > 16967 && n < 17032))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 16962)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 16967)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
; 17032 to 17109
.if((n > 17031 && n < 17110))
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
.if((n >= 17110) && (n < 17149))
; 17110 to 17148
.if((n > 17109 && n < 17129))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17131 || n = 17144)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 17132)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 17133)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17134)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17135)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17136)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17137)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17138)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17139)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17140)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17141)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17142)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #86
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 17143)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17145)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17146)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17147)
	; 18 bytes
	pha
	tya
	ldy #149
	delay_n 110
	dey
	Jne *-10
	tay
	pla
.elseif(n = 17148)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17150)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17151)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17152)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17153)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17154)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17155)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 17156)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17157)
	; 18 bytes
	php
	pha
	tya
	ldy #209
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17158)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #48
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17159)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17160)
	; 18 bytes
	php
	pha
	tya
	ldy #61
	pha
	lda #242
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17161)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #83
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17162)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17163)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #66
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17164)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 62
	dey
	Jne *-8
	tay
	pla
.elseif(n = 17165)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #55
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17166)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 17167)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 296
	Jmi *-10
	tay
	pla
.endif
.else
; 17168 to 17187
.if(n = 17168)
	; 18 bytes
	pha
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
	pla
.elseif(n = 17169)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #217
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17170)
	; 18 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 158
	iny
	Jpl *-11
	tay
	pla
.elseif(n = 17171)
	; 18 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 17172)
	; 18 bytes
	pha
	tya
	ldy #55
	delay_n 307
	dey
	Jne *-10
	tay
	pla
.elseif(n = 17173)
	; 18 bytes
	pha
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
	pla
.elseif(n = 17174)
	; 18 bytes
	php
	pha
	tya
	ldy #235
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17175)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10
	lda #6
	jsr delay_256a_16_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 17176)
	; 17 bytes
	pha
	tya
	pha
	ldy #128
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 17177)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #124
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 17178)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #35
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17179)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17180)
	; 17 bytes
	pha
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
	pla
.elseif(n = 17181 || n = 17183 || n = 17186)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17182 || n = 17187)
	; 10 bytes
	pha
	delay_n (n-3)
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
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17189 || n = 17193 || n = 17194 || n = 17199 || n = 17201 || n = 17202)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17190)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17191)
	; 11 bytes
	php
	pha
	php
	lda #67
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17192 || n = 17197)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17195 || n = 17200)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17196)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17198)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 17203)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17204)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17205 || n = 17210)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17206 || n = 17207 || n = 17209)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17208)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 17210 && n < 17218) || (n > 17218 && n < 17223) || (n > 17223 && n < 17344))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17218)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17223)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 17344) && (n < 17422))
.if((n >= 17344) && (n < 17383))
; 17344 to 17382
.if((n > 17343 && n < 17383))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17387 || n = 17400)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 17388)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 17389)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17390)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17391)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17392)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17393)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17394)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17395)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17396)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17397)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17398)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17399)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17401)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #93
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 17402 to 17421
.if(n = 17402)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17403)
	; 18 bytes
	php
	pha
	tya
	ldy #212
	pha
	lda #43
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17404)
	; 18 bytes
	php
	pha
	tya
	ldy #183
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17405)
	; 18 bytes
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	php
	lda #85
	jsr delay_a_25_clocks
	plp
	dey
	Jne *-9
	pla
	tay
	pla
.elseif(n = 17406)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #220
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17407)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/71
	delay_n 66
	dey
	Jne *-9
	tay
	pla
.elseif(n = 17408)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #130
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 17409)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 122
	Jne *-10
	tay
	pla
.elseif(n = 17410)
	; 17 bytes
	pha
	tya
	pha
	ldy #16
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 17411)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17412)
	; 17 bytes
	pha
	tya
	ldy #57
	delay_n 295
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 17413)
	; 18 bytes
	php
	pha
	tya
	ldy #223
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17414)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 224
	Jmi *-10
	tax
	pla
.elseif(n = 17415)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17416)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #88
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 17417)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #187
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17418)
	; 18 bytes
	php
	pha
	tya
	ldy #137
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17419)
	; 17 bytes
	pha
	tya
	pha
	ldy #174
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 17420)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17421)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #190
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 17422) && (n < 17461))
.if((n >= 17422) && (n < 17441))
; 17422 to 17440
.if(n = 17422)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #67
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17423)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 17424)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #41
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 17425)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17426)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #150
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17427)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 17428)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17429)
	; 18 bytes
	pha
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
	pla
.elseif(n = 17430)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17431)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #(n-279)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
	pla
.elseif(n = 17432)
	; 18 bytes
	pha
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
	pla
.elseif(n = 17433)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #71
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17434)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #36
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17435)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17436)
	; 18 bytes
	pha
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
	pla
.elseif(n = 17437 || n = 17439)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17438)
	; 10 bytes
	pha
	delay_n (n-3)
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
.if(n = 17441 || n = 17446)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17442 || n = 17444 || n = 17445 || n = 17449 || n = 17450 || n = 17455 || n = 17457 || n = 17458)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17443)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17447)
	; 11 bytes
	php
	pha
	php
	lda #68
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17448 || n = 17453)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17451 || n = 17456)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17452)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17454)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 17459)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17460)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
; 17461 to 17500
.if(n = 17461 || n = 17466)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17462 || n = 17463 || n = 17465)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17464)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 17466 && n < 17474) || (n > 17474 && n < 17479) || (n > 17479 && n < 17501))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17474)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17479)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
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
.else
.if((n >= 17501) && (n < 18751))
.if((n >= 17501) && (n < 18126))
.if((n >= 17501) && (n < 17813))
.if((n >= 17501) && (n < 17657))
; 17501 to 17656
.if((n > 17500 && n < 17641))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17641 || n = 17642)
	; 15 bytes
	php
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17643 || n = 17656)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 17644)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 17645)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17646)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17647)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17648)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17649)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17650)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17651)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17652)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17653)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17654)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17655)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 17657) && (n < 17735))
.if((n >= 17657) && (n < 17696))
.if((n >= 17657) && (n < 17676))
; 17657 to 17675
.if(n = 17657)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #204
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 17658 || n = 17659)
	; 18 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17660)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17661)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17662)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17663)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #172
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17664)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #211
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 17665)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17666)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #38
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17667)
	; 17 bytes
	pha
	tya
	ldy #165
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 17668)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17669)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17670)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_25_clocks
	lda #68
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17671)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 82
	Jne *-10
	tax
	pla
.elseif(n = 17672)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #70
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17673)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/91
	pha
	lda #52
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17674)
	; 18 bytes
	php
	pha
	tya
	ldy #165
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17675)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #159
	lda #79
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 17676 to 17695
.if(n = 17676)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 64
	dey
	Jne *-9
	tay
	pla
.elseif(n = 17677)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17678)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #48
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 17679)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17680)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17681)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #165
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17682)
	; 17 bytes
	pha
	tya
	ldy #113
	delay_n 150
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 17683)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 124
	Jne *-10
	tay
	pla
.elseif(n = 17684)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17685)
	; 18 bytes
	php
	pha
	tya
	ldy #242
	pha
	lda #34
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17686)
	; 18 bytes
	php
	pha
	tya
	ldy #151
	pha
	lda #78
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17687)
	; 17 bytes
	pha
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
	pla
.elseif(n = 17688)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 17689)
	; 18 bytes
	php
	pha
	tya
	ldy #190
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17690)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #90
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 17691)
	; 9 bytes
	php
	pha
	lda #69
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17692)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #146
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17693 || n = 17695)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 17694)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 17697 || n = 17702)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17698 || n = 17700 || n = 17701 || n = 17705 || n = 17706 || n = 17711 || n = 17713 || n = 17714)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17703)
	; 11 bytes
	php
	pha
	php
	lda #69
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17704 || n = 17709)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17707 || n = 17712)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17708)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17710)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 17715)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17716)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17717 || n = 17722)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17718 || n = 17719 || n = 17721)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17720)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 17722 && n < 17730) || (n > 17730 && n < 17735))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17730)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.else
; 17735 to 17812
.if(n = 17735)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 17735 && n < 17813))
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
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17891))
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
.if((n >= 17891) && (n < 17930))
.if((n >= 17891) && (n < 17910))
; 17891 to 17909
.if((n > 17890 && n < 17897))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17899)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 17900)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 17901)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17902)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17903)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17904)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17905)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17906)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17907)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17908)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17909)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 17910 to 17929
.if(n = 17910)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17911)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17912)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 17913)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #167
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 17914)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #208
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17915)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #201
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17916)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17917)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #75
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17918)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #39
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17919)
	; 17 bytes
	pha
	tya
	pha
	ldy #179
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 17920)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/81
	pha
	lda #42
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17921)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17922)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17923)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17924)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #82
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17925)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #107
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 17926)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #68
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17927)
	; 17 bytes
	pha
	tya
	pha
	ldy #121
	lda #$4A ;hides 'lsr a'
	brk
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 17928)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17929)
	; 18 bytes
	php
	pha
	tya
	ldy #199
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 17930) && (n < 17949))
; 17930 to 17948
.if(n = 17930)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17931)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17932)
	; 17 bytes
	pha
	tya
	ldy #127
	delay_n 135
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 17933)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #141
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17934)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #148
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17935)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #69
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17936)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17937)
	; 17 bytes
	pha
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
	pla
.elseif(n = 17938)
	; 18 bytes
	php
	pha
	tya
	ldy #181
	pha
	lda #60
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17939)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 17940)
	; 17 bytes
	pha
	tya
	ldy #216
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 17941)
	; 17 bytes
	pha
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
	pla
.elseif(n = 17942)
	; 18 bytes
	pha
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
	pla
.elseif(n = 17943)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #(n-279)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
	pla
.elseif(n = 17944)
	; 18 bytes
	php
	pha
	tya
	ldy #239
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17945)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17946)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 231
	Jmi *-10
	tax
	pla
.elseif(n = 17947)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17948)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #(n-18)/326
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 17949 to 17968
.if(n = 17949 || n = 17951 || n = 17954 || n = 17956 || n = 17957 || n = 17961 || n = 17962 || n = 17967)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17950 || n = 17955)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17952)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17953 || n = 17958)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17959)
	; 11 bytes
	php
	pha
	php
	lda #70
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 17960 || n = 17965)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 17963 || n = 17968)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17964)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17966)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.endif
.endif
.endif
.endif
.else
; 17969 to 18125
.if(n = 17969 || n = 17970)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17971)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17972)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17973 || n = 17978)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 17974 || n = 17975 || n = 17977)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17976)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 17978 && n < 17986) || (n > 17986 && n < 17991) || (n > 17991 && n < 18126))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 17986)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 17991)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
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
.if((n > 18125 && n < 18153))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18155)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18156)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 18157)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18158)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18159)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18160)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18161)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18162)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18163)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18164)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 18165) && (n < 18184))
; 18165 to 18183
.if(n = 18165)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18166)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18167)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18168)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 18169)
	; 18 bytes
	php
	pha
	tya
	ldy #242
	pha
	lda #36
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18170)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18171)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18172)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18173)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18174)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 234
	Jmi *-10
	tax
	pla
.elseif(n = 18175)
	; 18 bytes
	php
	pha
	tya
	ldy #204
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18176)
	; 17 bytes
	pha
	lda #$0A ;hides 'asl a'
	delay_n 3629
	Jpl *-12
	pla
.elseif(n = 18177)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #70
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18178)
	; 18 bytes
	pha
	tya
	ldy #62
	delay_n 288
	dey
	Jne *-10
	tay
	pla
.elseif(n = 18179)
	; 18 bytes
	php
	pha
	tya
	ldy #227
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18180)
	; 18 bytes
	php
	pha
	tya
	ldy #143
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18181)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #200
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 18182)
	; 17 bytes
	pha
	tya
	ldy #45
	delay_n 390
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 18183)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.endif
.else
; 18184 to 18203
.if(n = 18184)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #66
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18185)
	; 17 bytes
	pha
	tya
	pha
	ldy #62
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 18186)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18187)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #7
	jsr delay_256a_11_clocks_
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 18188)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 66
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18189)
	; 17 bytes
	pha
	tya
	ldy #219
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18190)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #239
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18191)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18192)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/101
	delay_n 96
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18193)
	; 18 bytes
	php
	pha
	tya
	ldy #233
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18194)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #7
	jsr delay_256a_16_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 18195)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 18196)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #230
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18197)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #83
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18198)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #236
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18199)
	; 18 bytes
	php
	pha
	tya
	ldy #202
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18200)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18201)
	; 18 bytes
	pha
	tya
	ldy #46
	delay_n 382
	dey
	Jpl *-10
	tay
	pla
.elseif(n = 18202)
	; 17 bytes
	pha
	tya
	ldy #170
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18203)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_11_clocks_
	pla
	plp
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #90
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18205 || n = 18207 || n = 18210 || n = 18212 || n = 18213 || n = 18217 || n = 18218 || n = 18223 || n = 18225 || n = 18226)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18206 || n = 18211)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 18208)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18209 || n = 18214)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18215)
	; 11 bytes
	php
	pha
	php
	lda #71
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18216 || n = 18221)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18219 || n = 18224)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18220)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18222)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 18227)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18228)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18229 || n = 18234)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18230 || n = 18231 || n = 18233)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18232)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 18234 && n < 18242) || (n > 18242 && n < 18247) || (n > 18247 && n < 18282))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18242)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18247)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
; 18282 to 18359
.if((n > 18281 && n < 18360))
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
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18399))
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
.if((n >= 18399) && (n < 18418))
; 18399 to 18417
.if((n > 18398 && n < 18409))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18411)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18412)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 18413)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18414)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18415)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18416)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18417)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18419)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18420)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18421)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18422)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18423)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18424)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 18425)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #136
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 18426)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #230
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18427)
	; 18 bytes
	php
	pha
	tya
	ldy #236
	pha
	lda #39
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18428)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #52
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 18429)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #239
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18430)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18431)
	; 17 bytes
	pha
	txa
	pha
	clc
	lda #$2A ;hides 'rol a'
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	Jcc *-7
	pla
	tax
	pla
.elseif(n = 18432)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #143
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 18433)
	; 17 bytes
	pha
	tya
	pha
	ldy #18
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 18434)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #236
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18435)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18436)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 18437)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
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
	; 17 bytes
	pha
	tya
	ldy #222
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18439)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18440)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #198
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18441)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #145
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18442)
	; 18 bytes
	php
	pha
	tya
	ldy #207
	pha
	lda #50
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18443)
	; 17 bytes
	pha
	tya
	pha
	ldy #196
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 18444)
	; 18 bytes
	php
	pha
	tya
	ldy #66
	pha
	lda #236
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 18445)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #163
	lda #81
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18446)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #71
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18447)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18448)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #90
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18449)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 18450)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #62
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18451)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 18452)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #249
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18453)
	; 18 bytes
	pha
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
	pla
.elseif(n = 18454)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #68
	lda #239
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18455)
	; 17 bytes
	pha
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
	pla
.elseif(n = 18456)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 18457 to 18476
.if(n = 18457)
	; 17 bytes
	pha
	tya
	ldy #119
	delay_n 150
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18458)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #40
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18459)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18460)
	; 18 bytes
	pha
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
	pla
.elseif(n = 18461 || n = 18463 || n = 18466 || n = 18468 || n = 18469 || n = 18473 || n = 18474)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 18462 || n = 18467)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 18464)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18465 || n = 18470)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18471)
	; 11 bytes
	php
	pha
	php
	lda #72
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18472)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18475)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18476)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18478)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 18479 || n = 18481 || n = 18482)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18480)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18483)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18484)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18485 || n = 18490)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18486 || n = 18487 || n = 18489)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18488)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 18490 && n < 18498) || (n > 18498 && n < 18503) || (n > 18503 && n < 18516))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18498)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18503)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 18516 to 18593
.if((n > 18515 && n < 18594))
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
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18665))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18667)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 18668)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 18669)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18670)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18671)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
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
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18673)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18674)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18675)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18676)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18678)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18679)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18680)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 18681)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #205
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18682)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18683)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18684)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18685)
	; 18 bytes
	php
	pha
	tya
	ldy #183
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18686)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 87
	Jne *-10
	tax
	pla
.elseif(n = 18687)
	; 17 bytes
	pha
	tya
	ldy #225
	delay_n 78
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18688)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #217
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18689)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_25_clocks
	lda #72
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18690)
	; 18 bytes
	pha
	tya
	ldy #65
	delay_n 278
	dey
	Jpl *-10
	tay
	pla
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 807
	Jcs *-11
	pla
	plp
.elseif(n = 18692)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #70
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18693)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #53
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 18694)
	; 15 bytes
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 2331
	Jne *-10
	pla
.elseif(n = 18695)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #147
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18696)
	; 16 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 2331
	Jne *-10
	pla
.elseif(n = 18697)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 18698)
	; 17 bytes
	pha
	sec
	sec
	lda #$0A ;hides 'asl a'
	delay_n 2331
	Jne *-10
	pla
.elseif(n = 18699)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 18700)
	; 17 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18701)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 18702)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-187)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 18703)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #69
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18704)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #65
	lda #251
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 18705)
	; 18 bytes
	pha
	tya
	ldy #201
	delay_n 88
	dey
	Jne *-10
	tay
	pla
.elseif(n = 18706)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 241
	Jmi *-10
	tax
	pla
.elseif(n = 18707)
	; 17 bytes
	brk
	delay_n (n-13)
.elseif(n = 18708)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #185
	ldy #18
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
	pla
.elseif(n = 18709)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #87
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18710)
	; 16 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 2331
	clc
	Jne *-11
	pla
.endif
.endif
.else
; 18711 to 18750
.if(n = 18711)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18712)
	; 17 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	delay_n 2331
	clc
	Jne *-11
	pla
.elseif(n = 18713)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #207
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 18714)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #41
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18715)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18716)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #210
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18717 || n = 18719 || n = 18722 || n = 18724 || n = 18725 || n = 18729 || n = 18730 || n = 18735 || n = 18737 || n = 18738)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18718 || n = 18723)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 18720)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18721 || n = 18726)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18727)
	; 11 bytes
	php
	pha
	php
	lda #73
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18728 || n = 18733)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18731 || n = 18736)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18732)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18734)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 18739)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18740)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18741 || n = 18746)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18742 || n = 18743 || n = 18745)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18744)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 18746 && n < 18751))
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
.endif
.endif
.endif
.endif
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
; 18751 to 18906
.if((n > 18750 && n < 18754) || (n > 18754 && n < 18759) || (n > 18759 && n < 18907))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18754)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18759)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
; 18907 to 18945
.if((n > 18906 && n < 18921))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18923 || n = 18936)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 18924)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 18925)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18926)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18927)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18928)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18929)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18930)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18931)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18932)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18933)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18934)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 244
	Jmi *-10
	tax
	pla
.elseif(n = 18935)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18937)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18938)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #64
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18939)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18940)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #193
	lda #66
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18941)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #210
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 18942)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18943)
	; 17 bytes
	pha
	tya
	pha
	ldy #(n-19)/76
	lda #74 ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 18944)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #186
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 18945)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #141
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.endif
.else
.if((n >= 18946) && (n < 18965))
; 18946 to 18964
.if(n = 18946)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #220
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18947)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #159
	lda #87
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18948)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18949)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18950)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #249
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18951)
	; 17 bytes
	pha
	tya
	ldy #177
	delay_n 102
	dey
	Jne *-9
	tay
	pla
.elseif(n = 18952)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #148
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 18953)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #222
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 18954)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #208
	lda #59
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18955)
	; 18 bytes
	php
	pha
	tya
	ldy #72
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18956)
	; 18 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #136
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 18957)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18958)
	; 18 bytes
	php
	pha
	tya
	ldy #177
	pha
	lda #68
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18959)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #73
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18960)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 18961)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 18962)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #72
	lda #231
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18963)
	; 17 bytes
	pha
	tya
	pha
	ldy #127
	lda #$4A ;hides 'lsr a'
	brk
	Jne *-3
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 18964)
	; 18 bytes
	pha
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
	pla
.endif
.else
; 18965 to 18984
.if(n = 18965)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #177
	lda #75
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18966)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #43
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 18967)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #(n-279)/256
	ldy #49
	dey
	Jpl *-1
	dex
	Jpl *-6
	tay
	pla
	tax
	pla
.elseif(n = 18968)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18969)
	; 18 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #62
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 18970)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #42
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18971)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18972)
	; 17 bytes
	pha
	tya
	ldy #47
	delay_n 390
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 18973 || n = 18975 || n = 18978 || n = 18980 || n = 18981)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 18974 || n = 18979)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 18976)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 18977 || n = 18982)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18983)
	; 11 bytes
	php
	pha
	php
	lda #74
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 18984)
	; 11 bytes
	brk
	delay_n (n-13)
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985 || n = 18986 || n = 18991 || n = 18993 || n = 18994)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18987 || n = 18992)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18988)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18989)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 18990)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 18995)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 18996)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18997 || n = 19002)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 18998 || n = 18999 || n = 19001)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19000)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 19002 && n < 19010) || (n > 19010 && n < 19015) || (n > 19015 && n < 19063))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19010)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19015)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
; 19063 to 19140
.if((n > 19062 && n < 19141))
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
.if((n >= 19141) && (n < 19180))
; 19141 to 19179
.if((n > 19140 && n < 19177))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19179)
	; 15 bytes
	pha
	delay_n (n-3)
.endif
.else
.if((n >= 19180) && (n < 19199))
; 19180 to 19198
.if(n = 19180)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 19181)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19182)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19183)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19184)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19185)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19186)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19187)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19188)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19189)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19190)
	; 17 bytes
	pha
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
	pla
.elseif(n = 19191)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19192)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 19193)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19194)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19195)
	; 17 bytes
	pha
	tya
	pha
	ldy #204
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 19196)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19197)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #101
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 19198)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 19199 to 19218
.if(n = 19199)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #249
	lda #45
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19200)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19201)
	; 18 bytes
	php
	pha
	tya
	ldy #139
	pha
	lda #99
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19202)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/101
	delay_n 96
	dey
	Jne *-9
	tay
	pla
.elseif(n = 19203)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19204)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19205)
	; 17 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #$4A ;hides 'lsr a'
	delay_n 4
	Jne *-3
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 19206)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #242
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19207)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 19208)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #246
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19209)
	; 18 bytes
	php
	pha
	tya
	ldy #202
	pha
	lda #56
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19210)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19211)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19212)
	; 17 bytes
	pha
	tya
	ldy #63
	delay_n 295
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 19213)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19214)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19215)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #173
	jsr delay_a_25_clocks
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19216)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19217)
	; 17 bytes
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	iny
	Jne *-8
	pla
	tay
	pla
.elseif(n = 19218)
	; 18 bytes
	php
	pha
	tya
	ldy #72
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
; 19219 to 19257
.if(n = 19219)
	; 17 bytes
	pha
	tya
	pha
	ldy #192
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 19220)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/91
	pha
	lda #52
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19221)
	; 18 bytes
	pha
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
	pla
.elseif(n = 19222)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/111
	pha
	lda #72
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19223)
	; 17 bytes
	pha
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
	pla
.elseif(n = 19224)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19225)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #72
	lda #231
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19226)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #43
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19227)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19228)
	; 19 bytes
	php
	pha
	.repeat 2
	 lda #99
	 jsr delay_a_25_clocks
	.endrepeat
	lda #74
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19229 || n = 19231 || n = 19234 || n = 19236 || n = 19237 || n = 19241 || n = 19242 || n = 19247 || n = 19249 || n = 19250)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 19230 || n = 19235)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19232)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19233 || n = 19238)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19239)
	; 11 bytes
	php
	pha
	php
	lda #75
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19240 || n = 19245)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 19243 || n = 19248)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19244)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19246)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 19251)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19252)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19253)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19254 || n = 19255 || n = 19257)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19256)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 19258 to 19296
.if(n = 19258)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 19258 && n < 19266) || (n > 19266 && n < 19271) || (n > 19271 && n < 19297))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19266)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19271)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
; 19297 to 19375
.if((n > 19296 && n < 19376))
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
.endif
.endif
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
; 19376 to 19453
.if((n > 19375 && n < 19433))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19435 || n = 19448)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 19436)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 19437)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19438)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19439)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19440)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19441)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19442)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19443)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19444)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19445)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19446)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19447)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19449)
	; 18 bytes
	php
	pha
	tya
	ldy #145
	pha
	lda #95
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19450)
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #88
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19451)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #73
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19452)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19453)
	; 18 bytes
	php
	delay_n 29
	lda #168
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 19454) && (n < 19493))
.if((n >= 19454) && (n < 19473))
; 19454 to 19472
.if(n = 19454)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #76
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19455)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19456)
	; 17 bytes
	pha
	tya
	pha
	ldy #18
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 19457)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #153
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19458)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #45
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 19459)
	; 18 bytes
	php
	pha
	tya
	ldy #243
	pha
	lda #41
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19460)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #47
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19461)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #83
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19462)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19463)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #209
	lda #61
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19464)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 137
	Jne *-10
	tay
	pla
.elseif(n = 19465)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #146
	jsr delay_a_25_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 19466)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 251
	Jmi *-10
	tax
	pla
.elseif(n = 19467)
	; 18 bytes
	php
	pha
	tya
	ldy #221
	pha
	lda #49
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19468)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19469)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19470)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #172
	jsr delay_a_25_clocks
	lda #75
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19471)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #103
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 19472)
	; 17 bytes
	pha
	tya
	ldy #139
	delay_n 135
	dey
	Jne *-9
	tay
	pla
.endif
.else
; 19473 to 19492
.if(n = 19473)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #217
	jsr delay_a_25_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 19474)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #56
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19475)
	; 17 bytes
	pha
	tya
	pha
	ldy #0
	lda #$4A ;hides 'lsr a'
	delay_n 4
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 19476 || n = 19478 || n = 19486 || n = 19491)
	; 10..18 bytes
	pha
	delay_n (n-3)
.elseif(n = 19477)
	; 17 bytes
	pha
	tya
	pha
	ldy #207
	lda #$0A ;hides 'asl a'
	brk
	Jpl *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 19479)
	; 18 bytes
	php
	pha
	tya
	ldy #69
	pha
	lda #239
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 19480)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #70
	lda #242
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19481)
	; 18 bytes
	php
	pha
	tya
	ldy #73
	pha
	lda #224
	jsr delay_a_25_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 19482)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #44
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19483)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19484)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 19485 || n = 19487 || n = 19490 || n = 19492)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
	delay_n 6
	delay_n (n-6)
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493 || n = 19497 || n = 19498 || n = 19503 || n = 19505 || n = 19506)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 19494)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19495)
	; 11 bytes
	php
	pha
	php
	lda #76
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19496 || n = 19501)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 19499 || n = 19504)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19500)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19502)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 19507)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19508)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19509 || n = 19514)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif(n = 19510 || n = 19511 || n = 19513)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19512)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif((n > 19514 && n < 19522) || (n > 19522 && n < 19527) || (n > 19527 && n < 19532))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19522)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19527)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.endif
.else
; 19532 to 19687
.if((n > 19531 && n < 19688))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19691 || n = 19704)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 19692)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 19693)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19694)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19695)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19696)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19697)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19698)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19699)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19700)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19701)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19702)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19703)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19705)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	lda #63
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19706)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #48
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 19707 to 19726
.if(n = 19707)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/101
	delay_n 96
	dey
	Jne *-9
	tay
	pla
.elseif(n = 19708)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #58
	jsr delay_a_25_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 19709)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #49
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19710)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #76
	lda #227
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19711)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #155
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19712)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #193
	lda #70
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19713)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19714)
	; 18 bytes
	php
	delay_n 29
	lda #173
	jsr delay_a_25_clocks
	lda #(n-258)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19715)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19716)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #78
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19717)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #65
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19718)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #195
	jsr delay_a_25_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 19719)
	; 17 bytes
	pha
	tya
	pha
	ldy #197
	lda #$4A ;hides 'lsr a'
	delay_n 7
	Jne *-3
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 19720)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #54
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19721)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #195
	lda #69
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19722)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19723)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #76
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19724)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #201
	lda #66
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19725)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #79
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19726)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 852
	Jcs *-11
	pla
	plp
.endif
.endif
.else
.if((n >= 19727) && (n < 19746))
; 19727 to 19745
.if(n = 19727)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #67
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19728)
	; 18 bytes
	pha
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
	pla
.elseif(n = 19729)
	; 18 bytes
	php
	pha
	tya
	ldy #219
	pha
	lda #51
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19730)
	; 17 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 78
	iny
	Jne *-10
	tay
	pla
.elseif(n = 19731)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 19732)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19733)
	; 18 bytes
	pha
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
	pla
.elseif(n = 19734)
	; 18 bytes
	pha
	tya
	ldy #37
	delay_n 514
	dey
	Jpl *-10
	tay
	pla
.elseif(n = 19735)
	; 18 bytes
	php
	pha
	tya
	ldy #212
	pha
	lda #54
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19736)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #219
	lda #58
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19737)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #44
	jsr delay_a_25_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 19738)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 139
	Jne *-10
	tay
	pla
.elseif(n = 19739)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19740)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #152
	jsr delay_a_25_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 19741 || n = 19743)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19742)
	; 10 bytes
	pha
	delay_n (n-3)
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
	delay_n 6
	delay_n (n-6)
.endif
.else
; 19746 to 19765
.if(n = 19746 || n = 19748 || n = 19749 || n = 19753 || n = 19754 || n = 19759 || n = 19761 || n = 19762)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 19747)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19750)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19751)
	; 11 bytes
	php
	pha
	php
	lda #77
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19752 || n = 19757)
	; 11 bytes
	brk
	delay_n (n-13)
.elseif(n = 19755 || n = 19760)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19756)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19758)
	; 13 bytes
	php
	pha
	lda #>(n-44)
	jsr delay_256a_16_clocks
	pla
	.repeat <(n-44)
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 19763)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19764)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19765)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.endif
.endif
.endif
.else
; 19766 to 19843
.if(n = 19766 || n = 19767 || n = 19769)
	; 14 bytes
	php
	pha
	lda #<(n-54)
	jsr delay_a_25_clocks
	lda #>(n-54)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19768)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19770)
	; 13 bytes
	delay_n 26
	delay_n (n-26)
.elseif((n > 19770 && n < 19778) || (n > 19778 && n < 19783) || (n > 19783 && n < 19844))
	; 14 bytes
	php
	pha
	lda #<(n-59)
	jsr delay_a_25_clocks
	lda #>(n-59)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19778)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19783)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
; 19844 to 19921
.if((n > 19843 && n < 19922))
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
.if((n >= 19922) && (n < 19961))
; 19922 to 19960
.if((n > 19921 && n < 19945))
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
	delay_n 5
	lda #<(n-61)
	jsr delay_a_25_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19947 || n = 19960)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 19948)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 19949)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_25_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19950)
	; 16 bytes
	php
	pha
	php
	lda #172
	jsr delay_a_25_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19951)
	; 16 bytes
	php
	pha
	php
	lda #173
	jsr delay_a_25_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks
	plp
	pla
	plp
.elseif(n = 19952)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_25_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19953)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_25_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19954)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_25_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19955)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_25_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19956)
	; 16 bytes
	php
	delay_n 16
	lda #172
	jsr delay_a_25_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19957)
	; 16 bytes
	php
	delay_n 16
	lda #173
	jsr delay_a_25_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19958)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #172
	jsr delay_a_25_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19959)
	; 17 bytes
	php
	delay_n 5
	brk
	lda #173
	jsr delay_a_25_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
.if((n >= 19961) && (n < 19981))
; 19961 to 19980
.if(n = 19961)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #79
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19962)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #224
	lda #57
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19963)
	; 18 bytes
	php
	pha
	tya
	ldy #72
	pha
	lda #238
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19964)
	; 17 bytes
	pha
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
	pla
.elseif(n = 19965)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #95
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19966)
	; 18 bytes
	php
	pha
	tya
	ldy #183
	pha
	lda #70
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19967)
	; 17 bytes
	pha
	txa
	pha
	ldx #232 ;hides 'inx'
	php
	lda #3
	jsr delay_256a_16_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 19968)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #86
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19969)
	; 17 bytes
	pha
	tya
	pha
	ldy #150
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 19970)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #72
	lda #245
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19971)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/86
	pha
	lda #47
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19972)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19973)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #77
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19974)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #99
	jsr delay_a_25_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 19975)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19976)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #210
	lda #63
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19977)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-307)/281
	lda #249
	jsr delay_a_25_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19978)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #172
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19979)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19980)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.endif
.else
; 19981 to 20000
.if(n = 19981)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_25_clocks
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19982)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #124
	jsr delay_a_25_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 19983)
	; 18 bytes
	php
	pha
	tya
	ldy #217
	pha
	lda #53
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19984)
	; 18 bytes
	php
	pha
	tya
	ldy #165
	pha
	lda #82
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19985)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #95
	jsr delay_a_25_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19986)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2848
	Jcs *-11
	pla
	plp
.elseif(n = 19987)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 19988)
	; 18 bytes
	pha
	txa
	pha
	tya
	ldx #165
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
	tay
	pla
	tax
	pla
.elseif(n = 19989)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #131
	jsr delay_a_25_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 19990)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-187)/161
	lda #105 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19991)
	; 17 bytes
	pha
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
	pla
.elseif(n = 19992)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #102
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19993)
	; 18 bytes
	pha
	tya
	ldy #52
	delay_n 372
	dey
	Jpl *-10
	tay
	pla
.elseif(n = 19994)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #46
	jsr delay_a_25_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19995)
	; 9 bytes
	php
	pha
	lda #78
	jsr delay_256a_11_clocks_
	pla
	plp
.elseif(n = 19996)
	; 19 bytes
	php
	pha
	.repeat 2
	 lda #99
	 jsr delay_a_25_clocks
	.endrepeat
	lda #77
	jsr delay_256a_16_clocks
	pla
	plp
.elseif(n = 19997 || n = 19999)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19998)
	; 10 bytes
	pha
	delay_n (n-3)
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
