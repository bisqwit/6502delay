; delay_n, delays n cycles with small code
; Will preserve A, X, Y, S, and P
;
; Uses JMP (unrelocatable code).
;
; Requires that a label @nearby_rts is defined,
; that points _somewhere_ that contains a $60 byte (RTS).
;
; Requires that a label @nearby_rts_14cyc is defined,
; that points _somewhere_ that contains a dummy 2-cycle
; opcode, such as NOP or LDA #1, followed by a $60 byte (RTS).
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
; - 1 bytes or less when n <= 2
; - 2 bytes or less when n <= 2
; - 3 bytes or less when n <= 4
; - 4 bytes or less when n <= 12
; - 5 bytes or less when n <= 14
; - 6 bytes or less when n <= 21
; - 7 bytes or less when n <= 26
; - 8 bytes or less when n <= 28
; - 9 bytes or less when n <= 38
; -10 bytes or less when n <= 300
; -11 bytes or less when n <= 305
; -12 bytes or less when n <= 314
; -13 bytes or less when n <= 1333
; -14 bytes or less when n <= 1515
; -15 bytes or less when n <= 2796
; -16 bytes or less when n <= 3342
; -17 bytes or less when n <= 4361
; -18 bytes or less when n <= 7447
; -19 bytes or less when n <= 20000
; Detailed summary:
; - When 2 <= n <= 10: min=1, avg=2.9, max=4
; - When 11 <= n <= 20: min=3, avg=4.7, max=6
; - When 21 <= n <= 30: min=5, avg=6.7, max=9
; - When 31 <= n <= 50: min=8, avg=8.8, max=10
; - When 51 <= n <= 75: min=8, avg=9.0, max=9
; - When 76 <= n <= 100: min=9, avg=9.0, max=9
; - When 101 <= n <= 200: min=9, avg=9.0, max=9
; - When 201 <= n <= 300: min=9, avg=9.0, max=10
; - When 301 <= n <= 500: min=10, avg=11.8, max=13
; - When 501 <= n <= 750: min=9, avg=11.8, max=13
; - When 751 <= n <= 1000: min=9, avg=11.8, max=13
; - When 1001 <= n <= 2000: min=9, avg=12.5, max=15
; - When 2001 <= n <= 3000: min=9, avg=13.6, max=16
; - When 3001 <= n <= 5000: min=9, avg=14.0, max=18
; - When 5001 <= n <= 7500: min=9, avg=14.4, max=19
; - When 7501 <= n <= 10000: min=9, avg=14.5, max=19
; - When 10001 <= n <= 20000: min=9, avg=14.5, max=19
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
; 1 to 312
.if(n = 1)
	.error "delay_n: 1-cycle delay cannot be implemented"
.elseif(n = 2)
	; 1 bytes
	nop
.elseif(n = 3)
	; 3 bytes
	jmp *+3
.elseif((n > 3 && n < 7) || n = 16 || n = 18 || n = 20 || n = 30 || n = 32 || (n > 298 && n < 302) || n = 306)
	; 2..12 bytes
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
.elseif((n .mod 2 = 1) && ((n > 8 && n < 16) || (n > 18 && n < 28) || (n > 30 && n < 40)))
	; 3..10 bytes
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
.elseif(n = 14)
	; 3 bytes
	jsr @nearby_rts_14cyc
.elseif(n = 17)
	; 6 bytes
	php
	rol $26
	ror $26
	plp
.elseif(n = 22 || n = 24 || n = 29 || n = 36 || n = 308 || n = 310)
	; 6..12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 26 || n = 28 || n = 42 || n = 311)
	; 6..12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 34)
	; 9 bytes
	php
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
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
.elseif(n = 41)
	; 10 bytes
	php
	pha
	lda $A5
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	plp
.elseif((n > 42 && n < 53) || (n > 53 && n < 299))
	; 9 bytes
	php
	pha
	lda #n-43
	jsr delay_a_27_clocks
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
.elseif((n > 302 && n < 306))
	; 11 bytes
	php
	pha
	php
	lda #n-50
	jsr delay_a_27_clocks
	plp
	pla
	plp
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
.elseif(n = 312)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
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
.elseif(n = 315 || n = 363)
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
.elseif(n = 318 || n = 346 || n = 360 || n = 381 || n = 388)
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
.elseif(n = 330 || n = 335 || n = 340)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
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
.elseif(n = 343 || n = 378)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 345 || n = 385)
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
.elseif(n = 350)
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
.elseif(n = 365 || n = 380)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-5
	sbc #1
	Jpl *-2
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
.elseif(n = 370)
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
.endif
.else
; 391 to 468
.if(n = 391 || n = 396 || n = 398 || n = 401 || n = 406 || n = 408 || n = 420 || n = 421 || n = 426 || n = 428 || n = 431 || n = 436 || n = 438 || n = 441 || n = 455 || n = 456 || n = 461 || n = 463 || n = 466 || n = 468)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 392 || n = 437 || n = 442 || n = 467)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
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
.elseif((n .mod 5 = 4) && (n = 394 || n = 404 || n = 409 || n = 419 || n = 434 || (n > 443 && n < 465)))
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
.elseif(n = 395)
	; 12 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	iny
	Jne *-3
	tay
	pla
	plp
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
.elseif(n = 399 || n = 414 || n = 424 || n = 429 || n = 439)
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
.elseif(n = 400)
	; 12 bytes
	php
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	Jmi *-1
	tax
	pla
	plp
.elseif((n .mod 5 = 2) && ((n > 401 && n < 433) || (n > 446 && n < 463)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-22)/5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 403 || n = 443)
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
.elseif(n = 405 || n = 410 || n = 450)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-5
	sbc #1
	Jpl *-2
	pla
	plp
.elseif(n = 411 || n = 425)
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
.elseif(n = 415)
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
.elseif(n = 418 || n = 446 || n = 453 || n = 460)
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
.elseif(n = 423)
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
.elseif(n = 430 || n = 451 || n = 458 || n = 465)
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
.elseif(n = 433)
	; 12 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	jsr @nearby_rts_14cyc
	Jne *-5
	pla
	plp
.elseif(n = 435)
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 4
	dex
	Jmi *-4
	tax
	pla
	plp
.elseif(n = 440 || n = 445)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 448)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.endif
.else
.if((n >= 469) && (n < 547))
; 469 to 546
.if(n = 469 || n = 474 || n = 484 || n = 494 || n = 499 || n = 509 || n = 519 || n = 524 || n = 529)
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
.elseif(n = 470 || n = 480 || n = 510)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 471 || n = 476 || n = 478 || n = 491 || n = 496 || n = 501 || n = 506 || n = 508 || n = 511 || n = 513 || n = 525 || n = 526 || n = 531 || n = 536 || n = 538 || n = 541 || n = 543 || n = 545 || n = 546)
	; 10..13 bytes
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
.elseif(n = 475 || n = 515)
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
.elseif(n = 477 || n = 492 || n = 502 || n = 507 || n = 512 || n = 527 || n = 532)
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
.elseif(n = 481 || n = 488 || n = 516 || n = 530)
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
.elseif(n = 483)
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
.elseif(n = 518)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 520 || n = 540)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-5
	sbc #1
	Jpl *-2
	pla
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
.elseif(n = 539)
	; 9 bytes
	php
	pha
	lda #2
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 542)
	; 11 bytes
	php
	pha
	lda $A5
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
.if(n = 547)
	; 11 bytes
	php
	pha
	lda $A5
	lda #2
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 548 || n = 550 || n = 553 || n = 555 || n = 560 || n = 561 || n = 566 || n = 571 || n = 573 || n = 576 || n = 580 || n = 581 || n = 588 || n = 596 || n = 601 || n = 606 || n = 608 || n = 611 || n = 616 || n = 618 || n = 623)
	; 11..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 551)
	; 11 bytes
	php
	pha
	php
	lda #2
	jsr delay_256a_16_clocks_b
	plp
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
.elseif(n = 556)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 558)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 563 || n = 591 || n = 598)
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
.elseif(n = 565 || n = 600 || n = 621)
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
.elseif(n = 568)
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
.elseif(n = 572 || n = 577 || n = 587 || n = 592 || n = 612)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 575 || n = 590 || n = 615)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
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
.elseif(n = 585 || n = 625)
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
.elseif(n = 586 || n = 593)
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
.elseif(n = 595 || n = 613)
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
.elseif(n = 610 || n = 620)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-5
	sbc #1
	Jpl *-2
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
.elseif(n = 628 || n = 635 || n = 663 || n = 698)
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
.elseif(n = 630 || n = 631 || n = 636 || n = 641 || n = 646 || n = 648 || n = 651 || n = 666 || n = 671 || n = 676 || n = 678 || n = 681 || n = 686 || n = 688 || n = 693 || n = 700 || n = 701)
	; 12..13 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 643 || n = 683)
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
.elseif(n = 645 || n = 650 || n = 655)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-5
	sbc #1
	Jpl *-2
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
.elseif((n .mod 5 = 2) && (n = 657 || (n > 666 && n < 698)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 658 || n = 685)
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
.elseif(n = 660 || n = 680 || n = 690 || n = 695)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
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
.elseif(n = 705 || n = 726 || n = 733 || n = 761 || n = 768 || n = 775)
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
.elseif(n = 706 || n = 708 || n = 711 || n = 716 || n = 718 || n = 721 || n = 736 || n = 741 || n = 746 || n = 751 || n = 756 || n = 758 || n = 770 || n = 771 || n = 776 || n = 778 || n = 781)
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
.elseif(n = 710 || n = 731 || n = 738)
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
.elseif(n = 715 || n = 723 || n = 755)
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
.elseif(n = 720 || n = 725 || n = 750 || n = 760 || n = 765)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 728)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 730 || n = 748)
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
.elseif(n = 735)
	; 13 bytes
	php
	pha
	clc
	lda #184 ;hides 'clv'
	bit $24
	adc #1
	Jne *-5
	pla
	plp
.elseif(n = 740)
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
.elseif(n = 763)
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
; 782 to 937
.if((n .mod 5 = 2) && ((n > 781 && n < 793) || (n > 811 && n < 938)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 783 || n = 786 || n = 788 || n = 791 || n = 797 || n = 799 || n = 801 || n = 802 || n = 804 || n = 806 || n = 811 || n = 813 || n = 816 || n = 821 || n = 826 || n = 828 || n = 840 || n = 841 || n = 846 || n = 848 || n = 851 || n = 853 || n = 856 || n = 858 || n = 861 || n = 876 || n = 881 || n = 886 || n = 888 || n = 891 || n = 896 || n = 898 || n = 911 || n = 916 || n = 918 || n = 921 || n = 926 || n = 928 || n = 931 || n = 933)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 4) && ((n > 783 && n < 795) || (n > 808 && n < 935)))
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
.elseif(n = 785 || n = 825)
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
.elseif(n = 790 || n = 830 || n = 855 || n = 860 || n = 870 || n = 890 || n = 895 || n = 900 || n = 925 || n = 930 || n = 935)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 793 || n = 820 || n = 865 || n = 910)
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
	; 9 bytes
	php
	pha
	lda #3
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 796 || n = 838 || n = 866 || n = 901 || n = 936)
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
.elseif(n = 798)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 803)
	; 11 bytes
	php
	pha
	lda $A5
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 805)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #3
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 807)
	; 11 bytes
	php
	pha
	php
	lda #3
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 808 || n = 836 || n = 843 || n = 871 || n = 878 || n = 885 || n = 906 || n = 920)
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
.elseif(n = 810 || n = 831 || n = 845 || n = 873 || n = 880 || n = 908 || n = 915)
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
.elseif(n = 815 || n = 850 || n = 913)
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
	tya
	ldy #(n-31)/12
	delay_n 7
	dey
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 833 || n = 905)
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
.elseif(n = 835 || n = 875)
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
.elseif(n = 868 || n = 903)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.endif
.endif
.else
.if((n >= 938) && (n < 1094))
.if((n >= 938) && (n < 1016))
; 938 to 1015
.if(n = 938 || n = 973 || n = 1008)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 940 || n = 960 || n = 965 || n = 970 || n = 975 || n = 1005 || n = 1010)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 941 || n = 948 || n = 955 || n = 983 || n = 990 || n = 1011)
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
.elseif(n = 943 || n = 950 || n = 971 || n = 978 || n = 985 || n = 1006 || n = 1013)
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
.elseif(n = 945 || n = 993)
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
.elseif(n = 946 || n = 951 || n = 956 || n = 958 || n = 961 || n = 966 || n = 968 || n = 981 || n = 986 || n = 988 || n = 991 || n = 996 || n = 998 || n = 1001)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 953)
	; 12 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	nop
	iny
	Jpl *-3
	tay
	pla
	plp
.elseif(n = 963 || n = 995 || n = 1003)
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
.elseif(n = 1000)
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
.elseif(n = 1015)
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
.endif
.else
; 1016 to 1093
.if(n = 1016 || n = 1021 || n = 1023 || n = 1026 || n = 1028 || n = 1031 || n = 1036 || n = 1038 || n = 1050 || n = 1053 || n = 1055 || n = 1058 || n = 1060 || n = 1065 || n = 1066 || n = 1071 || n = 1086 || n = 1091 || n = 1093)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 5 = 2) && ((n > 1016 && n < 1028) || (n > 1036 && n < 1093)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1018 || n = 1025 || n = 1046 || n = 1081)
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
.elseif((n .mod 5 = 4) && ((n > 1018 && n < 1050) || (n > 1063 && n < 1090)))
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
.elseif(n = 1020 || n = 1041 || n = 1048 || n = 1076 || n = 1083 || n = 1090)
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
.elseif(n = 1030 || n = 1040 || n = 1080)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
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
.elseif(n = 1035)
	; 12 bytes
	php
	pha
	lda $A5
	txa
	ldx #202 ;hides 'dex'
	Jne *-1
	tax
	pla
	plp
.elseif(n = 1043)
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
.elseif(n = 1045)
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
.elseif(n = 1051)
	; 9 bytes
	php
	pha
	lda #4
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1054)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 1059)
	; 11 bytes
	php
	pha
	lda $A5
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1061)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #4
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1063)
	; 11 bytes
	php
	pha
	php
	lda #4
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 1068)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1070)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif(n = 1075)
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
.elseif(n = 1078)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1085)
	; 13 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	bit $24
	sbc #1
	Jmi *-5
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
.elseif(n = 1096 || n = 1098 || n = 1101 || n = 1103 || n = 1106 || n = 1108 || n = 1120 || n = 1121 || n = 1126 || n = 1128 || n = 1131 || n = 1136 || n = 1138 || n = 1141 || n = 1143 || n = 1156 || n = 1161 || n = 1166 || n = 1168 || n = 1171 || n = 1173 || n = 1176 || n = 1190 || n = 1191 || n = 1196 || n = 1198 || n = 1201 || n = 1206 || n = 1208 || n = 1211 || n = 1213 || n = 1226 || n = 1231 || n = 1236 || n = 1238 || n = 1241 || n = 1243 || n = 1246 || n = 1248)
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
.elseif((n .mod 5 = 0) && (n = 1100 || n = 1110 || n = 1140 || n = 1150 || n = 1170 || n = 1175 || (n > 1204 && n < 1221) || n = 1240 || n = 1245))
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1105 || n = 1113 || n = 1145 || n = 1185 || n = 1233)
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
.elseif(n = 1115 || n = 1155 || n = 1163 || n = 1203)
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
.elseif(n = 1135 || n = 1180)
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
.elseif(n = 1148 || n = 1218)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 1183)
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
.elseif(n = 1225)
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
.if((n >= 1251) && (n < 1290))
; 1251 to 1289
.if(n = 1251 || n = 1258 || n = 1265 || n = 1286)
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
.elseif((n .mod 5 = 2) && ((n > 1251 && n < 1288)))
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1253)
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
.elseif((n .mod 5 = 4) && ((n > 1253 && n < 1290)))
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
.elseif(n = 1255 || n = 1280 || n = 1285)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1256 || n = 1263 || n = 1270)
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
.elseif(n = 1260 || n = 1261 || n = 1266 || n = 1271 || n = 1276 || n = 1278 || n = 1281)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1268)
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
.elseif(n = 1273)
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
.elseif(n = 1275 || n = 1283)
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
.elseif(n = 1288)
	; 13 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
.if((n >= 1290) && (n < 1309))
; 1290 to 1308
.if(n = 1290)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #n/5-4
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1291 || n = 1298)
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
.elseif(n = 1292)
	; 11 bytes
	php
	pha
	sec
	lda #(n-17)/5
	sbc #1
	Jne *-2
	pla
	plp
.elseif(n = 1293 || n = 1300)
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
.elseif(n = 1294)
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
.elseif(n = 1295)
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
.elseif(n = 1296 || n = 1301)
	; 12 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 1302)
	; 13 bytes
	php
	pha
	lda $A5
	tya
	ldy #0
	dey
	Jne *-1
	tay
	pla
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
.elseif(n = 1306)
	; 13 bytes
	php
	pha
	tya
	pha
	ldy #0
	dey
	Jne *-1
	pla
	tay
	pla
	plp
.elseif(n = 1307)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1308)
	; 13 bytes
	php
	pha
	lda $A5
	tya
	ldy #184 ;hides 'clv'
	dey
	Jne *-2
	tay
	pla
	plp
.endif
.else
; 1309 to 1328
.if(n = 1309 || n = 1311 || n = 1313 || n = 1314 || n = 1316 || n = 1318 || n = 1321 || n = 1323)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1310)
	; 11 bytes
	php
	pha
	lda $A5
	lda #5
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1312)
	; 9 bytes
	php
	pha
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1315)
	; 11 bytes
	php
	pha
	lda $A5
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1317)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #5
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1319)
	; 11 bytes
	php
	pha
	php
	lda #5
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 1320)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1322)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 1324)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 1326)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif(n = 1328)
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
.endif
.endif
.endif
.else
; 1329 to 1406
.if((n .mod 8 = 1) && ((n > 1328 && n < 1354) || (n > 1384 && n < 1402)))
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
.elseif(n = 1330 || n = 1334 || n = 1344 || n = 1365 || n = 1372 || n = 1386 || n = 1400)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1331 || n = 1339 || n = 1355 || n = 1371 || n = 1379 || n = 1395)
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
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6EA0'
	ldy #110
	iny
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1332 && n < 1404)))
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
.elseif((n .mod 7 = 5) && ((n > 1334 && n < 1406)))
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
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1338)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.elseif(n = 1343 || n = 1352 || n = 1357 || n = 1362 || n = 1364 || n = 1366 || n = 1374 || n = 1380 || n = 1383 || n = 1388 || n = 1390 || n = 1392 || n = 1402 || n = 1406)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
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
.elseif(n = 1350)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6CA0'
	ldy #108
	iny
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 9 = 1) && ((n > 1350 && n < 1388)))
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
.elseif(n = 1358 || n = 1367 || n = 1376)
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
.elseif(n = 1359)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $6BA0'
	ldy #107
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 1404)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $66A0'
	ldy #102
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 1407) && (n < 1485))
; 1407 to 1484
.if(n = 1407 || n = 1428 || n = 1442 || n = 1456 || n = 1463 || n = 1470)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1408 || n = 1415 || n = 1416 || n = 1418 || n = 1429 || n = 1434 || n = 1436 || n = 1437 || n = 1444 || n = 1446 || n = 1453 || n = 1455 || n = 1460 || n = 1462 || n = 1464 || n = 1472 || n = 1474 || n = 1478 || n = 1479)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1409 || n = 1425 || n = 1449 || n = 1457 || n = 1465 || n = 1481)
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
.elseif((n .mod 7 = 3) && (n = 1410 || n = 1424 || n = 1431 || (n > 1444 && n < 1481)))
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
.elseif((n .mod 8 = 3) && (n = 1411 || (n > 1426 && n < 1452) || n = 1467 || n = 1483))
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
.elseif((n .mod 7 = 5) && ((n > 1411 && n < 1483)))
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
.elseif((n .mod 9 = 1) && ((n > 1413 && n < 1451) || n = 1477))
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
.elseif(n = 1420)
	; 13 bytes
	php
	pha
	lda $A5
	tya
	ldy #56 ;hides 'sec'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif((n .mod 9 = 8) && ((n > 1420 && n < 1449) || n = 1484))
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
.elseif(n = 1422)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $64A0'
	ldy #100
	iny
	Jne *-4
	tay
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
.endif
.else
; 1485 to 1562
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
.elseif((n .mod 9 = 1) && ((n > 1485 && n < 1514) || (n > 1539 && n < 1559)))
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
.elseif((n .mod 7 = 3) && ((n > 1486 && n < 1523) || (n > 1535 && n < 1558)))
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
.elseif(n = 1488 || n = 1490 || n = 1492 || n = 1500 || n = 1506 || n = 1509)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1488 && n < 1560)))
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
.elseif(n = 1491 || n = 1499 || n = 1507 || n = 1523 || n = 1539 || n = 1547 || n = 1555)
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
.elseif((n .mod 9 = 8) && ((n > 1492 && n < 1521) || n = 1556))
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
.elseif(n = 1497 || n = 1505 || n = 1521 || n = 1537 || n = 1553 || n = 1561)
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
.elseif(n = 1498 || n = 1512 || n = 1514 || n = 1525 || n = 1526 || n = 1528 || n = 1533 || n = 1551 || n = 1554 || n = 1560)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1516)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1518 || n = 1546)
	; 14 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/7
	nop
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1519)
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
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #216 ;hides 'cld'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1532)
	; 13 bytes
	php
	pha
	lda $A5
	tya
	ldy #216 ;hides 'cld'
	dey
	Jne *-2
	tay
	pla
	plp
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
.elseif(n = 1535)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1541)
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
.elseif(n = 1542)
	; 14 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 1544)
	; 15 bytes
	php
	pha
	php
	sec
	lda #n/8-3
	bit $24
	sbc #1
	Jne *-4
	plp
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
.elseif(n = 1562)
	; 14 bytes
	php
	pha
	lda $A5
	tya
	ldy #220
	nop
	dey
	Jne *-2
	tay
	pla
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
.elseif((n .mod 7 = 3) && (n = 1564 || (n > 1577 && n < 1635)))
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
.elseif(n = 1565 || n = 1567 || n = 1569 || n = 1570 || n = 1572 || n = 1574 || n = 1577 || n = 1579 || n = 1581 || n = 1584 || n = 1588 || n = 1596 || n = 1604 || n = 1624 || n = 1631 || n = 1638)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 1566)
	; 11 bytes
	php
	pha
	lda $A5
	lda #6
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1568)
	; 9 bytes
	php
	pha
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1571)
	; 11 bytes
	php
	pha
	lda $A5
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1573)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #6
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1575)
	; 11 bytes
	php
	pha
	php
	lda #6
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 1576)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 1580)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 1582)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1583 || n = 1610 || n = 1637)
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
.elseif((n .mod 7 = 5) && ((n > 1586 && n < 1637)))
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
.elseif(n = 1589)
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
.elseif(n = 1590)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
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
.elseif((n .mod 8 = 1) && (n = 1593 || (n > 1608 && n < 1634)))
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
.elseif(n = 1595 || n = 1611 || n = 1619 || n = 1635)
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
.elseif(n = 1597 || n = 1598 || n = 1600 || n = 1605 || n = 1614 || n = 1616 || n = 1618 || n = 1623 || n = 1626 || n = 1632 || n = 1640)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1602)
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
.elseif((n .mod 9 = 1) && ((n > 1602 && n < 1631)))
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
.elseif(n = 1607)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.else
; 1641 to 1718
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
.elseif(n = 1642)
	; 13 bytes
	php
	pha
	lda $A5
	clc
	lda #24 ;hides 'clc'
	adc #1
	Jne *-3
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1642 && n < 1714)))
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
.elseif(n = 1644)
	; 13 bytes
	php
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	iny
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1645 || n = 1652 || n = 1680 || n = 1694 || n = 1701 || n = 1708)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1646 || n = 1682 || n = 1709)
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
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B5A0'
	ldy #181
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 7 = 3) && (n = 1648 || (n > 1661 && n < 1719)))
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
.elseif(n = 1649 || n = 1665 || n = 1681 || n = 1689 || n = 1705)
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
.elseif(n = 1651 || n = 1659 || n = 1667 || n = 1691 || n = 1707 || n = 1715)
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
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1654 || n = 1668 || n = 1670 || n = 1672 || n = 1677 || n = 1679 || n = 1686 || n = 1688 || n = 1695 || n = 1696 || n = 1698 || n = 1703 || n = 1712 || n = 1714 || n = 1716 || n = 1717)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1655)
	; 11 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
	plp
.elseif(n = 1656)
	; 13 bytes
	php
	pha
	lda $A5
	sec
	lda #234 ;hides 'nop'
	sbc #1
	Jne *-3
	pla
	plp
.elseif(n = 1658)
	; 13 bytes
	php
	pha
	lda $A5
	tya
	ldy #234 ;hides 'nop'
	dey
	Jne *-2
	tay
	pla
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
.elseif(n = 1663 || n = 1687)
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
.elseif((n .mod 9 = 1) && ((n > 1665 && n < 1703)))
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
.elseif(n = 1710)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BCA0'
	ldy #188
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 1719) && (n < 1797))
; 1719 to 1796
.if(n = 1719)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $BDA0'
	ldy #189
	dey
	Jne *-4
	tay
	pla
	plp
.elseif((n .mod 7 = 5) && ((n > 1719 && n < 1791)))
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
.elseif(n = 1721 || n = 1737 || n = 1745 || n = 1761 || n = 1777 || n = 1785 || n = 1793)
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
.elseif(n = 1722 || n = 1743 || n = 1750 || n = 1764 || n = 1775 || n = 1780 || n = 1796)
	; 13..15 bytes
	delay_n (n-2)
	nop
.elseif((n .mod 8 = 3) && (n = 1723 || n = 1731 || (n > 1762 && n < 1788)))
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
.elseif(n = 1724 || n = 1726 || n = 1730 || n = 1740 || n = 1742 || n = 1744 || n = 1749 || n = 1752 || n = 1766 || n = 1768)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n .mod 7 = 3) && ((n > 1724 && n < 1747) || (n > 1759 && n < 1796)))
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
.elseif((n .mod 9 = 1) && ((n > 1728 && n < 1766) || n = 1792))
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
.elseif(n = 1733 || n = 1757)
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
.elseif(n = 1735 || n = 1759)
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
.elseif(n = 1736 || n = 1754 || n = 1772)
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
.elseif(n = 1758)
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
.elseif(n = 1773)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $3DA0'
	ldy #61
	iny
	Jne *-4
	tay
	pla
	plp
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
.elseif(n = 1782)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $C4A0'
	ldy #196
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1784)
	; 14 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/7
	nop
	sbc #1
	Jne *-3
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
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
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
	; 14 bytes
	delay_n (n-4)
	delay_n 4
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
	; 14 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 8
	Jne *-5
	tay
	pla
	plp
.elseif(n = 1799)
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
.elseif(n = 1806 || n = 1813 || n = 1815)
	; 13..14 bytes
	delay_n (n-2)
	nop
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
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1814)
	; 14 bytes
	php
	pha
	lda $A5
	tya
	ldy #0
	nop
	dey
	Jne *-2
	tay
	pla
	plp
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
.elseif(n = 1819)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1820)
	; 14 bytes
	php
	pha
	lda $A5
	tya
	ldy #56 ;hides 'sec'
	nop
	iny
	Jne *-3
	tay
	pla
	plp
.elseif(n = 1821 || n = 1823 || n = 1825 || n = 1826 || n = 1828 || n = 1830 || n = 1833 || n = 1835)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1822)
	; 11 bytes
	php
	pha
	lda $A5
	lda #7
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1824)
	; 9 bytes
	php
	pha
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1827)
	; 11 bytes
	php
	pha
	lda $A5
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1829)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1831)
	; 11 bytes
	php
	pha
	php
	lda #7
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 1832)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
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
; 1836 to 1875
.if(n = 1836)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif((n .mod 9 = 1) && ((n > 1836 && n < 1874)))
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
.elseif(n = 1838)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 1839)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1840 || n = 1852)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 1841 || n = 1849 || n = 1865)
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
.elseif((n .mod 8 = 3) && ((n > 1842 && n < 1876)))
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
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $35A0'
	ldy #53
	iny
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1847)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 1848)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
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
.elseif(n = 1853 || n = 1862 || n = 1871)
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
.elseif(n = 1854)
	; 13 bytes
	php
	pha
	txa
	ldx #204 ;hides 'cpy abs'
	cmp $C5
	dex
	Jne *-4
	tax
	pla
	plp
.elseif(n = 1856 || n = 1860 || n = 1861 || n = 1866 || (n > 1867 && n < 1871) || n = 1874)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1857)
	; 13 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 1858)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 1863)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
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
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 1876) && (n < 2188))
.if((n >= 1876) && (n < 2032))
.if((n >= 1876) && (n < 1954))
; 1876 to 1953
.if(n = 1876 || n = 1878 || (n > 1883 && n < 1889) || (n > 1891 && n < 1897) || n = 1902 || n = 1904 || n = 1906 || (n > 1909 && n < 1913) || n = 1914 || n = 1919 || n = 1920 || n = 1922 || n = 1928 || n = 1930 || n = 1932 || n = 1933 || n = 1938 || (n > 1939 && n < 1943) || n = 1946 || n = 1948 || n = 1950)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 1877 || n = 1901 || n = 1949)
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
.elseif(n = 1879 || n = 1903 || n = 1951)
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
.elseif(n = 1880 || n = 1898 || n = 1916 || n = 1952)
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
.elseif((n .mod 8 = 1) && ((n > 1880 && n < 1938) || n = 1953))
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
.elseif((n .mod 9 = 1) && ((n > 1881 && n < 1946)))
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
.elseif((n .mod 8 = 3) && (n = 1883 || (n > 1898 && n < 1948)))
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
.elseif(n = 1917)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $2DA0'
	ldy #45
	iny
	Jne *-4
	tay
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
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	inx
	Jne *-4
	tax
	pla
	plp
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
.endif
.else
; 1954 to 2031
.if((n .mod 9 = 1) && ((n > 1953 && n < 2027)))
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
.elseif((n .mod 8 = 3) && (n = 1955 || (n > 1970 && n < 2028)))
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
.elseif((n > 1955 && n < 1959) || n = 1960 || (n > 1963 && n < 1968) || n = 1974 || n = 1976 || (n > 1981 && n < 1985) || n = 1991 || n = 1992 || n = 1994 || n = 1996 || n = 2000 || n = 2002 || n = 2004 || n = 2005 || n = 2010 || (n > 2011 && n < 2015) || n = 2018 || n = 2020 || n = 2022)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
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
.elseif((n .mod 8 = 1) && ((n > 1968 && n < 2010) || n = 2025))
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
.elseif((n .mod 9 = 8) && (n = 1970 || (n > 1996 && n < 2025)))
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
.elseif(n = 1973 || n = 2021)
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
.elseif(n = 1975 || n = 2023)
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
	cmp <$CD ;hides 'cmp $DBA0'
	ldy #219
	dey
	Jne *-4
	tay
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
.elseif(n = 2007)
	; 13 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $23A0'
	ldy #35
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 2028)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2029)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2030)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2031)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B7A0'
	ldy #183
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 2032) && (n < 2110))
.if((n >= 2032) && (n < 2071))
.if((n >= 2032) && (n < 2051))
; 2032 to 2050
.if(n = 2032)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
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
.elseif(n = 2036 || n = 2046)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2037)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2038)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/9
	delay_n 4
	dey
	Jne *-3
	tay
	pla
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
	php
	pha
	sec
	lda #n/17-1
	jsr @nearby_rts
	sbc #1
	Jne *-5
	pla
	plp
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
	; 14 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	jsr @nearby_rts
	sbc #1
	Jmi *-6
	pla
	plp
.elseif(n = 2050)
	; 14 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	jsr @nearby_rts
	dey
	Jmi *-5
	tay
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
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2055 || n = 2064)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2056)
	; 15 bytes
	php
	pha
	php
	sec
	lda #n/8-3
	bit $24
	sbc #1
	Jne *-4
	plp
	pla
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
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/13
	delay_n 8
	sbc #1
	Jne *-6
	pla
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
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
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
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #255
	bit $24
	dey
	Jne *-3
	pla
	tay
	pla
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
	; 15 bytes
	php
	pha
	lda $A5
	sec
	lda #0
	bit $24
	sbc #1
	Jne *-4
	pla
	plp
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
.if(n = 2071)
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
	delay_n 10
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2073 || n = 2077 || n = 2079 || n = 2081 || n = 2082 || n = 2084 || n = 2086 || n = 2089)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2074)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
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
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2078)
	; 11 bytes
	php
	pha
	lda $A5
	lda #8
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2080)
	; 9 bytes
	php
	pha
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2083)
	; 11 bytes
	php
	pha
	lda $A5
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2085)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 2090 to 2109
.if(n = 2090)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2091 || n = 2100)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2092)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	inx
	Jne *-4
	tax
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
.elseif(n = 2099)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2101)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2102)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/15
	rol $26
	ror $26
	sbc #1
	Jne *-6
	pla
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
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
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
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2109)
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
; 2110 to 2187
.if((n > 2109 && n < 2114) || n = 2118 || n = 2120 || n = 2122 || (n > 2125 && n < 2129) || n = 2130 || (n > 2134 && n < 2141) || (n > 2143 && n < 2150) || n = 2154 || (n > 2155 && n < 2159) || (n > 2161 && n < 2165) || n = 2166 || (n > 2170 && n < 2175) || n = 2176 || (n > 2179 && n < 2186))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n .mod 9 = 8) && (n = 2114 || (n > 2131 && n < 2187)))
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
.elseif((n .mod 9 = 1) && ((n > 2115 && n < 2180)))
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
.elseif(n = 2117 || n = 2129 || n = 2153 || n = 2165)
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
.elseif(n = 2119 || n = 2131 || n = 2155 || n = 2167)
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
	lda #234 ;hides 'nop'
	nop
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2123)
	; 12 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
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
.elseif(n = 2169)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $EFA0'
	ldy #239
	dey
	Jne *-4
	tay
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
.elseif(n = 2187)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0FA0'
	ldy #15
	iny
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2188) && (n < 2344))
.if((n >= 2188) && (n < 2266))
; 2188 to 2265
.if((n .mod 9 = 1) && ((n > 2187 && n < 2261)))
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
.elseif(n = 2189 || n = 2201 || n = 2225 || n = 2237 || n = 2261)
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
.elseif(n = 2190 || (n > 2191 && n < 2195) || (n > 2197 && n < 2201) || n = 2202 || n = 2207 || (n > 2208 && n < 2213) || (n > 2218 && n < 2222) || n = 2226 || n = 2229 || n = 2230 || (n > 2233 && n < 2237) || n = 2238 || (n > 2242 && n < 2247) || n = 2248 || (n > 2251 && n < 2258) || n = 2262 || n = 2264 || n = 2265)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2191 || n = 2203 || n = 2227 || n = 2239 || n = 2263)
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
.elseif((n .mod 9 = 8) && ((n > 2194 && n < 2241)))
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
.elseif(n = 2208)
	; 13 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	iny ;first loop only
	nop
	iny
	Jne *-4
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
	; 13 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 4
	iny
	Jne *-4
	tay
	pla
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
.elseif(n = 2223)
	; 13 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	iny
	Jne *-4
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
	; 13 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 6
	Jne *-4
	tax
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
.endif
.else
.if((n >= 2266) && (n < 2305))
.if((n >= 2266) && (n < 2285))
; 2266 to 2284
.if(n = 2266 || (n > 2269 && n < 2273) || n = 2274 || n = 2279 || n = 2280)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2267)
	; 13 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $06A0'
	ldy #6
	iny
	Jne *-4
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
.elseif(n = 2281)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2282)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #8
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2283)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2284)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	nop
	iny
	Jne *-5
	tay
	pla
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
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2290 || n = 2291 || n = 2300)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 2292)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/13
	delay_n 8
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2293)
	; 15 bytes
	php
	pha
	tya
	pha
	bit <$2C ;hides 'bit $04A0'
	ldy #4
	iny
	Jne *-4
	pla
	tay
	pla
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
	; 15 bytes
	php
	pha
	tya
	pha
	bit <$2C ;hides 'bit $03A0'
	ldy #3
	iny
	Jne *-4
	pla
	tay
	pla
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
.elseif(n = 2306 || n = 2310)
	; 14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2307 || n = 2318)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
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
.elseif(n = 2316)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2317)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2319)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2320)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	delay_n 8
	dey
	Jne *-5
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
.if(n = 2324 || n = 2325 || n = 2333 || n = 2335 || n = 2337 || n = 2338 || n = 2340 || n = 2342)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 2326)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #0
	delay_n 4
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2327)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
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
.elseif(n = 2329)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2330)
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #0
	delay_n 4
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2331)
	; 9 bytes
	php
	pha
	lda #9
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2332)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $B2A0'
	ldy #178
	delay_n 4
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 2334)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 2339)
	; 11 bytes
	php
	pha
	lda $A5
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2341)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.endif
.else
; 2344 to 2500
.if(n = 2344)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2345 || n = 2347 || n = 2352)
	; 12..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2346)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2348)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2351)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2353)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2354)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
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
.elseif(n = 2356)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n .mod 12 = 7) && ((n > 2358 && n < 2492)))
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
.elseif((n > 2360 && n < 2365))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 2364 && n < 2369) || (n > 2371 && n < 2375) || (n > 2375 && n < 2381) || n = 2382 || (n > 2383 && n < 2391) || n = 2392 || n = 2394 || (n > 2395 && n < 2405) || n = 2406 || (n > 2407 && n < 2417) || n = 2418 || (n > 2419 && n < 2429) || n = 2430 || (n > 2431 && n < 2441) || n = 2442 || (n > 2443 && n < 2453) || n = 2454 || (n > 2455 && n < 2465) || n = 2466 || (n > 2467 && n < 2477) || n = 2478 || (n > 2479 && n < 2489) || n = 2490 || (n > 2491 && n < 2495) || n = 2496 || (n > 2497 && n < 2501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n .mod 12 = 5) && ((n > 2368 && n < 2442) || (n > 2464 && n < 2490)))
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
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2375)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
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
.elseif(n = 2502 || (n > 2503 && n < 2513) || n = 2514 || (n > 2515 && n < 2525) || n = 2526 || (n > 2527 && n < 2537))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 2538)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
.if((n >= 2540) && (n < 2559))
; 2540 to 2558
.if(n = 2540 || n = 2541 || n = 2543)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 2542)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2544)
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
.elseif(n = 2545)
	; 15 bytes
	php
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
	plp
.elseif(n = 2546)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2547 || n = 2557)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/10
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2548)
	; 14 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	nop
	inx
	Jne *-5
	tax
	pla
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
	tya
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2553)
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
.elseif(n = 2554)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2555)
	; 15 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 2556)
	; 14 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	jsr @nearby_rts_14cyc
	sbc #1
	Jmi *-6
	pla
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
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/10
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2560 || n = 2571)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
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
	; 15 bytes
	delay_n (n-2)
	nop
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
	; 15 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
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
.elseif(n = 2566 || n = 2578)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
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
	; 15 bytes
	php
	pha
	php
	sec
	lda #n/12-2
	delay_n 7
	sbc #1
	Jne *-4
	plp
	pla
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
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2572)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2574)
	; 15 bytes
	php
	pha
	php
	clc
	lda #24 ;hides 'clc'
	delay_n 4
	adc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 2576)
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	delay_n 4
	iny
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 2577)
	; 14 bytes
	php
	pha
	sec
	lda #0
	nop
	bit $24
	sbc #1
	Jne *-5
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 2579) && (n < 2618))
.if((n >= 2579) && (n < 2598))
; 2579 to 2597
.if(n = 2579)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	nop
	bit $24
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2580)
	; 14 bytes
	php
	pha
	sec
	lda #(n-17)/11
	delay_n 6
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 2581)
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
.elseif(n = 2582)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2583 || n = 2589 || n = 2591 || n = 2593 || n = 2594 || n = 2596)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 2584)
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
.elseif(n = 2585)
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
.elseif(n = 2587)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2588)
	; 15 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 2590)
	; 11 bytes
	php
	pha
	lda $A5
	lda #10
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2592)
	; 9 bytes
	php
	pha
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2595)
	; 11 bytes
	php
	pha
	lda $A5
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2597)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2598 to 2617
.if(n = 2598 || n = 2601 || n = 2603 || n = 2605 || n = 2608)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 2600)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2602)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2604)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 2606)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 2607)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts
	lda #10
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2611)
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
.elseif(n = 2612)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2613)
	; 14 bytes
	delay_n (n-7)
	delay_n 7
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
.elseif(n = 2615)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2616)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2617)
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
; 2618 to 2656
.if(n = 2618 || n = 2619)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2620)
	; 13 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	jsr @nearby_rts_14cyc
	Jne *-4
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
.elseif(n = 2622 || n = 2624 || n = 2625 || (n > 2626 && n < 2631) || n = 2632 || n = 2634 || (n > 2635 && n < 2645) || n = 2646 || (n > 2647 && n < 2657))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 2626)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2631)
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
; 2657 to 2812
.if((n .mod 12 = 5) && ((n > 2656 && n < 2802)))
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
.elseif(n = 2658 || (n > 2659 && n < 2669) || n = 2670 || (n > 2671 && n < 2681) || n = 2682 || (n > 2683 && n < 2693) || n = 2694 || (n > 2695 && n < 2705) || n = 2706 || (n > 2707 && n < 2717) || n = 2718 || (n > 2719 && n < 2729) || n = 2730 || (n > 2731 && n < 2741) || n = 2742 || n = 2744 || (n > 2745 && n < 2753) || n = 2754 || (n > 2755 && n < 2765) || n = 2766 || (n > 2767 && n < 2777) || n = 2778 || (n > 2779 && n < 2789) || n = 2790 || n = 2792)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n .mod 12 = 7) && ((n > 2658 && n < 2804)))
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
.elseif(n = 2793 || n = 2795 || n = 2804)
	; 14..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 2794)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2796 || n = 2808)
	; 15 bytes
	php
	pha
	php
	sec
	lda #n/12-2
	delay_n 7
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 2797)
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
.elseif(n = 2798)
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/12
	delay_n 7
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2799)
	; 14 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	delay_n 4
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2800)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $03A0'
	ldy #3
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2802)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
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
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2807)
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
.elseif(n = 2809)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2810)
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
.elseif(n = 2811)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2812)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 2813) && (n < 2969))
.if((n >= 2813) && (n < 2891))
.if((n >= 2813) && (n < 2852))
; 2813 to 2851
.if(n = 2813 || n = 2825 || n = 2837)
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
.elseif(n = 2814 || n = 2840)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2815 || n = 2827 || n = 2839)
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
	; 15 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
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
.elseif(n = 2818 || n = 2830 || n = 2842)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 2819 || n = 2833)
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
.elseif(n = 2820 || n = 2832 || n = 2844)
	; 15 bytes
	php
	pha
	php
	sec
	lda #n/12-2
	delay_n 7
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 2821 || n = 2826 || n = 2829 || n = 2836 || n = 2841 || n = 2845 || n = 2847 || n = 2849 || n = 2850)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 2822)
	; 14 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2823)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2824)
	; 14 bytes
	php
	pha
	tya
	ldy #(n-19)/11
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2828)
	; 14 bytes
	php
	pha
	tya
	ldy #201 ;hides 'cmp #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2831)
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
.elseif(n = 2834)
	; 14 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 2835)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	delay_n 6
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 2838)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/13
	delay_n 8
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 2843)
	; 9 bytes
	php
	pha
	lda #11
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2846)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 2851)
	; 11 bytes
	php
	pha
	lda $A5
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 2852 to 2890
.if(n = 2852 || n = 2854 || n = 2857 || n = 2859 || n = 2864)
	; 11..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 2853)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 2856)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 2858)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 2860)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
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
.elseif(n = 2862)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
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
.elseif(n = 2865)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #11
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2866)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2867)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 2868)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2869)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2870)
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/12
	delay_n 7
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 2871)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2872)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2874 || n = 2876)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 2876 && n < 2882) || n = 2883 || n = 2884 || n = 2886 || (n > 2887 && n < 2891))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 2882)
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
; 2891 to 2968
.if((n > 2890 && n < 2897) || n = 2898 || (n > 2899 && n < 2909) || n = 2910 || (n > 2911 && n < 2921) || n = 2922 || (n > 2923 && n < 2933) || n = 2934 || (n > 2935 && n < 2945) || n = 2946 || (n > 2947 && n < 2957) || n = 2958 || (n > 2959 && n < 2969))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 2970 || (n > 2971 && n < 2981) || n = 2982 || (n > 2983 && n < 2993) || n = 2994 || (n > 2995 && n < 3005) || n = 3006 || (n > 3007 && n < 3017) || n = 3018 || (n > 3019 && n < 3029) || n = 3030 || (n > 3031 && n < 3039) || n = 3040 || n = 3042 || (n > 3043 && n < 3047))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.if(n = 3047 || n = 3048)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3049)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-244)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3050)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-245)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3051 || n = 3052)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $03A0'
	ldy #3
	cmp $C5
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3056 || n = 3068 || n = 3080)
	; 15 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/12
	delay_n 7
	sbc #1
	Jne *-4
	pla
	plp
.elseif(n = 3057 || n = 3071 || n = 3085)
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
.elseif(n = 3058 || n = 3070 || n = 3082)
	; 15 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/12
	delay_n 7
	dey
	Jne *-3
	tay
	pla
	plp
.elseif(n = 3059)
	; 14 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 6
	dey
	Jne *-5
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
.elseif(n = 3061)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3062)
	; 14 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 10
	Jne *-5
	tax
	pla
	plp
.elseif(n = 3063 || n = 3081)
	; 14..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 3064)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3066)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	cmp $C5
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3069 || n = 3083)
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
.elseif(n = 3072 || n = 3084)
	; 15 bytes
	php
	pha
	php
	sec
	lda #n/12-2
	delay_n 7
	sbc #1
	Jne *-4
	plp
	pla
	plp
.elseif(n = 3073)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/16
	delay_n 11
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3074)
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/12
	delay_n 7
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 3075)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3076)
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
.elseif(n = 3078)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.else
.if((n >= 3086) && (n < 3106))
; 3086 to 3105
.if(n = 3086)
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/12
	delay_n 7
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 3087 || n = 3100)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3088 || n = 3093 || n = 3101 || n = 3103 || n = 3105)
	; 10..16 bytes
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
.elseif(n = 3090)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	cmp $C5
	dey
	Jne *-6
	tay
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
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/15
	rol $26
	ror $26
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3094)
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
.elseif(n = 3095)
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
.elseif(n = 3096)
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
.elseif(n = 3097)
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
	; 15 bytes
	php
	pha
	tya
	pha
	ldy #0
	delay_n 7
	dey
	Jne *-3
	pla
	tay
	pla
	plp
.elseif(n = 3099)
	; 9 bytes
	php
	pha
	lda #12
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3102)
	; 11 bytes
	php
	pha
	lda $A5
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
.endif
.else
; 3106 to 3125
.if(n = 3106 || n = 3108 || n = 3110 || n = 3113 || n = 3115 || n = 3117 || n = 3120)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3107)
	; 11 bytes
	php
	pha
	lda $A5
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3109)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 3112)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3114)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3116)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3118)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3119)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3121)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #12
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3122)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3123)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3124)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3125)
	; 14 bytes
	delay_n (n-7)
	delay_n 7
.endif
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 3126) && (n < 3438))
.if((n >= 3126) && (n < 3282))
; 3126 to 3281
.if(n = 3126)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3127)
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
.elseif(n = 3128)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 3128 && n < 3133))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3132 && n < 3138) || (n > 3138 && n < 3143) || (n > 3143 && n < 3263) || n = 3264 || (n > 3265 && n < 3282))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 3143)
	; 13 bytes
	php
	pha
	lda #<(n+3) ;hides 'lsr a'
	Jne *-1
	lda #>(n+3)
	jsr delay_256a_16_clocks_b
	pla
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
.endif
.else
.if((n >= 3282) && (n < 3360))
.if((n >= 3282) && (n < 3321))
; 3282 to 3320
.if((n > 3281 && n < 3291) || n = 3292 || (n > 3293 && n < 3305))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 3293)
	; 13 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 7
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3305)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3306)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
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
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/13
	delay_n 8
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3309)
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
.elseif(n = 3310)
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
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3313)
	; 14 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3314)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $32A0'
	ldy #50
	delay_n 7
	iny
	Jne *-6
	tay
	pla
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
.elseif(n = 3317)
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
.elseif(n = 3319)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3320)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 4
	iny
	Jne *-6
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
.elseif(n = 3322 || n = 3336 || n = 3339 || n = 3341 || n = 3343 || n = 3348 || n = 3352 || n = 3353 || n = 3357 || n = 3359)
	; 10..17 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 3324 || n = 3338)
	; 16 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3325 || n = 3344)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3326 || n = 3354)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3327)
	; 15 bytes
	php
	pha
	sec
	lda #184 ;hides 'clv'
	delay_n 11
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 3328)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 3329)
	; 15 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3330)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3331)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3332)
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
	ldy #(n-19)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3340)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	delay_n 7
	iny
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 3342)
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
.elseif(n = 3345)
	; 15 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3346)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 4
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3347)
	; 15 bytes
	php
	pha
	tya
	ldy #0
	delay_n 8
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3350)
	; 15 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3355)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3356)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3358)
	; 11 bytes
	php
	pha
	lda $A5
	lda #13
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 3360 to 3437
.if(n = 3360)
	; 9 bytes
	php
	pha
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3361 || n = 3362 || n = 3364 || n = 3366 || n = 3369 || n = 3371 || n = 3373 || n = 3376)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3363)
	; 11 bytes
	php
	pha
	lda $A5
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3365)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 3368)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3370)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3372)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3374)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3375)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3377)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3378)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3379)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3380)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3381)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3382)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3383)
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
.elseif((n > 3384 && n < 3389))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3388 && n < 3394) || (n > 3394 && n < 3399) || (n > 3399 && n < 3438))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3394)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3399)
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
.if((n >= 3438) && (n < 3594))
.if((n >= 3438) && (n < 3516))
; 3438 to 3515
.if((n > 3437 && n < 3468) || (n > 3468 && n < 3487) || n = 3488 || (n > 3489 && n < 3516))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.endif
.else
.if((n >= 3516) && (n < 3555))
; 3516 to 3554
.if((n > 3515 && n < 3555))
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
.if((n >= 3555) && (n < 3574))
; 3555 to 3573
.if((n > 3554 && n < 3561))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3561)
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
.elseif(n = 3562)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #13
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3563)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 3564)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 3565)
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
.elseif(n = 3566)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3567)
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
.elseif(n = 3568)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3569)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/16
	delay_n 11
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3570)
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
.elseif(n = 3571)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3572)
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
.endif
.else
; 3574 to 3593
.if(n = 3574)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3575 || n = 3589)
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
.elseif(n = 3576 || n = 3577 || n = 3593)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 3578 || n = 3592)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/14
	delay_n 9
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3579)
	; 15 bytes
	php
	pha
	tya
	ldy #136 ;hides 'dey'
	delay_n 7
	jsr @nearby_rts_14cyc
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3580)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 3581)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/18
	delay_n 13
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 3582)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/14
	nop
	pla
	pha
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 3583)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3584)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3585)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/16
	delay_n 11
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3586)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $21A0'
	ldy #33
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3587)
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
.elseif(n = 3588)
	; 16 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $01A0'
	ldy #1
	nop
	bit $24
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 3590)
	; 16 bytes
	php
	pha
	lda $A5
	sec
	lda #(n-20)/14
	delay_n 9
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 3591)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 3594) && (n < 3672))
.if((n >= 3594) && (n < 3633))
.if((n >= 3594) && (n < 3613))
; 3594 to 3612
.if(n = 3594)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/14
	delay_n 9
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 3595)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3596)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/14
	nop
	pla
	pha
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 3597)
	; 16 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	delay_n 11
	inx
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 3598)
	; 15 bytes
	php
	pha
	tya
	ldy #224 ;hides 'cpx #imm'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3599)
	; 15 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 9
	dey
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3600)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $39A0'
	ldy #57
	delay_n 9
	iny
	Jne *-7
	tay
	pla
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
.elseif(n = 3602)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/15
	rol $26
	ror $26
	sbc #1
	Jne *-6
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
.elseif(n = 3605 || n = 3607 || n = 3612)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 3606)
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
.elseif(n = 3609)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3610)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3611)
	; 9 bytes
	php
	pha
	lda #14
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 3613 to 3632
.if(n = 3613 || n = 3615 || n = 3617 || n = 3618 || n = 3620 || n = 3622 || n = 3625 || n = 3627 || n = 3629 || n = 3632)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3614)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 3619)
	; 11 bytes
	php
	pha
	lda $A5
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3621)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 3624)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3626)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3628)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3630)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3631)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 3633 to 3671
.if(n = 3633)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #14
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3634)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 3636)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3637)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
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
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3640)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 3640 && n < 3645))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3644 && n < 3650) || (n > 3650 && n < 3655) || (n > 3655 && n < 3672))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3650)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3655)
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
; 3672 to 3750
.if((n > 3671 && n < 3751))
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
.else
.if((n >= 3751) && (n < 4376))
.if((n >= 3751) && (n < 4063))
.if((n >= 3751) && (n < 3907))
.if((n >= 3751) && (n < 3829))
; 3751 to 3828
.if((n > 3750 && n < 3817))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
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
.elseif(n = 3818)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3819)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3820)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3821)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 3822)
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
.elseif(n = 3823)
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
.elseif(n = 3824)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
	pla
	plp
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
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $12A0'
	ldy #18
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3827)
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
.elseif(n = 3828)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 6
	iny
	Jne *-7
	tay
	pla
	plp
.endif
.else
.if((n >= 3829) && (n < 3868))
; 3829 to 3867
.if(n = 3829)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/15
	rol $26
	ror $26
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3830 || n = 3847 || n = 3864)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3831 || n = 3840 || n = 3845 || n = 3848 || n = 3852 || n = 3863 || n = 3865)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 3832)
	; 16 bytes
	php
	pha
	clc
	lda #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	adc #1
	Jne *-8
	pla
	plp
.elseif(n = 3833)
	; 15 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	dey ;first loop only
	delay_n 9
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3834)
	; 16 bytes
	php
	pha
	txa
	ldx #44 ;hides 'bit abs'
	bit $0
	delay_n 9
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 3835 || n = 3853)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3836 || n = 3855)
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
.elseif(n = 3837)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 3838 || n = 3857)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 3839 || n = 3860)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3841)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/16
	delay_n 11
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 3842 || n = 3859)
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
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/16
	delay_n 11
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 3844 || n = 3861)
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
.elseif(n = 3846)
	; 15 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	nop
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3849 || n = 3866)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 3850)
	; 15 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $EA ;hides 'nop'
	delay_n 7
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3851)
	; 15 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3854)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #86
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jpl *-5
	pla
	tay
	pla
	plp
.elseif(n = 3856)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $36A0'
	ldy #54
	rol $26
	ror $26
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 3858)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3862)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
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
.if(n = 3868)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/17
	jsr @nearby_rts
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 3869 || n = 3871 || n = 3873 || n = 3874 || n = 3876 || n = 3878 || n = 3881 || n = 3883 || n = 3885 || n = 3888)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 3870)
	; 11 bytes
	php
	pha
	lda $A5
	lda #15
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3872)
	; 9 bytes
	php
	pha
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3875)
	; 11 bytes
	php
	pha
	lda $A5
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3877)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 3880)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 3882)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 3884)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 3886)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 3887)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 3889)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #15
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3890)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3893)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 3894)
	; 15 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 11
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 3895)
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
.elseif((n > 3896 && n < 3901))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 3900 && n < 3906))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
; 3907 to 4062
.if((n > 3906 && n < 3911) || (n > 3911 && n < 4063))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.endif
.else
.if((n >= 4063) && (n < 4219))
.if((n >= 4063) && (n < 4141))
.if((n >= 4063) && (n < 4102))
; 4063 to 4101
.if((n > 4062 && n < 4073))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4073)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-248)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4074)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-249)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4075 || n = 4076)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4077)
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
.elseif(n = 4078)
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
.elseif(n = 4079)
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
.elseif(n = 4080 || n = 4097)
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
.elseif(n = 4081)
	; 15 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $02A0'
	ldy #2
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4082 || n = 4099)
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
.elseif(n = 4084 || n = 4087 || n = 4095 || n = 4100 || n = 4101)
	; 15..16 bytes
	delay_n (n-2)
	nop
.elseif(n = 4085)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4086)
	; 16 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1EA2'
	ldx #30
	delay_n 9
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 4088)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4089)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/17
	jsr @nearby_rts
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4090)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	jsr @nearby_rts
	iny
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 4091)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4092)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $3EA0'
	ldy #62
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4093)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4094)
	; 17 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4096)
	; 17 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4098)
	; 15 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 7
	iny
	Jne *-6
	tay
	pla
	plp
.endif
.else
.if((n >= 4102) && (n < 4121))
; 4102 to 4120
.if(n = 4102)
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
.elseif(n = 4103)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/18
	delay_n 13
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4104)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4105)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4106 || n = 4108 || n = 4110 || n = 4117 || n = 4118 || n = 4120)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 4107)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4109)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 4111)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4112)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4113)
	; 15 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 7
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4114)
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
.elseif(n = 4115)
	; 15 bytes
	php
	pha
	tya
	ldy #0
	delay_n 11
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4116)
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
.endif
.else
; 4121 to 4140
.if(n = 4121)
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
.elseif(n = 4122)
	; 16 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	nop
	rol $26
	ror $26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4123)
	; 9 bytes
	php
	pha
	lda #16
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4124)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #216 ;hides 'cld'
	jsr @nearby_rts
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4125 || n = 4127 || n = 4129 || n = 4130 || n = 4132 || n = 4134 || n = 4137 || n = 4139)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 4126)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 4131)
	; 11 bytes
	php
	pha
	lda $A5
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4133)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4138)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4140)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.endif
.else
; 4141 to 4218
.if(n = 4141 || n = 4144)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4142)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4143)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4145)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #16
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4146)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4147)
	; 13 bytes
	php
	pha
	lda #<(n-41) ;hides 'asl a'
	Jpl *-1
	lda #>(n-41)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4148)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4149)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4150)
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
.elseif(n = 4151)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4152)
	; 13 bytes
	php
	pha
	lda #<(n-46) ;hides 'asl a'
	Jpl *-1
	lda #>(n-46)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif((n > 4152 && n < 4157))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4156 && n < 4162) || (n > 4162 && n < 4167) || (n > 4167 && n < 4219))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 4167)
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
.if((n >= 4219) && (n < 4297))
; 4219 to 4296
.if((n > 4218 && n < 4297))
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
.if((n >= 4297) && (n < 4336))
; 4297 to 4335
.if((n > 4296 && n < 4329))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4329)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-249)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4330)
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
.elseif(n = 4331)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4332)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4333)
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
.elseif(n = 4334)
	; 15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4335)
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
.endif
.else
; 4336 to 4375
.if(n = 4336 || n = 4339 || n = 4341 || n = 4346 || n = 4353 || n = 4356 || n = 4358 || n = 4360 || n = 4372 || n = 4373)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 4337 || n = 4354)
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
.elseif(n = 4338)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $10A0'
	ldy #16
	delay_n 9
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4340)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/17
	jsr @nearby_rts
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4342 || n = 4359)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/17
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 4343 || n = 4364)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4344 || n = 4361)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/17
	jsr @nearby_rts
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4345 || n = 4366)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4347)
	; 16 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4348)
	; 16 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	cmp $EAEA ;hides two 'nop's
	delay_n 7
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4349 || n = 4368)
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
.elseif(n = 4350)
	; 16 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	delay_n 13
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4351 || n = 4370)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
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
.elseif(n = 4355)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/18
	delay_n 13
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4357 || n = 4375)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4362)
	; 18 bytes
	php
	pha
	php
	sec
	lda #241
	delay_n 13
	sbc #1
	Jne *-7
	plp
	pla
	plp
.elseif(n = 4363)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4365)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $31A0'
	ldy #49
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4367)
	; 17 bytes
	php
	pha
	sec
	lda #150
	delay_n 24
	sbc #1
	Jne *-8
	pla
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
.elseif(n = 4374)
	; 15 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	iny ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
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
	; 16 bytes
	php
	pha
	php
	sec
	lda #0
	jsr @nearby_rts
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 4377)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4378)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4379)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4380 || n = 4381 || n = 4383 || n = 4385 || n = 4386 || n = 4388 || n = 4390 || n = 4393 || n = 4395 || n = 4397 || n = 4400)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 4382)
	; 11 bytes
	php
	pha
	lda $A5
	lda #17
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4384)
	; 9 bytes
	php
	pha
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4387)
	; 11 bytes
	php
	pha
	lda $A5
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4389)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 4392)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4394)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4396)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4398)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4399)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4401)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4402)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4405)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 4407)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 4408 && n < 4413))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4413 || n = 4414)
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
; 4415 to 4453
.if((n > 4414 && n < 4418) || (n > 4418 && n < 4423) || (n > 4423 && n < 4454))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4418)
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.endif
.endif
.else
; 4454 to 4531
.if((n > 4453 && n < 4532))
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
.else
.if((n >= 4532) && (n < 4610))
; 4532 to 4609
.if((n > 4531 && n < 4585))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4585)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #17
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4586)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-251)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4587 || n = 4588)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4589)
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
.elseif(n = 4590)
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
.elseif(n = 4591)
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
.elseif(n = 4592)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 4593)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/22
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4594)
	; 15 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4595)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4596)
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
.elseif(n = 4597)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4598)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4599)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	bit $24
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4600 || n = 4602 || n = 4604)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 4601)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4603)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4605)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4606)
	; 17 bytes
	php
	pha
	tya
	ldy #139
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4607)
	; 16 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4608)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 9
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4609)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/18
	delay_n 13
	dey
	Jne *-6
	tay
	pla
	plp
.endif
.else
.if((n >= 4610) && (n < 4649))
; 4610 to 4648
.if(n = 4610 || n = 4611 || n = 4613 || n = 4619 || n = 4621 || n = 4628 || n = 4629 || n = 4637 || n = 4639 || n = 4641 || n = 4642 || n = 4644 || n = 4646)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 4612)
	; 17 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4614)
	; 18 bytes
	php
	pha
	tya
	pha
	lda #<$AD ;hides 'lda $01A0'
	ldy #1
	nop
	pla
	pha
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 4615 || n = 4634)
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
.elseif(n = 4616)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4617 || n = 4636)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4618)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4620)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #(n-22)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4622)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 4623)
	; 17 bytes
	php
	pha
	php
	sec
	lda #(n-24)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
	pla
	plp
.elseif(n = 4624)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4625)
	; 16 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4626)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 9
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4627)
	; 16 bytes
	php
	pha
	tya
	ldy #0
	delay_n 13
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4630)
	; 17 bytes
	php
	pha
	tya
	ldy #159
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4631)
	; 17 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 6
	adc #1
	Jne *-9
	pla
	plp
.elseif(n = 4632)
	; 18 bytes
	php
	pha
	tya
	pha
	lda #<$AD ;hides 'lda $00A0'
	ldy #0
	nop
	pla
	pha
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 4633)
	; 17 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 6
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 4635)
	; 9 bytes
	php
	pha
	lda #18
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4638)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 4643)
	; 11 bytes
	php
	pha
	lda $A5
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4645)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 4648)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 4649 to 4687
.if(n = 4649 || n = 4651 || n = 4653 || n = 4656)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 4650)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4652)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4654)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4655)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4657)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #18
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4658)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4661)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4662)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4663)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 4664 && n < 4669))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4668 && n < 4674) || (n > 4674 && n < 4679) || (n > 4679 && n < 4688))
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
	; 13 bytes
	php
	pha
	lda #<(n+8) ;hides 'lsr a'
	Jne *-1
	lda #>(n+8)
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.endif
.endif
.endif
.endif
.else
.if((n >= 4688) && (n < 4844))
; 4688 to 4843
.if((n > 4687 && n < 4841))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4841)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-251)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4842)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-252)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4843)
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
.endif
.else
.if((n >= 4844) && (n < 4922))
.if((n >= 4844) && (n < 4883))
.if((n >= 4844) && (n < 4863))
; 4844 to 4862
.if(n = 4844)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 4845)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4846)
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
.elseif(n = 4847)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 4848)
	; 16 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $1AA2'
	ldx #26
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 4849)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4850)
	; 16 bytes
	php
	pha
	php
	sec
	lda #(n-24)/19
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 4851)
	; 16 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	adc #1
	Jne *-8
	pla
	plp
.elseif(n = 4852)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4853)
	; 16 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	delay_n 7
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4854)
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
.elseif(n = 4855)
	; 16 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	adc #1
	Jne *-8
	pla
	plp
.elseif(n = 4856)
	; 15 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-7
	pla
	plp
.elseif(n = 4857)
	; 16 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	bit $EA ;hides 'nop'
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4858)
	; 15 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4859)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	bit $24
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4860 || n = 4861)
	; 16..17 bytes
	delay_n (n-2)
	nop
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
.endif
.else
; 4863 to 4882
.if(n = 4863)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	rol $26
	ror $26
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 4864)
	; 14 bytes
	php
	pha
	tya
	ldy #n/19-1
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4865)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4866 || n = 4873 || n = 4882)
	; 15..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 4867)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4868)
	; 15 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	tax
	pla
	plp
.elseif(n = 4869)
	; 16 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $19A2'
	ldx #25
	jsr @nearby_rts
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 4870)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4871)
	; 16 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/19
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	pla
	tay
	pla
	plp
.elseif(n = 4872)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4874)
	; 17 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4875)
	; 17 bytes
	php
	pha
	txa
	pha
	ldx #232 ;hides 'inx'
	jsr @nearby_rts
	dex
	dex
	Jne *-6
	pla
	tax
	pla
	plp
.elseif(n = 4876)
	; 17 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4877)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 4878)
	; 17 bytes
	php
	pha
	tya
	bit <$2C ;hides 'bit $0DA0'
	ldy #13
	delay_n 11
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 4879)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	bit $24
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 4880)
	; 15 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	iny
	Jne *-6
	tay
	pla
	plp
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
.endif
.endif
.else
.if((n >= 4883) && (n < 4902))
; 4883 to 4901
.if(n = 4883)
	; 14 bytes
	php
	pha
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4884)
	; 16 bytes
	php
	pha
	lda $A5
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	pla
	plp
.elseif(n = 4885 || n = 4893 || n = 4895 || n = 4897 || n = 4898 || n = 4900)
	; 10..15 bytes
	delay_n (n-2)
	nop
.elseif(n = 4886)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-4
	tay
	pla
	plp
.elseif(n = 4887)
	; 14 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-6
	pla
	plp
.elseif(n = 4888)
	; 16 bytes
	php
	pha
	php
	sec
	lda #0
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-5
	plp
	pla
	plp
.elseif(n = 4889)
	; 14 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4890)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $18A0'
	ldy #24
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 4891)
	; 9 bytes
	php
	pha
	lda #19
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4892)
	; 16 bytes
	php
	pha
	lda $A5
	tya
	ldy #24 ;hides 'clc'
	jsr @nearby_rts_14cyc
	iny
	Jne *-5
	tay
	pla
	plp
.elseif(n = 4894)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 4899)
	; 11 bytes
	php
	pha
	lda $A5
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4901)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 4902 to 4921
.if(n = 4902 || n = 4905 || n = 4907 || n = 4909 || n = 4912)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 4904)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 4906)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 4908)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 4910)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 4911)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4913)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #19
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4914)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4917)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 4918)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 4919)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
; 4922 to 5000
.if((n > 4921 && n < 4925))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 4924 && n < 4930) || (n > 4930 && n < 4935) || (n > 4935 && n < 5001))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.if((n > 5000 && n < 5079))
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
.if((n >= 5079) && (n < 5118))
; 5079 to 5117
.if((n > 5078 && n < 5097))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5097)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-252)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5098)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-253)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5099)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5100)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5101)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5102)
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
.elseif(n = 5103)
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
.elseif(n = 5104)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5105)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5106)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5107)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5108)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5109)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5110)
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
.elseif(n = 5111)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5112)
	; 17 bytes
	php
	pha
	tya
	ldy #196 ;hides 'cpy zp'
	cmp $EAEA ;hides two 'nop's
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5113)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5114)
	; 17 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	dey ;first loop only
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5115)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5116)
	; 18 bytes
	php
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	lda #98
	jsr delay_a_27_clocks
	dex
	Jmi *-7
	pla
	tax
	pla
	plp
.elseif(n = 5117)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/20
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.endif
.else
.if((n >= 5118) && (n < 5137))
; 5118 to 5136
.if(n = 5118)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	delay_n 11
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5119)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/20
	bit $24
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5120)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5121)
	; 16 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	bit $24
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5122)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5123)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/22
	bit $24
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5124 || n = 5126 || n = 5133)
	; 16..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5125)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5127)
	; 17 bytes
	php
	pha
	php
	sec
	lda #(n-24)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
	pla
	plp
.elseif(n = 5128)
	; 17 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $C5C5 ;hides 'cmp zp'
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5129)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5130)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #176
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 5131)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5132)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5134)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5135)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 5136)
	; 18 bytes
	php
	pha
	tya
	ldy #119
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
.endif
.else
; 5137 to 5156
.if(n = 5137)
	; 16 bytes
	php
	pha
	sec
	lda #0
	bit $24
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5138)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 11
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5139)
	; 16 bytes
	php
	pha
	tya
	ldy #0
	bit $24
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5140)
	; 17 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5141)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5142)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5143)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5144 || n = 5149 || n = 5151 || n = 5153 || n = 5154 || n = 5156)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5145)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/22
	bit $24
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5146)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5147)
	; 9 bytes
	php
	pha
	lda #20
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5148)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5150)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 5155)
	; 11 bytes
	php
	pha
	lda $A5
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 5157 to 5312
.if(n = 5157)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5158 || n = 5161 || n = 5163 || n = 5165 || n = 5168)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 5160)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5162)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5164)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5166)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5167)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5169)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #20
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5170)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 5172)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5173)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5174)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5175)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 5176 && n < 5181))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5180 && n < 5186) || (n > 5186 && n < 5191) || (n > 5191 && n < 5313))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.else
.if((n >= 5313) && (n < 5469))
.if((n >= 5313) && (n < 5391))
.if((n >= 5313) && (n < 5352))
; 5313 to 5351
.if((n > 5312 && n < 5352))
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
.if((n >= 5352) && (n < 5371))
; 5352 to 5370
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
.elseif(n = 5353)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-253)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5354)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-254)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5355 || n = 5356)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5357)
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
.elseif(n = 5358)
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
.elseif(n = 5359)
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
.elseif(n = 5360)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5361)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5362)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5363)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/22
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5364)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5365)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/22
	bit $24
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5366)
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
.elseif(n = 5367)
	; 16 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5368)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $2AA0'
	ldy #42
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5369)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5370)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #31
	lda #133
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.else
; 5371 to 5390
.if(n = 5371)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5372)
	; 15 bytes
	php
	pha
	sec
	lda #(n-17)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5373)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5374)
	; 15 bytes
	php
	pha
	tya
	ldy #(n-19)/21
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5375)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5376)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5377)
	; 16 bytes
	php
	pha
	txa
	bit <$2C ;hides 'bit $17A2'
	ldx #23
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 5378)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5379)
	; 17 bytes
	php
	pha
	php
	sec
	lda #(n-24)/21
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	plp
	pla
	plp
.elseif(n = 5380)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5381)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #255
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 5382)
	; 17 bytes
	php
	pha
	sec
	lda #185
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5383)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5384)
	; 17 bytes
	php
	pha
	tya
	ldy #185
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5385)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/22
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5386)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #134
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 5387)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/22
	bit $24
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5388)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #152
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 5389)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5390)
	; 17 bytes
	php
	pha
	tya
	ldy #200 ;hides 'iny'
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 5391) && (n < 5430))
.if((n >= 5391) && (n < 5410))
; 5391 to 5409
.if(n = 5391)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 5392)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5393)
	; 15 bytes
	php
	pha
	sec
	lda #0
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-6
	pla
	plp
.elseif(n = 5394)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5395)
	; 15 bytes
	php
	pha
	tya
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	tay
	pla
	plp
.elseif(n = 5396 || n = 5405 || n = 5407 || n = 5409)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5397)
	; 15 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5398)
	; 17 bytes
	php
	pha
	tya
	ldy #163
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5399)
	; 15 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5400)
	; 16 bytes
	php
	pha
	txa
	cmp <$CD ;hides 'cmp $16A2'
	ldx #22
	jsr @nearby_rts_14cyc
	inx
	Jne *-7
	tax
	pla
	plp
.elseif(n = 5401)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5402)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 5403)
	; 9 bytes
	php
	pha
	lda #21
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5404)
	; 17 bytes
	php
	pha
	php
	sec
	lda #234 ;hides 'nop'
	nop
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	plp
	pla
	plp
.elseif(n = 5406)
	; 11 bytes
	php
	pha
	lda $A5
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
.endif
.else
; 5410 to 5429
.if(n = 5410 || n = 5412 || n = 5414 || n = 5417 || n = 5419 || n = 5421 || n = 5424)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5411)
	; 11 bytes
	php
	pha
	lda $A5
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5413)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 5416)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5418)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5420)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5422)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5423)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5425)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #21
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5426)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5429)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 5430 to 5468
.if(n = 5430)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5431)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 5432 && n < 5437))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5436 && n < 5442) || (n > 5442 && n < 5447) || (n > 5447 && n < 5469))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.endif
.endif
.else
; 5469 to 5625
.if((n > 5468 && n < 5609))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5609)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-254)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5610)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #n/255-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5611 || n = 5612)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5613)
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
.elseif(n = 5614)
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
.elseif(n = 5615)
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
.elseif(n = 5616)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5617)
	; 16 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	delay_n 7
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5618)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5619)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5620)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5621)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5622)
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
.elseif(n = 5623)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #193
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 5624)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5625)
	; 18 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #224
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 5627)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/22
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5628)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5629)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/22
	bit $24
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5630)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0CA0'
	ldy #12
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5631)
	; 16 bytes
	php
	pha
	sec
	lda #216 ;hides 'cld'
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5632 || n = 5637)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5633)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5634)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #85 ;hides 'eor zp,x'
	iny ;first loop only
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 5635)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5636)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #245 ;hides 'sbc zp,x'
	dey ;first loop only
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 5638)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-72)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 5639)
	; 18 bytes
	php
	pha
	tya
	ldy #19
	pha
	lda #240
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 5640)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 5641)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #105
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	pla
	plp
.elseif(n = 5642)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5643)
	; 17 bytes
	php
	pha
	sec
	lda #194
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5644)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.else
; 5645 to 5664
.if(n = 5645)
	; 17 bytes
	php
	pha
	tya
	ldy #194
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5646)
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
.elseif(n = 5647)
	; 17 bytes
	php
	pha
	tya
	ldy #201
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5648)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 5649)
	; 16 bytes
	php
	pha
	sec
	lda #0
	bit $24
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5650)
	; 18 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	delay_n 13
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 5651)
	; 16 bytes
	php
	pha
	tya
	ldy #0
	bit $24
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5652)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5653)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5654)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5655 || n = 5656 || n = 5661 || n = 5663)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5657)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5658)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #127
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jpl *-5
	pla
	tay
	pla
	plp
.elseif(n = 5659)
	; 9 bytes
	php
	pha
	lda #22
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5660)
	; 17 bytes
	php
	pha
	sec
	lda #171
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5662)
	; 11 bytes
	php
	pha
	lda $A5
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
.endif
.else
; 5665 to 5703
.if(n = 5665 || n = 5666 || n = 5668 || n = 5670 || n = 5673 || n = 5675 || n = 5677 || n = 5680)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5667)
	; 11 bytes
	php
	pha
	lda $A5
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5669)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 5672)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5674)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5676)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5678)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5679)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5681)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5682)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 5684)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5685)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5686)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5687)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 5688 && n < 5693))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5692 && n < 5698) || (n > 5698 && n < 5703))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.if((n > 5703 && n < 5782))
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
.else
.if((n >= 5782) && (n < 5860))
; 5782 to 5859
.if((n > 5781 && n < 5860))
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
.if((n >= 5860) && (n < 5899))
.if((n >= 5860) && (n < 5879))
; 5860 to 5878
.if((n > 5859 && n < 5865))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5865)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #n/255-1
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5866)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-256)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5867 || n = 5868)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 5869)
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
.elseif(n = 5870)
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
.elseif(n = 5871)
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
.elseif(n = 5872)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 5873)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5874)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5875)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5876)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5877)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5878)
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
; 5879 to 5898
.if(n = 5879)
	; 18 bytes
	php
	pha
	tya
	ldy #19
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 5880)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5881)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5882)
	; 16 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5883)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	jsr @nearby_rts_14cyc
	iny
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5884)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/23
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5885 || n = 5886 || n = 5889)
	; 17..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 5887)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5888)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 5890)
	; 18 bytes
	php
	pha
	tya
	ldy #57
	pha
	lda #62
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 5891)
	; 17 bytes
	php
	pha
	sec
	lda #178
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5892)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 5893)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5894)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5895)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5896)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #22
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5897)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 5898)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #231
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 5899) && (n < 5918))
; 5899 to 5917
.if(n = 5899)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5900)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #88
	lda #32
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 5901 || n = 5908 || n = 5917)
	; 10..17 bytes
	delay_n (n-2)
	nop
.elseif(n = 5902)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #n/26-1
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 5903)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	delay_n 251
	Jcs *-11
	pla
	plp
.elseif(n = 5904)
	; 16 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 24
	Jne *-7
	tax
	pla
	plp
.elseif(n = 5905)
	; 16 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5906)
	; 16 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5907)
	; 16 bytes
	php
	pha
	tya
	ldy #0
	delay_n 4
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 5909)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 5910)
	; 18 bytes
	php
	pha
	tya
	ldy #137
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
.elseif(n = 5911)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #144
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 5912)
	; 18 bytes
	php
	pha
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #195
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
	pla
	plp
.elseif(n = 5913)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 5914)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-72)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 5915)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5916)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 5918 to 5937
.if(n = 5918)
	; 11 bytes
	php
	pha
	lda $A5
	lda #23
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5919 || n = 5921 || n = 5922 || n = 5924 || n = 5926 || n = 5929 || n = 5931 || n = 5933 || n = 5936)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 5920)
	; 9 bytes
	php
	pha
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5923)
	; 11 bytes
	php
	pha
	lda $A5
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5925)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 5928)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 5930)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 5932)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 5934)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 5935)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5937)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 5938) && (n < 6094))
; 5938 to 6093
.if(n = 5938)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 5940)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5941)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 5942)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 5943)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 5944 && n < 5949))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 5948 && n < 5954) || (n > 5954 && n < 5959) || (n > 5959 && n < 6094))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.else
.if((n >= 6094) && (n < 6172))
.if((n >= 6094) && (n < 6133))
; 6094 to 6132
.if((n > 6093 && n < 6121))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6121)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-256)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6122)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-257)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6123 || n = 6124)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6125)
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
.elseif(n = 6126)
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
.elseif(n = 6127)
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
.elseif(n = 6128)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6129)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6130)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6131)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6132)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 6133) && (n < 6152))
; 6133 to 6151
.if(n = 6133)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6134)
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
.elseif(n = 6135)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6136)
	; 17 bytes
	php
	pha
	sec
	lda #211
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6137)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/24
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6138)
	; 17 bytes
	php
	pha
	tya
	ldy #211
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6139)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/24
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6140)
	; 17 bytes
	php
	pha
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #202 ;hides 'dex'
	Jne *-1
	Jcc *-5
	pla
	tax
	pla
	plp
.elseif(n = 6141)
	; 17 bytes
	delay_n (n-2)
	nop
.elseif(n = 6142)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 6143)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0BA0'
	ldy #11
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6144)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/25
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6145)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 6146)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6147)
	; 17 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6148)
	; 18 bytes
	php
	pha
	tya
	ldy #27
	pha
	lda #186
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6149)
	; 17 bytes
	php
	pha
	sec
	lda #219
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6150)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6151)
	; 17 bytes
	php
	pha
	tya
	ldy #219
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.else
; 6152 to 6171
.if(n = 6152)
	; 18 bytes
	php
	pha
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #203
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
	pla
	plp
.elseif(n = 6153)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6154)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #152
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 6155)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6156)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #49
	dey
	Jpl *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 6157)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6158)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 6159)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #23
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6160)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 6161)
	; 16 bytes
	php
	pha
	sec
	lda #0
	delay_n 7
	jsr @nearby_rts
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6162)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #n/26-1
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6163)
	; 16 bytes
	php
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6164)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6165)
	; 17 bytes
	php
	pha
	sec
	lda #212
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6166)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6167)
	; 17 bytes
	php
	pha
	tya
	ldy #212
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6168)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $0AA0'
	ldy #10
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6169)
	; 17 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	bit $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6170)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #30
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
.endif
.else
; 6172 to 6250
.if(n = 6172)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6173 || n = 6175 || n = 6177 || n = 6178 || n = 6180 || n = 6182 || n = 6185 || n = 6187 || n = 6189 || n = 6192)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6174)
	; 11 bytes
	php
	pha
	lda $A5
	lda #24
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6176)
	; 9 bytes
	php
	pha
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6179)
	; 11 bytes
	php
	pha
	lda $A5
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6181)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 6184)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6186)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6188)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6190)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6191)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6193)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6194)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 6196)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6197)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6198)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6199)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 6200 && n < 6205))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 6204 && n < 6210) || (n > 6210 && n < 6215) || (n > 6215 && n < 6251))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.if((n > 6250 && n < 6329))
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
.if((n >= 6329) && (n < 6368))
; 6329 to 6367
.if((n > 6328 && n < 6368))
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
.if((n >= 6368) && (n < 6387))
; 6368 to 6386
.if((n > 6367 && n < 6377))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6377)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-257)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6378)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-258)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6379 || n = 6380)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6381)
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
.elseif(n = 6382)
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
.elseif(n = 6383)
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
.elseif(n = 6384)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6385)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6386)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 6387 to 6406
.if(n = 6387)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6388)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6389)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6390)
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
.elseif(n = 6391)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6392)
	; 17 bytes
	php
	pha
	tya
	cmp #<$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6393)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $01A0'
	ldy #1
	nop
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6394)
	; 17 bytes
	php
	pha
	tya
	ldy #124
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jpl *-7
	tay
	pla
	plp
.elseif(n = 6395)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #32
	lda #159
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 6396)
	; 17 bytes
	php
	pha
	clc
	lda #36 ;hides 'bit zp'
	delay_n 9
	jsr @nearby_rts_14cyc
	adc #1
	Jne *-9
	pla
	plp
.elseif(n = 6397)
	; 17 bytes
	php
	pha
	sec
	lda #220
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6398)
	; 17 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	delay_n 9
	jsr @nearby_rts_14cyc
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6399)
	; 17 bytes
	php
	pha
	tya
	ldy #220
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6400)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6401)
	; 17 bytes
	php
	pha
	sec
	lda #228
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6402)
	; 17 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	cmp $2808 ;hides 'php' and 'plp'; reads from $2000
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6403)
	; 17 bytes
	php
	pha
	tya
	ldy #228
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6404)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #254
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6405)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6406)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #220
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 6407) && (n < 6485))
.if((n >= 6407) && (n < 6446))
.if((n >= 6407) && (n < 6426))
; 6407 to 6425
.if(n = 6407)
	; 17 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6408 || n = 6417 || n = 6423)
	; 17..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 6409)
	; 18 bytes
	php
	pha
	tya
	ldy #142
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6410)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #113
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 6411)
	; 18 bytes
	php
	pha
	tya
	ldy #136
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6412)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6413)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6414)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #24
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6415)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6416)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #90
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6418)
	; 17 bytes
	php
	pha
	tya
	cmp <$CD ;hides 'cmp $00A0'
	ldy #0
	nop
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6419)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	delay_n 6
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6420)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6421)
	; 17 bytes
	php
	pha
	tya
	ldy #194
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6422)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #n/26-1
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6424)
	; 18 bytes
	php
	pha
	tya
	ldy #61
	pha
	lda #64
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6425)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #81
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 6426 to 6445
.if(n = 6426)
	; 17 bytes
	php
	pha
	sec
	lda #221
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6427)
	; 9 bytes
	php
	pha
	lda #25
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6428)
	; 17 bytes
	php
	pha
	tya
	ldy #221
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6429 || n = 6431 || n = 6433 || n = 6434 || n = 6436 || n = 6438 || n = 6441 || n = 6443 || n = 6445)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6430)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 6435)
	; 11 bytes
	php
	pha
	lda $A5
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6437)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 6440)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6442)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6444)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 6446 to 6484
.if(n = 6446)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6447)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6448)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 6449)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6450)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 6452)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6453)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6454)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6455)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 6456 && n < 6461))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 6460 && n < 6466) || (n > 6466 && n < 6471) || (n > 6471 && n < 6485))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 6484 && n < 6563))
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
.if((n >= 6563) && (n < 6719))
.if((n >= 6563) && (n < 6641))
; 6563 to 6640
.if((n > 6562 && n < 6633))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6633)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-258)/255
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
	lda #(n-259)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6635 || n = 6636)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6637)
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
.elseif(n = 6638)
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
.elseif(n = 6639)
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
.elseif(n = 6640)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
.if((n >= 6641) && (n < 6680))
.if((n >= 6641) && (n < 6660))
; 6641 to 6659
.if(n = 6641)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6642)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6643)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6644)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6645)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6646)
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
.elseif(n = 6647)
	; 16 bytes
	php
	pha
	sec
	lda #(n-17)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6648)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6649)
	; 16 bytes
	php
	pha
	tya
	ldy #(n-19)/26
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6650)
	; 17 bytes
	php
	pha
	sec
	lda #201
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6651)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6652)
	; 17 bytes
	php
	pha
	tya
	ldy #201
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6653)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6654)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #25
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6655)
	; 17 bytes
	php
	pha
	tya
	ldy #237
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6656)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #n/26-1
	clc
	lda #42 ;hides 'rol a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6657)
	; 18 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #218
	dey
	Jne *-1
	tay
	pla
	Jcc *-10
	pla
	plp
.elseif(n = 6658)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6659)
	; 17 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	iny ;first loop only
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 6660 to 6679
.if(n = 6660)
	; 17 bytes
	php
	pha
	tya
	ldy #229
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6661)
	; 18 bytes
	php
	pha
	tya
	ldy #54
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
.elseif(n = 6662)
	; 18 bytes
	php
	pha
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #220
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
	pla
	plp
.elseif(n = 6663)
	; 18 bytes
	php
	pha
	tya
	ldy #151
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6664)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #130
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6665)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #234
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 6666)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6667)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 6668)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6669)
	; 18 bytes
	php
	pha
	tya
	ldy #175
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6670)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 6671)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #163
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 6672 || n = 6677)
	; 17..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 6673)
	; 16 bytes
	php
	pha
	sec
	lda #0
	delay_n 7
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-7
	pla
	plp
.elseif(n = 6674)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #165
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 6675)
	; 16 bytes
	php
	pha
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6676)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #190
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6678)
	; 18 bytes
	php
	pha
	lda $A5
	tya
	ldy #0
	delay_n 7
	jsr @nearby_rts_14cyc
	dey
	Jne *-6
	tay
	pla
	plp
.elseif(n = 6679)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 6680) && (n < 6699))
; 6680 to 6698
.if(n = 6680)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #28
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 6681)
	; 17 bytes
	php
	pha
	sec
	lda #238
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6682)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6683)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6684)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6685 || n = 6687 || n = 6689 || n = 6690 || n = 6692 || n = 6694 || n = 6697)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 6686)
	; 11 bytes
	php
	pha
	lda $A5
	lda #26
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6688)
	; 9 bytes
	php
	pha
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6691)
	; 11 bytes
	php
	pha
	lda $A5
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6693)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 6696)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 6698)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 6699 to 6718
.if(n = 6699 || n = 6701 || n = 6704)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6700)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6702)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6703)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6705)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6706)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 6708)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6709)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6710)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6711)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 6712 && n < 6717))
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
.endif
.else
; 6719 to 6875
.if((n > 6718 && n < 6722) || (n > 6722 && n < 6727) || (n > 6727 && n < 6876))
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
.endif
.endif
.endif
.else
.if((n >= 6876) && (n < 7188))
.if((n >= 6876) && (n < 7032))
.if((n >= 6876) && (n < 6954))
.if((n >= 6876) && (n < 6915))
.if((n >= 6876) && (n < 6895))
; 6876 to 6894
.if((n > 6875 && n < 6889))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6889)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-259)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6890)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6891 || n = 6892)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 6893)
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
.elseif(n = 6894)
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
.else
; 6895 to 6914
.if(n = 6895)
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
.elseif(n = 6896)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6897)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 6898)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6899)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6900)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6901)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6902)
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
.elseif(n = 6903)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6904)
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
.elseif(n = 6905)
	; 17 bytes
	php
	pha
	sec
	lda #246
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6906)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 6907)
	; 17 bytes
	php
	pha
	tya
	ldy #246
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6908)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6909)
	; 18 bytes
	php
	pha
	tya
	ldy #130
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 6910)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6911)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #169
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 6912)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	clc
	lda #$2A ;hides 'rol a'
	Jcc *-1
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6913)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #37
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 6914)
	; 17 bytes
	php
	pha
	sec
	lda #209
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.endif
.endif
.else
.if((n >= 6915) && (n < 6934))
; 6915 to 6933
.if(n = 6915)
	; 17 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
	Jcs *-10
	pla
	plp
.elseif(n = 6916)
	; 17 bytes
	php
	pha
	tya
	ldy #209
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6917)
	; 18 bytes
	php
	pha
	clc
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #200 ;hides 'iny'
	Jmi *-1
	tay
	pla
	Jcs *-10
	pla
	plp
.elseif(n = 6918)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6919)
	; 17 bytes
	php
	pha
	sec
	lda #238
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6920)
	; 18 bytes
	php
	pha
	tya
	ldy #102
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 6921)
	; 17 bytes
	php
	pha
	tya
	ldy #238
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6922)
	; 18 bytes
	php
	pha
	tya
	ldy #58
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 6923)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #57
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6924)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6925)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #26
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6926)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6927)
	; 18 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #227
	dey
	Jne *-1
	tay
	pla
	Jcc *-10
	pla
	plp
.elseif(n = 6928)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #238
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 6929)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #177
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6930)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 6931)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6932)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6933)
	; 17 bytes
	php
	pha
	sec
	lda #247
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.endif
.else
; 6934 to 6953
.if(n = 6934)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 6935)
	; 17 bytes
	php
	pha
	tya
	ldy #247
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 6936 || n = 6941 || n = 6943 || n = 6945 || n = 6946 || n = 6948 || n = 6950 || n = 6953)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 6937)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #130
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 6938)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 6939)
	; 9 bytes
	php
	pha
	lda #27
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6940)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 6942)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 6947)
	; 11 bytes
	php
	pha
	lda $A5
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6949)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 6952)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.endif
.else
; 6954 to 7031
.if(n = 6954)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 6955 || n = 6957 || n = 6960)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 6956)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 6958)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 6959)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6961)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6962)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 6964)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6965)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 6966)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 6967)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 6968 && n < 6973))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 6972 && n < 6978) || (n > 6978 && n < 6983) || (n > 6983 && n < 7032))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.else
.if((n >= 7032) && (n < 7110))
; 7032 to 7109
.if((n > 7031 && n < 7110))
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
.if((n >= 7110) && (n < 7149))
; 7110 to 7148
.if((n > 7109 && n < 7145))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7145)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-260)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 7147 || n = 7148)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.endif
.else
.if((n >= 7149) && (n < 7168))
; 7149 to 7167
.if(n = 7149)
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
.elseif(n = 7150)
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
.elseif(n = 7151)
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
.elseif(n = 7152)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7153)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7154)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7155)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7156)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7157)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7158)
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
.elseif(n = 7159)
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
.elseif(n = 7160)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 7161 || n = 7162)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7163)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7164)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #166
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7165)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-147)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 7166)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #119
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7167)
	; 18 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #235
	dey
	Jne *-1
	tay
	pla
	Jcc *-10
	pla
	plp
.endif
.else
; 7168 to 7187
.if(n = 7168)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152 ;hides 'tya'
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 7169)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	tya
	ldy #201
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 7170)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #188
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7171)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7172)
	; 18 bytes
	php
	pha
	txa
	pha
	clc
	lda #$0A ;hides 'asl a'
	ldx #237
	dex
	Jne *-1
	Jcc *-6
	pla
	tax
	pla
	plp
.elseif(n = 7173)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #203
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 7174)
	; 18 bytes
	php
	pha
	tya
	ldy #135
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7175)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #27
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7176)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #64
	lda #76
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 7177)
	; 18 bytes
	php
	pha
	txa
	ldx #232 ;hides 'inx'
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	inx
	inx
	Jne *-9
	tax
	pla
	plp
.elseif(n = 7178)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7179)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #60
	dex
	Jpl *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 7180)
	; 17 bytes
	php
	pha
	sec
	lda #247
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7181)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #159
	lda #(n-625)/596
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7182)
	; 17 bytes
	php
	pha
	tya
	ldy #247
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7183)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7184)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #143
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 7185)
	; 17 bytes
	php
	pha
	sec
	lda #0
	delay_n 9
	jsr @nearby_rts_14cyc
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7186)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7187)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	delay_n 9
	jsr @nearby_rts_14cyc
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 7188) && (n < 7344))
.if((n >= 7188) && (n < 7266))
.if((n >= 7188) && (n < 7227))
.if((n >= 7188) && (n < 7207))
; 7188 to 7206
.if(n = 7188)
	; 18 bytes
	php
	pha
	tya
	ldy #107
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7189)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #247
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 7190)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7191 || n = 7197 || n = 7199 || n = 7201 || n = 7202 || n = 7204 || n = 7206)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7192)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #92
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 7193)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 7194)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #178
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 7195)
	; 9 bytes
	php
	pha
	lda #28
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7196)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #29
	lda #205
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 7198)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 7203)
	; 11 bytes
	php
	pha
	lda $A5
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7205)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 7207 to 7226
.if(n = 7207)
	; 11 bytes
	php
	pha
	php
	lda #28
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7208)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7209 || n = 7211 || n = 7213 || n = 7216)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7210)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7212)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7214)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7215)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7217)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7218)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 7220)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7221)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7222)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7223)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 7225 || n = 7226)
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
; 7227 to 7265
.if(n = 7227 || n = 7228)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7228 && n < 7234) || (n > 7234 && n < 7239) || (n > 7239 && n < 7266))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 7265 && n < 7344))
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
.else
.if((n >= 7344) && (n < 7422))
; 7344 to 7421
.if((n > 7343 && n < 7401))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	lda #(n-262)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7403 || n = 7404)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 7405)
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
.elseif(n = 7406)
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
.elseif(n = 7407)
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
.elseif(n = 7408)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7409)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7410)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7411)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7412)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7413)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7414)
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
.elseif(n = 7415)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7416)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7417)
	; 18 bytes
	php
	pha
	tya
	ldy #54
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7418)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #168
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 7419)
	; 18 bytes
	php
	pha
	tya
	ldy #148
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7420)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7421)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #255
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.endif
.else
.if((n >= 7422) && (n < 7461))
.if((n >= 7422) && (n < 7441))
; 7422 to 7440
.if(n = 7422)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #172
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7423 || n = 7437)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7424)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7425)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7426)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7427)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	pla
	dey
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 7428)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7429)
	; 18 bytes
	php
	pha
	tya
	ldy #195
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7430)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7431)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #182
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 7432)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7433)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 7434)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #184
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 7435)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7436)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #190
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7438)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7439)
	; 18 bytes
	php
	pha
	tya
	ldy #140
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7440)
	; 18 bytes
	php
	pha
	tya
	ldy #<((n-19)/41)
	pha
	lda #>((n-19)/41)
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 7441 to 7460
.if(n = 7441)
	; 17 bytes
	php
	pha
	sec
	lda #0
	delay_n 24
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7442)
	; 17 bytes
	php
	pha
	sec
	lda #225
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7443)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	delay_n 24
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7444)
	; 17 bytes
	php
	pha
	tya
	ldy #225
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7445)
	; 18 bytes
	php
	pha
	tya
	ldy #158
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7446)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7447)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7448)
	; 19 bytes
	php
	pha
	lda #91
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #28
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7449)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	tya
	ldy #209
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 7450)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 7451)
	; 9 bytes
	php
	pha
	lda #29
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7452 || n = 7453 || n = 7455 || n = 7457 || n = 7458 || n = 7460)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7454)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 7459)
	; 11 bytes
	php
	pha
	lda $A5
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 7461 to 7500
.if(n = 7461)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7462 || n = 7465 || n = 7467 || n = 7469 || n = 7472)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 7464)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7466)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7468)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7470)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7471)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7473)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7474)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7477)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7478)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7479)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 7480 && n < 7485))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7484 && n < 7490) || (n > 7490 && n < 7495) || (n > 7495 && n < 7501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 7500 && n < 7657))
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
.if((n >= 7657) && (n < 7735))
.if((n >= 7657) && (n < 7696))
.if((n >= 7657) && (n < 7676))
; 7657 to 7675
.if(n = 7657)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-262)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7658)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-263)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7659 || n = 7660)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 7661)
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
.elseif(n = 7662)
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
.elseif(n = 7663)
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
.elseif(n = 7664)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7665)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7666)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7667)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7668)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7669)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7670)
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
.elseif(n = 7671)
	; 17 bytes
	php
	pha
	clc
	lda #24 ;hides 'clc'
	delay_n 26
	adc #1
	Jne *-9
	pla
	plp
.elseif(n = 7672)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	delay_n 242
	Jne *-11
	pla
	plp
.elseif(n = 7673)
	; 17 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	delay_n 26
	iny
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7674)
	; 17 bytes
	php
	pha
	sec
	lda #(n-17)/31
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7675)
	; 17 bytes
	php
	pha
	tya
	ldy #232
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.else
; 7676 to 7695
.if(n = 7676)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/31
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7677)
	; 18 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #252
	dey
	Jne *-1
	tay
	pla
	Jcc *-10
	pla
	plp
.elseif(n = 7678)
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
.elseif(n = 7679)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7680)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 7681)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7682)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #174
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 7683)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7684 || n = 7689)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7685)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #207
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7686)
	; 18 bytes
	php
	pha
	tya
	ldy #<((n-19)/41)
	pha
	lda #>((n-19)/41)
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7687)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7688)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7690)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #29
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7691)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #219
	lda #1
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7692)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #106
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 7693)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7694)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7695)
	; 18 bytes
	php
	pha
	tya
	ldy #202
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 7696) && (n < 7715))
; 7696 to 7714
.if(n = 7696)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #64
	lda #84
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 7697)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #149
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 7698)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7699)
	; 18 bytes
	php
	pha
	tya
	ldy #240
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7700)
	; 18 bytes
	php
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	lda #166
	jsr delay_a_27_clocks
	dex
	Jmi *-7
	pla
	tax
	pla
	plp
.elseif(n = 7701)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7702)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7703)
	; 17 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 24
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 7704)
	; 18 bytes
	php
	pha
	tya
	ldy #145
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7705)
	; 17 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 24
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7706)
	; 17 bytes
	php
	pha
	sec
	lda #233
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7707)
	; 9 bytes
	php
	pha
	lda #30
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7708)
	; 17 bytes
	php
	pha
	tya
	ldy #233
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7709 || n = 7711 || n = 7713 || n = 7714)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 7710)
	; 11 bytes
	php
	pha
	lda $A5
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
.endif
.else
; 7715 to 7734
.if(n = 7715)
	; 11 bytes
	php
	pha
	lda $A5
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7716 || n = 7718 || n = 7721 || n = 7723 || n = 7725 || n = 7728)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7717)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 7720)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7722)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7724)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7726)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7727)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7729)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7730)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 7732)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7733)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7734)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.else
; 7735 to 7812
.if(n = 7735)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 7736 && n < 7741))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7740 && n < 7746) || (n > 7746 && n < 7751) || (n > 7751 && n < 7813))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.endif
.else
.if((n >= 7813) && (n < 7969))
.if((n >= 7813) && (n < 7891))
; 7813 to 7890
.if((n > 7812 && n < 7891))
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
.if((n >= 7891) && (n < 7930))
; 7891 to 7929
.if((n > 7890 && n < 7913))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7913)
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-263)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7914)
	; 15 bytes
	php
	pha
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-264)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7915 || n = 7916)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 7917)
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
.elseif(n = 7918)
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
.elseif(n = 7919)
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
.elseif(n = 7920)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7921)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 7922)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7923)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7924)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7925)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7926)
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
.elseif(n = 7927)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7928)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #40
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 7929)
	; 18 bytes
	php
	pha
	tya
	ldy #69
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.endif
.else
.if((n >= 7930) && (n < 7949))
; 7930 to 7948
.if(n = 7930)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #208
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7931)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/31
	sec
	lda #10 ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7932)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #66
	lda #84
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 7933)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7934)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7935)
	; 18 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 7936)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 7937)
	; 17 bytes
	php
	pha
	sec
	lda #240
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7938)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7939)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7940)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #48
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 7941 || n = 7948)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 7942)
	; 18 bytes
	php
	pha
	tya
	ldy #57
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7943)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7944)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7945)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #53
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 7946)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #180
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 7947)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #88
	lda #55
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.else
; 7949 to 7968
.if(n = 7949)
	; 18 bytes
	php
	pha
	tya
	ldy #64
	pha
	lda #81
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 7950)
	; 18 bytes
	php
	pha
	tya
	ldy #76
	pha
	lda #62
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 7951)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #195
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 7952 || n = 7965 || n = 7967)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 7953)
	; 17 bytes
	php
	pha
	sec
	lda #0
	delay_n 26
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 7954)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #197
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 7955)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	delay_n 26
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 7956)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #64
	lda #88
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 7957)
	; 18 bytes
	php
	pha
	tya
	ldy #126
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
.elseif(n = 7958)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7959)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #166
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	pla
	plp
.elseif(n = 7960)
	; 19 bytes
	php
	pha
	lda #91
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #30
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7961)
	; 18 bytes
	php
	pha
	tya
	ldy #209
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 7962)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7963)
	; 9 bytes
	php
	pha
	lda #31
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7964)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #189
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 7966)
	; 11 bytes
	php
	pha
	lda $A5
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
.endif
.else
; 7969 to 8125
.if(n = 7969 || n = 7970 || n = 7972 || n = 7974 || n = 7977 || n = 7979 || n = 7981 || n = 7984)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 7971)
	; 11 bytes
	php
	pha
	lda $A5
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7973)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 7976)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 7978)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 7980)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 7982)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 7983)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7985)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7986)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 7988)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7989)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 7990)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 7991)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 7992 && n < 7997))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 7996 && n < 8002) || (n > 8002 && n < 8007) || (n > 8007 && n < 8126))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.endif
.else
.if((n >= 8126) && (n < 8438))
.if((n >= 8126) && (n < 8282))
.if((n >= 8126) && (n < 8204))
.if((n >= 8126) && (n < 8165))
; 8126 to 8164
.if((n > 8125 && n < 8165))
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
	lda #(n-264)/255
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
.elseif(n = 8171 || n = 8172)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
.elseif(n = 8175)
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
.elseif(n = 8176)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8177)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8178)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8179)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8180)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8181)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8182)
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
.elseif(n = 8183)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 8184 to 8203
.if(n = 8184)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8185)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8186)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #136
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8187)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #172
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	pla
	plp
.elseif(n = 8188)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #232
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 8189)
	; 18 bytes
	php
	pha
	tya
	ldy #215
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8190)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8191)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8192)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8193)
	; 18 bytes
	php
	pha
	tya
	ldy #121
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8194)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #203
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 8195)
	; 18 bytes
	php
	pha
	tya
	ldy #72
	pha
	lda #71
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8196)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #215
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8197)
	; 18 bytes
	php
	pha
	tya
	ldy #174
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8198)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8199)
	; 17 bytes
	php
	pha
	sec
	lda #248 ;hides 'sed'
	delay_n 26
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 8200)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #66
	lda #88
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8201)
	; 17 bytes
	php
	pha
	sec
	lda #248
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8202)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #146
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8203)
	; 17 bytes
	php
	pha
	tya
	ldy #248
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 8204) && (n < 8243))
.if((n >= 8204) && (n < 8223))
; 8204 to 8222
.if(n = 8204)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #53
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8205)
	; 17 bytes
	php
	pha
	sec
	lda #234 ;hides 'nop'
	delay_n 28
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 8206)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8207)
	; 17 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	delay_n 28
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8208)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 8209)
	; 18 bytes
	php
	pha
	tya
	ldy #130
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
.elseif(n = 8210)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #186
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 8211)
	; 18 bytes
	php
	pha
	tya
	ldy #0
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8212 || n = 8221)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 8213)
	; 18 bytes
	php
	pha
	tya
	ldy #33
	pha
	lda #200
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8214)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8215)
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #31
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8216)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #210
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8217)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #162
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 8218)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #63
	lda #94
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8219)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8220)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #241
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8222)
	; 11 bytes
	php
	pha
	lda $A5
	lda #32
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 8223 to 8242
.if(n = 8223 || n = 8225 || n = 8226 || n = 8228 || n = 8230 || n = 8233 || n = 8235 || n = 8237 || n = 8240)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8224)
	; 9 bytes
	php
	pha
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8227)
	; 11 bytes
	php
	pha
	lda $A5
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8229)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 8232)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8234)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8236)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8238)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8239)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8241)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8242)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
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
.elseif(n = 8244)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8245)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8246)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8247)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 8248 && n < 8253))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 8252 && n < 8258) || (n > 8258 && n < 8263) || (n > 8263 && n < 8282))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 8281 && n < 8425))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 8427 || n = 8428)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8429)
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
.elseif(n = 8430)
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
.elseif(n = 8431)
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
.elseif(n = 8432)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8433)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8434)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8435)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8436)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8437)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 8438) && (n < 8594))
.if((n >= 8438) && (n < 8516))
.if((n >= 8438) && (n < 8477))
.if((n >= 8438) && (n < 8457))
; 8438 to 8456
.if(n = 8438)
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
.elseif(n = 8439)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8440)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #32
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8441)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #51
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8442)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #31
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8443)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8444)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8445)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 8446)
	; 18 bytes
	php
	pha
	tya
	ldy #159
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8447)
	; 18 bytes
	php
	pha
	tya
	ldy #196
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
.elseif(n = 8448)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 8449)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jpl *-2
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 8450)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8451)
	; 18 bytes
	php
	pha
	tya
	ldy #68
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8452)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #62
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 8453)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #159
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8454)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8455)
	; 18 bytes
	php
	pha
	tya
	ldy #222
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8456)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #69
	dey
	Jpl *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.endif
.else
; 8457 to 8476
.if(n = 8457)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #168
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 8458)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8459)
	; 18 bytes
	php
	pha
	tya
	ldy #39
	pha
	lda #170
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8460)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #112
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 8461)
	; 17 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 1051
	Jne *-10
	pla
	plp
.elseif(n = 8462)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #228
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8463)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8464)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	tya
	ldy #238
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 8465)
	; 17 bytes
	php
	pha
	sec
	lda #0
	delay_n 28
	sbc #1
	Jne *-8
	pla
	plp
.elseif(n = 8466)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #211
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8467)
	; 17 bytes
	php
	pha
	tya
	ldy #0
	delay_n 28
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8468)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	ldx #240
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 8469)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8470)
	; 18 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #36
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 8471)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #208
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 8472)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8473)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #108
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 8474)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #192
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 8475)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8476)
	; 18 bytes
	delay_n (n-2)
	nop
.endif
.endif
.else
; 8477 to 8515
.if(n = 8477 || n = 8479 || n = 8481 || n = 8482 || n = 8484 || n = 8486 || n = 8489 || n = 8491 || n = 8493 || n = 8496)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8478)
	; 11 bytes
	php
	pha
	lda $A5
	lda #33
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8480)
	; 9 bytes
	php
	pha
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8483)
	; 11 bytes
	php
	pha
	lda $A5
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8485)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 8488)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8490)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 8492)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 8494)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 8495)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8497)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8498)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 8500)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8501)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8502)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8503)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 8504 && n < 8509))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 8508 && n < 8514) || n = 8515)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 8515 && n < 8519) || (n > 8519 && n < 8594))
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
.endif
.endif
.else
.if((n >= 8594) && (n < 8672))
; 8594 to 8671
.if((n > 8593 && n < 8672))
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
.if((n >= 8672) && (n < 8711))
.if((n >= 8672) && (n < 8691))
; 8672 to 8690
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
	lda #(n-267)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8683 || n = 8684)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
.elseif(n = 8685)
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
.elseif(n = 8688)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8689)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8690)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 8691 to 8710
.if(n = 8691)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8692)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8693)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8694)
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
.elseif(n = 8695)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8696)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8697)
	; 17 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	delay_n 28
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8698)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-297)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8699)
	; 18 bytes
	php
	pha
	tya
	ldy #69
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8700)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8701)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8702)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8703)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8704)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8705)
	; 18 bytes
	php
	pha
	tya
	ldy #202
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
.elseif(n = 8706)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #155
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8707)
	; 18 bytes
	php
	pha
	tya
	ldy #181
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8708)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8709)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	pha
	tya
	ldy #245
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 8710)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #18
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
.if((n >= 8711) && (n < 8731))
; 8711 to 8730
.if(n = 8711)
	; 18 bytes
	php
	pha
	tya
	ldy #164
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8712)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181 ;hides 'lda zp,x'
	nop
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 8713)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8714)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #216
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 8715)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8716)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #79
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8717)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8718)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8719)
	; 18 bytes
	php
	pha
	tya
	ldy #174
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8720)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8721)
	; 18 bytes
	php
	pha
	tya
	ldy #229
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8722)
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8723)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8724)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 8725)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #33
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8726)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #145
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8727)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #76
	lda #79
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8728)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8729)
	; 18 bytes
	php
	pha
	tya
	ldy #130
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8730)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #0
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 8731 to 8750
.if(n = 8731)
	; 9 bytes
	php
	pha
	lda #34
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8732)
	; 18 bytes
	php
	pha
	sec
	lda #249
	nop
	delay_n 28
	sbc #1
	Jne *-9
	pla
	plp
.elseif(n = 8733 || n = 8735 || n = 8737 || n = 8738 || n = 8740 || n = 8742 || n = 8745 || n = 8747 || n = 8749)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 8734)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 8739)
	; 11 bytes
	php
	pha
	lda $A5
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8741)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 8744)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 8746)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
.endif
.else
.if((n >= 8751) && (n < 9376))
.if((n >= 8751) && (n < 9063))
.if((n >= 8751) && (n < 8907))
; 8751 to 8906
.if(n = 8751)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8752)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 8753)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8754)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 8756)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8757)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8758)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8759)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 8760 && n < 8765))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 8764 && n < 8770) || (n > 8770 && n < 8775) || (n > 8775 && n < 8907))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
	; 15 bytes
	php
	pha
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-267)/255
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 8939 || n = 8940)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 8945)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
.if((n >= 8946) && (n < 8965))
; 8946 to 8964
.if(n = 8946)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8947)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8948)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8949)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8950)
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
.elseif(n = 8951)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8952)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #88
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 8953)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #79
	lda #79
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8954)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/36
	clc
	lda #10 ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8955)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8956)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #235
	lda #4
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8957)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8958)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 8959)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8960)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 8961)
	; 18 bytes
	php
	pha
	tya
	ldy #33
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8962)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-290)/271
	pha
	lda #230
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8963)
	; 18 bytes
	php
	pha
	tya
	ldy #208
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
.elseif(n = 8964)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 8965 to 8984
.if(n = 8965)
	; 18 bytes
	php
	pha
	tya
	ldy #142
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
.elseif(n = 8966)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8967)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8968)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #33
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8969)
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8970)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #208
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8971)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #31
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 8972)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8973)
	; 18 bytes
	php
	pha
	tya
	ldy #73
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 8974)
	; 18 bytes
	php
	pha
	tya
	ldy #199
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8975)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8976)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8977)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #117
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 8978)
	; 18 bytes
	php
	pha
	tya
	ldy #31
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 8979)
	; 18 bytes
	php
	pha
	tya
	ldy #69
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
.elseif(n = 8980)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-147)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 8981)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8982)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #34
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8983)
	; 17 bytes
	php
	pha
	tya
	ldy #(n-19)/36
	pha
	lda #10 ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-7
	tay
	pla
	plp
.elseif(n = 8984)
	; 18 bytes
	php
	pha
	tya
	ldy #163
	pha
	lda #14
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
.else
.if((n >= 8985) && (n < 9024))
; 8985 to 9023
.if(n = 8985 || n = 8989 || n = 8991 || n = 8993 || n = 8994 || n = 8996 || n = 8998 || n = 9001 || n = 9003 || n = 9005 || n = 9008)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 8986)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #160
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 8987)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8988)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #130
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 8990)
	; 11 bytes
	php
	pha
	lda $A5
	lda #35
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 8992)
	; 9 bytes
	php
	pha
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8995)
	; 11 bytes
	php
	pha
	lda $A5
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 8997)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 9000)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9002)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9004)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9006)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9007)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9009)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9010)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 9012)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9013)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9014)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9015)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 9016 && n < 9021))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9020 && n < 9024))
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
; 9024 to 9062
.if(n = 9024 || n = 9025 || (n > 9026 && n < 9031) || (n > 9031 && n < 9063))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.else
.if((n >= 9063) && (n < 9219))
.if((n >= 9063) && (n < 9141))
; 9063 to 9140
.if((n > 9062 && n < 9141))
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
.if((n >= 9141) && (n < 9180))
; 9141 to 9179
.if((n > 9140 && n < 9180))
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
.if((n >= 9180) && (n < 9199))
; 9180 to 9198
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
.elseif(n = 9195 || n = 9196)
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
.endif
.else
; 9199 to 9218
.if(n = 9199)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9201)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9202)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9203)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9204)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9205)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9206)
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
.elseif(n = 9207)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9208)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #128
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 9209)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9210)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9211)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9212)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9213)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #199
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	pla
	plp
.elseif(n = 9214)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9215)
	; 18 bytes
	php
	pha
	tya
	ldy #242
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9216)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9217)
	; 18 bytes
	php
	pha
	tya
	ldy #146
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
.elseif(n = 9218)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 9219) && (n < 9297))
.if((n >= 9219) && (n < 9258))
.if((n >= 9219) && (n < 9238))
; 9219 to 9237
.if(n = 9219)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9220)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #40
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 9221)
	; 18 bytes
	php
	pha
	tya
	ldy #214
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
.elseif(n = 9222)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #209
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 9223)
	; 18 bytes
	php
	pha
	tya
	ldy #77
	pha
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 9224)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9225)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9226)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9227)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9228)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9229)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9230)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #236
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9231)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #227
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 9232)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-130)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 9233)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #59
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9234)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #229
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #35
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9237)
	; 18 bytes
	delay_n (n-2)
	nop
.endif
.else
; 9238 to 9257
.if(n = 9238)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9239)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #249
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9240)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-297)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9241)
	; 18 bytes
	php
	pha
	tya
	ldy #174
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9242)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	clc
	lda #$0A ;hides 'asl a'
	Jcc *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9243)
	; 9 bytes
	php
	pha
	lda #36
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9244)
	; 18 bytes
	php
	pha
	tya
	ldy #74
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
.elseif(n = 9245 || n = 9247 || n = 9249 || n = 9250 || n = 9252 || n = 9254 || n = 9257)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 9246)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 9251)
	; 11 bytes
	php
	pha
	lda $A5
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9253)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.else
; 9258 to 9296
.if(n = 9258)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9259 || n = 9261 || n = 9264)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 9260)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9262)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9263)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9265)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9266)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 9268)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9269)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9270)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9271)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 9272 && n < 9277))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9276 && n < 9282) || (n > 9282 && n < 9287) || (n > 9287 && n < 9297))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 9296 && n < 9376))
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
.else
.if((n >= 9376) && (n < 9688))
.if((n >= 9376) && (n < 9532))
.if((n >= 9376) && (n < 9454))
; 9376 to 9453
.if((n > 9375 && n < 9449))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 9451 || n = 9452)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9457)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9458)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9459)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9460)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9461)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9462)
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
.elseif(n = 9463)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9464)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9465)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9466)
	; 18 bytes
	php
	pha
	tya
	ldy #141
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9467)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9468)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9469)
	; 18 bytes
	php
	pha
	tya
	ldy #150
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
.elseif(n = 9470)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9471)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #233
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 9472)
	; 18 bytes
	php
	pha
	tya
	ldy #69
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 9473 to 9492
.if(n = 9473)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #201
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9474)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #235
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 9475)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9476)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9477)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9478)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #(n-264)/271
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9479)
	; 18 bytes
	php
	pha
	tya
	ldy #220
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
.elseif(n = 9480)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #163
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9481)
	; 18 bytes
	php
	pha
	tya
	ldy #249
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9482)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9483)
	; 18 bytes
	php
	pha
	tya
	ldy #91
	pha
	lda #63
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9484)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #74
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 9485)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #35
	pha
	lda #222
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 9488)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 9489)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9490)
	; 18 bytes
	php
	pha
	tya
	ldy #76
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
.elseif(n = 9491)
	; 18 bytes
	php
	pha
	tya
	ldy #73
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
.elseif(n = 9492)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 9493) && (n < 9512))
; 9493 to 9511
.if(n = 9493)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #36
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9494)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #35
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9495)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9498)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #3
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9499)
	; 9 bytes
	php
	pha
	lda #37
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9500 || n = 9501 || n = 9503 || n = 9505 || n = 9506 || n = 9508 || n = 9510)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 9502)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 9507)
	; 11 bytes
	php
	pha
	lda $A5
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9509)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.endif
.else
; 9512 to 9531
.if(n = 9512)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9513 || n = 9515 || n = 9517 || n = 9520)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 9514)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9516)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9518)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9519)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9521)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9522)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 9524)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9525)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9526)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9527)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 9528 && n < 9532))
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
; 9532 to 9687
.if(n = 9532)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9532 && n < 9538) || (n > 9538 && n < 9543) || (n > 9543 && n < 9688))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 9707 || n = 9708)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9713)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9714)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9715)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9716)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9717)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9718)
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
.elseif(n = 9719)
	; 18 bytes
	php
	pha
	tya
	ldy #194
	pha
	lda #9
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9720)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9721)
	; 18 bytes
	php
	pha
	tya
	ldy #154
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
.elseif(n = 9722)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #67
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9723)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9724)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9725)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9726)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #66
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.else
.if((n >= 9727) && (n < 9746))
; 9727 to 9745
.if(n = 9727)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #88
	lda #75
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9728)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #231
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9729)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9730)
	; 18 bytes
	php
	pha
	tya
	ldy #82
	pha
	lda #76
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 9731)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9732)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9733)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9734)
	; 18 bytes
	php
	pha
	tya
	ldy #145
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9735)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #72
	lda #99
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9736)
	; 18 bytes
	php
	pha
	tya
	ldy #79
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
.elseif(n = 9737)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #249
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9738)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9739)
	; 18 bytes
	php
	pha
	tya
	ldy #216
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9740)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #37
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9741)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #145
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9742)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #77
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 9743)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9744)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9745)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	pla
	plp
.endif
.else
; 9746 to 9765
.if(n = 9746)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #162
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9747)
	; 18 bytes
	php
	pha
	tya
	ldy #0
	pha
	sec
	lda #$0A ;hides 'asl a'
	Jpl *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9748)
	; 18 bytes
	php
	pha
	tya
	ldy #207
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9749)
	; 18 bytes
	php
	pha
	tya
	ldy #69
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #240
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 9752 || n = 9757 || n = 9759 || n = 9761 || n = 9762 || n = 9764)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 9753)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9754)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #242
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 9755)
	; 9 bytes
	php
	pha
	lda #38
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9756)
	; 18 bytes
	php
	pha
	tya
	ldy #91
	pha
	lda #66
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9758)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 9763)
	; 11 bytes
	php
	pha
	lda $A5
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9765)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 9766 to 9843
.if(n = 9766 || n = 9769 || n = 9771 || n = 9773 || n = 9776)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 9768)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 9770)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 9772)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 9774)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 9775)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9777)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9778)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 9780)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9781)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9782)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 9783)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 9784 && n < 9789))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 9788 && n < 9794) || (n > 9794 && n < 9799) || (n > 9799 && n < 9844))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 9843 && n < 9922))
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
.elseif(n = 9963 || n = 9964)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9969)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 9970)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9971)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9972)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9973)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9976)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #16
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #237
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 9981 to 10000
.if(n = 9981)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9982)
	; 18 bytes
	php
	pha
	tya
	ldy #81
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
.elseif(n = 9983)
	; 18 bytes
	php
	pha
	tya
	ldy #188
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 9984)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #213
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 9985)
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
.elseif(n = 9986)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #166
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9987)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #71
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 9988)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #58
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 9989)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9990)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-132)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9991)
	; 18 bytes
	php
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	pha
	tya
	ldy #246
	dey
	Jne *-1
	tay
	pla
	Jcc *-11
	pla
	plp
.elseif(n = 9992)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9993)
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #0
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 9994)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #248
	dex
	Jne *-1
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 9995)
	; 18 bytes
	php
	pha
	tya
	ldy #232
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
.elseif(n = 9996)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 9997)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 9998)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #35
	lda #243
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 9999)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #139
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 10000)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	iny
	Jne *-7
	pla
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
.if((n >= 10001) && (n < 10020))
; 10001 to 10019
.if(n = 10001)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10002)
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10003)
	; 18 bytes
	php
	pha
	tya
	ldy #77
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
.elseif(n = 10004)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10005)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10006)
	; 19 bytes
	php
	pha
	tya
	ldy #228 ;hides 'cpx zp'
	dey ;first loop only
	pha
	lda #0
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-10
	tay
	pla
	plp
.elseif(n = 10007)
	; 18 bytes
	php
	pha
	tya
	ldy #227
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10008)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10009)
	; 18 bytes
	php
	pha
	tya
	ldy #222
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10010)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10011)
	; 9 bytes
	php
	pha
	lda #39
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10012)
	; 19 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #38
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10013 || n = 10015 || n = 10017 || n = 10018)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 10014)
	; 11 bytes
	php
	pha
	lda $A5
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
.elseif(n = 10019)
	; 11 bytes
	php
	pha
	lda $A5
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10020 to 10039
.if(n = 10020 || n = 10022 || n = 10025 || n = 10027 || n = 10029 || n = 10032)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10021)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 10024)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10026)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10028)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10030)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10031)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10033)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10034)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 10036)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10037)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10038)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10039)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
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
.elseif((n > 10040 && n < 10045))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10044 && n < 10050) || (n > 10050 && n < 10055) || (n > 10055 && n < 10079))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 10078 && n < 10157))
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
.else
.if((n >= 10157) && (n < 10235))
; 10157 to 10234
.if((n > 10156 && n < 10217))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 10219 || n = 10220)
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10225)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10226)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10227)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10228)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10229)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10230)
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
.elseif(n = 10231)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10232)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #8
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10233)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/41
	lda #233 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10236)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 10237)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-150)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 10238)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10239)
	; 18 bytes
	php
	pha
	tya
	ldy #72
	pha
	lda #99
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 10240)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10241)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10242)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10243)
	; 18 bytes
	php
	pha
	tya
	ldy #213
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10244)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-157)/131
	lda #97
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 10245)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10246)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #146
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10247)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10248)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10249)
	; 18 bytes
	php
	pha
	tya
	ldy #93
	pha
	lda #69
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10250)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10251)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #39
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10252)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 10253)
	; 18 bytes
	php
	pha
	tya
	ldy #238
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
.endif
.else
; 10254 to 10273
.if(n = 10254)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #74
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 10255)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #193
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10256)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10257)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #213
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 10258)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #83
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 10259)
	; 18 bytes
	php
	pha
	tya
	ldy #79
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
.elseif(n = 10260)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #238
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10261)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #88
	lda #81
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 10262 || n = 10269 || n = 10271 || n = 10273)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 10263)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10264)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #220
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 10265)
	; 18 bytes
	php
	pha
	tya
	ldy #218
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10266)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #6
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10267)
	; 9 bytes
	php
	pha
	lda #40
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10268)
	; 18 bytes
	php
	pha
	tya
	ldy #37
	pha
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10270)
	; 11 bytes
	php
	pha
	lda $A5
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
.endif
.endif
.else
; 10274 to 10312
.if(n = 10274 || n = 10276 || n = 10278 || n = 10281 || n = 10283 || n = 10285 || n = 10288)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 10275)
	; 11 bytes
	php
	pha
	lda $A5
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10277)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 10280)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10282)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10284)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10286)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10287)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10289)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10290)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 10292)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10293)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10294)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10295)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 10296 && n < 10301))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10300 && n < 10306) || (n > 10306 && n < 10311) || n = 10312)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 10312 && n < 10469))
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
.elseif(n = 10475 || n = 10476)
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
.elseif(n = 10480)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10481)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10482)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10483)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10484)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10485)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10486)
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
.elseif(n = 10487)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10488 to 10507
.if(n = 10488)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10489)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #144
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 10490)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #75
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10491)
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
.elseif(n = 10492)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10493)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10494)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10495)
	; 18 bytes
	php
	pha
	tya
	ldy #96
	pha
	lda #67
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 10496)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #151
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 10497)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #37
	lda #249
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-150)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 10500)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 10501)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10502)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #96
	lda #74
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 10503)
	; 18 bytes
	php
	pha
	tya
	ldy #234 ;hides 'nop'
	pha
	lda #55
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 10504)
	; 18 bytes
	php
	pha
	tya
	ldy #233
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10505)
	; 18 bytes
	php
	pha
	tya
	ldy #214
	pha
	lda #8
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10506)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-157)/131
	lda #97
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 10507)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.else
.if((n >= 10508) && (n < 10527))
; 10508 to 10526
.if(n = 10508)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #40
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #39
	pha
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #98
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10513)
	; 18 bytes
	php
	pha
	tya
	ldy #198
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10514)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #39
	lda #235
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10518)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #244
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10519)
	; 18 bytes
	php
	pha
	tya
	ldy #250
	pha
	lda #1
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10520)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-132)/106
	lda #72
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 10521)
	; 18 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	pha
	tya
	ldy #248 ;hides 'sed'
	dey
	Jne *-2
	tay
	pla
	Jcc *-10
	pla
	plp
.elseif(n = 10522)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10523)
	; 9 bytes
	php
	pha
	lda #41
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10524)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10525)
	; 10 bytes
	delay_n (n-2)
	nop
.elseif(n = 10526)
	; 11 bytes
	php
	pha
	lda $A5
	lda #41
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.else
; 10527 to 10546
.if(n = 10527 || n = 10529 || n = 10530 || n = 10532 || n = 10534 || n = 10537 || n = 10539 || n = 10541 || n = 10544)
	; 10..14 bytes
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
.elseif(n = 10531)
	; 11 bytes
	php
	pha
	lda $A5
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10533)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 10536)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10538)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10540)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10542)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10543)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10545)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10546)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 10548)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10549)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10550)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10551)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 10552 && n < 10557))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10556 && n < 10562) || (n > 10562 && n < 10567) || (n > 10567 && n < 10626))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 10625 && n < 10704))
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
.elseif(n = 10731 || n = 10732)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10737)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10738)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10739)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10740)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10741)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10744)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10745)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10746)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #160
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10747)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10748)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10749)
	; 18 bytes
	php
	pha
	tya
	ldy #185
	pha
	lda #17
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10750)
	; 18 bytes
	php
	pha
	tya
	ldy #219
	pha
	lda #8
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10751)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #195
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10752)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10753)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10754)
	; 18 bytes
	php
	pha
	tya
	ldy #95
	pha
	lda #72
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10755)
	; 18 bytes
	php
	pha
	tya
	ldy #244
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10756)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #90
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 10757)
	; 18 bytes
	php
	pha
	tya
	ldy #91
	pha
	lda #77
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10758)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10759)
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10760)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10761)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #95
	lda #79
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 10762 to 10781
.if(n = 10762)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #244
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
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
.elseif(n = 10764 || n = 10775 || n = 10781)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 10765)
	; 18 bytes
	php
	pha
	tya
	ldy #199
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10766)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10767)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #41
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10768)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #82
	lda #97
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10769)
	; 18 bytes
	php
	pha
	tya
	ldy #250
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
.elseif(n = 10770)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #158
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10771)
	; 18 bytes
	php
	pha
	tya
	ldy #63
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
.elseif(n = 10772)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10773)
	; 18 bytes
	php
	pha
	tya
	ldy #37
	pha
	lda #242
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 10774)
	; 18 bytes
	php
	pha
	tya
	ldy #239
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10776)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #250
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 10777)
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
.elseif(n = 10778)
	; 18 bytes
	php
	pha
	tya
	ldy #203
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 10779)
	; 9 bytes
	php
	pha
	lda #42
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10780)
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
.endif
.endif
.endif
.endif
.else
; 10782 to 10937
.if(n = 10782)
	; 11 bytes
	php
	pha
	lda $A5
	lda #42
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10783 || n = 10785 || n = 10786 || n = 10788 || n = 10790 || n = 10793 || n = 10795 || n = 10797 || n = 10800)
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
.elseif(n = 10787)
	; 11 bytes
	php
	pha
	lda $A5
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10789)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 10792)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 10794)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 10796)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 10798)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 10799)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10801)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10802)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 10804)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10805)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10806)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 10807)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 10808 && n < 10813))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 10812 && n < 10818) || (n > 10818 && n < 10823) || (n > 10823 && n < 10938))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.else
.if((n >= 10938) && (n < 11094))
.if((n >= 10938) && (n < 11016))
.if((n >= 10938) && (n < 10977))
; 10938 to 10976
.if((n > 10937 && n < 10977))
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
.elseif(n = 10987 || n = 10988)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10993)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 10994)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10995)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 10996 to 11015
.if(n = 10996)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 10997)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	ldy #244
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11000)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #84
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11001)
	; 18 bytes
	php
	pha
	tya
	ldy #37
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11004)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11005)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11006)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11007)
	; 18 bytes
	php
	pha
	tya
	ldy #164
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11008)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-130)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 11009)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #29
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11010)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11011)
	; 18 bytes
	php
	pha
	tya
	ldy #229
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11012)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11013)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11014)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #164
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11015)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-137)/111
	lda #77
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 11016) && (n < 11055))
.if((n >= 11016) && (n < 11035))
; 11016 to 11034
.if(n = 11016)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11017)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 11018)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11019)
	; 18 bytes
	php
	pha
	tya
	ldy #250
	pha
	lda #3
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11020)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11021)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11022)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #11
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 11023)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #157
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #0
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
.elseif(n = 11028)
	; 18 bytes
	delay_n (n-2)
	nop
.elseif(n = 11029)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #42
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11030)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #84
	lda #97
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #92
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #9
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
.elseif(n = 11036 || n = 11037 || n = 11039 || n = 11041 || n = 11042 || n = 11044 || n = 11046 || n = 11049 || n = 11051 || n = 11053)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 11038)
	; 11 bytes
	php
	pha
	lda $A5
	lda #43
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11040)
	; 9 bytes
	php
	pha
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11043)
	; 11 bytes
	php
	pha
	lda $A5
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11045)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 11048)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11050)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
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
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11056)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 11057)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11058)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 11060)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11061)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11062)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11063)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 11064 && n < 11069))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11068 && n < 11074) || (n > 11074 && n < 11079) || (n > 11079 && n < 11094))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 11093 && n < 11241))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 11243 || n = 11244)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11249)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11250)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11252)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11253)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11254)
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
.elseif(n = 11255)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11256)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11257)
	; 18 bytes
	php
	pha
	tya
	ldy #216 ;hides 'cld'
	pha
	lda #238
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 11258)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$69 ;hides 'adc #$18'
	clc ;first loop only
	ldx #56 ;hides 'sec'
	inx
	Jne *-2
	Jcc *-7
	pla
	tax
	pla
	plp
.elseif(n = 11259)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #239
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11260)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11261)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #107
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11262)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11263)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11264)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11265)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	pha
	lda #253
	jsr delay_a_27_clocks
	pla
	dex
	Jmi *-9
	tax
	pla
	plp
.elseif(n = 11266)
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
.elseif(n = 11267)
	; 18 bytes
	php
	pha
	tya
	ldy #152
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11268)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #154
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11269)
	; 18 bytes
	php
	pha
	tya
	ldy #250
	pha
	lda #4
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 11270 to 11289
.if(n = 11270)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 11271)
	; 18 bytes
	php
	pha
	tya
	ldy #96
	pha
	lda #75
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 11272)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/121
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11273)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #163
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11274)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #152
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11275)
	; 18 bytes
	php
	pha
	tya
	ldy #168
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11276)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #90
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11277)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #57
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 11278)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-142)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 11279)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11280)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #165
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 11281)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11282)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #201
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11283)
	; 18 bytes
	php
	pha
	tya
	ldy #87
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
.elseif(n = 11284)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11285)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-150)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 11286)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #43
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11287)
	; 19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.else
; 11290 to 11328
.if(n = 11290)
	; 17 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-5
	pla
	tay
	pla
	plp
.elseif(n = 11291)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11292 || n = 11293 || n = 11295 || n = 11297 || n = 11298 || n = 11300 || n = 11302 || n = 11305 || n = 11307 || n = 11309 || n = 11312)
	; 10..18 bytes
	delay_n (n-2)
	nop
.elseif(n = 11294)
	; 11 bytes
	php
	pha
	lda $A5
	lda #44
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11296)
	; 9 bytes
	php
	pha
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11299)
	; 11 bytes
	php
	pha
	lda $A5
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11301)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 11304)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11306)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11308)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11310)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11311)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11313)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11314)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 11316)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11317)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11318)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11319)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 11320 && n < 11325))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11324 && n < 11329))
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
.else
; 11329 to 11406
.if(n = 11329 || (n > 11330 && n < 11335) || (n > 11335 && n < 11407))
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
.endif
.endif
.else
.if((n >= 11407) && (n < 11485))
; 11407 to 11484
.if((n > 11406 && n < 11485))
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
.if((n >= 11485) && (n < 11524))
.if((n >= 11485) && (n < 11504))
; 11485 to 11503
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
.elseif(n = 11499 || n = 11500)
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
.endif
.else
; 11504 to 11523
.if(n = 11504)
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11505)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11506)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11507)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11508)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11509)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11510)
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
.elseif(n = 11511)
	; 18 bytes
	php
	pha
	tya
	ldy #221
	pha
	lda #11
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11512)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 11513)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11514)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/121
	pha
	lda #80
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11515)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11516)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11517)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11518)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11519)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/46
	pha
	lda #10 ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11520)
	; 18 bytes
	php
	pha
	tya
	ldy #217
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11521)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #95
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11522)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11523)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 11524) && (n < 11543))
; 11524 to 11542
.if(n = 11524)
	; 18 bytes
	php
	pha
	tya
	ldy #195
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11525)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11526)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-26)/46
	clc
	lda #10 ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11527)
	; 18 bytes
	php
	pha
	tya
	ldy #84
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11528)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11529)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11530)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11531)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #195
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11532)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #238
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 11533)
	; 18 bytes
	php
	pha
	tya
	ldy #202
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11534)
	; 18 bytes
	php
	pha
	tya
	ldy #245
	pha
	lda #6
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11535)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #44
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11536)
	; 19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #166
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
	; 18 bytes
	php
	pha
	tya
	ldy #172
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #192
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11547)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11548)
	; 18 bytes
	php
	pha
	tya
	ldy #183
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
.elseif(n = 11549 || n = 11551 || n = 11553 || n = 11554 || n = 11556 || n = 11558 || n = 11561)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11550)
	; 11 bytes
	php
	pha
	lda $A5
	lda #45
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11552)
	; 9 bytes
	php
	pha
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11555)
	; 11 bytes
	php
	pha
	lda $A5
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11557)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11562)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 11563) && (n < 11719))
; 11563 to 11718
.if(n = 11563 || n = 11565 || n = 11568)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 11564)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11566)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11567)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11569)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11570)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 11572)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11573)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11574)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11575)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 11576 && n < 11581))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11580 && n < 11586) || (n > 11586 && n < 11591) || (n > 11591 && n < 11719))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 11755 || n = 11756)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11761)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 11762)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11763)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11764)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11765)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11766)
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
.elseif(n = 11767)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 11768)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #145
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11772)
	; 18 bytes
	php
	pha
	tya
	ldy #161
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11773)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11774)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #88
	lda #98
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 11775)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #5 ;hides 'ora zp'
	iny ;first loop only
	lda #$4A ;hides 'lsr a'
	Jne *-1
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 11776)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #91
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.endif
.else
; 11777 to 11796
.if(n = 11777)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #167
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 11778)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11779)
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
.elseif(n = 11780)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11781)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11782)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11783)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11784)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #245 ;hides 'sbc zp,x'
	nop
	lda #$4A ;hides 'lsr a'
	Jne *-1
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 11785)
	; 18 bytes
	php
	pha
	tya
	ldy #222
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11786)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11787)
	; 18 bytes
	php
	pha
	tya
	ldy #213 ;hides 'cmp zp,x'
	iny ;first loop only
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 11788)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #45
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11791)
	; 18 bytes
	php
	pha
	txa
	ldx #202 ;hides 'dex'
	delay_n 53
	Jne *-9
	tax
	pla
	plp
.elseif(n = 11792)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #222
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11793)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #40
	lda #253
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 11794)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 11795)
	; 18 bytes
	php
	pha
	tya
	ldy #0
	pha
	lda #$0A ;hides 'asl a'
	sec
	Jpl *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
.if((n >= 11797) && (n < 11816))
; 11797 to 11815
.if(n = 11797)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #(n-277)/256
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #95
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11803)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11804)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 11805 || n = 11807 || n = 11809 || n = 11810 || n = 11812 || n = 11814)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 11806)
	; 11 bytes
	php
	pha
	lda $A5
	lda #46
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11808)
	; 9 bytes
	php
	pha
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11811)
	; 11 bytes
	php
	pha
	lda $A5
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11813)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.endif
.else
; 11816 to 11835
.if(n = 11816)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 11817 || n = 11819 || n = 11821 || n = 11824)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 11818)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 11820)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 11822)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 11823)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11825)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11826)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 11828)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11829)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 11830)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 11831)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 11832 && n < 11836))
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
; 11836 to 11875
.if(n = 11836)
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 11836 && n < 11842) || (n > 11842 && n < 11847) || (n > 11847 && n < 11876))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 11875 && n < 12009))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 12011 || n = 12012)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12017)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12018)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12019)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12020)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12021)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12022)
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
.elseif(n = 12023)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #93
	lda #95
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12024)
	; 18 bytes
	php
	pha
	tya
	ldy #245
	pha
	lda #8
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12025)
	; 18 bytes
	php
	pha
	tya
	ldy #86
	pha
	lda #97
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12026)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #200
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12027)
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
.elseif(n = 12028)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12029)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12030)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #51
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 12031)
	; 18 bytes
	php
	pha
	tya
	ldy #91
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
.if((n >= 12032) && (n < 12110))
.if((n >= 12032) && (n < 12071))
.if((n >= 12032) && (n < 12051))
; 12032 to 12050
.if(n = 12032)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #207
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12033)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12034)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #94
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12035)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12036)
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
.elseif(n = 12037)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #99
	jsr delay_a_27_clocks
	dey
	Jmi *-7
	pla
	tay
	pla
	plp
.elseif(n = 12040)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12041)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #135
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12042)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #46
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12043)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12044)
	; 18 bytes
	php
	pha
	tya
	ldy #185
	pha
	lda #24
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #96
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12048)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #131
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 12049)
	; 19 bytes
	delay_n (n-2)
	nop
.elseif(n = 12050)
	; 18 bytes
	php
	pha
	tya
	ldy #227
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
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
	; 18 bytes
	php
	pha
	tya
	ldy #191
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
.elseif(n = 12056 || n = 12057 || n = 12061 || n = 12063 || n = 12065 || n = 12066 || n = 12068 || n = 12070)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 12058)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #13
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12059)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12060)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #66
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 12062)
	; 11 bytes
	php
	pha
	lda $A5
	lda #47
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12064)
	; 9 bytes
	php
	pha
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12067)
	; 11 bytes
	php
	pha
	lda $A5
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12069)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 12072)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12073 || n = 12075 || n = 12077 || n = 12080)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12074)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12076)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12078)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12079)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12081)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12082)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 12084)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12085)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12086)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12087)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 12088 && n < 12093))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12092 && n < 12098) || (n > 12098 && n < 12103) || (n > 12103 && n < 12110))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 12109 && n < 12188))
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
.if((n >= 12188) && (n < 12344))
.if((n >= 12188) && (n < 12266))
; 12188 to 12265
.if((n > 12187 && n < 12265))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 12267 || n = 12268)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12273)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12274)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12275)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12276)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12277)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12278)
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
.elseif(n = 12279)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #179
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 12280)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12281)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #95
	lda #95
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 12286)
	; 18 bytes
	php
	pha
	tya
	ldy #141
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12287)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #201
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #24 ;hides 'clc'
	pha
	lda #75
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 12290)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #219
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12291)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12292)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12295)
	; 18 bytes
	php
	pha
	tya
	ldy #98
	pha
	lda #83
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12298)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #236
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12301)
	; 18 bytes
	php
	pha
	tya
	ldy #88
	pha
	lda #97
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #166
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
.if((n >= 12305) && (n < 12324))
; 12305 to 12323
.if(n = 12305 || n = 12312 || n = 12317 || n = 12319 || n = 12321 || n = 12322)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 12306)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 12307)
	; 18 bytes
	php
	pha
	tya
	ldy #0
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	Jcs *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12308)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #178
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #195
	lda #(n-392)/411
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #14
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12315)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12316)
	; 19 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #47
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12318)
	; 11 bytes
	php
	pha
	lda $A5
	lda #48
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12320)
	; 9 bytes
	php
	pha
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12323)
	; 11 bytes
	php
	pha
	lda $A5
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 12324 to 12343
.if(n = 12324 || n = 12326 || n = 12329 || n = 12331 || n = 12333 || n = 12336)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12325)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 12328)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12330)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12332)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12334)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12335)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12337)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12338)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 12340)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12341)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12342)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12343)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
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
.elseif((n > 12344 && n < 12349))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12348 && n < 12354) || (n > 12354 && n < 12359) || (n > 12359 && n < 12501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 12523 || n = 12524)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12529)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12530)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12531)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12532)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12533)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	ldy #149
	pha
	lda #43
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12536)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #50
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12543)
	; 18 bytes
	php
	pha
	tya
	ldy #202
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #117
	lda #73
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12546)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12547)
	; 18 bytes
	php
	pha
	tya
	ldy #232
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12548)
	; 18 bytes
	php
	pha
	tya
	ldy #187
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12551)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12552)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #216 ;hides 'cld'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 12553)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #48
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #199
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #241
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 12560 || n = 12567 || n = 12573 || n = 12575 || n = 12577 || n = 12578)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	ldy #(n-130)/111
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
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #224
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12571)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12572)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #17
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12574)
	; 11 bytes
	php
	pha
	lda $A5
	lda #49
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12576)
	; 9 bytes
	php
	pha
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 12579 to 12656
.if(n = 12579)
	; 11 bytes
	php
	pha
	lda $A5
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12580 || n = 12582 || n = 12585 || n = 12587 || n = 12589 || n = 12592)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12581)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 12584)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12586)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12588)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12590)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12591)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12593)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12594)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 12596)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12597)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12598)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12599)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 12600 && n < 12605))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12604 && n < 12610) || (n > 12610 && n < 12615) || (n > 12615 && n < 12657))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 12656 && n < 12735))
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
.elseif(n = 12779 || n = 12780)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12785)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 12786)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12787)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12788)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12789)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12792)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
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
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12794)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #228
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12799)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #241
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #175
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12802)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12805)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12806)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12807)
	; 18 bytes
	php
	pha
	tya
	ldy #91
	pha
	lda #98
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 12808)
	; 18 bytes
	php
	pha
	tya
	ldy #203
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #49
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12816)
	; 18 bytes
	php
	pha
	tya
	ldy #191
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12817)
	; 18 bytes
	php
	pha
	tya
	ldy #237
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12818)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 12822 || n = 12825 || n = 12829 || n = 12831)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 12823)
	; 18 bytes
	php
	pha
	tya
	ldy #96
	pha
	lda #91
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
.elseif(n = 12826)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #108
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 12827)
	; 9 bytes
	php
	pha
	lda #50
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12828)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #173
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 12830)
	; 11 bytes
	php
	pha
	lda $A5
	lda #50
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.elseif(n = 12833 || n = 12834 || n = 12836 || n = 12838 || n = 12841 || n = 12843 || n = 12845 || n = 12848)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 12835)
	; 11 bytes
	php
	pha
	lda $A5
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12837)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 12840)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 12842)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 12844)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 12846)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 12847)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12849)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12850)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.if(n = 12852)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12853)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 12854)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 12855)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 12856 && n < 12861))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 12860 && n < 12866) || (n > 12866 && n < 12871) || (n > 12871 && n < 12891))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 12890 && n < 12969))
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
.else
.if((n >= 12969) && (n < 13047))
; 12969 to 13046
.if((n > 12968 && n < 13033))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 13035 || n = 13036)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13041)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13042)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13043)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13044)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13045)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13048)
	; 18 bytes
	php
	pha
	tya
	ldy #128
	pha
	lda #60
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13053)
	; 18 bytes
	php
	pha
	tya
	ldy #98
	pha
	lda #92
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13054)
	; 18 bytes
	php
	pha
	tya
	ldy #237
	pha
	lda #14
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13055)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #67
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13056)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #110
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 13057)
	; 18 bytes
	php
	pha
	tya
	ldy #246
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13058)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13059)
	; 18 bytes
	php
	pha
	tya
	ldy #163
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13060)
	; 18 bytes
	php
	pha
	tya
	ldy #207
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 13063)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13064)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13065)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #81
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13068)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #50
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13069)
	; 18 bytes
	php
	pha
	tya
	ldy #225
	pha
	lda #17
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13070)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #186
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #229
	pha
	lda #16
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 13077 || n = 13085)
	; 10..18 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #17
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13083)
	; 9 bytes
	php
	pha
	lda #51
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13084)
	; 18 bytes
	php
	pha
	tya
	ldy #195
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.endif
.else
; 13086 to 13125
.if(n = 13086)
	; 11 bytes
	php
	pha
	lda $A5
	lda #51
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13087 || n = 13089 || n = 13090 || n = 13092 || n = 13094 || n = 13097 || n = 13099 || n = 13101 || n = 13104)
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
.elseif(n = 13091)
	; 11 bytes
	php
	pha
	lda $A5
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13093)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 13096)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13098)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13100)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13102)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13103)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13105)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13106)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 13108)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13109)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13110)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13111)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 13112 && n < 13117))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13116 && n < 13122) || (n > 13122 && n < 13126))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 13126) && (n < 13438))
.if((n >= 13126) && (n < 13282))
; 13126 to 13281
.if(n = 13126 || (n > 13127 && n < 13282))
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
.elseif(n = 13291 || n = 13292)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13297)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13298)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13299)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13300)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 13301 to 13320
.if(n = 13301)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13304)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13305)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #49
	lda #237
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13306)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #166
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13307)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13308)
	; 18 bytes
	php
	pha
	tya
	ldy #96
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 13309)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13310)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #246
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #211
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
.elseif(n = 13313)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #128
	lda #69
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13314)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #151
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 13318)
	; 18 bytes
	php
	pha
	tya
	ldy #143
	pha
	lda #(n-6)/256
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13319)
	; 18 bytes
	php
	pha
	tya
	ldy #95
	pha
	lda #99
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13322)
	; 18 bytes
	php
	pha
	tya
	ldy #251
	pha
	clc
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
	; 19 bytes
	delay_n (n-2)
	nop
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
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13333)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #51
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	txa
	pha
	ldx #202 ;hides 'dex'
	lda #49
	jsr delay_a_27_clocks
	dex
	dex
	Jne *-8
	pla
	tax
	pla
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #153
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13338)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #18
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
.if(n = 13340)
	; 18 bytes
	php
	pha
	tya
	ldy #173
	pha
	lda #(n-344)/361
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13341 || n = 13343 || n = 13345 || n = 13346 || n = 13348 || n = 13350 || n = 13353 || n = 13355 || n = 13357)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 13342)
	; 11 bytes
	php
	pha
	lda $A5
	lda #52
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13344)
	; 9 bytes
	php
	pha
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13347)
	; 11 bytes
	php
	pha
	lda $A5
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13349)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 13352)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13354)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13356)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13358)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13359)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.else
; 13360 to 13437
.if(n = 13360)
	; 13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13361)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13362)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 13364)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13365)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13366)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13367)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 13368 && n < 13373))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13372 && n < 13378) || (n > 13378 && n < 13383) || (n > 13383 && n < 13438))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 13437 && n < 13516))
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
.elseif(n = 13547 || n = 13548)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13553)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13554)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13556)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13557)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13560)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #188
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13563)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13564)
	; 18 bytes
	php
	pha
	tya
	ldy #215
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13568)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #222
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13569)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-290)/271
	pha
	lda #230
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #215
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13572)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #13
	lda #4
	jsr delay_256a_11_clocks_b_
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13573)
	; 18 bytes
	php
	pha
	tya
	ldy #251
	pha
	lda #13
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.endif
.else
; 13574 to 13593
.if(n = 13574)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13575)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13576)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-297)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13577)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13578)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #242
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13579)
	; 18 bytes
	php
	pha
	tya
	ldy #226
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13580)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #98
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 13583)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #195
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 13584)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13585)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13586)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #226
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13587)
	; 18 bytes
	php
	pha
	tya
	ldy #0
	pha
	clc
	lda #$0A ;hides 'asl a'
	Jne *-1
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13588)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #52
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #234
	pha
	lda #17
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 13592)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #238
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13593)
	; 19 bytes
	delay_n (n-2)
	nop
.endif
.endif
.endif
.endif
.else
.if((n >= 13594) && (n < 13672))
.if((n >= 13594) && (n < 13633))
; 13594 to 13632
.if(n = 13594)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #19
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13595)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13596)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234 ;hides 'nop'
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 13597 || n = 13599 || n = 13601 || n = 13602 || n = 13604 || n = 13606 || n = 13609 || n = 13611 || n = 13613 || n = 13616)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 13598)
	; 11 bytes
	php
	pha
	lda $A5
	lda #53
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13600)
	; 9 bytes
	php
	pha
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13603)
	; 11 bytes
	php
	pha
	lda $A5
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13605)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 13608)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13610)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 13612)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13614)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13615)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13617)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13618)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 13620)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13621)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13622)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13623)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 13624 && n < 13629))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13628 && n < 13633))
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
; 13633 to 13671
.if(n = 13633 || (n > 13634 && n < 13639) || (n > 13639 && n < 13672))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.else
; 13672 to 13750
.if((n > 13671 && n < 13751))
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
.else
.if((n >= 13751) && (n < 14376))
.if((n >= 13751) && (n < 14063))
.if((n >= 13751) && (n < 13907))
.if((n >= 13751) && (n < 13829))
.if((n >= 13751) && (n < 13790))
; 13751 to 13789
.if((n > 13750 && n < 13790))
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
.elseif(n = 13803 || n = 13804)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
; 13809 to 13828
.if(n = 13809)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 13810)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13811)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13812)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13813)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13814)
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
.elseif(n = 13815)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13816)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13817)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 13818)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #206
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-147)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 13821)
	; 18 bytes
	php
	pha
	tya
	ldy #206
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #219
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #230
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #234
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13833)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13837)
	; 18 bytes
	php
	pha
	tya
	ldy #147
	pha
	lda #53
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #53
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #54
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #147
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 19 bytes
	delay_n (n-2)
	nop
.elseif(n = 13847)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #51
	lda #237
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #118
	dex
	Jpl *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 13850)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #20
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13851)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13852)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 13853 || n = 13855 || n = 13857 || n = 13858 || n = 13860 || n = 13862 || n = 13865 || n = 13867)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 13854)
	; 11 bytes
	php
	pha
	lda $A5
	lda #54
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13856)
	; 9 bytes
	php
	pha
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13859)
	; 11 bytes
	php
	pha
	lda $A5
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13861)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 13866)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.else
; 13868 to 13906
.if(n = 13868)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 13869 || n = 13872)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 13870)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 13871)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13873)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13874)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 13876)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13877)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 13878)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 13879)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 13880 && n < 13885))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 13884 && n < 13890) || (n > 13890 && n < 13895) || (n > 13895 && n < 13907))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 13906 && n < 14057))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 14059 || n = 14060)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14065)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14066)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14067)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14068)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14069)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14070)
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
.elseif(n = 14071)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #52
	lda #231
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14072)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 14078)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14079)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #120
	dex
	Jpl *-1
	Jcs *-7
	pla
	tax
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #47
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 14084)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #213
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #190
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #210
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14090)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14091)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #119
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #210
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14097)
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #54
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
.if((n >= 14102) && (n < 14121))
; 14102 to 14120
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
.elseif(n = 14104 || n = 14109 || n = 14111 || n = 14113 || n = 14114 || n = 14116 || n = 14118)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #21
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14107)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14108)
	; 18 bytes
	php
	pha
	tya
	ldy #193
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14110)
	; 11 bytes
	php
	pha
	lda $A5
	lda #55
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14112)
	; 9 bytes
	php
	pha
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14115)
	; 11 bytes
	php
	pha
	lda $A5
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14117)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 14120)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 14121 to 14140
.if(n = 14121 || n = 14123 || n = 14125 || n = 14128)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 14122)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14124)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14126)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14127)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14129)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14130)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 14132)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14133)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14134)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14135)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 14136 && n < 14141))
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
; 14141 to 14218
.if((n > 14140 && n < 14146) || (n > 14146 && n < 14151) || (n > 14151 && n < 14219))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.endif
.else
.if((n >= 14219) && (n < 14297))
; 14219 to 14296
.if((n > 14218 && n < 14297))
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
.elseif(n = 14315 || n = 14316)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14321)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14322)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14323)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14324)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14325)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14326)
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
.elseif(n = 14327)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14328)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #55
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14333)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #(n-625)/596
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #159
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14337)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14338)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-130)/111
	pha
	lda #70
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14341)
	; 18 bytes
	php
	pha
	tya
	ldy #231
	pha
	lda #21
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14342)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #55
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-137)/111
	lda #77
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #247
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14358 || n = 14365 || n = 14367 || n = 14369 || n = 14370 || n = 14372 || n = 14374)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jmi *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14363)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14364)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #214
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14366)
	; 11 bytes
	php
	pha
	lda $A5
	lda #56
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14368)
	; 9 bytes
	php
	pha
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14371)
	; 11 bytes
	php
	pha
	lda $A5
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14373)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.if(n = 14376)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14377 || n = 14379 || n = 14381 || n = 14384)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 14378)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14380)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14382)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14383)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14385)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14386)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 14388)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14389)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14390)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14391)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 14392 && n < 14397))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 14396 && n < 14402) || (n > 14402 && n < 14407) || (n > 14407 && n < 14532))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if(n = 14571 || n = 14572)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14577)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14578)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14579)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14580)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14581)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14582)
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
.elseif(n = 14583)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14584)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #251
	lda #24
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14588)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14591)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14592)
	; 18 bytes
	php
	pha
	tya
	ldy #247
	pha
	lda #18
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14593)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #7
	pha
	lda #7
	jsr delay_256a_16_clocks_b
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
.elseif(n = 14596)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #235
	lda #28
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14597)
	; 18 bytes
	php
	pha
	tya
	ldy #197
	pha
	lda #33
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #247
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14600)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #161
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #197
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14607)
	; 18 bytes
	php
	pha
	clc
	lda #$0A ;hides 'asl a'
	delay_n 1819
	Jne *-10
	pla
	plp
.elseif(n = 14608)
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #56
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14609)
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
	Jpl *-9
	tay
	pla
	plp
.endif
.endif
.endif
.else
.if((n >= 14610) && (n < 14649))
.if((n >= 14610) && (n < 14629))
; 14610 to 14628
.if(n = 14610)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2080
	Jcs *-11
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #178
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #23
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14619)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14620)
	; 18 bytes
	php
	pha
	tya
	ldy #157
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14621 || n = 14623 || n = 14625 || n = 14626 || n = 14628)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 14622)
	; 11 bytes
	php
	pha
	lda $A5
	lda #57
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14624)
	; 9 bytes
	php
	pha
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14627)
	; 11 bytes
	php
	pha
	lda $A5
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14629 to 14648
.if(n = 14629)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14630 || n = 14633 || n = 14635 || n = 14637 || n = 14640)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 14632)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14634)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14636)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14638)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14639)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14641)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14642)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 14644)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14645)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14646)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14647)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.else
; 14649 to 14687
.if((n > 14648 && n < 14653))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 14652 && n < 14658) || (n > 14658 && n < 14663) || (n > 14663 && n < 14688))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 14687 && n < 14825))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 14827 || n = 14828)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14833)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 14834)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14835)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14836)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14837)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14838)
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
.elseif(n = 14839)
	; 18 bytes
	php
	pha
	tya
	ldy #247
	pha
	lda #19
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14840)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #57
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #<(n/2-15)
	lda #>(n/2-15)
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14847)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/91
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14853)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14854)
	; 18 bytes
	php
	pha
	tya
	ldy #215
	pha
	lda #(n-546)/511
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 14855)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #153
	pha
	lda #56
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 14863 to 14882
.if(n = 14863)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #57
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	ldy #115
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
.elseif(n = 14870 || n = 14877 || n = 14879 || n = 14881 || n = 14882)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-142)/116
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 14875)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14876)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #225
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 14878)
	; 11 bytes
	php
	pha
	lda $A5
	lda #58
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14880)
	; 9 bytes
	php
	pha
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 14883 to 14921
.if(n = 14883)
	; 11 bytes
	php
	pha
	lda $A5
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14884 || n = 14886 || n = 14889 || n = 14891 || n = 14893 || n = 14896)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 14885)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 14888)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 14890)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 14892)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 14894)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 14895)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14897)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14898)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 14900)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14901)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 14902)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 14903)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 14904 && n < 14909))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 14908 && n < 14914) || (n > 14914 && n < 14919) || n = 14920 || n = 14921)
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 14921 && n < 15001))
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
.endif
.else
.if((n >= 15001) && (n < 17501))
.if((n >= 15001) && (n < 16251))
.if((n >= 15001) && (n < 15626))
.if((n >= 15001) && (n < 15313))
.if((n >= 15001) && (n < 15157))
.if((n >= 15001) && (n < 15079))
; 15001 to 15078
.if((n > 15000 && n < 15079))
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
.elseif(n = 15083 || n = 15084)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15089)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15090)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15091)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15092)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15093)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15094)
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
.elseif(n = 15095)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15096)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #76
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15103)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
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
	; 18 bytes
	php
	pha
	tya
	ldy #184 ;hides 'clv'
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15111)
	; 18 bytes
	php
	pha
	tya
	ldy #196
	pha
	lda #36
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15116)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15117)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #84
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #58
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15122)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #222
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	pha
	tya
	ldy #127
	dey
	Jpl *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 15127 || n = 15133 || n = 15135)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #229
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 15130)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #25
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15131)
	; 9 bytes
	php
	pha
	lda #59
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15132)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #182
	lda #49
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15134)
	; 11 bytes
	php
	pha
	lda $A5
	lda #59
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.if(n = 15137 || n = 15138 || n = 15140 || n = 15142 || n = 15145 || n = 15147 || n = 15149 || n = 15152)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15139)
	; 11 bytes
	php
	pha
	lda $A5
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15141)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 15144)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15146)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15148)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15150)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15151)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15153)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15154)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 15157 to 15312
.if(n = 15157)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15158)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15159)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 15160 && n < 15165))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15164 && n < 15170) || (n > 15170 && n < 15175) || (n > 15175 && n < 15313))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 15339 || n = 15340)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15345)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15346)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15347)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15348)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15349)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15350)
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
.elseif(n = 15351)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 15352) && (n < 15371))
; 15352 to 15370
.if(n = 15352)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #194
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #130
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 15357)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #233
	jsr delay_a_27_clocks
	dey
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 15358)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15359)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #57
	lda #235
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #231
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-9
	tay
	pla
	plp
.elseif(n = 15362)
	; 18 bytes
	php
	pha
	tya
	ldy #229
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #165
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15365)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #229
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #59
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #185
	pha
	lda #42
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #202
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 15380 || n = 15389)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 19 bytes
	php
	pha
	tya
	ldy #36 ;hides 'bit zp'
	nop
	pha
	lda #123
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-10
	tay
	pla
	plp
.elseif(n = 15383)
	; 18 bytes
	php
	pha
	tya
	ldy #167
	pha
	lda #51
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15387)
	; 9 bytes
	php
	pha
	lda #60
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15388)
	; 18 bytes
	php
	pha
	tya
	ldy #141
	pha
	lda #68
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15390)
	; 11 bytes
	php
	pha
	lda $A5
	lda #60
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.else
; 15391 to 15468
.if(n = 15391 || n = 15393 || n = 15394 || n = 15396 || n = 15398 || n = 15401 || n = 15403 || n = 15405 || n = 15408)
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
.elseif(n = 15395)
	; 11 bytes
	php
	pha
	lda $A5
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15397)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 15400)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15402)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15404)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15406)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15407)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15409)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15410)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 15412)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15413)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15414)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15415)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 15416 && n < 15421))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15420 && n < 15426) || (n > 15426 && n < 15431) || (n > 15431 && n < 15469))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 15468 && n < 15547))
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
.if((n >= 15547) && (n < 15586))
; 15547 to 15585
.if((n > 15546 && n < 15586))
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
.elseif(n = 15595 || n = 15596)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15601)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15602)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15603)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15604)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15605)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-246)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15607)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15608)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #159
	lda #64
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
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
	pha
	tya
	pha
	ldy #9
	lda #6
	jsr delay_256a_16_clocks_b
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15617)
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #35
	jsr delay_a_27_clocks
	pla
	iny
	Jne *-9
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15623)
	; 18 bytes
	php
	pha
	tya
	ldy #188
	pha
	lda #42
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15624)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #42
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15628)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #57
	lda #235
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15629)
	; 18 bytes
	php
	pha
	tya
	ldy #223
	pha
	lda #29
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15630)
	; 18 bytes
	php
	pha
	tya
	ldy #233
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15631)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #60
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15632)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #153
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #6
	jsr delay_256a_16_clocks_b
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 15635)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-147)/121
	lda #233 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15636)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #223
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #214
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15642)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #27
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #137
	lda #80
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 15645 to 15664
.if(n = 15645 || n = 15647 || n = 15649 || n = 15650 || n = 15652 || n = 15654 || n = 15657 || n = 15659 || n = 15661 || n = 15664)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15646)
	; 11 bytes
	php
	pha
	lda $A5
	lda #61
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15648)
	; 9 bytes
	php
	pha
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15651)
	; 11 bytes
	php
	pha
	lda $A5
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15653)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 15656)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15658)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15660)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15662)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15663)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.else
; 15665 to 15703
.if(n = 15665)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15666)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 15668)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15669)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15670)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15671)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 15672 && n < 15677))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15676 && n < 15682) || (n > 15682 && n < 15687) || (n > 15687 && n < 15704))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 15703 && n < 15782))
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
.else
.if((n >= 15782) && (n < 15860))
; 15782 to 15859
.if((n > 15781 && n < 15849))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 15851 || n = 15852)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15857)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 15858)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15859)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15861)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	ldy #233
	pha
	lda #27
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15864)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #184
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 15865)
	; 18 bytes
	php
	pha
	tya
	ldy #139
	pha
	lda #73
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #217
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15868)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #178
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15869)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15870)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15871)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15872)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #139
	lda #80
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	clc
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
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
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
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
	; 18 bytes
	php
	pha
	tya
	ldy #230
	pha
	lda #28
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 15890)
	; 18 bytes
	php
	pha
	tya
	ldy #58
	pha
	lda #228
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #126
	pha
	lda #84
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #230
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 15897)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #58
	lda #235
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 15898)
	; 18 bytes
	php
	pha
	tya
	ldy #237
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
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
.elseif(n = 15900)
	; 19 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #61
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15901 || n = 15903 || n = 15905 || n = 15906 || n = 15908 || n = 15910 || n = 15913 || n = 15915 || n = 15917 || n = 15920)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 15902)
	; 11 bytes
	php
	pha
	lda $A5
	lda #62
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15904)
	; 9 bytes
	php
	pha
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15907)
	; 11 bytes
	php
	pha
	lda $A5
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15909)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 15912)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 15914)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 15916)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 15918)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 15919)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15921)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15922)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 15924)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15925)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 15926)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 15927)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 15928 && n < 15933))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 15932 && n < 15938))
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
.elseif((n > 15938 && n < 15943) || (n > 15943 && n < 16094))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 16107 || n = 16108)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
; 16113 to 16132
.if(n = 16113)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16114)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16115)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16116)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16117)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16118)
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
.elseif(n = 16119)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16120)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #165
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16125)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #179
	pha
	lda #49
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #131
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
	; 18 bytes
	php
	pha
	tya
	ldy #55
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #179
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16141)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #227
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #126
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jpl *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #215
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #62
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16154)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #29
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16155)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16156)
	; 18 bytes
	php
	pha
	tya
	ldy #163
	pha
	lda #58
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16157 || n = 16159 || n = 16161 || n = 16162 || n = 16164 || n = 16166 || n = 16169 || n = 16171)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 16158)
	; 11 bytes
	php
	pha
	lda $A5
	lda #63
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16160)
	; 9 bytes
	php
	pha
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16163)
	; 11 bytes
	php
	pha
	lda $A5
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16165)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16170)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.endif
.endif
.endif
.else
; 16172 to 16250
.if(n = 16172)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16173 || n = 16176)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 16174)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16175)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16177)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16178)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 16180)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16181)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16182)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16183)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 16184 && n < 16189))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16188 && n < 16194) || (n > 16194 && n < 16199) || (n > 16199 && n < 16251))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 16250 && n < 16329))
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
.elseif(n = 16363 || n = 16364)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16369)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16370)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16371)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16372)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16373)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	ldy #188
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16376)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #218
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #133
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #141
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
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
	pha
	tya
	ldy #139
	dey
	Jne *-1
	tay
	pla
	Jcs *-11
	pla
	plp
.elseif(n = 16392)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #167
	lda #64
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #91
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #63
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16406)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #<(n/2-15)
	lda #>(n/2-15)
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
.endif
.else
.if((n >= 16407) && (n < 16485))
.if((n >= 16407) && (n < 16446))
.if((n >= 16407) && (n < 16426))
; 16407 to 16425
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
.elseif(n = 16408 || n = 16413 || n = 16415 || n = 16417 || n = 16418 || n = 16420 || n = 16422 || n = 16425)
	; 10..19 bytes
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #30
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16411)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16412)
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #56
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16414)
	; 11 bytes
	php
	pha
	lda $A5
	lda #64
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16416)
	; 9 bytes
	php
	pha
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16419)
	; 11 bytes
	php
	pha
	lda $A5
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16421)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 16424)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.else
; 16426 to 16445
.if(n = 16426)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16427 || n = 16429 || n = 16432)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 16428)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16430)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16431)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16433)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16434)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 16436)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16437)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16438)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16439)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 16440 && n < 16445))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16445)
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
.else
; 16446 to 16484
.if((n > 16445 && n < 16450) || (n > 16450 && n < 16455) || (n > 16455 && n < 16485))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.endif
.else
; 16485 to 16562
.if((n > 16484 && n < 16563))
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
.if((n >= 16563) && (n < 16719))
.if((n >= 16563) && (n < 16641))
; 16563 to 16640
.if((n > 16562 && n < 16617))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 16619 || n = 16620)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16625)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16626)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16627)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16628)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16629)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #205
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16632)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #163
	jsr delay_a_27_clocks
	lda #64
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
	lda #$2A ;hides 'rol a'
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
	; 18 bytes
	php
	pha
	tya
	ldy #60
	pha
	lda #236
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16642)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #175
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #60
	lda #243
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16647)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #56 ;hides 'sec'
	pha
	lda #188
	jsr delay_a_27_clocks
	pla
	iny
	Jpl *-9
	tay
	pla
	plp
.elseif(n = 16650)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16651)
	; 18 bytes
	php
	pha
	tya
	ldy #98
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
.elseif(n = 16652)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #163
	lda #68
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #170
	jsr delay_a_27_clocks
	lda #64
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #131
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16659)
	; 18 bytes
	php
	pha
	tya
	ldy #130
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/106
	pha
	lda #65
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #131
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16664)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #177
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #31
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16667)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16668)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #157
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16669 || n = 16671 || n = 16673 || n = 16674 || n = 16676 || n = 16678)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 16670)
	; 11 bytes
	php
	pha
	lda $A5
	lda #65
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16672)
	; 9 bytes
	php
	pha
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16675)
	; 11 bytes
	php
	pha
	lda $A5
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16677)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.if(n = 16680)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16681 || n = 16683 || n = 16685 || n = 16688)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 16682)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16684)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16686)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16687)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16689)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16690)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 16692)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16693)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16694)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16695)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 16696 && n < 16701))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16700 && n < 16706) || (n > 16706 && n < 16711) || (n > 16711 && n < 16719))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 16718 && n < 16873))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16881)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 16882)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16883)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16884)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16885)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16886)
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
.elseif(n = 16887)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16888)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #248 ;hides 'sed'
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 16889)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #231
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
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
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
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
	lda #$2A ;hides 'rol a'
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #165
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #65
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16910)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 16912 || n = 16913)
	; 19 bytes
	delay_n (n-2)
	nop
.elseif(n = 16914)
	; 18 bytes
	php
	pha
	tya
	ldy #155
	pha
	lda #68
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
.if((n >= 16915) && (n < 16934))
; 16915 to 16933
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #133
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #169
	pha
	lda #59
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 16920 || n = 16925 || n = 16927 || n = 16929 || n = 16930 || n = 16932)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #32
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16923)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16924)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #238
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 16926)
	; 11 bytes
	php
	pha
	lda $A5
	lda #66
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16928)
	; 9 bytes
	php
	pha
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16931)
	; 11 bytes
	php
	pha
	lda $A5
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16933)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 16934 to 16953
.if(n = 16934 || n = 16937 || n = 16939 || n = 16941 || n = 16944)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 16936)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 16938)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 16940)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 16942)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 16943)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16945)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16946)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 16948)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16949)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 16950)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 16951)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.else
; 16954 to 17031
.if((n > 16953 && n < 16957))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 16956 && n < 16962) || (n > 16962 && n < 16967) || (n > 16967 && n < 17032))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 17031 && n < 17110))
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
.elseif(n = 17131 || n = 17132)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17137)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17138)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17139)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17140)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17141)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17142)
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
.elseif(n = 17143)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17144)
	; 18 bytes
	php
	pha
	tya
	ldy #137
	pha
	lda #84
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
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
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17148)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
.if((n >= 17149) && (n < 17168))
; 17149 to 17167
.if(n = 17149)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17157)
	; 18 bytes
	php
	pha
	tya
	ldy #209
	pha
	lda #41
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #61
	lda #247
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 17168 to 17187
.if(n = 17168)
	; 19 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	pha
	lda #201
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-10
	tay
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #85
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17171)
	; 18 bytes
	php
	pha
	tya
	ldy #0
	pha
	lda #$2A ;hides 'rol a'
	clc
	Jne *-2
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17172)
	; 19 bytes
	php
	pha
	lda #87
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17173)
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #66
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #10
	lda #6
	jsr delay_256a_16_clocks_b
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
.elseif(n = 17176)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #245
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #33
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17179)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17180)
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/131
	pha
	lda #90
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17181 || n = 17183 || n = 17185 || n = 17186)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 17182)
	; 11 bytes
	php
	pha
	lda $A5
	lda #67
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17184)
	; 9 bytes
	php
	pha
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17187)
	; 11 bytes
	php
	pha
	lda $A5
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.endif
.endif
.else
.if((n >= 17188) && (n < 17344))
; 17188 to 17343
.if(n = 17188 || n = 17190 || n = 17193 || n = 17195 || n = 17197 || n = 17200)
	; 11..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17189)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 17192)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17194)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17196)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17198)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17199)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17201)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17202)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 17204)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17205)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17206)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17207)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 17208 && n < 17213))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17212 && n < 17218) || (n > 17218 && n < 17223) || (n > 17223 && n < 17344))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 17387 || n = 17388)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17393)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17394)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17395)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17396)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17397)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17398)
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
.elseif(n = 17399)
	; 18 bytes
	php
	pha
	tya
	ldy #220
	pha
	lda #38
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17400)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #238
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	; 18 bytes
	php
	pha
	tya
	ldy #183
	pha
	lda #54
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17405)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #235
	pha
	lda #(n-546)/511
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17410)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #212
	lda #48
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17411)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #183
	lda #61
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17412)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #138 ;hides 'txa'
	lda #90
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #108
	lda #$69 ;hides 'adc #$38'
	sec ;first loop only
	Jne *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17415)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17416)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #235
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17417)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #187
	lda #59
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #245
	lda #37
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
	; 18 bytes
	php
	pha
	tya
	ldy #136
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #67
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17432 || n = 17436 || n = 17437 || n = 17439)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 17433)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #69
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17434)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #34
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17435)
	; 9 bytes
	php
	pha
	lda #68
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17438)
	; 11 bytes
	php
	pha
	lda $A5
	lda #68
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.if(n = 17441 || n = 17442 || n = 17444 || n = 17446 || n = 17449 || n = 17451 || n = 17453 || n = 17456)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17443)
	; 11 bytes
	php
	pha
	lda $A5
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17445)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 17448)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17450)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17452)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17454)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17455)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17457)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17458)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 17461 to 17500
.if(n = 17461)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17462)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17463)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 17464 && n < 17469))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17468 && n < 17474) || (n > 17474 && n < 17479) || (n > 17479 && n < 17501))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 17500 && n < 17641))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 17643 || n = 17644)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17649)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17650)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17651)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17652)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17653)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17654)
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
.elseif(n = 17655)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #61
	lda #255
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17656)
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #171
	jsr delay_a_27_clocks
	lda #(n-248)/256
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17661)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #168
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #252
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17667)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17670)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #162
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17671)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #(n-19)/91
	pha
	lda #50
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #68
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #61
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
.elseif(n = 17682 || n = 17693 || n = 17695)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 17683)
	; 18 bytes
	php
	pha
	tya
	ldy #138
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
	; 18 bytes
	php
	pha
	tya
	ldy #242
	pha
	lda #32
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #203
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #35
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17691)
	; 9 bytes
	php
	pha
	lda #69
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17692)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #146
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17694)
	; 11 bytes
	php
	pha
	lda $A5
	lda #69
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
.elseif(n = 17697 || n = 17698 || n = 17700 || n = 17702 || n = 17705 || n = 17707 || n = 17709 || n = 17712)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17699)
	; 11 bytes
	php
	pha
	lda $A5
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17701)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 17704)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17706)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17708)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17710)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17711)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17713)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17714)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 17716)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17717)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17718)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17719)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 17720 && n < 17725))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17724 && n < 17730) || (n > 17730 && n < 17735))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif((n > 17735 && n < 17813))
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
.if((n >= 17813) && (n < 17969))
.if((n >= 17813) && (n < 17891))
; 17813 to 17890
.if((n > 17812 && n < 17891))
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
.elseif(n = 17899 || n = 17900)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17905)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 17906)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17907)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17908)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17909)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #245
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17912)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-297)/271
	lda #237
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17922)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #221
	lda #47
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17928)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17932)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #206
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17936)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #199
	lda #56
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 17937)
	; 18 bytes
	php
	pha
	tya
	ldy #62
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #206
	pha
	lda #46
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 17942)
	; 19 bytes
	php
	pha
	lda #89
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #69
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17943)
	; 19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #36
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #206
	lda #53
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.endif
.else
; 17949 to 17968
.if(n = 17949 || n = 17951 || n = 17953 || n = 17954 || n = 17956 || n = 17958 || n = 17961 || n = 17963 || n = 17965 || n = 17968)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 17950)
	; 11 bytes
	php
	pha
	lda $A5
	lda #70
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17952)
	; 9 bytes
	php
	pha
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17955)
	; 11 bytes
	php
	pha
	lda $A5
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17957)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 17960)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 17962)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 17964)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 17966)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 17967)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.endif
.endif
.endif
.endif
.else
; 17969 to 18125
.if(n = 17969)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17970)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 17972)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17973)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 17974)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 17975)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 17976 && n < 17981))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 17980 && n < 17986) || (n > 17986 && n < 17991) || (n > 17991 && n < 18126))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.elseif(n = 18155 || n = 18156)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18161)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18162)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18163)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18164)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
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
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18166)
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
.elseif(n = 18167)
	; 18 bytes
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #107
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-9
	tay
	pla
	plp
.elseif(n = 18168)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #193
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18169)
	; 18 bytes
	php
	pha
	tya
	ldy #242
	pha
	lda #34
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #85
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #150
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18177)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18178)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #143
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 18181)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18182)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #204
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18183)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #(n-297)/271
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
	; 18 bytes
	php
	pha
	tya
	ldy #62
	pha
	lda #252
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #143
	lda #93
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18188)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
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
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #70
	jsr delay_256a_16_clocks_b
	pla
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
	pha
	tya
	pha
	ldy #10 ;hides 'asl a'
	lda #7
	jsr delay_256a_16_clocks_b
	dey
	Jne *-7
	pla
	tay
	pla
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
	; 19 bytes
	delay_n (n-2)
	nop
.elseif(n = 18202)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #37
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
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
.if(n = 18204)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #88
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18205 || n = 18207 || n = 18209 || n = 18210 || n = 18212 || n = 18214 || n = 18217 || n = 18219 || n = 18221 || n = 18224)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 18206)
	; 11 bytes
	php
	pha
	lda $A5
	lda #71
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18208)
	; 9 bytes
	php
	pha
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18211)
	; 11 bytes
	php
	pha
	lda $A5
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18213)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 18216)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18218)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18220)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18222)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18223)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18225)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18226)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 18228)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18229)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18230)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18231)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 18232 && n < 18237))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 18236 && n < 18242) || (n > 18242 && n < 18247) || (n > 18247 && n < 18282))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 18281 && n < 18360))
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
.if((n >= 18360) && (n < 18399))
; 18360 to 18398
.if((n > 18359 && n < 18399))
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
.elseif(n = 18411 || n = 18412)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18417)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.endif
.else
; 18418 to 18437
.if(n = 18418)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18419)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18420)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18421)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
.elseif(n = 18424)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #56
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #50
	jsr delay_a_27_clocks
	iny
	Jne *-7
	pla
	tay
	pla
	plp
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
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #233
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
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
	lda #(n-6)/256
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #196
	lda #60
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18458)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #38
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18459)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18460)
	; 19 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #71
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18461 || n = 18463 || n = 18465 || n = 18466 || n = 18468 || n = 18470 || n = 18473 || n = 18475)
	; 10..13 bytes
	delay_n (n-2)
	nop
.elseif(n = 18462)
	; 11 bytes
	php
	pha
	lda $A5
	lda #72
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18464)
	; 9 bytes
	php
	pha
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18467)
	; 11 bytes
	php
	pha
	lda $A5
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18469)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18474)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18476)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.endif
.endif
.else
; 18477 to 18515
.if(n = 18477 || n = 18480)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 18478)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18479)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18481)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18482)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 18484)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18485)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18486)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18487)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 18488 && n < 18493))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 18492 && n < 18498) || (n > 18498 && n < 18503) || (n > 18503 && n < 18516))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 18515 && n < 18594))
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
.else
.if((n >= 18594) && (n < 18672))
; 18594 to 18671
.if((n > 18593 && n < 18665))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 18667 || n = 18668)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18673)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18674)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18675)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18676)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18677)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	nop
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-247)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18680)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #140
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 18681)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #205
	lda #57
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #166
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #42 ;hides 'rol a'
	lda #181
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.elseif(n = 18687)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #170
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18688)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #217
	lda #52
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18689)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #157
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18690)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #158
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 18691 to 18710
.if(n = 18691)
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	pha
	tya
	ldy #159
	dey
	Jne *-1
	tay
	pla
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
	lda #(n-264)/271
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18693)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #161
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #164
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #72
	jsr delay_256a_16_clocks_b
	pla
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
	php
	pha
	lda #$4A ;hides 'lsr a'
	delay_n 2331
	Jne *-10
	pla
	plp
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
	php
	pha
	tya
	ldy #248 ;hides 'sed'
	pha
	lda #9
	jsr delay_256a_16_clocks_b
	pla
	iny
	Jne *-9
	tay
	pla
	plp
.elseif(n = 18706)
	; 19 bytes
	delay_n (n-2)
	nop
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
	; 19 bytes
	php
	pha
	tya
	ldy #197 ;hides 'cmp zp'
	nop
	pha
	lda #223
	jsr delay_a_27_clocks
	pla
	dey
	Jmi *-10
	tay
	pla
	plp
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
.if((n >= 18711) && (n < 18731))
; 18711 to 18730
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
.elseif(n = 18713 || n = 18717 || n = 18719 || n = 18721 || n = 18722 || n = 18724 || n = 18726 || n = 18729)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 18714)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #39
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18715)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18716)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #210
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18718)
	; 11 bytes
	php
	pha
	lda $A5
	lda #73
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18720)
	; 9 bytes
	php
	pha
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18723)
	; 11 bytes
	php
	pha
	lda $A5
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18725)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
.elseif(n = 18728)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 18730)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.endif
.else
; 18731 to 18750
.if(n = 18731 || n = 18733 || n = 18736)
	; 13..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 18732)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18734)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18735)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18737)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18738)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 18740)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18741)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18742)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18743)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 18744 && n < 18749))
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
.endif
.else
.if((n >= 18751) && (n < 19376))
.if((n >= 18751) && (n < 19063))
.if((n >= 18751) && (n < 18907))
; 18751 to 18906
.if((n > 18750 && n < 18754) || (n > 18754 && n < 18759) || (n > 18759 && n < 18907))
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
.endif
.else
.if((n >= 18907) && (n < 18985))
.if((n >= 18907) && (n < 18946))
; 18907 to 18945
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
.elseif(n = 18923 || n = 18924)
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
.elseif(n = 18926)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18929)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 18930)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18931)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18932)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18933)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18934)
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
.elseif(n = 18935)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #191
	lda #65
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18936)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #155
	lda #88
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18937)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #164
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #195
	lda #63
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #163
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
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
	; 19 bytes
	delay_n (n-2)
	nop
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
	lda #(n-277)/256
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
	; 19 bytes
	php
	pha
	lda #90
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #73
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #40
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 18971)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18972 || n = 18973 || n = 18975 || n = 18977 || n = 18978 || n = 18980 || n = 18982)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 18974)
	; 11 bytes
	php
	pha
	lda $A5
	lda #74
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18976)
	; 9 bytes
	php
	pha
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18979)
	; 11 bytes
	php
	pha
	lda $A5
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18981)
	; 12 bytes
	php
	pha
	nop
	lda $A5
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
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.endif
.endif
.endif
.else
; 18985 to 19062
.if(n = 18985 || n = 18987 || n = 18989 || n = 18992)
	; 12..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 18986)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 18988)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 18990)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 18991)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18993)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18994)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 18996)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18997)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 18998)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 18999)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 19000 && n < 19005))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 19004 && n < 19010) || (n > 19010 && n < 19015) || (n > 19015 && n < 19063))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 19062 && n < 19141))
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
	; 16 bytes
	delay_n (n-4)
	delay_n 4
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19185)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19186)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19187)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19188)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19189)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19190)
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
.elseif(n = 19191)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19192)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #73
	lda #225
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
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
.elseif(n = 19198)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #169
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
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
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #171
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
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
	ldy #(n-19)/106
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
	; 18 bytes
	php
	pha
	tya
	ldy #156
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19212)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #181
	lda #72
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19213)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
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
	Jpl *-8
	tay
	pla
	plp
.endif
.endif
.endif
.endif
.else
.if((n >= 19219) && (n < 19297))
.if((n >= 19219) && (n < 19258))
.if((n >= 19219) && (n < 19238))
; 19219 to 19237
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
	ldy #(n-19)/91
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #45
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #41
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19227)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19228)
	; 19 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #74
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19229 || n = 19231 || n = 19233 || n = 19234 || n = 19236)
	; 10..12 bytes
	delay_n (n-2)
	nop
.elseif(n = 19230)
	; 11 bytes
	php
	pha
	lda $A5
	lda #75
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19232)
	; 9 bytes
	php
	pha
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19235)
	; 11 bytes
	php
	pha
	lda $A5
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19237)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 19238 to 19257
.if(n = 19238 || n = 19241 || n = 19243 || n = 19245 || n = 19248)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 19240)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19242)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19244)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19246)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19247)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19249)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19250)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 19252)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19253)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19254)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19255)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.endif
.else
; 19258 to 19296
.if((n > 19257 && n < 19261))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 19260 && n < 19266) || (n > 19266 && n < 19271) || (n > 19271 && n < 19297))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 19296 && n < 19376))
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
.else
.if((n >= 19376) && (n < 19688))
.if((n >= 19376) && (n < 19532))
.if((n >= 19376) && (n < 19454))
; 19376 to 19453
.if((n > 19375 && n < 19433))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
	jsr delay_256a_16_clocks_b
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
.elseif(n = 19435 || n = 19436)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19441)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19442)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19443)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19444)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19445)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19446)
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
.elseif(n = 19447)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19448)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #249
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #185
	lda #71
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19452)
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #209
	pha
	lda #52
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	ldy #243
	pha
	lda #39
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #169
	lda #81
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #56 ;hides 'sec'
	lda #234
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #46
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19467)
	; 18 bytes
	php
	pha
	tya
	ldy #221
	pha
	lda #47
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19468)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19469)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #169
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #171
	jsr delay_a_27_clocks
	lda #75
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19472)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #24 ;hides 'clc'
	lda #151
	jsr delay_a_27_clocks
	iny
	Jpl *-7
	pla
	tay
	pla
	plp
.endif
.else
; 19473 to 19492
.if(n = 19473)
	; 18 bytes
	php
	pha
	tya
	ldy #142
	pha
	lda #96
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
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
.elseif(n = 19476 || n = 19485 || n = 19487 || n = 19489 || n = 19490 || n = 19492)
	; 10..19 bytes
	delay_n (n-2)
	nop
.elseif(n = 19477)
	; 18 bytes
	php
	pha
	tya
	ldy #207
	pha
	lda #(n-344)/361
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19478)
	; 19 bytes
	php
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
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #42
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19483)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19484)
	; 18 bytes
	php
	pha
	txa
	pha
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	ldx #168
	dex
	Jne *-1
	Jcs *-7
	pla
	tax
	pla
	plp
.elseif(n = 19486)
	; 11 bytes
	php
	pha
	lda $A5
	lda #76
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19488)
	; 9 bytes
	php
	pha
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19491)
	; 11 bytes
	php
	pha
	lda $A5
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.else
; 19493 to 19531
.if(n = 19493)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19494 || n = 19497 || n = 19499 || n = 19501 || n = 19504)
	; 12..14 bytes
	delay_n (n-2)
	nop
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
.elseif(n = 19496)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19498)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19500)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19502)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19503)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19505)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19506)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
.elseif(n = 19508)
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19509)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19510)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19511)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 19512 && n < 19517))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 19516 && n < 19522) || (n > 19522 && n < 19527) || (n > 19527 && n < 19532))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 19531 && n < 19688))
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
.elseif(n = 19691 || n = 19692)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19697)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19698)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19699)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19700)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19701)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
	jsr delay_a_27_clocks
	lda #(n-245)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19702)
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
.elseif(n = 19703)
	; 18 bytes
	php
	pha
	tya
	ldy #148
	pha
	lda #92
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
	pla
	plp
.elseif(n = 19704)
	; 18 bytes
	php
	pha
	tya
	ldy #155
	pha
	lda #86
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #74 ;hides 'lsr a'
	lda #230
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 19709)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #243
	lda #(n-392)/411
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #159
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
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
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #166
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	pha
	tya
	ldy #168
	dey
	Jne *-1
	tay
	pla
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
	; 19 bytes
	php
	pha
	lda #83
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 19 bytes
	php
	pha
	lda #88
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #76
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19734 || n = 19741 || n = 19743 || n = 19745)
	; 10..19 bytes
	delay_n (n-2)
	nop
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #43
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19739)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19740)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #106 ;hides 'ror a'
	lda #150
	jsr delay_a_27_clocks
	dey
	Jne *-7
	pla
	tay
	pla
	plp
.elseif(n = 19742)
	; 11 bytes
	php
	pha
	lda $A5
	lda #77
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19744)
	; 9 bytes
	php
	pha
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.else
; 19746 to 19765
.if(n = 19746 || n = 19748 || n = 19750 || n = 19753 || n = 19755 || n = 19757 || n = 19760)
	; 10..14 bytes
	delay_n (n-2)
	nop
.elseif(n = 19747)
	; 11 bytes
	php
	pha
	lda $A5
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19749)
	; 12 bytes
	php
	pha
	nop
	lda $A5
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif(n = 19752)
	; 13 bytes
	delay_n (n-8)
	delay_n 8
.elseif(n = 19754)
	; 13 bytes
	delay_n 10
	delay_n (n-10)
.elseif(n = 19756)
	; 12 bytes
	delay_n (n-12)
	jsr @nearby_rts
.elseif(n = 19758)
	; 12 bytes
	delay_n (n-14)
	jsr @nearby_rts_14cyc
.elseif(n = 19759)
	; 14 bytes
	php
	pha
	lda #<(n-5) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-5)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19761)
	; 14 bytes
	php
	pha
	lda $A5
	jsr @nearby_rts_14cyc
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19762)
	; 14 bytes
	php
	pha
	lda #<(n-50)
	jsr delay_256a_16_clocks_b
	lda #>(n-50)
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
	; 14 bytes
	php
	pha
	lda #<(n-10) ;hides 'rol a'
	clc
	Jpl *-2
	lda #>(n-10)
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19765)
	; 14 bytes
	php
	pha
	clc
	lda #<(n-11) ;hides 'rol a'
	Jcc *-1
	lda #>(n-11)
	jsr delay_256a_16_clocks_b
	pla
	plp
.endif
.endif
.endif
.else
; 19766 to 19843
.if(n = 19766)
	; 15 bytes
	php
	pha
	lda $A5
	lda #<(n-44) ;hides 'asl a'
	Jpl *-1
	lda #>(n-44)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19767)
	; 15 bytes
	php
	pha
	clc
	clc
	lda #<(n-13) ;hides 'rol a'
	Jcc *-1
	lda #>(n-13)
	jsr delay_256a_16_clocks_b
	pla
	plp
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
.elseif((n > 19768 && n < 19773))
	; 14 bytes
	php
	pha
	lda #<(n-56)
	jsr delay_a_27_clocks
	lda #>(n-56)
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif((n > 19772 && n < 19778) || (n > 19778 && n < 19783) || (n > 19783 && n < 19844))
	; 14 bytes
	php
	pha
	lda #<(n-61)
	jsr delay_a_27_clocks
	lda #>(n-61)
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
.if((n > 19843 && n < 19922))
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
.elseif(n = 19947 || n = 19948)
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
	; 17 bytes
	php
	pha
	php
	nop
	lda #170
	jsr delay_a_27_clocks
	lda #(n-240)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19953)
	; 17 bytes
	php
	pha
	php
	nop
	lda #171
	jsr delay_a_27_clocks
	lda #(n-241)/256
	jsr delay_256a_16_clocks_b
	plp
	pla
	plp
.elseif(n = 19954)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #169
	jsr delay_a_27_clocks
	lda #(n-242)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19955)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #170
	jsr delay_a_27_clocks
	lda #(n-243)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19956)
	; 17 bytes
	php
	pha
	jsr @nearby_rts
	lda #171
	jsr delay_a_27_clocks
	lda #(n-244)/256
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19957)
	; 17 bytes
	php
	pha
	jsr @nearby_rts_14cyc
	lda #170
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #162
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19960)
	; 18 bytes
	php
	pha
	tya
	ldy #69
	pha
	lda #248
	jsr delay_a_27_clocks
	pla
	dey
	Jne *-8
	tay
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #224
	lda #55
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	lda #$0A ;hides 'asl a'
	Jpl *-1
	lda #167
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #69
	lda #255
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #76
	lda #225
	jsr delay_a_27_clocks
	dey
	Jpl *-6
	pla
	tay
	pla
	plp
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
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #160
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
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
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #167
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19980)
	; 18 bytes
	php
	pha
	lda #$4A ;hides 'lsr a'
	Jne *-1
	lda #168
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
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
	ldy #(n-19)/121
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
	; 18 bytes
	php
	pha
	lda #$E9 ;hides 'sbc #$2A'
	rol a ;first loop only
	delay_n 2848
	Jcs *-11
	pla
	plp
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
	; 19 bytes
	php
	pha
	tya
	pha
	ldy #184 ;hides 'clv'
	lda #179
	jsr delay_a_27_clocks
	dey
	dey
	Jne *-8
	pla
	tay
	pla
	plp
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
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #165
	lda #$E9 ;hides 'sbc #$0A'
	asl a ;first loop only
	Jcs *-2
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19992)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #149
	lda #100
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19993)
	; 19 bytes
	php
	pha
	lda #92
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19994)
	; 18 bytes
	php
	pha
	tya
	pha
	ldy #0
	lda #44
	jsr delay_a_27_clocks
	dey
	Jne *-6
	pla
	tay
	pla
	plp
.elseif(n = 19995)
	; 9 bytes
	php
	pha
	lda #78
	jsr delay_256a_11_clocks_b_
	pla
	plp
.elseif(n = 19996)
	; 19 bytes
	php
	pha
	lda #95
	jsr delay_a_27_clocks
	lda #99
	jsr delay_a_27_clocks
	lda #77
	jsr delay_256a_16_clocks_b
	pla
	plp
.elseif(n = 19997 || n = 19999)
	; 10..11 bytes
	delay_n (n-2)
	nop
.elseif(n = 19998)
	; 11 bytes
	php
	pha
	lda $A5
	lda #78
	jsr delay_256a_11_clocks_b_
	pla
	plp
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
