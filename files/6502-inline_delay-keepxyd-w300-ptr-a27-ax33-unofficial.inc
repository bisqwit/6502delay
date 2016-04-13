; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, Z+N, C, and V
; Will preserve X, Y, S, D, and I (interrupt-disable flag)
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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 24
; - 7 bytes or less when n <= 291
; - 8 bytes or less when n <= 313
; - 9 bytes or less when n <= 1302
; -10 bytes or less when n <= 1879
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.1, max=5
; - When 21 <= n <= 30: min=4, avg=5.4, max=7
; - When 31 <= n <= 50: min=4, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.3, max=8
; - When 301 <= n <= 500: min=6, avg=7.8, max=9
; - When 501 <= n <= 750: min=6, avg=7.8, max=9
; - When 751 <= n <= 1000: min=7, avg=7.8, max=9
; - When 1001 <= n <= 2000: min=6, avg=8.7, max=11
; - When 2001 <= n <= 3000: min=8, avg=9.9, max=11
; - When 3001 <= n <= 5000: min=9, avg=10.7, max=11
; - When 5001 <= n <= 7500: min=9, avg=11.0, max=11
; - When 7501 <= n <= 10000: min=9, avg=11.0, max=11
; - When 10001 <= n <= 20000: min=9, avg=11.0, max=11
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
	cmp $C5
.elseif((n > 3 && n < 7) || (n > 15 && n < 19))
	; 2..5 bytes
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
.elseif(n = 23)
	; 6 bytes
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
.elseif(n = 26)
	; 5 bytes
	sec
	delay_n (n-2)
.elseif(n = 27)
	; 6 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 28)
	; 6 bytes
	clc
	clc
	delay_n (n-4)
.elseif((n > 28 && n < 39) || (n > 39 && n < 157))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 39)
	; 4 bytes
	lda #$4A ;hides 'lsr a'
	Jne *-1
.endif
.else
; 157 to 312
.if((n > 156 && n < 285))
	; 5 bytes
	lda #n-29
	jsr delay_a_27_clocks
.elseif(n = 285 || n = 286 || n = 292 || n = 302 || n = 307 || n = 312)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 287 || (n > 288 && n < 292))
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 288)
	; 6 bytes
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
.elseif(n = 293 || n = 303 || n = 308)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 294 || n = 296)
	; 8 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 297)
	; 8 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 298)
	; 6 bytes
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
.elseif(n = 299)
	; 8 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 301)
	; 8 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 304)
	; 7 bytes
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	nop
	Jne *-3
.elseif(n = 305 || n = 310)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 309)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
.elseif(n = 311)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 318 || n = 323 || n = 338 || n = 353 || n = 378)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 314 || n = 317 || n = 319 || n = 322 || n = 324 || n = 332 || n = 336 || n = 337 || n = 342 || n = 344 || n = 347 || n = 349 || n = 352 || n = 357 || n = 359 || n = 367 || n = 371 || n = 372 || n = 377 || n = 379 || n = 382 || n = 384 || n = 387 || n = 389)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316 || n = 321 || n = 326 || n = 351 || n = 361 || n = 366 || n = 386)
	; 8..9 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 0) && ((n > 324 && n < 341) || n = 355 || n = 360 || (n > 369 && n < 381) || n = 390))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 327 || n = 334 || n = 341 || n = 376)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 328 || n = 333 || n = 343 || n = 348 || n = 358 || n = 363 || n = 368 || n = 373 || n = 388)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 329 || n = 364)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 331)
	; 9 bytes
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
.elseif(n = 339)
	; 7 bytes
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
.elseif(n = 346)
	; 7 bytes
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
.elseif(n = 354)
	; 8 bytes
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
.elseif(n = 356)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 362 || n = 369)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 374)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 426 || n = 431 || n = 436 || n = 456 || n = 466)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 392 || n = 394 || n = 406 || n = 412 || n = 417 || n = 422 || n = 424 || n = 427 || n = 429 || n = 437 || n = 442 || n = 447 || n = 452 || n = 454 || n = 457 || n = 462 || n = 464)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 392 && n < 419) || (n > 432 && n < 449) || n = 458 || n = 468))
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif((n .mod 5 = 0) && (n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 397 || n = 411 || n = 467)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 399)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 400 || n = 410 || n = 415 || n = 425 || n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 401)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 404 || n = 432 || n = 439 || n = 446)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 407)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif(n = 409)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 414)
	; 7 bytes
	sec
	lda #58 ;hides 'nop'
	sbc #1
	Jpl *-3
.elseif(n = 416)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
.elseif(n = 419)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 421)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	tax
.elseif(n = 423 || n = 428 || n = 453 || n = 463)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 434)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 441)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
.elseif(n = 444 || n = 451)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 449)
	; 8 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	delay_n 9
	Jne *-5
.elseif(n = 459)
	; 8 bytes
	tya
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	tay
.elseif(n = 461)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jcs *-5
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 472 || n = 477 || n = 482 || n = 487 || n = 492 || n = 494 || n = 497 || n = 499 || n = 511 || n = 512 || n = 517 || n = 522 || n = 524 || n = 527 || n = 529 || n = 532 || n = 539 || n = 542)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 473 || n = 483 || n = 503 || n = 508 || n = 523 || n = 538 || n = 543)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 474 || n = 502 || n = 516 || n = 544)
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
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 478 || n = 488 || n = 493 || n = 498 || n = 513 || n = 518 || n = 528 || n = 533)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 479 || n = 486 || n = 514 || n = 521)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 481 || n = 509 || n = 537)
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
.elseif(n = 491)
	; 7 bytes
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jpl *-3
.elseif(n = 496 || n = 506 || n = 526)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 501)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 504)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 507)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
.elseif(n = 519)
	; 8 bytes
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
.elseif(n = 531)
	; 9 bytes
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
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
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 541)
	; 9 bytes
	tya
	ldy #(n-13)/8
	cmp $C5
	dey
	Jpl *-3
	tay
.elseif(n = 546)
	; 8 bytes
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jne *-4
.endif
.else
; 547 to 625
.if(n = 547 || n = 552 || n = 557 || n = 559 || n = 562 || n = 566 || n = 567 || n = 569 || n = 574 || n = 577 || n = 582 || n = 587 || n = 589 || n = 592 || n = 594 || n = 597 || n = 602 || n = 604 || n = 612 || n = 616 || n = 617 || n = 622 || n = 624)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && (n = 548 || n = 553 || n = 568 || (n > 582 && n < 594) || (n > 602 && n < 624)))
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 549 || n = 584 || n = 619)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
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
.elseif(n = 554)
	; 8 bytes
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jpl *-4
.elseif(n = 556)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
.elseif(n = 558 || n = 563 || n = 573 || n = 578 || n = 598)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
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
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
.elseif(n = 572 || n = 579)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 576 || n = 596 || n = 601)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 581 || n = 599)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 591)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 606)
	; 8 bytes
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
.elseif(n = 609)
	; 8 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 611)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	nop $04
	Jmi *-3
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
.if(n = 626 || n = 654 || n = 661)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 627 || n = 629 || n = 632 || n = 634 || n = 637 || n = 639 || n = 647 || n = 652 || n = 657 || n = 659 || n = 662 || n = 664 || n = 667 || n = 669 || n = 672 || n = 674 || n = 679 || n = 682 || n = 686 || n = 687 || n = 692 || n = 697 || n = 699 || n = 702)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 633 || n = 638 || n = 648)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 630 || n = 640 || n = 645 || n = 650)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 631)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 701)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 636 || n = 646 || n = 666 || n = 676 || n = 681)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 641)
	; 8 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
.elseif(n = 642 || n = 656 || n = 677 || n = 691)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && (n = 643 || (n > 652 && n < 684) || (n > 692 && n < 704)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 644 || n = 671)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 649 || n = 684)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 651)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 689 || n = 696)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 694)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	tay
.elseif(n = 701)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.endif
.else
; 704 to 781
.if(n = 704 || n = 707 || n = 709 || n = 721 || n = 722 || n = 727 || n = 732 || n = 737 || n = 739 || n = 742 || n = 744 || n = 756 || n = 757 || n = 762 || n = 764 || n = 767 || n = 769 || n = 772 || n = 774 || n = 777)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 706 || n = 746 || n = 751 || n = 776)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 5 = 3) && ((n > 707 && n < 779)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 711)
	; 8 bytes
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
.elseif(n = 712 || n = 719 || n = 747 || n = 754 || n = 761)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 714 || n = 749)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 716 || n = 734)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
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
.elseif(n = 724)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 726)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 729)
	; 7 bytes
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
.elseif(n = 731)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
.elseif(n = 736)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 759 || n = 766)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 771)
	; 9 bytes
	sec
	lda #(n-11)/8
	bit $24
	sbc #1
	Jpl *-4
.elseif(n = 779)
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
; 782 to 937
.if(n = 782 || n = 789 || n = 824 || n = 852 || n = 887 || n = 922 || n = 929 || n = 936)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 782 && n < 934)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
.elseif(n = 787 || n = 791 || n = 792 || n = 797 || n = 799 || n = 802 || n = 807 || n = 809 || n = 812 || n = 814 || n = 822 || n = 826 || n = 827 || n = 832 || n = 834 || n = 837 || n = 839 || n = 842 || n = 844 || n = 847 || n = 849 || n = 861 || n = 867 || n = 872 || n = 874 || n = 877 || n = 879 || n = 882 || n = 884 || n = 892 || n = 897 || n = 902 || n = 904 || n = 907 || n = 909 || n = 912 || n = 914 || n = 917 || n = 927 || n = 931 || n = 932 || n = 937)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif(n = 794 || n = 829 || n = 871 || n = 906)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 796 || n = 817 || n = 831 || n = 859 || n = 866 || n = 894 || n = 901)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.elseif(n = 801 || n = 836 || n = 899 || n = 934)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 804)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
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
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
.elseif(n = 816 || n = 841 || n = 846 || n = 856 || n = 876 || n = 886 || n = 911 || n = 916 || n = 926)
	; 9 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 821)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 857)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 862)
	; 7 bytes
	sec
	lda #250 ;hides 'nop'
	sbc #1
	Jmi *-3
.elseif(n = 864)
	; 7 bytes
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jpl *-2
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
.elseif(n = 891)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 919)
	; 8 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jpl *-4
.elseif(n = 921)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
.endif
.endif
.else
; 938 to 1250
.if((n .mod 5 = 3) && ((n > 937 && n < 1014) || (n > 1022 && n < 1249)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 939)
	; 7 bytes
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1251)))
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
.elseif(n = 942 || n = 944 || n = 947 || n = 949 || n = 952 || n = 954 || n = 967 || n = 972 || n = 974 || n = 977 || n = 979 || n = 982 || n = 987 || n = 989 || n = 997 || n = 1001 || n = 1002 || n = 1007 || n = 1009 || n = 1012 || n = 1014 || n = 1017 || n = 1019 || n = 1022 || n = 1032 || n = 1036 || n = 1037 || n = 1042 || n = 1044 || n = 1047 || n = 1049 || n = 1052 || n = 1057 || n = 1059 || n = 1067 || n = 1071 || n = 1072 || n = 1077 || n = 1079 || n = 1082 || n = 1084 || n = 1087 || n = 1089 || n = 1092 || n = 1094 || n = 1102 || n = 1106 || n = 1107 || n = 1112 || n = 1114 || n = 1117 || n = 1119 || n = 1122 || n = 1124 || n = 1127 || n = 1129 || n = 1137 || n = 1141 || n = 1142 || n = 1147 || n = 1149 || n = 1152 || n = 1154 || n = 1157 || n = 1159 || n = 1162 || n = 1164 || n = 1172 || n = 1176 || n = 1177 || n = 1182 || n = 1184 || n = 1187 || n = 1189 || n = 1192 || n = 1194 || n = 1197 || n = 1199 || n = 1207 || n = 1212 || n = 1217 || n = 1219 || n = 1222 || n = 1224 || n = 1227 || n = 1229 || n = 1232 || n = 1234 || n = 1242 || n = 1246 || n = 1247)
	; 8..9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 1) && ((n > 945 && n < 962) || n = 991 || n = 996 || (n > 1015 && n < 1027) || n = 1066 || n = 1086 || n = 1096 || n = 1126 || n = 1136 || n = 1156 || n = 1161 || (n > 1190 && n < 1207) || n = 1226 || n = 1231))
	; 8..9 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 7 = 5) && ((n > 956 && n < 972) || (n > 991 && n < 1007) || (n > 1026 && n < 1042) || (n > 1061 && n < 1077) || (n > 1096 && n < 1112) || (n > 1131 && n < 1147) || (n > 1166 && n < 1182) || (n > 1201 && n < 1217) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 959 || n = 994 || n = 1029 || n = 1064 || n = 1134 || n = 1169 || n = 1204 || n = 1239)
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
.elseif(n = 969 || n = 976 || n = 1004 || n = 1011 || n = 1039 || n = 1046 || n = 1081 || n = 1109 || n = 1116 || n = 1144 || n = 1151 || n = 1179 || n = 1186 || n = 1214 || n = 1221 || n = 1249)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 981)
	; 9 bytes
	tya
	ldy #(n-13)/8
	cmp $C5
	dey
	Jpl *-3
	tay
.elseif(n = 984)
	; 8 bytes
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 986 || n = 1211)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1031 || n = 1121 || n = 1166)
	; 9 bytes
	tya
	ldy #(n-14)/9
	delay_n 4
	dey
	Jpl *-3
	tay
.elseif(n = 1051 || n = 1091 || n = 1131 || n = 1171)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1054)
	; 8 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	sbc #1
	Jmi *-4
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
.elseif(n = 1099)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	tay
.elseif(n = 1101)
	; 8 bytes
	tya
	ldy #(n-3)/9 ;hides 'nop'
	nop
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
.endif
.endif
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 7 = 5) && (n = 1251 || (n > 1271 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1252 || n = 1257 || n = 1259 || n = 1262 || n = 1264 || n = 1267 || n = 1269 || n = 1277 || n = 1281 || n = 1282 || n = 1287 || n = 1295 || n = 1302 || n = 1304 || n = 1309 || n = 1316 || n = 1323)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 1252 && n < 1279)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
.elseif((n .mod 7 = 3) && (n = 1256 || n = 1284 || (n > 1297 && n < 1327)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1261)
	; 8 bytes
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 1266 || n = 1271 || n = 1276 || n = 1288 || n = 1294 || n = 1303 || n = 1322 || n = 1324)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1274)
	; 9 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1283)
	; 7 bytes
	sec
	lda #0
	sbc #1
	Jne *-2
.elseif(n = 1285)
	; 7 bytes
	tya
	ldy #0
	dey
	Jne *-1
	tay
.elseif(n = 1289)
	; 7 bytes
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
.elseif(n = 1290 || n = 1292 || n = 1296)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1291)
	; 7 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
.elseif(n = 1297)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1299 || n = 1315)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1301 || n = 1310)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1306 || n = 1320)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1308)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1311)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
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
.elseif(n = 1327)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1338 || n = 1350 || n = 1352 || n = 1366)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1330 || n = 1332 || n = 1344 || n = 1348 || n = 1351 || n = 1358 || n = 1360 || n = 1365 || n = 1367)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331)
	; 9 bytes
	txa
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1334)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jpl *-5
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
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
.elseif((n .mod 9 = 5) && ((n > 1336 && n < 1365)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1339)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1341 || n = 1357)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1343)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	tay
.elseif(n = 1345)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
.elseif(n = 1353 || n = 1362)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1359)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
.endif
.else
; 1368 to 1406
.if(n = 1368 || n = 1375 || n = 1382 || n = 1396)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1369)
	; 10 bytes
	tya
	ldy #123
	delay_n 6
	dey
	Jpl *-4
	tay
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1406)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1371 || n = 1395)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1372 || n = 1374 || n = 1379 || n = 1386 || n = 1388 || n = 1393)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373 || n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1381)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1383)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1385 || n = 1397)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1387)
	; 7 bytes
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
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
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
.elseif(n = 1392)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1394)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1399)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
.elseif(n = 1401 || n = 1406)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1402)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1414 || n = 1420 || n = 1421 || n = 1423 || n = 1428 || n = 1435 || n = 1439 || n = 1442 || n = 1449 || n = 1456 || n = 1458 || n = 1470 || n = 1474 || n = 1477 || n = 1484)
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
.elseif((n .mod 9 = 5) && ((n > 1408 && n < 1437) || (n > 1462 && n < 1482)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 3) && (n = 1410 || n = 1417 || (n > 1430 && n < 1481)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1411 || n = 1443 || n = 1451 || n = 1467 || n = 1483)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
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
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1415 || n = 1422 || n = 1441 || n = 1448 || n = 1450 || n = 1464 || n = 1476 || n = 1478)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1416 || n = 1425 || n = 1434 || n = 1479)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.elseif(n = 1455)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
.elseif(n = 1457)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1462)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
.elseif(n = 1465)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1486 || n = 1491 || n = 1498 || n = 1502 || n = 1505 || n = 1512 || n = 1514 || n = 1518 || n = 1519 || n = 1521)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1523)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1488)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1492 || n = 1504 || n = 1513)
	; 10 bytes
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
.elseif(n = 1495)
	; 8 bytes
	clc
	lda #90 ;hides 'nop'
	nop
	adc #1
	Jne *-4
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
	; 8 bytes
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
.elseif(n = 1507 || n = 1523)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1511)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	iny
	Jne *-5
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
.elseif(n = 1525)
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
.elseif(n = 1527)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif(n = 1528)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1529)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1530 || n = 1532)
	; 9 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1533 || n = 1537 || n = 1540 || n = 1546 || n = 1547 || n = 1554 || n = 1556 || n = 1561)
	; 9..10 bytes
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
.elseif((n .mod 7 = 3) && ((n > 1535 && n < 1558)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1539 || n = 1555)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1541)
	; 9 bytes
	tya
	ldy #(n-17)/12
	delay_n 7
	dey
	Jpl *-3
	tay
.elseif(n = 1542)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1548)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1549)
	; 8 bytes
	clc
	lda #84 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
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
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1579 || n = 1595 || n = 1619 || n = 1635)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1607) || n = 1620 || n = 1634))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1565 || n = 1577 || n = 1637)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1637)))
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
.elseif(n = 1568 || n = 1574 || n = 1575 || n = 1582 || n = 1584 || n = 1590 || n = 1596 || n = 1603 || n = 1609 || n = 1610 || n = 1612 || n = 1617 || n = 1624 || n = 1626 || n = 1631 || n = 1638 || n = 1640)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569 || n = 1623)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1570)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
.elseif(n = 1572)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1576 || n = 1583 || n = 1602 || n = 1604 || n = 1628 || n = 1630)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1581 || n = 1597 || n = 1605 || n = 1621)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1586)
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
.elseif((n .mod 9 = 5) && ((n > 1588 && n < 1617)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1591)
	; 10 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1593)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 1611)
	; 7 bytes
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
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
.elseif(n = 1618 || n = 1632)
	; 9 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1625)
	; 7 bytes
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
.elseif(n = 1627)
	; 7 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
.elseif(n = 1633)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	sta (@ptrtemp),y
	tay
.elseif(n = 1639)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
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
.elseif(n = 1642 || n = 1644 || n = 1658 || n = 1674 || n = 1681 || n = 1700 || n = 1702 || n = 1716)
	; 9..10 bytes
	delay_n (n-3)
	lda $A5
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1645 || n = 1666 || n = 1672 || n = 1673 || n = 1680 || n = 1682 || n = 1687 || n = 1689 || n = 1694 || n = 1701 || n = 1703 || n = 1708 || n = 1710)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1654)
	; 9..10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1647)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
.elseif((n .mod 7 = 3) && ((n > 1647 && n < 1719)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1649 || n = 1709)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1651 || n = 1667 || n = 1675 || n = 1691 || n = 1707)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif((n .mod 9 = 5) && ((n > 1651 && n < 1689) || n = 1715))
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1653 || n = 1677 || n = 1693 || n = 1717)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1656 || n = 1684 || n = 1698 || n = 1712)
	; 10 bytes
	tya
	ldy #(n-11)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
.elseif(n = 1663)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	nop $04
	iny
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
.elseif(n = 1668 || n = 1695)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1686)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
.elseif(n = 1696)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
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
.if(n = 1719)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 1722 || n = 1726 || n = 1729 || n = 1735 || n = 1736 || n = 1738 || n = 1743 || n = 1750 || n = 1752 || n = 1757)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1723 || n = 1731 || n = 1747)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1724 || n = 1733 || n = 1742)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1747)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1728 || n = 1730 || n = 1737)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1740)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1744)
	; 9 bytes
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
.elseif(n = 1749)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1751)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif(n = 1753)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1754)
	; 9 bytes
	delay_n (n-3)
	nop $04
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
.if(n = 1758 || n = 1766 || n = 1771 || n = 1792)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1759)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C3A0'
	ldy #195
	dey
	Jne *-4
	tay
.elseif((n .mod 7 = 3) && ((n > 1759 && n < 1789)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1763 || n = 1779)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1764)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1765 || n = 1773 || n = 1789)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1768)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1770)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1772)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
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
.elseif(n = 1778 || n = 1787 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
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
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	dey
	Jne *-4
	tay
.elseif(n = 1791)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 8 bytes
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1799 || n = 1807 || n = 1816 || n = 1822 || n = 1824 || n = 1825 || n = 1834)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1833)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1801 || n = 1802 || n = 1810)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
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
	tya
	cmp <$CD ;hides 'cmp $A4A0'
	ldy #164
	nop
	dey
	Jne *-5
	tay
.elseif((n .mod 8 = 3) && ((n > 1810 && n < 1836)))
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1812)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1813 || n = 1821)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1815)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1817 || n = 1829)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1818)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 1820)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1826)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 1828)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
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
.endif
.else
; 1836 to 1875
.if(n = 1836)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1837)
	; 8 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 1838 || n = 1847 || n = 1852 || n = 1855 || n = 1860 || n = 1870 || n = 1871)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	tay
.elseif(n = 1844)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 1845)
	; 9 bytes
	txa
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 1851 || n = 1867 || n = 1875)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1853 || n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1854)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 1856)
	; 10 bytes
	sec
	lda #(n-20)/17
	jsr @nearby_rts
	sbc #1
	Jpl *-5
.elseif(n = 1857 || n = 1866)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1858)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
.elseif(n = 1861 || n = 1869)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1862)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 1863)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1872)
	; 10 bytes
	clc
	lda #100 ;hides 'nop zp'
	nop $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
.elseif(n = 1873)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
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
.if((n >= 1876) && (n < 1915))
.if((n >= 1876) && (n < 1895))
; 1876 to 1894
.if(n = 1876)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
.elseif(n = 1877 || n = 1886)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 1878 || n = 1888)
	; 10 bytes
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
.elseif(n = 1880)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1881)
	; 9 bytes
	clc
	lda #68 ;hides 'nop zp'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
.elseif(n = 1882)
	; 9 bytes
	clc
	lda #68 ;hides 'nop zp'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 1883 || n = 1891)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1884)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1885 || n = 1893)
	; 9 bytes
	tya
	ldy #(n-5)/8
	cmp $C5
	dey
	Jne *-3
	tay
.elseif(n = 1887)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1889)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1890)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 1892)
	; 9 bytes
	clc
	lda #84 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
.elseif(n = 1894)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jne *-4
	tay
.endif
.else
; 1895 to 1914
.if(n = 1895 || n = 1904 || n = 1913)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1898 || n = 1900)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1899 || n = 1907)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1901)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
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
.elseif(n = 1906)
	; 9 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 1909)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
.elseif(n = 1910)
	; 9 bytes
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	adc #1
	Jne *-5
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
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1923 || n = 1939 || n = 1947)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1916 || n = 1918 || n = 1927 || n = 1936)
	; 10 bytes
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
.elseif(n = 1924 || n = 1942 || n = 1943 || n = 1951)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1928 || n = 1950)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 1932)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 1934 || n = 1946)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1935)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1938)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1944)
	; 10 bytes
	tya
	ldy #194 ;hides 'nop #imm'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	tay
.elseif(n = 1945)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
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
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if(n = 1954)
	; 9 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1955 || n = 1971 || n = 1979 || n = 1987)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1956 || n = 1983 || n = 1992)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 1957 || n = 1981 || n = 1989)
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
.elseif(n = 1959)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1960 || n = 1968 || n = 1969 || n = 1978 || n = 1986 || n = 1991)
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
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
.elseif(n = 1964)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
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
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
.elseif(n = 1970)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
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
.elseif(n = 1977)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 1980 || n = 1982 || n = 1988 || n = 1990)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 1984)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
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
.elseif(n = 1995 || n = 2011 || n = 2019)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 1996 || n = 2018)
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
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 1999 || n = 2004 || n = 2014 || n = 2022 || n = 2023)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2000)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2001 || n = 2010 || n = 2028)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2007 || n = 2031)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2008 || n = 2024)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2015)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2016)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2017)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2020)
	; 9 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2025)
	; 9 bytes
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2026)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2027)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if((n .mod 9 = 7) && ((n > 2031 && n < 2069)))
	; 10 bytes
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
	; 10 bytes
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
.elseif(n = 2035 || n = 2043)
	; 9 bytes
	sec
	lda #(n-3)/8
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 2036)
	; 9 bytes
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
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
.elseif(n = 2040 || n = 2062)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2042 || n = 2044 || n = 2052 || n = 2054)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2046 || n = 2055 || n = 2064)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 2047)
	; 9 bytes
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
.elseif(n = 2049 || n = 2063)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2051)
	; 9 bytes
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
.elseif(n = 2053)
	; 9 bytes
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
	tay
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
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2061)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2065)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	tay
.elseif(n = 2067)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2070)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	nop
	nop $04
	adc #1
	Jne *-6
.endif
.else
; 2071 to 2109
.if(n = 2071)
	; 8 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
.elseif(n = 2072)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	nop
	dey
	Jne *-5
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
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
.elseif(n = 2075 || n = 2084 || n = 2093)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2076 || n = 2077 || n = 2086 || n = 2104)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2078 || n = 2080 || n = 2096 || n = 2098)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2079 || n = 2103)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	tay
.elseif(n = 2085 || n = 2094)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2087 || n = 2090 || n = 2108)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	; 8 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
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
.elseif(n = 2095 || n = 2106)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2097)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
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
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 2101)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	tay
.elseif(n = 2102)
	; 9 bytes
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2107)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 2112)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2113 || n = 2121 || n = 2122 || n = 2130 || n = 2131 || n = 2140 || n = 2143)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2114 || n = 2116 || n = 2132)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2115 || n = 2139)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
.elseif(n = 2123)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2124)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2125)
	; 8 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 2126 || n = 2148)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2135)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
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
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
.elseif(n = 2142)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	sta (@ptrtemp),y
	tay
.elseif(n = 2144)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2145)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.if(n = 2149 || n = 2157 || n = 2158 || n = 2166 || n = 2167 || n = 2176 || n = 2184 || n = 2185)
	; 10 bytes
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
.elseif(n = 2151 || n = 2175 || n = 2187)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif((n .mod 9 = 3) && ((n > 2153 && n < 2182)))
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2159 || n = 2170)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2160)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2162)
	; 10 bytes
	sec
	lda #(n-20)/17
	jsr @nearby_rts
	sbc #1
	Jpl *-5
.elseif(n = 2164)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	tay
.elseif(n = 2168 || n = 2178 || n = 2180 || n = 2186)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2169)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2171)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2173)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2179)
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 2182)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	tay
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
.if((n >= 2188) && (n < 2207))
; 2188 to 2206
.if(n = 2188)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2189)
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	tay
.elseif(n = 2192 || n = 2201)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2193)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2194)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2195)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 11
	Jne *-5
	tay
.elseif(n = 2196 || n = 2206)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2197)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
.elseif(n = 2198)
	; 9 bytes
	clc
	lda #12 ;hides 'nop abs'
	nop $0
	adc #1
	Jne *-5
.elseif(n = 2199)
	; 8 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2200)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
.elseif(n = 2202)
	; 9 bytes
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
.elseif(n = 2203)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2204)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2205)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.endif
.else
; 2207 to 2226
.if(n = 2207)
	; 10 bytes
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
.elseif(n = 2208)
	; 8 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 2209)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
	tay
.elseif(n = 2210 || n = 2219)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2211 || n = 2223)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2212 || n = 2215 || n = 2220 || n = 2221)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2213 || n = 2225)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
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
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2217 || n = 2226)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
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
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.endif
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	tay
.elseif(n = 2228 || n = 2237)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2229)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2230 || n = 2239)
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
.elseif(n = 2233 || n = 2243)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2234)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2235)
	; 9 bytes
	sec
	lda #(n-3)/9
	delay_n 4
	sbc #1
	Jne *-4
.elseif(n = 2236)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
.elseif(n = 2238)
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
	clc
	lda #116 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
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
.endif
.else
; 2246 to 2265
.if(n = 2246 || n = 2255 || n = 2264)
	; 9 bytes
	tya
	ldy #(n-5)/9
	delay_n 4
	dey
	Jne *-3
	tay
.elseif(n = 2247 || n = 2259)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2248 || n = 2256 || n = 2257)
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
.elseif(n = 2251)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
.elseif(n = 2252)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
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
	cmp <$CD ;hides 'cmp $06A0'
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
.elseif(n = 2260)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
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
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if(n = 2266 || n = 2275 || n = 2276 || n = 2286 || n = 2292 || n = 2296 || n = 2302)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267 || n = 2268 || (n > 2276 && n < 2280) || n = 2288)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2269)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2274)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 2283)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
.elseif(n = 2284)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
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
.elseif(n = 2287)
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
.elseif(n = 2294)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2295)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2301)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	nop $04
	adc #1
	Jne *-5
.elseif(n = 2303)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	nop $04
	iny
	Jne *-4
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
.elseif((n .mod 2 = 0) && (n = 2306 || n = 2312 || (n > 2315 && n < 2323)))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 2313)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif(n = 2319)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	clc
	lda #90 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
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
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 2328)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 2331)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2336)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2338)
	; 10 bytes
	sec
	lda #250 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
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
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
.elseif(n = 2341)
	; 9 bytes
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2342)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
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
	; 10 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
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
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
.elseif(n = 2347 || n = 2361)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2348 || n = 2370)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2349)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2350 || n = 2352 || n = 2360 || n = 2362 || n = 2364 || n = 2372 || n = 2374 || n = 2376 || n = 2382)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2351)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2353 || n = 2363 || n = 2373)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2354)
	; 10 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2355 || n = 2367 || n = 2379)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2359)
	; 9 bytes
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2365 || n = 2375)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2366)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
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
.elseif(n = 2377)
	; 10 bytes
	clc
	lda #58 ;hides 'nop'
	nop
	bit $24
	adc #1
	Jne *-6
.elseif(n = 2378)
	; 10 bytes
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
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
.if((n .mod 12 = 7) && ((n > 2382 && n < 2420)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2384 || (n > 2385 && n < 2390) || n = 2394 || n = 2396 || n = 2398 || n = 2404 || n = 2406 || (n > 2407 && n < 2411) || n = 2420)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2385 || n = 2413)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2390 || n = 2412)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2391 || n = 2403 || n = 2415)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2392 || n = 2414)
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
.elseif(n = 2397 || n = 2411)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2399)
	; 9 bytes
	sec
	lda #(n-1)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2400)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2401)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
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
.elseif(n = 2416)
	; 10 bytes
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
.elseif(n = 2418)
	; 10 bytes
	tya
	ldy #(n-24)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
.elseif(n = 2421)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
.elseif(n = 2423 || n = 2433)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2424)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	bit $24
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
.elseif(n = 2426 || n = 2428 || n = 2430 || n = 2432 || n = 2438 || n = 2440 || n = 2444 || n = 2452 || n = 2459 || n = 2460)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2427 || n = 2451)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2429 || n = 2441 || n = 2453)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2431 || n = 2443)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2434 || n = 2456)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2442)
	; 10 bytes
	delay_n (n-3)
	lda $A5
.elseif(n = 2446)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2448)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
.elseif(n = 2449)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	nop $04
	adc #1
	Jne *-6
.elseif(n = 2450)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	tay
.elseif(n = 2454)
	; 10 bytes
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
.elseif(n = 2455)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
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
.endif
.else
; 2461 to 2500
.if(n = 2461 || n = 2462 || n = 2464 || n = 2466 || n = 2471 || n = 2472 || n = 2474 || n = 2476 || n = 2482 || n = 2484 || n = 2486 || n = 2496 || n = 2498)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 12 = 3) && ((n > 2462 && n < 2500)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2465 || n = 2477 || n = 2489)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2467 || n = 2479)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2468)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
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
.elseif(n = 2470)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2473 || n = 2493)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
.elseif(n = 2478 || n = 2500)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2481)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2483 || n = 2497)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2485 || n = 2495)
	; 10 bytes
	tya
	ldy #(n-5)/10
	nop
	cmp $C5
	dey
	Jne *-4
	tay
.elseif(n = 2488)
	; 10 bytes
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 2490)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2492)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2494)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2502 || n = 2524)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2503)
	; 9 bytes
	tya
	ldy #250 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	tay
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
.elseif((n > 2505 && n < 2509) || n = 2510 || n = 2514 || (n > 2515 && n < 2519) || n = 2520 || n = 2526 || n = 2528 || n = 2529)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2509)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2511 || n = 2523 || n = 2535)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2512)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
.elseif(n = 2515 || n = 2527)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2519 || n = 2536)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2522)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2530)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2531)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
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
	delay_n 4
	iny
	Jne *-4
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
.elseif(n = 2539)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.else
; 2540 to 2578
.if((n > 2539 && n < 2543) || n = 2548 || n = 2550 || n = 2552 || n = 2554 || n = 2560 || n = 2562 || n = 2564 || n = 2569 || n = 2574 || n = 2576)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2543)
	; 10 bytes
	sec
	lda #(n-3)/10
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif(n = 2547 || n = 2559 || n = 2571)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2549 || n = 2561 || n = 2573)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2551 || n = 2563)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2553)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
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
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2567)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2570)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2572)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2575)
	; 9 bytes
	sec
	lda #(n-1)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
.elseif(n = 2577)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
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
.if(n = 2579)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 2580 || n = 2582 || n = 2591 || n = 2592 || n = 2594 || n = 2602 || (n > 2612 && n < 2616))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2581)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2583 || n = 2595 || n = 2607)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2584)
	; 10 bytes
	sec
	lda #250 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jmi *-6
.elseif(n = 2585 || n = 2597 || n = 2609)
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
.elseif(n = 2587 || n = 2611)
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2590 || n = 2601 || n = 2612)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2593)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2596)
	; 9 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
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
	delay_n 4
	iny
	Jne *-4
	tay
.elseif(n = 2600)
	; 10 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	delay_n 5
	dey
	Jne *-5
	tay
.elseif(n = 2603)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2604)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 2610)
	; 9 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2616)
	; 9 bytes
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2617)
	; 10 bytes
	sec
	lda #218 ;hides 'nop'
	nop
	bit $24
	sbc #1
	Jne *-6
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2635 || n = 2647)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 3) && ((n > 2618 && n < 2656)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2620 || n = 2624 || n = 2626 || (n > 2627 && n < 2631) || n = 2636 || n = 2639 || n = 2648 || n = 2650)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 2625)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2627 || n = 2646)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2632)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
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
.elseif(n = 2638)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2640)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
.elseif(n = 2641)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 2642)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	tax
.elseif(n = 2644)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 2649)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2652)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 2653)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	delay_n 6
	adc #1
	Jne *-6
.elseif(n = 2654)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2658 || (n > 2659 && n < 2663) || n = 2664 || n = 2670 || n = 2673 || n = 2675 || n = 2680 || n = 2690 || n = 2692 || n = 2694)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2696)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2663 || n = 2677)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2665)
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
.elseif(n = 2667 || n = 2679 || n = 2691)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2668)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 2672)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2674)
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
.elseif(n = 2678 || n = 2689)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2682)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2684)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
.elseif(n = 2685)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	sbc #1
	Jne *-5
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
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
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
.endif
.else
; 2696 to 2734
.if(n = 2696 || n = 2697 || n = 2702 || n = 2704 || n = 2712 || n = 2713 || n = 2718 || n = 2724 || n = 2726 || n = 2730 || n = 2734)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
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
.elseif(n = 2701)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2703 || n = 2715 || n = 2727)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2705 || n = 2717 || n = 2729)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2706 || n = 2723)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 2709 || n = 2720)
	; 10 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2710)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2714)
	; 10 bytes
	sec
	lda #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	sbc #1
	Jne *-6
.elseif(n = 2716)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2719 || n = 2731)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2728)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.elseif(n = 2736 || n = 2737)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2738)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2739 || n = 2751)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2740)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 6
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
	cmp #<$CD ;hides 'cmp $07A0'
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
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
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
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2747)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2748)
	; 10 bytes
	clc
	lda #84 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
.elseif(n = 2750)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 2752)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.endif
.else
; 2754 to 2773
.if(n = 2754)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2755 || n = 2766)
	; 10 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2756)
	; 10 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 2757)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2758)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2759)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 2760)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2761)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2762 || n = 2768 || n = 2769)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2763)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2764)
	; 10 bytes
	tya
	cmp #<$CD ;hides 'cmp $05A0'
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
.elseif(n = 2767)
	; 10 bytes
	delay_n (n-2)
	nop
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
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
.endif
.endif
.else
; 2774 to 2812
.if(n = 2774)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif((n .mod 12 = 3) && ((n > 2774 && n < 2812)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2778 || (n > 2779 && n < 2783) || n = 2784 || n = 2790 || n = 2792 || n = 2794 || n = 2795 || n = 2800 || n = 2802 || n = 2804 || n = 2806 || n = 2807 || n = 2812)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2779)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2783)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
.elseif(n = 2788 || n = 2810)
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
.elseif(n = 2793)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2796)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
; 2813 to 2890
.if((n .mod 12 = 5) && ((n > 2812 && n < 2862) || n = 2885))
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
.elseif((n .mod 12 = 7) && ((n > 2814 && n < 2888)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2818 || n = 2826 || (n > 2827 && n < 2831) || n = 2838 || n = 2840 || n = 2841 || n = 2846 || n = 2848 || n = 2850 || n = 2852 || n = 2854 || (n > 2855 && n < 2859) || n = 2860 || n = 2862 || n = 2864 || n = 2866 || (n > 2867 && n < 2871) || n = 2877 || n = 2879 || n = 2881 || n = 2882 || n = 2884 || n = 2886 || n = 2888 || n = 2890)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2817 || n = 2831)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2819 || n = 2833 || n = 2889)
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
.elseif(n = 2822)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
.elseif((n .mod 12 = 3) && ((n > 2822 && n < 2884)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2824)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 2832)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
.elseif(n = 2834 || n = 2853 || n = 2872)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2836 || n = 2855 || n = 2874)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2842 || n = 2876)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2843)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 2844 || n = 2878)
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
.elseif(n = 2865)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2867)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	tax
.elseif(n = 2873)
	; 9 bytes
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2880)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tay
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2948)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2892 || n = 2894 || n = 2896 || n = 2898 || n = 2900 || n = 2902 || n = 2904 || n = 2906 || n = 2908 || n = 2913 || n = 2914 || n = 2916 || n = 2918 || n = 2920 || n = 2922 || n = 2924 || n = 2926 || n = 2932 || n = 2934 || (n > 2935 && n < 2939) || (n > 2939 && n < 2943) || n = 2952 || n = 2953 || n = 2956 || n = 2958 || n = 2960 || n = 2962 || n = 2964 || n = 2966 || n = 2968)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2893 || n = 2910 || n = 2927 || n = 2961)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif((n .mod 12 = 3) && ((n > 2894 && n < 2968)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif((n .mod 12 = 5) && ((n > 2896 && n < 2958)))
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif((n .mod 12 = 7) && ((n > 2898 && n < 2948)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2901 || n = 2915 || n = 2929)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2903 || n = 2917 || n = 2959)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2905 || n = 2925 || n = 2965)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2912 || n = 2946 || n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2928)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	bit $FF24 ;hides 'bit zp'
	sbc #1
	Jne *-6
.elseif(n = 2930)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	tay
.elseif(n = 2939)
	; 10 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
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
.elseif(n = 2949)
	; 10 bytes
	clc
	lda #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	adc #1
	Jne *-6
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
.elseif(n = 2954)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.endif
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
.elseif(n = 2970 || n = 2972 || (n > 2974 && n < 2978) || n = 2982 || n = 2984 || n = 2989 || n = 2990 || n = 2992 || n = 2994 || n = 2996 || n = 2998 || n = 3000 || n = 3002 || n = 3004 || (n > 3007 && n < 3012) || n = 3016 || n = 3018 || (n > 3019 && n < 3023) || n = 3028 || n = 3030 || (n > 3031 && n < 3039) || n = 3040 || n = 3042 || n = 3044 || n = 3047 || n = 3049 || n = 3050 || n = 3052 || n = 3054 || n = 3056 || (n > 3058 && n < 3062) || n = 3066 || (n > 3071 && n < 3075) || n = 3076 || n = 3078 || (n > 3085 && n < 3097) || n = 3098 || n = 3101 || n = 3104 || (n > 3105 && n < 3111) || n = 3112 || n = 3115 || n = 3117 || n = 3118 || n = 3120 || n = 3122 || n = 3124)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3008) || n = 3031 || n = 3067 || n = 3079))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 5) && ((n > 2972 && n < 3002) || (n > 3042 && n < 3114)))
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
.elseif(n = 2978 || n = 3012 || n = 3046 || n = 3080 || n = 3114)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif((n .mod 12 = 3) && ((n > 2978 && n < 3064)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2980 || n = 2997 || n = 3014 || n = 3082 || n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif((n .mod 14 = 3) && ((n > 2984 && n < 3014) || (n > 3068 && n < 3126)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2986 || n = 3024 || n = 3062 || n = 3081 || n = 3100 || n = 3119)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2988 || n = 3026 || n = 3064 || n = 3102 || n = 3121)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3023 || n = 3103 || n = 3123)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3025 || n = 3105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 3055)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3058)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.elseif(n = 3068)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 3070)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3075)
	; 9 bytes
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 3077)
	; 9 bytes
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3084)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
	tay
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3128 || n = 3129 || n = 3134 || n = 3135 || n = 3137 || n = 3142 || n = 3144 || n = 3146 || n = 3147 || n = 3149 || n = 3151 || n = 3152 || n = 3154 || n = 3156 || n = 3158 || n = 3160 || n = 3161 || n = 3166 || n = 3168 || n = 3170 || n = 3177 || n = 3179 || n = 3180 || n = 3186 || (n > 3188 && n < 3195) || n = 3196 || n = 3198 || n = 3200 || n = 3202)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 14 = 5) && ((n > 3126 && n < 3198)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3130)
	; 9 bytes
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3131 || n = 3148 || n = 3165 || n = 3182 || n = 3199)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3132)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3133 || n = 3150 || n = 3184 || n = 3201)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3136)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3138 || n = 3157 || n = 3176)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 3) && ((n > 3138 && n < 3196)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3140 || n = 3159 || n = 3178)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3143 || n = 3163)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3145 || n = 3185)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3162)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
.elseif(n = 3164)
	; 10 bytes
	tya
	ldy #n/14 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
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
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 3173)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 6
	sbc #1
	Jne *-6
.elseif(n = 3174)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3175)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 6
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
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
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
; 3204 to 3281
.if(n = 3204 || (n > 3205 && n < 3209) || n = 3210 || n = 3212 || n = 3213 || n = 3215 || n = 3217 || n = 3219 || n = 3220 || n = 3222 || n = 3224 || (n > 3225 && n < 3233) || n = 3234 || n = 3236 || n = 3238 || (n > 3239 && n < 3243) || n = 3244 || (n > 3245 && n < 3249) || (n > 3254 && n < 3260) || n = 3261 || n = 3264 || n = 3266 || n = 3268 || n = 3270 || n = 3272 || (n > 3273 && n < 3277) || n = 3278 || n = 3280)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3205 || n = 3245)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3209 || n = 3237 || n = 3265)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3210 && n < 3282)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3214 || n = 3271)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3216 || n = 3233 || n = 3250)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3218 || n = 3235 || n = 3252 || n = 3269)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3221)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
.elseif(n = 3223)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3243 || n = 3263)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3249)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
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
.elseif(n = 3260)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
.elseif(n = 3262)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3277)
	; 9 bytes
	sec
	lda #(n-1)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3279)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.endif
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3287 || n = 3288 || n = 3291 || n = 3294 || n = 3296 || n = 3298 || n = 3300 || n = 3306 || n = 3308 || n = 3310 || (n > 3311 && n < 3318) || n = 3322 || n = 3324 || n = 3326 || n = 3327 || n = 3329 || (n > 3330 && n < 3335) || n = 3336 || (n > 3337 && n < 3343) || n = 3344 || n = 3346 || n = 3348 || n = 3350 || n = 3353 || (n > 3354 && n < 3363) || n = 3364 || n = 3367 || n = 3370 || (n > 3371 && n < 3377) || n = 3378 || (n > 3379 && n < 3383) || n = 3384 || n = 3392 || n = 3394 || (n > 3395 && n < 3403) || n = 3408 || n = 3410 || (n > 3411 && n < 3419) || n = 3426 || (n > 3427 && n < 3433) || n = 3434 || n = 3436)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3283 || n = 3343 || n = 3383)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3284 || n = 3301 || n = 3318 || n = 3352 || n = 3369 || n = 3386 || n = 3403 || n = 3420 || n = 3437)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3285 || n = 3305 || n = 3325 || n = 3345 || n = 3385)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3286 || n = 3303 || n = 3320 || n = 3354 || n = 3371 || n = 3388 || n = 3422)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3289)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3290 || n = 3328 || n = 3347 || n = 3366 || n = 3404 || n = 3423)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3292 || n = 3311 || n = 3330 || n = 3368 || n = 3425)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif((n .mod 14 = 3) && ((n > 3292 && n < 3434)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3294 && n < 3436)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3297)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
.elseif(n = 3299)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3302)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3304)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
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
.elseif(n = 3387)
	; 9 bytes
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3389)
	; 10 bytes
	delay_n (n-2)
	nop
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
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3406)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3409)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
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
.elseif(n = 3424)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
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
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
; 3438 to 3593
.if(n = 3438 || n = 3440 || n = 3441 || n = 3446 || n = 3448 || (n > 3449 && n < 3454) || n = 3455 || (n > 3456 && n < 3461) || n = 3462 || n = 3464 || (n > 3465 && n < 3471) || n = 3472 || n = 3474 || n = 3476 || n = 3478 || n = 3479 || n = 3481 || n = 3484 || n = 3486 || (n > 3491 && n < 3497) || n = 3498 || n = 3500 || n = 3502 || n = 3504 || n = 3506 || (n > 3507 && n < 3517) || n = 3521 || (n > 3525 && n < 3531) || n = 3532 || (n > 3533 && n < 3537) || n = 3538 || n = 3540 || n = 3542 || n = 3544 || n = 3546 || (n > 3547 && n < 3556) || n = 3557 || n = 3560 || n = 3562 || n = 3564 || (n > 3565 && n < 3573) || n = 3574 || n = 3576 || (n > 3577 && n < 3583) || n = 3584 || n = 3586 || n = 3588 || n = 3591 || n = 3593)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3439)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3442 || n = 3480 || n = 3518 || n = 3537)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3443 || n = 3483 || n = 3523 || n = 3543 || n = 3563 || n = 3583)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3444 || n = 3482 || n = 3520 || n = 3577)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3445 || n = 3465 || n = 3485 || n = 3525 || n = 3565 || n = 3585)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif((n .mod 14 = 3) && ((n > 3446 && n < 3490) || (n > 3516 && n < 3574)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3448 && n < 3576)))
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
.elseif(n = 3456 || n = 3473 || n = 3490 || n = 3507 || n = 3524 || n = 3541 || n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3471 || n = 3488 || n = 3522 || n = 3539 || n = 3556 || n = 3590)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3487)
	; 10 bytes
	clc
	lda #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
.elseif(n = 3497)
	; 10 bytes
	sec
	lda #184 ;hides 'clv'
	jsr @nearby_rts
	sbc #1
	Jne *-6
.elseif(n = 3499)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 3501)
	; 9 bytes
	sec
	lda #(n-1)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3503)
	; 9 bytes
	tya
	ldy #(n-3)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3587)
	; 10 bytes
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3589)
	; 10 bytes
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
.endif
.else
; 3594 to 3750
.if(n = 3594 || n = 3613 || n = 3632 || n = 3651 || n = 3670 || n = 3689 || n = 3708 || n = 3727 || n = 3746)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3595 || (n > 3596 && n < 3603) || n = 3604 || n = 3606 || n = 3608 || (n > 3609 && n < 3613) || n = 3614 || (n > 3615 && n < 3623) || (n > 3626 && n < 3632) || n = 3633 || (n > 3634 && n < 3641) || n = 3642 || (n > 3646 && n < 3651) || n = 3652 || n = 3654 || n = 3656 || n = 3657 || n = 3659 || n = 3661 || n = 3662 || n = 3664 || (n > 3665 && n < 3670) || n = 3671 || n = 3673 || n = 3674 || n = 3676 || (n > 3677 && n < 3681) || n = 3682 || n = 3684 || (n > 3685 && n < 3689) || n = 3690 || n = 3693 || (n > 3694 && n < 3699) || (n > 3699 && n < 3703) || n = 3704 || n = 3706 || n = 3707 || n = 3712 || n = 3714 || (n > 3715 && n < 3723) || n = 3724 || (n > 3729 && n < 3743) || n = 3744 || n = 3749 || n = 3750)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3596 || n = 3615 || n = 3634 || n = 3653 || n = 3672 || n = 3691 || n = 3710 || n = 3729 || n = 3748)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3603 || n = 3623 || n = 3663 || n = 3683 || n = 3703 || n = 3723)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3605 || n = 3625 || n = 3645 || n = 3665 || n = 3685 || n = 3705 || n = 3725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3607 || n = 3624 || n = 3641 || n = 3658 || n = 3675 || n = 3692 || n = 3709 || n = 3726 || n = 3743)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3609 || n = 3626 || n = 3643 || n = 3660 || n = 3677 || n = 3694 || n = 3711 || n = 3728)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3644)
	; 9 bytes
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3646)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3655)
	; 10 bytes
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 3681)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3699)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3713)
	; 10 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3745)
	; 10 bytes
	sec
	lda #(n-1)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
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
; 3751 to 4062
.if((n > 3750 && n < 3760) || n = 3761 || n = 3764 || n = 3766 || (n > 3767 && n < 3772) || n = 3773 || n = 3775 || n = 3776 || n = 3778 || (n > 3779 && n < 3783) || (n > 3786 && n < 3794) || (n > 3796 && n < 3803) || n = 3804 || (n > 3805 && n < 3811) || n = 3812 || (n > 3813 && n < 3822) || n = 3826 || n = 3827 || n = 3829 || (n > 3830 && n < 3841) || n = 3842 || n = 3844 || n = 3846 || (n > 3847 && n < 3860) || n = 3861 || n = 3866 || (n > 3867 && n < 3879) || n = 3880 || n = 3882 || n = 3884 || (n > 3885 && n < 3891) || (n > 3891 && n < 3896) || n = 3897 || n = 3899 || n = 3904 || n = 3906 || (n > 3907 && n < 3913) || n = 3914 || n = 3916 || n = 3918 || (n > 3919 && n < 3923) || n = 3924 || (n > 3925 && n < 3930) || n = 3931 || (n > 3932 && n < 3936) || n = 3937 || (n > 3938 && n < 3943) || n = 3944 || n = 3946 || n = 3948 || (n > 3949 && n < 3955) || n = 3956 || (n > 3957 && n < 3963) || (n > 3966 && n < 3974) || n = 3975 || (n > 3976 && n < 3981) || n = 3982 || n = 3984 || (n > 3985 && n < 3993) || n = 3994 || n = 3996 || n = 3997 || n = 3999 || n = 4002 || n = 4004 || (n > 4005 && n < 4012) || n = 4013 || n = 4016 || (n > 4017 && n < 4023) || n = 4024 || (n > 4025 && n < 4031) || (n > 4034 && n < 4043) || n = 4044 || (n > 4045 && n < 4049) || (n > 4052 && n < 4063))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3760 || n = 3777 || n = 3794 || n = 3811 || n = 3828 || n = 3845 || n = 3862 || n = 3879 || n = 3896 || n = 3913 || n = 3930 || n = 3947 || n = 3964 || n = 3981 || n = 3998 || n = 4015 || n = 4032 || n = 4049)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3762 || n = 3779 || n = 3796 || n = 3813 || n = 3830 || n = 3847 || n = 3864 || n = 3881 || n = 3898 || n = 3915 || n = 3932 || n = 3949 || n = 3966 || n = 3983 || n = 4000 || n = 4017 || n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3763 || n = 3783 || n = 3823 || n = 3863 || n = 3883 || n = 3923 || n = 3943 || n = 3963 || n = 4003 || n = 4023 || n = 4043)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3765 || n = 3785 || n = 3825 || n = 3865 || n = 3885 || n = 3905 || n = 3925 || n = 3945 || n = 3965 || n = 3985 || n = 4005 || n = 4025 || n = 4045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3767 || n = 3786 || n = 3805 || n = 3824 || n = 3843 || n = 3900 || n = 3919 || n = 3938 || n = 3957 || n = 3976 || n = 3995 || n = 4014 || n = 4033 || n = 4052)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3772)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
.elseif(n = 3774)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3784 || n = 3822 || n = 3841 || n = 3917 || n = 3936 || n = 3955 || n = 3974 || n = 3993 || n = 4012 || n = 4031 || n = 4050)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3795)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3803)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
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
.elseif(n = 3891)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3901)
	; 9 bytes
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3902)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3903)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3907)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 4001)
	; 10 bytes
	sec
	lda #(n-1)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
.endif
.else
; 4063 to 4375
.if(n = 4063)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
.elseif(n = 4064 || n = 4067 || n = 4070 || (n > 4071 && n < 4083) || n = 4084 || n = 4086 || n = 4087 || n = 4089 || (n > 4090 && n < 4100) || n = 4101 || n = 4104 || n = 4106 || n = 4108 || (n > 4109 && n < 4117) || n = 4118 || (n > 4119 && n < 4123) || n = 4124 || n = 4127 || (n > 4128 && n < 4134) || n = 4135 || (n > 4136 && n < 4143) || n = 4144 || n = 4146 || (n > 4147 && n < 4151) || n = 4152 || (n > 4153 && n < 4158) || n = 4162 || n = 4167 || n = 4169 || (n > 4170 && n < 4183) || n = 4184 || n = 4186 || (n > 4187 && n < 4202) || (n > 4205 && n < 4219) || n = 4220 || n = 4222 || n = 4224 || (n > 4225 && n < 4236) || n = 4237 || n = 4239 || n = 4241 || n = 4244 || (n > 4245 && n < 4253) || n = 4254 || (n > 4255 && n < 4259) || n = 4260 || n = 4262 || n = 4264 || (n > 4266 && n < 4270) || n = 4271 || (n > 4272 && n < 4278) || n = 4279 || n = 4281 || n = 4282 || n = 4284 || n = 4286 || n = 4288 || (n > 4289 && n < 4297) || n = 4298 || (n > 4299 && n < 4303) || (n > 4306 && n < 4316) || n = 4317 || n = 4319 || n = 4320 || n = 4322 || n = 4324 || (n > 4325 && n < 4335) || n = 4336 || n = 4339 || n = 4341 || n = 4342 || n = 4344 || (n > 4345 && n < 4354) || n = 4358 || (n > 4359 && n < 4363) || n = 4364 || (n > 4365 && n < 4371) || n = 4372 || n = 4374)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4065 || n = 4105 || n = 4125 || n = 4145 || n = 4165 || n = 4205 || n = 4225 || n = 4245 || n = 4265 || n = 4285 || n = 4305 || n = 4325 || n = 4345 || n = 4365)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4066 || n = 4083 || n = 4100 || n = 4117 || n = 4134 || n = 4151 || n = 4168 || n = 4185 || n = 4202 || n = 4219 || n = 4236 || n = 4253 || n = 4270 || n = 4287 || n = 4304 || n = 4321 || n = 4338)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 4068 || n = 4085 || n = 4102 || n = 4119 || n = 4136 || n = 4153 || n = 4170 || n = 4187 || n = 4204 || n = 4221 || n = 4238 || n = 4255 || n = 4272 || n = 4289 || n = 4306 || n = 4323 || n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4069 || n = 4088 || n = 4107 || n = 4126 || n = 4164 || n = 4240 || n = 4259 || n = 4278 || n = 4297 || n = 4316 || n = 4335 || n = 4354 || n = 4373)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4071 || n = 4090 || n = 4109 || n = 4128 || n = 4147 || n = 4166 || n = 4223 || n = 4242 || n = 4261 || n = 4280 || n = 4299 || n = 4318 || n = 4337 || n = 4356 || n = 4375)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4103 || n = 4123 || n = 4143 || n = 4163 || n = 4183 || n = 4243 || n = 4263 || n = 4283 || n = 4303 || n = 4343 || n = 4363)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4158)
	; 9 bytes
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4159)
	; 10 bytes
	clc
	lda #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
.elseif(n = 4160)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4161)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4266)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tax
.elseif(n = 4355)
	; 10 bytes
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 4371)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	jsr @nearby_rts
	adc #1
	Jne *-6
.endif
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4383) || n = 4384 || (n > 4385 && n < 4392) || n = 4393 || (n > 4394 && n < 4403) || n = 4404 || (n > 4405 && n < 4409) || n = 4410 || n = 4412 || n = 4414 || n = 4416 || (n > 4417 && n < 4423) || n = 4424 || (n > 4425 && n < 4430) || n = 4431 || (n > 4432 && n < 4443) || n = 4444 || n = 4446 || n = 4448 || n = 4450 || (n > 4451 && n < 4463) || n = 4464 || n = 4466 || n = 4467 || (n > 4470 && n < 4483) || n = 4484 || n = 4486 || n = 4488 || (n > 4489 && n < 4503) || n = 4504 || n = 4507 || (n > 4508 && n < 4523) || n = 4524 || n = 4526 || (n > 4527 && n < 4543) || (n > 4546 && n < 4563) || n = 4564 || (n > 4565 && n < 4579) || n = 4580 || (n > 4585 && n < 4601) || n = 4602 || n = 4604 || (n > 4605 && n < 4620) || n = 4621 || n = 4624 || (n > 4625 && n < 4639) || n = 4640 || n = 4642 || n = 4644 || (n > 4645 && n < 4658) || n = 4659 || n = 4661 || n = 4662 || n = 4664 || (n > 4665 && n < 4672) || n = 4673 || n = 4675 || n = 4676 || n = 4678 || (n > 4679 && n < 4683) || n = 4684 || (n > 4685 && n < 4696) || n = 4697 || (n > 4698 && n < 4703) || n = 4704 || (n > 4705 && n < 4715) || n = 4716 || (n > 4717 && n < 4723) || n = 4724 || (n > 4725 && n < 4734) || n = 4735 || (n > 4736 && n < 4743) || n = 4744 || (n > 4745 && n < 4751) || n = 4752 || n = 4754 || (n > 4755 && n < 4763) || n = 4764 || (n > 4765 && n < 4772) || n = 4773 || (n > 4774 && n < 4783) || n = 4784 || (n > 4785 && n < 4791) || n = 4792 || (n > 4793 && n < 4797) || n = 4798 || (n > 4799 && n < 4803) || n = 4804 || (n > 4805 && n < 4810) || n = 4811 || (n > 4812 && n < 4823) || n = 4824 || (n > 4825 && n < 4829) || n = 4830 || n = 4832 || (n > 4833 && n < 4843) || n = 4844 || n = 4846 || n = 4847 || n = 4849 || (n > 4850 && n < 4863) || n = 4864 || n = 4866 || n = 4868 || (n > 4869 && n < 4873) || n = 4874 || (n > 4875 && n < 4883) || n = 4884 || (n > 4885 && n < 4903) || n = 4904 || (n > 4905 && n < 4915) || n = 4916 || (n > 4917 && n < 4923) || n = 4924 || (n > 4925 && n < 4929) || n = 4930 || (n > 4931 && n < 4943) || n = 4944 || (n > 4945 && n < 4963) || n = 4964 || (n > 4965 && n < 4983) || n = 4984 || (n > 4985 && n < 5001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4383 || n = 4423 || n = 4443 || n = 4463 || n = 4483 || n = 4503 || n = 4523 || n = 4543 || n = 4583 || n = 4623 || n = 4643 || n = 4663 || n = 4683 || n = 4703 || n = 4723 || n = 4743 || n = 4763 || n = 4783 || n = 4803 || n = 4823 || n = 4843 || n = 4863 || n = 4883 || n = 4903 || n = 4923 || n = 4943 || n = 4963 || n = 4983)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4385 || n = 4405 || n = 4425 || n = 4445 || n = 4465 || n = 4485 || n = 4505 || n = 4525 || n = 4545 || n = 4585 || n = 4605 || n = 4625 || n = 4645 || n = 4665 || n = 4685 || n = 4705 || n = 4725 || n = 4745 || n = 4765 || n = 4785 || n = 4805 || n = 4825 || n = 4845 || n = 4865 || n = 4885 || n = 4905 || n = 4925 || n = 4945 || n = 4965 || n = 4985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4392 || n = 4430 || n = 4468 || n = 4487 || n = 4506 || n = 4544 || n = 4563 || n = 4582 || n = 4601 || n = 4620 || n = 4639 || n = 4658 || n = 4677 || n = 4696 || n = 4715 || n = 4734 || n = 4753 || n = 4772 || n = 4791 || n = 4810 || n = 4829 || n = 4848)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4394 || n = 4413 || n = 4432 || n = 4451 || n = 4470 || n = 4489 || n = 4508 || n = 4527 || n = 4546 || n = 4565 || n = 4584 || n = 4603 || n = 4622 || n = 4641 || n = 4660 || n = 4679 || n = 4698 || n = 4717 || n = 4736 || n = 4755 || n = 4774 || n = 4793 || n = 4812 || n = 4831 || n = 4850)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 4409)
	; 10 bytes
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
.elseif(n = 4411)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
.elseif(n = 4415)
	; 9 bytes
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4417 || n = 4674 || n = 4931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4447 || n = 4751)
	; 10 bytes
	sec
	lda #(n-1)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
.elseif(n = 4449)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4469)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	tax
.elseif(n = 4579 || n = 4915)
	; 10 bytes
	sec
	lda #(n-1)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
.elseif(n = 4581 || n = 4917)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4672)
	; 9 bytes
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4797)
	; 10 bytes
	sec
	lda #(n-1)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
.elseif(n = 4799)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
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
.elseif(n = 4867)
	; 10 bytes
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4869)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4873)
	; 10 bytes
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
.elseif(n = 4875)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.elseif(n = 4929)
	; 9 bytes
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if(n = 5001 || n = 5002 || n = 5004 || (n > 5005 && n < 5023) || n = 5024 || (n > 5025 && n < 5043) || n = 5044 || (n > 5045 && n < 5061) || n = 5062 || n = 5064 || (n > 5065 && n < 5083) || n = 5084 || (n > 5085 && n < 5103) || n = 5104 || n = 5106 || (n > 5107 && n < 5123) || n = 5124 || (n > 5125 && n < 5149) || n = 5150 || (n > 5151 && n < 5186) || n = 5187 || (n > 5188 && n < 5251) || n = 5252 || (n > 5253 && n < 5443) || n = 5444 || (n > 5445 && n < 5501) || n = 5502 || (n > 5503 && n < 5700) || n = 5701 || (n > 5702 && n < 5957) || n = 5958 || (n > 5959 && n < 6214) || n = 6215 || (n > 6216 && n < 6471) || n = 6472 || (n > 6473 && n < 6728) || n = 6729 || (n > 6730 && n < 6985) || n = 6986 || (n > 6987 && n < 7242) || n = 7243 || (n > 7244 && n < 7499) || n = 7500)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5003 || n = 5023 || n = 5043 || n = 5063 || n = 5083 || n = 5103)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 5005 || n = 5025 || n = 5045 || n = 5065 || n = 5085 || n = 5105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5061)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	adc #1
	Jne *-6
.elseif(n = 5107)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5123)
	; 10 bytes
	sec
	lda #0
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 5125)
	; 10 bytes
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5149 || n = 5501)
	; 10 bytes
	sec
	lda #(n-1)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
.elseif(n = 5151 || n = 5503)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.elseif(n = 5186)
	; 9 bytes
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5188 || n = 5445 || n = 5702 || n = 5959 || n = 6216 || n = 6473 || n = 6730 || n = 6987 || n = 7244)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5251)
	; 10 bytes
	sec
	lda #(n-1)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
.elseif(n = 5253)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 5443)
	; 9 bytes
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5700)
	; 9 bytes
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5957)
	; 9 bytes
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6214)
	; 9 bytes
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6471)
	; 9 bytes
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6728)
	; 9 bytes
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6985)
	; 9 bytes
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 7242)
	; 9 bytes
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 7499)
	; 9 bytes
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.else
; 7501 to 10000
.if(n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n > 7501 && n < 7756) || n = 7757 || (n > 7758 && n < 8013) || n = 8014 || (n > 8015 && n < 8270) || n = 8271 || (n > 8272 && n < 8527) || n = 8528 || (n > 8529 && n < 8784) || n = 8785 || (n > 8786 && n < 9041) || n = 9042 || (n > 9043 && n < 9298) || n = 9299 || (n > 9300 && n < 9555) || n = 9556 || (n > 9557 && n < 9812) || n = 9813 || (n > 9814 && n < 10001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 7756)
	; 9 bytes
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8013)
	; 9 bytes
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8270)
	; 9 bytes
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8527)
	; 9 bytes
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 8784)
	; 9 bytes
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9041)
	; 9 bytes
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9298)
	; 9 bytes
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9555)
	; 9 bytes
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 9812)
	; 9 bytes
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10071 && n < 10326) || n = 10327 || (n > 10328 && n < 10583) || n = 10584 || (n > 10585 && n < 10840) || n = 10841 || (n > 10842 && n < 11097) || n = 11098 || (n > 11099 && n < 11354) || n = 11355 || (n > 11356 && n < 11611) || n = 11612 || (n > 11613 && n < 11868) || n = 11869 || (n > 11870 && n < 12125) || n = 12126 || (n > 12127 && n < 12382) || n = 12383 || (n > 12384 && n < 12639) || n = 12640 || (n > 12641 && n < 12896) || n = 12897 || (n > 12898 && n < 13153) || n = 13154 || (n > 13155 && n < 13410) || n = 13411 || (n > 13412 && n < 13667) || n = 13668 || (n > 13669 && n < 13924) || n = 13925 || (n > 13926 && n < 14181) || n = 14182 || (n > 14183 && n < 14438) || n = 14439 || (n > 14440 && n < 14695) || n = 14696 || (n > 14697 && n < 14952) || n = 14953 || (n > 14954 && n < 15001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10069)
	; 9 bytes
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10071 || n = 10328 || n = 10585 || n = 10842 || n = 11099 || n = 11356 || n = 11613 || n = 11870 || n = 12127 || n = 12384 || n = 12641 || n = 12898 || n = 13155 || n = 13412 || n = 13669 || n = 13926 || n = 14183 || n = 14440 || n = 14697 || n = 14954)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10326)
	; 9 bytes
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10583)
	; 9 bytes
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 10840)
	; 9 bytes
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11097)
	; 9 bytes
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11354)
	; 9 bytes
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11611)
	; 9 bytes
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 11868)
	; 9 bytes
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12125)
	; 9 bytes
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12382)
	; 9 bytes
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12639)
	; 9 bytes
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 12896)
	; 9 bytes
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13153)
	; 9 bytes
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13410)
	; 9 bytes
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13667)
	; 9 bytes
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 13924)
	; 9 bytes
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14181)
	; 9 bytes
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14438)
	; 9 bytes
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14695)
	; 9 bytes
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 14952)
	; 9 bytes
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15211 && n < 15466) || n = 15467 || (n > 15468 && n < 15723) || n = 15724 || (n > 15725 && n < 15980) || n = 15981 || (n > 15982 && n < 16237) || n = 16238 || (n > 16239 && n < 16494) || n = 16495 || (n > 16496 && n < 16751) || n = 16752 || (n > 16753 && n < 17008) || n = 17009 || (n > 17010 && n < 17265) || n = 17266 || (n > 17267 && n < 17522) || n = 17523 || (n > 17524 && n < 17779) || n = 17780 || (n > 17781 && n < 18036) || n = 18037 || (n > 18038 && n < 18293) || n = 18294 || (n > 18295 && n < 18550) || n = 18551 || (n > 18552 && n < 18807) || n = 18808 || (n > 18809 && n < 19064) || n = 19065 || (n > 19066 && n < 19321) || n = 19322 || (n > 19323 && n < 19578) || n = 19579 || (n > 19580 && n < 19835) || n = 19836 || (n > 19837 && n < 20001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15209)
	; 9 bytes
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15211 || n = 15468 || n = 15725 || n = 15982 || n = 16239 || n = 16496 || n = 16753 || n = 17010 || n = 17267 || n = 17524 || n = 17781 || n = 18038 || n = 18295 || n = 18552 || n = 18809 || n = 19066 || n = 19323 || n = 19580 || n = 19837)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 15466)
	; 9 bytes
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15723)
	; 9 bytes
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 15980)
	; 9 bytes
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 16237)
	; 9 bytes
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 16494)
	; 9 bytes
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 16751)
	; 9 bytes
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17008)
	; 9 bytes
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17265)
	; 9 bytes
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17522)
	; 9 bytes
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 17779)
	; 9 bytes
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18036)
	; 9 bytes
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18293)
	; 9 bytes
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18550)
	; 9 bytes
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 18807)
	; 9 bytes
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19064)
	; 9 bytes
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19321)
	; 9 bytes
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19578)
	; 9 bytes
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 19835)
	; 9 bytes
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.endif

.endif
	rest
.endmacro
