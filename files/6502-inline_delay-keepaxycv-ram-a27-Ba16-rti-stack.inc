; delay_n, delays n cycles with small code
; Will clobber S, Z+N, and D
; Will preserve A, X, Y, C, V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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

; Requires a disposable zeropage variable @zptemp.
;
; Requires a function called "delay_256a_16_clocks_b"
; that is defined as follows:
;;;;;;;;;;;;;;;;;;;;;;;;;
;; Delays A*256 clocks + overhead
;; Clobbers A. Preserves X,Y.
;; Time: A*256+16 clocks (including JSR)
;; Depends on delay_a_27_clocks
;;;;;;;;;;;;;;;;;;;;;;;;;
;delay_256a_16_clocks_b:
;	cmp #0
;	bne :+
;	rts
;delay_256a_11_clocks_b_:
;:       pha
;	 lda #(256-27-7-2-2-3)
;	 jsr delay_a_27_clocks
;	pla
;	sec
;	sbc #1
;	bne :-
;	rts
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
; - 1 bytes or less when n <= 3
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 10
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 23
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 36
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 315
; -11 bytes or less when n <= 1333
; -12 bytes or less when n <= 1855
; -13 bytes or less when n <= 3316
; -14 bytes or less when n <= 5397
; -15 bytes or less when n <= 6422
; -16 bytes or less when n <= 7162
; -17 bytes or less when n <= 9495
; -18 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=2, avg=3.4, max=4
; - When 21 <= n <= 30: min=4, avg=5.1, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=8.8, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=9.9, max=11
; - When 501 <= n <= 750: min=8, avg=9.9, max=11
; - When 751 <= n <= 1000: min=9, avg=10.0, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.5, max=13
; - When 2001 <= n <= 3000: min=9, avg=11.5, max=13
; - When 3001 <= n <= 5000: min=9, avg=12.3, max=14
; - When 5001 <= n <= 7500: min=9, avg=13.5, max=17
; - When 7501 <= n <= 10000: min=9, avg=14.0, max=18
; - When 10001 <= n <= 20000: min=9, avg=14.3, max=18
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
.elseif(n = 4 || n = 8 || n = 17 || n = 28)
	; 2..5 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 6)
	; 2 bytes
	delay_n (n-3)
	pha
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif((n > 8 && n < 12) || (n > 19 && n < 24) || (n > 32 && n < 38))
	; 3..8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 13)
	; 2 bytes
	brk 0
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 16 || n = 19 || n = 26 || n = 29 || n = 32 || n = 39)
	; 3..6 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18 || n = 31)
	; 4..6 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27)
	; 5 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.endif
.else
; 40 to 78
.if(n = 40)
	; 7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 41 || n = 43)
	; 7..8 bytes
	delay_n (n-2)
	nop
.elseif(n = 42 || n = 45)
	; 7..8 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 44)
	; 8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 46)
	; 8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 47 || n = 49 || n = 51 || n = 54 || (n > 55 && n < 60) || (n > 60 && n < 65) || (n > 65 && n < 69) || (n > 69 && n < 79))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 48)
	; 9 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	pla
	plp
.elseif(n = 50)
	; 9 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	plp
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk 0
	.endrepeat
.elseif(n = 53)
	; 8 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	plp
.elseif(n = 55)
	; 8 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	Jmi *-1
	ldy @zptemp
.elseif(n = 60)
	; 8 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	pla
.elseif(n = 65)
	; 9 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	pla
	plp
.elseif(n = 69)
	; 9 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	pla
	plp
.endif
.endif
.else
; 79 to 156
.if((n > 78 && n < 125) || (n > 125 && n < 130) || (n > 130 && n < 135) || (n > 135 && n < 157))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 125)
	; 9 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	pla
	plp
.elseif(n = 130)
	; 8 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	Jne *-1
	ldx @zptemp
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
.if((n > 156 && n < 170) || (n > 170 && n < 290) || (n > 290 && n < 295) || (n > 295 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
	pla
	plp
.elseif(n = 170)
	; 9 bytes
	php
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	pla
	plp
.elseif(n = 290)
	; 8 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	Jmi *-1
	ldy @zptemp
.elseif(n = 295)
	; 8 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
.elseif(n = 299 || n = 300 || n = 304 || n = 309)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 301)
	; 10 bytes
	pha
	delay_n (n-3)
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
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 305)
	; 10 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	ldx @zptemp
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
.elseif(n = 310)
	; 10 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	Jne *-3
	tax
	pla
.elseif(n = 311)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
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
.if(n = 313 || n = 320 || n = 355 || n = 369 || n = 376)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 314 || n = 316 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 338 || n = 339 || n = 344 || n = 346 || n = 349 || n = 354 || n = 356 || n = 359 || n = 361 || n = 364 || n = 366 || n = 373 || n = 374 || n = 379 || n = 381 || n = 384 || n = 386 || n = 389)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 350 || n = 371)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 323 || n = 328 || n = 333 || n = 368)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 325 || n = 330 || n = 335 || n = 340 || n = 345 || n = 360 || n = 375 || n = 380 || n = 388)
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
.elseif(n = 351)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 358)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 365)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 370)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	iny
	Jne *-3
	tay
	pla
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
.if(n = 391 || n = 431)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 392 || n = 407 || n = 417 || n = 422 || n = 432 || n = 462 || n = 467)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 393 || n = 400 || n = 405 || n = 430 || n = 435 || n = 440 || n = 445 || n = 450 || n = 465)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 394 || n = 399 || n = 401 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 434 || n = 436 || n = 443 || n = 444 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 464)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 395)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 396)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 5 = 2) && (n = 397 || n = 402 || n = 412 || n = 427 || (n > 436 && n < 458)))
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 398 || n = 403)
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 404)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 406 || n = 441 || n = 448)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 410)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
	ldy @zptemp
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
.elseif(n = 413 || n = 420 || n = 455)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 415)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	dey
	Jmi *-3
	tay
	pla
.elseif(n = 416)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 418 || n = 425 || n = 460)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 423 || n = 463)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 433 || n = 438 || n = 458)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
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
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 479 || n = 484 || n = 486 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 513 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 541 || n = 546)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 469 && n < 486) || (n > 499 && n < 521) || (n > 534 && n < 546)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 471 || n = 543)
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
.elseif(n = 474 || n = 481 || n = 523 || n = 530)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 476 || n = 518 || n = 525)
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
.elseif(n = 483 || n = 490 || n = 511)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 488 || n = 495 || n = 516)
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
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	dey
	Jmi *-3
	ldy @zptemp
.elseif(n = 493)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 503)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 508)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 509)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	Jpl *-2
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
.elseif(n = 533)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 538)
	; 11 bytes
	php
	sta @zptemp
	lda #2
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 539)
	; 9 bytes
	php
	pha
	lda #2
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 544)
	; 9 bytes
	php
	pha
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 548 || n = 549 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 571 || n = 574 || n = 584 || n = 589 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 619 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 550 || n = 555 || (n > 569 && n < 591) || (n > 604 && n < 626)))
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
.elseif(n = 553 || n = 588 || n = 616 || n = 623)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 560 || n = 581 || n = 595)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
.elseif((n .mod 5 = 3) && ((n > 562 && n < 579) || (n > 597 && n < 609)))
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.elseif(n = 583)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 591)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
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
.if((n >= 626) && (n < 704))
; 626 to 703
.if(n = 626 || n = 629 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 674 || n = 676 || n = 679 || n = 681 || n = 688 || n = 689 || n = 694 || n = 699)
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
.elseif(n = 628)
	; 9 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	ldy @zptemp
.elseif(n = 630 || n = 651 || n = 665 || n = 686 || n = 700)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 631 || n = 671)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 633)
	; 9 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jmi *-2
	tay
	pla
.elseif(n = 635 || n = 656 || n = 670 || n = 691)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 636 || n = 640 || n = 645 || n = 650 || n = 655 || n = 660 || n = 675 || n = 680 || n = 685 || n = 693 || n = 698)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 703)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 643)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 668)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 673)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 678)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 683)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.elseif(n = 696 || n = 701)
	; 10 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 703)
	; 10 bytes
	delay_n (n-13)
	brk 0
.endif
.else
; 704 to 781
.if(n = 704 || n = 706 || n = 709 || n = 714 || n = 716 || n = 724 || n = 729 || n = 731 || n = 734 || n = 739 || n = 744 || n = 746 || n = 749 || n = 758 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 705)
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
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 710 || n = 715 || n = 720 || n = 725 || n = 730 || n = 736 || n = 741 || n = 745 || n = 750 || n = 755 || n = 760 || n = 765 || n = 780)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 711 || n = 743)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 713 || n = 753 || n = 773 || n = 778)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 718)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 719 || n = 726 || n = 740 || n = 761 || n = 768 || n = 775)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 721 || n = 735 || n = 756 || n = 763 || n = 770)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 728)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 733)
	; 9 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	ldy @zptemp
.elseif(n = 738)
	; 9 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	iny
	Jpl *-2
	tay
	pla
.elseif(n = 748)
	; 11 bytes
	pha
	tya
	ldy #(n-20)/8
	sta @zptemp
	dey
	Jpl *-3
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
.elseif(n = 766)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 782) && (n < 860))
; 782 to 859
.if((n .mod 5 = 2) && ((n > 781 && n < 858)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 783)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 784 || n = 793 || n = 794 || n = 799 || n = 804 || n = 809 || n = 814 || n = 819 || n = 821 || n = 829 || n = 834 || n = 836 || n = 839 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 785 || n = 790 || n = 798 || n = 803 || n = 815 || n = 820 || n = 825 || n = 830 || n = 835 || n = 850 || n = 855)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 786 || n = 858)
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
	sta @zptemp
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 789 || n = 796 || n = 831 || n = 845 || n = 859)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 791 || n = 826 || n = 840)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 795)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 800)
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 801)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 805 || n = 833)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 806)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	dey
	Jmi *-3
	ldy @zptemp
.elseif(n = 808 || n = 813)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 810 || n = 824 || n = 838)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 811)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
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
.elseif(n = 818 || n = 848)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 823)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 841)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 6
	Jmi *-3
	ldx @zptemp
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
.elseif(n = 853)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
.endif
.else
; 860 to 937
.if((n .mod 5 = 0) && ((n > 859 && n < 871) || (n > 884 && n < 901) || (n > 919 && n < 936)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 861 || n = 868 || n = 875 || n = 896 || n = 903)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif((n .mod 5 = 2) && ((n > 861 && n < 938)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 863 || n = 871)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 864 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 933 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 866 || n = 873 || n = 880 || n = 901 || n = 908 || n = 915)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
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
.elseif(n = 878 || n = 893 || n = 913 || n = 918 || n = 923 || n = 928)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 883)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 888)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 894 || n = 929 || n = 936)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 898)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 905)
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	iny
	Jne *-3
	ldy @zptemp
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
.elseif(n = 911)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 931)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if(n = 938 || n = 945 || n = 966 || n = 980 || n = 1001 || n = 1008 || n = 1015)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 939 || n = 944 || n = 949 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 994 || n = 1003 || n = 1004 || n = 1009 || n = 1011 || n = 1014)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 940 || (n > 954 && n < 976) || (n > 989 && n < 1011)))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 941)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
	ldy @zptemp
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1013)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 943 || n = 950 || n = 971 || n = 978 || n = 985 || n = 999 || n = 1006 || n = 1013)
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
.elseif(n = 951 || n = 983 || n = 991)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 953 || n = 958 || n = 963 || n = 998)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
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
.elseif(n = 976)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 988)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.endif
.else
; 1016 to 1093
.if(n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1029 || n = 1039 || n = 1044 || n = 1046 || n = 1049 || n = 1053 || n = 1058 || n = 1064 || n = 1066 || n = 1073 || n = 1074 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 1016 && n < 1093)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
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
.elseif(n = 1020)
	; 8 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	ldx @zptemp
.elseif(n = 1023 || n = 1028 || n = 1035 || n = 1040 || n = 1045 || n = 1054 || n = 1059 || n = 1060 || n = 1065 || n = 1070 || n = 1075 || n = 1080)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 1030)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 1031)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 1033 || n = 1038)
	; 10 bytes
	delay_n (n-13)
	brk 0
.elseif((n .mod 7 = 5) && ((n > 1033 && n < 1056) || (n > 1068 && n < 1091)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1036 || n = 1043 || n = 1050 || n = 1071 || n = 1078 || n = 1085)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1051)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1056)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1061)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #4
	jsr delay_256a_16_clocks_b
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
.elseif(n = 1068)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
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
.elseif(n = 1088 || n = 1093)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1099 || n = 1108 || n = 1109 || n = 1114 || n = 1119 || n = 1124 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1149 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1178 || n = 1179 || n = 1184 || n = 1186 || n = 1189 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1116) || (n > 1129 && n < 1151) || (n > 1164 && n < 1186) || (n > 1199 && n < 1221) || (n > 1234 && n < 1251)))
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
.elseif(n = 1098 || n = 1133 || n = 1138 || n = 1158 || n = 1163 || n = 1193 || n = 1203 || n = 1208 || n = 1233)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 1101)
	; 9 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	pha
	Jne *-2
	ldy @zptemp
.elseif(n = 1103 || n = 1143 || n = 1151 || n = 1191 || n = 1231)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif((n .mod 7 = 0) && ((n > 1105 && n < 1121) || (n > 1140 && n < 1156) || (n > 1175 && n < 1191) || (n > 1210 && n < 1226) || n = 1246))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1116 || n = 1248)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1121)
	; 10 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1123 || n = 1168)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 1126)
	; 10 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	dey
	Jne *-3
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
.elseif(n = 1198)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1213)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1228)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
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
.elseif((n .mod 7 = 0) && (n = 1253 || n = 1260 || (n > 1280 && n < 1303) || n = 1316 || n = 1323))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1254 || n = 1256 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1274 || n = 1276 || n = 1283 || n = 1284 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1304 || n = 1309 || n = 1314)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 1255 || (n > 1269 && n < 1291) || n = 1310 || n = 1315))
	; 10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 7) && (n = 1263 || n = 1271 || (n > 1302 && n < 1328)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1268 || n = 1324)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1273 || n = 1278)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
	txa
	ldx #0
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 1299)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1301)
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	pha
	dey
	dey
	Jne *-4
	ldy @zptemp
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
	php
	sta @zptemp
	lda #5
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 1307)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1308 || n = 1326)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1313 || n = 1318)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1317)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1320 || n = 1325)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1322)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 1329 to 1406
.if(n = 1329 || n = 1334 || n = 1346 || n = 1350 || n = 1355 || n = 1360 || n = 1361 || n = 1381 || n = 1385 || n = 1388 || n = 1395 || n = 1406)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 0) && ((n > 1329 && n < 1401)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1331 || n = 1333 || n = 1338 || n = 1345 || n = 1354 || n = 1376 || n = 1390 || n = 1394 || n = 1401 || n = 1403)
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
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1399)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1336)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	ldy @zptemp
.elseif((n .mod 9 = 7) && ((n > 1338 && n < 1367) || n = 1402))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1340 || n = 1364 || n = 1396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1341)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jmi *-4
	tay
	pla
.elseif((n .mod 8 = 7) && (n = 1343 || (n > 1358 && n < 1384) || n = 1399))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1347 || n = 1397)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif((n .mod 9 = 3) && ((n > 1352 && n < 1390)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
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
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1373)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk 0
	Jmi *-3
	ldx @zptemp
.elseif(n = 1374)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1378)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jmi *-3
	tax
	pla
.elseif(n = 1382)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 1405)
	; 9 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1408 || n = 1413 || n = 1424 || n = 1429 || n = 1434)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1430 || n = 1437 || n = 1441 || n = 1444)
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
.elseif(n = 1411 || n = 1420)
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
.elseif(n = 1417 || n = 1438)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1418)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1422 || n = 1432)
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
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
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
.elseif(n = 1436)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1445)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	brk 0
	iny
	Jpl *-4
	ldy @zptemp
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
.elseif(n = 1448 || n = 1481)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 7 = 0) && ((n > 1448 && n < 1485)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1450)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	brk 0
	iny
	Jpl *-4
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
.elseif(n = 1452)
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
.elseif(n = 1455 || n = 1471 || n = 1479)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1457 || n = 1478 || n = 1480)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1458 || n = 1472 || n = 1473)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1459)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1460)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1462)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1465 || n = 1474 || n = 1483)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-24)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	ldy @zptemp
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
.endif
.endif
.else
; 1485 to 1562
.if(n = 1485 || n = 1490 || n = 1494 || n = 1499 || n = 1513 || n = 1520 || n = 1525 || n = 1534 || n = 1541 || n = 1550)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1486 || n = 1493 || n = 1504 || n = 1507 || n = 1509 || n = 1514 || n = 1521 || n = 1539 || n = 1544 || n = 1549 || n = 1553 || n = 1556)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 7) && (n = 1487 || n = 1495 || n = 1511 || (n > 1526 && n < 1552)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif((n .mod 9 = 3) && ((n > 1487 && n < 1516) || n = 1542 || n = 1560))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1511) || (n > 1523 && n < 1560)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 7 = 0) && ((n > 1490 && n < 1562)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif((n .mod 9 = 7) && (n = 1492 || n = 1501 || (n > 1527 && n < 1556)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1500 || n = 1536 || n = 1548)
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
.elseif(n = 1516 || n = 1532)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1517)
	; 9 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	ldy @zptemp
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
	; 9 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1523)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1529)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1530)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	tax
	pla
.elseif(n = 1557)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1558)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1562)
	; 11 bytes
	php
	sta @zptemp
	lda #6
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n .mod 9 = 7) && (n = 1564 || (n > 1599 && n < 1628)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1565 || n = 1570 || n = 1572 || n = 1577 || n = 1585 || n = 1593 || n = 1598 || n = 1612 || n = 1619 || n = 1626 || n = 1630 || n = 1633 || n = 1640)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1566 || n = 1571 || n = 1586 || n = 1590 || n = 1597 || n = 1606 || n = 1611 || n = 1625 || n = 1632 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 8 = 7) && (n = 1567 || (n > 1582 && n < 1608) || n = 1623 || n = 1639))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1569 || n = 1574)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 7 = 5) && ((n > 1572 && n < 1623) || n = 1636))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif((n .mod 7 = 0) && ((n > 1574 && n < 1639)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1576 || n = 1581 || n = 1595)
	; 11..12 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 1578 || n = 1605 || n = 1614)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1584 || n = 1620)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1588 || n = 1604 || n = 1628)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1592 || n = 1602)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1613 || n = 1635)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1629)
	; 9 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	ldy @zptemp
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
; 1641 to 1718
.if(n = 1641)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1642 || n = 1662 || n = 1702 || n = 1712)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1643)
	; 9 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1644 || n = 1656 || n = 1716)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 0) && ((n > 1644 && n < 1716)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1646 || n = 1651 || n = 1658 || n = 1667 || n = 1669 || n = 1674 || n = 1683 || n = 1698 || n = 1709 || n = 1714 || n = 1718)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1647 || n = 1655 || n = 1663 || n = 1679 || n = 1695 || n = 1703 || n = 1711)
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
.elseif((n .mod 7 = 5) && ((n > 1649 && n < 1714)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
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
.elseif(n = 1660 || n = 1676 || n = 1684 || n = 1700)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1661 || n = 1665 || n = 1670 || n = 1675 || n = 1682 || n = 1689 || n = 1691 || n = 1696 || n = 1705 || n = 1710)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1668 || n = 1677 || n = 1686 || n = 1704)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1672 || n = 1681 || n = 1690 || n = 1717)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1688)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	dex
	dex
	Jne *-4
	ldx @zptemp
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
.elseif(n = 1697 || n = 1707)
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
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if((n .mod 8 = 7) && (n = 1719 || n = 1735 || (n > 1750 && n < 1776) || n = 1791))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1735) || (n > 1747 && n < 1791)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1721 || n = 1724 || n = 1733 || n = 1738 || n = 1745 || n = 1761 || n = 1766 || n = 1768 || n = 1773 || n = 1784 || n = 1787)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 7 = 0) && ((n > 1721 && n < 1793)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1723 || n = 1747 || n = 1795)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1725 || n = 1730 || n = 1737 || n = 1739 || n = 1744 || n = 1749 || n = 1760 || n = 1765 || n = 1770 || n = 1774 || n = 1779 || n = 1781 || n = 1786 || n = 1793)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1726 || n = 1780 || n = 1789)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1728 || n = 1752 || n = 1788)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1731 || n = 1740 || n = 1758 || n = 1794)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1732 || n = 1756 || n = 1772)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1741)
	; 9 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1742 || n = 1782)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1746)
	; 9 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1753)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1754)
	; 11 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 1777)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1796)
	; 10 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	dey
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1798 || n = 1816 || n = 1834)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1805)
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1806 || n = 1809 || n = 1814 || n = 1821 || n = 1826 || n = 1828)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 7) && ((n > 1806 && n < 1832)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1808 || n = 1813 || n = 1822 || n = 1827 || n = 1835)
	; 10..11 bytes
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
.elseif(n = 1811 || n = 1833)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
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
.elseif(n = 1818)
	; 11 bytes
	php
	sta @zptemp
	lda #7
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 1819)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1820)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1825 || n = 1830)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 1829)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1832)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	ldx @zptemp
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
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif((n .mod 8 = 7) && ((n > 1838 && n < 1872)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1840 || n = 1850 || n = 1851 || n = 1858 || n = 1874)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1841 || n = 1849 || n = 1854 || n = 1856 || n = 1859 || n = 1864 || n = 1865 || n = 1873)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1842 || n = 1862)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 7) && ((n > 1842 && n < 1871)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1844 || n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1846)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
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
.elseif(n = 1867)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1869)
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
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
; 1876 to 2031
.if(n = 1876 || n = 1892 || n = 1900 || n = 1916 || n = 1940 || n = 1948 || n = 1964 || n = 1972 || n = 1988 || n = 2012 || n = 2020)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1877 || n = 1907 || n = 1917 || n = 1937 || n = 1977 || n = 1997 || n = 2017 || n = 2027)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1878 || n = 1881 || n = 1886 || n = 1889 || n = 1894 || n = 1905 || n = 1913 || n = 1918 || n = 1931 || n = 1941 || n = 1945 || n = 1950 || n = 1953 || n = 1955 || n = 1961 || n = 1966 || n = 1971 || n = 1984 || n = 1985 || n = 1989 || n = 1990 || n = 1993 || n = 2006 || n = 2013 || n = 2021 || n = 2024 || n = 2025 || n = 2030)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 7) && ((n > 1878 && n < 2032)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1880 || n = 1890 || n = 1898 || n = 1914 || n = 1930 || n = 1936 || n = 1946 || n = 1957 || n = 1970 || n = 1986 || n = 1994 || n = 2008 || n = 2018 || n = 2026)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 1882 || n = 1912 || n = 1922 || n = 1952 || n = 1962 || n = 1982 || n = 2002 || n = 2022)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1883 || n = 1995)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 9 = 3) && ((n > 1883 && n < 1903) || (n > 1919 && n < 1975) || (n > 1991 && n < 2029)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1885 || n = 1901 || n = 1973)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 9 = 7) && ((n > 1887 && n < 1943) || (n > 1959 && n < 1997) || n = 2014))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1891 || n = 1939 || n = 1963 || n = 2011)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1896 || n = 1908 || n = 1932 || n = 1944 || n = 1968 || n = 1980 || n = 2004 || n = 2016)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1899 || n = 1910 || n = 1921 || n = 1976 || n = 1998 || n = 2009)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1904 || n = 1926 || n = 1981 || n = 2003)
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
.elseif(n = 1909)
	; 12 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	iny
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 1923)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1928)
	; 11 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 9
	Jne *-4
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
.elseif(n = 1949)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1954)
	; 10 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	dey
	Jne *-3
	tay
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
.elseif(n = 2005)
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	pha
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2029)
	; 12 bytes
	pha
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
	pla
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
; 2032 to 2070
.if((n .mod 9 = 7) && ((n > 2031 && n < 2069)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2033 || n = 2043 || n = 2048 || n = 2049 || n = 2054 || n = 2057 || n = 2061 || n = 2066)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2034 || n = 2038 || n = 2058 || n = 2063)
	; 11..12 bytes
	pha
	delay_n (n-3)
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
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2037 || n = 2046 || n = 2064)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2039 || n = 2047)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 2042 || n = 2062)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2045)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2051)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2053)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2055)
	; 10 bytes
	sty @zptemp
	ldy #0
	pha
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 2060)
	; 11 bytes
	pha
	tya
	ldy #0
	sta @zptemp
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
.elseif(n = 2067)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2069)
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
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2107)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2072 || n = 2092 || n = 2102)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 2074)
	; 11 bytes
	php
	sta @zptemp
	lda #8
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 2075)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_11_clocks_b_
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
.elseif(n = 2077 || n = 2079 || n = 2082 || n = 2099)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2078 || n = 2083 || n = 2096 || n = 2101)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2081 || n = 2086)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
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
	php
	pha
	inc @zptemp
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2087)
	; 11 bytes
	php
	pha
	php
	lda #8
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 2088)
	; 11 bytes
	delay_n (n-13)
	brk 0
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
	ldy #24 ;hides 'clc'
	brk 0
	iny
	Jpl *-4
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
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2095 || n = 2104)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2108)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2115 || n = 2120 || n = 2129 || n = 2133 || n = 2134 || n = 2138 || n = 2144 || n = 2151 || n = 2156 || n = 2164 || n = 2169 || n = 2178 || n = 2183 || n = 2186)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2111)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2112 || n = 2124 || n = 2148 || n = 2160 || n = 2184)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 7) && ((n > 2112 && n < 2186)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2114 || n = 2125 || n = 2139 || n = 2161 || n = 2168 || n = 2175 || n = 2180)
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
.elseif(n = 2117 || n = 2137 || n = 2147 || n = 2157 || n = 2177 || n = 2187)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 9 = 3) && ((n > 2117 && n < 2182)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2119 || n = 2143 || n = 2155 || n = 2179)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2123)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2126)
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
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2130 || n = 2141 || n = 2174)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 10 = 2) && ((n > 2131 && n < 2163) || n = 2182))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2135 || n = 2146)
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
.elseif(n = 2150)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	brk 0
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 2153)
	; 11 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	pha
	inx
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2159)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2165)
	; 10 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2166)
	; 12 bytes
	sty @zptemp
	ldy #(n-24)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2170)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	sta @zptemp
	dey
	Jne *-4
	tay
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
.elseif(n = 2173)
	; 12 bytes
	sty @zptemp
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if(n = 2188)
	; 12 bytes
	pha
	tya
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2189)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jpl *-5
	ldy @zptemp
.elseif((n .mod 9 = 3) && ((n > 2189 && n < 2263)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2191 || n = 2215 || n = 2227 || n = 2251 || n = 2263)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2192 || n = 2202 || n = 2222 || n = 2252)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2193 || n = 2198 || n = 2204 || n = 2209 || n = 2214 || n = 2219 || n = 2228 || n = 2231 || n = 2246 || n = 2250 || n = 2254 || n = 2255 || n = 2258 || n = 2259 || n = 2264 || n = 2265)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 7) && ((n > 2193 && n < 2258)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2195 || n = 2206 || n = 2213 || n = 2225 || n = 2241 || n = 2245 || n = 2260)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2196 || n = 2220 || n = 2232 || n = 2256)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2197 || n = 2207 || n = 2247)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2200)
	; 12 bytes
	sty @zptemp
	ldy #(n-24)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2201 || n = 2223 || n = 2234)
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
.elseif(n = 2205)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2216)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2218 || n = 2229 || n = 2240)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2224)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2233 || n = 2261)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2236)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2237)
	; 10 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2238)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
	ldx @zptemp
.elseif(n = 2242)
	; 10 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
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
.elseif(n = 2249)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
.if((n >= 2266) && (n < 2305))
; 2266 to 2304
.if((n .mod 9 = 7) && ((n > 2265 && n < 2303)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2267 || n = 2277 || n = 2297)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2270 || n = 2274 || n = 2279 || n = 2286 || n = 2291 || n = 2301)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2273 || n = 2295)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2276 || n = 2283 || n = 2296)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2278 || n = 2300)
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
.elseif(n = 2285)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2287 || n = 2299)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2290)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
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
.elseif(n = 2303)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 2305 to 2343
.if(n = 2305 || n = 2310 || n = 2315 || n = 2320 || n = 2326 || n = 2334 || n = 2339)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2306)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2309 || n = 2313 || n = 2314 || n = 2318 || n = 2324 || n = 2333 || n = 2335 || n = 2338 || n = 2340)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2311)
	; 11 bytes
	sty @zptemp
	ldy #0
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2312 || n = 2322 || n = 2332)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 2317 || n = 2327)
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
.elseif(n = 2321)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2323)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2325)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #128
	brk 0
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 2330)
	; 11 bytes
	php
	sta @zptemp
	lda #9
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 2331)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2336)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2337 || n = 2342)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2341)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2343)
	; 11 bytes
	php
	pha
	php
	lda #9
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
.if((n >= 2344) && (n < 2383))
; 2344 to 2382
.if(n = 2344 || n = 2349)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2345)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2346 || n = 2351)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2347 || n = 2359 || n = 2371)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2348 || n = 2379 || n = 2380)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2350)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	sta @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2354 || n = 2369 || n = 2374)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2355 || n = 2366)
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
.elseif(n = 2356)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 2357 || n = 2367 || n = 2377)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2358 || n = 2375)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2360)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2361)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2362 || n = 2372 || n = 2382)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2363)
	; 12 bytes
	sty @zptemp
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2365)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2368)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2370)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
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
.elseif(n = 2384 || n = 2385 || n = 2390 || n = 2396 || n = 2403 || n = 2404 || n = 2408 || n = 2409 || n = 2411 || n = 2414 || n = 2418)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2386)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2387 || n = 2397 || n = 2417)
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
.elseif(n = 2389 || n = 2391 || n = 2413)
	; 12 bytes
	pha
	delay_n (n-3)
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
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2398)
	; 12 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	sta @zptemp
	dey
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2399 || n = 2421)
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
.elseif(n = 2401)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2406)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2410)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	inc @zptemp
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2415)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2420)
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
.else
.if((n >= 2422) && (n < 2461))
; 2422 to 2460
.if((n .mod 10 = 2) && ((n > 2421 && n < 2453)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2423 || n = 2426 || n = 2433 || n = 2440 || n = 2445 || n = 2450)
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
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jmi *-4
	ldy @zptemp
.elseif(n = 2427 || n = 2447 || n = 2457)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2428 || n = 2435 || n = 2444 || n = 2449)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2429)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2430)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 2431 || n = 2443 || n = 2455)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2437)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2438)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2439)
	; 12 bytes
	sty @zptemp
	ldy #(n-26)/19
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
	delay_n (n-13)
	brk 0
.endif
.else
; 2461 to 2500
.if(n = 2461)
	; 13 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2462 || n = 2482 || n = 2492)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2463)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2464 || n = 2469 || n = 2475 || n = 2480 || n = 2481 || n = 2486 || n = 2494 || n = 2495)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2465 || n = 2498)
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
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jmi *-5
	ldy @zptemp
.elseif(n = 2467 || n = 2479 || n = 2491)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2468)
	; 13 bytes
	pha
	txa
	ldx #245
	inc @zptemp
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 2470 || n = 2474 || n = 2488 || n = 2500)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2471)
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	brk 0
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
.elseif(n = 2473)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2476)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	brk 0
	Jne *-3
	tay
	pla
.elseif(n = 2477 || n = 2487 || n = 2497)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2478)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2483)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2485)
	; 10 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2489)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2490)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	sta @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2493)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
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
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2505 || n = 2510 || n = 2519 || n = 2524 || n = 2529 || n = 2533 || n = 2534 || n = 2536)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2506 || n = 2523)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2507 || n = 2517 || n = 2537)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2509)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2516)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 2518)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2525 || n = 2530 || n = 2535)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2531)
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2541 || n = 2555 || n = 2569)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2542 || n = 2552)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2543)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2548 || n = 2559 || n = 2570)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2549 || n = 2554 || n = 2558 || n = 2565 || n = 2576 || n = 2577)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2550)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
.elseif(n = 2553 || n = 2564)
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
.elseif(n = 2557)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2563 || n = 2575)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2566 || n = 2571 || n = 2573 || n = 2578)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2567)
	; 11 bytes
	sty @zptemp
	ldy #0
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
; 2579 to 2617
.if(n = 2579)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2581 || n = 2617)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2582 || n = 2588 || n = 2589 || n = 2591 || n = 2594 || n = 2596 || n = 2601)
	; 10..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2585 || n = 2609)
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
	; 11 bytes
	php
	sta @zptemp
	lda #10
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 2587)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2590 || n = 2595)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2593 || n = 2598)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 2597)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2599)
	; 11 bytes
	php
	pha
	php
	lda #10
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 2600 || n = 2605)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2602)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2603 || n = 2614)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2606)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2607)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2608)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.else
; 2618 to 2656
.if(n = 2618 || n = 2624 || n = 2637 || n = 2638 || n = 2642 || n = 2654 || n = 2655)
	; 12 bytes
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
.elseif(n = 2620 || n = 2633 || n = 2646)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2621 || n = 2645)
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2623 || n = 2635 || n = 2647)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2625 || n = 2636)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2626 || n = 2631 || n = 2643 || n = 2650 || n = 2656)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2629 || n = 2648)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2634)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2639)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2644)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	delay_n 6
	Jne *-4
	ldx @zptemp
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
.elseif(n = 2653)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 2657) && (n < 2735))
; 2657 to 2734
.if(n = 2657 || n = 2717)
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
.elseif(n = 2658 || n = 2694 || n = 2730)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2732)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2660 || n = 2661 || n = 2665 || n = 2666 || n = 2668 || n = 2673 || n = 2675 || n = 2678 || n = 2682 || n = 2690 || n = 2697 || n = 2699 || n = 2704 || n = 2720 || n = 2721 || n = 2726)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2662 || n = 2677 || n = 2722)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 11 = 1) && ((n > 2662 && n < 2697) || n = 2718 || n = 2729))
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
.elseif(n = 2667 || n = 2681 || n = 2709 || n = 2723)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 11 = 7) && ((n > 2668 && n < 2714)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2670 || n = 2706)
	; 13 bytes
	pha
	txa
	ldx #(n-18)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 2672 || n = 2686 || n = 2714 || n = 2728)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2679 || n = 2703 || n = 2716 || n = 2734)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2684)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2687)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2689 || n = 2725)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 2693 || n = 2710 || n = 2727)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2701)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2705)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2708)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2711)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2733)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if((n .mod 11 = 7) && ((n > 2734 && n < 2769)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
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
.elseif(n = 2738)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2739 || n = 2741 || n = 2753 || n = 2758 || n = 2764 || n = 2771)
	; 12 bytes
	pha
	delay_n (n-3)
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
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2745)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_15cyc
	Jne *-4
	ldy @zptemp
.elseif(n = 2747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2749)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2754)
	; 13 bytes
	pha
	txa
	ldx #(n-18)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 2759 || n = 2763 || n = 2769)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2761)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2766)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2781 || n = 2785 || n = 2786 || n = 2799 || n = 2804 || n = 2809)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2775 || n = 2780 || n = 2811)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2776)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
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
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2779 || n = 2791 || n = 2803)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2782)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	sta @zptemp
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 2783 || n = 2800)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2788)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 16
	dey
	Jmi *-5
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
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2792)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2793 || n = 2807)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2794)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	inc @zptemp
	iny
	Jne *-4
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
.elseif(n = 2797)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2802)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2805)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	iny
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
.elseif(n = 2814 || n = 2822 || n = 2825 || n = 2829 || n = 2831 || n = 2834 || n = 2841 || n = 2845 || n = 2847 || n = 2850)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2815 || n = 2827 || n = 2839)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2816 || n = 2830 || n = 2836 || n = 2846 || n = 2851)
	; 10..12 bytes
	pha
	delay_n (n-3)
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
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2819)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2821 || n = 2835)
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
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2824)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2833)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2837)
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
.elseif(n = 2838)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2842)
	; 11 bytes
	php
	sta @zptemp
	lda #11
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 2843)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2848)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2849)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2857 || n = 2865 || n = 2870 || n = 2871 || n = 2889)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 2856 || n = 2861 || n = 2888)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 2858)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 2859 || n = 2864)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 2860)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2862)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2863 || n = 2875 || n = 2887)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2866 || n = 2878 || n = 2883 || n = 2890)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2867)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
	brk 0
	dey
	Jne *-3
	tay
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
.elseif(n = 2879)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2881)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2882)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 16
	Jne *-4
	ldy @zptemp
.elseif(n = 2884)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
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
	txa
	ldx #(n-18)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.endif
.endif
.else
; 2891 to 2968
.if(n = 2891 || n = 2919 || n = 2933 || n = 2961)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 12 = 0) && ((n > 2891 && n < 2965)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2893 || n = 2945 || n = 2958)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2894 || n = 2901 || n = 2903 || n = 2906 || n = 2913 || n = 2915 || n = 2918 || n = 2920 || n = 2925 || n = 2929 || n = 2930 || n = 2937 || n = 2939 || n = 2942 || n = 2943 || n = 2949 || n = 2951 || n = 2954 || n = 2956)
	; 12..13 bytes
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
.elseif(n = 2896 || n = 2924 || n = 2938 || n = 2966)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2897 || n = 2914 || n = 2931 || n = 2948 || n = 2965)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2898 || n = 2963)
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
.elseif((n .mod 12 = 7) && ((n > 2898 && n < 2960)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2900 || n = 2957)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2902 || n = 2936 || n = 2953)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2905 || n = 2941)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2907 || n = 2927 || n = 2967)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2908 || n = 2926 || n = 2944 || n = 2950 || n = 2955)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2909 || n = 2921)
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
.elseif(n = 2910 || n = 2946)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2912 || n = 2932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2917 || n = 2962)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2934)
	; 13 bytes
	pha
	txa
	ldx #(n-18)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 2960)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2968)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if(n = 2969 || n = 2993 || n = 3041)
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
.elseif((n .mod 12 = 7) && ((n > 2970 && n < 3044)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2972 || n = 2992 || n = 3032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2973 || n = 2978 || n = 2979 || n = 2984 || n = 2985 || n = 2990 || n = 2996 || n = 2997 || n = 3001 || n = 3002 || n = 3009 || n = 3011 || n = 3014 || n = 3015 || n = 3020 || n = 3026 || n = 3033 || n = 3035 || n = 3038 || n = 3040)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2974 || n = 2991 || n = 2998 || n = 3037 || n = 3039 || n = 3046)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 14 = 7) && ((n > 2974 && n < 3018) || n = 3045))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 12 = 0) && ((n > 2975 && n < 3037)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2977 || n = 3013)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2980 || n = 2994 || n = 3008 || n = 3022)
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
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2982 || n = 3018)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2986)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2999 || n = 3016)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3005)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	delay_n 6
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3006 || n = 3030)
	; 13 bytes
	pha
	txa
	ldx #(n-18)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3010 || n = 3023)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3021)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 3025)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3027)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3028)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3029)
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3034)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	dey
	Jne *-4
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
.elseif(n = 3044)
	; 13 bytes
	delay_n (n-13)
	brk 0
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 6
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
.elseif(n = 3049 || n = 3085)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3050)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
	ldx @zptemp
.elseif(n = 3051 || n = 3056 || n = 3057 || n = 3061 || n = 3062 || n = 3069 || n = 3074 || n = 3076 || n = 3081 || n = 3083)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3053 || n = 3058 || n = 3063 || n = 3070 || n = 3075 || n = 3082)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3054)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 3065 || n = 3077)
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
.elseif(n = 3066)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3079)
	; 11 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.if(n = 3086 || n = 3100 || n = 3101 || n = 3103 || n = 3106 || n = 3108 || n = 3113)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3088)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3091)
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
.elseif(n = 3093 || n = 3102 || n = 3107 || n = 3124)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3094)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3095)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3096)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
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
.elseif(n = 3098)
	; 11 bytes
	php
	sta @zptemp
	lda #12
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 3099)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3104)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3105 || n = 3110)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3109)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3111)
	; 11 bytes
	php
	pha
	php
	lda #12
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 3112 || n = 3117 || n = 3125)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3114 || n = 3119)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3115 || n = 3120)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3116)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3118)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3121)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3122)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #12
	jsr delay_256a_16_clocks_b
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
; 3126 to 3203
.if(n = 3126 || n = 3144 || n = 3162 || n = 3180 || n = 3198)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
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
.elseif(n = 3129 || n = 3143 || n = 3171 || n = 3185 || n = 3199)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3130 || n = 3170)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3131 || n = 3137 || n = 3141 || n = 3146 || n = 3149 || n = 3150 || n = 3154 || n = 3155 || n = 3159 || n = 3163 || n = 3164 || n = 3168 || n = 3173 || n = 3177 || n = 3181 || n = 3182 || n = 3188 || n = 3195 || n = 3200)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3132 || n = 3172 || n = 3192)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3133 || n = 3151)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3134 || n = 3148 || n = 3176 || n = 3190)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3135 || n = 3152 || n = 3169 || n = 3186 || n = 3203)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3136 || n = 3178)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3138)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3139 || n = 3157 || n = 3175 || n = 3193)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3140 || n = 3174 || n = 3191)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3142 || n = 3161)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3145 || n = 3158 || n = 3184 || n = 3197)
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
.elseif(n = 3153 || n = 3179)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3156)
	; 12 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	pha
	jsr @nearby_rts_15cyc
	Jne *-5
	ldy @zptemp
.elseif(n = 3160 || n = 3165 || n = 3183 || n = 3196 || n = 3201)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3189)
	; 12 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 7
	dey
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3202)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3206 || n = 3209 || n = 3215 || n = 3231 || n = 3233 || n = 3236 || n = 3238)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3207)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3211 || n = 3229)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3214 || n = 3219 || n = 3226 || n = 3230 || n = 3235)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3216 || n = 3234)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3217)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3220 || n = 3237)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3221)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3222)
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
.elseif(n = 3223)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3224)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
.elseif(n = 3240)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3248 || n = 3249 || n = 3251 || n = 3278 || n = 3279)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3244)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	inc @zptemp
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 3245)
	; 12 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3247 || n = 3265)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3250 || n = 3268 || n = 3273)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3252 || n = 3270)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 3256 || n = 3275)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3257)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3262 || n = 3277)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
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
	txa
	ldx #24 ;hides 'clc'
	delay_n 7
	inx
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 3266)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3267)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3272)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
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
.if(n = 3282)
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
.elseif(n = 3283 || n = 3301 || n = 3319)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3284 || n = 3289 || n = 3291 || n = 3300)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3285 || n = 3290 || n = 3295 || n = 3303 || n = 3308 || n = 3315 || n = 3320)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3288 || n = 3306)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3292 || n = 3312)
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
.elseif(n = 3296 || n = 3309)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3297 || n = 3311)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3304)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3305)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3314)
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
.elseif(n = 3317)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 3321 to 3359
.if(n = 3321 || n = 3323 || n = 3326 || n = 3328 || n = 3329 || n = 3341 || n = 3349 || n = 3356 || n = 3357 || n = 3359)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3322)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3324 || n = 3342)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3325 || n = 3339 || n = 3353)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3331)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3333 || n = 3338 || n = 3340 || n = 3345 || n = 3358)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3334)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3335)
	; 12 bytes
	sty @zptemp
	ldy #0
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3336)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 3337)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3343)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3346)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3347)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3348)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3350)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3351)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3354)
	; 11 bytes
	php
	sta @zptemp
	lda #13
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 3355)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
.if((n >= 3360) && (n < 3379))
; 3360 to 3378
.if(n = 3360)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3361 || n = 3366)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3362 || n = 3364 || n = 3369 || n = 3377)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 3365)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3367)
	; 11 bytes
	php
	pha
	php
	lda #13
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 3368 || n = 3373)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3370 || n = 3375)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3371 || n = 3376)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 3372)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3374)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3378)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 3379 to 3398
.if(n = 3379)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3380 || n = 3393 || n = 3398)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3381 || n = 3395)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3382 || n = 3397)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3383)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3384)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3388)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
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
.elseif(n = 3390)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3391)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3392)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3394)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.endif
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3417)
	; 12 bytes
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
.elseif(n = 3401 || n = 3419)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3402)
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
.elseif(n = 3403 || n = 3421)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3405)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 10
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3406)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 10
	inx
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3407 || n = 3424)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3408)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3409 || n = 3427)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3411 || n = 3416 || n = 3418 || n = 3425 || n = 3426 || n = 3434)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3414 || n = 3432)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3415 || n = 3431)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3422)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3423 || n = 3437)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3430)
	; 12 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	brk 0
	Jne *-5
	ldy @zptemp
.elseif(n = 3433)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3435)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	brk 0
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
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3439 || n = 3443 || n = 3444 || n = 3449 || n = 3454 || n = 3460 || n = 3469 || n = 3474)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3440 || n = 3476)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3441 || n = 3458 || n = 3475)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3445 || n = 3463)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3446)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3448 || n = 3453 || n = 3459 || n = 3464 || n = 3471)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3450 || n = 3468)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
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
.elseif(n = 3455 || n = 3473)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
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
.elseif(n = 3461)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
.elseif(n = 3462)
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
.elseif(n = 3466)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 3477 to 3515
.if(n = 3477)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3478 || n = 3485 || n = 3496)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3481 || n = 3499)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3482)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3483 || n = 3488 || n = 3491 || n = 3494 || n = 3501 || n = 3505 || n = 3506)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3487)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3492 || n = 3509)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3495 || n = 3511)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3510)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3513)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3515)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 3516 to 3593
.if(n = 3516 || n = 3564 || n = 3580)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3517 || n = 3535 || n = 3553 || n = 3571 || n = 3589)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3518 || n = 3520 || n = 3525 || n = 3538 || n = 3561 || n = 3574 || n = 3585 || n = 3590)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3519 || n = 3523 || n = 3524 || n = 3528 || n = 3529 || n = 3533 || n = 3534 || n = 3537 || n = 3539 || n = 3542 || n = 3544 || n = 3545 || n = 3550 || n = 3551 || n = 3555 || n = 3557 || n = 3569 || n = 3570 || n = 3573 || n = 3578 || n = 3581 || n = 3586 || n = 3593)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3521 || n = 3549 || n = 3563 || n = 3577)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3522 || n = 3540 || n = 3558 || n = 3576)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3526 || n = 3554 || n = 3568 || n = 3582)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3527 || n = 3546 || n = 3584)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3530 || n = 3566)
	; 13 bytes
	delay_n (n-13)
	brk 0
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
.elseif(n = 3532 || n = 3552 || n = 3572 || n = 3592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3536)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jmi *-6
	ldy @zptemp
.elseif(n = 3541 || n = 3579)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3556)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3559 || n = 3575)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3562)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3583)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3588)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
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
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
; 3594 to 3632
.if(n = 3594 || n = 3612 || n = 3630)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3595)
	; 13 bytes
	pha
	sty @zptemp
	ldy #(n-10)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3597 || n = 3606 || n = 3614 || n = 3619)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3598)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3600 || n = 3601 || n = 3609 || n = 3613 || n = 3615 || n = 3618 || n = 3620)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3602 || n = 3624 || n = 3629)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3603)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3605)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 11
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3607 || n = 3625)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3608)
	; 13 bytes
	pha
	tya
	ldy #115
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 3610)
	; 11 bytes
	php
	sta @zptemp
	lda #14
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 3611)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3616)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3617 || n = 3622)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3621)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3623)
	; 11 bytes
	php
	pha
	php
	lda #14
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 3626 || n = 3631)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
; 3633 to 3671
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
	php
	pha
	inc @zptemp
	brk 0
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3635)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3636 || n = 3655)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3637)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3638 || n = 3649 || n = 3654 || n = 3663 || n = 3668)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3639)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3640)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3641 || n = 3660)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3642 || n = 3656)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3643 || n = 3661)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3646 || n = 3651 || n = 3658 || n = 3662 || n = 3667)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3647)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3648 || n = 3666)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3652)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
	brk 0
	sbc #1
	Jne *-4
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
.elseif(n = 3659)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	ldx @zptemp
.elseif(n = 3664)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	tax
	pla
.elseif(n = 3665)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3669)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3670)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk 0
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3671)
	; 12 bytes
	php
	delay_n (n-7)
	plp
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
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3673 || n = 3691)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3674 || n = 3693)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3675 || n = 3685 || n = 3700 || n = 3705)
	; 12..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3677 || n = 3710)
	; 12..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3678)
	; 13 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3679 || n = 3697)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3680)
	; 13 bytes
	pha
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 10
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3681 || n = 3683 || n = 3686 || n = 3688 || n = 3689 || n = 3694 || n = 3695 || n = 3699 || n = 3704)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3682)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3684 || n = 3702)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3687 || n = 3707)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3690)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3696)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3698)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 3703)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3709)
	; 13 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tay
	pla
.endif
.else
; 3711 to 3750
.if(n = 3711)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3712 || n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3713 || n = 3730 || n = 3747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3714 || n = 3734 || n = 3737 || n = 3740)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3715 || n = 3733)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3716 || n = 3721 || n = 3723 || n = 3739 || n = 3741 || n = 3748)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3717)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3719)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3720 || n = 3738)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
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
.elseif(n = 3724 || n = 3745)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3725)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3726 || n = 3744)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3727)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3728 || n = 3746)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3729)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3731 || n = 3750)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3736)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3742)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3743)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
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
.endif
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
; 3751 to 3789
.if(n = 3751 || n = 3769 || n = 3787)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3752 || n = 3786)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3753 || n = 3758 || n = 3760 || n = 3768 || n = 3771 || n = 3773 || n = 3776 || n = 3778 || n = 3789)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3756 || n = 3774)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3757)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3759 || n = 3770 || n = 3777)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3761 || n = 3779)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
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
.elseif(n = 3763)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3764 || n = 3781)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3765 || n = 3785)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3766)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3767)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3772)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3775)
	; 13 bytes
	pha
	sty @zptemp
	ldy #(n-10)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3780)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3782)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3783)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3784)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3788)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3795 || n = 3801 || n = 3806 || n = 3813)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3791)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3792 || n = 3810 || n = 3828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3793)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3794 || n = 3800 || n = 3809 || n = 3814 || n = 3819 || n = 3821 || n = 3824 || n = 3825)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3796 || n = 3818)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3797)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3798 || n = 3815)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3799)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3802 || n = 3817)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3805 || n = 3823)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3807 || n = 3826)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3808)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3811)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3816)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3822)
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
.elseif(n = 3827)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3830 || n = 3833 || n = 3834 || n = 3838 || n = 3839 || n = 3843 || n = 3848 || n = 3851 || n = 3856 || n = 3858 || n = 3861)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831 || n = 3850)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3832 || n = 3849)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3835 || n = 3857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3840)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3841 || n = 3859)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3844 || n = 3865)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3845)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3846 || n = 3864)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3847)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3852)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3860)
	; 14 bytes
	pha
	txa
	ldx #226
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 3862)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 3863)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3866)
	; 11 bytes
	php
	sta @zptemp
	lda #15
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 3867)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 3868 to 3906
.if(n = 3868 || n = 3869 || n = 3871 || n = 3874 || n = 3876 || n = 3881 || n = 3889 || n = 3894 || n = 3897 || n = 3899 || n = 3904)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870 || n = 3875 || n = 3898 || n = 3903)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3873 || n = 3878)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3877)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3879)
	; 11 bytes
	php
	pha
	php
	lda #15
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 3880 || n = 3885 || n = 3893)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3882 || n = 3900)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3887)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 3890)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3891)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3895)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3896)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3901)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3902)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
	jsr delay_256a_11_clocks_b_
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3908)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3909 || n = 3914 || n = 3915 || n = 3919 || n = 3920 || n = 3923 || n = 3924 || n = 3928 || n = 3929 || n = 3933 || n = 3935 || n = 3938 || n = 3941 || n = 3942)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3910 || n = 3916 || n = 3930)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3911)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3912 || n = 3932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3913 || n = 3931)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3917 || n = 3934)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3918 || n = 3936)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3921 || n = 3940)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3925)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3927)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3937)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3943)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	brk 0
	inx
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 3944)
	; 13 bytes
	sty @zptemp
	ldy #126
	delay_n 26
	dey
	Jpl *-5
	ldy @zptemp
.endif
.else
; 3946 to 3984
.if(n = 3946 || n = 3957 || n = 3970 || n = 3975)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3947)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3948)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	brk 0
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 3949 || n = 3967)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3950)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3951 || n = 3968)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3953 || n = 3958 || n = 3961 || n = 3966 || n = 3969 || n = 3971 || n = 3974)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3954 || n = 3972)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3955 || n = 3976)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3956)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3959)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3960 || n = 3981)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3962)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3963)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3964 || n = 3983)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3965 || n = 3984)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3973)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3977)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3978)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3979)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3982)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4003 || n = 4021 || n = 4039 || n = 4057)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3986)
	; 13 bytes
	pha
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 10
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3987 || n = 4027 || n = 4047)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3988 || n = 3993 || n = 4000 || n = 4006 || n = 4011 || n = 4013 || n = 4022 || n = 4029 || n = 4042)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3989 || n = 4033)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3990 || n = 4008 || n = 4026 || n = 4044 || n = 4062)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3991 || n = 4055)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3992 || n = 4012 || n = 4032 || n = 4052)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3994)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3995 || n = 4031)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3996)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3997 || n = 4016 || n = 4035 || n = 4054)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3998)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 3999 || n = 4004 || n = 4009 || n = 4014 || n = 4020 || n = 4023 || n = 4025 || n = 4028 || n = 4030 || n = 4034 || n = 4037 || n = 4038 || n = 4043 || n = 4046 || n = 4048 || n = 4049 || n = 4056 || n = 4061)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4001)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4002 || n = 4019 || n = 4036 || n = 4053)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4005)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	brk 0
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 4010)
	; 11 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	brk 0
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4015 || n = 4017 || n = 4051)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4018 || n = 4060)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4040 || n = 4059)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4045)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4050)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.endif
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if(n = 4063)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4064 || n = 4066 || n = 4069 || n = 4074 || n = 4077 || n = 4079 || n = 4082 || n = 4084 || n = 4089 || n = 4091 || n = 4094 || n = 4095 || n = 4099 || n = 4100)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 4067)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4068 || n = 4083 || n = 4090 || n = 4096 || n = 4101)
	; 12..13 bytes
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
.elseif(n = 4071)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4073)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4075 || n = 4093)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4080 || n = 4098)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4081)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4085)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4086)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4087)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4088)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4092)
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
; 4102 to 4140
.if(n = 4102)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4103)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4104 || n = 4121)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4105)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4106 || n = 4113 || n = 4115 || n = 4118 || n = 4120 || n = 4124 || n = 4125 || n = 4127 || n = 4130 || n = 4132 || n = 4137)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4107)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 4110 || n = 4117 || n = 4119 || n = 4126 || n = 4131)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4111)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4114)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4116)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4122)
	; 11 bytes
	php
	sta @zptemp
	lda #16
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 4123)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4128)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4129 || n = 4134)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4133)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4135)
	; 11 bytes
	php
	pha
	php
	lda #16
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 4136)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4138)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.if(n = 4141 || n = 4178)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4142)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4143)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4144)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4145 || n = 4151 || n = 4156 || n = 4157 || n = 4169 || n = 4174 || n = 4175 || n = 4179)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4147 || n = 4165)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4148)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4149 || n = 4168)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4150 || n = 4171)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4152 || n = 4170)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4153)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4154 || n = 4173)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4155 || n = 4172)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4158 || n = 4176)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
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
.elseif(n = 4161)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4162)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4163)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 9
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 4164)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 4180 to 4218
.if(n = 4180 || n = 4191 || n = 4204 || n = 4209 || n = 4215)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4181)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 11
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4182)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4183 || n = 4201)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4184)
	; 13 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	jsr @nearby_rts
	dey
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4185 || n = 4190 || n = 4196 || n = 4203 || n = 4208 || n = 4213)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4186)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4187 || n = 4207)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4188 || n = 4206)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4189)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4192)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
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
	brk 0
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
.elseif(n = 4198)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4199 || n = 4200 || n = 4202 || n = 4218)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 4212)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4214)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4216)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4217)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4237 || n = 4255 || n = 4273 || n = 4291)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4220)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	iny
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4221 || n = 4226 || n = 4229 || n = 4234 || n = 4239 || n = 4241 || n = 4246 || n = 4251 || n = 4254 || n = 4259 || n = 4264 || n = 4265 || n = 4269 || n = 4275 || n = 4277 || n = 4280 || n = 4284 || n = 4289 || n = 4293 || n = 4295)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4222 || n = 4235 || n = 4243 || n = 4258 || n = 4271 || n = 4276 || n = 4281 || n = 4294)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4223 || n = 4240 || n = 4257 || n = 4274)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4224 || n = 4242 || n = 4260 || n = 4278 || n = 4296)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4225 || n = 4244 || n = 4263 || n = 4282)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4227 || n = 4247 || n = 4267)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 4230 || n = 4249 || n = 4287)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4231 || n = 4253)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4232 || n = 4252 || n = 4272 || n = 4292)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4233)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4236)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4238 || n = 4256 || n = 4288)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4248 || n = 4266)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4250 || n = 4286)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4261)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4268)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-4
	ldx @zptemp
.elseif(n = 4270)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4283)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4285)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4290)
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
; 4297 to 4375
.if(n = 4297 || n = 4319 || n = 4341)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4298 || n = 4300 || n = 4303 || n = 4310 || n = 4311 || n = 4315 || n = 4316 || n = 4318 || n = 4322 || n = 4329 || n = 4331 || n = 4334 || n = 4336 || n = 4346 || n = 4349 || n = 4360 || n = 4361 || n = 4365 || n = 4369 || n = 4370 || n = 4374)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4317 || n = 4323 || n = 4328 || n = 4333 || n = 4335 || n = 4348 || n = 4353 || n = 4366 || n = 4371)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4301 || n = 4320 || n = 4339 || n = 4358)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4302 || n = 4324)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4304 || n = 4340 || n = 4343)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4305 || n = 4326)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4306 || n = 4344)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4307 || n = 4367)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4308 || n = 4342)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4309 || n = 4327 || n = 4345 || n = 4363)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4312 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4313 || n = 4347)
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
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4321 || n = 4357)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4325)
	; 11 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4330)
	; 11 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4337 || n = 4355 || n = 4373)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4338)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4351)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4354 || n = 4375)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4356)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
.if(n = 4376 || n = 4392 || n = 4397)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4377)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4378)
	; 11 bytes
	php
	sta @zptemp
	lda #17
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 4379)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4380 || n = 4381 || n = 4383 || n = 4386 || n = 4388 || n = 4393 || n = 4406 || n = 4409 || n = 4414)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4382 || n = 4387 || n = 4402)
	; 10..12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4384)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4385 || n = 4390)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4389)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4391)
	; 11 bytes
	php
	pha
	php
	lda #17
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 4394)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4395 || n = 4400)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4396)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4398)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4399)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4401)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4403)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4404)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
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
.elseif(n = 4407)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4408)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4434)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4416 || n = 4425 || n = 4443 || n = 4450)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4417 || n = 4435 || n = 4453)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 4419 || n = 4421 || n = 4424 || n = 4426 || n = 4436 || n = 4437 || n = 4441 || n = 4442 || n = 4444 || n = 4449)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4420 || n = 4439)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4422 || n = 4440)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4427 || n = 4447)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4428)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4429)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4430 || n = 4448)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4431 || n = 4433)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4432 || n = 4452)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4438)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4445)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4446)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4451)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 4454 to 4531
.if(n = 4454 || n = 4455 || n = 4457 || n = 4460 || n = 4462 || n = 4473 || n = 4475 || n = 4478 || n = 4493 || n = 4498 || n = 4503 || n = 4509 || n = 4511 || n = 4514 || n = 4516 || n = 4517 || n = 4524 || n = 4531)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4458 || n = 4476 || n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4459 || n = 4480 || n = 4501 || n = 4522)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4461 || n = 4470 || n = 4474 || n = 4479 || n = 4490 || n = 4497 || n = 4504 || n = 4513 || n = 4528)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4463 || n = 4481 || n = 4499)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4464 || n = 4485 || n = 4506)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4465 || n = 4483 || n = 4519)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4466 || n = 4484 || n = 4520)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4467 || n = 4487 || n = 4527)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4468 || n = 4486 || n = 4488 || n = 4505 || n = 4508 || n = 4521 || n = 4523)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4469)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4471 || n = 4489 || n = 4507 || n = 4525)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4472 || n = 4492)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4477 || n = 4496 || n = 4515)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4482)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4491 || n = 4510 || n = 4529)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4495)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4500)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4502)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4518)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk 0
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4526)
	; 13 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	delay_n 26
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
.if((n >= 4532) && (n < 4571))
; 4532 to 4570
.if(n = 4532 || n = 4552)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4533 || n = 4551 || n = 4569)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 4534 || n = 4553)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4535)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4536 || n = 4545 || n = 4549 || n = 4550 || n = 4554 || n = 4555 || n = 4563 || n = 4565 || n = 4568 || n = 4570)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4537)
	; 13 bytes
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
.elseif(n = 4539)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4540)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4541)
	; 12 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4542 || (n > 4557 && n < 4561))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4543 || n = 4561)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4544)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4546)
	; 12 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4547)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4548 || n = 4566)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4556)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4557)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4562)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4564)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4567)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 4571 to 4609
.if(n = 4571)
	; 13 bytes
	php
	pha
	sec
	lda #(n-17)/18
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4572)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4573 || n = 4598)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4574 || n = 4581 || n = 4588 || n = 4590 || n = 4593 || n = 4595 || n = 4599 || n = 4601 || n = 4606 || n = 4608)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4575 || n = 4582 || n = 4589 || n = 4594 || n = 4600 || n = 4607)
	; 11..13 bytes
	pha
	delay_n (n-3)
.elseif((n > 4575 && n < 4579) || n = 4580)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4579 || n = 4597)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4583)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4584 || n = 4602)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4585)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4586)
	; 10 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 4587)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 4592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4596)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4603)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4604)
	; 10 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4605)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4629)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4611 || n = 4613 || n = 4617 || n = 4619 || n = 4622 || n = 4626 || n = 4631 || n = 4636 || n = 4637 || n = 4639 || n = 4642 || n = 4644)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4618 || n = 4623 || n = 4638 || n = 4643)
	; 10..12 bytes
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
	; 11 bytes
	sty @zptemp
	ldy #0
	brk 0
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4616)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4620)
	; 11 bytes
	pha
	tya
	ldy #0
	brk 0
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4621)
	; 13 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4624)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4625)
	; 13 bytes
	php
	pha
	sec
	lda #0
	brk 0
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4627)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4628 || n = 4633 || n = 4648)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4630)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4634)
	; 11 bytes
	php
	sta @zptemp
	lda #18
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 4635)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4640)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4641 || n = 4646)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4645)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4647)
	; 11 bytes
	php
	pha
	php
	lda #18
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4657 || n = 4683)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	brk 0
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
.elseif(n = 4653 || n = 4661 || n = 4663 || n = 4666)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4654)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4655)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4658)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4659)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4660)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4662 || n = 4681)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4664)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4665 || n = 4670 || n = 4675 || n = 4677 || n = 4684)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4667 || n = 4686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4668)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4669)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4671)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4672)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4673 || n = 4678 || n = 4680)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4674)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	pha
	jsr @nearby_rts_15cyc
	Jne *-5
	ldx @zptemp
.elseif(n = 4676)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4679)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4682)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4685)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4687)
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
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
; 4688 to 4765
.if(n = 4688 || n = 4693 || n = 4708 || n = 4710 || n = 4725 || n = 4730 || n = 4735 || n = 4741 || n = 4746 || n = 4756 || n = 4760 || n = 4765)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4689 || n = 4694 || n = 4702 || n = 4709 || n = 4713 || n = 4714 || n = 4721 || n = 4726 || n = 4729 || n = 4734 || n = 4740 || n = 4745 || n = 4749 || n = 4754 || n = 4759 || n = 4764)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690)
	; 11 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4691)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4692 || n = 4712 || n = 4732 || n = 4752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4695 || n = 4716 || n = 4737 || n = 4758)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4696)
	; 13 bytes
	pha
	txa
	ldx #234 ;hides 'nop'
	brk 0
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4697)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4698 || n = 4720 || n = 4742)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4699)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4700 || n = 4719 || n = 4738 || n = 4757)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4701 || n = 4706 || n = 4718 || n = 4723 || n = 4728 || n = 4731 || n = 4733 || n = 4736 || n = 4739 || n = 4744 || n = 4748 || n = 4761 || n = 4763)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4703)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4704)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4705 || n = 4724 || n = 4743 || n = 4762)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4707 || n = 4727 || n = 4747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4711 || n = 4753)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4715)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4717)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4722)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4750)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4751)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.else
; 4766 to 4843
.if(n = 4766 || n = 4771 || n = 4780 || n = 4785 || n = 4788 || n = 4793 || n = 4799 || n = 4801 || n = 4804 || n = 4806 || n = 4811 || n = 4813 || n = 4820 || n = 4823 || n = 4824 || n = 4826 || n = 4828)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4767 || n = 4787 || n = 4807 || n = 4827)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4768 || n = 4791)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4769 || n = 4778 || n = 4783 || n = 4789 || n = 4794 || n = 4797 || n = 4802 || n = 4809 || n = 4818 || n = 4829 || n = 4834 || n = 4835 || n = 4839 || n = 4840)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4770 || n = 4775 || n = 4777 || n = 4784 || n = 4790 || n = 4798 || n = 4815 || n = 4822 || n = 4836 || n = 4841)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4772 || n = 4792 || n = 4812 || n = 4832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4773 || n = 4796)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4774 || n = 4816 || n = 4837)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4776 || n = 4795 || n = 4814 || n = 4833)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4779 || n = 4821 || n = 4842)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4781 || n = 4800 || n = 4819 || n = 4838)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4782)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4786 || n = 4808 || n = 4830)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4803 || n = 4825)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4805)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4810)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4817)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4831)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4843)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4849 || n = 4851 || n = 4854 || n = 4859 || n = 4873 || n = 4878 || n = 4881)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845 || n = 4864)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4846)
	; 14 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4847 || n = 4867)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4848)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4850 || n = 4855 || n = 4860 || n = 4866 || n = 4868 || n = 4870 || n = 4875 || n = 4880)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4852 || n = 4872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4853)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4856)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 4857)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4858 || n = 4879)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4863)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4865)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4869)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
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
.endif
.else
; 4883 to 4921
.if(n = 4883)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4884)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4885 || n = 4894 || n = 4899)
	; 10..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4886 || n = 4889 || n = 4892 || n = 4893 || n = 4895 || n = 4898 || n = 4900 || n = 4905 || n = 4913)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4887)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4888)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4890)
	; 11 bytes
	php
	sta @zptemp
	lda #19
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 4891)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4896)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4897 || n = 4902)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4901)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4903)
	; 11 bytes
	php
	pha
	php
	lda #19
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 4904 || n = 4909 || n = 4917)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4906 || n = 4911)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 4907 || n = 4912)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 4908)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4910)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4914)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4915)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4916)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 4918)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
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
.elseif(n = 4920)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4921)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4923 || n = 4929 || n = 4934 || n = 4944 || n = 4949 || n = 4954)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4925 || n = 4928 || n = 4931 || n = 4933 || n = 4938 || n = 4939 || n = 4943 || n = 4946 || n = 4948 || n = 4953 || n = 4956 || (n > 4957 && n < 4961))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4926)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4927 || n = 4947)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4930)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4932 || n = 4952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4935)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4936)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4937)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4940)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
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
.elseif(n = 4942)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4945 || n = 4950 || n = 4955)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4951)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4957)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 4961 to 5000
.if(n = 4961 || n = 4964 || n = 4981 || n = 4985 || n = 4988 || n = 4991 || n = 4993 || n = 4996 || n = 4997 || n = 5000)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4962)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4963 || n = 4984)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4965)
	; 11 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4966 || n = 4968 || n = 4973 || n = 4990 || n = 4995)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4967 || n = 4987)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4969 || n = 4974 || n = 4986 || n = 4994)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4970)
	; 11 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	brk 0
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4971)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4975)
	; 13 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	brk 0
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4976)
	; 13 bytes
	pha
	txa
	ldx #248 ;hides 'sed'
	brk 0
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4977)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4978)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4979)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4982)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4983)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 4989)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4998)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4999)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
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
; 5001 to 5078
.if(n = 5001 || n = 5023 || n = 5045)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5002 || n = 5004 || n = 5011 || n = 5013 || (n > 5015 && n < 5021) || n = 5022 || n = 5024 || n = 5025 || n = 5033 || n = 5036 || (n > 5037 && n < 5044) || n = 5046 || n = 5048 || n = 5051 || n = 5053 || n = 5056 || n = 5058 || n = 5059 || n = 5061 || n = 5063 || n = 5064 || n = 5066 || (n > 5075 && n < 5079))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5003 || n = 5065)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5005 || n = 5026 || n = 5068)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5006 || n = 5028 || n = 5050)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5007 || n = 5027 || n = 5047 || n = 5067)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5008 || n = 5015 || n = 5030 || n = 5035 || n = 5055 || n = 5070 || n = 5075)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5009 || n = 5014 || n = 5029 || n = 5034 || n = 5049 || n = 5054 || n = 5069 || n = 5074)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5010 || n = 5031 || n = 5073)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5012 || n = 5032 || n = 5052 || n = 5072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5021 || n = 5044)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5037 || n = 5062)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5057)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5060)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5071)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5081 || n = 5084 || n = 5086 || n = 5093)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5080)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	brk 0
	Jne *-5
	ldx @zptemp
.elseif(n = 5082)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5083 || n = 5088 || n = 5099 || n = 5104 || n = 5117)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5085)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	brk 0
	Jne *-5
	tax
	pla
.elseif(n = 5087 || n = 5107)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5089 || n = 5110)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5090 || n = 5113)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5091)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5092 || n = 5112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5094 || n = 5115)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5095)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5096)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 5097)
	; 14 bytes
	php
	pha
	sec
	lda #254
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5098 || n = 5100 || n = 5103 || n = 5105)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5102)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5106)
	; 13 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5108)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5109)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5111)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
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
.elseif(n = 5116)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5120 || n = 5123 || n = 5129 || n = 5134 || n = 5140 || n = 5148 || n = 5149 || n = 5151 || n = 5154 || n = 5156)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119 || n = 5124 || n = 5130 || n = 5135 || n = 5150 || n = 5155)
	; 10..13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 5122)
	; 13 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
	plp
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
.elseif(n = 5127)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5128 || n = 5139)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5131)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 5133)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5136)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5138)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5141)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5142)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5143)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5144)
	; 14 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5145)
	; 14 bytes
	pha
	tya
	ldy #0
	nop
	brk 0
	dey
	Jne *-4
	tay
	pla
	brk 0
.elseif(n = 5146)
	; 11 bytes
	php
	sta @zptemp
	lda #20
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 5147)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5152)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_16_clocks_b
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
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5160 || n = 5165 || n = 5178)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5161 || n = 5169 || n = 5175)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5162 || n = 5167)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5163 || n = 5168)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5164)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5166)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5170)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5171)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5172 || n = 5174)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5173 || n = 5194)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5176)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5177)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5179 || n = 5180 || n = 5193)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5181 || n = 5185 || n = 5190 || n = 5192 || n = 5195)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5182)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5183)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5184)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5186)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5187)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5188)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5189)
	; 13 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5191)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 5196 to 5234
.if(n = 5196 || n = 5217)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5197 || n = 5216)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5198 || (n > 5199 && n < 5203) || n = 5206 || n = 5208 || n = 5209 || n = 5211 || n = 5214 || n = 5219 || (n > 5221 && n < 5226) || n = 5227 || (n > 5228 && n < 5232) || n = 5234)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5199 || n = 5220)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5203)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5204 || n = 5226)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5205 || n = 5228)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5207 || n = 5232)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5210 || n = 5233)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5212)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5213)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5215)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5218)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5221)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 5235 to 5312
.if(n = 5235 || n = 5240 || n = 5242 || n = 5244 || n = 5245 || n = 5247 || n = 5249 || n = 5250 || (n > 5251 && n < 5256) || n = 5258 || n = 5261 || n = 5264 || n = 5266 || n = 5267 || n = 5269 || (n > 5270 && n < 5274) || n = 5275 || n = 5276 || (n > 5283 && n < 5287) || (n > 5288 && n < 5292) || (n > 5292 && n < 5297) || n = 5298 || n = 5300 || n = 5303 || n = 5305 || n = 5306 || n = 5310)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5236 || n = 5257 || n = 5278 || n = 5299)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5237 || n = 5312)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5238 || n = 5259 || n = 5280 || n = 5301)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5239 || n = 5263 || n = 5311)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5241 || n = 5262 || n = 5283 || n = 5304)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5243 || n = 5265 || n = 5287 || n = 5309)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5246 || n = 5277 || n = 5308)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5248 || n = 5270 || n = 5292)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
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
.elseif(n = 5256 || n = 5279 || n = 5302)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5260 || n = 5281)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5268)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5274 || n = 5297)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5288)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
	pla
.elseif(n = 5307)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
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
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif((n > 5314 && n < 5318) || n = 5319 || n = 5321 || n = 5324 || (n > 5325 && n < 5331) || n = 5333 || n = 5334 || n = 5338 || n = 5340 || n = 5342 || n = 5345 || n = 5347 || n = 5349 || n = 5350)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5318)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 5320 || n = 5341)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5322)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5323)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 5325 || n = 5346)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5331)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5332)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5335)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5337)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5339)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5343)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5348)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5351)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	pha
	iny
	Jne *-4
	ldy @zptemp
.endif
.else
; 5352 to 5390
.if(n = 5352)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5353 || n = 5355 || n = 5360 || n = 5364 || n = 5369 || n = 5373 || n = 5374 || n = 5376 || n = 5379 || n = 5381 || n = 5385)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5354 || n = 5365 || n = 5375 || n = 5386 || n = 5390)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5356)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	sta @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5357 || n = 5382)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5358 || n = 5380)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5359)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5361)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5362)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5363)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5366 || n = 5389)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5367)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5368)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5370)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5371)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5372)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	pha
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5377)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	sta @zptemp
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5378)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5383)
	; 12 bytes
	sty @zptemp
	ldy #0
	delay_n 16
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5384)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	cmp $C5
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5387)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5388)
	; 13 bytes
	pha
	tya
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
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
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5393 || n = 5409 || n = 5414)
	; 11..14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5394)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5395 || n = 5400 || n = 5406 || n = 5411)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5396 || n = 5399 || n = 5404 || n = 5405 || n = 5407 || n = 5410 || n = 5412 || n = 5417 || n = 5425)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5397)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5398)
	; 15 bytes
	pha
	txa
	ldx #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dex
	Jne *-6
	inc @zptemp,x
	tax
	pla
.elseif(n = 5401)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5402)
	; 11 bytes
	php
	sta @zptemp
	lda #21
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 5403)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5408)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5413)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5415)
	; 11 bytes
	php
	pha
	php
	lda #21
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5416 || n = 5421 || n = 5429)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5418 || n = 5423)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5419 || n = 5424)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5420)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5422)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5426)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5427)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5428)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.else
; 5430 to 5468
.if(n = 5430)
	; 14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5431 || n = 5455)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5432)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5433 || n = 5436 || n = 5449 || n = 5459)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5434)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5435 || n = 5458)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5437)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5438 || n = 5443 || n = 5445 || n = 5448 || (n > 5449 && n < 5455) || n = 5456 || n = 5460 || n = 5465 || n = 5467)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5439)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5440)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5441 || n = 5463)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5442)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5444)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5446 || n = 5468)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5457)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5461)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5462)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5464)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5466)
	; 12 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.endif
.else
; 5469 to 5625
.if(n = 5469 || n = 5609 || n = 5614)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif((n > 5469 && n < 5479) || n = 5480 || n = 5483 || n = 5484 || n = 5488 || n = 5489 || n = 5492 || n = 5493 || (n > 5494 && n < 5499) || (n > 5499 && n < 5503) || n = 5505 || n = 5506 || n = 5508 || n = 5510 || n = 5511 || (n > 5512 && n < 5519) || (n > 5519 && n < 5524) || n = 5526 || n = 5528 || n = 5531 || n = 5533 || n = 5535 || n = 5536 || (n > 5537 && n < 5549) || n = 5552 || n = 5553 || (n > 5557 && n < 5561) || n = 5563 || (n > 5564 && n < 5573) || n = 5574 || n = 5576 || n = 5577 || (n > 5578 && n < 5582) || (n > 5582 && n < 5587) || (n > 5587 && n < 5592) || n = 5593 || n = 5594 || n = 5597 || n = 5598 || (n > 5601 && n < 5606))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5479 || n = 5503 || n = 5575 || n = 5599)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5481 || n = 5504 || n = 5527 || n = 5550 || n = 5596 || n = 5619)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5482 || n = 5532 || n = 5557 || n = 5582 || n = 5607)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5485 || n = 5507 || n = 5529 || n = 5551 || n = 5573 || n = 5595 || n = 5617)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5486)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	ldx @zptemp
.elseif(n = 5487 || n = 5537 || n = 5562 || n = 5587 || n = 5612)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5490 || n = 5512 || n = 5534 || n = 5556 || n = 5578 || n = 5600 || n = 5622)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5491)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
	tax
	pla
.elseif(n = 5494 || n = 5525 || n = 5618)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 5509 || n = 5555 || n = 5601 || n = 5624)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5519)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5524 || n = 5564)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5549)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 5554)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 5606)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5608 || n = 5613 || n = 5615 || n = 5620 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5610)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5616)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5621)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5627 || n = 5629 || n = 5631 || n = 5636 || n = 5645 || n = 5662)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5628)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5630 || n = 5635 || n = 5641 || n = 5643 || n = 5646 || n = 5648 || n = 5651 || n = 5653 || n = 5656 || n = 5660 || n = 5661 || n = 5663)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5632 || n = 5657)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 5634)
	; 14 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 5637)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
.elseif(n = 5639)
	; 13 bytes
	sty @zptemp
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5640)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5642)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5644)
	; 13 bytes
	pha
	tya
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5647)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5649)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5650)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5652)
	; 14 bytes
	pha
	tya
	ldy #235
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
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
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5658)
	; 11 bytes
	php
	sta @zptemp
	lda #22
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 5659)
	; 9 bytes
	php
	pha
	lda #22
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5664)
	; 9 bytes
	php
	pha
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5670)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5666 || n = 5668 || n = 5673 || n = 5681)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 5669)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5671)
	; 11 bytes
	php
	pha
	php
	lda #22
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5672 || n = 5677 || n = 5685 || n = 5690)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5674 || n = 5679)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5675 || n = 5680)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5676)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5678)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5682)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5683)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5684 || n = 5686)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5687)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5689)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 10
	brk 0
	Jne *-6
	ldx @zptemp
.elseif(n = 5691 || n = 5692)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5693)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5694 || n = 5696 || n = 5697 || n = 5699 || n = 5701 || n = 5702)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5695)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5698)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5700)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5703)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 5704 to 5781
.if(n = 5704 || n = 5706 || n = 5708 || n = 5710 || n = 5713 || n = 5717 || n = 5718 || (n > 5719 && n < 5732) || n = 5733 || n = 5735 || n = 5736 || n = 5738 || n = 5740 || n = 5741 || (n > 5743 && n < 5747) || (n > 5747 && n < 5757) || (n > 5757 && n < 5762) || (n > 5762 && n < 5767) || (n > 5767 && n < 5773) || n = 5774 || n = 5776 || n = 5777 || n = 5779 || n = 5781)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5705 || n = 5715)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5707 || n = 5732 || n = 5757)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5709)
	; 12 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 16
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5711 || n = 5742 || n = 5773)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5712 || n = 5737 || n = 5762)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5714)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 5719 || n = 5743 || n = 5767)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5734 || n = 5780)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5739)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5775)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if(n = 5782 || n = 5807 || n = 5832 || n = 5857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5783 || n = 5784 || n = 5786 || (n > 5787 && n < 5791) || (n > 5791 && n < 5803) || n = 5806 || n = 5811 || n = 5813 || n = 5814 || (n > 5815 && n < 5820) || (n > 5820 && n < 5826) || (n > 5826 && n < 5831) || n = 5833 || n = 5834 || n = 5836 || n = 5838 || n = 5841 || (n > 5842 && n < 5849) || (n > 5849 && n < 5854) || n = 5856 || n = 5858 || n = 5859)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5785 || n = 5808 || n = 5831 || n = 5854)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5787 || n = 5812 || n = 5837)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5791 || n = 5815 || n = 5839)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5803 || n = 5826 || n = 5849)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5804 || n = 5835)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5805)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	pha
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5810)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	inc @zptemp
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5820)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5842)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 7
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5855)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	inc @zptemp
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5861)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	pha
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5862 || n = 5887)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5863)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5864 || n = 5865 || n = 5869 || n = 5875 || n = 5880 || n = 5885 || n = 5890 || n = 5892)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5866)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5867)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5868 || n = 5870 || n = 5873 || n = 5874 || n = 5879 || n = 5886 || n = 5891)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5876)
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
.elseif(n = 5877)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5878 || n = 5896)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5881)
	; 14 bytes
	sty @zptemp
	ldy #178
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5882)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5883)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5884)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	pha
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5888)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5889)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc @zptemp
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5893)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5894)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	inc @zptemp
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5895)
	; 13 bytes
	sty @zptemp
	ldy #0
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5897)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5898)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5899 to 5937
.if(n = 5899 || n = 5901 || n = 5904 || n = 5906 || n = 5913 || n = 5916 || n = 5917 || n = 5919 || n = 5922 || n = 5924 || n = 5929 || n = 5937)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 5900)
	; 13 bytes
	pha
	tya
	ldy #0
	inc @zptemp
	brk 0
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 5903 || n = 5905 || n = 5910 || n = 5918 || n = 5923)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5907)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5911)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5912)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5914)
	; 11 bytes
	php
	sta @zptemp
	lda #23
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 5915)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5921 || n = 5926)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 5925)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5927)
	; 11 bytes
	php
	pha
	php
	lda #23
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5928 || n = 5933)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5930 || n = 5935)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 5931 || n = 5936)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 5932)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5934)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5939)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5940 || n = 5942)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 5941 || n = 5946)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 5943)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5944)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5945 || n = 5947 || n = 5948 || n = 5961 || n = 5971 || n = 5975 || n = 6036 || n = 6076)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5949 || n = 5950 || n = 5952 || n = 5953 || n = 5955 || n = 5958 || n = 5960 || n = 5963 || (n > 5964 && n < 5971) || (n > 5971 && n < 5975) || (n > 5975 && n < 5982) || (n > 5983 && n < 5987) || n = 5988 || n = 5989 || (n > 5990 && n < 5995) || (n > 5995 && n < 6007) || (n > 6007 && n < 6012) || (n > 6012 && n < 6021) || (n > 6021 && n < 6026) || (n > 6026 && n < 6031) || (n > 6032 && n < 6036) || (n > 6037 && n < 6052) || n = 6053 || n = 6054 || n = 6056 || (n > 6057 && n < 6062) || (n > 6062 && n < 6076) || n = 6077 || n = 6078 || n = 6080 || n = 6081 || (n > 6083 && n < 6087) || (n > 6089 && n < 6094))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5951)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5954)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5956)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5957)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5959)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5962)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
.elseif(n = 5982 || n = 6007 || n = 6032 || n = 6082)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5983 || n = 6031 || n = 6055 || n = 6079)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5987 || n = 6012 || n = 6037 || n = 6062 || n = 6087)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5990 || n = 6021 || n = 6052 || n = 6083)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6089)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 19
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if(n = 6094 || (n > 6095 && n < 6100) || n = 6101 || n = 6102 || (n > 6103 && n < 6107) || (n > 6107 && n < 6112) || n = 6113 || n = 6115 || n = 6117 || n = 6120)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6095)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk 0
	Jne *-6
	ldx @zptemp
.elseif(n = 6100)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk 0
	Jne *-6
	tax
	pla
.elseif(n = 6103 || n = 6127)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6107 || n = 6132)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6112)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6114)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6116)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 6118 || n = 6123 || n = 6129)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6122)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6124)
	; 14 bytes
	pha
	tya
	ldy #191
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6125)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6126)
	; 15 bytes
	sty @zptemp
	ldy #211
	delay_n 24
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6128)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6130)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6131)
	; 15 bytes
	pha
	tya
	ldy #211
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 6133 to 6171
.if(n = 6133 || n = 6146)
	; 14..15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6134 || n = 6138 || n = 6139 || n = 6142 || n = 6144 || n = 6147 || n = 6152 || n = 6158 || n = 6159 || n = 6164)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6135 || n = 6143 || n = 6153 || n = 6154 || n = 6160 || n = 6165)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6136 || n = 6163)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6137 || n = 6162)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6140)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 6
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6141 || n = 6168)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
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
.elseif(n = 6148)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
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
.elseif(n = 6151)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 19
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6155)
	; 15 bytes
	sty @zptemp
	ldy #212
	delay_n 24
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 6157)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6161)
	; 14 bytes
	sty @zptemp
	ldy #181
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6166)
	; 15 bytes
	pha
	tya
	ldy #181
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6167)
	; 14 bytes
	sty @zptemp
	ldy #220
	delay_n 10
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6169)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6170)
	; 11 bytes
	php
	sta @zptemp
	lda #24
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 6171)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172 || n = 6173 || n = 6175 || n = 6178 || n = 6180 || n = 6185 || n = 6193 || n = 6199)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6177 || n = 6182)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6181)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6183)
	; 11 bytes
	php
	pha
	php
	lda #24
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6184 || n = 6189 || n = 6197 || n = 6202)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6186 || n = 6191)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6187 || n = 6192)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6188)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6190)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6194)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6195)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6196 || n = 6198)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6200)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6201 || n = 6203 || n = 6204 || n = 6217 || n = 6227 || n = 6231)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6205)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	pha
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6206 || n = 6208 || n = 6209 || n = 6211 || n = 6213 || n = 6214 || n = 6216 || (n > 6217 && n < 6227) || (n > 6227 && n < 6231) || (n > 6232 && n < 6237) || (n > 6238 && n < 6243) || (n > 6243 && n < 6251))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6207 || n = 6238)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6210)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6232)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6237)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
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
.if((n >= 6251) && (n < 6329))
; 6251 to 6328
.if((n > 6250 && n < 6257) || (n > 6257 && n < 6262) || (n > 6262 && n < 6269) || (n > 6269 && n < 6274) || (n > 6274 && n < 6282) || (n > 6282 && n < 6287) || (n > 6287 && n < 6292) || (n > 6292 && n < 6298) || n = 6299 || n = 6302 || n = 6304 || (n > 6307 && n < 6312) || (n > 6312 && n < 6323) || (n > 6323 && n < 6328))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6257 || n = 6282 || n = 6307)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6262 || n = 6287 || n = 6312)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6269 || n = 6300)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6274 || n = 6305)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6292)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6298)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	ldx @zptemp
.elseif(n = 6301 || n = 6306)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 6303)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
	pla
.elseif(n = 6323)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6328)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6329 to 6406
.if(n = 6329 || n = 6330 || (n > 6332 && n < 6336) || (n > 6337 && n < 6357) || (n > 6357 && n < 6362) || (n > 6362 && n < 6367) || (n > 6367 && n < 6371) || n = 6372 || n = 6375)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6331 || n = 6393)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6332 || n = 6357 || n = 6382)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6336 || n = 6367 || n = 6398)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6337 || n = 6362 || n = 6387)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 7
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6371)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 6373 || n = 6378 || n = 6380 || n = 6386 || n = 6389 || n = 6395 || n = 6400 || n = 6403 || n = 6405)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6374 || n = 6379 || n = 6385 || n = 6390 || n = 6399 || n = 6404)
	; 13..14 bytes
	pha
	delay_n (n-3)
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6388)
	; 15 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6391)
	; 14 bytes
	sty @zptemp
	ldy #228
	delay_n 10
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 6396)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6397)
	; 14 bytes
	sty @zptemp
	ldy #213
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
	ldy #213
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6406)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
; 6407 to 6445
.if(n = 6407)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6408)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6409)
	; 14 bytes
	sty @zptemp
	ldy #194
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6410)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6411)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
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
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6413 || n = 6423 || n = 6428 || n = 6429 || n = 6431 || n = 6434 || n = 6436 || n = 6441)
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
.elseif(n = 6415 || n = 6422 || n = 6430 || n = 6435)
	; 10..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6416)
	; 15 bytes
	sty @zptemp
	ldy #221
	delay_n 24
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6417)
	; 15 bytes
	pha
	tya
	ldy #183
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6418)
	; 15 bytes
	pha
	txa
	ldx #0
	delay_n 7
	brk 0
	dex
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 6419)
	; 14 bytes
	sty @zptemp
	ldy #229
	delay_n 10
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6420 || n = 6425 || n = 6440 || n = 6445)
	; 11..15 bytes
	delay_n (n-13)
	brk 0
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6426)
	; 11 bytes
	php
	sta @zptemp
	lda #25
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 6427)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6432)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6433 || n = 6438)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6437)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6439)
	; 11 bytes
	php
	pha
	php
	lda #25
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6442)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6447)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6448)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6449)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6450)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6451)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6452 || n = 6454)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6453 || n = 6458)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6455)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6456)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6457 || n = 6459 || n = 6473 || n = 6483)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
.elseif(n = 6461 || n = 6462 || n = 6464 || n = 6465 || n = 6467 || n = 6469 || n = 6470 || n = 6472 || (n > 6473 && n < 6483) || n = 6484)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6463)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6466)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6468)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6471)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 6485 to 6562
.if(n = 6485 || (n > 6487 && n < 6491) || (n > 6491 && n < 6501) || (n > 6501 && n < 6506) || (n > 6506 && n < 6517) || (n > 6517 && n < 6522) || (n > 6522 && n < 6543) || (n > 6543 && n < 6548) || (n > 6548 && n < 6553) || (n > 6553 && n < 6563))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6486 || n = 6517 || n = 6548)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6487)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
.elseif(n = 6501)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-6
	ldx @zptemp
.elseif(n = 6506)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-6
	tax
	pla
.elseif(n = 6543)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6579) || (n > 6579 && n < 6584) || (n > 6584 && n < 6588) || (n > 6588 && n < 6605) || (n > 6605 && n < 6610) || (n > 6610 && n < 6615) || (n > 6615 && n < 6626) || (n > 6626 && n < 6633))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6579)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 6588)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6605)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 26
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 6626)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	delay_n 6
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 6633)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6634)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6635)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 6636)
	; 14 bytes
	pha
	tya
	ldy #207
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6637)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6638 || n = 6639)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6640)
	; 14 bytes
	sty @zptemp
	ldy #201
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
.if((n >= 6641) && (n < 6680))
; 6641 to 6679
.if(n = 6641 || n = 6672)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6642)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6643 || n = 6647 || n = 6648 || n = 6650 || n = 6651 || n = 6656 || n = 6664 || n = 6665 || n = 6669 || n = 6670 || n = 6674 || n = 6679)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6644 || n = 6655 || n = 6660 || n = 6666 || n = 6675)
	; 14..15 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 6646 || n = 6677)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6649 || n = 6676)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6652)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 6
	dey
	Jne *-6
	ldy @zptemp
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
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
.elseif(n = 6658)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6659)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6661)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6662)
	; 15 bytes
	pha
	tya
	ldy #190
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6663)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6667)
	; 14 bytes
	sty @zptemp
	ldy #222
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6668)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6671)
	; 14 bytes
	sty @zptemp
	ldy #196
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6673)
	; 14 bytes
	sty @zptemp
	ldy #202
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6682)
	; 11 bytes
	php
	sta @zptemp
	lda #26
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 6683)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6684 || n = 6685 || n = 6687 || n = 6690 || n = 6692 || n = 6697 || n = 6705 || n = 6711)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6689 || n = 6694)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6693)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6695)
	; 11 bytes
	php
	pha
	php
	lda #26
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6696 || n = 6701 || n = 6709 || n = 6714)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6698 || n = 6703)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6699 || n = 6704)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6700)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6702)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6706)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6707)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6708 || n = 6710)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6712)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6713 || n = 6715 || n = 6716)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6717 || n = 6718)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 6719 to 6875
.if(n = 6719)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6720 || n = 6721 || n = 6723 || n = 6725 || n = 6726 || n = 6728 || (n > 6729 && n < 6734) || (n > 6734 && n < 6739) || (n > 6739 && n < 6743) || (n > 6743 && n < 6765) || (n > 6765 && n < 6770) || (n > 6770 && n < 6796) || n = 6797 || n = 6798 || n = 6800 || n = 6802 || n = 6803 || (n > 6804 && n < 6827) || (n > 6827 && n < 6832) || (n > 6832 && n < 6844) || (n > 6844 && n < 6858) || (n > 6858 && n < 6863) || (n > 6863 && n < 6876))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6722)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6724)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6727)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6729 || n = 6743 || n = 6804 || n = 6844)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6734 || n = 6765 || n = 6796 || n = 6827 || n = 6858)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 6799)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
; 6876 to 6914
.if((n > 6875 && n < 6881) || (n > 6881 && n < 6886) || n = 6887 || n = 6888)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6881)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 6889)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6890)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6891 || n = 6896 || n = 6898 || n = 6899 || n = 6906 || n = 6910)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 6892)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6893 || n = 6901)
	; 15 bytes
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
	; 14 bytes
	sty @zptemp
	ldy #246
	delay_n 10
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6897)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6900)
	; 15 bytes
	pha
	tya
	ldy #246
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6902)
	; 15 bytes
	sty @zptemp
	ldy #197
	delay_n 30
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6903)
	; 16 bytes
	pha
	txa
	ldx #255
	delay_n 7
	jsr @nearby_rts_15cyc
	dex
	Jne *-6
	inc @zptemp,x
	tax
	pla
.elseif(n = 6904)
	; 14 bytes
	sty @zptemp
	ldy #209
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6905)
	; 16 bytes
	sty @zptemp
	ldy #255
	delay_n 9
	brk 0
	dey
	Jne *-6
	ldy @zptemp
	brk 0
.elseif(n = 6907)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 29
	Jne *-6
	ldx @zptemp
.elseif(n = 6908)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 6911)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6912)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	Jne *-7
	tax
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
.elseif(n = 6914)
	; 15 bytes
	pha
	tya
	ldy #238
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 6915 to 6953
.if(n = 6915 || n = 6921 || n = 6922 || n = 6926 || n = 6927 || n = 6929 || n = 6934 || n = 6940 || n = 6941 || n = 6943 || n = 6946 || n = 6948 || n = 6953)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 6916 || n = 6923 || n = 6928 || n = 6942 || n = 6947)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 6917)
	; 14 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 6919)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6920)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6924)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
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
	txa
	ldx #(n-18)/31
	delay_n 26
	dex
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 6932)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6933 || n = 6952)
	; 11..15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6935)
	; 15 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 7
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-8
	ldx @zptemp
.elseif(n = 6936)
	; 16 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6937)
	; 14 bytes
	sty @zptemp
	ldy #210
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6938)
	; 11 bytes
	php
	sta @zptemp
	lda #27
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 6939)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6944)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6945 || n = 6950)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 6949)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6951)
	; 11 bytes
	php
	pha
	php
	lda #27
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954 || n = 6959)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 6955 || n = 6960)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 6956)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6957 || n = 6965 || n = 6970)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 6958)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6961 || n = 6967)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6962)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6963)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6964 || n = 6966)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 6968)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6969 || n = 6971 || n = 6972 || n = 6985 || n = 6995 || n = 6999)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6973 || n = 6974 || n = 6976 || n = 6977 || n = 6979 || n = 6981 || n = 6984 || n = 6986 || (n > 6987 && n < 6995) || (n > 6995 && n < 6999) || (n > 6999 && n < 7013) || (n > 7013 && n < 7018) || (n > 7018 && n < 7032))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6975)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6978)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6980)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6982 || n = 7013)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6983)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.if((n > 7031 && n < 7044) || (n > 7044 && n < 7049) || (n > 7049 && n < 7055) || (n > 7055 && n < 7060) || (n > 7060 && n < 7075) || (n > 7075 && n < 7080) || (n > 7080 && n < 7100) || (n > 7100 && n < 7106) || (n > 7106 && n < 7110))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7044 || n = 7075 || n = 7106)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 7055)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7060 || n = 7100)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if(n = 7110 || (n > 7111 && n < 7133) || n = 7134 || n = 7135 || (n > 7138 && n < 7142) || n = 7143 || n = 7144)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 7133)
	; 13 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7136)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 10
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 7137)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7138)
	; 13 bytes
	pha
	tya
	ldy #216 ;hides 'cld'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7145)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7146)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7147)
	; 14 bytes
	sty @zptemp
	ldy #238
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7148)
	; 14 bytes
	pha
	tya
	ldy #223
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 7149 to 7187
.if(n = 7149 || n = 7159 || n = 7163)
	; 15..17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7150 || n = 7151 || n = 7155 || n = 7160 || n = 7161 || n = 7165)
	; 15..16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7152)
	; 14 bytes
	pha
	tya
	ldy #238
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7153)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7154 || n = 7158 || n = 7166 || n = 7170 || n = 7172 || n = 7175 || n = 7183)
	; 14..16 bytes
	delay_n (n-2)
	nop
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
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7162)
	; 16 bytes
	sty @zptemp
	ldy #159
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 7164)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 10
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7167 || n = 7171 || n = 7176)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7168)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
	ldy #200 ;hides 'iny'
	delay_n 27
	dey
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7177)
	; 14 bytes
	sty @zptemp
	ldy #239
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #163
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7180)
	; 14 bytes
	pha
	tya
	ldy #224
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7181)
	; 14 bytes
	sty @zptemp
	ldy #211
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7182)
	; 14 bytes
	pha
	tya
	ldy #239
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
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
	sty @zptemp
	ldy #194
	delay_n 32
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7186)
	; 15 bytes
	pha
	tya
	ldy #211
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7187)
	; 15 bytes
	pha
	tya
	ldy #205
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
; 7188 to 7226
.if(n = 7188 || n = 7191 || n = 7196 || n = 7197 || n = 7199 || n = 7202 || n = 7204 || n = 7209 || n = 7217 || n = 7223)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7189)
	; 15 bytes
	sty @zptemp
	ldy #189
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7190)
	; 16 bytes
	pha
	tya
	ldy #194
	nop
	delay_n 30
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7192 || n = 7198 || n = 7203)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 7193)
	; 16 bytes
	pha
	tya
	ldy #167
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 7194)
	; 11 bytes
	php
	sta @zptemp
	lda #28
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 7195)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7200)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7201 || n = 7206)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7205)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7207)
	; 11 bytes
	php
	pha
	php
	lda #28
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7208 || n = 7213 || n = 7221 || n = 7226)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7210 || n = 7215)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7211 || n = 7216)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7212)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7214)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7218)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7219)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7220 || n = 7222)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7224)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7225)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 7227 to 7265
.if(n = 7227 || n = 7228 || n = 7241 || n = 7251 || n = 7255)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7229 || n = 7232 || n = 7233 || n = 7237 || n = 7238 || n = 7240 || (n > 7241 && n < 7251) || (n > 7251 && n < 7255) || (n > 7255 && n < 7261) || (n > 7261 && n < 7266))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7230 || n = 7261)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7231)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7234)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7235)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7236)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7239)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 7266 to 7343
.if(n = 7266 || n = 7297 || n = 7328)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif((n > 7266 && n < 7292) || (n > 7292 && n < 7297) || (n > 7297 && n < 7311) || (n > 7311 && n < 7316) || (n > 7316 && n < 7323) || (n > 7323 && n < 7328) || (n > 7328 && n < 7344))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7292 || n = 7323)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7311)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7316)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7354) || n = 7355 || n = 7357 || n = 7358 || (n > 7359 && n < 7385) || (n > 7385 && n < 7390) || (n > 7390 && n < 7401))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7354 || n = 7385 || n = 7416)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7356)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
.elseif(n = 7401)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-261)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7402)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
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
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 30
	iny
	Jne *-8
	ldy @zptemp
.elseif(n = 7406 || n = 7409 || n = 7418)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7407)
	; 15 bytes
	pha
	tya
	ldy #255
	delay_n 24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7408)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7410)
	; 15 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 30
	iny
	Jne *-8
	tay
	pla
.elseif(n = 7411)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7412)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7413)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7414)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7415)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #13
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 7417)
	; 14 bytes
	sty @zptemp
	ldy #247
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7419)
	; 14 bytes
	sty @zptemp
	ldy #218
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7420)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 11
	dey
	Jne *-7
	ldy @zptemp
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
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7423 || n = 7430 || n = 7433 || n = 7438 || n = 7439 || n = 7441 || n = 7449 || n = 7452 || n = 7453 || n = 7455 || n = 7458 || n = 7460)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424 || n = 7435 || n = 7440 || n = 7454 || n = 7459)
	; 10..15 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 7427)
	; 15 bytes
	sty @zptemp
	ldy #212
	delay_n 30
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7428)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7429)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7431)
	; 14 bytes
	sty @zptemp
	ldy #232
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7432)
	; 14 bytes
	sty @zptemp
	ldy #225
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7434)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk 0
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7436)
	; 14 bytes
	pha
	tya
	ldy #232
	delay_n 27
	dey
	Jne *-6
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
	sty @zptemp
	ldy #169
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7444)
	; 15 bytes
	sty @zptemp
	ldy #201
	delay_n 32
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7445)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 10
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7446)
	; 16 bytes
	sty @zptemp
	ldy #173
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 7447)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 7450)
	; 11 bytes
	php
	sta @zptemp
	lda #29
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 7451)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7456)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7457)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7464 || n = 7469 || n = 7477 || n = 7482)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7465 || n = 7473 || n = 7479)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7466 || n = 7471)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7467 || n = 7472)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7468)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7470)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7474)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7475)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7476)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7478)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7480)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7481 || n = 7484 || n = 7497)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
.elseif(n = 7485 || n = 7486 || n = 7488 || n = 7489 || n = 7491 || n = 7493 || n = 7494 || n = 7496 || (n > 7497 && n < 7501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7487)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7490)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7492)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7495)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.if((n > 7500 && n < 7507) || n = 7508 || n = 7510 || n = 7512 || n = 7513 || (n > 7514 && n < 7540) || (n > 7540 && n < 7545) || (n > 7545 && n < 7567) || (n > 7567 && n < 7571) || (n > 7572 && n < 7576) || (n > 7576 && n < 7602) || (n > 7602 && n < 7607) || (n > 7607 && n < 7612) || (n > 7612 && n < 7633) || (n > 7633 && n < 7638) || (n > 7638 && n < 7646) || (n > 7646 && n < 7651) || (n > 7651 && n < 7657))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7507 || n = 7511 || n = 7572 || n = 7612)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7509 || n = 7540 || n = 7571 || n = 7602 || n = 7633)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 7567)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7646)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
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
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657)
	; 14 bytes
	sty @zptemp
	ldy #255
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7658)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7659 || n = 7667 || n = 7672)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7660)
	; 14 bytes
	pha
	tya
	ldy #239
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7661)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 7662)
	; 14 bytes
	pha
	tya
	ldy #255
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7663)
	; 14 bytes
	sty @zptemp
	ldy #232
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7664)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7665 || n = 7670 || n = 7671)
	; 14..15 bytes
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
.elseif(n = 7673)
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
	txa
	ldx #(n-18)/31
	delay_n 26
	dex
	Jne *-5
	inc @zptemp,x
	tax
	pla
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7677)
	; 15 bytes
	pha
	tya
	ldy #219
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7678 || n = 7683 || n = 7685 || n = 7689 || n = 7694)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7679 || n = 7684 || n = 7690)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 7680)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	inc @zptemp
	delay_n 26
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
.elseif(n = 7682)
	; 15 bytes
	delay_n (n-13)
	brk 0
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
	; 14 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 7691)
	; 14 bytes
	sty @zptemp
	ldy #226
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7692)
	; 14 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7693)
	; 15 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 24
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 7695)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 7696 to 7734
.if(n = 7696)
	; 14 bytes
	sty @zptemp
	ldy #233
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7697 || n = 7699 || n = 7702 || n = 7704 || n = 7708 || n = 7709 || n = 7711 || n = 7714 || n = 7716 || n = 7721 || n = 7729)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 7698 || n = 7703 || n = 7710 || n = 7715)
	; 10..14 bytes
	pha
	delay_n (n-3)
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
	; 11 bytes
	php
	sta @zptemp
	lda #30
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 7707)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7712)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7713 || n = 7718)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7717)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7719)
	; 11 bytes
	php
	pha
	php
	lda #30
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7720 || n = 7725 || n = 7733)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7722 || n = 7727)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7723 || n = 7728)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7724)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7726)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7730)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7731)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7732 || n = 7734)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.else
; 7735 to 7812
.if(n = 7735)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7736)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7737 || n = 7739 || n = 7740 || n = 7753 || n = 7763 || n = 7767)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7738)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7741 || n = 7742 || n = 7744 || n = 7745 || n = 7747 || n = 7749 || n = 7750 || n = 7752 || (n > 7753 && n < 7757) || (n > 7757 && n < 7762) || (n > 7763 && n < 7767) || (n > 7767 && n < 7788) || (n > 7788 && n < 7793) || (n > 7793 && n < 7813))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7743)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7746)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7748)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7751)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7757 || n = 7788)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.if((n > 7812 && n < 7819) || (n > 7819 && n < 7823) || (n > 7824 && n < 7828) || (n > 7828 && n < 7850) || (n > 7850 && n < 7855) || (n > 7855 && n < 7868) || (n > 7868 && n < 7881) || (n > 7881 && n < 7886) || (n > 7886 && n < 7891))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7819 || n = 7850 || n = 7881)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7823)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #30
	jsr delay_256a_16_clocks_b
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
.elseif(n = 7828 || n = 7868)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7901) || n = 7902 || n = 7905 || n = 7907 || n = 7910 || n = 7911)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7901)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 7903 || n = 7908 || n = 7914 || n = 7919 || n = 7921 || n = 7926)
	; 13..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7904 || n = 7909 || n = 7915 || n = 7920)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7906)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk 0
	iny
	Jne *-5
	tay
	pla
.elseif(n = 7912)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7913 || n = 7924)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7916)
	; 14 bytes
	pha
	tya
	ldy #247
	delay_n 27
	dey
	Jne *-6
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
	pha
	tya
	ldy #226
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7923)
	; 15 bytes
	pha
	txa
	ldx #(n-18)/31
	delay_n 26
	dex
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 7925)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7927)
	; 14 bytes
	sty @zptemp
	ldy #240
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7928)
	; 14 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	dey
	Jne *-5
	ldy @zptemp
	plp
.elseif(n = 7929)
	; 14 bytes
	sty @zptemp
	ldy #233
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 7930 to 7968
.if(n = 7930 || n = 7956)
	; 15 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7931)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7932)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7933)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7934 || n = 7936 || n = 7939 || n = 7941 || n = 7945 || n = 7947 || n = 7950 || n = 7958 || n = 7959 || n = 7964 || n = 7965 || n = 7967)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 7935 || n = 7940 || n = 7951 || n = 7966)
	; 10..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 7937)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk 0
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7938)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 7942)
	; 14 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	brk 0
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 7943)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7944 || n = 7955)
	; 14..15 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7946)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts
	brk 0
	dey
	Jne *-7
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
.elseif(n = 7949)
	; 15 bytes
	sty @zptemp
	ldy #209
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7952)
	; 15 bytes
	sty @zptemp
	ldy #227
	delay_n 30
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 7957)
	; 15 bytes
	pha
	tya
	ldy #227
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7960)
	; 14 bytes
	sty @zptemp
	ldy #241
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7961)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 27
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7962)
	; 11 bytes
	php
	sta @zptemp
	lda #31
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 7963)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7968)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_16_clocks_b
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
.elseif(n = 7970 || n = 7972 || n = 7977 || n = 7985 || n = 7991)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 7973)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7975)
	; 11 bytes
	php
	pha
	php
	lda #31
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7976 || n = 7981 || n = 7989 || n = 7994)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 7978 || n = 7983)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 7979 || n = 7984)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 7980)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7982)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7986)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7987)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7988 || n = 7990)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 7992)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7993 || n = 7995 || n = 7996 || n = 8009 || n = 8019 || n = 8023 || n = 8084 || n = 8124)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7997 || n = 7998 || n = 8000 || n = 8001 || n = 8003 || n = 8005 || n = 8006 || n = 8008 || (n > 8009 && n < 8019) || (n > 8019 && n < 8023) || (n > 8023 && n < 8079) || (n > 8079 && n < 8084) || (n > 8084 && n < 8124) || n = 8125)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7999)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8002)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8004)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8007)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8079)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #31
	jsr delay_256a_16_clocks_b
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
.if((n > 8125 && n < 8156) || (n > 8156 && n < 8161) || (n > 8161 && n < 8165))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8156)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 8161)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 8165) && (n < 8184))
; 8165 to 8183
.if((n > 8164 && n < 8169))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8169)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8170)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-265)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8171 || n = 8175)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8172)
	; 14 bytes
	pha
	tya
	ldy #255
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8173)
	; 16 bytes
	php
	pha
	php
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8174)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 8176)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8177)
	; 15 bytes
	sty @zptemp
	ldy #215
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8178)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8179)
	; 15 bytes
	sty @zptemp
	ldy #(n-7)/36
	nop
	delay_n 29
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8180)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8181)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
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
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8185)
	; 16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8186 || n = 8190 || n = 8198)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8187)
	; 16 bytes
	stx @zptemp
	pha
	ldx #221
	delay_n 32
	dex
	Jne *-7
	ldx @zptemp
.elseif(n = 8188)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8189)
	; 13 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8191)
	; 14 bytes
	sty @zptemp
	ldy #248
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8192 || n = 8197 || n = 8203)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8193)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8194)
	; 13 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8195)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #241
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8202)
	; 15 bytes
	pha
	tya
	ldy #234
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
; 8204 to 8242
.if(n = 8204)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8205)
	; 16 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8206)
	; 15 bytes
	pha
	tya
	ldy #241
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8207 || n = 8209 || n = 8214 || n = 8232 || n = 8237)
	; 11..16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8208 || n = 8217 || n = 8220 || n = 8221 || n = 8223 || n = 8226 || n = 8228 || n = 8233 || n = 8241)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8210)
	; 16 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	delay_n 27
	iny
	Jne *-9
	tay
	pla
.elseif(n = 8211)
	; 16 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 8213)
	; 15 bytes
	sty @zptemp
	ldy #216 ;hides 'cld'
	nop
	delay_n 29
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 8215)
	; 15 bytes
	sty @zptemp
	ldy #216
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8216 || n = 8222 || n = 8227)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8218)
	; 11 bytes
	php
	sta @zptemp
	lda #32
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 8219)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8225 || n = 8230)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8229)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8231)
	; 11 bytes
	php
	pha
	php
	lda #32
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8234 || n = 8239)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8235 || n = 8240)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8236)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8238)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8242)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #32
	jsr delay_256a_16_clocks_b
	pla
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
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8244 || n = 8246)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8245 || n = 8250)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8247)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8248)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8249 || n = 8251 || n = 8252 || n = 8265 || n = 8275 || n = 8279)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8253 || n = 8254 || n = 8256 || n = 8257 || n = 8259 || n = 8261 || n = 8262 || n = 8264 || (n > 8265 && n < 8275) || (n > 8275 && n < 8279) || n = 8280 || n = 8281)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8255)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8258)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8260)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8263)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8335) || (n > 8335 && n < 8340) || (n > 8340 && n < 8380) || (n > 8380 && n < 8411) || (n > 8411 && n < 8416) || (n > 8416 && n < 8425))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8335)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8340 || n = 8380)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8411)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	brk 0
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 8416)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	brk 0
	iny
	Jne *-6
	tay
	pla
.elseif(n = 8425)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-265)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8426)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks_b
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
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 29
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8430 || n = 8432)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8431)
	; 15 bytes
	sty @zptemp
	ldy #(n-7)/36
	nop
	delay_n 29
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8433)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8434)
	; 15 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8435)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8436)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8437)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
.if((n >= 8438) && (n < 8457))
; 8438 to 8456
.if(n = 8438 || n = 8441 || n = 8446 || n = 8451)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8439)
	; 14 bytes
	sty @zptemp
	ldy #248
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8440)
	; 15 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	dey
	Jne *-6
	ldy @zptemp
	plp
.elseif(n = 8442)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8443)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8444)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8445)
	; 15 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	nop
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 8447 || n = 8452)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 8448)
	; 15 bytes
	pha
	tya
	ldy #222
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8449)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8450)
	; 15 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk 0
	dey
	Jne *-6
	ldy @zptemp
	plp
.elseif(n = 8453)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk 0
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8454)
	; 15 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	brk 0
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 8455)
	; 14 bytes
	sty @zptemp
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8456)
	; 15 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 8457 to 8476
.if(n = 8457 || n = 8459 || n = 8462 || n = 8464 || n = 8469 || n = 8476)
	; 12..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8458 || n = 8463)
	; 15 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 8461)
	; 16 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
	txa
	ldx #0
	nop
	delay_n 26
	dex
	Jne *-6
	inc @zptemp,x
	tax
	pla
.elseif(n = 8467)
	; 15 bytes
	sty @zptemp
	ldy #(n-7)/36
	nop
	delay_n 29
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8468)
	; 16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8470)
	; 16 bytes
	sty @zptemp
	ldy #217
	delay_n 7
	delay_n 27
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 8471)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 8472)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8473)
	; 14 bytes
	sty @zptemp
	ldy #249
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8474)
	; 11 bytes
	php
	sta @zptemp
	lda #33
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 8475)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8479 || n = 8482 || n = 8484 || n = 8489 || n = 8497 || n = 8503)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8481 || n = 8486)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8485)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8487)
	; 11 bytes
	php
	pha
	php
	lda #33
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8488 || n = 8493 || n = 8501 || n = 8506)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8490 || n = 8495)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8491 || n = 8496)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8492)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8494)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8498)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8499)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8500 || n = 8502)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8504)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8505 || n = 8507 || n = 8508)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8509 || n = 8510 || n = 8512 || n = 8513 || n = 8515)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8511)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8514)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 8516 to 8593
.if(n = 8516)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8517 || n = 8518 || n = 8520 || (n > 8521 && n < 8531) || (n > 8531 && n < 8535) || (n > 8535 && n < 8591) || n = 8592 || n = 8593)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8519)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8521 || n = 8531 || n = 8535)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8591)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if(n = 8594 || n = 8595 || (n > 8596 && n < 8636) || (n > 8636 && n < 8666) || (n > 8666 && n < 8672))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8596 || n = 8636)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8666)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 16
	iny
	Jne *-6
	ldy @zptemp
.endif
.else
.if((n >= 8672) && (n < 8711))
; 8672 to 8710
.if((n > 8671 && n < 8681))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8681)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-266)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8682)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8683 || n = 8693 || n = 8703 || n = 8708)
	; 14..15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8684)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8685)
	; 14 bytes
	sty @zptemp
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8686)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8687)
	; 15 bytes
	sty @zptemp
	ldy #248
	delay_n 30
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8688)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8689)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8690)
	; 14 bytes
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8691)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8692)
	; 15 bytes
	pha
	tya
	ldy #248
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8694 || n = 8702 || n = 8704 || n = 8707)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8695)
	; 16 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	nop
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8696)
	; 16 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	bit $24
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 8697)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8698)
	; 16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8699)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	bit $24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8700)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8701)
	; 16 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	cmp $C5
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8705)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	sta @zptemp
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8706)
	; 15 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8709)
	; 15 bytes
	sty @zptemp
	ldy #229
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8710)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	bit $24
	brk 0
	sbc #1
	Jne *-8
	pla
	plp
.endif
.else
; 8711 to 8750
.if(n = 8711)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 29
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8712)
	; 16 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	brk 0
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8713 || n = 8715 || n = 8718 || n = 8720 || n = 8721 || n = 8726 || n = 8728 || n = 8729 || n = 8732 || n = 8733 || n = 8735 || n = 8738 || n = 8740 || n = 8745)
	; 10..17 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8716)
	; 15 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8717 || n = 8725 || n = 8734 || n = 8739)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8719)
	; 15 bytes
	sty @zptemp
	ldy #198
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8722)
	; 15 bytes
	sty @zptemp
	ldy #249
	delay_n 30
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8723)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8724)
	; 15 bytes
	pha
	tya
	ldy #198
	delay_n 39
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8727)
	; 15 bytes
	pha
	tya
	ldy #249
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8730)
	; 11 bytes
	php
	sta @zptemp
	lda #34
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 8731)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8736)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8737 || n = 8742)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8741)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8743)
	; 11 bytes
	php
	pha
	php
	lda #34
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8744 || n = 8749)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8746)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8747)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8748)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8750)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.if(n = 8751)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 8752)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 8753 || n = 8759)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8754)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8755)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8756 || n = 8758)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 8757 || n = 8762)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8760)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8761 || n = 8763 || n = 8764 || n = 8777 || n = 8787 || n = 8791 || n = 8852 || n = 8892)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8765 || n = 8766 || n = 8768 || n = 8769 || n = 8771 || n = 8773 || n = 8774 || n = 8776 || (n > 8777 && n < 8787) || (n > 8787 && n < 8791) || (n > 8791 && n < 8847) || (n > 8847 && n < 8852) || (n > 8852 && n < 8892) || (n > 8892 && n < 8907))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8767)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8770)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8772)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8775)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8847)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #34
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8937)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	ldx @zptemp
.elseif(n = 8938)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks_b
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
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8941)
	; 16 bytes
	php
	pha
	php
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
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
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
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
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8947)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8948)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8949)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8950)
	; 16 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 8951)
	; 16 bytes
	sty @zptemp
	ldy #208
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 8952)
	; 16 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 4
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 8953)
	; 17 bytes
	php
	pha
	sty @zptemp
	ldy #203
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	pla
	plp
.elseif(n = 8954)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 152
	Jmi *-10
	ldy @zptemp
.elseif(n = 8955)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 4
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8956)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8957)
	; 16 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	delay_n 4
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8958 || n = 8960 || n = 8963)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8959 || n = 8964)
	; 15 bytes
	pha
	delay_n (n-3)
.elseif(n = 8961)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8962)
	; 16 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk 0
	dey
	Jne *-7
	ldy @zptemp
	plp
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
	brk 0
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8966)
	; 16 bytes
	pha
	tya
	ldy #242
	nop
	delay_n 30
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8967)
	; 15 bytes
	sty @zptemp
	ldy #0
	delay_n 30
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8968)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 8969 || n = 8973 || n = 8974 || n = 8978)
	; 16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8970 || n = 8979)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 8971)
	; 15 bytes
	sty @zptemp
	ldy #(n-7)/36
	nop
	delay_n 29
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8972)
	; 15 bytes
	pha
	tya
	ldy #0
	delay_n 30
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8975)
	; 15 bytes
	sty @zptemp
	ldy #236
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8976)
	; 15 bytes
	pha
	tya
	ldy #(n-12)/36
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8977)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
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
	php
	pha
	sec
	lda #249
	inc @zptemp
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8982)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 8983)
	; 15 bytes
	sty @zptemp
	ldy #204
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8984)
	; 16 bytes
	sty @zptemp
	ldy #191
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.endif
.endif
.endif
.else
; 8985 to 9062
.if(n = 8985 || n = 8988 || n = 8989 || n = 8991 || n = 8994 || n = 8996 || n = 9001 || n = 9009 || n = 9015)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 8986)
	; 11 bytes
	php
	sta @zptemp
	lda #35
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 8987)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8990 || n = 8995)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 8992)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8993 || n = 8998)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 8997)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8999)
	; 11 bytes
	php
	pha
	php
	lda #35
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9000 || n = 9005 || n = 9013 || n = 9018)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9002 || n = 9007)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9003 || n = 9008)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9004)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9006)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9010)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9011)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9012 || n = 9014)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9016)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9017 || n = 9019 || n = 9020 || n = 9033 || n = 9043 || n = 9047)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9021 || n = 9022 || n = 9024 || n = 9025 || n = 9027 || n = 9029 || n = 9030 || n = 9032 || (n > 9033 && n < 9043) || (n > 9043 && n < 9047) || (n > 9047 && n < 9063))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9023)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9026)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9028)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9031)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
; 9063 to 9140
.if((n > 9062 && n < 9103) || (n > 9103 && n < 9108) || (n > 9108 && n < 9141))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9103)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9108)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if((n > 9140 && n < 9148) || (n > 9148 && n < 9180))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9148)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 9180 to 9218
.if((n > 9179 && n < 9193))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9193)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-268)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9194)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9195 || n = 9206 || n = 9215)
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9198)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9199)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9200)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9201)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9202)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9203)
	; 15 bytes
	sty @zptemp
	ldy #209
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9204)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9205)
	; 15 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 39
	iny
	Jne *-8
	ldy @zptemp
.elseif(n = 9207)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
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
.elseif(n = 9209)
	; 16 bytes
	sty @zptemp
	ldy #214
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
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
.elseif(n = 9211)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	inc @zptemp
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9212)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 16
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9213)
	; 16 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	inc @zptemp
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 9214)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 9216)
	; 16 bytes
	pha
	tya
	ldy #236
	delay_n 7
	delay_n 27
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9217)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc @zptemp
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9218)
	; 16 bytes
	delay_n 6
	delay_n (n-6)
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
; 9219 to 9257
.if(n = 9219 || n = 9221 || n = 9227 || n = 9230 || n = 9244 || n = 9245 || n = 9247 || n = 9250 || n = 9252 || n = 9257)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9220)
	; 15 bytes
	sty @zptemp
	ldy #249
	delay_n 32
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9222)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	nop
	inc @zptemp
	brk 0
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9223)
	; 15 bytes
	sty @zptemp
	ldy #0
	nop
	delay_n 29
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9224)
	; 16 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9225)
	; 16 bytes
	pha
	tya
	ldy #249
	nop
	delay_n 30
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9226 || n = 9231 || n = 9240 || n = 9246 || n = 9251)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 9228)
	; 15 bytes
	pha
	tya
	ldy #0
	inc @zptemp
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9229)
	; 17 bytes
	sty @zptemp
	ldy #159
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 9232)
	; 16 bytes
	sty @zptemp
	ldy #205
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9233)
	; 17 bytes
	php
	pha
	sec
	lda #0
	inc @zptemp
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9234)
	; 17 bytes
	pha
	tya
	ldy #159
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 9235)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 9236)
	; 17 bytes
	pha
	sty @zptemp
	ldy #159
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 9237)
	; 16 bytes
	pha
	tya
	ldy #205
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9238)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
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
.elseif(n = 9241)
	; 15 bytes
	sty @zptemp
	ldy #243
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9242)
	; 11 bytes
	php
	sta @zptemp
	lda #36
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 9243)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9248)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9249 || n = 9254)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9253)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9255)
	; 11 bytes
	php
	pha
	php
	lda #36
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9256)
	; 11 bytes
	delay_n (n-13)
	brk 0
.endif
.else
; 9258 to 9296
.if(n = 9258 || n = 9263)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9259 || n = 9264)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9260)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9261 || n = 9269 || n = 9274)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9262)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9265 || n = 9271)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 9266)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9267)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9268 || n = 9270)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9272)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9273 || n = 9275 || n = 9276 || n = 9289)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9277 || n = 9278 || n = 9280 || n = 9281 || n = 9283 || n = 9285 || n = 9286 || n = 9288 || (n > 9289 && n < 9297))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9279)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9282)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9284)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9287)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 9297 to 9375
.if(n = 9297 || n = 9298 || (n > 9299 && n < 9303) || (n > 9303 && n < 9359) || (n > 9359 && n < 9364) || (n > 9364 && n < 9376))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9299 || n = 9303 || n = 9364)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9359)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #36
	jsr delay_256a_16_clocks_b
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
.if((n > 9375 && n < 9404) || (n > 9404 && n < 9449))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9404)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9449)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-269)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9450)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
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
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9455)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9456)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9457)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9458)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9459)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9460)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9461)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9462)
	; 16 bytes
	pha
	tya
	ldy #210
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9463)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 64
	Jne *-10
	tay
	pla
.elseif(n = 9464 || n = 9471)
	; 15..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 9465)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #28
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
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
	; 15 bytes
	sty @zptemp
	ldy #215
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9468)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 19
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9469)
	; 15 bytes
	sty @zptemp
	ldy #249
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9470)
	; 15 bytes
	delay_n (n-2)
	nop
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
	brk 0
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
.elseif(n = 9475 || n = 9476 || n = 9481 || n = 9490 || n = 9491)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9477 || n = 9482)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9478)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/41
	delay_n 10
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9479)
	; 15 bytes
	sty @zptemp
	ldy #0
	delay_n 32
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9480)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 9483)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9484)
	; 16 bytes
	pha
	tya
	ldy #0
	nop
	delay_n 30
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9485)
	; 17 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9486)
	; 17 bytes
	php
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
	plp
.elseif(n = 9487)
	; 16 bytes
	sty @zptemp
	ldy #237
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9488)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9489)
	; 16 bytes
	pha
	tya
	ldy #243
	delay_n 7
	delay_n 27
	dey
	Jne *-8
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
.if(n = 9493)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 9494 || n = 9500 || n = 9501 || n = 9503 || n = 9506 || n = 9508 || n = 9513 || n = 9521 || n = 9527)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9495 || n = 9502 || n = 9507)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 9496)
	; 18 bytes
	php
	pha
	tya
	ldy #81
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9497)
	; 17 bytes
	sty @zptemp
	ldy #146
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 9498)
	; 11 bytes
	php
	sta @zptemp
	lda #37
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 9499)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9504)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9505 || n = 9510)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9509)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9511)
	; 11 bytes
	php
	pha
	php
	lda #37
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9512 || n = 9517 || n = 9525 || n = 9530)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9514 || n = 9519)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9515 || n = 9520)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9516)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9518)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9522)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9523)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9524 || n = 9526)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9528)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9529 || n = 9531)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.else
; 9532 to 9687
.if(n = 9532 || n = 9545 || n = 9555 || n = 9559 || n = 9620 || n = 9660)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9533 || n = 9534 || n = 9536 || n = 9537 || n = 9539 || n = 9541 || n = 9542 || n = 9544 || (n > 9545 && n < 9555) || (n > 9555 && n < 9559) || (n > 9559 && n < 9615) || (n > 9615 && n < 9620) || (n > 9620 && n < 9660) || (n > 9660 && n < 9688))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9535)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9538)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9540)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9543)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9615)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #37
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9705)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-270)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9706)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9707)
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9710)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9711)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9712)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9713)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9714)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9715)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9716)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9717)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9718)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9719)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9720)
	; 16 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 7
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 9721)
	; 17 bytes
	pha
	tya
	pha
	ldy #13
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 9722 || n = 9726)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9723)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 7
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9724)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9725)
	; 16 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.else
; 9727 to 9765
.if(n = 9727 || n = 9732 || n = 9739 || n = 9743 || n = 9758 || n = 9763)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 9728 || n = 9731 || n = 9733 || n = 9737 || n = 9738 || n = 9742 || n = 9756 || n = 9757 || n = 9759 || n = 9762 || n = 9764)
	; 10..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 9729)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9730)
	; 16 bytes
	pha
	tya
	ldy #226
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9734)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 9
	brk 0
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9735)
	; 15 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
	php
	pha
	sec
	lda #221
	delay_n 39
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 9744)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #30
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 9745)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 9746)
	; 17 bytes
	pha
	txa
	ldx #0
	delay_n 7
	delay_n 26
	dex
	Jne *-7
	inc @zptemp,x
	tax
	pla
.elseif(n = 9747)
	; 17 bytes
	pha
	tya
	ldy #32
	delay_n 290
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 9748 || n = 9753)
	; 17 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9749)
	; 16 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 39
	Jne *-9
	ldx @zptemp
.elseif(n = 9750)
	; 17 bytes
	php
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
	plp
.elseif(n = 9751)
	; 17 bytes
	sty @zptemp
	ldy #168
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 9752)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 166
	Jmi *-10
	ldy @zptemp
.elseif(n = 9754)
	; 11 bytes
	php
	sta @zptemp
	lda #38
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 9755)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9760)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9761)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 9765)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9768 || n = 9773 || n = 9781 || n = 9786)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 9769 || n = 9777 || n = 9783)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 9770 || n = 9775)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 9771 || n = 9776)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 9772)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9774)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9778)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9779)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9780 || n = 9782)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 9784)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9785 || n = 9787 || n = 9788 || n = 9801 || n = 9811 || n = 9815)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9789 || n = 9790 || n = 9792 || n = 9793 || n = 9795 || n = 9797 || n = 9798 || n = 9800 || (n > 9801 && n < 9811) || (n > 9811 && n < 9815) || (n > 9815 && n < 9844))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9791)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9794)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9796)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9799)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 9844) && (n < 9922))
; 9844 to 9921
.if((n > 9843 && n < 9871) || (n > 9871 && n < 9876) || (n > 9876 && n < 9916) || (n > 9916 && n < 9922))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9871)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9876 || n = 9916)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 9961) && (n < 9981))
; 9961 to 9980
.if(n = 9961)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-271)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9962)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9963 || n = 9976)
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9966)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9967)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9968)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9969)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9970)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9971)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9972)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9973)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9974)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9975)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9977)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9978)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #201
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 9979)
	; 18 bytes
	php
	pha
	tya
	ldy #82
	pha
	lda #79
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 9980)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	ldy @zptemp
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 16 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 10
	delay_n 26
	iny
	Jne *-9
	ldy @zptemp
.elseif(n = 9982 || n = 9984 || n = 9987 || n = 9993 || n = 9997 || n = 9999)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 9983)
	; 16 bytes
	sty @zptemp
	ldy #232
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9985)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9986)
	; 17 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	iny
	Jne *-10
	tay
	pla
.elseif(n = 9988)
	; 16 bytes
	pha
	tya
	ldy #232
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9989)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9990)
	; 17 bytes
	pha
	sty @zptemp
	ldy #172
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 9991)
	; 16 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	delay_n 27
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 9992)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #134
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 9994)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9995)
	; 15 bytes
	sty @zptemp
	ldy #227
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9996)
	; 16 bytes
	pha
	tya
	ldy #0
	delay_n 7
	delay_n 27
	dey
	Jne *-8
	tay
	pla
.elseif(n = 9998)
	; 16 bytes
	pha
	delay_n (n-3)
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
.if((n >= 10001) && (n < 10079))
.if((n >= 10001) && (n < 10040))
; 10001 to 10039
.if(n = 10001 || n = 10017 || n = 10022)
	; 11..17 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10002 || n = 10004 || n = 10012 || n = 10013 || n = 10015 || n = 10018 || n = 10020 || n = 10025 || n = 10033 || n = 10039)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10003 || n = 10014 || n = 10019)
	; 10..16 bytes
	pha
	delay_n (n-3)
.elseif(n = 10005)
	; 17 bytes
	php
	pha
	sec
	lda #227
	delay_n 39
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 10006)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 68
	Jne *-10
	ldy @zptemp
.elseif(n = 10007)
	; 16 bytes
	sty @zptemp
	ldy #250
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10008)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #185
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 10009)
	; 17 bytes
	php
	pha
	sty @zptemp
	ldy #227
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	pla
	plp
.elseif(n = 10010)
	; 11 bytes
	php
	sta @zptemp
	lda #39
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 10011)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10016)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10021)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10023)
	; 11 bytes
	php
	pha
	php
	lda #39
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10024 || n = 10029 || n = 10037)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10026 || n = 10031)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10027 || n = 10032)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10028)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10030)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10034)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10035)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10036 || n = 10038)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.endif
.else
; 10040 to 10078
.if(n = 10040)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10041 || n = 10043 || n = 10044 || n = 10057 || n = 10067 || n = 10071)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10042)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10045 || n = 10046 || n = 10048 || n = 10049 || n = 10051 || n = 10053 || n = 10054 || n = 10056 || (n > 10057 && n < 10067) || (n > 10067 && n < 10071) || (n > 10071 && n < 10079))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10047)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10050)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10052)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10055)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 10079 to 10156
.if((n > 10078 && n < 10127) || (n > 10127 && n < 10132) || (n > 10132 && n < 10157))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10127)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10132)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10172) || (n > 10172 && n < 10217))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10172)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10217)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-272)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10218)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10219 || n = 10232)
	; 15..17 bytes
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10222)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10223)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10224)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10225)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10226)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10227)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10228)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10229)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10230)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10231)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10233)
	; 17 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10234)
	; 17 bytes
	php
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
	plp
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
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	delay_n 9
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 10236)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 11
	brk 0
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10237)
	; 17 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	delay_n 9
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10238 || n = 10240 || n = 10243)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10239 || n = 10244 || n = 10250)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10241)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 11
	brk 0
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10242)
	; 17 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	brk 0
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 10245)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	brk 0
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 10246)
	; 16 bytes
	pha
	tya
	ldy #238
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10247)
	; 16 bytes
	sty @zptemp
	ldy #0
	delay_n 9
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10248)
	; 17 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 10251)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #93
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
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
	; 16 bytes
	sty @zptemp
	ldy #218
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.endif
.else
; 10254 to 10273
.if(n = 10254 || n = 10261 || n = 10268 || n = 10269 || n = 10271)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10255)
	; 17 bytes
	pha
	sty @zptemp
	ldy #76
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 10256 || n = 10262 || n = 10270)
	; 10..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10257)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/41
	delay_n 10
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10258)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 10259)
	; 15 bytes
	sty @zptemp
	ldy #233
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 10263)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
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
.elseif(n = 10265)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10266)
	; 11 bytes
	php
	sta @zptemp
	lda #40
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 10267)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10272)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_16_clocks_b
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
.if(n = 10274 || n = 10276 || n = 10281 || n = 10289 || n = 10295)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10277)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10280 || n = 10285 || n = 10293 || n = 10298)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10282 || n = 10287)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10283 || n = 10288)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10284)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10286)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10290)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10291)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10292 || n = 10294)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10296)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10297 || n = 10299 || n = 10300)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10301 || n = 10302 || n = 10304 || n = 10305 || n = 10307 || n = 10309 || n = 10310 || n = 10312)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10303)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10306)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10308)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10311)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 10313) && (n < 10469))
; 10313 to 10468
.if(n = 10313 || n = 10323 || n = 10327 || n = 10388 || n = 10428)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10313 && n < 10323) || (n > 10323 && n < 10327) || (n > 10327 && n < 10383) || (n > 10383 && n < 10388) || (n > 10388 && n < 10428) || (n > 10428 && n < 10469))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10383)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #40
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10473)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-273)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10474)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10478)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10479)
	; 15 bytes
	sty @zptemp
	ldy #238
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 10480)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10481)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10482)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10483)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
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
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10486)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
; 10488 to 10507
.if(n = 10488)
	; 16 bytes
	sty @zptemp
	ldy #223
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
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
	sty @zptemp
	ldy #27
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 10491)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10492)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 10
	brk 0
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10493)
	; 17 bytes
	php
	pha
	sty @zptemp
	ldy #238
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	pla
	plp
.elseif(n = 10494 || n = 10496 || n = 10501 || n = 10506)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 10495)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10497)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	php
	cmp $C5
	plp
	brk 0
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10498)
	; 17 bytes
	php
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
	plp
.elseif(n = 10499)
	; 16 bytes
	sty @zptemp
	ldy #244
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10500)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10502)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #4
	jsr delay_256a_11_clocks_b_
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 10503)
	; 16 bytes
	sty @zptemp
	ldy #0
	delay_n 10
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10504)
	; 16 bytes
	pha
	tya
	ldy #244
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10505)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #97
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 10507)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #4
	jsr delay_256a_11_clocks_b_
	plp
	Jmi *-8
	pla
	tay
	pla
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	jsr @nearby_rts_15cyc
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10509)
	; 17 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1307
	Jne *-10
	pla
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
	; 18 bytes
	php
	pha
	tya
	ldy #244
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 10514)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #228
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10515)
	; 18 bytes
	php
	pha
	tya
	ldy #82
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
.elseif(n = 10516)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #86
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
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
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10519)
	; 17 bytes
	sty @zptemp
	ldy #219
	delay_n 4
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 10520)
	; 17 bytes
	php
	sty @zptemp
	ldy #(n-14)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 10521)
	; 17 bytes
	pha
	sty @zptemp
	ldy #79
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 10522)
	; 11 bytes
	php
	sta @zptemp
	lda #41
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 10523)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10524 || n = 10525)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10526)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.else
; 10527 to 10546
.if(n = 10527 || n = 10530 || n = 10532 || n = 10537 || n = 10545)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 10528)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10529 || n = 10534)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10531)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 10533)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10535)
	; 11 bytes
	php
	pha
	php
	lda #41
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10536 || n = 10541)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10538 || n = 10543)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10539 || n = 10544)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10540)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10542)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10546)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
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
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10548 || n = 10550)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10549 || n = 10554)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10551)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10552)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10553 || n = 10555 || n = 10556 || n = 10569 || n = 10579 || n = 10583)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10557 || n = 10558 || n = 10560 || n = 10561 || n = 10563 || n = 10565 || n = 10566 || n = 10568 || (n > 10569 && n < 10579) || (n > 10579 && n < 10583) || (n > 10583 && n < 10626))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10559)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10562)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10564)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10567)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.if((n > 10625 && n < 10639) || (n > 10639 && n < 10644) || (n > 10644 && n < 10684) || (n > 10684 && n < 10704))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10639)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10644 || n = 10684)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10729)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-274)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10730)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10734)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10735)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10736)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10737)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10738)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10739)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10740)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10741)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10742)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 10743) && (n < 10762))
; 10743 to 10761
.if(n = 10743)
	; 15 bytes
	sty @zptemp
	ldy #244
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 10744 || n = 10746 || n = 10751 || n = 10760)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10745 || n = 10747 || n = 10750)
	; 16..17 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 10752)
	; 17 bytes
	pha
	tya
	ldy #179
	delay_n 55
	dey
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
	brk 0
	dey
	Jne *-9
	tay
	pla
.elseif(n = 10754)
	; 17 bytes
	pha
	txa
	ldx #244
	delay_n 39
	dex
	Jne *-7
	inc @zptemp,x
	tax
	pla
.elseif(n = 10755)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 10756 || n = 10761)
	; 17 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10757)
	; 16 bytes
	sty @zptemp
	ldy #250
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10758)
	; 18 bytes
	php
	sty @zptemp
	ldy #79
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 10759)
	; 17 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	delay_n 30
	dey
	Jne *-9
	ldy @zptemp
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 16 bytes
	pha
	tya
	ldy #250
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10763)
	; 18 bytes
	php
	pha
	tya
	ldy #79
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10764)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #199
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 10765 || n = 10772 || n = 10777)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 10766)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10767)
	; 16 bytes
	pha
	tya
	ldy #239
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10768)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 10769)
	; 15 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10770)
	; 16 bytes
	sty @zptemp
	ldy #229
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10771)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 10773)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
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
.elseif(n = 10775)
	; 17 bytes
	pha
	txa
	pha
	tya
	ldy #27
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	tay
	pla
	tax
	pla
.elseif(n = 10776)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 10778)
	; 11 bytes
	php
	sta @zptemp
	lda #42
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 10779)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10780 || n = 10781)
	; 10..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 10783 || n = 10786 || n = 10788 || n = 10793 || n = 10801 || n = 10807)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10784)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10785 || n = 10790)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 10789)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10791)
	; 11 bytes
	php
	pha
	php
	lda #42
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10792 || n = 10797 || n = 10805 || n = 10810)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 10794 || n = 10799)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 10795 || n = 10800)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 10796)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10798)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10802)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10803)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10804 || n = 10806)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 10808)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10809 || n = 10811 || n = 10812 || n = 10825 || n = 10835 || n = 10839 || n = 10900)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10813 || n = 10814 || n = 10816 || n = 10817 || n = 10819 || n = 10821 || n = 10822 || n = 10824 || (n > 10825 && n < 10835) || (n > 10835 && n < 10839) || (n > 10839 && n < 10895) || (n > 10895 && n < 10900) || (n > 10900 && n < 10938))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10815)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10818)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10820)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10823)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10895)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if(n = 10938 || n = 10939 || (n > 10940 && n < 10977))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10940)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10985)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-275)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10986)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10990)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10991)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10992)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10993)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10994)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10995)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10997)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10998)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10999)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11000 || n = 11010)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 11001)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11002)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11003)
	; 16 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 11004)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	brk 0
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11005)
	; 16 bytes
	sty @zptemp
	ldy #234
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11006)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11007)
	; 15 bytes
	sty @zptemp
	ldy #250
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 11008)
	; 16 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11009)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	brk 0
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11011 || n = 11014)
	; 16 bytes
	delay_n (n-2)
	nop
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #152
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 11015)
	; 16 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
.if(n = 11016 || n = 11022)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11017)
	; 17 bytes
	php
	pha
	sec
	lda #250
	delay_n 39
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 11018)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #152
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 11019)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11020)
	; 16 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11021)
	; 17 bytes
	php
	pha
	sty @zptemp
	ldy #250
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	pla
	plp
.elseif(n = 11023)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 11024)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #83
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 11025)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 140
	Jmi *-10
	ldx @zptemp
.elseif(n = 11026)
	; 17 bytes
	php
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
	plp
.elseif(n = 11027)
	; 17 bytes
	sty @zptemp
	ldy #190
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 11028)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.elseif(n = 11029)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 11030)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 140
	Jmi *-10
	tax
	pla
.elseif(n = 11031)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #155
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11032)
	; 16 bytes
	sty @zptemp
	ldy #245
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11033)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #110
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 11034)
	; 11 bytes
	php
	sta @zptemp
	lda #43
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.endif
.else
; 11035 to 11054
.if(n = 11035)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11036 || n = 11037 || n = 11039 || n = 11042 || n = 11044 || n = 11049)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11038 || n = 11043)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11041 || n = 11046)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11045)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11047)
	; 11 bytes
	php
	pha
	php
	lda #43
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11048 || n = 11053)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11050)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11051)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11052)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11054)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 11055 to 11093
.if(n = 11055)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11056)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11057 || n = 11063)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 11058)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11059)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11060 || n = 11062)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11061 || n = 11066)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11064)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11065 || n = 11067 || n = 11068 || n = 11081 || n = 11091)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11069 || n = 11070 || n = 11072 || n = 11073 || n = 11075 || n = 11077 || n = 11078 || n = 11080 || (n > 11081 && n < 11091) || n = 11092 || n = 11093)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11071)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11074)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11076)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11079)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 11094 to 11250
.if(n = 11094 || (n > 11095 && n < 11151) || (n > 11151 && n < 11156) || (n > 11156 && n < 11196) || (n > 11196 && n < 11241))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11095 || n = 11156 || n = 11196)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11151)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11241)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-276)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11242)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11246)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11247)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11248)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11249)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11250)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
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
.if((n >= 11251) && (n < 11270))
; 11251 to 11269
.if(n = 11251)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11252)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11253)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11254)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11255)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11256)
	; 16 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 11257)
	; 16 bytes
	sty @zptemp
	ldy #250
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11258)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 143
	Jmi *-10
	tax
	pla
.elseif(n = 11259)
	; 16 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	brk 0
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 11260)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 11261)
	; 16 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	brk 0
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 11262 || n = 11264 || n = 11267 || n = 11269)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 11263 || n = 11268)
	; 15 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 11266)
	; 16 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 11270 to 11289
.if(n = 11270)
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
	; 15 bytes
	sty @zptemp
	ldy #0
	delay_n 39
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 11272 || n = 11283)
	; 16..17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11273 || n = 11275 || n = 11278 || n = 11280 || n = 11284)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11274 || n = 11279)
	; 16 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 11277)
	; 16 bytes
	sty @zptemp
	ldy #(n-7)/46
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
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
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11285)
	; 17 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	pla
	plp
.elseif(n = 11286)
	; 18 bytes
	pha
	txa
	sty @zptemp
	ldx #17
	ldy #123
	dey
	Jpl *-1
	dex
	Jpl *-6
	ldy @zptemp
	tax
	pla
.elseif(n = 11287)
	; 16 bytes
	sty @zptemp
	ldy #240
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11288)
	; 18 bytes
	php
	pha
	tya
	ldy #191
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11289)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #38
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	ldy @zptemp
	tax
	pla
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 11 bytes
	php
	sta @zptemp
	lda #44
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 11291)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11292 || n = 11293 || n = 11295 || n = 11298 || n = 11300 || n = 11305 || n = 11313 || n = 11319)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11297 || n = 11302)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11301)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11303)
	; 11 bytes
	php
	pha
	php
	lda #44
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11304 || n = 11309 || n = 11317 || n = 11322)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11306 || n = 11311)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11307 || n = 11312)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11308)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11310)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11314)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11315)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11316 || n = 11318)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11320)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11321 || n = 11323 || n = 11324)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11325 || n = 11326 || n = 11328)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11327)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 11329 to 11406
.if(n = 11329 || n = 11331 || n = 11333 || n = 11334 || n = 11336 || (n > 11337 && n < 11347) || (n > 11347 && n < 11351) || (n > 11351 && n < 11407))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11330)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11332)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11335)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11337 || n = 11347 || n = 11351)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if(n = 11407)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 11407 && n < 11412) || (n > 11412 && n < 11452) || (n > 11452 && n < 11485))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11412 || n = 11452)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11497)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-277)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11498)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11499 || n = 11519)
	; 15..16 bytes
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11502)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11503)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11504)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11505)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11506)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11507)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11508)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11509)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11510)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11511)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11512)
	; 16 bytes
	pha
	tya
	ldy #(n-12)/46
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11513)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 79
	Jne *-10
	ldy @zptemp
.elseif(n = 11514 || n = 11518 || n = 11523)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11515)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 11516)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 27
	dey
	Jne *-8
	ldy @zptemp
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
.elseif(n = 11520)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #213
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 11521)
	; 15 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 27
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11522)
	; 16 bytes
	sty @zptemp
	ldy #245
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524 || n = 11530 || n = 11535)
	; 16..17 bytes
	pha
	delay_n (n-3)
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
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #161
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 11527)
	; 16 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11528)
	; 17 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 11529 || n = 11534)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 11531)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #161
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 11532)
	; 16 bytes
	pha
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11533)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 11536)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #245
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 11537)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #117
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 11538)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #29
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 11539)
	; 18 bytes
	php
	pha
	tya
	ldy #90
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
.elseif(n = 11540)
	; 17 bytes
	php
	sty @zptemp
	ldy #(n-14)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 11541)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #245
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11542)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #124
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 11545)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 11546)
	; 11 bytes
	php
	sta @zptemp
	lda #45
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 11547)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11548 || n = 11549 || n = 11551 || n = 11554 || n = 11556 || n = 11561)
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11553 || n = 11558)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11557)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11559)
	; 11 bytes
	php
	pha
	php
	lda #45
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11560)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11562)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 11565 || n = 11573 || n = 11578)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11566)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11567)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11569 || n = 11575)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 11570)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11571)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11572 || n = 11574)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11576)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11577 || n = 11579 || n = 11580 || n = 11593 || n = 11603 || n = 11607 || n = 11668 || n = 11708)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11581 || n = 11582 || n = 11584 || n = 11585 || n = 11587 || n = 11589 || n = 11590 || n = 11592 || (n > 11593 && n < 11603) || (n > 11603 && n < 11607) || (n > 11607 && n < 11663) || (n > 11663 && n < 11668) || (n > 11668 && n < 11708) || (n > 11708 && n < 11719))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11583)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11586)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11588)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11591)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11663)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #45
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11753)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-278)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11754)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
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
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11759)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11760)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11761)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11762)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11763)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11764)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11765)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11766)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11767)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11768 || n = 11775)
	; 16..17 bytes
	pha
	delay_n (n-3)
.elseif(n = 11769)
	; 18 bytes
	php
	pha
	tya
	ldy #250
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11770)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11771)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk 0'
	nop
	brk 0
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 11772)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11773)
	; 17 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk 0'
	nop
	brk 0
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11774 || n = 11776)
	; 16..17 bytes
	delay_n (n-2)
	nop
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
	; 17 bytes
	php
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 26
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 11779)
	; 16 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 53
	Jne *-9
	ldx @zptemp
.elseif(n = 11780)
	; 16 bytes
	pha
	delay_n (n-3)
.elseif(n = 11781)
	; 17 bytes
	sty @zptemp
	ldy #203
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 11782)
	; 17 bytes
	php
	pha
	sec
	lda #0 ;hides 'brk imm'
	delay_n 4
	delay_n 26
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 11783)
	; 16 bytes
	sty @zptemp
	ldy #0
	nop
	delay_n 39
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 11784)
	; 16 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 53
	Jne *-9
	tax
	pla
.elseif(n = 11785)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 150
	Jmi *-10
	ldx @zptemp
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
.elseif(n = 11787)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 81
	Jne *-10
	ldy @zptemp
.elseif(n = 11788)
	; 16 bytes
	pha
	tya
	ldy #0
	nop
	delay_n 39
	dey
	Jne *-8
	tay
	pla
.elseif(n = 11789)
	; 18 bytes
	php
	pha
	tya
	ldy #214
	pha
	lda #14
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11790)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 150
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
	lda #17
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	tya
	pha
	ldy #203
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	pla
	tay
	pla
.elseif(n = 11794)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #45
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
.elseif(n = 11795)
	; 17 bytes
	pha
	txa
	pha
	tya
	ldx #218
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	tay
	pla
	tax
	pla
.elseif(n = 11796)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 11797) && (n < 11836))
; 11797 to 11835
.if(n = 11797)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #114
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 11798)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11799)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #45
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 11800)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 11801)
	; 18 bytes
	php
	pha
	tya
	ldy #85
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 11802)
	; 11 bytes
	php
	sta @zptemp
	lda #46
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 11803)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11804 || n = 11805 || n = 11807 || n = 11810 || n = 11812 || n = 11817 || n = 11825 || n = 11831)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11809 || n = 11814)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 11813)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11815)
	; 11 bytes
	php
	pha
	php
	lda #46
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11816 || n = 11821 || n = 11829 || n = 11834)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 11818 || n = 11823)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 11819 || n = 11824)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 11820)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11822)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11826)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11827)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11828 || n = 11830)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 11832)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11833 || n = 11835)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 11836 to 11875
.if(n = 11836 || n = 11849 || n = 11859 || n = 11863)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11837 || n = 11838 || n = 11840 || n = 11841 || n = 11843 || n = 11845 || n = 11846 || n = 11848 || (n > 11849 && n < 11859) || (n > 11859 && n < 11863) || (n > 11863 && n < 11876))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11839)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11842)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11844)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11847)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11876) && (n < 12188))
.if((n >= 11876) && (n < 12032))
; 11876 to 12031
.if((n > 11875 && n < 11919) || (n > 11919 && n < 11924) || (n > 11924 && n < 11964) || (n > 11964 && n < 12009))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11919)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11924 || n = 11964)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12009)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-279)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12010)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-280)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12011 || n = 12024 || n = 12031)
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12014)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12015)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12016)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12017)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12018)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12019)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12020)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12021)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12022)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12023)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	php
	pha
	tya
	pha
	ldy #200
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12027)
	; 17 bytes
	pha
	sty @zptemp
	ldy #40
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 12028)
	; 15 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 29
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 12029)
	; 18 bytes
	php
	sty @zptemp
	ldy #88
	pha
	lda #94
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	ldy @zptemp
	plp
.elseif(n = 12030)
	; 16 bytes
	delay_n (n-2)
	nop
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if(n = 12032)
	; 17 bytes
	pha
	tya
	pha
	ldy #40
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jpl *-7
	pla
	tay
	pla
.elseif(n = 12033)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	sta @zptemp
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12034)
	; 17 bytes
	sty @zptemp
	ldy #211
	delay_n 52
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12035 || n = 12041)
	; 17 bytes
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
	ldy #91
	lda #98
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12039)
	; 16 bytes
	sty @zptemp
	ldy #0
	delay_n 42
	dey
	Jne *-8
	ldy @zptemp
.elseif(n = 12040)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #119
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 12042)
	; 17 bytes
	pha
	sty @zptemp
	ldy #31
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 12043)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12044)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #170
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 12045)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #119
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12046)
	; 18 bytes
	php
	pha
	tya
	ldy #211
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12049)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #96
	lda #90
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 12050)
	; 17 bytes
	php
	sty @zptemp
	ldy #(n-14)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	ldy @zptemp
	plp
.endif
.else
; 12051 to 12070
.if(n = 12051)
	; 18 bytes
	php
	pha
	tya
	ldy #93
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12054)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #90
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12055)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 12056)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #145
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 12057)
	; 17 bytes
	sty @zptemp
	ldy #241
	delay_n 45
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12058)
	; 11 bytes
	php
	sta @zptemp
	lda #47
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 12059)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12060 || n = 12061 || n = 12063 || n = 12066 || n = 12068)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12062 || n = 12067)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12065 || n = 12070)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12069)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12072 || n = 12077 || n = 12085 || n = 12090)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12073 || n = 12081 || n = 12087)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12074 || n = 12079)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12075 || n = 12080)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12076)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12078)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12082)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12083)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12084 || n = 12086)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12088)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12089 || n = 12091 || n = 12092 || n = 12105)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12093 || n = 12094 || n = 12096 || n = 12097 || n = 12099 || n = 12101 || n = 12102 || n = 12104 || (n > 12105 && n < 12110))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12095)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12098)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12100)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12103)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 12110 to 12187
.if((n > 12109 && n < 12115) || (n > 12115 && n < 12119) || (n > 12119 && n < 12175) || (n > 12175 && n < 12180) || (n > 12180 && n < 12188))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12115 || n = 12119 || n = 12180)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12175)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12220) || (n > 12220 && n < 12265))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12220)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12265)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-280)/255
	jsr delay_256a_16_clocks_b
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
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12270)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12271)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12272)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12273)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12274)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12275)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12276)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12277)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12278)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12279)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #191
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 12283)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 12284)
	; 16 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	delay_n 26
	dey
	Jne *-9
	ldy @zptemp
.endif
.else
; 12285 to 12304
.if(n = 12285)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12286 || n = 12291)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12287)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12288)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #183
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
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
.elseif(n = 12290)
	; 17 bytes
	pha
	sty @zptemp
	ldy #12
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 12292)
	; 17 bytes
	sty @zptemp
	ldy #189
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12293)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #141
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12294)
	; 18 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	ldy @zptemp
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
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
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12299)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 12300)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 4
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12301)
	; 17 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 7
	delay_n 39
	iny
	Jne *-10
	ldy @zptemp
.elseif(n = 12302)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #90
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12303)
	; 17 bytes
	sty @zptemp
	ldy #212
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
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
.if(n = 12305)
	; 17 bytes
	php
	sty @zptemp
	ldy #(n-14)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 12306)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #31
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 12307)
	; 17 bytes
	sty @zptemp
	ldy #40
	delay_n 295
	dey
	Jpl *-9
	ldy @zptemp
.elseif(n = 12308)
	; 17 bytes
	pha
	tya
	ldy #212
	delay_n 53
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12309)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12310)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 12313)
	; 17 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12314)
	; 11 bytes
	php
	sta @zptemp
	lda #48
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 12315)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12316 || n = 12317 || n = 12319 || n = 12322 || n = 12324 || n = 12329 || n = 12337 || n = 12343)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12318 || n = 12323)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12321 || n = 12326)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12325)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12327)
	; 11 bytes
	php
	pha
	php
	lda #48
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12328 || n = 12333 || n = 12341)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12330 || n = 12335)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12331 || n = 12336)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12332)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12334)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12338)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12339)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12340 || n = 12342)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12345 || n = 12347 || n = 12348 || n = 12361 || n = 12371 || n = 12375 || n = 12436 || n = 12476)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12346)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12349 || n = 12350 || n = 12352 || n = 12353 || n = 12355 || n = 12357 || n = 12358 || n = 12360 || (n > 12361 && n < 12371) || (n > 12371 && n < 12375) || (n > 12375 && n < 12431) || (n > 12431 && n < 12436) || (n > 12436 && n < 12476) || (n > 12476 && n < 12501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12351)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12354)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12356)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12359)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12431)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #48
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12521)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-281)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12522)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12526)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12527)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12528)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12529)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12530)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12531)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12532)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12533)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12534)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12535)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12537)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #255
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 12538)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184
	lda #34
	jsr delay_a_27_clocks
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
	lda #95
	jsr delay_a_27_clocks
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
	; 16 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 29
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12541)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12542)
	; 17 bytes
	pha
	sty @zptemp
	ldy #216
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 12543 || n = 12548)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 12544)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12545)
	; 16 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc @zptemp
	delay_n 26
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12546)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #232
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
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
.elseif(n = 12549)
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #29
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12550)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 215
	Jmi *-10
	tay
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
	; 17 bytes
	pha
	tya
	ldy #209
	delay_n 55
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12553)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12554)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #108
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12555)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #187
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12556)
	; 17 bytes
	pha
	tya
	ldy #0
	inc @zptemp
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12557)
	; 17 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1563
	Jne *-10
	pla
	plp
.elseif(n = 12558)
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	tay
	pla
.endif
.else
; 12559 to 12578
.if(n = 12559)
	; 18 bytes
	php
	pha
	tya
	ldy #95
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12560)
	; 17 bytes
	php
	sty @zptemp
	ldy #(n-14)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 12561)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #114
	lda #75
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12562)
	; 18 bytes
	php
	pha
	tya
	ldy #112
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12563)
	; 18 bytes
	php
	pha
	tya
	ldy #98
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
.elseif(n = 12564)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12565)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 12566)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #209
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12567)
	; 17 bytes
	pha
	tya
	ldy #93
	delay_n 130
	dey
	Jne *-9
	tay
	pla
.elseif(n = 12568)
	; 18 bytes
	php
	pha
	tya
	ldy #141
	pha
	lda #(n-280)/256
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12569)
	; 18 bytes
	php
	pha
	tya
	ldy #251
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12570)
	; 11 bytes
	php
	sta @zptemp
	lda #49
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 12571)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12572 || n = 12573 || n = 12575 || n = 12578)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 12574)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12576)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_16_clocks_b
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
.elseif(n = 12580 || n = 12585 || n = 12593 || n = 12599)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12581)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12584 || n = 12589 || n = 12597 || n = 12602)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12586 || n = 12591)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12587 || n = 12592)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12588)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12590)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12594)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12595)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12596 || n = 12598)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12600)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12601 || n = 12603 || n = 12604 || n = 12617 || n = 12627 || n = 12631)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12605 || n = 12606 || n = 12608 || n = 12609 || n = 12611 || n = 12613 || n = 12614 || n = 12616 || (n > 12617 && n < 12627) || (n > 12627 && n < 12631) || (n > 12631 && n < 12657))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12607)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12610)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12612)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12615)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 12657) && (n < 12735))
; 12657 to 12734
.if((n > 12656 && n < 12687) || (n > 12687 && n < 12692) || (n > 12692 && n < 12732) || n = 12733 || n = 12734)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12687)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12692 || n = 12732)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12777)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-282)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12778)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12782)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12783)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12784)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12785)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12786)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12787)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12788)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12789)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12790)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12791)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 12793 to 12812
.if(n = 12793)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #206
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #79
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12796)
	; 16 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 32
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12797)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #95
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 12798)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 12799)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 58
	Jne *-10
	tax
	pla
.elseif(n = 12800)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12801)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	plp
	Jne *-8
	ldx @zptemp
	pla
.elseif(n = 12802)
	; 18 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 116
	iny
	Jpl *-11
	tay
	pla
.elseif(n = 12803)
	; 18 bytes
	php
	pha
	tya
	ldy #93
	pha
	lda #95
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12804)
	; 18 bytes
	php
	sty @zptemp
	ldy #24 ;hides 'clc'
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	ldy @zptemp
	plp
.elseif(n = 12805)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #188
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	; 17 bytes
	sty @zptemp
	ldy #0
	delay_n 45
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12808)
	; 17 bytes
	sty @zptemp
	ldy #(n-7)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12809)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 12810)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #188
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12811)
	; 18 bytes
	php
	pha
	tya
	ldy #104
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
.elseif(n = 12812)
	; 17 bytes
	sty @zptemp
	ldy #197
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
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
	; 17 bytes
	pha
	tya
	ldy #(n-12)/51
	delay_n 7
	delay_n 39
	dey
	Jne *-9
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
.elseif(n = 12815)
	; 17 bytes
	php
	sty @zptemp
	ldy #(n-14)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	ldy @zptemp
	plp
.elseif(n = 12816)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #237
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
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
.elseif(n = 12818)
	; 17 bytes
	pha
	sty @zptemp
	ldy #32
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 12819)
	; 18 bytes
	php
	pha
	tya
	ldy #100
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
.elseif(n = 12820)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/51
	pha
	lda #74 ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 12822 || n = 12828 || n = 12829 || n = 12831)
	; 10..18 bytes
	delay_n (n-2)
	nop
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
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12825)
	; 17 bytes
	sty @zptemp
	ldy #221
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 12826)
	; 11 bytes
	php
	sta @zptemp
	lda #50
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 12827)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12830)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.else
; 12832 to 12851
.if(n = 12832)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12833 || n = 12838)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 12834 || n = 12836 || n = 12841 || n = 12849)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 12835)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 12837)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12839)
	; 11 bytes
	php
	pha
	php
	lda #50
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12840 || n = 12845)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12842 || n = 12847)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 12843 || n = 12848)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 12844)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12846)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12850)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12851)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 12852 to 12890
.if(n = 12852 || n = 12854)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12853 || n = 12858)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 12855)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12856)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12857 || n = 12859 || n = 12860 || n = 12873 || n = 12883 || n = 12887)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12861 || n = 12862 || n = 12864 || n = 12865 || n = 12867 || n = 12869 || n = 12870 || n = 12872 || (n > 12873 && n < 12883) || (n > 12883 && n < 12887) || (n > 12887 && n < 12891))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12863)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12866)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12868)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12871)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 12891 to 12968
.if((n > 12890 && n < 12943) || (n > 12943 && n < 12948) || (n > 12948 && n < 12969))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12943)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12948)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 12988) || (n > 12988 && n < 13033))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12988)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13033)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-283)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13034)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13038)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13039)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13040)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13041)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13042)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13043)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13044)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13045)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13046)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 13047) && (n < 13086))
.if((n >= 13047) && (n < 13066))
; 13047 to 13065
.if(n = 13047)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13048 || n = 13055)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13049)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #138
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 13050)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #24 ;hides 'clc'
	inx
	Jne *-2
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 13051)
	; 18 bytes
	php
	pha
	tya
	ldy #181
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13052)
	; 16 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	delay_n 26
	dey
	Jne *-9
	ldy @zptemp
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
.elseif(n = 13056)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 562
	Jcs *-11
	pla
	plp
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
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 224
	Jmi *-10
	ldy @zptemp
.elseif(n = 13060)
	; 17 bytes
	sty @zptemp
	ldy #229
	delay_n 52
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 13061)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #237
	lda #21
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #225
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 13065)
	; 17 bytes
	pha
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #188
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.endif
.else
; 13066 to 13085
.if(n = 13066)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #163
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13067)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #5
	jsr delay_256a_11_clocks_b_
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 13068)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 7
	delay_n 39
	dey
	Jne *-9
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
.elseif(n = 13070)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #188
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 13071)
	; 18 bytes
	php
	pha
	tya
	ldy #251
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13072)
	; 17 bytes
	sty @zptemp
	ldy #201
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 13073)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13074)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #33
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 13075)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 13076)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13077)
	; 17 bytes
	pha
	tya
	ldy #201
	delay_n 60
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13078)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #18
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13081)
	; 18 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 13082)
	; 11 bytes
	php
	sta @zptemp
	lda #51
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 13083)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13084 || n = 13085)
	; 10..12 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086 || n = 13091)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 13087 || n = 13090 || n = 13092 || n = 13097 || n = 13105 || n = 13111)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 13088)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13089 || n = 13094)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13093)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13095)
	; 11 bytes
	php
	pha
	php
	lda #51
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13096 || n = 13101 || n = 13109 || n = 13114)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13098 || n = 13103)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13099 || n = 13104)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13100)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13102)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13106)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13107)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13108 || n = 13110)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13112)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13113 || n = 13115 || n = 13116)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13117 || n = 13118 || n = 13120 || n = 13121 || n = 13123 || n = 13125)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13119)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13122)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13124)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
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
.if(n = 13126 || n = 13128 || (n > 13129 && n < 13139) || (n > 13139 && n < 13143) || (n > 13143 && n < 13199) || (n > 13199 && n < 13204) || (n > 13204 && n < 13244) || (n > 13244 && n < 13282))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13127)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13129 || n = 13139 || n = 13143 || n = 13204 || n = 13244)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13199)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #51
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13289)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-284)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13290)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13294)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13295)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13296)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13297)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13298)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13299)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13300)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13302)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13303)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13304)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13305)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #102
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 13306)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #56
	jsr delay_a_27_clocks
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
	; 17 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	delay_n 27
	dey
	Jne *-10
	ldy @zptemp
.elseif(n = 13309)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #151
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #170
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #13
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 13314)
	; 17 bytes
	pha
	sty @zptemp
	ldy #100
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 13315)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #63
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13316)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #45
	lda #255
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 13321) && (n < 13340))
; 13321 to 13339
.if(n = 13321)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #190
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 13322)
	; 18 bytes
	php
	pha
	tya
	ldy #251
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13323)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13324)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13325)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #59
	jsr delay_a_27_clocks
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
	lda #36
	jsr delay_a_27_clocks
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
	lda #249
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13328)
	; 18 bytes
	pha
	txa
	sty @zptemp
	ldx #110
	ldy #22
	dey
	Jpl *-1
	dex
	Jne *-6
	ldy @zptemp
	tax
	pla
.elseif(n = 13329)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13330)
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13331)
	; 18 bytes
	php
	pha
	tya
	ldy #104
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
.elseif(n = 13332)
	; 17 bytes
	pha
	tya
	ldy #222
	delay_n 55
	dey
	Jne *-9
	tay
	pla
.elseif(n = 13333)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 13334)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #74
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 13335)
	; 18 bytes
	php
	sty @zptemp
	ldy #173
	pha
	lda #36
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
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
	; 11 bytes
	php
	sta @zptemp
	lda #52
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 13339)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 13340 to 13359
.if(n = 13340 || n = 13341 || n = 13343 || n = 13346 || n = 13348 || n = 13353)
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13345 || n = 13350)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13349)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13351)
	; 11 bytes
	php
	pha
	php
	lda #52
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13352 || n = 13357)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13354 || n = 13359)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13355)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13356)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13358)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13361 || n = 13367)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 13362)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13363)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13364 || n = 13366)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13365 || n = 13370)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13368)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13369 || n = 13371 || n = 13372 || n = 13385 || n = 13395 || n = 13399)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13373 || n = 13374 || n = 13376 || n = 13377 || n = 13379 || n = 13381 || n = 13382 || n = 13384 || (n > 13385 && n < 13395) || (n > 13395 && n < 13399) || (n > 13399 && n < 13438))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13375)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13378)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13380)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13383)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 13438) && (n < 13594))
.if((n >= 13438) && (n < 13516))
; 13438 to 13515
.if((n > 13437 && n < 13455) || (n > 13455 && n < 13460) || (n > 13460 && n < 13500) || (n > 13500 && n < 13516))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13455)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13460 || n = 13500)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13545)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-285)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13546)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13550)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13551)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13552)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13553)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13554)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13556)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13557)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13558)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13559)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	ldy #(n-19)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13562)
	; 17 bytes
	pha
	stx @zptemp
	ldx #232 ;hides 'inx'
	php
	lda #2
	jsr delay_256a_16_clocks_b
	plp
	Jne *-8
	ldx @zptemp
	pla
.elseif(n = 13563)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #105
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 13564)
	; 17 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 11
	delay_n 26
	dey
	Jne *-10
	ldy @zptemp
.elseif(n = 13565)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13566)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #147
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 13567)
	; 17 bytes
	pha
	txa
	pha
	ldx #232 ;hides 'inx'
	php
	lda #2
	jsr delay_256a_16_clocks_b
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
	lda #158
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13571)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 233
	Jmi *-10
	ldy @zptemp
.elseif(n = 13572)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #251
	ldy #136 ;hides 'dey'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
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
	sty @zptemp
	ldy #226
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 13575)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #58
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
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
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #197
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 13579)
	; 17 bytes
	sty @zptemp
	ldy #234
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 13580)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #58
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 13581)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13582)
	; 17 bytes
	pha
	tya
	ldy #45
	delay_n 290
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 13583)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #197
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #234
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 13587)
	; 17 bytes
	sty @zptemp
	ldy #96
	delay_n 135
	dey
	Jpl *-9
	ldy @zptemp
.elseif(n = 13588)
	; 18 bytes
	pha
	txa
	sty @zptemp
	ldx #234 ;hides 'nop'
	ldy #9
	dey
	Jpl *-1
	dex
	Jne *-7
	ldy @zptemp
	tax
	pla
.elseif(n = 13589)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #65
	jsr delay_a_27_clocks
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
	lda #202
	jsr delay_a_27_clocks
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
	; 11 bytes
	php
	sta @zptemp
	lda #53
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 13595)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13596 || n = 13597 || n = 13599 || n = 13602 || n = 13604 || n = 13609 || n = 13617 || n = 13623)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13601 || n = 13606)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13605)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13607)
	; 11 bytes
	php
	pha
	php
	lda #53
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13608 || n = 13613 || n = 13621 || n = 13626)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13610 || n = 13615)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13611 || n = 13616)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13612)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13614)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13618)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13619)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13620 || n = 13622)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13624)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13625 || n = 13627 || n = 13628 || n = 13641 || n = 13651 || n = 13655 || n = 13716)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13629 || n = 13630 || n = 13632 || n = 13633 || n = 13635 || n = 13637 || n = 13638 || n = 13640 || (n > 13641 && n < 13651) || (n > 13651 && n < 13655) || (n > 13655 && n < 13711) || (n > 13711 && n < 13716) || (n > 13716 && n < 13751))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13631)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13634)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13636)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13639)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13711)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #53
	jsr delay_256a_16_clocks_b
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
.if((n > 13750 && n < 13756) || (n > 13756 && n < 13790))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13756)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13801)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-286)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13802)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13806)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13807)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13808)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13810)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13811)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13812)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13813)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13814)
	; 17 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 63
	Jne *-10
	tax
	pla
.elseif(n = 13815)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13816)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 13817)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 13818)
	; 17 bytes
	pha
	sty @zptemp
	ldy #238
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 13819)
	; 18 bytes
	php
	pha
	tya
	ldy #230
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13820)
	; 17 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 10
	delay_n 26
	dey
	Jne *-10
	ldy @zptemp
.elseif(n = 13821)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #27
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 13822)
	; 18 bytes
	php
	pha
	tya
	ldy #128
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	lda #14
	jsr delay_a_27_clocks
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
	lda #18
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 13828)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #73
	jsr delay_a_27_clocks
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
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13832)
	; 17 bytes
	sty @zptemp
	ldy #35
	delay_n 390
	dey
	Jne *-9
	ldy @zptemp
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
	lda #166
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13835)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #47
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13836)
	; 18 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	pha
	lda #149
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	ldy @zptemp
	plp
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
	lda #170
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13839)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #147
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 13840)
	; 18 bytes
	php
	pha
	tya
	ldy #51
	pha
	lda #230
	jsr delay_a_27_clocks
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
	lda #149
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	ldy #108
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
.elseif(n = 13844)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #141
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 13845)
	; 18 bytes
	php
	pha
	tya
	ldy #223
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13846)
	; 17 bytes
	pha
	sty @zptemp
	ldy #104
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
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
	lda #156
	jsr delay_a_27_clocks
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
	lda #141
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 13850)
	; 11 bytes
	php
	sta @zptemp
	lda #54
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 13851)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13852 || n = 13853 || n = 13855 || n = 13858 || n = 13860 || n = 13865)
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13857 || n = 13862)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 13861)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13863)
	; 11 bytes
	php
	pha
	php
	lda #54
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13864)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13866)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 13869 || n = 13877 || n = 13882)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 13870)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13871)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 13872)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 13873 || n = 13879)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 13874)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13875)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13876 || n = 13878)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 13880)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13881 || n = 13883 || n = 13884 || n = 13897)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13885 || n = 13886 || n = 13888 || n = 13889 || n = 13891 || n = 13893 || n = 13894 || n = 13896 || (n > 13897 && n < 13907))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13887)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13890)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13892)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13895)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 13907 to 14062
.if(n = 13907 || n = 13911 || n = 13972 || n = 14012)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13907 && n < 13911) || (n > 13911 && n < 13967) || (n > 13967 && n < 13972) || (n > 13972 && n < 14012) || (n > 14012 && n < 14057))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13967)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14057)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-287)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14058)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14062)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
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
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14064)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14065)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14066)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14067)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14068)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14069)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
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
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14072)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #144
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 14073)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #208
	jsr delay_a_27_clocks
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
	lda #29
	jsr delay_a_27_clocks
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
	lda #247
	jsr delay_a_27_clocks
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
	lda #144
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 14078)
	; 17 bytes
	pha
	sty @zptemp
	ldy #47
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 14079)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #109
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 14080)
	; 18 bytes
	php
	pha
	tya
	ldy #128
	pha
	lda #68
	jsr delay_a_27_clocks
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
	lda #38
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14086)
	; 17 bytes
	sty @zptemp
	ldy #247
	delay_n 52
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 14087)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #75
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14088)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #178
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	sty @zptemp
	ldy #207
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 14091)
	; 17 bytes
	pha
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #206
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 14092)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14093)
	; 18 bytes
	php
	pha
	tya
	ldy #227
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14094)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14095)
	; 18 bytes
	php
	pha
	tya
	ldy #207
	pha
	lda #27
	jsr delay_a_27_clocks
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
	lda #206
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 14097)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #207
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 14098)
	; 18 bytes
	php
	pha
	tya
	ldy #247
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14099)
	; 18 bytes
	php
	pha
	tya
	ldy #110
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
.elseif(n = 14100)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14101)
	; 17 bytes
	sty @zptemp
	ldy #243
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
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
	lda #34
	jsr delay_a_27_clocks
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
	lda #211
	jsr delay_a_27_clocks
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
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14106)
	; 11 bytes
	php
	sta @zptemp
	lda #55
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 14107)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14108 || n = 14109 || n = 14111 || n = 14114 || n = 14116 || n = 14121 || n = 14129 || n = 14135)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14113 || n = 14118)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14117)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14119)
	; 11 bytes
	php
	pha
	php
	lda #55
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14120 || n = 14125 || n = 14133 || n = 14138)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14122 || n = 14127)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14123 || n = 14128)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14124)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14126)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14130)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14131)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14132 || n = 14134)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14136)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14137 || n = 14139 || n = 14140)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 14141 to 14218
.if(n = 14141 || n = 14142 || n = 14144 || n = 14145 || n = 14147 || n = 14149 || n = 14150 || n = 14152 || (n > 14153 && n < 14163) || (n > 14163 && n < 14167) || (n > 14167 && n < 14219))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14143)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14146)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14148)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14151)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14153 || n = 14163 || n = 14167)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if((n > 14218 && n < 14223) || (n > 14223 && n < 14228) || (n > 14228 && n < 14268) || (n > 14268 && n < 14297))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14223)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14228 || n = 14268)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14313)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-288)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14314)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14318)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14319)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14320)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14321)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14322)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14323)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14324)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14325)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14326)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14327)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14329)
	; 18 bytes
	php
	pha
	tya
	ldy #159
	pha
	lda #49
	jsr delay_a_27_clocks
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
	lda #62
	jsr delay_a_27_clocks
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
	lda #215
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 14332)
	; 17 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	delay_n 26
	dey
	Jne *-10
	ldy @zptemp
.elseif(n = 14333)
	; 17 bytes
	sty @zptemp
	ldy #247
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 14334)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #39
	jsr delay_a_27_clocks
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
	lda #170
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 14336) && (n < 14356))
; 14336 to 14355
.if(n = 14336)
	; 17 bytes
	pha
	sty @zptemp
	ldy #13
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 14337)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #111
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
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
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14340)
	; 17 bytes
	pha
	sty @zptemp
	ldy #247
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
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
	lda #69
	jsr delay_a_27_clocks
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
	lda #34
	jsr delay_a_27_clocks
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
	lda #46
	jsr delay_a_27_clocks
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
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14348)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #231
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14349)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #155
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 14350)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/281
	pha
	lda #240
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14351)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14352)
	; 17 bytes
	pha
	tya
	ldy #239
	delay_n 55
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14353)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 14354)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14355)
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
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14357)
	; 18 bytes
	php
	pha
	tya
	ldy #214
	pha
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14358)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #243
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 14359)
	; 18 bytes
	php
	pha
	tya
	ldy #239
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14360)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #220
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 14361)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #235
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14362)
	; 11 bytes
	php
	sta @zptemp
	lda #56
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 14363)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14364 || n = 14365 || n = 14367 || n = 14370 || n = 14372)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14366 || n = 14371)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14369 || n = 14374)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14373)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14375)
	; 11 bytes
	php
	pha
	php
	lda #56
	jsr delay_256a_16_clocks_b
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
.if(n = 14376 || n = 14381 || n = 14389 || n = 14394)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14377 || n = 14385 || n = 14391)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 14378 || n = 14383)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14379 || n = 14384)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14380)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14382)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14386)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14387)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14388 || n = 14390)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14392)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14393 || n = 14395 || n = 14396 || n = 14409 || n = 14419 || n = 14423 || n = 14484 || n = 14524)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14397 || n = 14398 || n = 14400 || n = 14401 || n = 14403 || n = 14405 || n = 14406 || n = 14408 || (n > 14409 && n < 14419) || (n > 14419 && n < 14423) || (n > 14423 && n < 14479) || (n > 14479 && n < 14484) || (n > 14484 && n < 14524) || (n > 14524 && n < 14532))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14399)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14402)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14404)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14407)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14479)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #56
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14569)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-289)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14570)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-290)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 14571) && (n < 14590))
; 14571 to 14589
.if(n = 14571 || n = 14584)
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14574)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14575)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14576)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14577)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14578)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14579)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14580)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14581)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14582)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14583)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14585)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14586)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #224
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14587)
	; 18 bytes
	php
	sty @zptemp
	ldy #247
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 14588)
	; 16 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 39
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 14589)
	; 18 bytes
	php
	pha
	tya
	ldy #235
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 14590 to 14609
.if(n = 14590)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 14591 || n = 14596)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 14592)
	; 17 bytes
	pha
	tya
	ldy #243
	delay_n 55
	dey
	Jne *-9
	tay
	pla
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
	lda #224
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14595)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #112
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 14597)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 251
	Jmi *-10
	ldy @zptemp
.elseif(n = 14598)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14599)
	; 17 bytes
	sty @zptemp
	ldy #0
	delay_n 52
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 14600)
	; 17 bytes
	pha
	txa
	pha
	tya
	ldy #112
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	tay
	pla
	tax
	pla
.elseif(n = 14601)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #55
	lda #231
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14602)
	; 17 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 251
	Jmi *-10
	tay
	pla
.elseif(n = 14603)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #79
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14604)
	; 17 bytes
	pha
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #215
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 14605)
	; 17 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1819
	Jne *-10
	pla
	plp
.elseif(n = 14606)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14607)
	; 17 bytes
	pha
	tya
	ldy #21
	delay_n 690
	dey
	Jne *-9
	tay
	pla
.elseif(n = 14608)
	; 18 bytes
	php
	sty @zptemp
	ldy #216 ;hides 'cld'
	pha
	lda #121
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	ldy @zptemp
	plp
.elseif(n = 14609)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #215
	jsr delay_a_27_clocks
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
	txa
	sty @zptemp
	ldy #37
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 14611)
	; 18 bytes
	php
	pha
	tya
	ldy #113
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 14612)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #32
	jsr delay_a_27_clocks
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
	lda #121
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 14614)
	; 18 bytes
	php
	pha
	tya
	ldy #139
	pha
	lda #64
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 14617)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 14618)
	; 11 bytes
	php
	sta @zptemp
	lda #57
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 14619)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14620 || n = 14621 || n = 14623 || n = 14626 || n = 14628 || n = 14633 || n = 14641 || n = 14647)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 14622 || n = 14627)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14625 || n = 14630)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 14629)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14631)
	; 11 bytes
	php
	pha
	php
	lda #57
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14632 || n = 14637 || n = 14645)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14634 || n = 14639)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14635 || n = 14640)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14636)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14638)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14642)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14643)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14644 || n = 14646)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14648)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14649 to 14687
.if(n = 14649 || n = 14651 || n = 14652 || n = 14665 || n = 14675 || n = 14679)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14650)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14653 || n = 14654 || n = 14656 || n = 14657 || n = 14659 || n = 14661 || n = 14662 || n = 14664 || (n > 14665 && n < 14675) || (n > 14675 && n < 14679) || (n > 14679 && n < 14688))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14655)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14658)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14660)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14663)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 14688) && (n < 14844))
; 14688 to 14843
.if((n > 14687 && n < 14735) || (n > 14735 && n < 14740) || (n > 14740 && n < 14780) || (n > 14780 && n < 14825))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14735)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14740 || n = 14780)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14825)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-290)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14826)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14827 || n = 14840)
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14830)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14831)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14832)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14833)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14834)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14835)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14836)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14837)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14838)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14839)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14841)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14842)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/61
	pha
	lda #20
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14843)
	; 18 bytes
	php
	pha
	tya
	ldy #218
	pha
	lda #27
	jsr delay_a_27_clocks
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
	; 17 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-10
	ldy @zptemp
.elseif(n = 14845)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14846)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #247
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14847)
	; 17 bytes
	sty @zptemp
	ldy #106
	delay_n 135
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 14848)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14849)
	; 17 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	delay_n 26
	dey
	Jne *-10
	tay
	pla
.elseif(n = 14850)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14851)
	; 18 bytes
	php
	pha
	tya
	ldy #206
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #114
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 14854)
	; 18 bytes
	php
	pha
	tya
	ldy #215
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14855)
	; 17 bytes
	sty @zptemp
	ldy #0
	delay_n 53
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 14856)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #170
	jsr delay_a_27_clocks
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
	lda #222
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #163
	lda #(n-267)/256
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14862)
	; 17 bytes
	pha
	sty @zptemp
	ldy #0
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #181
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 14864)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 14865)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #209
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14866)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #36
	jsr delay_a_27_clocks
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
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14869)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #188
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14872)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #28
	jsr delay_a_27_clocks
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
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14874)
	; 11 bytes
	php
	sta @zptemp
	lda #58
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 14875)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14876 || n = 14877 || n = 14879 || n = 14882)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
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
.elseif(n = 14884 || n = 14889 || n = 14897 || n = 14903)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 14885)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14888 || n = 14893 || n = 14901 || n = 14906)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 14890 || n = 14895)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 14891 || n = 14896)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 14892)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14894)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14898)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14899)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14900 || n = 14902)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 14904)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14905 || n = 14907 || n = 14908 || n = 14921)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14909 || n = 14910 || n = 14912 || n = 14913 || n = 14915 || n = 14917 || n = 14918 || n = 14920)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14911)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14914)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14916)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14919)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 14922 to 15000
.if((n > 14921 && n < 14931) || (n > 14931 && n < 14935) || (n > 14935 && n < 14991) || (n > 14991 && n < 14996) || (n > 14996 && n < 15001))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14931 || n = 14935 || n = 14996)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14991)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #58
	jsr delay_256a_16_clocks_b
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
.if((n > 15000 && n < 15036) || (n > 15036 && n < 15079))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15036)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15081)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-291)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15082)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15086)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15087)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15088)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15089)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15090)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15091)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15092)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15093)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15094)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15095)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15097)
	; 18 bytes
	php
	pha
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
	pla
	plp
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
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15099)
	; 18 bytes
	php
	pha
	tya
	ldy #232
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15100)
	; 17 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	delay_n 39
	dey
	Jne *-10
	ldy @zptemp
.elseif(n = 15101)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15102)
	; 18 bytes
	php
	sty @zptemp
	ldy #184
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15103)
	; 18 bytes
	php
	sty @zptemp
	ldy #191
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15104)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #29
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
	pha
	tya
	pha
	ldy #130
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15107)
	; 18 bytes
	php
	pha
	tya
	ldy #184
	pha
	lda #41
	jsr delay_a_27_clocks
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
	lda #38
	jsr delay_a_27_clocks
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
	lda #161
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15110)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 260
	Jmi *-10
	ldy @zptemp
.elseif(n = 15111)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #117
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 15112)
	; 18 bytes
	php
	pha
	tya
	ldy #128
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 15113)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #141
	lda #73
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #200 ;hides 'iny'
	php
	lda #224
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldy @zptemp
	pla
.endif
.endif
.else
.if((n >= 15118) && (n < 15137))
; 15118 to 15136
.if(n = 15118)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15119)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #83
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15120)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #168
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15121)
	; 18 bytes
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	php
	lda #85
	jsr delay_a_27_clocks
	plp
	iny
	Jne *-9
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
	lda #224
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 15123)
	; 18 bytes
	php
	pha
	tya
	ldy #117
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 15124)
	; 18 bytes
	php
	pha
	tya
	ldy #159
	pha
	lda #54
	jsr delay_a_27_clocks
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
	lda #42
	jsr delay_a_27_clocks
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
	; 17 bytes
	sty @zptemp
	ldy #108
	delay_n 135
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 15128)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #92
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15129)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 194
	Jmi *-10
	ldx @zptemp
.elseif(n = 15130)
	; 11 bytes
	php
	sta @zptemp
	lda #59
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 15131)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15132 || n = 15133 || n = 15135)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 15137 to 15156
.if(n = 15137 || n = 15142)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15138 || n = 15140 || n = 15145 || n = 15153)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 15139)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 15141)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15143)
	; 11 bytes
	php
	pha
	php
	lda #59
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15144 || n = 15149)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15146 || n = 15151)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15147 || n = 15152)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15148)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15150)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15154)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15155)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
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
	delay_n (n-13)
	brk 0
.elseif(n = 15158)
	; 14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15159)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15160)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15161 || n = 15163 || n = 15164 || n = 15177 || n = 15187 || n = 15191 || n = 15252 || n = 15292)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15165 || n = 15166 || n = 15168 || n = 15169 || n = 15171 || n = 15173 || n = 15174 || n = 15176 || (n > 15177 && n < 15187) || (n > 15187 && n < 15191) || (n > 15191 && n < 15247) || (n > 15247 && n < 15252) || (n > 15252 && n < 15292) || (n > 15292 && n < 15313))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15167)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15170)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15172)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15175)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15247)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #59
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15337)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-292)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15338)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15342)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15343)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15344)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15345)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15346)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15347)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15348)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15349)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15350)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #228
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 15351)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
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
	lda #83
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/71
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15356)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #71
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
.elseif(n = 15357)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 197
	Jmi *-10
	ldx @zptemp
.elseif(n = 15358)
	; 18 bytes
	php
	sty @zptemp
	ldy #137
	pha
	lda #71
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15359)
	; 17 bytes
	pha
	sty @zptemp
	ldy #15
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 15360)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #187
	lda #48
	jsr delay_a_27_clocks
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
	lda #71
	jsr delay_a_27_clocks
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
	lda #71
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #137
	lda #78
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 15366)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #236
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15367)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15368)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #238
	jsr delay_a_27_clocks
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
	lda #161
	jsr delay_a_27_clocks
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
	lda #78
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #165
	lda #(n-267)/256
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15372)
	; 17 bytes
	pha
	tya
	ldy #0
	delay_n 55
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
	lda #69
	jsr delay_a_27_clocks
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
	lda #171
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15376)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #65
	jsr delay_a_27_clocks
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
	lda #85
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15378)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #39
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 15379)
	; 18 bytes
	php
	pha
	tya
	ldy #120
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
.elseif(n = 15380)
	; 18 bytes
	php
	sty @zptemp
	ldy #197
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15381)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15382)
	; 18 bytes
	pha
	tya
	ldy #145
	delay_n 101
	dey
	Jne *-10
	tay
	pla
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
	lda #84
	jsr delay_a_27_clocks
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
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15386)
	; 11 bytes
	php
	sta @zptemp
	lda #60
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 15387)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15388 || n = 15389)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 15390)
	; 10 bytes
	pha
	delay_n (n-3)
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15394 || n = 15396 || n = 15401 || n = 15409 || n = 15415)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15392)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_16_clocks_b
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
.elseif(n = 15397)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15399)
	; 11 bytes
	php
	pha
	php
	lda #60
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15400 || n = 15405 || n = 15413 || n = 15418)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15402 || n = 15407)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15403 || n = 15408)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15404)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15406)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15410)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15411)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15412 || n = 15414)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15416)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15417 || n = 15419 || n = 15420 || n = 15433 || n = 15443 || n = 15447)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15421 || n = 15422 || n = 15424 || n = 15425 || n = 15427 || n = 15429 || n = 15430 || n = 15432 || (n > 15433 && n < 15443) || (n > 15443 && n < 15447) || (n > 15447 && n < 15469))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15423)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15426)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15428)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15431)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 15469) && (n < 15547))
; 15469 to 15546
.if((n > 15468 && n < 15503) || (n > 15503 && n < 15508) || (n > 15508 && n < 15547))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15503)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15508)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if(n = 15547 || (n > 15548 && n < 15586))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15548)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15593)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-293)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15594)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15598)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15599)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15600)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15601)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15602)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15603)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15604)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15605)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 15606 to 15625
.if(n = 15606)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15607)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #164
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15610)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #131
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 15611)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
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
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15614)
	; 18 bytes
	php
	sty @zptemp
	ldy #240
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15615)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15616)
	; 18 bytes
	php
	sty @zptemp
	ldy #57
	pha
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	ldy @zptemp
	plp
.elseif(n = 15617)
	; 17 bytes
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
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
	lda #77
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15619)
	; 18 bytes
	php
	pha
	tya
	ldy #240
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15620)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #35
	jsr delay_a_27_clocks
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
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 15622)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #6
	jsr delay_256a_11_clocks_b_
	plp
	Jmi *-8
	ldy @zptemp
	pla
.elseif(n = 15623)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 109
	Jne *-10
	ldy @zptemp
.elseif(n = 15624)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #54
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 15625)
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #240
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15627)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #6
	jsr delay_256a_11_clocks_b_
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #73
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
.elseif(n = 15631)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #223
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15634)
	; 18 bytes
	php
	sty @zptemp
	ldy #220
	pha
	lda #30
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15635)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #73
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 15636)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 72
	Jne *-10
	ldx @zptemp
.elseif(n = 15637)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #33
	jsr delay_a_27_clocks
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
	lda #175
	jsr delay_a_27_clocks
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
	lda #30
	jsr delay_a_27_clocks
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
	; 11 bytes
	php
	sta @zptemp
	lda #61
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 15643)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15644)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.else
; 15645 to 15664
.if(n = 15645 || n = 15647 || n = 15650 || n = 15652 || n = 15657)
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15649 || n = 15654)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15653)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15655)
	; 11 bytes
	php
	pha
	php
	lda #61
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15656 || n = 15661)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15658 || n = 15663)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15659 || n = 15664)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15660)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15662)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665 || n = 15671)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15666)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15667)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15668 || n = 15670)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15669 || n = 15674)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15672)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15673 || n = 15675 || n = 15676 || n = 15689 || n = 15699 || n = 15703)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15677 || n = 15678 || n = 15680 || n = 15681 || n = 15683 || n = 15685 || n = 15686 || n = 15688 || (n > 15689 && n < 15699) || (n > 15699 && n < 15703))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15679)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15682)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15684)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15687)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 15704 to 15781
.if((n > 15703 && n < 15759) || (n > 15759 && n < 15764) || (n > 15764 && n < 15782))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15759)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15764)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15804) || (n > 15804 && n < 15849))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15804)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15849)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-294)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15850)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15854)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15855)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15856)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15857)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15858)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15859)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
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
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15861)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15862)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15863)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #237
	jsr delay_a_27_clocks
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
	txa
	sty @zptemp
	ldx #22
	ldy #136 ;hides 'dey'
	Jne *-1
	dex
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 15866)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #240
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15867)
	; 17 bytes
	sty @zptemp
	ldy #244
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 15868)
	; 17 bytes
	php
	pha
	sty @zptemp
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldy #136 ;hides 'dey'
	Jne *-1
	Jcs *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 15869)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #55
	lda #249
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	ldy @zptemp
	pla
	plp
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
	sty @zptemp
	ldy #157
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
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
	lda #249
	jsr delay_a_27_clocks
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
	sec
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
	lda #60
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #157
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #49
	jsr delay_a_27_clocks
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
	lda #164
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15881)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15882)
	; 18 bytes
	pha
	tya
	ldy #230
	delay_n 64
	dey
	Jne *-10
	tay
	pla
.elseif(n = 15883)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15884)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15885)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #122
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
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
	sty @zptemp
	ldy #(n-14)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 15888)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15889)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 204
	Jmi *-10
	ldx @zptemp
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
	; 18 bytes
	php
	pha
	tya
	ldy #123
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 15892)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15893)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #89
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
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
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #168
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 15897)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 111
	Jne *-10
	ldy @zptemp
.elseif(n = 15898)
	; 11 bytes
	php
	sta @zptemp
	lda #62
	jsr delay_256a_11_clocks_b_
	lda @zptemp
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
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15900 || n = 15901 || n = 15903 || n = 15906 || n = 15908 || n = 15913 || n = 15921 || n = 15927)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15905 || n = 15910)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 15909)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15911)
	; 11 bytes
	php
	pha
	php
	lda #62
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15912 || n = 15917 || n = 15925 || n = 15930)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 15914 || n = 15919)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 15915 || n = 15920)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 15916)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15918)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15922)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15923)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15924 || n = 15926)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 15928)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15929 || n = 15931 || n = 15932)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15933 || n = 15934 || n = 15936 || n = 15937)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15935)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
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
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15939 || n = 15941 || n = 15942 || n = 15944 || (n > 15945 && n < 15955) || (n > 15955 && n < 15959) || (n > 15959 && n < 16015) || (n > 16015 && n < 16020) || (n > 16020 && n < 16060) || (n > 16060 && n < 16094))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15940)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15943)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15945 || n = 15955 || n = 15959 || n = 16020 || n = 16060)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16015)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #62
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16105)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-295)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16106)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16110)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16111)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16112)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16114)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16115)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16116)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16117)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16118)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16119)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #53
	jsr delay_a_27_clocks
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
	ldy #(n-19)/66
	pha
	lda #25
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16124)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #171
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 16125)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #244
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16126)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 16127)
	; 17 bytes
	sty @zptemp
	ldy #248
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 16128)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #49
	jsr delay_a_27_clocks
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
	lda #171
	jsr delay_a_27_clocks
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
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16131)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	lda #57
	jsr delay_a_27_clocks
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
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16136)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 278
	Jmi *-10
	ldy @zptemp
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
	lda #42
	jsr delay_a_27_clocks
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
	lda #89
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16140)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #199
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
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
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16143)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #124
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 16144)
	; 18 bytes
	php
	pha
	tya
	ldy #215
	pha
	lda #34
	jsr delay_a_27_clocks
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
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16146)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #42
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 16147)
	; 18 bytes
	php
	pha
	tya
	ldy #96
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
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16153)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #218
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16154)
	; 11 bytes
	php
	sta @zptemp
	lda #63
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 16155)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16156 || n = 16157 || n = 16159 || n = 16162 || n = 16164 || n = 16169)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16158 || n = 16163)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16160)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16161 || n = 16166)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16165)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16167)
	; 11 bytes
	php
	pha
	php
	lda #63
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16168)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16170)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.elseif(n = 16173 || n = 16181 || n = 16186)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16174)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16175)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16176)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16177 || n = 16183)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 16178)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16179)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16180 || n = 16182)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16184)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16185 || n = 16187 || n = 16188 || n = 16201 || n = 16211 || n = 16215)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16189 || n = 16190 || n = 16192 || n = 16193 || n = 16195 || n = 16197 || n = 16198 || n = 16200 || (n > 16201 && n < 16211) || (n > 16211 && n < 16215) || (n > 16215 && n < 16251))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16191)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16194)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16196)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16199)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.if((n > 16250 && n < 16271) || (n > 16271 && n < 16276) || (n > 16276 && n < 16316) || (n > 16316 && n < 16329))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16271)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16276 || n = 16316)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16361)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-296)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16362)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16366)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16367)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
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
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16369)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16370)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16371)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16372)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16373)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16374)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16375)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16376)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #246
	jsr delay_a_27_clocks
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
	lda #49
	jsr delay_a_27_clocks
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
	php
	pha
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
	pla
	plp
.elseif(n = 16380)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #40
	jsr delay_a_27_clocks
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
	lda #40
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16382)
	; 17 bytes
	pha
	sty @zptemp
	ldy #15
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 16383)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #251
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 16384)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16385)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #133
	lda #89
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16386)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16389)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #248
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16390)
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #66
	jsr delay_a_27_clocks
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
	lda #46
	jsr delay_a_27_clocks
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
	lda #32
	jsr delay_a_27_clocks
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
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16396)
	; 18 bytes
	php
	pha
	tya
	ldy #159
	pha
	lda #62
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16397)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #153
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16398)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16399)
	; 18 bytes
	php
	pha
	tya
	ldy #252
	pha
	lda #24
	jsr delay_a_27_clocks
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
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 16401)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #126
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 16402)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16403)
	; 18 bytes
	php
	pha
	tya
	ldy #127
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 16404)
	; 18 bytes
	php
	pha
	tya
	ldy #145
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16405)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #30
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
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
.if((n >= 16407) && (n < 16485))
; 16407 to 16484
.if(n = 16407)
	; 18 bytes
	php
	pha
	tya
	ldy #241
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16408 || n = 16412 || n = 16413 || n = 16415 || n = 16418 || n = 16420 || n = 16425 || n = 16433 || n = 16439)
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
	lda #93
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 16410)
	; 11 bytes
	php
	sta @zptemp
	lda #64
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 16411)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_11_clocks_b_
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16417 || n = 16422)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16421)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16423)
	; 11 bytes
	php
	pha
	php
	lda #64
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16424 || n = 16429 || n = 16437 || n = 16442)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16426 || n = 16431)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16427 || n = 16432)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16428)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16430)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16434)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16435)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16436 || n = 16438)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16440)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16441 || n = 16443 || n = 16444 || n = 16457 || n = 16467 || n = 16471)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16445 || n = 16446 || n = 16448 || n = 16449 || n = 16451 || n = 16453 || n = 16454 || n = 16456 || (n > 16457 && n < 16467) || (n > 16467 && n < 16471) || (n > 16471 && n < 16485))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16447)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16450)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16452)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16455)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16485 to 16562
.if((n > 16484 && n < 16527) || (n > 16527 && n < 16532) || (n > 16532 && n < 16563))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16527)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16532)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
; 16563 to 16640
.if((n > 16562 && n < 16572) || (n > 16572 && n < 16617))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16572)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16617)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-297)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16618)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16622)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16623)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16624)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16625)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16626)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16627)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16628)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16629)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16630)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16631)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16633)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/71
	pha
	lda #30
	jsr delay_a_27_clocks
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
	lda #62
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16635)
	; 18 bytes
	php
	pha
	tya
	ldy #248
	pha
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16636)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #76
	jsr delay_a_27_clocks
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
	lda #79
	jsr delay_a_27_clocks
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
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 16639)
	; 17 bytes
	pha
	sty @zptemp
	ldy #124
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 16640)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234
	lda #37
	jsr delay_a_27_clocks
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
	; 18 bytes
	pha
	tya
	ldy #241
	delay_n 64
	dey
	Jne *-10
	tay
	pla
.elseif(n = 16642)
	; 17 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 290
	dey
	Jne *-10
	tay
	pla
.elseif(n = 16643)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #53
	jsr delay_a_27_clocks
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
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16646)
	; 17 bytes
	pha
	sty @zptemp
	ldy #23
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	tay
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 16647)
	; 17 bytes
	sty @zptemp
	ldy #0
	delay_n 60
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 16648)
	; 18 bytes
	php
	pha
	tya
	ldy #241
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16649)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 287
	Jmi *-10
	ldy @zptemp
.elseif(n = 16650)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #241
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 2075
	Jne *-10
	pla
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
	lda #35
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	ldy #177
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16658)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #41
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldx @zptemp
	pla
.elseif(n = 16659)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #128
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
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
	lda #88
	jsr delay_a_27_clocks
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
	lda #251
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 16662)
	; 18 bytes
	php
	pha
	tya
	ldy #187
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16663)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #41
	jsr delay_a_27_clocks
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
.elseif(n = 16665)
	; 18 bytes
	php
	pha
	tya
	ldy #203
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16666)
	; 11 bytes
	php
	sta @zptemp
	lda #65
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 16667)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16668 || n = 16669 || n = 16671 || n = 16674 || n = 16676)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16670 || n = 16675)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16673 || n = 16678)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16677)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16679)
	; 11 bytes
	php
	pha
	php
	lda #65
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.endif
.else
; 16680 to 16718
.if(n = 16680 || n = 16685 || n = 16693 || n = 16698)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16681 || n = 16689 || n = 16695)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 16682 || n = 16687)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16683 || n = 16688)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16684)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16686)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16690)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16691)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16692 || n = 16694)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16696)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16697 || n = 16699 || n = 16700 || n = 16713)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16701 || n = 16702 || n = 16704 || n = 16705 || n = 16707 || n = 16709 || n = 16710 || n = 16712 || (n > 16713 && n < 16719))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16703)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16706)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16708)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16711)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 16719 to 16875
.if((n > 16718 && n < 16723) || (n > 16723 && n < 16727) || (n > 16727 && n < 16783) || (n > 16783 && n < 16788) || (n > 16788 && n < 16828) || (n > 16828 && n < 16873))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16723 || n = 16727 || n = 16788 || n = 16828)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16783)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16873)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-298)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16874)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16878)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16879)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16880)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16881)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16882)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16883)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16884)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16885)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16886)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16887)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16888)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 16889)
	; 17 bytes
	pha
	tya
	pha
	ldy #200 ;hides 'iny'
	php
	lda #255
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 16890)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16891)
	; 18 bytes
	php
	pha
	tya
	ldy #228
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16892)
	; 18 bytes
	php
	sty @zptemp
	ldy #194
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 16893)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #67
	jsr delay_a_27_clocks
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
	lda #34
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #143
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16896)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #241
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16897)
	; 18 bytes
	php
	pha
	tya
	ldy #194
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16898)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #228
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16899)
	; 18 bytes
	php
	pha
	tya
	ldy #211
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16900)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #84
	jsr delay_a_27_clocks
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
	lda #41
	jsr delay_a_27_clocks
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
	lda #162
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16903)
	; 18 bytes
	php
	pha
	tya
	ldy #252
	pha
	lda #$4A ;hides 'lsr a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16904)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16905)
	; 17 bytes
	pha
	sty @zptemp
	ldy #126
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 16906)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16907)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16908)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16909)
	; 18 bytes
	php
	sty @zptemp
	ldy #155
	pha
	lda #68
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
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
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16912)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #133
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16913)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #206
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16914)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #44
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
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
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16917)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #131
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 16918)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	php
	pha
	sty @zptemp
	ldy #131
	lda #95
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 16921)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #155
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16922)
	; 11 bytes
	php
	sta @zptemp
	lda #66
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 16923)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16924 || n = 16925 || n = 16927 || n = 16930 || n = 16932 || n = 16937 || n = 16945 || n = 16951)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16929 || n = 16934)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 16933)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16935)
	; 11 bytes
	php
	pha
	php
	lda #66
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16936 || n = 16941 || n = 16949)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16938 || n = 16943)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 16939 || n = 16944)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 16940)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16942)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16946)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16947)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16948 || n = 16950)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 16952)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16953)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 16954 to 17031
.if(n = 16954)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 16955 || n = 16956 || n = 16969 || n = 16979 || n = 16983)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16957 || n = 16958 || n = 16960 || n = 16961 || n = 16963 || n = 16965 || n = 16966 || n = 16968 || (n > 16969 && n < 16979) || (n > 16979 && n < 16983) || (n > 16983 && n < 17032))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16959)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16962)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16964)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16967)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 17032) && (n < 17110))
; 17032 to 17109
.if((n > 17031 && n < 17039) || (n > 17039 && n < 17044) || (n > 17044 && n < 17084) || (n > 17084 && n < 17110))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17039)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17044 || n = 17084)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17129)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-299)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17130)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17134)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17135)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17136)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17137)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17138)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17139)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17140)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17141)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17142)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #84
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 17143)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17145)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17146)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #46
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #173
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 18 bytes
	php
	sty @zptemp
	ldy #149
	pha
	lda #74
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 17150)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17151)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17152)
	; 18 bytes
	php
	sty @zptemp
	ldy #209
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 17153)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #65
	jsr delay_a_27_clocks
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
	lda #74
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #149
	lda #81
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17157)
	; 17 bytes
	pha
	tya
	ldy #126
	delay_n 130
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 17158)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #46
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #209
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17160)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/281
	pha
	lda #240
	jsr delay_a_27_clocks
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
	lda #81
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17162)
	; 17 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 296
	Jmi *-10
	ldy @zptemp
.elseif(n = 17163)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17164)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #209
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17165)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17166)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	lda #45
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #128
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 17172)
	; 17 bytes
	pha
	tya
	ldy #44
	delay_n 385
	dey
	Jne *-9
	tay
	pla
.elseif(n = 17173)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17174)
	; 18 bytes
	php
	pha
	tya
	ldy #235
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17175)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #133
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
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
	lda #122
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 17178)
	; 11 bytes
	php
	sta @zptemp
	lda #67
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 17179)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17180 || n = 17181 || n = 17183 || n = 17186)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
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
.if(n = 17188 || n = 17193 || n = 17201 || n = 17207)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17189)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17192 || n = 17197 || n = 17205 || n = 17210)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17194 || n = 17199)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17195 || n = 17200)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17196)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17198)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17202)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17203)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17204 || n = 17206)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17208)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17209 || n = 17211 || n = 17212 || n = 17225 || n = 17235 || n = 17239 || n = 17300 || n = 17340)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17213 || n = 17214 || n = 17216 || n = 17217 || n = 17219 || n = 17221 || n = 17222 || n = 17224 || (n > 17225 && n < 17235) || (n > 17235 && n < 17239) || (n > 17239 && n < 17295) || (n > 17295 && n < 17300) || (n > 17300 && n < 17340) || (n > 17340 && n < 17344))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17215)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17218)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17220)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17223)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17295)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #67
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17385)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-300)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17386)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17390)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17391)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17392)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17393)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17394)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17395)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17396)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17397)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17398)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17399)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17401)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #91
	jsr delay_a_27_clocks
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
	lda #62
	jsr delay_a_27_clocks
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
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17404)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 122
	Jne *-10
	ldy @zptemp
.elseif(n = 17405)
	; 17 bytes
	pha
	sty @zptemp
	ldy #16
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 17406)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #220
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17407)
	; 17 bytes
	sty @zptemp
	ldy #57
	delay_n 295
	dey
	Jpl *-9
	ldy @zptemp
.elseif(n = 17408)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #128
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #86
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
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
	lda #37
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #223
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17416)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #86
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 17417)
	; 17 bytes
	pha
	tya
	ldy #58
	delay_n 290
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 17418)
	; 18 bytes
	php
	pha
	tya
	ldy #137
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17419)
	; 18 bytes
	php
	pha
	tya
	ldy #232
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17420)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #44
	jsr delay_a_27_clocks
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
	lda #188
	jsr delay_a_27_clocks
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
	lda #170
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17423)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17424)
	; 18 bytes
	php
	pha
	tya
	ldy #58
	pha
	lda #254
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
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
	lda #93
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	tya
	ldy #128
	delay_n 130
	dey
	Jpl *-9
	tay
	pla
.elseif(n = 17428)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17429)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17430)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #42
	jsr delay_a_27_clocks
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
	ldx #67
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
	delay_n 125
	dey
	Jne *-10
	tay
	pla
.elseif(n = 17433)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #135
	ldx #232 ;hides 'inx'
	Jne *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
.elseif(n = 17434)
	; 11 bytes
	php
	sta @zptemp
	lda #68
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 17435)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17436 || n = 17437 || n = 17439)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 17441 to 17460
.if(n = 17441 || n = 17446)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17442 || n = 17444 || n = 17449 || n = 17457)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 17443)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17445)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17447)
	; 11 bytes
	php
	pha
	php
	lda #68
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17448 || n = 17453)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17450 || n = 17455)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17451 || n = 17456)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17452)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17454)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17458)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17459)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
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
	delay_n (n-13)
	brk 0
.elseif(n = 17462)
	; 14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17463)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17464)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17465 || n = 17467 || n = 17468 || n = 17481 || n = 17491 || n = 17495)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17469 || n = 17470 || n = 17472 || n = 17473 || n = 17475 || n = 17477 || n = 17478 || n = 17480 || (n > 17481 && n < 17491) || (n > 17491 && n < 17495) || (n > 17495 && n < 17501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17471)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17474)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17476)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17479)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.if((n > 17500 && n < 17551) || (n > 17551 && n < 17556) || (n > 17556 && n < 17596) || (n > 17596 && n < 17641))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17551)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17556 || n = 17596)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17641)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-301)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17642)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17646)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17647)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17648)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17649)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17650)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17651)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17652)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17653)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17654)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17655)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #202
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 17658)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17659)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 17660)
	; 18 bytes
	php
	sty @zptemp
	ldy #173
	pha
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 17661)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #252
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17662)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17663)
	; 18 bytes
	php
	sty @zptemp
	ldy #(n-14)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 17664)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #209
	jsr delay_a_27_clocks
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
	lda #61
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17666)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 82
	Jne *-10
	ldx @zptemp
.elseif(n = 17667)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #173
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17668)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17669)
	; 18 bytes
	php
	sty @zptemp
	ldy #165
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 17670)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #159
	lda #77
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17673)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #46
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldx @zptemp
	pla
.elseif(n = 17674)
	; 18 bytes
	php
	pha
	tya
	ldy #165
	pha
	lda #66
	jsr delay_a_27_clocks
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
	lda #77
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #165
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17677)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/81
	pha
	lda #40
	jsr delay_a_27_clocks
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
	lda #46
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
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
	lda #57
	jsr delay_a_27_clocks
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
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17682)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #45
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
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
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17685)
	; 17 bytes
	pha
	sty @zptemp
	ldy #136 ;hides 'dey'
	php
	lda #88
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldy @zptemp
	pla
.elseif(n = 17686)
	; 18 bytes
	php
	pha
	tya
	ldy #151
	pha
	lda #76
	jsr delay_a_27_clocks
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
	lda #60
	jsr delay_a_27_clocks
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
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17690)
	; 11 bytes
	php
	sta @zptemp
	lda #69
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 17691)
	; 9 bytes
	php
	pha
	lda #69
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17692 || n = 17693 || n = 17695)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17697 || n = 17702)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17698 || n = 17700 || n = 17705 || n = 17713 || n = 17719)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 17701)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17703)
	; 11 bytes
	php
	pha
	php
	lda #69
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17704 || n = 17709 || n = 17717 || n = 17722)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17706 || n = 17711)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17707 || n = 17712)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17708)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17710)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17714)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17715)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17716 || n = 17718)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17720)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17721 || n = 17723 || n = 17724)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17725 || n = 17726 || n = 17728 || n = 17729 || n = 17731 || n = 17733 || n = 17734)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17727)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17730)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17732)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17736 || (n > 17737 && n < 17747) || (n > 17747 && n < 17751) || (n > 17751 && n < 17807) || (n > 17807 && n < 17812))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17737 || n = 17747 || n = 17751 || n = 17812)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17807)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17852) || (n > 17852 && n < 17891))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17852)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17897)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-302)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17898)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17902)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17903)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17904)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17905)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17906)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17907)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17908)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17909)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 17910 to 17929
.if(n = 17910)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17911)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #165
	jsr delay_a_27_clocks
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
	lda #52
	jsr delay_a_27_clocks
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
	lda #55
	jsr delay_a_27_clocks
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
	lda #167
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17917)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #73
	jsr delay_a_27_clocks
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
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17919)
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #59
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17920)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/81
	pha
	lda #40
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #179
	lda #66
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17922)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #221
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17923)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17924)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #80
	jsr delay_a_27_clocks
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
	lda #105
	jsr delay_a_27_clocks
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
	lda #66
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17927)
	; 17 bytes
	sty @zptemp
	ldy #127
	delay_n 135
	dey
	Jpl *-9
	ldy @zptemp
.elseif(n = 17928)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #141
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17929)
	; 18 bytes
	php
	pha
	tya
	ldy #199
	pha
	lda #49
	jsr delay_a_27_clocks
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
	lda #166
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17931)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #199
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #93
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #169
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17936)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldx #61
	ldy #200 ;hides 'iny'
	Jmi *-1
	dex
	Jpl *-5
	ldy @zptemp
	tax
	pla
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
	lda #58
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	php
	sty @zptemp
	ldy #249
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 17943)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #206
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 17944)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #62
	lda #255
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
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
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17946)
	; 11 bytes
	php
	sta @zptemp
	lda #70
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 17947)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17948)
	; 12 bytes
	delay_n (n-2)
	nop
.endif
.else
; 17949 to 17968
.if(n = 17949 || n = 17951 || n = 17954 || n = 17956 || n = 17961)
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17953 || n = 17958)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 17957)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17959)
	; 11 bytes
	php
	pha
	php
	lda #70
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17960 || n = 17965)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17962 || n = 17967)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 17963 || n = 17968)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 17964)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17966)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.endif
.endif
.endif
.endif
.else
; 17969 to 18125
.if(n = 17969 || n = 17975)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17970)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17971)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17972 || n = 17974)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 17973 || n = 17978)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 17976)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17977 || n = 17979 || n = 17980 || n = 17993 || n = 18003 || n = 18007 || n = 18068 || n = 18108)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17981 || n = 17982 || n = 17984 || n = 17985 || n = 17987 || n = 17989 || n = 17990 || n = 17992 || (n > 17993 && n < 18003) || (n > 18003 && n < 18007) || (n > 18007 && n < 18063) || (n > 18063 && n < 18068) || (n > 18068 && n < 18108) || (n > 18108 && n < 18126))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17983)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17986)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17988)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17991)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18063)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #70
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18153)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-303)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18154)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18158)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18159)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18160)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18161)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18162)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18163)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18164)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
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
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18166)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18167)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18168)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 18169)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 234
	Jmi *-10
	ldx @zptemp
.elseif(n = 18170)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #38
	jsr delay_a_27_clocks
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
	lda #61
	jsr delay_a_27_clocks
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
	lda #52
	jsr delay_a_27_clocks
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
	lda #229
	jsr delay_a_27_clocks
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
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18176)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #198
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 18177)
	; 17 bytes
	sty @zptemp
	ldy #45
	delay_n 390
	dey
	Jpl *-9
	ldy @zptemp
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
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18180)
	; 17 bytes
	pha
	sty @zptemp
	ldy #62
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 18181)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #198
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #66
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
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
	lda #64
	jsr delay_a_27_clocks
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
	lda #46
	jsr delay_a_27_clocks
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
	jsr delay_256a_11_clocks_b_
	plp
	Jmi *-8
	pla
	tay
	pla
.elseif(n = 18188)
	; 18 bytes
	php
	sty @zptemp
	ldy #233
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 18189)
	; 18 bytes
	php
	pha
	tya
	ldy #230
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18190)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #239
	lda #42
	jsr delay_a_27_clocks
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
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18192)
	; 18 bytes
	php
	sty @zptemp
	ldy #149
	pha
	lda #81
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 18193)
	; 18 bytes
	php
	pha
	tya
	ldy #233
	pha
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18194)
	; 18 bytes
	php
	sty @zptemp
	ldy #202
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
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
	lda #45
	jsr delay_a_27_clocks
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
	lda #81
	jsr delay_a_27_clocks
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
	lda #43
	jsr delay_a_27_clocks
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
	lda #49
	jsr delay_a_27_clocks
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
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18201)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #202
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18202)
	; 11 bytes
	php
	sta @zptemp
	lda #71
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 18203)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.else
; 18204 to 18281
.if(n = 18204 || n = 18205 || n = 18207 || n = 18210 || n = 18212 || n = 18217 || n = 18225 || n = 18231)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18209 || n = 18214)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18213)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18215)
	; 11 bytes
	php
	pha
	php
	lda #71
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18216 || n = 18221 || n = 18229 || n = 18234)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18218 || n = 18223)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18219 || n = 18224)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18220)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18222)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18226)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18227)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18228 || n = 18230)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18232)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18233 || n = 18235 || n = 18236 || n = 18249 || n = 18259 || n = 18263)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18237 || n = 18238 || n = 18240 || n = 18241 || n = 18243 || n = 18245 || n = 18246 || n = 18248 || (n > 18249 && n < 18259) || (n > 18259 && n < 18263) || (n > 18263 && n < 18282))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18239)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18242)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18244)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18247)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 18282) && (n < 18360))
; 18282 to 18359
.if((n > 18281 && n < 18319) || (n > 18319 && n < 18324) || (n > 18324 && n < 18360))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18319)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18324)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18364) || (n > 18364 && n < 18399))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18364)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18409)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-304)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18410)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18414)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18415)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18416)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18417)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18419)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18420)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18421)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18422)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18423)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #134
	jsr delay_a_27_clocks
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
	lda #46
	jsr delay_a_27_clocks
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
	lda #37
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18428)
	; 17 bytes
	pha
	sty @zptemp
	ldy #18
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jne *-7
	ldy @zptemp
	pla
.elseif(n = 18429)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #239
	lda #43
	jsr delay_a_27_clocks
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
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18431)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #69
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18432)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #141
	jsr delay_a_27_clocks
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
	lda #44
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #198
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18436)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 18437)
	; 18 bytes
	php
	sty @zptemp
	ldy #207
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
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
	; 18 bytes
	php
	pha
	tya
	ldy #163
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18439)
	; 18 bytes
	php
	sty @zptemp
	ldy #66
	pha
	lda #234
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	ldy @zptemp
	plp
.elseif(n = 18440)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #198
	lda #59
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
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
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18443)
	; 18 bytes
	php
	pha
	tya
	ldy #196
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18444)
	; 18 bytes
	php
	pha
	tya
	ldy #66
	pha
	lda #234
	jsr delay_a_27_clocks
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
	lda #79
	jsr delay_a_27_clocks
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
	lda #170
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18447)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #75
	jsr delay_a_27_clocks
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
	lda #88
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #47
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
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
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18453)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18454)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #68
	lda #237
	jsr delay_a_27_clocks
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
	lda #61
	jsr delay_a_27_clocks
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
	; 18 bytes
	pha
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
	pla
.elseif(n = 18458)
	; 11 bytes
	php
	sta @zptemp
	lda #72
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 18459)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18460 || n = 18461 || n = 18463 || n = 18466 || n = 18468 || n = 18473)
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18465 || n = 18470)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18469)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18471)
	; 11 bytes
	php
	pha
	php
	lda #72
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18472)
	; 11 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18474)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
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
.if(n = 18477 || n = 18485 || n = 18490)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18478)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18479)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18480)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18481 || n = 18487)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 18482)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18483)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18484 || n = 18486)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18488)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18489 || n = 18491 || n = 18492 || n = 18505 || n = 18515)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18493 || n = 18494 || n = 18496 || n = 18497 || n = 18499 || n = 18501 || n = 18502 || n = 18504 || (n > 18505 && n < 18515))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18495)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18498)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18500)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18503)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 18516 to 18593
.if((n > 18515 && n < 18519) || (n > 18519 && n < 18575) || (n > 18575 && n < 18580) || (n > 18580 && n < 18594))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18519 || n = 18580)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18575)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18620) || (n > 18620 && n < 18665))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18620)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18665)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-305)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18666)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18670)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18671)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
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
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18673)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18674)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18675)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18676)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18677)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18678)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18679)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18680)
	; 17 bytes
	pha
	delay_n (n-3)
.elseif(n = 18681)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 87
	Jne *-10
	ldx @zptemp
.elseif(n = 18682)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #54
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #217
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18684)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18685)
	; 18 bytes
	php
	pha
	tya
	ldy #183
	pha
	lda #61
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #183
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18688)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #51
	jsr delay_a_27_clocks
	plp
	Jne *-8
	ldx @zptemp
	pla
.elseif(n = 18689)
	; 18 bytes
	php
	sty @zptemp
	ldy #249
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
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
	lda #68
	jsr delay_a_27_clocks
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
	lda #51
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tax
	pla
.elseif(n = 18694)
	; 18 bytes
	php
	pha
	tya
	ldy #249
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18695)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #147
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18696)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #249
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18697)
	; 18 bytes
	php
	pha
	tya
	ldy #65
	pha
	lda #242
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 18698)
	; 18 bytes
	php
	sty @zptemp
	ldy #173
	pha
	lda #67
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 18699)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #70
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 18700)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18701)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 241
	Jmi *-10
	ldx @zptemp
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
	lda #67
	jsr delay_a_27_clocks
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
	lda #249
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 18708)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #205
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 18709)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18710)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #74
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18712)
	; 18 bytes
	php
	pha
	tya
	ldy #201
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18713)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #205
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	pla
	tax
	pla
.elseif(n = 18714)
	; 11 bytes
	php
	sta @zptemp
	lda #73
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 18715)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18716 || n = 18717 || n = 18719 || n = 18722 || n = 18724 || n = 18729 || n = 18737 || n = 18743)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18721 || n = 18726)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18725)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18727)
	; 11 bytes
	php
	pha
	php
	lda #73
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18728 || n = 18733 || n = 18741 || n = 18746)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18730 || n = 18735)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18731 || n = 18736)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18732)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18734)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18738)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18739)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18740 || n = 18742)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 18744)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18745 || n = 18747 || n = 18748)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18749 || n = 18750)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.if(n = 18751)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18752 || n = 18753 || n = 18755 || n = 18757 || n = 18758 || n = 18760 || (n > 18761 && n < 18771) || (n > 18771 && n < 18775) || (n > 18775 && n < 18831) || (n > 18831 && n < 18836) || (n > 18836 && n < 18876) || (n > 18876 && n < 18907))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18754)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18756)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18759)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18761 || n = 18771 || n = 18775 || n = 18836 || n = 18876)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18831)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
.if((n >= 18907) && (n < 18926))
; 18907 to 18925
.if((n > 18906 && n < 18921))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18921)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-306)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18922)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18923)
	; 15 bytes
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
; 18926 to 18945
.if(n = 18926)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18927)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18928)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18929)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18930)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18931)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18932)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18933)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
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
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18936)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #208
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 18937)
	; 18 bytes
	php
	sty @zptemp
	ldy #149
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 18938)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #62
	jsr delay_a_27_clocks
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
	lda #45
	jsr delay_a_27_clocks
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
	lda #64
	jsr delay_a_27_clocks
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
	lda #208
	jsr delay_a_27_clocks
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
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18943)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/76
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18944)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #184
	jsr delay_a_27_clocks
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
	lda #139
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.endif
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
	lda #52
	jsr delay_a_27_clocks
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
	lda #85
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18948)
	; 18 bytes
	php
	sty @zptemp
	ldy #56 ;hides 'sec'
	pha
	lda #220
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	ldy @zptemp
	plp
.elseif(n = 18949)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #93
	jsr delay_a_27_clocks
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
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18951)
	; 18 bytes
	php
	sty @zptemp
	ldy #234 ;hides 'nop'
	pha
	lda #134
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	ldy @zptemp
	plp
.elseif(n = 18952)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #146
	jsr delay_a_27_clocks
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
	lda #220
	jsr delay_a_27_clocks
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
	lda #57
	jsr delay_a_27_clocks
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
	lda #222
	jsr delay_a_27_clocks
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
	lda #134
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #72
	lda #229
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 18958)
	; 18 bytes
	php
	pha
	tya
	ldy #177
	pha
	lda #66
	jsr delay_a_27_clocks
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
	lda #171
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18960)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #227
	jsr delay_a_27_clocks
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
	lda #229
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18963)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 18964)
	; 18 bytes
	php
	sty @zptemp
	ldy #184 ;hides 'clv'
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	ldy @zptemp
	plp
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
	lda #73
	jsr delay_a_27_clocks
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
	lda #41
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 18967)
	; 17 bytes
	sty @zptemp
	ldy #47
	delay_n 390
	dey
	Jpl *-9
	ldy @zptemp
.elseif(n = 18968)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #43
	jsr delay_a_27_clocks
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
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 18970)
	; 11 bytes
	php
	sta @zptemp
	lda #74
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 18971)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18972 || n = 18973 || n = 18975 || n = 18978 || n = 18980)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18977 || n = 18982)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 18981)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18983)
	; 11 bytes
	php
	pha
	php
	lda #74
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18984)
	; 11 bytes
	delay_n (n-13)
	brk 0
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985 || n = 18993 || n = 18999)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 18986 || n = 18991)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 18987 || n = 18992)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 18988)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18989 || n = 18997 || n = 19002)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 18990)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18994)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18995)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18996 || n = 18998)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19000)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19001 || n = 19003 || n = 19004 || n = 19017 || n = 19027 || n = 19031)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19005 || n = 19006 || n = 19008 || n = 19009 || n = 19011 || n = 19013 || n = 19014 || n = 19016 || (n > 19017 && n < 19027) || (n > 19027 && n < 19031) || (n > 19031 && n < 19063))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19007)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19010)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19012)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19015)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 19063) && (n < 19219))
.if((n >= 19063) && (n < 19141))
; 19063 to 19140
.if((n > 19062 && n < 19087) || (n > 19087 && n < 19092) || (n > 19092 && n < 19132) || (n > 19132 && n < 19141))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19087)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19092 || n = 19132)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19177)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-307)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19178)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19182)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19183)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19184)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19185)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19186)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19187)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19188)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19189)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19190)
	; 17 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	pla
	plp
	Jne *-10
	tay
	pla
.elseif(n = 19191)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #164
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19194)
	; 18 bytes
	php
	pha
	tya
	ldy #64
	pha
	lda #254
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 19195)
	; 18 bytes
	php
	pha
	tya
	ldy #204
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19196)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #56
	jsr delay_a_27_clocks
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
	lda #99
	jsr delay_a_27_clocks
	plp
	Jne *-8
	pla
	tay
	pla
.elseif(n = 19198)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #151
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19200)
	; 18 bytes
	php
	sty @zptemp
	ldy #181
	pha
	lda #65
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 19201)
	; 18 bytes
	php
	pha
	tya
	ldy #139
	pha
	lda #97
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19202)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #204
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19203)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #93
	jsr delay_a_27_clocks
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
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19205)
	; 18 bytes
	php
	pha
	tya
	ldy #181
	pha
	lda #65
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19206)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #240
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19207)
	; 17 bytes
	sty @zptemp
	ldy #63
	delay_n 295
	dey
	Jpl *-9
	ldy @zptemp
.elseif(n = 19208)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #244
	jsr delay_a_27_clocks
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
	lda #54
	jsr delay_a_27_clocks
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
	lda #54
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #202
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #72
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	ldy @zptemp
	plp
.elseif(n = 19214)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19215)
	; 18 bytes
	php
	sty @zptemp
	ldy #211
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 19216)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19217)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 19218)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #49
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jpl *-5
	ldy @zptemp
	tax
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
; 19219 to 19257
.if(n = 19219)
	; 18 bytes
	php
	pha
	tya
	ldy #150
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
.elseif(n = 19220)
	; 18 bytes
	php
	pha
	tya
	ldy #211
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19221)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #0
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 19222)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/111
	pha
	lda #70
	jsr delay_a_27_clocks
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
	lda #(n-280)/256
	jsr delay_a_27_clocks
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
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 19226)
	; 11 bytes
	php
	sta @zptemp
	lda #75
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 19227)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19228 || n = 19229 || n = 19231 || n = 19234 || n = 19236 || n = 19241 || n = 19249 || n = 19255)
	; 10..14 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19233 || n = 19238)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 19237)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19239)
	; 11 bytes
	php
	pha
	php
	lda #75
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19240 || n = 19245 || n = 19253)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19242 || n = 19247)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19243 || n = 19248)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19244)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19246)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19250)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19251)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19252 || n = 19254)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19256)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19257)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 19258 to 19296
.if(n = 19258)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19259 || n = 19260 || n = 19273 || n = 19283 || n = 19287)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19261 || n = 19262 || n = 19264 || n = 19265 || n = 19267 || n = 19269 || n = 19270 || n = 19272 || (n > 19273 && n < 19283) || (n > 19283 && n < 19287) || (n > 19287 && n < 19297))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19263)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19266)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19268)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19271)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 19297 to 19375
.if((n > 19296 && n < 19343) || (n > 19343 && n < 19348) || (n > 19348 && n < 19376))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19343)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19348)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
.if((n > 19375 && n < 19388) || (n > 19388 && n < 19433))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19388)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19433)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-308)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19434)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19438)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19439)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19440)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19441)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19442)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19443)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19444)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19445)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19446)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19447)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19449)
	; 18 bytes
	php
	pha
	tya
	ldy #145
	pha
	lda #93
	jsr delay_a_27_clocks
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
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19451)
	; 17 bytes
	pha
	sty @zptemp
	ldy #18
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	dey
	Jpl *-7
	ldy @zptemp
	pla
.elseif(n = 19452)
	; 17 bytes
	pha
	tya
	ldy #144
	delay_n 130
	dey
	Jne *-9
	tay
	pla
.elseif(n = 19453)
	; 18 bytes
	php
	pha
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
	lda #74
	jsr delay_a_27_clocks
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
	lda #45
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
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
	lda #43
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 19459)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 137
	Jne *-10
	ldy @zptemp
.elseif(n = 19460)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19461)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 251
	Jmi *-10
	ldx @zptemp
.elseif(n = 19462)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #52
	jsr delay_a_27_clocks
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
	lda #59
	jsr delay_a_27_clocks
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
	lda #144
	jsr delay_a_27_clocks
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
	; 17 bytes
	sty @zptemp
	ldy #139
	delay_n 135
	dey
	Jne *-9
	ldy @zptemp
.elseif(n = 19468)
	; 17 bytes
	pha
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	lda #215
	jsr delay_a_27_clocks
	plp
	Jmi *-8
	ldx @zptemp
	pla
.elseif(n = 19469)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #221
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 19470)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19471)
	; 17 bytes
	pha
	tya
	pha
	ldy #136 ;hides 'dey'
	php
	lda #101
	jsr delay_a_27_clocks
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
	lda #215
	jsr delay_a_27_clocks
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
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19475)
	; 18 bytes
	php
	pha
	tya
	ldy #152
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
.elseif(n = 19476)
	; 18 bytes
	php
	sty @zptemp
	ldy #73
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	ldy @zptemp
	plp
.elseif(n = 19477)
	; 18 bytes
	php
	pha
	tya
	ldy #207
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19478)
	; 18 bytes
	pha
	txa
	sty @zptemp
	ldx #34
	ldy #110
	dey
	Jne *-1
	dex
	Jpl *-6
	ldy @zptemp
	tax
	pla
.elseif(n = 19479)
	; 18 bytes
	php
	pha
	tya
	ldy #140
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
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
	lda #240
	jsr delay_a_27_clocks
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
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 19482)
	; 11 bytes
	php
	sta @zptemp
	lda #76
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 19483)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19484 || n = 19485 || n = 19487 || n = 19490 || n = 19492)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19486 || n = 19491)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19488)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_16_clocks_b
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
.if(n = 19493)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19496 || n = 19501 || n = 19509 || n = 19514)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19497 || n = 19505 || n = 19511)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 19498 || n = 19503)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19499 || n = 19504)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19500)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19502)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19506)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19507)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19508 || n = 19510)
	; 13..14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19512)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19513 || n = 19515 || n = 19516 || n = 19529)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19517 || n = 19518 || n = 19520 || n = 19521 || n = 19523 || n = 19525 || n = 19526 || n = 19528 || n = 19530 || n = 19531)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19519)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19522)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19524)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19527)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 19532 to 19687
.if((n > 19531 && n < 19539) || (n > 19539 && n < 19543) || (n > 19543 && n < 19599) || (n > 19599 && n < 19604) || (n > 19604 && n < 19644) || (n > 19644 && n < 19688))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19539 || n = 19543 || n = 19604 || n = 19644)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19599)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #76
	jsr delay_256a_16_clocks_b
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19689)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-309)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19690)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19694)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19695)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19696)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19697)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19698)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19699)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19700)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19701)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19702)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19703)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19705)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	lda #61
	jsr delay_a_27_clocks
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
	lda #46
	jsr delay_a_27_clocks
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
	; 18 bytes
	php
	pha
	tya
	ldy #214
	pha
	lda #51
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19708)
	; 17 bytes
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	php
	lda #56
	jsr delay_a_27_clocks
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
	lda #47
	jsr delay_a_27_clocks
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
	ldy #148
	lda #99
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
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
	lda #68
	jsr delay_a_27_clocks
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
	lda #45
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19714)
	; 18 bytes
	php
	pha
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
	pla
	plp
.elseif(n = 19715)
	; 18 bytes
	php
	pha
	sty @zptemp
	ldy #229
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 19716)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #76
	jsr delay_a_27_clocks
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
	lda #63
	jsr delay_a_27_clocks
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
	lda #193
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 19719)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #59
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19720)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #52
	jsr delay_a_27_clocks
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
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19722)
	; 17 bytes
	pha
	tya
	ldy #146
	delay_n 130
	dey
	Jne *-9
	tay
	pla
.elseif(n = 19723)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19724)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #201
	lda #64
	jsr delay_a_27_clocks
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
	lda #77
	jsr delay_a_27_clocks
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
	lda #65
	jsr delay_a_27_clocks
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
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19730)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #131
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19733)
	; 17 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 139
	Jne *-10
	ldy @zptemp
.elseif(n = 19734)
	; 18 bytes
	php
	sty @zptemp
	ldy #232
	pha
	lda #44
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 19735)
	; 18 bytes
	php
	pha
	tya
	ldy #212
	pha
	lda #52
	jsr delay_a_27_clocks
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
	lda #56
	jsr delay_a_27_clocks
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
	lda #42
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 19738)
	; 11 bytes
	php
	sta @zptemp
	lda #77
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 19739)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19740 || n = 19741 || n = 19743)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19745)
	; 11 bytes
	delay_n 6
	delay_n (n-6)
.endif
.else
; 19746 to 19765
.if(n = 19746 || n = 19748 || n = 19753 || n = 19761)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 19747)
	; 10 bytes
	pha
	delay_n (n-3)
.elseif(n = 19749)
	; 11 bytes
	php
	pha
	inc @zptemp
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19752 || n = 19757 || n = 19765)
	; 11..13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19754 || n = 19759)
	; 12 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 19755 || n = 19760)
	; 12 bytes
	delay_n 16
	delay_n (n-16)
.elseif(n = 19756)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19758)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19762)
	; 13 bytes
	php
	pha
	inc @zptemp
	brk 0
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19763)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19764)
	; 13 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.endif
.else
; 19766 to 19843
.if(n = 19766)
	; 14 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 19767)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 19768)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19769 || n = 19771 || n = 19772 || n = 19785 || n = 19795 || n = 19799)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19770)
	; 13 bytes
	delay_n (n-13)
	brk 0
.elseif(n = 19773 || n = 19774 || n = 19776 || n = 19777 || n = 19779 || n = 19781 || n = 19782 || n = 19784 || (n > 19785 && n < 19795) || (n > 19795 && n < 19799) || (n > 19799 && n < 19844))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19775)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-53) ;hides 'asl a'
	Jcc *-1
	lda #>(n-53)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19778)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19780)
	; 14 bytes
	php
	pha
	nop
	lda #<(n+6) ;hides 'lsr a'
	Jne *-1
	lda #>(n+6)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19783)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 19844) && (n < 19922))
; 19844 to 19921
.if((n > 19843 && n < 19855) || (n > 19855 && n < 19860) || (n > 19860 && n < 19900) || (n > 19900 && n < 19922))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19855)
	; 14 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19860 || n = 19900)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
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
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19945)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-310)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19946)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
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
	lda #169
	jsr delay_a_27_clocks
	lda #(n-237)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19950)
	; 16 bytes
	php
	pha
	php
	lda #170
	jsr delay_a_27_clocks
	lda #(n-238)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19951)
	; 16 bytes
	php
	pha
	php
	lda #171
	jsr delay_a_27_clocks
	lda #(n-239)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19952)
	; 16 bytes
	php
	delay_n 16
	lda #166
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19953)
	; 16 bytes
	php
	delay_n 16
	lda #167
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19954)
	; 16 bytes
	php
	delay_n 16
	lda #168
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19955)
	; 16 bytes
	php
	delay_n 16
	lda #169
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19956)
	; 16 bytes
	php
	delay_n 16
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19957)
	; 16 bytes
	php
	delay_n 16
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19958)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19959)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_15cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
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
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19962)
	; 17 bytes
	pha
	stx @zptemp
	ldx #232 ;hides 'inx'
	php
	lda #3
	jsr delay_256a_16_clocks_b
	plp
	Jne *-8
	ldx @zptemp
	pla
.elseif(n = 19963)
	; 18 bytes
	php
	pha
	tya
	ldy #72
	pha
	lda #236
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
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
	lda #68
	jsr delay_a_27_clocks
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
	jsr delay_256a_16_clocks_b
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
	lda #84
	jsr delay_a_27_clocks
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
	lda #243
	jsr delay_a_27_clocks
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
	lda #45
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #70
	lda #247
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	ldy @zptemp
	pla
	plp
.elseif(n = 19973)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #75
	jsr delay_a_27_clocks
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
	lda #97
	jsr delay_a_27_clocks
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
	lda #163
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19976)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #210
	lda #61
	jsr delay_a_27_clocks
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
	lda #247
	jsr delay_a_27_clocks
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
	sty @zptemp
	ldy #165
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
	plp
.elseif(n = 19980)
	; 18 bytes
	php
	sty @zptemp
	ldy #149
	pha
	lda #93
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	ldy @zptemp
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
	lda #169
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19982)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #122
	jsr delay_a_27_clocks
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
	lda #51
	jsr delay_a_27_clocks
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
	lda #80
	jsr delay_a_27_clocks
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
	lda #93
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19986)
	; 17 bytes
	pha
	txa
	sty @zptemp
	ldy #52
	ldx #202 ;hides 'dex'
	Jmi *-1
	dey
	Jne *-5
	ldy @zptemp
	tax
	pla
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
	lda #129
	jsr delay_a_27_clocks
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
	; 17 bytes
	pha
	tya
	ldy #148
	delay_n 130
	dey
	Jne *-9
	tay
	pla
.elseif(n = 19993)
	; 17 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 258
	Jmi *-10
	ldx @zptemp
.elseif(n = 19994)
	; 11 bytes
	php
	sta @zptemp
	lda #78
	jsr delay_256a_11_clocks_b_
	lda @zptemp
	plp
.elseif(n = 19995)
	; 9 bytes
	php
	pha
	lda #78
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19996 || n = 19997 || n = 19999)
	; 10..12 bytes
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
	jsr delay_256a_16_clocks_b
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
