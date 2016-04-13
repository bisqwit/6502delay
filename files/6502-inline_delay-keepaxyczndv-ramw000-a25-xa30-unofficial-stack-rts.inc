; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber S
; Will preserve A, X, Y, and P
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; WARNING: May write garbage into $00-$FF.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).

; Requires a disposable zeropage variable @zptemp.
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
; - 4 bytes or less when n <= 15
; - 5 bytes or less when n <= 19
; - 6 bytes or less when n <= 22
; - 7 bytes or less when n <= 27
; - 8 bytes or less when n <= 31
; - 9 bytes or less when n <= 36
; -10 bytes or less when n <= 299
; -11 bytes or less when n <= 305
; -12 bytes or less when n <= 327
; -13 bytes or less when n <= 1319
; -14 bytes or less when n <= 2402
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=4.5, max=6
; - When 21 <= n <= 30: min=6, avg=7.0, max=8
; - When 31 <= n <= 50: min=8, avg=9.0, max=10
; - When 51 <= n <= 75: min=8, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.1, max=11
; - When 301 <= n <= 500: min=10, avg=11.5, max=13
; - When 501 <= n <= 750: min=10, avg=11.6, max=13
; - When 751 <= n <= 1000: min=11, avg=11.6, max=13
; - When 1001 <= n <= 2000: min=10, avg=12.3, max=14
; - When 2001 <= n <= 3000: min=12, avg=13.4, max=15
; - When 3001 <= n <= 5000: min=13, avg=14.7, max=15
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
; 1 to 156
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 1 bytes
	pha
.elseif(n = 4 || n = 5 || n = 8 || n = 26 || n = 29)
	; 2..8 bytes
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
.elseif((n > 8 && n < 12) || n = 19 || n = 22 || (n > 30 && n < 34))
	; 3..9 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 4 bytes
	php
	inc $F6,X
	plp
.elseif(n = 14 || n = 21 || n = 28)
	; 4..8 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15 || n = 18 || n = 24 || n = 25 || n = 27 || n = 30 || n = 36 || n = 39)
	; 4..10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16 || n = 17)
	; 5 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 20)
	; 6 bytes
	php
	pha
	inc $F6,X
	pla
	plp
.elseif(n = 23)
	; 7 bytes
	nop
	delay_n (n-2)
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
.elseif(n = 37)
	; 10 bytes
	php
	pha
	clc
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	pla
	plp
.elseif(n = 38)
	; 8 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
.elseif(n = 40)
	; 9 bytes
	php
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
.elseif((n > 40 && n < 53) || (n > 53 && n < 157))
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
.endif
.else
; 157 to 312
.if((n > 156 && n < 297))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 297 || n = 298 || n = 300)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 299 || n = 305 || n = 310)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 301 || n = 303)
	; 11 bytes
	php
	pha
	php
	lda #n-48
	jsr delay_a_25_clocks
	plp
	pla
	plp
.elseif(n = 302)
	; 10 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
	plp
.elseif(n = 304)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 306 || n = 308)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 307)
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 309)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 311)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 312)
	; 10 bytes
	php
	pha
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	Jne *-2
	pla
	plp
.endif
.endif
.else
.if((n >= 313) && (n < 469))
.if((n >= 313) && (n < 391))
; 313 to 390
.if(n = 313 || n = 320 || n = 376 || n = 383)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 314 || n = 339 || n = 344 || n = 349 || n = 354 || n = 369 || n = 374 || n = 384 || n = 389)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 315 || n = 330 || n = 340 || n = 345 || n = 363 || n = 365 || n = 370 || n = 380)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 316 || n = 321 || n = 326 || n = 328 || n = 331 || n = 333 || n = 336 || n = 338 || n = 351 || n = 356 || n = 358 || n = 361 || n = 366 || n = 371 || n = 373 || n = 386)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 317 || n = 322 || n = 342 || n = 347 || n = 357 || n = 362 || n = 372 || n = 377 || n = 382 || n = 387)
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 318)
	; 11 bytes
	php
	pha
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	nop
	Jne *-3
	pla
	plp
.elseif(n = 319 || n = 324 || n = 329 || n = 334 || n = 359 || n = 364 || n = 379)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 323)
	; 12 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 9
	Jne *-5
	pla
	plp
.elseif(n = 325)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 327 || n = 332 || n = 337 || n = 352 || n = 367)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 335)
	; 12 bytes
	php
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	nop
	sta @zptemp
	Jne *-5
	pla
	plp
.elseif(n = 341 || n = 348 || n = 355 || n = 390)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 343 || n = 350 || n = 385)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 346 || n = 381 || n = 388)
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
.elseif(n = 360)
	; 11 bytes
	php
	pha
	clc
	lda #$EB ;hides 'sbc #$3A'
	.byte $3A ;nop ;first loop only
	nop
	Jne *-3
	pla
	plp
.elseif(n = 368)
	; 12 bytes
	php
	pha
	clc
	lda #$69 ;hides 'adc #$6A'
	ror a ;first loop only
	delay_n 4
	Jne *-4
	pla
	plp
.elseif(n = 375)
	; 12 bytes
	php
	pha
	clc
	lda #216 ;hides 'cld'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 378)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.endif
.else
.if((n >= 391) && (n < 430))
; 391 to 429
.if(n = 391 || n = 396 || n = 401 || n = 406 || n = 426)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 392)
	; 10 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jmi *-1
	ldx @zptemp
	plp
.elseif(n = 393)
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
.elseif(n = 394 || n = 404 || n = 409 || n = 419)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif((n .mod 5 = 0) && ((n > 394 && n < 416)))
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 397)
	; 10 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
	plp
.elseif(n = 398)
	; 12 bytes
	php
	inc $F6,X
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jmi *-1
	ldx @zptemp
	plp
.elseif(n = 399 || n = 414 || n = 424 || n = 429)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 401 && n < 428)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 403)
	; 12 bytes
	php
	pha
	inc $F6,X
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
	plp
.elseif(n = 408)
	; 12 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 411)
	; 11 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 413)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 416)
	; 11 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 418)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 420)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 421)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 423)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 425)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-19)/7 ;hides 'nop'
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 428)
	; 11 bytes
	php
	pha
	sec
	lda #58 ;hides 'nop'
	sbc #1
	Jpl *-3
	pla
	plp
.endif
.else
; 430 to 468
.if(n = 430)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 431 || n = 436 || n = 438 || n = 441 || n = 456 || n = 461 || n = 466)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 432 || n = 447 || n = 452 || n = 457 || n = 462)
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 433 || n = 435 || n = 440 || n = 445 || n = 450)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 4) && (n = 434 || (n > 443 && n < 465)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 437 || n = 442 || n = 467)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 439)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 443)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 446 || n = 453 || n = 460)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 448 || n = 455)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 451 || n = 458 || n = 465)
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
.elseif(n = 463)
	; 12 bytes
	php
	pha
	lda #$EB ;hides 'sbc #$18'
	clc ;first loop only
	delay_n 9
	Jne *-5
	pla
	plp
.elseif(n = 468)
	; 12 bytes
	php
	sty @zptemp
	ldy #200 ;hides 'iny'
	nop $04
	Jmi *-3
	ldy @zptemp
	plp
.endif
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 484 || n = 494 || n = 499 || n = 509 || n = 519 || n = 524 || n = 529 || n = 539 || n = 544)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 470 || n = 475 || n = 508 || n = 510 || n = 515 || n = 520 || n = 540 || n = 545)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 471 || n = 476 || n = 478 || n = 491 || n = 496 || n = 501 || n = 506 || n = 511 || n = 513 || n = 526 || n = 531 || n = 536 || n = 538 || n = 541 || n = 546)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 472 || n = 482 || n = 487 || n = 497 || n = 517 || n = 522 || n = 537)
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 473)
	; 12 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	sta @zptemp
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 477 || n = 492 || n = 502 || n = 507 || n = 512 || n = 527 || n = 532 || n = 542)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 479 || n = 489 || n = 504 || n = 514 || n = 534)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 480)
	; 12 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	ldy @zptemp
	plp
.elseif(n = 481 || n = 488 || n = 530)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 483 || n = 525)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 485)
	; 12 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 486)
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
.elseif(n = 490)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 493 || n = 500 || n = 528 || n = 535)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 495 || n = 523)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 498)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 503)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 505)
	; 11 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	adc #1
	Jpl *-3
	pla
	plp
.elseif(n = 516)
	; 11 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 518)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 521)
	; 11 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 533)
	; 12 bytes
	php
	pha
	clc
	lda #$EB ;hides 'sbc #$6A'
	ror a ;first loop only
	delay_n 7
	Jne *-4
	pla
	plp
.elseif(n = 543)
	; 12 bytes
	php
	sty @zptemp
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	ldy @zptemp
	plp
.endif
.else
; 547 to 625
.if(n = 547 || n = 572 || n = 577 || n = 587 || n = 592 || n = 612)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 548)
	; 12 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif((n .mod 5 = 4) && ((n > 548 && n < 580) || n = 604 || (n > 613 && n < 625)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 550 || n = 555 || n = 580 || n = 585 || n = 590 || n = 610)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 551 || n = 586 || n = 593)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 551 && n < 568) || n = 582 || (n > 596 && n < 608) || n = 617 || n = 622))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 553 || n = 560 || n = 595 || n = 623)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 556 || n = 563 || n = 591 || n = 598)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 558 || n = 565 || n = 600 || n = 621)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 561 || n = 566 || n = 571 || n = 576 || n = 581 || n = 596 || n = 601 || n = 606 || n = 608 || n = 611 || n = 616 || n = 618)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 568)
	; 12 bytes
	php
	pha
	sec
	lda #68 ;hides 'nop zp'
	nop
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 570)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 573)
	; 12 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	ldy @zptemp
	plp
.elseif(n = 575)
	; 12 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	dey
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 578)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 583)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 584 || n = 589 || n = 594 || n = 599 || n = 609)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 588)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 603)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 605)
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
.elseif(n = 613)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-28)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 615)
	; 12 bytes
	php
	sty @zptemp
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 620)
	; 12 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	iny
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 625)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	sta @zptemp
	Jmi *-3
	tax
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 626) && (n < 938))
.if((n >= 626) && (n < 782))
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626 || n = 633 || n = 668 || n = 675)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 626 && n < 653) || n = 662))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 628 || n = 663 || n = 698)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 4) && ((n > 628 && n < 645) || (n > 653 && n < 665)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/5
	dey
	Jpl *-1
	tay
	pla
	plp
.elseif(n = 630 || n = 658 || n = 693)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 631 || n = 636 || n = 641 || n = 646 || n = 651 || n = 666 || n = 671 || n = 676 || n = 678 || n = 681 || n = 686 || n = 688 || n = 701)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 635)
	; 11 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	ldy @zptemp
	plp
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
	; 11 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	pla
	plp
.elseif(n = 643 || n = 648 || n = 650 || n = 655 || n = 680 || n = 685 || n = 690 || n = 695 || n = 700)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 645)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif((n .mod 5 = 4) && (n = 649 || (n > 668 && n < 700)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 653)
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
.elseif(n = 656 || n = 670 || n = 691)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && (n = 657 || (n > 666 && n < 693)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 660)
	; 12 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	ldy @zptemp
	plp
.elseif(n = 661 || n = 696 || n = 703)
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
.elseif(n = 665)
	; 12 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 673)
	; 13 bytes
	php
	pha
	sec
	lda #(n-25)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 683)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 697)
	; 10 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	Jne *-1
	ldy @zptemp
	plp
.elseif(n = 702)
	; 10 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	pla
	plp
.endif
.else
; 704 to 781
.if((n .mod 5 = 4) && ((n > 703 && n < 780)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 705 || n = 715 || n = 720 || n = 730 || n = 748 || n = 750 || n = 755 || n = 760 || n = 765)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 706 || n = 711 || n = 716 || n = 718 || n = 721 || n = 736 || n = 741 || n = 746 || n = 751 || n = 756 || n = 758 || n = 771 || n = 776 || n = 778 || n = 781)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 706 && n < 778)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 708)
	; 12 bytes
	php
	pha
	inc $F6,X
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	pla
	plp
.elseif(n = 710 || n = 738)
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
.elseif(n = 713)
	; 13 bytes
	php
	pha
	sec
	lda #(n-25)/8
	bit $24
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 723)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 725)
	; 12 bytes
	php
	pha
	sec
	lda #$E9 ;hides 'sbc #$1A'
	.byte $1A ;nop ;first loop only
	delay_n 7
	Jne *-4
	pla
	plp
.elseif(n = 726)
	; 11 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 728 || n = 763 || n = 770)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 731)
	; 11 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 733 || n = 768 || n = 775)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 735)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 740)
	; 11 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	ldy @zptemp
	plp
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
.elseif(n = 745)
	; 11 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 753)
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
.elseif(n = 761)
	; 11 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	ldy @zptemp
	plp
.elseif(n = 766)
	; 11 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
	pla
	plp
.elseif(n = 773 || n = 780)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if((n .mod 5 = 2) && ((n > 781 && n < 858)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 783 || n = 786 || n = 788 || n = 791 || n = 806 || n = 811 || n = 816 || n = 821 || n = 826 || n = 841 || n = 846 || n = 848 || n = 851 || n = 853 || n = 856 || n = 858)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 860)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 785 || n = 790 || n = 820 || n = 830 || n = 835 || n = 855)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 793)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-28)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 795)
	; 12 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	ldy @zptemp
	plp
.elseif(n = 796 || n = 803 || n = 838)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 798 || n = 833)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 800)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 801 || n = 815 || n = 850)
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
.elseif(n = 805 || n = 840)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 808 || n = 836 || n = 843)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 810 || n = 831 || n = 845)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 813)
	; 12 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	ldy @zptemp
	plp
.elseif(n = 818)
	; 12 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 823)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #3
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 825)
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
.elseif(n = 828)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #3
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.endif
.else
; 860 to 937
.if((n .mod 5 = 0) && ((n > 859 && n < 871) || (n > 889 && n < 906) || n = 925))
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 861 || n = 881 || n = 886 || n = 888 || n = 891 || n = 896 || n = 898 || n = 911 || n = 916 || n = 918 || n = 921 || n = 926 || n = 928 || n = 931)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 861 && n < 938)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 863)
	; 13 bytes
	php
	pha
	sec
	lda #(n-26)/9
	delay_n 4
	sbc #1
	Jpl *-4
	pla
	plp
.elseif((n .mod 5 = 4) && ((n > 863 && n < 935)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 866)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 868 || n = 875 || n = 903 || n = 910)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-3
	nop
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 871)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 873)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-19)/7 ;hides 'nop'
	dey
	Jpl *-2
	ldy @zptemp
	plp
.elseif(n = 876)
	; 11 bytes
	php
	pha
	sec
	lda #250 ;hides 'nop'
	sbc #1
	Jmi *-3
	pla
	plp
.elseif(n = 878)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-24)/7 ;hides 'nop'
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 880 || n = 908 || n = 915)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-26)/7
	nop
	dey
	Jpl *-2
	tay
	pla
	plp
.elseif(n = 883)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 885 || n = 906 || n = 920)
	; 12 bytes
	php
	pha
	sec
	lda #(n-24)/7
	nop
	sbc #1
	Jpl *-3
	pla
	plp
.elseif(n = 893)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 901 || n = 936)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 913)
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
.elseif(n = 923)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 930)
	; 12 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	ldy @zptemp
	plp
.elseif(n = 933)
	; 12 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop
	adc #1
	Jpl *-4
	pla
	plp
.elseif(n = 935)
	; 12 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jpl *-3
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if(n = 938 || n = 945 || n = 973 || n = 1008 || n = 1015)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif((n .mod 5 = 4) && ((n > 938 && n < 1015)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 940 || n = 958 || n = 960 || n = 965 || n = 970 || n = 995 || n = 1000 || n = 1005 || n = 1010)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 941 || n = 948 || n = 983 || n = 990 || n = 1011)
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
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1013)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 943 || n = 978 || n = 985 || n = 1006 || n = 1013)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 946 || n = 951 || n = 956 || n = 961 || n = 966 || n = 968 || n = 981 || n = 986 || n = 988 || n = 991 || n = 996 || n = 1001)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 950)
	; 11 bytes
	php
	sty @zptemp
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 953)
	; 11 bytes
	php
	pha
	clc
	lda #122 ;hides 'nop'
	adc #1
	Jne *-3
	pla
	plp
.elseif(n = 955)
	; 11 bytes
	php
	pha
	tya
	ldy #122 ;hides 'nop'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 963 || n = 1003)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-27)/8
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 971)
	; 11 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 975)
	; 12 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	ldy @zptemp
	plp
.elseif(n = 976)
	; 11 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	tay
	pla
	plp
.elseif(n = 980)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	tay
	pla
	plp
.elseif(n = 993)
	; 12 bytes
	php
	sty @zptemp
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 998)
	; 12 bytes
	php
	pha
	tya
	ldy #218 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 1016 to 1093
.if(n = 1016 || n = 1021 || n = 1023 || n = 1026 || n = 1028 || n = 1031 || n = 1036 || n = 1051 || n = 1056 || n = 1058 || n = 1061 || n = 1063 || n = 1066 || n = 1071 || n = 1086 || n = 1091 || n = 1093)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && (n = 1017 || n = 1022 || (n > 1036 && n < 1093)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1018 || n = 1025 || n = 1046 || n = 1053 || n = 1060 || n = 1081)
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
.elseif((n .mod 5 = 4) && ((n > 1018 && n < 1090)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 1020 || n = 1041 || n = 1048 || n = 1055 || n = 1076 || n = 1083 || n = 1090)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1027)
	; 10 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	ldx @zptemp
	plp
.elseif((n .mod 5 = 0) && ((n > 1029 && n < 1046) || n = 1075 || n = 1080))
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1032)
	; 10 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
	plp
.elseif(n = 1033)
	; 12 bytes
	php
	inc $F6,X
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	ldx @zptemp
	plp
.elseif(n = 1038)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	inc @zptemp,x
	tax
	pla
	plp
.elseif(n = 1043 || n = 1050 || n = 1078 || n = 1085)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1065)
	; 12 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1068)
	; 12 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	sbc #1
	Jmi *-4
	pla
	plp
.elseif(n = 1070)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1073)
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
.elseif(n = 1088)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	iny
	iny
	Jne *-3
	tay
	pla
	plp
.endif
.endif
.else
; 1094 to 1250
.if((n .mod 5 = 4) && ((n > 1093 && n < 1250)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && (n = 1095 || (n > 1115 && n < 1131) || (n > 1150 && n < 1166) || (n > 1185 && n < 1201) || (n > 1220 && n < 1236)))
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
.elseif(n = 1096 || n = 1098 || n = 1101 || n = 1103 || n = 1106 || n = 1121 || n = 1126 || n = 1131 || n = 1136 || n = 1138 || n = 1141 || n = 1143 || n = 1156 || n = 1161 || n = 1166 || n = 1168 || n = 1171 || n = 1173 || n = 1176 || n = 1191 || n = 1196 || n = 1198 || n = 1201 || n = 1206 || n = 1208 || n = 1211 || n = 1213 || n = 1226 || n = 1231 || n = 1236 || n = 1238 || n = 1241 || n = 1243 || n = 1246 || n = 1248)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 1096 && n < 1248)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif((n .mod 5 = 0) && (n = 1100 || n = 1105 || (n > 1134 && n < 1151) || (n > 1169 && n < 1186) || (n > 1204 && n < 1221) || n = 1240))
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1108)
	; 12 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop $04
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1110)
	; 12 bytes
	php
	sty @zptemp
	ldy #(n-12)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 7 = 5) && ((n > 1110 && n < 1126) || (n > 1145 && n < 1161) || (n > 1180 && n < 1196) || (n > 1215 && n < 1231)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1113 || n = 1120 || n = 1148 || n = 1155 || n = 1183 || n = 1190 || n = 1218 || n = 1225)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1115)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1128)
	; 12 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1133)
	; 12 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1163 || n = 1203)
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
.elseif(n = 1178)
	; 13 bytes
	php
	pha
	sec
	lda #(n-26)/9
	delay_n 4
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1233)
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
.elseif(n = 1245)
	; 12 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1250)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	tay
	pla
	plp
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
.if((n .mod 7 = 5) && ((n > 1250 && n < 1266) || n = 1286 || n = 1293 || (n > 1306 && n < 1329)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 1251 && n < 1293)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif((n .mod 7 = 0) && (n = 1253 || n = 1260 || (n > 1287 && n < 1324)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif((n .mod 5 = 4) && ((n > 1253 && n < 1295)))
	; 11 bytes
	php
	pha
	tya
	ldy #(n-19)/5
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 1255 || n = 1280 || n = 1285 || n = 1290 || n = 1306 || n = 1308)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1256 || n = 1263 || n = 1270 || n = 1291 || n = 1298 || n = 1312 || n = 1319 || n = 1326)
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
.elseif(n = 1261 || n = 1266 || n = 1271 || n = 1276 || n = 1278 || n = 1281 || n = 1296 || n = 1301 || n = 1311 || n = 1320)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1268)
	; 12 bytes
	php
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1273 || n = 1313)
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
.elseif(n = 1275)
	; 12 bytes
	php
	pha
	clc
	lda #116 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 1283)
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
.elseif(n = 1299)
	; 11 bytes
	php
	pha
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
	plp
.elseif(n = 1300)
	; 11 bytes
	php
	sty @zptemp
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	ldy @zptemp
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
	; 13 bytes
	php
	pha
	php
	sec
	lda #0
	sbc #1
	Jne *-2
	plp
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
.elseif(n = 1310)
	; 13 bytes
	php
	pha
	php
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1315 || n = 1324)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1317)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1318)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1322)
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
.elseif(n = 1325)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1327)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329 || n = 1345 || n = 1353)
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
.elseif((n .mod 7 = 0) && ((n > 1329 && n < 1366)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1331 || n = 1339 || n = 1355)
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
.elseif(n = 1332)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1362)))
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
.elseif(n = 1334 || n = 1350 || n = 1366)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1364)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1336)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1338)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1341)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1343)
	; 13 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	ldy @zptemp
	plp
.elseif(n = 1346)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 4
	sbc #1
	Jmi *-5
	pla
	plp
.elseif(n = 1348)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
	pla
	plp
.elseif(n = 1352)
	; 13 bytes
	php
	sty @zptemp
	ldy #122 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1357)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1359)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1360)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1362)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $6AA0'
	ldy #106
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1364)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1367)
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
.endif
.else
; 1368 to 1406
.if((n .mod 7 = 3) && ((n > 1367 && n < 1397)))
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
.elseif(n = 1369 || n = 1378 || n = 1387)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1392) || n = 1405))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1371 || n = 1395)
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
.elseif((n .mod 7 = 0) && ((n > 1371 && n < 1401)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1373)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1374 || n = 1390)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1376)
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
.elseif(n = 1380)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $68A0'
	ldy #104
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1381)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1383)
	; 13 bytes
	php
	pha
	tya
	ldy #123
	inc $F6,X
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 1385)
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
.elseif(n = 1388 || n = 1402)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1392)
	; 13 bytes
	php
	pha
	sec
	lda #124
	inc $F6,X
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1394)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
	pla
	plp
.elseif(n = 1397)
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
.elseif(n = 1398)
	; 11 bytes
	php
	sty @zptemp
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1399)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
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
.elseif(n = 1403)
	; 11 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1404 || n = 1406)
	; 12 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if((n .mod 7 = 0) && ((n > 1406 && n < 1443)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1408)
	; 13 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	inc $F6,X
	dey
	Jmi *-4
	tay
	pla
	plp
.elseif(n = 1409 || n = 1425)
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
.elseif(n = 1410 || n = 1424 || n = 1431 || n = 1445)
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
.elseif(n = 1411 || n = 1427 || n = 1443)
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
.elseif(n = 1412)
	; 11 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1413)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $9BA0'
	ldy #155
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1415 || n = 1420 || n = 1436 || n = 1441)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1416 || n = 1444)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1417)
	; 11 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1418)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1419 || n = 1426 || n = 1440)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1422)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1423 || n = 1432)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1429)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1430 || n = 1439)
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
.elseif(n = 1433)
	; 11 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	ldx @zptemp
	plp
.elseif(n = 1434)
	; 13 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $62A0'
	ldy #98
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1437)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1438)
	; 11 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
	pla
	plp
.endif
.else
; 1446 to 1484
.if(n = 1446 || n = 1462 || n = 1478)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 7 = 5) && ((n > 1446 && n < 1483)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
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
.elseif((n .mod 7 = 0) && ((n > 1448 && n < 1485)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1450)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1451 || n = 1467 || n = 1483)
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
.elseif((n .mod 7 = 3) && ((n > 1451 && n < 1481)))
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
.elseif(n = 1453)
	; 13 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1455 || n = 1464)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1457 || n = 1465 || n = 1481)
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
.elseif(n = 1458)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1460)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1469)
	; 13 bytes
	php
	pha
	sec
	lda #(n-29)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
	plp
.elseif(n = 1471)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1472)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1474)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1476)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $A2A0'
	ldy #162
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1479)
	; 13 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $A3A0'
	ldy #163
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1485 && n < 1514)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1523)))
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
.elseif(n = 1488)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1518)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1490)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 7 = 0) && ((n > 1490 && n < 1520)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1492)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1493 || n = 1502)
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
.elseif(n = 1497 || n = 1521)
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
.elseif(n = 1499 || n = 1507 || n = 1523)
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
.elseif(n = 1500)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1506)
	; 12 bytes
	php
	sty @zptemp
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1509)
	; 12 bytes
	php
	pha
	clc
	lda #90 ;hides 'nop'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif(n = 1511)
	; 12 bytes
	php
	pha
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1514)
	; 12 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1516)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1518)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1520)
	; 12 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	nop
	adc #1
	Jne *-4
	pla
	plp
.endif
.else
; 1524 to 1562
.if(n = 1524)
	; 11 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1525)
	; 12 bytes
	php
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 7 = 0) && ((n > 1525 && n < 1562)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
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
.elseif(n = 1528)
	; 13 bytes
	delay_n (n-2)
	nop
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
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1531 || n = 1545 || n = 1552 || n = 1559)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1532 || n = 1544 || n = 1546)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1534)
	; 13 bytes
	php
	pha
	php
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1535 || n = 1562)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1536 || n = 1550 || n = 1557)
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
.elseif(n = 1537 || n = 1553)
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
.elseif(n = 1538)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1539 || n = 1555)
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
.elseif(n = 1541)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1542)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1543)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1548)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1549 || n = 1558)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1551)
	; 12 bytes
	php
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1556)
	; 12 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1560)
	; 12 bytes
	php
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	ldy @zptemp
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
.if(n = 1563)
	; 12 bytes
	php
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	nop
	adc #1
	Jne *-4
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1600)))
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
.elseif(n = 1565)
	; 12 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1602)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1567 || n = 1576)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 0) && ((n > 1567 && n < 1597)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1569)
	; 12 bytes
	php
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1570 || n = 1584 || n = 1588)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1572)
	; 13 bytes
	php
	sty @zptemp
	ldy #100 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1574)
	; 12 bytes
	php
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1577 || n = 1593)
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
.elseif(n = 1579 || n = 1595)
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
.elseif(n = 1581)
	; 12 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1583)
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
.elseif(n = 1586)
	; 12 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1590)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1591)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1597)
	; 12 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1598)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1600)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 7 = 0) && ((n > 1602 && n < 1639)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1604 || n = 1605 || n = 1626 || n = 1640)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1606 || n = 1613 || n = 1620 || n = 1634)
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
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1608 || n = 1615 || n = 1629)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1609 || n = 1633)
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
.elseif(n = 1611 || n = 1619 || n = 1635)
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
.elseif(n = 1612 || n = 1621)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1614)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1616)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1618 || n = 1630)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1622)
	; 11 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1623)
	; 12 bytes
	php
	stx @zptemp
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	ldx @zptemp
	plp
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
.elseif(n = 1627)
	; 11 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1628)
	; 12 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	tax
	pla
	plp
.elseif(n = 1632)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $B4A0'
	ldy #180
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1636)
	; 11 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1637)
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
.endif
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if(n = 1641)
	; 11 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1642 || n = 1644 || n = 1658)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && (n = 1643 || (n > 1656 && n < 1679)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 7 = 0) && ((n > 1644 && n < 1667)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1646)
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
.elseif(n = 1647)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1648 || n = 1662 || n = 1669 || n = 1676)
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
.elseif(n = 1649 || n = 1665)
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
.elseif(n = 1650)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1651 || n = 1667)
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
.elseif(n = 1653)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1654)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1655)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1656)
	; 12 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1660)
	; 13 bytes
	php
	pha
	php
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1661)
	; 12 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1663)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1668)
	; 12 bytes
	php
	sty @zptemp
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1670 || n = 1679)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1672)
	; 13 bytes
	php
	sty @zptemp
	ldy #90 ;hides 'nop'
	nop $04
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1673)
	; 12 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1674)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1675)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1677)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.endif
.else
; 1680 to 1718
.if((n .mod 7 = 0) && ((n > 1679 && n < 1716)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1681 || n = 1689 || n = 1705)
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
.elseif(n = 1682 || n = 1709)
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
.elseif((n .mod 7 = 3) && ((n > 1682 && n < 1719)))
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
.elseif(n = 1684 || n = 1693 || n = 1702)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1684 && n < 1714)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1686 || n = 1710)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1688)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1691 || n = 1707)
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
.elseif(n = 1695)
	; 13 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	ldx @zptemp
	plp
.elseif(n = 1696 || n = 1698 || n = 1703 || n = 1712)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1700)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	tax
	pla
	plp
.elseif(n = 1714)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1716)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1717)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1749)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1742) || n = 1755))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1721 || n = 1737 || n = 1745)
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
.elseif((n .mod 7 = 0) && ((n > 1721 && n < 1758)))
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1723 || n = 1731)
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
.elseif(n = 1724 || n = 1742)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1747)))
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
.elseif(n = 1726)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1728)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1730)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1733)
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
.elseif(n = 1735)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1738 || n = 1747)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1740)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1744)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1748)
	; 11 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	ldy @zptemp
	plp
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
.elseif(n = 1752)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1754 || n = 1756)
	; 12 bytes
	pha
	delay_n (n-3)
.endif
.else
; 1758 to 1796
.if(n = 1758)
	; 13 bytes
	php
	pha
	php
	sec
	lda #248 ;hides 'sed'
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1759)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && (n = 1760 || (n > 1773 && n < 1796)))
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
.elseif(n = 1761 || n = 1777 || n = 1793)
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
.elseif(n = 1762)
	; 11 bytes
	php
	sty @zptemp
	ldy #(n-12)/7 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1763 || n = 1779 || n = 1787)
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
.elseif(n = 1764 || n = 1771 || n = 1785 || n = 1792)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1765)
	; 11 bytes
	php
	pha
	sec
	lda #(n-15)/7 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1766 || n = 1782)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1767)
	; 11 bytes
	php
	pha
	tya
	ldy #(n-17)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1768 || n = 1784)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 5) && ((n > 1768 && n < 1791)))
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
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
.elseif(n = 1772)
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
.elseif(n = 1773)
	; 12 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1775 || n = 1780)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1778)
	; 12 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1786)
	; 12 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1789)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1791)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1794)
	; 12 bytes
	php
	sty @zptemp
	ldy #58 ;hides 'nop'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1796)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
.if(n = 1797 || n = 1804)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1798)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1799)
	; 12 bytes
	php
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1800)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1801 || n = 1810)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1802)
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
.elseif(n = 1803)
	; 12 bytes
	php
	sty @zptemp
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1805)
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
.elseif(n = 1806)
	; 12 bytes
	php
	sty @zptemp
	ldy #0
	nop
	dey
	Jne *-2
	ldy @zptemp
	plp
.elseif(n = 1807)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1808)
	; 12 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1811)
	; 12 bytes
	php
	pha
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1812)
	; 12 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1813)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1814)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1815)
	; 13 bytes
	pha
	delay_n (n-3)
.endif
.else
; 1816 to 1835
.if(n = 1816)
	; 14 bytes
	php
	pha
	php
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
	plp
	pla
	plp
.elseif(n = 1817)
	; 12 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1818)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1819 || n = 1828)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1820)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1821)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1822 || n = 1830)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1823 || n = 1832)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1824)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 1825 || n = 1833)
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
.elseif(n = 1826)
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
.elseif(n = 1827 || n = 1835)
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
.elseif(n = 1829)
	; 12 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1831)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
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
.endif
.endif
.else
.if((n >= 1836) && (n < 1856))
; 1836 to 1855
.if(n = 1836 || n = 1852)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1837)
	; 12 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1838 || n = 1854)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1839)
	; 12 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	ldx @zptemp
	plp
.elseif(n = 1840)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1841 || n = 1849)
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
.elseif(n = 1844)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	tax
	pla
	plp
.elseif(n = 1845)
	; 12 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1846 || n = 1855)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1847)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1848)
	; 12 bytes
	php
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
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
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 1856 to 1875
.if(n = 1856)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #7
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 1857)
	; 12 bytes
	php
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1858)
	; 13 bytes
	php
	sty @zptemp
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1859 || n = 1867 || n = 1875)
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
.elseif(n = 1860)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1861)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1862)
	; 12 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1863)
	; 13 bytes
	php
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1864 || n = 1873)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1865)
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
.elseif(n = 1866)
	; 13 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1868)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1869)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1870)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1871)
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
.elseif(n = 1872)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1874)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.endif
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
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1877 || n = 1901)
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
.elseif(n = 1878 || n = 1894 || n = 1902 || n = 1910)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1879 || n = 1903)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1880 || n = 1898)
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
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1914)))
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
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1910)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1883 || n = 1899 || n = 1907)
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
.elseif(n = 1884)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1885)
	; 13 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1886 || n = 1895 || n = 1904)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1887)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1888)
	; 13 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1890)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1892)
	; 13 bytes
	php
	sty @zptemp
	ldy #68 ;hides 'nop zp'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1893)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1896)
	; 13 bytes
	php
	pha
	clc
	lda #68 ;hides 'nop zp'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1906)
	; 13 bytes
	php
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 1908)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1911)
	; 12 bytes
	php
	sty @zptemp
	ldy #(n-3)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1912)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1914)
	; 13 bytes
	pha
	delay_n (n-3)
.endif
.else
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915 || n = 1931)
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
.elseif(n = 1916)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-8)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1917)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1918 || n = 1927)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1919)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1920)
	; 12 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1921)
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
.elseif(n = 1922)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1923)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1924)
	; 13 bytes
	php
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	adc #1
	Jne *-5
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
.elseif(n = 1926)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1928)
	; 13 bytes
	php
	pha
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1929)
	; 12 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1930)
	; 13 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1932)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1933)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 1934 to 1953
.if(n = 1934)
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
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1936 || n = 1945)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1937 || n = 1953)
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
.elseif(n = 1938)
	; 12 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1939 || n = 1947)
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
.elseif(n = 1940)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1941)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 1942 || n = 1950)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
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
.elseif(n = 1944)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1946)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1948)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1949)
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
.elseif(n = 1951)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1952)
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
.endif
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
.if((n >= 1954) && (n < 1973))
; 1954 to 1972
.if(n = 1954 || n = 1963 || n = 1972)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1955 || n = 1971)
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
.elseif(n = 1956)
	; 12 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1957 || n = 1967)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1958)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
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
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
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
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1964)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 1965)
	; 13 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1966)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1968)
	; 13 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1969)
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
.elseif(n = 1970)
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
.endif
.else
; 1973 to 1992
.if(n = 1973)
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
.elseif(n = 1974)
	; 12 bytes
	php
	sty @zptemp
	ldy #(n-12)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1975)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1976)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
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
	; 13 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1979)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1980)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1981 || n = 1990)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1982)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 1983)
	; 13 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1984)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 1985)
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
.elseif(n = 1986)
	; 13 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	inx
	inx
	Jne *-4
	tax
	pla
	plp
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
	; 13 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1989)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1991)
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
.elseif(n = 1992)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	ldy @zptemp
	plp
.endif
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2001 || n = 2009 || n = 2025)
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
.elseif(n = 1994 || n = 2012 || n = 2030)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 8 = 3) && ((n > 1994 && n < 2028)))
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
.elseif(n = 1996 || n = 2000 || n = 2013 || n = 2031)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 8) && ((n > 1996 && n < 2025)))
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
.elseif(n = 1998)
	; 13 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1998 && n < 2027)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2002)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2004)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2005)
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
.elseif(n = 2007)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2010)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2014 || n = 2022)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2016)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2018)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2020)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2021)
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
.elseif(n = 2023)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2028)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2029)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2033 || n = 2049)
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
	; 13 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2035 || n = 2044)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2036)
	; 13 bytes
	php
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2037)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2038 || n = 2046)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2039)
	; 13 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2040)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2042)
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
.elseif(n = 2043)
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
.elseif(n = 2045)
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
.elseif(n = 2047)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2048)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2050)
	; 13 bytes
	php
	pha
	sec
	lda #226 ;hides 'nop #imm'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 2051 to 2070
.if(n = 2051 || n = 2059)
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
.elseif(n = 2052)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2053 || n = 2062)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2054)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2055)
	; 13 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2056)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2057)
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
.elseif(n = 2058)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2060 || n = 2069)
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
.elseif(n = 2061)
	; 13 bytes
	php
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2063)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2064)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2065)
	; 13 bytes
	php
	pha
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2066)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2067)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	cmp $C5
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2068)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2070)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071 || n = 2080 || n = 2089)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2072)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	php
	cmp $C5
	plp
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2073)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2074)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2075 || n = 2084)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2076)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2077)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2078)
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
.elseif(n = 2079)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2081)
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
.elseif(n = 2082)
	; 12 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2083)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
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
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2087)
	; 12 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2088)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	inx
	Jne *-4
	tax
	pla
	plp
.endif
.else
; 2090 to 2109
.if(n = 2090)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2091)
	; 12 bytes
	php
	stx @zptemp
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	ldx @zptemp
	plp
.elseif(n = 2092)
	; 13 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2093)
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
.elseif(n = 2094)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2095)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2096)
	; 12 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	tax
	pla
	plp
.elseif(n = 2097)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2098 || n = 2107)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2099 || n = 2109)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2100)
	; 12 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2101)
	; 13 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2102)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
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
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2105)
	; 12 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2106)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2108)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #8
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2110) && (n < 2149))
.if((n >= 2110) && (n < 2129))
; 2110 to 2128
.if(n = 2110)
	; 13 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2111 || n = 2120)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2112)
	; 14 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2113)
	; 13 bytes
	php
	pha
	txa
	pha
	lax #8
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
	plp
.elseif(n = 2114)
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
.elseif(n = 2115)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2116 || n = 2125)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2117)
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
.elseif(n = 2118)
	; 12 bytes
	php
	sty @zptemp
	ldy #(n-12)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2119)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
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
.elseif(n = 2122)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2123)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2124)
	; 13 bytes
	php
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2126)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2127)
	; 12 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2128)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	ldy @zptemp
	plp
.endif
.else
; 2129 to 2148
.if(n = 2129)
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
.elseif(n = 2130 || n = 2135 || n = 2148)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2131)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2132)
	; 12 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2133)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2134 || n = 2143)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2136)
	; 12 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2137 || n = 2147)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2138)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
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
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2141)
	; 12 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2142)
	; 13 bytes
	php
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2144)
	; 13 bytes
	php
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2145)
	; 12 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2146)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	dey
	Jne *-4
	ldy @zptemp
	plp
.endif
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2169)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2150)
	; 12 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2151)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2153 || n = 2165)
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
.elseif(n = 2154)
	; 13 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2155 || n = 2167)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2156 || n = 2174 || n = 2183)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2157 || n = 2187)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2158 || n = 2171 || n = 2176)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 8) && ((n > 2158 && n < 2187)))
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
.elseif(n = 2160)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2162)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2163)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2164)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2166)
	; 14 bytes
	php
	pha
	tya
	ldy #195
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
	nop
.elseif(n = 2172)
	; 13 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	bit $24
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2173 || n = 2184)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2175)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2178)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2180)
	; 14 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2181)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2182)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2185)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
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
.if(n = 2188 || n = 2197 || n = 2206)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2189 || n = 2201)
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
.elseif(n = 2190)
	; 13 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2191 || n = 2203)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2192)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2193)
	; 13 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2194)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 2196)
	; 13 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2198)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2199)
	; 12 bytes
	php
	sty @zptemp
	ldy #(n-3)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2200)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2202)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2204)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-8)/9 ;hides 'nop zp,x'
	dey ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2205)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 2207 to 2226
.if(n = 2207)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2208)
	; 12 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2209)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2210 || n = 2219)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2211)
	; 12 bytes
	php
	pha
	sec
	lda #(n-15)/9 ;hides 'nop zp,x'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2212)
	; 13 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2213)
	; 12 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2214)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2215 || n = 2224)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2216)
	; 13 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2217)
	; 12 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2218)
	; 13 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2220)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2221)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2222)
	; 12 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2223)
	; 13 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2225)
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
.elseif(n = 2226)
	; 13 bytes
	php
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	ldx @zptemp
	plp
.endif
.endif
.else
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227 || n = 2239)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2228)
	; 13 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2229)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2230)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
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
.elseif(n = 2232)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2233 || n = 2242)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2234)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2235)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2236)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2237)
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
.elseif(n = 2238)
	; 14 bytes
	php
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
	plp
.elseif(n = 2241)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2243)
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
.elseif(n = 2244)
	; 12 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2245)
	; 13 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
	ldx @zptemp
	plp
.endif
.else
; 2246 to 2265
.if(n = 2246 || n = 2255 || n = 2264)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
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
	; 14 bytes
	delay_n (n-2)
	nop
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
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
	tax
	pla
	plp
.elseif(n = 2251 || n = 2260)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2252)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2253)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2254)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2256)
	; 13 bytes
	php
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2257)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2258)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2259)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2261)
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
.elseif(n = 2262)
	; 12 bytes
	php
	sty @zptemp
	ldy #(n-12)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2263)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2265)
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
.endif
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2267)
	; 12 bytes
	php
	pha
	tya
	ldy #(n-17)/9 ;hides 'nop'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2268)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2269 || n = 2278)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2270)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2271)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2272)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2273)
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
.elseif(n = 2274)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2275)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2276)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2277)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2279)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2280)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2281)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2282)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2283)
	; 13 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2284)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.endif
.else
; 2285 to 2304
.if(n = 2285)
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
.elseif(n = 2287 || n = 2296)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2288)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2289)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2290)
	; 13 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2291 || n = 2300)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2292)
	; 13 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2293)
	; 13 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2294)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2295)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2297)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2298)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2299)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2301)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2302)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 2304)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 2305) && (n < 2324))
; 2305 to 2323
.if(n = 2305 || n = 2314)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2306)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2307 || n = 2317)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2308)
	; 13 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	nop
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2309)
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
.elseif(n = 2310 || n = 2320)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2311)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2312)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2313)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2315)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	nop $04
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2316)
	; 13 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2318)
	; 13 bytes
	php
	sty @zptemp
	ldy #0
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2319)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2321)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2322)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2323)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 2324 to 2343
.if(n = 2324 || n = 2325 || n = 2331)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2326)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2327)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2328)
	; 14 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2329 || n = 2339)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2330 || n = 2342)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2332)
	; 13 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	bit $24
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2333)
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
.elseif(n = 2334)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
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
	; 13 bytes
	php
	sty @zptemp
	ldy #90 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2337)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	cmp $C5
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2338)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2340)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2341)
	; 13 bytes
	php
	sty @zptemp
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2343)
	; 13 bytes
	php
	sty @zptemp
	ldy #212 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
.if((n >= 2344) && (n < 2363))
; 2344 to 2362
.if(n = 2344)
	; 13 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2345)
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
.elseif(n = 2346)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2347 || n = 2359)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2348)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-16)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
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
	; 14 bytes
	php
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2351 || n = 2362)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2352)
	; 13 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	bit $24
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2353)
	; 14 bytes
	php
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2354)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2355)
	; 13 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2356 || n = 2361)
	; 14 bytes
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
	; 14 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
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
.endif
.else
; 2363 to 2382
.if(n = 2363)
	; 14 bytes
	php
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2364 || n = 2374)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2365)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #9
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 2366 || n = 2378)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2367 || n = 2377)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2368)
	; 13 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	inc $EA,X ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
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
.elseif(n = 2371)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2372 || n = 2375)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2373)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2376)
	; 14 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2379)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2380)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2382)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.endif
.endif
.else
; 2383 to 2421
.if((n .mod 12 = 7) && ((n > 2382 && n < 2420)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2384 || n = 2406)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2385)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2386 || n = 2392 || n = 2416 || n = 2421)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2387 || n = 2397)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2388)
	; 13 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2389 || n = 2399 || n = 2409)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2390 || n = 2402 || n = 2414)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2391)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
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
.elseif(n = 2394)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2396)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2398)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2400)
	; 14 bytes
	php
	sty @zptemp
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2401 || n = 2412)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2403)
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
.elseif(n = 2404 || n = 2415)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2408)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2410)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-12)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2411)
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
.elseif(n = 2413)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2418)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2420)
	; 14 bytes
	php
	pha
	clc
	lda #84 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	adc #1
	Jne *-6
	pla
	plp
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2423 || n = 2434)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2424 || n = 2444 || n = 2454)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2425)
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
.elseif(n = 2426 || n = 2438 || n = 2450)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif((n .mod 10 = 7) && ((n > 2426 && n < 2458)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2428)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2429 || n = 2441)
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
.elseif(n = 2430 || n = 2433 || n = 2440 || n = 2445 || n = 2452)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2431 || n = 2443 || n = 2455)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2432)
	; 14 bytes
	php
	sty @zptemp
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2435)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2436)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	bit $24
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2439 || n = 2449 || n = 2459)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2442 || n = 2446 || n = 2451 || n = 2456)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2448)
	; 12 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	ldx @zptemp
	plp
.elseif(n = 2453)
	; 12 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
	plp
.elseif(n = 2458)
	; 14 bytes
	php
	pha
	inc @zptemp
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
	plp
.elseif(n = 2460)
	; 14 bytes
	php
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	pla
	tax
	pla
	plp
.endif
.else
; 2461 to 2500
.if((n .mod 11 = 8) && ((n > 2460 && n < 2495)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif((n .mod 12 = 2) && ((n > 2461 && n < 2499)))
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2463)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2464 || n = 2484)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2465 || n = 2477 || n = 2489)
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
.elseif(n = 2466)
	; 14 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2467 || n = 2479 || n = 2491)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2468 || n = 2473 || n = 2480 || n = 2490)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2469 || n = 2499)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2470 || n = 2481)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2471 || n = 2476 || n = 2485 || n = 2488 || n = 2493 || n = 2496)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2475)
	; 14 bytes
	php
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2478 || n = 2500)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2482)
	; 14 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2487)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2492)
	; 13 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	bit $24
	dey
	Jne *-4
	ldy @zptemp
	plp
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
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	cmp $C5
	dey
	Jne *-4
	tay
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
.elseif(n = 2502 || n = 2528)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2502 && n < 2540)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2504 || n = 2524)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2505 || n = 2516 || n = 2538)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2506 || n = 2520)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2507 || n = 2517)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2508)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2509 || n = 2519 || n = 2529)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2510 || n = 2522 || n = 2534)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2511)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2512)
	; 13 bytes
	php
	sty @zptemp
	ldy #250 ;hides 'nop'
	nop $04
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2514 || n = 2536)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2518 || n = 2521 || n = 2531)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2523)
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
.elseif(n = 2526)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2530)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2532)
	; 14 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	cmp $C5
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2533)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2535)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540 || n = 2541)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2542)
	; 13 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2543)
	; 14 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2544 || n = 2555)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2545)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 4
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2546)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2547 || n = 2557)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/10
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2548)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 2551)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2552)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2553)
	; 13 bytes
	php
	stx @zptemp
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	ldx @zptemp
	plp
.elseif(n = 2554)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/10
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2556)
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	cmp $C5
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2558)
	; 13 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	tax
	pla
	plp
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2560 || n = 2571)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2561 || n = 2573)
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
.elseif(n = 2562)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2563 || n = 2575)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2564)
	; 13 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2565)
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
.elseif(n = 2566)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
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
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	nop
	cmp $C5
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2569)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2570)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2572)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2574)
	; 14 bytes
	php
	sty @zptemp
	ldy #0
	nop
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2576)
	; 13 bytes
	php
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2577)
	; 13 bytes
	php
	pha
	sec
	lda #0
	inc @zptemp
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2578)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2580 || n = 2602 || n = 2613)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2581 || n = 2595)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2582 || n = 2594 || n = 2606)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2583 || n = 2600 || n = 2605)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2584 || n = 2596 || n = 2601)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2585 || n = 2597 || n = 2609)
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
.elseif(n = 2586)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-12)/11 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2587 || n = 2599 || n = 2611)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2588)
	; 13 bytes
	php
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2589)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/11 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2590)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2591)
	; 13 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2592)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	inc @zptemp
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2593)
	; 13 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2598)
	; 14 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2603)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2604 || n = 2615)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2607)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2608)
	; 13 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2610)
	; 13 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2612)
	; 13 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2614)
	; 14 bytes
	php
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2616)
	; 14 bytes
	php
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	inc @zptemp
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2617)
	; 13 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	inc $F6,X
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 2618 to 2656
.if((n .mod 12 = 2) && ((n > 2617 && n < 2655)))
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2619)
	; 13 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2620)
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
	tay
	pla
	plp
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
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #10
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 2623 || n = 2635 || n = 2647)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2624 || n = 2646)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2625 || n = 2634 || n = 2638 || n = 2640)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2626 || n = 2637 || n = 2648)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
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
.elseif(n = 2628 || n = 2629 || n = 2639 || n = 2644 || n = 2650)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2631)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/12 ;hides 'nop'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2632 || n = 2643)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2636)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2641)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2649)
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
.elseif(n = 2651)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2652)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2653)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 4
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2655)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2656)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	tax
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if((n .mod 12 = 5) && ((n > 2656 && n < 2694)))
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
.elseif(n = 2658)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 4
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2696)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2660 || n = 2674 || n = 2688)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2661 || n = 2672 || n = 2673 || n = 2680 || n = 2694)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2662)
	; 14 bytes
	php
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2663 || n = 2677 || n = 2691)
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
.elseif(n = 2664)
	; 14 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2665)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2666 || n = 2678 || n = 2690)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2667)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	delay_n 4
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2668 || n = 2679)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2670 || n = 2692)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2675)
	; 14 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2676 || n = 2687)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2682)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2684)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2685)
	; 13 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2686)
	; 14 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2689)
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
.endif
.else
; 2696 to 2734
.if(n = 2696)
	; 13 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2697 || n = 2704 || n = 2711 || n = 2727 || n = 2728)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2698)
	; 13 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2699)
	; 13 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2700)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-16)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 11 = 6) && ((n > 2700 && n < 2735)))
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2702 || n = 2714 || n = 2726)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2703 || n = 2725)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2705 || n = 2717 || n = 2729)
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
.elseif(n = 2706 || n = 2715 || n = 2732)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2707 || n = 2719 || n = 2731)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2708)
	; 14 bytes
	php
	sty @zptemp
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2709)
	; 13 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	inc $EA,X ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2710)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2713)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2716 || n = 2730)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2718)
	; 14 bytes
	php
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	ldx @zptemp
	plp
.elseif(n = 2720)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2721)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2722)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2724)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	iny
	Jne *-5
	tay
	pla
	plp
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
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735 || n = 2749 || n = 2763)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 11 = 8) && ((n > 2735 && n < 2770)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2737)
	; 14 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2738 || n = 2750 || n = 2762)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2739)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2740)
	; 13 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2741 || n = 2753 || n = 2765)
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
.elseif(n = 2742)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2743 || n = 2755 || n = 2767)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2744 || n = 2772)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
	; 14 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	tax
	pla
	plp
.elseif(n = 2748)
	; 14 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2751)
	; 14 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2752 || n = 2757 || n = 2771)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2754)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2756)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2759)
	; 14 bytes
	php
	sty @zptemp
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2760)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2761)
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
.elseif(n = 2764)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2766)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2768)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2770)
	; 14 bytes
	php
	pha
	sec
	lda #212 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2773)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 2774 to 2812
.if((n .mod 12 = 2) && ((n > 2773 && n < 2811)))
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2775)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	inc @zptemp
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2776 || n = 2781 || n = 2782 || n = 2788 || n = 2793 || n = 2804)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2778 || n = 2800 || n = 2811)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2779 || n = 2791 || n = 2803)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2780 || n = 2802)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2783 || n = 2792)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2784)
	; 13 bytes
	php
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2785)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2787)
	; 13 bytes
	php
	pha
	clc
	lda #58 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2790)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2794)
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2795)
	; 14 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2796)
	; 14 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	cmp $C5
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2797 || n = 2808)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2799)
	; 13 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	inc @zptemp
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 2805)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2806)
	; 14 bytes
	php
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2807)
	; 14 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2809)
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
.elseif(n = 2812)
	; 13 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 2814 || n = 2828 || n = 2842)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif((n .mod 12 = 7) && ((n > 2814 && n < 2852)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2816)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2817)
	; 13 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2818)
	; 14 bytes
	php
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2819 || n = 2847)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2820)
	; 14 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	cmp $C5
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2821 || n = 2850)
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
.elseif(n = 2822 || n = 2834 || n = 2846)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2823)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/12 ;hides 'nop'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2824)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2826 || n = 2829 || n = 2836 || n = 2841 || n = 2848)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2830)
	; 14 bytes
	php
	sty @zptemp
	ldy #0
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2831 || n = 2845)
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
.elseif(n = 2832)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2833)
	; 13 bytes
	php
	pha
	sec
	lda #0
	inc $F6,X
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2835)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2838)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2840)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2843)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2844)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop
	nop $04
	iny
	Jne *-5
	ldy @zptemp
	plp
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2857 || n = 2862 || n = 2888)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2853 || n = 2860 || n = 2865 || n = 2872 || n = 2877 || n = 2881 || n = 2886)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854)
	; 13 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	ldx @zptemp
	plp
.elseif(n = 2855 || n = 2867 || n = 2869 || n = 2874 || n = 2880 || n = 2883)
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
.elseif(n = 2856)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2858 || n = 2870 || n = 2882)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2859)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
	pla
	plp
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
.elseif(n = 2863 || n = 2875 || n = 2887)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2864 || n = 2890)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2866)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2868)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	.byte $0C,$E6,@zptemp ;nop abs; hides 'inc @zptemp'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2871)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	inc @zptemp
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2876)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	inc @zptemp
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 2878)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2879)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #11
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 2889)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
; 2891 to 2968
.if(n = 2891)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2892 || n = 2905)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2893 || n = 2895 || n = 2910 || n = 2919 || n = 2922 || n = 2934 || n = 2946 || n = 2964 || n = 2965 || n = 2967)
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
.elseif((n .mod 12 = 2) && ((n > 2893 && n < 2967)))
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2896 || n = 2908 || n = 2913 || n = 2920 || n = 2925 || n = 2932 || n = 2937 || n = 2944 || n = 2949 || n = 2956 || n = 2961)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 5) && ((n > 2896 && n < 2958)))
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
.elseif((n .mod 14 = 0) && ((n > 2897 && n < 2941) || n = 2968))
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif((n .mod 12 = 7) && ((n > 2898 && n < 2960)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2900 || n = 2902 || n = 2914 || n = 2936 || n = 2950 || n = 2962)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 14 = 3) && ((n > 2900 && n < 2944)))
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
.elseif(n = 2903 || n = 2917 || n = 2931)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2904)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2907 || n = 2924 || n = 2941)
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
.elseif(n = 2916 || n = 2955)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2927)
	; 14 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $F6F6 ;hides 'inc $F6,X'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2928)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2938)
	; 14 bytes
	php
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2939)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2948)
	; 14 bytes
	php
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2951)
	; 14 bytes
	php
	pha
	sec
	lda #226 ;hides 'nop #imm'
	nop $F6F6 ;hides 'inc $F6,X'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2952)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2953)
	; 14 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 2958)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2960)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2963)
	; 14 bytes
	php
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if((n .mod 12 = 5) && ((n > 2968 && n < 3042)))
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
.elseif(n = 2970 || n = 3022 || n = 3035)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3044)))
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2972 || n = 2989 || n = 3006 || n = 3023)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 2973 || n = 2987 || n = 3001)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2974 || n = 2984 || n = 2986 || n = 2998 || n = 3008 || n = 3018 || n = 3032 || n = 3034 || n = 3042 || n = 3044)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 2975)
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
.elseif(n = 2976 || n = 3000 || n = 3003 || n = 3025 || n = 3030 || n = 3037)
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
.elseif(n = 2977 || n = 3011)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 12 = 2) && ((n > 2977 && n < 3039)))
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 2979)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2980 || n = 2992 || n = 2997 || n = 3004 || n = 3009 || n = 3016 || n = 3021 || n = 3028 || n = 3040 || n = 3045)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 14 = 0) && ((n > 2981 && n < 3025)))
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif((n .mod 14 = 3) && ((n > 2984 && n < 3028)))
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
.elseif(n = 2988)
	; 14 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2991)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2994 || n = 3046)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3012)
	; 14 bytes
	php
	sty @zptemp
	ldy #250 ;hides 'nop'
	nop
	nop $04
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3015)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/12 ;hides 'nop'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3020)
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	dex
	dex
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3033)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	inc $F6,X
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3036)
	; 13 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3039)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047 || n = 3049 || n = 3051 || n = 3063)
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
.elseif(n = 3048 || n = 3061)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3050 || n = 3062 || n = 3074)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-14)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 3052 || n = 3066 || n = 3080)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
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
	; 14 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3055 || n = 3067 || n = 3079)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-19)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3056 || n = 3058 || n = 3068 || n = 3070)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3057)
	; 13 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3059)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/13 ;hides 'nop'
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3060)
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
.elseif(n = 3064)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-12)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3069)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3071 || n = 3085)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3072)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3073)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3075)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	nop
	inc $F6,X
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3076 || n = 3081)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3078)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3082)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $EA ;hides 'nop'
	nop
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3083)
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
.elseif(n = 3084)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 3086 to 3125
.if(n = 3086)
	; 13 bytes
	php
	sty @zptemp
	ldy #0
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
	plp
.elseif(n = 3087 || n = 3100)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3088 || n = 3093)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3090 || n = 3095 || (n > 3100 && n < 3108) || n = 3109 || n = 3110 || (n > 3113 && n < 3122) || n = 3123)
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
.elseif(n = 3091)
	; 13 bytes
	php
	pha
	tya
	ldy #0
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3092)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $F6F6 ;hides 'inc $F6,X'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3094 || n = 3108 || n = 3122)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3096)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	inc $F6,X
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3097 || n = 3111 || n = 3125)
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
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3099 || n = 3113)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3112)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3124)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
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
.if(n = 3126 || n = 3152)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3127 || n = 3155)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3128 || n = 3162)
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
.elseif(n = 3129 || (n > 3130 && n < 3136) || n = 3140 || n = 3146 || n = 3148 || n = 3149 || n = 3151 || n = 3154 || (n > 3155 && n < 3159) || n = 3160 || n = 3161)
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
.elseif(n = 3130 || n = 3147)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3136)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #12
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 3137 || n = 3163)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3138 || n = 3143)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3139 || n = 3144)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3142 || n = 3159)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3145)
	; 14 bytes
	php
	sty @zptemp
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3150 || n = 3164)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3153)
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
.endif
.else
; 3165 to 3203
.if(n = 3165 || n = 3191)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3166)
	; 14 bytes
	php
	sty @zptemp
	ldy #90 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3167 || n = 3181 || n = 3195)
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
.elseif(n = 3168 || n = 3170 || n = 3172 || n = 3174 || n = 3175 || n = 3177 || n = 3190 || n = 3194 || n = 3199 || n = 3200 || n = 3203)
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
.elseif(n = 3169 || n = 3183 || n = 3197)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3171)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3173)
	; 14 bytes
	php
	sty @zptemp
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3176 || n = 3189 || n = 3202)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3178 || n = 3192)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3179)
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
.elseif(n = 3180)
	; 14 bytes
	php
	sty @zptemp
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3182)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3184)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 4
	nop
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3185)
	; 14 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3186)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3187)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	delay_n 4
	nop
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3188)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3193)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3196)
	; 14 bytes
	php
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3198)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3201)
	; 14 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 3204) && (n < 3243))
; 3204 to 3242
.if(n = 3204 || n = 3230)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3205 || n = 3207 || n = 3214 || n = 3216 || n = 3218 || n = 3219 || n = 3221 || n = 3222 || n = 3226 || n = 3229 || n = 3231 || n = 3233)
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
.elseif(n = 3206 || n = 3220 || n = 3234)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3208)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $F6F6 ;hides 'inc $F6,X'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3209 || n = 3223)
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
.elseif(n = 3210)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3211 || n = 3225)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3212)
	; 14 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3213)
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
.elseif(n = 3215 || n = 3228)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3217)
	; 14 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3224)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3227)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3232)
	; 13 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3235)
	; 13 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	delay_n 7
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3236)
	; 14 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	nop
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3237)
	; 13 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3238 || n = 3240 || n = 3242)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3239)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3241)
	; 13 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3256 || n = 3269)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3244 || n = 3261 || n = 3278)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3245 || n = 3246 || n = 3250 || n = 3252 || n = 3258 || n = 3259 || n = 3272 || n = 3273 || n = 3275 || n = 3277)
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
.elseif(n = 3247 || n = 3264)
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
.elseif(n = 3248 || n = 3262 || n = 3276)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3249 || n = 3266)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
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
.elseif(n = 3253 || n = 3281)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3254 || n = 3280)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3255)
	; 14 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3257)
	; 14 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3260)
	; 13 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 3265)
	; 13 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3267)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/13 ;hides 'nop'
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3268 || n = 3270)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3271)
	; 14 bytes
	php
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3274)
	; 14 bytes
	php
	pha
	clc
	lda #52 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3283 || n = 3300 || n = 3317)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3284 || n = 3286 || n = 3289 || n = 3299 || n = 3301 || n = 3302 || n = 3305 || n = 3310 || n = 3314 || n = 3320)
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
.elseif(n = 3285)
	; 14 bytes
	php
	pha
	sec
	lda #n/15-1 ;hides 'nop'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3287)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/15 ;hides 'nop'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3288)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-12)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3290 || n = 3304)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3291)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3292)
	; 14 bytes
	php
	sty @zptemp
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3293)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3294 || n = 3296)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3295 || n = 3309)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3297)
	; 14 bytes
	php
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3298)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3303)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3306 || n = 3319)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3307)
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
.elseif(n = 3308)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3311)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3312)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3313)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3315)
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
.elseif(n = 3316)
	; 14 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3318)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
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
.elseif(n = 3322 || (n > 3323 && n < 3328) || n = 3330 || n = 3331 || n = 3336 || (n > 3337 && n < 3345) || n = 3348 || n = 3350 || (n > 3351 && n < 3360))
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
.elseif(n = 3323 || n = 3337 || n = 3351)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3328)
	; 14 bytes
	php
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3329)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3332 || n = 3346)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3333)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3334)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3345)
	; 14 bytes
	php
	pha
	sec
	lda #0
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3347)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
; 3360 to 3437
.if((n .mod 14 = 0) && ((n > 3359 && n < 3431)))
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3361 || n = 3362 || n = 3364 || n = 3367 || (n > 3368 && n < 3374) || n = 3375 || n = 3376 || n = 3378 || n = 3381 || n = 3382 || n = 3384 || n = 3386 || n = 3387 || n = 3389 || n = 3390 || n = 3392 || n = 3394 || n = 3403 || n = 3406 || n = 3408 || (n > 3409 && n < 3414) || n = 3422 || n = 3424 || n = 3426 || n = 3427 || n = 3429 || n = 3432 || n = 3437)
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
.elseif((n .mod 14 = 3) && ((n > 3362 && n < 3434)))
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
.elseif(n = 3365 || n = 3379 || n = 3407 || n = 3421 || n = 3435)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3366 || n = 3383 || n = 3417 || n = 3434)
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
.elseif(n = 3368 || n = 3385)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3380 || n = 3397 || n = 3414 || n = 3431)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3393)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #13
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 3395 || n = 3400)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3396 || n = 3401)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3399)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-7)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3404)
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3415)
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3418)
	; 14 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3420)
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3425)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3428)
	; 14 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3436)
	; 14 bytes
	php
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3439 || n = 3440 || n = 3442 || n = 3443 || n = 3445 || n = 3446 || n = 3450 || n = 3452 || (n > 3453 && n < 3458) || n = 3459 || n = 3460 || n = 3462 || n = 3464 || n = 3469 || n = 3474 || n = 3476)
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
.elseif(n = 3444 || n = 3458 || n = 3472)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3447 || n = 3461 || n = 3475)
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
.elseif(n = 3448)
	; 14 bytes
	php
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3449)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3451)
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
.elseif(n = 3463)
	; 13 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	ldx @zptemp
	plp
.elseif(n = 3465)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3466)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3467)
	; 14 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3468)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
	plp
.elseif(n = 3470)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
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
.elseif(n = 3473)
	; 14 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3491 || n = 3505)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n > 3477 && n < 3482) || n = 3483 || n = 3488 || n = 3493 || n = 3494 || n = 3496 || n = 3498 || n = 3501 || n = 3506 || n = 3507 || n = 3509 || n = 3510)
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
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	inc $F6,X
	dex
	dex
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3484)
	; 13 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3485 || n = 3502)
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
.elseif(n = 3486 || n = 3500 || n = 3514)
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 3490 || n = 3492)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3495)
	; 14 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	nop
	inc $F6,X
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3497)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3499)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
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
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3508)
	; 14 bytes
	php
	sty @zptemp
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3511)
	; 14 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3512)
	; 13 bytes
	php
	sty @zptemp
	ldy #(n-12)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3513)
	; 14 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3515)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/14 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3550 || n = 3567)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3517)
	; 13 bytes
	php
	pha
	tya
	ldy #(n-17)/14 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3518 || n = 3520)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 14 = 5) && ((n > 3518 && n < 3590)))
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3521 || n = 3538 || n = 3555 || n = 3572)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n > 3521 && n < 3525) || n = 3526 || n = 3529 || n = 3530 || n = 3532 || n = 3534 || n = 3535 || n = 3537 || (n > 3538 && n < 3542) || n = 3543 || n = 3544 || n = 3546 || n = 3548 || n = 3549 || n = 3552 || n = 3554 || n = 3558 || n = 3560 || (n > 3561 && n < 3567) || n = 3568 || n = 3569 || n = 3571 || n = 3574 || (n > 3575 && n < 3584) || n = 3585 || n = 3586 || n = 3588 || (n > 3589 && n < 3594))
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
.elseif(n = 3525)
	; 14 bytes
	php
	pha
	sec
	lda #n/15-1 ;hides 'nop'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3527)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/15 ;hides 'nop'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif((n .mod 14 = 0) && ((n > 3527 && n < 3585)))
	; 14 bytes
	php
	sty @zptemp
	ldy #n/14-1
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif((n .mod 14 = 3) && ((n > 3530 && n < 3588)))
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
.elseif(n = 3536 || n = 3553)
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
.elseif(n = 3551)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3557)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
; 3594 to 3671
.if((n > 3593 && n < 3598) || n = 3599 || n = 3600 || n = 3602 || n = 3605 || (n > 3606 && n < 3618) || n = 3619 || n = 3620 || n = 3622 || (n > 3623 && n < 3635) || n = 3636 || n = 3637 || n = 3639 || (n > 3640 && n < 3650) || n = 3651 || n = 3654 || n = 3656 || (n > 3658 && n < 3662) || (n > 3662 && n < 3669) || n = 3670)
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
.elseif(n = 3598)
	; 14 bytes
	php
	sty @zptemp
	ldy #0
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 3603)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3604 || n = 3621 || n = 3638)
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
.elseif(n = 3606 || n = 3623 || n = 3640)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3618 || n = 3635 || n = 3669)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3650)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #14
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 3652 || n = 3657)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3653 || n = 3658)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3662)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3671)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	tax
	pla
	plp
.endif
.else
; 3672 to 3750
.if(n = 3672 || n = 3689 || n = 3706 || n = 3723 || n = 3740)
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
.elseif(n = 3673)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $F6F6 ;hides 'inc $F6,X'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3674 || n = 3691 || n = 3725 || n = 3742)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3675)
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3676 || (n > 3677 && n < 3686) || n = 3687 || n = 3688 || n = 3690 || n = 3693 || n = 3694 || n = 3696 || (n > 3697 && n < 3703) || n = 3704 || n = 3705 || n = 3707 || (n > 3708 && n < 3713) || (n > 3713 && n < 3720) || n = 3721 || n = 3722 || n = 3726 || n = 3728 || (n > 3729 && n < 3735) || n = 3736 || n = 3738 || n = 3739 || n = 3741 || (n > 3742 && n < 3751))
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
.elseif(n = 3677)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3686 || n = 3703 || n = 3720)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3692)
	; 14 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3697)
	; 14 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3708)
	; 14 bytes
	php
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	ldx @zptemp
	plp
.elseif(n = 3713)
	; 14 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3724)
	; 14 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3729)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3735)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3737)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
; 3751 to 3906
.if((n > 3750 && n < 3754) || n = 3755 || n = 3758 || n = 3760 || (n > 3761 && n < 3765) || n = 3766 || (n > 3767 && n < 3771) || n = 3773 || n = 3775 || n = 3778 || (n > 3779 && n < 3783) || n = 3784 || n = 3785 || n = 3787 || n = 3789 || n = 3790 || n = 3792 || (n > 3793 && n < 3804) || n = 3806 || n = 3807 || n = 3811 || (n > 3812 && n < 3817) || (n > 3817 && n < 3822) || n = 3823 || n = 3824 || n = 3826 || (n > 3827 && n < 3839) || n = 3840 || n = 3841 || n = 3843 || (n > 3844 && n < 3856) || n = 3857 || n = 3858 || n = 3860 || (n > 3861 && n < 3869) || (n > 3869 && n < 3873) || n = 3875 || n = 3877 || (n > 3878 && n < 3890) || n = 3891 || n = 3892 || n = 3894 || (n > 3895 && n < 3900) || (n > 3900 && n < 3905) || n = 3906)
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
.elseif(n = 3754 || n = 3771 || n = 3788 || n = 3805 || n = 3822 || n = 3839 || n = 3856 || n = 3873 || n = 3890)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3756)
	; 14 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3757 || n = 3774 || n = 3791 || n = 3808 || n = 3825 || n = 3842 || n = 3859 || n = 3876 || n = 3893)
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
.elseif(n = 3761)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3765)
	; 14 bytes
	php
	pha
	sec
	lda #n/15-1 ;hides 'nop'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3767)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/15 ;hides 'nop'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3772)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3776 || n = 3793 || n = 3810 || n = 3827 || n = 3844 || n = 3861 || n = 3878 || n = 3895)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3777)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3779)
	; 14 bytes
	php
	pha
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3783)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3786)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 3804)
	; 14 bytes
	php
	sty @zptemp
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3809)
	; 14 bytes
	php
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3812)
	; 14 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3817)
	; 14 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3869)
	; 14 bytes
	php
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	ldx @zptemp
	plp
.elseif(n = 3874)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3900)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3905)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.else
; 3907 to 4062
.if(n = 3907)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #15
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 3908 || n = 3913 || n = 3917 || n = 3918 || n = 3920 || n = 3922 || n = 3923 || n = 3925 || n = 3926 || n = 3928 || n = 3930 || n = 3931 || (n > 3932 && n < 3937) || (n > 3937 && n < 3941) || n = 3942 || n = 3943 || n = 3945 || (n > 3946 && n < 3958) || n = 3959 || n = 3960 || n = 3962 || (n > 3963 && n < 3975) || n = 3976 || n = 3977 || n = 3979 || n = 3981 || n = 3982 || n = 3984 || (n > 3985 && n < 3992) || n = 3993 || n = 3994 || n = 3996 || (n > 3997 && n < 4009) || n = 4010 || n = 4011 || n = 4013 || n = 4016 || (n > 4017 && n < 4026) || n = 4027 || n = 4028 || n = 4030 || (n > 4031 && n < 4043) || n = 4044 || n = 4045 || n = 4047 || (n > 4048 && n < 4060) || n = 4061 || n = 4062)
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
.elseif(n = 3909 || n = 3914)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3915)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-7)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3916)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
	plp
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
.elseif(n = 3921)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3924 || n = 3941 || n = 3958 || n = 3975 || n = 3992 || n = 4009 || n = 4026 || n = 4043 || n = 4060)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 3927 || n = 3944 || n = 3961 || n = 3978 || n = 3995 || n = 4012 || n = 4029 || n = 4046)
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
.elseif(n = 3929 || n = 3946 || n = 3963 || n = 3997 || n = 4014 || n = 4031 || n = 4048)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3932)
	; 14 bytes
	php
	sty @zptemp
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3937)
	; 14 bytes
	php
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3980)
	; 14 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 3983)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3985)
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
.elseif(n = 4017)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/16 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4080 || n = 4097 || n = 4114 || n = 4131 || n = 4148 || n = 4165 || n = 4182 || n = 4199 || n = 4216 || n = 4233 || n = 4250 || n = 4267 || n = 4284 || n = 4301 || n = 4318 || n = 4335 || n = 4352)
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
.elseif(n = 4064 || (n > 4065 && n < 4077) || n = 4078 || n = 4079 || n = 4081 || (n > 4082 && n < 4094) || n = 4095 || n = 4096 || n = 4098 || (n > 4099 && n < 4111) || n = 4112 || n = 4113 || n = 4115 || n = 4117 || n = 4118 || n = 4120 || (n > 4121 && n < 4128) || n = 4129 || n = 4130 || n = 4132 || (n > 4133 && n < 4145) || n = 4146 || n = 4147 || n = 4149 || (n > 4150 && n < 4154) || n = 4155 || n = 4156 || n = 4158 || n = 4160 || n = 4161 || n = 4163 || n = 4168 || n = 4170 || (n > 4172 && n < 4179) || n = 4180 || n = 4181 || n = 4183 || (n > 4184 && n < 4196) || n = 4197 || n = 4198 || n = 4200 || (n > 4201 && n < 4213) || n = 4214 || n = 4215 || n = 4217 || (n > 4218 && n < 4230) || n = 4231 || n = 4232 || n = 4234 || (n > 4235 && n < 4247) || n = 4248 || n = 4249 || n = 4251 || (n > 4252 && n < 4264) || n = 4265 || n = 4266 || n = 4268 || (n > 4269 && n < 4281) || n = 4282 || n = 4283 || n = 4285 || (n > 4286 && n < 4298) || n = 4299 || n = 4300 || n = 4302 || (n > 4303 && n < 4315) || n = 4316 || n = 4317 || n = 4319 || (n > 4320 && n < 4332) || n = 4333 || n = 4334 || n = 4336 || (n > 4337 && n < 4349) || n = 4350 || n = 4351 || n = 4353 || (n > 4354 && n < 4366) || n = 4367 || n = 4368 || n = 4370 || (n > 4371 && n < 4376))
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
.elseif(n = 4065 || n = 4082 || n = 4099 || n = 4116 || n = 4133 || n = 4150 || n = 4184 || n = 4201 || n = 4218 || n = 4235 || n = 4252 || n = 4269 || n = 4286 || n = 4303 || n = 4320 || n = 4337 || n = 4354)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4077 || n = 4094 || n = 4111 || n = 4128 || n = 4145 || n = 4162 || n = 4179 || n = 4196 || n = 4213 || n = 4230 || n = 4247 || n = 4264 || n = 4281 || n = 4298 || n = 4315 || n = 4332 || n = 4349)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-14)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
.elseif(n = 4119)
	; 14 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4121)
	; 14 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4154)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-12)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 4157)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4159)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4164)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #16
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 4166 || n = 4171)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4167 || n = 4172)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4366)
	; 14 bytes
	php
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 4371)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
; 4376 to 5000
.if((n > 4375 && n < 4382) || n = 4383 || n = 4384 || n = 4386 || (n > 4387 && n < 4420) || n = 4422 || n = 4427 || (n > 4429 && n < 4458) || n = 4459 || n = 4460 || n = 4462 || (n > 4463 && n < 4678) || n = 4679 || n = 4682 || n = 4684 || (n > 4686 && n < 4724) || n = 4725 || n = 4726 || n = 4728 || (n > 4729 && n < 4762) || n = 4763 || n = 4764 || n = 4766 || (n > 4767 && n < 4935) || n = 4936 || n = 4939 || n = 4941 || (n > 4943 && n < 5001))
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
.elseif(n = 4382)
	; 14 bytes
	php
	sty @zptemp
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 4385)
	; 14 bytes
	php
	pha
	clc
	lda #26 ;hides 'nop'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4387)
	; 14 bytes
	php
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4420)
	; 14 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 4421)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #17
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 4423 || n = 4428 || n = 4680 || n = 4685 || n = 4937 || n = 4942)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4424 || n = 4429 || n = 4681 || n = 4686 || n = 4938 || n = 4943)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4425)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	tay
	pla
	plp
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
.elseif(n = 4458 || n = 4762)
	; 14 bytes
	php
	sty @zptemp
	ldy #(n-12)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 4461 || n = 4765)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/19 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4463 || n = 4767)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-17)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4678)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #18
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 4724)
	; 14 bytes
	php
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 4727)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4729)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4935)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #19
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.endif
.endif
.endif
.endif
.else
.if((n >= 5001) && (n < 7501))
; 5001 to 7500
.if((n > 5000 && n < 5192) || n = 5193 || n = 5196 || n = 5198 || (n > 5200 && n < 5449) || n = 5450 || n = 5453 || n = 5455 || (n > 5457 && n < 5706) || n = 5707 || n = 5710 || n = 5712 || (n > 5714 && n < 5963) || n = 5964 || n = 5967 || n = 5969 || (n > 5971 && n < 6220) || n = 6221 || n = 6224 || n = 6226 || (n > 6228 && n < 6477) || n = 6478 || n = 6481 || n = 6483 || (n > 6485 && n < 6734) || n = 6735 || n = 6738 || n = 6740 || (n > 6742 && n < 6991) || n = 6992 || n = 6995 || n = 6997 || (n > 6999 && n < 7248) || n = 7249 || n = 7252 || n = 7254 || (n > 7256 && n < 7501))
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
.elseif(n = 5192)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #20
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 5194 || n = 5199 || n = 5451 || n = 5456 || n = 5708 || n = 5713 || n = 5965 || n = 5970 || n = 6222 || n = 6227 || n = 6479 || n = 6484 || n = 6736 || n = 6741 || n = 6993 || n = 6998 || n = 7250 || n = 7255)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5195 || n = 5200 || n = 5452 || n = 5457 || n = 5709 || n = 5714 || n = 5966 || n = 5971 || n = 6223 || n = 6228 || n = 6480 || n = 6485 || n = 6737 || n = 6742 || n = 6994 || n = 6999 || n = 7251 || n = 7256)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 5449)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #21
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 5706)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #22
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 5963)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #23
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 6220)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #24
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 6477)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #25
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 6734)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #26
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 6991)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #27
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 7248)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #28
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.else
; 7501 to 10000
.if((n > 7500 && n < 7505) || n = 7506 || n = 7509 || n = 7511 || (n > 7513 && n < 7762) || n = 7763 || n = 7766 || n = 7768 || (n > 7770 && n < 8019) || n = 8020 || n = 8023 || n = 8025 || (n > 8027 && n < 8276) || n = 8277 || n = 8280 || n = 8282 || (n > 8284 && n < 8533) || n = 8534 || n = 8537 || n = 8539 || (n > 8541 && n < 8790) || n = 8791 || n = 8794 || n = 8796 || (n > 8798 && n < 9047) || n = 9048 || n = 9051 || n = 9053 || (n > 9055 && n < 9304) || n = 9305 || n = 9308 || n = 9310 || (n > 9312 && n < 9561) || n = 9562 || n = 9565 || n = 9567 || (n > 9569 && n < 9818) || n = 9819 || n = 9822 || n = 9824 || (n > 9826 && n < 10001))
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
.elseif(n = 7505)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #29
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 7507 || n = 7512 || n = 7764 || n = 7769 || n = 8021 || n = 8026 || n = 8278 || n = 8283 || n = 8535 || n = 8540 || n = 8792 || n = 8797 || n = 9049 || n = 9054 || n = 9306 || n = 9311 || n = 9563 || n = 9568 || n = 9820 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7508 || n = 7513 || n = 7765 || n = 7770 || n = 8022 || n = 8027 || n = 8279 || n = 8284 || n = 8536 || n = 8541 || n = 8793 || n = 8798 || n = 9050 || n = 9055 || n = 9307 || n = 9312 || n = 9564 || n = 9569 || n = 9821 || n = 9826)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 7762)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #30
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 8019)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #31
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 8276)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #32
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 8533)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #33
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 8790)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #34
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 9047)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #35
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 9304)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #36
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 9561)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #37
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 9818)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #38
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.if((n >= 10001) && (n < 12501))
; 10001 to 12500
.if((n > 10000 && n < 10075) || n = 10076 || n = 10079 || n = 10081 || (n > 10083 && n < 10332) || n = 10333 || n = 10336 || n = 10338 || (n > 10340 && n < 10589) || n = 10590 || n = 10593 || n = 10595 || (n > 10597 && n < 10846) || n = 10847 || n = 10850 || n = 10852 || (n > 10854 && n < 11103) || n = 11104 || n = 11107 || n = 11109 || (n > 11111 && n < 11360) || n = 11361 || n = 11364 || n = 11366 || (n > 11368 && n < 11617) || n = 11618 || n = 11621 || n = 11623 || (n > 11625 && n < 11874) || n = 11875 || n = 11878 || n = 11880 || (n > 11882 && n < 12131) || n = 12132 || n = 12135 || n = 12137 || (n > 12139 && n < 12388) || n = 12389 || n = 12392 || n = 12394 || (n > 12396 && n < 12501))
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
.elseif(n = 10075)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #39
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 10077 || n = 10082 || n = 10334 || n = 10339 || n = 10591 || n = 10596 || n = 10848 || n = 10853 || n = 11105 || n = 11110 || n = 11362 || n = 11367 || n = 11619 || n = 11624 || n = 11876 || n = 11881 || n = 12133 || n = 12138 || n = 12390 || n = 12395)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10078 || n = 10083 || n = 10335 || n = 10340 || n = 10592 || n = 10597 || n = 10849 || n = 10854 || n = 11106 || n = 11111 || n = 11363 || n = 11368 || n = 11620 || n = 11625 || n = 11877 || n = 11882 || n = 12134 || n = 12139 || n = 12391 || n = 12396)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 10332)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #40
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 10589)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #41
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 10846)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #42
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 11103)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #43
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 11360)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #44
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 11617)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #45
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 11874)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #46
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 12131)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #47
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 12388)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #48
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.endif
.else
; 12501 to 15000
.if((n > 12500 && n < 12645) || n = 12646 || n = 12649 || n = 12651 || (n > 12653 && n < 12902) || n = 12903 || n = 12906 || n = 12908 || (n > 12910 && n < 13159) || n = 13160 || n = 13163 || n = 13165 || (n > 13167 && n < 13416) || n = 13417 || n = 13420 || n = 13422 || (n > 13424 && n < 13673) || n = 13674 || n = 13677 || n = 13679 || (n > 13681 && n < 13930) || n = 13931 || n = 13934 || n = 13936 || (n > 13938 && n < 14187) || n = 14188 || n = 14191 || n = 14193 || (n > 14195 && n < 14444) || n = 14445 || n = 14448 || n = 14450 || (n > 14452 && n < 14701) || n = 14702 || n = 14705 || n = 14707 || (n > 14709 && n < 14958) || n = 14959 || n = 14962 || n = 14964 || (n > 14966 && n < 15001))
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
.elseif(n = 12645)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #49
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 12647 || n = 12652 || n = 12904 || n = 12909 || n = 13161 || n = 13166 || n = 13418 || n = 13423 || n = 13675 || n = 13680 || n = 13932 || n = 13937 || n = 14189 || n = 14194 || n = 14446 || n = 14451 || n = 14703 || n = 14708 || n = 14960 || n = 14965)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12648 || n = 12653 || n = 12905 || n = 12910 || n = 13162 || n = 13167 || n = 13419 || n = 13424 || n = 13676 || n = 13681 || n = 13933 || n = 13938 || n = 14190 || n = 14195 || n = 14447 || n = 14452 || n = 14704 || n = 14709 || n = 14961 || n = 14966)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 12902)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #50
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13159)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #51
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13416)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #52
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13673)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #53
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 13930)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #54
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14187)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #55
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14444)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #56
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14701)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #57
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 14958)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #58
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.endif
.else
.if((n >= 15001) && (n < 17501))
; 15001 to 17500
.if((n > 15000 && n < 15215) || n = 15216 || n = 15219 || n = 15221 || (n > 15223 && n < 15472) || n = 15473 || n = 15476 || n = 15478 || (n > 15480 && n < 15729) || n = 15730 || n = 15733 || n = 15735 || (n > 15737 && n < 15986) || n = 15987 || n = 15990 || n = 15992 || (n > 15994 && n < 16243) || n = 16244 || n = 16247 || n = 16249 || (n > 16251 && n < 16500) || n = 16501 || n = 16504 || n = 16506 || (n > 16508 && n < 16757) || n = 16758 || n = 16761 || n = 16763 || (n > 16765 && n < 17014) || n = 17015 || n = 17018 || n = 17020 || (n > 17022 && n < 17271) || n = 17272 || n = 17275 || n = 17277 || (n > 17279 && n < 17501))
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
.elseif(n = 15215)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #59
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 15217 || n = 15222 || n = 15474 || n = 15479 || n = 15731 || n = 15736 || n = 15988 || n = 15993 || n = 16245 || n = 16250 || n = 16502 || n = 16507 || n = 16759 || n = 16764 || n = 17016 || n = 17021 || n = 17273 || n = 17278)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15218 || n = 15223 || n = 15475 || n = 15480 || n = 15732 || n = 15737 || n = 15989 || n = 15994 || n = 16246 || n = 16251 || n = 16503 || n = 16508 || n = 16760 || n = 16765 || n = 17017 || n = 17022 || n = 17274 || n = 17279)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 15472)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #60
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
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
.elseif(n = 15729)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #61
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 15986)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #62
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 16243)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #63
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 16500)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #64
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 16757)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #65
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 17014)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #66
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 17271)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #67
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.endif
.else
; 17501 to 20000
.if((n > 17500 && n < 17528) || n = 17529 || n = 17532 || n = 17534 || (n > 17536 && n < 17785) || n = 17786 || n = 17789 || n = 17791 || (n > 17793 && n < 18042) || n = 18043 || n = 18046 || n = 18048 || (n > 18050 && n < 18299) || n = 18300 || n = 18303 || n = 18305 || (n > 18307 && n < 18556) || n = 18557 || n = 18560 || n = 18562 || (n > 18564 && n < 18813) || n = 18814 || n = 18817 || n = 18819 || (n > 18821 && n < 19070) || n = 19071 || n = 19074 || n = 19076 || (n > 19078 && n < 19327) || n = 19328 || n = 19331 || n = 19333 || (n > 19335 && n < 19584) || n = 19585 || n = 19588 || n = 19590 || (n > 19592 && n < 19841) || n = 19842 || n = 19845 || n = 19847 || (n > 19849 && n < 20001))
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
.elseif(n = 17528)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #68
	jsr delay_256x_a_30_clocks
	ldx @zptemp
	pla
	plp
.elseif(n = 17530 || n = 17535 || n = 17787 || n = 17792 || n = 18044 || n = 18049 || n = 18301 || n = 18306 || n = 18558 || n = 18563 || n = 18815 || n = 18820 || n = 19072 || n = 19077 || n = 19329 || n = 19334 || n = 19586 || n = 19591 || n = 19843 || n = 19848)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17531 || n = 17536 || n = 17788 || n = 17793 || n = 18045 || n = 18050 || n = 18302 || n = 18307 || n = 18559 || n = 18564 || n = 18816 || n = 18821 || n = 19073 || n = 19078 || n = 19330 || n = 19335 || n = 19587 || n = 19592 || n = 19844 || n = 19849)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 17785)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #69
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18042)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #70
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18299)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #71
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18556)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #72
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 18813)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #73
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19070)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #74
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19327)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #75
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19584)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #76
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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
.elseif(n = 19841)
	; 13 bytes
	php
	pha
	stx @zptemp
	lax #77
	jsr delay_256x_a_30_clocks
	ldx @zptemp
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

.endif
	rest
.endmacro
