; delay_n, delays n cycles with small code
; Will clobber S, Z+N, C, and V
; Will preserve A, X, Y, D, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
;
; WARNING: May write garbage into $00-$FF.
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
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 31
; - 7 bytes or less when n <= 289
; - 8 bytes or less when n <= 292
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 320
; -11 bytes or less when n <= 1564
; -12 bytes or less when n <= 2835
; -13 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=3.9, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=6
; - When 31 <= n <= 50: min=6, avg=6.9, max=7
; - When 51 <= n <= 75: min=7, avg=7.0, max=7
; - When 76 <= n <= 100: min=7, avg=7.0, max=7
; - When 101 <= n <= 200: min=7, avg=7.0, max=7
; - When 201 <= n <= 300: min=7, avg=7.2, max=10
; - When 301 <= n <= 500: min=8, avg=9.6, max=11
; - When 501 <= n <= 750: min=8, avg=9.6, max=11
; - When 751 <= n <= 1000: min=9, avg=9.7, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.3, max=12
; - When 2001 <= n <= 3000: min=10, avg=11.3, max=13
; - When 3001 <= n <= 5000: min=11, avg=12.5, max=13
; - When 5001 <= n <= 7500: min=12, avg=13.0, max=13
; - When 7501 <= n <= 10000: min=13, avg=13.0, max=13
; - When 10001 <= n <= 20000: min=13, avg=13.0, max=13
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
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 32)
	; 2..7 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	inc $F6,X
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif((n > 8 && n < 12) || n = 19 || (n > 20 && n < 24) || n = 25)
	; 3..6 bytes
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
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 20 || n = 27 || n = 29 || n = 30)
	; 4..6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 24)
	; 6 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28)
	; 6 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif((n > 33 && n < 46) || (n > 46 && n < 157))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
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
.if((n > 156 && n < 290))
	; 7 bytes
	pha
	lda #n-34
	jsr delay_a_25_clocks
	pla
.elseif(n = 290 || n = 291 || n = 309)
	; 8..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 292 || n = 298)
	; 8..9 bytes
	pha
	delay_n (n-3)
.elseif(n = 293 || n = 294 || n = 296)
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
.elseif(n = 297 || n = 312)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 299 || n = 301)
	; 10 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 300 || n = 310)
	; 9 bytes
	pha
	sec
	lda #n/5-3
	sbc #1
	Jpl *-2
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
.elseif(n = 303 || n = 304)
	; 10 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 305)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
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
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 311)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 314 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 339 || n = 344 || n = 349 || n = 354 || n = 356 || n = 359 || n = 364 || n = 366 || n = 374 || n = 379 || n = 384 || n = 386 || n = 389)
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
.elseif(n = 320 || n = 325 || n = 330 || n = 345 || n = 360)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif((n .mod 5 = 3) && ((n > 322 && n < 339) || (n > 357 && n < 374) || n = 388))
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 336 || n = 343 || n = 378)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 346)
	; 9 bytes
	pha
	sec
	lda #$E9 ;hides 'sbc #$B8'
	clv ;first loop only
	nop
	Jne *-3
	pla
.elseif(n = 351)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 353 || n = 381)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
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
.elseif(n = 371)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 385)
	; 8 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jmi *-1
	ldx @zptemp
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
.if(n = 391 || n = 396)
	; 10 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 392 || n = 407 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 5 = 3) && ((n > 392 && n < 409) || (n > 427 && n < 444) || n = 463))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 394 || n = 399 || n = 414 || n = 419 || n = 421 || n = 424 || n = 429 || n = 434 || n = 436 || n = 444 || n = 449 || n = 454 || n = 456 || n = 459 || n = 464)
	; 10..11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 401)
	; 10 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 404)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 413)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 416)
	; 10 bytes
	pha
	sec
	lda #(n-17)/7
	nop
	sbc #1
	Jpl *-3
	pla
.elseif(n = 418)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 423 || n = 451 || n = 458)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 426)
	; 10 bytes
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
	pla
.elseif(n = 430 || n = 435 || n = 460)
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 431)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 441 || n = 448)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 461)
	; 9 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	pha
	Jmi *-2
	ldy @zptemp
.elseif(n = 466)
	; 10 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	sta @zptemp
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
.if(n = 469 || n = 479 || n = 484 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541)
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
.elseif(n = 471)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 472 || n = 482 || n = 497 || n = 507 || n = 527)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 473)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	iny
	Jpl *-3
	ldy @zptemp
.elseif(n = 474 || n = 481 || n = 523)
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
.elseif(n = 476 || n = 518 || n = 546)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif((n .mod 5 = 3) && ((n > 497 && n < 514) || (n > 532 && n < 544)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 509)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
	ldy @zptemp
.elseif(n = 511)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 514)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
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
.elseif(n = 548 || n = 568 || n = 573 || n = 578 || n = 583 || n = 603 || n = 608)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 549 || n = 554 || n = 559 || n = 564 || n = 569 || n = 574 || n = 584 || n = 589 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 619 || n = 624)
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
.elseif(n = 553 || n = 588 || n = 616 || n = 623)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 566)
	; 10 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	nop
	dey
	Jmi *-3
	ldy @zptemp
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
.elseif(n = 581)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 598)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 606)
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
	; 9 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	pha
	Jmi *-2
	ldx @zptemp
.elseif(n = 618)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	sta @zptemp
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
; 626 to 781
.if(n = 626 || n = 633 || n = 661 || n = 668 || n = 766 || n = 773)
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
.elseif(n = 629 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 664 || n = 666 || n = 669 || n = 674 || n = 676 || n = 679 || n = 681 || n = 689 || n = 694 || n = 699 || n = 704 || n = 706 || n = 709 || n = 714 || n = 716 || n = 724 || n = 729 || n = 734 || n = 739 || n = 744 || n = 746 || n = 749 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
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
.elseif(n = 631 || n = 671 || n = 711)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 638 || n = 643 || n = 648 || n = 673 || n = 678 || n = 683 || n = 688 || n = 693 || n = 698 || n = 708 || n = 713 || n = 718 || n = 723 || n = 741 || n = 743 || n = 748 || n = 753 || n = 758 || n = 778)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 778)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 649 || n = 663 || n = 684 || n = 719 || n = 726 || n = 761 || n = 768)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 5 = 0) && (n = 650 || (n > 659 && n < 686) || (n > 699 && n < 781)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 651 || n = 686 || n = 728)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 653)
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	iny
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 690)
	; 8 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	Jne *-1
	ldy @zptemp
.elseif(n = 695)
	; 8 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	pla
.elseif(n = 696 || n = 703 || n = 731)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 701)
	; 10 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 721 || n = 756 || n = 763)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 733)
	; 9 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 751)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 754)
	; 9 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	ldy @zptemp
.elseif(n = 759)
	; 9 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	tay
	pla
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
.elseif((n .mod 5 = 3) && (n = 783 || (n > 812 && n < 829) || (n > 847 && n < 864) || (n > 882 && n < 899) || (n > 917 && n < 934)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 784 || n = 794 || n = 799 || n = 804 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 834 || n = 839 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 864 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 909 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
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
.elseif(n = 786)
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
	; 10 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jmi *-3
	ldy @zptemp
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
.elseif(n = 791 || n = 826 || n = 931)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 798 || n = 833 || n = 861 || n = 868 || n = 896 || n = 903)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 801 || n = 836 || n = 871 || n = 878 || n = 913)
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
.elseif(n = 806)
	; 10 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jmi *-3
	ldy @zptemp
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
.elseif(n = 816)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 841)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	pha
	pha
	Jmi *-3
	ldx @zptemp
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
.elseif(n = 911)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.endif
.endif
.else
; 938 to 1250
.if(n = 938 || n = 966 || n = 1001 || n = 1008 || n = 1036 || n = 1043 || n = 1071 || n = 1078 || n = 1106 || n = 1113 || n = 1141 || n = 1148 || n = 1176 || n = 1183 || n = 1211 || n = 1218 || n = 1246)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 939 || n = 944 || n = 949 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 994 || n = 1004 || n = 1009 || n = 1014 || n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1029 || n = 1039 || n = 1044 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091 || n = 1094 || n = 1096 || n = 1099 || n = 1109 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1149 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1179 || n = 1184 || n = 1189 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1224 || n = 1226 || n = 1229 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 939 && n < 1016) || (n > 1029 && n < 1251)))
	; 9 bytes
	pha
	sec
	lda #n/5-2
	sbc #1
	Jne *-2
	pla
.elseif(n = 941 || n = 948 || n = 976 || n = 983 || n = 1011 || n = 1018 || n = 1046 || n = 1053 || n = 1088 || n = 1116 || n = 1123 || n = 1151 || n = 1158 || n = 1186 || n = 1193 || n = 1221 || n = 1228)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1248)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && (n = 943 || n = 971 || n = 978 || (n > 998 && n < 1014) || (n > 1033 && n < 1049) || (n > 1068 && n < 1084) || (n > 1103 && n < 1119) || (n > 1138 && n < 1154) || (n > 1173 && n < 1189) || (n > 1208 && n < 1224) || n = 1244))
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
.elseif(n = 951 || n = 991)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif((n .mod 5 = 3) && ((n > 952 && n < 964) || (n > 987 && n < 1004) || (n > 1022 && n < 1039) || n = 1068 || n = 1073 || (n > 1092 && n < 1109) || (n > 1127 && n < 1144) || (n > 1162 && n < 1179) || (n > 1197 && n < 1214) || n = 1233 || n = 1248))
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 964)
	; 9 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	Jne *-2
	ldy @zptemp
.elseif(n = 968)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	dey
	Jmi *-3
	ldy @zptemp
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
.elseif(n = 1020)
	; 8 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	ldx @zptemp
.elseif(n = 1025)
	; 8 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
.elseif(n = 1026)
	; 10 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 1031)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 1058)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1101)
	; 9 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
	ldy @zptemp
.elseif(n = 1191 || n = 1231)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1238)
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 4
	Jne *-3
	ldy @zptemp
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
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 7 = 5) && (n = 1251 || n = 1258 || n = 1279 || n = 1286 || (n > 1299 && n < 1329)))
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
.elseif((n .mod 7 = 0) && (n = 1253 || (n > 1280 && n < 1324)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1254 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1274 || n = 1276 || n = 1284 || n = 1289 || n = 1294 || n = 1304 || n = 1313 || n = 1325)
	; 10..11 bytes
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
.elseif(n = 1268 || n = 1273 || n = 1278 || n = 1283 || n = 1299 || n = 1301)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1271 || n = 1303 || n = 1311 || n = 1327)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1293)
	; 9 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1296)
	; 9 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1297)
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
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
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
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $6DA0'
	ldy #109
	iny
	Jne *-4
	ldy @zptemp
.elseif((n .mod 7 = 0) && ((n > 1329 && n < 1366)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1331 || n = 1336 || n = 1345)
	; 11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1338 || n = 1346)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1339 || n = 1357)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1341)
	; 11 bytes
	pha
	sec
	lda #(n-21)/11
	inc $F6,X
	sbc #1
	Jpl *-4
	pla
.elseif(n = 1343 || n = 1359 || n = 1367)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 1352)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1353 || n = 1362)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1355)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $96A0'
	ldy #150
	dey
	Jne *-4
	ldy @zptemp
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
.if((n .mod 7 = 3) && ((n > 1367 && n < 1404)))
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
.elseif((n .mod 7 = 5) && ((n > 1369 && n < 1399)))
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
.elseif((n .mod 7 = 0) && ((n > 1371 && n < 1401)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1373)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $98A0'
	ldy #152
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1374 || n = 1381 || n = 1395)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1376)
	; 11 bytes
	pha
	tya
	ldy #123
	inc $F6,X
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1378 || n = 1394 || n = 1402)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1383 || n = 1399)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1385)
	; 11 bytes
	pha
	sec
	lda #(n-21)/11
	inc $F6,X
	sbc #1
	Jpl *-4
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
.elseif(n = 1401)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	inc $F6,X
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 1405)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	ldy @zptemp
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
.endif
.else
.if((n >= 1407) && (n < 1485))
.if((n >= 1407) && (n < 1446))
; 1407 to 1445
.if((n .mod 7 = 0) && ((n > 1406 && n < 1443)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1408 || n = 1413 || n = 1429 || n = 1434)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1430 || n = 1437)
	; 11 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1412 || n = 1419 || n = 1433 || n = 1440)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1415 || n = 1423 || n = 1439)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1416 || n = 1425 || n = 1443)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1417 || n = 1424 || n = 1438 || n = 1445)
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1418)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 1422)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1426)
	; 9 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	ldx @zptemp
.elseif(n = 1427)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $62A0'
	ldy #98
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1431)
	; 9 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	Jne *-2
	tax
	pla
.elseif(n = 1432 || n = 1441)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.endif
.else
; 1446 to 1484
.if(n = 1446)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1448 || n = 1457)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 7 = 0) && ((n > 1448 && n < 1485)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1450 || n = 1458 || n = 1474)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif((n .mod 7 = 3) && ((n > 1451 && n < 1481)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1453)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1455 || n = 1471 || n = 1479)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1465 || n = 1483)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1467)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1481)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $A4A0'
	ldy #164
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
; 1485 to 1562
.if(n = 1485)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1486 || n = 1504 || n = 1513 || n = 1549 || n = 1558)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1558)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1488 || n = 1497 || n = 1506 || n = 1542 || n = 1560)
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
.elseif((n .mod 7 = 0) && ((n > 1490 && n < 1562)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1492 || n = 1516 || n = 1532 || n = 1556)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1493 || n = 1507 || n = 1521)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1495 || n = 1511 || n = 1527 || n = 1535 || n = 1551)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1499)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $A6A0'
	ldy #166
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1500 || n = 1548)
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1509)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $A7A0'
	ldy #167
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1518)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $A8A0'
	ldy #168
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1520)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
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
.elseif(n = 1525)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	ldx @zptemp
.elseif(n = 1528 || n = 1537 || n = 1555)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1534)
	; 11 bytes
	pha
	sec
	lda #(n-22)/12
	delay_n 7
	sbc #1
	Jpl *-4
	pla
.elseif(n = 1539)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
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
.elseif(n = 1544)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $ABA0'
	ldy #171
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1553)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $ACA0'
	ldy #172
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1577 || n = 1581 || n = 1593 || n = 1597 || n = 1598 || n = 1619 || n = 1633 || n = 1640)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 3) && ((n > 1563 && n < 1628)))
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
.elseif((n .mod 7 = 5) && ((n > 1565 && n < 1623) || n = 1636))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1567 || n = 1583 || n = 1591 || n = 1607 || n = 1623 || n = 1639)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif((n .mod 7 = 0) && ((n > 1567 && n < 1639)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1569 || n = 1605 || n = 1614)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1570 || n = 1586 || n = 1602 || n = 1618 || n = 1626)
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
.elseif(n = 1574)
	; 10 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1590)
	; 10 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1600 || n = 1609)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1611 || n = 1635 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1616)
	; 10 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	ldx @zptemp
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
	sty @zptemp
	cmp #<$CD ;hides 'cmp $B4A0'
	ldy #180
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1629)
	; 9 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	ldy @zptemp
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
.endif
.else
.if((n >= 1641) && (n < 1680))
; 1641 to 1679
.if((n .mod 7 = 3) && (n = 1641 || (n > 1654 && n < 1677)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1642 || n = 1658 || n = 1674)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1643)
	; 9 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1645 || n = 1652 || n = 1659 || n = 1673)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1646)
	; 9 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
.elseif(n = 1647 || n = 1663 || n = 1679)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	Jne *-3
	ldy @zptemp
.elseif((n .mod 7 = 5) && ((n > 1649 && n < 1679)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1651 || n = 1665)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1653)
	; 11 bytes
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
.elseif(n = 1660)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1661)
	; 10 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	ldy @zptemp
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
	cmp <$CD ;hides 'cmp $B8A0'
	ldy #184
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1668 || n = 1677)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1670)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 1672)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1675)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.endif
.else
; 1680 to 1718
.if((n .mod 7 = 0) && ((n > 1679 && n < 1716)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1681 || n = 1717)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1682 || n = 1698 || n = 1714)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 7 = 3) && ((n > 1682 && n < 1719)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
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
.elseif(n = 1686)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1688)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 1689 || n = 1691 || n = 1696 || n = 1705 || n = 1710)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1695 || n = 1703)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1702)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 1707)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1709)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1712)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1716)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
.if((n >= 1719) && (n < 1758))
; 1719 to 1757
.if(n = 1719 || n = 1735 || n = 1751)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif((n .mod 7 = 0) && ((n > 1721 && n < 1758)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1723)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1724 || n = 1756)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1754)))
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
.elseif(n = 1730 || n = 1738 || n = 1754)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1731 || n = 1740 || n = 1749)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1733)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $C0A0'
	ldy #192
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1737)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 1742)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1744)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1745)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1747)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.endif
.else
; 1758 to 1796
.if(n = 1758)
	; 10 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1759 || n = 1775 || n = 1791)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif((n .mod 7 = 3) && ((n > 1759 && n < 1796)))
	; 10 bytes
	pha
	sec
	lda #(n-10)/7
	nop
	sbc #1
	Jne *-3
	pla
.elseif(n = 1761)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
.elseif(n = 1764 || n = 1778 || n = 1785 || n = 1792)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1765 || n = 1768 || n = 1773 || n = 1787)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1766)
	; 10 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1770 || n = 1786)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 1772 || n = 1780)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1777)
	; 11 bytes
	pha
	delay_n (n-3)
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1789)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1794)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1796)
	; 10 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	ldy @zptemp
.endif
.endif
.else
.if((n >= 1797) && (n < 1836))
.if((n >= 1797) && (n < 1816))
; 1797 to 1815
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
.elseif(n = 1798)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1799)
	; 10 bytes
	sty @zptemp
	ldy #0
	nop
	dey
	Jne *-2
	ldy @zptemp
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
	; 10 bytes
	pha
	sec
	lda #0
	nop
	sbc #1
	Jne *-3
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
.elseif(n = 1805)
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1806 || n = 1809)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1807 || n = 1815)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1808 || n = 1813)
	; 11 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 1814)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $C9A0'
	ldy #201
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 1816 to 1835
.if(n = 1816 || n = 1825)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1817)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1821)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1822)
	; 10 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1823 || n = 1831)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1824)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
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
.elseif(n = 1829)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1830)
	; 10 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1832)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	ldx @zptemp
.elseif(n = 1833)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	iny
	Jne *-4
	ldy @zptemp
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
	; 10 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 8 = 7) && ((n > 1838 && n < 1872)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1840 || n = 1854)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1841)
	; 11 bytes
	sty @zptemp
	ldy #204 ;hides 'cpy abs'
	cmp #$A9
	dey
	Jne *-4
	ldy @zptemp
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
	; 10 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1846 || n = 1864 || n = 1873)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif((n .mod 9 = 3) && ((n > 1847 && n < 1876)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1849)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
	; 11 bytes
	sty @zptemp
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1859)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $32A0'
	ldy #50
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1861)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1862)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1867)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1869)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1877)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1878)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	ldy @zptemp
.elseif((n .mod 8 = 7) && ((n > 1878 && n < 1912)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1880 || n = 1910)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 1881 || n = 1885 || n = 1889)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1884 || n = 1893 || n = 1902)
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
	sty @zptemp
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1888 || n = 1897)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1891 || n = 1909)
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
.elseif(n = 1899)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1904)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1905)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1907)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1912)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1913)
	; 10 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.endif
.else
.if((n >= 1915) && (n < 1934))
; 1915 to 1933
.if(n = 1915 || n = 1933)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1916 || n = 1924)
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
	; 11 bytes
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	adc #1
	Jne *-5
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
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1920 || n = 1929)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1921)
	; 11 bytes
	pha
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1922)
	; 10 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1923)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	ldy @zptemp
.elseif(n = 1925)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1926)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
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
.elseif(n = 1930)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1931)
	; 10 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1932)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 1934 to 1953
.if(n = 1934)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1935 || n = 1943 || n = 1951)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1938 || n = 1947)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1939)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1940 || n = 1948)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1941)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1942)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 1944)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
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
.elseif(n = 1946)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1949)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1950)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 1952)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1953)
	; 11 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
.if((n .mod 8 = 2) && ((n > 1953 && n < 1987)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 1956 || n = 1965 || n = 1974 || n = 1992)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1957)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 1958)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	iny
	Jne *-4
	ldy @zptemp
.elseif((n .mod 8 = 7) && ((n > 1958 && n < 1992)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1960 || n = 1969 || n = 1987)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1963 || n = 1990)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 1966)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 1976)
	; 11 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1977)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	dey
	Jne *-4
	ldy @zptemp
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
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1985)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1989)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 2006 || n = 2024)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1994 || n = 2002 || n = 2018 || n = 2026)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 1995)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1998)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 8 = 7) && ((n > 1998 && n < 2032)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 2000 || n = 2030)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
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
.elseif(n = 2003 || n = 2025)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	pha
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2008 || n = 2017)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2011)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2013)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2014)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2021)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $E0A0'
	ldy #224
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2022)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 2029)
	; 11 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 11 bytes
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2033 || n = 2043)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	pha
	Jne *-3
	ldx @zptemp
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
.elseif(n = 2037 || n = 2046)
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
.elseif(n = 2039 || n = 2047)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 2040)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2042 || n = 2050)
	; 11 bytes
	pha
	sec
	lda #(n-10)/8
	bit $24
	sbc #1
	Jne *-4
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
.elseif(n = 2048)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2049)
	; 11 bytes
	stx @zptemp
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	inx
	Jne *-4
	ldx @zptemp
.endif
.else
; 2051 to 2070
.if(n = 2051)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2052)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2053 || n = 2062)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2055)
	; 10 bytes
	sty @zptemp
	ldy #0
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 2056)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2057)
	; 11 bytes
	stx @zptemp
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2058)
	; 11 bytes
	pha
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	pla
.elseif(n = 2059 || n = 2068)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2064)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 2066)
	; 11 bytes
	stx @zptemp
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2067)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2069)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2070)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.endif
.endif
.else
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
.if(n = 2071 || n = 2080)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
	pla
.elseif(n = 2072)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2073 || n = 2082)
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
.elseif(n = 2075)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2078)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2079)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2081)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2083)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2084)
	; 10 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 2085)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2087)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.endif
.else
; 2090 to 2109
.if(n = 2090)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2091 || n = 2100 || n = 2109)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2092 || n = 2102)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2093)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2094)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2095 || n = 2104)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2096)
	; 10 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	nop
	adc #1
	Jne *-4
	pla
.elseif(n = 2097)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2101)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2103)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	iny
	Jne *-4
	ldy @zptemp
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
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2111)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2113 || n = 2131 || n = 2140)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2114)
	; 10 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
.elseif(n = 2115 || n = 2128)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2119)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2120 || n = 2130)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2121)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2123)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2125 || n = 2134 || n = 2143)
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2129)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $ECA0'
	ldy #236
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2132 || n = 2142)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2133)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
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
.elseif(n = 2137)
	; 11 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2138)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $13A0'
	ldy #19
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2139)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2141)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2147)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	iny
	Jne *-4
	ldy @zptemp
.endif
.else
; 2149 to 2187
.if(n = 2149 || n = 2167 || n = 2176 || n = 2185)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2150 || n = 2180)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2151)
	; 11 bytes
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 9 = 1) && ((n > 2151 && n < 2180)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2155)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2156)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2157 || n = 2168)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
.elseif(n = 2159 || n = 2164 || n = 2169 || n = 2186)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2162)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2165)
	; 11 bytes
	stx @zptemp
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2166)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2173)
	; 11 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2174)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2175)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2177)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
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
.elseif(n = 2183)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2187)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
.if((n >= 2188) && (n < 2227))
; 2188 to 2226
.if((n .mod 9 = 1) && ((n > 2187 && n < 2225)))
	; 11 bytes
	pha
	sec
	lda #(n-10)/9
	delay_n 4
	sbc #1
	Jne *-4
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
.elseif(n = 2191)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2192 || n = 2202 || n = 2222)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2193)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2194 || n = 2218)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2195)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	nop
	iny
	Jne *-5
	ldy @zptemp
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
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
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
.elseif(n = 2203 || n = 2212)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2204 || n = 2214)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2205)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2213)
	; 11 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2216)
	; 11 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2219)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2223)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
.if((n >= 2227) && (n < 2246))
; 2227 to 2245
.if(n = 2227)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2228)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2229)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2230)
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
.elseif(n = 2232)
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
.elseif(n = 2235 || n = 2244)
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
.elseif(n = 2237)
	; 11 bytes
	stx @zptemp
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2238)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	pha
	pha
	Jne *-3
	ldx @zptemp
.elseif(n = 2239)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2240)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2241)
	; 11 bytes
	pha
	delay_n (n-3)
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 2246 to 2265
.if(n = 2246)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2247)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2248 || n = 2257)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 2253 || n = 2262)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2254)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2255)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2256)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2258 || n = 2259)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2263)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2264)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2265)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2267)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2268)
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
.elseif(n = 2271 || n = 2280)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2272 || n = 2282)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2273)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2274)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2275 || n = 2284)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2276)
	; 11 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2277)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2283)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	iny
	Jne *-4
	ldy @zptemp
.endif
.else
; 2285 to 2304
.if(n = 2285)
	; 11 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2286)
	; 11 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2289 || n = 2298)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 2292 || n = 2304)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2293)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2295)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2299)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2300)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2302)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2303)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
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
.elseif(n = 2309)
	; 11 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2310)
	; 11 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2311)
	; 11 bytes
	sty @zptemp
	ldy #0
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2312 || n = 2322)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2313 || n = 2318)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2317)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2319)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2320)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
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
.elseif(n = 2323)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.endif
.else
; 2324 to 2343
.if(n = 2324)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2325)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2326 || n = 2338)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2327)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	pha
	delay_n (n-3)
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
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2332)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2333)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2334)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
	Jne *-4
	ldy @zptemp
.elseif(n = 2335)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	nop
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	nop
	adc #1
	Jne *-6
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
.elseif(n = 2343)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D4A0'
	ldy #212
	nop
	dey
	Jne *-5
	tay
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2345)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2346)
	; 12 bytes
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2347 || n = 2359 || n = 2371)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2348)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jne *-5
	pla
.elseif(n = 2349 || n = 2354 || n = 2368 || n = 2379)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2356)
	; 12 bytes
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2357 || n = 2367)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2358)
	; 12 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2360 || n = 2370 || n = 2380)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2361)
	; 11 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	inc $EA,X ;hides 'nop'
	dey
	Jne *-4
	tay
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
.elseif(n = 2369)
	; 12 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2372 || n = 2382)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2373)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2375)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
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
.elseif(n = 2381)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	nop
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 2383 to 2421
.if((n .mod 12 = 7) && ((n > 2382 && n < 2420)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2384 || n = 2385 || n = 2396 || n = 2409 || n = 2414)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2386 || n = 2398 || n = 2410)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2387 || n = 2417)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2389)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2390 || n = 2420)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2391)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2392 || n = 2402)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2393)
	; 11 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2394 || n = 2405 || n = 2416)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2397 || n = 2408)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2399 || n = 2421)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2401)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2403)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2411 || n = 2413)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2415)
	; 11 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	pha
	iny
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if(n = 2422 || n = 2434 || n = 2458)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2423 || n = 2433 || n = 2445)
	; 12 bytes
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
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $24A0'
	ldy #36
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2426)
	; 11 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	pha
	iny
	Jne *-4
	ldy @zptemp
.elseif((n .mod 10 = 7) && ((n > 2426 && n < 2458)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2428)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	adc #1
	Jne *-6
	pla
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
.elseif(n = 2430 || n = 2440 || n = 2450)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2431 || n = 2443 || n = 2455)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2432 || n = 2442 || n = 2452)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2435 || n = 2444 || n = 2449)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2438)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2439)
	; 12 bytes
	sty @zptemp
	ldy #127
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2441)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	ldx @zptemp
.elseif(n = 2446)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	Jne *-3
	tax
	pla
.elseif(n = 2451)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 2453)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2454)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2456)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2459)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	ldy @zptemp
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 12 bytes
	pha
	sec
	lda #128
	jsr @nearby_rts_14cyc
	sbc #1
	Jpl *-5
	pla
.elseif(n = 2462 || n = 2492)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2463 || n = 2474 || n = 2485)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2464 || n = 2469 || n = 2478 || n = 2481 || n = 2486 || n = 2489)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 11 = 1) && ((n > 2464 && n < 2499)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2466)
	; 12 bytes
	sty @zptemp
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2467 || n = 2479 || n = 2491)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2468 || n = 2473 || n = 2483)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2470 || n = 2482 || n = 2494)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2471 || n = 2493)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2477 || n = 2497)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2480 || n = 2490 || n = 2500)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
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
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2499)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.if(n = 2501)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2502 || n = 2512 || n = 2522)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 7) && ((n > 2502 && n < 2540)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2504 || n = 2526)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2505 || n = 2511 || n = 2524 || n = 2533 || n = 2534)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2506 || n = 2518 || n = 2530)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2507 || n = 2529)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2510)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2513)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2514)
	; 11 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2516)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2517 || n = 2537)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2521)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2523)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2525)
	; 11 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2528)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2535)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $E6A0'
	ldy #230
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2536)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2538)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	pha
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540 || n = 2550)
	; 11 bytes
	pha
	sec
	lda #n/10-1
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2541 || n = 2555)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2542 || n = 2554)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2543)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
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
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2546)
	; 11 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 2547)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2548)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2549)
	; 11 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	pha
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2553)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2557)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2558)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.else
; 2559 to 2578
.if(n = 2559)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2560)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	pla
.elseif(n = 2561)
	; 11 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2563 || n = 2575)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2564)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2565 || n = 2571 || n = 2577)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2566 || n = 2578)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2567)
	; 11 bytes
	sty @zptemp
	ldy #0
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2570)
	; 11 bytes
	pha
	sec
	lda #0
	inc @zptemp
	sbc #1
	Jne *-4
	pla
.elseif(n = 2572)
	; 11 bytes
	pha
	tya
	ldy #0
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2573)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2574)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2576)
	; 12 bytes
	pha
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	pha
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
.if((n >= 2579) && (n < 2598))
; 2579 to 2597
.if(n = 2579)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2580 || n = 2592)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2581)
	; 11 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2582)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 4
	sbc #1
	Jne *-5
	pla
.elseif(n = 2583)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	inc @zptemp
	dey
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
.elseif(n = 2587)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2589 || n = 2594)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2590)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2591)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2593)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2595)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2596)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2597)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 2598 to 2617
.if(n = 2598)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2599 || n = 2611)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2600)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2601)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2602 || n = 2614)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2603)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2604 || n = 2616)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2605)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	pha
	pha
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2606 || n = 2617)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2607)
	; 12 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2608)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2609)
	; 12 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	inc @zptemp
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2610)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	inc $F6,X
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2612)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2613)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	tay
	pla
.elseif(n = 2615)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2621 || n = 2632 || n = 2637 || n = 2642 || n = 2643 || n = 2654)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2619 || n = 2630 || n = 2641)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2620 || n = 2633)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2622 || n = 2631 || n = 2655)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2623 || n = 2635 || n = 2647)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2624)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2625 || n = 2636)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2626 || n = 2638 || n = 2650)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2627)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2634)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2639)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2644)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	pha
	pha
	Jne *-4
	ldx @zptemp
.elseif(n = 2645)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2646)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 4
	nop
	iny
	Jne *-5
	ldy @zptemp
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
	.repeat 4
	 nop
	.endrepeat
	Jne *-5
	tax
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
.elseif(n = 2653)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.if((n >= 2657) && (n < 2696))
; 2657 to 2695
.if(n = 2657)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif((n .mod 11 = 7) && ((n > 2657 && n < 2692)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2696)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2660 || n = 2665 || n = 2666 || n = 2673 || n = 2678 || n = 2682 || n = 2687 || n = 2690)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2661 || n = 2672 || n = 2694)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2662 || n = 2674 || n = 2686)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2663 || n = 2685)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
.elseif(n = 2667 || n = 2681)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2668)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	iny
	Jne *-5
	ldy @zptemp
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
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2677)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2679)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2689)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2692)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2693)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 2696 to 2734
.if(n = 2696 || n = 2718 || n = 2729)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2697 || n = 2704 || n = 2720 || n = 2721 || n = 2726 || n = 2733)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2698 || n = 2710 || n = 2722 || n = 2734)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2699 || n = 2725)
	; 12 bytes
	pha
	delay_n (n-3)
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
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2702 || n = 2713)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2703)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2705 || n = 2716 || n = 2727)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
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
.elseif(n = 2707 || n = 2719 || n = 2731)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2708)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 2709 || n = 2723)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2711)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	nop
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2715 || n = 2732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2730)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if(n = 2735 || n = 2757 || n = 2768)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2737 || n = 2765)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2738 || n = 2749 || n = 2771)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2741)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2743 || n = 2755 || n = 2767)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2744)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2745)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	ldy @zptemp
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
	delay_n 4
	nop
	dey
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
.elseif(n = 2753 || n = 2764 || n = 2769)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2754)
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
	delay_n (n-3)
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
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2775 || n = 2781 || n = 2786 || n = 2797)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2776 || n = 2802)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2777)
	; 11 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	pha
	dex
	dex
	Jne *-4
	ldx @zptemp
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
.elseif(n = 2779 || n = 2791 || n = 2803)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2780 || n = 2785 || n = 2809)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2787)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2788)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2789)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	pha
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2790 || n = 2801 || n = 2812)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2792)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	inc @zptemp
	adc #1
	Jne *-5
	pla
.elseif(n = 2793 || n = 2804)
	; 11 bytes
	pha
	sec
	lda #(n-10)/11
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2795)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2798)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2799)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2800)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2805)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2807)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2811)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	ldy @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2825 || n = 2829 || n = 2834 || n = 2841 || n = 2846)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 7) && ((n > 2814 && n < 2852)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2816)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	inc @zptemp
	sbc #1
	Jne *-5
	pla
.elseif(n = 2817)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2818 || n = 2830 || n = 2842)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
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
.elseif(n = 2821 || n = 2835 || n = 2849)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2823)
	; 11 bytes
	sty @zptemp
	ldy #0
	pha
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2824 || n = 2838)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2826)
	; 11 bytes
	pha
	sec
	lda #0
	inc $F6,X
	sbc #1
	Jne *-4
	pla
.elseif(n = 2828)
	; 11 bytes
	pha
	tya
	ldy #0
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2831)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2833)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2836 || n = 2848)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2837)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $ECA0'
	ldy #236
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2840)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2843)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2845)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2847)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	ldx @zptemp
.elseif(n = 2850)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
.elseif(n = 2853 || n = 2858 || n = 2865 || n = 2870 || n = 2889)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854 || n = 2866 || n = 2878 || n = 2890)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2855)
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
.elseif(n = 2857 || n = 2883)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2859 || n = 2872 || n = 2885)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2860 || n = 2879)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 2861)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2862 || n = 2881)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2863 || n = 2875 || n = 2887)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2864)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $48 ;hides 'pha'
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2867)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2869)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 7
	inx
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 2871)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2873)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	pha
	iny
	Jne *-5
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2877)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2886)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2888)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif
.else
.if((n >= 2891) && (n < 2930))
; 2891 to 2929
.if(n = 2891 || n = 2905 || n = 2919)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2893 || n = 2929)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2894 || n = 2901 || n = 2906 || n = 2913 || n = 2918 || n = 2925)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2897)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2898)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2899 || n = 2911 || n = 2923)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2900 || n = 2917)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2902 || n = 2914 || n = 2926)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2903)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 2907 || n = 2927)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2908 || n = 2922)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2909)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2915)
	; 12 bytes
	sty @zptemp
	ldy #224 ;hides 'cpx #imm'
	cmp $4848 ;hides two 'pha's
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2920)
	; 12 bytes
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $F6F6 ;hides 'inc $F6,X'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2921)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $F3A0'
	ldy #243
	pha
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2937 || n = 2942 || n = 2949 || n = 2954)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2931)
	; 12 bytes
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2933 || n = 2961)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2934 || n = 2968)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 2935 || n = 2947 || n = 2959)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2936)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2938 || n = 2950 || n = 2962)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2939 || n = 2944 || n = 2960)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 2941)
	; 12 bytes
	sty @zptemp
	ldy #196 ;hides 'cpy zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2943)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2945 || n = 2958)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2946)
	; 12 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2948)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
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
.elseif(n = 2953)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2955)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 2956)
	; 12 bytes
	sty @zptemp
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2957)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2963)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2965)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2966)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2967)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
.if((n >= 2969) && (n < 3008))
; 2969 to 3007
.if(n = 2969)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $EA ;hides 'nop'
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2970 || n = 3004)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2972)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2973 || n = 2978 || n = 2985 || n = 2990 || n = 2997 || n = 3002)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2974 || n = 2986 || n = 2998)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 2975 || n = 2989 || n = 3003)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2977 || n = 2979 || n = 2991 || n = 3001)
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
.elseif(n = 2982 || n = 2999)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2984)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2987)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 2992 || n = 3006)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 2993)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 2996)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3005)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	pha
	pha
	iny
	Jne *-5
	ldy @zptemp
.endif
.else
; 3008 to 3046
.if(n = 3008)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3009 || n = 3014 || n = 3033 || n = 3038)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010 || n = 3022 || n = 3034 || n = 3046)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 3011 || n = 3035 || n = 3040 || n = 3044)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 3015 || n = 3028 || n = 3041)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3016)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3017 || n = 3045)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3018)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3019 || n = 3031 || n = 3043)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3020)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3021)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	pha
	pha
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 3023)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3025 || n = 3037)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3026)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	inc $F6,X
	iny
	Jne *-4
	tay
	pla
.elseif(n = 3027)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3029)
	; 12 bytes
	sty @zptemp
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3030)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3039)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3042)
	; 12 bytes
	sty @zptemp
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	pha
	iny
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3049 || n = 3051 || n = 3061 || n = 3063 || n = 3075)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3050)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3052)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3053)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3054 || n = 3080)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3055 || n = 3067)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3056 || n = 3068 || n = 3083)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3057 || n = 3062 || n = 3069 || n = 3074 || n = 3081)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3058 || n = 3070)
	; 11 bytes
	pha
	sec
	lda #(n-10)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
.elseif(n = 3059 || n = 3073)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3064 || n = 3078)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3065)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3066)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3071)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3076)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3077)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3079)
	; 11 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3085)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $F6F6 ;hides 'inc $F6,X'
	adc #1
	Jne *-6
	pla
.endif
.else
; 3086 to 3125
.if(n = 3086)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087 || n = 3101 || n = 3115)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3088 || n = 3107)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3089)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	inc $F6,X
	adc #1
	Jne *-6
	pla
.elseif(n = 3090 || n = 3104 || n = 3118)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3091)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	inc $F6,X
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif((n > 3093 && n < 3097) || n = 3098 || n = 3099 || n = 3102 || n = 3103 || n = 3108 || n = 3111 || n = 3113 || n = 3116 || n = 3122 || n = 3124 || n = 3125)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3097)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3100)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	pha
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3105)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3109)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3110)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3114)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3117)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3121)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3123)
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
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
.if((n >= 3126) && (n < 3204))
; 3126 to 3203
.if(n = 3126 || n = 3164 || n = 3183)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3127 || n = 3167 || n = 3187)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3128 || n = 3147 || n = 3166)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 14 = 7) && ((n > 3128 && n < 3200)))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3130 || n = 3156 || n = 3169 || n = 3182 || n = 3195)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3131 || n = 3136 || n = 3137 || n = 3139 || n = 3141 || n = 3144 || n = 3149 || n = 3151 || n = 3154 || n = 3159 || n = 3163 || n = 3165 || n = 3168 || n = 3173 || n = 3175 || n = 3177 || n = 3178 || n = 3181 || n = 3193)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif((n .mod 14 = 10) && ((n > 3131 && n < 3203)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3133)
	; 12 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 10
	dey
	Jne *-5
	ldy @zptemp
.elseif((n .mod 14 = 12) && ((n > 3133 && n < 3191)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3135 || n = 3152 || n = 3186)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3138)
	; 12 bytes
	sty @zptemp
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3140 || n = 3191)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3142 || n = 3161 || n = 3180)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3145 || n = 3158 || n = 3184 || n = 3197)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3150 || n = 3170)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3153 || n = 3179)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3155 || n = 3172)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3189)
	; 12 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3192)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 3196)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3198)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	cmp $48 ;hides 'pha'
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3200)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	nop
	pha
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3201)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $F6F6 ;hides 'inc $F6,X'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3203)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
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
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3206 || n = 3240)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3207)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3208 || n = 3221 || n = 3234)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3209 || n = 3211 || n = 3214 || n = 3219 || n = 3222 || n = 3224 || n = 3226 || n = 3228 || n = 3229 || n = 3233 || n = 3235 || n = 3238)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 3212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3213 || n = 3227 || n = 3241)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3215)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	pha
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3216 || n = 3230)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3217)
	; 12 bytes
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3220)
	; 12 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3223 || n = 3236)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3225 || n = 3242)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3231)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	pha
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3237)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3239)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	nop
	pha
	dex
	dex
	Jne *-5
	ldx @zptemp
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3245 || n = 3248 || n = 3251 || (n > 3262 && n < 3267) || n = 3268 || n = 3279)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3244 || n = 3272)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3247 || n = 3273)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3249 || n = 3262 || n = 3275)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3250 || n = 3270)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3252)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3253)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 3254 || n = 3271)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3255 || n = 3269)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3256)
	; 11 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 7
	adc #1
	Jne *-5
	pla
.elseif(n = 3257)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
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
.elseif(n = 3261 || n = 3280)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3267)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3277)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3278)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3281)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if(n = 3282 || n = 3285 || n = 3295 || n = 3298 || n = 3303 || n = 3315 || n = 3317 || n = 3319 || n = 3320)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3283 || n = 3297)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3284)
	; 11 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 7
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 3287 || n = 3307)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3289)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3290)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3291)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3292)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 3294 || n = 3313)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3299 || n = 3312)
	; 12 bytes
	pha
	sec
	lda #(n-10)/13
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3300 || n = 3314)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3301)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3304)
	; 12 bytes
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	adc #1
	Jne *-6
	pla
.elseif(n = 3305)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3306)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3308)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
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
.elseif(n = 3318)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 3321 to 3359
.if(n = 3321)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3322)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3323 || n = 3324 || n = 3329 || n = 3331 || n = 3333 || n = 3334 || n = 3336 || n = 3341 || n = 3343 || n = 3345 || n = 3346 || n = 3348 || n = 3349 || n = 3355 || n = 3357)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3325 || n = 3339 || n = 3353)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3327)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/13
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3328 || n = 3342 || n = 3356)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
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
.elseif(n = 3332 || n = 3352)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3335 || n = 3354)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3337)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3338)
	; 12 bytes
	pha
	sec
	lda #0
	nop
	inc $F6,X
	sbc #1
	Jne *-5
	pla
.elseif(n = 3340)
	; 12 bytes
	pha
	tya
	ldy #0
	nop
	inc $F6,X
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3347)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3350)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3351)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3359)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.endif
.endif
.else
; 3360 to 3437
.if(n = 3360 || (n > 3361 && n < 3367) || n = 3368 || n = 3369 || n = 3371 || n = 3374 || n = 3377 || n = 3379 || n = 3380 || n = 3383 || n = 3385 || n = 3388 || n = 3391 || n = 3396 || n = 3399 || n = 3401 || n = 3403 || n = 3404 || n = 3406 || n = 3415 || n = 3417 || n = 3419 || n = 3420 || n = 3422 || n = 3425 || n = 3433 || n = 3435 || n = 3436)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3361 || n = 3378)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif((n .mod 14 = 7) && ((n > 3366 && n < 3438)))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 14 = 10) && ((n > 3369 && n < 3427)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3371 && n < 3429)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3373 || n = 3390 || n = 3407 || n = 3424)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3375 || n = 3394)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3376 || n = 3393 || n = 3410 || n = 3427)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3382)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3387)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3389)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3392 || n = 3432)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3397)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3405)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 10
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3408)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3411)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3413)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3421)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3429)
	; 12 bytes
	sty @zptemp
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3430)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
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
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
.if((n >= 3438) && (n < 3477))
; 3438 to 3476
.if(n = 3438 || n = 3439 || n = 3443 || n = 3445 || n = 3448 || n = 3453 || n = 3455 || n = 3460 || n = 3462 || n = 3466 || n = 3469 || n = 3471 || n = 3473 || n = 3474 || n = 3476)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3440 || n = 3454 || n = 3468)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3441)
	; 12 bytes
	sty @zptemp
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3442 || n = 3470)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3444)
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
.elseif(n = 3447 || n = 3467)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3449)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3450)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3451 || n = 3465)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3452 || n = 3472)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3456)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	ldx @zptemp
.elseif(n = 3457)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3458)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3459 || n = 3464)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3461)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
.elseif(n = 3463)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3475)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	inc $F6,X
	dex
	dex
	Jne *-5
	tax
	pla
.endif
.else
; 3477 to 3515
.if(n = 3477 || n = 3481 || n = 3483 || n = 3486 || n = 3491 || n = 3494 || n = 3499 || n = 3500 || n = 3505 || n = 3511 || n = 3513)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3478 || n = 3495)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3479 || n = 3493 || n = 3507)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3482 || n = 3496 || n = 3510)
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
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
.elseif(n = 3485)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	pha
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3487)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3488)
	; 12 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	nop
	inc $F6,X
	adc #1
	Jne *-6
	pla
.elseif(n = 3489 || n = 3508)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3490)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3492 || n = 3509)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3501)
	; 12 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3502)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3503)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3515)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	pha
	pha
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3519 || n = 3523 || n = 3528 || n = 3533 || n = 3534 || n = 3536 || n = 3537 || n = 3539 || n = 3542 || n = 3545 || n = 3551 || n = 3553 || (n > 3554 && n < 3560) || n = 3561 || n = 3564 || n = 3569 || n = 3571 || (n > 3572 && n < 3577) || n = 3578 || n = 3581 || n = 3583 || n = 3585 || n = 3586 || n = 3588 || n = 3589 || n = 3593)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3517 || n = 3562)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3518)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	inc $F6,X
	sbc #1
	Jne *-6
	pla
.elseif(n = 3520)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
.elseif((n .mod 14 = 7) && ((n > 3520 && n < 3578)))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3522 || n = 3541 || n = 3579)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 14 = 10) && ((n > 3523 && n < 3581)))
	; 12 bytes
	pha
	sec
	lda #(n-10)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3525 || n = 3544)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif((n .mod 14 = 12) && ((n > 3525 && n < 3583)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3527 || n = 3584)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3529 || n = 3546)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3530 || n = 3550 || n = 3570 || n = 3590)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3531 || n = 3548 || n = 3565)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3532 || n = 3572 || n = 3592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3543 || n = 3560)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3547 || n = 3567 || n = 3587)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3591)
	; 12 bytes
	sty @zptemp
	ldy #0
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
; 3594 to 3750
.if(n = 3594)
	; 12 bytes
	pha
	sec
	lda #0
	delay_n 9
	sbc #1
	Jne *-5
	pla
.elseif(n = 3595 || n = 3600 || n = 3602 || (n > 3603 && n < 3607) || n = 3608 || n = 3609 || n = 3613 || n = 3615 || n = 3618 || n = 3619 || n = 3621 || (n > 3622 && n < 3627) || n = 3629 || n = 3634 || n = 3635 || n = 3638 || n = 3640 || (n > 3641 && n < 3645) || n = 3646 || n = 3649 || n = 3651 || n = 3653 || n = 3654 || n = 3656 || n = 3657 || n = 3661 || n = 3663 || n = 3666 || n = 3668 || n = 3669 || n = 3671 || n = 3673 || n = 3675 || n = 3676 || n = 3678 || n = 3680 || n = 3681 || n = 3683 || n = 3685 || n = 3686 || n = 3688 || n = 3689 || n = 3691 || n = 3694 || n = 3695 || n = 3700 || (n > 3701 && n < 3706) || n = 3708 || n = 3709 || n = 3711 || n = 3714 || n = 3719 || n = 3721 || (n > 3722 && n < 3727) || n = 3728 || n = 3729 || (n > 3736 && n < 3742) || (n > 3742 && n < 3747) || n = 3748)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
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
.elseif(n = 3597 || n = 3614 || n = 3631 || n = 3648 || n = 3665 || n = 3682 || n = 3699 || n = 3716 || n = 3733 || n = 3750)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3598 || n = 3617 || n = 3636 || n = 3655 || n = 3674 || n = 3693 || n = 3731)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3599 || n = 3616 || n = 3633 || n = 3650 || n = 3667 || n = 3684 || n = 3718 || n = 3735)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3601 || n = 3620 || n = 3639 || n = 3658 || n = 3677 || n = 3715 || n = 3734)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3603 || n = 3622 || n = 3641 || n = 3660 || n = 3698 || n = 3736)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3607 || n = 3627 || n = 3647 || n = 3687 || n = 3707 || n = 3727)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3610 || n = 3630 || n = 3670 || n = 3690 || n = 3710)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3611 || n = 3628 || n = 3645 || n = 3662 || n = 3679 || n = 3696 || n = 3713 || n = 3730 || n = 3747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3612 || n = 3632 || n = 3652 || n = 3672 || n = 3692 || n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3637 || n = 3697 || n = 3742)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3659)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	pha
	delay_n 10
	Jne *-5
	ldx @zptemp
.elseif(n = 3664)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	tax
	pla
.elseif(n = 3701)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	ldx @zptemp
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
.elseif(n = 3717)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3749)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
; 3751 to 4062
.if(n = 3751 || n = 3756 || (n > 3757 && n < 3764) || n = 3765 || n = 3766 || n = 3768 || n = 3771 || n = 3773 || (n > 3774 && n < 3781) || n = 3782 || n = 3783 || n = 3785 || n = 3789 || (n > 3793 && n < 3798) || n = 3799 || n = 3800 || n = 3804 || n = 3806 || n = 3808 || n = 3809 || n = 3811 || n = 3813 || n = 3814 || n = 3816 || n = 3819 || (n > 3820 && n < 3826) || n = 3828 || n = 3833 || n = 3834 || n = 3836 || (n > 3837 && n < 3845) || n = 3846 || n = 3851 || n = 3853 || (n > 3854 && n < 3862) || n = 3863 || n = 3868 || n = 3873 || (n > 3874 && n < 3883) || n = 3884 || n = 3885 || n = 3889 || n = 3891 || (n > 3892 && n < 3900) || n = 3901 || n = 3904 || n = 3906 || n = 3908 || n = 3909 || n = 3911 || (n > 3912 && n < 3917) || n = 3918 || n = 3919 || n = 3923 || n = 3925 || n = 3928 || n = 3929 || n = 3931 || n = 3933 || n = 3935 || n = 3936 || n = 3938 || n = 3941 || n = 3942 || n = 3944 || n = 3946 || n = 3948 || n = 3953 || n = 3955 || n = 3957 || n = 3958 || n = 3960 || n = 3961 || n = 3963 || n = 3965 || n = 3966 || n = 3969 || (n > 3973 && n < 3978) || n = 3979 || n = 3980 || n = 3982 || n = 3984 || n = 3986 || n = 3989 || n = 3991 || (n > 3992 && n < 3997) || n = 3998 || n = 3999 || n = 4001 || n = 4003 || n = 4004 || n = 4006 || n = 4008 || n = 4009 || n = 4011 || (n > 4012 && n < 4016) || n = 4017 || n = 4018 || n = 4020 || n = 4023 || n = 4025 || n = 4026 || n = 4028 || n = 4029 || n = 4031 || n = 4033 || n = 4034 || n = 4037 || (n > 4041 && n < 4047) || n = 4048 || n = 4049 || n = 4051 || n = 4055 || (n > 4059 && n < 4063))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 3752)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
.elseif(n = 3753 || n = 3791 || n = 3829 || n = 3848 || n = 3924 || n = 3943 || n = 3962 || n = 3981 || n = 4000 || n = 4038 || n = 4057)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
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
.elseif(n = 3755 || n = 3774 || n = 3793 || n = 3831 || n = 3850 || n = 3907 || n = 3926 || n = 3945 || n = 3964 || n = 3983 || n = 4021 || n = 4040 || n = 4059)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3757 || n = 3802 || n = 3817)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3764 || n = 3781 || n = 3798 || n = 3815 || n = 3832 || n = 3849 || n = 3866 || n = 3883 || n = 3900 || n = 3917 || n = 3934 || n = 3951 || n = 3968 || n = 3985 || n = 4002 || n = 4019 || n = 4036 || n = 4053)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3767 || n = 3784 || n = 3801 || n = 3818 || n = 3835 || n = 3852 || n = 3869 || n = 3886 || n = 3903 || n = 3920 || n = 3937 || n = 3954 || n = 3971 || n = 3988 || n = 4005 || n = 4022 || n = 4039 || n = 4056)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 3769 || n = 3786 || n = 3803 || n = 3820 || n = 3837 || n = 3854 || n = 3871 || n = 3888 || n = 3905 || n = 3922 || n = 3939 || n = 3956 || n = 3973 || n = 3990 || n = 4007 || n = 4024 || n = 4041 || n = 4058)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3770 || n = 3790 || n = 3830 || n = 3870 || n = 3890 || n = 3910 || n = 3930 || n = 3950 || n = 3970 || n = 4010 || n = 4030 || n = 4050)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 3772 || n = 3792 || n = 3812 || n = 3872 || n = 3892 || n = 3912 || n = 3932 || n = 3952 || n = 3972 || n = 3992 || n = 4012 || n = 4032 || n = 4052)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3787 || n = 3827 || n = 3847 || n = 3887 || n = 3927 || n = 3947 || n = 3967 || n = 3987 || n = 4027 || n = 4047)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3788 || n = 3807 || n = 3826 || n = 3845 || n = 3864 || n = 3902 || n = 3921 || n = 3940 || n = 3959 || n = 3978 || n = 3997 || n = 4016 || n = 4035 || n = 4054)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3805)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3862)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 3865)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3867)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
	pla
.elseif(n = 3874)
	; 12 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3949)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-5
	ldy @zptemp
.endif
.else
; 4063 to 4375
.if(n = 4063 || n = 4064 || n = 4066 || n = 4069 || n = 4071 || n = 4074 || n = 4077 || (n > 4078 && n < 4087) || n = 4088 || n = 4089 || n = 4091 || n = 4093 || n = 4094 || n = 4096 || (n > 4097 && n < 4104) || n = 4105 || n = 4106 || n = 4108 || n = 4113 || n = 4115 || (n > 4116 && n < 4121) || n = 4122 || n = 4123 || n = 4125 || n = 4128 || n = 4129 || n = 4131 || n = 4134 || n = 4136 || n = 4137 || n = 4139 || n = 4140 || n = 4142 || (n > 4143 && n < 4147) || n = 4148 || n = 4151 || n = 4153 || n = 4156 || n = 4157 || n = 4159 || (n > 4160 && n < 4167) || n = 4169 || n = 4174 || n = 4176 || (n > 4177 && n < 4187) || n = 4188 || n = 4191 || n = 4193 || (n > 4194 && n < 4205) || n = 4208 || (n > 4212 && n < 4223) || n = 4224 || n = 4229 || n = 4231 || (n > 4232 && n < 4240) || n = 4241 || n = 4242 || n = 4246 || n = 4248 || n = 4251 || (n > 4252 && n < 4257) || n = 4258 || n = 4259 || n = 4261 || n = 4264 || n = 4265 || n = 4269 || n = 4271 || n = 4275 || n = 4276 || n = 4278 || n = 4280 || n = 4281 || n = 4283 || n = 4284 || n = 4286 || n = 4288 || n = 4289 || n = 4293 || n = 4295 || (n > 4296 && n < 4301) || n = 4302 || n = 4303 || n = 4305 || n = 4309 || (n > 4313 && n < 4320) || n = 4321 || n = 4322 || n = 4324 || n = 4326 || n = 4329 || n = 4331 || (n > 4332 && n < 4339) || n = 4340 || n = 4341 || n = 4343 || n = 4346 || n = 4348 || n = 4349 || n = 4351 || (n > 4352 && n < 4358) || n = 4360 || n = 4365 || n = 4366 || n = 4368 || n = 4369 || n = 4371 || (n > 4372 && n < 4376))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4065)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 4067 || n = 4127 || n = 4147 || n = 4167 || n = 4207 || n = 4227 || n = 4267 || n = 4307 || n = 4327 || n = 4367)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4068)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4070)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	tax
	pla
.elseif(n = 4072 || n = 4112 || n = 4132 || n = 4152 || n = 4212 || n = 4232 || n = 4252 || n = 4272 || n = 4292 || n = 4312 || n = 4332 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4073 || n = 4090 || n = 4107 || n = 4124 || n = 4141 || n = 4158 || n = 4175 || n = 4192 || n = 4209 || n = 4226 || n = 4243 || n = 4260 || n = 4277 || n = 4294 || n = 4311 || n = 4328 || n = 4345)
	; 12 bytes
	pha
	sec
	lda #(n-10)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
.elseif(n = 4075 || n = 4092 || n = 4109 || n = 4126 || n = 4143 || n = 4160 || n = 4177 || n = 4194 || n = 4211 || n = 4228 || n = 4245 || n = 4262 || n = 4279 || n = 4296 || n = 4313 || n = 4330 || n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4076 || n = 4095 || n = 4114 || n = 4133 || n = 4171 || n = 4247 || n = 4266 || n = 4285 || n = 4304 || n = 4323 || n = 4361)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4078 || n = 4097 || n = 4116 || n = 4135 || n = 4154 || n = 4173 || n = 4230 || n = 4249 || n = 4268 || n = 4287 || n = 4306 || n = 4344 || n = 4363)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4087 || n = 4104 || n = 4121 || n = 4138 || n = 4155 || n = 4172 || n = 4189 || n = 4206 || n = 4223 || n = 4240 || n = 4257 || n = 4274 || n = 4291 || n = 4308 || n = 4325 || n = 4342)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4110 || n = 4130 || n = 4150 || n = 4170 || n = 4190 || n = 4250 || n = 4270 || n = 4290 || n = 4310 || n = 4350 || n = 4370)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4111 || n = 4149 || n = 4168 || n = 4187 || n = 4225 || n = 4244 || n = 4263 || n = 4282 || n = 4301 || n = 4320 || n = 4339 || n = 4358)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4205)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4210)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4273)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_14cyc
	Jne *-5
	tax
	pla
.elseif(n = 4359)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.endif
.else
.if((n >= 4376) && (n < 4688))
; 4376 to 4687
.if(n = 4376 || n = 4378 || n = 4379 || n = 4381 || (n > 4382 && n < 4387) || n = 4388 || n = 4389 || n = 4391 || (n > 4392 && n < 4396) || n = 4397 || n = 4398 || n = 4400 || (n > 4401 && n < 4405) || n = 4406 || n = 4408 || n = 4409 || n = 4411 || n = 4414 || n = 4417 || n = 4419 || (n > 4420 && n < 4427) || n = 4428 || n = 4429 || n = 4431 || n = 4433 || n = 4435 || n = 4436 || n = 4438 || (n > 4439 && n < 4447) || n = 4448 || n = 4449 || n = 4455 || n = 4457 || (n > 4458 && n < 4467) || n = 4468 || n = 4469 || n = 4473 || n = 4474 || (n > 4477 && n < 4487) || n = 4488 || n = 4489 || n = 4493 || n = 4495 || (n > 4496 && n < 4507) || n = 4508 || n = 4509 || n = 4511 || n = 4514 || (n > 4515 && n < 4527) || n = 4528 || n = 4531 || n = 4533 || (n > 4534 && n < 4547) || n = 4549 || (n > 4553 && n < 4567) || n = 4568 || n = 4569 || n = 4571 || (n > 4572 && n < 4586) || n = 4588 || (n > 4592 && n < 4605) || n = 4606 || n = 4609 || n = 4611 || (n > 4612 && n < 4624) || n = 4625 || n = 4626 || n = 4628 || n = 4631 || (n > 4632 && n < 4643) || n = 4644 || n = 4645 || n = 4649 || n = 4651 || (n > 4652 && n < 4662) || n = 4663 || n = 4664 || n = 4666 || n = 4668 || n = 4669 || n = 4671 || n = 4673 || (n > 4674 && n < 4681) || n = 4682 || n = 4683 || n = 4685)
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4377 || n = 4396 || n = 4415 || n = 4434 || n = 4453 || n = 4491 || n = 4529 || n = 4548 || n = 4567 || n = 4586 || n = 4605 || n = 4624 || n = 4643 || n = 4662 || n = 4681)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4380 || n = 4399 || n = 4437 || n = 4475 || n = 4494 || n = 4513 || n = 4551 || n = 4570 || n = 4589 || n = 4608 || n = 4627 || n = 4646 || n = 4665 || n = 4684)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4382 || n = 4401 || n = 4420 || n = 4439 || n = 4458 || n = 4477 || n = 4496 || n = 4515 || n = 4534 || n = 4553 || n = 4572 || n = 4591 || n = 4610 || n = 4629 || n = 4648 || n = 4667 || n = 4686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4387 || n = 4407 || n = 4427 || n = 4447 || n = 4467 || n = 4487 || n = 4507 || n = 4527 || n = 4547 || n = 4587 || n = 4607 || n = 4647 || n = 4687)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4390 || n = 4430 || n = 4450 || n = 4470 || n = 4490 || n = 4510 || n = 4530 || n = 4550 || n = 4590 || n = 4630 || n = 4650 || n = 4670)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4392 || n = 4412 || n = 4432 || n = 4452 || n = 4472 || n = 4492 || n = 4512 || n = 4532 || n = 4552 || n = 4592 || n = 4612 || n = 4632 || n = 4652 || n = 4672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4405)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4413)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4451)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4454)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
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
.elseif(n = 4471)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	ldx @zptemp
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
.elseif(n = 4674)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-5
	ldx @zptemp
.endif
.else
; 4688 to 5000
.if(n = 4688 || n = 4689 || n = 4691 || (n > 4692 && n < 4700) || n = 4701 || n = 4702 || n = 4704 || n = 4706 || n = 4708 || n = 4709 || n = 4711 || (n > 4712 && n < 4719) || n = 4720 || n = 4721 || n = 4723 || n = 4725 || n = 4726 || n = 4728 || n = 4729 || n = 4731 || (n > 4732 && n < 4738) || n = 4739 || n = 4740 || n = 4742 || (n > 4743 && n < 4747) || n = 4748 || n = 4749 || n = 4751 || (n > 4752 && n < 4757) || n = 4758 || n = 4759 || n = 4761 || (n > 4762 && n < 4767) || n = 4768 || n = 4769 || n = 4771 || (n > 4772 && n < 4776) || n = 4777 || n = 4778 || n = 4780 || (n > 4781 && n < 4787) || n = 4788 || n = 4789 || n = 4791 || n = 4793 || n = 4794 || n = 4796 || n = 4797 || n = 4799 || (n > 4800 && n < 4807) || n = 4808 || n = 4809 || n = 4811 || n = 4813 || n = 4815 || n = 4816 || n = 4818 || (n > 4819 && n < 4827) || n = 4828 || n = 4829 || n = 4831 || n = 4834 || n = 4835 || n = 4837 || (n > 4838 && n < 4847) || n = 4848 || n = 4849 || n = 4851 || n = 4853 || n = 4854 || n = 4856 || (n > 4857 && n < 4867) || n = 4868 || n = 4869 || n = 4873 || n = 4875 || n = 4878 || n = 4879 || n = 4881 || (n > 4882 && n < 4887) || n = 4888 || n = 4889 || n = 4891 || (n > 4892 && n < 4907) || n = 4908 || n = 4909 || n = 4911 || (n > 4912 && n < 4919) || n = 4920 || n = 4921 || n = 4923 || n = 4925 || n = 4926 || n = 4928 || n = 4929 || n = 4931 || (n > 4932 && n < 4947) || n = 4948 || n = 4949 || n = 4951 || (n > 4952 && n < 4967) || n = 4968 || n = 4969 || n = 4971 || (n > 4972 && n < 4987) || n = 4988 || n = 4989 || n = 4991 || (n > 4992 && n < 5001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 4690 || n = 4710 || n = 4730 || n = 4750 || n = 4770 || n = 4790 || n = 4810 || n = 4830 || n = 4850 || n = 4870 || n = 4890 || n = 4910 || n = 4930 || n = 4950 || n = 4970 || n = 4990)
	; 12 bytes
	pha
	sec
	lda #(n-10)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4692 || n = 4712 || n = 4732 || n = 4752 || n = 4772 || n = 4792 || n = 4812 || n = 4832 || n = 4852 || n = 4872 || n = 4892 || n = 4912 || n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4700 || n = 4719 || n = 4738 || n = 4757 || n = 4776 || n = 4795 || n = 4814 || n = 4833)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4703 || n = 4722 || n = 4741 || n = 4760 || n = 4779 || n = 4798 || n = 4817 || n = 4836 || n = 4855)
	; 12 bytes
	pha
	sec
	lda #(n-10)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4705 || n = 4724 || n = 4743 || n = 4762 || n = 4781 || n = 4800 || n = 4819 || n = 4838 || n = 4857)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4707 || n = 4727 || n = 4747 || n = 4767 || n = 4787 || n = 4807 || n = 4827 || n = 4847 || n = 4867 || n = 4887 || n = 4907 || n = 4927 || n = 4947 || n = 4967 || n = 4987)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4871)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4874)
	; 12 bytes
	pha
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
.elseif(n = 4876)
	; 12 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4877)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4880)
	; 12 bytes
	pha
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
	pla
.elseif(n = 4882)
	; 12 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4919)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4922)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
.elseif(n = 4924)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
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
; 5001 to 10000
.if((n > 5000 && n < 5007) || n = 5008 || n = 5009 || n = 5011 || (n > 5012 && n < 5027) || n = 5028 || n = 5029 || n = 5031 || (n > 5032 && n < 5047) || n = 5048 || n = 5049 || n = 5051 || (n > 5052 && n < 5067) || n = 5068 || n = 5069 || n = 5071 || (n > 5072 && n < 5087) || n = 5088 || n = 5089 || n = 5091 || (n > 5092 && n < 5107) || n = 5108 || n = 5111 || n = 5113 || (n > 5114 && n < 5127) || n = 5128 || n = 5129 || n = 5131 || (n > 5132 && n < 5153) || n = 5154 || n = 5155 || n = 5157 || (n > 5158 && n < 10001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.elseif(n = 5007 || n = 5027 || n = 5047 || n = 5067 || n = 5087 || n = 5107)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 5109)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 5127)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 5153)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5156)
	; 12 bytes
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
.elseif(n = 5158)
	; 12 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
	; 13 bytes
	pha
	txa
	pha
	ldx #>(n-52)
	lda #<(n-52)
	jsr delay_256x_a_30_clocks
	pla
	tax
	pla
.endif
.endif

.endif
	rest
.endmacro
