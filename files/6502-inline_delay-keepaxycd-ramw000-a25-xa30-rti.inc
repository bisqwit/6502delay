; delay_n, delays n cycles with small code
; Will clobber Z+N, and V
; Will preserve A, X, Y, S, C, D, and I (interrupt-disable flag)
;
; WARNING: May write garbage into $00-$FF.
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 7
; - 3 bytes or less when n <= 9
; - 4 bytes or less when n <= 20
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 33
; - 7 bytes or less when n <= 35
; - 8 bytes or less when n <= 46
; - 9 bytes or less when n <= 297
; -10 bytes or less when n <= 310
; -11 bytes or less when n <= 1312
; -12 bytes or less when n <= 2350
; -13 bytes or less when n <= 3154
; -14 bytes or less when n <= 4640
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.3, max=4
; - When 11 <= n <= 20: min=2, avg=3.5, max=4
; - When 21 <= n <= 30: min=4, avg=5.3, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=8, avg=8.8, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.1, max=11
; - When 501 <= n <= 750: min=8, avg=10.2, max=11
; - When 751 <= n <= 1000: min=9, avg=10.2, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.6, max=12
; - When 2001 <= n <= 3000: min=10, avg=11.6, max=13
; - When 3001 <= n <= 5000: min=10, avg=12.5, max=15
; - When 5001 <= n <= 7500: min=11, avg=14.1, max=15
; - When 7501 <= n <= 10000: min=12, avg=14.9, max=15
; - When 10001 <= n <= 20000: min=14, avg=15.0, max=15
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
; 1 to 78
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 2 bytes
	bit $24
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 28 || n = 34 || n = 41 || n = 43)
	; 2..8 bytes
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
.elseif(n = 9 || n = 11 || (n > 19 && n < 24) || n = 33 || n = 35 || n = 37 || n = 46)
	; 3..8 bytes
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
.elseif(n = 13)
	; 2 bytes
	brk
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 31 || n = 44)
	; 4..8 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 19 || n = 26 || n = 32 || n = 39 || n = 45)
	; 4..8 bytes
	delay_n (n-13)
	brk
.elseif(n = 24 || n = 25 || n = 38)
	; 5..7 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 27 || n = 40)
	; 5..7 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 29 || n = 30)
	; 6 bytes
	delay_n (n-15)
	jsr @nearby_rts_15cyc
.elseif(n = 36)
	; 8 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 42)
	; 8 bytes
	delay_n (n-3)
	bit $24
.elseif((n > 46 && n < 52) || n = 54 || (n > 55 && n < 60) || (n > 60 && n < 79))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 52)
	; 8 bytes
	.repeat 4
	 brk
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
.endif
.else
; 79 to 156
.if((n > 78 && n < 130) || (n > 130 && n < 135) || (n > 135 && n < 157))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.if((n > 156 && n < 290) || (n > 290 && n < 295) || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.elseif(n = 297 || n = 312)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 298 || n = 304 || n = 309 || n = 311)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 299)
	; 10 bytes
	pha
	tya
	ldy #(n-19)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 300)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 301 || n = 308)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
	brk
	delay_n (n-13)
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
.elseif(n = 314 || n = 316 || n = 319 || n = 321 || n = 324 || n = 326 || n = 329 || n = 331 || n = 338 || n = 339 || n = 344 || n = 346 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 361 || n = 364 || n = 366 || n = 373 || n = 374 || n = 379 || n = 381 || n = 384 || n = 386 || n = 389)
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
.elseif(n = 325 || n = 330 || n = 360 || n = 375)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
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
.elseif(n = 335 || n = 340 || n = 345 || n = 365 || n = 370 || n = 380)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
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
.elseif(n = 350 || n = 371)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
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
.elseif(n = 385)
	; 8 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jmi *-1
	ldx @zptemp
.elseif(n = 388)
	; 10 bytes
	stx @zptemp
	ldx $A6
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
.elseif(n = 393)
	; 10 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
.elseif(n = 394 || n = 399 || n = 401 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 443 || n = 444 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 395)
	; 10 bytes
	delay_n (n-5)
	inc @zptemp
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
	brk
	delay_n (n-13)
.elseif(n = 400 || n = 405 || n = 415 || n = 430 || n = 440 || n = 445 || n = 450)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
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
.elseif(n = 410 || n = 435 || n = 465)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
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
.elseif(n = 413 || n = 420 || n = 455)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 423)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	ldx @zptemp
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
.elseif(n = 463)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
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
.if(n = 469 || n = 471 || n = 479 || n = 484 || n = 486 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 513 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 480 || n = 505 || n = 515 || n = 520 || n = 535 || n = 540 || n = 545)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
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
.elseif(n = 475 || n = 485 || n = 500 || n = 510)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 476 || n = 518 || n = 525 || n = 546)
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
.elseif(n = 503 || n = 533)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.elseif(n = 538)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 549 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 571 || n = 574 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 609 || n = 611 || n = 619 || n = 624)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 550 || n = 555 || n = 570 || n = 575 || n = 610 || n = 615 || n = 620)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
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
.elseif((n .mod 5 = 3) && ((n > 562 && n < 579) || (n > 597 && n < 614)))
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
.elseif(n = 580 || n = 585 || n = 590 || n = 605 || n = 625)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 583)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
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
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 688 || n = 689 || n = 694 || n = 699)
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
.elseif(n = 630 || n = 651 || n = 665 || n = 686 || n = 700)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 633)
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
.elseif(n = 645 || n = 675 || n = 680 || n = 685)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
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
.elseif(n = 650 || n = 655 || n = 660)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
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
	bit $24
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
.elseif(n = 693)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
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
	inc $F6,X
	delay_n (n-6)
.elseif(n = 703)
	; 10 bytes
	brk
	delay_n (n-13)
.endif
.else
; 704 to 781
.if(n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 724 || n = 729 || n = 731 || n = 734 || n = 736 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 758 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
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
	brk
	delay_n (n-13)
.elseif((n .mod 5 = 0) && ((n > 709 && n < 731) || n = 760 || n = 765 || n = 780))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
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
.elseif(n = 745 || n = 766)
	; 11 bytes
	php
	delay_n (n-7)
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
.elseif(n = 750)
	; 10 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jpl *-3
	ldy @zptemp
.elseif(n = 754)
	; 9 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	dey
	Jmi *-2
	ldy @zptemp
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
.elseif(n = 783 || n = 818 || n = 848 || n = 878 || n = 893 || n = 913 || n = 918 || n = 923 || n = 928)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 784 || n = 793 || n = 794 || n = 799 || n = 801 || n = 804 || n = 806 || n = 809 || n = 811 || n = 814 || n = 819 || n = 821 || n = 829 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 863 || n = 864 || n = 869 || n = 871 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 933 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 801) || (n > 814 && n < 836) || (n > 849 && n < 871) || (n > 884 && n < 901) || (n > 919 && n < 936)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 786 || n = 813 || n = 858)
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
.elseif(n = 791 || n = 826 || n = 840 || n = 931)
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 798 || n = 805 || n = 833 || n = 861 || n = 868 || n = 875 || n = 896 || n = 903)
	; 10 bytes
	sty @zptemp
	ldy #n/7-2
	nop
	dey
	Jpl *-2
	ldy @zptemp
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
.elseif(n = 823)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 853 || n = 898)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 883)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
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
.elseif(n = 939 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 991 || n = 994 || n = 1003 || n = 1004 || n = 1009 || n = 1011 || n = 1014)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 940 || (n > 954 && n < 976) || (n > 989 && n < 1011)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
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
.if(n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1029 || n = 1039 || n = 1044 || n = 1046 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1073 || n = 1074 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
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
.elseif(n = 1023)
	; 10 bytes
	stx @zptemp
	ldx $A6
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
.elseif(n = 1028)
	; 10 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
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
	brk
	delay_n (n-13)
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
.elseif((n .mod 5 = 0) && ((n > 1034 && n < 1046) || (n > 1059 && n < 1081)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
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
.elseif(n = 1053 || n = 1088 || n = 1093)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
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
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1099 || n = 1101 || n = 1109 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1143 || n = 1144 || n = 1149 || n = 1151 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1178 || n = 1179 || n = 1184 || n = 1186 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1116) || (n > 1129 && n < 1151) || (n > 1164 && n < 1186) || (n > 1199 && n < 1221) || (n > 1234 && n < 1251)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
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
.elseif((n .mod 5 = 3) && (n = 1098 || n = 1103 || n = 1133 || n = 1138 || n = 1158 || n = 1163 || (n > 1192 && n < 1209) || n = 1233))
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.elseif((n .mod 7 = 0) && ((n > 1105 && n < 1121) || (n > 1140 && n < 1156) || (n > 1175 && n < 1191) || (n > 1210 && n < 1226) || n = 1246))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1108 || n = 1228)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1123 || n = 1168)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 1213)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
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
.endif
.else
.if((n >= 1251) && (n < 1876))
.if((n >= 1251) && (n < 1563))
.if((n >= 1251) && (n < 1407))
.if((n >= 1251) && (n < 1329))
; 1251 to 1328
.if((n .mod 7 = 5) && ((n > 1250 && n < 1266) || n = 1279 || n = 1286 || (n > 1299 && n < 1329)))
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
.elseif((n .mod 7 = 0) && (n = 1253 || n = 1260 || (n > 1280 && n < 1324)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1254 || n = 1256 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1274 || n = 1276 || n = 1283 || n = 1284 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1304 || n = 1311 || n = 1313 || n = 1318 || n = 1325)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 1255 || (n > 1269 && n < 1291)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
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
.elseif(n = 1293)
	; 9 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1301)
	; 11 bytes
	delay_n (n-3)
	bit $24
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
.elseif(n = 1310)
	; 11 bytes
	pha
	tya
	ldy #(n-23)/11
	inc $F6,X
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1312)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1315 || n = 1327)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $93A0'
	ldy #147
	dey
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
.elseif(n = 1331 || n = 1345)
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
.elseif(n = 1336)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	nop
	iny
	Jpl *-5
	ldy @zptemp
.elseif(n = 1338)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $6CA0'
	ldy #108
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1339 || n = 1357 || n = 1366)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1343 || n = 1359)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1346 || n = 1350 || n = 1360 || n = 1367)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1347)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $6BA0'
	ldy #107
	iny
	Jne *-4
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
.elseif(n = 1354)
	; 11 bytes
	pha
	tya
	ldy #(n-23)/11
	inc $F6,X
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1355)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $6AA0'
	ldy #106
	iny
	Jne *-4
	ldy @zptemp
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
.elseif((n .mod 7 = 0) && ((n > 1371 && n < 1401)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1373)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk
	Jmi *-3
	ldx @zptemp
.elseif(n = 1374 || n = 1381 || n = 1395)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1375 || n = 1402)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1376)
	; 11 bytes
	pha
	tya
	ldy #(n-23)/11
	inc $F6,X
	dey
	Jpl *-3
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
.elseif(n = 1382)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 1383)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1388 || n = 1396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1390)
	; 12 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	dex
	dex
	Jne *-3
	tax
	pla
.elseif(n = 1394)
	; 12 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1399)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 1403)
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
.if((n .mod 7 = 0) && ((n > 1406 && n < 1443)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1408)
	; 11 bytes
	stx @zptemp
	ldx $A6
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	ldx @zptemp
.elseif(n = 1409 || n = 1423 || n = 1430 || n = 1437)
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
.elseif(n = 1411 || n = 1429)
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
.elseif(n = 1415)
	; 11 bytes
	delay_n (n-5)
	inc @zptemp
.elseif((n .mod 9 = 3) && ((n > 1415 && n < 1444)))
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $63A0'
	ldy #99
	iny
	Jne *-4
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $9EA0'
	ldy #158
	dey
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
.elseif(n = 1439)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $60A2'
	ldx #96
	inx
	Jne *-4
	ldx @zptemp
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
.elseif(n = 1448)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
.elseif(n = 1452 || n = 1479)
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1455)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1457)
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
.elseif(n = 1458 || n = 1472)
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
	bit $24
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
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1471)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 1473)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $A3A0'
	ldy #163
	dey
	Jne *-4
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
.elseif(n = 1480)
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
.elseif(n = 1481)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $A4A0'
	ldy #164
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 1485) && (n < 1524))
; 1485 to 1523
.if(n = 1485 || n = 1520)
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
.elseif(n = 1486 || n = 1493 || n = 1507 || n = 1514 || n = 1521)
	; 11 bytes
	delay_n (n-2)
	nop
.elseif(n = 1487 || n = 1511)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1490)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $A5A0'
	ldy #165
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 7 = 0) && ((n > 1490 && n < 1520)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
.elseif(n = 1494)
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
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 1499)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $A6A0'
	ldy #166
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1500)
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 1501)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1502 || n = 1522)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1504)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $A6A0'
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
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1518)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $A8A0'
	ldy #168
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1523)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $A8A0'
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
.elseif(n = 1525)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	ldx @zptemp
.elseif((n .mod 7 = 0) && ((n > 1525 && n < 1562)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
.elseif((n .mod 9 = 7) && ((n > 1527 && n < 1556)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
.elseif(n = 1534)
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
.elseif(n = 1535 || n = 1539 || n = 1549)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1542 || n = 1551 || n = 1560)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1543)
	; 11 bytes
	sty @zptemp
	ldy #(n-19)/12
	delay_n 7
	dey
	Jpl *-3
	ldy @zptemp
.elseif(n = 1544)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ABA0'
	ldy #171
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1553)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ACA0'
	ldy #172
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ACA0'
	ldy #172
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1562)
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
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
.if((n >= 1563) && (n < 1602))
; 1563 to 1601
.if(n = 1563 || n = 1570 || n = 1577 || n = 1598)
	; 11 bytes
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
.elseif(n = 1567 || n = 1579)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 7 = 0) && ((n > 1567 && n < 1597)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1569 || n = 1578)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1571)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
.elseif(n = 1581)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $AFA0'
	ldy #175
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1583 || n = 1599)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1590)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $B0A0'
	ldy #176
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1591 || n = 1600)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1592)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1593)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1597)
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
.endif
.else
; 1602 to 1640
.if(n = 1602)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 7 = 0) && ((n > 1602 && n < 1639)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1608 || n = 1615 || n = 1622 || n = 1636)
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1609 || n = 1618 || n = 1627)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1611)
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
.elseif(n = 1616)
	; 10 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	inx
	inx
	Jne *-3
	ldx @zptemp
.elseif((n .mod 7 = 2) && ((n > 1618 && n < 1641)))
	; 11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1625)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $B4A0'
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
	; 11 bytes
	inc $F6,X
	delay_n (n-6)
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
.if(n = 1641 || n = 1655)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1642 || n = 1662)
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
	; 11 bytes
	stx @zptemp
	ldx $A6
	ldx #234 ;hides 'nop'
	dex
	Jne *-2
	ldx @zptemp
.elseif(n = 1647 || n = 1665 || n = 1675)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1661)
	; 10 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	nop
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1663 || n = 1672)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
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
	bit <$2C ;hides 'bit $B8A0'
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
.elseif(n = 1669)
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
.elseif(n = 1670)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1674)
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
.elseif(n = 1679)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1681 || n = 1690 || n = 1717)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1682 || n = 1689 || n = 1691 || n = 1696 || n = 1703 || n = 1705 || n = 1710 || n = 1714)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1683 || n = 1718)
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
.elseif(n = 1697)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1698)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $BCA0'
	ldy #188
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1702 || n = 1712)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1707)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $BDA0'
	ldy #189
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1709)
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
.elseif(n = 1711)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.if(n = 1719)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
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
	bit <$2C ;hides 'bit $BEA0'
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
.elseif(n = 1723 || n = 1747)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1725)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $BFA0'
	ldy #191
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1726 || n = 1735 || n = 1744)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $C0A0'
	ldy #192
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1737)
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
.elseif(n = 1738 || n = 1745 || n = 1754)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1746)
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
.if(n = 1758)
	; 10 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1759 || n = 1760 || n = 1765 || n = 1768 || n = 1773 || n = 1781 || n = 1787)
	; 11 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1766)
	; 10 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1767 || n = 1794)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1770)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	dey
	Jne *-4
	ldy @zptemp
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
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1774)
	; 10 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1775 || n = 1791)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1777)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
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
	bit <$2C ;hides 'bit $3BA0'
	ldy #59
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1786)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1788)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
	bit <$2C ;hides 'bit $3AA0'
	ldy #58
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1795)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
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
.elseif((n .mod 9 = 7) && ((n > 1797 && n < 1835)))
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
.elseif(n = 1802 || n = 1822)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 1805)
	; 10 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 1806 || n = 1826)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1814)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $C9A0'
	ldy #201
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1815 || n = 1823)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1819 || n = 1831)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1827)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	ldx @zptemp
.elseif(n = 1833)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1841)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $34A0'
	ldy #52
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1843 || n = 1861 || n = 1870)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1846)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $34A0'
	ldy #52
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1847 || n = 1863 || n = 1871)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
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
.elseif(n = 1854 || n = 1858)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1855 || n = 1867)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1859)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1877)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1878)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 9 = 7) && ((n > 1878 && n < 1907)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1881 || n = 1885 || n = 1889 || n = 1890 || n = 1894 || n = 1898)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1882 || n = 1912)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1887 || n = 1895)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1891 || n = 1903)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
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
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1907)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1909)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $2CA0'
	ldy #44
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1914)
	; 11 bytes
	stx @zptemp
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	ldx @zptemp
.endif
.else
; 1915 to 1953
.if(n = 1915 || n = 1933 || n = 1942 || n = 1951)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1917 || n = 1953)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1918)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2CA0'
	ldy #44
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1919 || n = 1935 || n = 1943)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1922 || n = 1952)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1927 || n = 1939)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1931)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1941)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 1949)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1950)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	iny
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 1954) && (n < 1993))
; 1954 to 1992
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
.elseif(n = 1957)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1958)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1959 || n = 1967 || n = 1991)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 9 = 7) && ((n > 1959 && n < 1988)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1962 || n = 1982)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1963 || n = 1975)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1964 || n = 1972 || n = 1988)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	bit $24
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1966 || n = 1971 || n = 1984 || n = 1989)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1970)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1973)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1976)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1977)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $DBA0'
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
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1985)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 1986)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $24A0'
	ldy #36
	iny
	Jne *-4
	ldy @zptemp
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
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 1998 || n = 2006 || n = 2024)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1994)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1995)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1999 || n = 2011)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
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
.elseif(n = 2005 || n = 2014 || n = 2023)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2007 || n = 2015 || n = 2031)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2021)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2029)
	; 11 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2030)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	iny
	Jne *-4
	ldy @zptemp
.endif
.endif
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
.elseif(n = 2033 || n = 2038 || n = 2043 || n = 2061)
	; 12 bytes
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
.elseif(n = 2035 || n = 2047)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2039)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/8
	bit $24
	dey
	Jne *-3
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
	; 11 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inx
	Jne *-4
	tax
	pla
.elseif(n = 2048)
	; 11 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2049)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2057)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2058 || n = 2069)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
	delay_n 10
	Jne *-5
	tay
	pla
.elseif(n = 2066)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2067)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	iny
	Jne *-4
	ldy @zptemp
.endif
.else
; 2071 to 2109
.if(n = 2071 || n = 2083 || n = 2107)
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
.elseif(n = 2074 || n = 2078 || n = 2079)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2075)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	iny
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
.elseif((n .mod 9 = 7) && ((n > 2076 && n < 2105)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2080)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	iny
	Jne *-4
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
	stx @zptemp
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2087)
	; 12 bytes
	stx @zptemp
	ldx $A6
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	ldx @zptemp
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
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2096 || n = 2105)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2097)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2103)
	; 11 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	inx
	Jne *-4
	ldx @zptemp
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
.if(n = 2110 || n = 2114 || n = 2115 || n = 2128 || n = 2133)
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
.elseif((n .mod 9 = 7) && ((n > 2112 && n < 2141)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2119 || n = 2143)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2120)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2121)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2123)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2125)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	dey
	Jne *-4
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
.elseif(n = 2129)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2130)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	iny
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
.elseif(n = 2134)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	dey
	Jne *-4
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
.elseif(n = 2137)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2138)
	; 11 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2139)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2146)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2147)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 2149 to 2187
.if((n .mod 9 = 7) && ((n > 2148 && n < 2186)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2151 || n = 2159 || n = 2164 || n = 2169 || n = 2173 || n = 2178 || n = 2186 || n = 2187)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2152 || n = 2162 || n = 2182)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.elseif(n = 2155 || n = 2179)
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
	ldy #<$2C ;hides 'bit $11A0'
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
.elseif(n = 2165)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2166)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2174)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2175)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2177)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2183)
	; 11 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inx
	Jne *-4
	ldx @zptemp
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
.elseif(n = 2190 || n = 2199)
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
.elseif(n = 2192 || n = 2202)
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
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2194 || n = 2203)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2195)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2196)
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
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	dey
	Jne *-4
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
.elseif(n = 2200)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $39A0'
	ldy #57
	nop
	iny
	Jne *-5
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
.elseif(n = 2204)
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
.elseif(n = 2206)
	; 11 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 4
	iny
	Jne *-4
	ldy @zptemp
.endif
.else
; 2207 to 2226
.if(n = 2207)
	; 11 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2208 || n = 2217 || n = 2226)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2209 || n = 2213 || n = 2214)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2212)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2215)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2218)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	inc @zptemp
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2219)
	; 11 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inx
	Jne *-4
	ldx @zptemp
.elseif(n = 2220)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2222)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
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
.endif
.else
; 2227 to 2265
.if(n = 2227 || n = 2263)
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
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2229)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	iny
	Jne *-4
	ldy @zptemp
.elseif((n .mod 9 = 7) && ((n > 2229 && n < 2258)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2231 || n = 2236 || n = 2241 || n = 2250 || n = 2254 || n = 2258 || n = 2259)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2237)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2238)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	Jne *-4
	ldx @zptemp
.elseif(n = 2240)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2242)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
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
.elseif(n = 2246)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2247)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	iny
	Jne *-4
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
.elseif(n = 2255)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2260)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	iny
	Jne *-4
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
.elseif(n = 2264)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2265)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	iny
	Jne *-4
	ldy @zptemp
.endif
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
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2274)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2276 || n = 2277 || n = 2281 || n = 2285 || n = 2286 || n = 2290)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2283)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2291)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	iny
	Jne *-4
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2301)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2309)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2310)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 2313 || n = 2317 || n = 2318)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2319)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	bit $24
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2326)
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
.elseif(n = 2327)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
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
	; 11 bytes
	sty @zptemp
	ldy #128
	brk
	dey
	Jpl *-3
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2332 || n = 2342)
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
	; 12 bytes
	stx @zptemp
	ldx #44 ;hides 'bit abs'
	bit $0
	nop
	inx
	Jne *-5
	ldx @zptemp
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
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
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
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2346 || n = 2349 || n = 2351 || n = 2354 || n = 2356 || n = 2368 || n = 2374 || n = 2379)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2347 || n = 2359 || n = 2371)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2348)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	nop
	dey
	Jne *-5
	ldy @zptemp
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
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2357 || n = 2367)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2360)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2361)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2369)
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
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2380)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2381)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D8A0'
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
.elseif(n = 2384 || n = 2385 || n = 2390 || n = 2397 || n = 2404 || n = 2409 || n = 2411 || n = 2414 || n = 2418)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2387 || n = 2415)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
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
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	; 12 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	bit $24
	dey
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2394 || n = 2405 || n = 2416)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2403)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
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
.elseif(n = 2413)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #56 ;hides 'sec'
	inc @zptemp
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2417)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
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
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $FF24 ;hides 'bit zp'
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2427)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2429 || n = 2457)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2435)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #242
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2437)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 2444)
	; 12 bytes
	stx @zptemp
	ldx $A6
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
.elseif(n = 2447)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
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
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	ldy @zptemp
.endif
.else
; 2461 to 2500
.if(n = 2461 || n = 2483)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
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
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2464 || n = 2466 || n = 2469 || n = 2474 || n = 2481 || n = 2486 || n = 2488 || n = 2489 || n = 2494 || n = 2500)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2465 || n = 2487 || n = 2498)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 2470)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2471)
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	brk
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
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2476)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
	pla
.elseif(n = 2477 || n = 2497)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2480)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2485 || n = 2499)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2490)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2493)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2505 || n = 2510 || n = 2511 || n = 2517 || n = 2524 || n = 2529 || n = 2533 || n = 2534)
	; 12 bytes
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
.elseif(n = 2507)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
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
	brk
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
.elseif(n = 2525)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2526 || n = 2537)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
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
.elseif(n = 2535)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2536)
	; 12 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	ldx @zptemp
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
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2541 || n = 2555)
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
	brk
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2548 || n = 2559)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2553 || n = 2564)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2554 || n = 2558 || n = 2565 || n = 2566 || n = 2570 || n = 2571 || n = 2576 || n = 2577)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2567)
	; 12 bytes
	sty @zptemp
	ldy #0
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2578)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/12
	delay_n 7
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 2582 || n = 2589 || n = 2594)
	; 12 bytes
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
.elseif(n = 2590)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2591)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2593)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2595)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	nop
	dey
	Jne *-5
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
.elseif(n = 2599 || n = 2611)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2600)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #235
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2601 || n = 2606)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2602)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2603 || n = 2614)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 4
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2617)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.endif
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2620 || n = 2621 || n = 2632 || n = 2637 || n = 2642 || n = 2643 || n = 2654 || n = 2655)
	; 12..13 bytes
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
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2625 || n = 2636)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2626)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/12
	delay_n 7
	dex
	Jne *-3
	ldx @zptemp
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
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2631)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2633)
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
.elseif(n = 2634)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2638)
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
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2653)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2656)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	ldy @zptemp
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
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2696)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2660 || n = 2661 || n = 2665 || n = 2666 || n = 2673 || n = 2675 || n = 2678 || n = 2687 || n = 2690)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	nop
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 2669 || n = 2680 || n = 2691)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2670)
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
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	nop
	iny
	Jne *-5
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif((n > 2696 && n < 2700) || n = 2702 || n = 2704 || n = 2708 || n = 2720 || n = 2721 || n = 2726 || n = 2733)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	dey
	Jne *-5
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
.elseif(n = 2705)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	nop
	dey
	Jne *-5
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
.elseif(n = 2707 || n = 2719 || n = 2731)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2709 || n = 2723)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2710)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2711)
	; 12 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	nop
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 2713)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
.elseif(n = 2722)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2725)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	nop
	inx
	Jne *-5
	ldx @zptemp
.endif
.endif
.else
.if((n >= 2735) && (n < 2774))
; 2735 to 2773
.if((n .mod 11 = 7) && ((n > 2734 && n < 2769)))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
.elseif(n = 2738 || n = 2753 || n = 2758 || n = 2759 || n = 2763 || n = 2764 || n = 2769)
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
	ldy #<$2C ;hides 'bit $07A0'
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
	ldy #<$2C ;hides 'bit $07A0'
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
.elseif(n = 2761)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
.if((n >= 2774) && (n < 2793))
; 2774 to 2792
.if((n > 2773 && n < 2777) || n = 2781 || n = 2785 || n = 2786)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2777)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	bit $24
	dex
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 2778)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2779 || n = 2791)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2782)
	; 12 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	bit $24
	dex
	dex
	Jne *-5
	tax
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
.elseif(n = 2784)
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
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2789)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	bit $24
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2790)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
.endif
.else
; 2793 to 2812
.if(n = 2793)
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2796 || n = 2808)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2797)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2799)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2800)
	; 12 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2801)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2803)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2809)
	; 13 bytes
	delay_n (n-2)
	nop
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
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	ldy @zptemp
.endif
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
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	bit $24
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2814 || n = 2819 || n = 2822 || (n > 2828 && n < 2832) || n = 2834 || n = 2836 || n = 2841 || n = 2842 || n = 2846 || n = 2848)
	; 12..13 bytes
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/11
	inc $F6,X
	dey
	Jne *-3
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
	; 12 bytes
	sty @zptemp
	ldy #0
	delay_n 4
	nop
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2824 || n = 2843)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 2833)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2847)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	ldx @zptemp
.elseif(n = 2850)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	ldx @zptemp
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
.elseif(n = 2853 || n = 2858 || n = 2860 || n = 2864 || n = 2865 || n = 2870 || n = 2889)
	; 12..13 bytes
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
.elseif(n = 2855)
	; 13 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
	pla
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
.elseif(n = 2857 || n = 2876)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2861 || n = 2873)
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
.elseif(n = 2866 || n = 2878 || n = 2890)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/12
	delay_n 7
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 2867)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2869)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2883)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/12
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2888)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 2893 || n = 2958)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2894 || n = 2901 || n = 2903 || n = 2906 || n = 2908 || n = 2913 || n = 2915 || n = 2918 || n = 2920 || n = 2925 || n = 2926 || n = 2930 || n = 2937 || n = 2939 || (n > 2941 && n < 2945) || n = 2949 || n = 2951 || n = 2954 || n = 2956)
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
.elseif(n = 2898 || n = 2950 || n = 2963)
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
	brk
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
.elseif(n = 2909 || n = 2921 || n = 2945)
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
	brk
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
.elseif(n = 2929)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
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
.elseif(n = 2955)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/12
	delay_n 7
	dey
	Jne *-3
	tay
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2972 || n = 2992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2973 || n = 2978 || n = 2979 || n = 2984 || n = 2985 || n = 2990 || n = 2996 || n = 2997 || n = 3002)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2974 || n = 2986)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/12
	delay_n 7
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 2977)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2991)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/12
	delay_n 7
	dey
	Jne *-3
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
	txa
	ldx #(n-18)/12
	delay_n 7
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.endif
.else
.if((n >= 3008) && (n < 3027))
; 3008 to 3026
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
.elseif(n = 3009 || n = 3014 || n = 3020)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3010)
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	inc $F6,X
	iny
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
.elseif(n = 3012 || n = 3024)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3015)
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
.elseif(n = 3017)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3019)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3021)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 4
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3025)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
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
.endif
.else
; 3027 to 3046
.if(n = 3027)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3028 || n = 3041)
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
.elseif(n = 3029)
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
.elseif(n = 3030)
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
.elseif(n = 3031 || n = 3043)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3033 || n = 3038 || n = 3040)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3036)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3037)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
	brk
	delay_n (n-13)
.elseif(n = 3045)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-5
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
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3051 || n = 3056 || n = 3057 || n = 3061 || n = 3062 || n = 3069 || n = 3074 || n = 3076 || n = 3081)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3053 || n = 3065)
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
.elseif(n = 3055 || n = 3067)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3059 || n = 3073)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3063 || n = 3075)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
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
.elseif(n = 3080)
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
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3088 || n = 3094 || n = 3110 || n = 3122)
	; 12..13 bytes
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
.elseif(n = 3090 || n = 3108)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
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
.elseif(n = 3095 || n = 3111)
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
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3097)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3103 || n = 3121)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3104)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3107)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3113)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	delay_n 4
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3114)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3118)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3124)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 3125)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	delay_n 4
	dey
	Jne *-6
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3128 || n = 3147)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3129 || n = 3143)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3131 || n = 3136 || n = 3137 || n = 3141 || n = 3146 || n = 3149 || n = 3150 || n = 3154 || n = 3159 || n = 3160 || n = 3164)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3132 || n = 3145 || n = 3158)
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
.elseif(n = 3133)
	; 13 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	rol $26
	ror $26
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3135 || n = 3152)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3139 || n = 3157)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3140)
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
.elseif(n = 3151)
	; 13 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3153)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3155)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.endif
.else
; 3165 to 3203
.if(n = 3165)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3166)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3167 || n = 3187)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3168 || n = 3173 || n = 3177 || n = 3178 || n = 3182 || n = 3195 || n = 3200 || n = 3201)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3169 || n = 3186 || n = 3203)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3171 || n = 3185 || n = 3199)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3172 || n = 3192)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3175 || n = 3193)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3179)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3181)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
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
.elseif(n = 3184 || n = 3197)
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
.elseif(n = 3188)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	inc @zptemp
	inx
	Jne *-6
	tax
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3202)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3206 || n = 3214 || n = 3215 || n = 3231 || n = 3233 || n = 3238)
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
.elseif(n = 3211 || n = 3229)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 3217)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3219)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 4
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3230)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
	ldx @zptemp
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
.elseif(n = 3239)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3240)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.endif
.else
.if((n >= 3243) && (n < 3262))
; 3243 to 3261
.if(n = 3243 || n = 3248 || n = 3251)
	; 13 bytes
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
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inc @zptemp
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3246 || n = 3260)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3249)
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
.elseif(n = 3250)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	inc $F6,X
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3252)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
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
.elseif(n = 3254)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3255)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3256)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3257)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
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
.elseif(n = 3259)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3261)
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
; 3262 to 3281
.if(n = 3262 || n = 3275)
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
.elseif(n = 3265)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3266)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	inc $F6,X
	iny
	Jne *-6
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
.elseif(n = 3268)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3269)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3270)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3271)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
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
.elseif(n = 3274)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3276)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3277)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3278)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3279)
	; 14 bytes
	php
	pha
	sec
	lda #233
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3280)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
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
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3284)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #234 ;hides 'nop'
	delay_n 7
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 3285 || n = 3290 || n = 3298 || n = 3303 || n = 3304 || n = 3308)
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
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3289)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
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
.elseif(n = 3295)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 4
	iny
	Jne *-6
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
.elseif(n = 3297)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3305)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3309)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3314)
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
.elseif(n = 3315)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3317)
	; 13 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3320)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3322 || n = 3356)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3323 || n = 3328 || n = 3329 || n = 3341 || n = 3346 || n = 3357 || n = 3359)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3333)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3334)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3335)
	; 13 bytes
	sty @zptemp
	ldy #0
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3336)
	; 14 bytes
	pha
	txa
	ldx #237
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 3337 || n = 3355)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3343)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3345)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3348)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3349)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $2FA0'
	ldy #47
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3350)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3351)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.endif
.else
.if((n >= 3360) && (n < 3399))
; 3360 to 3398
.if(n = 3360 || n = 3378 || n = 3396)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3361)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif((n > 3361 && n < 3365) || n = 3369 || n = 3374 || n = 3377 || n = 3380 || n = 3393 || n = 3398)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3365)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3366)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3367 || n = 3381 || n = 3395)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3368)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3370 || n = 3389)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3371)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
	ldx @zptemp
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
.elseif(n = 3373 || n = 3391)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3376)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	dex
	dex
	Jne *-5
	tax
	pla
.elseif(n = 3379)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 3382 || n = 3397)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3385)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	inc @zptemp
	iny
	Jne *-6
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3415 || n = 3431)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3401)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	inc $F6,X
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3402 || n = 3417)
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
.elseif(n = 3405)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	rol $26
	ror $26
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3406)
	; 13 bytes
	pha
	tya
	ldy #44 ;hides 'bit abs'
	inc $F6,X
	delay_n 7
	iny
	Jne *-6
	tay
	pla
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
	brk
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
.elseif(n = 3411 || n = 3416 || n = 3425 || n = 3434)
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
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3418)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3421)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3426)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3430)
	; 12 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	brk
	Jne *-5
	ldy @zptemp
.elseif(n = 3433)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
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
.if((n >= 3438) && (n < 3457))
; 3438 to 3456
.if(n = 3438)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3439 || n = 3443 || n = 3444 || n = 3448 || n = 3449 || n = 3453)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3440)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	inc @zptemp
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3441)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3442)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3445)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 3447)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3450)
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3452)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3454)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inc @zptemp
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3455)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3456)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	ldx @zptemp
.endif
.else
; 3457 to 3476
.if(n = 3457)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3458)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3459)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	ldx @zptemp
.elseif(n = 3460)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	inc $F6,X
	iny
	Jne *-6
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
.elseif(n = 3463)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3464)
	; 13 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	tax
	pla
.elseif(n = 3465)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3468)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3469)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3470)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3471)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3472)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3473)
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
.elseif(n = 3474)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3476)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inc $F6,X
	inx
	Jne *-6
	tax
	pla
.endif
.endif
.else
.if((n >= 3477) && (n < 3496))
; 3477 to 3495
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
.elseif(n = 3478)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3479 || n = 3493)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3480)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3483 || n = 3488)
	; 12 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3486)
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3489)
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
.elseif(n = 3491)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	inc $F6,X
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3492)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3494)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3495)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 3496 to 3515
.if(n = 3496)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3497 || n = 3514)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3501)
	; 12 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	jsr @nearby_rts
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3502)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3505)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	inc $F6,X
	dey
	Jne *-6
	tay
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
.elseif(n = 3507)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3508)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3509)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3510)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3511)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3513)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3515)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	ldy @zptemp
.endif
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3518 || n = 3519 || n = 3523 || n = 3524 || n = 3528 || n = 3529 || n = 3533 || n = 3534 || n = 3537 || n = 3542 || n = 3544 || n = 3545 || n = 3551)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3521 || n = 3549)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3525)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3536)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3538)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3539)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3541)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3543)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3547)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3550)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	inc $F6,X
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3556 || n = 3569 || n = 3570 || n = 3573 || n = 3578 || n = 3586 || n = 3593)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3557)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3559 || n = 3575)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3560)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3561)
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
.elseif(n = 3562)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3563 || n = 3577)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3566)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3567 || n = 3587)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3572 || n = 3592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3574)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3579)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3581)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3583)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3584)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3585)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #255
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3588)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3590)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	delay_n 7
	dey
	Jne *-6
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
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
.if((n >= 3594) && (n < 3613))
; 3594 to 3612
.if(n = 3594 || n = 3612)
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
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	inc @zptemp
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
.elseif(n = 3597)
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
.elseif(n = 3600 || n = 3601 || n = 3609)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3602)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3606)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3607)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 3613 to 3632
.if(n = 3613 || n = 3614 || n = 3618 || n = 3629)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3615)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3616)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3617)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3619)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3620)
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
.elseif(n = 3621)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3622)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3623)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3624)
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
.elseif(n = 3625)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3627)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3628)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3630)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3631)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 3632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.endif
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
.elseif(n = 3634 || n = 3635 || n = 3649)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3637)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3638)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3639)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3640)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	inc $F6,X
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3641)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3642)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 3646)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 3647)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.endif
.else
; 3652 to 3671
.if(n = 3652)
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3654)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3655)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3656)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
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
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3659)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	ldx @zptemp
.elseif(n = 3660)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3661)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3662)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3663 || n = 3668)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 3664)
	; 10 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
	pla
.elseif(n = 3665)
	; 12 bytes
	inc $F6,X
	delay_n (n-6)
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
.elseif(n = 3667)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3669)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3670)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	brk
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
.elseif(n = 3673)
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
	brk
	delay_n (n-13)
.elseif(n = 3678)
	; 13 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3679 || n = 3697)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3680)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $A7A0'
	ldy #167
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3681 || n = 3683 || n = 3686 || n = 3688 || n = 3689 || n = 3694 || n = 3695 || n = 3699 || n = 3704)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3682)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3687 || n = 3707)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3690)
	; 11 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	brk
	dey
	Jne *-4
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
.elseif(n = 3700)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	inc $F6,X
	inx
	Jne *-6
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
.elseif(n = 3703)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3705)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
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
.if((n >= 3711) && (n < 3731))
; 3711 to 3730
.if(n = 3711)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3712)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3713 || n = 3730)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3714)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3715)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3716)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3717)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3718)
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
.elseif(n = 3720)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3721)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
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
.elseif(n = 3725)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3726)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
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
.elseif(n = 3728)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3729)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
.endif
.else
; 3731 to 3750
.if(n = 3731 || n = 3750)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3732)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3733)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 3735)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3737 || n = 3740 || n = 3748)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3738)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3741)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3742)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3743)
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
.elseif(n = 3744)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3745)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3746)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 11
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
.if((n >= 3751) && (n < 3770))
; 3751 to 3769
.if(n = 3751 || n = 3769)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3753 || n = 3758)
	; 12 bytes
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
.elseif(n = 3756)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3757)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3759)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3760)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3761)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
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
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3764)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3765)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3766)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3767)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.else
; 3770 to 3789
.if(n = 3770)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3771 || n = 3773 || n = 3776 || n = 3778 || n = 3785 || n = 3789)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3774)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3775)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3777)
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
.elseif(n = 3779)
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
.elseif(n = 3780)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3781)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3782)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3786)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3787)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3788)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3791)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3792)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3794 || n = 3795 || n = 3800 || n = 3801)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3796)
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
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3798)
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
.elseif(n = 3802)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3803)
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
.elseif(n = 3805)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3806)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3807)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3808)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 3809 to 3828
.if(n = 3809)
	; 13 bytes
	sty @zptemp
	ldy #190
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
	nop
.elseif(n = 3810 || n = 3828)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3811)
	; 13 bytes
	inc $F6,X
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
.elseif(n = 3813)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3814)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3815)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3816)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 3817)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3820)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3821)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	inc $F6,X
	iny
	Jne *-6
	tay
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
.elseif(n = 3823)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3824 || n = 3825)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3826)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.endif
.else
.if((n >= 3829) && (n < 3868))
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3830 || n = 3833 || n = 3839 || n = 3843)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3832)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $AEA0'
	ldy #174
	brk
	dey
	Jne *-6
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
.elseif(n = 3841)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 3844)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 3845)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3846)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
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
.endif
.else
; 3848 to 3867
.if(n = 3848 || n = 3856 || n = 3861)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3849 || n = 3866)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3850)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3854)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3858)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 3859)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3860)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $AFA0'
	ldy #175
	brk
	dey
	Jne *-6
	tay
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
.elseif(n = 3864)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3865)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 3867)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
	pla
.endif
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
.elseif(n = 3869)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3870)
	; 13 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
	pla
.elseif(n = 3871 || n = 3888 || n = 3905)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3872 || n = 3892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3873 || n = 3879 || n = 3881 || (n > 3883 && n < 3887) || n = 3889 || n = 3890 || n = 3896 || n = 3897 || n = 3904)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3875)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3876)
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
.elseif(n = 3877 || n = 3895)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3878)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3880)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 3883)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3887)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3891)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3893)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3894)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3898)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3903)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3906)
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
	brk
	delay_n (n-13)
.elseif(n = 3909)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3910)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3911)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
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
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3914)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3915 || n = 3919 || n = 3920 || n = 3923 || n = 3924 || n = 3929 || n = 3933 || n = 3935 || n = 3938)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	brk
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3927)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3928)
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
.elseif(n = 3930)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3937)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	inc $F6,X
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 3941)
	; 13 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3942)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3943)
	; 12 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	brk
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
	brk
	inx
	inx
	Jne *-5
	tax
	pla
.elseif(n = 3949 || n = 3967)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 3953 || n = 3961 || n = 3966 || (n > 3968 && n < 3972) || n = 3974)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3955 || n = 3976)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3957)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3958)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3959 || n = 3978)
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
.elseif(n = 3965)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $B4A0'
	ldy #180
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3975)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3977)
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
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3984)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
.if((n >= 3985) && (n < 4004))
; 3985 to 4003
.if(n = 3985 || n = 4003)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3986)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3987)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3989)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3990)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3991)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3993)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 3997)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3998)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3999)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4000)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4001)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4002)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 4004 to 4023
.if(n = 4004 || n = 4009 || n = 4014 || n = 4023)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4005)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	brk
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 4006)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4007)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4008)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 4012)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4013)
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
.elseif(n = 4015 || n = 4017)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4016)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4018)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4019)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4020)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4021)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4022)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
; 4024 to 4062
.if(n = 4024 || n = 4041 || n = 4058)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4025 || (n > 4027 && n < 4031) || n = 4034 || n = 4046 || n = 4048 || n = 4049 || n = 4056 || n = 4061)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4026 || n = 4044 || n = 4062)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4027 || n = 4047)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4031)
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
.elseif(n = 4032 || n = 4052)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4033)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4035 || n = 4054)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4036 || n = 4053)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4037)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4038)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4039 || n = 4057)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 4045)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4050)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4051)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4055)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4060)
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
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
.if((n >= 4063) && (n < 4082))
; 4063 to 4081
.if(n = 4063)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4064 || n = 4066 || n = 4077 || n = 4079)
	; 12..13 bytes
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
.elseif(n = 4068)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 4069)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4075)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 4078)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4080)
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
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 4082 to 4101
.if(n = 4082 || n = 4084 || n = 4089 || n = 4094 || n = 4095 || n = 4099 || n = 4100)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4083)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4085)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4086)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
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
	brk
	delay_n (n-13)
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
.elseif(n = 4093)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4096)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 4097)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4098)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4101)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
; 4102 to 4140
.if(n = 4102)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4103)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4104 || n = 4121 || n = 4138)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4105)
	; 13 bytes
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	inc $F6,X
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4107 || n = 4127)
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
.elseif(n = 4109 || n = 4126)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4110 || (n > 4112 && n < 4116) || n = 4118 || n = 4120 || n = 4123 || n = 4128 || n = 4131 || n = 4133 || n = 4136 || n = 4137 || n = 4140)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4111 || n = 4129)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4112 || n = 4132)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4116 || n = 4134)
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
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4119)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4122)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 4124)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4125)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $BBA0'
	ldy #187
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4130)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4135)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4139)
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
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4159)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4142 || n = 4178)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4143 || n = 4160 || n = 4177)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4144)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4145 || n = 4151 || n = 4156 || n = 4157 || n = 4162 || n = 4169 || n = 4174 || n = 4175 || n = 4179)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BCA0'
	ldy #188
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4147 || n = 4165)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4148)
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
.elseif(n = 4149 || n = 4168)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4150)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 4153 || n = 4171)
	; 13 bytes
	inc $F6,X
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
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4161)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
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
.if((n >= 4180) && (n < 4199))
; 4180 to 4198
.if(n = 4180)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4183)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 4185 || n = 4190 || n = 4191 || n = 4196)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4188)
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
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
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
.elseif(n = 4194)
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
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 4199 to 4218
.if(n = 4199)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4200)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4201)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4202)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4203 || n = 4208 || n = 4213 || n = 4215)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4204)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 4205)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4206)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4207)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4209)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
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
.elseif(n = 4211)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 4217)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 11
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4218)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.endif
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
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4220)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk
	iny
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4221 || n = 4226 || n = 4229 || n = 4234 || n = 4238 || n = 4239 || n = 4241 || n = 4246 || n = 4251 || n = 4259 || n = 4264 || n = 4265 || n = 4269 || n = 4270 || (n > 4274 && n < 4278) || n = 4280 || n = 4281 || n = 4284 || n = 4289 || (n > 4292 && n < 4296))
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	brk
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
.elseif(n = 4230 || n = 4249 || n = 4268 || n = 4287)
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
.elseif(n = 4233 || n = 4254)
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
.elseif(n = 4235)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C0A0'
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
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4243 || n = 4261)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4248 || n = 4266)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4250 || n = 4286)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4256)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4271)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4283)
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
.elseif(n = 4285)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4288)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $BAA0'
	ldy #186
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
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
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if(n = 4297 || n = 4319)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4298 || n = 4300 || n = 4303 || n = 4310 || n = 4311 || n = 4315 || n = 4316 || n = 4318 || n = 4322 || n = 4329 || n = 4331 || n = 4334)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4301 || n = 4320)
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
.elseif(n = 4304)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4305)
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
.elseif(n = 4306)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4307)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4308 || n = 4325)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4309 || n = 4327)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4312)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4313 || n = 4330)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4314 || n = 4332)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4317)
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
.elseif(n = 4321)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4323)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C4A0'
	ldy #196
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4326)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4328)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	nop
	inc $F6,X
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4333)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4335)
	; 13 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 4336 to 4375
.if(n = 4336 || n = 4341 || n = 4349 || n = 4354 || n = 4360 || n = 4365 || n = 4366 || n = 4369 || n = 4370 || n = 4374)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4337 || n = 4355 || n = 4373)
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
.elseif(n = 4339 || n = 4358)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4340)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4342)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4343)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4344)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4345 || n = 4363)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4346)
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
.elseif(n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4348)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 4350 || n = 4368)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4351)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4353 || n = 4371)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4356)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4357)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4359)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4361)
	; 13 bytes
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	delay_n 7
	dey
	Jne *-6
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4375)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
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
.if(n = 4376)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4377 || n = 4396)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4378)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4379 || n = 4383 || n = 4384 || n = 4388 || n = 4389 || n = 4394 || n = 4398 || n = 4403 || n = 4406 || n = 4408 || n = 4409 || n = 4414)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4380)
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
.elseif(n = 4381 || n = 4399)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4382 || n = 4401)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4385)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4386 || n = 4404)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4387 || n = 4407)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4390)
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
.elseif(n = 4391)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4392 || n = 4412)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4393)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4395)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4397)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4400)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4402)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 4411)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
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
.elseif(n = 4416)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4417 || n = 4435 || n = 4453)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 4419 || n = 4424 || n = 4436 || n = 4437 || n = 4441 || n = 4442 || n = 4444 || n = 4449)
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
.elseif(n = 4421)
	; 13 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4425)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4426)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
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
	brk
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
	brk
	delay_n (n-13)
.elseif(n = 4431)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 4433)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C9A0'
	ldy #201
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4438)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4443)
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
.elseif(n = 4445)
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
.elseif(n = 4446)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4450)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	brk
	dey
	Jne *-6
	ldy @zptemp
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
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4455 || n = 4457 || n = 4460 || n = 4462 || n = 4469 || (n > 4472 && n < 4476) || n = 4478 || n = 4479 || n = 4486)
	; 12..14 bytes
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
.elseif(n = 4458 || n = 4476)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4459 || n = 4480)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4461)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4463 || n = 4481)
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
.elseif(n = 4464 || n = 4485)
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
.elseif(n = 4465 || n = 4483)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4466 || n = 4484)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4467 || n = 4487)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4468)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4470)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4471 || n = 4489)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
.elseif(n = 4477)
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
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4488)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4490)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4491)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 4493 to 4531
.if(n = 4493 || n = 4498 || n = 4508 || n = 4509 || n = 4511 || n = 4514 || n = 4516 || n = 4517 || n = 4531)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4494 || n = 4512 || n = 4530)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4495)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4496 || n = 4515)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4497)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4499)
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
.elseif(n = 4500 || n = 4522)
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
.elseif(n = 4501)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4502)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4503 || n = 4519)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4504)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4505)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4506)
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
.elseif(n = 4507 || n = 4525)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4510 || n = 4529)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4513)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4518)
	; 13 bytes
	pha
	txa
	ldx #n/18-1
	brk
	dex
	Jne *-3
	inc @zptemp,x
	tax
	pla
.elseif(n = 4520)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4521)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CDA0'
	ldy #205
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4523)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4524)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4526)
	; 13 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	delay_n 26
	Jne *-6
	ldy @zptemp
.elseif(n = 4527)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4528)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
.endif
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
.elseif(n = 4533 || n = 4551)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/18
	brk
	dey
	Jne *-3
	tay
	pla
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
	brk
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
	pha
	txa
	ldx #238
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4541)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4542)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CEA0'
	ldy #206
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4543 || n = 4561)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
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
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/18
	brk
	dex
	Jne *-3
	ldx @zptemp
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
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4556)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4557)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4558)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4559)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4560)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $31A0'
	ldy #49
	brk
	iny
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4567)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4569)
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
.endif
.else
; 4571 to 4609
.if(n = 4571)
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
.elseif(n = 4574)
	; 13 bytes
	pha
	tya
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
	nop
.elseif(n = 4575)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4576 || n = 4581 || n = 4588 || n = 4590 || n = 4593 || n = 4595 || (n > 4598 && n < 4602) || n = 4606 || n = 4608)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4577)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4578)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4579 || n = 4597)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/18
	brk
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 4580)
	; 14 bytes
	brk
	sty @zptemp
	ldy #n/20-1
	nop
	brk
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4582)
	; 13 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
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
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4585)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4586)
	; 10 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 4587 || n = 4607)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4589)
	; 12 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	dex
	Jne *-3
	ldx @zptemp
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
.elseif(n = 4594)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 4596)
	; 12 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 4603)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
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
.if((n >= 4610) && (n < 4629))
; 4610 to 4628
.if(n = 4610)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4611 || n = 4613 || (n > 4616 && n < 4620) || n = 4622)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
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
	brk
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
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4621)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4623)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #0
	brk
	dey
	Jne *-3
	tay
	pla
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
	brk
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 4626)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4627)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4628)
	; 13 bytes
	brk
	delay_n (n-13)
.endif
.else
; 4629 to 4648
.if(n = 4629 || n = 4648)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4630)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D2A0'
	ldy #210
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4631)
	; 12 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 4633)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4634)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4635)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4636)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4637)
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
.elseif(n = 4638)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4639)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4640)
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
.elseif(n = 4641)
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
.elseif(n = 4642)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3FA0'
	ldy #63
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4643)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4644)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4645)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 4646)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4647)
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
.if((n >= 4649) && (n < 4668))
; 4649 to 4667
.if(n = 4649)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4651)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4652)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4653)
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
.elseif(n = 4654)
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
.elseif(n = 4655)
	; 13 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 4656)
	; 13 bytes
	pha
	txa
	ldx #24 ;hides 'clc'
	brk
	inx
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4657 || n = 4659)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4658 || n = 4663)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4660)
	; 14 bytes
	sty @zptemp
	ldy #141
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4661)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4662)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4664)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4665)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4666)
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
.elseif(n = 4667)
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
; 4668 to 4687
.if(n = 4668)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4669 || n = 4674 || n = 4683)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4670)
	; 13 bytes
	stx @zptemp
	ldx #44 ;hides 'bit abs'
	bit $0
	brk
	inx
	Jne *-6
	ldx @zptemp
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
.elseif(n = 4673)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4675)
	; 13 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	brk
	inx
	Jne *-6
	tax
	pla
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
.elseif(n = 4677)
	; 13 bytes
	pha
	tya
	ldy #44 ;hides 'bit abs'
	inc @zptemp
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4678)
	; 13 bytes
	pha
	tya
	ldy #44 ;hides 'bit abs'
	delay_n 19
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4679)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	jsr @nearby_rts_14cyc
	Jne *-6
	tax
	pla
.elseif(n = 4680)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4681)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4682)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4684)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4685)
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	brk
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
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
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4689 || n = 4694 || n = 4702 || n = 4709 || n = 4714 || (n > 4720 && n < 4724) || n = 4726)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4691 || n = 4706)
	; 13..14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4692 || n = 4712)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4693 || n = 4715)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4695 || n = 4716)
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
	brk
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4697 || n = 4699)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4698 || n = 4720)
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
.elseif(n = 4700 || n = 4719)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4701)
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
.elseif(n = 4703)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4704)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4705 || n = 4724)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4707)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4708)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4710)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4711)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4713)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4717)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4718)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4725)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $CDA0'
	ldy #205
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 4727 to 4765
.if(n = 4727 || n = 4747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4728 || n = 4729 || n = 4734 || n = 4740 || n = 4744 || n = 4745 || n = 4749 || n = 4754 || n = 4760)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4730)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4731)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4732 || n = 4752)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4733)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4735)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4736)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4737 || n = 4758)
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
.elseif(n = 4738 || n = 4757)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4739)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4741)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4742 || n = 4764)
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
.elseif(n = 4743 || n = 4762)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4746 || n = 4765)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4748)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4753)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4756)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4759)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4761)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4763)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4769 || n = 4771 || n = 4774 || n = 4778 || n = 4783 || n = 4789 || n = 4794 || n = 4797 || n = 4802 || n = 4804)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767 || n = 4787)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4768)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4770)
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
.elseif(n = 4772 || n = 4792)
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4775)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4776 || n = 4795)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4777)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4779)
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
.elseif(n = 4780)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4781 || n = 4800)
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
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4784)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4785)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4786)
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
.elseif(n = 4788)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4790)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4791)
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
.elseif(n = 4793)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4798)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4799)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4801)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4803)
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
.if((n >= 4805) && (n < 4824))
; 4805 to 4823
.if(n = 4805)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4806)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4807)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4808)
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
.elseif(n = 4809 || n = 4811)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4813)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4814)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4815)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4816)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4818)
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
.elseif(n = 4819)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4820)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4821)
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
.elseif(n = 4822)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4823)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	brk
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 4824 to 4843
.if(n = 4824)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4825)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4826 || n = 4839)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4827)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4828)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4829)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4830)
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
.elseif(n = 4831)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4832)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4833)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4834 || n = 4835 || n = 4840)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4836)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4837)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4838)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4841)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4842)
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
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if(n = 4844 || n = 4849 || n = 4853 || n = 4854 || n = 4859)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $24A0'
	ldy #36
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4846)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4847)
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
.elseif(n = 4850)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $24A0'
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
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4852)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4855)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4858)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4860)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
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
.endif
.else
; 4863 to 4882
.if(n = 4863)
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
.elseif(n = 4864)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4865)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4866)
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
.elseif(n = 4867)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4868)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4869)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4870)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4871)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4872)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4873)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 4875)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 4878 || n = 4880)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4879)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4881)
	; 14 bytes
	php
	pha
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
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
.endif
.else
.if((n >= 4883) && (n < 4902))
; 4883 to 4901
.if(n = 4883)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4885)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4886 || n = 4889 || n = 4893 || n = 4894)
	; 13..14 bytes
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4890)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4891)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4895)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4896)
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
.elseif(n = 4897)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4898)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4899)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4900)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4901)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $34A0'
	ldy #52
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
; 4902 to 4921
.if(n = 4902 || n = 4909 || n = 4914)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4903)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4904)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4905)
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
.elseif(n = 4906)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4907)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4908)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4910)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4911)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4913)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4915)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4916)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4917)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	brk
	iny
	Jne *-6
	tay
	pla
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4921)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
.if((n >= 4922) && (n < 4941))
; 4922 to 4940
.if(n = 4922)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 4923 || n = 4928 || n = 4929 || n = 4934)
	; 13..14 bytes
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
.elseif(n = 4925)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4927)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4930)
	; 14 bytes
	pha
	txa
	ldx #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dex
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 4931)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4932)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4933)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4935)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4938)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4939)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	brk
	dey
	Jne *-6
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
.endif
.else
; 4941 to 4960
.if(n = 4941)
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4943 || n = 4944 || n = 4949 || n = 4954)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4945)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4946)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4947)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4948)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4950)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4951)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4952)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4953)
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
.elseif(n = 4955)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4956)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4957)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4958)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4959)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4960)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 4961 to 5000
.if(n = 4961)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4962 || n = 4984)
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
.elseif(n = 4963)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4964 || n = 4965 || (n > 4968 && n < 4972) || n = 4974 || n = 4976 || n = 4981 || n = 4986 || n = 4991 || n = 4994 || n = 4996)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4966)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4967 || n = 4987)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4968 || n = 4989)
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
.elseif(n = 4973)
	; 13 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 7
	brk
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4978)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	brk
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4982)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4983)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 4985)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	nop
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4988)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4990)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4993)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 4995)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4997)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
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
	stx @zptemp
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5000)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	brk
	iny
	Jne *-6
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
.if((n >= 5001) && (n < 5040))
; 5001 to 5039
.if(n = 5001 || n = 5023)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5002)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5003)
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
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5005 || n = 5026)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5006 || n = 5028)
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
.elseif(n = 5007 || n = 5027)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5008)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5009 || n = 5011 || n = 5014 || n = 5016 || n = 5029 || n = 5033 || n = 5034 || n = 5036)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5010 || n = 5031)
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
.elseif(n = 5012 || n = 5032)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5013)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 5015 || n = 5035)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5017)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5018 || n = 5038)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 5019 || n = 5039)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5020)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5021)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5022)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5024)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5025)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5030)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5037)
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
.if((n >= 5040) && (n < 5059))
; 5040 to 5058
.if(n = 5040)
	; 14 bytes
	brk
	sty @zptemp
	ldy #n/20-1
	nop
	brk
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5041)
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
.elseif(n = 5042)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5043)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5044)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5045)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5046)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5047)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5048)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5049 || n = 5054 || n = 5056)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5050)
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
.elseif(n = 5051)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5052)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5053)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 5055)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5057)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5058)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.endif
.else
; 5059 to 5078
.if(n = 5059)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5060)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5061)
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
.elseif(n = 5062)
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
.elseif(n = 5063)
	; 14 bytes
	sty @zptemp
	ldy #158
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5064 || n = 5069 || n = 5074)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5065)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5066)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5067)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5068)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5070)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5071)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_15cyc
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5072)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5073)
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
.elseif(n = 5075)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5076)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5077)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5078)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	inc $EA,X ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 5079) && (n < 5118))
.if((n >= 5079) && (n < 5098))
; 5079 to 5097
.if(n = 5079)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5080)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	brk
	Jne *-5
	ldx @zptemp
.elseif(n = 5081)
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
.elseif(n = 5082)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5083)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5084)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5086)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 5087)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5088)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	brk
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5089)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5090)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/20
	jsr @nearby_rts_15cyc
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 5091)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5092)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5093)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5094)
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
	brk
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
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
.endif
.else
; 5098 to 5117
.if(n = 5098 || n = 5100 || n = 5103 || n = 5105 || n = 5117)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5099)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	nop
	dex
	Jne *-4
	ldx @zptemp
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
	inc $F6,X
	delay_n (n-6)
.elseif(n = 5104)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.elseif(n = 5106)
	; 13 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 5107)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 5110)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
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
.elseif(n = 5112)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5113)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
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
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5115)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5116)
	; 11 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118 || n = 5122 || n = 5123 || n = 5129 || n = 5134)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	delay_n 4
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 5120)
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	dey
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
.elseif(n = 5124)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 5128)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5130)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5131)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 5135)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
.endif
.else
; 5137 to 5156
.if(n = 5137)
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
	; 11 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5139 || n = 5145)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5140 || n = 5142)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5141)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5143)
	; 13 bytes
	php
	pha
	clc
	lda #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 5144)
	; 13 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 5146)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5147)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5148)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5149)
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
.elseif(n = 5150)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5151)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 5152)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5153)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5154)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5155)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5156)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
.if((n >= 5157) && (n < 5176))
; 5157 to 5175
.if(n = 5157)
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
.elseif(n = 5159)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5160)
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
.elseif(n = 5161)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5162)
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
.elseif(n = 5163)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5164)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5165)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5166)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5167)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5168)
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
.elseif(n = 5169)
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
.elseif(n = 5170)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5171)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5172)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5173)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5174)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5175)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	brk
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 5176 to 5195
.if(n = 5176)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5177)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5178)
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
.elseif(n = 5179 || n = 5183)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5180)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5181)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	brk
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 5184)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5185)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5186)
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
.elseif(n = 5187)
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
.elseif(n = 5188)
	; 14 bytes
	sty @zptemp
	ldy #157
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5189)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5190)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5191)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5192)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5193)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5194)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5195)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $28A0'
	ldy #40
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 5196) && (n < 5215))
; 5196 to 5214
.if(n = 5196)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5197)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5198)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5199)
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
.elseif(n = 5200 || n = 5201 || n = 5206)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5202)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
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
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5204)
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
.elseif(n = 5205)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5207)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5208)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5209)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5210)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5211)
	; 14 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	delay_n 26
	Jne *-7
	ldy @zptemp
.elseif(n = 5212)
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
.elseif(n = 5213)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5214)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	ldx @zptemp
.endif
.else
; 5215 to 5234
.if(n = 5215)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5216)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	delay_n 26
	Jne *-7
	tay
	pla
.elseif(n = 5217 || n = 5222 || n = 5228)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5218)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5219)
	; 13 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	brk
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5220)
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
.elseif(n = 5221)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5223)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5224)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5225)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5226)
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
.elseif(n = 5227)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5229 || n = 5230)
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
.elseif(n = 5231)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5232)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5233)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5234)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
.if((n >= 5235) && (n < 5254))
; 5235 to 5253
.if(n = 5235)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5236)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5237)
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
.elseif(n = 5238)
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
.elseif(n = 5239)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5240)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5241)
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
.elseif(n = 5242)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5243)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5244)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5245)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5246)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5247)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5248)
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
.elseif(n = 5249)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5250)
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
.elseif(n = 5251)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5252)
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
.elseif(n = 5253)
	; 14 bytes
	delay_n (n-2)
	nop
.endif
.else
; 5254 to 5273
.if(n = 5254)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5255)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5256)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5257)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5258 || n = 5271 || n = 5272)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5259)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5260)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5261)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5262)
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
.elseif(n = 5263)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5264)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5265)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5266)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5267)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5268)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
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
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5270)
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
.elseif(n = 5273)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5274) && (n < 5293))
; 5274 to 5292
.if(n = 5274)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5275)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	bit $24
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5276)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5277)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5278)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5279)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5280 || n = 5284 || n = 5289)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5281)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5282)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5283)
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
.elseif(n = 5285)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5286)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5287)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
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
.elseif(n = 5290)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5291)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5292)
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
.endif
.else
; 5293 to 5312
.if(n = 5293 || n = 5294 || n = 5301 || n = 5306)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5295)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5296)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5297)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5298)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5299)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5300)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5302)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5303)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5304)
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
.elseif(n = 5305)
	; 14 bytes
	pha
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	ldy @zptemp
	pla
.elseif(n = 5307)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5308)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5309)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5310)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5311)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5312)
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
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
.if((n >= 5313) && (n < 5332))
; 5313 to 5331
.if(n = 5313)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5314)
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
.elseif(n = 5315)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5316)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5317)
	; 14 bytes
	sty @zptemp
	ldy #177
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5318)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 5319)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5320)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5321 || n = 5328)
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
.elseif(n = 5322 || n = 5327)
	; 14 bytes
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
.elseif(n = 5324)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5325)
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
.elseif(n = 5326)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5329)
	; 13 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5330)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 5331)
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
; 5332 to 5351
.if(n = 5332)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5333 || n = 5338 || n = 5350)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5334)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5335)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5336)
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
.elseif(n = 5339)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5340)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5341)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5342)
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
.elseif(n = 5343)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5344)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5345)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5346)
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
.elseif(n = 5347)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5348)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5349)
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
.elseif(n = 5351)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	iny
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
.if(n = 5352)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5353)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5354)
	; 14 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 5355 || n = 5369)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5356)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5357)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5358)
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5360)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5361)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5362)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 5364)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5365)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
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
	bit <$2C ;hides 'bit $01A0'
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
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5372)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5373)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5374 || n = 5381 || n = 5390)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5375)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5376)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
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
	bit $24
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5378)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5379)
	; 12 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0085,X ;hides 'sta $00'
	dex
	Jne *-5
	tax
	pla
.elseif(n = 5380)
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
.elseif(n = 5382)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5383)
	; 13 bytes
	sty @zptemp
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 5385)
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
.elseif(n = 5386)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5387)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 5389)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5393)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $16A0'
	ldy #22
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5394 || n = 5417)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5395)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5396)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5397 || n = 5419)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5398 || n = 5399 || n = 5403 || n = 5404 || n = 5408 || n = 5409 || n = 5414 || n = 5421 || n = 5425 || n = 5426)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5400)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5401)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5402 || n = 5424)
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
.elseif(n = 5405)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5406)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5407)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5410)
	; 13 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5411)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5412)
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
.elseif(n = 5413 || n = 5427 || n = 5428)
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
.elseif(n = 5415)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5416)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5418)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5420)
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
.elseif(n = 5422)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5423)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5429)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
.if((n >= 5430) && (n < 5449))
; 5430 to 5448
.if(n = 5430)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5431)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5432)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5433)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5434)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5435)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5436)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 5438)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5439)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5440)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5441)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5442 || n = 5443 || n = 5447 || n = 5448)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5444)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5445)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5446)
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
.endif
.else
; 5449 to 5468
.if((n > 5448 && n < 5452))
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
.elseif(n = 5452)
	; 14 bytes
	sty @zptemp
	ldy #165
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5453)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5454)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5455)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5456)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5457)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5458)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5459)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5460)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5461)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	brk
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5463)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5464 || n = 5465)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5466)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5467)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5468)
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
.endif
.endif
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
.if((n >= 5469) && (n < 5488))
; 5469 to 5487
.if(n = 5469 || n = 5470)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5471)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5472)
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
.elseif(n = 5473)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif((n > 5473 && n < 5477))
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
.elseif(n = 5477)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5478)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5479)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5480)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5481)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5482)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5483)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5484)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5485)
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
.elseif(n = 5487)
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
; 5488 to 5507
.if(n = 5488)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
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
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5490)
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
.elseif(n = 5492 || n = 5496)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5493)
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
.elseif(n = 5494)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5495)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5497)
	; 14 bytes
	sty @zptemp
	ldy #183
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5498)
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
.elseif(n = 5499)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5500)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5501)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5502)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5503)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5504)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5505)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5506)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5507)
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
; 5508 to 5546
.if(n = 5508)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5509)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5510)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
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
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5512 || n = 5534)
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
.elseif(n = 5513 || n = 5514 || n = 5531 || n = 5535 || n = 5536 || n = 5539)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5515 || n = 5542)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5516)
	; 14 bytes
	pha
	tya
	ldy #172
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5517 || n = 5521 || n = 5522 || n = 5538 || n = 5540 || n = 5541 || n = 5544 || n = 5546)
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
.elseif(n = 5518)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	inc $EA,X ;hides 'nop'
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5519 || n = 5545)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5520)
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
.elseif(n = 5523)
	; 14 bytes
	pha
	tya
	ldy #167
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5524)
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
.elseif(n = 5525)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5526)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5527)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5528)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5529)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5530)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5532)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
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
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5537)
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
.elseif(n = 5543)
	; 14 bytes
	sty @zptemp
	ldy #173
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5547) && (n < 5586))
; 5547 to 5585
.if(n = 5547 || n = 5574)
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
.elseif(n = 5548)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5549)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 5550)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5551 || n = 5573)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5552 || n = 5553 || n = 5558 || n = 5559 || n = 5563 || n = 5564)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5555)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5556 || n = 5578)
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
.elseif(n = 5557 || n = 5582)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5560 || (n > 5564 && n < 5569) || n = 5570 || n = 5581 || n = 5583 || n = 5585)
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
.elseif(n = 5561)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5562)
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
.elseif(n = 5569)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5571)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5572)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5575)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5576)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
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
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5579)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5580)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5584)
	; 14 bytes
	sty @zptemp
	ldy #169
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
.if((n >= 5586) && (n < 5606))
; 5586 to 5605
.if(n = 5586 || n = 5588 || n = 5590 || n = 5591)
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
.elseif(n = 5587)
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
.elseif(n = 5589)
	; 14 bytes
	pha
	tya
	ldy #169
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5592)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5593)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5594)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5595)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5596)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5597)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5598)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
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
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5600)
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
.elseif(n = 5601)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5602)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5603)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5604)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5605)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
; 5606 to 5625
.if(n = 5606)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5607)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5608 || n = 5609 || n = 5613 || n = 5614 || n = 5619 || n = 5625)
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
.elseif(n = 5612)
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
.elseif(n = 5615)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5616)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5617)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5618)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5620)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5621)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5622)
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
.elseif(n = 5623)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5624)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
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
.endif
.else
.if((n >= 5626) && (n < 5938))
.if((n >= 5626) && (n < 5782))
.if((n >= 5626) && (n < 5704))
.if((n >= 5626) && (n < 5665))
.if((n >= 5626) && (n < 5645))
; 5626 to 5644
.if(n = 5626)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 5627)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5628)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5629 || n = 5630 || n = 5635 || n = 5636)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 5631)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5632)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	dey
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
.elseif(n = 5634)
	; 12 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0095,X ;hides 'sta $00,X'
	dex
	Jne *-5
	tax
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
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	brk
	dey
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5641)
	; 14 bytes
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5642)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5643)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
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
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5646 || n = 5656 || n = 5659)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5647)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5648 || n = 5658 || n = 5660 || n = 5661)
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
.elseif(n = 5649)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5650)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5651)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5652)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5653)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	nop
	dey
	Jne *-5
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
.elseif(n = 5657)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5662)
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
.elseif(n = 5663)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5664)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5688)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5666 || n = 5667 || n = 5673 || n = 5679 || n = 5681 || n = 5690 || n = 5696 || n = 5697)
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
.elseif(n = 5668)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5669)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5670 || n = 5693)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5671 || n = 5695)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5672 || n = 5678 || n = 5684 || n = 5702)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5674)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5675 || n = 5701)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5676 || n = 5700)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5677)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5680)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5682)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5683)
	; 14 bytes
	sty @zptemp
	ldy #172
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5685)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5686)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 5689)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
	ldx @zptemp
.elseif(n = 5691)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5692)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5694)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	jsr @nearby_rts_14cyc
	Jne *-7
	tax
	pla
.elseif(n = 5698)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5699)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5703)
	; 14 bytes
	sty @zptemp
	ldy #178
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5704) && (n < 5743))
.if((n >= 5704) && (n < 5723))
; 5704 to 5722
.if(n = 5704)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5705)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	inc @zptemp
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5706)
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
.elseif(n = 5707)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5708)
	; 14 bytes
	pha
	tya
	ldy #178
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5709)
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
.elseif(n = 5710)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5711)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5712)
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
.elseif(n = 5713)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5714)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5715)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5716)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5717)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5718)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5719)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5720)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inc @zptemp
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5721)
	; 14 bytes
	pha
	tya
	ldy #173
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5722)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 5723 to 5742
.if(n = 5723)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5724)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5725 || n = 5729 || n = 5730)
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
.elseif(n = 5726)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5727)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5728)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5731)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5732)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5733)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5734)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5735)
	; 14 bytes
	sty @zptemp
	ldy #179
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5736)
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
.elseif(n = 5737)
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
.elseif(n = 5738)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5739)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5740)
	; 14 bytes
	pha
	tya
	ldy #179
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5741)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5742)
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
; 5743 to 5781
.if(n = 5743 || n = 5767)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5744 || n = 5749 || n = 5750 || n = 5759 || n = 5764 || n = 5774 || n = 5775)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5745 || n = 5751 || n = 5752 || n = 5768 || n = 5769 || n = 5777)
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
.elseif(n = 5746)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5747 || n = 5778)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5748 || n = 5772)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5753 || n = 5779)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5754)
	; 14 bytes
	pha
	tya
	ldy #174
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5755)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5756)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5757)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5758)
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
.elseif(n = 5760)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5761)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5762)
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
.elseif(n = 5763)
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
.elseif(n = 5765)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5766)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5770)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5771)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5773)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5776)
	; 14 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 7
	brk
	dey
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5780)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5781)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 7
	brk
	dey
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 5782) && (n < 5860))
.if((n >= 5782) && (n < 5821))
.if((n >= 5782) && (n < 5801))
; 5782 to 5800
.if(n = 5782)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5783)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5784)
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
.elseif(n = 5785)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5786 || n = 5788 || n = 5792 || n = 5793 || n = 5800)
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
.elseif(n = 5787)
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
.elseif(n = 5789)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5790)
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
.elseif(n = 5791)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5794)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5795)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5796)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5797)
	; 14 bytes
	sty @zptemp
	ldy #193
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5798)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5799)
	; 14 bytes
	sty @zptemp
	ldy #181
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 5801 to 5820
.if(n = 5801)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5802)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5803)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5804)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5805)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5806)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5807)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5808)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5809)
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
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5811)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5812)
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
.elseif(n = 5813)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5814)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5815)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5816)
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
.elseif(n = 5817)
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
.elseif(n = 5818)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5819)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5820)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.endif
.endif
.else
; 5821 to 5859
.if(n = 5821 || n = 5823 || n = 5828 || n = 5841 || n = 5845 || n = 5850 || n = 5851 || n = 5858)
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
.elseif(n = 5822 || n = 5833 || n = 5834 || n = 5846 || n = 5856 || n = 5859)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5824)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5825)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5826 || n = 5849)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5827)
	; 14 bytes
	sty @zptemp
	ldy #194
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5829)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5830)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5831 || n = 5854)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5832 || n = 5857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5835)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5836)
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
.elseif(n = 5837)
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
.elseif(n = 5838)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5839)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5840)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5842)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5843)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5844)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5847)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5848)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5852)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5853)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5855)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if(n = 5860)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	inc @zptemp
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5861)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	bit $24
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5863)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5864 || n = 5868 || n = 5873)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5865 || n = 5874 || n = 5878)
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
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5869)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc @zptemp
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5870)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5875)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5876)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5877)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/23
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.endif
.else
; 5879 to 5898
.if(n = 5879 || n = 5891)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5880)
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
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5883)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	bit $24
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5884)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	bit $24
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5885)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5886)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	iny
	Jne *-5
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
	bit $EA ;hides 'nop'
	bit $24
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
.elseif(n = 5890)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5892)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
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
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5895)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5896)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.endif
.else
; 5899 to 5937
.if(n = 5899)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5900)
	; 13 bytes
	pha
	tya
	ldy #0
	inc @zptemp
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5901 || n = 5903 || n = 5905 || n = 5913 || n = 5921 || n = 5923 || n = 5926 || n = 5929 || n = 5931 || n = 5936)
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
.elseif(n = 5902 || n = 5933)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5904 || n = 5908 || n = 5918 || n = 5930)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5906)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	sta @zptemp
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5907 || n = 5932)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5909)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5910)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5911 || n = 5935)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5912 || n = 5937)
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
.elseif(n = 5914)
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
.elseif(n = 5915)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5916)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5917)
	; 14 bytes
	sty @zptemp
	ldy #197
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5919)
	; 14 bytes
	pha
	tya
	ldy #179
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5920)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5922)
	; 14 bytes
	pha
	tya
	ldy #197
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5924)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 19
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5925)
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
.elseif(n = 5927)
	; 14 bytes
	sty @zptemp
	ldy #185
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5928)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5934)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
.if((n >= 5938) && (n < 5977))
; 5938 to 5976
.if(n = 5938)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5939)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5940)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5941 || (n > 5942 && n < 5947) || n = 5948 || n = 5949 || (n > 5952 && n < 5957) || n = 5960 || n = 5965 || (n > 5966 && n < 5974) || n = 5975 || n = 5976)
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
.elseif(n = 5942)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5947 || n = 5974)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5950)
	; 14 bytes
	pha
	txa
	ldx #36 ;hides 'bit zp'
	inc $07F6,X ;hides 'inc $07,X'
	brk
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5951)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 19
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5952)
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
.elseif(n = 5957)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5958)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5959)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5961)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5962)
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
.elseif(n = 5963)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts_15cyc
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5964)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5966)
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
.endif
.else
; 5977 to 6015
.if(n = 5977)
	; 14 bytes
	sty @zptemp
	ldy #199
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5978)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 19
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5979 || n = 6006)
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
.elseif(n = 5980)
	; 14 bytes
	sty @zptemp
	ldy #181
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5981)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5982 || n = 6007)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5983)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5984 || n = 5989 || n = 5997 || n = 6009 || n = 6014)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5985)
	; 14 bytes
	pha
	tya
	ldy #181
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5986)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5987 || n = 6012)
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
.elseif(n = 5988)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5990)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5991)
	; 14 bytes
	sty @zptemp
	ldy #187
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5992)
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
.elseif(n = 5993 || n = 5994 || (n > 5997 && n < 6001) || (n > 6001 && n < 6005) || n = 6008 || n = 6015)
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
.elseif(n = 5995)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5996)
	; 14 bytes
	pha
	tya
	ldy #187
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6001)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6005)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6010)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6011)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6013)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 6016) && (n < 6055))
; 6016 to 6054
.if(n = 6016)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 6017 || n = 6019 || n = 6020 || n = 6022 || n = 6024 || n = 6025 || n = 6027 || n = 6040 || n = 6041 || n = 6043 || n = 6045 || (n > 6046 && n < 6051))
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
.elseif(n = 6018 || n = 6044)
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
.elseif(n = 6021 || n = 6052)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6023)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6026)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6028)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6029)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6030)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6031)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6032)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6033)
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
.elseif(n = 6034)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6035)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6036)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6037)
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
.elseif(n = 6038)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 39
	Jne *-7
	tay
	pla
.elseif(n = 6039)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6042)
	; 13 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 19
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6046)
	; 14 bytes
	sty @zptemp
	ldy #183
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6051)
	; 14 bytes
	pha
	tya
	ldy #183
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6053)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6054)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
; 6055 to 6093
.if(n = 6055 || n = 6079)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6056 || n = 6061 || n = 6063 || n = 6066 || n = 6068 || n = 6069 || n = 6071 || (n > 6072 && n < 6077) || n = 6080 || n = 6081 || n = 6093)
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
.elseif(n = 6057 || n = 6088)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6058)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6059)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6060 || n = 6084)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6062 || n = 6087)
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
.elseif(n = 6064 || n = 6085 || n = 6086 || n = 6090)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6065 || n = 6091)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6067)
	; 14 bytes
	sty @zptemp
	ldy #202
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6070)
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
.elseif(n = 6072)
	; 14 bytes
	pha
	tya
	ldy #202
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6077)
	; 13 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6078)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6082)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6083)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6089)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6092)
	; 14 bytes
	pha
	tya
	ldy #190
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
.if((n >= 6094) && (n < 6113))
; 6094 to 6112
.if(n = 6094)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 19
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6095)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	brk
	Jne *-6
	ldx @zptemp
.elseif(n = 6096)
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
.elseif(n = 6097)
	; 14 bytes
	sty @zptemp
	ldy #203
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6098 || n = 6099 || n = 6104 || n = 6105 || n = 6111)
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
.elseif(n = 6101)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6102)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6103)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6106)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6107)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6108)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/24
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6109)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6110)
	; 14 bytes
	delay_n (n-2)
	nop
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
.endif
.else
; 6113 to 6132
.if(n = 6113 || n = 6115 || n = 6120 || n = 6129)
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
.elseif(n = 6114)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6116)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 6117)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6118 || n = 6123)
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
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc $F6,X
	iny
	Jne *-5
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
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc $F6,X
	iny
	Jne *-5
	tay
	pla
.elseif(n = 6125)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6126)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6127)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6128)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6130)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6131)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6132)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133 || n = 6135)
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
.elseif(n = 6134 || n = 6142 || n = 6147)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6136)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6137)
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
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	nop
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 6139)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6140)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6141)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $F6F6 ;hides 'inc $F6,X'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 6143)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	nop
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6144)
	; 12 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $00F6,X ;hides 'inc $00,X'
	dex
	Jne *-5
	tax
	pla
.elseif(n = 6145)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc $F6,X
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6146)
	; 13 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0095,X ;hides 'sta $00,X'
	nop
	dex
	Jne *-6
	tax
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
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 6152 to 6171
.if(n = 6152 || n = 6158 || n = 6159 || n = 6164)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6153)
	; 14 bytes
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6154)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6155)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6156)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 19
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6157)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6160)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6161)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6162)
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
.elseif(n = 6163)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6165 || n = 6167 || n = 6170)
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
.elseif(n = 6166)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 9
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6168)
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
.elseif(n = 6169)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6171)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172 || n = 6173 || n = 6175 || n = 6177 || n = 6179 || n = 6180 || n = 6185 || n = 6186 || n = 6191 || n = 6193 || (n > 6195 && n < 6199) || (n > 6200 && n < 6207) || n = 6208 || n = 6210 || n = 6213 || n = 6218 || n = 6219 || (n > 6222 && n < 6226) || (n > 6226 && n < 6232) || n = 6233 || n = 6235 || n = 6236 || n = 6241 || n = 6242 || n = 6246 || n = 6248 || n = 6250)
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
.elseif(n = 6174 || n = 6200 || n = 6226)
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
.elseif(n = 6176 || n = 6207 || n = 6238)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6178)
	; 14 bytes
	sty @zptemp
	ldy #187
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6181 || n = 6212 || n = 6243)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6182 || n = 6232)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6183)
	; 14 bytes
	pha
	tya
	ldy #187
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6184 || n = 6189 || n = 6209 || n = 6214 || n = 6234 || n = 6239 || n = 6240 || n = 6245)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6187 || n = 6237)
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
.elseif(n = 6188)
	; 14 bytes
	pha
	tya
	ldy #193
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6190 || n = 6217 || n = 6244)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6192)
	; 14 bytes
	pha
	tya
	ldy #206
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6194)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6195 || n = 6222 || n = 6249)
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
.elseif(n = 6199)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6211)
	; 14 bytes
	sty @zptemp
	ldy #188
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6215)
	; 14 bytes
	sty @zptemp
	ldy #194
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6216)
	; 14 bytes
	pha
	tya
	ldy #188
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6220)
	; 14 bytes
	pha
	tya
	ldy #194
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6221 || n = 6247)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
.if((n >= 6251) && (n < 6290))
; 6251 to 6289
.if(n = 6251 || (n > 6252 && n < 6257) || n = 6258 || n = 6260 || n = 6263 || n = 6265 || n = 6267 || n = 6268 || n = 6272 || n = 6280 || n = 6281 || n = 6283 || n = 6285 || n = 6286 || n = 6288)
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
.elseif(n = 6252 || n = 6278)
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
.elseif(n = 6257 || n = 6282)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6259 || n = 6264 || n = 6284 || n = 6289)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6261)
	; 14 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 27
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6262 || n = 6287)
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
.elseif(n = 6266)
	; 14 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6269)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6270)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 26
	inx
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 6271)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6273)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6274)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6275)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 26
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 6276)
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
.elseif(n = 6277)
	; 14 bytes
	sty @zptemp
	ldy #190
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6279)
	; 14 bytes
	sty @zptemp
	ldy #196
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 6290 to 6328
.if((n > 6289 && n < 6298) || n = 6313 || (n > 6316 && n < 6323) || n = 6324 || n = 6326 || n = 6327)
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
.elseif(n = 6298)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	ldx @zptemp
.elseif(n = 6299)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6300)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6301)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	ldx @zptemp
.elseif(n = 6302 || n = 6309 || n = 6314)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6303)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
	pla
.elseif(n = 6304)
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
.elseif(n = 6305)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6306)
	; 14 bytes
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	delay_n 26
	Jne *-5
	tax
	pla
.elseif(n = 6307)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6308)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 6310)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6311 || n = 6316)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 6312)
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
.elseif(n = 6315)
	; 14 bytes
	pha
	tya
	ldy #191
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6323)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6325)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
.endif
.endif
.else
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if(n = 6329 || n = 6335 || n = 6340 || n = 6341 || (n > 6343 && n < 6348) || n = 6349 || n = 6350 || (n > 6352 && n < 6356) || n = 6358 || n = 6360 || n = 6361 || n = 6363 || n = 6365 || n = 6366)
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
.elseif(n = 6330 || n = 6356)
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
.elseif(n = 6331)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6332 || n = 6357)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6333 || n = 6334 || n = 6338 || n = 6339 || n = 6359 || n = 6364)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6336 || n = 6367)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6337 || n = 6362)
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
.elseif(n = 6342)
	; 14 bytes
	pha
	tya
	ldy #211
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6343)
	; 14 bytes
	sty @zptemp
	ldy #192
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6348)
	; 14 bytes
	pha
	tya
	ldy #192
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6351)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6352)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 6368 to 6406
.if(n = 6368 || n = 6370 || n = 6385 || n = 6386 || n = 6388 || n = 6390 || n = 6392)
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
.elseif(n = 6369 || n = 6373 || n = 6375 || n = 6378 || n = 6380 || n = 6389 || n = 6395 || n = 6402 || n = 6403 || n = 6405)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6371)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 6372)
	; 14 bytes
	pha
	tya
	ldy #212
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6374)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	delay_n 7
	dex
	Jne *-5
	ldx @zptemp
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
.elseif(n = 6379 || n = 6406)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
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
.elseif(n = 6382)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 7
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6383)
	; 14 bytes
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
.elseif(n = 6387)
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
.elseif(n = 6391)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6393)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6394)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	bit $24
	iny
	Jne *-7
	ldy @zptemp
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
	inc $F6,X
	delay_n (n-6)
.elseif(n = 6398)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6399)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	delay_n 9
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 6400)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	inc @zptemp
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6404)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 9
	dey
	Jne *-5
	tay
	pla
.endif
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
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6408 || n = 6434)
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
	pha
	tya
	ldy #<$9D ;hides 'sta $00A0,X'
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6411 || n = 6438)
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6413 || (n > 6414 && n < 6418) || n = 6419 || (n > 6420 && n < 6424) || n = 6425 || n = 6428 || n = 6430 || (n > 6434 && n < 6438) || n = 6440 || n = 6441 || n = 6443)
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
.elseif(n = 6418)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	inc @zptemp
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6420 || n = 6426 || n = 6431)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6424)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6427)
	; 14 bytes
	sty @zptemp
	ldy #214
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6429)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6432)
	; 14 bytes
	pha
	tya
	ldy #214
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6433)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6439)
	; 14 bytes
	sty @zptemp
	ldy #201
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6442)
	; 14 bytes
	sty @zptemp
	ldy #195
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6444)
	; 14 bytes
	pha
	tya
	ldy #201
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6445)
	; 14 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
.endif
.else
; 6446 to 6484
.if(n = 6446 || n = 6448 || n = 6449 || (n > 6450 && n < 6455) || n = 6456 || n = 6458 || n = 6459 || n = 6461 || n = 6463 || n = 6464 || (n > 6465 && n < 6471) || (n > 6471 && n < 6475) || n = 6477 || n = 6479 || (n > 6481 && n < 6485))
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
.elseif(n = 6447)
	; 14 bytes
	pha
	tya
	ldy #195
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6450)
	; 14 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6455)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6457 || n = 6462)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6465)
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
.elseif(n = 6471)
	; 14 bytes
	sty @zptemp
	ldy #202
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6475)
	; 14 bytes
	sty @zptemp
	ldy #196
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6476)
	; 14 bytes
	pha
	tya
	ldy #202
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6478)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 19
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 6480)
	; 14 bytes
	pha
	tya
	ldy #196
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6481)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
; 6485 to 6562
.if(n = 6485 || n = 6489 || n = 6490 || (n > 6493 && n < 6501) || n = 6502 || n = 6504 || n = 6505 || (n > 6508 && n < 6512) || n = 6515 || n = 6516 || n = 6518 || n = 6520 || n = 6521 || n = 6523 || (n > 6524 && n < 6533) || n = 6534 || n = 6536 || n = 6537 || n = 6539 || (n > 6541 && n < 6546) || n = 6549 || n = 6551 || n = 6554 || n = 6556 || n = 6558 || n = 6560 || n = 6561)
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
.elseif(n = 6486 || n = 6517 || n = 6548)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6487 || n = 6514 || n = 6541)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6488 || n = 6493 || n = 6524 || n = 6550 || n = 6555)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6492 || n = 6519 || n = 6546)
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
.elseif(n = 6501)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-6
	ldx @zptemp
.elseif(n = 6503)
	; 14 bytes
	sty @zptemp
	ldy #203
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6506)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-6
	tax
	pla
.elseif(n = 6507 || n = 6533 || n = 6559)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6508)
	; 14 bytes
	sty @zptemp
	ldy #197
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6512 || n = 6538)
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
.elseif(n = 6513)
	; 14 bytes
	pha
	tya
	ldy #197
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6535)
	; 14 bytes
	sty @zptemp
	ldy #204
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6540)
	; 14 bytes
	pha
	tya
	ldy #204
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6547)
	; 14 bytes
	sty @zptemp
	ldy #218
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6552)
	; 14 bytes
	pha
	tya
	ldy #218
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6557)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6562)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if(n = 6563 || n = 6565 || n = 6566 || (n > 6568 && n < 6572) || n = 6575 || n = 6576 || n = 6578 || n = 6580 || n = 6583 || (n > 6586 && n < 6590) || (n > 6590 && n < 6595) || (n > 6595 && n < 6599) || (n > 6600 && n < 6604) || n = 6606 || n = 6608 || n = 6609 || n = 6613 || n = 6614 || (n > 6617 && n < 6622) || (n > 6622 && n < 6626) || (n > 6627 && n < 6631) || n = 6632 || n = 6634 || n = 6635 || n = 6638 || n = 6639)
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
.elseif(n = 6564 || n = 6590 || n = 6616)
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
.elseif(n = 6567)
	; 14 bytes
	sty @zptemp
	ldy #205
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6568 || n = 6595 || n = 6622)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6572)
	; 14 bytes
	pha
	tya
	ldy #205
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6573 || n = 6600 || n = 6627)
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
.elseif(n = 6574)
	; 14 bytes
	sty @zptemp
	ldy #199
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6577)
	; 14 bytes
	sty @zptemp
	ldy #219
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6579)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6581 || n = 6586 || n = 6617 || n = 6633)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6582)
	; 14 bytes
	pha
	tya
	ldy #219
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6585 || n = 6611 || n = 6637)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6599)
	; 14 bytes
	sty @zptemp
	ldy #206
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6604)
	; 14 bytes
	pha
	tya
	ldy #206
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6605)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 26
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 6607)
	; 14 bytes
	sty @zptemp
	ldy #200
	nop
	delay_n 26
	dey
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
.elseif(n = 6612)
	; 14 bytes
	pha
	tya
	ldy #200
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6626)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	.repeat 4
	 nop
	.endrepeat
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6631)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	inc $F6,X
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 6643 || n = 6648 || n = 6659 || n = 6679)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6644 || n = 6647 || (n > 6659 && n < 6663) || (n > 6663 && n < 6667) || (n > 6668 && n < 6672) || n = 6675)
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
.elseif(n = 6650)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	delay_n 4
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6651)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 4
	nop
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6652)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6653)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $F6F6 ;hides 'inc $F6,X'
	nop
	iny
	Jne *-6
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
.elseif(n = 6655)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6656)
	; 13 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $00F6,X ;hides 'inc $00,X'
	nop
	dex
	Jne *-6
	tax
	pla
.elseif(n = 6657)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 4
	inc $F6,X
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6658)
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0095,X ;hides 'sta $00,X'
	delay_n 4
	dex
	Jne *-7
	tax
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
	brk
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
.elseif(n = 6673)
	; 14 bytes
	sty @zptemp
	ldy #202
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6674)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	inc $F6,X
	dey
	Jne *-6
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
.else
; 6680 to 6718
.if(n = 6680 || (n > 6681 && n < 6695) || n = 6696 || n = 6698 || n = 6699 || n = 6701 || n = 6707 || (n > 6711 && n < 6719))
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
.elseif(n = 6695)
	; 14 bytes
	sty @zptemp
	ldy #209
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6697)
	; 14 bytes
	sty @zptemp
	ldy #223
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6700)
	; 14 bytes
	pha
	tya
	ldy #209
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6702)
	; 14 bytes
	pha
	tya
	ldy #223
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6703)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6704)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	delay_n 26
	Jne *-6
	ldx @zptemp
.elseif(n = 6705 || n = 6710)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6706)
	; 14 bytes
	sty @zptemp
	ldy #203
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6708)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6709)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop
	delay_n 26
	Jne *-6
	tax
	pla
.elseif(n = 6711)
	; 14 bytes
	pha
	tya
	ldy #203
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 6719) && (n < 6797))
; 6719 to 6796
.if((n > 6718 && n < 6727) || n = 6728 || n = 6729 || n = 6731 || n = 6737 || n = 6740 || n = 6742 || n = 6743 || (n > 6744 && n < 6757) || n = 6758 || n = 6760 || n = 6761 || n = 6763 || n = 6766 || n = 6768 || n = 6769 || n = 6771 || (n > 6772 && n < 6777) || (n > 6777 && n < 6784) || n = 6785 || n = 6786 || n = 6788 || n = 6790 || (n > 6792 && n < 6796))
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
.elseif(n = 6727)
	; 14 bytes
	sty @zptemp
	ldy #224
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6730 || n = 6757 || n = 6784)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6732)
	; 14 bytes
	pha
	tya
	ldy #224
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6733)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6734 || n = 6765 || n = 6796)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6735 || n = 6762 || n = 6789)
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
.elseif(n = 6736 || n = 6741 || n = 6767 || n = 6772)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6738)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts_15cyc
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6739 || n = 6770)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6744)
	; 14 bytes
	pha
	tya
	ldy #204
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6759)
	; 14 bytes
	sty @zptemp
	ldy #211
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6764)
	; 14 bytes
	pha
	tya
	ldy #211
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6777)
	; 14 bytes
	pha
	tya
	ldy #205
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6787)
	; 14 bytes
	sty @zptemp
	ldy #226
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6791)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6792)
	; 14 bytes
	pha
	tya
	ldy #226
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6797 to 6875
.if(n = 6797 || n = 6799 || n = 6800 || n = 6802 || n = 6804 || (n > 6805 && n < 6810) || (n > 6811 && n < 6816) || (n > 6817 && n < 6822) || (n > 6823 && n < 6827) || n = 6830 || n = 6831 || n = 6833 || (n > 6834 && n < 6838) || (n > 6838 && n < 6843) || (n > 6843 && n < 6847) || (n > 6847 && n < 6852) || n = 6853 || n = 6854 || n = 6856 || n = 6857 || n = 6859 || n = 6861 || n = 6862 || n = 6864 || (n > 6865 && n < 6870) || (n > 6871 && n < 6876))
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
.elseif(n = 6798 || n = 6803 || n = 6829 || n = 6834 || n = 6860)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6801 || n = 6832 || n = 6863)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6805)
	; 14 bytes
	sty @zptemp
	ldy #206
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6810)
	; 14 bytes
	pha
	tya
	ldy #206
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6811 || n = 6838 || n = 6865)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6816 || n = 6843 || n = 6870)
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
.elseif(n = 6817)
	; 14 bytes
	sty @zptemp
	ldy #227
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6822)
	; 14 bytes
	pha
	tya
	ldy #227
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6823)
	; 14 bytes
	sty @zptemp
	ldy #213
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6827 || n = 6858)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6828)
	; 14 bytes
	pha
	tya
	ldy #213
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6847)
	; 14 bytes
	sty @zptemp
	ldy #228
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6852)
	; 14 bytes
	pha
	tya
	ldy #228
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6855)
	; 14 bytes
	sty @zptemp
	ldy #214
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6871)
	; 14 bytes
	sty @zptemp
	ldy #208
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
; 6876 to 6953
.if(n = 6876)
	; 14 bytes
	pha
	tya
	ldy #208
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6877)
	; 14 bytes
	sty @zptemp
	ldy #229
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif((n > 6877 && n < 6881) || n = 6884 || n = 6885 || n = 6890 || n = 6893 || n = 6895 || (n > 6897 && n < 6903) || n = 6906 || (n > 6915 && n < 6919) || n = 6921 || n = 6923 || n = 6926 || n = 6928 || n = 6929 || n = 6931 || (n > 6932 && n < 6937) || (n > 6937 && n < 6942) || (n > 6942 && n < 6951) || n = 6952)
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
.elseif(n = 6881)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 6882)
	; 14 bytes
	pha
	tya
	ldy #229
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6883 || n = 6888 || n = 6891 || n = 6896 || n = 6905 || n = 6910 || n = 6914 || n = 6915 || n = 6922 || n = 6927 || n = 6932 || n = 6953)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6887)
	; 14 bytes
	sty @zptemp
	ldy #215
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6889 || n = 6920 || n = 6951)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6892)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6894 || n = 6925)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 6903)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6904)
	; 14 bytes
	sty @zptemp
	ldy #209
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6907)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
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
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	inc @zptemp
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6912)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 7
	dey
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
.elseif(n = 6919)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
.elseif(n = 6930)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6937)
	; 14 bytes
	sty @zptemp
	ldy #210
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6942)
	; 14 bytes
	pha
	tya
	ldy #210
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6954 to 7031
.if(n = 6954 || n = 6955 || n = 6957 || (n > 6958 && n < 6967) || n = 6968 || n = 6969 || n = 6971 || n = 6973 || n = 6974 || (n > 6975 && n < 6982) || n = 6985 || n = 6986 || (n > 6989 && n < 6997) || (n > 6997 && n < 7002) || n = 7004 || n = 7006 || n = 7007 || n = 7009 || n = 7011 || n = 7012 || n = 7014 || n = 7016 || n = 7017 || n = 7019 || (n > 7020 && n < 7027) || (n > 7027 && n < 7032))
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
.elseif(n = 6956 || n = 6987 || n = 7018)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6958 || n = 6984 || n = 6989 || n = 7015 || n = 7020)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6967)
	; 14 bytes
	sty @zptemp
	ldy #232
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6970)
	; 14 bytes
	sty @zptemp
	ldy #211
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6972)
	; 14 bytes
	pha
	tya
	ldy #232
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6975)
	; 14 bytes
	pha
	tya
	ldy #211
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6982 || n = 7013)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 6983)
	; 14 bytes
	sty @zptemp
	ldy #218
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6988)
	; 14 bytes
	pha
	tya
	ldy #218
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6997)
	; 14 bytes
	sty @zptemp
	ldy #233
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7002)
	; 14 bytes
	pha
	tya
	ldy #233
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7003)
	; 14 bytes
	sty @zptemp
	ldy #212
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7005)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7008)
	; 14 bytes
	pha
	tya
	ldy #212
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7010)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7027)
	; 14 bytes
	sty @zptemp
	ldy #234
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if(n = 7032)
	; 14 bytes
	pha
	tya
	ldy #234
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif((n > 7032 && n < 7036) || (n > 7036 && n < 7041) || n = 7042 || n = 7043 || n = 7045 || n = 7048 || n = 7050 || (n > 7052 && n < 7057) || (n > 7057 && n < 7062) || (n > 7062 && n < 7069) || (n > 7069 && n < 7074) || n = 7076 || n = 7078 || n = 7081 || n = 7083 || n = 7085 || n = 7086 || (n > 7087 && n < 7092) || (n > 7092 && n < 7102) || (n > 7102 && n < 7106) || n = 7109)
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
.elseif(n = 7036)
	; 14 bytes
	sty @zptemp
	ldy #213
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7041)
	; 14 bytes
	pha
	tya
	ldy #213
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7044 || n = 7075 || n = 7106)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7046 || n = 7051 || n = 7077 || n = 7082 || n = 7108)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7047)
	; 14 bytes
	sty @zptemp
	ldy #220
	delay_n 27
	dey
	Jne *-6
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
.elseif(n = 7052)
	; 14 bytes
	pha
	tya
	ldy #220
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7057)
	; 14 bytes
	sty @zptemp
	ldy #235
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7062)
	; 14 bytes
	pha
	tya
	ldy #235
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7069)
	; 14 bytes
	sty @zptemp
	ldy #214
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7074)
	; 14 bytes
	pha
	tya
	ldy #214
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7079)
	; 14 bytes
	sty @zptemp
	ldy #221
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7084)
	; 14 bytes
	pha
	tya
	ldy #221
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7087)
	; 14 bytes
	sty @zptemp
	ldy #236
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7092)
	; 14 bytes
	pha
	tya
	ldy #236
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7102)
	; 14 bytes
	sty @zptemp
	ldy #215
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7107)
	; 14 bytes
	pha
	tya
	ldy #215
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if(n = 7110)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	ldx @zptemp
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
.elseif(n = 7112 || n = 7114 || (n > 7117 && n < 7122) || (n > 7122 && n < 7135) || n = 7138 || n = 7145 || n = 7146)
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
.elseif(n = 7113 || n = 7139 || n = 7144)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7115)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	tax
	pla
.elseif(n = 7116)
	; 14 bytes
	pha
	tya
	ldy #222
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7117)
	; 14 bytes
	sty @zptemp
	ldy #237
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7122)
	; 14 bytes
	pha
	tya
	ldy #237
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7135)
	; 14 bytes
	sty @zptemp
	ldy #216
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7136)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7137)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7140)
	; 14 bytes
	pha
	tya
	ldy #216
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7141)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	rol $26
	ror $26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7143)
	; 14 bytes
	sty @zptemp
	ldy #223
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7147)
	; 14 bytes
	sty @zptemp
	ldy #238
	jsr @nearby_rts
	brk
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
.if((n > 7148 && n < 7152) || (n > 7152 && n < 7159) || (n > 7159 && n < 7163) || n = 7172 || n = 7174 || n = 7176 || n = 7178 || n = 7179 || n = 7181 || (n > 7182 && n < 7186) || n = 7187)
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
.elseif(n = 7159)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	bit $24
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7163)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	delay_n 7
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7164)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7165)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $F6F6 ;hides 'inc $F6,X'
	delay_n 4
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7166)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	inc @zptemp
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7167)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	inc $F6,X
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 7170 || n = 7175)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7171)
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0085,X ;hides 'sta $00'
	delay_n 7
	dex
	Jne *-7
	tax
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
.elseif(n = 7177)
	; 14 bytes
	sty @zptemp
	ldy #239
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.elseif(n = 7186)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	nop
	inc $F6,X
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
; 7188 to 7265
.if((n > 7187 && n < 7199) || n = 7200 || n = 7202 || n = 7203 || n = 7205 || (n > 7207 && n < 7212) || (n > 7212 && n < 7230) || n = 7231 || n = 7233 || n = 7236 || n = 7238 || n = 7240 || n = 7241 || n = 7243 || (n > 7244 && n < 7261) || n = 7262 || n = 7264 || n = 7265)
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
.elseif(n = 7199 || n = 7230 || n = 7261)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7201)
	; 14 bytes
	sty @zptemp
	ldy #218
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7204 || n = 7235)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7206)
	; 14 bytes
	pha
	tya
	ldy #218
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7207)
	; 14 bytes
	sty @zptemp
	ldy #240
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7212)
	; 14 bytes
	pha
	tya
	ldy #240
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7232 || n = 7237 || n = 7263)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7234)
	; 14 bytes
	sty @zptemp
	ldy #219
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7239)
	; 14 bytes
	pha
	tya
	ldy #219
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7242)
	; 14 bytes
	pha
	tya
	ldy #241
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7244)
	; 14 bytes
	pha
	tya
	ldy #226
	delay_n 27
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7267)
	; 14 bytes
	sty @zptemp
	ldy #220
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7268 || n = 7294 || n = 7299 || n = 7325 || n = 7330)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7269 || n = 7270 || (n > 7272 && n < 7276) || (n > 7276 && n < 7292) || n = 7293 || n = 7295 || n = 7296 || n = 7298 || n = 7301 || n = 7304 || n = 7306 || n = 7307 || (n > 7308 && n < 7323) || n = 7324 || n = 7326 || n = 7329 || n = 7331 || n = 7334 || n = 7336 || n = 7337 || n = 7339 || (n > 7340 && n < 7344))
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
.elseif(n = 7271)
	; 14 bytes
	sty @zptemp
	ldy #227
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7272)
	; 14 bytes
	pha
	tya
	ldy #220
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7276)
	; 14 bytes
	pha
	tya
	ldy #227
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7292 || n = 7323)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7300)
	; 14 bytes
	sty @zptemp
	ldy #221
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7302)
	; 14 bytes
	pha
	tya
	ldy #243
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7303)
	; 14 bytes
	sty @zptemp
	ldy #228
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7305)
	; 14 bytes
	pha
	tya
	ldy #221
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7308)
	; 14 bytes
	pha
	tya
	ldy #228
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7327)
	; 14 bytes
	sty @zptemp
	ldy #244
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7332)
	; 14 bytes
	pha
	tya
	ldy #244
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7333)
	; 14 bytes
	sty @zptemp
	ldy #222
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7335)
	; 14 bytes
	sty @zptemp
	ldy #229
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7338)
	; 14 bytes
	pha
	tya
	ldy #222
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7340)
	; 14 bytes
	pha
	tya
	ldy #229
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7354) || n = 7355 || n = 7358 || n = 7360 || (n > 7362 && n < 7366) || (n > 7367 && n < 7371) || (n > 7372 && n < 7385) || n = 7386 || n = 7388 || n = 7389 || (n > 7392 && n < 7396) || n = 7397 || n = 7398 || (n > 7399 && n < 7404) || (n > 7404 && n < 7415))
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
.elseif(n = 7354 || n = 7385 || n = 7416)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7356 || n = 7361 || n = 7387 || n = 7392 || n = 7418)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7357)
	; 14 bytes
	sty @zptemp
	ldy #245
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 7362)
	; 14 bytes
	pha
	tya
	ldy #245
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7366)
	; 14 bytes
	sty @zptemp
	ldy #223
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7367)
	; 14 bytes
	sty @zptemp
	ldy #230
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7371)
	; 14 bytes
	pha
	tya
	ldy #223
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7372)
	; 14 bytes
	pha
	tya
	ldy #230
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7391)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 11
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7396)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 11
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7399)
	; 14 bytes
	sty @zptemp
	ldy #224
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 7415)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 4
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7417)
	; 14 bytes
	sty @zptemp
	ldy #247
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7419)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
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
	ldy #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	inc $F6,X
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7423 || n = 7449 || n = 7454)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	delay_n 9
	dey
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
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0095,X ;hides 'sta $00,X'
	delay_n 7
	dex
	Jne *-7
	tax
	pla
.elseif(n = 7427 || n = 7428 || n = 7430 || n = 7433 || n = 7435 || (n > 7437 && n < 7442) || (n > 7442 && n < 7447) || n = 7448 || n = 7450 || n = 7451 || n = 7453 || n = 7455 || n = 7456 || (n > 7457 && n < 7461))
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
.elseif(n = 7429)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	brk
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
	brk
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
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7447)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 7457)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 7461 to 7500
.if(n = 7461 || n = 7462 || n = 7464 || n = 7466 || n = 7467 || n = 7469 || (n > 7470 && n < 7475) || n = 7476 || n = 7479 || n = 7481 || n = 7484 || (n > 7486 && n < 7491) || n = 7492 || n = 7494 || n = 7496 || n = 7497 || n = 7499)
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
.elseif(n = 7463)
	; 14 bytes
	sty @zptemp
	ldy #233
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7465)
	; 14 bytes
	sty @zptemp
	ldy #226
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7468)
	; 14 bytes
	pha
	tya
	ldy #233
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7470)
	; 14 bytes
	pha
	tya
	ldy #226
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7475)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7477)
	; 14 bytes
	sty @zptemp
	ldy #249
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7478)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7480)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7482)
	; 14 bytes
	pha
	tya
	ldy #249
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7485)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7486)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7491)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7493)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 7495)
	; 14 bytes
	sty @zptemp
	ldy #234
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7498)
	; 14 bytes
	sty @zptemp
	ldy #227
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7500)
	; 14 bytes
	pha
	tya
	ldy #234
	delay_n 27
	dey
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
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
.if((n >= 7501) && (n < 7657))
.if((n >= 7501) && (n < 7579))
; 7501 to 7578
.if(n = 7501 || n = 7502 || (n > 7503 && n < 7507) || n = 7508 || n = 7510 || n = 7513 || n = 7515 || (n > 7516 && n < 7520) || (n > 7520 && n < 7525) || n = 7526 || (n > 7527 && n < 7531) || (n > 7532 && n < 7536) || n = 7538 || n = 7539 || n = 7541 || n = 7543 || n = 7544 || n = 7546 || (n > 7547 && n < 7559) || (n > 7559 && n < 7564) || n = 7565 || n = 7566 || n = 7568 || n = 7570 || n = 7574 || n = 7575 || n = 7577)
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
.elseif(n = 7503)
	; 14 bytes
	pha
	tya
	ldy #227
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7507)
	; 14 bytes
	sty @zptemp
	ldy #250
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7509 || n = 7540 || n = 7571)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7511 || n = 7516 || n = 7542 || n = 7547 || n = 7573 || n = 7578)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7512)
	; 14 bytes
	pha
	tya
	ldy #250
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7514 || n = 7545 || n = 7576)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7520)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7525)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7527)
	; 14 bytes
	sty @zptemp
	ldy #235
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7531)
	; 14 bytes
	sty @zptemp
	ldy #228
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7532)
	; 14 bytes
	pha
	tya
	ldy #235
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7536)
	; 14 bytes
	pha
	tya
	ldy #228
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7537)
	; 14 bytes
	sty @zptemp
	ldy #251
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7559)
	; 14 bytes
	sty @zptemp
	ldy #236
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7564)
	; 14 bytes
	sty @zptemp
	ldy #229
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7567)
	; 14 bytes
	sty @zptemp
	ldy #252
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7569)
	; 14 bytes
	pha
	tya
	ldy #229
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7572)
	; 14 bytes
	pha
	tya
	ldy #252
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 7579 to 7656
.if((n > 7578 && n < 7591) || (n > 7591 && n < 7596) || (n > 7597 && n < 7602) || n = 7603 || n = 7605 || n = 7606 || n = 7608 || (n > 7609 && n < 7623) || (n > 7623 && n < 7627) || n = 7629 || n = 7631 || n = 7634 || n = 7636 || n = 7637 || n = 7639 || (n > 7640 && n < 7646) || n = 7647 || n = 7649 || n = 7650 || n = 7652 || n = 7654 || n = 7656)
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
.elseif(n = 7591)
	; 14 bytes
	sty @zptemp
	ldy #237
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7596)
	; 14 bytes
	pha
	tya
	ldy #237
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7597)
	; 14 bytes
	sty @zptemp
	ldy #230
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7602 || n = 7633)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7604 || n = 7609 || n = 7640 || n = 7648 || n = 7653)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7607 || n = 7638)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7623)
	; 14 bytes
	sty @zptemp
	ldy #238
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7627)
	; 14 bytes
	sty @zptemp
	ldy #254
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7628)
	; 14 bytes
	pha
	tya
	ldy #238
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7630)
	; 14 bytes
	sty @zptemp
	ldy #231
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7632)
	; 14 bytes
	pha
	tya
	ldy #254
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7635)
	; 14 bytes
	pha
	tya
	ldy #231
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7655)
	; 14 bytes
	sty @zptemp
	ldy #239
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
; 7657 to 7695
.if(n = 7657)
	; 14 bytes
	sty @zptemp
	ldy #255
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7658 || n = 7659 || n = 7665 || n = 7667 || n = 7670 || (n > 7671 && n < 7676) || n = 7679 || n = 7682 || (n > 7683 && n < 7687) || (n > 7687 && n < 7692) || n = 7693 || n = 7694)
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
	brk
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
.elseif(n = 7664 || n = 7695)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 7671 || n = 7683)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7676)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7677)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $F6F6 ;hides 'inc $F6,X'
	inc $F6,X
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7678)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7680)
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $00F6,X ;hides 'inc $00,X'
	inc $F6,X
	dex
	Jne *-7
	tax
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
.elseif(n = 7687)
	; 14 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 7697 || n = 7702 || n = 7728 || n = 7733)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7698 || n = 7699 || (n > 7702 && n < 7719) || (n > 7719 && n < 7724) || n = 7725 || n = 7730)
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
.elseif(n = 7700 || n = 7731)
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
.elseif(n = 7719)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 26
	Jne *-7
	ldx @zptemp
.elseif(n = 7724)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 7
	delay_n 26
	Jne *-7
	tax
	pla
.elseif(n = 7726)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7727)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7729)
	; 14 bytes
	sty @zptemp
	ldy #234
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7732)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7734)
	; 14 bytes
	pha
	tya
	ldy #234
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 7735 to 7812
.if((n > 7734 && n < 7751) || (n > 7751 && n < 7756) || n = 7758 || n = 7760 || n = 7761 || n = 7763 || n = 7765 || n = 7766 || (n > 7767 && n < 7783) || (n > 7783 && n < 7788) || n = 7789 || n = 7791 || n = 7792 || n = 7794 || (n > 7795 && n < 7800) || (n > 7800 && n < 7813))
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
.elseif(n = 7751)
	; 14 bytes
	sty @zptemp
	ldy #242
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7756)
	; 14 bytes
	pha
	tya
	ldy #242
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7757 || n = 7788)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7759 || n = 7764 || n = 7790 || n = 7795)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7767)
	; 14 bytes
	pha
	tya
	ldy #235
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7783)
	; 14 bytes
	sty @zptemp
	ldy #243
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7800)
	; 14 bytes
	pha
	tya
	ldy #236
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if(n = 7813 || n = 7814 || (n > 7815 && n < 7819) || n = 7822 || n = 7823 || n = 7825 || n = 7827 || (n > 7828 && n < 7833) || (n > 7833 && n < 7847) || n = 7848 || n = 7849 || n = 7851 || n = 7853 || n = 7854 || n = 7856 || (n > 7857 && n < 7861) || (n > 7861 && n < 7866) || (n > 7866 && n < 7879) || n = 7880 || n = 7882 || n = 7885 || n = 7887 || n = 7889 || n = 7890)
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
.elseif(n = 7815)
	; 14 bytes
	sty @zptemp
	ldy #244
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7819 || n = 7850 || n = 7881)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7820)
	; 14 bytes
	pha
	tya
	ldy #244
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7821 || n = 7826 || n = 7852 || n = 7857 || n = 7883 || n = 7888)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7828)
	; 14 bytes
	sty @zptemp
	ldy #237
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7833)
	; 14 bytes
	pha
	tya
	ldy #237
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7847)
	; 14 bytes
	sty @zptemp
	ldy #245
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7861)
	; 14 bytes
	sty @zptemp
	ldy #238
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7866)
	; 14 bytes
	pha
	tya
	ldy #238
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7879)
	; 14 bytes
	sty @zptemp
	ldy #246
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7884)
	; 14 bytes
	pha
	tya
	ldy #246
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if(n = 7891 || n = 7892 || (n > 7894 && n < 7898) || n = 7900 || n = 7902 || n = 7915 || n = 7918 || n = 7920 || n = 7922)
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
.elseif(n = 7893)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 27
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 7894)
	; 14 bytes
	sty @zptemp
	ldy #239
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7898)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 27
	iny
	Jne *-7
	tay
	pla
.elseif(n = 7899)
	; 14 bytes
	pha
	tya
	ldy #239
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7901)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 7903 || n = 7905 || n = 7908 || n = 7910 || n = 7914 || n = 7919 || n = 7923 || n = 7925 || n = 7928)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7904)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	dex ;first loop only
	brk
	dex
	Jne *-5
	ldx @zptemp
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
.elseif(n = 7907)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 7909)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 7911)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 7912)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/31
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7913)
	; 14 bytes
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
.elseif(n = 7921)
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7924 || n = 7929)
	; 14 bytes
	delay_n (n-3)
	bit $24
.elseif(n = 7926)
	; 12 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7927)
	; 14 bytes
	sty @zptemp
	ldy #240
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 7930 to 7968
.if(n = 7930 || n = 7934 || n = 7936 || n = 7939 || n = 7941 || n = 7945 || n = 7950)
	; 13..14 bytes
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
	; 12 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7933)
	; 14 bytes
	php
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 7935)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #0 ;hides 'brk imm'
	nop
	brk
	dex
	Jne *-5
	ldx @zptemp
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
.elseif(n = 7938)
	; 14 bytes
	inc $F6,X
	delay_n (n-6)
.elseif(n = 7940)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
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
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 7944)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 7946 || n = 7947 || n = 7949 || (n > 7950 && n < 7960) || (n > 7961 && n < 7965) || n = 7967 || n = 7968)
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
.elseif(n = 7965)
	; 14 bytes
	pha
	tya
	ldy #241
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7966)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 27
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7975) || (n > 7975 && n < 7980) || (n > 7980 && n < 7993) || (n > 7993 && n < 7998) || (n > 7998 && n < 8007) || (n > 8007 && n < 8012) || (n > 8012 && n < 8026) || (n > 8026 && n < 8031) || (n > 8031 && n < 8039) || (n > 8039 && n < 8044) || (n > 8044 && n < 8059) || (n > 8059 && n < 8064) || (n > 8064 && n < 8071) || (n > 8071 && n < 8076) || (n > 8076 && n < 8090) || n = 8091 || n = 8093 || n = 8094 || n = 8096 || (n > 8097 && n < 8103) || (n > 8103 && n < 8108) || (n > 8108 && n < 8125))
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
.elseif(n = 7975)
	; 14 bytes
	sty @zptemp
	ldy #249
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7980)
	; 14 bytes
	pha
	tya
	ldy #249
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 7993)
	; 14 bytes
	sty @zptemp
	ldy #242
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 7998)
	; 14 bytes
	pha
	tya
	ldy #242
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8007)
	; 14 bytes
	sty @zptemp
	ldy #250
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8012)
	; 14 bytes
	pha
	tya
	ldy #250
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8026)
	; 14 bytes
	sty @zptemp
	ldy #243
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8031)
	; 14 bytes
	pha
	tya
	ldy #243
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8039)
	; 14 bytes
	sty @zptemp
	ldy #251
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8044)
	; 14 bytes
	pha
	tya
	ldy #251
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8059)
	; 14 bytes
	sty @zptemp
	ldy #244
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8064)
	; 14 bytes
	pha
	tya
	ldy #244
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8071)
	; 14 bytes
	sty @zptemp
	ldy #252
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8076)
	; 14 bytes
	pha
	tya
	ldy #252
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8090)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 26
	dex
	dex
	Jne *-7
	ldx @zptemp
.elseif(n = 8092)
	; 14 bytes
	sty @zptemp
	ldy #245
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8095)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 26
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 8097)
	; 14 bytes
	pha
	tya
	ldy #245
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8103)
	; 14 bytes
	sty @zptemp
	ldy #253
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8108)
	; 14 bytes
	pha
	tya
	ldy #253
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8125)
	; 14 bytes
	sty @zptemp
	ldy #246
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
; 8126 to 8203
.if((n > 8125 && n < 8130) || (n > 8130 && n < 8135) || (n > 8135 && n < 8140) || (n > 8140 && n < 8156) || n = 8157 || n = 8159 || n = 8160 || n = 8162 || (n > 8163 && n < 8167) || (n > 8167 && n < 8172) || (n > 8172 && n < 8183) || (n > 8183 && n < 8187) || n = 8189 || n = 8194 || n = 8197 || n = 8198 || (n > 8200 && n < 8204))
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
.elseif(n = 8130)
	; 14 bytes
	pha
	tya
	ldy #246
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8135)
	; 14 bytes
	sty @zptemp
	ldy #254
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8140)
	; 14 bytes
	pha
	tya
	ldy #254
	delay_n 27
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8156)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 8158)
	; 14 bytes
	sty @zptemp
	ldy #247
	nop
	delay_n 26
	dey
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
.elseif(n = 8163)
	; 14 bytes
	pha
	tya
	ldy #247
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8167)
	; 14 bytes
	sty @zptemp
	ldy #255
	delay_n 27
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 8183)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8187)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8188)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8190)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8191)
	; 14 bytes
	sty @zptemp
	ldy #248
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8192)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.endif
.else
; 8204 to 8281
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
.elseif((n > 8204 && n < 8210) || (n > 8210 && n < 8224) || (n > 8224 && n < 8229) || (n > 8229 && n < 8257) || (n > 8257 && n < 8262) || (n > 8262 && n < 8282))
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
.elseif(n = 8210)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8224)
	; 14 bytes
	sty @zptemp
	ldy #249
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8229)
	; 14 bytes
	pha
	tya
	ldy #249
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8257)
	; 14 bytes
	sty @zptemp
	ldy #250
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8262)
	; 14 bytes
	pha
	tya
	ldy #250
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 8282 to 8437
.if((n > 8281 && n < 8290) || (n > 8290 && n < 8295) || (n > 8295 && n < 8323) || (n > 8323 && n < 8328) || (n > 8328 && n < 8356) || (n > 8356 && n < 8361) || (n > 8361 && n < 8389) || (n > 8389 && n < 8394) || (n > 8394 && n < 8411) || n = 8412 || n = 8414 || n = 8415 || n = 8417 || (n > 8418 && n < 8422) || (n > 8422 && n < 8427) || (n > 8427 && n < 8433) || n = 8434 || n = 8436 || n = 8437)
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
.elseif(n = 8290)
	; 14 bytes
	sty @zptemp
	ldy #251
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8295)
	; 14 bytes
	pha
	tya
	ldy #251
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8323)
	; 14 bytes
	sty @zptemp
	ldy #252
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8328)
	; 14 bytes
	pha
	tya
	ldy #252
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8356)
	; 14 bytes
	sty @zptemp
	ldy #253
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8361)
	; 14 bytes
	pha
	tya
	ldy #253
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8389)
	; 14 bytes
	sty @zptemp
	ldy #254
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8394)
	; 14 bytes
	pha
	tya
	ldy #254
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8411)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	brk
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 8413 || n = 8418 || n = 8435)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8422)
	; 14 bytes
	sty @zptemp
	ldy #255
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 8433)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
; 8438 to 8750
.if(n = 8438)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8439 || n = 8441 || n = 8442 || n = 8447 || (n > 8451 && n < 8455) || (n > 8455 && n < 8460) || (n > 8460 && n < 8466) || n = 8467 || (n > 8468 && n < 8666) || (n > 8666 && n < 8671) || (n > 8671 && n < 8689) || (n > 8689 && n < 8694) || (n > 8694 && n < 8699) || (n > 8700 && n < 8704) || n = 8706 || (n > 8707 && n < 8722) || (n > 8722 && n < 8751))
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
.elseif(n = 8440 || n = 8445 || n = 8446 || n = 8450 || n = 8451 || n = 8468)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8443)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8444)
	; 13 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	brk
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 8448)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	brk
	dey
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
.elseif(n = 8455)
	; 14 bytes
	sty @zptemp
	ldy #0
	nop
	delay_n 26
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 8466)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 8666)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8671)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	bit $24
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8689)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	bit $24
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8694)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	bit $24
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8699)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8700)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	bit $24
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8704)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $FF24 ;hides 'bit zp'
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
	bit $24
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8707)
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0085,X ;hides 'sta $00'
	brk
	dex
	Jne *-7
	tax
	pla
.elseif(n = 8722)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 8751) && (n < 9376))
; 8751 to 9375
.if((n > 8750 && n < 8921) || (n > 8921 && n < 8926) || (n > 8926 && n < 8937) || (n > 8937 && n < 8942) || n = 8943 || n = 8944 || (n > 8945 && n < 8950) || (n > 8950 && n < 8954) || n = 8957 || n = 8958 || (n > 8962 && n < 8978) || (n > 8978 && n < 9181) || (n > 9181 && n < 9206) || (n > 9206 && n < 9214) || n = 9215 || n = 9216 || (n > 9217 && n < 9376))
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
.elseif(n = 8921)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8926)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 4
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8937)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	ldx @zptemp
.elseif(n = 8942)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
	pla
.elseif(n = 8945)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8950)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8954)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 8955)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8956)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 4
	nop
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 8959)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EAEA ;hides two 'nop's
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8960)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $EA ;hides 'nop'
	nop
	brk
	dey
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
.elseif(n = 8962)
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $0095,X ;hides 'sta $00,X'
	brk
	dex
	Jne *-7
	tax
	pla
.elseif(n = 8978)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	inc $EA,X ;hides 'nop'
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9181)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	inc @zptemp
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9206)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inc @zptemp
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9214)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9217)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	inc @zptemp
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 9376 to 10000
.if((n > 9375 && n < 9436) || (n > 9436 && n < 9462) || (n > 9462 && n < 9469) || n = 9470 || n = 9471 || (n > 9473 && n < 9686) || (n > 9686 && n < 9691) || (n > 9691 && n < 9713) || (n > 9713 && n < 9718) || (n > 9719 && n < 9724) || (n > 9724 && n < 9729) || (n > 9729 && n < 10001))
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
.elseif(n = 9436)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 19
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9462)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	inc $F6,X
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9469)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	bit $F6F6 ;hides 'inc $F6,X'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9472)
	; 14 bytes
	pha
	txa
	ldx #0 ;hides 'brk imm'
	inc $00F6,X ;hides 'inc $00,X'
	brk
	dex
	Jne *-7
	tax
	pla
.elseif(n = 9473)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 19
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9686)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 9691)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 9713)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9718)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 9719)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 9724)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	delay_n 9
	brk
	dey
	Jne *-7
	ldy @zptemp
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
.endif
.endif
.endif
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 11216) || (n > 11216 && n < 11221) || (n > 11221 && n < 11249) || (n > 11249 && n < 11254) || (n > 11254 && n < 11260) || (n > 11260 && n < 11265) || (n > 11265 && n < 20001))
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
.elseif(n = 11216)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 11221)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 11249)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 11254)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 11260)
	; 14 bytes
	sty @zptemp
	ldy #0 ;hides 'brk imm'
	nop
	delay_n 26
	dey
	Jne *-7
	ldy @zptemp
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
.endif
.endif

.endif
	rest
.endmacro
