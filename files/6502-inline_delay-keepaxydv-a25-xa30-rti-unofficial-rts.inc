; delay_n, delays n cycles with small code
.setcpu "6502X"
; Warning: Unofficial instructions are used!
;
; Will clobber Z+N, and C
; Will preserve A, X, Y, S, D, V, and I (interrupt-disable flag)
;
; Requires a dummy interrupt handler that does nothing but RTI.
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
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
; - 2 bytes or less when n <= 4
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 17
; - 5 bytes or less when n <= 20
; - 6 bytes or less when n <= 31
; - 7 bytes or less when n <= 33
; - 8 bytes or less when n <= 44
; - 9 bytes or less when n <= 297
; -10 bytes or less when n <= 299
; -11 bytes or less when n <= 322
; -12 bytes or less when n <= 1328
; -13 bytes or less when n <= 2360
; -14 bytes or less when n <= 3765
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.7, max=4
; - When 11 <= n <= 20: min=2, avg=3.8, max=5
; - When 21 <= n <= 30: min=4, avg=5.5, max=6
; - When 31 <= n <= 50: min=6, avg=7.5, max=9
; - When 51 <= n <= 75: min=7, avg=8.6, max=9
; - When 76 <= n <= 100: min=8, avg=8.9, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=11
; - When 301 <= n <= 500: min=8, avg=10.2, max=12
; - When 501 <= n <= 750: min=8, avg=10.3, max=12
; - When 751 <= n <= 1000: min=9, avg=10.3, max=12
; - When 1001 <= n <= 2000: min=8, avg=11.0, max=13
; - When 2001 <= n <= 3000: min=10, avg=12.1, max=14
; - When 3001 <= n <= 5000: min=10, avg=13.0, max=15
; - When 5001 <= n <= 7500: min=11, avg=14.5, max=15
; - When 7501 <= n <= 10000: min=12, avg=14.9, max=15
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
	; 2 bytes
	cmp $C5
.elseif((n > 3 && n < 7) || n = 15 || n = 17 || n = 18 || n = 28 || n = 30 || n = 32)
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
.elseif(n = 9 || n = 11 || n = 19 || n = 20 || n = 22 || n = 33 || n = 35 || n = 37)
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
.elseif(n = 14 || n = 21)
	; 4..6 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 16 || n = 29 || n = 34)
	; 4..8 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 23 || n = 26 || n = 39)
	; 4..6 bytes
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
.elseif(n = 36)
	; 8 bytes
	pha
	clc
	lda #$2A ;hides 'rol a'
	nop
	Jcc *-2
	pla
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
.elseif(n = 42)
	; 8 bytes
	pha
	lda #$2A ;hides 'rol a'
	delay_n 7
	Jpl *-3
	pla
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
.elseif(n = 45 || n = 47 || (n > 53 && n < 59) || n = 61 || n = 63 || n = 65 || (n > 66 && n < 70) || (n > 70 && n < 78))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 46)
	; 6 bytes
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
.elseif(n = 48)
	; 7 bytes
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
.elseif(n = 49)
	; 8 bytes
	delay_n (n-3)
	cmp $C5
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
.elseif(n = 70)
	; 8 bytes
	pha
	lda #$4A ;hides 'lsr a'
	nop $04
	Jne *-3
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
.elseif(n = 298 || n = 300 || n = 304 || n = 309 || n = 311)
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
.elseif(n = 302 || n = 307)
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 305 || n = 308)
	; 10..11 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 314 || n = 316 || n = 319 || n = 321 || n = 324 || n = 326 || n = 328 || n = 329 || n = 331 || n = 333 || n = 338 || n = 339 || n = 343 || n = 344 || n = 346 || n = 349 || n = 351 || n = 354 || n = 356 || n = 359 || n = 361 || n = 364 || n = 366 || n = 373 || n = 374 || n = 378 || n = 379 || n = 381 || n = 384 || n = 386 || n = 389)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 315 || n = 330 || n = 360 || n = 375)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
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
	; 10 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	iny
	iny
	Jpl *-3
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
.elseif(n = 335 || n = 340 || n = 345 || n = 350 || n = 365 || n = 370 || n = 380 || n = 385)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 336 || n = 371)
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
.elseif(n = 358)
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
.if(n = 391 || n = 394 || n = 396 || n = 399 || n = 401 || n = 413 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 438 || n = 444 || n = 448 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464)
	; 10..12 bytes
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
.elseif(n = 395 || n = 410 || n = 420 || n = 435 || n = 465)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
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
.elseif(n = 398)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $AAA0'
	ldy #170
	dey
	Jmi *-4
	tay
	pla
.elseif(n = 400 || n = 405 || n = 430 || n = 440 || n = 445 || n = 450 || n = 455)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
.elseif(n = 403)
	; 10 bytes
	brk
	delay_n (n-13)
.elseif(n = 404 || n = 418 || n = 425 || n = 460)
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
.elseif(n = 433)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $2EA0'
	ldy #46
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 441)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 443)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D0A0'
	ldy #208
	iny
	Jne *-4
	tay
	pla
.elseif(n = 458)
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
	pha
	lda $A5
	tya
	ldy #(n-22)/7
	nop
	dey
	Jpl *-2
	tay
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
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 471 || n = 473 || n = 479 || n = 484 || n = 486 || n = 489 || n = 493 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 518 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 538 || n = 539 || n = 546)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 470 || n = 480 || n = 490 || n = 505 || n = 515 || n = 520 || n = 525 || n = 535 || n = 540 || n = 545)
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
.elseif(n = 485 || n = 510)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
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
.elseif(n = 503)
	; 11 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 513)
	; 11 bytes
	brk
	delay_n (n-13)
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
	tya
	cmp <$CD ;hides 'cmp $3AA0'
	ldy #58
	dey
	Jne *-4
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
	cmp $C5
	dey
	Jpl *-3
	tay
	pla
.elseif(n = 549 || n = 553 || n = 554 || n = 556 || n = 559 || n = 564 || n = 566 || n = 569 || n = 573 || n = 574 || n = 581 || n = 583 || n = 584 || n = 589 || n = 591 || n = 594 || n = 596 || n = 599 || n = 601 || n = 604 || n = 608 || n = 609 || n = 611 || n = 616 || n = 619 || n = 623 || n = 624)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 550 || n = 560 || n = 570 || n = 575 || n = 610 || n = 615 || n = 620)
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
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	dey
	Jmi *-3
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
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	iny
	Jne *-4
	tay
	pla
.elseif(n = 580 || n = 585 || n = 590 || n = 595 || n = 605 || n = 625)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
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
.elseif(n = 598)
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
.elseif(n = 603)
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
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 686 || n = 688 || n = 689 || n = 693 || n = 694 || n = 699 || n = 701)
	; 10..12 bytes
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
.elseif(n = 630 || n = 650 || n = 655 || n = 660)
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-4
	dey
	Jpl *-1
	tay
	pla
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
.elseif(n = 643)
	; 11 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 645 || n = 665 || n = 675 || n = 680 || n = 690 || n = 700)
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
.elseif(n = 653)
	; 11 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 673)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-22)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 683)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
.if(n = 704 || n = 706 || n = 709 || n = 711 || n = 714 || n = 716 || n = 718 || n = 728 || n = 729 || n = 731 || n = 734 || n = 736 || n = 739 || n = 744 || n = 746 || n = 749 || n = 751 || n = 753 || n = 763 || n = 764 || n = 769 || n = 771 || n = 774 || n = 776 || n = 779 || n = 781)
	; 10..12 bytes
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
	; 10 bytes
	brk
	delay_n (n-13)
.elseif((n .mod 5 = 0) && ((n > 709 && n < 736) || (n > 749 && n < 771) || n = 780))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 713)
	; 11 bytes
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jmi *-4
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
.elseif(n = 745)
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
.elseif(n = 758)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	dey
	Jmi *-4
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
.elseif(n = 773)
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
.elseif(n = 778)
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
.elseif(n = 783 || n = 784 || n = 794 || n = 798 || n = 799 || n = 801 || n = 804 || n = 806 || n = 809 || n = 814 || n = 819 || n = 821 || n = 823 || n = 829 || n = 833 || n = 834 || n = 836 || n = 839 || n = 841 || n = 844 || n = 846 || n = 849 || n = 851 || n = 853 || n = 854 || n = 856)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 806) || n = 815 || (n > 824 && n < 841) || n = 850 || n = 855))
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
	cmp $C5
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
.elseif(n = 791 || n = 826)
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
.elseif(n = 803 || n = 810 || n = 824 || n = 838)
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
.elseif(n = 848)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DCA0'
	ldy #220
	dey
	Jmi *-4
	tay
	pla
.endif
.else
; 860 to 937
.if((n .mod 5 = 0) && ((n > 859 && n < 876) || (n > 884 && n < 906) || (n > 919 && n < 936)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif(n = 861 || n = 896 || n = 931)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif((n .mod 5 = 2) && ((n > 861 && n < 938)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 863 || n = 868 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 898 || n = 899 || n = 904 || n = 906 || n = 909 || n = 911 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 934)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 864)
	; 9 bytes
	pha
	tya
	ldy #(n-10)/7 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
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
.elseif(n = 878)
	; 11 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 883)
	; 11 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $61A2'
	ldx #97
	dex
	Jne *-4
	tax
	pla
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
.elseif(n = 893)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $62A0'
	ldy #98
	dey
	Jne *-4
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
.elseif(n = 903)
	; 11 bytes
	pha
	tya
	ldy #(n-21)/9
	delay_n 4
	dey
	Jpl *-3
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
.elseif(n = 918)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #(n-22)/7
	nop
	dey
	Jpl *-2
	tay
	pla
.elseif(n = 923)
	; 11 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	nop
	iny
	iny
	Jne *-4
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
.elseif(n = 933)
	; 11 bytes
	pha
	tya
	ldy #(n-9)/11 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	dey
	Jne *-4
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if(n = 938 || n = 939 || n = 941 || n = 944 || n = 949 || n = 951 || n = 954 || n = 956 || n = 959 || n = 961 || n = 963 || n = 968 || n = 974 || n = 979 || n = 981 || n = 984 || n = 986 || n = 989 || n = 994 || n = 998 || n = 1003 || n = 1004 || n = 1009 || n = 1011 || n = 1014)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 940 || n = 945 || (n > 954 && n < 981) || (n > 989 && n < 1016)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 5 = 2) && ((n > 941 && n < 1013)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif((n .mod 7 = 5) && (n = 943 || n = 950 || (n > 963 && n < 986) || (n > 998 && n < 1014)))
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
.elseif(n = 958)
	; 12 bytes
	pha
	tya
	ldy #(n-23)/11
	delay_n 6
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 966 || n = 1001)
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
.elseif(n = 1008)
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
; 1016 to 1093
.if(n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1031 || n = 1033 || n = 1039 || n = 1043 || n = 1044 || n = 1046 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1074 || n = 1078 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
	; 10..12 bytes
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
.elseif((n .mod 7 = 5) && (n = 1020 || (n > 1033 && n < 1056) || (n > 1068 && n < 1091)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1023 || n = 1058 || n = 1093)
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
.elseif((n .mod 5 = 0) && ((n > 1029 && n < 1041) || n = 1050 || (n > 1059 && n < 1086)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
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
.elseif(n = 1053)
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
.elseif(n = 1071)
	; 11 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1073)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $75A0'
	ldy #117
	dey
	Jpl *-4
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
.elseif(n = 1088)
	; 12 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1098 || n = 1099 || n = 1101 || n = 1103 || n = 1109 || n = 1113 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1138 || n = 1143 || n = 1144 || n = 1148 || n = 1149 || n = 1151 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1168 || n = 1169 || n = 1171 || n = 1178 || n = 1179 || n = 1183 || n = 1184 || n = 1186 || n = 1189 || n = 1191 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1203 || n = 1204 || n = 1206 || n = 1213 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1231 || n = 1234 || n = 1236 || n = 1238 || n = 1239 || n = 1241 || n = 1249)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 1094 && n < 1121) || n = 1130 || (n > 1139 && n < 1156) || (n > 1164 && n < 1191) || (n > 1199 && n < 1226) || (n > 1234 && n < 1251)))
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
.elseif(n = 1123 || n = 1158 || n = 1193)
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
.elseif(n = 1163)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $7FA0'
	ldy #127
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1198)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $7CA0'
	ldy #124
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1208)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $85A0'
	ldy #133
	dey
	Jne *-4
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
.elseif(n = 1228)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1233)
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
.elseif(n = 1253 || n = 1254 || n = 1256 || n = 1259 || n = 1264 || n = 1266 || n = 1269 || n = 1271 || n = 1273 || n = 1274 || n = 1276 || n = 1278 || n = 1283 || n = 1284 || n = 1288 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1301 || n = 1302 || n = 1304 || n = 1309 || n = 1311 || n = 1316 || n = 1318 || n = 1322 || n = 1323 || n = 1327)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && (n = 1255 || n = 1260 || (n > 1274 && n < 1291)))
	; 11 bytes
	pha
	lda $A5
	tya
	ldy #n/5-3
	dey
	Jne *-1
	tay
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
	cmp $C5
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
	cmp <$CD ;hides 'cmp $71A0'
	ldy #113
	iny
	Jne *-4
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
.elseif(n = 1313)
	; 12 bytes
	brk
	delay_n (n-13)
.elseif(n = 1315)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $6FA0'
	ldy #111
	iny
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
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
	pla
.endif
.else
.if((n >= 1329) && (n < 1368))
; 1329 to 1367
.if(n = 1329)
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
.elseif(n = 1330 || n = 1336 || n = 1337 || n = 1339 || n = 1351 || n = 1355 || n = 1357 || n = 1358 || n = 1365 || n = 1367)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1338 || n = 1345 || n = 1359 || n = 1366)
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
	cmp #<$CD ;hides 'cmp $69A0'
	ldy #105
	iny
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
.elseif(n = 1372 || n = 1374 || n = 1379 || n = 1381 || n = 1386 || n = 1393 || n = 1395 || n = 1400 || n = 1402)
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
	ldy #(n-23)/11
	delay_n 6
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
	brk
	delay_n (n-13)
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
.elseif(n = 1394 || n = 1403)
	; 11..12 bytes
	php
	delay_n (n-7)
	plp
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
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
	pla
.elseif(n = 1399)
	; 11 bytes
	delay_n (n-3)
	nop $04
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
.if((n .mod 9 = 3) && ((n > 1406 && n < 1444)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1408 || n = 1422 || n = 1429)
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
.elseif(n = 1409 || n = 1411 || n = 1439)
	; 11..12 bytes
	brk
	delay_n (n-13)
.elseif(n = 1410)
	; 9 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
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
.elseif(n = 1414 || n = 1418 || n = 1421 || n = 1427 || n = 1435 || n = 1437 || n = 1442)
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
.elseif(n = 1430)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1455)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1457 || n = 1471)
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
.elseif(n = 1467)
	; 12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1474)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $85A0'
	ldy #133
	nop
	dey
	Jne *-5
	tay
	pla
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
.if(n = 1485 || n = 1499 || n = 1520)
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
	brk
	delay_n (n-13)
.elseif(n = 1539)
	; 11 bytes
	delay_n (n-3)
	nop $04
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
	cmp <$CD ;hides 'cmp $ABA0'
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
.if(n = 1563 || n = 1568 || n = 1575 || n = 1581 || n = 1582 || n = 1589 || n = 1591 || n = 1597)
	; 11..12 bytes
	delay_n (n-2)
	nop
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1576)
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $AEA0'
	ldy #174
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1583 || n = 1590)
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
	brk
	delay_n (n-13)
.endif
.else
; 1602 to 1640
.if(n = 1602 || n = 1627)
	; 11..12 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 1603 || n = 1610 || n = 1616 || n = 1617 || n = 1619 || n = 1624 || n = 1626 || n = 1631 || n = 1638)
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
	delay_n 6
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
	delay_n 6
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 1611 || n = 1618)
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
	cmp <$CD ;hides 'cmp $B2A0'
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
.elseif(n = 1633)
	; 11 bytes
	brk
	delay_n (n-13)
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
	ldy #250 ;hides 'nop'
	delay_n 4
	dey
	dey
	Jne *-5
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
	cmp <$CD ;hides 'cmp $B5A0'
	ldy #181
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 1645 || n = 1652 || n = 1666 || n = 1673 || n = 1679)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1674)
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
.elseif(n = 1647 || n = 1661 || n = 1663)
	; 11..12 bytes
	brk
	delay_n (n-13)
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
	cmp <$CD ;hides 'cmp $B6A0'
	ldy #182
	dey
	Jne *-4
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
	cmp <$CD ;hides 'cmp $B7A0'
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1662 || n = 1669)
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
	cmp #<$CD ;hides 'cmp $B9A0'
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
.elseif(n = 1681 || n = 1688 || n = 1709)
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
.elseif(n = 1691 || n = 1698)
	; 12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1705)
	; 12 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 1707)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	brk
	delay_n (n-13)
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
.elseif(n = 1723 || n = 1737 || n = 1744)
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
.elseif(n = 1735)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1759 || n = 1768 || n = 1771 || n = 1777 || n = 1778)
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
	delay_n 6
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
	; 11 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 1773 || n = 1789)
	; 11..12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 1779)
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
.elseif(n = 1781 || n = 1795)
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
.elseif(n = 1787)
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
	cmp <$CD ;hides 'cmp $39A0'
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
	; 11 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C9A0'
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
	cmp $C5
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1825)
	; 12 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $A5A0'
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
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	iny
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
.elseif(n = 1833)
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
.elseif(n = 1841 || n = 1854 || n = 1858 || n = 1859 || n = 1862 || n = 1867 || n = 1870)
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
	tya
	ldy #116 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	iny
	Jne *-5
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
	delay_n 6
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
	ldy #205 ;hides 'cmp abs'
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1861)
	; 12 bytes
	pha
	tya
	ldy #(n-5)/16 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
.elseif(n = 1863)
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
.elseif(n = 1877 || n = 1878 || n = 1885 || n = 1894 || n = 1895 || n = 1898)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1879 || n = 1907)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1887)
	; 13 bytes
	pha
	tya
	ldy #(n-27)/15
	rol $26
	ror $26
	dey
	Jpl *-5
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
.elseif(n = 1889 || n = 1897 || n = 1905)
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1906)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	delay_n 6
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
	ldy #(n-27)/15
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
.elseif(n = 1921 || n = 1939)
	; 12 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 1922 || n = 1931 || n = 1949 || n = 1950)
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
.elseif(n = 1941)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 1964)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	cmp $C5
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
	delay_n 6
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
	cmp <$CD ;hides 'cmp $DAA0'
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
.elseif(n = 1975 || n = 1976 || n = 1984 || n = 1985)
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
	cmp $C5
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1989)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.endif
.else
; 1993 to 2031
.if(n = 1993 || n = 1994 || n = 1998 || n = 2006 || n = 2011 || n = 2021 || n = 2029 || n = 2030)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1995)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 1997 || n = 2015)
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
	delay_n 6
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
.elseif(n = 2007)
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
.elseif(n = 2031)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
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
.elseif(n = 2038 || n = 2039 || n = 2048 || n = 2057 || n = 2066)
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2050)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	brk
	iny
	Jpl *-4
	tay
	pla
.elseif(n = 2051 || n = 2061)
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
	delay_n 10
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
.if((n >= 2071) && (n < 2090))
; 2071 to 2089
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
.elseif(n = 2074 || n = 2075 || n = 2077 || n = 2078 || n = 2083)
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
	cmp <$CD ;hides 'cmp $BCA0'
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
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	iny
	Jne *-4
	tay
	pla
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
	ldy #(n-29)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 2087)
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
.endif
.else
; 2090 to 2109
.if(n = 2090)
	; 11 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	iny
	Jne *-4
	tay
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
.elseif(n = 2093)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2094)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2095 || n = 2096 || n = 2105)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
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
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	iny
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.endif
.else
.if((n >= 2110) && (n < 2149))
; 2110 to 2148
.if(n = 2110 || n = 2111 || n = 2114 || n = 2115 || n = 2120 || n = 2128 || n = 2129 || n = 2137 || n = 2138 || n = 2147)
	; 12..13 bytes
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
	delay_n 6
	dey
	Jne *-4
	tay
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2130)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
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
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	dey
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
.elseif(n = 2151)
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
	delay_n 6
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
.elseif(n = 2187)
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
.elseif(n = 2193)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
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
	delay_n 6
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
.elseif(n = 2203 || n = 2204 || n = 2213)
	; 12..13 bytes
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
.if(n = 2227 || n = 2228 || n = 2230 || n = 2236 || n = 2237 || n = 2246 || n = 2248 || n = 2255 || n = 2257 || n = 2258 || n = 2263 || n = 2264)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2229)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
	tay
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
	delay_n 6
	Jne *-4
	tax
	pla
.elseif(n = 2245)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
.elseif(n = 2274 || n = 2283 || n = 2301)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2275 || n = 2285 || n = 2286 || n = 2295 || n = 2303)
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
	; 13 bytes
	pha
	lda $A5
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
	delay_n 8
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2333)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2345 || n = 2369)
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
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2349 || n = 2357 || n = 2359 || n = 2360 || n = 2371)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	ldy #213 ;hides 'cmp zp,x'
	delay_n 4
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2354 || n = 2356 || n = 2361 || n = 2379)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2367)
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
.elseif(n = 2368 || n = 2380)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2381)
	; 12 bytes
	pha
	tya
	ldy #137 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2403 || n = 2415)
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
	; 13 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	delay_n 4
	nop $04
	iny
	Jne *-6
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
.elseif(n = 2423 || n = 2426 || n = 2428 || n = 2433 || n = 2438 || n = 2440 || n = 2444 || n = 2445)
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
.elseif(n = 2427)
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
	delay_n 6
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
.elseif(n = 2439 || n = 2451)
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
.elseif(n = 2450)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
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
	delay_n 8
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
	; 12 bytes
	brk
	delay_n (n-13)
.endif
.else
.if((n >= 2461) && (n < 2481))
; 2461 to 2480
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
.elseif(n = 2462)
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
.elseif(n = 2465)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2466 || n = 2474 || n = 2480)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2467)
	; 13 bytes
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
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2470)
	; 11 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	brk
	dey
	Jpl *-4
	tay
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
.elseif(n = 2472)
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
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 2479)
	; 12 bytes
	pha
	lda $A5
	tya
	ldy #136 ;hides 'dey'
	brk
	Jne *-3
	tay
	pla
.endif
.else
; 2481 to 2500
.if(n = 2481)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BEA0'
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
	cmp $C5
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2483 || n = 2491)
	; 12..13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2484 || n = 2496)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2486 || n = 2488)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2487 || n = 2498)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2489)
	; 12 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 2494)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 2499)
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
.elseif(n = 2503 || n = 2505 || n = 2515 || n = 2527 || n = 2539)
	; 13..14 bytes
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
.elseif(n = 2506 || n = 2516 || n = 2522 || n = 2524 || n = 2533 || n = 2534 || n = 2536)
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
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2521)
	; 12 bytes
	pha
	tya
	ldy #130 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2523 || n = 2535)
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
	delay_n 6
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
	php
	delay_n (n-7)
	plp
.endif
.else
; 2559 to 2578
.if(n = 2559 || n = 2571)
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
	delay_n 6
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
.if((n >= 2579) && (n < 2598))
; 2579 to 2597
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
	; 12 bytes
	pha
	tya
	ldy #128 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2582 || n = 2590 || n = 2594)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2583)
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
.elseif(n = 2587 || n = 2591)
	; 13 bytes
	php
	delay_n (n-7)
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2597)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.endif
.else
; 2598 to 2617
.if(n = 2598)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2599 || n = 2611)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2600)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #235
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2601)
	; 12 bytes
	pha
	tya
	ldy #(n-1)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 2603)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	cmp $C5
	dey
	Jne *-6
	tay
	pla
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
	ldy #236 ;hides 'cpx abs'
	delay_n 5
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2608)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
	; 12 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-5
	tay
	pla
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
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2620 || n = 2624 || n = 2642 || n = 2643 || n = 2646 || n = 2654 || n = 2656)
	; 12..13 bytes
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
.elseif(n = 2623 || n = 2635 || n = 2648)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2625 || n = 2638)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	iny
	Jne *-5
	tay
	pla
.elseif(n = 2631 || n = 2655)
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
.elseif(n = 2649)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 8
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
.if(n = 2657 || n = 2669)
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2659 || n = 2670 || n = 2671)
	; 13..14 bytes
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
	delay_n 6
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
.elseif(n = 2665 || n = 2666 || n = 2668 || n = 2678 || n = 2680 || n = 2687)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2667 || n = 2682)
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
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2679 || n = 2691)
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
.elseif(n = 2690)
	; 11 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	brk
	iny
	Jne *-4
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2697)
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
.elseif(n = 2702 || n = 2704 || n = 2708 || n = 2709 || n = 2720 || n = 2725 || n = 2726)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2703)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
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
.elseif(n = 2713)
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
.elseif(n = 2733)
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
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2741)
	; 12 bytes
	pha
	tya
	ldy #(n-1)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
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
	cmp <$CD ;hides 'cmp $D3A0'
	ldy #211
	delay_n 4
	dey
	Jne *-6
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
.elseif(n = 2763)
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
.elseif(n = 2764)
	; 12 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2769)
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 8
	dey
	Jne *-6
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
.if((n > 2773 && n < 2777) || n = 2785 || n = 2786 || n = 2788 || n = 2797)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	delay_n 6
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
	delay_n 8
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
.elseif(n = 2790 || n = 2791 || n = 2803 || n = 2809)
	; 13..14 bytes
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
.elseif(n = 2799 || n = 2811)
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
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2824 || n = 2829 || n = 2834 || n = 2836 || n = 2846 || n = 2848)
	; 12..13 bytes
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
	delay_n 6
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
.elseif(n = 2823 || n = 2835)
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
	delay_n 6
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
	delay_n 8
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
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2847)
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
.elseif(n = 2850)
	; 13 bytes
	pha
	tya
	ldy #128
	delay_n 4
	brk
	dey
	Jpl *-5
	tay
	pla
.elseif(n = 2851)
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
.if((n >= 2852) && (n < 2871))
; 2852 to 2870
.if(n = 2852)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 9
	Jne *-4
	tax
	pla
.elseif(n = 2853 || n = 2858 || n = 2860 || n = 2870)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2854)
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
.elseif(n = 2856 || n = 2868)
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
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2859 || n = 2863)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2861)
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
.elseif(n = 2862)
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
.elseif(n = 2864)
	; 13 bytes
	pha
	tya
	ldy #(n-43)/31
	delay_n 26
	dey
	Jpl *-5
	tay
	pla
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
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2867)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	cmp $C5
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2869)
	; 13 bytes
	brk
	delay_n (n-13)
.endif
.else
; 2871 to 2890
.if(n = 2871 || n = 2883)
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
.elseif(n = 2875)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 2876 || n = 2886)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2877)
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
.elseif(n = 2880)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
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
.elseif(n = 2887)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	nop $04
	brk
	Jne *-5
	tay
	pla
.elseif(n = 2888)
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
.elseif(n = 2893 || n = 2905)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 2894 || n = 2906 || n = 2908 || n = 2918 || n = 2920 || n = 2925)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895)
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
	delay_n 8
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
.elseif(n = 2907 || n = 2922)
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
.elseif(n = 2913)
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	cmp $C5
	dey
	Jne *-6
	tay
	pla
.elseif(n = 2929)
	; 13 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 2930 to 2968
.if(n = 2930 || n = 2934 || n = 2942 || n = 2944 || n = 2948 || n = 2954 || n = 2956)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2931 || n = 2943 || n = 2955)
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
.elseif(n = 2949)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 2950)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
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
.elseif(n = 2958)
	; 12 bytes
	pha
	tya
	ldy #122 ;hides 'nop'
	nop
	brk
	iny
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
.elseif(n = 2963)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2967)
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
	pha
	tya
	ldy #(n-9)/14 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop $04
	dey
	Jne *-6
	tay
	pla
.elseif((n .mod 6 = 2) && ((n > 2977 && n < 3003)))
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2979 || n = 2991 || n = 3003)
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
.elseif(n = 2985)
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
	delay_n 8
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
.elseif(n = 3009 || n = 3014 || n = 3016 || n = 3020 || n = 3026 || n = 3028 || n = 3030 || n = 3038 || n = 3040 || n = 3044 || n = 3045)
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
.elseif(n = 3015)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3027 || n = 3042)
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
.if((n >= 3047) && (n < 3066))
; 3047 to 3065
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
.elseif(n = 3048 || n = 3060)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3049)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3050 || n = 3064)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3051 || n = 3063)
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
	delay_n 10
	Jne *-5
	tax
	pla
.elseif(n = 3056)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3061)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.endif
.else
; 3066 to 3085
.if(n = 3066 || n = 3074)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3067 || n = 3069)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3068)
	; 12 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	iny
	Jne *-5
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
.elseif(n = 3072)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3073)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3075)
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
.elseif(n = 3078)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3079)
	; 12 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3080)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
.elseif(n = 3085)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
	pla
.endif
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3088 || n = 3094 || n = 3096 || n = 3107 || n = 3110 || n = 3122 || n = 3125)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3087 || n = 3102 || n = 3117)
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
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3095 || n = 3113)
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
.elseif(n = 3097)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	delay_n 7
	dey
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
.elseif(n = 3103 || n = 3121)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3104)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3105)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3109)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 3114)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C2A0'
	ldy #194
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3115)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $3EA0'
	ldy #62
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3118)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	delay_n 4
	dey
	Jne *-6
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
.elseif(n = 3127 || n = 3133 || n = 3155)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3128 || n = 3136 || n = 3138 || n = 3142 || n = 3146 || n = 3150 || n = 3154 || n = 3159 || n = 3160)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3129)
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
.elseif(n = 3130)
	; 13 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	delay_n 4
	inx
	Jne *-6
	tax
	pla
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
	cmp #<$CD ;hides 'cmp $0FA0'
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
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3147)
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
.elseif(n = 3151)
	; 13 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
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
.endif
.else
.if((n >= 3165) && (n < 3184))
; 3165 to 3183
.if(n = 3165)
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 4
	dey
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
.elseif(n = 3172)
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
.elseif(n = 3174)
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
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 6
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
	cmp <$CD ;hides 'cmp $C6A0'
	ldy #198
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3180)
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
.elseif(n = 3183)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
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
	delay_n 8
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
.elseif(n = 3186 || n = 3193 || n = 3199 || n = 3200)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3187)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3188)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
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
	ldy #212 ;hides 'nop zp,x'
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3192)
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
.elseif(n = 3198)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3201)
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
.endif
.else
.if((n >= 3204) && (n < 3243))
.if((n >= 3204) && (n < 3223))
; 3204 to 3222
.if(n = 3204 || n = 3218)
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
	delay_n 8
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3206 || n = 3214 || n = 3220)
	; 13 bytes
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
.elseif(n = 3208)
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
.elseif(n = 3213)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3215)
	; 14 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	delay_n 6
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 3216)
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3219)
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
.endif
.else
; 3223 to 3242
.if(n = 3223)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	cmp $C5
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3232)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3233)
	; 13 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 3234)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 3236 || n = 3238 || n = 3240)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3237)
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
.elseif(n = 3241)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.endif
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
.elseif(n = 3248 || n = 3250 || n = 3254 || n = 3256 || n = 3271 || n = 3278)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3249 || n = 3262 || n = 3275)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
.elseif(n = 3253 || n = 3265 || n = 3277 || n = 3281)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3255)
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
.elseif(n = 3261 || n = 3280)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
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
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3266)
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	iny ;first loop only
	brk
	iny
	Jne *-5
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
.elseif(n = 3273)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 4
	iny
	Jne *-6
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
.elseif(n = 3284 || n = 3285 || n = 3294 || n = 3295 || n = 3304 || n = 3308 || n = 3315)
	; 12..14 bytes
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
	; 12 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
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
.elseif(n = 3296)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	delay_n 6
	dey
	Jne *-7
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
	delay_n 8
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
.elseif(n = 3305)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/14
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
.elseif(n = 3309)
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
.if((n >= 3321) && (n < 3340))
; 3321 to 3339
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
.elseif(n = 3324)
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
.elseif(n = 3326 || n = 3329)
	; 12..13 bytes
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
.elseif(n = 3328)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	nop $04
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3330)
	; 11 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
	pla
.elseif(n = 3331)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3332)
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
.elseif(n = 3335)
	; 13 bytes
	php
	pha
	clc
	lda #90 ;hides 'nop'
	brk
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 3336)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
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
.endif
.else
; 3340 to 3359
.if(n = 3340)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3341 || n = 3346 || n = 3348 || n = 3353)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3342)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
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
.elseif(n = 3344 || n = 3358)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3349)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3350)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $9FA0'
	ldy #159
	jsr @nearby_rts
	dey
	Jne *-7
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
.elseif(n = 3352)
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
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	delay_n 7
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3357)
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
.elseif(n = 3359)
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.endif
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
.elseif(n = 3361 || n = 3395)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif((n > 3361 && n < 3365) || n = 3374 || n = 3380 || n = 3382 || n = 3389 || n = 3398)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3365 || n = 3383)
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
.elseif(n = 3366)
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
.elseif(n = 3367 || n = 3368 || n = 3379 || n = 3385)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3369)
	; 13 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3370)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D2A0'
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
	cmp <$CD ;hides 'cmp $D2A0'
	ldy #210
	delay_n 7
	dey
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
.elseif(n = 3373)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3377)
	; 13 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3381)
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
.elseif(n = 3384)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	cmp $C5
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
.endif
.else
; 3399 to 3437
.if(n = 3399 || n = 3406 || n = 3415 || n = 3416 || n = 3430 || n = 3431)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3405 || n = 3433)
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
.elseif(n = 3407 || n = 3421)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3408)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 3417)
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
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	cmp $C5
	dey
	Jne *-7
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
.elseif(n = 3439 || n = 3441 || n = 3449 || n = 3457)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3453)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 3459)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3460)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 8
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
.elseif(n = 3469)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	nop
	cmp $C5
	dey
	Jne *-7
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
.elseif(n = 3475)
	; 13 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 6
	dex
	dex
	Jne *-6
	tax
	pla
.elseif(n = 3476)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $19A0'
	ldy #25
	delay_n 6
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3479 || n = 3485 || n = 3488 || n = 3494)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	delay_n 8
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3491)
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
.elseif(n = 3493)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3495)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 8
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
.elseif(n = 3501)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3505 || n = 3511)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3506)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3513)
	; 13 bytes
	delay_n (n-3)
	nop $04
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
.elseif(n = 3517 || n = 3529)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3518 || n = 3524 || n = 3528 || n = 3533 || n = 3534 || n = 3539 || n = 3542 || n = 3544 || n = 3550)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-10)/15 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3521)
	; 14 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 6
	inx
	Jne *-7
	tax
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
.elseif(n = 3523 || n = 3553)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3525 || n = 3543)
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
.elseif(n = 3535)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3536)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 6
	iny
	Jne *-7
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
.elseif(n = 3538)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	nop
	cmp $C5
	iny
	Jne *-7
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
.elseif(n = 3551)
	; 14 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	delay_n 5
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 3555 to 3593
.if(n = 3555 || n = 3583)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3556 || n = 3560 || n = 3566 || n = 3569 || n = 3570 || n = 3578 || n = 3591)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3557 || n = 3585)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3559 || n = 3573)
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
.elseif(n = 3561)
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
.elseif(n = 3579)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
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
	; 13 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	iny
	Jne *-6
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
.elseif(n = 3597 || n = 3627)
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
.elseif(n = 3598 || n = 3600 || n = 3601 || n = 3605 || n = 3613 || n = 3614 || n = 3618 || n = 3629)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3599 || n = 3616)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3602)
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
.elseif(n = 3604 || n = 3621)
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
.elseif(n = 3606 || n = 3619)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3607 || n = 3625)
	; 13 bytes
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
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3615)
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
.elseif(n = 3617)
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
.elseif(n = 3620)
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
.elseif(n = 3626)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
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
.elseif(n = 3631)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.repeat 5
	 nop
	.endrepeat
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3632)
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
	delay_n (n-4)
	delay_n 4
.elseif(n = 3635 || n = 3646)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
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
.elseif(n = 3637)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3638)
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
	pha
	tya
	cmp #<$CD ;hides 'cmp $A5A0'
	ldy #165
	brk
	dey
	Jne *-6
	tay
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
.elseif(n = 3647)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3649)
	; 14 bytes
	pha
	tya
	ldy #116 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	brk
	iny
	Jne *-7
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
.elseif(n = 3654 || n = 3668 || n = 3669)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 6
	dey
	Jne *-7
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
	; 12 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	nop
	brk
	iny
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
.elseif(n = 3665)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	delay_n 9
	iny
	Jne *-7
	tay
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
.elseif(n = 3670)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 8
	dey
	Jne *-6
	tay
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
.elseif(n = 3673 || n = 3699 || n = 3709)
	; 13..14 bytes
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
.elseif(n = 3678 || n = 3686 || n = 3688 || n = 3693 || n = 3694 || n = 3696 || n = 3703 || n = 3704 || n = 3710)
	; 12..14 bytes
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
.elseif(n = 3680)
	; 14 bytes
	pha
	lda $A5
	brk
	txa
	ldx #202 ;hides 'dex'
	brk
	Jne *-3
	tax
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
	txa
	ldx #204 ;hides 'cpy abs'
	delay_n 5
	delay_n 7
	dex
	Jne *-7
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
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 6
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3687)
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
.if(n = 3711 || n = 3714 || n = 3719 || n = 3726 || n = 3734 || n = 3737 || n = 3740 || n = 3742 || n = 3744 || n = 3749)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3713 || n = 3727 || n = 3745)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3715 || n = 3733)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 3721)
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
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	delay_n 7
	iny
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
.elseif(n = 3731)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 6
	iny
	Jne *-7
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
.elseif(n = 3741)
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
.elseif(n = 3746)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 6
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3748)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B2A0'
	ldy #178
	jsr @nearby_rts
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
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
.if((n >= 3751) && (n < 3770))
; 3751 to 3769
.if(n = 3751)
	; 13 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3752 || n = 3769)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3753)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $EA ;hides 'nop'
	delay_n 9
	dey
	Jne *-7
	tay
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
.elseif(n = 3757 || n = 3758 || n = 3764 || n = 3765)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-10)/15 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3761)
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
.elseif(n = 3763)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3766)
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
.elseif(n = 3767)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3768)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	pla
	pha
	iny
	Jne *-5
	pla
	tay
	pla
.endif
.else
; 3770 to 3789
.if(n = 3770)
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
	ldy #58 ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-5
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
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 6
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3776 || n = 3778 || n = 3785 || n = 3789)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3782)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3783)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3784)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	nop $04
	iny
	Jne *-7
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
.endif
.else
; 3790 to 3828
.if(n = 3790 || n = 3794 || n = 3796 || n = 3801 || n = 3805 || n = 3806 || n = 3809 || n = 3824)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3791)
	; 13 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3795 || n = 3813)
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
.elseif(n = 3799 || n = 3817)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3800)
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
.elseif(n = 3811)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B5A0'
	ldy #181
	jsr @nearby_rts
	dey
	Jne *-7
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
.elseif(n = 3816)
	; 12 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 3823)
	; 13 bytes
	brk
	delay_n (n-13)
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
.if((n >= 3829) && (n < 3848))
; 3829 to 3847
.if(n = 3829 || n = 3841)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3830 || n = 3845)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3831)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3832)
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
; 3848 to 3867
.if(n = 3848 || n = 3856 || n = 3858 || n = 3866)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3849)
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
.elseif(n = 3852)
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3857)
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
.elseif(n = 3859 || n = 3865)
	; 13..14 bytes
	brk
	delay_n (n-13)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3870 || (n > 3872 && n < 3876) || n = 3884 || n = 3886 || n = 3890 || n = 3894 || n = 3901 || n = 3902 || n = 3904)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3876 || n = 3897)
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
.elseif(n = 3877 || n = 3895)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3878 || n = 3879 || n = 3889)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3880)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3881)
	; 12 bytes
	pha
	tya
	ldy #(n-1)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
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
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3885 || n = 3903)
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
.elseif(n = 3887)
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
.elseif(n = 3896)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B9A0'
	ldy #185
	jsr @nearby_rts
	dey
	Jne *-7
	tay
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3906)
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
.elseif(n = 3908 || n = 3942)
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
.elseif(n = 3910 || n = 3944)
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
.elseif(n = 3911 || n = 3920 || n = 3924 || n = 3933 || n = 3938 || n = 3940 || n = 3941)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	dey
	Jne *-5
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
.elseif(n = 3917)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $27A0'
	ldy #39
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 3925 || n = 3929 || n = 3943)
	; 13..14 bytes
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
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
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
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 7
	iny
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
	cmp <$CD ;hides 'cmp $DAA0'
	ldy #218
	delay_n 9
	dey
	Jne *-7
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
.endif
.else
.if((n >= 3946) && (n < 3965))
; 3946 to 3964
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
.elseif(n = 3953)
	; 14 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	pla
	pha
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 3954)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3955 || n = 3961)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3957)
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
.elseif(n = 3958)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3959)
	; 12 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3960)
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
.elseif(n = 3964)
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
; 3965 to 3984
.if(n = 3965 || n = 3966 || n = 3969 || (n > 3973 && n < 3977) || n = 3982)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3967)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 3968)
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
.elseif(n = 3970)
	; 11 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	brk
	iny
	Jne *-4
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
.elseif(n = 3972)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 3973)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 3980)
	; 12 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3981)
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
.elseif(n = 3984)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
.if((n >= 3985) && (n < 4024))
.if((n >= 3985) && (n < 4004))
; 3985 to 4003
.if(n = 3985 || n = 3986 || n = 4003)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 3987)
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
	cmp <$CD ;hides 'cmp $DDA0'
	ldy #221
	delay_n 9
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 3992)
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
.elseif(n = 3997 || n = 3999)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3998)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4000)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4001)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BEA0'
	ldy #190
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4002)
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
.else
; 4004 to 4023
.if(n = 4004 || n = 4009 || n = 4014 || n = 4018)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4005)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 8
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4006)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DEA0'
	ldy #222
	delay_n 9
	dey
	Jne *-7
	tay
	pla
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
	cmp <$CD ;hides 'cmp $06A0'
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
	nop
	brk
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
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
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
.elseif(n = 4022)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BFA0'
	ldy #191
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4023)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 4025)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4028 || n = 4030 || n = 4034 || n = 4035 || n = 4045 || n = 4046 || n = 4048 || n = 4050 || n = 4053 || n = 4054 || n = 4061)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4029 || n = 4047)
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
.elseif(n = 4032 || n = 4052)
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
.elseif(n = 4033 || n = 4051)
	; 13 bytes
	php
	delay_n (n-7)
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
.elseif(n = 4037)
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
.elseif(n = 4038)
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
.elseif(n = 4039 || n = 4057)
	; 13 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 4055)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4056)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
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
; 4063 to 4101
.if(n = 4063)
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
.elseif(n = 4064 || n = 4066 || n = 4071 || n = 4077 || n = 4082 || n = 4084 || n = 4094 || n = 4099 || n = 4100)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4065 || n = 4083 || n = 4101)
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
.elseif(n = 4073)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 4095)
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
.if(n = 4102 || n = 4110 || n = 4111 || n = 4113 || n = 4114 || n = 4118 || n = 4120 || n = 4122 || n = 4128 || n = 4130 || n = 4136 || n = 4138)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4103 || n = 4121 || n = 4139)
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
.elseif(n = 4105 || n = 4123)
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
.elseif(n = 4112 || n = 4132)
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
.elseif(n = 4115)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 4125)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $BBA0'
	ldy #187
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4127)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4129)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4131)
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
.elseif(n = 4133)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4135)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4137)
	; 12 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4140)
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
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4144 || n = 4159 || n = 4167 || n = 4176)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4145 || n = 4154 || n = 4156 || n = 4161 || n = 4162 || n = 4164 || n = 4171 || n = 4179)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4146)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $BCA0'
	ldy #188
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4147 || n = 4165)
	; 13 bytes
	brk
	delay_n (n-13)
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
	pha
	tya
	ldy #(n-12)/21
	cmp $C5
	brk
	dey
	Jne *-5
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
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4153 || n = 4178)
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
.elseif(n = 4155)
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
.elseif(n = 4163)
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
.elseif(n = 4173)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4174)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 4180 to 4218
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
.elseif(n = 4181 || n = 4185 || n = 4190 || n = 4196 || n = 4198 || n = 4208 || n = 4213 || n = 4215)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4182 || n = 4199 || n = 4216)
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
.elseif(n = 4183 || n = 4201 || n = 4207)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4184 || n = 4195 || n = 4218)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4191 || n = 4209)
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
.elseif(n = 4200)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4202)
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
.elseif(n = 4203)
	; 14 bytes
	pha
	tya
	ldy #126
	nop
	delay_n 26
	dey
	Jpl *-6
	tay
	pla
.elseif(n = 4204)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4205)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 4217)
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
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4237 || n = 4255)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4220 || n = 4243)
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
.elseif(n = 4221 || n = 4226 || n = 4234 || n = 4238 || n = 4244 || n = 4246 || n = 4247 || n = 4251 || n = 4254)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4224 || n = 4242)
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
.elseif(n = 4227)
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
.elseif(n = 4228 || n = 4245)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4229)
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
.elseif(n = 4230 || n = 4249)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4231 || n = 4239)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4232 || n = 4252)
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
.elseif(n = 4233)
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
.elseif(n = 4236)
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
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4248)
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
.elseif(n = 4250)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
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
.endif
.else
; 4258 to 4296
.if(n = 4258)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 11
	iny
	Jne *-6
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
.elseif(n = 4260 || n = 4278 || n = 4296)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/18
	brk
	dey
	Jne *-3
	tay
	pla
.elseif(n = 4261 || n = 4264 || n = 4266 || n = 4270 || n = 4271 || n = 4274 || (n > 4279 && n < 4283) || n = 4289 || n = 4293)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4262 || n = 4279)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4263)
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
.elseif(n = 4265 || n = 4283)
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
.elseif(n = 4267 || n = 4285 || n = 4286)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4268 || n = 4287)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4269)
	; 13 bytes
	pha
	tya
	ldy #(n-1)/22 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4272 || n = 4292)
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
.elseif(n = 4273)
	; 12 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	nop $04
	brk
	Jne *-5
	tax
	pla
.elseif(n = 4275)
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
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4284)
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
.elseif(n = 4288)
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
.elseif(n = 4291)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4294)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4295)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
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
.elseif(n = 4298 || n = 4300 || n = 4304 || n = 4305 || n = 4308 || n = 4311 || n = 4315 || n = 4316 || n = 4318 || n = 4329 || n = 4334)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4317)
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
.elseif(n = 4301 || n = 4319)
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
.elseif(n = 4302 || n = 4324)
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
.elseif(n = 4303 || n = 4320 || n = 4321)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4306 || n = 4325)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4307)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $B3A0'
	ldy #179
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4309 || n = 4327)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4310)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	nop $04
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4312)
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
.elseif(n = 4326)
	; 14 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4328)
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
.elseif(n = 4333)
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
.if(n = 4336 || n = 4341 || n = 4349 || n = 4351 || n = 4361 || n = 4365 || n = 4374)
	; 13..14 bytes
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
.elseif(n = 4339 || n = 4357 || n = 4375)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4342)
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
.elseif(n = 4343 || n = 4345 || n = 4360)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4344 || n = 4363)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4346)
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
.elseif(n = 4352 || n = 4372)
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
.elseif(n = 4354)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4356)
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
.elseif(n = 4358)
	; 13 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	nop
	brk
	iny
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
.elseif(n = 4366)
	; 12 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	brk
	iny
	Jne *-5
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
.elseif(n = 4370)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk
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
.if((n >= 4376) && (n < 4688))
.if((n >= 4376) && (n < 4532))
.if((n >= 4376) && (n < 4454))
.if((n >= 4376) && (n < 4415))
; 4376 to 4414
.if(n = 4376 || n = 4388 || n = 4390 || (n > 4393 && n < 4397) || n = 4403 || n = 4406 || n = 4408 || n = 4413)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377 || n = 4393 || n = 4411)
	; 13 bytes
	php
	delay_n (n-7)
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
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $30A0'
	ldy #48
	jsr @nearby_rts
	iny
	Jne *-7
	tay
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
.elseif(n = 4381 || n = 4399 || n = 4405)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4382 || n = 4401)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4383 || n = 4414)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
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
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	delay_n 9
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4387)
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
.elseif(n = 4389 || n = 4407)
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
.elseif(n = 4391 || n = 4409)
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
.elseif(n = 4392 || n = 4412)
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
.elseif(n = 4398)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4400)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2FA0'
	ldy #47
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4402)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
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
.endif
.else
; 4415 to 4453
.if(n = 4415)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4416 || n = 4424 || n = 4428 || n = 4431 || n = 4442 || n = 4444 || n = 4446 || n = 4449)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4419 || n = 4429 || n = 4447)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4421)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	delay_n 9
	iny
	Jne *-7
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
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4425 || n = 4443)
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
.elseif(n = 4430 || n = 4448)
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
.elseif(n = 4436)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 4438)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4441)
	; 13 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4450)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $B9A0'
	ldy #185
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4451)
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
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
; 4454 to 4492
.if(n = 4454 || n = 4460 || n = 4466 || n = 4467 || n = 4473 || n = 4474 || n = 4478 || n = 4480 || n = 4482 || n = 4491)
	; 12..14 bytes
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
	ldy #(n-10)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4457)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	delay_n 9
	dey
	Jne *-7
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
.elseif(n = 4459)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4461 || n = 4479)
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4465)
	; 13 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4468 || n = 4486)
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
.elseif(n = 4469 || n = 4471 || n = 4489)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4470)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	nop
	iny
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4475)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 4484)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2BA0'
	ldy #43
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4487)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4488)
	; 14 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4490)
	; 14 bytes
	delay_n (n-3)
	nop $04
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4496 || n = 4502 || n = 4503 || n = 4508 || n = 4509 || n = 4514 || n = 4516 || n = 4518 || n = 4523)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4498)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4499 || n = 4517)
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4501 || n = 4519)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4505)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4506 || n = 4527)
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
	cmp <$CD ;hides 'cmp $06A0'
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
.elseif(n = 4515)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4520)
	; 12 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	tay
	pla
.elseif(n = 4521)
	; 12 bytes
	pha
	tya
	ldy #(n-1)/20 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4524)
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
.elseif(n = 4525)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4526)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4528)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
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
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
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
	nop
	brk
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4536 || n = 4540 || n = 4545 || n = 4550 || n = 4554 || n = 4556 || n = 4563 || n = 4568 || n = 4570)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4537 || n = 4539 || n = 4555 || n = 4559)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4538 || n = 4569)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4543 || n = 4561)
	; 13 bytes
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
.elseif(n = 4546)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
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
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4549 || n = 4558)
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
.elseif(n = 4557)
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
.elseif(n = 4560)
	; 14 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 4567)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $27A0'
	ldy #39
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 4571 to 4609
.if(n = 4571 || n = 4589 || n = 4607)
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
.elseif(n = 4572 || n = 4592)
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
.elseif(n = 4573 || n = 4598 || n = 4599)
	; 12..14 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 4573 && n < 4577) || n = 4581 || n = 4586 || n = 4593 || n = 4595 || n = 4600 || n = 4604 || n = 4606)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4577)
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
.elseif(n = 4578 || n = 4603)
	; 13..14 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4579 || n = 4585 || n = 4597)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4580)
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
.elseif(n = 4582)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
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
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 4587 || n = 4605)
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
	; 13 bytes
	pha
	nop
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 4601)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4608)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D1A0'
	ldy #209
	brk
	dey
	Jne *-6
	tay
	pla
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
.if(n = 4610)
	; 11 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	brk
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4611 || n = 4613 || n = 4618 || n = 4622 || n = 4624 || n = 4626 || n = 4634 || n = 4635)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612 || n = 4632)
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
.elseif(n = 4615 || n = 4633)
	; 13 bytes
	brk
	delay_n (n-13)
.elseif(n = 4616 || n = 4617 || n = 4627 || n = 4639)
	; 12..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4619)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4628 || n = 4647)
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
.elseif(n = 4629 || n = 4648)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	brk
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4640)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
.elseif(n = 4641)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	nop $04
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4642)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $3FA0'
	ldy #63
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4643)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C1A0'
	ldy #193
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4644)
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
.elseif(n = 4645)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
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
	ldy #244 ;hides 'nop zp,x'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4660 || n = 4661 || n = 4671 || n = 4680 || n = 4682 || n = 4684)
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
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $DDA0'
	ldy #221
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4652 || n = 4672)
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
.elseif(n = 4653)
	; 13 bytes
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	brk
	iny
	Jne *-4
	tay
	pla
.elseif(n = 4654 || n = 4676)
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
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4657 || n = 4659 || n = 4670 || n = 4681)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4658)
	; 14 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jmi *-1
	ldy #0 ;hides 'brk imm'
	nop
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 4663)
	; 12 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4664 || n = 4665 || n = 4669 || n = 4678)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4666)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $C2A0'
	ldy #194
	nop
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4668)
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
.elseif(n = 4673)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 4677)
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
.elseif(n = 4679)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	clc
	Jcc *+2
	brk
	Jne *-6
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
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4689 || n = 4694 || n = 4700 || n = 4707 || n = 4714 || n = 4722 || n = 4726)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4691)
	; 14 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 4692 || n = 4712)
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
.elseif(n = 4693)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4695)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/20 ;hides 'nop'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4701)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4702)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4703 || n = 4709)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4704)
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
.elseif(n = 4705 || n = 4724)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4706 || n = 4708 || n = 4713 || n = 4723)
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
.elseif(n = 4711)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 6
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4715)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $E0A0'
	ldy #224
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4716)
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
.elseif(n = 4717)
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
.elseif(n = 4721)
	; 14 bytes
	delay_n (n-3)
	cmp $C5
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
.endif
.else
.if((n >= 4727) && (n < 4746))
; 4727 to 4745
.if(n = 4727)
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
.elseif(n = 4731)
	; 14 bytes
	pha
	tya
	ldy #143
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4732)
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
.elseif(n = 4733)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	Jne *-1
	ldx #0 ;hides 'brk imm'
	nop
	dex
	Jne *-3
	tax
	pla
.elseif(n = 4734 || n = 4744 || n = 4745)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4735)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4736)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 4738)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $C5C5 ;hides 'cmp zp'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4739)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4742)
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
.elseif(n = 4743)
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
.else
; 4746 to 4765
.if((n > 4745 && n < 4750) || n = 4751 || n = 4761)
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
.elseif(n = 4752)
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
.elseif(n = 4753)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4754)
	; 13 bytes
	delay_n (n-2)
	nop
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
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4757)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4758)
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
.elseif(n = 4759)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4760)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/19 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4762)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
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
.elseif(n = 4764)
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
.elseif(n = 4765)
	; 14 bytes
	php
	pha
	sec
	lda #250 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766 || n = 4774 || n = 4776 || n = 4783 || n = 4789 || n = 4794 || n = 4796 || n = 4802)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4767)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4768 || n = 4770 || n = 4771 || n = 4780 || n = 4790 || n = 4793 || n = 4801 || n = 4803 || n = 4804)
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
.elseif(n = 4769)
	; 14 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4772 || n = 4792)
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
.elseif(n = 4773)
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
.elseif(n = 4775)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4777)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4778)
	; 14 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4781 || n = 4800)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4782)
	; 14 bytes
	pha
	tya
	ldy #159
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.elseif(n = 4786)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4787)
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
.elseif(n = 4788)
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
.elseif(n = 4791)
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
.elseif(n = 4795)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4797)
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
.elseif(n = 4798)
	; 14 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4799)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 4805 to 4843
.if(n = 4805 || n = 4825)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4806)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
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
.elseif(n = 4809 || n = 4814 || n = 4816 || n = 4823 || n = 4831 || n = 4834)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4811)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $C8A0'
	ldy #200
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4812 || n = 4832)
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
.elseif(n = 4813 || n = 4839)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4815 || n = 4835)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
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
.elseif(n = 4818)
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
.elseif(n = 4819 || n = 4838)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4820)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4821 || n = 4842)
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
.elseif(n = 4822 || n = 4826 || n = 4833 || n = 4843)
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
.elseif(n = 4824)
	; 14 bytes
	pha
	tya
	ldy #90 ;hides 'nop'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4827)
	; 14 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	brk
	dey
	Jne *-5
	tay
	pla
	plp
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
.elseif(n = 4836)
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
.elseif(n = 4837)
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
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4853 || n = 4854 || n = 4856 || n = 4859 || n = 4864 || n = 4878)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845 || n = 4865)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4846 || n = 4847 || n = 4849 || (n > 4865 && n < 4870) || n = 4871 || n = 4880)
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
.elseif(n = 4855 || n = 4875)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
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
.elseif(n = 4858)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $CAA0'
	ldy #202
	nop
	brk
	dey
	Jne *-7
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
.endif
.else
.if((n >= 4883) && (n < 4902))
; 4883 to 4901
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
	ldy #244 ;hides 'nop zp,x'
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
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 11
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4891 || n = 4893 || n = 4900 || n = 4901)
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
.elseif(n = 4894 || n = 4898)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4895)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $22A0'
	ldy #34
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4896)
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
.elseif(n = 4897)
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
.elseif(n = 4899)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.else
; 4902 to 4921
.if(n = 4902)
	; 14 bytes
	pha
	tya
	ldy #163
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4903)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $17A0'
	ldy #23
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4904)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4905)
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
.elseif(n = 4906)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 4
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4907 || n = 4914 || n = 4920)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4908)
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
.elseif(n = 4909)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 14
	dey
	Jne *-7
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
.elseif(n = 4912)
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
.elseif(n = 4913 || n = 4921)
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
.elseif(n = 4915)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4916)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $21A0'
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
	cmp <$CD ;hides 'cmp $21A0'
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
	delay_n 4
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4919)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
.if((n >= 4922) && (n < 4961))
; 4922 to 4960
.if(n = 4922 || n = 4923 || n = 4927 || n = 4933 || n = 4944 || n = 4950 || n = 4951 || n = 4958)
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
.elseif(n = 4925)
	; 14 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4926 || n = 4947)
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
.elseif(n = 4928 || n = 4934 || n = 4936 || n = 4942 || n = 4943 || n = 4949 || n = 4954)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4929)
	; 14 bytes
	pha
	tya
	ldy #149
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4930)
	; 13 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4931)
	; 14 bytes
	pha
	tya
	ldy #205 ;hides 'cmp abs'
	delay_n 5
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4932 || n = 4952)
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
.elseif(n = 4935 || n = 4955)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 4945)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4946)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $15A0'
	ldy #21
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4948)
	; 14 bytes
	pha
	tya
	pha
	ldy #9 ;hides 'ora #imm'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 4953)
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
.elseif(n = 4956)
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
.elseif(n = 4957)
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
.elseif(n = 4959)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.endif
.else
; 4961 to 5000
.if(n = 4961)
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
.elseif(n = 4962 || n = 4984)
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
.elseif(n = 4963 || n = 4964 || n = 4970 || n = 4974 || n = 4976 || n = 4986 || n = 4991 || n = 4994 || n = 4996)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4965 || n = 4985)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 4966)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4967)
	; 14 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	cmp $C5
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4968 || n = 4989)
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
.elseif(n = 4969)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4971 || n = 4981 || n = 4990 || n = 5000)
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
.elseif(n = 4972 || n = 4992)
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
.elseif(n = 4973)
	; 13 bytes
	pha
	tya
	ldy #(n-1)/22 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dey
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
.elseif(n = 4978)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $CFA0'
	ldy #207
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4979 || n = 4999)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4980)
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
.elseif(n = 4987)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4988)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4993)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4995)
	; 14 bytes
	pha
	tya
	ldy #151
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4997)
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
.elseif(n = 4998)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	delay_n 9
	brk
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
.if(n = 5001 || n = 5011 || n = 5018 || n = 5020 || n = 5021 || n = 5038)
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
.elseif(n = 5002)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D0A0'
	ldy #208
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5003 || n = 5034)
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
.elseif(n = 5006 || n = 5028)
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
.elseif(n = 5007)
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
.elseif(n = 5008)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5009)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5010)
	; 11 bytes
	pha
	tya
	ldy #(n-10)/20 ;hides 'nop'
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5012 || n = 5032)
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
.elseif(n = 5013)
	; 13 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5014 || n = 5016 || n = 5033 || n = 5036)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5015)
	; 13 bytes
	php
	pha
	sec
	lda #(n-15)/20 ;hides 'nop'
	brk
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 5017 || n = 5019 || n = 5039)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5022)
	; 14 bytes
	pha
	tya
	ldy #167
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5023 || n = 5025)
	; 13..14 bytes
	brk
	delay_n (n-13)
.elseif(n = 5024)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop'
	nop $04
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5029)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5030)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5031)
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
.elseif(n = 5035)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
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
; 5040 to 5078
.if((n > 5039 && n < 5045) || n = 5046 || n = 5047 || n = 5053 || n = 5058 || n = 5060 || n = 5063 || n = 5066 || n = 5068 || n = 5069 || n = 5078)
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
.elseif(n = 5045)
	; 14 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 5050)
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
.elseif(n = 5051)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5052 || n = 5072)
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
.elseif(n = 5054 || n = 5056 || n = 5064 || n = 5067 || n = 5074)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5055 || n = 5075)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
	tay
	pla
.elseif(n = 5057 || n = 5077)
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
.elseif(n = 5059)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5061)
	; 14 bytes
	pha
	tya
	ldy #153
	nop
	delay_n 26
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
.elseif(n = 5070)
	; 12 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	brk
	iny
	Jne *-5
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
.elseif(n = 5073)
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
.elseif(n = 5076)
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
.endif
.else
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5099 || n = 5108)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5080 || n = 5081 || n = 5084 || n = 5086 || n = 5090 || n = 5091 || n = 5102)
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
	; 14 bytes
	pha
	tya
	ldy #169
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5083)
	; 14 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 5088)
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
.elseif(n = 5089 || n = 5103 || n = 5105 || n = 5107 || n = 5110)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5095)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #(n-15)/20
	nop
	brk
	dey
	Jne *-4
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
	; 13 bytes
	pha
	tya
	ldy #(n-9)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
	pla
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
	; 14 bytes
	pha
	nop
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	nop
	iny
	Jne *-4
	tay
	pla
.elseif(n = 5109)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	brk
	iny
	Jne *-7
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
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
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
.elseif(n = 5117)
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
.endif
.else
; 5118 to 5156
.if(n = 5118 || n = 5122 || n = 5123 || n = 5129 || n = 5130 || n = 5140)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119 || n = 5127 || n = 5128 || n = 5139)
	; 13..14 bytes
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
.elseif(n = 5131 || n = 5143 || n = 5144 || (n > 5148 && n < 5155))
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
.elseif(n = 5134)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5135)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
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
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	nop
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5155)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5156)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
.if((n >= 5157) && (n < 5196))
; 5157 to 5195
.if(n = 5157 || n = 5178)
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
.elseif(n = 5158)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5159)
	; 13 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	brk
	inx
	Jne *-6
	tax
	pla
.elseif(n = 5160 || n = 5182)
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
.elseif(n = 5161)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5162 || n = 5187)
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
	lda #(n-15)/22 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5164 || n = 5183 || n = 5184 || n = 5193)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5165)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif((n > 5165 && n < 5169) || (n > 5172 && n < 5176) || n = 5179 || n = 5185 || n = 5186 || n = 5188 || n = 5190 || n = 5192)
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
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5170)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D7A0'
	ldy #215
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5171)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 5172)
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
.elseif(n = 5176)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5177)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5180)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	iny
	Jne *-5
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
.elseif(n = 5191)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	iny
	Jne *-5
	tay
	pla
.elseif(n = 5194)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $D8A0'
	ldy #216
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5195)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $D8A0'
	ldy #216
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 5196 to 5234
.if(n = 5196)
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
.elseif(n = 5198)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F7A0'
	ldy #247
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 5200 || n = 5208 || n = 5211 || n = 5213 || n = 5217 || n = 5221 || n = 5229 || n = 5230 || n = 5234)
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
.elseif(n = 5201)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	brk
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5202)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	brk
	iny
	Jne *-5
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
.elseif(n = 5204 || n = 5226)
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
.elseif(n = 5205 || n = 5206 || n = 5214 || n = 5222 || n = 5227 || n = 5228)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5207)
	; 14 bytes
	php
	pha
	clc
	lda #20 ;hides 'nop zp,x'
	nop
	brk
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 5209)
	; 14 bytes
	pha
	tya
	pha
	ldy #21 ;hides 'ora zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 5210)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5215)
	; 14 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 5218)
	; 14 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5219)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5220)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5223)
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
.elseif(n = 5224)
	; 12 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	brk
	iny
	Jne *-5
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
.elseif(n = 5231)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5232)
	; 14 bytes
	pha
	tya
	ldy #174
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5233)
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
.endif
.endif
.else
.if((n >= 5235) && (n < 5274))
; 5235 to 5273
.if(n = 5235 || n = 5236 || n = 5238 || n = 5252 || n = 5254 || n = 5255 || n = 5257 || n = 5258 || n = 5263 || n = 5265 || n = 5273)
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
.elseif(n = 5237 || n = 5262)
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
.elseif(n = 5239)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $07A0'
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
	cmp <$CD ;hides 'cmp $07A0'
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
	cmp $C5
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5242)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/24 ;hides 'nop'
	delay_n 4
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5243 || n = 5249 || n = 5253 || n = 5264 || n = 5271 || n = 5272)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5244)
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
.elseif(n = 5245)
	; 14 bytes
	pha
	tya
	ldy #194 ;hides 'nop #imm'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
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
.elseif(n = 5248 || n = 5270)
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
.elseif(n = 5250)
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
.elseif(n = 5256)
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
.elseif(n = 5259)
	; 14 bytes
	pha
	tya
	ldy #159
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5261)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5266)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $25A0'
	ldy #37
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5267)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	nop
	brk
	dey
	Jne *-7
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
.endif
.else
; 5274 to 5312
.if(n = 5274 || n = 5276 || n = 5280 || n = 5286 || (n > 5294 && n < 5299) || n = 5301 || n = 5305 || n = 5307 || n = 5308 || n = 5311)
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
.elseif(n = 5275)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5277)
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
.elseif(n = 5278)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	.byte $AD,0,0 ;lda abs; hides 'brk'
	iny
	Jne *-5
	pla
	tay
	pla
.elseif(n = 5279)
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
.elseif(n = 5281)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
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
.elseif(n = 5283 || n = 5304)
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
.elseif(n = 5284 || n = 5285 || n = 5289 || n = 5293 || n = 5294 || n = 5306)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5287 || n = 5312)
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
.elseif(n = 5288)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 21
	Jne *-7
	tax
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
.elseif(n = 5292)
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
.elseif(n = 5299)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop $04
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5300)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop $04
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5302)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5303)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5309)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/25 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5310)
	; 14 bytes
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	dey ;first loop only
	brk
	iny
	Jne *-5
	pla
	tay
	pla
.endif
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
.elseif(n = 5315 || n = 5327 || n = 5338 || n = 5339)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5316 || n = 5340)
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
.elseif((n > 5316 && n < 5322) || n = 5326 || (n > 5327 && n < 5331) || n = 5332 || n = 5333 || (n > 5340 && n < 5344) || n = 5347 || n = 5350 || n = 5351)
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
.elseif(n = 5322)
	; 14 bytes
	pha
	tya
	ldy #177
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.elseif(n = 5324)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5331)
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
.elseif(n = 5345)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5348)
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
.elseif(n = 5349)
	; 14 bytes
	pha
	tya
	ldy #100 ;hides 'nop zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 5352 to 5390
.if(n = 5352)
	; 14 bytes
	pha
	tya
	ldy #178
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif((n > 5352 && n < 5356) || n = 5361 || n = 5368 || n = 5373)
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
.elseif(n = 5358 || n = 5380)
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
.elseif(n = 5359)
	; 14 bytes
	delay_n (n-3)
	cmp $C5
.elseif(n = 5360 || n = 5369 || n = 5379 || n = 5381 || n = 5389 || n = 5390)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5362 || n = 5387)
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
.elseif(n = 5363 || n = 5374 || n = 5383 || n = 5384)
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
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5370)
	; 14 bytes
	delay_n (n-3)
	nop $04
.elseif(n = 5371)
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
	; 14 bytes
	php
	pha
	sec
	lda #244 ;hides 'nop zp,x'
	.byte $2C,0,0 ;bit abs; hides 'brk'
	sbc #1
	Jne *-6
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
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
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
.elseif(n = 5393 || (n > 5394 && n < 5400) || n = 5405 || n = 5409 || n = 5416 || (n > 5417 && n < 5422) || n = 5427)
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
.elseif(n = 5394 || n = 5417)
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
.elseif(n = 5400)
	; 12 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	brk
	dey
	Jne *-5
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
.elseif(n = 5402 || n = 5424)
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
.elseif(n = 5403 || n = 5404 || n = 5408 || n = 5414 || n = 5425 || n = 5426)
	; 14 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
	cmp <$CD ;hides 'cmp $1FA0'
	ldy #31
	nop
	brk
	iny
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
.elseif(n = 5413)
	; 14 bytes
	brk
	delay_n (n-13)
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
.elseif(n = 5422)
	; 13 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F6A0'
	ldy #246
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5423)
	; 13 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5428)
	; 14 bytes
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5429)
	; 14 bytes
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	nop
	brk
	iny
	Jne *-5
	pla
	tay
	pla
.endif
.else
; 5430 to 5468
.if((n > 5429 && n < 5433) || n = 5441 || n = 5443 || (n > 5448 && n < 5454) || n = 5455 || n = 5465)
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
.elseif(n = 5434)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5435)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1EA0'
	ldy #30
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5436 || n = 5460)
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
.elseif(n = 5438)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5439 || n = 5447 || n = 5448 || n = 5456 || n = 5464)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5440 || n = 5463)
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
.elseif(n = 5442)
	; 14 bytes
	pha
	tya
	ldy #181
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5457)
	; 14 bytes
	pha
	tya
	ldy #165
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5458)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $1DA0'
	ldy #29
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5459)
	; 14 bytes
	pha
	txa
	cmp <$CD ;hides 'cmp $1DA2'
	ldx #29
	nop
	brk
	inx
	Jne *-7
	tax
	pla
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
.if((n >= 5469) && (n < 5547))
.if((n >= 5469) && (n < 5508))
; 5469 to 5507
.if(n = 5469 || n = 5470 || n = 5492 || n = 5501)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5471)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5472)
	; 14 bytes
	pha
	tya
	ldy #182
	jsr @nearby_rts
	brk
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
	nop
	brk
	dey
	Jne *-5
	tay
	pla
	plp
.elseif((n > 5473 && n < 5482) || n = 5485 || n = 5494 || n = 5495 || n = 5497 || n = 5498 || n = 5500 || (n > 5502 && n < 5506))
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
.elseif(n = 5482)
	; 14 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1CA2'
	ldx #28
	nop
	brk
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5483)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1CA0'
	ldy #28
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5484)
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
.elseif(n = 5486)
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
.elseif(n = 5490)
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
.elseif(n = 5493)
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
.elseif(n = 5496)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	brk
	dey
	Jne *-7
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
.elseif(n = 5502)
	; 14 bytes
	pha
	tya
	ldy #183
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5506)
	; 14 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $1BA2'
	ldx #27
	nop
	brk
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5507)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1BA0'
	ldy #27
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 5508 to 5546
.if(n = 5508)
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
.elseif(n = 5509)
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
.elseif(n = 5510)
	; 12 bytes
	pha
	tya
	ldy #(n-10)/22 ;hides 'nop'
	nop
	brk
	dey
	Jne *-5
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
.elseif(n = 5512 || n = 5534)
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
.elseif(n = 5513 || n = 5514 || n = 5535 || n = 5536 || n = 5539)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5515)
	; 14 bytes
	php
	pha
	sec
	lda #(n-15)/22 ;hides 'nop'
	nop
	brk
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5516)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5517)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5518 || n = 5519 || (n > 5523 && n < 5527) || n = 5528 || n = 5529 || n = 5538 || (n > 5539 && n < 5546))
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
.elseif(n = 5520)
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
.elseif(n = 5521)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5522)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 4
	brk
	dey
	Jne *-7
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
.elseif(n = 5527)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 5531)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $1AA0'
	ldy #26
	nop
	brk
	iny
	Jne *-7
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
.elseif(n = 5546)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EAEA ;hides two 'nop's
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 5547 to 5625
.if(n = 5547 || n = 5574 || n = 5601)
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
.elseif(n = 5548)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 6
	brk
	dey
	Jne *-7
	tay
	pla
.elseif((n > 5548 && n < 5554) || n = 5559 || n = 5560 || (n > 5564 && n < 5574) || n = 5575 || (n > 5580 && n < 5587) || n = 5588 || n = 5590 || n = 5591 || n = 5593 || (n > 5594 && n < 5598) || (n > 5604 && n < 5611) || n = 5616 || n = 5619)
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
.elseif(n = 5556 || n = 5578 || n = 5600 || n = 5622)
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
.elseif(n = 5557 || n = 5558 || n = 5563 || n = 5564 || n = 5579 || n = 5594 || n = 5602 || (n > 5612 && n < 5616) || n = 5624 || n = 5625)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5561 || n = 5592 || n = 5623)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5562 || n = 5587 || n = 5612)
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
.elseif(n = 5580 || n = 5604)
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
.elseif(n = 5603)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $17A0'
	ldy #23
	nop
	brk
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
.elseif(n = 5618)
	; 14 bytes
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
	ldy #(n-10)/24 ;hides 'nop'
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
.elseif(n = 5629 || n = 5630 || n = 5641 || n = 5648 || n = 5649 || n = 5653 || (n > 5656 && n < 5662))
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
	delay_n 6
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5634 || n = 5635 || n = 5646 || n = 5656 || n = 5664)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5636)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	delay_n 6
	dey
	Jne *-5
	tay
	pla
.elseif(n = 5637 || n = 5662)
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
	delay_n 6
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5639 || n = 5640)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 5645)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 14
	dey
	Jne *-7
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
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	brk
	iny
	Jne *-6
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
.elseif(n = 5663)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5689)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif((n > 5665 && n < 5670) || n = 5671 || n = 5672 || n = 5677 || (n > 5678 && n < 5682) || n = 5683 || n = 5684 || n = 5686 || (n > 5689 && n < 5693) || (n > 5694 && n < 5698) || (n > 5700 && n < 5704))
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
.elseif(n = 5670 || n = 5693)
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
.elseif(n = 5673)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5674)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	delay_n 4
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5675)
	; 14 bytes
	pha
	tya
	ldy #236 ;hides 'cpx abs'
	delay_n 5
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5676 || n = 5700)
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
.elseif(n = 5678)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5682)
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
	; 14 bytes
	pha
	tya
	ldy #172
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5694)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 23
	Jne *-7
	tax
	pla
.elseif(n = 5698)
	; 13 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	brk
	iny
	Jne *-6
	tay
	pla
.elseif(n = 5699)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $EDA0'
	ldy #237
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 5704 to 5781
.if((n > 5703 && n < 5709) || n = 5710 || n = 5715 || n = 5717 || n = 5719 || n = 5720 || (n > 5724 && n < 5736) || n = 5738 || n = 5740 || n = 5741 || (n > 5742 && n < 5746) || n = 5750 || n = 5751 || n = 5753 || (n > 5754 && n < 5760) || n = 5761 || (n > 5764 && n < 5770) || (n > 5772 && n < 5778) || n = 5779)
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
.elseif(n = 5709 || n = 5736 || n = 5763)
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
.elseif(n = 5713 || n = 5714 || n = 5718 || n = 5739 || n = 5749 || n = 5764 || n = 5780)
	; 14 bytes
	delay_n (n-2)
	nop
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
	cmp #<$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5723)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5724 || n = 5748 || n = 5772)
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
.elseif(n = 5742)
	; 14 bytes
	pha
	tya
	ldy #191
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5746)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $11A0'
	ldy #17
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5752)
	; 14 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5760)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/23 ;hides 'nop'
	nop $04
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5770)
	; 14 bytes
	pha
	txa
	cmp #<$CD ;hides 'cmp $10A2'
	ldx #16
	nop
	brk
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5771)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	nop
	brk
	iny
	Jne *-7
	tay
	pla
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
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5785) || n = 5786 || n = 5788 || (n > 5790 && n < 5794) || (n > 5796 && n < 5802) || (n > 5802 && n < 5808) || n = 5813 || n = 5815 || n = 5816 || (n > 5820 && n < 5831) || (n > 5832 && n < 5837) || n = 5838 || n = 5841 || (n > 5844 && n < 5853) || n = 5856 || n = 5858 || n = 5859)
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
.elseif(n = 5785 || n = 5808 || n = 5831 || n = 5854)
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
.elseif(n = 5789 || n = 5811 || n = 5814 || n = 5839 || n = 5857)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5790 || n = 5817)
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
.elseif(n = 5794)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5795)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5796 || n = 5820 || n = 5844)
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
.elseif(n = 5802)
	; 14 bytes
	pha
	tya
	ldy #193
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.elseif(n = 5810)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5818)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5819)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0EA0'
	ldy #14
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5832)
	; 14 bytes
	pha
	tya
	ldy #194
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5842)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5843)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $F3A0'
	ldy #243
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5853)
	; 14 bytes
	pha
	tya
	ldy #177
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5855)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop
	dey
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 5860) && (n < 5899))
; 5860 to 5898
.if(n = 5860)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	clc
	Jcc *+2
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5861 || n = 5863 || n = 5869 || n = 5870 || n = 5872 || (n > 5873 && n < 5877) || (n > 5877 && n < 5885) || (n > 5892 && n < 5896))
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
.elseif(n = 5862 || n = 5887)
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
.elseif(n = 5864 || n = 5873 || n = 5891)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5865)
	; 13 bytes
	pha
	tya
	ldy #(n-9)/24 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 5868 || n = 5892)
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
.elseif(n = 5885)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	brk
	iny
	Jne *-7
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
.elseif(n = 5890)
	; 13 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5896)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/27 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
	pla
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
.else
; 5899 to 5937
.if(n = 5899)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop $04
	iny
	Jne *-7
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
.elseif(n = 5901 || n = 5903 || (n > 5904 && n < 5909) || n = 5911 || n = 5913 || n = 5917 || n = 5918 || n = 5920 || n = 5923 || n = 5924 || (n > 5926 && n < 5933) || n = 5934 || n = 5936)
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
.elseif(n = 5904 || n = 5914 || n = 5935)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5909)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5910)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	nop $04
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5915)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5921)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5925)
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
.elseif(n = 5926)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
.if((n >= 5938) && (n < 6016))
; 5938 to 6015
.if(n = 5938)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $F7A0'
	ldy #247
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5939 || n = 5966 || n = 5970 || n = 5989 || n = 5997 || n = 6014)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5940 || n = 5988)
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
.elseif((n > 5940 && n < 5952) || n = 5953 || n = 5954 || (n > 5955 && n < 5962) || n = 5965 || n = 5967 || n = 5969 || (n > 5970 && n < 5979) || n = 5980 || n = 5981 || n = 5983 || n = 5984 || (n > 5990 && n < 5995) || n = 5996 || (n > 5997 && n < 6006) || (n > 6006 && n < 6010) || n = 6013 || n = 6015)
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
.elseif(n = 5952)
	; 14 bytes
	pha
	tya
	ldy #180
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5955)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5962 || n = 5987 || n = 6012)
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
	tya
	cmp <$CD ;hides 'cmp $08A0'
	ldy #8
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5964 || n = 5995)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 5979 || n = 6006)
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
.elseif(n = 5982)
	; 14 bytes
	pha
	tya
	ldy #199
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
	cmp #<$CD ;hides 'cmp $07A0'
	ldy #7
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5990)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6010)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/24 ;hides 'nop'
	delay_n 4
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6011)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $06A0'
	ldy #6
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.endif
.else
; 6016 to 6093
.if(n = 6016)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 4
	brk
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 6017 || (n > 6019 && n < 6026) || n = 6027 || n = 6029 || n = 6031 || n = 6032 || n = 6040 || n = 6041 || (n > 6042 && n < 6051) || (n > 6051 && n < 6057) || n = 6061 || n = 6063 || (n > 6065 && n < 6072) || (n > 6072 && n < 6082) || n = 6086 || (n > 6090 && n < 6094))
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
.elseif(n = 6018)
	; 14 bytes
	pha
	tya
	ldy #182
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6019)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6026 || n = 6057 || n = 6088)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6028 || n = 6039 || n = 6059 || n = 6064 || n = 6089 || n = 6090)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6030)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6033)
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
.elseif(n = 6034)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6035)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $05A0'
	ldy #5
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6036 || n = 6060 || n = 6084)
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
.elseif(n = 6037 || n = 6062 || n = 6087)
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
.elseif(n = 6042)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6058)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $04A0'
	ldy #4
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6065)
	; 14 bytes
	pha
	tya
	ldy #84 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
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
.elseif(n = 6082)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6083)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6085)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	nop $04
	brk
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if(n = 6094)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif((n > 6094 && n < 6099) || n = 6101 || n = 6104 || n = 6105 || n = 6111 || n = 6113 || n = 6115 || n = 6116 || n = 6118 || n = 6122 || (n > 6123 && n < 6129))
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
.elseif(n = 6099)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	nop $04
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6102)
	; 14 bytes
	pha
	tya
	ldy #203
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6103)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6106)
	; 14 bytes
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6107)
	; 14 bytes
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6108 || n = 6132)
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
.elseif(n = 6109)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/25 ;hides 'nop zp,x'
	nop $FF44 ;hides 'nop zp'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6110)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	nop $04
	brk
	dey
	Jne *-7
	tay
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
.elseif(n = 6114)
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
.elseif(n = 6117)
	; 14 bytes
	pha
	tya
	ldy #185
	nop
	delay_n 26
	dey
	Jne *-6
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
.elseif(n = 6120)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 4
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6121)
	; 13 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6123)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6129)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	dey
	Jne *-7
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
.endif
.else
; 6133 to 6171
.if(n = 6133 || n = 6134 || n = 6136 || n = 6138 || n = 6140 || n = 6142 || n = 6148 || n = 6149 || n = 6151 || n = 6157 || (n > 6158 && n < 6162) || n = 6163 || (n > 6164 && n < 6168) || (n > 6168 && n < 6172))
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
.elseif(n = 6135)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	nop $04
	brk
	dey
	Jne *-7
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
.elseif(n = 6139 || n = 6146 || n = 6147 || n = 6152 || n = 6164)
	; 14 bytes
	delay_n (n-2)
	nop
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
	delay_n 8
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6153)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
	tay
	pla
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
.endif
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 4
	brk
	iny
	Jne *-7
	tay
	pla
.elseif((n > 6172 && n < 6176) || (n > 6176 && n < 6181) || (n > 6183 && n < 6187) || n = 6188 || n = 6190 || n = 6191 || n = 6193 || n = 6194 || (n > 6195 && n < 6212) || n = 6213 || n = 6215 || (n > 6216 && n < 6220) || n = 6221 || n = 6223 || n = 6224 || n = 6226 || (n > 6227 && n < 6237) || n = 6238 || (n > 6239 && n < 6243) || n = 6244 || (n > 6245 && n < 6249) || n = 6250)
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
.elseif(n = 6176)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6182)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6189 || n = 6214 || n = 6227 || n = 6239 || n = 6245)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6195 || n = 6222)
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
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 7
	brk
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 6249)
	; 14 bytes
	pha
	tya
	ldy #189
	nop
	delay_n 26
	dey
	Jne *-6
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
.if(n = 6251 || (n > 6252 && n < 6260) || n = 6261 || n = 6263 || (n > 6264 && n < 6274) || (n > 6276 && n < 6282) || (n > 6282 && n < 6287) || n = 6288 || (n > 6289 && n < 6303) || n = 6304 || n = 6308 || n = 6309 || n = 6311 || n = 6313 || (n > 6316 && n < 6328))
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
.elseif(n = 6252)
	; 14 bytes
	pha
	tya
	ldy #208
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6260)
	; 14 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	nop
	nop $04
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6262 || n = 6287 || n = 6312)
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
.elseif(n = 6264 || n = 6276 || n = 6289 || n = 6307 || n = 6314)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6282)
	; 14 bytes
	pha
	tya
	ldy #190
	nop
	delay_n 26
	dey
	Jne *-6
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
.elseif(n = 6310)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 6316)
	; 14 bytes
	brk
	delay_n (n-13)
.elseif(n = 6328)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/27 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 6329 to 6406
.if(n = 6329 || n = 6331 || (n > 6332 && n < 6336) || n = 6340 || n = 6341 || n = 6344 || n = 6345 || n = 6347 || (n > 6348 && n < 6352) || n = 6355 || n = 6356 || (n > 6357 && n < 6362) || n = 6363 || n = 6365 || n = 6366 || n = 6368 || n = 6370 || n = 6371 || (n > 6372 && n < 6376) || n = 6377 || n = 6382 || n = 6385 || n = 6386 || n = 6388 || n = 6390 || n = 6391 || (n > 6392 && n < 6396) || n = 6397)
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
.elseif(n = 6330 || n = 6357 || n = 6384)
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
.elseif(n = 6332)
	; 14 bytes
	pha
	tya
	ldy #218 ;hides 'nop'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
	pla
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
	brk
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6338 || n = 6339 || n = 6364 || n = 6369 || n = 6378 || n = 6380 || n = 6389 || n = 6400 || n = 6403 || n = 6405)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 4
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6346)
	; 14 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6352)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $EAEA ;hides two 'nop's
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6353)
	; 14 bytes
	pha
	tya
	ldy #(n-9)/26 ;hides 'nop zp,x'
	nop $EA ;hides 'nop'
	nop
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6354)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 6
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6379)
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 7
	iny
	Jne *-5
	tay
	pla
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
.elseif(n = 6383)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6392)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
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
.elseif(n = 6399)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	cmp $EAEA ;hides two 'nop's
	cmp $C5
	dey
	Jne *-7
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
.elseif(n = 6402)
	; 14 bytes
	pha
	tya
	ldy #213
	jsr @nearby_rts
	brk
	dey
	Jne *-6
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
.endif
.else
; 6407 to 6562
.if(n = 6407 || n = 6413 || (n > 6414 && n < 6429) || n = 6430 || (n > 6432 && n < 6437) || (n > 6438 && n < 6447) || (n > 6447 && n < 6460) || n = 6461 || n = 6463 || n = 6464 || (n > 6465 && n < 6480) || n = 6481 || n = 6483 || (n > 6484 && n < 6491) || (n > 6493 && n < 6506) || (n > 6506 && n < 6510) || n = 6511 || n = 6512 || (n > 6513 && n < 6519) || n = 6520 || n = 6521 || n = 6523 || (n > 6524 && n < 6544) || n = 6545 || (n > 6546 && n < 6552) || n = 6554 || (n > 6555 && n < 6563))
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
.elseif(n = 6408)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 6409)
	; 14 bytes
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
.elseif(n = 6411 || n = 6438 || n = 6465 || n = 6492 || n = 6519)
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
.elseif(n = 6429 || n = 6460 || n = 6491 || n = 6522 || n = 6553)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6431 || n = 6462 || n = 6484 || n = 6493 || n = 6524 || n = 6555)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6437)
	; 14 bytes
	pha
	tya
	ldy #68 ;hides 'nop zp'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6506)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
	tax
	pla
.elseif(n = 6510)
	; 14 bytes
	pha
	tya
	ldy #(n-10)/26 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
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
.elseif(n = 6544)
	; 13 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	delay_n 26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 6546)
	; 14 bytes
	pha
	tya
	ldy #198
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.endif
.endif
.else
.if((n >= 6563) && (n < 6719))
; 6563 to 6718
.if((n > 6562 && n < 6573) || (n > 6573 && n < 6579) || n = 6580 || n = 6581 || n = 6583 || n = 6585 || (n > 6586 && n < 6600) || (n > 6600 && n < 6610) || n = 6611 || n = 6613 || n = 6614 || n = 6616 || (n > 6617 && n < 6627) || (n > 6627 && n < 6631) || n = 6632 || n = 6633 || (n > 6634 && n < 6642) || n = 6643 || n = 6644 || n = 6647 || (n > 6648 && n < 6654) || (n > 6657 && n < 6672) || n = 6673 || n = 6674 || n = 6676 || (n > 6681 && n < 6698) || (n > 6698 && n < 6702) || (n > 6702 && n < 6708) || (n > 6711 && n < 6719))
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
.elseif(n = 6573 || n = 6600 || n = 6627 || n = 6654 || n = 6681)
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
.elseif(n = 6579)
	; 14 bytes
	pha
	tya
	ldy #199
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6584 || n = 6615 || n = 6646 || n = 6677 || n = 6708)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 6586 || n = 6617 || n = 6648 || n = 6679 || n = 6710)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 6631)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 8
	iny
	Jne *-7
	tay
	pla
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
	delay_n 6
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
.elseif(n = 6680)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6698)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	.byte $CD,0,0 ;cmp abs; hides 'brk'
	brk
	dey
	Jne *-7
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
.else
; 6719 to 6875
.if((n > 6718 && n < 6732) || n = 6733 || n = 6734 || n = 6736 || n = 6737 || n = 6740 || n = 6742 || (n > 6744 && n < 6760) || n = 6761 || (n > 6762 && n < 6770) || n = 6771 || (n > 6772 && n < 6777) || (n > 6777 && n < 6789) || n = 6790 || n = 6791 || (n > 6792 && n < 6796) || (n > 6796 && n < 6801) || n = 6802 || (n > 6803 && n < 6810) || (n > 6810 && n < 6816) || (n > 6816 && n < 6822) || n = 6823 || n = 6824 || (n > 6825 && n < 6832) || n = 6833 || (n > 6834 && n < 6843) || (n > 6843 && n < 6849) || n = 6850 || n = 6851 || n = 6853 || (n > 6854 && n < 6863) || n = 6864 || (n > 6865 && n < 6870) || (n > 6870 && n < 6876))
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
.elseif(n = 6735 || n = 6762 || n = 6789 || n = 6816 || n = 6870)
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
.elseif(n = 6738)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6741 || n = 6772 || n = 6803 || n = 6834 || n = 6865)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6743)
	; 14 bytes
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	delay_n 26
	dey
	Jne *-7
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
.elseif(n = 6760)
	; 13 bytes
	pha
	tya
	ldy #(n-10)/27 ;hides 'nop'
	delay_n 7
	brk
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6770 || n = 6801 || n = 6832 || n = 6863)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 6796)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 6825)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6843)
	; 14 bytes
	pha
	tya
	ldy #207
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6849)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6854)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
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
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
; 6876 to 7031
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
.elseif((n > 6876 && n < 6882) || n = 6884 || n = 6885 || n = 6887 || (n > 6888 && n < 6894) || n = 6895 || (n > 6897 && n < 6908) || n = 6911 || (n > 6915 && n < 6924) || n = 6926 || (n > 6927 && n < 6940) || n = 6941 || (n > 6942 && n < 6956) || n = 6957 || (n > 6958 && n < 6972) || n = 6973 || n = 6974 || (n > 6975 && n < 6986) || n = 6988 || (n > 6989 && n < 6996) || n = 6997 || (n > 6998 && n < 7002) || (n > 7002 && n < 7008) || n = 7009 || (n > 7010 && n < 7018) || n = 7019 || (n > 7020 && n < 7032))
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
.elseif(n = 6883)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6888 || n = 6896 || n = 6910 || n = 6914 || n = 6915 || n = 6927 || n = 6958 || n = 6989 || n = 6998 || n = 7020)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6894 || n = 6925 || n = 6956 || n = 6987 || n = 7018)
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
	delay_n 9
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 6940)
	; 14 bytes
	pha
	tya
	ldy #58 ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 6986)
	; 14 bytes
	pha
	tya
	ldy #218 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
	pla
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
.endif
.else
; 7032 to 7187
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
.elseif((n > 7032 && n < 7041) || (n > 7041 && n < 7049) || n = 7050 || (n > 7051 && n < 7057) || (n > 7057 && n < 7062) || (n > 7062 && n < 7074) || (n > 7075 && n < 7080) || (n > 7082 && n < 7086) || (n > 7086 && n < 7092) || (n > 7092 && n < 7107) || (n > 7107 && n < 7111) || n = 7112 || n = 7114 || (n > 7115 && n < 7122) || (n > 7122 && n < 7139) || n = 7143 || (n > 7144 && n < 7150) || n = 7151 || (n > 7152 && n < 7164) || (n > 7164 && n < 7168) || (n > 7169 && n < 7173) || n = 7174 || (n > 7175 && n < 7182) || n = 7183 || n = 7184 || n = 7186 || n = 7187)
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
.elseif(n = 7049 || n = 7080 || n = 7111 || n = 7142 || n = 7173)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7051 || n = 7082 || n = 7113 || n = 7144 || n = 7175)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7057)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 7
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 7075)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	delay_n 7
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7081)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop $2808 ;hides 'php' and 'plp'; reads from $2000
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7086)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 7115)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	delay_n 26
	Jne *-7
	tax
	pla
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
.elseif(n = 7139)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 7150)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
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
.elseif(n = 7168)
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
.elseif(n = 7185)
	; 14 bytes
	pha
	tya
	ldy #52 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
; 7188 to 7343
.if((n > 7187 && n < 7204) || n = 7205 || (n > 7206 && n < 7212) || (n > 7212 && n < 7235) || n = 7236 || n = 7238 || n = 7240 || n = 7241 || (n > 7242 && n < 7253) || n = 7254 || (n > 7255 && n < 7260) || (n > 7260 && n < 7266) || n = 7267 || (n > 7268 && n < 7272) || (n > 7272 && n < 7297) || n = 7298 || n = 7300 || n = 7301 || n = 7303 || n = 7304 || (n > 7305 && n < 7328) || n = 7329 || n = 7331 || (n > 7332 && n < 7338) || (n > 7338 && n < 7344))
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
.elseif(n = 7204 || n = 7235 || n = 7266 || n = 7297 || n = 7328)
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
.elseif(n = 7237 || n = 7255 || n = 7268 || n = 7299 || n = 7330)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7260)
	; 14 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	delay_n 9
	brk
	dey
	Jne *-7
	tay
	pla
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
.endif
.else
; 7344 to 7500
.if((n > 7343 && n < 7359) || n = 7360 || (n > 7362 && n < 7370) || (n > 7371 && n < 7390) || n = 7391 || n = 7393 || n = 7394 || (n > 7396 && n < 7404) || (n > 7404 && n < 7419) || (n > 7425 && n < 7430) || (n > 7430 && n < 7434) || n = 7435 || n = 7436 || (n > 7437 && n < 7452) || n = 7453 || (n > 7454 && n < 7459) || (n > 7459 && n < 7465) || (n > 7465 && n < 7470) || (n > 7470 && n < 7482) || n = 7484 || (n > 7485 && n < 7498) || n = 7499)
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
.elseif(n = 7359 || n = 7390 || n = 7421 || n = 7452 || n = 7483)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7361 || n = 7392 || n = 7423 || n = 7454 || n = 7485)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7370)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	jsr @nearby_rts
	brk
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 7395)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 7419)
	; 14 bytes
	pha
	tya
	ldy #212 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dey
	Jne *-7
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
.elseif(n = 7422)
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
.elseif(n = 7459)
	; 14 bytes
	pha
	tya
	ldy #226 ;hides 'nop #imm'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7465)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	nop
	delay_n 26
	dey
	Jne *-7
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
.elseif(n = 7498)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 7500)
	; 14 bytes
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
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
.if((n >= 7501) && (n < 8751))
.if((n >= 7501) && (n < 8126))
.if((n >= 7501) && (n < 7813))
; 7501 to 7812
.if(n = 7501 || n = 7502 || (n > 7503 && n < 7510) || n = 7511 || n = 7513 || n = 7515 || (n > 7516 && n < 7536) || (n > 7536 && n < 7542) || n = 7543 || n = 7544 || n = 7546 || (n > 7547 && n < 7569) || n = 7570 || n = 7571 || (n > 7572 && n < 7576) || n = 7577 || (n > 7578 && n < 7600) || n = 7601 || (n > 7602 && n < 7607) || n = 7608 || (n > 7609 && n < 7632) || n = 7633 || n = 7634 || n = 7636 || n = 7637 || n = 7639 || (n > 7640 && n < 7651) || n = 7652 || (n > 7653 && n < 7662) || (n > 7662 && n < 7666) || n = 7667 || n = 7670 || (n > 7671 && n < 7681) || n = 7682 || (n > 7683 && n < 7692) || (n > 7692 && n < 7700) || (n > 7702 && n < 7724) || (n > 7724 && n < 7731) || (n > 7734 && n < 7762) || n = 7763 || n = 7765 || n = 7766 || n = 7768 || (n > 7769 && n < 7793) || n = 7794 || (n > 7795 && n < 7800) || (n > 7800 && n < 7813))
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
.elseif(n = 7512 || n = 7516 || n = 7547 || n = 7578 || n = 7609 || n = 7640 || n = 7653 || n = 7671 || n = 7683 || n = 7702 || n = 7733 || n = 7764 || n = 7769 || n = 7795)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7514 || n = 7545 || n = 7576 || n = 7607 || n = 7638 || n = 7669 || n = 7700 || n = 7731 || n = 7762 || n = 7793)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
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
.elseif(n = 7542)
	; 14 bytes
	pha
	tya
	ldy #251
	jsr @nearby_rts
	brk
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 7600)
	; 13 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	delay_n 26
	iny
	Jne *-6
	tay
	pla
.elseif(n = 7602)
	; 14 bytes
	pha
	tya
	ldy #230
	nop
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.else
.if((n >= 7813) && (n < 7969))
; 7813 to 7968
.if((n > 7812 && n < 7824) || n = 7825 || (n > 7826 && n < 7833) || (n > 7833 && n < 7855) || n = 7856 || (n > 7857 && n < 7866) || (n > 7866 && n < 7886) || n = 7887 || (n > 7888 && n < 7899) || (n > 7899 && n < 7906) || n = 7907 || n = 7911 || n = 7912 || (n > 7913 && n < 7917) || n = 7918 || (n > 7919 && n < 7926) || n = 7927 || (n > 7933 && n < 7937) || n = 7938 || n = 7943 || (n > 7944 && n < 7948) || n = 7949 || (n > 7950 && n < 7965) || (n > 7965 && n < 7969))
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
.elseif(n = 7824 || n = 7855 || n = 7886 || n = 7917)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/31
	delay_n 26
	dey
	Jne *-5
	tay
	pla
.elseif(n = 7826 || n = 7857 || n = 7888 || n = 7908 || n = 7910 || n = 7919 || n = 7928 || n = 7930 || n = 7939 || n = 7941 || n = 7950)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7913 || n = 7933 || n = 7944)
	; 14 bytes
	php
	delay_n (n-7)
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
.elseif(n = 7929)
	; 14 bytes
	delay_n (n-3)
	cmp $C5
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
.endif
.else
; 7969 to 8125
.if((n > 7968 && n < 7998) || (n > 7998 && n < 8010) || (n > 8010 && n < 8024) || n = 8025 || (n > 8026 && n < 8031) || (n > 8031 && n < 8060) || (n > 8060 && n < 8064) || (n > 8064 && n < 8095) || n = 8096 || (n > 8097 && n < 8126))
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
.elseif(n = 8010)
	; 14 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	jsr @nearby_rts
	brk
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 8026)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8060)
	; 14 bytes
	pha
	tya
	ldy #26 ;hides 'nop'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
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
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
; 8126 to 8437
.if((n > 8125 && n < 8130) || (n > 8130 && n < 8161) || n = 8162 || (n > 8163 && n < 8188) || (n > 8188 && n < 8192) || n = 8194 || n = 8195 || (n > 8196 && n < 8200) || (n > 8200 && n < 8229) || (n > 8229 && n < 8235) || (n > 8235 && n < 8259) || n = 8261 || (n > 8262 && n < 8270) || (n > 8270 && n < 8281) || n = 8282 || (n > 8283 && n < 8295) || (n > 8295 && n < 8305) || (n > 8305 && n < 8328) || (n > 8328 && n < 8361) || (n > 8361 && n < 8394) || (n > 8394 && n < 8416) || n = 8417 || (n > 8418 && n < 8427) || (n > 8427 && n < 8438))
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
.elseif(n = 8161)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
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
.elseif(n = 8235)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8259)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	iny
	Jne *-7
	tay
	pla
.elseif(n = 8260)
	; 13 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	delay_n 26
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 8270)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	nop
	delay_n 26
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 8283 || n = 8418)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8305)
	; 14 bytes
	pha
	tya
	ldy #20 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	iny
	Jne *-7
	tay
	pla
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
.endif
.else
; 8438 to 8750
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
.elseif(n = 8439 || (n > 8440 && n < 8448) || (n > 8451 && n < 8460) || (n > 8460 && n < 8515) || (n > 8515 && n < 8538) || (n > 8540 && n < 8550) || (n > 8550 && n < 8585) || (n > 8585 && n < 8671) || (n > 8671 && n < 8694) || (n > 8694 && n < 8704) || (n > 8705 && n < 8751))
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
.elseif(n = 8440 || n = 8450 || n = 8451 || n = 8540)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8515)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	dey ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 8539)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	.byte $0C,0,0 ;nop abs; hides 'brk'
	brk
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8550)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8585)
	; 14 bytes
	pha
	tya
	ldy #244 ;hides 'nop zp,x'
	iny ;first loop only
	delay_n 26
	dey
	Jne *-7
	tay
	pla
.elseif(n = 8671)
	; 14 bytes
	pha
	tya
	ldy #0 ;hides 'brk imm'
	iny ;first loop only
	cmp $C5
	brk
	iny
	Jne *-7
	tay
	pla
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
.endif
.endif
.endif
.else
; 8751 to 10000
.if((n > 8750 && n < 8760) || (n > 8760 && n < 8795) || n = 8796 || (n > 8797 && n < 8926) || (n > 8926 && n < 8942) || (n > 8942 && n < 8950) || (n > 8950 && n < 8959) || (n > 8961 && n < 9052) || n = 9053 || (n > 9054 && n < 9309) || n = 9310 || (n > 9311 && n < 9566) || n = 9567 || (n > 9568 && n < 9691) || (n > 9691 && n < 9718) || (n > 9718 && n < 9724) || (n > 9724 && n < 9729) || (n > 9729 && n < 9823) || n = 9824 || (n > 9825 && n < 10001))
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
.elseif(n = 8760)
	; 14 bytes
	pha
	tya
	ldy #250 ;hides 'nop'
	nop
	delay_n 26
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 8797 || n = 9054 || n = 9311 || n = 9568 || n = 9825)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 8942)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 39
	Jne *-7
	tax
	pla
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
	delay_n 6
	brk
	dey
	Jne *-7
	tay
	pla
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
.endif
.else
.if((n >= 10001) && (n < 15001))
; 10001 to 15000
.if((n > 10000 && n < 10080) || n = 10081 || (n > 10082 && n < 10337) || n = 10338 || (n > 10339 && n < 10594) || n = 10595 || (n > 10596 && n < 10851) || n = 10852 || (n > 10853 && n < 11108) || n = 11109 || (n > 11110 && n < 11221) || (n > 11221 && n < 11254) || (n > 11254 && n < 11265) || (n > 11265 && n < 11365) || n = 11366 || (n > 11367 && n < 11622) || n = 11623 || (n > 11624 && n < 11879) || n = 11880 || (n > 11881 && n < 12136) || n = 12137 || (n > 12138 && n < 12393) || n = 12394 || (n > 12395 && n < 12650) || n = 12651 || (n > 12652 && n < 12907) || n = 12908 || (n > 12909 && n < 13164) || n = 13165 || (n > 13166 && n < 13421) || n = 13422 || (n > 13423 && n < 13678) || n = 13679 || (n > 13680 && n < 13935) || n = 13936 || (n > 13937 && n < 14192) || n = 14193 || (n > 14194 && n < 14449) || n = 14450 || (n > 14451 && n < 14706) || n = 14707 || (n > 14708 && n < 14963) || n = 14964 || (n > 14965 && n < 15001))
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
.if((n > 15000 && n < 15220) || n = 15221 || (n > 15222 && n < 15477) || n = 15478 || (n > 15479 && n < 15734) || n = 15735 || (n > 15736 && n < 15991) || n = 15992 || (n > 15993 && n < 16248) || n = 16249 || (n > 16250 && n < 16505) || n = 16506 || (n > 16507 && n < 16762) || n = 16763 || (n > 16764 && n < 17019) || n = 17020 || (n > 17021 && n < 17276) || n = 17277 || (n > 17278 && n < 17533) || n = 17534 || (n > 17535 && n < 17790) || n = 17791 || (n > 17792 && n < 18047) || n = 18048 || (n > 18049 && n < 18304) || n = 18305 || (n > 18306 && n < 18561) || n = 18562 || (n > 18563 && n < 18818) || n = 18819 || (n > 18820 && n < 19075) || n = 19076 || (n > 19077 && n < 19332) || n = 19333 || (n > 19334 && n < 19589) || n = 19590 || (n > 19591 && n < 19846) || n = 19847 || (n > 19848 && n < 20001))
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
