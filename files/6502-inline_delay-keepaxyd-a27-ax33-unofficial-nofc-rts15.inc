; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Z+N, C, and V
; Will preserve A, X, Y, S, D, and I (interrupt-disable flag)
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
;
; Requires that a label @nearby_rts_15cyc is defined,
; that points _somewhere_ that contains a JMP to a location that does RTS.
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
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 22
; - 7 bytes or less when n <= 33
; - 8 bytes or less when n <= 293
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 320
; -11 bytes or less when n <= 1303
; -12 bytes or less when n <= 1326
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=3, avg=4.4, max=6
; - When 21 <= n <= 30: min=5, avg=6.4, max=7
; - When 31 <= n <= 50: min=6, avg=7.0, max=8
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=10
; - When 301 <= n <= 500: min=8, avg=9.7, max=11
; - When 501 <= n <= 750: min=8, avg=9.8, max=11
; - When 751 <= n <= 1000: min=9, avg=9.8, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.7, max=13
; - When 2001 <= n <= 3000: min=10, avg=12.0, max=13
; - When 3001 <= n <= 5000: min=11, avg=12.7, max=13
; - When 5001 <= n <= 7500: min=11, avg=13.0, max=13
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
; 1 to 156
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	cmp $C5
.elseif((n > 3 && n < 7) || n = 17 || n = 20 || n = 26 || n = 29 || n = 32)
	; 2..7 bytes
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
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16)
	; 5 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 18 || n = 25 || n = 27 || n = 30)
	; 5..7 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 28)
	; 7 bytes
	pha
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
.elseif(n = 31)
	; 6 bytes
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif(n = 33)
	; 7 bytes
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif(n = 34)
	; 8 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 35)
	; 8 bytes
	pha
	clc
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
.elseif((n > 35 && n < 46) || (n > 46 && n < 157))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 46)
	; 6 bytes
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
.endif
.else
; 157 to 312
.if((n > 156 && n < 292))
	; 7 bytes
	pha
	lda #n-36
	jsr delay_a_27_clocks
	pla
.elseif(n = 292 || n = 293 || n = 299 || n = 309)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 294 || (n > 295 && n < 299))
	; 9 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 295)
	; 8 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
.elseif(n = 300 || n = 310)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 301 || n = 303)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 302 || n = 307)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 304 || n = 306)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 305)
	; 8 bytes
	pha
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
	pla
.elseif(n = 308)
	; 10 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 311)
	; 9 bytes
	pha
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	nop
	Jne *-3
	pla
.elseif(n = 312)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 369 || n = 376)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 314 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 339 || n = 343 || n = 344 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 364 || n = 366 || n = 374 || n = 378 || n = 379 || n = 384 || n = 386 || n = 389)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 335 || n = 340 || n = 350 || n = 355 || n = 365 || n = 370 || n = 375 || n = 380)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 316)
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	pla
.elseif(n = 317 || n = 322 || n = 327 || n = 352 || n = 357 || n = 372)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 318)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 320 || n = 325 || n = 330 || n = 345 || n = 360 || n = 385)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 323 || n = 328 || n = 333 || n = 358 || n = 368 || n = 373)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif((n .mod 5 = 2) && ((n > 331 && n < 348) || n = 362 || n = 367 || (n > 376 && n < 388)))
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 334 || n = 341 || n = 348 || n = 383)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 336 || n = 371)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 338)
	; 11 bytes
	pha
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
.elseif(n = 346)
	; 9 bytes
	pha
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	pla
.elseif(n = 353)
	; 9 bytes
	pha
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
	pla
.elseif(n = 361)
	; 10 bytes
	pha
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	pla
.elseif(n = 363)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 381)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.endif
.else
; 391 to 468
.if(n = 391 || n = 394 || n = 396 || n = 399 || n = 401 || n = 413 || n = 419 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 444 || n = 449 || n = 454 || n = 459 || n = 461 || n = 464)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 392 || n = 407 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 393)
	; 10 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
.elseif((n .mod 5 = 0) && ((n > 394 && n < 426) || (n > 439 && n < 456) || n = 465))
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif((n .mod 5 = 2) && (n = 397 || n = 402 || n = 412 || n = 427 || (n > 436 && n < 458)))
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 398 || n = 403 || n = 433 || n = 438 || n = 443 || n = 463)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 404 || n = 418)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
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
.elseif(n = 408)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
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
.elseif(n = 411 || n = 439 || n = 446 || n = 453)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 414)
	; 9 bytes
	pha
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
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
.elseif(n = 421)
	; 9 bytes
	pha
	sec
	lda #58 ;hides 'nop'
	sbc #1
	Jpl *-3
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
.elseif(n = 430 || n = 435 || n = 460)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 441)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 448)
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
	pla
.elseif(n = 451 || n = 458)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 456)
	; 10 bytes
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	delay_n 9
	Jne *-5
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
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jcs *-5
	pla
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 518 || n = 519 || n = 524 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 546)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 485 || n = 495 || n = 500 || n = 505 || n = 520 || n = 525 || n = 535 || n = 540)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 472 || n = 482 || n = 497 || n = 507 || n = 527)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 473 || n = 503 || n = 513 || n = 533)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 474 || n = 481 || n = 509 || n = 523)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 475 || n = 480 || n = 490 || n = 510 || n = 515 || n = 530 || n = 545)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
.elseif(n = 476)
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
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
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 486 || n = 493 || n = 521 || n = 528)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 488 || n = 516 || n = 544)
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
.elseif(n = 498)
	; 9 bytes
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jpl *-3
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
.elseif(n = 511)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 514)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	pla
.elseif(n = 526)
	; 10 bytes
	pha
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
	pla
.elseif(n = 538)
	; 11 bytes
	pha
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
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
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 549 || n = 554 || n = 559 || n = 564 || n = 566 || n = 569 || n = 573 || n = 574 || n = 581 || n = 584 || n = 589 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 616 || n = 619 || n = 623 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 549 && n < 561) || n = 575 || (n > 589 && n < 601) || (n > 609 && n < 626)))
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
	pla
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
.elseif(n = 553)
	; 10 bytes
	pha
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 556 || n = 591)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 561)
	; 10 bytes
	pha
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jpl *-4
	pla
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
.elseif(n = 565 || n = 570 || n = 580 || n = 585 || n = 605)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
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
.elseif(n = 577 || n = 582 || n = 587 || n = 592 || n = 602 || n = 622)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 578)
	; 11 bytes
	pha
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
.elseif(n = 579 || n = 586)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 583 || n = 603 || n = 608)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
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
.elseif(n = 598)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.if(n = 626 || n = 633 || n = 661 || n = 668)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 627 || n = 632 || n = 637 || n = 647 || n = 652 || n = 657)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 628 || n = 656 || n = 691)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 686 || n = 689 || n = 693 || n = 694 || n = 699 || n = 701)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 630 || n = 635 || n = 640 || n = 645 || n = 655)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
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
	; 9 bytes
	pha
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
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
.elseif(n = 643 || n = 653 || n = 673 || n = 683 || n = 688)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 648)
	; 10 bytes
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
	pla
.elseif(n = 649 || n = 663 || n = 684 || n = 698)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 5 = 0) && (n = 650 || (n > 659 && n < 691) || n = 700))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
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
.elseif(n = 695)
	; 8 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	pla
.elseif(n = 696 || n = 703)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.endif
.else
; 704 to 781
.if(n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 728 || n = 729 || n = 734 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 763 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 704 && n < 781)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
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
.elseif(n = 713 || n = 753 || n = 758)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 718)
	; 10 bytes
	pha
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
	pla
.elseif(n = 719 || n = 726 || n = 754 || n = 761 || n = 768)
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
.elseif(n = 723 || n = 741)
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
.elseif(n = 731)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 733)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 736)
	; 9 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jpl *-3
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
	delay_n (n-7)
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
.elseif(n = 766 || n = 773)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 778)
	; 11 bytes
	pha
	sec
	lda #(n-18)/8
	bit $24
	sbc #1
	Jpl *-4
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
.elseif(n = 783 || n = 823 || n = 848 || n = 853 || n = 863 || n = 883 || n = 893 || n = 918 || n = 923 || n = 933)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 784 || n = 794 || n = 798 || n = 799 || n = 804 || n = 806 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 833 || n = 834 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 868 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 936)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
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
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 789 || n = 796 || n = 831 || n = 859 || n = 894 || n = 929 || n = 936)
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
.elseif(n = 801 || n = 836 || n = 878 || n = 913)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 803 || n = 824 || n = 838 || n = 866 || n = 873 || n = 901 || n = 908)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 808 || n = 843 || n = 906)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 811)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jmi *-3
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
.elseif(n = 818)
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 864)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 869)
	; 9 bytes
	pha
	sec
	lda #250 ;hides 'nop'
	sbc #1
	Jmi *-3
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
.elseif(n = 898)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 926)
	; 10 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jpl *-4
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
.if(n = 938 || n = 939 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1031 || n = 1039 || n = 1043 || n = 1044 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1064 || n = 1066 || n = 1074 || n = 1078 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1021) || (n > 1029 && n < 1091)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 941 || n = 976 || n = 983 || n = 1011 || n = 1018 || n = 1046 || n = 1053 || n = 1088)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1093)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && (n = 943 || (n > 963 && n < 979) || (n > 998 && n < 1014) || (n > 1033 && n < 1049) || (n > 1068 && n < 1084)))
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
	; 9 bytes
	pha
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
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
.elseif(n = 953 || n = 958 || n = 963 || n = 968 || n = 998 || n = 1003 || n = 1023 || n = 1033 || n = 1073 || n = 1093)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
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
.elseif(n = 1025)
	; 8 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
.elseif(n = 1028)
	; 10 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
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
.elseif(n = 1058)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1061)
	; 10 bytes
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	sbc #1
	Jmi *-4
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
.if(n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1109 || n = 1113 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1148 || n = 1149 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1179 || n = 1183 || n = 1184 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1251)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif((n .mod 5 = 2) && ((n > 1096 && n < 1248)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1098 || n = 1138 || n = 1178)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 5 = 3) && (n = 1103 || n = 1133 || n = 1143 || n = 1163 || n = 1168 || (n > 1197 && n < 1214) || n = 1233 || n = 1238))
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif((n .mod 7 = 5) && ((n > 1103 && n < 1119) || (n > 1138 && n < 1154) || (n > 1173 && n < 1189) || (n > 1208 && n < 1224) || n = 1244))
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
.elseif(n = 1116 || n = 1123 || n = 1151 || n = 1158 || n = 1186 || n = 1193 || n = 1221 || n = 1228)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1141 || n = 1211 || n = 1246)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1176 || n = 1248)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
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
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || (n > 1278 && n < 1329)))
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
.elseif(n = 1253 || n = 1254 || n = 1259 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1284 || n = 1288 || n = 1289 || n = 1294 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1316 || n = 1323 || n = 1325)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1254 && n < 1286)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif((n .mod 7 = 3) && (n = 1256 || n = 1263 || n = 1291 || (n > 1304 && n < 1327)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
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
.elseif(n = 1268)
	; 10 bytes
	pha
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
.elseif(n = 1273 || n = 1278 || n = 1283)
	; 11 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1281)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1290)
	; 9 bytes
	pha
	sec
	lda #0
	sbc #1
	Jne *-2
	pla
.elseif(n = 1292)
	; 9 bytes
	pha
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1295)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
.elseif(n = 1296)
	; 9 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1297 || n = 1299 || n = 1303)
	; 11 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1301)
	; 11 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 1306 || n = 1322)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1308 || n = 1317)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1310)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1313)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 1315)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1318)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
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
.elseif(n = 1324)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1327)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1336 || n = 1357)
	; 12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1330 || n = 1334 || n = 1337 || n = 1339 || n = 1351 || n = 1355 || n = 1358 || n = 1365 || n = 1367)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1345 || n = 1359)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
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
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1338)
	; 11 bytes
	pha
	txa
	pha
	lax #5
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
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
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1348 || n = 1364)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
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
	ldy #(n-10)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1360)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1366)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
.endif
.else
; 1368 to 1406
.if((n .mod 7 = 3) && ((n > 1367 && n < 1390) || n = 1403))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1369)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1371 || n = 1380)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1372 || n = 1374 || n = 1379 || n = 1381 || n = 1386 || n = 1393 || n = 1395 || n = 1400 || n = 1406)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1373)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1376)
	; 12 bytes
	pha
	tya
	ldy #123
	delay_n 6
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1378 || n = 1402)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1383)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1390)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1392 || n = 1404)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1394)
	; 9 bytes
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jne *-3
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
.elseif(n = 1397 || n = 1399)
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1401)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if((n .mod 9 = 3) && ((n > 1406 && n < 1444) || n = 1470 || n = 1479))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 1) && ((n > 1407 && n < 1430) || n = 1457 || n = 1471 || n = 1478))
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1409 || n = 1430 || n = 1472)
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
.elseif(n = 1411)
	; 12 bytes
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jpl *-6
	pla
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1413)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	pla
.elseif(n = 1414 || n = 1421 || n = 1427 || n = 1435 || n = 1437 || n = 1442 || n = 1446 || n = 1449 || n = 1456 || n = 1463 || n = 1465 || n = 1477 || n = 1481 || n = 1484)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && (n = 1417 || n = 1424 || (n > 1437 && n < 1481)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1418 || n = 1450 || n = 1458 || n = 1474)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1420 || n = 1436 || n = 1444 || n = 1460)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1423 || n = 1432 || n = 1441)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1428 || n = 1476)
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
.elseif(n = 1439)
	; 12 bytes
	pha
	sec
	lda #(n-9)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
.elseif(n = 1448 || n = 1455 || n = 1469 || n = 1483)
	; 12 bytes
	delay_n (n-3)
	cmp $C5
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
.elseif(n = 1453)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1462)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
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
.elseif(n = 1467)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.else
; 1485 to 1562
.if(n = 1485 || n = 1499 || n = 1520 || n = 1555)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1486 || n = 1495)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1530) || (n > 1542 && n < 1558)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif((n .mod 9 = 3) && ((n > 1487 && n < 1507) || (n > 1532 && n < 1561)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1490 || n = 1514 || n = 1530 || n = 1546 || n = 1562)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1491 || n = 1493 || n = 1498 || n = 1505 || n = 1509 || n = 1519 || n = 1521 || n = 1525 || n = 1526 || n = 1528 || n = 1540 || n = 1544 || n = 1547 || n = 1553 || n = 1554 || n = 1561)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1492 || n = 1516 || n = 1532)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1500 || n = 1512 || n = 1548)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1502)
	; 10 bytes
	pha
	clc
	lda #90 ;hides 'nop'
	nop
	adc #1
	Jne *-4
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
.elseif(n = 1511)
	; 12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1513)
	; 10 bytes
	pha
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
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
.elseif(n = 1534)
	; 9 bytes
	pha
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
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
.elseif(n = 1537 || n = 1539)
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1541)
	; 11 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1556)
	; 10 bytes
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
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
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1568 || n = 1575 || n = 1577 || n = 1581 || n = 1582 || n = 1589 || n = 1591 || n = 1597 || n = 1603 || n = 1610 || n = 1616 || n = 1617 || n = 1619 || n = 1624 || n = 1631 || n = 1633 || n = 1638 || n = 1640)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1614) || n = 1627))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1637)))
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
.elseif((n .mod 9 = 3) && (n = 1569 || (n > 1595 && n < 1624)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1570 || n = 1586 || n = 1602 || n = 1626)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1574 || n = 1607)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1576 || n = 1630)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1583 || n = 1590 || n = 1611)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1588 || n = 1604 || n = 1612 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1593)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 1598)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1600)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 1609 || n = 1635)
	; 11..12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1618)
	; 9 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	adc #1
	Jne *-3
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
.elseif(n = 1625 || n = 1639)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1632)
	; 9 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
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
.elseif(n = 1637)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
	pla
.endif
.else
; 1641 to 1718
.if((n .mod 7 = 3) && (n = 1641 || (n > 1654 && n < 1719)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1642 || n = 1658 || n = 1674 || n = 1682 || n = 1698 || n = 1714)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1645 || n = 1647 || n = 1652 || n = 1673 || n = 1679 || n = 1687 || n = 1689 || n = 1694 || n = 1696 || n = 1701 || n = 1703 || n = 1708 || n = 1710 || n = 1715 || n = 1717)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646)
	; 9 bytes
	pha
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1648)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1649 || n = 1665 || n = 1707)
	; 11..12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1651)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1653 || n = 1661)
	; 11..12 bytes
	php
	delay_n (n-7)
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
.elseif((n .mod 9 = 3) && ((n > 1658 && n < 1696)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1660 || n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1663)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 1667 || n = 1681 || n = 1688 || n = 1709)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 1672 || n = 1712)
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
.elseif(n = 1675 || n = 1702)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1691 || n = 1705)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719 || n = 1768 || n = 1789)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1791)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1721 || n = 1735 || n = 1777 || n = 1784)
	; 12 bytes
	delay_n (n-3)
	cmp $C5
.elseif((n .mod 9 = 3) && ((n > 1721 && n < 1750) || n = 1785 || n = 1794))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1723 || n = 1737 || n = 1744 || n = 1793)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1724 || n = 1756 || n = 1772 || n = 1780 || n = 1796)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1754) || (n > 1766 && n < 1796)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1726)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1728 || n = 1752 || n = 1764)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1729 || n = 1733 || n = 1736 || n = 1742 || n = 1743 || n = 1745 || n = 1750 || n = 1757 || n = 1759 || n = 1765 || n = 1773 || n = 1775 || n = 1778)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1730 || n = 1738 || n = 1754 || n = 1770 || n = 1786)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1747)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1758)
	; 9 bytes
	pha
	sec
	lda #(n-8)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
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
	; 11 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 1766 || n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1791)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 8
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
.elseif(n = 1798 || n = 1822)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1799 || n = 1806 || n = 1814 || n = 1823 || n = 1829 || n = 1831)
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
	; 10 bytes
	pha
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1805 || n = 1816)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1807)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1808 || n = 1809 || n = 1811 || n = 1817)
	; 12 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1813)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 1815 || n = 1833)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1818 || n = 1826 || n = 1834)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1819)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1820 || n = 1828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1825)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif(n = 1827)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
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
.elseif(n = 1835)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
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
.elseif(n = 1838 || n = 1849 || n = 1871)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
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
.elseif(n = 1840)
	; 12 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
	pla
.elseif(n = 1841 || n = 1845 || n = 1854 || n = 1859 || n = 1862)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1842 || n = 1858 || n = 1874)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1843)
	; 10 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1844)
	; 10 bytes
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
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
.elseif(n = 1847 || n = 1869)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
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
	delay_n (n-7)
	plp
.elseif(n = 1852)
	; 11 bytes
	pha
	txa
	pha
	lax #7
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
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
.elseif(n = 1856 || n = 1861 || n = 1867)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1863)
	; 12 bytes
	pha
	sec
	lda #(n-27)/17
	jsr @nearby_rts
	sbc #1
	Jpl *-5
	pla
.elseif(n = 1864 || n = 1873)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1865)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1870)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 1877 || n = 1878 || n = 1895 || n = 1903 || n = 1910)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1885 || n = 1887 || n = 1905 || n = 1907)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 1880)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif((n .mod 8 = 2) && ((n > 1881 && n < 1915)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1883)
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	delay_n 6
	iny
	Jne *-5
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
	; 11 bytes
	pha
	clc
	lda #68 ;hides 'nop zp'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
.elseif(n = 1889)
	; 11 bytes
	pha
	clc
	lda #68 ;hides 'nop zp'
	delay_n 4
	adc #1
	Jne *-5
	pla
.elseif(n = 1891)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1894)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 1899)
	; 11 bytes
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
.elseif(n = 1901)
	; 11 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1904)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
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
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1926 || n = 1937)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1916)
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 1917 || n = 1941 || n = 1951 || n = 1953)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	; 12 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 1921 || n = 1939)
	; 12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1922 || n = 1930 || n = 1946)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1923 || n = 1925 || n = 1934 || n = 1943)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1924 || n = 1940 || n = 1948)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1931 || n = 1949 || n = 1950)
	; 12 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1935)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 1942)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1945)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1952)
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
.endif
.endif
.else
; 1954 to 2031
.if((n .mod 8 = 2) && (n = 1954 || n = 1962 || (n > 1977 && n < 2003) || n = 2018 || n = 2026))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1955 || n = 1969 || n = 1987 || n = 1989 || n = 1995 || n = 1997 || n = 2005 || n = 2007 || n = 2009 || n = 2011 || n = 2013 || n = 2015 || n = 2024 || n = 2031)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 9 = 3) && ((n > 1955 && n < 2029)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1957 || n = 2023)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif(n = 1958 || n = 2000)
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
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 1961)
	; 11 bytes
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 9 = 1) && (n = 1963 || (n > 1989 && n < 2018)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1964 || n = 1988 || n = 1996 || n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1966 || n = 2014)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1967 || n = 1973 || n = 1976 || n = 1982 || n = 1985 || n = 1993 || n = 1998 || n = 2006 || n = 2021 || n = 2029 || n = 2030)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1968 || n = 1980 || n = 2004 || n = 2016)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1970)
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
.elseif(n = 1975)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 1977)
	; 12 bytes
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
.elseif(n = 1984)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 2003 || n = 2025)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2022)
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
.elseif(n = 2027)
	; 11 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if(n = 2032)
	; 11 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2033)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 2035 || n = 2053 || n = 2062)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2038)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2039 || n = 2048 || n = 2054 || n = 2057 || n = 2066)
	; 12 bytes
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
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	pla
.elseif(n = 2042 || n = 2050)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 2043)
	; 11 bytes
	pha
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
.elseif(n = 2045)
	; 11 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2047 || n = 2069)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2049 || n = 2051 || n = 2059 || n = 2061 || n = 2063)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2056 || n = 2070)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2058)
	; 11 bytes
	pha
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 2065)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-5
	tay
	pla
.elseif(n = 2067)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif(n = 2068)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2107)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2072)
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
.elseif((n .mod 9 = 3) && ((n > 2072 && n < 2101)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2074 || n = 2086)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2075 || n = 2090 || n = 2093)
	; 12 bytes
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
.elseif(n = 2077)
	; 12 bytes
	pha
	clc
	lda #68 ;hides 'nop zp'
	nop
	nop $04
	adc #1
	Jne *-6
	pla
.elseif(n = 2078)
	; 10 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	pla
.elseif(n = 2079)
	; 12 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	nop $04
	iny
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
.elseif(n = 2081 || n = 2083)
	; 12 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 2085 || n = 2087 || n = 2103 || n = 2105)
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
.elseif(n = 2092)
	; 12 bytes
	pha
	lda $A5
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
	pla
.elseif(n = 2094 || n = 2095 || n = 2097 || n = 2104 || n = 2108)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2096)
	; 10 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
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
	; 12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 2101)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
	pla
.elseif(n = 2102)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
.elseif(n = 2109)
	; 11 bytes
	pha
	txa
	pha
	lax #8
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2122 || n = 2146)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 5) && ((n > 2110 && n < 2148)))
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
.elseif(n = 2113 || n = 2135)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2114)
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 2115 || n = 2131)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
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
	; 12 bytes
	delay_n (n-3)
	cmp $C5
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
.elseif(n = 2119)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2121 || n = 2123 || n = 2139)
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
.elseif(n = 2126 || n = 2140)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2128)
	; 12 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 2130)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2132)
	; 10 bytes
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
.elseif(n = 2133 || n = 2144)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2137)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
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
.elseif(n = 2142)
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
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2151 || n = 2153 || n = 2159 || n = 2164 || n = 2173 || n = 2175 || n = 2176 || n = 2185 || n = 2187)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2150 || n = 2156 || n = 2162 || n = 2165 || n = 2174 || n = 2183)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2155 || n = 2166 || n = 2177)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif(n = 2157 || n = 2168)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2158 || n = 2182)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 2169)
	; 12 bytes
	pha
	sec
	lda #(n-27)/17
	jsr @nearby_rts
	sbc #1
	Jpl *-5
	pla
.elseif(n = 2171)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
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
.elseif(n = 2180)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2186)
	; 11 bytes
	pha
	clc
	lda #58 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
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
.if(n = 2188 || n = 2224)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2189)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
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
.elseif(n = 2191 || n = 2193 || n = 2195 || n = 2214 || n = 2225)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2192 || n = 2198 || n = 2203 || n = 2213 || n = 2219 || n = 2222)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2194 || n = 2218)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 2200)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
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
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 2205)
	; 11 bytes
	pha
	clc
	lda #12 ;hides 'nop abs'
	nop $0
	adc #1
	Jne *-5
	pla
.elseif(n = 2206)
	; 10 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2207)
	; 11 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2209)
	; 11 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2215)
	; 10 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2216)
	; 12 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	nop
	bit $24
	sbc #1
	Jne *-6
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
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2229 || n = 2241 || n = 2257)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2228 || n = 2237 || n = 2246 || n = 2255 || n = 2263 || n = 2264)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2230 || n = 2254)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 2233 || n = 2242)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2234 || n = 2245)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
.elseif(n = 2236)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 2240 || n = 2250)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2243)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
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
.elseif(n = 2248)
	; 12 bytes
	pha
	clc
	lda #116 ;hides 'nop zp,x'
	delay_n 9
	adc #1
	Jne *-6
	pla
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
.elseif(n = 2258)
	; 10 bytes
	pha
	sec
	lda #(n-8)/9 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
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
	cmp <$CD ;hides 'cmp $06A0'
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
.if(n = 2266 || n = 2302)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2267 || n = 2300)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
.elseif(n = 2272)
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
.elseif(n = 2273 || n = 2282 || n = 2283 || n = 2293 || n = 2299 || n = 2303)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2274 || n = 2275 || n = 2277 || (n > 2283 && n < 2287) || n = 2295)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2276)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
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
.elseif(n = 2281)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
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
.elseif(n = 2290)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2291)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2294)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
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
.elseif(n = 2301)
	; 11 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2305 to 2343
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
	; 11 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	nop $04
	adc #1
	Jne *-5
	pla
.elseif(n = 2309 || n = 2317 || n = 2318 || n = 2343)
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
.elseif(n = 2311 || n = 2322 || n = 2333)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
.elseif((n .mod 2 = 1) && (n = 2313 || (n > 2318 && n < 2330) || n = 2335))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
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
.elseif(n = 2320)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2324)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2326 || n = 2338)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 11 bytes
	pha
	clc
	lda #90 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
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
.elseif(n = 2339)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
	pla
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
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2355 || n = 2377)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2345)
	; 12 bytes
	pha
	sec
	lda #250 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	pla
.elseif(n = 2346 || n = 2357 || n = 2359 || n = 2361 || n = 2365 || n = 2367 || n = 2369 || n = 2371 || n = 2379 || n = 2381)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2347)
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2348)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2349)
	; 12 bytes
	pha
	clc
	lda #100 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
	pla
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
.elseif(n = 2351)
	; 12 bytes
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
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
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2354 || n = 2368)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2356 || n = 2373)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 2360 || n = 2370 || n = 2380)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2362 || n = 2374)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2366)
	; 11 bytes
	pha
	txa
	pha
	lax #9
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2372 || n = 2382)
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
.elseif(n = 2375)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 6
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
.if(n = 2383 || n = 2385 || n = 2389 || n = 2391 || (n > 2392 && n < 2397) || n = 2401 || n = 2403 || n = 2405 || n = 2411 || n = 2413 || (n > 2414 && n < 2418))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2384)
	; 12 bytes
	pha
	clc
	lda #58 ;hides 'nop'
	nop
	bit $24
	adc #1
	Jne *-6
	pla
.elseif(n = 2386 || n = 2398 || n = 2410)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2390 || n = 2402 || n = 2414)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2397 || n = 2419)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif(n = 2399 || n = 2421)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2404 || n = 2418)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2406)
	; 11 bytes
	pha
	sec
	lda #(n-8)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2407)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
; 2422 to 2500
.if(n = 2422 || n = 2434 || n = 2458 || n = 2470 || n = 2482 || n = 2494)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2423 || n = 2425 || n = 2427 || n = 2428 || n = 2433 || n = 2435 || n = 2437 || n = 2439 || n = 2444 || n = 2445 || n = 2447 || n = 2451 || n = 2455 || n = 2457 || n = 2459 || n = 2461 || n = 2464 || (n > 2465 && n < 2470) || n = 2471 || n = 2473 || n = 2478 || n = 2479 || n = 2481 || n = 2483 || n = 2489 || n = 2491 || n = 2493 || n = 2499)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 12 = 0) && ((n > 2423 && n < 2497)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2426 || n = 2438 || n = 2450 || n = 2474 || n = 2486)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2429)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
	pla
.elseif(n = 2430 || n = 2440 || n = 2480 || n = 2500)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif((n .mod 11 = 1) && ((n > 2431 && n < 2499)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2441 || n = 2463 || n = 2485)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif(n = 2442 || n = 2452 || n = 2492)
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
.elseif(n = 2446)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
.elseif(n = 2449)
	; 12 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
.elseif(n = 2453)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2456)
	; 12 bytes
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	nop $04
	adc #1
	Jne *-6
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
.elseif(n = 2475)
	; 12 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2477)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2488)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2495)
	; 12 bytes
	pha
	sec
	lda #(n-9)/11 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
.elseif(n = 2497)
	; 12 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
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
.if(n = 2501 || n = 2503 || n = 2505 || (n > 2512 && n < 2516) || n = 2517 || n = 2521 || (n > 2522 && n < 2526) || n = 2527 || n = 2533 || (n > 2534 && n < 2538) || n = 2539 || n = 2541 || (n > 2546 && n < 2550) || n = 2555 || n = 2557 || n = 2559 || n = 2561 || n = 2563 || n = 2565 || n = 2567 || n = 2569 || n = 2571 || n = 2576)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2502 || n = 2512 || n = 2552)
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
.elseif(n = 2504 || n = 2546 || n = 2574)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2505 && n < 2579)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2507 || n = 2529 || n = 2573)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 0) && ((n > 2507 && n < 2569)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2509 || n = 2531 || n = 2553 || n = 2564 || n = 2575)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
.elseif(n = 2511 || n = 2528 || n = 2545)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2516)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2519)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2522 || n = 2534 || n = 2558 || n = 2570)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2526 || n = 2543 || n = 2577)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2538)
	; 11 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	pla
.elseif(n = 2540)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2550)
	; 12 bytes
	pha
	sec
	lda #n/10-1
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif(n = 2560)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
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
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579 || n = 2596)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2581 || n = 2583 || n = 2585 || n = 2587 || n = 2589 || n = 2591 || n = 2593 || n = 2598 || n = 2599 || n = 2601 || n = 2609 || n = 2615)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2582)
	; 11 bytes
	pha
	sec
	lda #(n-8)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 2590 || n = 2602 || n = 2614)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2594 || n = 2607)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2597 || n = 2608)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2600)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2603)
	; 11 bytes
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
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
.elseif(n = 2610)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2611)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2612)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
.if(n = 2618 || n = 2625 || n = 2642 || n = 2654)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619 || n = 2630 || n = 2641)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif((n > 2619 && n < 2623) || n = 2627 || n = 2629 || n = 2631 || (n > 2632 && n < 2638) || n = 2643 || n = 2646 || n = 2653 || n = 2655)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2623)
	; 11 bytes
	pha
	txa
	pha
	lax #10
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2624)
	; 12 bytes
	pha
	sec
	lda #218 ;hides 'nop'
	nop
	bit $24
	sbc #1
	Jne *-6
	pla
.elseif(n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 2645)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
.elseif(n = 2649)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
	Jne *-5
	tax
	pla
.elseif(n = 2651)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 6
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2656)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2665 || (n > 2666 && n < 2670) || n = 2671 || n = 2673 || n = 2677 || n = 2680 || n = 2682 || n = 2687 || n = 2689 || n = 2697 || n = 2699 || n = 2701 || n = 2703 || n = 2704 || n = 2706 || n = 2708 || n = 2709 || n = 2711 || n = 2717 || (n > 2718 && n < 2722) || n = 2723 || n = 2725 || n = 2731 || n = 2733)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2658 || n = 2672 || n = 2714 || n = 2728)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2659)
	; 12 bytes
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
.elseif(n = 2660)
	; 12 bytes
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	delay_n 6
	adc #1
	Jne *-6
	pla
.elseif(n = 2661 || n = 2716 || n = 2727)
	; 12 bytes
	pha
	sec
	lda #(n-10)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 12 = 10) && ((n > 2661 && n < 2735)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 11 = 1) && (n = 2663 || (n > 2684 && n < 2730)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 0) && ((n > 2663 && n < 2725)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 2) && ((n > 2665 && n < 2703) || n = 2726))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2670 || n = 2684)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 2679 || n = 2713 || n = 2730)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2681 || n = 2715 || n = 2732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2691)
	; 11 bytes
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
.elseif(n = 2692)
	; 11 bytes
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	sbc #1
	Jne *-5
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
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2695)
	; 12 bytes
	pha
	tya
	ldy #12 ;hides 'nop abs'
	nop $0
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2705)
	; 11 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.elseif(n = 2737 || n = 2739 || n = 2741 || (n > 2742 && n < 2746) || n = 2755 || n = 2757 || n = 2767 || n = 2769)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2738)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 11 = 1) && ((n > 2739 && n < 2774)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2742 || n = 2756)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2746 || n = 2758 || n = 2770)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2747)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 6
	dey
	Jne *-5
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
	jsr @nearby_rts_15cyc
	Jne *-4
	tay
	pla
.elseif(n = 2752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 2754 || n = 2768)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2759)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.elseif(n = 2763)
	; 12 bytes
	pha
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
.elseif(n = 2764)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.if(n = 2774 || n = 2786)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n > 2774 && n < 2778) || n = 2785 || (n > 2786 && n < 2790) || n = 2791 || n = 2797 || n = 2799 || (n > 2800 && n < 2804) || n = 2807 || n = 2809 || n = 2811)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	delay_n 6
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2780)
	; 11 bytes
	pha
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
.elseif(n = 2781)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2782 || n = 2794 || n = 2806)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2790)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 2795)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
; 2813 to 2968
.if(n = 2813 || n = 2814 || n = 2819 || n = 2821 || n = 2823 || n = 2825 || n = 2833 || (n > 2834 && n < 2838) || n = 2839 || n = 2841 || n = 2843 || n = 2845 || n = 2847 || n = 2848 || n = 2853 || n = 2855 || n = 2857 || (n > 2858 && n < 2866) || n = 2867 || n = 2869 || n = 2871 || n = 2873 || (n > 2874 && n < 2878) || n = 2879 || n = 2881 || n = 2884 || (n > 2885 && n < 2890) || n = 2891 || n = 2893 || n = 2895 || (n > 2896 && n < 2900) || n = 2901 || n = 2903 || n = 2905 || n = 2907 || n = 2909 || n = 2911 || n = 2913 || n = 2915 || n = 2920 || n = 2921 || n = 2923 || n = 2925 || n = 2927 || n = 2929 || n = 2931 || n = 2933 || n = 2935 || n = 2937 || n = 2939 || n = 2941 || (n > 2942 && n < 2950) || n = 2955 || n = 2957 || (n > 2958 && n < 2962) || n = 2963 || n = 2965 || n = 2967)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2817 && n < 2963)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 12 = 0) && ((n > 2819 && n < 2869) || (n > 2891 && n < 2965)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 2) && ((n > 2821 && n < 2955)))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2824 || n = 2838 || n = 2908 || n = 2922 || n = 2936)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2826 || n = 2840 || n = 2896 || n = 2910 || n = 2924 || n = 2966)
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2829)
	; 12 bytes
	pha
	clc
	lda #68 ;hides 'nop zp'
	delay_n 9
	adc #1
	Jne *-6
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
.elseif(n = 2849 || n = 2883 || n = 2900 || n = 2917 || n = 2934 || n = 2968)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2850)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 2851 || n = 2885 || n = 2919 || n = 2953)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2852)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
	pla
.elseif(n = 2872 || n = 2912 || n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 2880)
	; 11 bytes
	pha
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 2956)
	; 12 bytes
	pha
	clc
	lda #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	adc #1
	Jne *-6
	pla
.elseif(n = 2958)
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 2969 to 3125
.if(n = 2969 || n = 2971 || n = 2973 || n = 2975 || n = 2977 || n = 2979 || (n > 2981 && n < 2985) || n = 2989 || n = 2991 || n = 2993 || (n > 2994 && n < 2998) || n = 2999 || n = 3001 || n = 3003 || n = 3005 || n = 3007 || n = 3009 || n = 3011 || (n > 3014 && n < 3019) || n = 3023 || n = 3025 || (n > 3026 && n < 3030) || n = 3031 || n = 3033 || n = 3035 || n = 3037 || (n > 3038 && n < 3046) || n = 3047 || n = 3049 || n = 3051 || n = 3054 || n = 3056 || n = 3057 || n = 3059 || n = 3061 || n = 3063 || (n > 3065 && n < 3070) || n = 3071 || n = 3073 || (n > 3078 && n < 3082) || n = 3083 || n = 3085 || n = 3088 || (n > 3092 && n < 3104) || n = 3105 || (n > 3106 && n < 3110) || n = 3111 || (n > 3112 && n < 3118) || n = 3119 || n = 3122 || n = 3124 || n = 3125)
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 2970 || n = 2987 || n = 3004 || n = 3021 || n = 3089 || n = 3123)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2972 || n = 3032 || n = 3112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 12 = 10) && ((n > 2973 && n < 3071)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 12 = 0) && ((n > 2975 && n < 3073)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 2) && ((n > 2977 && n < 3039) || n = 3074 || n = 3086))
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 12) && ((n > 2979 && n < 3009) || (n > 3049 && n < 3121)))
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
.elseif(n = 2985 || n = 3019 || n = 3053 || n = 3087 || n = 3121)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 10) && ((n > 2991 && n < 3021) || (n > 3075 && n < 3119)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3030 || n = 3110)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3052)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3055)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
	tax
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
.elseif(n = 3075)
	; 12 bytes
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
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
	; 11 bytes
	pha
	sec
	lda #0
	delay_n 7
	sbc #1
	Jne *-4
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
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if((n > 3125 && n < 3130) || n = 3131 || n = 3133 || n = 3135 || n = 3136 || (n > 3140 && n < 3146) || n = 3147 || n = 3149 || n = 3151 || n = 3153 || n = 3154 || n = 3156 || n = 3158 || n = 3159 || n = 3161 || n = 3163 || (n > 3164 && n < 3169) || n = 3171 || n = 3173 || n = 3175 || n = 3177 || (n > 3182 && n < 3188) || n = 3193 || (n > 3195 && n < 3202) || n = 3203 || n = 3205 || n = 3207 || n = 3209 || n = 3211 || (n > 3212 && n < 3216) || n = 3217 || (n > 3218 && n < 3223) || n = 3224 || n = 3226 || n = 3227 || n = 3229 || n = 3231 || (n > 3232 && n < 3240) || n = 3241 || n = 3243 || n = 3245 || (n > 3246 && n < 3250) || n = 3251 || (n > 3252 && n < 3256) || (n > 3260 && n < 3270) || n = 3271 || n = 3273 || n = 3275 || (n > 3276 && n < 3282))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3130 || n = 3150 || n = 3170 || n = 3250 || n = 3270)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 10) && ((n > 3131 && n < 3217) || n = 3244 || n = 3272))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3133 && n < 3275)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3137)
	; 11 bytes
	pha
	txa
	pha
	lax #12
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3138 || n = 3155 || n = 3172 || n = 3189 || n = 3206 || n = 3223 || n = 3240 || n = 3257)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3139)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3140 || n = 3157 || n = 3191 || n = 3208 || n = 3225 || n = 3242 || n = 3259 || n = 3276)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3152 || n = 3192 || n = 3212 || n = 3252)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3164)
	; 12 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
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
	; 12 bytes
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
.elseif(n = 3180)
	; 12 bytes
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 6
	sbc #1
	Jne *-6
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
	ldy #244 ;hides 'nop zp,x'
	delay_n 6
	dey
	Jne *-5
	tay
	pla
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
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
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
.elseif(n = 3228)
	; 11 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
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
.elseif(n = 3256)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	pla
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
.endif
.else
; 3282 to 3437
.if(n = 3282 || n = 3283 || n = 3285 || n = 3287 || n = 3289 || n = 3294 || n = 3295 || (n > 3296 && n < 3300) || n = 3301 || (n > 3302 && n < 3308) || n = 3313 || n = 3315 || (n > 3316 && n < 3325) || n = 3329 || n = 3331 || (n > 3332 && n < 3342) || n = 3343 || (n > 3344 && n < 3350) || n = 3351 || (n > 3352 && n < 3356) || n = 3357 || n = 3360 || (n > 3361 && n < 3370) || n = 3371 || (n > 3372 && n < 3376) || n = 3377 || (n > 3378 && n < 3384) || n = 3385 || (n > 3386 && n < 3390) || n = 3391 || n = 3397 || n = 3399 || n = 3401 || (n > 3402 && n < 3410) || n = 3411 || n = 3413 || n = 3415 || n = 3417 || (n > 3418 && n < 3426) || n = 3433 || (n > 3434 && n < 3438))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3284)
	; 11 bytes
	pha
	sec
	lda #(n-8)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
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
.elseif((n .mod 14 = 12) && ((n > 3287 && n < 3429)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3290 || n = 3350 || n = 3390 || n = 3430)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3291 || n = 3308 || n = 3325 || n = 3359 || n = 3376 || n = 3393 || n = 3410 || n = 3427)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3292 || n = 3312 || n = 3332 || n = 3352 || n = 3392 || n = 3432)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3293 || n = 3310 || n = 3327 || n = 3361 || n = 3378 || n = 3395 || n = 3429)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif((n .mod 14 = 10) && ((n > 3299 && n < 3427)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3309)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
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
.elseif(n = 3394)
	; 11 bytes
	pha
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3396)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3416)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3431)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
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
.endif
.endif
.else
; 3438 to 3750
.if(n = 3438 || n = 3439 || n = 3441 || n = 3443 || n = 3445 || (n > 3446 && n < 3450) || n = 3451 || n = 3453 || n = 3455 || (n > 3456 && n < 3461) || n = 3462 || (n > 3463 && n < 3468) || n = 3469 || n = 3471 || (n > 3472 && n < 3478) || n = 3479 || n = 3481 || n = 3483 || (n > 3484 && n < 3490) || n = 3491 || n = 3493 || n = 3494 || (n > 3498 && n < 3504) || n = 3505 || n = 3507 || n = 3509 || n = 3511 || n = 3513 || (n > 3514 && n < 3524) || n = 3525 || n = 3527 || n = 3528 || (n > 3532 && n < 3538) || n = 3539 || (n > 3540 && n < 3546) || n = 3547 || n = 3549 || n = 3551 || n = 3553 || (n > 3554 && n < 3563) || n = 3564 || n = 3567 || n = 3569 || n = 3571 || (n > 3572 && n < 3580) || n = 3581 || (n > 3582 && n < 3590) || n = 3591 || n = 3593 || n = 3595 || n = 3598 || (n > 3599 && n < 3610) || n = 3611 || n = 3613 || n = 3615 || (n > 3616 && n < 3630) || (n > 3633 && n < 3648) || n = 3649 || (n > 3653 && n < 3660) || n = 3661 || n = 3663 || n = 3664 || n = 3666 || n = 3668 || n = 3669 || n = 3671 || (n > 3672 && n < 3682) || n = 3683 || (n > 3684 && n < 3688) || n = 3689 || n = 3691 || (n > 3692 && n < 3699) || n = 3700 || (n > 3701 && n < 3706) || (n > 3706 && n < 3710) || n = 3711 || (n > 3712 && n < 3716) || n = 3717 || n = 3719 || n = 3721 || (n > 3722 && n < 3730) || n = 3731 || n = 3734 || (n > 3735 && n < 3750))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif((n .mod 14 = 10) && ((n > 3439 && n < 3497) || (n > 3523 && n < 3581)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3441 && n < 3583)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3444 || n = 3478 || n = 3495 || n = 3529 || n = 3546 || n = 3563 || n = 3597 || n = 3614 || n = 3631 || n = 3648 || n = 3665 || n = 3682 || n = 3699 || n = 3716 || n = 3733 || n = 3750)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3446)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3450 || n = 3490 || n = 3530 || n = 3550 || n = 3570 || n = 3590 || n = 3610 || n = 3630 || n = 3670 || n = 3690 || n = 3710 || n = 3730)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3452 || n = 3472 || n = 3492 || n = 3532 || n = 3572 || n = 3592 || n = 3612 || n = 3632 || n = 3652 || n = 3672 || n = 3692 || n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3461)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
.elseif(n = 3463 || n = 3480 || n = 3497 || n = 3514 || n = 3531 || n = 3548 || n = 3565 || n = 3599 || n = 3616 || n = 3633 || n = 3650 || n = 3667 || n = 3684 || n = 3701 || n = 3718 || n = 3735)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3504)
	; 12 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
.elseif(n = 3506)
	; 12 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3508)
	; 11 bytes
	pha
	sec
	lda #(n-8)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 3594)
	; 12 bytes
	pha
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3651)
	; 11 bytes
	pha
	txa
	pha
	lax #14
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3653)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3660)
	; 12 bytes
	pha
	clc
	lda #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	adc #1
	Jne *-6
	pla
.elseif(n = 3662)
	; 12 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3688)
	; 12 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 9
	adc #1
	Jne *-6
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
.elseif(n = 3720)
	; 12 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 9
	adc #1
	Jne *-6
	pla
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
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
; 3751 to 4375
.if(n = 3751 || n = 3753 || (n > 3754 && n < 3767) || n = 3768 || n = 3771 || (n > 3772 && n < 3784) || n = 3785 || (n > 3786 && n < 3790) || n = 3791 || (n > 3792 && n < 3801) || (n > 3803 && n < 3810) || n = 3811 || (n > 3812 && n < 3818) || n = 3819 || (n > 3820 && n < 3830) || n = 3831 || n = 3833 || n = 3834 || n = 3836 || (n > 3837 && n < 3850) || n = 3851 || n = 3853 || (n > 3854 && n < 3867) || n = 3868 || (n > 3872 && n < 3886) || n = 3887 || n = 3889 || n = 3891 || (n > 3892 && n < 3898) || (n > 3898 && n < 3903) || n = 3904 || n = 3906 || n = 3907 || n = 3909 || n = 3911 || n = 3913 || (n > 3914 && n < 3920) || n = 3921 || (n > 3922 && n < 3930) || n = 3931 || (n > 3932 && n < 3937) || n = 3938 || (n > 3939 && n < 3950) || n = 3951 || n = 3953 || n = 3955 || (n > 3956 && n < 3970) || (n > 3973 && n < 3988) || n = 3989 || n = 3991 || (n > 3992 && n < 4005) || n = 4006 || n = 4009 || n = 4011 || (n > 4012 && n < 4022) || n = 4023 || (n > 4024 && n < 4030) || n = 4031 || (n > 4032 && n < 4039) || n = 4040 || (n > 4041 && n < 4050) || n = 4051 || (n > 4052 && n < 4056) || n = 4057 || (n > 4058 && n < 4070) || n = 4071 || n = 4074 || (n > 4075 && n < 4090) || n = 4091 || (n > 4092 && n < 4107) || n = 4108 || n = 4111 || (n > 4112 && n < 4124) || n = 4125 || (n > 4126 && n < 4130) || n = 4131 || (n > 4132 && n < 4141) || n = 4142 || (n > 4143 && n < 4150) || n = 4151 || (n > 4152 && n < 4158) || n = 4159 || (n > 4160 && n < 4165) || n = 4166 || n = 4168 || n = 4169 || n = 4171 || n = 4173 || n = 4174 || n = 4176 || (n > 4177 && n < 4190) || n = 4191 || n = 4193 || (n > 4194 && n < 4209) || (n > 4212 && n < 4226) || n = 4227 || n = 4229 || n = 4231 || (n > 4232 && n < 4243) || n = 4244 || (n > 4245 && n < 4250) || n = 4251 || (n > 4252 && n < 4260) || n = 4261 || (n > 4262 && n < 4270) || n = 4271 || (n > 4272 && n < 4277) || n = 4278 || (n > 4279 && n < 4290) || n = 4291 || n = 4293 || n = 4295 || (n > 4296 && n < 4310) || (n > 4313 && n < 4328) || n = 4329 || n = 4331 || (n > 4332 && n < 4345) || n = 4346 || n = 4348 || n = 4349 || n = 4351 || (n > 4352 && n < 4362) || n = 4363 || n = 4365 || (n > 4366 && n < 4370) || n = 4371 || (n > 4372 && n < 4376))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3752 || n = 4008)
	; 12 bytes
	pha
	sec
	lda #(n-8)/16 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
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
.elseif(n = 3767 || n = 3784 || n = 3801 || n = 3818 || n = 3835 || n = 3852 || n = 3869 || n = 3886 || n = 3903 || n = 3920 || n = 3937 || n = 3954 || n = 3971 || n = 3988 || n = 4005 || n = 4022 || n = 4039 || n = 4056 || n = 4073 || n = 4090 || n = 4107 || n = 4124 || n = 4141 || n = 4158 || n = 4175 || n = 4192 || n = 4209 || n = 4226 || n = 4243 || n = 4260 || n = 4277 || n = 4294 || n = 4311 || n = 4328 || n = 4345)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3769 || n = 3786 || n = 3803 || n = 3820 || n = 3837 || n = 3854 || n = 3871 || n = 3888 || n = 3905 || n = 3922 || n = 3939 || n = 3956 || n = 3973 || n = 3990 || n = 4007 || n = 4024 || n = 4041 || n = 4058 || n = 4075 || n = 4092 || n = 4109 || n = 4126 || n = 4143 || n = 4160 || n = 4177 || n = 4194 || n = 4211 || n = 4228 || n = 4245 || n = 4262 || n = 4279 || n = 4296 || n = 4313 || n = 4330 || n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3770 || n = 3790 || n = 3830 || n = 3850 || n = 3870 || n = 3890 || n = 3930 || n = 3950 || n = 3970 || n = 4010 || n = 4030 || n = 4050 || n = 4110 || n = 4130 || n = 4150 || n = 4170 || n = 4190 || n = 4210 || n = 4230 || n = 4250 || n = 4270 || n = 4290 || n = 4310 || n = 4350 || n = 4370)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3772 || n = 3792 || n = 3812 || n = 3832 || n = 3872 || n = 3892 || n = 3912 || n = 3932 || n = 3952 || n = 3972 || n = 3992 || n = 4012 || n = 4032 || n = 4052 || n = 4072 || n = 4112 || n = 4132 || n = 4152 || n = 4172 || n = 4212 || n = 4232 || n = 4252 || n = 4272 || n = 4292 || n = 4312 || n = 4332 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 3810)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
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
.elseif(n = 3908)
	; 11 bytes
	pha
	txa
	pha
	lax #15
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 3910 || n = 4167)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3914)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4070)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
	pla
.elseif(n = 4165)
	; 11 bytes
	pha
	txa
	pha
	lax #16
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4362)
	; 12 bytes
	pha
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 4366)
	; 12 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 4376 to 5000
.if(n = 4376 || n = 4377 || n = 4379 || (n > 4380 && n < 4390) || n = 4391 || (n > 4392 && n < 4410) || n = 4411 || (n > 4412 && n < 4416) || n = 4417 || (n > 4418 && n < 4422) || n = 4423 || (n > 4424 && n < 4430) || n = 4431 || (n > 4432 && n < 4450) || n = 4451 || n = 4453 || n = 4455 || (n > 4456 && n < 4470) || n = 4471 || (n > 4472 && n < 4476) || (n > 4476 && n < 4490) || n = 4491 || (n > 4492 && n < 4510) || n = 4511 || (n > 4512 && n < 4530) || n = 4531 || (n > 4532 && n < 4550) || n = 4551 || (n > 4552 && n < 4570) || n = 4571 || (n > 4572 && n < 4590) || n = 4591 || (n > 4592 && n < 4610) || n = 4611 || (n > 4612 && n < 4630) || n = 4631 || (n > 4632 && n < 4650) || n = 4651 || (n > 4652 && n < 4670) || n = 4671 || (n > 4672 && n < 4679) || n = 4680 || (n > 4681 && n < 4690) || n = 4691 || (n > 4692 && n < 4710) || n = 4711 || (n > 4712 && n < 4730) || n = 4731 || (n > 4732 && n < 4750) || n = 4751 || (n > 4752 && n < 4758) || n = 4759 || (n > 4760 && n < 4770) || n = 4771 || (n > 4772 && n < 4790) || n = 4791 || (n > 4792 && n < 4804) || n = 4805 || (n > 4806 && n < 4810) || n = 4811 || (n > 4812 && n < 4830) || n = 4831 || (n > 4832 && n < 4850) || n = 4851 || (n > 4852 && n < 4870) || n = 4871 || (n > 4872 && n < 4890) || n = 4891 || (n > 4892 && n < 4910) || n = 4911 || (n > 4912 && n < 4930) || n = 4931 || (n > 4932 && n < 4936) || n = 4937 || (n > 4938 && n < 4950) || n = 4951 || (n > 4952 && n < 4970) || n = 4971 || (n > 4972 && n < 4990) || n = 4991 || (n > 4992 && n < 5001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4378)
	; 12 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
.elseif(n = 4380)
	; 12 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4390 || n = 4430 || n = 4450 || n = 4470 || n = 4490 || n = 4510 || n = 4530 || n = 4550 || n = 4570 || n = 4590 || n = 4610 || n = 4630 || n = 4650 || n = 4670 || n = 4690 || n = 4710 || n = 4730 || n = 4750 || n = 4770 || n = 4790 || n = 4810 || n = 4830 || n = 4850 || n = 4870 || n = 4890 || n = 4910 || n = 4930 || n = 4950 || n = 4970 || n = 4990)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4392 || n = 4412 || n = 4432 || n = 4452 || n = 4472 || n = 4492 || n = 4512 || n = 4532 || n = 4552 || n = 4572 || n = 4592 || n = 4612 || n = 4632 || n = 4652 || n = 4672 || n = 4692 || n = 4712 || n = 4732 || n = 4752 || n = 4772 || n = 4792 || n = 4812 || n = 4832 || n = 4852 || n = 4872 || n = 4892 || n = 4912 || n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4410)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4416)
	; 12 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
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
.elseif(n = 4422)
	; 11 bytes
	pha
	txa
	pha
	lax #17
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4424 || n = 4681 || n = 4938)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 4454 || n = 4758)
	; 12 bytes
	pha
	sec
	lda #(n-8)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
.elseif(n = 4456 || n = 4760)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4476)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	tax
	pla
.elseif(n = 4679)
	; 11 bytes
	pha
	txa
	pha
	lax #18
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 4804)
	; 12 bytes
	pha
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
.elseif(n = 4806)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4936)
	; 11 bytes
	pha
	txa
	pha
	lax #19
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if((n > 5000 && n < 5010) || n = 5011 || (n > 5012 && n < 5030) || n = 5031 || (n > 5032 && n < 5050) || n = 5051 || (n > 5052 && n < 5068) || n = 5069 || n = 5071 || (n > 5072 && n < 5090) || n = 5091 || (n > 5092 && n < 5110) || n = 5111 || n = 5113 || (n > 5114 && n < 5130) || n = 5131 || (n > 5132 && n < 5156) || n = 5157 || (n > 5158 && n < 5193) || n = 5194 || (n > 5195 && n < 5450) || n = 5451 || (n > 5452 && n < 5508) || n = 5509 || (n > 5510 && n < 5707) || n = 5708 || (n > 5709 && n < 5964) || n = 5965 || (n > 5966 && n < 6221) || n = 6222 || (n > 6223 && n < 6478) || n = 6479 || (n > 6480 && n < 6735) || n = 6736 || (n > 6737 && n < 6992) || n = 6993 || (n > 6994 && n < 7249) || n = 7250 || (n > 7251 && n < 7501))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5010 || n = 5030 || n = 5050 || n = 5070 || n = 5090 || n = 5110)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072 || n = 5092 || n = 5112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5068)
	; 12 bytes
	pha
	clc
	lda #26 ;hides 'nop'
	jsr @nearby_rts_15cyc
	adc #1
	Jne *-6
	pla
.elseif(n = 5114)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5130)
	; 12 bytes
	pha
	sec
	lda #0
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 5132)
	; 12 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5156 || n = 5508)
	; 12 bytes
	pha
	sec
	lda #(n-8)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
.elseif(n = 5158 || n = 5510)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5193)
	; 11 bytes
	pha
	txa
	pha
	lax #20
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5195 || n = 5452 || n = 5709 || n = 5966 || n = 6223 || n = 6480 || n = 6737 || n = 6994 || n = 7251)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 5450)
	; 11 bytes
	pha
	txa
	pha
	lax #21
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5707)
	; 11 bytes
	pha
	txa
	pha
	lax #22
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 5964)
	; 11 bytes
	pha
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6221)
	; 11 bytes
	pha
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6478)
	; 11 bytes
	pha
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6735)
	; 11 bytes
	pha
	txa
	pha
	lax #26
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 6992)
	; 11 bytes
	pha
	txa
	pha
	lax #27
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7249)
	; 11 bytes
	pha
	txa
	pha
	lax #28
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 7506) || n = 7507 || (n > 7508 && n < 7763) || n = 7764 || (n > 7765 && n < 8020) || n = 8021 || (n > 8022 && n < 8277) || n = 8278 || (n > 8279 && n < 8534) || n = 8535 || (n > 8536 && n < 8791) || n = 8792 || (n > 8793 && n < 9048) || n = 9049 || (n > 9050 && n < 9305) || n = 9306 || (n > 9307 && n < 9562) || n = 9563 || (n > 9564 && n < 9819) || n = 9820 || (n > 9821 && n < 10001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7506)
	; 11 bytes
	pha
	txa
	pha
	lax #29
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 7508 || n = 7765 || n = 8022 || n = 8279 || n = 8536 || n = 8793 || n = 9050 || n = 9307 || n = 9564 || n = 9821)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7763)
	; 11 bytes
	pha
	txa
	pha
	lax #30
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8020)
	; 11 bytes
	pha
	txa
	pha
	lax #31
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8277)
	; 11 bytes
	pha
	txa
	pha
	lax #32
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8534)
	; 11 bytes
	pha
	txa
	pha
	lax #33
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 8791)
	; 11 bytes
	pha
	txa
	pha
	lax #34
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9048)
	; 11 bytes
	pha
	txa
	pha
	lax #35
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9305)
	; 11 bytes
	pha
	txa
	pha
	lax #36
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9562)
	; 11 bytes
	pha
	txa
	pha
	lax #37
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 9819)
	; 11 bytes
	pha
	txa
	pha
	lax #38
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10076) || n = 10077 || (n > 10078 && n < 10333) || n = 10334 || (n > 10335 && n < 10590) || n = 10591 || (n > 10592 && n < 10847) || n = 10848 || (n > 10849 && n < 11104) || n = 11105 || (n > 11106 && n < 11361) || n = 11362 || (n > 11363 && n < 11618) || n = 11619 || (n > 11620 && n < 11875) || n = 11876 || (n > 11877 && n < 12132) || n = 12133 || (n > 12134 && n < 12389) || n = 12390 || (n > 12391 && n < 12646) || n = 12647 || (n > 12648 && n < 12903) || n = 12904 || (n > 12905 && n < 13160) || n = 13161 || (n > 13162 && n < 13417) || n = 13418 || (n > 13419 && n < 13674) || n = 13675 || (n > 13676 && n < 13931) || n = 13932 || (n > 13933 && n < 14188) || n = 14189 || (n > 14190 && n < 14445) || n = 14446 || (n > 14447 && n < 14702) || n = 14703 || (n > 14704 && n < 14959) || n = 14960 || (n > 14961 && n < 15001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10076)
	; 11 bytes
	pha
	txa
	pha
	lax #39
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10078 || n = 10335 || n = 10592 || n = 10849 || n = 11106 || n = 11363 || n = 11620 || n = 11877 || n = 12134 || n = 12391 || n = 12648 || n = 12905 || n = 13162 || n = 13419 || n = 13676 || n = 13933 || n = 14190 || n = 14447 || n = 14704 || n = 14961)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10333)
	; 11 bytes
	pha
	txa
	pha
	lax #40
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10590)
	; 11 bytes
	pha
	txa
	pha
	lax #41
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 10847)
	; 11 bytes
	pha
	txa
	pha
	lax #42
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11104)
	; 11 bytes
	pha
	txa
	pha
	lax #43
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11361)
	; 11 bytes
	pha
	txa
	pha
	lax #44
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11618)
	; 11 bytes
	pha
	txa
	pha
	lax #45
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 11875)
	; 11 bytes
	pha
	txa
	pha
	lax #46
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12132)
	; 11 bytes
	pha
	txa
	pha
	lax #47
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12389)
	; 11 bytes
	pha
	txa
	pha
	lax #48
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12646)
	; 11 bytes
	pha
	txa
	pha
	lax #49
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 12903)
	; 11 bytes
	pha
	txa
	pha
	lax #50
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13160)
	; 11 bytes
	pha
	txa
	pha
	lax #51
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13417)
	; 11 bytes
	pha
	txa
	pha
	lax #52
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13674)
	; 11 bytes
	pha
	txa
	pha
	lax #53
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 13931)
	; 11 bytes
	pha
	txa
	pha
	lax #54
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14188)
	; 11 bytes
	pha
	txa
	pha
	lax #55
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14445)
	; 11 bytes
	pha
	txa
	pha
	lax #56
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14702)
	; 11 bytes
	pha
	txa
	pha
	lax #57
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 14959)
	; 11 bytes
	pha
	txa
	pha
	lax #58
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.else
; 15001 to 20000
.if((n > 15000 && n < 15216) || n = 15217 || (n > 15218 && n < 15473) || n = 15474 || (n > 15475 && n < 15730) || n = 15731 || (n > 15732 && n < 15987) || n = 15988 || (n > 15989 && n < 16244) || n = 16245 || (n > 16246 && n < 16501) || n = 16502 || (n > 16503 && n < 16758) || n = 16759 || (n > 16760 && n < 17015) || n = 17016 || (n > 17017 && n < 17272) || n = 17273 || (n > 17274 && n < 17529) || n = 17530 || (n > 17531 && n < 17786) || n = 17787 || (n > 17788 && n < 18043) || n = 18044 || (n > 18045 && n < 18300) || n = 18301 || (n > 18302 && n < 18557) || n = 18558 || (n > 18559 && n < 18814) || n = 18815 || (n > 18816 && n < 19071) || n = 19072 || (n > 19073 && n < 19328) || n = 19329 || (n > 19330 && n < 19585) || n = 19586 || (n > 19587 && n < 19842) || n = 19843 || (n > 19844 && n < 20001))
	; 13 bytes
	pha
	txa
	pha
	lda #>(n-55)
	ldx #<(n-55)
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15216)
	; 11 bytes
	pha
	txa
	pha
	lax #59
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15218 || n = 15475 || n = 15732 || n = 15989 || n = 16246 || n = 16503 || n = 16760 || n = 17017 || n = 17274 || n = 17531 || n = 17788 || n = 18045 || n = 18302 || n = 18559 || n = 18816 || n = 19073 || n = 19330 || n = 19587 || n = 19844)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15473)
	; 11 bytes
	pha
	txa
	pha
	lax #60
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15730)
	; 11 bytes
	pha
	txa
	pha
	lax #61
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 15987)
	; 11 bytes
	pha
	txa
	pha
	lax #62
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16244)
	; 11 bytes
	pha
	txa
	pha
	lax #63
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16501)
	; 11 bytes
	pha
	txa
	pha
	lax #64
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 16758)
	; 11 bytes
	pha
	txa
	pha
	lax #65
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17015)
	; 11 bytes
	pha
	txa
	pha
	lax #66
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17272)
	; 11 bytes
	pha
	txa
	pha
	lax #67
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17529)
	; 11 bytes
	pha
	txa
	pha
	lax #68
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 17786)
	; 11 bytes
	pha
	txa
	pha
	lax #69
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18043)
	; 11 bytes
	pha
	txa
	pha
	lax #70
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18300)
	; 11 bytes
	pha
	txa
	pha
	lax #71
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18557)
	; 11 bytes
	pha
	txa
	pha
	lax #72
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 18814)
	; 11 bytes
	pha
	txa
	pha
	lax #73
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19071)
	; 11 bytes
	pha
	txa
	pha
	lax #74
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19328)
	; 11 bytes
	pha
	txa
	pha
	lax #75
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19585)
	; 11 bytes
	pha
	txa
	pha
	lax #76
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.elseif(n = 19842)
	; 11 bytes
	pha
	txa
	pha
	lax #77
	jsr delay_256a_x_33_clocks
	pla
	tax
	pla
.endif
.endif
.endif

.endif
	rest
.endmacro
