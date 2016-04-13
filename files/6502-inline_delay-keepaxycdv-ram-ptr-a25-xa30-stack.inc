; delay_n, delays n cycles with small code
; Will clobber S, and Z+N
; Will preserve A, X, Y, C, D, V, and I (interrupt-disable flag)
;
; WARNING: THIS VERSION CLOBBERS THE STACK POINTER.
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
; - 4 bytes or less when n <= 18
; - 5 bytes or less when n <= 22
; - 6 bytes or less when n <= 30
; - 7 bytes or less when n <= 33
; - 8 bytes or less when n <= 38
; - 9 bytes or less when n <= 298
; -10 bytes or less when n <= 315
; -11 bytes or less when n <= 1300
; -12 bytes or less when n <= 1855
; -13 bytes or less when n <= 2623
; -14 bytes or less when n <= 4201
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.1, max=3
; - When 11 <= n <= 20: min=3, avg=3.9, max=5
; - When 21 <= n <= 30: min=5, avg=5.8, max=6
; - When 31 <= n <= 50: min=7, avg=8.4, max=9
; - When 51 <= n <= 75: min=8, avg=8.9, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=9.9, max=11
; - When 501 <= n <= 750: min=8, avg=10.0, max=11
; - When 751 <= n <= 1000: min=9, avg=10.0, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.5, max=13
; - When 2001 <= n <= 3000: min=10, avg=11.7, max=14
; - When 3001 <= n <= 5000: min=11, avg=12.9, max=15
; - When 5001 <= n <= 7500: min=12, avg=14.6, max=15
; - When 7501 <= n <= 10000: min=15, avg=15.0, max=15
; - When 10001 <= n <= 20000: min=15, avg=15.0, max=15
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
	; 1 bytes
	pha
.elseif(n = 4 || n = 8 || n = 16 || n = 17 || n = 31 || n = 32 || n = 34)
	; 2..8 bytes
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
.elseif((n > 8 && n < 12) || n = 13 || n = 19 || (n > 20 && n < 24) || n = 25 || (n > 34 && n < 38))
	; 3..8 bytes
	delay_n (n-7)
	delay_n 7
.elseif(n = 12)
	; 3 bytes
	jsr @nearby_rts
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 15)
	; 3 bytes
	jsr @nearby_rts_15cyc
.elseif(n = 18 || n = 20 || n = 27 || n = 29 || n = 30 || n = 33 || n = 39)
	; 4..9 bytes
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
	clc
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
.elseif((n > 40 && n < 45) || n = 46 || n = 47 || n = 49 || n = 51 || n = 52 || n = 54 || (n > 55 && n < 60) || (n > 60 && n < 65) || (n > 65 && n < 69) || (n > 69 && n < 79))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
	pla
	plp
.elseif(n = 45)
	; 9 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
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
.else
; 79 to 156
.if((n > 78 && n < 125) || (n > 125 && n < 130) || (n > 130 && n < 135) || (n > 135 && n < 157))
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.if((n > 156 && n < 170) || (n > 170 && n < 290) || (n > 290 && n < 295) || n = 296)
	; 9 bytes
	php
	pha
	lda #n-41
	jsr delay_a_25_clocks
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
.elseif(n = 297 || n = 312)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 298 || n = 299)
	; 9..10 bytes
	pha
	delay_n (n-3)
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
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 304 || n = 309)
	; 10 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 311)
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
.elseif(n = 323 || n = 368)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 325 || n = 330 || n = 335 || n = 340 || n = 345 || n = 360 || n = 365 || n = 370 || n = 375 || n = 380 || n = 388)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif(n = 328 || n = 333)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
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
.elseif(n = 393 || n = 400 || n = 405 || n = 410 || n = 415 || n = 430 || n = 435 || n = 440 || n = 445 || n = 450 || n = 465)
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
.elseif(n = 398 || n = 438)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 403)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 433 || n = 458)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
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
.if(n = 469 || n = 479 || n = 484 || n = 486 || n = 489 || n = 494 || n = 499 || n = 501 || n = 504 || n = 506 || n = 513 || n = 519 || n = 524 || n = 526 || n = 529 || n = 531 || n = 534 || n = 536 || n = 539 || n = 541)
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
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
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
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 631 || n = 671 || n = 703)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
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
.elseif(n = 640 || n = 645 || n = 650 || n = 655 || n = 660 || n = 675 || n = 680 || n = 685 || n = 693 || n = 698)
	; 9..10 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 5 = 2) && (n = 642 || (n > 661 && n < 703)))
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
.elseif(n = 643 || n = 683)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 696)
	; 10 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 701)
	; 10 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
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
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
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
.elseif(n = 713 || n = 778)
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
.elseif(n = 748 || n = 753 || n = 773)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif(n = 783 || n = 911)
	; 10 bytes
	sty @zptemp
	ldy #(n-15)/8
	pha
	dey
	Jpl *-2
	ldy @zptemp
.elseif(n = 784 || n = 793 || n = 794 || n = 799 || n = 801 || n = 804 || n = 806 || n = 809 || n = 811 || n = 814 || n = 819 || n = 821 || n = 829 || n = 834 || n = 836 || n = 839 || n = 844 || n = 846 || n = 849 || n = 851 || n = 854 || n = 856 || n = 864 || n = 869 || n = 874 || n = 879 || n = 881 || n = 884 || n = 886 || n = 889 || n = 891 || n = 899 || n = 904 || n = 906 || n = 909 || n = 914 || n = 916 || n = 919 || n = 921 || n = 924 || n = 926 || n = 933 || n = 934)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 0) && ((n > 784 && n < 801) || (n > 814 && n < 836) || (n > 849 && n < 871) || (n > 884 && n < 901) || (n > 919 && n < 936)))
	; 10 bytes
	pha
	delay_n (n-3)
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
	ldy #96
	sta @zptemp
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
.elseif(n = 818 || n = 878 || n = 918)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
.elseif(n = 823 || n = 863 || n = 871)
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
.elseif(n = 848 || n = 893 || n = 913 || n = 923 || n = 928)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
	ldy #72
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
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
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
.elseif(n = 1023 || n = 1028 || n = 1035 || n = 1040 || n = 1045 || n = 1060 || n = 1065 || n = 1070 || n = 1075 || n = 1080)
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
	stx @zptemp
	ldx #202 ;hides 'dex'
	Jne *-1
	sta (@ptrtemp,x)
	ldx @zptemp
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
	sta (@ptrtemp,x)
	tax
	pla
.elseif(n = 1033)
	; 11 bytes
	sty @zptemp
	ldy #(n-16)/9
	delay_n 4
	dey
	Jpl *-3
	ldy @zptemp
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
.elseif(n = 1053)
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
.elseif(n = 1088 || n = 1093)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.endif
.endif
.else
; 1094 to 1250
.if(n = 1094 || n = 1096 || n = 1099 || n = 1108 || n = 1109 || n = 1114 || n = 1119 || n = 1121 || n = 1124 || n = 1126 || n = 1129 || n = 1131 || n = 1134 || n = 1136 || n = 1144 || n = 1149 || n = 1154 || n = 1156 || n = 1159 || n = 1161 || n = 1164 || n = 1166 || n = 1169 || n = 1171 || n = 1178 || n = 1179 || n = 1184 || n = 1186 || n = 1189 || n = 1194 || n = 1196 || n = 1199 || n = 1201 || n = 1204 || n = 1206 || n = 1214 || n = 1219 || n = 1221 || n = 1224 || n = 1226 || n = 1229 || n = 1234 || n = 1236 || n = 1239 || n = 1241 || n = 1249)
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
.elseif(n = 1098 || n = 1138 || n = 1158 || n = 1193)
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
.elseif(n = 1133 || n = 1163 || n = 1198 || n = 1203 || n = 1208 || n = 1233)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
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
.elseif((n .mod 7 = 0) && (n = 1253 || n = 1260 || (n > 1280 && n < 1324)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1254 || n = 1256 || n = 1259 || n = 1261 || n = 1264 || n = 1266 || n = 1269 || n = 1274 || n = 1276 || n = 1283 || n = 1284 || n = 1289 || n = 1291 || n = 1294 || n = 1296 || n = 1301 || n = 1304 || n = 1313 || n = 1318 || n = 1325)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 1255 || n = 1270 || n = 1275 || n = 1280 || n = 1285 || n = 1290 || n = 1306 || n = 1310)
	; 10..11 bytes
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
.elseif(n = 1273)
	; 11 bytes
	pha
	tya
	ldy #(n-18)/5
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1278)
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
	tya
	ldy #0
	dey
	Jne *-1
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 1299)
	; 11 bytes
	php
	delay_n (n-7)
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
.elseif(n = 1308 || n = 1317 || n = 1326)
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
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1315)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
; 1329 to 1406
.if(n = 1329 || n = 1334 || n = 1346 || n = 1350 || n = 1355 || n = 1360 || n = 1361 || n = 1369 || n = 1374 || n = 1381 || n = 1385 || n = 1388 || n = 1395 || n = 1406)
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
.elseif(n = 1331 || n = 1333 || n = 1338 || n = 1345 || n = 1354 || n = 1373 || n = 1378 || n = 1390 || n = 1394 || n = 1401 || n = 1403)
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
	; 12 bytes
	sty @zptemp
	ldy #(n-13)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
	; 12 bytes
	delay_n (n-4)
	delay_n 4
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
; 1407 to 1484
.if((n .mod 7 = 0) && ((n > 1406 && n < 1485)))
	; 10 bytes
	sty @zptemp
	ldy #n/7-1
	nop
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1408 || n = 1413 || n = 1418 || n = 1424 || n = 1429 || n = 1434 || n = 1445 || n = 1450 || n = 1457 || n = 1478 || n = 1480)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1409 || n = 1430 || n = 1437 || n = 1441 || n = 1444 || n = 1458 || n = 1472 || n = 1473)
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
.elseif(n = 1411 || n = 1420 || n = 1465 || n = 1474 || n = 1483)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 7 = 5) && (n = 1412 || n = 1419 || (n > 1432 && n < 1483)))
	; 10 bytes
	pha
	tya
	ldy #(n-12)/7
	nop
	dey
	Jne *-2
	tay
	pla
.elseif(n = 1415 || n = 1423 || n = 1439 || n = 1455 || n = 1471 || n = 1479)
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1416 || n = 1425 || n = 1443 || n = 1452)
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
.elseif(n = 1422 || n = 1432 || n = 1462)
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
.elseif(n = 1427 || n = 1467)
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
.elseif(n = 1436 || n = 1460)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1446)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	nop
	dey
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 1448 || n = 1481)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
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
	delay_n 4
	nop
	dey
	Jne *-4
	tay
	pla
.elseif(n = 1459)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1469)
	; 12 bytes
	sty @zptemp
	ldy #(n-13)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
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
.else
; 1485 to 1562
.if(n = 1485 || n = 1490 || n = 1494 || n = 1499 || n = 1513 || n = 1520 || n = 1534 || n = 1541 || n = 1550)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1486 || n = 1493 || n = 1504 || n = 1507 || n = 1509 || n = 1514 || n = 1521 || n = 1523 || n = 1539 || n = 1544 || n = 1549 || n = 1553 || n = 1556)
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
.elseif(n = 1502 || n = 1522 || n = 1562)
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
.elseif(n = 1525)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jmi *-4
	ldx @zptemp
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
.endif
.endif
.endif
.else
.if((n >= 1563) && (n < 1719))
.if((n >= 1563) && (n < 1641))
; 1563 to 1640
.if(n = 1563 || n = 1570 || n = 1577 || n = 1585 || n = 1593 || n = 1598 || n = 1612 || n = 1619 || n = 1626 || n = 1630 || n = 1633 || n = 1640)
	; 11..12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 7) && (n = 1564 || (n > 1599 && n < 1628)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
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
.elseif((n .mod 8 = 7) && (n = 1567 || (n > 1582 && n < 1608) || n = 1623 || n = 1639))
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
.elseif(n = 1569 || n = 1578 || n = 1605 || n = 1614)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif(n = 1571 || n = 1576 || n = 1586 || n = 1590 || n = 1597 || n = 1606 || n = 1611 || n = 1625 || n = 1632 || n = 1637)
	; 10..11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1572 || n = 1584 || n = 1620)
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
.elseif(n = 1579)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 1595)
	; 12 bytes
	sty @zptemp
	ldy #(n-13)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 1613)
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
.elseif(n = 1635)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 1725 || n = 1730 || n = 1737 || n = 1739 || n = 1746 || n = 1754 || n = 1760 || n = 1765 || n = 1770 || n = 1774 || n = 1779 || n = 1781 || n = 1786 || n = 1793)
	; 11 bytes
	pha
	delay_n (n-3)
.elseif(n = 1726 || n = 1744 || n = 1780 || n = 1789)
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
.elseif((n .mod 9 = 3) && ((n > 1730 && n < 1759) || n = 1794))
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
.elseif(n = 1763)
	; 12 bytes
	sty @zptemp
	ldy #(n-13)/7
	nop
	dey
	Jne *-2
	sta (@ptrtemp),y
	ldy @zptemp
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
; 1797 to 1875
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
.elseif((n .mod 9 = 7) && (n = 1798 || (n > 1815 && n < 1871)))
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
.elseif(n = 1800 || n = 1824 || n = 1836 || n = 1860 || n = 1872)
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
.elseif(n = 1802 || n = 1822 || n = 1842 || n = 1862)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 9 = 3) && ((n > 1802 && n < 1831) || (n > 1847 && n < 1876)))
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
.elseif(n = 1806 || n = 1809 || n = 1814 || n = 1841 || n = 1849 || n = 1854 || n = 1856 || n = 1859 || n = 1864 || n = 1865 || n = 1873)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 7) && ((n > 1806 && n < 1872)))
	; 10 bytes
	sty @zptemp
	ldy #(n-7)/8
	pha
	dey
	Jne *-2
	ldy @zptemp
.elseif(n = 1808 || n = 1813 || n = 1818 || n = 1826 || n = 1835 || n = 1840 || n = 1850 || n = 1851 || n = 1858 || n = 1874)
	; 11..12 bytes
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
.elseif(n = 1817 || n = 1827)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1819 || n = 1867)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1820 || n = 1828 || n = 1844 || n = 1868)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/8
	sta @zptemp
	dey
	Jne *-3
	tay
	pla
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
.elseif(n = 1832)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 4
	Jne *-3
	ldx @zptemp
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
	pha
	tya
	ldy #(n-12)/11
	delay_n 4
	nop
	dey
	Jne *-4
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
.elseif(n = 1877 || n = 1907 || n = 1917 || n = 1937 || n = 1957 || n = 1977 || n = 1997 || n = 2017 || n = 2027)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 1878 || n = 1881 || n = 1886 || n = 1889 || n = 1894 || n = 1905 || n = 1913 || n = 1918 || n = 1931 || n = 1941 || n = 1945 || n = 1949 || n = 1950 || n = 1953 || n = 1955 || n = 1961 || n = 1966 || n = 1971 || n = 1984 || n = 1985 || n = 1989 || n = 1990 || n = 1993 || n = 2006 || n = 2013 || n = 2021 || n = 2024 || n = 2025 || n = 2030)
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
.elseif(n = 1880 || n = 1890 || n = 1898 || n = 1914 || n = 1930 || n = 1936 || n = 1946 || n = 1970 || n = 1986 || n = 1994 || n = 2008 || n = 2018 || n = 2026)
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
.elseif(n = 1899 || n = 1910 || n = 1921 || n = 1954 || n = 1976 || n = 1998 || n = 2009)
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
.elseif(n = 2074 || n = 2078 || n = 2079 || n = 2085 || n = 2090 || n = 2094)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2075 || n = 2108)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
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
.elseif(n = 2081)
	; 12 bytes
	sty @zptemp
	ldy #(n-24)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	ldy @zptemp
.elseif(n = 2084)
	; 10 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	dex
	dex
	Jne *-3
	ldx @zptemp
.elseif(n = 2087 || n = 2097)
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
.elseif(n = 2093)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2096 || n = 2101)
	; 11 bytes
	pha
	delay_n (n-3)
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
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	iny
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
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
.endif
.endif
.else
; 2110 to 2187
.if(n = 2110 || n = 2115 || n = 2120 || n = 2129 || n = 2133 || n = 2134 || n = 2138 || n = 2144 || n = 2150 || n = 2151 || n = 2156 || n = 2164 || n = 2165 || n = 2169 || n = 2178 || n = 2183 || n = 2186)
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
.elseif(n = 2114 || n = 2125 || n = 2139 || n = 2161 || n = 2170 || n = 2175 || n = 2180)
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 2135 || n = 2146 || n = 2168)
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
.elseif(n = 2166)
	; 12 bytes
	sty @zptemp
	ldy #(n-24)/17
	jsr @nearby_rts
	dey
	Jpl *-4
	ldy @zptemp
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
.elseif(n = 2192 || n = 2202 || n = 2222 || n = 2242 || n = 2252)
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
.elseif(n = 2195 || n = 2206 || n = 2213 || n = 2225 || n = 2241 || n = 2260)
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
.elseif(n = 2197 || n = 2207 || n = 2237 || n = 2247)
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
.elseif(n = 2201 || n = 2223 || n = 2234 || n = 2245)
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
.elseif(n = 2238)
	; 10 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-3
	ldx @zptemp
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
	pha
	tya
	ldy #248 ;hides 'sed'
	nop
	dey
	Jne *-3
	tay
	pla
	plp
.endif
.else
; 2266 to 2343
.if((n .mod 9 = 7) && ((n > 2265 && n < 2303)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/9
	delay_n 4
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2267 || n = 2277 || n = 2297 || n = 2317 || n = 2327 || n = 2337)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2268 || n = 2292 || n = 2304 || n = 2328 || n = 2340)
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2269 || n = 2321)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2270 || n = 2274 || n = 2279 || n = 2286 || n = 2291 || n = 2301 || n = 2309 || n = 2313 || n = 2314 || n = 2318 || n = 2324 || n = 2329)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 9 = 3) && ((n > 2270 && n < 2308)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 10 = 2) && (n = 2272 || n = 2282 || (n > 2311 && n < 2343)))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2273 || n = 2295 || n = 2306)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2276 || n = 2283 || n = 2296 || n = 2305 || n = 2310 || n = 2315 || n = 2320 || n = 2326 || n = 2338 || n = 2343)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2278 || n = 2300 || n = 2333)
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
.elseif(n = 2285 || n = 2319)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2287 || n = 2299 || n = 2323 || n = 2335)
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
.elseif(n = 2290 || n = 2341)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2294 || n = 2308 || n = 2336)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2303 || n = 2331)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
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
.elseif(n = 2325)
	; 10 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	pha
	iny
	Jne *-3
	ldy @zptemp
.elseif(n = 2330)
	; 11 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	sta @zptemp
	iny
	Jne *-4
	tay
	pla
.elseif(n = 2334)
	; 11 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	jsr @nearby_rts
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
.endif
.endif
.else
.if((n >= 2344) && (n < 2422))
; 2344 to 2421
.if(n = 2344 || n = 2355 || n = 2366 || n = 2399 || n = 2421)
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
.elseif(n = 2345)
	; 10 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2346 || n = 2349 || n = 2354 || n = 2356 || n = 2369 || n = 2374 || n = 2384 || n = 2385 || n = 2389 || n = 2390 || n = 2396 || n = 2404 || n = 2409 || n = 2411 || n = 2414 || n = 2418)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 12 = 7) && ((n > 2346 && n < 2420)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2348 || n = 2365 || n = 2379 || n = 2380 || n = 2391 || n = 2403 || n = 2408 || n = 2413)
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
.elseif(n = 2351)
	; 12 bytes
	delay_n 6
	delay_n (n-6)
.elseif((n .mod 12 = 0) && ((n > 2351 && n < 2413)))
	; 11 bytes
	pha
	tya
	ldy #n/12-1
	delay_n 7
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2353 || n = 2370)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 10 = 7) && ((n > 2356 && n < 2398) || n = 2417))
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
.elseif(n = 2360 || n = 2386)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2361 || n = 2394 || n = 2405 || n = 2416)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 10 = 2) && ((n > 2361 && n < 2403)))
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
.elseif(n = 2373 || n = 2401 || n = 2415)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2378 || n = 2406 || n = 2420)
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
.elseif(n = 2393)
	; 11 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	pha
	dey
	dey
	Jne *-4
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
.endif
.else
; 2422 to 2500
.if((n .mod 10 = 2) && ((n > 2421 && n < 2463) || n = 2482 || n = 2492))
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif(n = 2423 || n = 2426 || n = 2428 || n = 2433 || n = 2440 || n = 2445 || n = 2450 || n = 2459 || n = 2464 || n = 2469 || n = 2473 || n = 2474 || n = 2478 || n = 2481 || n = 2483 || n = 2486 || n = 2488 || n = 2494 || n = 2495)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2425 || n = 2463)
	; 12 bytes
	pha
	tya
	ldy #(n-31)/19
	jsr @nearby_rts_14cyc
	dey
	Jpl *-4
	tay
	pla
.elseif((n .mod 10 = 7) && ((n > 2426 && n < 2458) || (n > 2476 && n < 2498)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2429 || n = 2485 || n = 2499)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2430 || n = 2435 || n = 2444 || n = 2449 || n = 2470 || n = 2475 || n = 2480 || n = 2500)
	; 11..12 bytes
	pha
	delay_n (n-3)
.elseif((n .mod 12 = 7) && ((n > 2430 && n < 2492)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2434 || n = 2490)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2438 || n = 2471 || n = 2493)
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
.elseif(n = 2454 || n = 2465 || n = 2476 || n = 2498)
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
.elseif(n = 2461)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 2489)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2502 || n = 2512 || n = 2522 || n = 2542 || n = 2552)
	; 11 bytes
	pha
	tya
	ldy #(n-12)/10
	inc @zptemp
	dey
	Jne *-3
	tay
	pla
.elseif((n .mod 12 = 7) && ((n > 2502 && n < 2540) || n = 2563 || n = 2575))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2504 || n = 2526 || n = 2548 || n = 2559 || n = 2570)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2505 || n = 2510 || n = 2514 || n = 2519 || n = 2524 || n = 2529 || n = 2533 || n = 2534 || n = 2536 || n = 2549 || n = 2550 || n = 2554 || n = 2558 || n = 2561 || n = 2565 || n = 2576 || n = 2577)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2506 || n = 2523 || n = 2540)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2507 || n = 2517 || n = 2537 || n = 2547)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	pha
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2509 || n = 2531 || n = 2553 || n = 2564)
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
.elseif(n = 2513 || n = 2541 || n = 2555 || n = 2569)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2518 || n = 2560 || n = 2574)
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
.elseif(n = 2525 || n = 2530 || n = 2535 || n = 2566 || n = 2571 || n = 2573 || n = 2578)
	; 12 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 2543)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2546)
	; 11 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	nop
	dex
	dex
	Jne *-4
	ldx @zptemp
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
.elseif(n = 2557)
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
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
.elseif(n = 2581 || n = 2603 || n = 2614)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2582 || n = 2589 || n = 2594 || n = 2601 || n = 2606)
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
.elseif(n = 2586 || n = 2597)
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
.elseif(n = 2587 || n = 2599 || n = 2611)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
.elseif(n = 2590 || n = 2595 || n = 2600)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 2591)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2596)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2598)
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
.elseif(n = 2605)
	; 11 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 6
	iny
	Jne *-4
	ldy @zptemp
.elseif(n = 2607)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2617)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2637 || n = 2638 || n = 2642 || n = 2654 || n = 2656)
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
.elseif(n = 2623 || n = 2635 || n = 2647)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2624)
	; 14 bytes
	php
	pha
	sec
	lda #237
	delay_n 4
	nop
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2625 || n = 2636)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2626 || n = 2631 || n = 2643 || n = 2650 || n = 2655)
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
.elseif(n = 2639 || n = 2653)
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
.elseif((n .mod 11 = 7) && ((n > 2657 && n < 2714)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-3
	ldy @zptemp
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2732)))
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 2660 || n = 2661 || n = 2665 || n = 2666 || n = 2668 || n = 2673 || n = 2675 || n = 2678 || n = 2682 || n = 2690 || n = 2697 || n = 2699 || n = 2704 || n = 2720 || n = 2721 || n = 2725 || n = 2726 || n = 2730 || n = 2733)
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
.elseif(n = 2679 || n = 2694 || n = 2703 || n = 2716 || n = 2734)
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
.elseif(n = 2689 || n = 2701)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 2711)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
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
.elseif(n = 2738 || n = 2759 || n = 2769)
	; 12 bytes
	delay_n (n-2)
	nop
.elseif(n = 2739 || n = 2753 || n = 2758 || n = 2763 || n = 2771)
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
.elseif(n = 2744 || n = 2761)
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
.elseif(n = 2749 || n = 2766)
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
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 2764)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 2774 to 2812
.if(n = 2774 || n = 2781 || n = 2786 || n = 2788 || n = 2809)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2775 || n = 2780 || n = 2799 || n = 2804 || n = 2811)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2776 || n = 2802)
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
.elseif(n = 2785)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 2787)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
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
.elseif(n = 2814 || n = 2822 || n = 2825 || n = 2829 || n = 2831 || n = 2834 || n = 2836 || n = 2841 || n = 2846 || n = 2848)
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
.elseif(n = 2816 || n = 2830 || n = 2850)
	; 12 bytes
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
.elseif(n = 2819 || n = 2838)
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
	delay_n 6
	dey
	Jne *-3
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
.elseif(n = 2842)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2845)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 2847)
	; 11 bytes
	stx @zptemp
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
.elseif(n = 2855 || n = 2859 || n = 2866 || n = 2871 || n = 2878 || n = 2883 || n = 2888 || n = 2890)
	; 12..13 bytes
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
.elseif(n = 2857 || n = 2876)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2877)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2879 || n = 2884)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	tya
	ldy #(n-18)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	tay
	pla
.endif
.endif
.else
; 2891 to 2968
.if((n .mod 14 = 7) && ((n > 2890 && n < 2934) || n = 2961))
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
.elseif(n = 2894 || n = 2901 || n = 2903 || n = 2906 || n = 2908 || n = 2913 || n = 2915 || n = 2918 || n = 2920 || n = 2925 || n = 2930 || n = 2937 || n = 2939 || n = 2942 || n = 2944 || n = 2949 || n = 2951 || n = 2954 || n = 2956)
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
.elseif(n = 2896 || n = 2910 || n = 2924 || n = 2938 || n = 2966)
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
.elseif(n = 2907 || n = 2927 || n = 2967)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2926 || n = 2943 || n = 2950 || n = 2955)
	; 12 bytes
	pha
	delay_n (n-3)
.elseif(n = 2929 || n = 2941)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 2934 || n = 2946)
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.else
.if((n >= 2969) && (n < 3047))
; 2969 to 3046
.if(n = 2969 || n = 2981 || n = 2993 || n = 3029 || n = 3041)
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
.elseif(n = 2973 || n = 2978 || n = 2985 || n = 2990 || n = 2996 || n = 2997 || n = 3002 || n = 3009 || n = 3011 || n = 3014 || n = 3026 || n = 3033 || n = 3035 || n = 3038 || n = 3040 || n = 3044)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2974 || n = 2979 || n = 2986 || n = 2991 || n = 2998 || n = 3015 || n = 3034 || n = 3039 || n = 3046)
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
.elseif(n = 2977 || n = 3037)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
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
.elseif(n = 2982 || n = 2999 || n = 3016)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2984 || n = 3010 || n = 3023)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3001 || n = 3025)
	; 13 bytes
	sty @zptemp
	ldy #(n-13)/12
	delay_n 7
	dey
	Jne *-3
	sta (@ptrtemp),y
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
.elseif(n = 3006 || n = 3018)
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
	; 11 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 6
	iny
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3049)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3050)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-4
	ldx @zptemp
.elseif(n = 3051 || n = 3053 || n = 3058 || n = 3063 || n = 3070 || n = 3075 || n = 3082)
	; 12 bytes
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
.elseif(n = 3055 || n = 3067)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
.elseif(n = 3056)
	; 13 bytes
	delay_n 6
	delay_n (n-6)
.elseif(n = 3057 || n = 3061 || n = 3062 || n = 3069 || n = 3074 || n = 3076 || n = 3081 || n = 3083)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3066 || n = 3085)
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
.if(n = 3086 || n = 3094 || n = 3096 || n = 3103 || n = 3105 || n = 3108 || n = 3122 || n = 3124 || n = 3125)
	; 12..14 bytes
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
.elseif(n = 3088 || n = 3114)
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
.elseif(n = 3090 || n = 3109)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3091 || n = 3113)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3095 || n = 3111)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3097)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3098)
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
.elseif(n = 3099)
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
.elseif(n = 3104)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3107)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3110)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3118)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3121)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
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
; 3126 to 3203
.if(n = 3126 || n = 3138 || n = 3146 || n = 3151 || n = 3160 || n = 3165 || n = 3170)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3130 || n = 3131 || n = 3133 || n = 3136 || n = 3137 || n = 3144 || n = 3149 || n = 3150 || n = 3154 || n = 3155 || n = 3163 || n = 3168 || n = 3173 || n = 3178 || n = 3181 || n = 3182 || n = 3188 || n = 3193 || n = 3201)
	; 13..14 bytes
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
.elseif(n = 3135 || n = 3152 || n = 3169 || n = 3186 || n = 3203)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3139)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
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
.elseif(n = 3141 || n = 3183)
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
.elseif(n = 3142 || n = 3161 || n = 3180)
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
	delay_n 18
	Jne *-5
	ldy @zptemp
.elseif(n = 3159 || n = 3175)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3164 || n = 3196)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3195)
	; 13 bytes
	pha
	sty @zptemp
	ldy #245
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3198)
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
.elseif(n = 3206 || n = 3209 || n = 3214 || n = 3215 || n = 3229 || n = 3233 || n = 3234 || n = 3238)
	; 13..14 bytes
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
.elseif(n = 3211)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
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
.elseif(n = 3216 || n = 3221 || n = 3226 || n = 3230 || n = 3235 || n = 3240)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3217)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
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
.elseif(n = 3231)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/13
	nop
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3236)
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
.if(n = 3243 || n = 3248 || n = 3273 || n = 3278 || n = 3279)
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
	delay_n (n-3)
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
.elseif(n = 3247 || n = 3267)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3249)
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
.elseif(n = 3250)
	; 13 bytes
	pha
	sty @zptemp
	ldy #(n-10)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3251)
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
.elseif(n = 3252 || n = 3272)
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
.elseif(n = 3256 || n = 3275)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3257 || n = 3270)
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
.elseif(n = 3265)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
; 3282 to 3359
.if(n = 3282 || n = 3342 || n = 3357)
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
.elseif((n .mod 14 = 7) && ((n > 3282 && n < 3354)))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3284 || n = 3289 || n = 3300 || n = 3321 || n = 3328 || n = 3333 || n = 3338 || n = 3350 || n = 3359)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3285 || n = 3290 || n = 3295 || n = 3304 || n = 3306 || n = 3315 || n = 3320 || n = 3326 || n = 3329 || n = 3341 || n = 3343 || n = 3346 || n = 3348 || n = 3349 || n = 3354 || n = 3355)
	; 13..14 bytes
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
.elseif(n = 3287 || n = 3307 || n = 3347)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 14 = 12) && ((n > 3287 && n < 3359)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3292 || n = 3312 || n = 3332 || n = 3352)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3293 || n = 3310 || n = 3327)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3294 || n = 3313 || n = 3351)
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
.elseif(n = 3299 || n = 3318 || n = 3337)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3303 || n = 3319)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3305 || n = 3322 || n = 3356)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3308 || n = 3324)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3317 || n = 3331 || n = 3345)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3323)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
	tya
	ldy #(n-18)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.endif
.else
; 3360 to 3437
.if(n = 3360 || n = 3362 || n = 3363 || n = 3369 || n = 3371 || n = 3374 || n = 3377 || n = 3380 || n = 3384 || n = 3391 || n = 3396 || n = 3401 || n = 3411 || n = 3416 || n = 3418 || n = 3425 || n = 3426 || n = 3430)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3361 || n = 3378 || n = 3412 || n = 3429)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3364 || n = 3376 || n = 3385 || n = 3398 || n = 3403)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3365)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
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
.elseif((n .mod 14 = 7) && ((n > 3366 && n < 3438)))
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3368 || n = 3379 || n = 3421)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3370 || n = 3389 || n = 3408)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3375 || n = 3394 || n = 3413 || n = 3432)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3382 || n = 3397)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 16 = 7) && ((n > 3382 && n < 3432)))
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3387 || n = 3427)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3388 || n = 3404 || n = 3420 || n = 3436)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3393 || n = 3435)
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
.elseif(n = 3419 || n = 3433)
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
.elseif(n = 3434)
	; 13 bytes
	pha
	tya
	ldy #171
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
	nop
.endif
.endif
.endif
.else
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
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
.elseif(n = 3439 || n = 3443 || n = 3444 || n = 3448 || n = 3449 || n = 3453 || n = 3454 || n = 3460 || n = 3469 || n = 3474 || n = 3481 || n = 3482 || n = 3486 || n = 3491 || n = 3494 || n = 3499 || n = 3504 || n = 3505 || n = 3513)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3440 || n = 3445 || n = 3450 || n = 3455 || n = 3459 || n = 3464 || n = 3473 || n = 3478 || n = 3496 || n = 3501 || n = 3506)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3441 || n = 3458 || n = 3475 || n = 3492 || n = 3509)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3442 || n = 3470 || n = 3484 || n = 3498 || n = 3512)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3446 || n = 3463 || n = 3480 || n = 3497 || n = 3514)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3447 || n = 3467 || n = 3487)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif((n .mod 14 = 7) && ((n > 3450 && n < 3508)))
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
.elseif(n = 3457 || n = 3502)
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
.elseif(n = 3462 || n = 3477)
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
	; 13 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3468 || n = 3500)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3476)
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
.elseif(n = 3483)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3485)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	nop
	delay_n 6
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3488 || n = 3510)
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
.elseif(n = 3495 || n = 3511)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
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
.elseif(n = 3517 || n = 3562)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3518 || n = 3525 || n = 3530 || n = 3538 || n = 3544 || n = 3555 || n = 3557 || n = 3566 || n = 3585 || n = 3590)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3519 || n = 3561)
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
.elseif(n = 3520)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	.repeat 4
	 nop
	.endrepeat
	dey
	Jne *-6
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
.elseif(n = 3523 || n = 3524 || n = 3528 || n = 3529 || n = 3533 || n = 3534 || n = 3539 || n = 3542 || n = 3545 || n = 3550 || n = 3551 || n = 3556 || n = 3558 || n = 3569 || n = 3570 || n = 3574 || n = 3578 || n = 3581 || n = 3586 || n = 3589 || n = 3593)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3553)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3559 || n = 3575)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3571)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3573)
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
.elseif(n = 3576)
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
.elseif(n = 3583)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/14
	delay_n 9
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.if(n = 3594 || n = 3611 || n = 3628)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3598 || n = 3617)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3600 || n = 3601 || n = 3609 || n = 3613 || n = 3614 || n = 3618 || n = 3629 || n = 3630)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3602 || n = 3606 || n = 3631)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3603 || n = 3622)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 3605)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 11
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3607 || n = 3627)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3608)
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
.elseif(n = 3612 || n = 3632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3615)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3626)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
; 3633 to 3671
.if(n = 3633 || n = 3650 || n = 3667)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3634 || n = 3635 || n = 3638 || n = 3643 || n = 3646 || n = 3649 || n = 3654 || n = 3669)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3639 || n = 3671)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3640 || n = 3661)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 3645 || n = 3662)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3647)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3648 || n = 3653 || n = 3658 || n = 3663 || n = 3665)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3651 || n = 3668)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 3656)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3659)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-5
	ldx @zptemp
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
.elseif(n = 3666)
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
.elseif(n = 3670)
	; 13 bytes
	pha
	sty @zptemp
	ldy #(n-10)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3673 || n = 3678 || n = 3681 || n = 3683 || n = 3686 || n = 3688 || n = 3689 || n = 3694 || n = 3695 || n = 3699 || n = 3700 || n = 3705)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3674 || n = 3693)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3675 || n = 3685 || n = 3690)
	; 13 bytes
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
.elseif(n = 3677)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 3679 || n = 3696)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3680)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3682 || n = 3697)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/15
	delay_n 10
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3684)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3691)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3702)
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
.elseif(n = 3703)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3704)
	; 13 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_15cyc
	Jne *-6
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
.elseif(n = 3710)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 3711 to 3750
.if(n = 3711)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
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
.elseif(n = 3714 || n = 3715 || n = 3720 || n = 3726 || n = 3733 || n = 3734 || n = 3737 || n = 3738 || n = 3744 || n = 3746)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3716 || n = 3721 || n = 3739)
	; 13 bytes
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
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 3725)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3727)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3741)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3745)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3748)
	; 14 bytes
	delay_n 6
	delay_n (n-6)
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
.if(n = 3751 || n = 3783)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3752 || n = 3769 || n = 3786)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3753 || n = 3758 || n = 3760 || n = 3770 || n = 3784)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3755 || n = 3774)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3759 || n = 3766 || n = 3771 || n = 3776 || n = 3785 || n = 3789)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3763)
	; 14 bytes
	pha
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	pla
	pha
	iny
	Jne *-5
	ldy @zptemp
	pla
.elseif(n = 3764 || n = 3781)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3765)
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
.elseif(n = 3767 || n = 3787)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3773)
	; 14 bytes
	pha
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	lda $4868 ;hides 'pla' and 'pha'
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3778)
	; 14 bytes
	pha
	sty @zptemp
	ldy #5 ;hides 'ora zp'
	nop
	pla
	pha
	iny
	Jne *-5
	ldy @zptemp
	pla
.elseif(n = 3779)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3782)
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
.if(n = 3790 || n = 3794 || n = 3795 || n = 3800 || n = 3809 || n = 3814 || n = 3819 || n = 3824 || n = 3828)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3791)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3792 || n = 3812)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
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
.elseif(n = 3801 || n = 3806)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3805)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
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
.elseif(n = 3811)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3813)
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
.elseif(n = 3816)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3821)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3825)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
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
.else
; 3829 to 3906
.if(n = 3829 || n = 3833 || n = 3834 || n = 3838 || n = 3839 || n = 3851 || n = 3856 || n = 3858 || n = 3873 || n = 3881 || n = 3885 || n = 3889 || n = 3890 || n = 3894 || n = 3896 || n = 3904)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3830 || n = 3853 || n = 3865 || n = 3870 || n = 3875 || n = 3880 || n = 3886 || n = 3891 || n = 3903)
	; 12..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 3831 || n = 3850 || n = 3869)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3832 || n = 3849 || n = 3866 || n = 3883 || n = 3900)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3835 || n = 3857 || n = 3901)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3836 || n = 3868 || n = 3884)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3837 || n = 3854 || n = 3871 || n = 3888 || n = 3905)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3840 || n = 3906)
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
.elseif(n = 3841 || n = 3859 || n = 3877)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3842 || n = 3893)
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
.elseif(n = 3843 || n = 3860)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 3844 || n = 3861 || n = 3874 || n = 3878 || n = 3899)
	; 13..14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3845 || n = 3864 || n = 3902)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3846 || n = 3882)
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
.elseif(n = 3847 || n = 3887)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3848)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3852 || n = 3872 || n = 3892)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3855 || n = 3876 || n = 3897)
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
.elseif(n = 3862)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 3863 || n = 3879 || n = 3895)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3867)
	; 11 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_14cyc
	Jne *-4
	tax
	pla
.elseif(n = 3898)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.endif
.endif
.else
.if((n >= 3907) && (n < 3985))
; 3907 to 3984
.if(n = 3907 || n = 3926 || n = 3945 || n = 3964 || n = 3983)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3908 || n = 3910 || n = 3915 || n = 3920 || n = 3925 || n = 3930 || n = 3935 || n = 3937 || n = 3962 || n = 3971)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 3909 || n = 3914 || n = 3919 || n = 3923 || n = 3924 || n = 3928 || n = 3929 || n = 3936 || n = 3938 || n = 3941 || n = 3942 || n = 3953 || n = 3958 || n = 3961 || n = 3966 || n = 3969 || n = 3970 || n = 3974 || n = 3977 || n = 3982)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3911 || n = 3943 || n = 3975)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3912 || n = 3932 || n = 3952 || n = 3972)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3913 || n = 3955 || n = 3976)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3916 || n = 3948 || n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3917 || n = 3934 || n = 3951 || n = 3968)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3918 || n = 3960 || n = 3981)
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
.elseif(n = 3921 || n = 3940 || n = 3959 || n = 3978)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3922 || n = 3939 || n = 3956 || n = 3973)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3927 || n = 3947 || n = 3967)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3931)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3933 || n = 3946)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3944)
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
.elseif(n = 3949)
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 10
	iny
	Jne *-5
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
.elseif(n = 3957)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3963)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3965 || n = 3984)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 3979)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 3985 to 4062
.if(n = 3985 || n = 4002 || n = 4019 || n = 4036 || n = 4053)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
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
.elseif(n = 3989 || n = 4011 || n = 4033)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3990 || n = 4007 || n = 4024 || n = 4041 || n = 4058)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3991 || n = 4023 || n = 4039 || n = 4055)
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
.elseif(n = 3993 || n = 3995 || n = 4000 || n = 4010 || n = 4015 || n = 4022 || n = 4030 || n = 4050 || n = 4062)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 3996 || n = 4028 || n = 4044 || n = 4060)
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
.elseif(n = 3998 || n = 3999 || n = 4001 || n = 4004 || n = 4006 || n = 4009 || n = 4014 || n = 4025 || n = 4026 || n = 4029 || n = 4034 || n = 4037 || n = 4038 || n = 4042 || n = 4043 || n = 4045 || n = 4046 || n = 4049 || n = 4051 || n = 4056 || n = 4061)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4003 || n = 4057)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4005)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 13
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4013)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4017)
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
.elseif(n = 4018)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4021 || n = 4040 || n = 4059)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4031 || n = 4048)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4064)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4065)
	; 11 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	jsr @nearby_rts_15cyc
	Jne *-4
	ldx @zptemp
.elseif(n = 4066 || n = 4085)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4067)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4068 || n = 4090 || n = 4095 || n = 4100)
	; 12..13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4069 || n = 4074 || n = 4077 || n = 4079 || n = 4080 || n = 4084 || n = 4088 || n = 4089 || n = 4091 || n = 4094 || n = 4096 || n = 4099)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 4082)
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
.elseif(n = 4083)
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
.elseif(n = 4093)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4098)
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
.elseif(n = 4101)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 4102 to 4140
.if(n = 4102)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 4105 || n = 4106 || n = 4113 || n = 4114 || n = 4118 || n = 4120 || n = 4123 || n = 4125 || n = 4128 || n = 4129 || n = 4134 || n = 4136 || n = 4137 || n = 4140)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4110 || n = 4115 || n = 4119 || n = 4124 || n = 4133)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4111 || n = 4130)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
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
.elseif(n = 4116 || n = 4135)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4117)
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
.elseif(n = 4122)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	inx
	inx
	Jne *-6
	ldx @zptemp
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
.elseif(n = 4139)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4146 || n = 4150 || n = 4158 || n = 4171 || n = 4176)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4142 || n = 4145 || n = 4151 || n = 4153 || n = 4156 || n = 4157 || n = 4159 || n = 4162 || n = 4164 || n = 4169 || n = 4174 || n = 4175 || n = 4179)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4144 || n = 4165)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4147 || n = 4167)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4148 || n = 4170)
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
.elseif(n = 4152)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 4161 || n = 4178)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.elseif(n = 4166)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.endif
.else
; 4180 to 4218
.if(n = 4180 || n = 4197 || n = 4204 || n = 4215)
	; 13..14 bytes
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
.elseif(n = 4182 || n = 4199)
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
.elseif(n = 4183 || n = 4201)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
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
.elseif(n = 4185 || n = 4191 || n = 4196 || n = 4198 || n = 4203 || n = 4208 || n = 4213)
	; 13..14 bytes
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
.elseif(n = 4187 || n = 4207)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4188)
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
.elseif(n = 4189 || n = 4206)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4190)
	; 13 bytes
	pha
	sty @zptemp
	ldy #220
	jsr @nearby_rts_14cyc
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
.elseif(n = 4193 || n = 4216)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
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
	delay_n (n-4189)
	tya
	ldy #209
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4200)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
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
.elseif(n = 4205)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
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
	php
	delay_n (n-7)
	plp
.endif
.endif
.endif
.else
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if(n = 4219 || n = 4237 || n = 4255)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4220 || n = 4226 || n = 4235 || n = 4243 || n = 4248 || n = 4250 || n = 4260 || n = 4266 || n = 4271 || n = 4277 || n = 4285 || n = 4290 || n = 4295)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4221)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4223 || n = 4240 || n = 4257 || n = 4274 || n = 4291)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4224 || n = 4278)
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
.elseif(n = 4228 || n = 4245 || n = 4262 || n = 4279 || n = 4296)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4229 || n = 4234 || n = 4238 || n = 4241 || n = 4242 || n = 4246 || n = 4251 || n = 4254 || n = 4259 || n = 4261 || n = 4264 || n = 4265 || n = 4269 || n = 4270 || n = 4276 || n = 4281 || n = 4283 || n = 4284 || n = 4289 || n = 4293 || n = 4294)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4233 || n = 4275)
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
.elseif(n = 4236 || n = 4258 || n = 4280)
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
.elseif(n = 4239)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4256 || n = 4286)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 4288)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 4298 || n = 4300 || n = 4303 || n = 4309 || n = 4310 || n = 4314 || n = 4315 || n = 4321 || n = 4322 || n = 4326 || n = 4329 || n = 4334 || n = 4340 || n = 4343 || n = 4346 || n = 4349 || n = 4351 || n = 4354 || n = 4360 || n = 4361 || n = 4365 || n = 4366 || n = 4369 || n = 4374)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4304 || n = 4311 || n = 4316 || n = 4323 || n = 4328 || n = 4335 || n = 4350 || n = 4355 || n = 4362 || n = 4370)
	; 13 bytes
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
.elseif(n = 4302 || n = 4324 || n = 4368)
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
.elseif(n = 4306 || n = 4344 || n = 4363)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4307 || n = 4327 || n = 4367)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4308 || n = 4325 || n = 4342)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4312 || n = 4332 || n = 4352 || n = 4372)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4313 || n = 4330 || n = 4347)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4317 || n = 4338)
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
.elseif(n = 4318)
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
.elseif(n = 4331)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4333 || n = 4375)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4336)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4337 || n = 4371)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4345)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4348)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4353)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/17
	jsr @nearby_rts
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
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
.elseif(n = 4357)
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
.elseif(n = 4359)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 4373)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.if(n = 4376 || n = 4379 || n = 4384 || n = 4389 || n = 4391 || n = 4394 || n = 4398 || n = 4403 || n = 4409 || n = 4411 || n = 4414)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4377 || n = 4396)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4378)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
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
.elseif(n = 4383 || n = 4402)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4385)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4386)
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
.elseif(n = 4387 || n = 4407)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4388)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
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
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4395 || n = 4404 || n = 4408)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4397)
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
.elseif(n = 4400)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4405)
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 4406)
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
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4434 || n = 4453)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4416 || n = 4421 || n = 4423 || n = 4430 || n = 4437 || n = 4442 || n = 4450)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4417 || n = 4438)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 4419)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 4422 || n = 4443)
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
.elseif(n = 4424)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4425)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4426)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4427 || n = 4447)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4428)
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
.elseif(n = 4429)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4431)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
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
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4435)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4436 || n = 4441 || n = 4445 || n = 4448 || n = 4449)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4440)
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
.elseif(n = 4444)
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
.elseif(n = 4446)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
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
; 4454 to 4531
.if(n = 4454 || n = 4455 || n = 4460 || n = 4466 || n = 4469 || n = 4474 || n = 4479 || n = 4481 || n = 4489 || n = 4493 || n = 4494 || n = 4498 || n = 4502 || n = 4508 || n = 4509 || n = 4511 || n = 4514 || n = 4516 || n = 4517 || n = 4531)
	; 13..14 bytes
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
.elseif(n = 4457)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4458 || n = 4477 || n = 4496 || n = 4515)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4459 || n = 4480 || n = 4501)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4461 || n = 4470 || n = 4475 || n = 4488 || n = 4490 || n = 4499 || n = 4504 || n = 4513 || n = 4518 || n = 4528 || n = 4530)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4462)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4463 || n = 4465 || n = 4483 || n = 4484 || n = 4503)
	; 14 bytes
	php
	delay_n (n-7)
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
.elseif(n = 4467 || n = 4487 || n = 4507 || n = 4527)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4468 || n = 4486 || n = 4523)
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
.elseif(n = 4471)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	nop
	jsr @nearby_rts_15cyc
	Jne *-5
	ldx @zptemp
.elseif(n = 4472 || n = 4492 || n = 4512)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4473 || n = 4495)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4478 || n = 4500 || n = 4522)
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
.elseif(n = 4482 || n = 4520)
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
.elseif(n = 4491 || n = 4510 || n = 4529)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4497)
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
.elseif(n = 4505)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4519)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4521)
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
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4526)
	; 14 bytes
	sty @zptemp
	ldy #136 ;hides 'dey'
	delay_n 28
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if(n = 4532 || n = 4552 || n = 4592)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4533 || n = 4536 || n = 4545 || n = 4546 || n = 4549 || n = 4550 || n = 4554 || n = 4555 || n = 4563 || n = 4568 || n = 4571 || n = 4574 || n = 4576 || n = 4581 || n = 4589 || n = 4593 || n = 4594 || n = 4609)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4534 || n = 4553 || n = 4572 || n = 4591)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4535 || n = 4537 || n = 4542 || n = 4551 || n = 4556 || n = 4570 || n = 4575 || n = 4595 || n = 4600 || n = 4608)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4538 || n = 4584)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4539 || n = 4561 || n = 4583)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4540 || n = 4578)
	; 14 bytes
	pha
	txa
	ldx #(n-18)/19
	jsr @nearby_rts_14cyc
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4541 || n = 4559 || n = 4560 || n = 4598 || n = 4599)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4543 || n = 4564 || n = 4585 || n = 4606)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4544 || n = 4566 || n = 4588)
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
.elseif(n = 4547 || n = 4587 || n = 4607)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4548 || n = 4567 || n = 4586 || n = 4605)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4557 || n = 4577)
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
.elseif(n = 4562)
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
.elseif(n = 4565 || n = 4580 || n = 4601 || n = 4603 || n = 4604)
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
.elseif(n = 4569 || n = 4590)
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
.elseif(n = 4573)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4579 || n = 4597)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-5
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
.elseif(n = 4596)
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
.elseif(n = 4602)
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
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4629 || n = 4648)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4611)
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
.elseif(n = 4612 || n = 4632)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4613 || n = 4635 || n = 4646)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4614 || n = 4616 || n = 4626 || n = 4628 || n = 4631 || n = 4633 || n = 4634 || n = 4645)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4615)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 13
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4617 || n = 4637)
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
.elseif(n = 4618)
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
.elseif(n = 4619 || n = 4636)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif((n > 4620 && n < 4624) || n = 4641 || n = 4642)
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
.elseif(n = 4624 || n = 4643)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4625)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	ldx @zptemp
.elseif(n = 4627 || n = 4647)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4630)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4638)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
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
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4671)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4650 || n = 4664 || n = 4669 || n = 4673 || n = 4678 || n = 4683)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4651 || n = 4655 || n = 4665 || n = 4670 || n = 4675 || n = 4677 || n = 4682 || n = 4684)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4652 || n = 4672)
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
.elseif(n = 4654 || n = 4676)
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
.elseif(n = 4656)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4657)
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
.elseif(n = 4658)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4659)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4660)
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
.elseif(n = 4661)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4662 || n = 4681)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4663)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
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
	pha
	tya
	ldy #(n-12)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4674)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-5
	ldx @zptemp
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
	delay_n 6
	delay_n (n-6)
.elseif(n = 4685)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 13
	dey
	Jne *-6
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
.if(n = 4688 || n = 4689 || n = 4694 || n = 4696 || n = 4702 || n = 4709 || n = 4714 || n = 4721 || n = 4723 || n = 4726 || n = 4728 || n = 4729 || n = 4734 || n = 4736 || n = 4740 || n = 4744 || n = 4745 || n = 4749 || n = 4751 || n = 4754 || n = 4756 || n = 4759 || n = 4764)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4690 || n = 4711 || n = 4753)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4691 || n = 4748)
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
.elseif(n = 4693 || n = 4715)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4697 || n = 4717)
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
.elseif(n = 4699 || n = 4722)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
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
.elseif(n = 4701 || n = 4703 || n = 4708 || n = 4710 || n = 4730 || n = 4735 || n = 4741 || n = 4746 || n = 4750 || n = 4755 || n = 4760 || n = 4765)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4704)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 4706 || n = 4725 || n = 4763)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4707 || n = 4727 || n = 4747)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4713)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4718)
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
.elseif(n = 4731 || n = 4739)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4733)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4761)
	; 14 bytes
	sty @zptemp
	ldy #216
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
	nop
.endif
.else
.if((n >= 4766) && (n < 4805))
; 4766 to 4804
.if(n = 4766)
	; 14 bytes
	pha
	tya
	ldy #216
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	tay
	pla
	nop
.elseif(n = 4767 || n = 4787)
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
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4769 || n = 4771 || n = 4774 || n = 4778 || n = 4780 || n = 4783 || n = 4785 || n = 4789 || n = 4794 || n = 4796 || n = 4797 || n = 4802 || n = 4804)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4770 || n = 4775 || n = 4784 || n = 4790 || n = 4798)
	; 13 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 4773)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
	php
	pha
	sec
	lda #(n-17)/20
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
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
.elseif(n = 4782 || n = 4801)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 4793)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4799)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
; 4805 to 4843
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
	; 14 bytes
	pha
	tya
	ldy #(n-18)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 4807 || n = 4827)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4808 || n = 4830)
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
.elseif(n = 4809 || n = 4811 || n = 4829 || n = 4834 || n = 4835 || n = 4840)
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
.elseif(n = 4812 || n = 4832)
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
.elseif(n = 4814 || n = 4833)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4815 || n = 4817 || n = 4822 || n = 4836 || n = 4841)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4816 || n = 4837)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4819 || n = 4838)
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
	sty @zptemp
	ldy #(n-13)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4821 || n = 4842)
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
.elseif(n = 4823)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4824 || n = 4843)
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
.elseif(n = 4828)
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4831)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4849 || n = 4851 || n = 4854 || n = 4859 || n = 4873 || n = 4878)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4845 || n = 4864)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4846 || n = 4868)
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
.elseif(n = 4847 || n = 4867)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4848)
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
.elseif(n = 4850 || n = 4855 || n = 4860 || n = 4866 || n = 4870 || n = 4875 || n = 4880)
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
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.elseif(n = 4858 || n = 4879)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.else
; 4883 to 4921
.if(n = 4883 || n = 4906)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4884 || n = 4905)
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
.elseif(n = 4885 || n = 4890 || n = 4895 || n = 4910 || n = 4915)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 4886 || n = 4889 || n = 4893 || n = 4894 || n = 4898 || n = 4902 || n = 4909 || n = 4914 || n = 4916 || n = 4920)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4887 || n = 4907)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4888 || n = 4911)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4891 || n = 4913)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4892 || n = 4912)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4896 || n = 4918)
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
.elseif(n = 4897 || n = 4917)
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
.elseif(n = 4899)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4900 || n = 4921)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4901 || n = 4904)
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
.elseif(n = 4903)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4919)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922 || n = 4930 || n = 4943 || n = 4945 || n = 4950 || n = 4955 || n = 4966 || n = 4970 || n = 4975 || n = 4990 || n = 4995)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 4923 || n = 4928 || n = 4929 || n = 4934 || n = 4936 || n = 4944 || n = 4949 || n = 4954 || n = 4964 || n = 4965 || n = 4969 || n = 4971 || n = 4974 || n = 4976 || n = 4981 || n = 4986 || n = 4991 || n = 4994 || n = 4996 || n = 5000)
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
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4926 || n = 4968 || n = 4989)
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
.elseif(n = 4927 || n = 4947 || n = 4967 || n = 4987)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4931 || n = 4939 || n = 4959)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4932 || n = 4952 || n = 4972 || n = 4992)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4933 || n = 4993)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 4935 || n = 4957 || n = 4979)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4937 || n = 4977 || n = 4997)
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
.elseif(n = 4938 || n = 4958)
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
.elseif(n = 4940 || n = 4962 || n = 4984)
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
.elseif(n = 4941 || n = 4946 || n = 4961 || n = 4983 || n = 4988)
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
.elseif(n = 4942 || n = 4963)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 4951 || n = 4999)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4956 || n = 4980)
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
.elseif(n = 4960)
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4973)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4978)
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
.elseif(n = 4982)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4998)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5004 || n = 5076)
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
.elseif(n = 5005 || n = 5026 || n = 5068)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
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
.elseif(n = 5008 || n = 5009 || n = 5011 || n = 5014 || n = 5016 || n = 5029 || n = 5033 || n = 5034 || n = 5036 || n = 5046 || n = 5049 || n = 5051 || n = 5054 || n = 5056 || n = 5069 || n = 5074)
	; 13..14 bytes
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
.elseif(n = 5013 || n = 5053)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
.elseif(n = 5015 || n = 5024 || n = 5030 || n = 5035 || n = 5055 || n = 5070 || n = 5075)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5017 || n = 5037 || n = 5057 || n = 5077)
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
.elseif(n = 5018 || n = 5038 || n = 5058)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5019 || n = 5039 || n = 5059)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5020 || n = 5022 || (n > 5039 && n < 5044) || n = 5060 || (n > 5062 && n < 5067))
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
.elseif(n = 5021 || n = 5044)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5025)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5048)
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5071)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5078)
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
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if(n = 5079 || n = 5099)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5080)
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 6
	jsr @nearby_rts_14cyc
	Jne *-6
	ldx @zptemp
.elseif(n = 5081 || n = 5084 || n = 5086 || n = 5101 || n = 5102 || (n > 5103 && n < 5107))
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
.elseif(n = 5082 || n = 5108)
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
.elseif(n = 5083)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5085)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	nop
	jsr @nearby_rts_14cyc
	Jne *-7
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
.elseif(n = 5088)
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
.elseif(n = 5089 || n = 5110)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5090 || n = 5113)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5091)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5093)
	; 14 bytes
	sty @zptemp
	ldy #(n-13)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
	ldy @zptemp
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
.elseif(n = 5095 || n = 5117)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5096)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 5098)
	; 14 bytes
	pha
	tya
	ldy #(n-18)/20
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	sta (@ptrtemp),y
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
.elseif(n = 5103)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
; 5118 to 5156
.if(n = 5118 || n = 5129 || n = 5134 || n = 5140 || n = 5154)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5119 || n = 5143)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5120)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_15cyc
	iny
	Jne *-5
	sta (@ptrtemp),y
	tay
	pla
.elseif(n = 5121 || n = 5139)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5122 || n = 5123 || n = 5125 || n = 5126 || n = 5128 || (n > 5143 && n < 5148) || (n > 5148 && n < 5152))
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
.elseif(n = 5124 || n = 5148)
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
.elseif(n = 5127)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts_15cyc
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 5130 || n = 5135 || n = 5156)
	; 13 bytes
	pha
	delay_n (n-3)
.elseif(n = 5131 || n = 5152)
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
.elseif(n = 5133 || n = 5155)
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
	; 14 bytes
	php
	pha
	sec
	lda #0
	jsr @nearby_rts_15cyc
	sbc #1
	Jne *-5
	pla
	plp
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 5153)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5178 || n = 5199 || n = 5220)
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
.elseif(n = 5159 || n = 5205 || n = 5228)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5160 || n = 5182 || n = 5204 || n = 5226)
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
.elseif(n = 5161 || n = 5176 || n = 5185 || n = 5197 || n = 5202 || n = 5208 || n = 5224 || n = 5229 || n = 5231)
	; 13..14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5162 || n = 5175 || n = 5179 || n = 5180 || n = 5184 || n = 5201 || n = 5206 || n = 5217 || n = 5222 || n = 5230)
	; 14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5164 || n = 5187 || n = 5233)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5165)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 5166 || n = 5168 || n = 5170 || n = 5171 || n = 5174 || n = 5183 || (n > 5187 && n < 5191) || n = 5192 || n = 5193 || n = 5195 || n = 5198 || n = 5200 || n = 5203 || n = 5209 || n = 5211 || n = 5213 || n = 5214 || n = 5216 || n = 5219 || n = 5227 || n = 5234)
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
.elseif(n = 5167 || n = 5191)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5172 || n = 5196)
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
.elseif(n = 5173 || n = 5194 || n = 5215)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5177 || n = 5221)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5181 || n = 5207)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5186 || n = 5212)
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
.elseif(n = 5218)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
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
.elseif(n = 5225)
	; 14 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5232)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 5235 to 5312
.if(n = 5235 || n = 5240 || n = 5242 || n = 5247 || n = 5249 || n = 5252 || n = 5255 || n = 5258 || n = 5261 || n = 5266 || n = 5269 || n = 5271 || n = 5275 || n = 5284 || n = 5293 || n = 5296 || n = 5298 || n = 5303 || n = 5305 || n = 5308 || n = 5310)
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
.elseif(n = 5236 || n = 5257 || n = 5278 || n = 5299)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5237 || n = 5312)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5238 || n = 5264 || n = 5290)
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
.elseif(n = 5239 || n = 5263 || n = 5311)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
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
.elseif(n = 5244 || n = 5268)
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
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5246)
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
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
.elseif(n = 5250 || n = 5277)
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
.elseif(n = 5251 || n = 5274 || n = 5297)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5253 || n = 5267 || n = 5272 || n = 5276 || n = 5280 || n = 5285 || n = 5289 || n = 5294 || n = 5301 || n = 5306)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5254 || n = 5260 || n = 5273 || n = 5281 || n = 5286 || n = 5291 || n = 5295 || n = 5300 || n = 5307)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5256 || n = 5279 || n = 5302)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5259)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5282)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
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
.endif
.endif
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
; 5313 to 5390
.if(n = 5313 || n = 5315 || n = 5319 || n = 5324 || n = 5329 || n = 5330 || n = 5347 || (n > 5349 && n < 5353) || n = 5354 || (n > 5371 && n < 5375) || n = 5376 || n = 5379 || n = 5381 || n = 5384)
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
.elseif(n = 5314 || n = 5336 || n = 5358 || n = 5380)
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
.elseif(n = 5316 || n = 5340 || n = 5364)
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
.elseif(n = 5317 || n = 5321 || n = 5328 || n = 5339 || n = 5344 || n = 5349 || n = 5356 || n = 5361 || n = 5365 || n = 5370 || n = 5386)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5318)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_14cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 5320 || n = 5341 || n = 5362)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5322 || n = 5327 || n = 5333 || n = 5338 || n = 5345 || n = 5355 || n = 5360 || n = 5369 || n = 5377 || n = 5382 || n = 5390)
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
.elseif(n = 5325 || n = 5346 || n = 5367)
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
.elseif(n = 5331 || n = 5353 || n = 5375)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5332)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5334 || n = 5378)
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5335 || n = 5359)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5337 || n = 5363)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5342 || n = 5368)
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
.elseif(n = 5343 || n = 5366 || n = 5389)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5348 || n = 5371)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5357)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.endif
.else
; 5391 to 5468
.if(n = 5391 || n = 5395 || n = 5400 || n = 5405 || n = 5427 || n = 5438 || n = 5444 || n = 5449)
	; 14 bytes
	pha
	delay_n (n-3)
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
.elseif(n = 5393 || n = 5396 || n = 5398 || n = 5401 || n = 5403 || n = 5406 || n = 5408 || n = 5409 || n = 5411 || n = 5413 || n = 5416 || n = 5418 || n = 5423 || n = 5425 || (n > 5427 && n < 5431) || n = 5433 || n = 5442 || n = 5445 || n = 5447 || (n > 5449 && n < 5455) || n = 5456 || n = 5459 || n = 5464)
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
.elseif(n = 5394 || n = 5417 || n = 5440)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5397 || n = 5419 || n = 5441 || n = 5463)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5399 || n = 5404 || n = 5414 || n = 5421 || n = 5426 || n = 5443 || n = 5448 || n = 5465)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5402 || n = 5424 || n = 5446 || n = 5468)
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
.elseif(n = 5407 || n = 5431 || n = 5455)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5410)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 20
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5412 || n = 5435 || n = 5458)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5415 || n = 5467)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
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
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5432)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5434 || n = 5461)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5437 || n = 5462)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5439 || n = 5466)
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
.elseif(n = 5457)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 5469) && (n < 5547))
; 5469 to 5546
.if(n = 5469 || (n > 5473 && n < 5479) || n = 5480 || (n > 5494 && n < 5498) || (n > 5498 && n < 5503) || n = 5505 || n = 5511 || n = 5513 || (n > 5515 && n < 5519) || (n > 5520 && n < 5524) || n = 5525 || n = 5526 || n = 5528 || n = 5533 || n = 5535 || (n > 5537 && n < 5542) || n = 5543 || n = 5544 || n = 5546)
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
.elseif(n = 5470 || n = 5483 || n = 5487 || n = 5492 || n = 5506 || n = 5514 || n = 5531 || n = 5536)
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
.elseif(n = 5472 || n = 5498 || n = 5524)
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
.elseif(n = 5479 || n = 5503)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5481 || n = 5504 || n = 5527)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5482)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5484 || n = 5508 || n = 5532)
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
.elseif(n = 5485 || n = 5507 || n = 5529)
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
.elseif(n = 5488 || n = 5515 || n = 5542)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5489 || n = 5494 || n = 5510 || n = 5530)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5490 || n = 5512 || n = 5534)
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
.elseif(n = 5493 || n = 5520)
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
.elseif(n = 5509)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 5537)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5547 to 5625
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
.elseif(n = 5548 || n = 5560 || n = 5561 || (n > 5562 && n < 5569) || n = 5570 || n = 5572 || n = 5577 || n = 5579 || (n > 5582 && n < 5587) || (n > 5587 && n < 5595) || n = 5606 || n = 5608 || n = 5609 || n = 5611 || (n > 5612 && n < 5617) || n = 5618)
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
.elseif(n = 5549)
	; 13 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-6
	ldx @zptemp
.elseif(n = 5550 || n = 5596 || n = 5619)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5551 || n = 5573 || n = 5595 || n = 5617)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/22
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5552 || n = 5553 || n = 5558 || n = 5598 || n = 5602 || n = 5624)
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
.elseif(n = 5555 || n = 5601)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5556 || n = 5578 || n = 5600 || n = 5622)
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
.elseif(n = 5557 || n = 5559 || n = 5581 || n = 5603 || n = 5625)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5562 || n = 5587 || n = 5612)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
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
.elseif(n = 5571 || n = 5597)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5575 || n = 5599 || n = 5623)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5576)
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
.elseif(n = 5582)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5605)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5607)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 5620)
	; 14 bytes
	pha
	sty @zptemp
	ldy #(n-10)/22
	pha
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	ldy @zptemp
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
.endif
.else
.if((n >= 5626) && (n < 5938))
; 5626 to 5937
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
.elseif(n = 5627 || n = 5630 || n = 5631 || (n > 5632 && n < 5637) || n = 5638 || n = 5640 || n = 5643 || n = 5648 || n = 5651 || n = 5653 || n = 5656 || (n > 5657 && n < 5662) || n = 5663 || n = 5664 || n = 5666 || n = 5669 || (n > 5671 && n < 5675) || n = 5678 || n = 5679 || n = 5681 || (n > 5682 && n < 5687) || n = 5692 || (n > 5695 && n < 5700) || n = 5702 || n = 5703 || n = 5705 || n = 5708 || n = 5710 || n = 5715 || n = 5717 || n = 5718 || (n > 5719 && n < 5724) || n = 5725 || n = 5726 || (n > 5727 && n < 5731) || n = 5733 || n = 5735 || n = 5738 || (n > 5739 && n < 5743) || (n > 5743 && n < 5748) || (n > 5748 && n < 5753) || (n > 5753 && n < 5757) || n = 5761 || (n > 5763 && n < 5767) || (n > 5767 && n < 5772) || (n > 5772 && n < 5779) || n = 5781 || n = 5786 || n = 5788 || n = 5789 || (n > 5791 && n < 5796) || (n > 5796 && n < 5803) || n = 5804 || n = 5809 || n = 5811 || n = 5813 || n = 5814 || n = 5816 || n = 5818 || n = 5819 || (n > 5820 && n < 5826) || n = 5827 || n = 5830 || (n > 5832 && n < 5836) || n = 5838 || (n > 5839 && n < 5844) || (n > 5844 && n < 5849) || n = 5850 || n = 5853 || n = 5856 || n = 5859 || n = 5861 || n = 5864 || n = 5865 || n = 5867 || n = 5869 || n = 5870 || n = 5873 || n = 5876 || (n > 5877 && n < 5882) || (n > 5883 && n < 5887) || (n > 5888 && n < 5892) || n = 5894 || n = 5896 || n = 5899 || (n > 5900 && n < 5907) || n = 5908 || n = 5910 || n = 5913 || n = 5915 || (n > 5916 && n < 5920) || (n > 5920 && n < 5925) || (n > 5925 && n < 5932) || n = 5933 || n = 5934 || n = 5936)
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
.elseif(n = 5628 || n = 5652 || n = 5676 || n = 5700 || n = 5724 || n = 5748 || n = 5772 || n = 5796 || n = 5820 || n = 5844 || n = 5868 || n = 5892 || n = 5916)
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
.elseif(n = 5629 || n = 5645 || n = 5668 || n = 5691 || n = 5714 || n = 5760 || n = 5783 || n = 5806 || n = 5829 || n = 5852 || n = 5858 || n = 5875)
	; 14 bytes
	pha
	delay_n (n-3)
.elseif(n = 5632 || n = 5657 || n = 5707 || n = 5782)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5637 || n = 5662 || n = 5687 || n = 5712 || n = 5737 || n = 5787 || n = 5837 || n = 5912 || n = 5937)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5639)
	; 13 bytes
	sty @zptemp
	ldy #0
	nop
	jsr @nearby_rts_15cyc
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5641 || n = 5646 || n = 5667 || n = 5690 || n = 5713 || n = 5759 || n = 5828 || n = 5851 || n = 5874)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5642 || n = 5665 || n = 5688 || n = 5711 || n = 5734 || n = 5757 || n = 5780 || n = 5803 || n = 5826 || n = 5849 || n = 5872)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/23
	delay_n 18
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
.elseif(n = 5647 || n = 5671 || n = 5695 || n = 5719 || n = 5743 || n = 5767 || n = 5791 || n = 5815 || n = 5839 || n = 5863 || n = 5887 || n = 5911 || n = 5935)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5649 || n = 5675 || n = 5701 || n = 5727 || n = 5753 || n = 5779 || n = 5857 || n = 5883 || n = 5909)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5650 || n = 5677 || n = 5704 || n = 5731 || n = 5812 || n = 5866 || n = 5893 || n = 5920)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5654 || n = 5680 || n = 5706 || n = 5732 || n = 5758 || n = 5784 || n = 5810 || n = 5836 || n = 5862 || n = 5888 || n = 5914)
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
.elseif(n = 5655 || n = 5682 || n = 5709 || n = 5736 || n = 5763 || n = 5790 || n = 5817 || n = 5871 || n = 5898 || n = 5925)
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
.elseif(n = 5670 || n = 5693 || n = 5716 || n = 5739 || n = 5762 || n = 5785 || n = 5808 || n = 5831 || n = 5854 || n = 5877)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 5805)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 18
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 5807 || n = 5832 || n = 5882 || n = 5907 || n = 5932)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5855)
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5860)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5895)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 18
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 5897)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
	pla
.elseif(n = 5900)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 5938 to 6250
.if(n = 5938 || n = 5939 || (n > 5940 && n < 5947) || (n > 5947 && n < 5952) || (n > 5952 && n < 5957) || n = 5958 || n = 5960 || n = 5963 || n = 5965 || (n > 5966 && n < 5974) || (n > 5974 && n < 5979) || n = 5980 || n = 5981 || (n > 5983 && n < 5987) || (n > 5988 && n < 5992) || (n > 5992 && n < 6001) || (n > 6001 && n < 6006) || (n > 6007 && n < 6011) || n = 6014 || n = 6015 || n = 6017 || (n > 6018 && n < 6028) || n = 6029 || n = 6030 || n = 6034 || n = 6035 || n = 6038 || n = 6040 || n = 6041 || n = 6043 || (n > 6044 && n < 6055) || n = 6056 || n = 6058 || n = 6059 || n = 6061 || n = 6063 || n = 6064 || (n > 6065 && n < 6070) || (n > 6070 && n < 6079) || n = 6080 || n = 6081 || n = 6083 || n = 6085 || n = 6086 || n = 6088 || n = 6090 || n = 6092 || n = 6093 || (n > 6096 && n < 6103) || (n > 6103 && n < 6107) || n = 6110 || n = 6111 || n = 6113 || n = 6115 || n = 6116 || (n > 6117 && n < 6122) || (n > 6122 && n < 6127) || (n > 6127 && n < 6132) || (n > 6132 && n < 6136) || (n > 6137 && n < 6141) || n = 6142 || (n > 6143 && n < 6148) || n = 6149 || n = 6150 || (n > 6151 && n < 6156) || (n > 6157 && n < 6162) || (n > 6163 && n < 6168) || (n > 6169 && n < 6174) || (n > 6174 && n < 6182) || (n > 6182 && n < 6187) || n = 6188 || n = 6189 || (n > 6190 && n < 6195) || (n > 6195 && n < 6200) || (n > 6200 && n < 6207) || (n > 6207 && n < 6212) || (n > 6212 && n < 6217) || (n > 6217 && n < 6221) || (n > 6222 && n < 6226) || (n > 6226 && n < 6232) || (n > 6232 && n < 6237) || (n > 6237 && n < 6242) || n = 6243 || n = 6245 || n = 6246 || n = 6248 || n = 6250)
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
.elseif(n = 5940 || n = 5964 || n = 5988 || n = 6012 || n = 6036 || n = 6060 || n = 6084 || n = 6108 || n = 6132)
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
.elseif(n = 5947 || n = 5974 || n = 6001 || n = 6028 || n = 6082 || n = 6109 || n = 6136 || n = 6163 || n = 6190 || n = 6217 || n = 6244)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5952 || n = 5979 || n = 6006 || n = 6033 || n = 6087 || n = 6114 || n = 6141 || n = 6168 || n = 6195 || n = 6222 || n = 6249)
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
.elseif(n = 5957 || n = 5982 || n = 6032 || n = 6107 || n = 6157 || n = 6207 || n = 6232)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5959 || n = 5983 || n = 6007 || n = 6031 || n = 6055 || n = 6079 || n = 6103 || n = 6127)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5961 || n = 6013 || n = 6039 || n = 6065 || n = 6091 || n = 6117 || n = 6143 || n = 6169 || n = 6221 || n = 6247)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5962 || n = 5987 || n = 6062 || n = 6112 || n = 6137 || n = 6162 || n = 6187 || n = 6212 || n = 6237)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5966 || n = 5992 || n = 6018 || n = 6044 || n = 6070 || n = 6096 || n = 6122 || n = 6148 || n = 6174 || n = 6200 || n = 6226)
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
.elseif(n = 6011)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	nop
	jsr @nearby_rts_15cyc
	dex
	dex
	Jne *-7
	ldx @zptemp
.elseif(n = 6016)
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
.elseif(n = 6037)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 7
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 6057 || n = 6182)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
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
.elseif(n = 6094)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6095)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 10
	jsr @nearby_rts_15cyc
	Jne *-7
	ldx @zptemp
.elseif(n = 6151)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
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
.elseif(n = 6242)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 18
	dex
	dex
	Jne *-7
	ldx @zptemp
.endif
.endif
.endif
.else
.if((n >= 6251) && (n < 6876))
.if((n >= 6251) && (n < 6563))
; 6251 to 6562
.if(n = 6251 || (n > 6252 && n < 6257) || (n > 6257 && n < 6262) || (n > 6262 && n < 6269) || n = 6270 || n = 6272 || n = 6275 || n = 6277 || (n > 6278 && n < 6282) || (n > 6282 && n < 6287) || (n > 6287 && n < 6298) || (n > 6299 && n < 6303) || n = 6305 || n = 6306 || (n > 6307 && n < 6312) || (n > 6312 && n < 6323) || n = 6324 || n = 6326 || n = 6327 || n = 6329 || n = 6331 || (n > 6332 && n < 6337) || (n > 6337 && n < 6351) || (n > 6352 && n < 6356) || (n > 6357 && n < 6362) || (n > 6362 && n < 6377) || n = 6378 || n = 6380 || n = 6381 || n = 6383 || n = 6385 || n = 6386 || (n > 6387 && n < 6403) || n = 6404 || n = 6405 || n = 6409 || n = 6410 || (n > 6412 && n < 6429) || (n > 6429 && n < 6433) || (n > 6434 && n < 6438) || (n > 6438 && n < 6455) || (n > 6455 && n < 6460) || (n > 6460 && n < 6465) || (n > 6465 && n < 6481) || (n > 6481 && n < 6486) || (n > 6487 && n < 6492) || (n > 6492 && n < 6501) || (n > 6501 && n < 6506) || (n > 6507 && n < 6512) || n = 6513 || (n > 6514 && n < 6519) || (n > 6519 && n < 6533) || (n > 6533 && n < 6538) || n = 6539 || n = 6540 || (n > 6541 && n < 6546) || (n > 6546 && n < 6557) || n = 6558 || n = 6560 || n = 6561)
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
.elseif(n = 6252 || n = 6278 || n = 6304 || n = 6356 || n = 6382 || n = 6408 || n = 6434 || n = 6460 || n = 6486 || n = 6512 || n = 6538)
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
.elseif(n = 6257 || n = 6282 || n = 6332)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	nop
	delay_n 18
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6262 || n = 6287 || n = 6312 || n = 6337 || n = 6362 || n = 6387)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/25
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6269)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 6
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 6271 || n = 6298 || n = 6352 || n = 6379 || n = 6406 || n = 6433 || n = 6487 || n = 6514 || n = 6541)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6273 || n = 6299 || n = 6325 || n = 6351 || n = 6377 || n = 6403 || n = 6429 || n = 6455 || n = 6481 || n = 6507 || n = 6533 || n = 6559)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6274)
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 20
	iny
	Jne *-7
	tay
	pla
.elseif(n = 6276 || n = 6303 || n = 6330 || n = 6357 || n = 6384 || n = 6411 || n = 6438 || n = 6465 || n = 6492 || n = 6519 || n = 6546)
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
.elseif(n = 6307)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6323)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6328)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 20
	dey
	Jne *-7
	tay
	pla
.elseif(n = 6407)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6412)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 20
	dey
	Jne *-6
	tay
	pla
.elseif(n = 6501)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
	ldx @zptemp
.elseif(n = 6506)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 27
	Jne *-7
	tax
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
.else
; 6563 to 6875
.if(n = 6563 || (n > 6564 && n < 6568) || (n > 6568 && n < 6573) || (n > 6573 && n < 6585) || (n > 6585 && n < 6590) || (n > 6590 && n < 6595) || (n > 6595 && n < 6600) || (n > 6600 && n < 6611) || (n > 6611 && n < 6616) || (n > 6616 && n < 6622) || (n > 6622 && n < 6627) || (n > 6627 && n < 6637) || (n > 6637 && n < 6642) || (n > 6642 && n < 6649) || (n > 6649 && n < 6654) || (n > 6654 && n < 6663) || (n > 6663 && n < 6668) || (n > 6668 && n < 6676) || (n > 6676 && n < 6681) || (n > 6681 && n < 6703) || (n > 6704 && n < 6708) || (n > 6709 && n < 6730) || n = 6731 || n = 6732 || n = 6734 || n = 6736 || n = 6737 || (n > 6738 && n < 6757) || (n > 6757 && n < 6762) || (n > 6762 && n < 6784) || (n > 6784 && n < 6789) || n = 6790 || (n > 6791 && n < 6796) || (n > 6796 && n < 6811) || (n > 6811 && n < 6816) || (n > 6816 && n < 6838) || (n > 6838 && n < 6843) || (n > 6843 && n < 6865) || (n > 6865 && n < 6870) || (n > 6870 && n < 6876))
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
.elseif(n = 6564 || n = 6590 || n = 6616 || n = 6642)
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
.elseif(n = 6568 || n = 6595 || n = 6622 || n = 6649 || n = 6676 || n = 6703 || n = 6730 || n = 6757 || n = 6784 || n = 6811 || n = 6838 || n = 6865)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6573 || n = 6600 || n = 6627 || n = 6654 || n = 6681 || n = 6708 || n = 6735 || n = 6762 || n = 6789 || n = 6816 || n = 6843 || n = 6870)
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
.elseif(n = 6585 || n = 6611 || n = 6637)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 6663)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
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
.elseif(n = 6704)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 28
	Jne *-7
	ldx @zptemp
.elseif(n = 6709)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 28
	Jne *-7
	tax
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
.elseif(n = 6791)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 6796)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	jsr @nearby_rts_15cyc
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 6876 to 7500
.if((n > 6875 && n < 6892) || (n > 6892 && n < 6897) || (n > 6897 && n < 6907) || (n > 6907 && n < 6912) || (n > 6912 && n < 6919) || (n > 6919 && n < 6924) || (n > 6924 && n < 7110) || (n > 7110 && n < 7115) || (n > 7115 && n < 7501))
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
.elseif(n = 6892)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/27
	delay_n 7
	jsr @nearby_rts_15cyc
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
.elseif(n = 6907)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 29
	Jne *-7
	ldx @zptemp
.elseif(n = 6912)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 29
	Jne *-7
	tax
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
.elseif(n = 7110)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	ldx @zptemp
.elseif(n = 7115)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 30
	Jne *-7
	tax
	pla
.endif
.endif
.endif
.else
; 7501 to 10000
.if((n > 7500 && n < 10001))
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
.endif
.endif
.else
; 10001 to 20000
.if((n > 10000 && n < 20001))
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
.endif

.endif
	rest
.endmacro
