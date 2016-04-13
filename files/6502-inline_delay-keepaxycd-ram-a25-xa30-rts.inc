; delay_n, delays n cycles with small code
; Will clobber Z+N, and V
; Will preserve A, X, Y, S, C, D, and I (interrupt-disable flag)
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 5
; - 3 bytes or less when n <= 7
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 17
; - 6 bytes or less when n <= 19
; - 7 bytes or less when n <= 24
; - 8 bytes or less when n <= 29
; - 9 bytes or less when n <= 38
; -10 bytes or less when n <= 302
; -11 bytes or less when n <= 1312
; -12 bytes or less when n <= 2328
; -13 bytes or less when n <= 2863
; -14 bytes or less when n <= 3633
; -15 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.6, max=4
; - When 11 <= n <= 20: min=3, avg=4.9, max=7
; - When 21 <= n <= 30: min=6, avg=7.4, max=9
; - When 31 <= n <= 50: min=8, avg=8.9, max=10
; - When 51 <= n <= 75: min=8, avg=8.9, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=8, avg=9.0, max=9
; - When 201 <= n <= 300: min=8, avg=9.0, max=10
; - When 301 <= n <= 500: min=8, avg=10.1, max=11
; - When 501 <= n <= 750: min=8, avg=10.2, max=11
; - When 751 <= n <= 1000: min=9, avg=10.2, max=11
; - When 1001 <= n <= 2000: min=8, avg=10.7, max=12
; - When 2001 <= n <= 3000: min=10, avg=11.8, max=14
; - When 3001 <= n <= 5000: min=11, avg=13.5, max=15
; - When 5001 <= n <= 7500: min=14, avg=14.9, max=15
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
	; 2 bytes
	bit $24
.elseif(n = 4 || n = 6 || n = 26 || n = 39)
	; 2..10 bytes
	delay_n (n-2)
	nop
.elseif(n = 5)
	; 2 bytes
	inc @zptemp
.elseif(n = 7)
	; 2 bytes
	php
	plp
.elseif(n = 8)
	; 4 bytes
	.repeat 4
	 nop
	.endrepeat
.elseif(n = 9 || n = 11 || n = 13 || n = 19 || n = 31 || n = 33)
	; 3..9 bytes
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
.elseif(n = 14 || n = 21 || n = 28)
	; 4..8 bytes
	.repeat n/7
	 php
	 plp
	.endrepeat
.elseif(n = 15 || n = 17 || n = 22 || n = 24 || n = 27 || n = 29 || n = 36)
	; 5..9 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16 || n = 18 || n = 20)
	; 5..7 bytes
	delay_n (n-14)
	delay_n 14
.elseif(n = 23 || n = 30)
	; 7..9 bytes
	nop
	delay_n (n-2)
.elseif(n = 25)
	; 8 bytes
	delay_n 4
	delay_n (n-4)
.elseif(n = 32)
	; 9 bytes
	pha
	tya
	ldy #(n-12)/5
	dey
	Jne *-1
	tay
	pla
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
	; 9 bytes
	pha
	tya
	ldy #(n-17)/5
	dey
	Jpl *-1
	tay
	pla
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
.elseif((n > 40 && n < 53) || n = 54 || (n > 55 && n < 60) || (n > 60 && n < 79))
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
	; 11 bytes
	php
	pha
	php
	lda #255
	jsr delay_a_25_clocks
	plp
	pla
	plp
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
.if(n = 391 || n = 394 || n = 396 || n = 399 || n = 401 || n = 414 || n = 419 || n = 421 || n = 424 || n = 426 || n = 429 || n = 431 || n = 434 || n = 436 || n = 443 || n = 444 || n = 449 || n = 451 || n = 454 || n = 456 || n = 459 || n = 461 || n = 464)
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
.elseif(n = 398 || n = 433 || n = 438 || n = 458)
	; 11 bytes
	pha
	txa
	ldx #(n-18)/5
	dex
	Jne *-1
	inc @zptemp,x
	tax
	pla
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
.if(n = 626 || n = 629 || n = 631 || n = 634 || n = 639 || n = 641 || n = 644 || n = 646 || n = 654 || n = 659 || n = 661 || n = 664 || n = 666 || n = 669 || n = 671 || n = 674 || n = 676 || n = 679 || n = 681 || n = 688 || n = 689 || n = 694 || n = 699 || n = 701)
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
.elseif(n = 643 || n = 703)
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
	; 11 bytes
	pha
	tya
	ldy #n/12-2
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
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
.if(n = 1016 || n = 1019 || n = 1021 || n = 1024 || n = 1026 || n = 1029 || n = 1039 || n = 1044 || n = 1046 || n = 1049 || n = 1051 || n = 1054 || n = 1056 || n = 1059 || n = 1061 || n = 1064 || n = 1066 || n = 1073 || n = 1074 || n = 1079 || n = 1084 || n = 1086 || n = 1089 || n = 1091)
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $91A0'
	ldy #145
	dey
	Jne *-4
	ldy @zptemp
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
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $68A0'
	ldy #104
	iny
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #123
	delay_n 6
	dey
	Jpl *-4
	tay
	pla
.elseif(n = 1378)
	; 11 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $68A0'
	ldy #104
	iny
	Jne *-4
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
	sty @zptemp
	bit <$2C ;hides 'bit $9BA0'
	ldy #155
	dey
	Jne *-4
	ldy @zptemp
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
	delay_n 6
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
	sty @zptemp
	bit <$2C ;hides 'bit $B5A0'
	ldy #181
	dey
	Jne *-4
	ldy @zptemp
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
	sty @zptemp
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
	delay_n 6
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
.elseif(n = 2003)
	; 11 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DEA0'
	ldy #222
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2025)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
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
.elseif(n = 2058)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2069)
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
	delay_n 6
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
.elseif(n = 2150 || n = 2151 || n = 2159 || n = 2164 || n = 2169 || n = 2173 || n = 2178 || n = 2186 || n = 2187)
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
.elseif(n = 2157)
	; 11 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 2168)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
	delay_n 6
	Jne *-4
	ldx @zptemp
.elseif(n = 2240)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
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
	delay_n 6
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $2DA0'
	ldy #45
	nop
	iny
	Jne *-5
	ldy @zptemp
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
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	; 11 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2346 || n = 2349 || n = 2351 || n = 2354 || n = 2356 || n = 2363 || n = 2374 || n = 2379)
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
	delay_n 6
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
.elseif(n = 2365)
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	nop
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2368 || n = 2380)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	nop
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2387 || n = 2401 || n = 2415)
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
	delay_n 6
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2403)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DAA0'
	ldy #218
	nop
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2406 || n = 2420)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
	delay_n 6
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
.elseif(n = 2437 || n = 2447)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/10
	nop
	bit $24
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2439)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
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
.if(n = 2461 || n = 2464 || n = 2466 || n = 2469 || n = 2473 || n = 2474 || n = 2478 || n = 2481 || n = 2483 || n = 2486 || n = 2488 || n = 2494)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif((n .mod 11 = 1) && ((n > 2464 && n < 2499)))
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
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
.elseif(n = 2471 || n = 2493)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
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
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	nop
	dey
	Jne *-5
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
.elseif(n = 2489)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
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
.else
.if((n >= 2501) && (n < 3751))
.if((n >= 2501) && (n < 3126))
.if((n >= 2501) && (n < 2813))
.if((n >= 2501) && (n < 2657))
.if((n >= 2501) && (n < 2579))
.if((n >= 2501) && (n < 2540))
; 2501 to 2539
.if(n = 2501)
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
.elseif(n = 2505 || n = 2510 || n = 2514 || n = 2517 || n = 2524 || n = 2529 || n = 2533 || n = 2534)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
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
.elseif(n = 2513)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
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
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2549 || n = 2561)
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
.elseif(n = 2550 || n = 2554 || n = 2558 || n = 2565 || n = 2566 || n = 2570 || n = 2576 || n = 2577)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	delay_n 6
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
.elseif(n = 2571)
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
	tya
	ldy #<$2C ;hides 'bit $B9A0'
	ldy #185
	inc @zptemp
	dey
	Jne *-6
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
	delay_n 6
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
	delay_n 6
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
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	nop
	inx
	Jne *-5
	ldx @zptemp
.elseif(n = 2613)
	; 12 bytes
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 14
	Jne *-5
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
	; 12 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	nop
	inx
	Jne *-5
	tax
	pla
.endif
.endif
.else
; 2618 to 2656
.if(n = 2618 || n = 2620 || n = 2627 || n = 2637 || n = 2642 || n = 2643 || n = 2654)
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
.elseif(n = 2621 || n = 2633)
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
	delay_n 6
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
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
	delay_n 6
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2648)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
	delay_n 6
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
.elseif(n = 2658 || n = 2672 || n = 2686)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 2660 || n = 2661 || n = 2665 || n = 2666 || n = 2673 || n = 2675 || n = 2678 || n = 2687 || n = 2690 || n = 2692)
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
	delay_n 6
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
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 2693)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2694)
	; 12 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	nop
	iny
	Jne *-5
	tay
	pla
.endif
.else
; 2696 to 2734
.if(n = 2696 || n = 2718 || n = 2729)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2697 || n = 2699 || n = 2702 || n = 2704 || n = 2708 || n = 2720 || n = 2721 || n = 2725 || n = 2726 || n = 2730 || n = 2733)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
	delay_n 6
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
	delay_n 6
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
.elseif(n = 2738 || n = 2745 || n = 2747 || n = 2750 || n = 2752 || n = 2758 || n = 2759 || n = 2769)
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
	; 12 bytes
	pha
	tya
	ldy #(n-12)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
.elseif(n = 2741 || n = 2753)
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
	; 12 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
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
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2766)
	; 12 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	nop
	iny
	Jne *-5
	ldy @zptemp
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
; 2774 to 2812
.if((n > 2773 && n < 2777) || n = 2781 || n = 2785 || n = 2786 || n = 2792 || n = 2797 || n = 2809)
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
.elseif(n = 2779 || n = 2791 || n = 2803)
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
.elseif(n = 2787 || n = 2802)
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
.elseif(n = 2790 || n = 2801)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/11
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2793)
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
.elseif(n = 2814 || n = 2819 || n = 2822 || n = 2824 || (n > 2828 && n < 2832) || n = 2834 || n = 2836 || n = 2841 || n = 2842 || n = 2846 || n = 2848)
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
	delay_n 6
	dey
	Jne *-4
	ldy @zptemp
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
	; 12 bytes
	pha
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-4
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 2853 || n = 2858 || n = 2860 || n = 2864 || n = 2865 || n = 2870 || n = 2872 || n = 2889)
	; 12..14 bytes
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
.elseif(n = 2857)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2859)
	; 13 bytes
	php
	delay_n (n-7)
	plp
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 2888)
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
.elseif(n = 2893 || n = 2958)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2894 || n = 2901 || n = 2903 || n = 2906 || n = 2908 || n = 2912 || n = 2913 || n = 2915 || n = 2918 || n = 2920 || (n > 2924 && n < 2928) || n = 2930 || n = 2932 || n = 2937 || n = 2939 || n = 2942 || n = 2944 || n = 2949 || n = 2951 || n = 2954 || n = 2956 || n = 2960)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2895 || n = 2943 || n = 2955)
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
.elseif(n = 2898 || n = 2950 || n = 2963)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
.elseif(n = 2907 || n = 2922 || n = 2967)
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
.elseif(n = 2909 || n = 2921 || n = 2945 || n = 2957)
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
.elseif(n = 2917 || n = 2962)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2929)
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
.elseif(n = 2941)
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
.elseif(n = 2972)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 2973 || n = 2978 || n = 2985 || n = 2990 || n = 2992 || n = 2996 || n = 2997 || n = 3002)
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 2979 || n = 2991)
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
.elseif(n = 2982 || n = 2999)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 2984)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3009 || n = 3010 || n = 3014 || n = 3026 || n = 3028 || n = 3032 || n = 3033 || n = 3038 || n = 3040 || n = 3044)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3018 || n = 3030)
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
.elseif(n = 3019 || n = 3031 || n = 3043)
	; 11 bytes
	sty @zptemp
	ldy #(n-7)/12
	delay_n 7
	dey
	Jne *-3
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 6
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
.elseif(n = 3029 || n = 3041)
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
.else
.if((n >= 3047) && (n < 3086))
; 3047 to 3085
.if(n = 3047)
	; 12 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 6
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
.elseif(n = 3049)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
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
.elseif(n = 3051 || n = 3063 || n = 3075)
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
.elseif(n = 3054 || n = 3080)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
.elseif(n = 3056 || n = 3057 || n = 3061 || n = 3062 || n = 3066 || n = 3069 || n = 3074 || n = 3076 || n = 3081)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
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
.elseif(n = 3085)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 3086 to 3125
.if(n = 3086 || n = 3088 || n = 3094 || n = 3096 || n = 3103 || n = 3107 || n = 3108 || n = 3121 || n = 3122 || n = 3124)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3091)
	; 13 bytes
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
	delay_n 8
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
.elseif(n = 3097 || n = 3112)
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
.elseif(n = 3104)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3105)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 6
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3110)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $C2A0'
	ldy #194
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
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
	delay_n 8
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
.if((n >= 3126) && (n < 3145))
; 3126 to 3144
.if(n = 3126)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3127 || n = 3142)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3128)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 3131 || n = 3136 || n = 3137)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 3132)
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
.elseif(n = 3133)
	; 13 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	rol $26
	ror $26
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3134)
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
.elseif(n = 3139)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
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
.elseif(n = 3141)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3144)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3145 to 3164
.if(n = 3145 || n = 3158)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3146)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $C4A0'
	ldy #196
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3148 || n = 3162)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3149 || n = 3150 || n = 3154)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3151)
	; 13 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 4
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3152)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3153)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
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
.elseif(n = 3157)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3159)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3160)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1FA0'
	ldy #31
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3161)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3164)
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3167 || n = 3181)
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
.elseif(n = 3168 || n = 3173 || n = 3178 || n = 3193 || n = 3201)
	; 13..14 bytes
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
.elseif(n = 3172 || n = 3187 || n = 3202)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3175)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3177 || n = 3192)
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
.elseif(n = 3179)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3180 || n = 3196)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3182)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 4
	iny
	Jne *-6
	tay
	pla
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
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
.elseif(n = 3195)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 4
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3198)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3200)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	delay_n 6
	dey
	Jne *-7
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
.elseif(n = 3206 || n = 3214 || n = 3215 || n = 3219 || n = 3224 || n = 3229 || n = 3233 || n = 3234)
	; 13..14 bytes
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
.elseif(n = 3211)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $C8A0'
	ldy #200
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3212 || n = 3228)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3216)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3217)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
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
.elseif(n = 3223 || n = 3236)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
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
.elseif(n = 3230)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 4
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3231)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3238)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $CAA0'
	ldy #202
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3239)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3240)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	nop
	bit $24
	inx
	Jne *-7
	ldx @zptemp
.endif
.else
; 3243 to 3281
.if(n = 3243 || n = 3248 || n = 3250 || n = 3278)
	; 13..14 bytes
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
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 4
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3252 || n = 3267)
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3257 || n = 3270)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
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
	txa
	ldx #(n-18)/14
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 3268)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 4
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3272)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	inc @zptemp
	dey
	Jne *-6
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
.elseif(n = 3277)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
.if((n >= 3282) && (n < 3301))
; 3282 to 3300
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
.elseif(n = 3283 || n = 3297)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
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
.elseif(n = 3285 || n = 3290 || n = 3298)
	; 13 bytes
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
.elseif(n = 3287)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3288)
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
.elseif(n = 3292)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3293)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3294)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 3299)
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
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.endif
.else
; 3301 to 3320
.if(n = 3301)
	; 12 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3303 || n = 3319)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3304)
	; 13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3307)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3308)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3309)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/13
	delay_n 8
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3310)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
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
.elseif(n = 3312)
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
.elseif(n = 3313)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3314)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/13
	delay_n 8
	dey
	Jne *-5
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
.elseif(n = 3318)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3323 || n = 3328 || n = 3332 || n = 3341 || n = 3345 || n = 3346 || n = 3348 || n = 3355 || n = 3359)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3324)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3329)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	inc @zptemp
	dey
	Jne *-6
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
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
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
	delay_n 8
	dey
	Jne *-5
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
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 4
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3340)
	; 13 bytes
	pha
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3342 || n = 3357)
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
.elseif(n = 3347)
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3352)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
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
.if(n = 3360 || n = 3362 || n = 3363 || n = 3368 || n = 3369 || n = 3374 || n = 3376 || n = 3380 || n = 3392)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3364)
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
.elseif(n = 3370 || n = 3389)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3371)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2EA0'
	ldy #46
	delay_n 7
	iny
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
.elseif(n = 3373 || n = 3390)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
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
.elseif(n = 3377 || n = 3391)
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
.elseif(n = 3379 || n = 3393)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3396)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3398)
	; 13 bytes
	stx @zptemp
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
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
.elseif(n = 3400 || n = 3414 || n = 3428)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3401 || n = 3406 || n = 3411 || n = 3416 || n = 3422 || n = 3425 || n = 3430)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3402 || n = 3417 || n = 3432)
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
.elseif(n = 3407 || n = 3424)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3408)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3409 || n = 3423 || n = 3437)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
.elseif(n = 3427)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3433)
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
.elseif(n = 3434)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3435)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $D6A0'
	ldy #214
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3439 || n = 3443 || n = 3444 || n = 3448 || n = 3453 || n = 3460 || n = 3467 || n = 3472 || n = 3474)
	; 13..14 bytes
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
.elseif(n = 3445)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3449)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3450)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3452 || n = 3468)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3457)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3459)
	; 13 bytes
	stx @zptemp
	ldx $A6
	ldx #202 ;hides 'dex'
	jsr @nearby_rts
	Jne *-4
	ldx @zptemp
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
	ldy #<$2C ;hides 'bit $D8A0'
	ldy #216
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3481 || n = 3486 || n = 3488)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3483)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
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
.elseif(n = 3485)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 8
	iny
	Jne *-6
	ldy @zptemp
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3504)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3505)
	; 14 bytes
	php
	delay_n (n-7)
	plp
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
	delay_n 8
	dey
	Jne *-6
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if(n = 3516 || n = 3532)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3517 || n = 3547)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3518 || n = 3523 || n = 3528 || n = 3533 || n = 3534 || n = 3542 || n = 3544 || n = 3545 || n = 3550 || n = 3551)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3519)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3520)
	; 13 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 8
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3521 || n = 3535 || n = 3549)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3522 || n = 3537 || n = 3552)
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
.elseif(n = 3524)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	inc @zptemp
	iny
	Jne *-6
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
.elseif(n = 3526 || n = 3540 || n = 3554)
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
.elseif(n = 3529)
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
.elseif(n = 3536)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 6
	iny
	Jne *-7
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3543)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3553)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	inc @zptemp
	iny
	Jne *-6
	tay
	pla
.endif
.else
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if(n = 3555 || n = 3556 || n = 3569 || n = 3570 || n = 3572)
	; 13..14 bytes
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
.elseif(n = 3558)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $DEA0'
	ldy #222
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3559)
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
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 3562)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3563)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3564)
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
.elseif(n = 3568)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3571)
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
.elseif(n = 3573)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3575)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3576)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3577)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/14
	delay_n 9
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3578)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3579)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3580)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 3582)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3583 || n = 3593)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3585)
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
.elseif(n = 3586)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3587)
	; 13 bytes
	sty @zptemp
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
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
.elseif(n = 3589)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3592)
	; 13 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
	pla
.endif
.endif
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
.elseif(n = 3597 || n = 3612 || n = 3627)
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
.elseif(n = 3598 || n = 3600 || n = 3601 || n = 3608 || n = 3609 || (n > 3612 && n < 3616) || n = 3618 || n = 3629 || n = 3630)
	; 13..14 bytes
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
	txa
	ldx #0
	delay_n 9
	dex
	Jne *-4
	inc @zptemp,x
	tax
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3617)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3619)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3620)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 6
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 3621)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1EA0'
	ldy #30
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3623)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3624)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3625)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 3631)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 3632)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 3634 || n = 3663 || n = 3668)
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
.elseif(n = 3635 || n = 3646 || n = 3647 || n = 3649)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3636)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3637 || n = 3652)
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
.elseif(n = 3639 || n = 3655 || n = 3671)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3640)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3643)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3644 || n = 3660)
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
.elseif(n = 3648 || n = 3666)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3651)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 6
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 3656)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 6
	iny
	Jne *-7
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
	; 13 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
	ldx @zptemp
.elseif(n = 3661)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3664)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 13
	Jne *-6
	tax
	pla
.elseif(n = 3665)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $35A0'
	ldy #53
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3669)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3670)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.endif
.endif
.else
.if((n >= 3672) && (n < 3711))
; 3672 to 3710
.if(n = 3672 || n = 3687 || n = 3702)
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
.elseif(n = 3673 || n = 3677 || n = 3678 || n = 3681 || n = 3688 || n = 3689 || n = 3694 || n = 3695 || n = 3699 || n = 3700 || n = 3704 || n = 3705 || n = 3710)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3674 || n = 3693)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 3676 || n = 3692 || n = 3708)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
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
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 6
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 3682 || n = 3697)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3683)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $CCA0'
	ldy #204
	delay_n 9
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 3685)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3686)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3690)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 3698)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3707)
	; 13 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3709)
	; 14 bytes
	pha
	lda $A5
	txa
	ldx #232 ;hides 'inx'
	delay_n 7
	dex
	dex
	Jne *-5
	tax
	pla
.endif
.else
.if((n >= 3711) && (n < 3731))
; 3711 to 3730
.if(n = 3711)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3712 || n = 3727)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3713 || n = 3730)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3714 || n = 3715 || n = 3720 || n = 3721 || n = 3726)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3716)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 6
	iny
	Jne *-7
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
	; 14 bytes
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	delay_n 9
	iny
	Jne *-5
	tay
	pla
.elseif(n = 3728)
	; 14 bytes
	pha
	txa
	ldx #24 ;hides 'clc'
	delay_n 9
	inx
	Jne *-5
	inc @zptemp,x
	tax
	pla
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
.if(n = 3731)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3732)
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
.elseif(n = 3733)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 7
	dey
	Jne *-6
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3737 || n = 3743 || n = 3744 || n = 3748)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3738)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 7
	dey
	Jne *-6
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
.elseif(n = 3740)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
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
.elseif(n = 3750)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 7
	iny
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
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
.if((n >= 3751) && (n < 3790))
; 3751 to 3789
.if(n = 3751)
	; 13 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 11
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3753 || n = 3758 || n = 3773 || n = 3776 || n = 3779 || n = 3784 || n = 3785 || n = 3789)
	; 14 bytes
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
.elseif(n = 3755 || n = 3774)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3756 || n = 3772 || n = 3788)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3757 || n = 3787)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/15
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3759)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 9
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3760)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 6
	iny
	Jne *-7
	tay
	pla
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
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 9
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3764 || n = 3781)
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
.elseif(n = 3767 || n = 3783)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
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
.elseif(n = 3770)
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
.elseif(n = 3771)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3775)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 6
	iny
	Jne *-7
	tay
	pla
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
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $14A0'
	ldy #20
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.endif
.else
.if((n >= 3790) && (n < 3809))
; 3790 to 3808
.if(n = 3790 || n = 3794 || n = 3795 || n = 3800 || n = 3801 || n = 3806)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3791)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	rol $26
	ror $26
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3792 || n = 3807)
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
.elseif(n = 3796)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
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
	; 12 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	ldy @zptemp
.elseif(n = 3808)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 3809 to 3828
.if(n = 3809 || n = 3824)
	; 14 bytes
	delay_n (n-2)
	nop
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
	pha
	tya
	ldy #<$2C ;hides 'bit $B5A0'
	ldy #181
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3812)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 6
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 6
	iny
	Jne *-7
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3825)
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
.elseif(n = 3826)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3827)
	; 14 bytes
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 3828)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 3830)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	delay_n 7
	dey
	Jne *-6
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3833 || n = 3838 || n = 3839)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3840)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 3841)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 3843)
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
.elseif(n = 3844)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3845)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3846)
	; 13 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3847)
	; 13 bytes
	sty @zptemp
	ldy #0
	rol $26
	ror $26
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 3848 to 3867
.if(n = 3848 || n = 3853 || n = 3856 || n = 3865)
	; 13..14 bytes
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
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3852)
	; 13 bytes
	pha
	tya
	ldy #0
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3857)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	delay_n 9
	iny
	Jne *-7
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3860)
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
.elseif(n = 3861)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 7
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3862)
	; 12 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	delay_n 14
	Jne *-5
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
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
.if(n = 3868 || n = 3884)
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
.elseif(n = 3870 || n = 3873 || n = 3881 || (n > 3884 && n < 3888) || n = 3890 || n = 3896 || n = 3901 || n = 3904)
	; 13..14 bytes
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
.elseif(n = 3872)
	; 14 bytes
	delay_n (n-5)
	inc @zptemp
.elseif(n = 3874)
	; 13 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 14
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3875)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 3876 || n = 3897)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3877)
	; 13 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3878)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3879 || n = 3895)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3880)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3882)
	; 13 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 7
	inx
	Jne *-6
	tax
	pla
.elseif(n = 3883 || n = 3900)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3889)
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
.elseif(n = 3892)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 3902)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3903)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 3906)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
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
.elseif(n = 3908)
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
.elseif(n = 3911 || n = 3927 || n = 3943)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3912)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 3913)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 3915)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3916 || n = 3932)
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
.elseif(n = 3918)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3919 || n = 3923 || n = 3924 || n = 3928 || n = 3929 || n = 3933 || n = 3936 || n = 3938 || n = 3944)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3920)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 3921 || n = 3940)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 3931)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 7
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3935)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 3937)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 8
	dex
	dex
	Jne *-7
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
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 3948 || n = 3964 || n = 3980)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3949)
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	rol $26
	ror $26
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 3950 || n = 3953 || n = 3961 || n = 3965 || n = 3966 || n = 3970 || n = 3977 || n = 3982)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3951 || n = 3968)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3952)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	delay_n 9
	dey
	Jne *-7
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
.elseif(n = 3955 || n = 3976)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
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
.elseif(n = 3959 || n = 3975)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 3960 || n = 3981)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
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
.elseif(n = 3967)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 3969)
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
.elseif(n = 3971)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	rol $26
	ror $26
	iny
	Jne *-7
	tay
	pla
.elseif(n = 3972)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 3974)
	; 13 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 7
	inx
	Jne *-6
	ldx @zptemp
.elseif(n = 3978)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
; 3985 to 4023
.if(n = 3985 || n = 4002 || n = 4019)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 3986)
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
.elseif(n = 3987 || n = 3992 || n = 3993 || n = 3998 || n = 3999 || n = 4004 || n = 4009 || n = 4013 || n = 4014)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3990 || n = 4007)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 3991 || n = 4023)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 3996 || n = 4012)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 3997 || n = 4016)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4003)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4005)
	; 13 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 4008)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4010)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 7
	iny
	Jne *-6
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
.elseif(n = 4015)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4017)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $BFA0'
	ldy #191
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4018)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4020)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4021)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
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
.elseif(n = 4025 || n = 4029 || n = 4030 || n = 4045 || n = 4046 || n = 4056 || n = 4061)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4026)
	; 13 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4027)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 7
	iny
	Jne *-6
	tay
	pla
.elseif(n = 4028 || n = 4044 || n = 4060)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4031 || n = 4048)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4032 || n = 4052)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4033 || n = 4034 || (n > 4048 && n < 4052))
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
.elseif(n = 4035 || n = 4054)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4039 || n = 4055)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4047)
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
.elseif(n = 4057)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4062)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
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
	txa
	ldx #(n-18)/17
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4065)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4066)
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
.elseif(n = 4067)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
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
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4071)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/16
	delay_n 11
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4072 || n = 4077)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4073)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4078)
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
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4080)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4081)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 4082 to 4101
.if(n = 4082)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4083)
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
.elseif(n = 4084)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $3EA0'
	ldy #62
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 4088 || n = 4089 || n = 4093 || n = 4094 || n = 4099)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	txa
	ldx #<$2C ;hides 'bit $1DA2'
	ldx #29
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4097)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4098)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4100)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $C3A0'
	ldy #195
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
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
.elseif(n = 4105 || n = 4110 || n = 4113 || n = 4118 || n = 4123 || n = 4125 || n = 4128 || n = 4132 || n = 4137 || n = 4140)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4107)
	; 13 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
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
.elseif(n = 4111 || n = 4130)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4112)
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
.elseif(n = 4114)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4115)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E4A0'
	ldy #228
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4116 || n = 4135)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4117 || n = 4120 || n = 4136 || n = 4139)
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
.elseif(n = 4122)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4124)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4127)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 14
	inx
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4129)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
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
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4134)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.endif
.endif
.else
.if((n >= 4141) && (n < 4180))
; 4141 to 4179
.if(n = 4141 || n = 4158)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4142 || n = 4145 || n = 4151 || n = 4156 || n = 4157 || n = 4159 || n = 4162 || (n > 4173 && n < 4177) || n = 4179)
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
.elseif(n = 4144)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4146)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4147 || n = 4165)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4148)
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
.elseif(n = 4149)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4150)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $1AA0'
	ldy #26
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4152 || n = 4170)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4153 || n = 4161 || n = 4171 || n = 4178)
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
.elseif(n = 4154 || n = 4173)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
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
	stx @zptemp
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4166)
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
.elseif(n = 4167)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 11
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.endif
.else
.if((n >= 4180) && (n < 4199))
; 4180 to 4198
.if(n = 4180 || n = 4197)
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $E8A0'
	ldy #232
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4183)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 4185)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
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
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4188)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 4190)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4191 || n = 4193 || n = 4196 || n = 4198)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4192)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4202 || n = 4203)
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
.elseif(n = 4204)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $E9A0'
	ldy #233
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4205)
	; 13 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 14
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4206)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4207)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4208 || n = 4213 || n = 4215)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4209)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4214)
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
.if((n >= 4219) && (n < 4258))
; 4219 to 4257
.if(n = 4219 || n = 4237 || n = 4255)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4220 || n = 4221 || n = 4229 || n = 4238 || n = 4239 || n = 4256)
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
.elseif(n = 4223 || n = 4240 || n = 4257)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4224)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4225 || n = 4244)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4226 || n = 4243)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4227 || n = 4232 || n = 4242 || n = 4246 || n = 4247 || n = 4251)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4231 || n = 4248)
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
.elseif(n = 4233 || n = 4254)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4234)
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
.elseif(n = 4235)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EBA0'
	ldy #235
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4236)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4241)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4250)
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
.elseif(n = 4252)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4253)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.endif
.else
; 4258 to 4296
.if(n = 4258)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4259 || n = 4261 || (n > 4263 && n < 4267) || n = 4270 || n = 4276 || n = 4281 || n = 4283 || n = 4284 || n = 4289 || n = 4293)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4260 || n = 4278)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4262 || n = 4279 || n = 4296)
	; 12 bytes
	pha
	tya
	ldy #(n-12)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
.elseif(n = 4263 || n = 4282)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4267)
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
.elseif(n = 4269 || n = 4286)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4271)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4272)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4273)
	; 13 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 16
	Jne *-6
	tax
	pla
.elseif(n = 4274 || n = 4291)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4275)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4277)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4280)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4285)
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
.elseif(n = 4290)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4292)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4294)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4295)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
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
.if(n = 4297 || n = 4304 || n = 4305 || n = 4321 || n = 4323)
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
.elseif(n = 4298 || n = 4300 || n = 4303 || n = 4310 || n = 4315 || n = 4322 || n = 4327 || n = 4329 || n = 4332 || n = 4334)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4299 || n = 4316)
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
.elseif(n = 4301 || n = 4320)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4302 || n = 4324)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4306)
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
	sty @zptemp
	ldy #<$2C ;hides 'bit $11A0'
	ldy #17
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4308 || n = 4325)
	; 12 bytes
	sty @zptemp
	ldy #(n-7)/17
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4309)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4311 || n = 4328)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #(n-10)/17
	jsr @nearby_rts
	dex
	Jne *-4
	ldx @zptemp
.elseif(n = 4312 || n = 4333)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4314)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4317)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4318 || n = 4335)
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
.elseif(n = 4319)
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
.elseif(n = 4326)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4331)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.else
.if((n >= 4336) && (n < 4356))
; 4336 to 4355
.if(n = 4336 || n = 4353)
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
.elseif(n = 4337)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4338)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4339)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4340 || n = 4355)
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
.elseif(n = 4341 || n = 4346 || n = 4349 || n = 4351)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4345)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
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
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4350)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4352)
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
.elseif(n = 4354)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.endif
.else
; 4356 to 4375
.if(n = 4356)
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
	pha
	tya
	ldy #<$2C ;hides 'bit $CFA0'
	ldy #207
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4358)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4359)
	; 12 bytes
	sty @zptemp
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	ldy @zptemp
.elseif(n = 4360 || n = 4361 || n = 4365 || n = 4366)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4362)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4363)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
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
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4368)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
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
	; 14 bytes
	pha
	txa
	ldx #0
	jsr @nearby_rts
	dex
	Jne *-4
	inc @zptemp,x
	tax
	pla
.elseif(n = 4371)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4372)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4373)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4374)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4375)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.endif
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
.if((n >= 4376) && (n < 4395))
; 4376 to 4394
.if(n = 4376 || n = 4383 || n = 4389)
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
.elseif(n = 4377)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4378)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D0A0'
	ldy #208
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4379 || n = 4384 || n = 4388 || n = 4393)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4380)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4381)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4382)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4385)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4386)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4387)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4390)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
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
.elseif(n = 4392)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4394)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
	ldx @zptemp
.endif
.else
; 4395 to 4414
.if(n = 4395)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $D1A0'
	ldy #209
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4396)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4398 || n = 4403)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4399)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	rol $26
	ror $26
	dex
	dex
	Jne *-7
	tax
	pla
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
.elseif(n = 4401)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4402)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4404)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4405)
	; 14 bytes
	sty @zptemp
	ldy #56 ;hides 'sec'
	delay_n 15
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4406)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4407)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4408)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	.byte $2C,$E6,@zptemp ;bit abs; hides 'inc @zptemp'
	delay_n 7
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4409 || n = 4414)
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
.elseif(n = 4410)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 15
	iny
	Jne *-7
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
.elseif(n = 4412)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
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
.endif
.else
; 4415 to 4453
.if(n = 4415 || n = 4434 || n = 4453)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4417 || n = 4435)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
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
.elseif(n = 4419 || n = 4429 || n = 4430 || (n > 4443 && n < 4447) || (n > 4447 && n < 4451))
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
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4422 || n = 4440)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4423)
	; 14 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4424)
	; 14 bytes
	pha
	txa
	ldx #24 ;hides 'clc'
	jsr @nearby_rts
	inx
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 4425)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4426)
	; 13 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts
	iny
	Jne *-6
	ldy @zptemp
.elseif(n = 4427)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
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
.elseif(n = 4432)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4433)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	delay_n 9
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4436 || n = 4441)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4437)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4438)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4442)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4443)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4447)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
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
.elseif(n = 4452)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.endif
.endif
.else
.if((n >= 4454) && (n < 4493))
.if((n >= 4454) && (n < 4473))
; 4454 to 4472
.if(n = 4454)
	; 14 bytes
	stx @zptemp
	ldx $A6
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	ldx @zptemp
.elseif(n = 4455 || n = 4460)
	; 14 bytes
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
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4458)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4459)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4461)
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4462)
	; 14 bytes
	pha
	txa
	ldx #234 ;hides 'nop'
	jsr @nearby_rts
	dex
	Jne *-5
	inc @zptemp,x
	tax
	pla
.elseif(n = 4463)
	; 14 bytes
	php
	delay_n (n-7)
	plp
.elseif(n = 4464)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4465)
	; 14 bytes
	pha
	tya
	ldy #44 ;hides 'bit abs'
	delay_n 17
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4466 || n = 4468)
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
.elseif(n = 4467)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4469)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F8A0'
	ldy #248
	delay_n 9
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
.elseif(n = 4471)
	; 14 bytes
	stx @zptemp
	ldx #202 ;hides 'dex'
	php
	rol $26
	ror $26
	plp
	Jne *-7
	ldx @zptemp
.elseif(n = 4472)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.endif
.else
; 4473 to 4492
.if(n = 4473 || n = 4481 || n = 4482 || n = 4486 || n = 4490)
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
.elseif(n = 4474 || n = 4479)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4475)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	delay_n 9
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4476)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	php
	rol $26
	ror $26
	plp
	Jne *-7
	tax
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
.elseif(n = 4478)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4480)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4483)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $D5A0'
	ldy #213
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4484)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $2BA0'
	ldy #43
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4485)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4487)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
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
.elseif(n = 4489)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4491)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4492)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 4493 to 4531
.if(n = 4493)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4494 || n = 4512 || n = 4530)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4495 || n = 4519)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4496 || n = 4515)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4497 || n = 4502 || n = 4503 || n = 4508 || n = 4509 || n = 4513 || n = 4514 || n = 4516 || n = 4518)
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
.elseif(n = 4498 || n = 4517 || n = 4531)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4499)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $2AA0'
	ldy #42
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4500)
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
.elseif(n = 4501 || n = 4522)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4506 || n = 4527)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4507 || n = 4525)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4510 || n = 4529)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4511)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4520)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4521)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $29A0'
	ldy #41
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4526)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $29A0'
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
	ldy #<$2C ;hides 'bit $05A0'
	ldy #5
	delay_n 9
	iny
	Jne *-7
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
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4533 || n = 4535 || (n > 4536 && n < 4541) || n = 4545 || n = 4549 || n = 4551 || n = 4554 || (n > 4555 && n < 4559) || n = 4570)
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
.elseif(n = 4536 || n = 4550 || n = 4555)
	; 14 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4543 || n = 4561)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4544)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4546)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
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
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4548 || n = 4567)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4559)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4560)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4562)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $D9A0'
	ldy #217
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4563)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4564)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4565)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4566)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4568)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $27A0'
	ldy #39
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4569)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.endif
.else
; 4571 to 4609
.if(n = 4571 || n = 4573 || n = 4575 || n = 4576 || n = 4580 || n = 4581 || n = 4594 || n = 4598 || n = 4599 || n = 4603 || n = 4608)
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
.elseif(n = 4572 || n = 4591)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4574 || n = 4588 || n = 4593 || n = 4607)
	; 14 bytes
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/18
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4582)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
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
	bit <$2C ;hides 'bit $02A0'
	ldy #2
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4584 || n = 4602)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4585 || n = 4606)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4586 || n = 4605)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4587)
	; 14 bytes
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 14
	dey
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4589)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $DAA0'
	ldy #218
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4590)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4592)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4595)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4596)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4600)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4601)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4604)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4609)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DBA0'
	ldy #219
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 4610 to 4687
.if(n = 4610 || n = 4629 || n = 4648 || n = 4667 || n = 4686)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4611 || n = 4632 || n = 4653 || n = 4674)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4612 || n = 4626 || n = 4631 || n = 4645 || n = 4650 || n = 4664 || n = 4669 || n = 4683)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4613)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4614)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4615)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 13
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4616 || n = 4617 || (n > 4620 && n < 4624) || n = 4628 || (n > 4632 && n < 4639) || (n > 4639 && n < 4643) || n = 4649 || (n > 4654 && n < 4662) || n = 4665 || n = 4666 || n = 4670 || n = 4671 || n = 4675 || n = 4677 || n = 4678 || n = 4680 || n = 4682 || n = 4684)
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
.elseif(n = 4618)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4619)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4620)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4624 || n = 4643 || n = 4662 || n = 4681)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4627)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4630)
	; 14 bytes
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	dex
	dex
	Jne *-7
	tax
	pla
.elseif(n = 4639 || n = 4663 || n = 4687)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4644 || n = 4668)
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4651)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $DDA0'
	ldy #221
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4652 || n = 4672)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4654 || n = 4676)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
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
.elseif(n = 4679)
	; 14 bytes
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 18
	Jne *-7
	tax
	pla
.elseif(n = 4685)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 13
	dey
	Jne *-7
	ldy @zptemp
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
.if((n >= 4688) && (n < 4766))
.if((n >= 4688) && (n < 4727))
; 4688 to 4726
.if(n = 4688 || n = 4702 || n = 4707 || n = 4721 || n = 4726)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4689)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4690)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 13
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4691 || n = 4696 || n = 4697 || n = 4699 || n = 4701 || n = 4703 || n = 4704 || n = 4706 || n = 4708 || n = 4713 || n = 4717 || n = 4718 || n = 4722 || n = 4723 || n = 4725)
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
.elseif(n = 4692)
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
.elseif(n = 4693)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4694)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $21A0'
	ldy #33
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4695 || n = 4716)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4698 || n = 4720)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4700 || n = 4719)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
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
.elseif(n = 4709)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4712)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4714)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $20A0'
	ldy #32
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4715)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E0A0'
	ldy #224
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.endif
.else
; 4727 to 4765
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
.elseif(n = 4728 || n = 4733 || n = 4734 || n = 4739 || n = 4741 || n = 4744 || n = 4746 || (n > 4747 && n < 4751) || n = 4754 || n = 4755 || n = 4760 || n = 4761 || n = 4763 || n = 4765)
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
.elseif(n = 4732 || n = 4753)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4735)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4736)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $E1A0'
	ldy #225
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 4737 || n = 4758)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4738 || n = 4757)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4740 || n = 4745 || n = 4759 || n = 4764)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4742)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4743 || n = 4762)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4747)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
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
.elseif(n = 4752)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
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
.endif
.endif
.else
; 4766 to 4843
.if(n = 4766 || (n > 4767 && n < 4772) || n = 4775 || n = 4780 || n = 4782 || n = 4784 || n = 4785 || n = 4790 || n = 4791 || n = 4796 || n = 4801 || (n > 4802 && n < 4807) || n = 4809 || n = 4811 || n = 4813 || n = 4817 || n = 4818 || n = 4822 || n = 4823 || n = 4825 || n = 4826 || n = 4828 || n = 4834 || n = 4839 || n = 4843)
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
.elseif(n = 4767 || n = 4787 || n = 4827)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4772 || n = 4792 || n = 4832)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4773)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1DA0'
	ldy #29
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4774 || n = 4816 || n = 4837)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4776 || n = 4795 || n = 4814 || n = 4833)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
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
.elseif(n = 4778 || n = 4783 || n = 4797 || n = 4802 || n = 4835 || n = 4840)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4779 || n = 4821 || n = 4842)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4781 || n = 4800 || n = 4819 || n = 4838)
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4786 || n = 4808 || n = 4830)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4788 || n = 4812 || n = 4836)
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
.elseif(n = 4789)
	; 14 bytes
	sty @zptemp
	ldy #184 ;hides 'clv'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 4793)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $1CA0'
	ldy #28
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4794)
	; 14 bytes
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.elseif(n = 4807 || n = 4831)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4810)
	; 14 bytes
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 17
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4815)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $1BA0'
	ldy #27
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
.elseif(n = 4824)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4829)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	delay_n 14
	iny
	Jne *-7
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
.endif
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
; 4844 to 4882
.if(n = 4844 || n = 4848 || n = 4849 || n = 4853 || (n > 4863 && n < 4867) || n = 4869 || n = 4870 || n = 4875 || n = 4880 || n = 4881)
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
.elseif(n = 4845)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4846)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	delay_n 16
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4847 || n = 4867)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4850)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4851)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 16
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4852)
	; 13 bytes
	sty @zptemp
	ldy #(n-7)/19
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4854 || n = 4859 || n = 4878)
	; 14 bytes
	delay_n (n-2)
	nop
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
	; 13 bytes
	pha
	tya
	ldy #(n-12)/19
	delay_n 14
	dey
	Jne *-5
	tay
	pla
.elseif(n = 4858 || n = 4879)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 4863)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4868)
	; 14 bytes
	sty @zptemp
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4871)
	; 13 bytes
	sty @zptemp
	ldy #0
	delay_n 14
	dey
	Jne *-5
	ldy @zptemp
.elseif(n = 4872)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4873)
	; 14 bytes
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 14
	iny
	Jne *-7
	tay
	pla
.elseif(n = 4874)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
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
	; 13 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 14
	iny
	Jne *-6
	ldy @zptemp
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
; 4883 to 4921
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
.elseif(n = 4884 || n = 4905)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4885 || n = 4886 || (n > 4887 && n < 4892) || (n > 4892 && n < 4896) || n = 4897 || n = 4901 || n = 4902 || n = 4906 || (n > 4908 && n < 4912) || (n > 4912 && n < 4918))
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
.elseif(n = 4887 || n = 4907)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4892 || n = 4912)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4896 || n = 4918)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
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
.elseif(n = 4900 || n = 4921)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
	; 13 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
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
.endif
.endif
.else
; 4922 to 5000
.if(n = 4922 || n = 4923 || (n > 4927 && n < 4932) || (n > 4932 && n < 4940) || n = 4943 || n = 4944 || (n > 4947 && n < 4951) || (n > 4952 && n < 4956) || (n > 4956 && n < 4961) || n = 4964 || n = 4965 || (n > 4968 && n < 4972) || n = 4973 || n = 4974 || (n > 4975 && n < 4980) || n = 4981 || n = 4985 || n = 4986 || n = 4990 || n = 4991 || (n > 4992 && n < 4999) || n = 5000)
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
	ldy #234 ;hides 'nop'
	delay_n 14
	dey
	Jne *-6
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
.elseif(n = 4926 || n = 4947 || n = 4968 || n = 4989)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4927 || n = 4951 || n = 4975 || n = 4999)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4932 || n = 4956 || n = 4980)
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
.elseif(n = 4940 || n = 4962)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4941)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $15A0'
	ldy #21
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 4942 || n = 4963 || n = 4984)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
.elseif(n = 4952 || n = 4972 || n = 4992)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 4961)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $ECA0'
	ldy #236
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 4982)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $13A2'
	ldx #19
	jsr @nearby_rts
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 4983)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $EDA0'
	ldy #237
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
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
.if(n = 5001 || n = 5002 || n = 5011 || (n > 5012 && n < 5023) || (n > 5032 && n < 5045) || n = 5048 || n = 5049 || (n > 5052 && n < 5066) || n = 5069 || n = 5070 || n = 5074 || n = 5075 || n = 5077 || n = 5078)
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
.elseif(n = 5003)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $EEA0'
	ldy #238
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5004 || n = 5028 || n = 5076)
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
.elseif(n = 5005 || n = 5026 || n = 5047 || n = 5068)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5006 || n = 5050)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5007 || n = 5027 || n = 5067)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
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
.elseif(n = 5009)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $12A0'
	ldy #18
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5010 || n = 5031 || n = 5052 || n = 5073)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5012 || n = 5032 || n = 5072)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5023 || n = 5071)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
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
.elseif(n = 5029)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $11A0'
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
	bit <$2C ;hides 'bit $EFA0'
	ldy #239
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5045)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F0A0'
	ldy #240
	jsr @nearby_rts
	dey
	Jne *-7
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
.elseif(n = 5066)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0FA0'
	ldy #15
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.endif
.else
; 5079 to 5156
.if((n > 5078 && n < 5087) || n = 5090 || n = 5091 || (n > 5095 && n < 5100) || (n > 5100 && n < 5107) || n = 5111 || n = 5117 || n = 5118 || (n > 5119 && n < 5124) || n = 5125 || n = 5126 || n = 5128 || n = 5133 || n = 5137 || (n > 5138 && n < 5143) || (n > 5143 && n < 5148) || n = 5149 || n = 5154)
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
.elseif(n = 5087 || n = 5107)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/20
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5088)
	; 14 bytes
	stx @zptemp
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts
	inx
	Jne *-7
	ldx @zptemp
.elseif(n = 5089 || n = 5110 || n = 5131 || n = 5152)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5092 || n = 5112)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/20
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5093)
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $0EA2'
	ldx #14
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5094 || n = 5115 || n = 5136)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5095 || n = 5119 || n = 5143)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5100 || n = 5124 || n = 5148)
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
.elseif(n = 5108)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $F3A0'
	ldy #243
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5109)
	; 14 bytes
	sty @zptemp
	ldy #24 ;hides 'clc'
	delay_n 15
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 15
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5116 || n = 5138)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5127)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 15
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5129)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5130)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5132)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 15
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5134)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5153)
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 15
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5155)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $F5A0'
	ldy #245
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
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
.else
.if((n >= 5157) && (n < 5235))
; 5157 to 5234
.if(n = 5157 || n = 5178 || n = 5199 || n = 5220)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5158)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 15
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5159 || (n > 5160 && n < 5167) || (n > 5167 && n < 5171) || n = 5174 || n = 5175 || (n > 5178 && n < 5182) || (n > 5182 && n < 5191) || n = 5195 || (n > 5199 && n < 5204) || (n > 5205 && n < 5210) || n = 5211 || n = 5212 || n = 5216 || n = 5217 || (n > 5220 && n < 5226) || (n > 5226 && n < 5234))
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
.elseif(n = 5160 || n = 5182 || n = 5204 || n = 5226)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5167 || n = 5191)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5171)
	; 14 bytes
	stx @zptemp
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	ldx @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5176)
	; 14 bytes
	pha
	txa
	ldx #<$2C ;hides 'bit $0AA2'
	ldx #10
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
.elseif(n = 5177)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $0AA0'
	ldy #10
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 5197)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5198)
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $09A0'
	ldy #9
	jsr @nearby_rts
	iny
	Jne *-7
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
	; 14 bytes
	pha
	txa
	bit <$2C ;hides 'bit $08A2'
	ldx #8
	jsr @nearby_rts
	inx
	Jne *-7
	tax
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
.else
; 5235 to 5312
.if(n = 5235)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $07A0'
	ldy #7
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5236 || n = 5257 || n = 5278 || n = 5299)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5237 || n = 5238 || n = 5242 || n = 5243 || (n > 5244 && n < 5248) || (n > 5248 && n < 5255) || n = 5258 || n = 5259 || (n > 5263 && n < 5268) || n = 5269 || (n > 5270 && n < 5276) || n = 5279 || n = 5280 || (n > 5283 && n < 5287) || (n > 5288 && n < 5292) || (n > 5292 && n < 5297) || n = 5300 || n = 5301 || (n > 5304 && n < 5311) || n = 5312)
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
.elseif(n = 5241 || n = 5262 || n = 5283 || n = 5304)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5244 || n = 5268 || n = 5292)
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
.elseif(n = 5248 || n = 5270)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $06A0'
	ldy #6
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 5263 || n = 5287 || n = 5311)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
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
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
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
	; 14 bytes
	pha
	tya
	bit <$2C ;hides 'bit $05A0'
	ldy #5
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 5302)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $04A0'
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
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.endif
.endif
.endif
.else
; 5313 to 5625
.if(n = 5313 || n = 5315 || n = 5317 || n = 5321 || n = 5322 || (n > 5325 && n < 5335) || n = 5337 || n = 5338 || n = 5342 || n = 5343 || (n > 5346 && n < 5358) || n = 5363 || (n > 5367 && n < 5380) || n = 5384 || n = 5385 || (n > 5388 && n < 5392) || (n > 5392 && n < 5402) || (n > 5402 && n < 5407) || (n > 5407 && n < 5412) || (n > 5412 && n < 5424) || (n > 5424 && n < 5431) || (n > 5431 && n < 5436) || (n > 5436 && n < 5446) || (n > 5446 && n < 5455) || (n > 5455 && n < 5460) || (n > 5460 && n < 5468) || (n > 5468 && n < 5479) || (n > 5479 && n < 5484) || (n > 5484 && n < 5490) || (n > 5490 && n < 5503) || (n > 5503 && n < 5508) || (n > 5508 && n < 5512) || (n > 5512 && n < 5527) || (n > 5527 && n < 5532) || n = 5533 || (n > 5534 && n < 5551) || (n > 5551 && n < 5556) || (n > 5556 && n < 5575) || n = 5576 || n = 5577 || n = 5579 || (n > 5580 && n < 5599) || (n > 5600 && n < 5604) || (n > 5604 && n < 5622) || n = 5624 || n = 5625)
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
.elseif(n = 5314 || n = 5336 || n = 5358 || n = 5380 || n = 5402 || n = 5424 || n = 5446 || n = 5468 || n = 5490 || n = 5512 || n = 5534 || n = 5578 || n = 5600 || n = 5622)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/22
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5316 || n = 5340 || n = 5364 || n = 5412 || n = 5436 || n = 5460 || n = 5484 || n = 5508 || n = 5532 || n = 5556 || n = 5580 || n = 5604)
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
.elseif(n = 5318)
	; 14 bytes
	stx @zptemp
	ldx #232 ;hides 'inx'
	delay_n 14
	dex
	dex
	Jne *-7
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
.elseif(n = 5320 || n = 5341 || n = 5362)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/21
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
.elseif(n = 5325 || n = 5346 || n = 5367)
	; 14 bytes
	pha
	tya
	ldy #(n-12)/21
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5335 || n = 5359 || n = 5407 || n = 5431 || n = 5455 || n = 5479 || n = 5503 || n = 5527 || n = 5551 || n = 5575 || n = 5599 || n = 5623)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	ldy @zptemp
.elseif(n = 5339)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5344)
	; 14 bytes
	pha
	tya
	ldy #<$2C ;hides 'bit $02A0'
	ldy #2
	jsr @nearby_rts
	iny
	Jne *-7
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
.elseif(n = 5381)
	; 14 bytes
	sty @zptemp
	ldy #<$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	iny
	Jne *-7
	ldy @zptemp
.elseif(n = 5382)
	; 14 bytes
	sty @zptemp
	bit <$2C ;hides 'bit $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5383)
	; 14 bytes
	sty @zptemp
	ldy #0
	delay_n 16
	dey
	Jne *-6
	ldy @zptemp
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
	; 14 bytes
	sty @zptemp
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	ldy @zptemp
.elseif(n = 5388)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 16
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5392)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 16
	dey
	Jne *-7
	tay
	pla
.endif
.endif
.else
; 5626 to 6250
.if(n = 5626)
	; 14 bytes
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 17
	dey
	Jne *-7
	tay
	pla
.elseif(n = 5627 || (n > 5628 && n < 5644) || n = 5645 || n = 5646 || (n > 5647 && n < 5652) || (n > 5652 && n < 5671) || (n > 5671 && n < 5676) || (n > 5676 && n < 5695) || (n > 5695 && n < 5700) || (n > 5700 && n < 5719) || (n > 5719 && n < 5724) || (n > 5724 && n < 5743) || (n > 5743 && n < 5748) || (n > 5748 && n < 5767) || (n > 5767 && n < 5772) || (n > 5772 && n < 5791) || (n > 5791 && n < 5796) || (n > 5796 && n < 5815) || (n > 5815 && n < 5820) || (n > 5820 && n < 5839) || (n > 5839 && n < 5844) || (n > 5844 && n < 5863) || (n > 5863 && n < 5868) || (n > 5868 && n < 5887) || (n > 5887 && n < 5892) || (n > 5892 && n < 5897) || (n > 5897 && n < 5911) || (n > 5911 && n < 5916) || (n > 5916 && n < 5935) || (n > 5935 && n < 5940) || (n > 5940 && n < 5959) || (n > 5959 && n < 5964) || (n > 5964 && n < 5983) || (n > 5983 && n < 5988) || (n > 5988 && n < 6007) || (n > 6007 && n < 6012) || (n > 6012 && n < 6031) || (n > 6031 && n < 6036) || (n > 6037 && n < 6042) || (n > 6042 && n < 6055) || (n > 6055 && n < 6060) || (n > 6060 && n < 6079) || (n > 6079 && n < 6084) || (n > 6084 && n < 6089) || (n > 6089 && n < 6094) || (n > 6094 && n < 6103) || (n > 6103 && n < 6108) || (n > 6108 && n < 6127) || (n > 6127 && n < 6132) || (n > 6132 && n < 6151) || (n > 6151 && n < 6156) || (n > 6156 && n < 6251))
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
.elseif(n = 5628 || n = 5652 || n = 5676 || n = 5700 || n = 5724 || n = 5748 || n = 5772 || n = 5796 || n = 5820 || n = 5844 || n = 5868 || n = 5892 || n = 5916 || n = 5940 || n = 5964 || n = 5988 || n = 6012 || n = 6036 || n = 6060 || n = 6084 || n = 6108 || n = 6132)
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
.elseif(n = 5644)
	; 14 bytes
	pha
	tya
	ldy #0
	delay_n 17
	dey
	Jne *-6
	tay
	pla
.elseif(n = 5647 || n = 5671 || n = 5695 || n = 5719 || n = 5743 || n = 5767 || n = 5791 || n = 5815 || n = 5839 || n = 5863 || n = 5887 || n = 5911 || n = 5935 || n = 5959 || n = 5983 || n = 6007 || n = 6031 || n = 6055 || n = 6079 || n = 6103 || n = 6127)
	; 14 bytes
	sty @zptemp
	ldy #(n-7)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
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
.endif
.endif
.else
; 6251 to 7500
.if((n > 6250 && n < 7501))
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
