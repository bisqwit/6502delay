; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber A, S, Z+N, C, D, and V
; Will preserve X, Y, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; WARNING: May write garbage into $200-$2FF.
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
; - 3 bytes or less when n <= 17
; - 4 bytes or less when n <= 21
; - 5 bytes or less when n <= 284
; - 6 bytes or less when n <= 288
; - 7 bytes or less when n <= 298
; - 8 bytes or less when n <= 1289
; - 9 bytes or less when n <= 1305
; -10 bytes or less when n <= 2233
; -11 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=1.9, max=3
; - When 11 <= n <= 20: min=2, avg=3.2, max=4
; - When 21 <= n <= 30: min=4, avg=4.7, max=5
; - When 31 <= n <= 50: min=4, avg=5.0, max=5
; - When 51 <= n <= 75: min=5, avg=5.0, max=5
; - When 76 <= n <= 100: min=5, avg=5.0, max=5
; - When 101 <= n <= 200: min=5, avg=5.0, max=5
; - When 201 <= n <= 300: min=5, avg=5.3, max=8
; - When 301 <= n <= 500: min=6, avg=7.5, max=8
; - When 501 <= n <= 750: min=6, avg=7.5, max=8
; - When 751 <= n <= 1000: min=7, avg=7.6, max=8
; - When 1001 <= n <= 2000: min=6, avg=8.2, max=10
; - When 2001 <= n <= 3000: min=8, avg=9.4, max=11
; - When 3001 <= n <= 5000: min=8, avg=10.3, max=11
; - When 5001 <= n <= 7500: min=9, avg=10.9, max=11
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
.elseif(n = 9 || n = 10 || n = 20 || n = 22)
	; 3..5 bytes
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
.elseif(n = 16 || n = 17 || n = 21 || n = 26)
	; 3..4 bytes
	delay_n (n-13)
	brk
.elseif(n = 18 || n = 19)
	; 4 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 23)
	; 5 bytes
	lda #$2A ;hides 'rol a'
	pha
	Jpl *-2
.elseif(n = 24)
	; 4 bytes
	lda #$0A ;hides 'asl a'
	Jpl *-1
.elseif(n = 25)
	; 5 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif(n = 285 || n = 286 || n = 307 || n = 312)
	; 6..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 287 || n = 301)
	; 6..7 bytes
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
.elseif((n > 291 && n < 298))
	; 7 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 300)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 303 || n = 308)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
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
.if((n >= 391) && (n < 430))
; 391 to 429
.if(n = 391)
	; 8 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 392 || n = 412 || n = 417 || n = 422 || n = 427)
	; 8 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 3) && ((n > 392 && n < 419)))
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 394 || n = 406 || n = 424 || n = 429)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 395 || n = 405 || n = 420)
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 396)
	; 8 bytes
	brk
	delay_n (n-13)
.elseif(n = 397)
	; 7 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	brk
	Jne *-4
.elseif(n = 399)
	; 8 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 400 || n = 410 || n = 415 || n = 425)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 401 || n = 426)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 402)
	; 7 bytes
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
.elseif(n = 404)
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
.elseif(n = 419)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
.elseif(n = 421)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 423 || n = 428)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.endif
.else
; 430 to 468
.if((n .mod 5 = 0) && ((n > 429 && n < 451) || n = 465))
	; 7 bytes
	tya
	ldy #n/5-2
	dey
	Jpl *-1
	tay
.elseif(n = 431 || n = 436 || n = 456 || n = 466)
	; 8 bytes
	pha
	delay_n (n-3)
.elseif(n = 432 || n = 439 || n = 446)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 433 || n = 438 || n = 443 || n = 448 || n = 458 || n = 468)
	; 7 bytes
	sec
	lda #(n-8)/5
	sbc #1
	Jpl *-2
.elseif(n = 434 || n = 454 || n = 464)
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 437)
	; 7 bytes
	sec
	lda #$E9 ;hides 'sbc #$08'
	php ;first loop only
	nop
	Jne *-3
.elseif(n = 441)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
.elseif((n .mod 5 = 2) && ((n > 441 && n < 463)))
	; 8 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 453 || n = 463)
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
.elseif(n = 455 || n = 460)
	; 7 bytes
	tya
	ldy #n/5-1
	dey
	Jne *-1
	tay
.elseif(n = 459)
	; 7 bytes
	tya
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
	tay
.elseif(n = 461)
	; 8 bytes
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_15cyc
	Jcs *-5
.elseif(n = 467)
	; 8 bytes
	tya
	ldy #(n-12)/7
	nop
	dey
	Jpl *-2
	tay
.endif
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
.if((n .mod 5 = 2) && ((n > 546 && n < 568) || (n > 581 && n < 603) || (n > 611 && n < 623)))
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
.elseif(n = 559)
	; 7 bytes
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	brk
	Jne *-4
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
.elseif(n = 569 || n = 574 || n = 589 || n = 594 || n = 599 || n = 604)
	; 8 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 577)
	; 7 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	brk
	Jne *-4
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
.elseif(n = 609)
	; 8 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
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
.elseif((n .mod 5 = 1) && (n = 636 || n = 646 || (n > 665 && n < 692) || n = 706 || n = 716 || n = 721 || n = 736 || n = 746 || n = 751 || n = 776))
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif(n = 641)
	; 8 bytes
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	jsr @nearby_rts_15cyc
	Jne *-5
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
.elseif(n = 701)
	; 8 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 741 || n = 781)
	; 8 bytes
	tya
	ldy #(n-13)/8
	pha
	dey
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
.if((n >= 938) && (n < 1094))
; 938 to 1093
.if((n .mod 5 = 3) && ((n > 937 && n < 1014) || (n > 1022 && n < 1094)))
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
.elseif((n .mod 5 = 2) && ((n > 941 && n < 953) || (n > 966 && n < 988) || (n > 996 && n < 1018) || (n > 1031 && n < 1058) || (n > 1066 && n < 1093)))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 944 || n = 949 || n = 954 || n = 959 || n = 974 || n = 979 || n = 989 || n = 994 || n = 1009 || n = 1014 || n = 1019 || n = 1022 || n = 1024 || n = 1044 || n = 1049 || n = 1059 || n = 1064 || n = 1079 || n = 1084 || n = 1089)
	; 7..8 bytes
	pla
	delay_n (n-4)
.elseif((n .mod 5 = 1) && ((n > 945 && n < 962) || (n > 985 && n < 1002) || n = 1016 || n = 1021 || n = 1036 || n = 1066 || n = 1071 || n = 1086))
	; 7..8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 956 && n < 972) || (n > 991 && n < 1007) || (n > 1026 && n < 1042) || (n > 1061 && n < 1077)))
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
.elseif(n = 969 || n = 976 || n = 1004 || n = 1011 || n = 1039 || n = 1046 || n = 1081)
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
.elseif(n = 1051 || n = 1091)
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
.elseif(n = 1061)
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
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && (n = 1094 || n = 1114 || (n > 1123 && n < 1135) || (n > 1148 && n < 1170) || (n > 1183 && n < 1205) || (n > 1218 && n < 1240)))
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
.elseif((n .mod 5 = 1) && (n = 1096 || n = 1106 || n = 1121 || n = 1126 || n = 1136 || (n > 1155 && n < 1167) || n = 1176 || (n > 1190 && n < 1207) || n = 1226 || n = 1231 || n = 1241 || n = 1246))
	; 8 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1096 && n < 1112) || (n > 1131 && n < 1147) || (n > 1166 && n < 1182) || (n > 1201 && n < 1217) || n = 1237 || n = 1244))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 3) && ((n > 1097 && n < 1249)))
	; 7 bytes
	sec
	lda #(n-3)/5
	sbc #1
	Jne *-2
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
.elseif((n .mod 5 = 2) && ((n > 1101 && n < 1128) || (n > 1136 && n < 1163) || (n > 1171 && n < 1198) || (n > 1206 && n < 1233) || n = 1242 || n = 1247))
	; 8 bytes
	delay_n (n-2)
	nop
.elseif(n = 1109 || n = 1116 || n = 1144 || n = 1151 || n = 1179 || n = 1186 || n = 1214 || n = 1221 || n = 1249)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1119)
	; 8 bytes
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
.elseif(n = 1131 || n = 1171 || n = 1211)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1141)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if(n = 1251 || n = 1272 || n = 1279 || n = 1286)
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif((n .mod 5 = 2) && ((n > 1251 && n < 1268) || (n > 1276 && n < 1288)))
	; 8 bytes
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
.elseif(n = 1256 || n = 1284)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif((n .mod 5 = 4) && ((n > 1258 && n < 1275)))
	; 8 bytes
	pla
	delay_n (n-4)
.elseif(n = 1261)
	; 8 bytes
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
.elseif(n = 1266 || n = 1271 || n = 1276 || n = 1281 || n = 1288)
	; 8 bytes
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
.endif
.else
; 1290 to 1328
.if(n = 1290)
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
.elseif(n = 1292 || n = 1294 || n = 1320 || n = 1322 || n = 1324)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1292 && n < 1329)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1295 || n = 1297)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1296)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif((n .mod 7 = 3) && ((n > 1297 && n < 1327)))
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1302 || n = 1316)
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
.elseif(n = 1306)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
	Jne *-4
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
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
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
.elseif(n = 1371)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
	tax
.elseif(n = 1372 || n = 1386)
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
.elseif(n = 1374 || n = 1388 || n = 1393)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1376 || n = 1390 || n = 1392 || n = 1406)
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
.elseif(n = 1379 || n = 1395)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
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
.elseif(n = 1399)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
.elseif(n = 1400)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1401)
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	brk
	adc #1
	Jpl *-5
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
.if(n = 1407 || n = 1423 || n = 1428 || n = 1430 || n = 1458 || n = 1465)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1408)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	iny
	Jne *-4
	tay
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
.elseif(n = 1414 || n = 1439 || n = 1442 || n = 1449 || n = 1456 || n = 1470 || n = 1474 || n = 1484)
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
.elseif(n = 1420 || n = 1422 || n = 1427 || n = 1450 || n = 1464 || n = 1476 || n = 1478)
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
.elseif(n = 1432)
	; 9 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 1441)
	; 9 bytes
	clc
	lda #184 ;hides 'clv'
	brk
	adc #1
	Jne *-5
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
.elseif(n = 1460)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 1462)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
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
.if(n = 1485 || n = 1493 || n = 1509)
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
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1509) || n = 1522))
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
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 8 bytes
	tya
	ldy #(n-5)/7
	nop
	dey
	Jne *-2
	tay
.elseif(n = 1490)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1491 || n = 1499 || n = 1507 || n = 1523)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1492 || n = 1518)
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
.elseif(n = 1498 || n = 1502 || n = 1505 || n = 1511 || n = 1512)
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
.elseif(n = 1525 || n = 1533 || n = 1541)
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
.elseif(n = 1527)
	; 7 bytes
	sec
	lda #(n-1)/7 ;hides 'nop'
	sbc #1
	Jne *-3
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
.elseif(n = 1530 || n = 1532 || n = 1546 || n = 1548 || n = 1558)
	; 8..9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1537 || n = 1556)
	; 9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1539 || n = 1547 || n = 1555)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1540 || n = 1554 || n = 1561)
	; 9 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1568 || n = 1574 || n = 1575 || n = 1582 || n = 1583 || n = 1590 || n = 1596 || n = 1610 || n = 1624 || n = 1638)
	; 9 bytes
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
.elseif(n = 1584 || n = 1591 || n = 1593 || n = 1609 || n = 1612 || n = 1617 || n = 1631)
	; 8..9 bytes
	pla
	delay_n (n-4)
.elseif(n = 1586)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 1632)
	; 9 bytes
	delay_n (n-4)
	pla
.elseif(n = 1633)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $B5A0'
	ldy #181
	dey
	Jne *-4
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
.elseif(n = 1642 || n = 1644 || n = 1656 || n = 1658 || n = 1674 || n = 1681 || n = 1700 || n = 1702 || n = 1712 || n = 1716)
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
.elseif(n = 1645 || n = 1682 || n = 1689 || n = 1703 || n = 1710)
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
.elseif(n = 1654 || n = 1684)
	; 9..10 bytes
	brk
	delay_n (n-13)
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
	; 8 bytes
	tya
	ldy #90 ;hides 'nop'
	pha
	iny
	Jne *-3
	tay
.elseif(n = 1665)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif((n .mod 7 = 0) && ((n > 1665 && n < 1695) || n = 1708))
	; 9 bytes
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
.elseif(n = 1696)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
.elseif(n = 1698)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	nop
	iny
	Jne *-5
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
.elseif(n = 1722 || n = 1729 || n = 1736 || n = 1750)
	; 9 bytes
	delay_n (n-2)
	nop
.elseif(n = 1723 || n = 1731 || n = 1747)
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1724)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1725 || n = 1732 || n = 1746)
	; 8 bytes
	sec
	lda #(n-3)/7
	nop
	sbc #1
	Jne *-3
.elseif(n = 1726 || n = 1730 || n = 1740 || n = 1742 || n = 1754)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 1728)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1733 || n = 1749 || n = 1757)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
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
.elseif(n = 1737)
	; 7 bytes
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
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
.if(n = 1758 || n = 1766 || n = 1792)
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
.elseif((n .mod 8 = 3) && ((n > 1762 && n < 1788)))
	; 8 bytes
	sec
	lda #(n-3)/8
	pha
	sbc #1
	Jne *-3
.elseif(n = 1764)
	; 8 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
.elseif(n = 1765 || n = 1773 || n = 1789)
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1768)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
.elseif(n = 1770 || n = 1782)
	; 9 bytes
	pha
	delay_n (n-3)
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
	cmp <$CD ;hides 'cmp $C5A0'
	ldy #197
	dey
	Jne *-4
	tay
.elseif(n = 1778 || n = 1796)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 1784)
	; 9 bytes
	pha
	tya
	ldy #n/8-1
	pha
	dey
	Jne *-2
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
.elseif(n = 1798 || n = 1807 || n = 1833 || n = 1834)
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
.elseif(n = 1801)
	; 9 bytes
	sec
	lda #(n-1)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
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
.elseif(n = 1804)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $C8A0'
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
.elseif(n = 1809)
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
.elseif(n = 1838 || n = 1847 || n = 1860 || n = 1871)
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
.elseif(n = 1849)
	; 9 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
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
.elseif(n = 1852 || n = 1873)
	; 9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 1858)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
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
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
.elseif((n .mod 8 = 5) && ((n > 1876 && n < 1902)))
	; 8 bytes
	tya
	ldy #(n-5)/8
	pha
	dey
	Jne *-2
	tay
.elseif(n = 1878 || n = 1880 || n = 1888 || n = 1896 || n = 1900 || n = 1914)
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
.elseif(n = 1897 || n = 1908)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 1898)
	; 10 bytes
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
.elseif(n = 1903)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	dey
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
.elseif(n = 1918 || n = 1926 || n = 1928 || n = 1932 || n = 1934 || n = 1936 || n = 1942 || n = 1944)
	; 9 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1930)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
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
.elseif(n = 1948)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
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
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
.elseif(n = 1977 || n = 1980)
	; 9..10 bytes
	pla
	delay_n (n-4)
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
.if(n = 1993)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
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
.elseif(n = 1999 || n = 2004 || n = 2023)
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
.elseif(n = 2002)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
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
.elseif(n = 2017)
	; 9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2026)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 9 bytes
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
.elseif(n = 2034)
	; 10 bytes
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
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
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	iny
	Jne *-4
	tay
.elseif(n = 2039 || n = 2048)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2040)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2041)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	brk
	adc #1
	Jpl *-5
.elseif(n = 2042 || n = 2050)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2044)
	; 10 bytes
	pha
	pla
	delay_n (n-7)
.elseif(n = 2046)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
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
.elseif(n = 2049)
	; 9 bytes
	pla
	delay_n (n-4)
.endif
.else
; 2051 to 2070
.if(n = 2051)
	; 8 bytes
	sec
	lda #0
	pha
	sbc #1
	Jne *-3
.elseif(n = 2052)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2053)
	; 8 bytes
	tya
	ldy #0
	pha
	dey
	Jne *-2
	tay
.elseif(n = 2054)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2055 || n = 2064)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 2056)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
.elseif(n = 2057 || n = 2066)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2058)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 10
	Jne *-4
	tay
.elseif(n = 2059)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2062)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
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
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
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
.elseif(n = 2074)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
.elseif(n = 2075 || n = 2084)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2076)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2077 || n = 2086 || n = 2088)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2078)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2079)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2080)
	; 10 bytes
	delay_n (n-4)
	pla
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
.elseif(n = 2085)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2087)
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
.endif
.else
; 2090 to 2109
.if(n = 2090)
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
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	tay
.elseif(n = 2093)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2094)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2095 || n = 2097)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2096)
	; 10 bytes
	pha
	delay_n (n-7)
	pla
.elseif(n = 2098)
	; 10 bytes
	delay_n (n-4)
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
	sec
	lda #(n-3)/9
	nop
	nop
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
.elseif(n = 2103)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2104)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2105)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
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
.elseif(n = 2108)
	; 10 bytes
	delay_n (n-2)
	nop
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
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2112 || n = 2121 || n = 2130 || n = 2144)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2113 || n = 2122 || n = 2124 || n = 2131 || n = 2140 || n = 2142)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2114 || n = 2132)
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
.elseif(n = 2123 || n = 2133)
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
.elseif(n = 2126 || n = 2148)
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
	ldy #236 ;hides 'cpx abs'
	cmp $C5
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
.elseif(n = 2137)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
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
	sec
	lda #(n-3)/9
	nop
	nop
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
.if((n >= 2149) && (n < 2168))
; 2149 to 2167
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
.elseif(n = 2151)
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
.elseif(n = 2153)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2154)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
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
.elseif(n = 2156 || n = 2165)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2157)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2158)
	; 9 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	bit $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2159)
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
.elseif(n = 2162)
	; 10 bytes
	sec
	lda #126
	jsr @nearby_rts
	sbc #1
	Jpl *-5
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
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	tay
.elseif(n = 2166)
	; 9 bytes
	pha
	delay_n (n-3)
.elseif(n = 2167)
	; 9 bytes
	pla
	delay_n (n-4)
.endif
.else
; 2168 to 2187
.if(n = 2168 || n = 2184 || n = 2186)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2169)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $48 ;hides 'pha'
	dey
	Jne *-4
	tay
.elseif(n = 2170)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2171)
	; 9 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	pha
	dey
	Jne *-4
	tay
.elseif(n = 2172)
	; 9 bytes
	sec
	lda #(n-3)/9
	nop
	nop
	sbc #1
	Jne *-4
.elseif(n = 2173)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
.elseif(n = 2174)
	; 9 bytes
	tya
	ldy #(n-5)/9
	nop
	nop
	dey
	Jne *-3
	tay
.elseif(n = 2175 || n = 2187)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2176)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2177)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2178)
	; 10 bytes
	pla
	delay_n (n-4)
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
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
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
.elseif(n = 2185)
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
.elseif(n = 2195 || n = 2205)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
	nop
	nop
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
.elseif(n = 2212)
	; 9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2215)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
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
.elseif(n = 2218)
	; 9 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
.elseif(n = 2220 || n = 2221)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2222)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2232)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
.elseif(n = 2240)
	; 9 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2306 || n = 2312 || n = 2316 || n = 2318 || n = 2322)
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
.elseif(n = 2310 || n = 2311)
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
.elseif(n = 2314)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	nop
	dey
	Jne *-5
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
.elseif(n = 2320)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	pha
	dey
	Jne *-5
	tay
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
	tya
	ldy #44 ;hides 'bit abs'
	inc $1A00,X ;hides 'nop'
	iny
	Jne *-5
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
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
.elseif(n = 2341)
	; 8 bytes
	sec
	lda #(n-1)/10 ;hides 'nop'
	pha
	sbc #1
	Jne *-4
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
.if((n >= 2344) && (n < 2363))
; 2344 to 2362
.if(n = 2344 || n = 2362)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2345 || n = 2357)
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
.elseif(n = 2347 || n = 2352)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2348)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2349)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2350)
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
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2354)
	; 10 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2355)
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
.elseif(n = 2360)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2361)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.endif
.else
; 2363 to 2382
.if(n = 2363)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2364 || n = 2376)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2365 || n = 2375)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2366)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	tay
.elseif(n = 2367 || n = 2379)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2368)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2369 || n = 2381)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2370)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2371)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2372)
	; 10 bytes
	clc
	lda #137 ;hides 'nop #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	adc #1
	Jne *-6
.elseif(n = 2373)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2374)
	; 10 bytes
	tya
	ldy #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 2377)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	nop
	sbc #1
	Jne *-5
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
.elseif(n = 2386 || n = 2388 || n = 2396 || n = 2398 || n = 2408)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2387 || n = 2394 || n = 2407 || n = 2419)
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
.elseif(n = 2397)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2399)
	; 9 bytes
	sec
	lda #(n-1)/11 ;hides 'nop'
	nop
	nop
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
	nop
	nop
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
.elseif(n = 2411)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2413)
	; 9 bytes
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	pha
	iny
	Jne *-4
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
.elseif(n = 2420)
	; 9 bytes
	clc
	lda #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	adc #1
	Jne *-5
.elseif(n = 2421)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	brk
	sbc #1
	Jmi *-5
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
.elseif(n = 2427 || n = 2451)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2428 || n = 2430 || n = 2432 || n = 2442 || n = 2444)
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
.elseif(n = 2433)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2434 || n = 2456)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
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
.elseif(n = 2440 || n = 2459)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2452)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 2454)
	; 10 bytes
	sec
	lda #(n-22)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
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
.if(n = 2461)
	; 9 bytes
	sec
	lda #250 ;hides 'nop'
	brk
	sbc #1
	Jmi *-5
.elseif(n = 2462)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif((n .mod 12 = 3) && ((n > 2462 && n < 2500)))
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2464)
	; 10 bytes
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
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
.elseif(n = 2466 || n = 2472 || n = 2474 || n = 2484 || n = 2486 || n = 2496 || n = 2498)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2467 || n = 2478 || n = 2500)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2468)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
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
.elseif(n = 2473 || n = 2493)
	; 9 bytes
	sec
	lda #(n-3)/10
	nop
	pha
	sbc #1
	Jne *-4
.elseif(n = 2476)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 2479)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2480 || n = 2491)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2481)
	; 8 bytes
	sec
	lda #248 ;hides 'sed'
	pha
	sbc #1
	Jne *-4
.elseif(n = 2482)
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
.elseif(n = 2485 || n = 2495)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
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
.endif
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
.if((n >= 2501) && (n < 2540))
.if((n >= 2501) && (n < 2520))
; 2501 to 2519
.if(n = 2501)
	; 8 bytes
	sec
	lda #(n-1)/10 ;hides 'nop'
	pha
	sbc #1
	Jne *-4
.elseif(n = 2502)
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
.elseif(n = 2504 || n = 2506 || n = 2516 || n = 2518)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2505)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2507)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2508)
	; 10 bytes
	pha
	tya
	ldy #250
	nop
	pha
	dey
	Jne *-3
	tay
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
.elseif(n = 2511)
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
.elseif(n = 2513)
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
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 2515)
	; 9 bytes
	tya
	ldy #(n-5)/10
	nop
	pha
	dey
	Jne *-3
	tay
.elseif(n = 2517)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2519)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.endif
.else
; 2520 to 2539
.if(n = 2520)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2521 || n = 2538)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2522)
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2523 || n = 2535)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2524)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2525 || n = 2537)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2526 || n = 2527)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2528)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2529)
	; 10 bytes
	pla
	delay_n (n-4)
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
	nop
	nop
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
	nop
	nop
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
.elseif(n = 2536)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2539)
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
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2541)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2542)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jmi *-6
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
.elseif(n = 2546 || n = 2557)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2547)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2548 || n = 2550)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2549)
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
.elseif(n = 2552)
	; 10 bytes
	sec
	lda #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jne *-6
.elseif(n = 2553)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	nop
	nop
	adc #1
	Jne *-5
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
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2558)
	; 9 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop $48,X ;hides 'pha'
	dey
	Jne *-4
	tay
.endif
.else
; 2559 to 2578
.if(n = 2559 || n = 2571)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2560 || n = 2576)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2561 || n = 2573)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2564)
	; 10 bytes
	sec
	lda #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jpl *-6
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
.elseif(n = 2568)
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2569)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 2574)
	; 10 bytes
	tya
	ldy #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jpl *-5
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
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	nop
	inx
	Jne *-5
	tax
.endif
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
.elseif(n = 2580)
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
.elseif(n = 2583 || n = 2595)
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
	cmp <$CD ;hides 'cmp $15A0'
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
.elseif(n = 2591)
	; 10 bytes
	pha
	sec
	lda #(n-6)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2592)
	; 10 bytes
	sec
	lda #130 ;hides 'nop #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
.elseif(n = 2593)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	pha
	sbc #1
	Jne *-5
.elseif(n = 2594)
	; 10 bytes
	tya
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 2596)
	; 9 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
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
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	nop
	dey
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
.elseif(n = 2602)
	; 10 bytes
	tya
	ldy #236 ;hides 'cpx abs'
	inc $1A00,X ;hides 'nop'
	dey
	Jne *-5
	tay
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
.elseif(n = 2607)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2611 || n = 2614)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2613)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2615)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
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
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2635 || n = 2636 || n = 2647)
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
.elseif(n = 2620)
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2624 || n = 2626 || n = 2648)
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
.elseif(n = 2627 || n = 2646)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2637)
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
.elseif(n = 2639)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 2658 || n = 2659 || n = 2671 || n = 2680 || n = 2695)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2660 || n = 2675)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2661 || n = 2673)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2662 || n = 2664)
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2682)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2683)
	; 9 bytes
	tya
	ldy #122 ;hides 'nop'
	brk
	iny
	Jne *-4
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
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2690)
	; 10 bytes
	tya
	ldy #12 ;hides 'nop abs'
	inc $1A00,X ;hides 'nop'
	iny
	Jne *-5
	tay
.elseif(n = 2692)
	; 10 bytes
	pha
	delay_n (n-3)
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
	; 9 bytes
	tya
	ldy #(n-5)/11
	delay_n 6
	dey
	Jne *-3
	tay
.elseif(n = 2701)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 2706)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2707)
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
	; 9 bytes
	sec
	lda #(n-3)/11
	delay_n 6
	sbc #1
	Jne *-4
.elseif(n = 2710)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
.elseif(n = 2712 || n = 2730)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2718)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2721)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2723)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
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
.elseif(n = 2736)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2737)
	; 10 bytes
	pla
	delay_n (n-4)
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
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2758)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 2780)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2781 || n = 2807)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2782 || n = 2794)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2783)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 2784)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 16
	sbc #1
	Jmi *-6
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
.elseif(n = 2793)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2795)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2796)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2798)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
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
.if((n >= 2813) && (n < 2832))
; 2813 to 2831
.if(n = 2813 || n = 2825)
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
.elseif(n = 2815 || n = 2827)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2816 || n = 2828)
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
.elseif(n = 2818)
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
.elseif(n = 2823)
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
.elseif(n = 2829)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2830)
	; 10 bytes
	sec
	lda #250 ;hides 'nop'
	delay_n 16
	sbc #1
	Jmi *-6
.elseif(n = 2831)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.endif
.else
; 2832 to 2851
.if(n = 2832)
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
.elseif(n = 2835 || n = 2847)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2836)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 2837 || n = 2849)
	; 9 bytes
	tya
	ldy #(n-5)/12
	delay_n 7
	dey
	Jne *-3
	tay
.elseif(n = 2838)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2839 || n = 2851)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2840)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2841)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2842)
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
.elseif(n = 2846)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2848)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2850)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.endif
.endif
.else
; 2852 to 2890
.if(n = 2852)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2853 || n = 2872)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2854 || n = 2866 || n = 2879 || n = 2881 || n = 2890)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2855 || n = 2874)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
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
.elseif(n = 2863 || n = 2869 || n = 2875 || n = 2887)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2864 || n = 2876 || n = 2886 || n = 2888)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2865)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2867)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2868)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2870)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2873)
	; 9 bytes
	txa
	pha
	lax #11
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 2880)
	; 9 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 16
	Jne *-4
	tay
.elseif(n = 2882)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 2884)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	pha
	iny
	Jne *-5
	tay
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
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2892 || n = 2894 || n = 2914 || n = 2918 || n = 2926)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2893 || n = 2910)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2895 || n = 2907 || n = 2919)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2896)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0FA0'
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
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 2899 || n = 2911 || n = 2923)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2900)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 2901)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2902)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 2903)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2904)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 2905 || n = 2925)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 2906 || n = 2922 || n = 2924)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2908)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	pha
	iny
	Jne *-5
	tay
.elseif(n = 2912)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2913)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 2915)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2916)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 2917)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 2920)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
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
.elseif(n = 2931 || n = 2943)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
.elseif(n = 2932)
	; 10 bytes
	tya
	cmp <$CD ;hides 'cmp $0CA0'
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
.elseif(n = 2934 || n = 2936)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2935 || n = 2941 || n = 2947)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2937)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 2946)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2948)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.endif
.else
; 2949 to 2968
.if(n = 2949)
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
.elseif(n = 2952 || n = 2953 || n = 2962)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2954)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 2955 || n = 2967)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
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
.elseif(n = 2961)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2963)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 2964)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
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
.elseif(n = 2970 || n = 2972 || n = 2976 || n = 2984 || n = 2994 || n = 2996)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3008)))
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 2973)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 2974)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
.elseif(n = 2975)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 2977)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	nop
	pha
	sbc #1
	Jne *-5
.elseif(n = 2978)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 2979 || n = 2991 || n = 3003)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2985 || n = 2999)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 2986)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 2987)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
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
.elseif(n = 2989 || n = 2998 || n = 3000 || n = 3002)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 2990)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
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
.elseif(n = 3009 || n = 3045)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3010 || (n > 3035 && n < 3039))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3011)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3012 || n = 3046)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3013)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3014)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
.elseif(n = 3022 || n = 3030 || n = 3032 || n = 3042 || n = 3044)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3023)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3024)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3025)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3026)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 3033)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 3051 || n = 3063)
	; 9 bytes
	sec
	lda #(n-3)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 3054 || n = 3056)
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
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.if(n = 3066 || n = 3078)
	; 10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3069)
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
.elseif(n = 3080)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3081)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 3087)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3088 || n = 3089 || n = 3091 || n = 3094 || n = 3096 || n = 3106 || n = 3107 || n = 3109 || n = 3115 || n = 3118 || n = 3124)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3090)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3092)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
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
.elseif(n = 3099 || n = 3117)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3100)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 3103 || n = 3123)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3104 || n = 3120 || n = 3122)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3113)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3114)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3116)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.if(n = 3126 || n = 3128 || n = 3142 || n = 3144 || n = 3146 || n = 3147 || n = 3152 || n = 3160 || n = 3161)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3127 || n = 3141)
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
.elseif(n = 3131 || n = 3148)
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
.elseif(n = 3133 || n = 3156 || n = 3158)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3134)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3135 || n = 3153)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3136)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3137 || n = 3155)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3138 || n = 3157)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3139)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3140 || n = 3159)
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
.elseif(n = 3145)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3149)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3150)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3151)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3154)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 18
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
.if(n = 3165 || n = 3182)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3166)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3167 || n = 3181)
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
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3172)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
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
	nop
	nop
	nop
	dey
	Jne *-5
	tay
.elseif(n = 3176)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
	sec
	lda #212 ;hides 'nop zp,x'
	bit $48 ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 3180)
	; 10 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	sbc #1
	Jne *-6
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
.elseif(n = 3186 || n = 3202)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
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
.elseif(n = 3189)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
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
.elseif(n = 3192)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $4848 ;hides two 'pha's
	dey
	Jne *-5
	tay
.elseif(n = 3193)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3194)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3195)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3196)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
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
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3199)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3200)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
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
; 3204 to 3242
.if(n = 3204 || n = 3206 || n = 3208 || n = 3217 || n = 3219 || n = 3220 || n = 3222 || n = 3232 || n = 3234 || n = 3236 || n = 3238)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3207 || n = 3225)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3209 || n = 3227)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3210)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3211 || n = 3239)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3212 || n = 3224 || n = 3226)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3213)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-5
	tay
.elseif(n = 3214)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3215 || n = 3230)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3216 || n = 3233)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3218 || n = 3235)
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
.elseif(n = 3228)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3229)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3231)
	; 10 bytes
	pla
	tya
	ldy #248 ;hides 'sed'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3237)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3240)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3241)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	nop
	delay_n 6
	sbc #1
	Jne *-6
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
.else
.if((n >= 3243) && (n < 3262))
; 3243 to 3261
.if(n = 3243 || n = 3261)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3244)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3245)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3246 || n = 3248 || n = 3256)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3247)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3249)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
.elseif(n = 3250)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3251)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
.elseif(n = 3252)
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
	ldy #250 ;hides 'nop'
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3254)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3255)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3257)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3258)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3259)
	; 10 bytes
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3260)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
.endif
.else
; 3262 to 3281
.if(n = 3262)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 3263 || n = 3281)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3264)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3265)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3266)
	; 10 bytes
	sec
	lda #(n-3)/13
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3267)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3268)
	; 10 bytes
	tya
	ldy #(n-5)/13
	nop
	delay_n 6
	dey
	Jne *-4
	tay
.elseif(n = 3269)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif((n .mod 2 = 0) && ((n > 3269 && n < 3277)))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3271)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3273)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3275)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3277)
	; 9 bytes
	sec
	lda #(n-1)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
.elseif(n = 3278)
	; 10 bytes
	clc
	lda #100 ;hides 'nop zp'
	nop
	brk
	adc #1
	Jne *-6
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
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3283)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3284 || n = 3301 || n = 3318)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.elseif(n = 3287 || n = 3296 || n = 3298 || n = 3308 || n = 3310 || (n > 3311 && n < 3315) || n = 3316)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3288)
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
.elseif(n = 3290)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 3293 || n = 3307)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3294)
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
.elseif(n = 3297 || n = 3315)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3299 || n = 3317)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3300)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
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
.endif
.else
; 3321 to 3359
.if(n = 3321)
	; 9 bytes
	clc
	lda #90 ;hides 'nop'
	brk
	adc #1
	Jne *-5
.elseif(n = 3322 || n = 3329 || n = 3332 || n = 3334 || n = 3340 || n = 3341 || n = 3346 || n = 3358 || n = 3359)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3323)
	; 9 bytes
	tya
	ldy #90 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 3324 || n = 3326 || n = 3336 || n = 3338 || n = 3356)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3325 || n = 3345)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3327 || n = 3339 || n = 3357)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3328 || n = 3347)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3330)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3331)
	; 10 bytes
	sec
	lda #0
	nop
	delay_n 6
	sbc #1
	Jne *-5
.elseif(n = 3333 || n = 3351)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3335 || n = 3353)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3337)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3342)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3343)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3344)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3348)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3349)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3350)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3352)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3354)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3355)
	; 10 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if((n > 3359 && n < 3363) || n = 3364 || n = 3370 || n = 3376 || n = 3381 || n = 3382 || n = 3394 || (n > 3395 && n < 3399))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3363 || n = 3377)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3365 || n = 3379 || n = 3393)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3366)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3367)
	; 10 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 10
	adc #1
	Jne *-6
.elseif(n = 3368)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3369)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3371 || n = 3389)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3372 || n = 3374 || n = 3390 || n = 3392)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3373)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3375 || n = 3391)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3378)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3380)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3383)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3384)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
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
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3387)
	; 9 bytes
	txa
	pha
	lax #13
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3388)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.endif
.else
; 3399 to 3437
.if((n > 3398 && n < 3403) || (n > 3411 && n < 3415) || (n > 3429 && n < 3433) || n = 3434 || n = 3436)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3403 || n = 3420 || n = 3437)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3404)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3405 || n = 3423)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3406)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
.elseif(n = 3407 || n = 3425)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3408)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3409)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
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
.elseif(n = 3419 || n = 3433)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3424)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 3426)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
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
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
	tay
.elseif(n = 3429)
	; 10 bytes
	pla
	delay_n (n-4)
.endif
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3453)
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
.elseif(n = 3440 || n = 3455 || n = 3470)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3441 || n = 3459)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3442)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 3446 || n = 3462 || n = 3464 || n = 3476)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3447)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3448 || n = 3450 || n = 3452 || n = 3460 || n = 3466 || n = 3467 || n = 3469 || n = 3472 || n = 3474)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3449 || n = 3463)
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
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
.elseif(n = 3456)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3457)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 3458)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3468)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-5
	tax
.elseif(n = 3471)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3495 || n = 3513)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3478 || n = 3504 || n = 3506)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3479 || n = 3497 || n = 3515)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3480)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3481)
	; 10 bytes
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
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3484 || n = 3486 || (n > 3491 && n < 3495) || n = 3496 || n = 3502 || n = 3508 || n = 3509 || n = 3514)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3485)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3487)
	; 10 bytes
	clc
	lda #90 ;hides 'nop'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
.elseif(n = 3488)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3489)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3490 || n = 3507)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3491 || n = 3505)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3498)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
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
.elseif(n = 3510)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3511)
	; 10 bytes
	sec
	lda #234 ;hides 'nop'
	nop
	delay_n 6
	sbc #1
	Jne *-6
.elseif(n = 3512)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3534 || n = 3536 || n = 3570 || n = 3572)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3517 || n = 3545 || n = 3559 || n = 3573)
	; 10 bytes
	sec
	lda #(n-3)/14
	delay_n 9
	sbc #1
	Jne *-5
.elseif(n = 3518 || n = 3537)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3519 || n = 3547 || n = 3561 || n = 3575)
	; 10 bytes
	tya
	ldy #(n-5)/14
	delay_n 9
	dey
	Jne *-4
	tay
.elseif(n = 3520 || n = 3577)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3521 || n = 3526 || n = 3527 || n = 3529 || n = 3532 || n = 3538 || n = 3544 || n = 3546 || n = 3548 || n = 3550 || n = 3557 || n = 3562 || n = 3564 || n = 3566 || n = 3568 || n = 3574 || n = 3576 || (n > 3577 && n < 3583) || n = 3584 || n = 3586 || n = 3593)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3522 || n = 3539 || n = 3556 || n = 3590)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3523 || n = 3543 || n = 3563 || n = 3583)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3524 || n = 3541 || n = 3558 || n = 3592)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3525 || n = 3565)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3528 || n = 3588)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3530 || n = 3560)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3531 || n = 3549 || n = 3567 || n = 3585)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3533 || n = 3551 || n = 3569 || n = 3587)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3535 || n = 3553 || n = 3571)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3540)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	adc #1
	Jne *-6
.elseif(n = 3542)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	iny
	Jne *-5
	tay
.elseif(n = 3552)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3554)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3555 || n = 3591)
	; 10 bytes
	pla
	delay_n (n-4)
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
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if(n = 3594 || n = 3613 || n = 3632 || n = 3651 || n = 3670)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3595 || (n > 3596 && n < 3603) || n = 3604 || (n > 3609 && n < 3613) || n = 3614 || n = 3616 || n = 3619 || n = 3622 || (n > 3627 && n < 3632) || n = 3637 || n = 3640 || n = 3649 || n = 3652 || n = 3654 || n = 3656 || (n > 3665 && n < 3670) || n = 3671)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3596 || n = 3615 || n = 3634 || n = 3653)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3603 || n = 3621 || n = 3639)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3605 || n = 3623 || n = 3641 || n = 3659)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3606 || n = 3608 || n = 3642 || n = 3647 || n = 3660)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3607 || n = 3624 || n = 3658)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3609 || n = 3626 || n = 3643)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3617 || n = 3638)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3618 || n = 3633)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3620 || n = 3635 || n = 3650)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3625 || n = 3645 || n = 3665)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3627 || n = 3648 || n = 3661)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3636)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
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
.elseif(n = 3657)
	; 8 bytes
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
.elseif(n = 3662)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $48,X ;hides 'pha'
	pha
	dey
	Jne *-5
	tay
.elseif(n = 3663)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3664)
	; 10 bytes
	delay_n (n-4)
	pla
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3691 || n = 3710 || n = 3748)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3673)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 3674 || n = 3676 || n = 3682 || n = 3688 || n = 3690 || n = 3700 || n = 3702 || n = 3704 || n = 3706 || n = 3712 || n = 3718 || n = 3719 || n = 3724 || n = 3730 || n = 3736 || n = 3737 || n = 3739 || n = 3742 || n = 3744)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3675 || n = 3693 || n = 3711 || n = 3729 || n = 3747)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3677 || n = 3695 || n = 3713 || n = 3731 || n = 3749)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3678 || n = 3738)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3679 || n = 3697 || n = 3733)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3680 || n = 3740)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3681)
	; 9 bytes
	sec
	lda #184 ;hides 'clv'
	brk
	sbc #1
	Jne *-5
.elseif(n = 3683)
	; 9 bytes
	tya
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 3684 || n = 3686 || n = 3696 || n = 3698 || n = 3714 || n = 3716 || n = 3732 || n = 3734 || n = 3750)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3685 || n = 3705 || n = 3725)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3687 || n = 3717 || n = 3735)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3689 || n = 3708 || n = 3727 || n = 3746)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3692 || n = 3709 || n = 3726 || n = 3743)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3694 || n = 3728)
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
.elseif(n = 3701 || n = 3722)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3703 || n = 3723)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3707)
	; 10 bytes
	sec
	lda #(n-1)/17 ;hides 'nop'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 3715)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
.elseif(n = 3720 || n = 3741)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
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
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
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
.if(n = 3751 || n = 3769 || n = 3787)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3752 || n = 3788)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3753 || n = 3768 || n = 3798)
	; 10 bytes
	sec
	lda #(n-3)/15
	delay_n 10
	sbc #1
	Jne *-5
.elseif(n = 3754 || (n > 3755 && n < 3760) || n = 3761 || n = 3766 || n = 3773 || n = 3775 || n = 3778 || (n > 3779 && n < 3783) || (n > 3789 && n < 3794) || n = 3797 || n = 3799 || n = 3802 || n = 3808 || n = 3810 || n = 3812 || n = 3814 || (n > 3815 && n < 3819) || n = 3820 || n = 3826)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3755 || n = 3770 || n = 3800 || n = 3815)
	; 10 bytes
	tya
	ldy #(n-5)/15
	delay_n 10
	dey
	Jne *-4
	tay
.elseif(n = 3760 || n = 3777 || n = 3794 || n = 3811 || n = 3828)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3762 || n = 3779 || n = 3796 || n = 3813)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3763 || n = 3823)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3764 || n = 3806 || n = 3827)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3765 || n = 3783 || n = 3801 || n = 3819)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3767 || n = 3785 || n = 3803 || n = 3821)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3771 || n = 3789 || n = 3807)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 3776)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
.elseif(n = 3784 || n = 3822)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3786 || n = 3805 || n = 3824)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3804)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3809)
	; 10 bytes
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.endif
.else
; 3829 to 3906
.if(n = 3829 || (n > 3830 && n < 3834) || n = 3835 || n = 3836 || n = 3838 || n = 3844 || (n > 3848 && n < 3855) || n = 3856 || n = 3866 || n = 3868 || n = 3870 || n = 3871 || n = 3880 || n = 3882 || n = 3884 || n = 3886 || n = 3887 || n = 3889 || n = 3892 || n = 3899 || n = 3906)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3830 || n = 3847 || n = 3864 || n = 3881 || n = 3898)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3834)
	; 10 bytes
	tya
	ldy #192 ;hides 'cpy #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 3837 || n = 3855 || n = 3873 || n = 3891)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3839 || n = 3857 || n = 3875 || n = 3893)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3840 || n = 3842 || n = 3858 || n = 3876 || n = 3878 || n = 3894 || n = 3904)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3841)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3843 || n = 3900)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3845 || n = 3862 || n = 3879 || n = 3896)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3846 || n = 3888)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3848 || n = 3869 || n = 3890)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3859 || n = 3877 || n = 3895 || n = 3903)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3860)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
.elseif(n = 3861 || n = 3897 || n = 3905)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3863 || n = 3883)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3865 || n = 3885)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3867)
	; 10 bytes
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 3872)
	; 10 bytes
	sec
	lda #194 ;hides 'nop #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
.elseif(n = 3874)
	; 10 bytes
	tya
	ldy #194 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
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
.endif
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
.elseif(n = 3908 || n = 3910 || n = 3916 || n = 3918 || (n > 3919 && n < 3923) || n = 3924 || n = 3926 || n = 3928 || n = 3934 || n = 3935 || n = 3937 || n = 3940 || n = 3942 || n = 3944 || n = 3946 || n = 3954 || n = 3956 || (n > 3957 && n < 3961) || n = 3962 || n = 3970 || n = 3975 || n = 3977 || n = 3978 || n = 3980 || n = 3982 || (n > 3987 && n < 3993) || n = 3994 || n = 3996 || n = 3997 || (n > 4007 && n < 4012) || n = 4013 || n = 4018 || n = 4024 || (n > 4025 && n < 4031) || n = 4036 || n = 4042 || n = 4044 || (n > 4045 && n < 4049) || n = 4054 || (n > 4059 && n < 4063))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 3909 || n = 3927 || n = 3945 || n = 3963 || n = 3981 || n = 3999 || n = 4017 || n = 4035 || n = 4053)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 3911 || n = 3929 || n = 3947 || n = 3965 || n = 3983 || n = 4001 || n = 4019 || n = 4037 || n = 4055)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 3912 || n = 3914 || n = 3948 || n = 3968 || n = 3984 || n = 3986 || n = 4002 || n = 4004 || n = 4006 || n = 4020 || n = 4022 || n = 4038 || n = 4040)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3913 || n = 3930 || n = 3964 || n = 3998 || n = 4015 || n = 4032 || n = 4049)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 3915 || n = 3932 || n = 3949 || n = 3966 || n = 4000 || n = 4034 || n = 4051)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 3917 || n = 3936 || n = 3955 || n = 3974 || n = 3993 || n = 4012 || n = 4031 || n = 4050)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 3919 || n = 3938 || n = 3957 || n = 3976 || n = 3995 || n = 4014 || n = 4033 || n = 4052)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 3923 || n = 3943 || n = 4023 || n = 4043)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 3925 || n = 3985 || n = 4005 || n = 4025 || n = 4045)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 3931 || n = 3967 || n = 4021 || n = 4039 || n = 4057)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 3933 || n = 3951 || n = 3987 || n = 4007 || n = 4041 || n = 4059)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 3939)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	adc #1
	Jne *-6
.elseif(n = 3941)
	; 10 bytes
	txa
	ldx #232 ;hides 'inx'
	brk
	inx
	inx
	Jne *-5
	tax
.elseif(n = 3950)
	; 10 bytes
	clc
	lda #68 ;hides 'nop zp'
	nop
	brk
	adc #1
	Jne *-6
.elseif(n = 3952)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3953 || n = 4016 || n = 4058)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 3961)
	; 9 bytes
	clc
	lda #58 ;hides 'nop'
	brk
	adc #1
	Jne *-5
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
.elseif(n = 3972 || n = 4056)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 3973)
	; 10 bytes
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 3979)
	; 10 bytes
	sec
	lda #(n-1)/17 ;hides 'nop'
	delay_n 10
	sbc #1
	Jne *-6
.elseif(n = 4003)
	; 9 bytes
	tya
	ldy #56 ;hides 'sec'
	brk
	iny
	Jne *-4
	tay
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
; 4063 to 4218
.if(n = 4063)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
.elseif(n = 4064 || n = 4070 || n = 4072 || n = 4078 || (n > 4079 && n < 4083) || n = 4084 || n = 4086 || n = 4087 || n = 4096 || n = 4097 || n = 4101 || n = 4104 || n = 4106 || n = 4108 || (n > 4113 && n < 4117) || n = 4118 || n = 4122 || n = 4124 || n = 4132 || n = 4133 || n = 4135 || (n > 4136 && n < 4140) || n = 4144 || n = 4150 || n = 4152 || (n > 4153 && n < 4158) || n = 4169 || (n > 4170 && n < 4179) || n = 4180 || n = 4186 || (n > 4187 && n < 4197) || n = 4198 || (n > 4205 && n < 4215) || n = 4216)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4065 || n = 4165 || n = 4205)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4066 || n = 4083 || n = 4100 || n = 4117 || n = 4134 || n = 4151 || n = 4168 || n = 4185 || n = 4202)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 4067 || n = 4077 || n = 4095 || n = 4113 || n = 4131 || n = 4149 || n = 4162 || n = 4167)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4068 || n = 4085 || n = 4102 || n = 4119 || n = 4136 || n = 4153 || n = 4170 || n = 4187 || n = 4204)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4069 || n = 4088 || n = 4126 || n = 4164)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4071 || n = 4089 || n = 4107 || n = 4125 || n = 4143 || n = 4161 || n = 4179 || n = 4197 || n = 4215)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 4073 || n = 4091 || n = 4109 || n = 4127 || n = 4145 || n = 4163 || n = 4181 || n = 4199 || n = 4217)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4074 || n = 4076 || n = 4092 || n = 4094 || n = 4110 || n = 4112 || n = 4146 || n = 4148 || n = 4200 || n = 4218)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4075 || n = 4093 || n = 4111 || n = 4129 || n = 4160 || n = 4201)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4079 || n = 4121 || n = 4142 || n = 4184)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4090 || n = 4128 || n = 4147 || n = 4166)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4098 || n = 4140 || n = 4182)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4099)
	; 10 bytes
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4103 || n = 4123 || n = 4183)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4105)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	sbc #1
	Jne *-6
.elseif(n = 4120)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4130)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4141)
	; 10 bytes
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 4203)
	; 10 bytes
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
.endif
.else
; 4219 to 4375
.if(n = 4219 || n = 4236 || n = 4270 || n = 4304 || n = 4338)
	; 10 bytes
	sec
	lda #(n-3)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
.elseif(n = 4220 || n = 4254 || n = 4256 || n = 4274 || n = 4290 || n = 4292 || n = 4324 || n = 4326 || n = 4328 || n = 4344 || n = 4346 || n = 4362 || n = 4364 || n = 4366)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4221 || n = 4238 || n = 4255 || n = 4272 || n = 4306 || n = 4340)
	; 10 bytes
	tya
	ldy #(n-5)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
.elseif(n = 4222 || (n > 4226 && n < 4233) || n = 4234 || n = 4241 || n = 4244 || n = 4246 || n = 4248 || n = 4249 || n = 4252 || n = 4258 || n = 4260 || n = 4262 || n = 4264 || n = 4267 || n = 4276 || n = 4277 || n = 4279 || n = 4281 || n = 4282 || n = 4284 || n = 4286 || n = 4288 || (n > 4293 && n < 4297) || n = 4298 || (n > 4299 && n < 4303) || (n > 4311 && n < 4316) || n = 4317 || n = 4319 || n = 4320 || n = 4322 || n = 4330 || (n > 4331 && n < 4335) || n = 4336 || n = 4339 || n = 4342 || n = 4348 || n = 4349 || n = 4351 || n = 4353 || n = 4358 || n = 4360 || (n > 4367 && n < 4371) || n = 4372 || n = 4374)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4223 || n = 4242 || n = 4261 || n = 4280 || n = 4299 || n = 4318 || n = 4337 || n = 4356 || n = 4375)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4224 || n = 4308 || n = 4350)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4225 || n = 4245 || n = 4265 || n = 4285 || n = 4345 || n = 4365)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4226 || n = 4247 || n = 4268 || n = 4310 || n = 4331 || n = 4352)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4233 || n = 4251 || n = 4269 || n = 4287 || n = 4305 || n = 4323 || n = 4341 || n = 4359)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 4235 || n = 4253 || n = 4271 || n = 4289 || n = 4307 || n = 4325 || n = 4343 || n = 4361)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4237 || n = 4273 || n = 4291 || n = 4309 || n = 4327)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4239 || n = 4257 || n = 4275 || n = 4293 || n = 4311 || n = 4329 || n = 4347 || n = 4367)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4240 || n = 4259 || n = 4278 || n = 4297 || n = 4316 || n = 4335 || n = 4354 || n = 4373)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4243 || n = 4263 || n = 4283 || n = 4303)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4250)
	; 10 bytes
	tya
	ldy #136 ;hides 'dey'
	delay_n 26
	Jne *-5
	tay
.elseif(n = 4266)
	; 9 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
	tax
.elseif(n = 4321)
	; 9 bytes
	sec
	lda #216 ;hides 'cld'
	brk
	sbc #1
	Jne *-5
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
.elseif(n = 4363)
	; 9 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
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
.endif
.else
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
; 4376 to 4531
.if(n = 4376 || n = 4378 || n = 4384 || (n > 4385 && n < 4392) || n = 4393 || n = 4396 || n = 4402 || n = 4404 || (n > 4405 && n < 4409) || n = 4410 || n = 4412 || n = 4414 || (n > 4419 && n < 4423) || n = 4424 || (n > 4425 && n < 4430) || (n > 4437 && n < 4443) || n = 4444 || n = 4446 || n = 4448 || n = 4450 || n = 4456 || (n > 4457 && n < 4463) || n = 4464 || n = 4466 || n = 4475 || n = 4477 || n = 4479 || n = 4481 || n = 4482 || n = 4484 || n = 4486 || (n > 4491 && n < 4497) || n = 4498 || (n > 4499 && n < 4503) || n = 4504 || n = 4510 || n = 4511 || (n > 4514 && n < 4518) || n = 4519 || n = 4522 || (n > 4527 && n < 4532))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4377 || n = 4395 || n = 4413 || n = 4431 || n = 4449 || n = 4467 || n = 4485 || n = 4503 || n = 4521)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 4379 || n = 4397 || n = 4433 || n = 4451 || n = 4469 || n = 4487 || n = 4505 || n = 4523)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4380 || n = 4382 || n = 4398 || n = 4400 || n = 4416 || n = 4418 || n = 4452 || n = 4454 || n = 4472 || n = 4488 || n = 4490 || n = 4524 || n = 4526)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4381 || n = 4399 || n = 4417 || n = 4435 || n = 4453 || n = 4471 || n = 4507)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4383 || n = 4423 || n = 4443 || n = 4463 || n = 4483)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4385 || n = 4405 || n = 4425 || n = 4445 || n = 4465 || n = 4525)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4392 || n = 4430 || n = 4468 || n = 4506)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4394 || n = 4432 || n = 4470 || n = 4489 || n = 4508 || n = 4527)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4401 || n = 4419 || n = 4437 || n = 4455 || n = 4473 || n = 4509)
	; 10 bytes
	pla
	delay_n (n-4)
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
.elseif(n = 4434 || n = 4476 || n = 4497 || n = 4518)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4436 || n = 4457 || n = 4499 || n = 4520)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4447)
	; 10 bytes
	sec
	lda #(n-1)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
.elseif(n = 4474)
	; 10 bytes
	tya
	ldy #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4478)
	; 10 bytes
	clc
	lda #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	adc #1
	Jne *-6
.elseif(n = 4480)
	; 10 bytes
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
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
.elseif(n = 4512)
	; 10 bytes
	sec
	lda #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	sbc #1
	Jne *-6
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
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4537) || n = 4538 || n = 4540 || (n > 4546 && n < 4555) || n = 4556 || n = 4558 || n = 4564 || (n > 4565 && n < 4575) || n = 4576 || n = 4594)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4537)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
.elseif(n = 4539 || n = 4557 || n = 4575 || n = 4593)
	; 9 bytes
	sec
	lda #(n-3)/18
	brk
	sbc #1
	Jne *-4
.elseif(n = 4541 || n = 4559 || n = 4577 || n = 4595)
	; 9 bytes
	tya
	ldy #(n-5)/18
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4542 || n = 4578 || n = 4580 || n = 4587 || n = 4596 || n = 4598 || n = 4605)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4543 || n = 4583)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4544 || n = 4563 || n = 4582)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4545 || n = 4585)
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
.elseif(n = 4555)
	; 10 bytes
	clc
	lda #58 ;hides 'nop'
	delay_n 16
	adc #1
	Jne *-6
.elseif(n = 4560)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4561 || n = 4586 || n = 4589 || n = 4597 || n = 4604)
	; 9..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4562)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4579)
	; 10 bytes
	sec
	lda #(n-1)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
.elseif(n = 4581)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
.elseif(n = 4584)
	; 8 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
.elseif(n = 4588 || n = 4599 || n = 4606)
	; 9..10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4590 || n = 4608)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4591 || n = 4609)
	; 10 bytes
	delay_n (n-4)
	pla
.elseif(n = 4592)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4600)
	; 8 bytes
	sec
	lda #0 ;hides 'brk imm'
	nop
	sbc #1
	Jne *-4
.elseif(n = 4601)
	; 9 bytes
	clc
	lda #26 ;hides 'nop'
	brk
	adc #1
	Jne *-5
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
	pha
	delay_n (n-7)
	pla
.endif
.else
; 4610 to 4687
.if(n = 4610)
	; 10 bytes
	delay_n 8
	delay_n (n-8)
.elseif(n = 4611)
	; 9 bytes
	sec
	lda #0
	brk
	sbc #1
	Jne *-4
.elseif(n = 4612 || n = 4618 || n = 4619 || n = 4621 || (n > 4625 && n < 4639) || n = 4640 || n = 4642 || (n > 4647 && n < 4654) || n = 4655 || n = 4657 || n = 4659 || n = 4661 || n = 4662 || n = 4664 || n = 4666 || (n > 4667 && n < 4672) || n = 4673 || n = 4680 || n = 4682)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4613)
	; 9 bytes
	tya
	ldy #0
	brk
	dey
	Jne *-3
	tay
.elseif(n = 4614 || n = 4616 || n = 4675 || n = 4684 || n = 4686)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4615 || n = 4674)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4617 || n = 4647 || n = 4676 || n = 4687)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4620 || n = 4639 || n = 4658 || n = 4677)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4622 || n = 4660 || n = 4679)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4623 || n = 4663)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4624)
	; 10 bytes
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 4625 || n = 4645 || n = 4665 || n = 4685)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4641)
	; 9 bytes
	clc
	lda #24 ;hides 'clc'
	brk
	adc #1
	Jne *-5
.elseif(n = 4643)
	; 9 bytes
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	tay
.elseif(n = 4644)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4646)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4654)
	; 10 bytes
	sec
	lda #212 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
.elseif(n = 4656)
	; 10 bytes
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4667)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	brk
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
.elseif(n = 4678)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4681)
	; 9 bytes
	sec
	lda #(n-1)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
.elseif(n = 4683)
	; 9 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if(n = 4688 || n = 4709 || n = 4730 || n = 4814 || n = 4835)
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
	ldy #213 ;hides 'cmp zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif((n > 4689 && n < 4696) || n = 4697 || (n > 4698 && n < 4703) || n = 4704 || n = 4706 || n = 4708 || n = 4710 || n = 4712 || n = 4714 || n = 4716 || (n > 4717 && n < 4723) || n = 4724 || n = 4726 || n = 4727 || n = 4729 || (n > 4730 && n < 4734) || n = 4735 || (n > 4736 && n < 4743) || n = 4744 || (n > 4745 && n < 4749) || n = 4750 || n = 4752 || n = 4754 || (n > 4755 && n < 4763) || n = 4764 || (n > 4765 && n < 4770) || n = 4773 || (n > 4774 && n < 4781) || n = 4782 || n = 4784 || (n > 4785 && n < 4791) || (n > 4793 && n < 4797) || n = 4798 || (n > 4799 && n < 4803) || n = 4804 || (n > 4805 && n < 4810) || n = 4811 || (n > 4814 && n < 4823) || n = 4824 || (n > 4825 && n < 4829) || n = 4830 || n = 4832 || n = 4834 || (n > 4835 && n < 4843))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4696 || n = 4734 || n = 4753 || n = 4772 || n = 4791 || n = 4810 || n = 4829)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4698 || n = 4717 || n = 4736 || n = 4755 || n = 4774 || n = 4793 || n = 4812 || n = 4831)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4703 || n = 4723 || n = 4743 || n = 4763 || n = 4783 || n = 4803 || n = 4823 || n = 4843)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 4705 || n = 4725 || n = 4745 || n = 4765 || n = 4785 || n = 4805 || n = 4825)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4707 || n = 4728 || n = 4749 || n = 4770)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4711)
	; 10 bytes
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4713)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts
	sbc #1
	Jne *-6
.elseif(n = 4715)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
.elseif(n = 4751)
	; 10 bytes
	sec
	lda #(n-1)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
.elseif(n = 4771)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
.elseif(n = 4781)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 4792)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	brk
	dey
	Jne *-5
	tay
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
.elseif(n = 4813)
	; 10 bytes
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
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
.endif
.else
; 4844 to 5000
.if(n = 4844 || n = 4846 || n = 4847 || n = 4849 || (n > 4850 && n < 4854) || n = 4855 || (n > 4856 && n < 4863) || n = 4864 || n = 4866 || n = 4868 || (n > 4869 && n < 4873) || n = 4874 || n = 4876 || (n > 4877 && n < 4883) || n = 4884 || (n > 4885 && n < 4896) || n = 4897 || (n > 4898 && n < 4903) || n = 4904 || (n > 4905 && n < 4915) || n = 4916 || n = 4918 || (n > 4919 && n < 4923) || n = 4924 || (n > 4925 && n < 4929) || n = 4930 || (n > 4933 && n < 4938) || n = 4939 || n = 4941 || n = 4942 || n = 4944 || (n > 4945 && n < 4959) || n = 4960 || n = 4962 || n = 4968 || n = 4970 || (n > 4971 && n < 4980) || n = 4981 || n = 4984 || (n > 4985 && n < 5001))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 4845 || n = 4865 || n = 4885 || n = 4905 || n = 4925 || n = 4945 || n = 4965 || n = 4985)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 4848)
	; 10 bytes
	sec
	lda #(n-3)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
.elseif(n = 4850)
	; 10 bytes
	tya
	ldy #(n-5)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
.elseif(n = 4854 || n = 4896 || n = 4938 || n = 4959 || n = 4980)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 4856 || n = 4877 || n = 4898 || n = 4919 || n = 4940 || n = 4982)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 4863 || n = 4883 || n = 4903 || n = 4923 || n = 4943 || n = 4983)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
.elseif(n = 4915)
	; 10 bytes
	sec
	lda #(n-1)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
.elseif(n = 4917)
	; 10 bytes
	tya
	ldy #(n-3)/21 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
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
.elseif(n = 4931)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 4932 || n = 4964 || n = 4966)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 4933 || n = 4967)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 4961)
	; 9 bytes
	sec
	lda #248 ;hides 'sed'
	brk
	sbc #1
	Jne *-5
.elseif(n = 4963)
	; 9 bytes
	tya
	ldy #248 ;hides 'sed'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 4969)
	; 10 bytes
	sec
	lda #216 ;hides 'cld'
	delay_n 16
	sbc #1
	Jne *-6
.elseif(n = 4971)
	; 10 bytes
	tya
	ldy #216 ;hides 'cld'
	delay_n 16
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
.if((n >= 5001) && (n < 7501))
.if((n >= 5001) && (n < 6251))
.if((n >= 5001) && (n < 5626))
.if((n >= 5001) && (n < 5313))
.if((n >= 5001) && (n < 5157))
; 5001 to 5156
.if(n = 5001)
	; 9 bytes
	sec
	lda #(n-1)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
.elseif(n = 5002 || (n > 5007 && n < 5015) || n = 5016 || (n > 5017 && n < 5022) || (n > 5025 && n < 5043) || n = 5044 || (n > 5045 && n < 5061) || n = 5062 || (n > 5066 && n < 5078) || (n > 5078 && n < 5083) || n = 5084 || n = 5086 || (n > 5087 && n < 5094) || n = 5095 || (n > 5098 && n < 5103) || n = 5104 || n = 5109 || n = 5110 || (n > 5118 && n < 5123) || n = 5124 || n = 5126 || n = 5128 || (n > 5129 && n < 5148) || (n > 5151 && n < 5157))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5003)
	; 9 bytes
	tya
	ldy #(n-3)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
.elseif(n = 5004 || n = 5006 || n = 5097 || n = 5117)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5005 || n = 5025 || n = 5045 || n = 5065 || n = 5085 || n = 5105)
	; 10 bytes
	tya
	ldy #(n-5)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
.elseif(n = 5007 || n = 5098 || n = 5118)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 5015)
	; 10 bytes
	sec
	lda #(n-1)/23 ;hides 'nop'
	delay_n 16
	sbc #1
	Jne *-6
.elseif(n = 5017)
	; 10 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5022 || n = 5064 || n = 5106 || n = 5127 || n = 5148)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 5023 || n = 5043 || n = 5063 || n = 5083 || n = 5103)
	; 10 bytes
	sec
	lda #(n-3)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
.elseif(n = 5024 || n = 5066 || n = 5087 || n = 5108 || n = 5129 || n = 5150)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
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
.elseif(n = 5078)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	tax
.elseif(n = 5094)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
.elseif(n = 5096 || n = 5115 || n = 5116)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5107)
	; 10 bytes
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
.elseif(n = 5111)
	; 9 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	sbc #1
	Jne *-5
.elseif(n = 5112)
	; 9 bytes
	sec
	lda #0 ;hides 'brk imm'
	nop
	nop
	sbc #1
	Jne *-5
.elseif(n = 5113)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	iny
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
.elseif(n = 5149)
	; 10 bytes
	sec
	lda #(n-1)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
.elseif(n = 5151)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.else
; 5157 to 5312
.if((n > 5156 && n < 5169) || n = 5170 || n = 5172 || (n > 5173 && n < 5182) || n = 5183 || n = 5185 || n = 5187 || n = 5191 || n = 5194 || (n > 5195 && n < 5209) || n = 5210 || n = 5212 || (n > 5213 && n < 5217) || (n > 5217 && n < 5232) || n = 5233 || (n > 5234 && n < 5251) || n = 5252 || n = 5254 || (n > 5255 && n < 5274) || n = 5275 || (n > 5276 && n < 5291) || n = 5292 || n = 5294 || n = 5296 || (n > 5297 && n < 5313))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5169 || n = 5232 || n = 5274 || n = 5295)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 5171 || n = 5192 || n = 5213 || n = 5234 || n = 5255 || n = 5276 || n = 5297)
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
.elseif(n = 5182)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	adc #1
	Jne *-6
.elseif(n = 5184)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
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
.elseif(n = 5188)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5189)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5190)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 5193)
	; 10 bytes
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	brk
	adc #1
	Jne *-6
.elseif(n = 5195)
	; 10 bytes
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
	tay
.elseif(n = 5209)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
.elseif(n = 5211)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 5291)
	; 10 bytes
	clc
	lda #26 ;hides 'nop'
	delay_n 16
	adc #1
	Jne *-6
.elseif(n = 5293)
	; 10 bytes
	tya
	ldy #26 ;hides 'nop'
	delay_n 16
	iny
	Jne *-5
	tay
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
; 5313 to 5468
.if((n > 5312 && n < 5316) || n = 5317 || (n > 5318 && n < 5337) || n = 5338 || (n > 5339 && n < 5349) || n = 5350 || (n > 5353 && n < 5358) || n = 5359 || (n > 5360 && n < 5366) || (n > 5374 && n < 5379) || n = 5380 || n = 5382 || n = 5384 || (n > 5385 && n < 5393) || (n > 5393 && n < 5443) || n = 5444 || (n > 5447 && n < 5457) || n = 5458 || (n > 5459 && n < 5469))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5316 || n = 5337)
	; 10 bytes
	sec
	lda #(n-3)/21
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 5318 || n = 5339)
	; 10 bytes
	tya
	ldy #(n-5)/21
	delay_n 16
	dey
	Jne *-4
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
.elseif(n = 5351 || n = 5372 || n = 5445)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5352 || n = 5373 || n = 5446)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5353 || n = 5374 || n = 5447)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 5358)
	; 10 bytes
	sec
	lda #244 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
.elseif(n = 5360)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 5366)
	; 9 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	sbc #1
	Jne *-5
.elseif(n = 5367)
	; 9 bytes
	clc
	lda #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	adc #1
	Jne *-5
.elseif(n = 5368)
	; 9 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
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
	ldy #244 ;hides 'nop zp,x'
	nop
	brk
	dey
	Jne *-5
	tay
.elseif(n = 5379)
	; 10 bytes
	sec
	lda #0
	delay_n 16
	sbc #1
	Jne *-5
.elseif(n = 5381)
	; 10 bytes
	tya
	ldy #0
	delay_n 16
	dey
	Jne *-4
	tay
.elseif(n = 5383)
	; 10 bytes
	sec
	lda #(n-1)/23 ;hides 'nop'
	delay_n 16
	sbc #1
	Jne *-6
.elseif(n = 5385)
	; 10 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5393)
	; 10 bytes
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	brk
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
.elseif(n = 5457)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
.elseif(n = 5459)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5501) || n = 5502 || (n > 5503 && n < 5604) || (n > 5604 && n < 5622))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 5501)
	; 10 bytes
	sec
	lda #(n-1)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
.elseif(n = 5503)
	; 10 bytes
	tya
	ldy #(n-3)/22 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
.elseif(n = 5622)
	; 10 bytes
	clc
	lda #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	adc #1
	Jne *-6
.elseif(n = 5623)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
.elseif(n = 5624)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	nop
	nop
	nop
	sbc #1
	Jne *-6
.elseif(n = 5625)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	tay
.endif
.endif
.endif
.else
; 5626 to 6250
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
.elseif((n > 5626 && n < 5700) || n = 5701 || n = 5706 || (n > 5707 && n < 5751) || n = 5752 || (n > 5753 && n < 5859) || (n > 5859 && n < 5878) || (n > 5882 && n < 5957) || n = 5958 || (n > 5961 && n < 6114) || (n > 6114 && n < 6132) || n = 6133 || (n > 6138 && n < 6214) || n = 6215 || (n > 6218 && n < 6251))
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
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
.elseif(n = 5702 || n = 5959 || n = 6216)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5703 || n = 5960 || n = 6217)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5704 || n = 5961 || n = 6218)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif(n = 5705)
	; 10 bytes
	sec
	lda #248 ;hides 'sed'
	delay_n 16
	sbc #1
	Jne *-6
.elseif(n = 5707)
	; 10 bytes
	tya
	ldy #248 ;hides 'sed'
	delay_n 16
	dey
	Jne *-5
	tay
.elseif(n = 5751)
	; 10 bytes
	sec
	lda #(n-1)/23 ;hides 'nop'
	delay_n 16
	sbc #1
	Jne *-6
.elseif(n = 5753)
	; 10 bytes
	tya
	ldy #(n-3)/23 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
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
.elseif(n = 5878)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	nop
	sbc #1
	Jne *-6
.elseif(n = 5879)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	pha
	sbc #1
	Jne *-6
.elseif(n = 5880)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
	nop
	iny
	Jne *-5
	tay
.elseif(n = 5881)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EA ;hides 'nop'
	pha
	iny
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
.elseif(n = 5957)
	; 9 bytes
	txa
	pha
	lax #23
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6114)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-5
	tay
.elseif(n = 6132)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $4848 ;hides two 'pha's
	sbc #1
	Jne *-6
.elseif(n = 6134)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	bit $48 ;hides 'pha'
	pha
	sbc #1
	Jne *-6
.elseif(n = 6135)
	; 10 bytes
	clc
	lda #0 ;hides 'brk imm'
	nop $48,X ;hides 'pha'
	pha
	adc #1
	Jne *-6
.elseif(n = 6136)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $48 ;hides 'pha'
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
.elseif(n = 6214)
	; 9 bytes
	txa
	pha
	lax #24
	jsr delay_256a_x_33_clocks
	pla
	tax
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 6296) || (n > 6296 && n < 6369) || (n > 6369 && n < 6387) || n = 6388 || n = 6390 || n = 6391 || n = 6393 || (n > 6394 && n < 6471) || n = 6472 || (n > 6475 && n < 6728) || n = 6729 || (n > 6732 && n < 6985) || n = 6986 || (n > 6989 && n < 7242) || n = 7243 || (n > 7246 && n < 7499) || n = 7500)
	; 11 bytes
	txa
	pha
	lda #>(n-48)
	ldx #<(n-48)
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6296)
	; 10 bytes
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
.elseif(n = 6369)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
.elseif(n = 6387)
	; 10 bytes
	clc
	lda #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
.elseif(n = 6389)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
.elseif(n = 6392)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	sbc #1
	Jne *-6
.elseif(n = 6394)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	tay
.elseif(n = 6471)
	; 9 bytes
	txa
	pha
	lax #25
	jsr delay_256a_x_33_clocks
	pla
	tax
.elseif(n = 6473 || n = 6730 || n = 6987 || n = 7244)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 6474 || n = 6731 || n = 6988 || n = 7245)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 6475 || n = 6732 || n = 6989 || n = 7246)
	; 10 bytes
	pla
	delay_n (n-4)
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
.endif
.else
; 7501 to 10000
.if(n = 7501 || n = 7758 || n = 8015 || n = 8272 || n = 8529 || n = 8786 || n = 9043 || n = 9300 || n = 9557 || n = 9814)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 7502 || n = 7759 || n = 8016 || n = 8273 || n = 8530 || n = 8787 || n = 9044 || n = 9301 || n = 9558 || n = 9815)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7503 || n = 7760 || n = 8017 || n = 8274 || n = 8531 || n = 8788 || n = 9045 || n = 9302 || n = 9559 || n = 9816)
	; 10 bytes
	pla
	delay_n (n-4)
.elseif((n > 7503 && n < 7756) || n = 7757 || (n > 7760 && n < 7899) || (n > 7899 && n < 7917) || n = 7918 || (n > 7919 && n < 7928) || n = 7929 || (n > 7930 && n < 8013) || n = 8014 || (n > 8017 && n < 8270) || n = 8271 || (n > 8274 && n < 8527) || n = 8528 || (n > 8531 && n < 8784) || n = 8785 || (n > 8788 && n < 9041) || n = 9042 || (n > 9045 && n < 9298) || n = 9299 || (n > 9302 && n < 9555) || n = 9556 || (n > 9559 && n < 9812) || n = 9813 || (n > 9816 && n < 10001))
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
.elseif(n = 7899)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
.elseif(n = 7917)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
.elseif(n = 7919)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
.elseif(n = 7928)
	; 10 bytes
	sec
	lda #0 ;hides 'brk imm'
	nop
	brk
	sbc #1
	Jne *-6
.elseif(n = 7930)
	; 10 bytes
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	tay
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
