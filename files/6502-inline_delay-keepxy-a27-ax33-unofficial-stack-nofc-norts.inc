; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, S, Z+N, C, D, and V
; Will preserve X, Y, and I (interrupt-disable flag)
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
; - 5 bytes or less when n <= 21
; - 6 bytes or less when n <= 288
; - 7 bytes or less when n <= 293
; - 8 bytes or less when n <= 1289
; - 9 bytes or less when n <= 1305
; -10 bytes or less when n <= 2233
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=3, avg=4.2, max=5
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=4, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.3, max=8
; - When 301 <= n <= 500: min=6, avg=7.5, max=8
; - When 501 <= n <= 750: min=6, avg=7.6, max=8
; - When 751 <= n <= 1000: min=7, avg=7.6, max=8
; - When 1001 <= n <= 2000: min=6, avg=8.2, max=10
; - When 2001 <= n <= 3000: min=8, avg=9.5, max=11
; - When 3001 <= n <= 5000: min=9, avg=10.8, max=11
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
.elseif(n = 8 || n = 12 || n = 19)
	; 2..5 bytes
	delay_n (n-4)
	pla
.elseif(n = 9 || n = 10 || n = 13)
	; 3..4 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 285 || n = 286 || n = 297 || n = 307 || n = 312)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 287 || n = 294 || n = 301)
	; 6..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 288 || n = 302)
	; 6..7 bytes
	pla
	delay_n (n-4)
.elseif(n = 289 || n = 290)
	; 7 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 291)
	; 7 bytes
	delay_n (n-4)
	pla
.elseif(n = 292)
	; 7 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 293 || n = 303 || n = 308)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 295 || n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 296)
	; 8 bytes
	delay_n 12
	delay_n (n-12)
.elseif(n = 298)
	; 6 bytes
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
.elseif(n = 299 || n = 306)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 309)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
.elseif(n = 311)
	; 7 bytes
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	nop
	pha
	Jne *-4
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313)
	; 6 bytes
	sec
	lda #$E9 ;hides 'sbc #$08'
	php ;first loop only
	Jne *-2
.elseif(n = 314 || n = 317 || n = 319 || n = 324 || n = 329 || n = 344 || n = 349 || n = 359 || n = 364 || n = 379 || n = 384 || n = 387 || n = 389)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif(n = 315 || n = 320 || n = 345 || n = 350 || n = 365 || n = 385)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 316 || n = 326 || n = 336 || n = 351 || n = 356 || n = 366 || n = 386)
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 318 || n = 323 || n = 338 || n = 353 || n = 378)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 321)
	; 7 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	nop
	pha
	Jne *-4
.elseif(n = 322 || n = 332 || n = 337 || n = 352 || n = 357 || n = 367 || n = 372 || n = 377 || n = 382)
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 331)
	; 8 bytes
	sec
	lda #(n-11)/8
	pha
	sbc #1
	Jpl *-3
.elseif(n = 339)
	; 7 bytes
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
.elseif(n = 342)
	; 7 bytes
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	delay_n 6
	Jne *-4
.elseif(n = 346)
	; 7 bytes
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
.elseif(n = 347)
	; 7 bytes
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	pha
	Jne *-3
.elseif(n = 354)
	; 8 bytes
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	nop
	nop
	Jne *-4
.elseif(n = 361)
	; 8 bytes
	clc
	lda #216 ;hides 'cld'
	nop
	adc #1
	Jne *-4
.elseif(n = 362 || n = 369)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 371)
	; 7 bytes
	sec
	lda #$69 ;hides 'adc #$1A'
	.byte $1A ;nop ;first loop only
	pha
	Jne *-3
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
.if(n = 391)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif((n .mod 5 = 2) && (n = 392 || (n > 411 && n < 428) || (n > 441 && n < 463)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 392 && n < 419) || (n > 432 && n < 449) || n = 458 || n = 468))
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 394 || n = 406 || n = 419 || n = 424 || n = 429 || n = 434 || n = 454 || n = 464)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 0) && (n = 395 || n = 405 || n = 420 || (n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 396 || n = 401 || n = 426 || n = 431 || n = 436 || n = 441 || n = 456 || n = 466)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 397 || n = 467)
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
.elseif(n = 411)
	; 7 bytes
	sec
	lda #$EB ;hides 'sbc #$7A'
	.byte $7A ;nop ;first loop only
	pha
	Jne *-3
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
.elseif(n = 421 || n = 461)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 423 || n = 428 || n = 453 || n = 463)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 437)
	; 7 bytes
	sec
	lda #$E9 ;hides 'sbc #$08'
	php ;first loop only
	nop
	Jne *-3
.elseif(n = 444)
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
.elseif(n = 451)
	; 7 bytes
	sec
	lda #$69 ;hides 'adc #$D8'
	cld ;first loop only
	pha
	Jne *-3
.elseif(n = 459)
	; 7 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
	tay
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
.elseif((n .mod 5 = 2) && ((n > 471 && n < 498) || (n > 511 && n < 533) || n = 542))
	; 8 bytes
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
.elseif(n = 494 || n = 499 || n = 504 || n = 511 || n = 524 || n = 529 || n = 539)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 496 || n = 506 || n = 526 || n = 536)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 501)
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
.elseif(n = 519)
	; 8 bytes
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
.elseif(n = 531)
	; 8 bytes
	sec
	lda #(n-11)/8
	pha
	sbc #1
	Jpl *-3
.elseif(n = 534)
	; 8 bytes
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
.elseif(n = 541)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
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
.if((n .mod 5 = 2) && ((n > 546 && n < 568) || (n > 576 && n < 603) || (n > 611 && n < 623)))
	; 8 bytes
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
.elseif((n .mod 5 = 4) && (n = 559 || n = 569 || n = 574 || (n > 588 && n < 610)))
	; 8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 566 || n = 576 || n = 596 || n = 601 || n = 616)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 0) && ((n > 569 && n < 586) || n = 595 || (n > 614 && n < 626)))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 571)
	; 8 bytes
	sec
	lda #(n-11)/8
	pha
	sbc #1
	Jpl *-3
.elseif(n = 572 || n = 579)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 581)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 611)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
	tax
.elseif(n = 624)
	; 7 bytes
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jmi *-3
.endif
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
.elseif((n .mod 5 = 2) && ((n > 626 && n < 638) || (n > 646 && n < 673) || n = 682 || n = 687 || (n > 696 && n < 708) || (n > 721 && n < 743) || (n > 756 && n < 778)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 628 || n = 633 || n = 638 || n = 648)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 629 || n = 634 || n = 639 || n = 644 || n = 659 || n = 664 || n = 669 || n = 674 || n = 679 || n = 692 || n = 694 || n = 699 || n = 704 || n = 709 || n = 714 || n = 734 || n = 739 || n = 744 || n = 749 || n = 756 || n = 764 || n = 769 || n = 774 || n = 779)
	; 7..8 bytes
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
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif((n .mod 5 = 0) && (n = 635 || (n > 654 && n < 781)))
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif((n .mod 5 = 1) && ((n > 635 && n < 647) || (n > 665 && n < 692) || n = 706 || n = 716 || n = 721 || n = 736 || n = 746 || n = 751 || n = 776))
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 642 || n = 656 || n = 677 || n = 712 || n = 719 || n = 747 || n = 754 || n = 761)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && (n = 643 || (n > 652 && n < 684) || (n > 692 && n < 779)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 649 || n = 684 || n = 726)
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
.elseif(n = 654 || n = 661 || n = 759 || n = 766)
	; 8 bytes
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jpl *-3
.elseif(n = 688)
	; 6 bytes
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
.elseif(n = 689 || n = 696 || n = 724)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 701 || n = 741 || n = 781)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
	Jpl *-2
	tay
.elseif(n = 711)
	; 8 bytes
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
.elseif(n = 717)
	; 7 bytes
	tya
	ldy #26 ;hides 'nop'
	iny
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
.elseif(n = 752)
	; 7 bytes
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
.elseif(n = 771)
	; 8 bytes
	sec
	lda #(n-11)/8
	pha
	sbc #1
	Jpl *-3
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
.elseif((n .mod 5 = 4) && (n = 784 || n = 799 || (n > 808 && n < 820) || (n > 833 && n < 855) || (n > 868 && n < 890) || (n > 903 && n < 915) || n = 924))
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
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
.elseif((n .mod 5 = 2) && ((n > 786 && n < 813) || (n > 821 && n < 848) || (n > 866 && n < 883) || (n > 891 && n < 918) || (n > 926 && n < 938)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 791 || n = 806 || n = 816 || n = 826 || n = 841 || n = 846 || n = 856 || n = 876 || n = 881 || n = 886 || n = 896 || n = 911 || n = 916 || n = 926)
	; 8 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 811)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
.elseif(n = 821 || n = 861)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 851 || n = 891)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
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
.elseif(n = 931)
	; 8 bytes
	sec
	lda #(n-11)/8
	pha
	sbc #1
	Jpl *-3
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
.elseif((n .mod 5 = 2) && ((n > 941 && n < 953) || (n > 966 && n < 988) || (n > 996 && n < 1018) || (n > 1031 && n < 1058) || (n > 1066 && n < 1093) || (n > 1101 && n < 1128) || (n > 1136 && n < 1163) || (n > 1171 && n < 1198) || (n > 1206 && n < 1233) || n = 1242 || n = 1247))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 974 || n = 979 || n = 989 || n = 994 || n = 1009 || n = 1014 || n = 1019 || n = 1022 || n = 1024 || n = 1029 || n = 1044 || n = 1049 || n = 1059 || n = 1064 || n = 1079 || n = 1084 || n = 1089 || n = 1094 || n = 1114 || n = 1124 || n = 1129 || n = 1134 || n = 1149 || n = 1154 || n = 1159 || n = 1164 || n = 1169 || n = 1184 || n = 1189 || n = 1194 || n = 1199 || n = 1204 || n = 1219 || n = 1224 || n = 1229 || n = 1234 || n = 1239)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 945 && n < 962) || (n > 985 && n < 1002) || n = 1016 || n = 1021 || n = 1031 || n = 1036 || n = 1066 || n = 1071 || n = 1086 || n = 1096 || n = 1106 || n = 1121 || n = 1126 || n = 1136 || (n > 1155 && n < 1167) || n = 1176 || (n > 1190 && n < 1207) || n = 1226 || n = 1231 || n = 1241 || n = 1246))
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 956 && n < 972) || (n > 991 && n < 1007) || (n > 1026 && n < 1042) || (n > 1061 && n < 1077) || (n > 1096 && n < 1112) || (n > 1131 && n < 1147) || (n > 1166 && n < 1182) || (n > 1201 && n < 1217) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
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
.elseif(n = 1051 || n = 1091 || n = 1131 || n = 1171 || n = 1211)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
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
.elseif(n = 1061 || n = 1141)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 1236)
	; 8 bytes
	tya
	ldy #136 ;hides 'dey'
	nop
	nop
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
.elseif(n = 1252 || n = 1257 || n = 1262 || n = 1267 || n = 1277 || n = 1282 || n = 1287 || n = 1302 || n = 1306 || n = 1316 || n = 1327)
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
.elseif(n = 1259 || n = 1264 || n = 1269 || n = 1274 || n = 1295 || n = 1297 || n = 1304 || n = 1318)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1261)
	; 8 bytes
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 1266 || n = 1271 || n = 1276 || n = 1281 || n = 1288 || n = 1292 || n = 1294 || n = 1320 || n = 1322 || n = 1324)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1290)
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
.elseif(n = 1296)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1299 || n = 1315 || n = 1323)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif((n .mod 8 = 5) && ((n > 1300 && n < 1326)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1303)
	; 9 bytes
	tya
	ldy #(n-16)/11
	delay_n 6
	dey
	Jpl *-3
	tay
.elseif(n = 1308)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1310)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1311)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
.elseif(n = 1313)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1360)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1330 || n = 1344 || n = 1348 || n = 1351 || n = 1358)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1339 || n = 1355)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1332)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	nop
	sbc #1
	Jmi *-5
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1337 || n = 1346 || n = 1364)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1338 || n = 1350 || n = 1352 || n = 1366)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1341 || n = 1357 || n = 1365)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
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
.elseif(n = 1353 || n = 1362)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1359)
	; 9 bytes
	sec
	lda #(n-15)/12
	delay_n 7
	sbc #1
	Jpl *-4
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
.if(n = 1368 || n = 1375 || n = 1382 || n = 1396)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1371 || n = 1379 || n = 1395)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1372 || n = 1386 || n = 1399)
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
.elseif(n = 1374 || n = 1388 || n = 1393 || n = 1402)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1376 || n = 1390 || n = 1392 || n = 1401 || n = 1406)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1383)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1385)
	; 9 bytes
	tya
	ldy #114
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
.elseif(n = 1394)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1403)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
.elseif(n = 1404)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	sbc #1
	Jmi *-5
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1423 || n = 1428 || n = 1430 || n = 1441 || n = 1458 || n = 1465)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1408 || n = 1420 || n = 1422 || n = 1427 || n = 1450 || n = 1462 || n = 1464 || n = 1476 || n = 1478)
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
.elseif((n .mod 7 = 3) && (n = 1410 || n = 1417 || (n > 1430 && n < 1481)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1411 || n = 1435 || n = 1443 || n = 1451 || n = 1467 || n = 1483)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && ((n > 1412 && n < 1438) || n = 1453 || n = 1469 || n = 1477))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1414 || n = 1439 || n = 1442 || n = 1449 || n = 1456 || n = 1460 || n = 1470 || n = 1471 || n = 1474 || n = 1484)
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
.elseif(n = 1416 || n = 1425 || n = 1434 || n = 1479)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1424)
	; 7 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
.elseif(n = 1432)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
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
.elseif(n = 1448)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
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
.endif
.else
; 1485 to 1562
.if(n = 1485 || n = 1493 || n = 1509 || n = 1525 || n = 1533 || n = 1541)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1486 || n = 1514 || n = 1519 || n = 1521 || n = 1528 || n = 1537 || n = 1556)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1509) || n = 1522 || (n > 1535 && n < 1558)))
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
	nop
	nop
	sbc #1
	Jne *-4
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 9 = 5) && (n = 1490 || (n > 1525 && n < 1563)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1491 || n = 1499 || n = 1507 || n = 1523 || n = 1539 || n = 1547 || n = 1555)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1492 || n = 1518 || n = 1530 || n = 1532 || n = 1546 || n = 1548 || n = 1558)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1498 || n = 1502 || n = 1505 || n = 1511 || n = 1512 || n = 1540 || n = 1554 || n = 1561)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1500)
	; 8 bytes
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1504)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1506)
	; 8 bytes
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
.elseif(n = 1513)
	; 7 bytes
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
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
	pha
	delay_n (n-7)
	pla
.elseif(n = 1527)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
.elseif(n = 1529)
	; 7 bytes
	tya
	ldy #(n-3)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
.elseif(n = 1534)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1542)
	; 8 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
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
.if(n = 1563 || n = 1579 || n = 1595 || n = 1603 || n = 1619 || n = 1635)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1607) || n = 1620 || n = 1634))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1567 || n = 1600)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1568 || n = 1574 || n = 1575 || n = 1582 || n = 1583 || n = 1586 || n = 1590 || n = 1596 || n = 1610 || n = 1624 || n = 1638)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1569 || n = 1623)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1570 || n = 1584 || n = 1591 || n = 1593 || n = 1609 || n = 1612 || n = 1617 || n = 1626 || n = 1631 || n = 1633 || n = 1640)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1572)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1576 || n = 1602 || n = 1604 || n = 1616 || n = 1628 || n = 1630)
	; 8..9 bytes
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
.elseif(n = 1588)
	; 8 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1598 || n = 1607)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
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
.elseif(n = 1618)
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
.elseif(n = 1632)
	; 9 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 1642 || n = 1644 || n = 1656 || n = 1658 || n = 1660 || n = 1674 || n = 1681 || n = 1700 || n = 1702 || n = 1712 || n = 1714 || n = 1716)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1645 || n = 1654 || n = 1682 || n = 1689 || n = 1696 || n = 1703 || n = 1710)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1646)
	; 9 bytes
	delay_n (n-4)
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
.elseif(n = 1649)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1651 || n = 1667 || n = 1675 || n = 1691 || n = 1707 || n = 1715)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1652 || n = 1670 || n = 1679 || n = 1688)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 5) && (n = 1653 || n = 1661 || n = 1677 || (n > 1692 && n < 1718)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
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
.elseif(n = 1665 || n = 1705)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1666 || n = 1673 || n = 1680 || n = 1684 || n = 1687 || n = 1694 || n = 1698 || n = 1708)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1668 || n = 1695)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1672)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
	pha
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
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1791)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 1722 || n = 1729 || n = 1736 || n = 1750 || n = 1758 || n = 1766 || n = 1792)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 3) && (n = 1723 || n = 1731 || n = 1747 || (n > 1762 && n < 1788)))
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1724 || n = 1778 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 3) && (n = 1725 || n = 1732 || n = 1746 || (n > 1759 && n < 1789)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1726 || n = 1730 || n = 1740 || n = 1742 || n = 1754 || n = 1768 || n = 1770 || n = 1782 || n = 1786)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1728 || n = 1784)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
	tay
.elseif((n .mod 8 = 5) && (n = 1733 || (n > 1748 && n < 1774) || n = 1789))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1735 || n = 1775)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1737)
	; 7 bytes
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
.elseif(n = 1738 || n = 1743 || n = 1752 || n = 1759 || n = 1761 || n = 1777 || n = 1780)
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
.elseif(n = 1744)
	; 9 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 1756)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
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
.elseif(n = 1795)
	; 8 bytes
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1799 || n = 1822 || n = 1826)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1800 || n = 1806 || n = 1808 || n = 1816 || n = 1824)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1802)
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
.elseif(n = 1809 || n = 1831)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 3) && ((n > 1810 && n < 1836)))
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1812)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1814 || n = 1823 || n = 1832)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
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
.elseif(n = 1817)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 1818)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 1820)
	; 8 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1825)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
	nop
	nop
	Jne *-3
	tax
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
.elseif(n = 1838 || n = 1847 || n = 1858 || n = 1871)
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
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1841)
	; 8 bytes
	sec
	lda #184 ;hides 'clv'
	pha
	sbc #1
	Jne *-4
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
.elseif(n = 1844 || n = 1846 || n = 1854 || n = 1856 || n = 1870 || n = 1872)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 5) && ((n > 1844 && n < 1870)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1848)
	; 8 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 1849 || n = 1852 || n = 1873)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1850 || n = 1868)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif((n .mod 8 = 3) && ((n > 1850 && n < 1876)))
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1855)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1857 || n = 1866)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1860)
	; 10 bytes
	pha
	pla
	delay_n (n-7)
.elseif(n = 1862)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 1863)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1865)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
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
.if((n >= 1876) && (n < 1915))
; 1876 to 1914
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
.elseif((n .mod 8 = 5) && ((n > 1876 && n < 1902)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1878 || n = 1880 || n = 1888 || n = 1896 || n = 1900 || n = 1910 || n = 1914)
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
	nop
	nop
	adc #1
	Jne *-5
.elseif((n .mod 8 = 3) && ((n > 1882 && n < 1908)))
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1884)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif((n .mod 9 = 5) && ((n > 1885 && n < 1914)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
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
	pla
	delay_n (n-4)
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
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
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
.elseif(n = 1898 || n = 1903)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1902)
	; 8 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
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
.elseif(n = 1909)
	; 8 bytes
	sec
	lda #(n-1)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
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
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
	tay
.endif
.else
; 1915 to 1953
.if((n .mod 8 = 3) && ((n > 1914 && n < 1940)))
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1916)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif((n .mod 8 = 5) && ((n > 1916 && n < 1950)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1918 || n = 1926 || n = 1928 || n = 1932 || n = 1934 || n = 1936 || n = 1942 || n = 1944 || n = 1948)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1919 || n = 1930 || n = 1952)
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
.elseif(n = 1922 || n = 1940)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1924 || n = 1953)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1927 || n = 1943 || n = 1946 || n = 1951)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 1929)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
.elseif(n = 1935)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 1945)
	; 8 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	sbc #1
	Jne *-4
.elseif(n = 1947)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1950)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1956 || n = 1992)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 1959)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 1960 || n = 1982 || n = 1986 || n = 1988 || n = 1990)
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
.elseif(n = 1962 || n = 1977 || n = 1980)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
	pha
	sec
	lda #245
	pha
	sbc #1
	Jne *-3
.elseif(n = 1968)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
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
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 1975)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 1978 || n = 1991)
	; 9..10 bytes
	delay_n (n-2)
	nop
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
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2017)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1994 || n = 2012 || n = 2030)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1995 || n = 2011 || n = 2019 || n = 2027)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1996 || n = 2018)
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
.elseif(n = 1998 || n = 2000 || n = 2006 || n = 2008 || n = 2014 || n = 2016 || n = 2022 || n = 2024)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1999 || n = 2002 || n = 2023 || n = 2026)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2001 || n = 2010 || n = 2028)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 2003)
	; 8 bytes
	tya
	ldy #56 ;hides 'sec'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 2004)
	; 10 bytes
	pha
	pla
	delay_n (n-7)
.elseif(n = 2007 || n = 2031)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2009)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2015)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
	nop
	nop
	sbc #1
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032 || n = 2044 || n = 2054 || n = 2056)
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
.elseif(n = 2035 || n = 2043)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 2036)
	; 9 bytes
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
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
.elseif(n = 2040 || n = 2062)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2042 || n = 2047 || n = 2050 || n = 2059)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2046 || n = 2055 || n = 2064)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 2051)
	; 8 bytes
	sec
	lda #0
	pha
	sbc #1
	Jne *-3
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
.elseif(n = 2060)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2065)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2067)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2068)
	; 9 bytes
	clc
	lda #68 ;hides 'nop zp'
	bit $48 ;hides 'pha'
	adc #1
	Jne *-5
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
	; 8 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
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
.elseif(n = 2074 || n = 2076 || n = 2092 || n = 2094)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2075 || n = 2084 || n = 2093)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2077 || n = 2086 || n = 2088 || n = 2095 || n = 2097)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2078 || n = 2080 || n = 2096 || n = 2098)
	; 10 bytes
	delay_n (n-4)
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
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2085)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2087 || n = 2108)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2089)
	; 8 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
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
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 2101)
	; 10 bytes
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2102)
	; 9 bytes
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2104)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2106)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
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
.if(n = 2110 || n = 2112 || n = 2121 || n = 2130 || n = 2144 || n = 2146)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2148)))
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2113 || n = 2122 || n = 2124 || n = 2131 || n = 2140 || n = 2142)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2114 || n = 2116)
	; 10 bytes
	delay_n (n-4)
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
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2123 || n = 2133 || n = 2143)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2125)
	; 8 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 2126 || n = 2137 || n = 2148)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2132)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
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
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2136)
	; 8 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 2145)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.endif
.else
; 2149 to 2187
.if(n = 2149)
	; 9 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2150)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2151 || n = 2175 || n = 2187)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2152 || n = 2157 || n = 2162 || n = 2176)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2153 || n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2154 || n = 2172)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 2155 || n = 2185)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
.elseif(n = 2158)
	; 9 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	bit $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2159 || n = 2170)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2160)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2161)
	; 8 bytes
	sec
	lda #216 ;hides 'cld'
	pha
	sbc #1
	Jne *-4
.elseif(n = 2163)
	; 8 bytes
	tya
	ldy #216 ;hides 'cld'
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2164 || n = 2166 || n = 2168 || n = 2184 || n = 2186)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2167 || n = 2178)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2169)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2171)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2173)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2179)
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	nop
	nop
	adc #1
	Jne *-5
.elseif(n = 2180)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2181)
	; 8 bytes
	sec
	lda #(n-1)/10 ;hides 'nop'
	pha
	sbc #1
	Jne *-4
.elseif(n = 2182)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	pha
	dey
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
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if(n = 2188 || n = 2222)
	; 10 bytes
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
.elseif(n = 2193)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
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
	ldy #12 ;hides 'nop abs'
	nop $0
	iny
	Jne *-4
	tay
.elseif(n = 2202)
	; 9 bytes
	clc
	lda #36 ;hides 'bit zp'
	nop
	nop
	adc #1
	Jne *-5
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
.elseif(n = 2207 || n = 2212 || n = 2220)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
	sec
	lda #184 ;hides 'clv'
	nop
	pha
	sbc #1
	Jne *-5
.elseif(n = 2211 || n = 2223)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2217 || n = 2226)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
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
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227 || n = 2238)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
.elseif(n = 2229)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2230)
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
.elseif(n = 2232 || n = 2240)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2233)
	; 8 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 2239)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2241)
	; 10 bytes
	clc
	lda #116 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 2242)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2243)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
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
	nop
	nop
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
.elseif(n = 2248)
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
.elseif(n = 2250 || n = 2257)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
.elseif(n = 2256)
	; 9 bytes
	pha
	delay_n (n-3)
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
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266 || n = 2276)
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
.elseif(n = 2268 || n = 2277 || n = 2279)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2269)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
.elseif(n = 2273 || n = 2282)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
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
.elseif(n = 2275)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2278)
	; 10 bytes
	pha
	delay_n (n-3)
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
	nop
	nop
	dey
	Jne *-4
	tay
.endif
.else
; 2285 to 2304
.if(n = 2285 || n = 2297)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2286 || n = 2292 || n = 2296)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2287)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2288)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2291 || n = 2300)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2293 || n = 2304)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
	; 8 bytes
	clc
	lda #26 ;hides 'nop'
	pha
	adc #1
	Jne *-4
.elseif(n = 2302)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2303)
	; 8 bytes
	tya
	ldy #26 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.endif
.endif
.else
; 2305 to 2343
.if(n = 2305 || n = 2315)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2306 || n = 2312 || n = 2316 || n = 2318 || n = 2322 || n = 2340)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2310 || n = 2311 || n = 2317 || n = 2328 || n = 2336 || n = 2339)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2313)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2314 || n = 2320)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2319 || n = 2331)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2321)
	; 8 bytes
	clc
	lda #24 ;hides 'clc'
	pha
	adc #1
	Jne *-4
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
.elseif(n = 2325)
	; 9 bytes
	clc
	lda #90 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
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
.elseif(n = 2332)
	; 9 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
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
.elseif(n = 2338)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2341)
	; 8 bytes
	sec
	lda #(n-1)/10 ;hides 'nop'
	pha
	sbc #1
	Jne *-4
.elseif(n = 2342)
	; 10 bytes
	clc
	lda #100 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
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
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2351 || n = 2362 || n = 2366 || n = 2376 || n = 2378)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2347 || n = 2349 || n = 2352 || n = 2374)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2348 || n = 2370)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2350 || n = 2361 || n = 2372)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2353 || n = 2363 || n = 2373)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2354 || n = 2364)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2358)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2359)
	; 9 bytes
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2360)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2365 || n = 2375)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2368)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2371)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2377)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	nop
	sbc #1
	Jne *-5
.elseif(n = 2380)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2382)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.endif
.else
; 2383 to 2421
.if(n = 2383)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2384 || n = 2410)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2385 || n = 2395)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2386 || n = 2388 || n = 2396 || n = 2398 || n = 2402 || n = 2408)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2387 || n = 2394 || n = 2407 || n = 2416 || n = 2419)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2389 || n = 2409)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2390 || n = 2412)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2391 || n = 2403 || n = 2415)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2392 || n = 2414)
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
	nop
	nop
	sbc #1
	Jne *-5
.elseif(n = 2400)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2401)
	; 9 bytes
	tya
	ldy #(n-3)/11 ;hides 'nop'
	nop
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2404)
	; 10 bytes
	pha
	sec
	lda #(n-6)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2406)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/11
	delay_n 6
	dey
	Jne *-3
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
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2420)
	; 9 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	adc #1
	Jne *-5
.elseif(n = 2421)
	; 9 bytes
	clc
	lda #36 ;hides 'bit zp'
	nop $48,X ;hides 'pha'
	adc #1
	Jne *-5
.endif
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
.elseif(n = 2423 || n = 2433 || n = 2473 || n = 2493)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2424)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	pha
	iny
	Jne *-4
	tay
.elseif((n .mod 10 = 5) && ((n > 2424 && n < 2456) || n = 2485 || n = 2495))
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2426 || n = 2431 || n = 2437 || n = 2438 || n = 2457 || n = 2460 || n = 2471 || n = 2479 || n = 2482)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 3) && (n = 2427 || (n > 2450 && n < 2500)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2428 || n = 2430 || n = 2432 || n = 2442 || n = 2444 || n = 2452 || n = 2454 || n = 2461 || n = 2466 || n = 2470 || n = 2472 || n = 2476 || n = 2484 || n = 2486 || n = 2492 || n = 2494 || n = 2496 || n = 2498)
	; 9..10 bytes
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
.elseif(n = 2434 || n = 2456 || n = 2467 || n = 2478 || n = 2500)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
.elseif(n = 2440 || n = 2443 || n = 2459)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2446)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2448)
	; 9 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	adc #1
	Jne *-5
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
.elseif(n = 2462)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2464 || n = 2474)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2468)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2481)
	; 8 bytes
	sec
	lda #248 ;hides 'sed'
	pha
	sbc #1
	Jne *-4
.elseif(n = 2483)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
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
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dey
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
.if(n = 2501)
	; 8 bytes
	sec
	lda #(n-1)/10 ;hides 'nop'
	pha
	sbc #1
	Jne *-4
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
.elseif(n = 2504 || n = 2506 || n = 2508 || n = 2516 || n = 2518 || n = 2528 || n = 2534)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2505 || n = 2515)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2507 || n = 2519 || n = 2529)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2509)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2510)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2511 || n = 2523 || n = 2535)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2512)
	; 9 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2513 || n = 2525 || n = 2537)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2514)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2517 || n = 2526 || n = 2527)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2520 || n = 2521 || n = 2530 || n = 2532)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2522)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2531)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	nop
	adc #1
	Jne *-5
.elseif(n = 2533)
	; 9 bytes
	tya
	ldy #26 ;hides 'nop'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2536)
	; 10 bytes
	pha
	sec
	lda #253
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2538)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/10
	nop
	pha
	dey
	Jne *-3
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
.if(n = 2540)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2541 || n = 2548 || n = 2550 || n = 2569 || n = 2572)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2542 || n = 2554)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2543)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
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
.elseif(n = 2551)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 4 = 0) && ((n > 2551 && n < 2565) || n = 2576))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2553)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	nop
	adc #1
	Jne *-5
.elseif(n = 2555)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	nop
	iny
	Jne *-4
	tay
.elseif(n = 2558)
	; 9 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop $48,X ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2562)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2563)
	; 9 bytes
	sec
	lda #0
	nop
	pha
	sbc #1
	Jne *-4
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
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2574)
	; 10 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2575)
	; 9 bytes
	sec
	lda #(n-1)/11 ;hides 'nop'
	nop
	nop
	sbc #1
	Jne *-5
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
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	pha
	dey
	Jne *-5
	tay
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
.elseif(n = 2580 || n = 2586 || n = 2604)
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
	ldy #(n-8)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2583 || n = 2595 || n = 2607)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2584 || n = 2608)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2585 || n = 2597 || n = 2609)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
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
.elseif(n = 2589 || n = 2594 || n = 2613)
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
.elseif(n = 2591 || n = 2602)
	; 10 bytes
	pha
	sec
	lda #(n-6)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2592 || n = 2605)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2593)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	pha
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
.elseif(n = 2616)
	; 9 bytes
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2617)
	; 9 bytes
	sec
	lda #(n-1)/12 ;hides 'nop'
	nop
	pha
	sbc #1
	Jne *-5
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2625 || n = 2635 || n = 2636 || n = 2647)
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
.elseif(n = 2620 || n = 2627 || n = 2638 || n = 2646)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2628)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2629)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2630)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2632 || n = 2654)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2637 || n = 2651)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2639 || n = 2640)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2641)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 2642)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
	tax
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
.elseif(n = 2649)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2650)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
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
	nop
	nop
	nop
	adc #1
	Jne *-6
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
.elseif(n = 2658 || n = 2659 || n = 2671 || n = 2680 || n = 2683 || n = 2695)
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
.elseif(n = 2662 || n = 2664 || n = 2666 || n = 2674)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2663 || n = 2677)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2665)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
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
	nop
	nop
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
.elseif(n = 2694)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2697 || n = 2704 || n = 2726)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2698)
	; 9 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
.elseif(n = 2699 || n = 2710)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 11 = 5) && ((n > 2699 && n < 2734)))
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2701 || n = 2734)
	; 10 bytes
	pha
	sec
	lda #(n-6)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2702 || n = 2713 || n = 2719 || n = 2724)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2706 || n = 2708 || n = 2712 || n = 2714 || n = 2723 || n = 2725 || n = 2730 || n = 2732)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2707 || n = 2721)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2709 || n = 2720)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2716)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2718)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2728)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	nop
	dey
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
.elseif(n = 2736)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2737 || n = 2745)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2738)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	nop
	nop
	nop
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
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2743)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2744)
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
.elseif(n = 2747)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
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
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2752)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	nop
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
	lda #(n-1)/13 ;hides 'nop zp,x'
	nop
	nop
	nop
	sbc #1
	Jne *-6
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
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2760)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2761)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	pha
	adc #1
	Jne *-5
.elseif(n = 2762)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 2763)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2764)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2765)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2767 || n = 2768)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2769)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2770)
	; 10 bytes
	pla
	delay_n (n-4)
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
	nop
	nop
	nop
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
.if(n = 2774 || n = 2800)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
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
.elseif((n > 2781 && n < 2785) || n = 2794 || n = 2796)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2785)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	pha
	adc #1
	Jne *-5
.elseif(n = 2786 || n = 2797 || n = 2808)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2788 || n = 2810)
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
.elseif(n = 2792 || n = 2804 || n = 2806)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2798)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2802)
	; 10 bytes
	pha
	tya
	ldy #(n-8)/11
	delay_n 6
	dey
	Jne *-3
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
	; 9 bytes
	sec
	lda #(n-1)/12 ;hides 'nop'
	nop
	pha
	sbc #1
	Jne *-5
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
.elseif((n .mod 12 = 7) && ((n > 2814 && n < 2852)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828 || n = 2836 || n = 2848)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2817)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2818 || n = 2830 || (n > 2841 && n < 2845) || n = 2846)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2819)
	; 9 bytes
	sec
	lda #0
	delay_n 6
	sbc #1
	Jne *-4
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
	; 9 bytes
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2822)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 2823 || n = 2835 || n = 2847)
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
	clc
	lda #20 ;hides 'nop zp,x'
	bit $48 ;hides 'pha'
	adc #1
	Jne *-5
.elseif(n = 2832)
	; 9 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	adc #1
	Jne *-5
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
.elseif(n = 2838)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2840)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
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
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2853 || n = 2877)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2854 || n = 2866 || n = 2869 || n = 2872 || (n > 2878 && n < 2883) || n = 2884 || n = 2890)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2855 || n = 2870)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2856)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	adc #1
	Jne *-6
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
.elseif(n = 2859 || n = 2871 || n = 2883)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2860)
	; 10 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	adc #1
	Jne *-6
.elseif(n = 2861 || n = 2885)
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
.elseif(n = 2863 || n = 2875 || n = 2887)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2864 || n = 2874 || n = 2876 || n = 2886 || n = 2888)
	; 10 bytes
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
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2873)
	; 9 bytes
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2889)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif((n > 2891 && n < 2895) || n = 2896 || n = 2905 || n = 2906 || n = 2908 || n = 2914 || n = 2918 || n = 2920 || n = 2926 || n = 2938 || (n > 2949 && n < 2954) || (n > 2961 && n < 2966))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 2898 || n = 2958)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif((n .mod 12 = 7) && ((n > 2898 && n < 2948)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2900 || n = 2960)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2901 || n = 2915)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2902 || n = 2941 || n = 2954)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
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
.elseif(n = 2910 || n = 2912 || n = 2922 || n = 2924 || n = 2934 || n = 2936 || n = 2946 || n = 2948)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 9) && ((n > 2912 && n < 2962)))
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2916)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2927)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	sbc #1
	Jne *-5
.elseif(n = 2928)
	; 9 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	sbc #1
	Jne *-5
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
.elseif(n = 2932)
	; 10 bytes
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2939)
	; 10 bytes
	sec
	lda #226 ;hides 'nop #imm'
	bit $48 ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 2940)
	; 10 bytes
	sec
	lda #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 2942)
	; 10 bytes
	tya
	ldy #226 ;hides 'nop #imm'
	nop $48,X ;hides 'pha'
	pha
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
.elseif(n = 2966)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	nop
	dey
	Jne *-5
	tay
.elseif(n = 2968)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	nop
	pha
	dey
	Jne *-5
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
.elseif(n = 2970 || n = 2972 || n = 2984 || n = 2994 || n = 2996)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3008)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2973 || n = 2987)
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
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2976 || n = 2978 || n = 2986 || n = 2989 || n = 2998 || n = 3000 || n = 3002)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2977)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	pha
	sbc #1
	Jne *-5
.elseif(n = 2979 || n = 2991 || n = 3003)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2980)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2982)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2985 || n = 2999)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2988)
	; 10 bytes
	tya
	ldy #200 ;hides 'iny'
	delay_n 6
	dey
	dey
	Jne *-5
	tay
.elseif(n = 2992)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2997)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3001)
	; 9 bytes
	sec
	lda #(n-1)/12 ;hides 'nop'
	nop
	pha
	sbc #1
	Jne *-5
.elseif(n = 3004)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $06A0'
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
.elseif((n > 3009 && n < 3013) || n = 3014 || n = 3024 || n = 3026 || (n > 3035 && n < 3039))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3013)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3015 || n = 3027 || n = 3039)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 3016)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $05A0'
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
.elseif(n = 3018)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
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
.elseif(n = 3022 || n = 3030 || n = 3032 || n = 3042 || n = 3044 || n = 3046)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3025)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3028)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3031)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3040)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3043)
	; 9 bytes
	sec
	lda #234 ;hides 'nop'
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 3049 || n = 3052 || n = 3059 || n = 3081)
	; 10 bytes
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
.elseif(n = 3051 || n = 3063)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 3053 || n = 3065)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 3054 || n = 3056 || n = 3066 || n = 3078 || n = 3080)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3064)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3067 || n = 3079)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3068)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
.elseif(n = 3069 || n = 3083)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3070)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 3072 || n = 3074)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3075)
	; 9 bytes
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 3076)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $00A0'
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
.elseif(n = 3082)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
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
.if(n = 3086 || n = 3112)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n > 3086 && n < 3093) || n = 3094 || n = 3096 || (n > 3099 && n < 3108) || n = 3109 || (n > 3113 && n < 3123) || n = 3124)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3093 || n = 3108)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3095 || n = 3110)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3097 || n = 3111 || n = 3125)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3123)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
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
.if(n = 3126 || n = 3128 || n = 3135 || n = 3137 || (n > 3141 && n < 3149) || n = 3150 || n = 3152 || n = 3154 || (n > 3155 && n < 3162) || n = 3163)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3127 || n = 3141 || n = 3155)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3129)
	; 10 bytes
	sec
	lda #184 ;hides 'clv'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 3130)
	; 9 bytes
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3131)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3132)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3133)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3134)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3136 || n = 3149)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3138 || n = 3151 || n = 3164)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3139 || n = 3153)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3140)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
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
.if(n = 3165 || n = 3166 || n = 3176 || n = 3178 || n = 3184 || n = 3186 || n = 3189 || (n > 3190 && n < 3194) || n = 3199 || n = 3202)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3167 || n = 3181 || n = 3195)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3168)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
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
	lda #(n-1)/13 ;hides 'nop zp,x'
	nop
	nop
	nop
	sbc #1
	Jne *-6
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
.elseif(n = 3177 || n = 3190)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3179)
	; 10 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	bit $48 ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 3180)
	; 10 bytes
	sec
	lda #n/15 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 3182)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
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
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3194)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3196)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3198)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3201)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
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
; 3204 to 3281
.if((n > 3203 && n < 3209) || n = 3212 || (n > 3216 && n < 3221) || n = 3222 || (n > 3231 && n < 3237) || n = 3238 || n = 3244 || (n > 3245 && n < 3249) || n = 3250 || n = 3259 || (n > 3260 && n < 3265) || n = 3269 || n = 3270 || n = 3272 || n = 3274 || n = 3276 || n = 3278)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3209 || n = 3237 || n = 3265)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3210)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3211 || n = 3239 || n = 3267 || n = 3281)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3213)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3214 || n = 3240 || n = 3266)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif((n .mod 15 = 5) && ((n > 3214 && n < 3276)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif((n .mod 13 = 5) && ((n > 3215 && n < 3269)))
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
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
.elseif(n = 3224 || n = 3226 || n = 3252 || n = 3254 || n = 3256 || n = 3280)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3225)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3227)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 15 = 3) && ((n > 3227 && n < 3274)))
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3231 || n = 3257)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3241)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	delay_n 6
	sbc #1
	Jne *-6
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
.elseif(n = 3253)
	; 9 bytes
	tya
	ldy #(n-3)/13 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3271)
	; 10 bytes
	sec
	lda #(n-1)/15 ;hides 'nop'
	nop
	delay_n 6
	sbc #1
	Jne *-6
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
.if((n >= 3282) && (n < 3360))
; 3282 to 3359
.if(n = 3282)
	; 10 bytes
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
.elseif((n > 3283 && n < 3288) || n = 3291 || (n > 3295 && n < 3302) || n = 3308 || (n > 3309 && n < 3318) || n = 3322 || (n > 3323 && n < 3331) || n = 3332 || n = 3334 || n = 3336 || (n > 3337 && n < 3348) || (n > 3351 && n < 3360))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3288 || n = 3303 || n = 3348)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3289)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3290 || n = 3305 || n = 3350)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3292 || n = 3318)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 3) && ((n > 3292 && n < 3350)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3294 || n = 3320)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 14 = 5) && ((n > 3294 && n < 3352)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3306)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
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
.elseif(n = 3331)
	; 10 bytes
	sec
	lda #0
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3333)
	; 10 bytes
	tya
	ldy #0
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.endif
.else
; 3360 to 3437
.if((n > 3359 && n < 3363) || n = 3364 || n = 3366 || n = 3368 || (n > 3369 && n < 3377) || (n > 3380 && n < 3387) || n = 3388 || n = 3392 || n = 3394 || (n > 3395 && n < 3403) || n = 3404 || n = 3406 || (n > 3411 && n < 3415) || n = 3420 || n = 3422 || n = 3426 || (n > 3427 && n < 3433) || n = 3434 || n = 3436 || n = 3437)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3363 || n = 3377 || n = 3405 || n = 3419 || n = 3433)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3364 && n < 3436)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3367)
	; 10 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 10
	adc #1
	Jne *-6
.elseif(n = 3369)
	; 10 bytes
	tya
	ldy #58 ;hides 'nop'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3378 || n = 3408 || n = 3423)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3380 || n = 3410 || n = 3425)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
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
	pha
	delay_n (n-3)
.elseif(n = 3391)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3395)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3403)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	delay_n 10
	iny
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
.elseif(n = 3415)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	nop
	sbc #1
	Jne *-6
.elseif(n = 3416)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
	sbc #1
	Jne *-6
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
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	pha
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
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if(n = 3438 || n = 3453 || n = 3498)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3439)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif((n .mod 15 = 5) && ((n > 3439 && n < 3516)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif((n > 3440 && n < 3447) || n = 3448 || n = 3450 || n = 3452 || n = 3456 || n = 3458 || n = 3460 || n = 3462 || (n > 3463 && n < 3468) || n = 3469 || n = 3471 || n = 3472 || n = 3474 || n = 3480 || n = 3482 || n = 3484 || (n > 3485 && n < 3489) || n = 3490 || (n > 3491 && n < 3498) || n = 3499 || n = 3502 || (n > 3507 && n < 3511) || n = 3512 || n = 3514)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3447 || n = 3461 || n = 3489)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3448 && n < 3506)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3451)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	nop
	delay_n 6
	adc #1
	Jne *-6
.elseif(n = 3454)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 pha
	.endrepeat
	Jne *-5
	tax
.elseif(n = 3457)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 3459)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
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
.elseif(n = 3473)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3475)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
.elseif(n = 3476 || n = 3478 || n = 3504 || n = 3506)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479 || n = 3507)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3481)
	; 10 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	delay_n 6
	adc #1
	Jne *-6
.elseif(n = 3483)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	nop
	delay_n 6
	iny
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
.elseif(n = 3511)
	; 10 bytes
	sec
	lda #(n-1)/15 ;hides 'nop'
	nop
	delay_n 6
	sbc #1
	Jne *-6
.elseif(n = 3513)
	; 10 bytes
	tya
	ldy #234 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3518 || (n > 3519 && n < 3528) || n = 3529 || n = 3532 || (n > 3533 && n < 3539) || n = 3544 || n = 3546 || (n > 3547 && n < 3558) || (n > 3561 && n < 3573) || n = 3574 || (n > 3575 && n < 3587) || (n > 3590 && n < 3594))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 14 = 3) && ((n > 3516 && n < 3574)))
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif((n .mod 14 = 5) && ((n > 3518 && n < 3576)))
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3528 || n = 3543 || n = 3558 || n = 3588)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3530 || n = 3560 || n = 3590)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3539)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $48 ;hides 'pha'
	pha
	adc #1
	Jne *-6
.elseif(n = 3540)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	adc #1
	Jne *-6
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
.endif
.else
; 3594 to 3750
.if((n > 3593 && n < 3603) || n = 3604 || (n > 3605 && n < 3618) || n = 3619 || (n > 3620 && n < 3633) || n = 3634 || (n > 3635 && n < 3644) || n = 3645 || n = 3649 || (n > 3650 && n < 3657) || n = 3658 || n = 3664 || (n > 3665 && n < 3673) || n = 3674 || n = 3676 || n = 3677 || n = 3679 || n = 3682 || (n > 3683 && n < 3693) || n = 3694 || (n > 3695 && n < 3699) || (n > 3699 && n < 3707) || n = 3711 || n = 3712 || n = 3714 || (n > 3715 && n < 3721) || n = 3722 || n = 3724 || (n > 3725 && n < 3738) || n = 3739 || (n > 3740 && n < 3745) || n = 3746 || (n > 3747 && n < 3751))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 15 = 3) && ((n > 3602 && n < 3634) || (n > 3662 && n < 3739)))
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif((n .mod 15 = 5) && ((n > 3604 && n < 3741)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
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
.elseif(n = 3647)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3648)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3657)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	tax
.elseif(n = 3659)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 3660)
	; 10 bytes
	sec
	lda #n/15 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 3661)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3662)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3673)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 3675)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 3681)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3683)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
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
.elseif(n = 3707)
	; 10 bytes
	sec
	lda #(n-1)/17 ;hides 'nop'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 3709)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
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
.elseif(n = 3721)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	delay_n 6
	sbc #1
	Jne *-6
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
	ldy #(n-3)/16 ;hides 'nop'
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
.if(n = 3751)
	; 10 bytes
	sec
	lda #(n-1)/15 ;hides 'nop'
	nop
	delay_n 6
	sbc #1
	Jne *-6
.elseif(n = 3752 || n = 3754 || (n > 3755 && n < 3763) || (n > 3763 && n < 3768) || n = 3769 || (n > 3770 && n < 3777) || n = 3778 || (n > 3779 && n < 3783) || n = 3784 || (n > 3785 && n < 3795) || n = 3796 || n = 3797 || n = 3799 || (n > 3800 && n < 3813) || n = 3814 || (n > 3815 && n < 3828) || n = 3829 || (n > 3830 && n < 3843) || n = 3844 || (n > 3845 && n < 3860) || (n > 3860 && n < 3891) || (n > 3891 && n < 3901) || n = 3902 || n = 3906)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif((n .mod 15 = 3) && ((n > 3752 && n < 3829)))
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif((n .mod 15 = 5) && ((n > 3754 && n < 3831)))
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
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
.elseif(n = 3777)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
.elseif(n = 3779)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3795)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 3843)
	; 10 bytes
	sec
	lda #0
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3845)
	; 10 bytes
	tya
	ldy #0
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3860)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
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
.elseif(n = 3903)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3904)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3905)
	; 10 bytes
	pla
	delay_n (n-4)
.endif
.else
; 3907 to 4062
.if(n = 3907)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif((n > 3907 && n < 3911) || n = 3912 || (n > 3913 && n < 3923) || (n > 3923 && n < 3945) || n = 3946 || (n > 3947 && n < 3969) || n = 3970 || (n > 3971 && n < 3979) || n = 3980 || (n > 3981 && n < 4001) || n = 4002 || (n > 4003 && n < 4063))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3911)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 10
	adc #1
	Jne *-6
.elseif(n = 3913)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3923)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 3945)
	; 10 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 10
	adc #1
	Jne *-6
.elseif(n = 3947)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-5
	tay
.elseif(n = 3969)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 3971)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3979)
	; 10 bytes
	sec
	lda #(n-1)/17 ;hides 'nop'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 3981)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
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
.elseif(n = 4003)
	; 10 bytes
	tya
	ldy #(n-3)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 4063 to 4375
.if((n > 4062 && n < 4158) || n = 4159 || (n > 4162 && n < 4217) || n = 4218 || (n > 4219 && n < 4251) || n = 4252 || (n > 4253 && n < 4376))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4158)
	; 9 bytes
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4160)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4161)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4162)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4217)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 4219)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 10
	dey
	Jne *-5
	tay
.elseif(n = 4251)
	; 10 bytes
	sec
	lda #(n-1)/17 ;hides 'nop'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 4253)
	; 10 bytes
	tya
	ldy #(n-3)/17 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	tay
.endif
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4415) || n = 4416 || (n > 4419 && n < 4672) || n = 4673 || (n > 4676 && n < 4929) || n = 4930 || (n > 4933 && n < 5001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 4418 || n = 4675 || n = 4932)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4419 || n = 4676 || n = 4933)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4672)
	; 9 bytes
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
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
; 5001 to 10000
.if((n > 5000 && n < 5186) || n = 5187 || (n > 5190 && n < 5443) || n = 5444 || (n > 5447 && n < 5700) || n = 5701 || (n > 5704 && n < 5957) || n = 5958 || (n > 5961 && n < 6214) || n = 6215 || (n > 6218 && n < 6471) || n = 6472 || (n > 6475 && n < 6728) || n = 6729 || (n > 6732 && n < 6985) || n = 6986 || (n > 6989 && n < 7242) || n = 7243 || (n > 7246 && n < 7499) || n = 7500 || (n > 7503 && n < 7756) || n = 7757 || (n > 7760 && n < 8013) || n = 8014 || (n > 8017 && n < 8270) || n = 8271 || (n > 8274 && n < 8527) || n = 8528 || (n > 8531 && n < 8784) || n = 8785 || (n > 8788 && n < 9041) || n = 9042 || (n > 9045 && n < 9298) || n = 9299 || (n > 9302 && n < 9555) || n = 9556 || (n > 9559 && n < 9812) || n = 9813 || (n > 9816 && n < 10001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5186)
	; 9 bytes
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5188 || n = 5445 || n = 5702 || n = 5959 || n = 6216 || n = 6473 || n = 6730 || n = 6987 || n = 7244 || n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5189 || n = 5446 || n = 5703 || n = 5960 || n = 6217 || n = 6474 || n = 6731 || n = 6988 || n = 7245 || n = 7502 || n = 7759 || n = 8016 || n = 8273 || n = 8530 || n = 8787 || n = 9044 || n = 9301 || n = 9558 || n = 9815)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5190 || n = 5447 || n = 5704 || n = 5961 || n = 6218 || n = 6475 || n = 6732 || n = 6989 || n = 7246 || n = 7503 || n = 7760 || n = 8017 || n = 8274 || n = 8531 || n = 8788 || n = 9045 || n = 9302 || n = 9559 || n = 9816)
	; 10 bytes
	pla
	delay_n (n-4)
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
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10069) || n = 10070 || (n > 10073 && n < 10326) || n = 10327 || (n > 10330 && n < 10583) || n = 10584 || (n > 10587 && n < 10840) || n = 10841 || (n > 10844 && n < 11097) || n = 11098 || (n > 11101 && n < 11354) || n = 11355 || (n > 11358 && n < 11611) || n = 11612 || (n > 11615 && n < 11868) || n = 11869 || (n > 11872 && n < 12125) || n = 12126 || (n > 12129 && n < 12382) || n = 12383 || (n > 12386 && n < 12639) || n = 12640 || (n > 12643 && n < 12896) || n = 12897 || (n > 12900 && n < 13153) || n = 13154 || (n > 13157 && n < 13410) || n = 13411 || (n > 13414 && n < 13667) || n = 13668 || (n > 13671 && n < 13924) || n = 13925 || (n > 13928 && n < 14181) || n = 14182 || (n > 14185 && n < 14438) || n = 14439 || (n > 14442 && n < 14695) || n = 14696 || (n > 14699 && n < 14952) || n = 14953 || (n > 14956 && n < 15001))
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
.elseif(n = 10072 || n = 10329 || n = 10586 || n = 10843 || n = 11100 || n = 11357 || n = 11614 || n = 11871 || n = 12128 || n = 12385 || n = 12642 || n = 12899 || n = 13156 || n = 13413 || n = 13670 || n = 13927 || n = 14184 || n = 14441 || n = 14698 || n = 14955)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10073 || n = 10330 || n = 10587 || n = 10844 || n = 11101 || n = 11358 || n = 11615 || n = 11872 || n = 12129 || n = 12386 || n = 12643 || n = 12900 || n = 13157 || n = 13414 || n = 13671 || n = 13928 || n = 14185 || n = 14442 || n = 14699 || n = 14956)
	; 10 bytes
	pla
	delay_n (n-4)
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
.if((n > 15000 && n < 15209) || n = 15210 || (n > 15213 && n < 15466) || n = 15467 || (n > 15470 && n < 15723) || n = 15724 || (n > 15727 && n < 15980) || n = 15981 || (n > 15984 && n < 16237) || n = 16238 || (n > 16241 && n < 16494) || n = 16495 || (n > 16498 && n < 16751) || n = 16752 || (n > 16755 && n < 17008) || n = 17009 || (n > 17012 && n < 17265) || n = 17266 || (n > 17269 && n < 17522) || n = 17523 || (n > 17526 && n < 17779) || n = 17780 || (n > 17783 && n < 18036) || n = 18037 || (n > 18040 && n < 18293) || n = 18294 || (n > 18297 && n < 18550) || n = 18551 || (n > 18554 && n < 18807) || n = 18808 || (n > 18811 && n < 19064) || n = 19065 || (n > 19068 && n < 19321) || n = 19322 || (n > 19325 && n < 19578) || n = 19579 || (n > 19582 && n < 19835) || n = 19836 || (n > 19839 && n < 20001))
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
.elseif(n = 15212 || n = 15469 || n = 15726 || n = 15983 || n = 16240 || n = 16497 || n = 16754 || n = 17011 || n = 17268 || n = 17525 || n = 17782 || n = 18039 || n = 18296 || n = 18553 || n = 18810 || n = 19067 || n = 19324 || n = 19581 || n = 19838)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15213 || n = 15470 || n = 15727 || n = 15984 || n = 16241 || n = 16498 || n = 16755 || n = 17012 || n = 17269 || n = 17526 || n = 17783 || n = 18040 || n = 18297 || n = 18554 || n = 18811 || n = 19068 || n = 19325 || n = 19582 || n = 19839)
	; 10 bytes
	pla
	delay_n (n-4)
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
