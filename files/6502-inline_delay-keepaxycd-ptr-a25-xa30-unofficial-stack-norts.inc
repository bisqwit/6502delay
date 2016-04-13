; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber S, Z+N, and V
; Will preserve A, X, Y, C, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.

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
; - 4 bytes or less when n <= 14
; - 5 bytes or less when n <= 17
; - 6 bytes or less when n <= 21
; - 7 bytes or less when n <= 24
; - 8 bytes or less when n <= 28
; - 9 bytes or less when n <= 32
; -10 bytes or less when n <= 302
; -11 bytes or less when n <= 1310
; -12 bytes or less when n <= 1356
; -13 bytes or less when n <= 2328
; -14 bytes or less when n <= 3128
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=4, avg=4.9, max=6
; - When 21 <= n <= 30: min=6, avg=7.7, max=9
; - When 31 <= n <= 50: min=8, avg=9.1, max=10
; - When 51 <= n <= 75: min=8, avg=8.9, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.0, max=11
; - When 501 <= n <= 750: min=8, avg=10.1, max=11
; - When 751 <= n <= 1000: min=9, avg=10.1, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.8, max=13
; - When 2001 <= n <= 3000: min=10, avg=12.1, max=14
; - When 3001 <= n <= 5000: min=11, avg=13.9, max=15
; - When 5001 <= n <= 7500: min=13, avg=15.0, max=15
; - When 7501 <= n <= 10000: min=13, avg=15.0, max=15
; - When 10001 <= n <= 20000: min=13, avg=15.0, max=15
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
.elseif(n = 4 || n = 5 || n = 8 || n = 39)
	; 2..10 bytes
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
.elseif((n > 8 && n < 12) || n = 13)
	; 3..4 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 4 bytes
	rol $36,X
	ror $36,X
.elseif(n = 14 || n = 21 || n = 28)
	; 4..8 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 5 bytes
	.repeat 5
	 pha
	.endrepeat
.elseif((n > 15 && n < 19) || n = 20)
	; 5..6 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 19)
	; 6 bytes
	php
	delay_n 12
	plp
.elseif(n = 22)
	; 7 bytes
	delay_n (n-19)
	delay_n 19
.elseif(n = 23 || n = 30)
	; 7..9 bytes
	nop
	delay_n (n-2)
.elseif(n = 24 || n = 29 || n = 31)
	; 7..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 25)
	; 8 bytes
	delay_n 4
	delay_n (n-4)
.elseif(n = 26)
	; 8 bytes
	php
	pha
	delay_n 12
	pla
	plp
.elseif(n = 27)
	; 8 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 32)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 33)
	; 10 bytes
	php
	delay_n 12
	.repeat 2
	 plp
	 php
	.endrepeat
	plp
.elseif(n = 34)
	; 9 bytes
	php
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	plp
.elseif(n = 35)
	; 9 bytes
	php
	pha
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
	plp
.elseif(n = 36)
	; 10 bytes
	php
	pha
	sec
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	plp
.elseif(n = 37)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 38)
	; 8 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
.endif
.else
; 40 to 78
.if(n = 40)
	; 9 bytes
	php
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
.elseif((n > 40 && n < 53) || (n > 53 && n < 60) || (n > 60 && n < 79))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
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
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 135) || (n > 135 && n < 157))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 135)
	; 8 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
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
.elseif(n = 304 || n = 308 || n = 309 || n = 311)
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
.elseif(n = 314 || n = 316 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 339 || n = 343 || n = 344 || n = 346 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 361 || n = 364 || n = 366 || n = 374 || n = 378 || n = 379 || n = 381 || n = 384 || n = 386 || n = 389 || n = 391 || n = 394 || n = 399 || n = 401 || n = 413 || n = 414 || n = 419 || n = 421 || n = 424 || n = 429 || n = 431 || n = 434 || n = 436 || n = 444 || n = 448 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464)
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
.elseif((n .mod 5 = 3) && ((n > 322 && n < 339) || n = 358 || n = 368 || n = 373 || n = 398 || n = 403 || (n > 432 && n < 444) || n = 458 || n = 463))
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 325)
	; 10 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	iny
	Jpl *-3
	tay
	pla
.elseif(n = 330 || n = 335 || n = 340 || n = 345 || n = 350 || n = 360 || n = 365 || n = 370 || n = 375 || n = 380 || n = 385 || n = 393 || n = 395 || n = 400 || n = 405 || n = 410 || n = 420 || n = 426 || n = 430 || n = 435 || n = 440 || n = 445 || n = 450 || n = 455 || n = 465)
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
.elseif(n = 406)
	; 10 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jpl *-3
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
.elseif(n = 415)
	; 10 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
.elseif(n = 416)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 423)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
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
.elseif(n = 466)
	; 10 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	tay
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
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 486 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 518 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 546)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 475 || n = 480 || n = 485 || n = 490 || n = 503 || n = 505 || n = 510 || n = 515 || n = 520 || n = 525 || n = 535 || n = 540 || n = 545)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 472 || n = 482 || n = 497 || n = 507 || n = 527)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 473 || n = 493 || n = 513 || n = 533 || n = 538)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 474 || n = 481 || n = 509 || n = 523 || n = 530)
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
.elseif(n = 477 || n = 487 || n = 492 || n = 502 || n = 512 || n = 517 || n = 522 || n = 532 || n = 537 || n = 542)
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
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jpl *-3
	tay
	pla
.elseif(n = 488 || n = 495 || n = 516 || n = 544)
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
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jpl *-3
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
.elseif(n = 498)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 500)
	; 9 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	Jpl *-2
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
.elseif(n = 541)
	; 10 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 543)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.endif
.else
; 547 to 625
.if((n .mod 5 = 2) && ((n > 546 && n < 573) || n = 597 || (n > 606 && n < 618)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
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
.elseif(n = 549 || n = 553 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 574 || n = 581 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 623 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 550 || n = 560 || (n > 569 && n < 596) || (n > 604 && n < 626)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 551 || n = 558 || n = 593 || n = 614 || n = 621)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 555)
	; 10 bytes
	pha
	tya
	ldy #(n-11)/8 ;hides 'nop zp'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 561)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 563)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 565 || n = 579 || n = 586 || n = 600)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 568)
	; 10 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	dey
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 571)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 573 || n = 578 || n = 583 || n = 598 || n = 603 || n = 608)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 577 || n = 582 || n = 587 || n = 592 || n = 602 || n = 622)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
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
.elseif(n = 613)
	; 10 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
	pla
.elseif(n = 618)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
	tax
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 686 || n = 689 || n = 693 || n = 694 || n = 699)
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
.elseif(n = 628 || n = 635 || n = 656 || n = 670 || n = 691)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 630 || n = 643 || n = 645 || n = 650 || n = 655 || n = 660 || n = 665 || n = 675 || n = 680 || n = 690 || n = 698 || n = 700)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 633 || n = 696)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 636)
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
.elseif(n = 640)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 703)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
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
.elseif(n = 649 || n = 663 || n = 684)
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
.elseif(n = 653 || n = 673 || n = 683 || n = 688)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 685)
	; 10 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jpl *-3
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
.elseif(n = 703)
	; 10 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jpl *-3
	tay
	pla
.endif
.else
; 704 to 781
.if(n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 728 || n = 729 || n = 731 || n = 734 || n = 736 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 763 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 705 || n = 733)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif((n .mod 5 = 2) && ((n > 706 && n < 778)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 708)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 710 || n = 715 || n = 720 || n = 725 || n = 730 || n = 735 || n = 741 || n = 745 || n = 750 || n = 760 || n = 765 || n = 770 || n = 780)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 713 || n = 718 || n = 753 || n = 758 || n = 773 || n = 778)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 719 || n = 726 || n = 740 || n = 754 || n = 761 || n = 768 || n = 775)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 721)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 723)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 724)
	; 9 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
	pla
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
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
.endif
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
.elseif(n = 783 || n = 793 || n = 818 || n = 823 || n = 848 || n = 853 || n = 863 || n = 878 || n = 883 || n = 893 || n = 898 || n = 913 || n = 918 || n = 923 || n = 933)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 784 || n = 794 || n = 798 || n = 799 || n = 801 || n = 804 || n = 806 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 833 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 868 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 806) || n = 815 || (n > 824 && n < 841) || (n > 849 && n < 876) || (n > 884 && n < 906) || (n > 919 && n < 936)))
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
	ldy #(n-11)/8 ;hides 'nop zp'
	nop
	dey
	Jne *-3
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
.elseif(n = 820)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
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
.elseif(n = 864)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 871)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jpl *-2
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
.elseif(n = 928)
	; 10 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
	pla
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if(n = 938 || n = 939 || n = 941 || n = 944 || n = 949 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1011 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1039 || n = 1043 || n = 1044 || n = 1046 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1078 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 940 || n = 945 || n = 951 || n = 955 || n = 960 || n = 965 || n = 970 || n = 975 || n = 980 || n = 990 || n = 995 || n = 1000 || n = 1005 || n = 1010 || n = 1015 || n = 1028 || n = 1030 || n = 1035 || n = 1040 || n = 1050 || n = 1060 || n = 1065 || n = 1070 || n = 1075 || n = 1080 || n = 1085)
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
.elseif(n = 948)
	; 9 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
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
.elseif(n = 966 || n = 1001 || n = 1036 || n = 1071)
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
.elseif(n = 991)
	; 10 bytes
	pha
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
.elseif(n = 993)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1038)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1045)
	; 10 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
	pla
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
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1121) || n = 1130 || (n > 1139 && n < 1156) || (n > 1164 && n < 1191) || (n > 1199 && n < 1226) || (n > 1234 && n < 1251)))
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
.elseif(n = 1106)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	tay
	pla
.elseif(n = 1108)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
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
.elseif(n = 1135)
	; 10 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1141 || n = 1211 || n = 1246)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1228)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
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
.elseif(n = 1253 || n = 1254 || n = 1256 || n = 1259 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1288 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1302 || n = 1309 || n = 1311 || n = 1316 || n = 1318 || n = 1322 || n = 1323)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1255 || n = 1260 || n = 1275 || n = 1280 || n = 1285 || n = 1290 || n = 1295 || n = 1301 || n = 1310)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1261)
	; 10 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
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
.elseif(n = 1270)
	; 10 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	nop
	iny
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
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1331 || n = 1338 || n = 1345 || n = 1359 || n = 1366)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1330 || n = 1336 || n = 1337 || n = 1339 || n = 1351 || n = 1355 || n = 1357 || n = 1358 || n = 1365 || n = 1367)
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
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1364)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1340 || n = 1348 || n = 1364)
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
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
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
	bit <$2C ;hides 'bit $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1344 || n = 1353 || n = 1362)
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
.elseif(n = 1347 || n = 1354)
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
.elseif(n = 1350)
	; 11 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
	pla
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
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1392 || n = 1404)
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
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1406)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1371 || n = 1380 || n = 1389)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1372 || n = 1374 || n = 1379 || n = 1381 || n = 1386 || n = 1393 || n = 1395 || n = 1400)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373 || n = 1390 || n = 1394 || n = 1399)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1375 || n = 1382)
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
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $68A0'
	ldy #104
	iny
	Jne *-4
	tay
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
	; 11 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	nop
	dey
	dey
	Jne *-4
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
.elseif(n = 1388)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1396)
	; 9 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
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
.elseif(n = 1401)
	; 11 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1402)
	; 11 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1403)
	; 11 bytes
	php
	delay_n (n-7)
	plp
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
.if(n = 1485 || n = 1499 || n = 1518 || n = 1520)
	; 11 bytes
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
.elseif(n = 1488 || n = 1497 || n = 1506)
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
.elseif(n = 1490 || n = 1491 || n = 1493 || n = 1498 || n = 1502 || n = 1505 || n = 1509 || n = 1519 || n = 1521)
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
	; 10 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1507)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif(n = 1511)
	; 12 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1515)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1522)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A8A0'
	ldy #168
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1523)
	; 10 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
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
.elseif(n = 1525 || n = 1526 || n = 1528 || n = 1540 || n = 1544 || n = 1547 || n = 1553 || n = 1554 || n = 1561)
	; 11..12 bytes
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
.elseif(n = 1534 || n = 1539 || n = 1546 || n = 1555 || n = 1562)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1535)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1536)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
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
.elseif(n = 1543)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1548)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1549)
	; 10 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif(n = 1557)
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
.elseif(n = 1558)
	; 10 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
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
.if(n = 1563 || n = 1568 || n = 1575 || n = 1582 || n = 1589 || n = 1591)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1564 || n = 1588)
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
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
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
	; 10 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1569 || n = 1578 || n = 1596)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1570)
	; 11 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1571)
	; 11 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1572 || n = 1584)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1574)
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
.elseif(n = 1579 || n = 1593 || n = 1600)
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
.elseif(n = 1581)
	; 11 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1583 || n = 1590 || n = 1597)
	; 11 bytes
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
.endif
.else
; 1602 to 1640
.if(n = 1602)
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
.elseif(n = 1603 || n = 1610 || n = 1616 || n = 1617 || n = 1619 || n = 1624 || n = 1631 || n = 1633 || n = 1638)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1604 || n = 1612 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1605 || n = 1614 || n = 1632)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1606)
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
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	delay_n 4
	nop
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 1611 || n = 1618 || n = 1623 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1620)
	; 9 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
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
.elseif(n = 1625)
	; 11 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 1626)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1627)
	; 11 bytes
	php
	delay_n (n-7)
	plp
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
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641 || n = 1655 || n = 1661)
	; 11..12 bytes
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
.elseif(n = 1645 || n = 1647 || n = 1652 || n = 1673 || n = 1679)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1651 || n = 1669 || n = 1674)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1648)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
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
.elseif(n = 1670)
	; 11 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
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
.elseif(n = 1683 || n = 1690 || n = 1697)
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
.elseif(n = 1718)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/14 ;hides 'nop'
	delay_n 7
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
	bit $24
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
.elseif(n = 1759 || n = 1765 || n = 1773 || n = 1778 || n = 1781)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1760)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
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
.elseif(n = 1782 || n = 1786 || n = 1795)
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
.elseif(n = 1792)
	; 10 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
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
.elseif(n = 1840 || n = 1849 || n = 1858 || n = 1863)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841 || n = 1854 || n = 1859 || n = 1862 || n = 1867 || n = 1870)
	; 12 bytes
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
	; 10 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
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
.elseif(n = 1851)
	; 12 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
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
	; 10 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
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
.elseif(n = 1861)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 1871)
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
.elseif(n = 1877 || n = 1878 || n = 1885 || n = 1894 || n = 1895 || n = 1898 || n = 1903)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1887 || n = 1889 || n = 1899 || n = 1905 || n = 1912)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
	; 11 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
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
.elseif(n = 1886)
	; 11 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
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
.elseif(n = 1890)
	; 11 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 1897)
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
	; 10 bytes
	pha
	tya
	ldy #(n-1)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
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
.elseif(n = 1913)
	; 11 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
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
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
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
.elseif(n = 1921 || n = 1930 || n = 1941)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1922 || n = 1931 || n = 1939 || n = 1949 || n = 1950)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1923)
	; 12 bytes
	php
	pha
	sec
	lda #212 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
	plp
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
	; 10 bytes
	pha
	tya
	ldy #(n-19)/9 ;hides 'nop zp,x'
	iny ;first loop only
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
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 1951)
	; 12 bytes
	pha
	tya
	ldy #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
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
.if((n >= 1954) && (n < 1973))
; 1954 to 1972
.if(n = 1954)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	dey
	Jne *-4
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
.elseif(n = 1956 || n = 1965)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1957 || n = 1966 || n = 1967)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1958)
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
.elseif(n = 1964)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1968)
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
.elseif(n = 1971)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1972)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 1973 to 1992
.if(n = 1973)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1974 || n = 1983 || n = 1992)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1975)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1976 || n = 1984 || n = 1985)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1977)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1978)
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	delay_n 9
	dey
	Jpl *-5
	tay
	pla
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
.elseif(n = 1980)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
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
.elseif(n = 1986)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
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
.elseif(n = 1988)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 2043 || n = 2049)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2074 || n = 2075 || n = 2077 || n = 2078 || n = 2093)
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
	; 10 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
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
.elseif(n = 2083 || n = 2092 || n = 2094 || n = 2101)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2085)
	; 12 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 2086)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2087 || n = 2096 || n = 2105)
	; 12 bytes
	php
	delay_n (n-7)
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
.if(n = 2110 || n = 2111 || n = 2114 || n = 2120 || n = 2129 || n = 2138 || n = 2147)
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
.elseif(n = 2113)
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
.elseif(n = 2115 || n = 2119 || n = 2128 || n = 2130 || n = 2137 || n = 2146)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2116)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop'
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
	lda #(n-15)/9 ;hides 'nop'
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
.elseif(n = 2123)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2125)
	; 10 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
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
.elseif(n = 2131)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
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
	; 10 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
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
.elseif(n = 2139)
	; 12 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
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
	; 10 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
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
.if(n = 2149)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2150 || n = 2155 || n = 2156 || n = 2164 || n = 2165 || n = 2173 || n = 2174 || n = 2176 || n = 2183 || n = 2185 || n = 2186)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
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
.elseif(n = 2191 || n = 2192 || n = 2194 || n = 2203 || n = 2213 || n = 2219 || n = 2222)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2193 || n = 2200 || n = 2209 || n = 2214 || n = 2218)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
	; 10 bytes
	pha
	tya
	ldy #(n-1)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
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
.elseif(n = 2204)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2205)
	; 13 bytes
	pha
	tya
	ldy #128
	delay_n 12
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2206)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
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
	; 10 bytes
	pha
	tya
	ldy #(n-19)/9 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
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
.if(n = 2227 || n = 2228 || n = 2230 || n = 2236 || n = 2237 || n = 2246 || n = 2248 || n = 2255 || n = 2258 || n = 2264)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229 || n = 2257 || n = 2263)
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
	; 13 bytes
	php
	delay_n (n-7)
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
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	dey
	Jne *-4
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
	; 12 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2254)
	; 11 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2259)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2260)
	; 10 bytes
	pha
	tya
	ldy #(n-10)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
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
	; 11 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
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
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop $04
	iny
	Jne *-4
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
.elseif(n = 2313 || n = 2319)
	; 12 bytes
	pha
	delay_n (n-3)
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
.if(n = 2324)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 2326 || n = 2339)
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
	; 11 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
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
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
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
.elseif(n = 2338)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2341)
	; 11 bytes
	pha
	tya
	ldy #(n-9)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2342)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/11 ;hides 'nop zp,x'
	delay_n 4
	dey
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
.elseif(n = 2345 || n = 2356 || n = 2367 || n = 2373 || n = 2379)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2346)
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2349)
	; 13 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
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
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
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
	ldy #(n-21)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2354 || n = 2361 || n = 2374)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
	; 13 bytes
	php
	pha
	txa
	pha
	lax #9
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
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
.endif
.else
; 2383 to 2421
.if(n = 2383 || n = 2395 || n = 2407)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2384 || n = 2389 || n = 2390 || n = 2394 || n = 2401 || n = 2402 || n = 2404 || n = 2414 || n = 2416)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2385 || n = 2391 || n = 2403 || n = 2411 || n = 2415)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2386)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D8A0'
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
.elseif(n = 2392 || n = 2406 || n = 2420)
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
.elseif(n = 2408)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
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
.elseif(n = 2413)
	; 13 bytes
	php
	pha
	sec
	lda #218 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
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
.if(n = 2422 || n = 2442 || n = 2452)
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
.elseif(n = 2423 || n = 2426 || n = 2428 || n = 2433 || n = 2438 || n = 2440 || n = 2445 || n = 2450)
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
.elseif(n = 2427 || n = 2437 || n = 2439 || n = 2449 || n = 2451)
	; 11..13 bytes
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
	ldy #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	iny
	Jne *-5
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
.elseif(n = 2444)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
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
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	bit $24
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 2461 to 2500
.if(n = 2461 || n = 2473 || n = 2499)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 11 = 1) && ((n > 2464 && n < 2499)))
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
.elseif(n = 2467 || n = 2479 || n = 2491)
	; 13 bytes
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
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
	iny
	Jne *-5
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
.elseif(n = 2474 || n = 2478 || n = 2486 || n = 2488)
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
.elseif(n = 2477 || n = 2489)
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
.elseif(n = 2480)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BEA0'
	ldy #190
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2481)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BEA0'
	ldy #190
	delay_n 4
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 2483)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CEA0'
	ldy #206
	bit $24
	dey
	Jne *-6
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
.elseif(n = 2493)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BFA0'
	ldy #191
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2494)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.if(n = 2501 || n = 2525 || n = 2537)
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
	bit $24
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
.elseif(n = 2505 || n = 2511 || n = 2513 || n = 2523 || n = 2535)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2506 || n = 2514 || n = 2522 || n = 2524 || n = 2533 || n = 2534 || n = 2536)
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
.elseif(n = 2510)
	; 11 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2516)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	bit $24
	inx
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2517)
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
.elseif(n = 2528)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
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
.elseif(n = 2543 || n = 2547 || n = 2559 || n = 2565 || n = 2571)
	; 12 bytes
	pha
	delay_n (n-3)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 2548)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2549 || n = 2561)
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
.elseif(n = 2554)
	; 12 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	nop
	nop $04
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2555 || n = 2558 || n = 2570 || n = 2576)
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
.elseif(n = 2566)
	; 12 bytes
	pha
	tya
	ldy #(n-22)/12 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 2581 || n = 2591 || n = 2593 || n = 2599 || n = 2611)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2582 || n = 2590 || n = 2594)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583 || n = 2587 || n = 2589 || n = 2598 || n = 2600 || n = 2609)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2584)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/11 ;hides 'nop'
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
.elseif(n = 2595)
	; 11 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	iny
	Jne *-4
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
	; 11 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2606)
	; 11 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
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
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	bit $24
	dey
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
	; 11 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2642 || n = 2646 || n = 2654 || n = 2656)
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
.elseif(n = 2620 || n = 2631 || n = 2643 || n = 2655)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2622 || n = 2637)
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
.elseif(n = 2623 || n = 2635 || n = 2647 || n = 2648)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2624)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2625 || n = 2638)
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
.elseif(n = 2626)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/12 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2627)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #10
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.if(n = 2657 || n = 2669 || n = 2681)
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
.elseif(n = 2658 || n = 2672 || n = 2686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2659 || n = 2671)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2660)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 2665 || n = 2666 || n = 2668 || n = 2678 || n = 2680 || n = 2687 || n = 2690 || n = 2692)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
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
	; 11 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 2693)
	; 11 bytes
	pha
	tya
	ldy #(n-9)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2694)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/11 ;hides 'nop zp,x'
	delay_n 4
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
.elseif(n = 2697 || n = 2703 || n = 2708 || n = 2715 || n = 2721)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2698 || n = 2732)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 2702 || n = 2704 || n = 2709 || n = 2720 || n = 2725 || n = 2726 || n = 2730)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2705)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/11 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
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
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	bit $24
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2713)
	; 14 bytes
	pha
	tya
	ldy #245
	delay_n 4
	nop
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
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
	; 12 bytes
	pha
	tya
	ldy #(n-7)/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
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
.elseif(n = 2733)
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
.elseif(n = 2737)
	; 14 bytes
	pha
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
	pla
.elseif(n = 2738 || n = 2744 || n = 2750 || n = 2752 || n = 2758)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2741)
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
.elseif(n = 2743 || n = 2755 || n = 2767)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
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
.elseif(n = 2753)
	; 12 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
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
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
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
.elseif(n = 2763 || n = 2769)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2764)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2765)
	; 12 bytes
	pha
	tya
	ldy #(n-9)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2766)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/13 ;hides 'nop zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2768)
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
.if(n = 2774 || n = 2776 || n = 2786 || n = 2788 || n = 2797)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2775 || n = 2785 || n = 2787 || n = 2799 || n = 2809 || n = 2811)
	; 12..13 bytes
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
.elseif(n = 2778)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2779)
	; 12 bytes
	pha
	tya
	ldy #(n-23)/13 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
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
	; 11 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
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
.elseif(n = 2791 || n = 2803)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2792)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	delay_n 4
	iny
	Jne *-6
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2802)
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
.if(n = 2813 || n = 2821 || n = 2823 || n = 2835 || n = 2847)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2824 || n = 2829 || n = 2834 || n = 2836 || n = 2846 || n = 2848)
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
.elseif(n = 2830)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2831)
	; 12 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
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
.elseif(n = 2838 || n = 2850)
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
.elseif(n = 2839 || n = 2851)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2841)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2842)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop $04
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2843)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 2853 || n = 2858 || n = 2860 || n = 2864 || n = 2869 || n = 2870 || n = 2872)
	; 12..14 bytes
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
.elseif(n = 2855 || n = 2859 || n = 2871 || n = 2883)
	; 12 bytes
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
.elseif(n = 2861 || n = 2873 || n = 2885)
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
.elseif(n = 2862 || n = 2877)
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
.elseif(n = 2863 || n = 2875 || n = 2887 || n = 2889)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2865)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 2874)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2876)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 2884)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #11
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
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
.elseif(n = 2888)
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
.elseif(n = 2893 || n = 2894 || n = 2905 || n = 2906 || n = 2908 || n = 2912 || n = 2918 || n = 2920 || n = 2925)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895 || n = 2907 || n = 2913 || n = 2919 || n = 2929)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2896 || n = 2910 || n = 2924)
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
	; 13..14 bytes
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
.elseif(n = 2902)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 2926)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
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
.if(n = 2930 || n = 2932 || n = 2942 || n = 2944 || n = 2954 || n = 2956 || n = 2960 || n = 2965)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2931 || n = 2941 || n = 2943 || n = 2955 || n = 2967)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2934 || n = 2946 || n = 2958)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2937)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
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
.elseif(n = 2948)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2949)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 4
	inx
	Jne *-6
	tax
	pla
.elseif(n = 2950)
	; 12 bytes
	pha
	tya
	ldy #(n-22)/12 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
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
.elseif(n = 2963)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2971 || n = 2973 || n = 2983 || n = 2995 || n = 3007)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2972)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
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
	pha
	tya
	ldy #(n-9)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2978 || n = 2984 || n = 2990 || n = 2992 || n = 2996 || n = 3002)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2979 || n = 2991 || n = 3003)
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
.elseif(n = 2982 || n = 2997)
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
.elseif(n = 2985)
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
.elseif(n = 2986)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	bit $24
	dey
	Jne *-6
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
.elseif(n = 3009 || n = 3014 || n = 3016 || n = 3026 || n = 3028 || n = 3032 || n = 3037 || n = 3038 || n = 3040 || n = 3044 || n = 3045)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/12 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3015 || n = 3025 || n = 3027 || n = 3039)
	; 12..13 bytes
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
.elseif(n = 3018 || n = 3030)
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
.elseif(n = 3019 || n = 3031 || n = 3043)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3020)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3021)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	bit $24
	iny
	Jne *-6
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
.elseif(n = 3049 || n = 3056 || n = 3061 || n = 3066 || n = 3073 || n = 3074)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3051 || n = 3063 || n = 3075 || n = 3081)
	; 12..13 bytes
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
.elseif(n = 3053)
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
.elseif(n = 3054 || n = 3080)
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
.elseif(n = 3057)
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
.elseif(n = 3062)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3065)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3067 || n = 3069 || n = 3079)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3076)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3077)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
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
.elseif(n = 3085)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	nop
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3088 || n = 3094 || n = 3096 || n = 3103 || n = 3107 || n = 3108 || n = 3110 || n = 3121 || n = 3122)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087 || n = 3095)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3089)
	; 13 bytes
	php
	pha
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3090 || n = 3109)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3091)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 3093 || n = 3119)
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
.elseif(n = 3097 || n = 3125)
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
.elseif(n = 3101)
	; 13 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
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
.elseif(n = 3111)
	; 13 bytes
	pha
	tya
	ldy #(n-7)/16 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3112)
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
.elseif(n = 3113)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3123)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3124)
	; 13 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
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
.if(n = 3126)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3127 || n = 3155)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3128)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3129)
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
.elseif(n = 3132 || n = 3147)
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
.elseif(n = 3133)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3134 || n = 3148 || n = 3162)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3135 || n = 3137 || n = 3151)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3136 || n = 3142 || n = 3149 || n = 3150 || n = 3152 || n = 3159 || n = 3160)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3139 || n = 3153)
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
.elseif(n = 3140 || n = 3157)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3141)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #12
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
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
.elseif(n = 3144)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
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
.elseif(n = 3146)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3154)
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
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 18
	Jne *-7
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
.elseif(n = 3164)
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
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165 || n = 3172)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3166)
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
.elseif(n = 3168 || n = 3173)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3169)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.elseif(n = 3171)
	; 12 bytes
	pha
	tya
	ldy #(n-7)/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3174)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3175)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3176)
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
.elseif(n = 3178)
	; 12 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
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
.elseif(n = 3180)
	; 12 bytes
	pha
	tya
	ldy #(n-8)/13 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3181)
	; 12 bytes
	pha
	tya
	ldy #(n-9)/13 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3182)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/13 ;hides 'nop zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3183)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3184 to 3203
.if(n = 3184 || n = 3197)
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
.elseif(n = 3185)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3186 || n = 3187 || n = 3199)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3188)
	; 13 bytes
	pha
	tya
	ldy #(n-8)/15 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3189)
	; 13 bytes
	pha
	tya
	ldy #(n-9)/15 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3190)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3191)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3192)
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
.elseif(n = 3193 || n = 3198)
	; 13 bytes
	pha
	delay_n (n-3)
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
	; 12 bytes
	pha
	tya
	ldy #(n-23)/13 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.elseif(n = 3200)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3201)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3202)
	; 13 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3203)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	nop
	bit $24
	dey
	Jne *-7
	tay
	pla
.endif
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
	ldy #(n-25)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3206 || n = 3214 || n = 3220 || n = 3229 || n = 3234 || n = 3238)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3207 || n = 3222 || n = 3237)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 3212 || n = 3228)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3213 || n = 3233 || n = 3240)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3215)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $B2A0'
	ldy #178
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3216)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3217 || n = 3239)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3219)
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
.elseif(n = 3223 || n = 3236)
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
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
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
.elseif(n = 3230)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 3241)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
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
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	nop
	bit $24
	inx
	Jne *-7
	tax
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
.elseif(n = 3248 || n = 3250 || n = 3254 || n = 3271 || n = 3278)
	; 13..14 bytes
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
.elseif(n = 3251 || n = 3279)
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
.elseif(n = 3252 || n = 3267)
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
.elseif(n = 3253 || n = 3265 || n = 3281)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3255 || n = 3261 || n = 3272 || n = 3277)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3256 || n = 3257)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 3268)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $B5A0'
	ldy #181
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3269)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3270)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3273)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 4
	iny
	Jne *-6
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
.if(n = 3282 || n = 3297 || n = 3312)
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
.elseif(n = 3283 || n = 3285 || n = 3289 || n = 3305 || n = 3309 || n = 3319)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3284 || n = 3294 || n = 3298 || n = 3304 || n = 3315)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3286)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/14 ;hides 'nop'
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
.elseif(n = 3288 || n = 3302 || n = 3316)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3290)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3291)
	; 13 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3292 || n = 3308)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3293 || n = 3295 || n = 3303)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3306)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 3310)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 3320)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3335 || n = 3349)
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
.elseif(n = 3324 || n = 3356)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3327 || n = 3342 || n = 3357)
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
.elseif(n = 3328 || n = 3331 || n = 3332 || n = 3334 || n = 3341 || n = 3346 || n = 3348 || n = 3359)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3329 || n = 3333 || n = 3345 || n = 3347)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3351)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3352 || n = 3353)
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
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3362 || n = 3373 || n = 3374 || n = 3380)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3361 || n = 3378 || n = 3395)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3363 || n = 3377 || n = 3391)
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
.elseif(n = 3364 || n = 3392)
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
.elseif(n = 3365 || n = 3379 || n = 3393)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 3365 && n < 3370) || n = 3382 || n = 3383)
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
.elseif(n = 3375 || n = 3394)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3376)
	; 13 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3381 || n = 3389)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3384)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3385)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	bit $24
	iny
	Jne *-7
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
	delay_n (n-4)
	delay_n 4
.elseif(n = 3396)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3397)
	; 12 bytes
	pha
	tya
	ldy #(n-5)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3398)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #13
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3406 || n = 3415 || n = 3416 || n = 3422 || n = 3430 || n = 3437)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3400 || n = 3414 || n = 3428)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3401 || n = 3405 || n = 3421 || n = 3431)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3402)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/16 ;hides 'nop zp,x'
	delay_n 9
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
.elseif(n = 3407)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3408)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3409)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3413)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3417 || n = 3432)
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
.elseif(n = 3418)
	; 12 bytes
	pha
	tya
	ldy #(n-26)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
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
.elseif(n = 3424)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3425)
	; 13 bytes
	pha
	tya
	ldy #(n-9)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3426)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3427)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3433)
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
.elseif(n = 3434)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3435)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 7
	dey
	Jne *-6
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
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3439 || n = 3445 || n = 3459 || n = 3473)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3440)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	nop $04
	dey
	Jne *-6
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
	; 14 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3444 || n = 3448 || n = 3454 || n = 3458 || n = 3464 || n = 3465 || n = 3472 || n = 3474)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3446 || n = 3463)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 3449)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3450)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3452 || n = 3468)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3453)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3455)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3457)
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
.elseif(n = 3460)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3461)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	php
	cmp $C5
	plp
	Jne *-6
	tax
	pla
.elseif(n = 3466)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D8A0'
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
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3469)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3471)
	; 14 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
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
.if((n >= 3477) && (n < 3496))
; 3477 to 3495
.if(n = 3477 || n = 3492)
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
	ldy #52 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3479 || n = 3486 || n = 3488 || n = 3494)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3480)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3481)
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
.elseif(n = 3482)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3484)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3485)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $3FA0'
	ldy #63
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 3489)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
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
.elseif(n = 3491)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3493)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3495)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 3496 to 3515
.if(n = 3496)
	; 13 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	delay_n 14
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3497 || n = 3514)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3498 || n = 3512)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3501 || n = 3513)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3504)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3505)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3506)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 12
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3507)
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
.elseif(n = 3509)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3510)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3511)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
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
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3532)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3517 || n = 3519 || n = 3533)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3518 || n = 3524 || n = 3528 || n = 3534 || n = 3539 || n = 3542 || n = 3544)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3520)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/15 ;hides 'nop'
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
.elseif(n = 3522 || n = 3537 || n = 3552)
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
.elseif(n = 3523 || n = 3529 || n = 3543)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3525)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3526 || n = 3540 || n = 3554)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3535)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	nop
	iny
	Jne *-6
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
.elseif(n = 3545)
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
.elseif(n = 3549)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3550)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3551)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3553)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3557 || n = 3571 || n = 3577 || n = 3585)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3556 || n = 3558 || n = 3566 || n = 3569 || n = 3570 || n = 3572)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3559 || n = 3573 || n = 3587)
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
.elseif(n = 3560 || n = 3588)
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
.elseif(n = 3561 || n = 3575 || n = 3589)
	; 14 bytes
	php
	delay_n (n-7)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 3574)
	; 13 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3576)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3578)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3579)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3581)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3583)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	delay_n 11
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3591)
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
.elseif(n = 3592)
	; 13 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
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
.if(n = 3594)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3597 || n = 3612 || n = 3627)
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
.elseif(n = 3598 || n = 3600 || n = 3605 || n = 3613 || n = 3614 || n = 3618 || n = 3629)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599 || n = 3616)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3601)
	; 14 bytes
	php
	pha
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3602)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3603 || n = 3622)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3604 || (n > 3606 && n < 3610) || n = 3617 || n = 3620 || n = 3621 || n = 3632)
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
.elseif(n = 3606 || n = 3615 || n = 3619)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3623)
	; 13 bytes
	pha
	tya
	ldy #(n-7)/16 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3624)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3625)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 4
	nop
	iny
	Jne *-7
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
.elseif(n = 3630)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3631)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 3633 to 3671
.if(n = 3633 || n = 3650 || n = 3667)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3634 || (n > 3636 && n < 3640) || n = 3649 || n = 3651 || n = 3654)
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
.elseif(n = 3635 || n = 3646 || n = 3652 || n = 3661 || n = 3662)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636 || n = 3653 || n = 3663)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3640)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	nop
	inx
	Jne *-7
	tax
	pla
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
.elseif(n = 3642 || n = 3657)
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
.elseif(n = 3644 || n = 3660)
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
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
.elseif(n = 3648 || n = 3666)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3655)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #14
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
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
.elseif(n = 3664)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 11
	Jne *-6
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
.elseif(n = 3668)
	; 13 bytes
	pha
	tya
	ldy #(n-8)/15 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3669)
	; 13 bytes
	pha
	tya
	ldy #(n-9)/15 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3670)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3671)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3687 || n = 3702)
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
.elseif(n = 3673 || n = 3680 || n = 3688 || n = 3693 || n = 3705 || n = 3709)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3676 || n = 3692 || n = 3708)
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
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3678 || n = 3686 || n = 3689 || n = 3694 || n = 3703 || n = 3704 || n = 3710)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3681)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3682)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 11
	iny
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
.elseif(n = 3684 || n = 3701)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3685)
	; 13 bytes
	pha
	tya
	ldy #(n-25)/15 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3690)
	; 12 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
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
	; 14 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3696)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3697)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3699)
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
.elseif(n = 3700)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
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
.endif
.else
; 3711 to 3750
.if(n = 3711 || n = 3721 || n = 3725 || n = 3743)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3712)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 3713 || n = 3729)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3714 || n = 3733 || n = 3744)
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
.elseif(n = 3715)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3716)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/17 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3717 || n = 3732 || n = 3747)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3719 || n = 3720 || n = 3726 || n = 3737 || n = 3741 || n = 3742 || n = 3748 || n = 3749)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3723)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3724 || n = 3740)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3727)
	; 14 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3728)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3730)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	delay_n 4
	nop
	dey
	Jne *-7
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
.elseif(n = 3738)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 7
	dey
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
.elseif(n = 3745)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 4
	nop
	iny
	Jne *-7
	tay
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
.elseif(n = 3752 || n = 3769)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3753 || n = 3758 || n = 3764 || n = 3776 || n = 3779 || n = 3783)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3754)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3755 || n = 3774)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3756 || n = 3772 || n = 3788)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3757 || n = 3763 || n = 3765 || n = 3773 || n = 3780 || n = 3784 || n = 3789)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3759)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3760)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/15 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3761)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3762 || n = 3777)
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
.elseif(n = 3766 || n = 3767 || n = 3782)
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
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
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
.elseif(n = 3775)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 4
	nop
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3778)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 3781)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3785)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3786)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
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
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3794 || n = 3795 || n = 3806 || n = 3809 || n = 3814 || n = 3823 || n = 3824)
	; 14 bytes
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
.elseif(n = 3792 || n = 3807 || n = 3822)
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
.elseif(n = 3793 || n = 3812)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3796 || n = 3805 || n = 3811 || n = 3815)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3797 || (n > 3798 && n < 3802) || n = 3816 || n = 3817)
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
.elseif(n = 3798)
	; 14 bytes
	pha
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
	pla
.elseif(n = 3802)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3803 || n = 3820)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 3808)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3810)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 12
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3813)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
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
	; 13 bytes
	pha
	tya
	ldy #(n-5)/18 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3825)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3826)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/18 ;hides 'nop zp,x'
	delay_n 11
	dey
	Jne *-6
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
.elseif(n = 3828)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829 || n = 3840 || n = 3847 || n = 3857)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3830 || n = 3832 || (n > 3840 && n < 3844) || n = 3848 || (n > 3857 && n < 3862) || n = 3865)
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
.elseif(n = 3831 || n = 3850)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3833 || n = 3838 || n = 3839 || n = 3853 || n = 3856)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3844)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
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
.elseif(n = 3846 || n = 3864)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3849)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
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
	; 13 bytes
	pha
	tya
	ldy #0
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3854)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3855)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3862)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 3866)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3867)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
	pla
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3884 || n = 3900)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3870 || n = 3872 || n = 3877 || n = 3887 || n = 3891 || n = 3901 || n = 3903)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3871 || n = 3888 || n = 3905)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3873 || n = 3885 || n = 3886 || n = 3890 || n = 3902)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3874)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3875 || n = 3878 || n = 3879 || n = 3889 || (n > 3891 && n < 3897) || n = 3906)
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
.elseif(n = 3876 || n = 3897)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3880)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3882)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	tax
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
.elseif(n = 3898)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
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
.elseif(n = 3904)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	sta (@ptrtemp),y
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3908 || n = 3910 || n = 3917 || n = 3923 || n = 3925 || n = 3929 || n = 3933 || n = 3942)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3909)
	; 12 bytes
	pha
	tya
	ldy #(n-5)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3911 || n = 3924 || n = 3928 || n = 3941)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3912)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #15
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 3913)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3914)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/16 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
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
.elseif(n = 3916 || n = 3932)
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3919)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3920)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3921)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3922 || n = 3939)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3927 || n = 3938 || n = 3940 || n = 3944)
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
.elseif(n = 3930)
	; 12 bytes
	pha
	tya
	ldy #(n-26)/16 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
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
.elseif(n = 3934)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/18 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3935)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3937)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	.repeat 4
	 nop
	.endrepeat
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 3943)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3948 || n = 3964 || n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3949 || n = 3950 || n = 3958 || n = 3965 || n = 3966 || n = 3975 || n = 3982)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3951 || n = 3957 || n = 3959 || n = 3967 || n = 3976)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3952)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	delay_n 9
	dey
	Jne *-7
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
.elseif(n = 3954)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	rol $26
	ror $26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3955 || n = 3968 || n = 3969 || n = 3974 || n = 3977 || n = 3984)
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
.elseif(n = 3956 || n = 3973)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3960 || n = 3981)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3961)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3970)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3971)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	rol $26
	ror $26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3972)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3978)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	delay_n 7
	dey
	Jne *-6
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
.if((n >= 3985) && (n < 4024))
; 3985 to 4023
.if(n = 3985)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3986 || n = 3991 || n = 3993 || n = 3999 || n = 4013)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3987 || n = 4000 || n = 4001 || n = 4003 || n = 4018)
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
.elseif(n = 3988)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/17 ;hides 'nop'
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
.elseif(n = 3990 || n = 4007)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3992 || n = 3997 || n = 3998 || n = 4004 || n = 4009 || n = 4014)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3996 || n = 4012)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4002 || n = 4021)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4005)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4008)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4010)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4011)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4015)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4016)
	; 14 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4017)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4019)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4020)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4022)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4023)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4041 || n = 4058)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 4026)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 4028 || n = 4044 || n = 4060)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4029 || n = 4030 || n = 4045 || n = 4046 || n = 4061)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4031 || n = 4047)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4032 || (n > 4033 && n < 4037) || (n > 4047 && n < 4057))
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
.elseif(n = 4033)
	; 14 bytes
	pha
	tya
	ldy #(n-5)/19 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4037)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/19 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4038)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4039)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4040 || n = 4059)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4057)
	; 14 bytes
	pha
	tya
	ldy #(n-29)/19 ;hides 'nop zp,x'
	iny ;first loop only
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4062)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
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
.if(n = 4063 || n = 4095 || n = 4100)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4064 || (n > 4065 && n < 4070) || (n > 4070 && n < 4074) || (n > 4081 && n < 4085) || n = 4087 || n = 4101)
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
.elseif(n = 4065 || n = 4086)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4070)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	.repeat 4
	 nop
	.endrepeat
	Jne *-7
	tax
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
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
.elseif(n = 4077 || n = 4093 || n = 4094 || n = 4099)
	; 14 bytes
	delay_n (n-2)
	nop
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
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4080 || n = 4098)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4081)
	; 14 bytes
	pha
	tya
	ldy #(n-7)/21 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4085)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4088)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4089)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
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
.if((n > 4101 && n < 4106) || n = 4113 || n = 4117 || (n > 4119 && n < 4125) || n = 4130 || n = 4131 || n = 4136 || n = 4139 || n = 4140)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4110 || n = 4111 || n = 4118 || n = 4128 || n = 4137)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4112 || n = 4119 || n = 4129 || n = 4138)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4114)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4115)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4116 || n = 4135)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4125)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4127)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4132)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4134)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4142)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4143 || n = 4160 || n = 4177 || n = 4194 || n = 4211)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4144 || n = 4148 || n = 4159 || n = 4161 || (n > 4163 && n < 4168) || n = 4174 || n = 4178 || (n > 4180 && n < 4186) || n = 4193 || (n > 4197 && n < 4204) || (n > 4206 && n < 4210) || (n > 4214 && n < 4219))
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
.elseif(n = 4145 || n = 4156 || n = 4162 || n = 4171 || n = 4175 || n = 4179 || n = 4196 || n = 4213)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146 || n = 4153 || n = 4155 || n = 4163 || n = 4172 || n = 4176 || n = 4180 || n = 4195 || n = 4197 || n = 4214)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4147)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4149 || n = 4191 || n = 4212)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4150)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 11
	iny
	Jne *-6
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
.elseif(n = 4152)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop'
	delay_n 12
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4154 || n = 4173 || n = 4192)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4157)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4158)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
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
	php
	pha
	txa
	pha
	lax #16
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4170 || n = 4188 || n = 4206)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
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
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4189)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 12
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4190)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
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
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 14
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if((n > 4218 && n < 4222) || n = 4226 || n = 4227 || n = 4229 || (n > 4233 && n < 4240) || n = 4244 || n = 4246 || n = 4266 || n = 4267 || n = 4269 || n = 4272 || n = 4274 || n = 4280 || (n > 4282 && n < 4287) || n = 4288 || (n > 4290 && n < 4294))
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
.elseif(n = 4222)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/18 ;hides 'nop'
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
.elseif(n = 4224 || n = 4242 || n = 4278)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4225 || n = 4231 || n = 4243 || n = 4252 || n = 4256 || n = 4261 || n = 4263 || n = 4265 || n = 4271 || n = 4282 || n = 4290)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4228 || n = 4245 || n = 4262 || n = 4279 || n = 4296)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4232 || n = 4247 || n = 4251 || n = 4255 || n = 4264 || n = 4270 || n = 4281 || n = 4289)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4233 || n = 4254 || n = 4275)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4240)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
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
.elseif(n = 4248)
	; 14 bytes
	pha
	tya
	ldy #(n-8)/20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4250)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop zp,x'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4253)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4257)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4258)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4259)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4260)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/17 ;hides 'nop'
	rol $26
	ror $26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4273)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
	tax
	pla
.elseif(n = 4276)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
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
.elseif(n = 4294)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4295)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 4297 to 4375
.if(n = 4297 || (n > 4299 && n < 4306) || n = 4307 || n = 4310 || n = 4311 || (n > 4317 && n < 4325) || n = 4326 || n = 4329 || (n > 4333 && n < 4338) || (n > 4338 && n < 4344) || n = 4345 || (n > 4350 && n < 4359) || (n > 4359 && n < 4363) || n = 4369 || (n > 4370 && n < 4376))
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
.elseif(n = 4298 || n = 4308 || n = 4315 || n = 4327 || n = 4332 || n = 4346 || n = 4349 || n = 4365 || n = 4366)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4309 || n = 4316 || n = 4328 || n = 4333)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4312)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4313 || n = 4330 || n = 4347)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/17
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4314 || n = 4350 || n = 4368)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4317 || n = 4338 || n = 4359)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
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
.elseif(n = 4364)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 12
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4367)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4370)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
; 4376 to 4453
.if((n > 4375 && n < 4380) || n = 4381 || (n > 4387 && n < 4391) || (n > 4391 && n < 4397) || n = 4398 || (n > 4405 && n < 4411) || (n > 4411 && n < 4418) || n = 4419 || n = 4424 || n = 4425 || n = 4427 || n = 4430 || (n > 4431 && n < 4438) || (n > 4443 && n < 4454))
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
.elseif(n = 4380)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 12
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4382 || n = 4401 || n = 4420 || n = 4439)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4383 || n = 4387 || n = 4400 || n = 4405 || n = 4423 || n = 4429 || n = 4442)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4384)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4385)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4386 || n = 4404 || n = 4422 || n = 4440)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4391)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 12
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4397)
	; 13 bytes
	pha
	tya
	ldy #(n-5)/18 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4399)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 4402)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/18 ;hides 'nop zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4403 || n = 4428 || n = 4441)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4411)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 12
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4418)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 12
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4421)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4426)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #17
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4431)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 12
	iny
	Jne *-7
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
.elseif(n = 4443)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4455 || n = 4463 || (n > 4464 && n < 4474) || n = 4478 || n = 4481 || n = 4482 || n = 4484 || (n > 4485 && n < 4489) || n = 4490 || n = 4491 || n = 4495 || n = 4497 || (n > 4499 && n < 4506) || (n > 4506 && n < 4510) || n = 4514 || n = 4516 || (n > 4518 && n < 4527) || n = 4531)
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
.elseif(n = 4456)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop'
	delay_n 12
	dey
	Jne *-6
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
.elseif(n = 4458 || n = 4477 || n = 4496 || n = 4515)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4459 || n = 4461 || n = 4480 || n = 4499 || n = 4518)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4460 || n = 4479 || n = 4498 || n = 4517)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4462)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/21 ;hides 'nop zp,x'
	nop
	delay_n 12
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4464 || n = 4485 || n = 4506 || n = 4527)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4474)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4475)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4476)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	php
	rol $26
	ror $26
	plp
	Jne *-7
	tax
	pla
.elseif(n = 4483)
	; 14 bytes
	pha
	tya
	ldy #(n-31)/21 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 12
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4489)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4492)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4493)
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4510)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/18 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
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
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	rol $26
	ror $26
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
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4533 || n = 4535 || (n > 4537 && n < 4546) || (n > 4548 && n < 4553) || n = 4554 || (n > 4556 && n < 4564) || n = 4567 || n = 4568 || n = 4570 || n = 4571 || n = 4573 || (n > 4575 && n < 4582) || n = 4585 || n = 4586 || n = 4589 || n = 4592 || (n > 4594 && n < 4600) || (n > 4602 && n < 4610))
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
.elseif(n = 4534 || n = 4553 || n = 4572 || n = 4591)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4536 || n = 4555 || n = 4574 || n = 4593)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4537 || n = 4556 || n = 4575 || n = 4594)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4546)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 4548 || n = 4566 || n = 4584 || n = 4602)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4564)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4565)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4569 || n = 4590)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4587)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 14
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4588)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/21 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4600)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4601)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 4610 to 4687
.if(n = 4610 || n = 4629 || n = 4648 || n = 4667 || n = 4686)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4611 || n = 4632 || n = 4653 || n = 4674)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4612 || n = 4631 || n = 4650 || n = 4669 || n = 4685)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4613 || n = 4651 || n = 4670)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n > 4613 && n < 4618) || (n > 4620 && n < 4627) || n = 4628 || (n > 4632 && n < 4641) || (n > 4641 && n < 4645) || n = 4647 || n = 4649 || n = 4652 || (n > 4653 && n < 4665) || n = 4666 || n = 4668 || (n > 4670 && n < 4674) || (n > 4674 && n < 4679) || (n > 4679 && n < 4683) || n = 4684 || n = 4687)
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
.elseif(n = 4618)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4619)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4620)
	; 14 bytes
	pha
	tya
	ldy #0
	nop
	delay_n 11
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4627)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4630)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 4641)
	; 14 bytes
	pha
	tya
	ldy #(n-5)/19 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4645)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/19 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4646)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4665)
	; 14 bytes
	pha
	tya
	ldy #(n-29)/19 ;hides 'nop zp,x'
	iny ;first loop only
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4679)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
	tax
	pla
.elseif(n = 4683)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #18
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if(n = 4688 || n = 4707 || n = 4726 || n = 4745 || n = 4764 || n = 4783 || n = 4802)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4689 || n = 4708 || n = 4727 || n = 4746 || n = 4763 || n = 4765 || n = 4784 || n = 4803 || n = 4822 || n = 4841 || n = 4843)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4690)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif((n > 4690 && n < 4695) || n = 4696 || (n > 4697 && n < 4702) || n = 4703 || n = 4704 || n = 4706 || n = 4709 || (n > 4710 && n < 4716) || (n > 4716 && n < 4724) || (n > 4730 && n < 4737) || (n > 4737 && n < 4743) || n = 4744 || (n > 4746 && n < 4750) || n = 4751 || n = 4752 || (n > 4753 && n < 4758) || n = 4759 || n = 4761 || (n > 4765 && n < 4779) || n = 4780 || n = 4782 || (n > 4784 && n < 4800) || n = 4801 || (n > 4803 && n < 4819) || n = 4820 || (n > 4822 && n < 4829) || (n > 4829 && n < 4838) || n = 4839)
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
.elseif(n = 4695 || n = 4716 || n = 4737 || n = 4758 || n = 4779 || n = 4821 || n = 4842)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4697)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4702)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4705 || n = 4724 || n = 4743 || n = 4762 || n = 4781 || n = 4800 || n = 4819 || n = 4838)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4710)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 11
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4725)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4728)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4729)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4730)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 11
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4750)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 11
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4753)
	; 14 bytes
	pha
	tya
	ldy #(n-7)/21 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4760)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop'
	delay_n 12
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4829)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4840)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 14
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 4844 to 5000
.if(n = 4844 || n = 4845 || (n > 4846 && n < 4851) || (n > 4851 && n < 4857) || n = 4858 || n = 4862 || (n > 4863 && n < 4870) || n = 4871 || n = 4872 || n = 4874 || n = 4875 || n = 4877 || n = 4880 || n = 4883 || n = 4887 || (n > 4890 && n < 4905) || (n > 4905 && n < 4909) || (n > 4910 && n < 4924) || n = 4925 || (n > 4927 && n < 4940) || n = 4941 || n = 4944 || n = 4946 || (n > 4947 && n < 4966) || n = 4967 || (n > 4968 && n < 4987) || n = 4988 || (n > 4989 && n < 5001))
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
.elseif(n = 4851)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4859 || n = 4878 || n = 4942)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4860 || n = 4879 || n = 4927 || n = 4943)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4861)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 12
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 4863 || n = 4884 || n = 4905 || n = 4926 || n = 4947 || n = 4968 || n = 4989)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4870)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4873)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
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
.elseif(n = 4881)
	; 14 bytes
	pha
	tya
	ldy #(n-5)/23 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4885)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4886)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop zp,x'
	delay_n 16
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4888)
	; 14 bytes
	pha
	tya
	ldy #(n-8)/20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4889)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4890)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop zp,x'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4909)
	; 14 bytes
	pha
	tya
	ldy #(n-33)/23 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4910)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4924)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/21 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4940)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #19
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 4945)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 12
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4966)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 12
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4987)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 12
	iny
	Jne *-7
	tay
	pla
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
; 5001 to 5625
.if((n > 5000 && n < 5010) || (n > 5010 && n < 5024) || (n > 5024 && n < 5031) || (n > 5031 && n < 5052) || (n > 5052 && n < 5073) || (n > 5073 && n < 5094) || (n > 5094 && n < 5113) || n = 5114 || (n > 5115 && n < 5134) || n = 5135 || (n > 5136 && n < 5155) || n = 5156 || (n > 5157 && n < 5178) || (n > 5178 && n < 5197) || n = 5198 || (n > 5200 && n < 5220) || (n > 5220 && n < 5241) || (n > 5241 && n < 5260) || n = 5261 || (n > 5263 && n < 5283) || (n > 5283 && n < 5288) || (n > 5288 && n < 5300) || (n > 5300 && n < 5304) || (n > 5304 && n < 5323) || n = 5324 || (n > 5325 && n < 5346) || (n > 5346 && n < 5367) || (n > 5367 && n < 5388) || (n > 5388 && n < 5392) || (n > 5392 && n < 5415) || (n > 5415 && n < 5433) || (n > 5433 && n < 5438) || (n > 5438 && n < 5454) || n = 5455 || (n > 5457 && n < 5461) || (n > 5461 && n < 5599) || (n > 5599 && n < 5617) || (n > 5617 && n < 5622) || (n > 5622 && n < 5626))
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
.elseif(n = 5010 || n = 5031 || n = 5052 || n = 5073 || n = 5094 || n = 5115 || n = 5136 || n = 5157 || n = 5178 || n = 5220 || n = 5241 || n = 5262 || n = 5283 || n = 5304 || n = 5325 || n = 5346 || n = 5367)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5024 || n = 5392)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5113)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 12
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5134)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/21 ;hides 'nop zp,x'
	nop
	delay_n 12
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5155)
	; 14 bytes
	pha
	tya
	ldy #(n-31)/21 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 12
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5197)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #20
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 5199 || n = 5456)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5200 || n = 5263 || n = 5457)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5260)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/21 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5288)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 21
	Jne *-7
	tax
	pla
.elseif(n = 5300)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5323)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 5388)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5415)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5433)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5438)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5454)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #21
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 5461)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5599)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5617)
	; 14 bytes
	pha
	tya
	ldy #(n-5)/23 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5622)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop zp,x'
	delay_n 16
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 5626 to 6250
.if((n > 5625 && n < 5645) || (n > 5645 && n < 5711) || n = 5712 || (n > 5714 && n < 5760) || (n > 5760 && n < 5968) || n = 5969 || (n > 5971 && n < 6225) || n = 6226 || (n > 6228 && n < 6251))
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
.elseif(n = 5645)
	; 14 bytes
	pha
	tya
	ldy #(n-33)/23 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5711)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #22
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 5713 || n = 5970 || n = 6227)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5714 || n = 5971 || n = 6228)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5760)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5968)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #23
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6225)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #24
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6482) || n = 6483 || (n > 6485 && n < 6739) || n = 6740 || (n > 6742 && n < 6996) || n = 6997 || (n > 6999 && n < 7253) || n = 7254 || (n > 7256 && n < 7501))
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
.elseif(n = 6482)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #25
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6484 || n = 6741 || n = 6998 || n = 7255)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6485 || n = 6742 || n = 6999 || n = 7256)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6739)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #26
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 6996)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #27
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 7253)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #28
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7510) || n = 7511 || (n > 7513 && n < 7767) || n = 7768 || (n > 7770 && n < 8024) || n = 8025 || (n > 8027 && n < 8281) || n = 8282 || (n > 8284 && n < 8538) || n = 8539 || (n > 8541 && n < 8795) || n = 8796 || (n > 8798 && n < 9052) || n = 9053 || (n > 9055 && n < 9309) || n = 9310 || (n > 9312 && n < 9566) || n = 9567 || (n > 9569 && n < 9823) || n = 9824 || (n > 9826 && n < 10001))
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
.elseif(n = 7510)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #29
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 7512 || n = 7769 || n = 8026 || n = 8283 || n = 8540 || n = 8797 || n = 9054 || n = 9311 || n = 9568 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7513 || n = 7770 || n = 8027 || n = 8284 || n = 8541 || n = 8798 || n = 9055 || n = 9312 || n = 9569 || n = 9826)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7767)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #30
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8024)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #31
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8281)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #32
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8538)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #33
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 8795)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #34
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9052)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #35
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9309)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #36
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9566)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #37
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 9823)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #38
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10080) || n = 10081 || (n > 10083 && n < 10337) || n = 10338 || (n > 10340 && n < 10594) || n = 10595 || (n > 10597 && n < 10851) || n = 10852 || (n > 10854 && n < 11108) || n = 11109 || (n > 11111 && n < 11365) || n = 11366 || (n > 11368 && n < 11622) || n = 11623 || (n > 11625 && n < 11879) || n = 11880 || (n > 11882 && n < 12136) || n = 12137 || (n > 12139 && n < 12393) || n = 12394 || (n > 12396 && n < 12650) || n = 12651 || (n > 12653 && n < 12907) || n = 12908 || (n > 12910 && n < 13164) || n = 13165 || (n > 13167 && n < 13421) || n = 13422 || (n > 13424 && n < 13678) || n = 13679 || (n > 13681 && n < 13935) || n = 13936 || (n > 13938 && n < 14192) || n = 14193 || (n > 14195 && n < 14449) || n = 14450 || (n > 14452 && n < 14706) || n = 14707 || (n > 14709 && n < 14963) || n = 14964 || (n > 14966 && n < 15001))
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
.elseif(n = 10080)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #39
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 10082 || n = 10339 || n = 10596 || n = 10853 || n = 11110 || n = 11367 || n = 11624 || n = 11881 || n = 12138 || n = 12395 || n = 12652 || n = 12909 || n = 13166 || n = 13423 || n = 13680 || n = 13937 || n = 14194 || n = 14451 || n = 14708 || n = 14965)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10083 || n = 10340 || n = 10597 || n = 10854 || n = 11111 || n = 11368 || n = 11625 || n = 11882 || n = 12139 || n = 12396 || n = 12653 || n = 12910 || n = 13167 || n = 13424 || n = 13681 || n = 13938 || n = 14195 || n = 14452 || n = 14709 || n = 14966)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 10337)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #40
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 10594)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #41
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 10851)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #42
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11108)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #43
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11365)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #44
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11622)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #45
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 11879)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #46
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12136)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #47
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12393)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #48
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12650)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #49
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 12907)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #50
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13164)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #51
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13421)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #52
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13678)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #53
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 13935)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #54
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14192)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #55
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14449)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #56
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14706)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #57
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 14963)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #58
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15220) || n = 15221 || (n > 15223 && n < 15477) || n = 15478 || (n > 15480 && n < 15734) || n = 15735 || (n > 15737 && n < 15991) || n = 15992 || (n > 15994 && n < 16248) || n = 16249 || (n > 16251 && n < 16505) || n = 16506 || (n > 16508 && n < 16762) || n = 16763 || (n > 16765 && n < 17019) || n = 17020 || (n > 17022 && n < 17276) || n = 17277 || (n > 17279 && n < 17533) || n = 17534 || (n > 17536 && n < 17790) || n = 17791 || (n > 17793 && n < 18047) || n = 18048 || (n > 18050 && n < 18304) || n = 18305 || (n > 18307 && n < 18561) || n = 18562 || (n > 18564 && n < 18818) || n = 18819 || (n > 18821 && n < 19075) || n = 19076 || (n > 19078 && n < 19332) || n = 19333 || (n > 19335 && n < 19589) || n = 19590 || (n > 19592 && n < 19846) || n = 19847 || (n > 19849 && n < 20001))
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
.elseif(n = 15220)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #59
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 15222 || n = 15479 || n = 15736 || n = 15993 || n = 16250 || n = 16507 || n = 16764 || n = 17021 || n = 17278 || n = 17535 || n = 17792 || n = 18049 || n = 18306 || n = 18563 || n = 18820 || n = 19077 || n = 19334 || n = 19591 || n = 19848)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15223 || n = 15480 || n = 15737 || n = 15994 || n = 16251 || n = 16508 || n = 16765 || n = 17022 || n = 17279 || n = 17536 || n = 17793 || n = 18050 || n = 18307 || n = 18564 || n = 18821 || n = 19078 || n = 19335 || n = 19592 || n = 19849)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 15477)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #60
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 15734)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #61
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 15991)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #62
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 16248)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #63
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 16505)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #64
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 16762)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #65
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17019)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #66
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17276)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #67
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17533)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #68
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 17790)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #69
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18047)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #70
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18304)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #71
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18561)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #72
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 18818)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #73
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19075)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #74
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19332)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #75
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19589)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #76
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 19846)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #77
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.endif
.endif

.endif
	rest
.endmacro
